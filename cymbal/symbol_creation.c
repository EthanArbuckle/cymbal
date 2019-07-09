//
//  symbol_mapping.c
//  cymbal
//
//  Created by Ethan Arbuckle on 12/12/17.
//  Copyright © 2017 Ethan Arbuckle. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>
#include "symbol_creation.h"
#import "symbol_storage.h"
#import "image_lookup.h"

int initial_mapping_has_finished;
int totalmethods = 0;
int totalclasses = 0;

struct method_t {
    SEL name;
    const char *types;
    IMP imp;
};

// Queue where all the symbolication magic happens
dispatch_queue_t _symbolication_worker_queue(void)
{
    static dispatch_queue_t __symbolication_worker_queue;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        __symbolication_worker_queue = dispatch_queue_create("com.ethanarbuckle.symbolicator.workerqueue", DISPATCH_QUEUE_SERIAL);
    });
    return __symbolication_worker_queue;
}

// Semaphore that will stop the main thread from executing (as long as we can)
dispatch_semaphore_t _symbolication_mainthread_semaphore(void)
{
    static dispatch_semaphore_t __symbolication_mainthread_semaphore;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        __symbolication_mainthread_semaphore = dispatch_semaphore_create(0);
    });
    return __symbolication_mainthread_semaphore;
}

#pragma mark - Symbol Parsing
// Map all images loaded into this process. This also installs a listener for any images added after runtime
void map_all_symbols(void *_context)
{
    clock_t start, end;
    start = clock();
    
    // Discover binary images and begin looking for symbols
    unsigned int image_count = 0;
    const char **discovered_images = all_loaded_images(&image_count);
    for (int idx = 0; idx < image_count; idx++)
    {
        map_symbols_from_image_at_path(discovered_images[idx]);
    }
    
    free(discovered_images);
    
    initial_mapping_has_finished = 1;
    
    // Listen for new images added after runtime
    _dyld_register_func_for_add_image(&image_added_to_runtime);
    
    // We're done, let the main thread continue
    dispatch_semaphore_signal(_symbolication_mainthread_semaphore());
    
    end = clock() - start;
    printf("mapped %d methods from %d classes in %f seconds\n", totalmethods, totalclasses, (double) end / CLOCKS_PER_SEC);
}

// Map every class in a given image
int map_symbols_from_image_at_path(const char *image_path)
{
    unsigned int class_count = 0;
    const char **class_names = objc_copyClassNamesForImage(image_path, &class_count);
    
    if (class_count < 1)
    {
        return -1;
    }
    
    for (int idx = 0; idx < class_count; idx++)
    {
        Class target_class = objc_getClass(class_names[idx]);
        map_symbols_from_class(target_class, image_path);
    }
    
    free(class_names);
    
    return 0;
}

// Maps every method in a given class
int map_symbols_from_class(Class target_class, const char *image_path)
{
    __atomic_fetch_add(&totalclasses, 1, __ATOMIC_SEQ_CST);
    
    const char *class_name = class_getName(target_class);
    
    // Copy instance method list and map each one
    unsigned int method_count = 0;
    Method *current_methods = class_copyMethodList(target_class, &method_count);
    
    for (int idx = 0; idx < method_count; idx++)
    {
        Method target_instance_method = current_methods[idx];
        create_symbol_for_method(target_instance_method, NO, image_path, class_name);
    }
    free(current_methods);
    
    // Class methods are found on the meta class object of a normal class
    Class meta_class = object_getClass((struct objc_object *)target_class);
    
    // Copy class method list and map
    current_methods = class_copyMethodList(meta_class, &method_count);
    for (int idx = 0; idx < method_count; idx++)
    {
        Method target_class_method = current_methods[idx];
        create_symbol_for_method(target_class_method, YES, image_path, class_name);
    }
    free(current_methods);
    
    return 0;
}

// Create and store a symbolicated symbol for a Method for later symbolication use
int create_symbol_for_method(Method method_to_cache, BOOL is_class_method, const char *image_path, const char *class_name)
{
    __atomic_fetch_add(&totalmethods, 1, __ATOMIC_SEQ_CST);
    
    struct ObjectiveCSymbol *current_symbol = (struct ObjectiveCSymbol *)malloc(sizeof(struct ObjectiveCSymbol));
    current_symbol->containing_image_path = image_path;
    current_symbol->is_class_method = (int)is_class_method;
    current_symbol->class_name = class_name;
    current_symbol->selector_name = (const char *)(void *)((struct method_t *)method_to_cache)->name;
    current_symbol->impl_address = (uint64_t)((struct method_t *)method_to_cache)->imp;
    store_symbol(current_symbol);
    
    return 0;
}

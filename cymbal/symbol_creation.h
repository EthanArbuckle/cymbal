//
//  symbol_mapping.h
//  cymbal
//
//  Created by Ethan Arbuckle on 12/12/17.
//  Copyright © 2017 Ethan Arbuckle. All rights reserved.
//

#ifndef symbol_mapping_h
#define symbol_mapping_h

#include <objc/runtime.h>

extern int initial_mapping_has_finished;

dispatch_queue_t _symbolication_worker_queue(void);
dispatch_semaphore_t _symbolication_mainthread_semaphore(void);

void map_all_symbols(void *_context); // void*_context is required because this is a dispatch_async_f callback
int map_symbols_from_image_at_path(const char *image_path);
int map_symbols_from_class(Class target_class, const char *image_path);
int create_symbol_for_method(Method method_to_cache, BOOL is_class_method, const char *image_path, const char *class_name);

#endif /* symbol_mapping_h */

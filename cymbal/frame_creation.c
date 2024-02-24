//
//  frame_creation.c
//  cymbal
//
//  Created by Ethan Arbuckle on 12/12/17.
//  Copyright © 2017 Ethan Arbuckle. All rights reserved.
//

#include <stdio.h> //sprintf
#include <stdlib.h> //malloc
#include <libgen.h> //basename
#include <string.h> //strnstr
#include <dlfcn.h> //dladdr
#include <inttypes.h> // PRIu64
#import "symbol_storage.h"
#import "cpp_demangler.h"
#include "frame_lookup_cache.h"

#define MAX_FRAME_LEN 512

const char *create_objc_frame_for_symbol(struct ObjectiveCSymbol *symbol, void *address) {
    if (!symbol || !symbol->class_name || !symbol->selector_name || !symbol->containing_image_path) {
        return NULL;
    }
    
    char *constructed_frame = (char *)malloc(MAX_FRAME_LEN * sizeof(char));
    if (constructed_frame == NULL) {
        return NULL;
    }
    
    char *image_name = basename((char *)symbol->containing_image_path);
    uint64_t function_return_offset = (uint64_t)address - symbol->impl_address;
    
    int written = snprintf(constructed_frame, MAX_FRAME_LEN, "%-35s 0x%016lx %s[%s %s] + %" PRIu64,
                               image_name,
                               (unsigned long)symbol->impl_address,
                               symbol->is_class_method ? "+" : "-",
                               symbol->class_name,
                               symbol->selector_name,
                               function_return_offset);

    if (written >= MAX_FRAME_LEN) {
        free(constructed_frame);
        return NULL;
    }
    
    return constructed_frame;
}

const char *create_c_symbol_name(void *address)
{
    // Fetch info about what is at this address
    Dl_info frame_information;
    if (dladdr((const void *)address, &frame_information) == 0) {
        return NULL;
    }

    return frame_information.dli_sname;
}

const char *create_c_frame_for_address(void *address)
{
    // Fetch info about what is at this address
    Dl_info frame_information;
    if (dladdr((const void *)address, &frame_information) == 0)
    {
        return NULL;
    }
    
    // Sometimes we garbage items that have populated symbol names but invalid bases.
    // Confirm dli_fbase is within the realm of possibilies
    if (frame_information.dli_fbase >= (void *)0x100000000 && frame_information.dli_sname != NULL) {
        // Get the executable name from the full image path
        char *image_name = basename((char *)frame_information.dli_fname);
        
        // Is it a mangled c++ symbol?
        const char *symbol_name = frame_information.dli_sname;
        if (symbol_name == NULL) {
            return NULL;
        }
    
        const char *demangled = NULL;
        if (strlen(symbol_name) >= 2 && strncmp(symbol_name, "_Z", 2) == 0) {
            // Demangle
            demangled = demangle_cpp_symbol(symbol_name);
            symbol_name = demangled;
        }
        
        // Determine offset
        uint64_t function_return_offset = (uint64_t)address - (uint64_t)frame_information.dli_saddr;
        int required_size = snprintf(NULL, 0, "%-35s 0x%016lx %s + %llu", image_name, (unsigned long)address, symbol_name, function_return_offset) + 1;
        
        char *constructed_frame = (char *)malloc(required_size * sizeof(char));
        if (constructed_frame == NULL) {
            printf("Failed to alloc mem for frame\n");
            return NULL;
        }
        
        snprintf(constructed_frame, required_size, "%-35s 0x%016lx %s + %llu", image_name, (unsigned long)address, symbol_name, function_return_offset);
        
        if (demangled) {
            free((void *)demangled);
        }
    
        return constructed_frame;
    }
    
    // Fallback to whatever dladdr returns
    return frame_information.dli_sname;
}

const char *symbolicated_frame_for_address(void *address)
{
    // Check cache to see if frame already exists for this address
    const char *cached_frame = cached_frame_for_address(address);
    if (cached_frame != NULL && strlen(cached_frame) > 0)
    {
        return cached_frame;
    }
    
    /* Create a frame identical to what +callStackSymbols would give us.
     This can contain multiple things:
     * symbolicated objective-c message
     * <redacted>
     * __objective-c_block
     
     If we discover a block or an already-symbolicated item (lack of <redacted>), return it without searching our own symbol cache
     */
    const char *frame = create_c_frame_for_address(address);
    if (frame == NULL || strncmp(frame, "<redacted>", 10) == 0)
    {
        // This is a redacted item. Attempt to generate a symbolicated frame by search symbol bank for an item closesly matching this address
        struct ObjectiveCSymbol *found_symbol = search_for_symbol_at_address(address);
        if (found_symbol != NULL)
        {
            // An objc symbol was found, throw away the first frame
            if (frame)
            {
                free((void *)frame);
            }
            
            // Generate the replacement
            frame = create_objc_frame_for_symbol(found_symbol, address);
        }
    }
    
    if (frame != NULL)
    {
        cache_frame_for_lookup_address(frame, address);
    }
    
    return frame;
}

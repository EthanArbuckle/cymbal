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
#import "symbol_storage.h"
#import "cpp_demangler.h"
#include "frame_lookup_cache.h"

#define MAX_FRAME_LEN 512

const char *create_objc_frame_for_symbol(struct ObjectiveCSymbol *symbol, void *address)
{
    // -/+[class selector]
    char objc_message_constructed[MAX_FRAME_LEN];
    const char *method_type_indicator = (symbol->is_class_method) ? "+" : "-";
    sprintf(objc_message_constructed, "%s[%s %s]", method_type_indicator, symbol->class_name, symbol->selector_name);
    
    char adr_and_objc_msg[MAX_FRAME_LEN];
    // We don't know where execution stopped so we'll put function entry address instead of return address
    sprintf(adr_and_objc_msg, "0x%016lx %s", (unsigned long)symbol->impl_address, objc_message_constructed);
    
    // binary_image + (spaces filling 35char gap) + adr_and_objc_msg + offset
    const char *constructed_frame = (const char *)malloc(MAX_FRAME_LEN * sizeof(char));
    char *image_name = basename((char *)symbol->containing_image_path);
    
    /* Offset is where we jumped out of this function, in relation to its entry point.
     We can determine it by substracting the return address (where we left the function) from the known entry point
     
     T result is the number following the + sign after the symbol name:
     "16  CoreFoundation                      0x00000001884962b8 CFRunLoopRunSpecific + 444"
     */
    int function_return_offset = (int)(address - symbol->impl_address);
    sprintf((char *)constructed_frame, "%-35s %s + %d", image_name, adr_and_objc_msg, function_return_offset);
    
    return constructed_frame;
}

const char *create_c_symbol_name(void *address)
{
    // Fetch info about what is at this address
    Dl_info frame_information;
    dladdr((const void *)address, &frame_information);
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
    if (frame_information.dli_fbase >= (void *)0x100000000 && frame_information.dli_sname != NULL)
    {
        // Get the executable name from the full image path
        char *image_name = basename((char *)frame_information.dli_fname);
        
        // Is it a mangled c++ symbol?
        const char *symbol_name = frame_information.dli_sname;
        if (strnstr(symbol_name, "_Z", 5) != NULL)
        {
            // Attempt demangle
            const char *demangled = demangle_cpp_symbol(symbol_name);
            if (demangled != NULL)
            {
                symbol_name = demangled;
            }
        }
        
        // Determine offset
        int function_return_offset = (int)(address - (uint64_t)frame_information.dli_saddr);
        
        const char *constructed_frame = (const char *)malloc(MAX_FRAME_LEN * sizeof(char));
        // image_name + (spaces filling 35char gap) + address + symbol + offset
        sprintf((char *)constructed_frame, "%-35s 0x%016lx %s + %d", image_name, (unsigned long)address, symbol_name, function_return_offset);
        return constructed_frame;
    }
    
    // Last resort
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
    if (frame == NULL || strstr(frame, "<redacted>") != NULL)
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

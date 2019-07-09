//
//  cymbal.c
//  cymbal
//
//  Created by Ethan Arbuckle on 12/12/17.
//  Copyright © 2017 Ethan Arbuckle. All rights reserved.
//

#import "cymbal.h"
#include "symbol_creation.h"
#include "callstack_creation.h"
#include "frame_creation.h"

#pragma mark - Internal
// A lookup cache for callers that will have repeat lookups of the same address.
// This differs from the other symbol lookup cache because it only includes class names or function names (not image or address)
static CFMutableDictionaryRef __lookup_cache(void)
{
    static CFMutableDictionaryRef _lookup_cache;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _lookup_cache = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, NULL, NULL);
    });
    return _lookup_cache;
}

__attribute__((constructor)) static void _symbolicator_injected()
{
    printf("cymbal starting\n");
    
    /* All of the symbol mapping happens on this background queue. It is unideal for the app to execute code until this mapping process has completed as it could lead
     to failed symbolication attempts (symbolicating an address that has not yet been mapped). The main thread will freeze and wait for either to happen:
     1) the mapping thread to signal that it has completed, and app execution can continue as normal.
     2) the mapping operation is taking too long, and we need to unfreeze the main thread so Watchdog doesn't kill the process.
     When this happens the background thread continues mapping but the app becomes active.
     */
    dispatch_async_f(_symbolication_worker_queue(), 0, &map_all_symbols);
    dispatch_semaphore_wait(_symbolication_mainthread_semaphore(), dispatch_time(DISPATCH_TIME_NOW, /* watchdog timeout */ 3 * NSEC_PER_SEC));
    
    // Create Objective-c bridge to use these functions
    // +[Cymbal callstack]
    Class Cymbal = objc_allocateClassPair(objc_getClass("NSObject"), "Cymbal", 0);
    objc_registerClassPair(Cymbal);
    
    IMP symbolicateImp = imp_implementationWithBlock((id)^(id _self, void *address) {
        
        const char *symbol = cymbolicate(address);
        CFStringRef cfsymbol = CFStringCreateWithCString(kCFAllocatorDefault, symbol, kCFStringEncodingUTF8);
        free((void *)symbol);
        return cfsymbol;
    });
    class_addMethod(objc_getMetaClass("Cymbal"), sel_registerName("symbolicate:"), symbolicateImp, "@@:Q");
    
    IMP callstackImp = imp_implementationWithBlock((id)^(id _self) {
        CFArrayRef callstack = cymbal_callstack();
        CFMutableArrayRef mutableCallstack = CFArrayCreateMutableCopy(kCFAllocatorDefault, 0, callstack);
        CFAutorelease(mutableCallstack);
        CFRelease(callstack);
        
        // remove the first frame because it is a Cymbal function
        CFArrayRemoveValueAtIndex(mutableCallstack, 0);
        return mutableCallstack;
    });
    class_addMethod(objc_getMetaClass("Cymbal"), sel_registerName("callstack"), callstackImp, "@@");
    
    // TODO: need a way to catch new classes/methods being created after launch using runtime functions. maybe need to interpose them?
}

#pragma mark - Public
EXPORT CFArrayRef cymbal_callstack(void)
{
    int default_callstack_length = 50;
    // Sometimes there are more frames available preceding start(), but I asked around and no one would find it useful
    int stop_at_start = 1;
    return create_symbolicated_callstack(default_callstack_length, stop_at_start);
}

EXPORT const char *cymbolicate(void *address)
{
    struct Cymbal *symbol = (struct Cymbal *)malloc(sizeof(struct Cymbal));
    if (cymbal_symbol_for_address(address, &symbol) != KERN_SUCCESS)
    {
        free(symbol);
        return NULL;
    }
    
    uint32_t max_len = 512;
    char *symname = (char *)malloc(max_len);
    if (symbol->type == cymbalTypeObjectiveC)
    {
        const char *method_type_indicator = (symbol->is_class_method) ? "+" : "-";
        snprintf(symname, max_len, "%s[%s %s]", method_type_indicator, symbol->class_name, symbol->method_name);
    }
    else if (symbol->type == cymbalTypeC)
    {
        snprintf(symname, max_len, "%s", symbol->symbol_name);
    }
    
    free(symbol);
    
    return symname;
}

EXPORT int cymbal_symbol_for_address(void *address, struct Cymbal **cymbal)
{
    struct Cymbal *return_symbol;
    
    // Before anything, check to see if this exact address has already been given to us
    return_symbol = (struct Cymbal *)CFDictionaryGetValue(__lookup_cache(), (const void *)address);
    if (return_symbol != NULL)
    {
        **cymbal = *return_symbol;
        return KERN_SUCCESS;
    }
    
    return_symbol = *cymbal;
    if (return_symbol == NULL)
    {
        return KERN_MEMORY_ERROR;
    }
    
    // Try objc first
    struct ObjectiveCSymbol *candidate = search_for_symbol_at_address(address);
    if (candidate != NULL)
    {
        return_symbol->type = cymbalTypeObjectiveC;
        return_symbol->class_name = candidate->class_name;
        return_symbol->method_name = candidate->selector_name;
        return_symbol->is_class_method = candidate->is_class_method;
    }
    else
    {
        // Didnt find it - fallback to generating a +callStackSymbols frame.
        // This doesn't always mean we're returning a redacted item; this could be a symbolicated C function or an objective-c block
        const char *symname = create_c_symbol_name(address);
        if (symname == NULL)
        {
            // Failed to create a c frame, possibly invalid address
            printf("failed to symbolicate 0x%llx\n", (unsigned long long)address);
            return KERN_FAILURE;
        }
        else
        {
            return_symbol->type = cymbalTypeC;
            return_symbol->symbol_name = symname;
        }
    }
    
    // Whatever our end result was, cache it so we don't need to do frame creation again
    struct Cymbal *cacheSymbol = malloc(sizeof(struct Cymbal));
    *cacheSymbol = *return_symbol;
    CFDictionarySetValue(__lookup_cache(), (const void *)address, cacheSymbol);
    
    return KERN_SUCCESS;
}


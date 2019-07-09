//
//  symbol_storage.c
//  cymbal
//
//  Created by Ethan Arbuckle on 12/12/17.
//  Copyright © 2017 Ethan Arbuckle. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>
#include <pthread.h>
#include "symbol_storage.h"

#pragma mark - Symbol Storage
// Where every symbol we map will be stored
static CFMutableDictionaryRef _symbol_freezer(void)
{
    static CFMutableDictionaryRef __symbol_freezer;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        __symbol_freezer = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, NULL, NULL);
    });
    return __symbol_freezer;
}

pthread_mutex_t *_symbol_freezer_lock(void)
{
    static pthread_mutex_t __symbol_freezer_lock;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        pthread_mutex_init(&__symbol_freezer_lock, 0);
    });
    return &__symbol_freezer_lock;
}

#pragma mark - Add/fetch symbols
// Store a given symbol
void store_symbol(struct ObjectiveCSymbol *symbol)
{
    // Thread safe insertion of symbol into symbol pool
    pthread_mutex_lock(_symbol_freezer_lock());
    CFDictionarySetValue(_symbol_freezer(), (const void *)symbol->impl_address, symbol);
    pthread_mutex_unlock(_symbol_freezer_lock());
}

// Attempt to locate a symbolicated object for a given address
struct ObjectiveCSymbol *search_for_symbol_at_address(void *function_address)
{
    // We are given a Method's return address, however what we have stored are Method's entry point addresses
    // We will look for a cached symbol within "max_method_size" of the given return address
    int max_method_size = 8192;
    for (int offset = 0; offset <= max_method_size; offset++)
    {
        // Thread safe lookup
        pthread_mutex_lock(_symbol_freezer_lock());
        struct ObjectiveCSymbol *cached_symbol = (struct ObjectiveCSymbol *)CFDictionaryGetValue(_symbol_freezer(), (const void *)(function_address - offset));
        pthread_mutex_unlock(_symbol_freezer_lock());
        
        // Did we find something matching this address? If not substract 1 from function_address and try again
        if (cached_symbol != NULL)
        {
            return cached_symbol;
        }
    }
    
    return NULL;
}

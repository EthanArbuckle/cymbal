//
//  frame_lookup_cache.c
//  cymbal
//
//  Created by Ethan Arbuckle on 12/12/17.
//  Copyright © 2017 Ethan Arbuckle. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>
#include <pthread.h>

// This stores "lookups", the symbolicated frame we created for an address.
// Frame creation is a slow operation, so we store them in case the address is symbolicated again
static CFMutableDictionaryRef _lookup_cache(void)
{
    static CFMutableDictionaryRef __symbol_freezer;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        __symbol_freezer = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, NULL, NULL);
    });
    return __symbol_freezer;
}

pthread_mutex_t *_lookup_cache_lock(void)
{
    static pthread_mutex_t __lookup_cache_lock;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        pthread_mutex_init(&__lookup_cache_lock, 0);
    });
    return &__lookup_cache_lock;
}

void cache_frame_for_lookup_address(const char *frame, void *address)
{
    // Thread safe storing
    pthread_mutex_lock(_lookup_cache_lock());
    
    // Store a copy of the pointer
    size_t framelen = strlen(frame) + 1;
    char *cachedptr = (char *)malloc(framelen);
    strncpy(cachedptr, frame, framelen);
    
    CFDictionarySetValue(_lookup_cache(), (const void *)address, cachedptr);
    pthread_mutex_unlock(_lookup_cache_lock());
}

// This will return NULL if nothing is cached for this address
const char *cached_frame_for_address(void *address)
{
    // Thread safe fetching
    pthread_mutex_lock(_lookup_cache_lock());
    
    const char *cached_frame = (const char *)CFDictionaryGetValue(_lookup_cache(), (const void *)address);
    
    if (cached_frame)
    {
        // Create a copy of the cached pointer
        size_t framelen = strlen(cached_frame) + 1;
        char *cachedptr = (char *)malloc(framelen);
        strncpy(cachedptr, cached_frame, framelen);
        cached_frame = cachedptr;
    }
    pthread_mutex_unlock(_lookup_cache_lock());
    
    return cached_frame;
}

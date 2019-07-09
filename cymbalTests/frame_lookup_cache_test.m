//
//  test_frame_lookup_cache.m
//  cymbalTests
//
//  Created by Ethan Arbuckle on 7/5/19.
//  Copyright © 2019 Ethan Arbuckle. All rights reserved.
//

#import <XCTest/XCTest.h>
#include "frame_lookup_cache.h"
#include "frame_creation.h"
#include <dlfcn.h>

@interface LookupCacheTests : XCTestCase
@end

@implementation LookupCacheTests

- (void)testFrameStorageRetrieval
{
    // A frame is generated
    const char *frame_to_cache = "libsystem_kernel.dylib              0x0000000106b2e1c4 __open + 0x00";
    void *address = (void *)0x00badf100d;
    
    // And stored in the cache
    cache_frame_for_lookup_address(frame_to_cache, address);
    
    // And then the cached frame is queried
    const char *cached_frame = cached_frame_for_address(address);

    // And it matches what was supplied into the cache
    NSString *a = [NSString stringWithUTF8String:frame_to_cache];
    NSString *b = [NSString stringWithUTF8String:cached_frame];
    NSAssert([a isEqualToString:b], @"The frame returned from the lookup cache did not match what was stored");
}

- (void)testFrameCreationUsedCachedFrame
{
    // An address is symbolicated
    void *address = dlsym(RTLD_DEFAULT, "open");
    const char *symbolicated = symbolicated_frame_for_address(address);
    NSAssert(strstr(symbolicated, "__open") != NULL, @"Pre-cache frame is not the expected value");
    free((void *)symbolicated);
    
    // Replace the cached frame with a fake value
    const char *cachedvalue = "cached frame";
    cache_frame_for_lookup_address(cachedvalue, address);
    
    // The same address is symbolicated again
    symbolicated = symbolicated_frame_for_address(address);
    
    // The faked cache entry was returned
    NSAssert(strcmp(symbolicated, cachedvalue) == 0, @"Post-cache frame is not the expected value");
    free((void *)symbolicated);
}

- (void)testFrameCreationCachedFrame
{
    // An address to symbolicate
    void *address = dlsym(RTLD_DEFAULT, "close");

    // And its not already cached
    NSAssert(cached_frame_for_address(address) == NULL, @"Cache already contains an entry for test function");

    // Is symbolicated
    const char *symbolicated = symbolicated_frame_for_address(address);
    NSAssert(strstr(symbolicated, "close +") != NULL, @"Pre-cache frame is not the expected value");
    free((void *)symbolicated);

    // And it added the frame to the cache
    NSAssert(cached_frame_for_address(address) != NULL, @"Frame did not get added to the lookup cache");
}

@end

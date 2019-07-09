//
//  image_lookup_tests.m
//  cymbalTests
//
//  Created by Ethan Arbuckle on 7/5/19.
//  Copyright © 2019 Ethan Arbuckle. All rights reserved.
//

#import <XCTest/XCTest.h>
#include "image_lookup.h"
#include <dlfcn.h>

@interface ImageLookupTests : XCTestCase
@end

@implementation ImageLookupTests

- (void)testNewImageAddedToRuntimeDetection
{
    // The number of images that Cymbal is currently aware of
    unsigned int current_image_count = 0;
    all_loaded_images(&current_image_count);
    
    // A new library is linked
    dlopen("/System/Library/Frameworks/AdSupport.framework/AdSupport", 0);
    
    // The number of images Cymbal is now aware of
    unsigned int new_image_count = 0;
    all_loaded_images(&new_image_count);
    
    // The new image(s) were detected and mapped
    NSAssert(new_image_count > current_image_count, @"Did not detect new images being added to the runtime");
}

- (void)testSystemImagesWereDetected
{
    // Get all mapped images from Cymbal
    unsigned int image_count = 0;
    const char **loaded_images = all_loaded_images(&image_count);
    
    // It contains entries
    NSAssert(image_count > 100, @"No images were mapped");
    
    int discovered_expected_images = 0;
    NSArray *expectedImages = @[@"AssertionServices", @"CoreSymbolication", @"libsystem_pthread.dylib"];
    for (NSString *expectedImage in expectedImages)
    {
        for (int i = 0; i < image_count; i++)
        {
            const char *curimg = loaded_images[i];
            if (strstr(curimg, [expectedImage UTF8String]) != NULL)
            {
                discovered_expected_images++;
            }
        }
    }
    
    // All expected system images were mapped
    NSAssert(discovered_expected_images == [expectedImages count], @"Did not map all expected system images");
}

@end

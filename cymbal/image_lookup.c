//
//  image_lookup.c
//  cymbal
//
//  Created by Ethan Arbuckle on 12/12/17.
//  Copyright © 2017 Ethan Arbuckle. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>
#include <mach-o/dyld.h>
#include <dlfcn.h>
#include "symbol_creation.h"

// c string from CFString (wow)
const char *utf8string(CFStringRef string)
{
    // length can like triple during conversion for some reason, CFStringGetCString will silently fail if the buffer is undersized
    CFIndex length = CFStringGetMaximumSizeForEncoding(CFStringGetLength(string), kCFStringEncodingUTF8);
    char *buffer = (char *)malloc(length);
    if (buffer != NULL) {
        CFStringGetCString(string, buffer, length, kCFStringEncodingUTF8);
    }
    
    return buffer;
}

// Contains images that we've already encounters and mapped
static CFMutableArrayRef _mapped_images(void)
{
    static CFMutableArrayRef __mapped_images;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        __mapped_images = CFArrayCreateMutable(kCFAllocatorDefault, 0, NULL);
    });
    return __mapped_images;
}

// Triggered when an image is added dynamically after runtime.
// When first registered, dyld enumerates all loaded images and passes them to the newly registered callback (this)
// This can begin before the initial symbol mapping process finishes, which would cause parallel mapping of duplicate images. To counter, this will
// ignore any images passed before the initial mapping process has completed
void image_added_to_runtime(const struct mach_header *mh, intptr_t vmaddr_slide) {
    if (initial_mapping_has_finished == 0) {
        return;
    }
    
    // If we don't already have this image mapped, map it
    Dl_info info;
    if (dladdr(mh, &info) == 0) {
        return;
    }
            
    if (!CFArrayContainsValue(_mapped_images(), CFRangeMake(0, CFArrayGetCount(_mapped_images())), info.dli_fname)) {
        map_symbols_from_image_at_path(info.dli_fname);
        
        // Store it so we don't remap it again accidently
        CFArrayAppendValue(_mapped_images(), info.dli_fname);
    }
}

// Returns a list of all loaded dependencies + application binary
const char **all_loaded_images(unsigned int *count)
{
    // Build a list of all images dyld has to offer us
    uint32_t image_count = _dyld_image_count();
    const char **images = (const char **)calloc(image_count + 1, sizeof(char *));
    if (images == NULL) {
        return NULL;
    }

    for (int idx = 0; idx < image_count; idx++) {
        const char *current_image = _dyld_get_image_name(idx);
        images[idx] = current_image;
        CFArrayAppendValue(_mapped_images(), current_image);
    }
    
    // Add the main application binary to the list
    CFURLRef bundlePath = CFBundleCopyExecutableURL(CFBundleGetMainBundle());
    CFStringRef stringBundlePath = CFURLGetString(bundlePath);
    
    const char *app_path = utf8string(stringBundlePath);
    if (app_path) {
        images[*count] = app_path;
        CFArrayAppendValue(_mapped_images(), app_path);
    }
    
    CFRelease(bundlePath);
    
    *count = image_count;
    return images;
}

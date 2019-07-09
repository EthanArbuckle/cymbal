//
//  image_lookup.h
//  cymbal
//
//  Created by Ethan Arbuckle on 12/12/17.
//  Copyright © 2017 Ethan Arbuckle. All rights reserved.
//
#include <mach-o/dyld.h>

#ifndef image_lookup_h
#define image_lookup_h

void image_added_to_runtime(const struct mach_header *mh, intptr_t vmaddr_slide);
const char **all_loaded_images(unsigned int *count);

#endif

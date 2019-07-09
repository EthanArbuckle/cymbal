//
//  frame_lookup_cache.h
//  cymbal
//
//  Created by Ethan Arbuckle on 12/12/17.
//  Copyright © 2017 Ethan Arbuckle. All rights reserved.
//

void cache_frame_for_lookup_address(const char *frame, void *address);
const char *cached_frame_for_address(void *address);

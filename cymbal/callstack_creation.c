//
//  callstack_creation.c
//  cymbal
//
//  Created by Ethan Arbuckle on 12/12/17.
//  Copyright © 2017 Ethan Arbuckle. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>
#include <execinfo.h>
#include "frame_creation.h"

#define MAX_CALLSTACK_DEPTH 128


CFArrayRef create_symbolicated_callstack(int depth, int nothing_precedes_start)
{
    CFMutableArrayRef mutable_callstack = CFArrayCreateMutable(kCFAllocatorDefault, 0, NULL);
    
    int callstack_depth = (MAX_CALLSTACK_DEPTH > depth) ? depth : MAX_CALLSTACK_DEPTH;
    
    // Retrieve list of return addresses
    void *address_history[MAX_CALLSTACK_DEPTH];
    backtrace(address_history, callstack_depth);
    
    for (int i = 0; i < callstack_depth; i++)
    {
        void *return_address = address_history[i];
        if (return_address == NULL)
        {
            break;
        }
        
        // Retrieve final symbolicated frame
        const char *frame = symbolicated_frame_for_address(return_address);
        if (frame != NULL)
        {
            if (strlen(frame) >= 1)
            {
                // Add it to callstack array in CFStringRef representation
                CFStringRef final_frame = CFStringCreateWithFormat(kCFAllocatorDefault, 0, CFSTR("%d  %s"), i, frame);
                CFAutorelease(final_frame);
                CFArrayAppendValue(mutable_callstack, final_frame);
            }
            
            if (nothing_precedes_start == 1)
            {
                // stop when "start() + 1" is reached
                if (strstr(frame, " start + 1") != NULL || strstr(frame, " start_wqthread +") != NULL)
                {
                    free((void *)frame);
                    break;
                }
            }
            
            free((void *)frame);
        }
    }
    
    CFArrayRef final_callstack = CFArrayCreateCopy(kCFAllocatorDefault, mutable_callstack);
    CFRelease(mutable_callstack);
    
    return final_callstack;
}

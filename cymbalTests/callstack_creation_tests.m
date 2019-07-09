//
//  callstack_creation_tests.m
//  cymbalTests
//
//  Created by Ethan Arbuckle on 7/5/19.
//  Copyright © 2019 Ethan Arbuckle. All rights reserved.
//

#import <XCTest/XCTest.h>
#include "callstack_creation.h"

@interface CallstackCreationTests : XCTestCase
@end

@implementation CallstackCreationTests


- (void)testCallstackCreationLength
{
    // Callstack with depth of 8
    int stop_at_start = 0;
    CFArrayRef callstack = create_symbolicated_callstack(8, stop_at_start);

    // Contains 8 frames
    NSAssert(CFArrayGetCount(callstack) == 8, @"Symbolicated callstack did not contain the expected amount of frames (8)");
    CFRelease(callstack);
    
    // Callstack with depth of 20
    callstack = create_symbolicated_callstack(20, stop_at_start);
    
    // Contains 20 frames
    NSAssert(CFArrayGetCount(callstack) == 20, @"Symbolicated callstack did not contain the expected amount of frames (20)");
    CFRelease(callstack);
    
    // Callstack with depth of 20 and does not have to stop at start()
    // this is not a useful test but it executes an additional codepath (stop_at_start==1) so test anyways
    stop_at_start = 0;
    callstack = create_symbolicated_callstack(20, stop_at_start);
    NSAssert(CFArrayGetCount(callstack) == 20, @"Symbolicated callstack did not contain the expected amount of frames (20)");
    CFRelease(callstack);
}

- (void)testCallstackCreationStartedWithMain
{
    // Callstack that contains as many frames as possible
    // Specified that nothing should precede start()
    int stop_at_start = 1;
    CFArrayRef callstack = create_symbolicated_callstack(50, stop_at_start);
    CFIndex framecnt = CFArrayGetCount(callstack);
    
    // The first frame is start()
    CFStringRef ff = CFArrayGetValueAtIndex(callstack, framecnt - 1);
    NSAssert(CFStringFind(ff, CFSTR("start +"), 0).location != kCFNotFound, @"First frame in callstack was not start()");
    
    // The second frame is main()
    CFStringRef sf = CFArrayGetValueAtIndex(callstack, framecnt - 2);
    NSAssert(CFStringFind(sf, CFSTR("main +"), 0).location != kCFNotFound, @"Second frame in callstack was not main()");
}

@end

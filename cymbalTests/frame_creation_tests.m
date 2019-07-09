//
//  cymbalTests.m
//  cymbalTests
//
//  Created by Ethan Arbuckle on 12/11/17.
//  Copyright © 2017 Ethan Arbuckle. All rights reserved.
//

#import <XCTest/XCTest.h>
#include "frame_creation.h"
#include <dlfcn.h>
#include <objc/runtime.h>


@interface FrameCreationTests : XCTestCase
@end

@implementation FrameCreationTests

- (void)testObjcInstanceMethodFrameCreation
{
    // An objective-c instance method
    struct ObjectiveCSymbol *test_symbol = malloc(sizeof(struct ObjectiveCSymbol));
    test_symbol->selector_name = "exampleMethod";
    test_symbol->class_name = "exampleClass";
    test_symbol->containing_image_path = "exampleImagePath";
    test_symbol->impl_address = 0;
    test_symbol->is_class_method = 0;
    
    // A frame is created
    const char *frame = create_objc_frame_for_symbol(test_symbol, 0);
    
    // And the frame meets expectations
    const char *expected = "exampleImagePath                    0x0000000000000000 -[exampleClass exampleMethod] + 0";
    NSAssert(strcmp(frame, expected) == 0, @"failed to create frame for an objective-c instance method");

    free((void *)frame);
}

- (void)testObjcClassMethodFrameCreation
{
    // An objective-c class method
    struct ObjectiveCSymbol *test_symbol = malloc(sizeof(struct ObjectiveCSymbol));
    test_symbol->selector_name = "exampleMethod";
    test_symbol->class_name = "exampleClass";
    test_symbol->containing_image_path = "exampleImagePath";
    test_symbol->impl_address = 0;
    test_symbol->is_class_method = 1;
    
    // A frame is created
    const char *frame = create_objc_frame_for_symbol(test_symbol, 0);
    
    free(test_symbol);
    
    // And the frame meets expectations
    const char *expected = "exampleImagePath                    0x0000000000000000 +[exampleClass exampleMethod] + 0";
    NSAssert(strcmp(frame, expected) == 0, @"failed to create frame for an objective-c class method");
    
    free((void *)frame);
}

- (void)testCSymbolFetching
{
    // The address of open()
    void *open_location = dlsym(RTLD_DEFAULT, "open");
    
    // Find the symbol name at the address
    const char *symbol = create_c_symbol_name(open_location);
    
    // It is "__open"
    NSAssert(strcmp(symbol, "__open") == 0, @"failed to fetch the symbol name of a c function");
}

- (void)testCFrameCreation
{
    // The address of open()
    void *open_location = dlsym(RTLD_DEFAULT, "open");
    
    // Find the symbol name at the address
    const char *frame = create_c_frame_for_address(open_location);
    
    NSString *expected = @"libsystem_kernel.dylib              placeholder __open + 0";
    expected = [expected stringByReplacingOccurrencesOfString:@"placeholder" withString:[NSString stringWithFormat:@"0x%016llx", (unsigned long long)open_location]];

    // It is "__open"
    NSAssert(strcmp(frame, expected.UTF8String) == 0, @"failed to fetch the symbol name of a c function");
}

- (void)testSymbolicatedFrameForAddressObjc
{
    // The address of a method that appears as <redacted> inside callstacks
    IMP redactedImp = class_getMethodImplementation(objc_getClass("NSFileManager"), sel_registerName("_processHasUbiquityContainerEntitlement"));
    
    // A symbolicated frame is created for the address
    const char *symbolicated_frame = symbolicated_frame_for_address((void *)redactedImp);
    
    // The symbolicated frame contains the plaintext (unredacted) class and method name
    const char *expected = "-[NSFileManager _processHasUbiquityContainerEntitlement]";
    NSAssert(strstr(symbolicated_frame, expected) != NULL, @"Failed to create symbolicated frame for an objective-c method address");
}

- (void)testSymbolicatedFrameForAddressC
{
    // The address of open()
    void *open_location = dlsym(RTLD_DEFAULT, "open");
    
    // A symbolicated frame is created for the address
    const char *symbolicated_frame = symbolicated_frame_for_address(open_location);
    
    // The symbolicated frame contains the plaintext (unredacted) class and method name
    const char *expected = "__open + 0";
    NSAssert(strstr(symbolicated_frame, expected) != NULL, @"Failed to create symbolicated frame for a c function address");
}

@end

//
//  cymbal_api_tests.m
//  cymbalTests
//
//  Created by Ethan Arbuckle on 7/7/19.
//  Copyright © 2019 Ethan Arbuckle. All rights reserved.
//

#import <XCTest/XCTest.h>
#include "cymbal.h"
#include <objc/message.h>
#include <dlfcn.h>

@interface CymbalAPITests : XCTestCase
@end

int _callstack_is_valid(CFArrayRef callstack)
{
    CFIndex framecnt = CFArrayGetCount(callstack);
    // Length is over 5
    if (framecnt < 5)
    {
        return 0;
    }
    
    // The first frame is start()
    CFStringRef ff = CFArrayGetValueAtIndex(callstack, framecnt - 1);
    return CFStringFind(ff, CFSTR("start +"), 0).location != kCFNotFound;
}

@implementation CymbalAPITests

- (void)testCymbalCallstackCFunctionInvocation
{
    // Callstack is fetched using the c function
    CFArrayRef callstack = cymbal_callstack();
    NSAssert(_callstack_is_valid(callstack) == 1, @"cymbal_callstack() did not return a valid callstack");
}

- (void)testCymbalCallstackObjcInvocation
{
    // Callstack is fetched using the objective-c method +[Cymbal callstack]
    CFArrayRef callstack = ((CFArrayRef (*)(id, SEL))objc_msgSend)(objc_getClass("Cymbal"), sel_registerName("callstack"));
    NSAssert(_callstack_is_valid(callstack) == 1, @"+[Cymbal callstack] did not return a valid callstack");
}

- (void)testCymbolicateObjcInstanceMethodSymbol
{
    // The address of a method that appears as <redacted> inside callstacks
    IMP redactedImp = class_getMethodImplementation(objc_getClass("NSFileManager"), sel_registerName("_processHasUbiquityContainerEntitlement"));
    
    // A symbolicated frame is created for the address
    const char *symbolicated_frame = cymbolicate((void *)redactedImp);
    
    // The symbolicated frame contains the plaintext (unredacted) class and method name
    const char *expected = "-[NSFileManager _processHasUbiquityContainerEntitlement]";
    NSAssert(strstr(symbolicated_frame, expected) != NULL, @"Failed to create symbolicated frame for an objective-c instance method address");
}

- (void)testCymbolicateObjcClassMethodSymbol
{
    // The address of a method that appears as <redacted> inside callstacks
    IMP redactedImp = class_getMethodImplementation(objc_getMetaClass("NSURLSession"), sel_registerName("sharedSession"));
    
    // A symbolicated frame is created for the address
    const char *symbolicated_frame = cymbolicate((void *)redactedImp);
    
    // The symbolicated frame contains the plaintext (unredacted) class and method name
    const char *expected = "+[NSURLSession sharedSession]";
    NSAssert(strstr(symbolicated_frame, expected) != NULL, @"Failed to create symbolicated frame for an objective-c class method address");
}

- (void)testCymbolicateCSymbol
{
    // The address of open()
    void *open_location = dlsym(RTLD_DEFAULT, "open");
    
    // A symbolicated frame is created for the address
    const char *symbolicated_frame = cymbolicate(open_location);
    
    // The symbolicated frame contains the plaintext (unredacted) class and method name
    const char *expected = "__open";
    NSAssert(strstr(symbolicated_frame, expected) != NULL, @"Failed to create symbolicated frame for a c function address");
}

- (void)testCymbolLookupObjcInstanceMethodSymbol
{
    // The address of a method that appears as <redacted> inside callstacks
    IMP redactedImp = class_getMethodImplementation(objc_getClass("NSFileManager"), sel_registerName("_processHasUbiquityContainerEntitlement"));
    
    // A Cymbal object is found for the address
    struct Cymbal *symbol = (struct Cymbal *)malloc(sizeof(struct Cymbal));
    int err = cymbal_symbol_for_address((void *)redactedImp, &symbol);
    NSAssert(err == KERN_SUCCESS, @"Failed to lookup get Cymbal for an objective-c instance method");
    
    // Cymbal member values are all as expected
    int is_valid = 0;
    is_valid += (strcmp(symbol->class_name, "NSFileManager") == 0);
    is_valid += (strcmp(symbol->method_name, "_processHasUbiquityContainerEntitlement") == 0);
    is_valid += (symbol->is_class_method == 0);
    NSAssert(is_valid == 3, @"Objective-c instance method Cymbal contained unexpected member values");
    
    free(symbol);
}

- (void)testCymbolLookupObjcClassMethodSymbol
{
    // The address of a method that appears as <redacted> inside callstacks
    IMP redactedImp = class_getMethodImplementation(objc_getMetaClass("NSURLSession"), sel_registerName("sharedSession"));
    
    // A Cymbal object is found for the address
    struct Cymbal *symbol = (struct Cymbal *)malloc(sizeof(struct Cymbal));
    int err = cymbal_symbol_for_address((void *)redactedImp, &symbol);
    NSAssert(err == KERN_SUCCESS, @"Failed to lookup get Cymbal for an objective-c class method");
    
    // Cymbal member values are all as expected
    int is_valid = 0;
    is_valid += (strcmp(symbol->class_name, "NSURLSession") == 0);
    is_valid += (strcmp(symbol->method_name, "sharedSession") == 0);
    is_valid += (symbol->is_class_method == 1);
    NSAssert(is_valid == 3, @"Objective-c class method Cymbal contained unexpected member values");
    
    free(symbol);
}

- (void)testCymbolLookupCSymbol
{
    // The address of open()
    void *open_location = dlsym(RTLD_DEFAULT, "open");
    
    // A Cymbal object is found for the address
    struct Cymbal *symbol = (struct Cymbal *)malloc(sizeof(struct Cymbal));
    int err = cymbal_symbol_for_address(open_location, &symbol);
    NSAssert(err == KERN_SUCCESS, @"Failed to lookup get Cymbal for a c function");
    
    // Cymbal symbol_name is "__open"
    NSAssert(strcmp(symbol->symbol_name, "__open") == 0, @"C function Cymbal contained unexpected member values");
    
    free(symbol);
}

@end

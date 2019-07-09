//
//  test_symbol_creation.m
//  cymbalTests
//
//  Created by Ethan Arbuckle on 7/5/19.
//  Copyright © 2019 Ethan Arbuckle. All rights reserved.
//

#import <XCTest/XCTest.h>
#include "symbol_creation.h"
#include "symbol_storage.h"
#include <objc/runtime.h>

@interface SymbolCreationTests : XCTestCase
@end

@implementation SymbolCreationTests

- (void)testCreateSymbolForInstanceMethod
{
    // A new method is added to the runtime that Cymbal does not know about
    IMP newMethodImp = imp_implementationWithBlock(^void(id _self, SEL __cmd){});
    SEL newMethodName = sel_registerName("testNewInstanceMethod");
    class_addMethod([self class], newMethodName, newMethodImp, "v@:");
    Method newMethod = class_getInstanceMethod([self class], newMethodName);
    
    // Symbol creation for the new method
    const char *test_image_path = "/System/Library/Some/Image/Path.framework/Path";
    create_symbol_for_method(newMethod, 0, test_image_path, class_getName([self class]));
    
    // The new symbol now appears in searches
    struct ObjectiveCSymbol *found = search_for_symbol_at_address((void *)newMethodImp);
    NSAssert(found != NULL, @"Failed to create a symbol from an objective-c instance method");
    
    // The returned information is as expected
    NSAssert(strcmp(found->containing_image_path, test_image_path) == 0, @"The returned symbol has an unexpected image_path");
    NSAssert(found->is_class_method == 0, @"The returned symbol has an unexpected class_method");
    NSAssert(strcmp(found->class_name, class_getName([self class])) == 0, @"The returned symbol has an unexpected class_name");
    NSAssert(strcmp(found->selector_name, sel_getName(newMethodName)) == 0, @"The returned symbol has an unexpected selector_name");
}

- (void)testCreateSymbolForClassMethod
{
    // A new method is added to the runtime that Cymbal does not know about
    Class metaself = objc_getMetaClass(class_getName([self class]));
    IMP newMethodImp = imp_implementationWithBlock(^void(id _self, SEL __cmd){});
    SEL newMethodName = sel_registerName("testNewClassMethod");
    class_addMethod(metaself, newMethodName, newMethodImp, "v@:");
    Method newMethod = class_getClassMethod(metaself, newMethodName);
    
    // Symbol creation for the new method
    const char *test_image_path = "/System/Library/Some/Image/Path.framework/Path";
    create_symbol_for_method(newMethod, 1, test_image_path, class_getName([self class]));
    
    // The new symbol now appears in searches
    struct ObjectiveCSymbol *found = search_for_symbol_at_address((void *)newMethodImp);
    NSAssert(found != NULL, @"Failed to create a symbol from an objective-c method");
    
    // The returned information is as expected
    NSAssert(strcmp(found->containing_image_path, test_image_path) == 0, @"The returned symbol has an unexpected image_path");
    NSAssert(found->is_class_method == 1, @"The returned symbol has an unexpected class_method");
    NSAssert(strcmp(found->class_name, class_getName([self class])) == 0, @"The returned symbol has an unexpected class_name");
    NSAssert(strcmp(found->selector_name, sel_getName(newMethodName)) == 0, @"The returned symbol has an unexpected selector_name");
}

- (void)testMapSymbolsFromClass
{
    // New methods are added to the runtime that Cymbal does not know about
    IMP newMethodImp_A = imp_implementationWithBlock(^void(id _self, SEL __cmd){});
    SEL newMethodName_A = sel_registerName("testNewMethodA");
    class_addMethod([self class], newMethodName_A, newMethodImp_A, "v@:");
    // Another new method
    IMP newMethodImp_B = imp_implementationWithBlock(^void(id _self, SEL __cmd){});
    SEL newMethodName_B = sel_registerName("testNewMethodB");
    class_addMethod([self class], newMethodName_B, newMethodImp_B, "v@:");
    
    // Symbol creation for the Method's containing class
    const char *test_image_path = "/System/Library/Some/Image/Path.framework/Path";
    map_symbols_from_class([self class], test_image_path);
    
    // The new symbols now appear in searches
    struct ObjectiveCSymbol *foundA = search_for_symbol_at_address((void *)newMethodImp_A);
    struct ObjectiveCSymbol *foundB = search_for_symbol_at_address((void *)newMethodImp_B);
    NSAssert(foundA != NULL, @"Failed to create symbol A from an objective-c class");
    NSAssert(foundB != NULL, @"Failed to create symbol B from an objective-c class");
}

@end

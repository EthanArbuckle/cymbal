//
//  symbol_storage_tests.m
//  cymbalTests
//
//  Created by Ethan Arbuckle on 7/5/19.
//  Copyright © 2019 Ethan Arbuckle. All rights reserved.
//

#import <XCTest/XCTest.h>
#include "symbol_storage.h"
#include <objc/runtime.h>

@interface SymbolStorageTests : XCTestCase
@end

@implementation SymbolStorageTests

- (void)testSymbolStorage
{
    // An objective-c symbol
    const char *class = "EATestClassName";
    const char *selector = "test_method";
    void *address = (void *)0x0000badf100d;
    
    struct ObjectiveCSymbol *symbol = malloc(sizeof(struct ObjectiveCSymbol));
    symbol->class_name = class;
    symbol->selector_name = selector;
    symbol->impl_address = (uint64_t)address;
    symbol->is_class_method = 0;
    
    // Is stored in the symbol storage
    store_symbol(symbol);
    
    // And is now searchable
    struct ObjectiveCSymbol *found = search_for_symbol_at_address(address);
    NSAssert(found != NULL, @"Failed to store and retrieve an objective-c symbol");
    
    free(symbol);
}

- (void)testFuzzySearchForSymbolWithinBoundary
{
    // An objective-c symbol
    const char *class = "EATestClassNameB";
    const char *selector = "test_method_fuzzy";
    void *address = (void *)0x0000badf00d;
    
    struct ObjectiveCSymbol *symbol = malloc(sizeof(struct ObjectiveCSymbol));
    symbol->class_name = class;
    symbol->selector_name = selector;
    symbol->impl_address = (uint64_t)address;
    symbol->is_class_method = 0;
    
    // Is stored in the symbol storage
    store_symbol(symbol);
    
    // And we get a return address that is 8k bytes down the stack
    void *ret_address = address + 8000;
    
    // The return address can be used to find the symbol
    struct ObjectiveCSymbol *found = search_for_symbol_at_address(ret_address);
    NSAssert(found != NULL, @"Failed to store and retrieve an objective-c symbol with a fuzzy return address");
    
    free(symbol);
}

- (void)testFuzzySearchForSymbolOutsideOfBoundaryAfter
{
    // An objective-c symbol
    const char *class = "EATestClassNameC";
    const char *selector = "test_method_fuzzy_outside";
    void *address = (void *)0x00badd00df00d;
    
    struct ObjectiveCSymbol *symbol = malloc(sizeof(struct ObjectiveCSymbol));
    symbol->class_name = class;
    symbol->selector_name = selector;
    symbol->impl_address = (uint64_t)address;
    symbol->is_class_method = 0;
    
    // Is stored in the symbol storage
    store_symbol(symbol);
    
    // And we get a return address that is 10k bytes down the stack (into the next function boundary)
    void *ret_address = address + 10000;
    
    // The return address does not find a symbol
    struct ObjectiveCSymbol *found = search_for_symbol_at_address(ret_address);
    NSAssert(found == NULL, @"Failed to store and retrieve an objective-c symbol with a fuzzy return address");
    
    free(symbol);
}

- (void)testFuzzySearchForSymbolOutsideOfBoundaryBefore
{
    // An objective-c symbol
    const char *class = "EATestClassNameC";
    const char *selector = "test_method_fuzzy_outside";
    void *address = (void *)0x00badd00df00d;
    
    struct ObjectiveCSymbol *symbol = malloc(sizeof(struct ObjectiveCSymbol));
    symbol->class_name = class;
    symbol->selector_name = selector;
    symbol->impl_address = (uint64_t)address;
    symbol->is_class_method = 0;
    
    // Is stored in the symbol storage
    store_symbol(symbol);
    
    // And we get a return address that is 10k bytes before the target entry point
    void *ret_address = address - 10000;
    
    // The return address does not find a symbol
    struct ObjectiveCSymbol *found = search_for_symbol_at_address(ret_address);
    NSAssert(found == NULL, @"Failed to store and retrieve an objective-c symbol with a fuzzy return address");
    
    free(symbol);
}

@end

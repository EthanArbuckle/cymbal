//
//  cpp_demangler_tests.m
//  cymbalTests
//
//  Created by Ethan Arbuckle on 7/5/19.
//  Copyright © 2019 Ethan Arbuckle. All rights reserved.
//

#import <XCTest/XCTest.h>
#include "cpp_demangler.h"

@interface CppDemangleTests : XCTestCase
@end

@implementation CppDemangleTests

- (void)testDemangleCppSymbolLong
{
    // A mangled c++ symbol is demangled
    const char *mangled_symbol = "___ZN5dyld320dyld_get_sdk_versionEPK11mach_header_block_invoke";
    const char *demangled = demangle_cpp_symbol(mangled_symbol);
    
    // And it was demangled as expected
    const char *expected = "invocation function for block in dyld3::dyld_get_sdk_version(mach_header const*)";
    NSAssert(strcmp(demangled, expected) == 0, @"A long c++ symbol did not correctly demangle");
}

- (void)testDemangleCppSymbolShort
{
    // A mangled c++ symbol is demangled
    const char *mangled_symbol = "_ZN11header_info15getHeaderInfoRWEv";
    const char *demangled = demangle_cpp_symbol(mangled_symbol);
    
    // And it was demangled as expected
    const char *expected = "header_info::getHeaderInfoRW()";
    NSAssert(strcmp(demangled, expected) == 0, @"A short c++ symbol did not correctly demangle");
}

- (void)testDemangleCppSymbolInvalid
{
    // An invalid c++ symbol is demangled
    const char *mangled_symbol = "______IM_A_C_FUNCTION";
    const char *demangled = demangle_cpp_symbol(mangled_symbol);
    
    // And it demangled to NULL
    NSAssert(demangled == NULL, @"Demangling an invalid c++ symbol did not return NULL");
}

@end

//
//  cpp_demangler.c
//  cymbal
//
//  Created by Ethan Arbuckle on 12/12/17.
//  Copyright © 2017 Ethan Arbuckle. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>
#include <dlfcn.h>


char *demangle_cpp_symbol(const char *mangled_symbol) {
    // Get a pointer to abi::__cxa_demangle function pointer
    static dispatch_once_t onceToken;
    static void *__cxa_demangle = NULL;
    dispatch_once(&onceToken, ^{
        __cxa_demangle = dlsym(dlopen(NULL, RTLD_NOW), "__cxa_demangle");
    });
    
    if (__cxa_demangle == NULL) {
        return NULL;
    }
    
    return ((char * (*)(const char *, char *, size_t *, int *))__cxa_demangle)(mangled_symbol, NULL, NULL, NULL);
}

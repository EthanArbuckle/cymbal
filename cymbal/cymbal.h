//
//  cymbal.h
//  cymbal
//
//  Created by Ethan Arbuckle on 12/11/17.
//  Copyright © 2017 Ethan Arbuckle. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>

#define EXPORT __attribute__ ((visibility("default")))

typedef enum {
	cymbalTypeObjectiveC = 0,
	cymbalTypeC = 1
} kCymbalType;

struct Cymbal {
    // indicates if this is a c or objective-c symbol
	kCymbalType type;
    
    // for objective-c symbols
	const char *class_name;
	const char *method_name;
    int is_class_method;
    
    // everything else
	const char *symbol_name;
	
} ;

EXPORT CFArrayRef cymbal_callstack(void);
EXPORT int cymbal_symbol_for_address(void *address, struct Cymbal **cymbal);
EXPORT const char *cymbolicate(void *address);

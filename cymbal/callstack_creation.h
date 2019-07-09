//
//  callstack_creation.h
//  cymbal
//
//  Created by Ethan Arbuckle on 12/12/17.
//  Copyright © 2017 Ethan Arbuckle. All rights reserved.
//

#import <CoreFoundation/CoreFoundation.h>

CFArrayRef create_symbolicated_callstack(int depth, int nothing_precedes_start);

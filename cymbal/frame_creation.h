//
//  frame_creation.h
//  cymbal
//
//  Created by Ethan Arbuckle on 12/12/17.
//  Copyright © 2017 Ethan Arbuckle. All rights reserved.
//

#import "symbol_storage.h"

const char *create_objc_frame_for_symbol(struct ObjectiveCSymbol *symbol, void *address);
const char *create_c_symbol_name(void *address);
const char *create_c_frame_for_address(void *address);
const char *symbolicated_frame_for_address(void *address);

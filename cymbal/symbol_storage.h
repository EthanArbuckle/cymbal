//
//  symbol_storage.h
//  cymbal
//
//  Created by Ethan Arbuckle on 12/12/17.
//  Copyright © 2017 Ethan Arbuckle. All rights reserved.
//

struct ObjectiveCSymbol
{
    const char *selector_name;
    const char *class_name;
    const char *containing_image_path;
    uint64_t impl_address;
    int is_class_method;
};

void store_symbol(struct ObjectiveCSymbol *symbol);
struct ObjectiveCSymbol *search_for_symbol_at_address(void *function_address);

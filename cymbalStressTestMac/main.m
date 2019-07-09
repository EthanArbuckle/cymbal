//
//  main.m
//  cymbalStressTestMac
//
//  Created by Ethan Arbuckle on 7/4/19.
//  Copyright © 2019 Ethan Arbuckle. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <objc/runtime.h>
#import "cymbal.h"

int main(int argc, const char * argv[])
{
    @autoreleasepool
    {
        // Every [NSObject alloc] invocation will trigger a cymbal callstack to be created
        Method target = class_getClassMethod(objc_getClass("NSObject"), sel_registerName("alloc"));
        IMP origimp = method_getImplementation(target);
        IMP newimp = imp_implementationWithBlock(^void*(id a, SEL b){
            
            CFArrayRef callstack = cymbal_callstack();
            NSLog(@"cymbal: %@", callstack);
            CFRelease(callstack);
            
            return ((void * (*)(id, SEL))origimp)(a, b);
        });
        method_setImplementation(target, newimp);
        
        // Create some NSObjects
        __unused NSString *test = [[NSString alloc] init];
        [[[NSURLSession sharedSession] dataTaskWithURL:[NSURL URLWithString:@"https://google.com"] completionHandler:^(NSData * _Nullable data, NSURLResponse * _Nullable response, NSError * _Nullable error) {
        
            CFRunLoopStop(CFRunLoopGetMain());

        }] resume];
    }
    
    CFRunLoopRun();
    
    return 0;
}

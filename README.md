## Cymbal

A very fast objective-c symbolicator. Cymbal finds symbolicated method names for memory addresses.

#### Usage / API
###### callstack retrieval
`CFArrayRef cymbal_callstack(void);`

	// Get a callstack for the current execution context
    CFArrayRef callstack = cymbal_callstack();
    NSLog(@"cymbal: %@", callstack);
    
    // Cymbal callstacks need to be released after use
    CFRelease(callstack);


###### single address symbolication
`const char *cymbolicate(void *address);`

	// Some address that points to an objective-c method
    IMP redactedImp = class_getMethodImplementation(objc_getClass("NSFileManager"), sel_registerName("_processHasUbiquityContainerEntitlement"));
    
    // A symbolicated frame is created for the address
    const char *symbolicated_frame = cymbolicate((void *)redactedImp);
    NSLog(@"%s", symbolicated_frame);
    
    // Frames need to be released
    free((void *)symbolicated_frame);
    
    
    
######  single address symbolication with more detail
`int cymbal_symbol_for_address(void *address, struct Cymbal **cymbal);`

    // Some address that points to an objective-c method
    IMP redactedImp = class_getMethodImplementation(objc_getClass("NSFileManager"), sel_registerName("_processHasUbiquityContainerEntitlement"));
    
    // A Cymbal object is found for the address
    struct Cymbal *symbol = (struct Cymbal *)malloc(sizeof(struct Cymbal));
    cymbal_symbol_for_address((void *)redactedImp, &symbol);
    
    NSLog(@"%s", symbol->class_name);
    NSLog(@"%s", symbol->method_name);
    NSLog(@"%d", symbol->is_class_method);
    
    // Cymbals need to be released
    free(symbol);
    
    
    
    
#### Cymbol vs NSThread

This is a callstack generated via `+[NSThread callStackSymbols]`:


	0   cymbalStressTestiOS                 0x0000000100d44570 __main_block_invoke_2 + 188
	1   CFNetwork                           0x00000001821f1c1c <redacted> + 32
	2   CFNetwork                           0x000000018220a93c <redacted> + 152
	3   Foundation                          0x000000018268ae88 <redacted> + 16
	4   Foundation                          0x00000001825cc8d0 <redacted> + 72
	5   Foundation                          0x00000001825cbcac <redacted> + 848
	6   libdispatch.dylib                   0x000000010110d220 _dispatch_client_callout + 16
	7   libdispatch.dylib                   0x0000000101119850 _dispatch_block_invoke_direct + 232
	8   libdispatch.dylib                   0x000000010110d220 _dispatch_client_callout + 16
	9   libdispatch.dylib                   0x0000000101119850 _dispatch_block_invoke_direct + 232
	10  libdispatch.dylib                   0x0000000101119734 dispatch_block_perform + 104
	11  Foundation                          0x000000018268c750 <redacted> + 376
	12  libdispatch.dylib                   0x000000010110d220 _dispatch_client_callout + 16
	13  libdispatch.dylib                   0x000000010111a4d8 _dispatch_continuation_pop + 588
	14  libdispatch.dylib                   0x0000000101118dc8 _dispatch_async_redirect_invoke + 628
	15  libdispatch.dylib                   0x000000010111e84c _dispatch_root_queue_drain + 604
	16  libdispatch.dylib                   0x000000010111e584 _dispatch_worker_thread3 + 136
	17  libsystem_pthread.dylib             0x0000000181887fac _pthread_wqthread + 1176
	18  libsystem_pthread.dylib             0x0000000181887b08 start_wqthread + 4
	

In this same execution context, this is a callstack generated with Cymbol:

    "0  cymbalStressTestiOS                 0x0000000100d44948 create_symbolicated_callstack + 152",
    "1  cymbalStressTestiOS                 0x0000000100d44f54 cymbal_callstack + 40",
    "2  cymbalStressTestiOS                 0x0000000100d44528 __main_block_invoke_2 + 116",
    "3  CFNetwork                           0x00000001821f189c -[__NSURLSessionLocal taskForClass:request:uploadFile:bodyData:completion:] + 896",
    "4  CFNetwork                           0x000000018220a584 -[__NSCFLocalSessionTask _task_onqueue_didFinish] + 952",
    "5  Foundation                          0x000000018268acac -[NSOperation __graphDescription:] + 476",
    "6  Foundation                          0x00000001825cc888 -[NSBlockOperation main] + 72",
    "7  Foundation                          0x00000001825cb95c -[__NSOperationInternal _start:] + 848",
    "8  libdispatch.dylib                   0x000000010110d220 _dispatch_client_callout + 16",
    "9  libdispatch.dylib                   0x0000000101119850 _dispatch_block_invoke_direct + 232",
    "10  libdispatch.dylib                   0x000000010110d220 _dispatch_client_callout + 16",
    "11  libdispatch.dylib                   0x0000000101119850 _dispatch_block_invoke_direct + 232",
    "12  libdispatch.dylib                   0x0000000101119734 dispatch_block_perform + 104",
    "13  Foundation                          0x000000018268c438 -[NSOperationQueue removeObserver:forKeyPath:] + 792",
    "14  libdispatch.dylib                   0x000000010110d220 _dispatch_client_callout + 16",
    "15  libdispatch.dylib                   0x000000010111a4d8 _dispatch_continuation_pop + 588",
    "16  libdispatch.dylib                   0x0000000101118dc8 _dispatch_async_redirect_invoke + 628",
    "17  libdispatch.dylib                   0x000000010111e84c _dispatch_root_queue_drain + 604",
    "18  libdispatch.dylib                   0x000000010111e584 _dispatch_worker_thread3 + 136",
    "19  libsystem_pthread.dylib             0x0000000181887fac _pthread_wqthread + 1176",
    "20  libsystem_pthread.dylib             0x0000000181887b08 start_wqthread + 4"

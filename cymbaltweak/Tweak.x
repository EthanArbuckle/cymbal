%config(generator=internal);

#include "cymbal.h"

%hook NSThread

+ (id)callStackSymbols
{
    return (NSArray *)cymbal_callstack();
}

%end
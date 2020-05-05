#ifndef ACUTILS_TYPES_H
#define ACUTILS_TYPES_H

#include "stddef.h"

#ifndef __cplusplus
#   ifndef __STDC_VERSION__
#       define __STDC_VERSION__ 0L
#   endif
#   if __STDC_VERSION__ >= 199901L
#       include <stdbool.h>
#   else
        typedef enum { false, true } bool;
#   endif
#endif

typedef void*(*ACUtilsReallocator)(void* ptr, size_t size);
typedef void(*ACUtilsDeallocator)(void* ptr);
typedef size_t(*ACUtilsGrowStrategy)(size_t requiredSize, size_t typeSize);

#endif /* ACUTILS_TYPES_H */

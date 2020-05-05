#ifndef ACUTILS_MACROS_H
#define ACUTILS_MACROS_H

#if defined(__STDC__)
#   define ACUTILS_C_STANDARD_89
#   if defined(__STDC_VERSION__)
#       define ACUTILS_C_STANDARD_90
#       if (__STDC_VERSION__ >= 199409L)
#           define ACUTILS_C_STANDARD_94
#       endif
#       if (__STDC_VERSION__ >= 199901L)
#           define ACUTILS_C_STANDARD_99
#       endif
#       if (__STDC_VERSION__ >=  201112L)
#           define ACUTILS_C_STANDARD_11
#       endif
#       if (__STDC_VERSION__ >= 201710L)
#           define ACUTILS_C_STANDARD_17
#           define ACUTILS_C_STANDARD_18
#       endif
#   endif
#endif

/*
 * ACUTILS_SHD_FUNC: static (header) function
 * ACUTILS_SIHD_FUNC: static inline (header) function (>=C99) or static (header) function (<C99)
 */
#ifdef ACUTILS_ONE_SOURCE
#   define ACUTILS_SHD_FUNC static
#   ifdef ACUTILS_C_STANDARD_99
#       define ACUTILS_SIHD_FUNC static inline
#   else
#       define ACUTILS_SIHD_FUNC static
#   endif
#else
#   define ACUTILS_SHD_FUNC
#   define ACUTILS_SIHD_FUNC
#endif

#endif /* ACUTILS_MACROS_H */

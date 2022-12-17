
#ifndef XLLR_OPENJDK_EXPORTS_H
#define XLLR_OPENJDK_EXPORTS_H

#ifdef SHARED_EXPORTS_BUILT_AS_STATIC
#  define XLLR_OPENJDK_EXPORTS
#  define XLLR_OPENJDK_NO_EXPORT
#else
#  ifndef XLLR_OPENJDK_EXPORTS
#    ifdef xllr_openjdk_EXPORTS
        /* We are building this library */
#      define XLLR_OPENJDK_EXPORTS __declspec(dllexport)
#    else
        /* We are using this library */
#      define XLLR_OPENJDK_EXPORTS __declspec(dllimport)
#    endif
#  endif

#  ifndef XLLR_OPENJDK_NO_EXPORT
#    define XLLR_OPENJDK_NO_EXPORT 
#  endif
#endif

#ifndef XLLR_OPENJDK_DEPRECATED
#  define XLLR_OPENJDK_DEPRECATED __declspec(deprecated)
#endif

#ifndef XLLR_OPENJDK_DEPRECATED_EXPORT
#  define XLLR_OPENJDK_DEPRECATED_EXPORT XLLR_OPENJDK_EXPORTS XLLR_OPENJDK_DEPRECATED
#endif

#ifndef XLLR_OPENJDK_DEPRECATED_NO_EXPORT
#  define XLLR_OPENJDK_DEPRECATED_NO_EXPORT XLLR_OPENJDK_NO_EXPORT XLLR_OPENJDK_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef XLLR_OPENJDK_NO_DEPRECATED
#    define XLLR_OPENJDK_NO_DEPRECATED
#  endif
#endif

#endif /* XLLR_OPENJDK_EXPORTS_H */

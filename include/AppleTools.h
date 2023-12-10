/*
 * AppleTools.h
 *
 * May system level features have to use Objective C Apple speciic functions. This
 * file contains a set of C callable functions that are implemented in Objective C.
 */


#ifdef __cplusplus
extern "C" {
#endif

void *makeViewMetalCompatible(void* handle);

int detectDarkTheme(void);

void *makeViewMetalCompatible(void* handle);

#ifdef __cplusplus
}   // extern "C"
#endif

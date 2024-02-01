#ifndef __DRIVERLIB_VERSION__
	#define DRIVERLIB_VER_MAJOR 2
	#define DRIVERLIB_VER_MINOR 91
	#define DRIVERLIB_VER_PATCH 13
	#define DRIVERLIB_VER_BUILD 01
#endif

#define getVersion() ((uint32_t)DRIVERLIB_VER_MAJOR<<24 | \
                      (uint32_t)DRIVERLIB_VER_MINOR<<16 | \
                      (uint32_t)DRIVERLIB_VER_PATCH<<8 | \
                      (uint32_t)DRIVERLIB_VER_BUILD)

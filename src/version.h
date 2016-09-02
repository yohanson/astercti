#ifndef VERSION
#define VERSION "0.0.20-rc1"
#ifdef DEBUG
 #define FULLVERSION VERSION " (debug)"
#else
 #define FULLVERSION VERSION
#endif
#endif

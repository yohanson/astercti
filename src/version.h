#ifndef VERSION
#define VERSION "0.0.24"
#ifdef DEBUG
 #define FULLVERSION VERSION " (debug)"
#else
 #define FULLVERSION VERSION
#endif
#endif

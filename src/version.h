#ifndef VERSION
#define VERSION "0.0.23"
#ifdef DEBUG
 #define FULLVERSION VERSION " (debug)"
#else
 #define FULLVERSION VERSION
#endif
#endif

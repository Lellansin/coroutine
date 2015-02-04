#ifndef C_DEBUG_H
#define C_DEBUG_H

#ifdef _debug
#define debug printf
#else
int debug(const char* str, ...);
#endif

#endif

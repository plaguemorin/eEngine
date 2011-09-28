/**
 * Global
 */
 
#ifndef GLOBAL_H_
#define GLOBAL_H_

#define TRUE		(1)
#define FALSE		(0)

#define YES			TRUE
#define NO			FALSE

typedef unsigned char BOOL;


#ifndef offsetof
#define offsetof(type, member)  __builtin_offsetof (type, member)
#endif

#endif

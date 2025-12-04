/*****************************************************
 * File: stdtypes.h
 * Author: Mohamed Gamal
 * Description: Standard data type definitions
 *****************************************************/

#ifndef STDTYPES_H_
#define STDTYPES_H_

/* Unsigned integer types */
typedef unsigned char       	uint8_t;
typedef unsigned short int      uint16_t;
typedef unsigned int     	    uint32_t;
typedef unsigned long long  	uint64_t;

/* Signed integer types */
typedef signed char         	sint8_t;
typedef signed short int        sint16_t;
typedef signed int              sint32_t;
typedef signed long long    	sint64_t;

/* Floating point types */
typedef float               float32_t;
typedef double              float64_t;

/* Boolean type */
typedef enum {
    FALSE = 0,
    TRUE  = 1
} bool_t;

/* Null pointer definition */
#ifndef NULL
#define NULL ((void*)0)
#endif

#endif /* STDTYPES_H_ */


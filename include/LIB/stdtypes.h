/******************************************************************************
 * @file    stdtypes.h
 * @author  Mohamed Gamal / Eng.Gemy
 * @brief   Standard Data Type Definitions
 *          Provides freestanding typedefs for unsigned/signed integers,
 *          floats, boolean and NULL. Used as leaf include by all MCAL/HAL/OS
 *          drivers to avoid pulling <stdint.h> on bare-metal.
 * @date    2024
 * @version 1.0
 * @note    Keep include guard STDTYPES_H_. Do not mix with <stdbool.h> bool.
 *          s* prefix follows project convention (sint8_t vs int8_t).
 ******************************************************************************/

#ifndef STDTYPES_H_
#define STDTYPES_H_

/******************************************************************************
 * @brief Unsigned integer types
 * @details Exact-width aliases for bare-metal. Sizes match ARM EABI:
 *          8 → unsigned char, 16 → unsigned short, 32 → unsigned int,
 *          64 → unsigned long long.
 ******************************************************************************/
typedef unsigned char       	uint8_t;   /**< 8-bit unsigned */
typedef unsigned short int      uint16_t;  /**< 16-bit unsigned */
typedef unsigned int     	    uint32_t;  /**< 32-bit unsigned */
typedef unsigned long long  	uint64_t;  /**< 64-bit unsigned */

/******************************************************************************
 * @brief Signed integer types
 ******************************************************************************/
typedef signed char         	sint8_t;   /**< 8-bit signed */
typedef signed short int        sint16_t;  /**< 16-bit signed */
typedef signed int              sint32_t;  /**< 32-bit signed */
typedef signed long long    	sint64_t;  /**< 64-bit signed */

/******************************************************************************
 * @brief Floating point types
 ******************************************************************************/
typedef float               float32_t; /**< 32-bit float (IEEE754) */
typedef double              float64_t; /**< 64-bit double */

/******************************************************************************
 * @brief Boolean type
 * @details Project-specific bool_t follows enum {FALSE=0, TRUE=1}.
 *          Do not confuse with stdbool.h bool (true/false).
 ******************************************************************************/
typedef enum {
    FALSE = 0,  /**< Logical false (0) */
    TRUE  = 1   /**< Logical true (1) */
} bool_t;

/******************************************************************************
 * @brief Null pointer definition
 * @details Provides freestanding NULL if <stddef.h> not included.
 *          Guarded to coexist with toolchain headers.
 ******************************************************************************/
#ifndef NULL
#define NULL ((void*)0) /**< Null pointer constant */
#endif

#endif /* STDTYPES_H_ */

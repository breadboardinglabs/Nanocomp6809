/***************************************************************************//**

  @file         cmocextra.h

  @author       Dave Henry

  @date         Created Monday 26th December 2022

  @brief        Extra definitions for CMOC.

  @copyright    Copyright (c) 2022, Dave Henry.  Released under the Revised
                BSD License.  See LICENSE.txt for details.

*******************************************************************************/

#ifndef _CMOC_BASIC_TYPES_
#define _CMOC_BASIC_TYPES_
typedef unsigned char byte;
typedef signed char   sbyte;
typedef unsigned int  word;
typedef signed int    sword;
typedef unsigned long dword;
typedef signed long   sdword;
#endif

// Define NULL pointer
#ifndef NULL
#define NULL ((void *) 0)
#endif

#ifndef _CMOC_HAVE_BOOL_
typedef unsigned char bool;
// Define FALSE,false=0 and TRUE,true=1
enum {FALSE, TRUE};
enum {false,true};
#define _CMOC_HAVE_BOOL_
#endif

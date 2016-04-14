/*  Copyright 2013 Aalborg University. All rights reserved.
 *   
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  
 *  1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *  
 *  2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *  
 *  THIS SOFTWARE IS PROVIDED BY Aalborg University ''AS IS'' AND ANY
 *  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Aalborg University OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 *  USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 *  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 *  
 *  The views and conclusions contained in the software and
 *  documentation are those of the authors and should not be interpreted
 *  as representing official policies, either expressed.
 */

#ifndef HP_MACROS_H
#define HP_MACROS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define alloc_struct(p) 		\
  do {                                	\
    p = calloc(1, sizeof *p);          	\
    if(p==NULL) goto cleanup; 		\
  } while (0)

#define string_copy(copy, original) 			\
  do { 							\
    copy = malloc(sizeof(char)*(strlen(original)+1)); 	\
    if(copy == NULL) 					\
    { 							\
      goto cleanup; 					\
    } 							\
    strcpy(copy, original); 				\
  }while(0)

#define null_ok_string_copy(copy, original) 	\
  do{ 						\
    if(original != NULL) 			\
    { 						\
      string_copy(copy, original); 		\
    }  						\
  }while(0)

#define null_nok_string_copy(copy, original) 	\
  do{ 						\
    if( original != NULL ) 			\
    { 						\
      string_copy(copy, original); 		\
    } 						\
    else 					\
    { 						\
      printf(#original "cannot be NULL\n"); 	\
      goto cleanup; 				\
    } 						\
  }while(0)

#define null_nok_pointer_ass(to_ass, ass) 	\
  do{ 						\
    if( ass != NULL ) 				\
    { 						\
      to_ass = ass; 				\
    } 						\
    else 					\
    { 						\
      goto cleanup; 				\
    } 						\
  }while(0)


#define free_pointer(p) if(p != NULL) free(p)

#endif


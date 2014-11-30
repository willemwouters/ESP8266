/* Customer ID=7011; Build=0x2b6f6; Copyright (c) 1999-2006 by Tensilica Inc.  ALL RIGHTS RESERVED.
   These coded instructions, statements, and computer programs are the
   copyrighted works and confidential proprietary information of Tensilica Inc.
   They may not be modified, copied, reproduced, distributed, or disclosed to
   third parties in any manner, medium, or form, in whole or in part, without
   the prior written consent of Tensilica Inc.
*/

/* "packaged libraries" are the form of the library prior to loading. 

   This is an opaque structure. 
*/
typedef struct xtlib_packaged_library
{
  char dummy;
} 
xtlib_packaged_library;

/* Some users will want to use a custom memory copy or setting function, (if, 
   for example, they want the library to be copied via DMA instead of copied 
   normally.  */

typedef void * (*memcpy_func) (void * dest, const void * src, unsigned int n);
typedef void * (*memset_func) (void * s, int c, unsigned int n);


/* Error handling */
enum {
  XTLIB_NO_ERR = 0,
  XTLIB_NOT_ELF = 1,
  XTLIB_NOT_DYNAMIC = 2,
  XTLIB_NOT_STATIC = 3,
  XTLIB_NO_DYNAMIC_SEGMENT = 4,
  XTLIB_UNKNOWN_SYMBOL = 5,
  XTLIB_NOT_ALIGNED = 6,
};

/* If a function in the API fails, call this function to get 
   one of the error codes above. 
*/
unsigned int xtlib_error();


/* An overlay can be loaded via xtlib_load_overlay. 

   Returns the address of the entry point defined by the -e option to
   package_loadable_library script. Or NULL if for some reason the
   load fails. Check the error code in that case.
   
   No additional information is needed or required. To unload the library
   just overwrite it with the new stuff. Make sure it has released all
   the resources it has reserved first though.

   To use a custom memory copy and setting functions, use the second form.
*/

void * xtlib_load_overlay(xtlib_packaged_library * library);
void * xtlib_load_overlay_custom_fn(xtlib_packaged_library * library, 
				    memcpy_func copy_fn, memset_func set_fn);


/* To load a position-independent library, you'll need to allocate
   memory of xtlib_pi_library_size.
 */

unsigned int xtlib_pi_library_size(xtlib_packaged_library * library);


/* To actually load the library:

   First, allocate a xtlib_pil_info. On the stack or via 
   malloc is fine. 

   Next, call xtlib_load_pi_library. It will fill out the structure, 
   which you'll need later for symbol lookup and related. Accessing the
   fields directly may result in unexpected behavior. If you stick to the
   API, you'll be fine.

   xtlib_load_pi_library returns the address of the entry point defined 
   by the -e option to package_loadable_library script. Or NULL if for 
   some reason the load fails. Check the error code in that case.

   When you want to unload the library, call "xtlib_unload_pi_library" first
   and it will call the termination function of the pil.

   To lookup the address of a symbol, use xtlib_lookup_pi_library_symbol,
   which will return the correct address, or NULL if the symbol isn't found.

   To use a custom memory functions, use the second form of 
   xtlib_load_pi_library.
*/


typedef void (*init_fini_func)(void);

typedef struct xtlib_pil_info {
  void * src_addr;
  void * dst_addr;
  void * start_sym;
  void * text_addr;
  init_fini_func init;
  init_fini_func fini;
  void * rel;
  int rela_count;
  void * hash;
  void * symtab;
  void * strtab;
  int align;
} xtlib_pil_info;

void * xtlib_load_pi_library(xtlib_packaged_library * library, 
			     void * destination_address, 
			     xtlib_pil_info * lib_info);

void * xtlib_load_pi_library_custom_fn(xtlib_packaged_library * library, 
				       void * destination_address, 
				       xtlib_pil_info * lib_info,
				       memcpy_func mcpy_fn,
				       memset_func mset_fn);

void xtlib_unload_pi_library(xtlib_pil_info * lib_info);

void * xtlib_lookup_pi_library_symbol(xtlib_pil_info * lib_info, const char * symbolname);

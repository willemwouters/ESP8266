#ifndef __ROFS_H
#define __ROFS_H
#include "c_types.h"
typedef struct {
			size_t size;
			uint8_t gzip;
			const char *name;
			uint32_t offset;
		} RO_FILE_ENTRY;

		typedef struct {
			size_t count;
			RO_FILE_ENTRY files[];
		} RO_FS;

#endif

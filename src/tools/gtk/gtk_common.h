/*
 * Common infrastructure for split GTK files
 * This file contains all the wrapper infrastructure needed by split GTK files
 */

#ifndef GTK_COMMON_H
#define GTK_COMMON_H

// Standard system includes needed for types (must come first)
#include <stddef.h>
#include <dlfcn.h>

#include "../../include/wrappedlibs.h"
#include "../../include/bridge.h"
#include "../../include/alternate.h"
#include "../../include/debug.h"
#include "../../include/callback.h"
#include "../../include/librarian.h"
#include "../../include/gtkclass.h"
#include "../../include/library.h"
#include "../../wrapped/generated/wrapper.h"

// Include super80.h after core includes
#include "../../include/super80.h"


// Signal offset tracking structures
typedef struct sigoffset_s {
    uint32_t offset;
    int     n;
} sigoffset_t;

typedef struct sigoffset_array_s {
    sigoffset_t *a;
    int     cap;
    int     sz;
} sigoffset_array_t;

// Hash map type definitions - macros from khash.h
KHASH_SET_INIT_INT(signalmap)
KHASH_MAP_INIT_INT64(sigoffset, sigoffset_array_t)
KHASH_MAP_INIT_INT64(customclass, char*)

// Hash map type aliases
typedef kh_signalmap_t kh_signalmap_t;
typedef kh_sigoffset_t kh_sigoffset_t;
typedef kh_customclass_t kh_customclass_t;

// External declarations for global hash maps (defined in gtk_core.c)
extern kh_signalmap_t *my_signalmap;
extern kh_sigoffset_t *my_sigoffset;
extern kh_customclass_t *my_customclass;

#endif // GTK_COMMON_H

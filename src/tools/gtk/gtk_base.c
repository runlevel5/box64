#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "gtk_macros.h"

/*
 * GObject and GInitiallyUnowned foundation classes
 */

// ----- GObjectClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GObject, constructor, void*, (size_t type, uint32_t n_construct_properties, void* construct_properties), "Lup", type, n_construct_properties, construct_properties);
WRAPPER(GObject, set_property, void, (void* object, uint32_t property_id, void* value, void* pspec), "pupp", object, property_id, value, pspec);
WRAPPER(GObject, get_property, void, (void* object, uint32_t property_id, void* value, void* pspec), "pupp", object, property_id, value, pspec);
WRAPPER(GObject, dispose, void, (void* object), "p", object);
WRAPPER(GObject, finalize, void, (void* object), "p", object);
WRAPPER(GObject, dispatch_properties_changed, void*, (size_t type, uint32_t n_pspecs, void* pspecs), "Lup", type, n_pspecs, pspecs);
WRAPPER(GObject, notify, void*, (size_t type, void* pspecs), "Lp", type, pspecs);
WRAPPER(GObject, constructed, void, (void* object), "p", object);

#define SUPERGO() \
    GO(constructor, pFLup);                 \
    GO(set_property, vFpupp);               \
    GO(get_property, vFpupp);               \
    GO(dispose, vFp);                       \
    GO(finalize, vFp);                      \
    GO(dispatch_properties_changed, vFLup); \
    GO(notify, vFpp);                       \
    GO(constructed, vFp);

// wrap (so bridge all calls, just in case)
void wrapGObjectClass(void* cl)
{
    my_GObjectClass_t* class = (my_GObjectClass_t*)cl;
    #define GO(A, W) class->A = reverse_##A##_GObject (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGObjectClass(void* cl)
{
    my_GObjectClass_t* class = (my_GObjectClass_t*)cl;
    #define GO(A, W)   class->A = find_##A##_GObject (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGObjectClass(void* cl)
{
    my_GObjectClass_t* class = (my_GObjectClass_t*)cl;
    #define GO(A, W) autobridge_##A##_GObject (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// unwrap (and use callback if not a native call anymore)
void unwrapGObjectInstance(void* cl)
{
    my_GObject_t* class = (my_GObject_t*)cl;
}
// autobridge
void bridgeGObjectInstance(void* cl)
{
    my_GObject_t* class = (my_GObject_t*)cl;
}

// ----- GInitiallyUnownedClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GInitiallyUnowned, constructor, void*, (size_t type, uint32_t n_construct_properties, void* construct_properties), "Lup", type, n_construct_properties, construct_properties);
WRAPPER(GInitiallyUnowned, set_property, void, (void* object, uint32_t property_id, void* value, void* pspec), "pupp", object, property_id, value, pspec);
WRAPPER(GInitiallyUnowned, get_property, void, (void* object, uint32_t property_id, void* value, void* pspec), "pupp", object, property_id, value, pspec);
WRAPPER(GInitiallyUnowned, dispose, void, (void* object), "p", object);
WRAPPER(GInitiallyUnowned, finalize, void, (void* object), "p", object);
WRAPPER(GInitiallyUnowned, dispatch_properties_changed, void*, (size_t type, uint32_t n_pspecs, void* pspecs), "Lup", type, n_pspecs, pspecs);
WRAPPER(GInitiallyUnowned, notify, void*, (size_t type, void* pspecs), "Lp", type, pspecs);
WRAPPER(GInitiallyUnowned, constructed, void, (void* object), "p", object);

#define SUPERGO() \
    GO(constructor, pFLup);                 \
    GO(set_property, vFpupp);               \
    GO(get_property, vFpupp);               \
    GO(dispose, vFp);                       \
    GO(finalize, vFp);                      \
    GO(dispatch_properties_changed, vFLup); \
    GO(notify, vFLp);                       \
    GO(constructed, vFp);

// wrap (so bridge all calls, just in case)
void wrapGInitiallyUnownedClass(void* cl)
{
    my_GInitiallyUnownedClass_t* class = (my_GInitiallyUnownedClass_t*)cl;
    #define GO(A, W) class->A = reverse_##A##_GInitiallyUnowned (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGInitiallyUnownedClass(void* cl)
{
    my_GInitiallyUnownedClass_t* class = (my_GInitiallyUnownedClass_t*)cl;
    #define GO(A, W)   class->A = find_##A##_GInitiallyUnowned (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGInitiallyUnownedClass(void* cl)
{
    my_GInitiallyUnownedClass_t* class = (my_GInitiallyUnownedClass_t*)cl;
    #define GO(A, W) autobridge_##A##_GInitiallyUnowned (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGInitiallyUnownedInstance(void* cl)
{
    my_GInitiallyUnowned_t* class = (my_GInitiallyUnowned_t*)cl;
}
// autobridge
void bridgeGInitiallyUnownedInstance(void* cl)
{
    my_GInitiallyUnowned_t* class = (my_GInitiallyUnowned_t*)cl;
}

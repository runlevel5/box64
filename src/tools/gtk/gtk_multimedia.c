#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "gtk_macros.h"

// gtk_multimedia.c - GStreamer multimedia classes for GTK split

// ----- GstObjectClass ------
WRAPPER(GstObject, deep_notify, void, (void* object, void* origin, void* pspec), "ppp", object, origin, pspec);

#define SUPERGO()               \
    GO(deep_notify, vFppp);     \

// wrap (so bridge all calls, just in case)
void wrapGstObjectClass(void* cl)
{
    my_GstObjectClass_t* class = (my_GstObjectClass_t*)cl;
    wrapGInitiallyUnownedClass(&class->parent);
    #define GO(A, W) class->A = reverse_##A##_GstObject (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGstObjectClass(void* cl)
{
    my_GstObjectClass_t* class = (my_GstObjectClass_t*)cl;
    unwrapGInitiallyUnownedClass(&class->parent);
    #define GO(A, W)   class->A = find_##A##_GstObject (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGstObjectClass(void* cl)
{
    my_GstObjectClass_t* class = (my_GstObjectClass_t*)cl;
    bridgeGInitiallyUnownedClass(&class->parent);
    #define GO(A, W) autobridge_##A##_GstObject (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGstObjectInstance(my_GstObject_t* class)
{
    unwrapGInitiallyUnownedInstance(&class->parent);
}
// autobridge
void bridgeGstObjectInstance(my_GstObject_t* class)
{
    bridgeGInitiallyUnownedInstance(&class->parent);
}

// ----- GstAllocatorClass ------
WRAPPER(GstAllocator, alloc, void*, (void* allocator, size_t size, void* params), "pLp", allocator, size, params);
WRAPPER(GstAllocator, free, void, (void* allocator, void* memory), "pp", allocator, memory);

#define SUPERGO()               \
    GO(alloc, pFpLp);           \
    GO(free, vFpp);             \

// wrap (so bridge all calls, just in case)
void wrapGstAllocatorClass(void* cl)
{
    my_GstAllocatorClass_t* class = (my_GstAllocatorClass_t*)cl;
    wrapGstObjectClass(&class->parent);
    #define GO(A, W) class->A = reverse_##A##_GstAllocator (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGstAllocatorClass(void* cl)
{
    my_GstAllocatorClass_t* class = (my_GstAllocatorClass_t*)cl;
    unwrapGstObjectClass(&class->parent);
    #define GO(A, W)   class->A = find_##A##_GstAllocator (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGstAllocatorClass(void* cl)
{
    my_GstAllocatorClass_t* class = (my_GstAllocatorClass_t*)cl;
    bridgeGstObjectClass(&class->parent);
    #define GO(A, W) autobridge_##A##_GstAllocator (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGstAllocatorInstance(my_GstAllocator_t* class)
{
    unwrapGstObjectInstance(&class->parent);
}

void bridgeGstAllocatorInstance(my_GstAllocator_t* class)
{
    bridgeGstObjectInstance(&class->parent);
}

// ----- GstElementClass ------
WRAPPER(GstElement, change_state, int, (void* element, int transition), "pi", element, transition);
WRAPPER(GstElement, request_new_pad, void*, (void* element, void* templ, void* name, void* caps), "pppp", element, templ, name, caps);
WRAPPER(GstElement, release_pad, void, (void* element, void* pad), "pp", element, pad);
WRAPPER(GstElement, get_state, int, (void* element, void* state, void* pending, uint64_t timeout), "pppU", element, state, pending, timeout);
WRAPPER(GstElement, set_state, int, (void* element, int state), "pi", element, state);
WRAPPER(GstElement, send_event, int, (void* element, void* event), "pp", element, event);
WRAPPER(GstElement, query, int, (void* element, void* query), "pp", element, query);
WRAPPER(GstElement, set_context, void, (void* element, void* context), "pp", element, context);

#define SUPERGO()                       \
    GO(change_state, iFpi);             \
    GO(request_new_pad, pFpppp);        \
    GO(release_pad, vFpp);              \
    GO(get_state, iFpppp);              \
    GO(set_state, iFpi);                \
    GO(send_event, iFpp);               \
    GO(query, iFpp);                    \
    GO(set_context, vFpp);              \

// wrap (so bridge all calls, just in case)
void wrapGstElementClass(void* cl)
{
    my_GstElementClass_t* class = (my_GstElementClass_t*)cl;
    wrapGstObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstElement (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGstElementClass(void* cl)
{
    my_GstElementClass_t* class = (my_GstElementClass_t*)cl;
    unwrapGstObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstElement (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGstElementClass(void* cl)
{
    my_GstElementClass_t* class = (my_GstElementClass_t*)cl;
    bridgeGstObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstElement (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

void unwrapGstElementInstance(my_GstElement_t* class)
{
    unwrapGstObjectInstance(&class->parent);
}
// autobridge
void bridgeGstElementInstance(my_GstElement_t* class)
{
    bridgeGstObjectInstance(&class->parent);
}

// ----- GstBinClass ------
WRAPPER(GstBin, add_element, int, (void* bin, void* element), "pp", bin, element);
WRAPPER(GstBin, remove_element, int, (void* bin, void* element), "pp", bin, element);
WRAPPER(GstBin, handle_message, void, (void* bin, void* message), "pp", bin, message);

#define SUPERGO()                       \
    GO(add_element, iFpp);              \
    GO(remove_element, iFpp);           \
    GO(handle_message, vFpp);           \

// wrap (so bridge all calls, just in case)
void wrapGstBinClass(void* cl)
{
    my_GstBinClass_t* class = (my_GstBinClass_t*)cl;
    wrapGstElementClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstBin (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGstBinClass(void* cl)
{
    my_GstBinClass_t* class = (my_GstBinClass_t*)cl;
    unwrapGstElementClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstBin (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGstBinClass(void* cl)
{
    my_GstBinClass_t* class = (my_GstBinClass_t*)cl;
    bridgeGstElementClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstBin (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

void unwrapGstBinInstance(my_GstBin_t* class)
{
    unwrapGstElementInstance(&class->parent);
}
// autobridge
void bridgeGstBinInstance(my_GstBin_t* class)
{
    bridgeGstElementInstance(&class->parent);
}

// ----- GstTaskPoolClass ------
WRAPPER(GstTaskPool, prepare, void, (void* pool, void* error), "pp", pool, error);
WRAPPER(GstTaskPool, cleanup, void, (void* pool), "p", pool);
WRAPPER(GstTaskPool, push, void*, (void* pool, void* func, void* user_data, void* error), "pppp", pool, func, user_data, error);
WRAPPER(GstTaskPool, join, void, (void* pool, void* id), "pp", pool, id);

#define SUPERGO()                       \
    GO(prepare, vFpp);                  \
    GO(cleanup, vFp);                   \
    GO(push, pFpppp);                   \
    GO(join, vFpp);                     \

// wrap (so bridge all calls, just in case)
void wrapGstTaskPoolClass(void* cl)
{
    my_GstTaskPoolClass_t* class = (my_GstTaskPoolClass_t*)cl;
    wrapGstObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GstTaskPool (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGstTaskPoolClass(void* cl)
{
    my_GstTaskPoolClass_t* class = (my_GstTaskPoolClass_t*)cl;
    unwrapGstObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GstTaskPool (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGstTaskPoolClass(void* cl)
{
    my_GstTaskPoolClass_t* class = (my_GstTaskPoolClass_t*)cl;
    bridgeGstObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GstTaskPool (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGstTaskPoolInstance(my_GstTaskPool_t* class)
{
    unwrapGstObjectInstance(&class->parent);
}
// autobridge
void bridgeGstTaskPoolInstance(my_GstTaskPool_t* class)
{
    bridgeGstObjectInstance(&class->parent);
}

// ----- GstURIHandlerInterface ------
WRAPPER(GstURIHandler,get_type, int, (size_t type), "L", type);
WRAPPER(GstURIHandler,get_protocols, void*, (size_t type), "L", type);
WRAPPER(GstURIHandler,get_uri, void*, (void* handler), "p", handler);
WRAPPER(GstURIHandler,set_uri, int, (void* handler, void* uri, void* error), "ppp", handler, uri, error);

#define SUPERGO()                       \
    GO(get_type, iFL);                  \
    GO(get_protocols, pFL);             \
    GO(get_uri, pFp);                   \
    GO(set_uri, iFppp);                 \

// wrap (so bridge all calls, just in case)
void wrapGstURIHandlerInterface(my_GstURIHandlerInterface_t* iface)
{
    // parent don't need wrazpping
    #define GO(A, W) iface->A = reverse_##A##_GstURIHandler (W, iface->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGstURIHandlerInterface(my_GstURIHandlerInterface_t* iface)
{
    // parent don't need wrazpping
    #define GO(A, W)   iface->A = find_##A##_GstURIHandler (W, iface->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGstURIHandlerInterface(my_GstURIHandlerInterface_t* iface)
{
    // parent don't need wrazpping
    #define GO(A, W) autobridge_##A##_GstURIHandler (W, iface->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

// ----- Additional GStreamer Instance functions for linker compatibility -----

void unwrapGstBaseTransformInstance(my_GstBaseTransform_t* class)
{
    unwrapGstElementInstance(&class->parent);
}

void bridgeGstBaseTransformInstance(my_GstBaseTransform_t* class)
{
    bridgeGstElementInstance(&class->parent);
}

void unwrapGstVideoDecoderInstance(my_GstVideoDecoder_t* class)
{
    unwrapGstElementInstance(&class->parent);
}

void bridgeGstVideoDecoderInstance(my_GstVideoDecoder_t* class)
{
    bridgeGstElementInstance(&class->parent);
}

void unwrapGstVideoEncoderInstance(my_GstVideoEncoder_t* class)
{
    unwrapGstElementInstance(&class->parent);
}

void bridgeGstVideoEncoderInstance(my_GstVideoEncoder_t* class)
{
    bridgeGstElementInstance(&class->parent);
}

void unwrapGstBaseSinkInstance(my_GstBaseSink_t* class)
{
    unwrapGstElementInstance(&class->parent);
}

void bridgeGstBaseSinkInstance(my_GstBaseSink_t* class)
{
    bridgeGstElementInstance(&class->parent);
}

void unwrapGstVideoSinkInstance(my_GstVideoSink_t* class)
{
    unwrapGstBaseSinkInstance(&class->parent);
}

void bridgeGstVideoSinkInstance(my_GstVideoSink_t* class)
{
    bridgeGstBaseSinkInstance(&class->parent);
}

void unwrapGstGLBaseFilterInstance(my_GstGLBaseFilter_t* class)
{
    unwrapGstBaseTransformInstance(&class->parent);
}

void bridgeGstGLBaseFilterInstance(my_GstGLBaseFilter_t* class)
{
    bridgeGstBaseTransformInstance(&class->parent);
}

void unwrapGstGLFilterInstance(my_GstGLFilter_t* class)
{
    unwrapGstGLBaseFilterInstance(&class->parent);
}

void bridgeGstGLFilterInstance(my_GstGLFilter_t* class)
{
    bridgeGstGLBaseFilterInstance(&class->parent);
}

void unwrapGstAggregatorInstance(my_GstAggregator_t* class)
{
    unwrapGstElementInstance(&class->parent);
}

void bridgeGstAggregatorInstance(my_GstAggregator_t* class)
{
    bridgeGstElementInstance(&class->parent);
}

void unwrapGstVideoAggregatorInstance(my_GstVideoAggregator_t* class)
{
    unwrapGstAggregatorInstance(&class->aggregator);
}

void bridgeGstVideoAggregatorInstance(my_GstVideoAggregator_t* class)
{
    bridgeGstAggregatorInstance(&class->aggregator);
}

void unwrapGstPadInstance(my_GstPad_t* class)
{
    unwrapGstObjectInstance(&class->parent);
}

void bridgeGstPadInstance(my_GstPad_t* class)
{
    bridgeGstObjectInstance(&class->parent);
}

void unwrapGstAggregatorPadInstance(my_GstAggregatorPad_t* class)
{
    unwrapGstPadInstance(&class->parent);
}

void bridgeGstAggregatorPadInstance(my_GstAggregatorPad_t* class)
{
    bridgeGstPadInstance(&class->parent);
}

void unwrapGstVideoAggregatorPadInstance(my_GstVideoAggregatorPad_t* class)
{
    unwrapGstAggregatorPadInstance(&class->parent);
}

void bridgeGstVideoAggregatorPadInstance(my_GstVideoAggregatorPad_t* class)
{
    bridgeGstAggregatorPadInstance(&class->parent);
}

void unwrapGstBaseSrcInstance(my_GstBaseSrc_t* class)
{
    unwrapGstElementInstance(&class->parent);
}

void bridgeGstBaseSrcInstance(my_GstBaseSrc_t* class)
{
    bridgeGstElementInstance(&class->parent);
}

void unwrapGstPushSrcInstance(my_GstPushSrc_t* class)
{
    unwrapGstBaseSrcInstance(&class->parent);
}

void bridgeGstPushSrcInstance(my_GstPushSrc_t* class)
{
    bridgeGstBaseSrcInstance(&class->parent);
}

void unwrapGstGLBaseSrcInstance(my_GstGLBaseSrc_t* class)
{
    unwrapGstPushSrcInstance(&class->parent);
}

void bridgeGstGLBaseSrcInstance(my_GstGLBaseSrc_t* class)
{
    bridgeGstPushSrcInstance(&class->parent);
}

void unwrapGstAudioDecoderInstance(my_GstAudioDecoder_t* class)
{
    unwrapGstElementInstance(&class->parent);
}

void bridgeGstAudioDecoderInstance(my_GstAudioDecoder_t* class)
{
    bridgeGstElementInstance(&class->parent);
}

void unwrapGstAudioEncoderInstance(my_GstAudioEncoder_t* class)
{
    unwrapGstElementInstance(&class->parent);
}

void bridgeGstAudioEncoderInstance(my_GstAudioEncoder_t* class)
{
    bridgeGstElementInstance(&class->parent);
}

void unwrapGstVideoFilterInstance(my_GstVideoFilter_t* class)
{
    unwrapGstBaseTransformInstance(&class->parent);
}

void bridgeGstVideoFilterInstance(my_GstVideoFilter_t* class)
{
    bridgeGstBaseTransformInstance(&class->parent);
}

void unwrapGstAudioFilterInstance(my_GstAudioFilter_t* class)
{
    unwrapGstBaseTransformInstance(&class->parent);
}

void bridgeGstAudioFilterInstance(my_GstAudioFilter_t* class)
{
    bridgeGstBaseTransformInstance(&class->parent);
}

void unwrapGstBufferPoolInstance(my_GstBufferPool_t* class)
{
    unwrapGstObjectInstance(&class->object);
}

void bridgeGstBufferPoolInstance(my_GstBufferPool_t* class)
{
    bridgeGstObjectInstance(&class->object);
}

void unwrapGstVideoBufferPoolInstance(my_GstVideoBufferPool_t* class)
{
    unwrapGstBufferPoolInstance(&class->bufferpool);
}

void bridgeGstVideoBufferPoolInstance(my_GstVideoBufferPool_t* class)
{
    bridgeGstBufferPoolInstance(&class->bufferpool);
}

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "gtk_macros.h"

/*
 * GTK Class Core Infrastructure
 * Contains all global variables, main dispatch functions,
 */

// Global variables - definitions for what was declared in gtk_macros.h
bridge_t* my_bridge = NULL;
const char* (*g_type_name)(size_t) = NULL;
size_t (*g_type_parent)(size_t) = NULL;
void* (*g_type_class_peek)(size_t) = NULL;

// GTK class ID definitions - one for each class
#define GTKCLASS(A) size_t my_##A = (size_t)-1;
#define GTKIFACE(A) GTKCLASS(A)
GTKCLASSES()
#undef GTKIFACE
#undef GTKCLASS

// Hash map variable definitions (types defined in gtk_common.h)
kh_signalmap_t *my_signalmap = NULL;
kh_sigoffset_t *my_sigoffset = NULL;
kh_customclass_t *my_customclass = NULL;

// Core infrastructure functions

// Custom class registration functions
void addRegisteredClass(size_t klass, char* name)
{
    if(!klass)
        return;
    if(!my_customclass) {
        my_customclass = kh_init(customclass);
    }
    khint_t k;
    int ret;
    k = kh_put(customclass, my_customclass, klass, &ret);
    kh_value(my_customclass, k) = strdup(name);
}

int checkRegisteredClass(size_t klass)
{
    if(!my_customclass)
        return 0;
    khint_t k = kh_get(customclass, my_customclass, klass);
    return (k==kh_end(my_customclass))?0:1;
}

// Main GTK class dispatch functions that call into individual class modules
void wrapGTKClass(void* cl, size_t type)
{
    #define GTKIFACE(A)
    #define GTKCLASS(A)                             \
    if(type==my_##A)                                \
        wrap##A##Class((my_##A##Class_t*)cl);       \
    else

    printf_log(LOG_DEBUG, "wrapGTKClass(%p, %zd (%s))\n", cl, type, g_type_name(type));
    GTKCLASSES()
    if(type<0x35) {}  // GInterface (8) and other simple objects have no structure
    else {
        if(my_MetaFrames2==(size_t)-1 && !strcmp(g_type_name(type), "MetaFrames")) {
            my_MetaFrames2 = type;
            wrapMetaFrames2Class((my_MetaFrames2Class_t*)cl);
        } else
            printf_log(LOG_NONE, "Warning, Custom Class initializer with unknown class type 0x%zx (%s)\n", type, g_type_name(type));
    }
    #undef GTKCLASS
    #undef GTKIFACE
}

void unwrapGTKClass(void* cl, size_t type)
{
    #define GTKIFACE(A)
    #define GTKCLASS(A)                             \
    if(type==my_##A)                                \
        unwrap##A##Class((my_##A##Class_t*)cl);     \
    else

    printf_log(LOG_DEBUG, "...unwrapGTKClass(%p, %zd (%s))\n", cl, type, g_type_name(type));
    GTKCLASSES()
    if(type<0x35) {}  // GInterface (8) and other simple objects have no structure
    else
        printf_log(LOG_NONE, "Warning: fail to unwrapGTKClass for type %zx (%s)\n", type, g_type_name(type));
    #undef GTKCLASS
    #undef GTKIFACE
}

void bridgeGTKClass(void* cl, size_t type)
{
    #define GTKIFACE(A)
    #define GTKCLASS(A)                             \
    if(type==my_##A)                                \
        bridge##A##Class((my_##A##Class_t*)cl);     \
    else

    printf_log(LOG_DEBUG, "bridgeGTKClass(%p, %zd (%s))\n", cl, type, g_type_name(type));
    GTKCLASSES()
    if(type<0x35) {}  // GInterface (8) and other simple objects have no structure
    else {
        printf_log(LOG_NONE, "Warning, AutoBridge GTK Class with unknown class type 0x%zx (%s)\n", type, g_type_name(type));
    }
    #undef GTKCLASS
    #undef GTKIFACE
}

void wrapGTKInterface(void* cl, size_t type)
{
    #define GTKCLASS(A)
    #define GTKIFACE(A)                                 \
    if(type==my_##A)                                    \
        wrap##A##Interface((my_##A##Interface_t*)cl);   \
    else

    printf_log(LOG_DEBUG, "wrapGTKInterface(%p, %zd (%s))\n", cl, type, g_type_name(type));
    GTKCLASSES()
    if(type<0x35) {}  // GInterface (8) and other simple objects have no structure
    else {
        printf_log(LOG_NONE, "Warning, Custom Interface initializer with unknown class type 0x%zx (%s)\n", type, g_type_name(type));
    }
    #undef GTKIFACE
    #undef GTKCLASS
}

void unwrapGTKInterface(void* cl, size_t type)
{
    #define GTKCLASS(A)
    #define GTKIFACE(A)                                 \
    if(type==my_##A)                                    \
        unwrap##A##Interface((my_##A##Interface_t*)cl); \
    else

    printf_log(LOG_DEBUG, "unwrapGTKInterface(%p, %zd (%s))\n", cl, type, g_type_name(type));
    GTKCLASSES()
    if(type<0x35) {}  // GInterface (8) and other simple objects have no structure
    else
    {}  // else no warning, one is enough...
    #undef GTKIFACE
    #undef GTKCLASS
}

void bridgeGTKInterface(void* cl, size_t type)
{
    #define GTKCLASS(A)
    #define GTKIFACE(A)                                 \
    if(type==my_##A)                                    \
        bridge##A##Interface((my_##A##Interface_t*)cl); \
    else

    printf_log(LOG_DEBUG, "bridgeGTKInterface(%p, %zd (%s))\n", cl, type, g_type_name(type));
    GTKCLASSES()
    if(type<0x35) {}  // GInterface (8) and other simple objects have no structure
    else {
        printf_log(LOG_NONE, "Warning, AutoBridge GTK Interface with unknown class type 0x%zx (%s)\n", type, g_type_name(type));
    }
    #undef GTKCLASS
    #undef GTKIFACE
}

void unwrapGTKInstance(void* cl, size_t type)
{
    printf_log(LOG_DEBUG, "unwrapGTKInstance(%p, %zd (%s))\n", cl, type, g_type_name(type));

    // Only call Instance functions for classes that actually have them
    if(type==my_GObject)
        unwrapGObjectInstance((my_GObject_t*)cl);
    else if(type==my_GInitiallyUnowned)
        unwrapGInitiallyUnownedInstance((my_GInitiallyUnowned_t*)cl);
    else if(type==my_GApplication)
        unwrapGApplicationInstance((my_GApplication_t*)cl);
    else if(type==my_GtkApplication)
        unwrapGtkApplicationInstance((my_GtkApplication_t*)cl);
    else if(type==my_GtkObject)
        unwrapGtkObjectInstance((my_GtkObject_t*)cl);
    else if(type==my_GtkWidget2)
        unwrapGtkWidget2Instance((my_GtkWidget2_t*)cl);
    else if(type==my_GtkWidget3)
        unwrapGtkWidget3Instance((my_GtkWidget3_t*)cl);
    else if(type==my_GtkContainer2)
        unwrapGtkContainer2Instance((my_GtkContainer2_t*)cl);
    else if(type==my_GtkContainer3)
        unwrapGtkContainer3Instance((my_GtkContainer3_t*)cl);
    else if(type==my_GtkAction)
        unwrapGtkActionInstance((my_GtkAction_t*)cl);
    else if(type==my_GtkDrawingArea3)
        unwrapGtkDrawingArea3Instance((my_GtkDrawingArea3_t*)cl);
    else if(type==my_GtkMisc2)
        unwrapGtkMisc2Instance((my_GtkMisc2_t*)cl);
    else if(type==my_GtkMisc3)
        unwrapGtkMisc3Instance((my_GtkMisc3_t*)cl);
    else if(type==my_GtkImage3)
        unwrapGtkImage3Instance((my_GtkImage3_t*)cl);
    else if(type==my_GtkLabel2)
        unwrapGtkLabel2Instance((my_GtkLabel2_t*)cl);
    else if(type==my_GtkLabel3)
        unwrapGtkLabel3Instance((my_GtkLabel3_t*)cl);
    else if(type==my_GtkTreeView2)
        unwrapGtkTreeView2Instance((my_GtkTreeView2_t*)cl);
    else if(type==my_GtkBin2)
        unwrapGtkBin2Instance((my_GtkBin2_t*)cl);
    else if(type==my_GtkBin3)
        unwrapGtkBin3Instance((my_GtkBin3_t*)cl);
    else if(type==my_GtkWindow2)
        unwrapGtkWindow2Instance((my_GtkWindow2_t*)cl);
    else if(type==my_GtkWindow3)
        unwrapGtkWindow3Instance((my_GtkWindow3_t*)cl);
    else if(type==my_GtkTable2)
        unwrapGtkTable2Instance((my_GtkTable2_t*)cl);
    else if(type==my_GtkFixed2)
        unwrapGtkFixed2Instance((my_GtkFixed2_t*)cl);
    else if(type==my_GtkFixed3)
        unwrapGtkFixed3Instance((my_GtkFixed3_t*)cl);
    else if(type==my_GtkApplicationWindow)
        unwrapGtkApplicationWindowInstance((my_GtkApplicationWindow_t*)cl);
    else if(type==my_GtkListBox)
        unwrapGtkListBoxInstance((my_GtkListBox_t*)cl);
    else if(type==my_GtkListBoxRow)
        unwrapGtkListBoxRowInstance((my_GtkListBoxRow_t*)cl);
    else if(type==my_GtkButton2)
        unwrapGtkButton2Instance((my_GtkButton2_t*)cl);
    else if(type==my_GtkButton3)
        unwrapGtkButton3Instance((my_GtkButton3_t*)cl);
    else if(type==my_GtkComboBox2)
        unwrapGtkComboBox2Instance((my_GtkComboBox2_t*)cl);
    else if(type==my_GtkToggleButton2)
        unwrapGtkToggleButton2Instance((my_GtkToggleButton2_t*)cl);
    else if(type==my_GtkToggleButton3)
        unwrapGtkToggleButton3Instance((my_GtkToggleButton3_t*)cl);
    else if(type==my_GtkMenuButton3)
        unwrapGtkMenuButton3Instance((my_GtkMenuButton3_t*)cl);
    else if(type==my_GtkCheckButton2)
        unwrapGtkCheckButton2Instance((my_GtkCheckButton2_t*)cl);
    else if(type==my_GtkCheckButton3)
        unwrapGtkCheckButton3Instance((my_GtkCheckButton3_t*)cl);
    else if(type==my_GtkEntry2)
        unwrapGtkEntry2Instance((my_GtkEntry2_t*)cl);
    else if(type==my_GtkSpinButton2)
        unwrapGtkSpinButton2Instance((my_GtkSpinButton2_t*)cl);
    else if(type==my_GtkProgress2)
        unwrapGtkProgress2Instance((my_GtkProgress2_t*)cl);
    else if(type==my_GtkProgressBar2)
        unwrapGtkProgressBar2Instance((my_GtkProgressBar2_t*)cl);
    else if(type==my_GtkFrame2)
        unwrapGtkFrame2Instance((my_GtkFrame2_t*)cl);
    else if(type==my_GtkMenuShell2)
        unwrapGtkMenuShell2Instance((my_GtkMenuShell2_t*)cl);
    else if(type==my_GtkMenuBar2)
        unwrapGtkMenuBar2Instance((my_GtkMenuBar2_t*)cl);
    else if(type==my_GtkTextView2)
        unwrapGtkTextView2Instance((my_GtkTextView2_t*)cl);
    else if(type==my_GtkTextView3)
        unwrapGtkTextView3Instance((my_GtkTextView3_t*)cl);
    else if(type==my_GtkGrid3)
        unwrapGtkGrid3Instance((my_GtkGrid3_t*)cl);
    else if(type==my_GtkEventController)
        unwrapGtkEventControllerInstance((my_GtkEventController_t*)cl);
    else if(type==my_GtkGesture)
        unwrapGtkGestureInstance((my_GtkGesture_t*)cl);
    else if(type==my_GtkGestureSingle)
        unwrapGtkGestureSingleInstance((my_GtkGestureSingle_t*)cl);
    else if(type==my_GtkGestureLongPress)
        unwrapGtkGestureLongPressInstance((my_GtkGestureLongPress_t*)cl);
    else if(type==my_GtkNotebook2)
        unwrapGtkNotebook2Instance((my_GtkNotebook2_t*)cl);
    else if(type==my_GtkCellRenderer2)
        unwrapGtkCellRenderer2Instance((my_GtkCellRenderer2_t*)cl);
    else if(type==my_GtkCellRendererText2)
        unwrapGtkCellRendererText2Instance((my_GtkCellRendererText2_t*)cl);
    else if(type==my_MetaFrames2)
        unwrapMetaFrames2Instance((my_MetaFrames2_t*)cl);
    else if(type==my_GDBusObjectManagerClient)
        unwrapGDBusObjectManagerClientInstance((my_GDBusObjectManagerClient_t*)cl);
    else if(type==my_GDBusInterfaceSkeleton)
        unwrapGDBusInterfaceSkeletonInstance((my_GDBusInterfaceSkeleton_t*)cl);
    else if(type==my_AtkObject)
        unwrapAtkObjectInstance((my_AtkObject_t*)cl);
    else if(type==my_AtkUtil)
        unwrapAtkUtilInstance((my_AtkUtil_t*)cl);
    else if(type==my_GstObject)
        unwrapGstObjectInstance((my_GstObject_t*)cl);
    else if(type==my_GstAllocator)
        unwrapGstAllocatorInstance((my_GstAllocator_t*)cl);
    else if(type==my_GstTaskPool)
        unwrapGstTaskPoolInstance((my_GstTaskPool_t*)cl);
    else if(type==my_GDBusProxy)
        unwrapGDBusProxyInstance((my_GDBusProxy_t*)cl);
    else if(type==my_GstElement)
        unwrapGstElementInstance((my_GstElement_t*)cl);
    else if(type==my_GstBin)
        unwrapGstBinInstance((my_GstBin_t*)cl);
    else if(type==my_GstBaseTransform)
        unwrapGstBaseTransformInstance((my_GstBaseTransform_t*)cl);
    else if(type==my_GstVideoDecoder)
        unwrapGstVideoDecoderInstance((my_GstVideoDecoder_t*)cl);
    else if(type==my_GstVideoEncoder)
        unwrapGstVideoEncoderInstance((my_GstVideoEncoder_t*)cl);
    else if(type==my_GstBaseSink)
        unwrapGstBaseSinkInstance((my_GstBaseSink_t*)cl);
    else if(type==my_GstVideoSink)
        unwrapGstVideoSinkInstance((my_GstVideoSink_t*)cl);
    else if(type==my_GstGLBaseFilter)
        unwrapGstGLBaseFilterInstance((my_GstGLBaseFilter_t*)cl);
    else if(type==my_GstGLFilter)
        unwrapGstGLFilterInstance((my_GstGLFilter_t*)cl);
    else if(type==my_GstAggregator)
        unwrapGstAggregatorInstance((my_GstAggregator_t*)cl);
    else if(type==my_GstVideoAggregator)
        unwrapGstVideoAggregatorInstance((my_GstVideoAggregator_t*)cl);
    else if(type==my_GstPad)
        unwrapGstPadInstance((my_GstPad_t*)cl);
    else if(type==my_GstAggregatorPad)
        unwrapGstAggregatorPadInstance((my_GstAggregatorPad_t*)cl);
    else if(type==my_GstVideoAggregatorPad)
        unwrapGstVideoAggregatorPadInstance((my_GstVideoAggregatorPad_t*)cl);
    else if(type==my_GstBaseSrc)
        unwrapGstBaseSrcInstance((my_GstBaseSrc_t*)cl);
    else if(type==my_GstPushSrc)
        unwrapGstPushSrcInstance((my_GstPushSrc_t*)cl);
    else if(type==my_GstGLBaseSrc)
        unwrapGstGLBaseSrcInstance((my_GstGLBaseSrc_t*)cl);
    else if(type==my_GstAudioDecoder)
        unwrapGstAudioDecoderInstance((my_GstAudioDecoder_t*)cl);
    else if(type==my_GstAudioEncoder)
        unwrapGstAudioEncoderInstance((my_GstAudioEncoder_t*)cl);
    else if(type==my_GstVideoFilter)
        unwrapGstVideoFilterInstance((my_GstVideoFilter_t*)cl);
    else if(type==my_GstAudioFilter)
        unwrapGstAudioFilterInstance((my_GstAudioFilter_t*)cl);
    else if(type==my_GstBufferPool)
        unwrapGstBufferPoolInstance((my_GstBufferPool_t*)cl);
    else if(type==my_GstVideoBufferPool)
        unwrapGstVideoBufferPoolInstance((my_GstVideoBufferPool_t*)cl);
    else if(type<0x35) {}  // GInterface (8) and other simple objects have no structure
    else
    {}  // else no warning, one is enough...
}

void bridgeGTKInstance(void* cl, size_t type)
{
    printf_log(LOG_DEBUG, "bridgeGTKInstance(%p, %zd (%s))\n", cl, type, g_type_name(type));

    // Only call Instance functions for classes that actually have them
    if(type==my_GObject)
        bridgeGObjectInstance((my_GObject_t*)cl);
    else if(type==my_GInitiallyUnowned)
        bridgeGInitiallyUnownedInstance((my_GInitiallyUnowned_t*)cl);
    else if(type==my_GApplication)
        bridgeGApplicationInstance((my_GApplication_t*)cl);
    else if(type==my_GtkApplication)
        bridgeGtkApplicationInstance((my_GtkApplication_t*)cl);
    else if(type==my_GtkObject)
        bridgeGtkObjectInstance((my_GtkObject_t*)cl);
    else if(type==my_GtkWidget2)
        bridgeGtkWidget2Instance((my_GtkWidget2_t*)cl);
    else if(type==my_GtkWidget3)
        bridgeGtkWidget3Instance((my_GtkWidget3_t*)cl);
    else if(type==my_GtkContainer2)
        bridgeGtkContainer2Instance((my_GtkContainer2_t*)cl);
    else if(type==my_GtkContainer3)
        bridgeGtkContainer3Instance((my_GtkContainer3_t*)cl);
    else if(type==my_GtkAction)
        bridgeGtkActionInstance((my_GtkAction_t*)cl);
    else if(type==my_GtkDrawingArea3)
        bridgeGtkDrawingArea3Instance((my_GtkDrawingArea3_t*)cl);
    else if(type==my_GtkMisc2)
        bridgeGtkMisc2Instance((my_GtkMisc2_t*)cl);
    else if(type==my_GtkMisc3)
        bridgeGtkMisc3Instance((my_GtkMisc3_t*)cl);
    else if(type==my_GtkImage3)
        bridgeGtkImage3Instance((my_GtkImage3_t*)cl);
    else if(type==my_GtkLabel2)
        bridgeGtkLabel2Instance((my_GtkLabel2_t*)cl);
    else if(type==my_GtkLabel3)
        bridgeGtkLabel3Instance((my_GtkLabel3_t*)cl);
    else if(type==my_GtkTreeView2)
        bridgeGtkTreeView2Instance((my_GtkTreeView2_t*)cl);
    else if(type==my_GtkBin2)
        bridgeGtkBin2Instance((my_GtkBin2_t*)cl);
    else if(type==my_GtkBin3)
        bridgeGtkBin3Instance((my_GtkBin3_t*)cl);
    else if(type==my_GtkWindow2)
        bridgeGtkWindow2Instance((my_GtkWindow2_t*)cl);
    else if(type==my_GtkWindow3)
        bridgeGtkWindow3Instance((my_GtkWindow3_t*)cl);
    else if(type==my_GtkTable2)
        bridgeGtkTable2Instance((my_GtkTable2_t*)cl);
    else if(type==my_GtkFixed2)
        bridgeGtkFixed2Instance((my_GtkFixed2_t*)cl);
    else if(type==my_GtkFixed3)
        bridgeGtkFixed3Instance((my_GtkFixed3_t*)cl);
    else if(type==my_GtkApplicationWindow)
        bridgeGtkApplicationWindowInstance((my_GtkApplicationWindow_t*)cl);
    else if(type==my_GtkListBox)
        bridgeGtkListBoxInstance((my_GtkListBox_t*)cl);
    else if(type==my_GtkListBoxRow)
        bridgeGtkListBoxRowInstance((my_GtkListBoxRow_t*)cl);
    else if(type==my_GtkButton2)
        bridgeGtkButton2Instance((my_GtkButton2_t*)cl);
    else if(type==my_GtkButton3)
        bridgeGtkButton3Instance((my_GtkButton3_t*)cl);
    else if(type==my_GtkComboBox2)
        bridgeGtkComboBox2Instance((my_GtkComboBox2_t*)cl);
    else if(type==my_GtkToggleButton2)
        bridgeGtkToggleButton2Instance((my_GtkToggleButton2_t*)cl);
    else if(type==my_GtkToggleButton3)
        bridgeGtkToggleButton3Instance((my_GtkToggleButton3_t*)cl);
    else if(type==my_GtkMenuButton3)
        bridgeGtkMenuButton3Instance((my_GtkMenuButton3_t*)cl);
    else if(type==my_GtkCheckButton2)
        bridgeGtkCheckButton2Instance((my_GtkCheckButton2_t*)cl);
    else if(type==my_GtkCheckButton3)
        bridgeGtkCheckButton3Instance((my_GtkCheckButton3_t*)cl);
    else if(type==my_GtkEntry2)
        bridgeGtkEntry2Instance((my_GtkEntry2_t*)cl);
    else if(type==my_GtkSpinButton2)
        bridgeGtkSpinButton2Instance((my_GtkSpinButton2_t*)cl);
    else if(type==my_GtkProgress2)
        bridgeGtkProgress2Instance((my_GtkProgress2_t*)cl);
    else if(type==my_GtkProgressBar2)
        bridgeGtkProgressBar2Instance((my_GtkProgressBar2_t*)cl);
    else if(type==my_GtkFrame2)
        bridgeGtkFrame2Instance((my_GtkFrame2_t*)cl);
    else if(type==my_GtkMenuShell2)
        bridgeGtkMenuShell2Instance((my_GtkMenuShell2_t*)cl);
    else if(type==my_GtkMenuBar2)
        bridgeGtkMenuBar2Instance((my_GtkMenuBar2_t*)cl);
    else if(type==my_GtkTextView2)
        bridgeGtkTextView2Instance((my_GtkTextView2_t*)cl);
    else if(type==my_GtkTextView3)
        bridgeGtkTextView3Instance((my_GtkTextView3_t*)cl);
    else if(type==my_GtkGrid3)
        bridgeGtkGrid3Instance((my_GtkGrid3_t*)cl);
    else if(type==my_GtkEventController)
        bridgeGtkEventControllerInstance((my_GtkEventController_t*)cl);
    else if(type==my_GtkGesture)
        bridgeGtkGestureInstance((my_GtkGesture_t*)cl);
    else if(type==my_GtkGestureSingle)
        bridgeGtkGestureSingleInstance((my_GtkGestureSingle_t*)cl);
    else if(type==my_GtkGestureLongPress)
        bridgeGtkGestureLongPressInstance((my_GtkGestureLongPress_t*)cl);
    else if(type==my_GtkNotebook2)
        bridgeGtkNotebook2Instance((my_GtkNotebook2_t*)cl);
    else if(type==my_GtkCellRenderer2)
        bridgeGtkCellRenderer2Instance((my_GtkCellRenderer2_t*)cl);
    else if(type==my_GtkCellRendererText2)
        bridgeGtkCellRendererText2Instance((my_GtkCellRendererText2_t*)cl);
    else if(type==my_MetaFrames2)
        bridgeMetaFrames2Instance((my_MetaFrames2_t*)cl);
    else if(type==my_GDBusObjectManagerClient)
        bridgeGDBusObjectManagerClientInstance((my_GDBusObjectManagerClient_t*)cl);
    else if(type==my_GDBusInterfaceSkeleton)
        bridgeGDBusInterfaceSkeletonInstance((my_GDBusInterfaceSkeleton_t*)cl);
    else if(type==my_AtkObject)
        bridgeAtkObjectInstance((my_AtkObject_t*)cl);
    else if(type==my_AtkUtil)
        bridgeAtkUtilInstance((my_AtkUtil_t*)cl);
    else if(type==my_GstObject)
        bridgeGstObjectInstance((my_GstObject_t*)cl);
    else if(type==my_GstAllocator)
        bridgeGstAllocatorInstance((my_GstAllocator_t*)cl);
    else if(type==my_GstTaskPool)
        bridgeGstTaskPoolInstance((my_GstTaskPool_t*)cl);
    else if(type==my_GDBusProxy)
        bridgeGDBusProxyInstance((my_GDBusProxy_t*)cl);
    else if(type==my_GstElement)
        bridgeGstElementInstance((my_GstElement_t*)cl);
    else if(type==my_GstBin)
        bridgeGstBinInstance((my_GstBin_t*)cl);
    else if(type==my_GstBaseTransform)
        bridgeGstBaseTransformInstance((my_GstBaseTransform_t*)cl);
    else if(type==my_GstVideoDecoder)
        bridgeGstVideoDecoderInstance((my_GstVideoDecoder_t*)cl);
    else if(type==my_GstVideoEncoder)
        bridgeGstVideoEncoderInstance((my_GstVideoEncoder_t*)cl);
    else if(type==my_GstBaseSink)
        bridgeGstBaseSinkInstance((my_GstBaseSink_t*)cl);
    else if(type==my_GstVideoSink)
        bridgeGstVideoSinkInstance((my_GstVideoSink_t*)cl);
    else if(type==my_GstGLBaseFilter)
        bridgeGstGLBaseFilterInstance((my_GstGLBaseFilter_t*)cl);
    else if(type==my_GstGLFilter)
        bridgeGstGLFilterInstance((my_GstGLFilter_t*)cl);
    else if(type==my_GstAggregator)
        bridgeGstAggregatorInstance((my_GstAggregator_t*)cl);
    else if(type==my_GstVideoAggregator)
        bridgeGstVideoAggregatorInstance((my_GstVideoAggregator_t*)cl);
    else if(type==my_GstPad)
        bridgeGstPadInstance((my_GstPad_t*)cl);
    else if(type==my_GstAggregatorPad)
        bridgeGstAggregatorPadInstance((my_GstAggregatorPad_t*)cl);
    else if(type==my_GstVideoAggregatorPad)
        bridgeGstVideoAggregatorPadInstance((my_GstVideoAggregatorPad_t*)cl);
    else if(type==my_GstBaseSrc)
        bridgeGstBaseSrcInstance((my_GstBaseSrc_t*)cl);
    else if(type==my_GstPushSrc)
        bridgeGstPushSrcInstance((my_GstPushSrc_t*)cl);
    else if(type==my_GstGLBaseSrc)
        bridgeGstGLBaseSrcInstance((my_GstGLBaseSrc_t*)cl);
    else if(type==my_GstAudioDecoder)
        bridgeGstAudioDecoderInstance((my_GstAudioDecoder_t*)cl);
    else if(type==my_GstAudioEncoder)
        bridgeGstAudioEncoderInstance((my_GstAudioEncoder_t*)cl);
    else if(type==my_GstVideoFilter)
        bridgeGstVideoFilterInstance((my_GstVideoFilter_t*)cl);
    else if(type==my_GstAudioFilter)
        bridgeGstAudioFilterInstance((my_GstAudioFilter_t*)cl);
    else if(type==my_GstBufferPool)
        bridgeGstBufferPoolInstance((my_GstBufferPool_t*)cl);
    else if(type==my_GstVideoBufferPool)
        bridgeGstVideoBufferPoolInstance((my_GstVideoBufferPool_t*)cl);
    else if(type<0x35) {}  // GInterface (8) and other simple objects have no structure
    else {
        printf_log(LOG_NONE, "Warning, AutoBridge GTK Class with unknown class type 0x%zx (%s)\n", type, g_type_name(type));
    }
}

// Copy functions for class wrapping/unwrapping
typedef union my_GClassAll_s {
    #define GTKCLASS(A) my_##A##Class_t A;
    #define GTKIFACE(A) my_##A##Interface_t A;
    GTKCLASSES()
    #undef GTKIFACE
    #undef GTKCLASS
} my_GClassAll_t;

#define GO(A) \
static void* my_gclassall_ref_##A = NULL;   \
static my_GClassAll_t my_gclassall_##A;

SUPER()
#undef GO

void* unwrapCopyGTKClass(void* klass, size_t type)
{
    if(!klass) return klass;
    if(checkRegisteredClass(type))
        return klass;
    #define GO(A) if(klass == my_gclassall_ref_##A) return &my_gclassall_##A;
    SUPER()
    #undef GO
    // check if class is the exact type we know
    size_t sz = 0;
    #define GTKIFACE(A)
    #define GTKCLASS(A) if(type==my_##A) sz = sizeof(my_##A##Class_t); else
    GTKCLASSES()
    if(type<0x35) {}  // GInterface (8) and other simple objects have no structure
    else {
        printf_log(LOG_NONE, "Warning, unwrapCopyGTKClass called with unknown class type 0x%zx (%s)\n", type, g_type_name(type));
        return klass;
    }
    #undef GTKCLASS
    #undef GTKIFACE
    my_GClassAll_t *newklass = NULL;
    #define GO(A) if(!newklass && !my_gclassall_ref_##A) {my_gclassall_ref_##A = klass; newklass = &my_gclassall_##A;}
    SUPER()
    #undef GO
    if(!newklass) {
        printf_log(LOG_NONE, "Warning: no more slot for unwrapCopyGTKClass\n");
        return klass;
    }
    memcpy(newklass, klass, sz);
    unwrapGTKClass(newklass, type);
    return newklass;
}

void* unwrapCopyGTKInterface(void* iface, size_t type)
{
    if(!iface) return iface;
    if(checkRegisteredClass(type))
        return iface;
    #define GO(A) if(iface == my_gclassall_ref_##A) return &my_gclassall_##A;
    SUPER()
    #undef GO
    // check if class is the exact type we know
    size_t sz = 0;
    #define GTKIFACE(A) if(type==my_##A) sz = sizeof(my_##A##Interface_t); else
    #define GTKCLASS(A)
    GTKCLASSES()
    if(type<0x35) {}  // GInterface (8) and other simple objects have no structure
    else {
        printf_log(LOG_NONE, "Warning, unwrapCopyGTKInterface called with unknown class type 0x%zx (%s)\n", type, g_type_name(type));
        return iface;
    }
    #undef GTKCLASS
    #undef GTKIFACE
    my_GClassAll_t *newiface = NULL;
    #define GO(A) if(!newiface && !my_gclassall_ref_##A) {my_gclassall_ref_##A = iface; newiface = &my_gclassall_##A;}
    SUPER()
    #undef GO
    if(!newiface) {
        printf_log(LOG_NONE, "Warning: no more slot for unwrapCopyGTKInterface\n");
        return iface;
    }
    memcpy(newiface, iface, sz);
    unwrapGTKInterface(newiface, type);
    return newiface;
}

void* wrapCopyGTKClass(void* klass, size_t type)
{
    if(!klass) return klass;
    while(checkRegisteredClass(type))
        type = g_type_parent(type);
    printf_log(LOG_DEBUG, "wrapCopyGTKClass(%p, %zd (%s))\n", klass, type, g_type_name(type));
    bridgeGTKClass(klass, type);
    return klass;
}

void* wrapCopyGTKInterface(void* iface, size_t type)
{
    if(!iface) return iface;
    while(checkRegisteredClass(type))
        type = g_type_parent(type);
    printf_log(LOG_DEBUG, "wrapCopyGTKInterface(%p, %zd (%s))\n", iface, type, g_type_name(type));
    bridgeGTKInterface(iface, type);
    return iface;
}

// Module initialization and cleanup functions
void InitGTKClass(bridge_t *bridge)
{
    my_bridge  = bridge;
    my_signalmap = kh_init(signalmap);
    my_sigoffset = kh_init(sigoffset);
}

void FiniGTKClass()
{
    if(my_signalmap) {
        /*khint_t k;
        kh_foreach_key(my_signalmap, k,
            my_signal_t* p = (my_signal_t*)(uintptr_t)k;
            box_free(p);
        );*/ // lets assume all signals data is freed by gtk already
        kh_destroy(signalmap, my_signalmap);
        my_signalmap = NULL;
    }
    if(my_sigoffset) {
        sigoffset_array_t* p;
        kh_foreach_value_ref(my_sigoffset, p,
            box_free(p->a);
        );
        kh_destroy(sigoffset, my_sigoffset);
        my_sigoffset = NULL;
    }
}

// GTK class ID setter functions - one for each class
#define GTKCLASS(A)             \
void Set##A##ID(size_t id)      \
{                               \
    my_##A = id;                \
}
#define GTKIFACE(A)  GTKCLASS(A)
GTKCLASSES()
#undef GTKIFACE
#undef GTKCLASS

// Automatic bridging function
void AutoBridgeGtk(void*(*ref)(size_t), void(*unref)(void*))
{
    void* p;
    #define GTKIFACE(A)
    #define GTKCLASS(A)                \
    if(my_##A && my_##A!=(size_t)-1) { \
        p = ref(my_##A);               \
        bridgeGTKClass(p, my_##A);     \
        unref(p);                      \
    }
    GTKCLASSES()
    #undef GTKIFACE
    #undef GTKCLASS
}

// Function pointer setters
void SetGTypeName(void* f)
{
    g_type_name = f;
}

void SetGClassPeek(void* f)
{
    g_type_class_peek = f;
}

void SetGTypeParent(void* f)
{
    g_type_parent = f;
}

// ---- Complete signal and type infrastructure ----

// Signal callback infrastructure (signal2 through signal10)
// signal2 ...
#define GO(A)   \
static uintptr_t my_signal2_fct_##A = 0;                                        \
static void* my_signal2_##A(void* a, void* b)                                   \
{                                                                               \
    return (void*)RunFunctionFmt(my_signal2_fct_##A, "pp", a, b);   \
}
SUPER()
#undef GO
static void* find_signal2_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_signal2_fct_##A == (uintptr_t)fct) return my_signal2_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_signal2_fct_##A == 0) {my_signal2_fct_##A = (uintptr_t)fct; return my_signal2_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal2 callback\n");
    return NULL;
}

// signal3 ...
#define GO(A)   \
static uintptr_t my_signal3_fct_##A = 0;                                            \
static void* my_signal3_##A(void* a, void* b, void* c)                              \
{                                                                                   \
    return (void*)RunFunctionFmt(my_signal3_fct_##A, "ppp", a, b, c);   \
}
SUPER()
#undef GO
static void* find_signal3_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_signal3_fct_##A == (uintptr_t)fct) return my_signal3_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_signal3_fct_##A == 0) {my_signal3_fct_##A = (uintptr_t)fct; return my_signal3_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal3 callback\n");
    return NULL;
}

// signal4 ...
#define GO(A)   \
static uintptr_t my_signal4_fct_##A = 0;                                                \
static void* my_signal4_##A(void* a, void* b, void* c, void* d)                         \
{                                                                                       \
    return (void*)RunFunctionFmt(my_signal4_fct_##A, "pppp", a, b, c, d);   \
}
SUPER()
#undef GO
static void* find_signal4_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_signal4_fct_##A == (uintptr_t)fct) return my_signal4_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_signal4_fct_##A == 0) {my_signal4_fct_##A = (uintptr_t)fct; return my_signal4_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal4 callback\n");
    return NULL;
}

// signal5 ...
#define GO(A)   \
static uintptr_t my_signal5_fct_##A = 0;                                                    \
static void* my_signal5_##A(void* a, void* b, void* c, void* d, void* e)                    \
{                                                                                           \
    return (void*)RunFunctionFmt(my_signal5_fct_##A, "ppppp", a, b, c, d, e);   \
}
SUPER()
#undef GO
static void* find_signal5_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_signal5_fct_##A == (uintptr_t)fct) return my_signal5_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_signal5_fct_##A == 0) {my_signal5_fct_##A = (uintptr_t)fct; return my_signal5_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal5 callback\n");
    return NULL;
}

// signal6 ...
#define GO(A)   \
static uintptr_t my_signal6_fct_##A = 0;                                                        \
static void* my_signal6_##A(void* a, void* b, void* c, void* d, void* e, void* f)               \
{                                                                                               \
    return (void*)RunFunctionFmt(my_signal6_fct_##A, "pppppp", a, b, c, d, e, f);   \
}
SUPER()
#undef GO
static void* find_signal6_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_signal6_fct_##A == (uintptr_t)fct) return my_signal6_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_signal6_fct_##A == 0) {my_signal6_fct_##A = (uintptr_t)fct; return my_signal6_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal6 callback\n");
    return NULL;
}

// signal7 ...
#define GO(A)   \
static uintptr_t my_signal7_fct_##A = 0;                                                            \
static void* my_signal7_##A(void* a, void* b, void* c, void* d, void* e, void* f, void* g)          \
{                                                                                                   \
    return (void*)RunFunctionFmt(my_signal7_fct_##A, "ppppppp", a, b, c, d, e, f, g);   \
}
SUPER()
#undef GO
static void* find_signal7_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_signal7_fct_##A == (uintptr_t)fct) return my_signal7_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_signal7_fct_##A == 0) {my_signal7_fct_##A = (uintptr_t)fct; return my_signal7_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal7 callback\n");
    return NULL;
}

// signal8 ...
#define GO(A)                                                                                  \
    static uintptr_t my_signal8_fct_##A = 0;                                                   \
    static void* my_signal8_##A(void* a, void* b, void* c, void* d, void* e, void* f, void* g) \
    {                                                                                          \
        return (void*)RunFunctionFmt(my_signal8_fct_##A, "ppppppp", a, b, c, d, e, f, g);      \
    }
SUPER()
#undef GO
static void* find_signal8_Fct(void* fct)
{
    if (!fct) return fct;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
#define GO(A) \
    if (my_signal8_fct_##A == (uintptr_t)fct) return my_signal8_##A;
    SUPER()
#undef GO
#define GO(A)                                \
    if (my_signal8_fct_##A == 0) {           \
        my_signal8_fct_##A = (uintptr_t)fct; \
        return my_signal8_##A;               \
    }
    SUPER()
#undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal8 callback\n");
    return NULL;
}

// signal9 ...
#define GO(A)                                                                                  \
    static uintptr_t my_signal9_fct_##A = 0;                                                   \
    static void* my_signal9_##A(void* a, void* b, void* c, void* d, void* e, void* f, void* g) \
    {                                                                                          \
        return (void*)RunFunctionFmt(my_signal9_fct_##A, "ppppppp", a, b, c, d, e, f, g);      \
    }
SUPER()
#undef GO
static void* find_signal9_Fct(void* fct)
{
    if (!fct) return fct;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
#define GO(A) \
    if (my_signal9_fct_##A == (uintptr_t)fct) return my_signal9_##A;
    SUPER()
#undef GO
#define GO(A)                                \
    if (my_signal9_fct_##A == 0) {           \
        my_signal9_fct_##A = (uintptr_t)fct; \
        return my_signal9_##A;               \
    }
    SUPER()
#undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal9 callback\n");
    return NULL;
}

// signal10 ...
#define GO(A)                                                                                   \
    static uintptr_t my_signal10_fct_##A = 0;                                                   \
    static void* my_signal10_##A(void* a, void* b, void* c, void* d, void* e, void* f, void* g) \
    {                                                                                           \
        return (void*)RunFunctionFmt(my_signal10_fct_##A, "ppppppp", a, b, c, d, e, f, g);      \
    }
SUPER()
#undef GO
static void* find_signal10_Fct(void* fct)
{
    if (!fct) return fct;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
#define GO(A) \
    if (my_signal10_fct_##A == (uintptr_t)fct) return my_signal10_##A;
    SUPER()
#undef GO
#define GO(A)                                 \
    if (my_signal10_fct_##A == 0) {           \
        my_signal10_fct_##A = (uintptr_t)fct; \
        return my_signal10_##A;               \
    }
    SUPER()
#undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal10 callback\n");
    return NULL;
}

// Signal infrastructure constants and types
typedef void* (*finder_t)(void*);
static const finder_t finders[] = { find_signal2_Fct, find_signal3_Fct, find_signal4_Fct, find_signal5_Fct, find_signal6_Fct, find_signal7_Fct, find_signal8_Fct, find_signal9_Fct, find_signal10_Fct };
#define MAX_SIGNAL_N (10 - 2)

// Forward declaration
void my_unwrap_signal_offset(void* klass);

void my_add_signal_offset(size_t itype, uint32_t offset, int n)
{
    printf_log(LOG_DEBUG, "my_add_signal_offset(0x%zx, %d, %d)\n", itype, offset, n);
    if(!offset || !itype) // no offset means no overload...
        return;
    if(n<0 || n>MAX_SIGNAL_N) {
        printf_log(LOG_NONE, "Warning, signal with too many args (%d) in my_add_signal_offset\n", n);
        return;
    }
    int ret;
    khint_t k = kh_put(sigoffset, my_sigoffset, itype, &ret);
    sigoffset_array_t *p = &kh_value(my_sigoffset, k);
    if(ret) {
        p->a = NULL; p->cap = 0; p->sz = 0;
    }
    // check if offset already there
    for(int i=0; i<p->sz; ++i)
        if(p->a[i].offset == offset) {
            printf_log(LOG_INFO, "Offset already there... Bye\n");
            return; // already there, bye
        }
    if(p->sz==p->cap) {
        p->cap+=4;
        p->a = (sigoffset_t*)box_realloc(p->a, sizeof(sigoffset_t)*p->cap);
    }
    p->a[p->sz].offset = offset;
    p->a[p->sz++].n = n;
}

void my_unwrap_signal_offset(void* klass)
{
    if(!klass)
        return;
    size_t itype = *(size_t*)klass;
    khint_t k = kh_get(sigoffset, my_sigoffset, itype);
    if(k==kh_end(my_sigoffset))
        return;
    sigoffset_array_t *p = &kh_value(my_sigoffset, k);
    printf_log(LOG_DEBUG, "my_unwrap_signal_offset(%p) type=0x%zx with %d signals with offset\n", klass, itype, p->sz);
    for(int i=0; i<p->sz; ++i) {
        void** f = (void**)((uintptr_t)klass + p->a[i].offset);
        void* new_f = GetNativeFnc((uintptr_t)*f);
        if(!new_f) {
            // Not a native function: autobridge it
            new_f = finders[p->a[i].n](f);
        }
        if(new_f != *f) {
            printf_log(LOG_DEBUG, "Unwrapping %p[%p: offset=0x%x, n=%d] -> %p (with alternate)\n", *f, f, p->a[i].offset, p->a[i].n, new_f);
            if(!hasAlternate(new_f))
                addAlternate(new_f, *f);
            *f = new_f;
        }
    }
}

// First the structure my_GTypeInfo_t statics, with paired x64 source pointer
#define GO(A) \
static my_GTypeInfo_t     my_gtypeinfo_##A = {0};   \
static my_GTypeInfo_t    ref_gtypeinfo_##A = {0};   \
static int              used_gtypeinfo_##A = 0;
SUPER()
#undef GO

// Then the static functions callback that may be used with the structure
// base_init ...
#define GO(A)   \
static uintptr_t my_base_init_fct_##A = 0;                          \
static int my_base_init_##A(void* a)                                \
{                                                                   \
    return RunFunctionFmt(my_base_init_fct_##A, "p", a);     \
}
SUPER()
#undef GO
static void* find_base_init_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_base_init_fct_##A == (uintptr_t)fct) return my_base_init_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_base_init_fct_##A == 0) {my_base_init_fct_##A = (uintptr_t)fct; return my_base_init_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo base_init callback\n");
    return NULL;
}

// base_finalize ...
#define GO(A)   \
static uintptr_t my_base_finalize_fct_##A = 0;                      \
static int my_base_finalize_##A(void* a)                            \
{                                                                   \
    return RunFunctionFmt(my_base_finalize_fct_##A, "p", a); \
}
SUPER()
#undef GO
static void* find_base_finalize_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_base_finalize_fct_##A == (uintptr_t)fct) return my_base_finalize_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_base_finalize_fct_##A == 0) {my_base_finalize_fct_##A = (uintptr_t)fct; return my_base_finalize_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo base_finalize callback\n");
    return NULL;
}

// class_init ...
#define GO(A)   \
static uintptr_t my_class_init_fct_##A = 0;                                 \
static size_t parent_class_init_##A = 0;                                    \
static int my_class_init_##A(void* a, void* b)                              \
{                                                                           \
    printf_log(LOG_DEBUG, "Custom Class init %d for class %p (parent=%p:%s)\n", A, a, (void*)parent_class_init_##A, g_type_name(parent_class_init_##A));\
    int ret = RunFunctionFmt(my_class_init_fct_##A, "pp", a, b);            \
    size_t type = parent_class_init_##A;                                    \
    while(checkRegisteredClass(type))                                       \
        type = g_type_parent(type);                                         \
    unwrapGTKClass(a, type);                                                \
    my_unwrap_signal_offset(a);                                             \
    if(!strcmp(g_type_name(type), "AtkUtil")) {                             \
        my_AtkUtilClass_t* p = (my_AtkUtilClass_t*)g_type_class_peek(type); \
        unwrapGTKClass(p, type);                                            \
    }                                                                       \
    return ret;                                                             \
}
SUPER()
#undef GO
static void* find_class_init_Fct(void* fct, size_t parent)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_class_init_fct_##A == (uintptr_t)fct && parent_class_init_##A==parent) return my_class_init_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_class_init_fct_##A == 0) {my_class_init_fct_##A = (uintptr_t)fct; parent_class_init_##A=parent; return my_class_init_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo class_init callback\n");
    return NULL;
}

// class_finalize ...
#define GO(A)   \
static uintptr_t my_class_finalize_fct_##A = 0;                                 \
static int my_class_finalize_##A(void* a, void* b)                              \
{                                                                               \
    return RunFunctionFmt(my_class_finalize_fct_##A, "pp", a, b);               \
}
SUPER()
#undef GO
static void* find_class_finalize_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_class_finalize_fct_##A == (uintptr_t)fct) return my_class_finalize_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_class_finalize_fct_##A == 0) {my_class_finalize_fct_##A = (uintptr_t)fct; return my_class_finalize_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo class_finalize callback\n");
    return NULL;
}

// instance_init ...
#define GO(A)   \
static uintptr_t my_instance_init_fct_##A = 0;                              \
static size_t parent_instance_init_##A = 0;                                 \
static int my_instance_init_##A(void* a, void* b)                           \
{                                                                           \
    printf_log(LOG_DEBUG, "Custom Instance init %d for class %p (parent=%p:%s)\n", A, a, (void*)parent_instance_init_##A, g_type_name(parent_instance_init_##A));\
    int ret = RunFunctionFmt(my_instance_init_fct_##A, "pp", a, b);         \
    size_t type = parent_instance_init_##A;                                 \
    while(checkRegisteredClass(type))                                       \
        type = g_type_parent(type);                                         \
    unwrapGTKInstance(a, type);                                             \
    bridgeGTKInstance(a, type);                                             \
    return ret;                                                             \
}
SUPER()
#undef GO
static void* find_instance_init_Fct(void* fct, size_t parent)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_instance_init_fct_##A == (uintptr_t)fct && parent_instance_init_##A==parent) return my_instance_init_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_instance_init_fct_##A == 0) {my_instance_init_fct_##A = (uintptr_t)fct; parent_instance_init_##A=parent; return my_instance_init_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo instance_init callback\n");
    return NULL;
}

// And now the get slot / assign... Taking into account that the desired callback may already be a wrapped one (so unwrapping it)
my_GTypeInfo_t* findFreeGTypeInfo(my_GTypeInfo_t* fcts, size_t parent)
{
    if(!fcts) return NULL;
    #define GO(A) if(used_gtypeinfo_##A && memcmp(&ref_gtypeinfo_##A, fcts, sizeof(my_GTypeInfo_t))==0) return &my_gtypeinfo_##A;
    SUPER()
    #undef GO
    #define GO(A) if(used_gtypeinfo_##A == 0) {                                                 \
        used_gtypeinfo_##A = 1;                                                                 \
        memcpy(&ref_gtypeinfo_##A, fcts, sizeof(my_GTypeInfo_t));                               \
        my_gtypeinfo_##A.class_size = fcts->class_size;                                         \
        my_gtypeinfo_##A.base_init = find_base_init_Fct(fcts->base_init);                       \
        my_gtypeinfo_##A.base_finalize = find_base_finalize_Fct(fcts->base_finalize);           \
        my_gtypeinfo_##A.class_init = find_class_init_Fct(fcts->class_init, parent);            \
        my_gtypeinfo_##A.class_finalize = find_class_finalize_Fct(fcts->class_finalize);        \
        my_gtypeinfo_##A.class_data = fcts->class_data;                                         \
        my_gtypeinfo_##A.instance_size = fcts->instance_size;                                   \
        my_gtypeinfo_##A.n_preallocs = fcts->n_preallocs;                                       \
        my_gtypeinfo_##A.instance_init = find_instance_init_Fct(fcts->instance_init, parent);   \
        my_gtypeinfo_##A.value_table = findFreeGTypeValueTable(fcts->value_table);              \
        return &my_gtypeinfo_##A;                                                               \
    }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo callback\n");
    return NULL;
}

// GTypeValueTable infrastructure
static my_GTypeValueTable_t my_gtypevaluetable_static = {0};
static my_GTypeValueTable_t ref_gtypevaluetable_static = {0};
static int used_gtypevaluetable_static = 0;

my_GTypeValueTable_t* findFreeGTypeValueTable(my_GTypeValueTable_t* fcts)
{
    if(!fcts) return NULL;
    if(used_gtypevaluetable_static && memcmp(&ref_gtypevaluetable_static, fcts, sizeof(my_GTypeValueTable_t))==0)
        return &my_gtypevaluetable_static;
    if(used_gtypevaluetable_static == 0) {
        used_gtypevaluetable_static = 1;
        memcpy(&ref_gtypevaluetable_static, fcts, sizeof(my_GTypeValueTable_t));
        memcpy(&my_gtypevaluetable_static, fcts, sizeof(my_GTypeValueTable_t));
        return &my_gtypevaluetable_static;
    }
    printf_log(LOG_NONE, "Warning, no more slot for GTypeValueTable callback\n");
    return fcts; // fallback to original
}

// ---- GtkTypeInfo ----

// First the structure my_GtkTypeInfo_t statics, with paired x64 source pointer
#define GO(A) \
static my_GtkTypeInfo_t     my_gtktypeinfo_##A = {0};   \
static my_GtkTypeInfo_t    ref_gtktypeinfo_##A = {0};  \
static int                used_gtktypeinfo_##A = 0;
SUPER()
#undef GO
// Then the static functions callback that may be used with the structure
#define GO(A)   \
static int fct_gtk_parent_##A = 0 ;                                                 \
static uintptr_t fct_gtk_class_init_##A = 0;                                        \
static int my_gtk_class_init_##A(void* g_class) {                                   \
    printf_log(LOG_DEBUG, "Calling fct_gtk_class_init_" #A " wrapper\n");           \
    int ret = (int)RunFunctionFmt(fct_gtk_class_init_##A, "p", g_class);            \
    unwrapGTKClass(g_class, fct_gtk_parent_##A);                                    \
    return ret;                                                                     \
}                                                                                   \
static uintptr_t fct_gtk_object_init_##A = 0;                                       \
static int my_gtk_object_init_##A(void* object, void* data) {                       \
    printf_log(LOG_DEBUG, "Calling fct_gtk_object_init_" #A " wrapper\n");          \
    return (int)RunFunctionFmt(fct_gtk_object_init_##A, "pp", object, data);        \
}                                                                                   \
static uintptr_t fct_gtk_base_class_init_##A = 0;                                   \
static int my_gtk_base_class_init_##A(void* instance, void* data) {                 \
    printf_log(LOG_DEBUG, "Calling fct_gtk_base_class_init_" #A " wrapper\n");      \
    return (int)RunFunctionFmt(fct_gtk_base_class_init_##A, "pp", instance, data);  \
}

SUPER()
#undef GO
// And now the get slot / assign... Taking into account that the desired callback may already be a wrapped one (so unwrapping it)
my_GtkTypeInfo_t* findFreeGtkTypeInfo(my_GtkTypeInfo_t* fcts, size_t parent)
{
    if(!fcts) return NULL;
    #define GO(A) if(used_gtktypeinfo_##A && memcmp(&ref_gtktypeinfo_##A, fcts, sizeof(my_GtkTypeInfo_t))==0) return &my_gtktypeinfo_##A;
    SUPER()
    #undef GO
    #define GO(A) if(used_gtktypeinfo_##A == 0) {          \
        memcpy(&ref_gtktypeinfo_##A, fcts, sizeof(my_GtkTypeInfo_t));        \
        fct_gtk_parent_##A = parent;                        \
        my_gtktypeinfo_##A.type_name = fcts->type_name; \
        my_gtktypeinfo_##A.object_size = fcts->object_size; \
        my_gtktypeinfo_##A.class_size = fcts->class_size; \
        my_gtktypeinfo_##A.class_init_func = (fcts->class_init_func)?((GetNativeFnc((uintptr_t)fcts->class_init_func))?GetNativeFnc((uintptr_t)fcts->class_init_func):(void*)my_gtk_class_init_##A):NULL;    \
        fct_gtk_class_init_##A = (uintptr_t)fcts->class_init_func;           \
        my_gtktypeinfo_##A.object_init_func = (fcts->object_init_func)?((GetNativeFnc((uintptr_t)fcts->object_init_func))?GetNativeFnc((uintptr_t)fcts->object_init_func):(void*)my_gtk_object_init_##A):NULL;    \
        fct_gtk_object_init_##A = (uintptr_t)fcts->object_init_func;         \
        my_gtktypeinfo_##A.reserved_1 = fcts->reserved_1;                 \
        my_gtktypeinfo_##A.reserved_2 = fcts->reserved_2;                 \
        my_gtktypeinfo_##A.base_class_init_func = (fcts->base_class_init_func)?((GetNativeFnc((uintptr_t)fcts->base_class_init_func))?GetNativeFnc((uintptr_t)fcts->base_class_init_func):(void*)my_gtk_base_class_init_##A):NULL;    \
        fct_gtk_base_class_init_##A = (uintptr_t)fcts->base_class_init_func;   \
        return &my_gtktypeinfo_##A;                       \
    }

    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GtkTypeInfo callback\n");
    return NULL;
}

/*
 * GTK Class Infrastructure Macros
 * Contains all macro definitions needed by split GTK files
 */

#ifndef GTK_MACROS_H
#define GTK_MACROS_H

// Feature test macro for dladdr()
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

// System includes for types used in macros
#include <stddef.h>
#include <dlfcn.h>

#include "../../include/wrappedlibs.h"
#include "gtk_common.h"
#include "../../include/bridge.h"
#include "../../include/debug.h"
#include "../../include/callback.h"
#include "../../include/librarian.h"
#include "../../include/gtkclass.h"
#include "../../include/library.h"

// Include super80.h after core includes
#include "super80.h"

// Global variables shared across all GTK files (defined in gtk_core.c)
extern bridge_t* my_bridge;

// Class wrapping functions called by dispatcher (implemented in specific files)
// Base classes (in gtk_base.c)
void wrapGObjectClass(void* cl);
void unwrapGObjectClass(void* cl);
void bridgeGObjectClass(void* cl);
void wrapGInitiallyUnownedClass(void* cl);
void unwrapGInitiallyUnownedClass(void* cl);
void bridgeGInitiallyUnownedClass(void* cl);
// Base instance functions
void unwrapGObjectInstance(void* cl);
void bridgeGObjectInstance(void* cl);
void unwrapGInitiallyUnownedInstance(void* cl);
void bridgeGInitiallyUnownedInstance(void* cl);

// Application classes (in gtk_application.c)
void wrapGApplicationClass(void* cl);
void unwrapGApplicationClass(void* cl);
void bridgeGApplicationClass(void* cl);
void wrapGtkApplicationClass(void* cl);
void unwrapGtkApplicationClass(void* cl);
void bridgeGtkApplicationClass(void* cl);
void wrapGtkApplicationWindowClass(void* cl);
void unwrapGtkApplicationWindowClass(void* cl);
void bridgeGtkApplicationWindowClass(void* cl);

// Widget classes (in gtk_widgets.c) - All GTK widgets
void wrapGtkDrawingArea3Class(void* cl);
void unwrapGtkDrawingArea3Class(void* cl);
void bridgeGtkDrawingArea3Class(void* cl);
void wrapGtkObjectClass(void* cl);
void unwrapGtkObjectClass(void* cl);
void bridgeGtkObjectClass(void* cl);
void wrapGtkWidget2Class(void* cl);
void unwrapGtkWidget2Class(void* cl);
void bridgeGtkWidget2Class(void* cl);
void wrapGtkWidget3Class(void* cl);
void unwrapGtkWidget3Class(void* cl);
void bridgeGtkWidget3Class(void* cl);
void wrapGtkContainer2Class(void* cl);
void unwrapGtkContainer2Class(void* cl);
void bridgeGtkContainer2Class(void* cl);
void wrapGtkContainer3Class(void* cl);
void unwrapGtkContainer3Class(void* cl);
void bridgeGtkContainer3Class(void* cl);
void wrapGtkActionClass(void* cl);
void unwrapGtkActionClass(void* cl);
void bridgeGtkActionClass(void* cl);
void wrapGtkLabel2Class(void* cl);
void unwrapGtkLabel2Class(void* cl);
void bridgeGtkLabel2Class(void* cl);
void wrapGtkLabel3Class(void* cl);
void unwrapGtkLabel3Class(void* cl);
void bridgeGtkLabel3Class(void* cl);
void wrapGtkMisc2Class(void* cl);
void unwrapGtkMisc2Class(void* cl);
void bridgeGtkMisc2Class(void* cl);
void wrapGtkMisc3Class(void* cl);
void unwrapGtkMisc3Class(void* cl);
void bridgeGtkMisc3Class(void* cl);
void wrapGtkImage3Class(void* cl);
void unwrapGtkImage3Class(void* cl);
void bridgeGtkImage3Class(void* cl);
void wrapGtkTreeView2Class(void* cl);
void unwrapGtkTreeView2Class(void* cl);
void bridgeGtkTreeView2Class(void* cl);
void wrapGtkBin2Class(void* cl);
void unwrapGtkBin2Class(void* cl);
void bridgeGtkBin2Class(void* cl);
void wrapGtkBin3Class(void* cl);
void unwrapGtkBin3Class(void* cl);
void bridgeGtkBin3Class(void* cl);
void wrapGtkWindow2Class(void* cl);
void unwrapGtkWindow2Class(void* cl);
void bridgeGtkWindow2Class(void* cl);
void wrapGtkWindow3Class(void* cl);
void unwrapGtkWindow3Class(void* cl);
void bridgeGtkWindow3Class(void* cl);
void wrapGtkTable2Class(void* cl);
void unwrapGtkTable2Class(void* cl);
void bridgeGtkTable2Class(void* cl);
void wrapGtkFixed2Class(void* cl);
void unwrapGtkFixed2Class(void* cl);
void bridgeGtkFixed2Class(void* cl);
void wrapGtkFixed3Class(void* cl);
void unwrapGtkFixed3Class(void* cl);
void bridgeGtkFixed3Class(void* cl);
void wrapGtkListBoxClass(void* cl);
void unwrapGtkListBoxClass(void* cl);
void bridgeGtkListBoxClass(void* cl);
void wrapGtkListBoxRowClass(void* cl);
void unwrapGtkListBoxRowClass(void* cl);
void bridgeGtkListBoxRowClass(void* cl);
void wrapGtkButton2Class(void* cl);
void unwrapGtkButton2Class(void* cl);
void bridgeGtkButton2Class(void* cl);
void wrapGtkButton3Class(void* cl);
void unwrapGtkButton3Class(void* cl);
void bridgeGtkButton3Class(void* cl);
void wrapGtkComboBox2Class(void* cl);
void unwrapGtkComboBox2Class(void* cl);
void bridgeGtkComboBox2Class(void* cl);
void wrapGtkToggleButton2Class(void* cl);
void unwrapGtkToggleButton2Class(void* cl);
void bridgeGtkToggleButton2Class(void* cl);
void wrapGtkToggleButton3Class(void* cl);
void unwrapGtkToggleButton3Class(void* cl);
void bridgeGtkToggleButton3Class(void* cl);
void wrapGtkMenuButton3Class(void* cl);
void unwrapGtkMenuButton3Class(void* cl);
void bridgeGtkMenuButton3Class(void* cl);
void wrapGtkCheckButton2Class(void* cl);
void unwrapGtkCheckButton2Class(void* cl);
void bridgeGtkCheckButton2Class(void* cl);
void wrapGtkCheckButton3Class(void* cl);
void unwrapGtkCheckButton3Class(void* cl);
void bridgeGtkCheckButton3Class(void* cl);
void wrapGtkEntry2Class(void* cl);
void unwrapGtkEntry2Class(void* cl);
void bridgeGtkEntry2Class(void* cl);
void wrapGtkSpinButton2Class(void* cl);
void unwrapGtkSpinButton2Class(void* cl);
void bridgeGtkSpinButton2Class(void* cl);
void wrapGtkProgress2Class(void* cl);
void unwrapGtkProgress2Class(void* cl);
void bridgeGtkProgress2Class(void* cl);
void wrapGtkProgressBar2Class(void* cl);
void unwrapGtkProgressBar2Class(void* cl);
void bridgeGtkProgressBar2Class(void* cl);
void wrapGtkFrame2Class(void* cl);
void unwrapGtkFrame2Class(void* cl);
void bridgeGtkFrame2Class(void* cl);
void wrapGtkMenuShell2Class(void* cl);
void unwrapGtkMenuShell2Class(void* cl);
void bridgeGtkMenuShell2Class(void* cl);
void wrapGtkMenuBar2Class(void* cl);
void unwrapGtkMenuBar2Class(void* cl);
void bridgeGtkMenuBar2Class(void* cl);
void wrapGtkTextView2Class(void* cl);
void unwrapGtkTextView2Class(void* cl);
void bridgeGtkTextView2Class(void* cl);
void wrapGtkTextView3Class(void* cl);
void unwrapGtkTextView3Class(void* cl);
void bridgeGtkTextView3Class(void* cl);
void wrapGtkGrid3Class(void* cl);
void unwrapGtkGrid3Class(void* cl);
void bridgeGtkGrid3Class(void* cl);
void wrapGtkEventControllerClass(void* cl);
void unwrapGtkEventControllerClass(void* cl);
void bridgeGtkEventControllerClass(void* cl);
void wrapGtkGestureClass(void* cl);
void unwrapGtkGestureClass(void* cl);
void bridgeGtkGestureClass(void* cl);
void wrapGtkGestureSingleClass(void* cl);
void unwrapGtkGestureSingleClass(void* cl);
void bridgeGtkGestureSingleClass(void* cl);
void wrapGtkGestureLongPressClass(void* cl);
void unwrapGtkGestureLongPressClass(void* cl);
void bridgeGtkGestureLongPressClass(void* cl);
void wrapGtkNotebook2Class(void* cl);
void unwrapGtkNotebook2Class(void* cl);
void bridgeGtkNotebook2Class(void* cl);
void wrapGtkCellRenderer2Class(void* cl);
void unwrapGtkCellRenderer2Class(void* cl);
void bridgeGtkCellRenderer2Class(void* cl);
void wrapGtkCellRendererText2Class(void* cl);
void unwrapGtkCellRendererText2Class(void* cl);
void bridgeGtkCellRendererText2Class(void* cl);

// GStreamer classes (in gtk_multimedia.c)
void wrapGstObjectClass(void* cl);
void unwrapGstObjectClass(void* cl);
void bridgeGstObjectClass(void* cl);
void wrapGstElementClass(void* cl);
void unwrapGstElementClass(void* cl);
void bridgeGstElementClass(void* cl);
void wrapGstBinClass(void* cl);
void unwrapGstBinClass(void* cl);
void bridgeGstBinClass(void* cl);
void wrapGstPadClass(void* cl);
void unwrapGstPadClass(void* cl);
void bridgeGstPadClass(void* cl);
void wrapGstBaseSrcClass(void* cl);
void unwrapGstBaseSrcClass(void* cl);
void bridgeGstBaseSrcClass(void* cl);
void wrapGstBaseSinkClass(void* cl);
void unwrapGstBaseSinkClass(void* cl);
void bridgeGstBaseSinkClass(void* cl);
void wrapGstBaseTransformClass(void* cl);
void unwrapGstBaseTransformClass(void* cl);
void bridgeGstBaseTransformClass(void* cl);
void wrapGstPushSrcClass(void* cl);
void unwrapGstPushSrcClass(void* cl);
void bridgeGstPushSrcClass(void* cl);
void wrapGstBufferPoolClass(void* cl);
void unwrapGstBufferPoolClass(void* cl);
void bridgeGstBufferPoolClass(void* cl);
void wrapGstAllocatorClass(void* cl);
void unwrapGstAllocatorClass(void* cl);
void bridgeGstAllocatorClass(void* cl);
void wrapGstTaskPoolClass(void* cl);
void unwrapGstTaskPoolClass(void* cl);
void bridgeGstTaskPoolClass(void* cl);
void wrapGstAggregatorClass(void* cl);
void unwrapGstAggregatorClass(void* cl);
void bridgeGstAggregatorClass(void* cl);
void wrapGstAggregatorPadClass(void* cl);
void unwrapGstAggregatorPadClass(void* cl);
void bridgeGstAggregatorPadClass(void* cl);
void wrapGstVideoAggregatorClass(void* cl);
void unwrapGstVideoAggregatorClass(void* cl);
void bridgeGstVideoAggregatorClass(void* cl);
void wrapGstVideoAggregatorPadClass(void* cl);
void unwrapGstVideoAggregatorPadClass(void* cl);
void bridgeGstVideoAggregatorPadClass(void* cl);
void wrapGstAudioDecoderClass(void* cl);
void unwrapGstAudioDecoderClass(void* cl);
void bridgeGstAudioDecoderClass(void* cl);
void wrapGstAudioEncoderClass(void* cl);
void unwrapGstAudioEncoderClass(void* cl);
void bridgeGstAudioEncoderClass(void* cl);
void wrapGstAudioFilterClass(void* cl);
void unwrapGstAudioFilterClass(void* cl);
void bridgeGstAudioFilterClass(void* cl);
void wrapGstVideoDecoderClass(void* cl);
void unwrapGstVideoDecoderClass(void* cl);
void bridgeGstVideoDecoderClass(void* cl);
void wrapGstVideoEncoderClass(void* cl);
void unwrapGstVideoEncoderClass(void* cl);
void bridgeGstVideoEncoderClass(void* cl);
void wrapGstVideoFilterClass(void* cl);
void unwrapGstVideoFilterClass(void* cl);
void bridgeGstVideoFilterClass(void* cl);
void wrapGstVideoSinkClass(void* cl);
void unwrapGstVideoSinkClass(void* cl);
void bridgeGstVideoSinkClass(void* cl);
void wrapGstVideoBufferPoolClass(void* cl);
void unwrapGstVideoBufferPoolClass(void* cl);
void bridgeGstVideoBufferPoolClass(void* cl);
void wrapGstGLBaseFilterClass(void* cl);
void unwrapGstGLBaseFilterClass(void* cl);
void bridgeGstGLBaseFilterClass(void* cl);
void wrapGstGLBaseSrcClass(void* cl);
void unwrapGstGLBaseSrcClass(void* cl);
void bridgeGstGLBaseSrcClass(void* cl);
void wrapGstGLFilterClass(void* cl);
void unwrapGstGLFilterClass(void* cl);
void bridgeGstGLFilterClass(void* cl);

// Accessibility classes (in gtk_accessibility.c)
void wrapAtkObjectClass(void* cl);
void unwrapAtkObjectClass(void* cl);
void bridgeAtkObjectClass(void* cl);
void wrapAtkUtilClass(void* cl);
void unwrapAtkUtilClass(void* cl);
void bridgeAtkUtilClass(void* cl);

// D-Bus classes
void wrapGDBusInterfaceSkeletonClass(void* cl);
void unwrapGDBusInterfaceSkeletonClass(void* cl);
void bridgeGDBusInterfaceSkeletonClass(void* cl);
void wrapGDBusObjectManagerClientClass(void* cl);
void unwrapGDBusObjectManagerClientClass(void* cl);
void bridgeGDBusObjectManagerClientClass(void* cl);
void wrapGDBusProxyClass(void* cl);
void unwrapGDBusProxyClass(void* cl);
void bridgeGDBusProxyClass(void* cl);

// Meta/Custom classes
void wrapMetaFrames2Class(void* cl);
void unwrapMetaFrames2Class(void* cl);
void bridgeMetaFrames2Class(void* cl);

// Main macro infrastructure for GTK class wrapping

#define WRAPPED_RET(A, NAME, RET, FRET, DEF, FMT, ...)  \
static uintptr_t my_##NAME##_fct_##A = 0;   \
static RET my_##NAME##_##A DEF              \
{                                           \
    printf_log(LOG_DEBUG, "Calling " #NAME "_" #A " wrapper\n");                \
    return FRET((RET)RunFunctionFmt(my_##NAME##_fct_##A, FMT, __VA_ARGS__));    \
}

#define WRAPPED(A, NAME, RET, DEF, FMT, ...)  \
static uintptr_t my_##NAME##_fct_##A = 0;   \
static RET my_##NAME##_##A DEF              \
{                                           \
    printf_log(LOG_DEBUG, "Calling " #NAME "_" #A " wrapper\n");             \
    return (RET)RunFunctionFmt(my_##NAME##_fct_##A, FMT, __VA_ARGS__);\
}

#define FIND(A, NAME) \
static void* find_##NAME##_##A(wrapper_t W, void* fct)                            \
{                                                                                 \
    if(!fct) return fct;                                                          \
    void* tmp = GetNativeFnc((uintptr_t)fct);                                     \
    if(tmp) {AddAutomaticBridge(my_bridge, W, fct, 0, #NAME "_" #A); return tmp;} \
    if(my_##NAME##_##A##_fct_0 == (uintptr_t)fct) return my_##NAME##_##A##_0;     \
    if(my_##NAME##_##A##_fct_1 == (uintptr_t)fct) return my_##NAME##_##A##_1;     \
    if(my_##NAME##_##A##_fct_2 == (uintptr_t)fct) return my_##NAME##_##A##_2;     \
    if(my_##NAME##_##A##_fct_3 == (uintptr_t)fct) return my_##NAME##_##A##_3;     \
    if(my_##NAME##_##A##_fct_4 == (uintptr_t)fct) return my_##NAME##_##A##_4;     \
    if(my_##NAME##_##A##_fct_5 == (uintptr_t)fct) return my_##NAME##_##A##_5;     \
    if(my_##NAME##_##A##_fct_6 == (uintptr_t)fct) return my_##NAME##_##A##_6;     \
    if(my_##NAME##_##A##_fct_7 == (uintptr_t)fct) return my_##NAME##_##A##_7;     \
    if(my_##NAME##_##A##_fct_8 == (uintptr_t)fct) return my_##NAME##_##A##_8;     \
    if(my_##NAME##_##A##_fct_9 == (uintptr_t)fct) return my_##NAME##_##A##_9;     \
    if(my_##NAME##_##A##_fct_10 == (uintptr_t)fct) return my_##NAME##_##A##_10;   \
    if(my_##NAME##_##A##_fct_11 == (uintptr_t)fct) return my_##NAME##_##A##_11;   \
    if(my_##NAME##_##A##_fct_12 == (uintptr_t)fct) return my_##NAME##_##A##_12;   \
    if(my_##NAME##_##A##_fct_13 == (uintptr_t)fct) return my_##NAME##_##A##_13;   \
    if(my_##NAME##_##A##_fct_14 == (uintptr_t)fct) return my_##NAME##_##A##_14;   \
    if(my_##NAME##_##A##_fct_15 == (uintptr_t)fct) return my_##NAME##_##A##_15;   \
    if(my_##NAME##_##A##_fct_16 == (uintptr_t)fct) return my_##NAME##_##A##_16;   \
    if(my_##NAME##_##A##_fct_17 == (uintptr_t)fct) return my_##NAME##_##A##_17;   \
    if(my_##NAME##_##A##_fct_18 == (uintptr_t)fct) return my_##NAME##_##A##_18;   \
    if(my_##NAME##_##A##_fct_19 == (uintptr_t)fct) return my_##NAME##_##A##_19;   \
    if(my_##NAME##_##A##_fct_20 == (uintptr_t)fct) return my_##NAME##_##A##_20;   \
    if(my_##NAME##_##A##_fct_21 == (uintptr_t)fct) return my_##NAME##_##A##_21;   \
    if(my_##NAME##_##A##_fct_22 == (uintptr_t)fct) return my_##NAME##_##A##_22;   \
    if(my_##NAME##_##A##_fct_23 == (uintptr_t)fct) return my_##NAME##_##A##_23;   \
    if(my_##NAME##_##A##_fct_24 == (uintptr_t)fct) return my_##NAME##_##A##_24;   \
    if(my_##NAME##_##A##_fct_25 == (uintptr_t)fct) return my_##NAME##_##A##_25;   \
    if(my_##NAME##_##A##_fct_26 == (uintptr_t)fct) return my_##NAME##_##A##_26;   \
    if(my_##NAME##_##A##_fct_27 == (uintptr_t)fct) return my_##NAME##_##A##_27;   \
    if(my_##NAME##_##A##_fct_28 == (uintptr_t)fct) return my_##NAME##_##A##_28;   \
    if(my_##NAME##_##A##_fct_29 == (uintptr_t)fct) return my_##NAME##_##A##_29;   \
    if(my_##NAME##_##A##_fct_30 == (uintptr_t)fct) return my_##NAME##_##A##_30;   \
    if(my_##NAME##_##A##_fct_31 == (uintptr_t)fct) return my_##NAME##_##A##_31;   \
    if(my_##NAME##_##A##_fct_32 == (uintptr_t)fct) return my_##NAME##_##A##_32;   \
    if(my_##NAME##_##A##_fct_33 == (uintptr_t)fct) return my_##NAME##_##A##_33;   \
    if(my_##NAME##_##A##_fct_34 == (uintptr_t)fct) return my_##NAME##_##A##_34;   \
    if(my_##NAME##_##A##_fct_35 == (uintptr_t)fct) return my_##NAME##_##A##_35;   \
    if(my_##NAME##_##A##_fct_36 == (uintptr_t)fct) return my_##NAME##_##A##_36;   \
    if(my_##NAME##_##A##_fct_37 == (uintptr_t)fct) return my_##NAME##_##A##_37;   \
    if(my_##NAME##_##A##_fct_38 == (uintptr_t)fct) return my_##NAME##_##A##_38;   \
    if(my_##NAME##_##A##_fct_39 == (uintptr_t)fct) return my_##NAME##_##A##_39;   \
    if(my_##NAME##_##A##_fct_0 == 0) {my_##NAME##_##A##_fct_0 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_0, fct); return my_##NAME##_##A##_0; } \
    if(my_##NAME##_##A##_fct_1 == 0) {my_##NAME##_##A##_fct_1 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_1, fct); return my_##NAME##_##A##_1; } \
    if(my_##NAME##_##A##_fct_2 == 0) {my_##NAME##_##A##_fct_2 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_2, fct); return my_##NAME##_##A##_2; } \
    if(my_##NAME##_##A##_fct_3 == 0) {my_##NAME##_##A##_fct_3 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_3, fct); return my_##NAME##_##A##_3; } \
    if(my_##NAME##_##A##_fct_4 == 0) {my_##NAME##_##A##_fct_4 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_4, fct); return my_##NAME##_##A##_4; } \
    if(my_##NAME##_##A##_fct_5 == 0) {my_##NAME##_##A##_fct_5 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_5, fct); return my_##NAME##_##A##_5; } \
    if(my_##NAME##_##A##_fct_6 == 0) {my_##NAME##_##A##_fct_6 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_6, fct); return my_##NAME##_##A##_6; } \
    if(my_##NAME##_##A##_fct_7 == 0) {my_##NAME##_##A##_fct_7 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_7, fct); return my_##NAME##_##A##_7; } \
    if(my_##NAME##_##A##_fct_8 == 0) {my_##NAME##_##A##_fct_8 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_8, fct); return my_##NAME##_##A##_8; } \
    if(my_##NAME##_##A##_fct_9 == 0) {my_##NAME##_##A##_fct_9 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_9, fct); return my_##NAME##_##A##_9; } \
    if(my_##NAME##_##A##_fct_10 == 0) {my_##NAME##_##A##_fct_10 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_10, fct); return my_##NAME##_##A##_10; } \
    if(my_##NAME##_##A##_fct_11 == 0) {my_##NAME##_##A##_fct_11 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_11, fct); return my_##NAME##_##A##_11; } \
    if(my_##NAME##_##A##_fct_12 == 0) {my_##NAME##_##A##_fct_12 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_12, fct); return my_##NAME##_##A##_12; } \
    if(my_##NAME##_##A##_fct_13 == 0) {my_##NAME##_##A##_fct_13 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_13, fct); return my_##NAME##_##A##_13; } \
    if(my_##NAME##_##A##_fct_14 == 0) {my_##NAME##_##A##_fct_14 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_14, fct); return my_##NAME##_##A##_14; } \
    if(my_##NAME##_##A##_fct_15 == 0) {my_##NAME##_##A##_fct_15 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_15, fct); return my_##NAME##_##A##_15; } \
    if(my_##NAME##_##A##_fct_16 == 0) {my_##NAME##_##A##_fct_16 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_16, fct); return my_##NAME##_##A##_16; } \
    if(my_##NAME##_##A##_fct_17 == 0) {my_##NAME##_##A##_fct_17 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_17, fct); return my_##NAME##_##A##_17; } \
    if(my_##NAME##_##A##_fct_18 == 0) {my_##NAME##_##A##_fct_18 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_18, fct); return my_##NAME##_##A##_18; } \
    if(my_##NAME##_##A##_fct_19 == 0) {my_##NAME##_##A##_fct_19 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_19, fct); return my_##NAME##_##A##_19; } \
    if(my_##NAME##_##A##_fct_20 == 0) {my_##NAME##_##A##_fct_20 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_20, fct); return my_##NAME##_##A##_20; } \
    if(my_##NAME##_##A##_fct_21 == 0) {my_##NAME##_##A##_fct_21 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_21, fct); return my_##NAME##_##A##_21; } \
    if(my_##NAME##_##A##_fct_22 == 0) {my_##NAME##_##A##_fct_22 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_22, fct); return my_##NAME##_##A##_22; } \
    if(my_##NAME##_##A##_fct_23 == 0) {my_##NAME##_##A##_fct_23 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_23, fct); return my_##NAME##_##A##_23; } \
    if(my_##NAME##_##A##_fct_24 == 0) {my_##NAME##_##A##_fct_24 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_24, fct); return my_##NAME##_##A##_24; } \
    if(my_##NAME##_##A##_fct_25 == 0) {my_##NAME##_##A##_fct_25 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_25, fct); return my_##NAME##_##A##_25; } \
    if(my_##NAME##_##A##_fct_26 == 0) {my_##NAME##_##A##_fct_26 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_26, fct); return my_##NAME##_##A##_26; } \
    if(my_##NAME##_##A##_fct_27 == 0) {my_##NAME##_##A##_fct_27 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_27, fct); return my_##NAME##_##A##_27; } \
    if(my_##NAME##_##A##_fct_28 == 0) {my_##NAME##_##A##_fct_28 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_28, fct); return my_##NAME##_##A##_28; } \
    if(my_##NAME##_##A##_fct_29 == 0) {my_##NAME##_##A##_fct_29 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_29, fct); return my_##NAME##_##A##_29; } \
    if(my_##NAME##_##A##_fct_30 == 0) {my_##NAME##_##A##_fct_30 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_30, fct); return my_##NAME##_##A##_30; } \
    if(my_##NAME##_##A##_fct_31 == 0) {my_##NAME##_##A##_fct_31 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_31, fct); return my_##NAME##_##A##_31; } \
    if(my_##NAME##_##A##_fct_32 == 0) {my_##NAME##_##A##_fct_32 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_32, fct); return my_##NAME##_##A##_32; } \
    if(my_##NAME##_##A##_fct_33 == 0) {my_##NAME##_##A##_fct_33 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_33, fct); return my_##NAME##_##A##_33; } \
    if(my_##NAME##_##A##_fct_34 == 0) {my_##NAME##_##A##_fct_34 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_34, fct); return my_##NAME##_##A##_34; } \
    if(my_##NAME##_##A##_fct_35 == 0) {my_##NAME##_##A##_fct_35 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_35, fct); return my_##NAME##_##A##_35; } \
    if(my_##NAME##_##A##_fct_36 == 0) {my_##NAME##_##A##_fct_36 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_36, fct); return my_##NAME##_##A##_36; } \
    if(my_##NAME##_##A##_fct_37 == 0) {my_##NAME##_##A##_fct_37 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_37, fct); return my_##NAME##_##A##_37; } \
    if(my_##NAME##_##A##_fct_38 == 0) {my_##NAME##_##A##_fct_38 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_38, fct); return my_##NAME##_##A##_38; } \
    if(my_##NAME##_##A##_fct_39 == 0) {my_##NAME##_##A##_fct_39 = (uintptr_t)fct; addCheckAlternate((void*)my_##NAME##_##A##_fct_39, fct); return my_##NAME##_##A##_39; } \
    printf_log(LOG_NONE, "Warning, no more slot for " #A " " #NAME " gtkclass callback\n");    \
    return NULL;    \
}

#define REVERSE(A, NAME)   \
static void* reverse_##NAME##_##A(wrapper_t W, void* fct)                       \
{                                                                               \
    if(!fct) return fct;                                                        \
    if((void*)my_##NAME##_##A##_0 == fct) return (void*)my_##NAME##_##A##_fct_0;\
    if((void*)my_##NAME##_##A##_1 == fct) return (void*)my_##NAME##_##A##_fct_1;\
    if((void*)my_##NAME##_##A##_2 == fct) return (void*)my_##NAME##_##A##_fct_2;\
    if((void*)my_##NAME##_##A##_3 == fct) return (void*)my_##NAME##_##A##_fct_3;\
    if((void*)my_##NAME##_##A##_4 == fct) return (void*)my_##NAME##_##A##_fct_4;\
    if((void*)my_##NAME##_##A##_5 == fct) return (void*)my_##NAME##_##A##_fct_5;\
    if((void*)my_##NAME##_##A##_6 == fct) return (void*)my_##NAME##_##A##_fct_6;\
    if((void*)my_##NAME##_##A##_7 == fct) return (void*)my_##NAME##_##A##_fct_7;\
    if((void*)my_##NAME##_##A##_8 == fct) return (void*)my_##NAME##_##A##_fct_8;\
    if((void*)my_##NAME##_##A##_9 == fct) return (void*)my_##NAME##_##A##_fct_9;\
    if((void*)my_##NAME##_##A##_10 == fct) return (void*)my_##NAME##_##A##_fct_10;\
    if((void*)my_##NAME##_##A##_11 == fct) return (void*)my_##NAME##_##A##_fct_11;\
    if((void*)my_##NAME##_##A##_12 == fct) return (void*)my_##NAME##_##A##_fct_12;\
    if((void*)my_##NAME##_##A##_13 == fct) return (void*)my_##NAME##_##A##_fct_13;\
    if((void*)my_##NAME##_##A##_14 == fct) return (void*)my_##NAME##_##A##_fct_14;\
    if((void*)my_##NAME##_##A##_15 == fct) return (void*)my_##NAME##_##A##_fct_15;\
    if((void*)my_##NAME##_##A##_16 == fct) return (void*)my_##NAME##_##A##_fct_16;\
    if((void*)my_##NAME##_##A##_17 == fct) return (void*)my_##NAME##_##A##_fct_17;\
    if((void*)my_##NAME##_##A##_18 == fct) return (void*)my_##NAME##_##A##_fct_18;\
    if((void*)my_##NAME##_##A##_19 == fct) return (void*)my_##NAME##_##A##_fct_19;\
    if((void*)my_##NAME##_##A##_20 == fct) return (void*)my_##NAME##_##A##_fct_20;\
    if((void*)my_##NAME##_##A##_21 == fct) return (void*)my_##NAME##_##A##_fct_21;\
    if((void*)my_##NAME##_##A##_22 == fct) return (void*)my_##NAME##_##A##_fct_22;\
    if((void*)my_##NAME##_##A##_23 == fct) return (void*)my_##NAME##_##A##_fct_23;\
    if((void*)my_##NAME##_##A##_24 == fct) return (void*)my_##NAME##_##A##_fct_24;\
    if((void*)my_##NAME##_##A##_25 == fct) return (void*)my_##NAME##_##A##_fct_25;\
    if((void*)my_##NAME##_##A##_26 == fct) return (void*)my_##NAME##_##A##_fct_26;\
    if((void*)my_##NAME##_##A##_27 == fct) return (void*)my_##NAME##_##A##_fct_27;\
    if((void*)my_##NAME##_##A##_28 == fct) return (void*)my_##NAME##_##A##_fct_28;\
    if((void*)my_##NAME##_##A##_29 == fct) return (void*)my_##NAME##_##A##_fct_29;\
    if((void*)my_##NAME##_##A##_30 == fct) return (void*)my_##NAME##_##A##_fct_30;\
    if((void*)my_##NAME##_##A##_31 == fct) return (void*)my_##NAME##_##A##_fct_31;\
    if((void*)my_##NAME##_##A##_32 == fct) return (void*)my_##NAME##_##A##_fct_32;\
    if((void*)my_##NAME##_##A##_33 == fct) return (void*)my_##NAME##_##A##_fct_33;\
    if((void*)my_##NAME##_##A##_34 == fct) return (void*)my_##NAME##_##A##_fct_34;\
    if((void*)my_##NAME##_##A##_35 == fct) return (void*)my_##NAME##_##A##_fct_35;\
    if((void*)my_##NAME##_##A##_36 == fct) return (void*)my_##NAME##_##A##_fct_36;\
    if((void*)my_##NAME##_##A##_37 == fct) return (void*)my_##NAME##_##A##_fct_37;\
    if((void*)my_##NAME##_##A##_38 == fct) return (void*)my_##NAME##_##A##_fct_38;\
    if((void*)my_##NAME##_##A##_39 == fct) return (void*)my_##NAME##_##A##_fct_39;\
    Dl_info info;                                                               \
    if(dladdr(fct, &info))                                                      \
        return (void*)AddCheckBridge(my_bridge, W, fct, 0, NULL);               \
    return fct;                                                                 \
}

#define AUTOBRIDGE(A, NAME)   \
static void autobridge_##NAME##_##A(wrapper_t W, void* fct)         \
{                                                                   \
    if(!fct)                                                        \
        return;                                                     \
    Dl_info info;                                                   \
    if(dladdr(fct, &info))                                          \
        AddAutomaticBridge(my_bridge, W, fct, 0, #NAME "_" #A); \
}

#define WRAPPER(A, NAME, RET, DEF, FMT, ...)        \
WRAPPED( 0, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED( 1, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED( 2, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED( 3, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED( 4, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED( 5, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED( 6, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED( 7, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED( 8, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED( 9, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(10, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(11, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(12, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(13, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(14, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(15, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(16, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(17, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(18, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(19, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(20, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(21, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(22, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(23, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(24, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(25, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(26, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(27, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(28, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(29, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(30, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(31, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(32, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(33, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(34, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(35, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(36, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(37, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(38, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
WRAPPED(39, NAME##_##A, RET, DEF, FMT, __VA_ARGS__) \
FIND(A, NAME)                                       \
REVERSE(A, NAME)                                    \
AUTOBRIDGE(A, NAME)

#define WRAPPER_RET(A, NAME, RET, FRET, DEF, FMT, ...)        \
WRAPPED_RET( 0, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET( 1, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET( 2, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET( 3, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET( 4, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET( 5, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET( 6, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET( 7, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET( 8, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET( 9, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(10, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(11, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(12, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(13, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(14, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(15, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(16, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(17, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(18, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(19, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(20, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(21, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(22, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(23, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(24, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(25, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(26, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(27, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(28, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(29, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(30, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(31, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(32, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(33, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(34, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(35, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(36, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(37, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(38, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
WRAPPED_RET(39, NAME##_##A, RET, FRET, DEF, FMT, __VA_ARGS__) \
FIND(A, NAME)                                       \
REVERSE(A, NAME)                                    \
AUTOBRIDGE(A, NAME)

// Instance function declarations (defined in split files)
// Base classes
void unwrapGObjectInstance(void* cl);
void bridgeGObjectInstance(void* cl);
void unwrapGInitiallyUnownedInstance(void* cl);
void bridgeGInitiallyUnownedInstance(void* cl);

// Application classes
void unwrapGApplicationInstance(my_GApplication_t* class);
void bridgeGApplicationInstance(my_GApplication_t* class);
void unwrapGtkApplicationInstance(my_GtkApplication_t* class);
void bridgeGtkApplicationInstance(my_GtkApplication_t* class);

// Widget classes
void unwrapGtkObjectInstance(my_GtkObject_t* class);
void bridgeGtkObjectInstance(my_GtkObject_t* class);
void unwrapGtkWidget2Instance(my_GtkWidget2_t* class);
void bridgeGtkWidget2Instance(my_GtkWidget2_t* class);
void unwrapGtkWidget3Instance(my_GtkWidget3_t* class);
void bridgeGtkWidget3Instance(my_GtkWidget3_t* class);
void unwrapGtkContainer2Instance(my_GtkContainer2_t* class);
void bridgeGtkContainer2Instance(my_GtkContainer2_t* class);
void unwrapGtkContainer3Instance(my_GtkContainer3_t* class);
void bridgeGtkContainer3Instance(my_GtkContainer3_t* class);
void unwrapGtkActionInstance(my_GtkAction_t* class);
void bridgeGtkActionInstance(my_GtkAction_t* class);
void unwrapGtkDrawingArea3Instance(my_GtkDrawingArea3_t* class);
void bridgeGtkDrawingArea3Instance(my_GtkDrawingArea3_t* class);
void unwrapGtkMisc2Instance(my_GtkMisc2_t* class);
void bridgeGtkMisc2Instance(my_GtkMisc2_t* class);
void unwrapGtkMisc3Instance(my_GtkMisc3_t* class);
void bridgeGtkMisc3Instance(my_GtkMisc3_t* class);
void unwrapGtkImage3Instance(my_GtkImage3_t* class);
void bridgeGtkImage3Instance(my_GtkImage3_t* class);
void unwrapGtkLabel2Instance(my_GtkLabel2_t* class);
void bridgeGtkLabel2Instance(my_GtkLabel2_t* class);
void unwrapGtkLabel3Instance(my_GtkLabel3_t* class);
void bridgeGtkLabel3Instance(my_GtkLabel3_t* class);
void unwrapGtkTreeView2Instance(my_GtkTreeView2_t* class);
void bridgeGtkTreeView2Instance(my_GtkTreeView2_t* class);
void unwrapGtkBin2Instance(my_GtkBin2_t* class);
void bridgeGtkBin2Instance(my_GtkBin2_t* class);
void unwrapGtkBin3Instance(my_GtkBin3_t* class);
void bridgeGtkBin3Instance(my_GtkBin3_t* class);
void unwrapGtkWindow2Instance(my_GtkWindow2_t* class);
void bridgeGtkWindow2Instance(my_GtkWindow2_t* class);
void unwrapGtkWindow3Instance(my_GtkWindow3_t* class);
void bridgeGtkWindow3Instance(my_GtkWindow3_t* class);
void unwrapGtkTable2Instance(my_GtkTable2_t* class);
void bridgeGtkTable2Instance(my_GtkTable2_t* class);
void unwrapGtkFixed2Instance(my_GtkFixed2_t* class);
void bridgeGtkFixed2Instance(my_GtkFixed2_t* class);
void unwrapGtkFixed3Instance(my_GtkFixed3_t* class);
void bridgeGtkFixed3Instance(my_GtkFixed3_t* class);
void unwrapGtkApplicationWindowInstance(my_GtkApplicationWindow_t* class);
void bridgeGtkApplicationWindowInstance(my_GtkApplicationWindow_t* class);
void unwrapGtkListBoxInstance(my_GtkListBox_t* class);
void bridgeGtkListBoxInstance(my_GtkListBox_t* class);
void unwrapGtkListBoxRowInstance(my_GtkListBoxRow_t* class);
void bridgeGtkListBoxRowInstance(my_GtkListBoxRow_t* class);
void unwrapGtkButton2Instance(my_GtkButton2_t* class);
void bridgeGtkButton2Instance(my_GtkButton2_t* class);
void unwrapGtkButton3Instance(my_GtkButton3_t* class);
void bridgeGtkButton3Instance(my_GtkButton3_t* class);
void unwrapGtkComboBox2Instance(my_GtkComboBox2_t* class);
void bridgeGtkComboBox2Instance(my_GtkComboBox2_t* class);
void unwrapGtkToggleButton2Instance(my_GtkToggleButton2_t* class);
void bridgeGtkToggleButton2Instance(my_GtkToggleButton2_t* class);
void unwrapGtkToggleButton3Instance(my_GtkToggleButton3_t* class);
void bridgeGtkToggleButton3Instance(my_GtkToggleButton3_t* class);
void unwrapGtkMenuButton3Instance(my_GtkMenuButton3_t* class);
void bridgeGtkMenuButton3Instance(my_GtkMenuButton3_t* class);
void unwrapGtkCheckButton2Instance(my_GtkCheckButton2_t* class);
void bridgeGtkCheckButton2Instance(my_GtkCheckButton2_t* class);
void unwrapGtkCheckButton3Instance(my_GtkCheckButton3_t* class);
void bridgeGtkCheckButton3Instance(my_GtkCheckButton3_t* class);
void unwrapGtkEntry2Instance(my_GtkEntry2_t* class);
void bridgeGtkEntry2Instance(my_GtkEntry2_t* class);
void unwrapGtkSpinButton2Instance(my_GtkSpinButton2_t* class);
void bridgeGtkSpinButton2Instance(my_GtkSpinButton2_t* class);
void unwrapGtkProgress2Instance(my_GtkProgress2_t* class);
void bridgeGtkProgress2Instance(my_GtkProgress2_t* class);
void unwrapGtkProgressBar2Instance(my_GtkProgressBar2_t* class);
void bridgeGtkProgressBar2Instance(my_GtkProgressBar2_t* class);
void unwrapGtkFrame2Instance(my_GtkFrame2_t* class);
void bridgeGtkFrame2Instance(my_GtkFrame2_t* class);
void unwrapGtkMenuShell2Instance(my_GtkMenuShell2_t* class);
void bridgeGtkMenuShell2Instance(my_GtkMenuShell2_t* class);
void unwrapGtkMenuBar2Instance(my_GtkMenuBar2_t* class);
void bridgeGtkMenuBar2Instance(my_GtkMenuBar2_t* class);
void unwrapGtkTextView2Instance(my_GtkTextView2_t* class);
void bridgeGtkTextView2Instance(my_GtkTextView2_t* class);
void unwrapGtkTextView3Instance(my_GtkTextView3_t* class);
void bridgeGtkTextView3Instance(my_GtkTextView3_t* class);
void unwrapGtkGrid3Instance(my_GtkGrid3_t* class);
void bridgeGtkGrid3Instance(my_GtkGrid3_t* class);
void unwrapGtkEventControllerInstance(my_GtkEventController_t* class);
void bridgeGtkEventControllerInstance(my_GtkEventController_t* class);
void unwrapGtkGestureInstance(my_GtkGesture_t* class);
void bridgeGtkGestureInstance(my_GtkGesture_t* class);
void unwrapGtkGestureSingleInstance(my_GtkGestureSingle_t* class);
void bridgeGtkGestureSingleInstance(my_GtkGestureSingle_t* class);
void unwrapGtkGestureLongPressInstance(my_GtkGestureLongPress_t* class);
void bridgeGtkGestureLongPressInstance(my_GtkGestureLongPress_t* class);
void unwrapGtkNotebook2Instance(my_GtkNotebook2_t* class);
void bridgeGtkNotebook2Instance(my_GtkNotebook2_t* class);
void unwrapGtkCellRenderer2Instance(my_GtkCellRenderer2_t* class);
void bridgeGtkCellRenderer2Instance(my_GtkCellRenderer2_t* class);
void unwrapGtkCellRendererText2Instance(my_GtkCellRendererText2_t* class);
void bridgeGtkCellRendererText2Instance(my_GtkCellRendererText2_t* class);
void unwrapMetaFrames2Instance(my_MetaFrames2_t* class);
void bridgeMetaFrames2Instance(my_MetaFrames2_t* class);
void unwrapGDBusObjectManagerClientInstance(my_GDBusObjectManagerClient_t* class);
void bridgeGDBusObjectManagerClientInstance(my_GDBusObjectManagerClient_t* class);
void unwrapGDBusInterfaceSkeletonInstance(my_GDBusInterfaceSkeleton_t* class);
void bridgeGDBusInterfaceSkeletonInstance(my_GDBusInterfaceSkeleton_t* class);

// Accessibility classes
void unwrapAtkObjectInstance(my_AtkObject_t* class);
void bridgeAtkObjectInstance(my_AtkObject_t* class);
void unwrapAtkUtilInstance(my_AtkUtil_t* class);
void bridgeAtkUtilInstance(my_AtkUtil_t* class);

// GStreamer multimedia classes
void unwrapGstObjectInstance(my_GstObject_t* class);
void bridgeGstObjectInstance(my_GstObject_t* class);
void unwrapGstAllocatorInstance(my_GstAllocator_t* class);
void bridgeGstAllocatorInstance(my_GstAllocator_t* class);
void unwrapGstTaskPoolInstance(my_GstTaskPool_t* class);
void bridgeGstTaskPoolInstance(my_GstTaskPool_t* class);
void unwrapGDBusProxyInstance(my_GDBusProxy_t* class);
void bridgeGDBusProxyInstance(my_GDBusProxy_t* class);
void unwrapGstElementInstance(my_GstElement_t* class);
void bridgeGstElementInstance(my_GstElement_t* class);
void unwrapGstBinInstance(my_GstBin_t* class);
void bridgeGstBinInstance(my_GstBin_t* class);
void unwrapGstBaseTransformInstance(my_GstBaseTransform_t* class);
void bridgeGstBaseTransformInstance(my_GstBaseTransform_t* class);
void unwrapGstVideoDecoderInstance(my_GstVideoDecoder_t* class);
void bridgeGstVideoDecoderInstance(my_GstVideoDecoder_t* class);
void unwrapGstVideoEncoderInstance(my_GstVideoEncoder_t* class);
void bridgeGstVideoEncoderInstance(my_GstVideoEncoder_t* class);
void unwrapGstBaseSinkInstance(my_GstBaseSink_t* class);
void bridgeGstBaseSinkInstance(my_GstBaseSink_t* class);
void unwrapGstVideoSinkInstance(my_GstVideoSink_t* class);
void bridgeGstVideoSinkInstance(my_GstVideoSink_t* class);
void unwrapGstGLBaseFilterInstance(my_GstGLBaseFilter_t* class);
void bridgeGstGLBaseFilterInstance(my_GstGLBaseFilter_t* class);
void unwrapGstGLFilterInstance(my_GstGLFilter_t* class);
void bridgeGstGLFilterInstance(my_GstGLFilter_t* class);
void unwrapGstAggregatorInstance(my_GstAggregator_t* class);
void bridgeGstAggregatorInstance(my_GstAggregator_t* class);
void unwrapGstVideoAggregatorInstance(my_GstVideoAggregator_t* class);
void bridgeGstVideoAggregatorInstance(my_GstVideoAggregator_t* class);
void unwrapGstPadInstance(my_GstPad_t* class);
void bridgeGstPadInstance(my_GstPad_t* class);
void unwrapGstAggregatorPadInstance(my_GstAggregatorPad_t* class);
void bridgeGstAggregatorPadInstance(my_GstAggregatorPad_t* class);
void unwrapGstVideoAggregatorPadInstance(my_GstVideoAggregatorPad_t* class);
void bridgeGstVideoAggregatorPadInstance(my_GstVideoAggregatorPad_t* class);
void unwrapGstBaseSrcInstance(my_GstBaseSrc_t* class);
void bridgeGstBaseSrcInstance(my_GstBaseSrc_t* class);
void unwrapGstPushSrcInstance(my_GstPushSrc_t* class);
void bridgeGstPushSrcInstance(my_GstPushSrc_t* class);
void unwrapGstGLBaseSrcInstance(my_GstGLBaseSrc_t* class);
void bridgeGstGLBaseSrcInstance(my_GstGLBaseSrc_t* class);
void unwrapGstAudioDecoderInstance(my_GstAudioDecoder_t* class);
void bridgeGstAudioDecoderInstance(my_GstAudioDecoder_t* class);
void unwrapGstAudioEncoderInstance(my_GstAudioEncoder_t* class);
void bridgeGstAudioEncoderInstance(my_GstAudioEncoder_t* class);
void unwrapGstVideoFilterInstance(my_GstVideoFilter_t* class);
void bridgeGstVideoFilterInstance(my_GstVideoFilter_t* class);
void unwrapGstAudioFilterInstance(my_GstAudioFilter_t* class);
void bridgeGstAudioFilterInstance(my_GstAudioFilter_t* class);
void unwrapGstBufferPoolInstance(my_GstBufferPool_t* class);
void bridgeGstBufferPoolInstance(my_GstBufferPool_t* class);
void unwrapGstVideoBufferPoolInstance(my_GstVideoBufferPool_t* class);
void bridgeGstVideoBufferPoolInstance(my_GstVideoBufferPool_t* class);

// Interface function declarations
void wrapGstURIHandlerInterface(my_GstURIHandlerInterface_t* iface);
void unwrapGstURIHandlerInterface(my_GstURIHandlerInterface_t* iface);
void bridgeGstURIHandlerInterface(my_GstURIHandlerInterface_t* iface);

// Main dispatcher functions (must be externally callable)
void wrapGTKClass(void* cl, size_t type);
void unwrapGTKClass(void* cl, size_t type);
void bridgeGTKClass(void* cl, size_t type);
void unwrapGTKInstance(void* cl, size_t type);
void bridgeGTKInstance(void* cl, size_t type);

// Interface dispatcher functions
void wrapGTKInterface(void* cl, size_t type);
void unwrapGTKInterface(void* cl, size_t type);
void bridgeGTKInterface(void* cl, size_t type);

#endif /* GTK_MACROS_H */

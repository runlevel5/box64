#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "gtk_macros.h"

/*
 * GApplication and GtkApplication classes
 */

WRAPPER(GApplication, startup, void, (void* application), "p", application);
WRAPPER(GApplication, activate, void, (void* application), "p", application);
WRAPPER(GApplication, open, void, (void* application, void* files, int n_files, void* hint), "ppip", application, files, n_files, hint);
WRAPPER(GApplication, command_line, void, (void* application, void* command_line), "pp", application, command_line);
WRAPPER(GApplication, local_command_line, void, (void* application, void* arguments, void* exit_status), "ppp", application, arguments, exit_status);
WRAPPER(GApplication, before_emit, void*, (void* application, void* platform_data), "pp", application, platform_data);
WRAPPER(GApplication, after_emit, void, (void* application, void* platform_data), "pp", application, platform_data);
WRAPPER(GApplication, add_platform_data, void, (void* application, void* builder), "pp", application, builder);
WRAPPER(GApplication, quit_mainloop, void, (void* application), "p", application);
WRAPPER(GApplication, run_mainloop, void, (void* application), "p", application);
WRAPPER(GApplication, shutdown, void, (void* application), "p", application);
WRAPPER(GApplication, dbus_register, void, (void* application, void* connection, void* object_path, void* error), "pppp", application, connection, object_path, error);
WRAPPER(GApplication, dbus_unregister, void, (void* application, void* connection, void* object_path), "ppp", application, connection, object_path);
WRAPPER(GApplication, handle_local_options, void, (void* application, void* options), "pp", application, options);
WRAPPER(GApplication, name_lost, void, (void* application), "p", application);

#define SUPERGO()                     \
    GO(startup, vFp);                 \
    GO(activate, vFp);                \
    GO(open, vFppip);                 \
    GO(command_line, vFpp);           \
    GO(local_command_line, vFppp);    \
    GO(before_emit, vFpp);            \
    GO(after_emit, vFpp);             \
    GO(add_platform_data, vFpp);      \
    GO(quit_mainloop, vFp);           \
    GO(run_mainloop, vFp);            \
    GO(shutdown, vFp);                \
    GO(dbus_register, vFpppp);        \
    GO(dbus_unregister, vFppp);       \
    GO(handle_local_options, vFpp);   \
    GO(name_lost, vFp);

// wrap (so bridge all calls, just in case)
void wrapGApplicationClass(void* cl)
{
    my_GApplicationClass_t* class = (my_GApplicationClass_t*)cl;
    wrapGObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GApplication (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGApplicationClass(void* cl)
{
    my_GApplicationClass_t* class = (my_GApplicationClass_t*)cl;
    unwrapGObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GApplication (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGApplicationClass(void* cl)
{
    my_GApplicationClass_t* class = (my_GApplicationClass_t*)cl;
    bridgeGObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GApplication (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGApplicationInstance(my_GApplication_t* class)
{
    unwrapGObjectInstance(&class->parent);
}
// autobridge
void bridgeGApplicationInstance(my_GApplication_t* class)
{
    bridgeGObjectInstance(&class->parent);
}
#undef SUPERGO

// ----- GtkApplicationClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkApplication, window_added, void, (void* application, void* window), "pp", application, window);
WRAPPER(GtkApplication, window_removed, void, (void* application, void* window), "pp", application, window);

#define SUPERGO() \
    GO(window_added, pFpp);   \
    GO(window_removed, vFpp);

void wrapGtkApplicationClass(void* cl)
{
    my_GtkApplicationClass_t* class = (my_GtkApplicationClass_t*)cl;
    wrapGApplicationClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkApplication (W, class->A)
    SUPERGO()
    #undef GO
}

void unwrapGtkApplicationClass(void* cl)
{
    my_GtkApplicationClass_t* class = (my_GtkApplicationClass_t*)cl;
    unwrapGApplicationClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkApplication (W, class->A)
    SUPERGO()
    #undef GO
}

void bridgeGtkApplicationClass(void* cl)
{
    my_GtkApplicationClass_t* class = (my_GtkApplicationClass_t*)cl;
    bridgeGApplicationClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkApplication (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkApplicationInstance(my_GtkApplication_t* class)
{
    unwrapGApplicationInstance(&class->parent);
}

void bridgeGtkApplicationInstance(my_GtkApplication_t* class)
{
    bridgeGApplicationInstance(&class->parent);
}

// ----- GDBusProxy Class/Instance for linker compatibility -----
WRAPPER(GDBusProxy, g_properties_changed, void, (void* proxy, void* changed_properties, void* invalidated_properties), "ppp", proxy, changed_properties, invalidated_properties);
WRAPPER(GDBusProxy, g_signal, void, (void* proxy, void* sender_name, void* signal_name, void* parameters), "pppp", proxy, sender_name, signal_name, parameters);

#define SUPERGO()                   \
    GO(g_properties_changed, vFppp);\
    GO(g_signal, vFpppp);           \

// wrap (so bridge all calls, just in case)
void wrapGDBusProxyClass(void* cl)
{
    my_GDBusProxyClass_t* class = (my_GDBusProxyClass_t*)cl;
    wrapGObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GDBusProxy (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGDBusProxyClass(void* cl)
{
    my_GDBusProxyClass_t* class = (my_GDBusProxyClass_t*)cl;
    unwrapGObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GDBusProxy (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGDBusProxyClass(void* cl)
{
    my_GDBusProxyClass_t* class = (my_GDBusProxyClass_t*)cl;
    bridgeGObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GDBusProxy (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGDBusProxyInstance(my_GDBusProxy_t* class)
{
    unwrapGObjectInstance(&class->parent);
}

void bridgeGDBusProxyInstance(my_GDBusProxy_t* class)
{
    bridgeGObjectInstance(&class->parent);
}

// ----- GtkObjectClass ------
// wrapper x64 -> natives of callbacks

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "gtk_macros.h"

/*
 * GTK widget classes (Widget, Container, Controls)
 */

WRAPPER(GtkObject, set_arg, void, (void* object, void* arg, uint32_t arg_id), "ppu", object, arg, arg_id);
WRAPPER(GtkObject, get_arg, void, (void* object, void* arg, uint32_t arg_id), "ppu", object, arg, arg_id);
WRAPPER(GtkObject, destroy, void, (void* object), "p", object);

#define SUPERGO() \
    GO(set_arg, vFppu); \
    GO(get_arg, vFppu); \
    GO(destroy, vFp);
// wrap (so bridge all calls, just in case)
void wrapGtkObjectClass(void* cl)
{
    my_GtkObjectClass_t* class = (my_GtkObjectClass_t*)cl;
    wrapGInitiallyUnownedClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkObject (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkObjectClass(void* cl)
{
    my_GtkObjectClass_t* class = (my_GtkObjectClass_t*)cl;
    unwrapGInitiallyUnownedClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkObject (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkObjectClass(void* cl)
{
    my_GtkObjectClass_t* class = (my_GtkObjectClass_t*)cl;
    bridgeGInitiallyUnownedClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkObject (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkObjectInstance(my_GtkObject_t* class)
{
    unwrapGInitiallyUnownedInstance(&class->parent);
}
// autobridge
void bridgeGtkObjectInstance(my_GtkObject_t* class)
{
    bridgeGInitiallyUnownedInstance(&class->parent);
}

// ----- GtkWidget2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkWidget2, dispatch_child_properties_changed, void, (void* widget, uint32_t n_pspecs, void* pspecs), "pup", widget, n_pspecs, pspecs);
WRAPPER(GtkWidget2, show,              void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, show_all,          void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, hide,              void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, hide_all,          void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, map,               void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, unmap,             void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, realize,           void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, unrealize,         void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, size_request,      void, (void* widget, void* requisition), "pp", widget, requisition);
WRAPPER(GtkWidget2, size_allocate,     void, (void* widget, void* allocation), "pp", widget, allocation);
WRAPPER(GtkWidget2, state_changed,     void, (void* widget, int previous_state), "pi", widget, previous_state);
WRAPPER(GtkWidget2, parent_set,        void, (void* widget, void* previous_parent), "pp", widget, previous_parent);
WRAPPER(GtkWidget2, hierarchy_changed, void, (void* widget, void* previous_toplevel), "pp", widget, previous_toplevel);
WRAPPER(GtkWidget2, style_set,         void, (void* widget, void* previous_style), "pp", widget, previous_style);
WRAPPER(GtkWidget2, direction_changed, void, (void* widget, int previous_direction), "pi", widget, previous_direction);
WRAPPER(GtkWidget2, grab_notify,       void, (void* widget, int was_grabbed), "pi", widget, was_grabbed);
WRAPPER(GtkWidget2, child_notify,      void, (void* widget, void* pspec), "pp", widget, pspec);
WRAPPER(GtkWidget2, mnemonic_activate, int, (void* widget, int group_cycling), "pi", widget, group_cycling);
WRAPPER(GtkWidget2, grab_focus,        void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, focus,             int, (void* widget, int direction), "pi", widget, direction);
WRAPPER(GtkWidget2, event,             int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, button_press_event,int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, button_release_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, scroll_event,      int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, motion_notify_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, delete_event,       int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, destroy_event,      int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, expose_event,       int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, key_press_event,    int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, key_release_event,  int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, enter_notify_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, leave_notify_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, configure_event,    int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, focus_in_event,     int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, focus_out_event,    int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, map_event,          int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, unmap_event,        int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, property_notify_event,  int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, selection_clear_event,  int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, selection_request_event,int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, selection_notify_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, proximity_in_event,  int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, proximity_out_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, visibility_notify_event, int, (void* widget, void* event), "p", widget, event);
WRAPPER(GtkWidget2, client_event,        int, (void* widget, void* event), "p", widget, event);
WRAPPER(GtkWidget2, no_expose_event,     int, (void* widget, void* event), "p", widget, event);
WRAPPER(GtkWidget2, window_state_event,  int, (void* widget, void* event), "p", widget, event);
WRAPPER(GtkWidget2, selection_get,       void, (void* widget, void* selection_data, uint32_t info, uint32_t time_), "ppuu", widget, selection_data, info, time_);
WRAPPER(GtkWidget2, selection_received,  void, (void* widget, void* selection_data, uint32_t time_), "ppu", widget, selection_data, time_);
WRAPPER(GtkWidget2, drag_begin,          void, (void* widget, void* context), "pp", widget, context);
WRAPPER(GtkWidget2, drag_end,            void, (void* widget, void* context), "pp", widget, context);
WRAPPER(GtkWidget2, drag_data_get,       void, (void* widget, void* context, void* selection_data, uint32_t info, uint32_t time_), "pppuu", widget, context, selection_data, info, time_);
WRAPPER(GtkWidget2, drag_data_delete,    void, (void* widget, void* context), "pp", widget, context);
WRAPPER(GtkWidget2, drag_leave,          void, (void* widget, void* context, uint32_t time_), "ppu", widget, context, time_);
WRAPPER(GtkWidget2, drag_motion,         int, (void* widget, void* context, int32_t x, int32_t y, uint32_t time_), "ppiiu", widget, context, x, y, time_);
WRAPPER(GtkWidget2, drag_drop,           int, (void* widget, void* context, int32_t x, int32_t y, uint32_t time_), "ppiiu", widget, context, x, y, time_);
WRAPPER(GtkWidget2, drag_data_received,  void, (void* widget, void* context, int32_t x, int32_t y, void* selection_data, uint32_t info, uint32_t time_), "ppiipuu", widget, context, x, y, selection_data, info, time_);
WRAPPER(GtkWidget2,  popup_menu,         int  , (void* widget), "p", widget);
WRAPPER(GtkWidget2,  show_help,          int  , (void* widget, int help_type), "pi", widget, help_type);
WRAPPER(GtkWidget2, get_accessible,      void*, (void* widget), "p", widget);
WRAPPER(GtkWidget2, screen_changed,      void , (void* widget, void* previous_screen), "pp", widget, previous_screen);
WRAPPER(GtkWidget2, can_activate_accel,  int  , (void* widget, uint32_t signal_id), "pu", widget, signal_id);
WRAPPER(GtkWidget2, grab_broken_event,   int  , (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2,  composited_changed, void , (void* widget), "p", widget);
WRAPPER(GtkWidget2,  query_tooltip,      int  , (void* widget, int32_t x, int32_t y, int keyboard_tooltip, void* tooltip), "piiip", widget, x, y, keyboard_tooltip, tooltip);

#define SUPERGO() \
    GO(dispatch_child_properties_changed, vFpup);   \
    GO(show, vFp);                                  \
    GO(show_all, vFp);                              \
    GO(hide, vFp);                                  \
    GO(hide_all, vFp);                              \
    GO(map, vFp);                                   \
    GO(unmap, vFp);                                 \
    GO(realize, vFp);                               \
    GO(unrealize, vFp);                             \
    GO(size_request, vFpp);                         \
    GO(size_allocate, vFpp);                        \
    GO(state_changed, vFpi);                        \
    GO(parent_set, vFpp);                           \
    GO(hierarchy_changed, vFpp);                    \
    GO(style_set, vFpp);                            \
    GO(direction_changed, vFpi);                    \
    GO(grab_notify, vFpi);                          \
    GO(child_notify, vFpp);                         \
    GO(mnemonic_activate, iFpi);                    \
    GO(grab_focus, vFp);                            \
    GO(focus, iFpi);                                \
    GO(event, iFpp);                                \
    GO(button_press_event, iFpp);                   \
    GO(button_release_event, iFpp);                 \
    GO(scroll_event, iFpp);                         \
    GO(motion_notify_event, iFpp);                  \
    GO(delete_event, iFpp);                         \
    GO(destroy_event, iFpp);                        \
    GO(expose_event, iFpp);                         \
    GO(key_press_event, iFpp);                      \
    GO(key_release_event, iFpp);                    \
    GO(enter_notify_event, iFpp);                   \
    GO(leave_notify_event, iFpp);                   \
    GO(configure_event, iFpp);                      \
    GO(focus_in_event, iFpp);                       \
    GO(focus_out_event, iFpp);                      \
    GO(map_event, iFpp);                            \
    GO(unmap_event, iFpp);                          \
    GO(property_notify_event, iFpp);                \
    GO(selection_clear_event, iFpp);                \
    GO(selection_request_event, iFpp);              \
    GO(selection_notify_event, iFpp);               \
    GO(proximity_in_event, iFpp);                   \
    GO(proximity_out_event, iFpp);                  \
    GO(visibility_notify_event, iFpp);              \
    GO(client_event, iFpp);                         \
    GO(no_expose_event, iFpp);                      \
    GO(window_state_event, iFpp);                   \
    GO(selection_get, vFppuu);                      \
    GO(selection_received, vFppu);                  \
    GO(drag_begin, vFpp);                           \
    GO(drag_end, vFpp);                             \
    GO(drag_data_get, vFpppuu);                     \
    GO(drag_data_delete, vFpp);                     \
    GO(drag_leave, vFppu);                          \
    GO(drag_motion, iFppiiu);                       \
    GO(drag_drop, iFppiiu);                         \
    GO(drag_data_received, vFppiipuu);              \
    GO(popup_menu, iFp);                            \
    GO(show_help, iFpi);                            \
    GO(get_accessible, pFp);                        \
    GO(screen_changed, vFpp);                       \
    GO(can_activate_accel, iFpu);                   \
    GO(grab_broken_event, iFpp);                    \
    GO(composited_changed, vFp);                    \
    GO(query_tooltip, iFpiiip);

// wrap (so bridge all calls, just in case)
void wrapGtkWidget2Class(void* cl)
{
    my_GtkWidget2Class_t* class = (my_GtkWidget2Class_t*)cl;
    wrapGtkObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkWidget2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkWidget2Class(void* cl)
{
    my_GtkWidget2Class_t* class = (my_GtkWidget2Class_t*)cl;
    unwrapGtkObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkWidget2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkWidget2Class(void* cl)
{
    my_GtkWidget2Class_t* class = (my_GtkWidget2Class_t*)cl;
    bridgeGtkObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkWidget2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkWidget2Instance(my_GtkWidget2_t* class)
{
    unwrapGtkObjectInstance(&class->private);
}
// autobridge
void bridgeGtkWidget2Instance(my_GtkWidget2_t* class)
{
    bridgeGtkObjectInstance(&class->private);
}

// ----- GtkWidget3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkWidget3, dispatch_child_properties_changed, void, (void* widget, uint32_t n_pspecs, void* pspecs), "pup", widget, n_pspecs, pspecs);
WRAPPER(GtkWidget3, destroy, void,              (void* widget), "p", widget);
WRAPPER(GtkWidget3, show, void,                 (void* widget), "p", widget);
WRAPPER(GtkWidget3, show_all, void,             (void* widget), "p", widget);
WRAPPER(GtkWidget3, hide, void,                 (void* widget), "p", widget);
WRAPPER(GtkWidget3, map, void,                  (void* widget), "p", widget);
WRAPPER(GtkWidget3, unmap, void,                (void* widget), "p", widget);
WRAPPER(GtkWidget3, realize, void,              (void* widget), "p", widget);
WRAPPER(GtkWidget3, unrealize, void,            (void* widget), "p", widget);
WRAPPER(GtkWidget3, size_allocate, void,        (void* widget, void* allocation), "pp", widget, allocation);
WRAPPER(GtkWidget3, state_changed, void,        (void* widget, int previous_state), "pi", widget, previous_state);
WRAPPER(GtkWidget3, state_flags_changed, void,  (void* widget, int previous_state_flags), "pi", widget, previous_state_flags);
WRAPPER(GtkWidget3, parent_set, void,           (void* widget, void* previous_parent), "pp", widget, previous_parent);
WRAPPER(GtkWidget3, hierarchy_changed, void,    (void* widget, void* previous_toplevel), "pp", widget, previous_toplevel);
WRAPPER(GtkWidget3, style_set, void,            (void* widget, void* previous_style), "pp", widget, previous_style);
WRAPPER(GtkWidget3, direction_changed, void,    (void* widget, int previous_direction), "pi", widget, previous_direction);
WRAPPER(GtkWidget3, grab_notify, void,          (void* widget, int was_grabbed), "pi", widget, was_grabbed);
WRAPPER(GtkWidget3, child_notify, void,         (void* widget, void* child_property), "pp", widget, child_property);
WRAPPER(GtkWidget3, draw, int,                  (void* widget, void* cr), "pp", widget, cr);
WRAPPER(GtkWidget3, get_request_mode, int,      (void* widget), "p", widget);
WRAPPER(GtkWidget3, get_preferred_height, void, (void* widget, void* minimum_height, void* natural_height), "ppp", widget, minimum_height, natural_height);
WRAPPER(GtkWidget3, get_preferred_width_for_height, void,  (void* widget, int height, void* minimum_width, void* natural_width), "pipp", widget, height, minimum_width, natural_width);
WRAPPER(GtkWidget3, get_preferred_width, void,  (void* widget, void* minimum_width, void* natural_width), "ppp", widget, minimum_width, natural_width);
WRAPPER(GtkWidget3, get_preferred_height_for_width, void,  (void* widget, int width, void* minimum_height, void* natural_height), "pipp", widget, width, minimum_height, natural_height);
WRAPPER(GtkWidget3, mnemonic_activate, int,     (void* widget, int group_cycling), "pi", widget, group_cycling);
WRAPPER(GtkWidget3, grab_focus, void,           (void* widget), "p", widget);
WRAPPER(GtkWidget3, focus, int,                 (void* widget, int direction), "pi", widget, direction);
WRAPPER(GtkWidget3, move_focus, void,           (void* widget, int direction), "pi", widget, direction);
WRAPPER(GtkWidget3, keynav_failed, int,         (void* widget, int direction), "pi", widget, direction);
WRAPPER(GtkWidget3, event, int,                 (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, button_press_event, int,    (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, button_release_event, int,  (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, scroll_event, int,          (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, motion_notify_event, int,   (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, delete_event, int,          (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, destroy_event, int,         (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, key_press_event, int,       (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, key_release_event, int,     (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, enter_notify_event, int,    (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, leave_notify_event, int,    (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, configure_event, int,       (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, focus_in_event, int,        (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, focus_out_event, int,       (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, map_event, int,             (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, unmap_event, int,           (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, property_notify_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, selection_clear_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, selection_request_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, selection_notify_event, int,(void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, proximity_in_event, int,    (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, proximity_out_event, int,   (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, visibility_notify_event, int,   (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, window_state_event, int,    (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, damage_event, int,          (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, grab_broken_event, int,     (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, selection_get, void,        (void* widget, void* selection_data, uint32_t info, uint32_t time_), "ppuu", widget, selection_data, info, time_);
WRAPPER(GtkWidget3, selection_received, void,   (void* widget, void* selection_data, uint32_t time_), "ppu", widget, selection_data, time_);
WRAPPER(GtkWidget3, drag_begin, void,           (void* widget, void* context), "pp", widget, context);
WRAPPER(GtkWidget3, drag_end, void,             (void* widget, void* context), "pp", widget, context);
WRAPPER(GtkWidget3, drag_data_get, void,        (void* widget, void* context, void* selection_data, uint32_t info, uint32_t time_), "pppuu", widget, context, selection_data, info, time_);
WRAPPER(GtkWidget3, drag_data_delete, void,     (void* widget, void* context), "pp", widget, context);
WRAPPER(GtkWidget3, drag_leave, void,           (void* widget, void* context, uint32_t time_), "ppu", widget, context, time_);
WRAPPER(GtkWidget3, drag_motion, int,           (void* widget, void* context, int x, int y, uint32_t time_), "ppiiu", widget, context, x, y, time_);
WRAPPER(GtkWidget3, drag_drop, int,             (void* widget, void* context, int x, int y, uint32_t time_), "ppiiu", widget, context, x, y, time_);
WRAPPER(GtkWidget3, drag_data_received, void,   (void* widget, void* context, int x, int y, void* selection_data, uint32_t info, uint32_t time_), "ppiipuu", widget, context, x, y, selection_data, info, time_);
WRAPPER(GtkWidget3, drag_failed, int,           (void* widget, void* context, int result), "ppi", widget, context, result);
WRAPPER(GtkWidget3, popup_menu, int,            (void* widget), "p", widget);
WRAPPER(GtkWidget3, show_help, int,             (void* widget, int help_type), "pi", widget, help_type);
WRAPPER(GtkWidget3, get_accessible, void*,      (void *widget), "p", widget);
WRAPPER(GtkWidget3, screen_changed, void,       (void* widget, void* previous_screen), "pp", widget, previous_screen);
WRAPPER(GtkWidget3, can_activate_accel, int,    (void* widget, uint32_t signal_id), "pu", widget, signal_id);
WRAPPER(GtkWidget3, composited_changed, void,   (void* widget), "p", widget);
WRAPPER(GtkWidget3, query_tooltip, int,         (void* widget, int x, int y, int keyboard_tooltip, void* tooltip), "piiip", widget, x, y, keyboard_tooltip, tooltip);
WRAPPER(GtkWidget3, compute_expand, void,       (void* widget, void* hexpand_p, void* vexpand_p), "ppp", widget, hexpand_p, vexpand_p);
WRAPPER(GtkWidget3, adjust_size_request, void,  (void* widget, int orientation, void* minimum_size, void* natural_size), "pipp", widget, orientation, minimum_size, natural_size);
WRAPPER(GtkWidget3, adjust_size_allocation, void, (void*widget, int orientation, void* minimum_size, void* natural_size, void* allocated_pos, void* allocated_size), "pipppp", widget, orientation, minimum_size, natural_size, allocated_pos, allocated_size);
WRAPPER(GtkWidget3, style_updated, void,        (void* widget), "p", widget);
WRAPPER(GtkWidget3, touch_event, int,           (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, get_preferred_height_and_baseline_for_width, void, (void* widget, int width, void* minimum_height, void* natural_height, void* minimum_baseline, void* natural_baseline), "pipppp", widget, width, minimum_height, natural_height, minimum_baseline, natural_baseline);
WRAPPER(GtkWidget3, adjust_baseline_request, void,  (void* widget, void* minimum_baseline, void* natural_baseline), "ppp", widget, minimum_baseline, natural_baseline);
WRAPPER(GtkWidget3, adjust_baseline_allocation, void,  (void* widget, void* baseline), "pp", widget, baseline);
WRAPPER(GtkWidget3, queue_draw_region, void,    (void* widget, void* region), "pp", widget, region);

#define SUPERGO() \
    GO(dispatch_child_properties_changed, vFpup);   \
    GO(destroy, vFp);                               \
    GO(show, vFp);                                  \
    GO(show_all, vFp);                              \
    GO(hide, vFp);                                  \
    GO(map, vFp);                                   \
    GO(unmap, vFp);                                 \
    GO(realize, vFp);                               \
    GO(unrealize, vFp);                             \
    GO(size_allocate, vFpp);                        \
    GO(state_changed, vFpi);                        \
    GO(state_flags_changed, vFpi);                  \
    GO(parent_set, vFpp);                           \
    GO(hierarchy_changed, vFpp);                    \
    GO(style_set, vFpp);                            \
    GO(direction_changed, vFpi);                    \
    GO(grab_notify, vFpi);                          \
    GO(child_notify, vFpp);                         \
    GO(draw, iFpp);                                 \
    GO(get_request_mode, iFp);                      \
    GO(get_preferred_height, vFppp);                \
    GO(get_preferred_width_for_height, vFpipp);     \
    GO(get_preferred_width, vFppp);                 \
    GO(get_preferred_height_for_width, vFpipp);     \
    GO(mnemonic_activate, iFpi);                    \
    GO(grab_focus, vFp);                            \
    GO(focus, iFpi);                                \
    GO(move_focus, vFpi);                           \
    GO(keynav_failed, iFpi);                        \
    GO(event, iFpp);                                \
    GO(button_press_event, iFpp);                   \
    GO(button_release_event, iFpp);                 \
    GO(scroll_event, iFpp);                         \
    GO(motion_notify_event, iFpp);                  \
    GO(delete_event, iFpp);                         \
    GO(destroy_event, iFpp);                        \
    GO(key_press_event, iFpp);                      \
    GO(key_release_event, iFpp);                    \
    GO(enter_notify_event, iFpp);                   \
    GO(leave_notify_event, iFpp);                   \
    GO(configure_event, iFpp);                      \
    GO(focus_in_event, iFpp);                       \
    GO(focus_out_event, iFpp);                      \
    GO(map_event, iFpp);                            \
    GO(unmap_event, iFpp);                          \
    GO(property_notify_event, iFpp);                \
    GO(selection_clear_event, iFpp);                \
    GO(selection_request_event, iFpp);              \
    GO(selection_notify_event, iFpp);               \
    GO(proximity_in_event, iFpp);                   \
    GO(proximity_out_event, iFpp);                  \
    GO(visibility_notify_event, iFpp);              \
    GO(window_state_event, iFpp);                   \
    GO(damage_event, iFpp);                         \
    GO(grab_broken_event, iFpp);                    \
    GO(selection_get, vFppuu);                      \
    GO(selection_received, vFppu);                  \
    GO(drag_begin, vFpp);                           \
    GO(drag_end, vFpp);                             \
    GO(drag_data_get, vFpppuu);                     \
    GO(drag_data_delete, vFpp);                     \
    GO(drag_leave, vFppu);                          \
    GO(drag_motion, iFppiiu);                       \
    GO(drag_drop, iFppiiu);                         \
    GO(drag_data_received, vFppiipuu);              \
    GO(drag_failed, iFppi);                         \
    GO(popup_menu, iFp);                            \
    GO(show_help, iFpi);                            \
    GO(get_accessible, pFp);                        \
    GO(screen_changed, vFpp);                       \
    GO(can_activate_accel, iFpu);                   \
    GO(grab_broken_event, iFpp);                    \
    GO(composited_changed, vFp);                    \
    GO(query_tooltip, iFpiiip);                     \
    GO(compute_expand, vFppp);                      \
    GO(adjust_size_request, vFpipp);                \
    GO(adjust_size_allocation, vFpipppp);           \
    GO(style_updated, vFp);                         \
    GO(touch_event, iFpp);                          \
    GO(get_preferred_height_and_baseline_for_width, vFpipppp);\
    GO(adjust_baseline_request, vFppp);             \
    GO(adjust_baseline_allocation, vFpp);           \
    GO(queue_draw_region, vFpp);                    \

// wrap (so bridge all calls, just in case)
void wrapGtkWidget3Class(void* cl)
{
    my_GtkWidget3Class_t* class = (my_GtkWidget3Class_t*)cl;
    wrapGInitiallyUnownedClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkWidget3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkWidget3Class(void* cl)
{
    my_GtkWidget3Class_t* class = (my_GtkWidget3Class_t*)cl;
    unwrapGInitiallyUnownedClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkWidget3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkWidget3Class(void* cl)
{
    my_GtkWidget3Class_t* class = (my_GtkWidget3Class_t*)cl;
    bridgeGInitiallyUnownedClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkWidget3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkWidget3Instance(my_GtkWidget3_t* class)
{
    unwrapGInitiallyUnownedInstance(&class->parent);
}
// autobridge
void bridgeGtkWidget3Instance(my_GtkWidget3_t* class)
{
    bridgeGInitiallyUnownedInstance(&class->parent);
}

// ----- GtkContainer2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkContainer2, add, void, (void* container, void* widget), "pp", container, widget);
WRAPPER(GtkContainer2, remove, void, (void* container, void* widget), "pp", container, widget);
WRAPPER(GtkContainer2, check_resize, void, (void* container), "p", container);
WRAPPER(GtkContainer2, forall, void, (void* container, int include_internals, void* callback, void* callback_data), "pipp", container, include_internals, AddCheckBridge(my_bridge, vFpp, callback, 0, NULL), callback_data);
WRAPPER(GtkContainer2, set_focus_child, void, (void* container, void* widget), "pp", container, widget);
WRAPPER(GtkContainer2, child_type, int, (void* container), "p", container);
WRAPPER(GtkContainer2, composite_name, void*, (void* container, void* child), "pp", container, child);
WRAPPER(GtkContainer2, set_child_property, void, (void* container, void* child, uint32_t property_id, void* value, void* pspec), "ppupp", container, child, property_id, value, pspec);
WRAPPER(GtkContainer2, get_child_property, void, (void* container, void* child, uint32_t property_id, void* value, void* pspec), "ppupp", container, child, property_id, value, pspec);

#define SUPERGO() \
    GO(add, vFpp);                  \
    GO(remove, vFpp);               \
    GO(check_resize, vFp);          \
    GO(forall, vFpipp);             \
    GO(set_focus_child, vFpp);      \
    GO(child_type, iFp);            \
    GO(composite_name, pFpp);       \
    GO(set_child_property, vFppupp);\
    GO(get_child_property, vFppupp);

// wrap (so bridge all calls, just in case)
void wrapGtkContainer2Class(void* cl)
{
    my_GtkContainer2Class_t* class = (my_GtkContainer2Class_t*)cl;
    wrapGtkWidget2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkContainer2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkContainer2Class(void* cl)
{
    my_GtkContainer2Class_t* class = (my_GtkContainer2Class_t*)cl;
    unwrapGtkWidget2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkContainer2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkContainer2Class(void* cl)
{
    my_GtkContainer2Class_t* class = (my_GtkContainer2Class_t*)cl;
    bridgeGtkWidget2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkContainer2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkContainer2Instance(my_GtkContainer2_t* class)
{
    unwrapGtkWidget2Instance(&class->parent);
}
// autobridge
void bridgeGtkContainer2Instance(my_GtkContainer2_t* class)
{
    bridgeGtkWidget2Instance(&class->parent);
}

// ----- GtkContainer3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkContainer3, add, void, (void* container, void* widget), "pp", container, widget);
WRAPPER(GtkContainer3, remove, void, (void* container, void* widget), "pp", container, widget);
WRAPPER(GtkContainer3, check_resize, void, (void* container), "p", container);
WRAPPER(GtkContainer3, forall, void, (void* container, int include_internals, void* callback, void* callback_data), "pipp", container, include_internals, AddCheckBridge(my_bridge, vFpp, callback, 0, NULL), callback_data);
WRAPPER(GtkContainer3, set_focus_child, void, (void* container, void* widget), "pp", container, widget);
WRAPPER(GtkContainer3, child_type, int, (void* container), "p", container);
WRAPPER(GtkContainer3, composite_name, void*, (void* container, void* child), "pp", container, child);
WRAPPER(GtkContainer3, set_child_property, void, (void* container, void* child, uint32_t property_id, void* value, void* pspec), "ppupp", container, child, property_id, value, pspec);
WRAPPER(GtkContainer3, get_child_property, void, (void* container, void* child, uint32_t property_id, void* value, void* pspec), "ppupp", container, child, property_id, value, pspec);
WRAPPER(GtkContainer3, get_path_for_child, void*, (void* container, void* child), "pp", container, child);

#define SUPERGO() \
    GO(add, vFpp);                  \
    GO(remove, vFpp);               \
    GO(check_resize, vFp);          \
    GO(forall, vFpipp);             \
    GO(set_focus_child, vFpp);      \
    GO(child_type, iFp);            \
    GO(composite_name, pFpp);       \
    GO(set_child_property, vFppupp);\
    GO(get_child_property, vFppupp);\
    GO(get_path_for_child, pFpp);   \

// wrap (so bridge all calls, just in case)
void wrapGtkContainer3Class(void* cl)
{
    my_GtkContainer3Class_t* class = (my_GtkContainer3Class_t*)cl;
    wrapGtkWidget3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkContainer3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkContainer3Class(void* cl)
{
    my_GtkContainer3Class_t* class = (my_GtkContainer3Class_t*)cl;
    unwrapGtkWidget3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkContainer3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkContainer3Class(void* cl)
{
    my_GtkContainer3Class_t* class = (my_GtkContainer3Class_t*)cl;
    bridgeGtkWidget3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkContainer3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkContainer3Instance(my_GtkContainer3_t* class)
{
    unwrapGtkWidget3Instance(&class->parent);
}
// autobridge
void bridgeGtkContainer3Instance(my_GtkContainer3_t* class)
{
    bridgeGtkWidget3Instance(&class->parent);
}

// ----- GtkActionClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkAction, activate, void, (void* action), "p", action);
WRAPPER(GtkAction, create_menu_item, void*, (void* action), "p", action);
WRAPPER(GtkAction, create_tool_item, void*, (void* action), "p", action);
WRAPPER(GtkAction, connect_proxy, void , (void* action, void* proxy), "pp", action, proxy);
WRAPPER(GtkAction, disconnect_proxy, void , (void* action, void* proxy), "pp", action, proxy);
WRAPPER(GtkAction, create_menu, void*, (void* action), "p", action);

#define SUPERGO() \
    GO(activate, vFp);          \
    GO(create_menu_item, pFp);  \
    GO(create_tool_item, pFp);  \
    GO(connect_proxy, vFpp);    \
    GO(disconnect_proxy, vFpp); \
    GO(create_menu, pFp);       \

// wrap (so bridge all calls, just in case)
void wrapGtkActionClass(void* cl)
{
    my_GtkActionClass_t* class = (my_GtkActionClass_t*)cl;
    wrapGObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkAction (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkActionClass(void* cl)
{
    my_GtkActionClass_t* class = (my_GtkActionClass_t*)cl;
    unwrapGObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkAction (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkActionClass(void* cl)
{
    my_GtkActionClass_t* class = (my_GtkActionClass_t*)cl;
    bridgeGObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkAction (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

void unwrapGtkActionInstance(my_GtkAction_t* class)
{
    unwrapGObjectInstance(&class->parent);
}
// autobridge
void bridgeGtkActionInstance(my_GtkAction_t* class)
{
    bridgeGObjectInstance(&class->parent);
}

// ----- GtkDrawingArea3Class ------

// wrap (so bridge all calls, just in case)
void wrapGtkDrawingArea3Class(void* cl)
{
    my_GtkDrawingArea3Class_t* class = (my_GtkDrawingArea3Class_t*)cl;
    wrapGtkWidget3Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkDrawingArea3Class(void* cl)
{
    my_GtkDrawingArea3Class_t* class = (my_GtkDrawingArea3Class_t*)cl;
    unwrapGtkWidget3Class(&class->parent_class);
}
// autobridge
void bridgeGtkDrawingArea3Class(void* cl)
{
    my_GtkDrawingArea3Class_t* class = (my_GtkDrawingArea3Class_t*)cl;
    bridgeGtkWidget3Class(&class->parent_class);
}

void unwrapGtkDrawingArea3Instance(my_GtkDrawingArea3_t* class)
{
    unwrapGtkWidget3Instance(&class->parent);
}
// autobridge
void bridgeGtkDrawingArea3Instance(my_GtkDrawingArea3_t* class)
{
    bridgeGtkWidget3Instance(&class->parent);
}

// ----- GtkMisc2Class ------

// wrap (so bridge all calls, just in case)
void wrapGtkMisc2Class(void* cl)
{
    my_GtkMisc2Class_t* class = (my_GtkMisc2Class_t*)cl;
    wrapGtkWidget2Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkMisc2Class(void* cl)
{
    my_GtkMisc2Class_t* class = (my_GtkMisc2Class_t*)cl;
    unwrapGtkWidget2Class(&class->parent_class);
}
// autobridge
void bridgeGtkMisc2Class(void* cl)
{
    my_GtkMisc2Class_t* class = (my_GtkMisc2Class_t*)cl;
    bridgeGtkWidget2Class(&class->parent_class);
}

void unwrapGtkMisc2Instance(my_GtkMisc2_t* class)
{
    unwrapGtkWidget2Instance(&class->parent);
}
// autobridge
void bridgeGtkMisc2Instance(my_GtkMisc2_t* class)
{
    bridgeGtkWidget2Instance(&class->parent);
}

// ----- GtkMisc3Class ------
// no wrapper x86 -> natives of callbacks

#define SUPERGO()           \

// wrap (so bridge all calls, just in case)
void wrapGtkMisc3Class(void* cl)
{
    my_GtkMisc3Class_t* class = (my_GtkMisc3Class_t*)cl;
    wrapGtkWidget3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkMisc3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkMisc3Class(void* cl)
{
    my_GtkMisc3Class_t* class = (my_GtkMisc3Class_t*)cl;
    unwrapGtkWidget3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkMisc3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkMisc3Class(void* cl)
{
    my_GtkMisc3Class_t* class = (my_GtkMisc3Class_t*)cl;
    bridgeGtkWidget3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkMisc3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkMisc3Instance(my_GtkMisc3_t* class)
{
    unwrapGtkWidget3Instance(&class->parent);
}
// autobridge
void bridgeGtkMisc3Instance(my_GtkMisc3_t* class)
{
    bridgeGtkWidget3Instance(&class->parent);
}

// ----- GtkImage3Class ------

// wrap (so bridge all calls, just in case)
void wrapGtkImage3Class(void* cl)
{
    my_GtkImage3Class_t* class = (my_GtkImage3Class_t*)cl;
    wrapGtkMisc3Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkImage3Class(void* cl)
{
    my_GtkImage3Class_t* class = (my_GtkImage3Class_t*)cl;
    unwrapGtkMisc3Class(&class->parent_class);
}
// autobridge
void bridgeGtkImage3Class(void* cl)
{
    my_GtkImage3Class_t* class = (my_GtkImage3Class_t*)cl;
    bridgeGtkMisc3Class(&class->parent_class);
}

void unwrapGtkImage3Instance(my_GtkImage3_t* class)
{
    unwrapGtkMisc3Instance(&class->parent);
}
// autobridge
void bridgeGtkImage3Instance(my_GtkImage3_t* class)
{
    bridgeGtkMisc3Instance(&class->parent);
}


// ----- GtkLabel2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkLabel2, move_cursor, void, (void* label, int step, int count, int extend_selection), "piii", label, step, count, extend_selection);
WRAPPER(GtkLabel2, copy_clipboard, void, (void* label), "p", label);
WRAPPER(GtkLabel2, populate_popup, void, (void* label, void* menu), "pp", label, menu);
WRAPPER(GtkLabel2, activate_link, int, (void* label, void* uri), "pp", label, uri);

#define SUPERGO() \
    GO(move_cursor, vFpiii);    \
    GO(copy_clipboard, vFp);    \
    GO(populate_popup, vFpp);   \
    GO(activate_link, iFpp);    \

// wrap (so bridge all calls, just in case)
void wrapGtkLabel2Class(void* cl)
{
    my_GtkLabel2Class_t* class = (my_GtkLabel2Class_t*)cl;
    wrapGtkMisc2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkLabel2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkLabel2Class(void* cl)
{
    my_GtkLabel2Class_t* class = (my_GtkLabel2Class_t*)cl;
    unwrapGtkMisc2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkLabel2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkLabel2Class(void* cl)
{
    my_GtkLabel2Class_t* class = (my_GtkLabel2Class_t*)cl;
    bridgeGtkMisc2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkLabel2 (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

void unwrapGtkLabel2Instance(my_GtkLabel2_t* class)
{
    unwrapGtkMisc2Instance(&class->misc);
}
// autobridge
void bridgeGtkLabel2Instance(my_GtkLabel2_t* class)
{
    bridgeGtkMisc2Instance(&class->misc);
}

// ----- GtkLabel3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkLabel3, move_cursor, void, (void* label, int step, int count, int extend_selection), "piii", label, step, count, extend_selection);
WRAPPER(GtkLabel3, copy_clipboard, void, (void* label), "p", label);
WRAPPER(GtkLabel3, populate_popup, void, (void* label, void* menu), "pp", label, menu);
WRAPPER(GtkLabel3, activate_link, int, (void* label, void* uri), "pp", label, uri);

#define SUPERGO() \
    GO(move_cursor, vFpiii);    \
    GO(copy_clipboard, vFp);    \
    GO(populate_popup, vFpp);   \
    GO(activate_link, iFpp);    \

// wrap (so bridge all calls, just in case)
void wrapGtkLabel3Class(void* cl)
{
    my_GtkLabel3Class_t* class = (my_GtkLabel3Class_t*)cl;
    wrapGtkMisc3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkLabel3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkLabel3Class(void* cl)
{
    my_GtkLabel3Class_t* class = (my_GtkLabel3Class_t*)cl;
    unwrapGtkMisc3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkLabel3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkLabel3Class(void* cl)
{
    my_GtkLabel3Class_t* class = (my_GtkLabel3Class_t*)cl;
    bridgeGtkMisc3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkLabel3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkLabel3Instance(my_GtkLabel3_t* class)
{
    unwrapGtkMisc3Instance(&class->misc);
}
// autobridge
void bridgeGtkLabel3Instance(my_GtkLabel3_t* class)
{
    bridgeGtkMisc3Instance(&class->misc);
}

// ----- GtkTreeView2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkTreeView2, set_scroll_adjustments, void, (void* tree_view, void* hadjustment, void* vadjustment), "ppp", tree_view, hadjustment, vadjustment);
WRAPPER(GtkTreeView2, row_activated, void, (void* tree_view, void* path, void* column), "ppp", tree_view, path, column);
WRAPPER(GtkTreeView2, test_expand_row, int, (void* tree_view, void* iter, void* path), "ppp", tree_view, iter, path);
WRAPPER(GtkTreeView2, test_collapse_row, int, (void* tree_view, void* iter, void* path), "ppp", tree_view, iter, path);
WRAPPER(GtkTreeView2, row_expanded, void, (void* tree_view, void* iter, void* path), "ppp", tree_view, iter, path);
WRAPPER(GtkTreeView2, row_collapsed, void, (void* tree_view, void* iter, void* path), "ppp", tree_view, iter, path);
WRAPPER(GtkTreeView2, columns_changed, void, (void* tree_view), "p", tree_view);
WRAPPER(GtkTreeView2, cursor_changed, void, (void* tree_view), "p", tree_view);
WRAPPER(GtkTreeView2, move_cursor, int, (void* tree_view, int step, int count), "pii", tree_view, step, count);
WRAPPER(GtkTreeView2, select_all, int, (void* tree_view), "p", tree_view);
WRAPPER(GtkTreeView2, unselect_all, int, (void* tree_view), "p", tree_view);
WRAPPER(GtkTreeView2, select_cursor_row, int, (void* tree_view, int start_editing), "pi", tree_view, start_editing);
WRAPPER(GtkTreeView2, toggle_cursor_row, int, (void* tree_view), "p", tree_view);
WRAPPER(GtkTreeView2, expand_collapse_cursor_row, int, (void* tree_view, int logical, int expand, int open_all), "piii", tree_view, logical, expand, open_all);
WRAPPER(GtkTreeView2, select_cursor_parent, int, (void* tree_view), "p", tree_view);
WRAPPER(GtkTreeView2, start_interactive_search, int, (void* tree_view), "p", tree_view);

#define SUPERGO() \
    GO(set_scroll_adjustments, vFppp);      \
    GO(row_activated, vFppp);               \
    GO(test_expand_row, iFppp);             \
    GO(test_collapse_row, iFppp);           \
    GO(row_expanded, vFppp);                \
    GO(row_collapsed, vFppp);               \
    GO(columns_changed, vFp);               \
    GO(cursor_changed, vFp);                \
    GO(move_cursor, iFppp);                 \
    GO(select_all, iFp);                    \
    GO(unselect_all, iFp);                  \
    GO(select_cursor_row, iFpi);            \
    GO(toggle_cursor_row, iFp);             \
    GO(expand_collapse_cursor_row, iFpiii); \
    GO(select_cursor_parent, iFp);          \
    GO(start_interactive_search, iFp);      \

// wrap (so bridge all calls, just in case)
void wrapGtkTreeView2Class(void* cl)
{
    my_GtkTreeView2Class_t* class = (my_GtkTreeView2Class_t*)cl;
    wrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkTreeView2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkTreeView2Class(void* cl)
{
    my_GtkTreeView2Class_t* class = (my_GtkTreeView2Class_t*)cl;
    unwrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkTreeView2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkTreeView2Class(void* cl)
{
    my_GtkTreeView2Class_t* class = (my_GtkTreeView2Class_t*)cl;
    bridgeGtkContainer2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkTreeView2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkTreeView2Instance(my_GtkTreeView2_t* class)
{
    unwrapGtkContainer2Instance(&class->parent);
}
// autobridge
void bridgeGtkTreeView2Instance(my_GtkTreeView2_t* class)
{
    bridgeGtkContainer2Instance(&class->parent);
}

// ----- GtkBin2Class ------

// wrap (so bridge all calls, just in case)
void wrapGtkBin2Class(void* cl)
{
    my_GtkBin2Class_t* class = (my_GtkBin2Class_t*)cl;
    wrapGtkContainer2Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkBin2Class(void* cl)
{
    my_GtkBin2Class_t* class = (my_GtkBin2Class_t*)cl;
    unwrapGtkContainer2Class(&class->parent_class);
}
// autobridge
void bridgeGtkBin2Class(void* cl)
{
    my_GtkBin2Class_t* class = (my_GtkBin2Class_t*)cl;
    bridgeGtkContainer2Class(&class->parent_class);
}

void unwrapGtkBin2Instance(my_GtkBin2_t* class)
{
    unwrapGtkContainer2Instance(&class->container);
}
// autobridge
void bridgeGtkBin2Instance(my_GtkBin2_t* class)
{
    bridgeGtkContainer2Instance(&class->container);
}

// ----- GtkBin3Class ------
void wrapGtkBin3Class(void* cl)
{
    my_GtkBin3Class_t* class = (my_GtkBin3Class_t*)cl;
    wrapGtkContainer3Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkBin3Class(void* cl)
{
    my_GtkBin3Class_t* class = (my_GtkBin3Class_t*)cl;
    unwrapGtkContainer3Class(&class->parent_class);
}
// autobridge
void bridgeGtkBin3Class(void* cl)
{
    my_GtkBin3Class_t* class = (my_GtkBin3Class_t*)cl;
    bridgeGtkContainer3Class(&class->parent_class);
}

void unwrapGtkBin3Instance(my_GtkBin3_t* class)
{
    unwrapGtkContainer3Instance(&class->container);
}
// autobridge
void bridgeGtkBin3Instance(my_GtkBin3_t* class)
{
    bridgeGtkContainer3Instance(&class->container);
}
// ----- GtkWindow2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkWindow2, set_focus, void, (void* window, void* focus), "pp", window, focus);
WRAPPER(GtkWindow2, frame_event, int, (void* window, void* event), "pp", window, event);
WRAPPER(GtkWindow2, activate_focus, void, (void* window), "p", window);
WRAPPER(GtkWindow2, activate_default, void, (void* window), "p", window);
WRAPPER(GtkWindow2, move_focus, void, (void* window, int direction), "pi", window, direction);
WRAPPER(GtkWindow2, keys_changed, void, (void* window), "p", window);

#define SUPERGO()               \
    GO(set_focus, vFpp);        \
    GO(frame_event, iFpp);      \
    GO(activate_focus, vFp);    \
    GO(activate_default, vFp);  \
    GO(move_focus, vFpi);       \
    GO(keys_changed, vFp);      \


// wrap (so bridge all calls, just in case)
void wrapGtkWindow2Class(void* cl)
{
    my_GtkWindow2Class_t* class = (my_GtkWindow2Class_t*)cl;
    wrapGtkBin2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkWindow2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkWindow2Class(void* cl)
{
    my_GtkWindow2Class_t* class = (my_GtkWindow2Class_t*)cl;
    unwrapGtkBin2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkWindow2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkWindow2Class(void* cl)
{
    my_GtkWindow2Class_t* class = (my_GtkWindow2Class_t*)cl;
    bridgeGtkBin2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkWindow2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkWindow2Instance(my_GtkWindow2_t* class)
{
    unwrapGtkBin2Instance(&class->parent);
}
// autobridge
void bridgeGtkWindow2Instance(my_GtkWindow2_t* class)
{
    bridgeGtkBin2Instance(&class->parent);
}

// ----- GtkWindow3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkWindow3, set_focus, void, (void* window, void* focus), "pp", window, focus);
WRAPPER(GtkWindow3, activate_focus, void, (void* window), "p", window);
WRAPPER(GtkWindow3, activate_default, void, (void* window), "p", window);
WRAPPER(GtkWindow3, keys_changed, void, (void* window), "p", window);
WRAPPER(GtkWindow3, enable_debugging, int, (void* window, int toggle), "pi", window, toggle);

#define SUPERGO()               \
    GO(set_focus, vFpp);        \
    GO(activate_focus, vFp);    \
    GO(activate_default, vFp);  \
    GO(keys_changed, vFp);      \
    GO(enable_debugging, iFpi);


// wrap (so bridge all calls, just in case)
void wrapGtkWindow3Class(void* cl)
{
    my_GtkWindow3Class_t* class = (my_GtkWindow3Class_t*)cl;
    wrapGtkBin3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkWindow3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkWindow3Class(void* cl)
{
    my_GtkWindow3Class_t* class = (my_GtkWindow3Class_t*)cl;
    unwrapGtkBin3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkWindow3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkWindow3Class(void* cl)
{
    my_GtkWindow3Class_t* class = (my_GtkWindow3Class_t*)cl;
    bridgeGtkBin3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkWindow3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkWindow3Instance(my_GtkWindow3_t* class)
{
    unwrapGtkBin3Instance(&class->bin);
}
// autobridge
void bridgeGtkWindow3Instance(my_GtkWindow3_t* class)
{
    bridgeGtkBin3Instance(&class->bin);
}

// ----- GtkApplicationWindowClass ------
// wrap (so bridge all calls, just in case)
void wrapGtkApplicationWindowClass(void* cl)
{
    my_GtkApplicationWindowClass_t* class = (my_GtkApplicationWindowClass_t*)cl;
    wrapGtkWindow3Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkApplicationWindowClass(void* cl)
{
    my_GtkApplicationWindowClass_t* class = (my_GtkApplicationWindowClass_t*)cl;
    unwrapGtkWindow3Class(&class->parent_class);
}
// autobridge
void bridgeGtkApplicationWindowClass(void* cl)
{
    my_GtkApplicationWindowClass_t* class = (my_GtkApplicationWindowClass_t*)cl;
    bridgeGtkWindow3Class(&class->parent_class);
}

void unwrapGtkApplicationWindowInstance(my_GtkApplicationWindow_t* class)
{
    unwrapGtkWindow3Instance(&class->parent);
}
// autobridge
void bridgeGtkApplicationWindowInstance(my_GtkApplicationWindow_t* class)
{
    bridgeGtkWindow3Instance(&class->parent);
}
// ----- GtkListBoxClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkListBoxClass,row_selected, void, (void *box, void *row), "pp", box, row);
WRAPPER(GtkListBoxClass,row_activated, void, (void *box, void *row), "pp", box, row);
WRAPPER(GtkListBoxClass,activate_cursor_row, void, (void *box), "p", box);
WRAPPER(GtkListBoxClass,toggle_cursor_row, void, (void *box), "p", box);
WRAPPER(GtkListBoxClass,move_cursor, void, (void *box, int step, int count), "pii", box, step, count);
WRAPPER(GtkListBoxClass,selected_rows_changed, void, (void *box), "p", box);
WRAPPER(GtkListBoxClass,select_all, void, (void *box), "p", box);
WRAPPER(GtkListBoxClass,unselect_all, void, (void *box), "p", box);

#define SUPERGO()                  \
    GO(row_selected, vFpp);        \
    GO(row_activated, vFpp);    \
    GO(activate_cursor_row, vFp);    \
    GO(toggle_cursor_row, vFp);    \
    GO(move_cursor, vFpii);    \
    GO(selected_rows_changed, vFp);    \
    GO(select_all, vFp);    \
    GO(unselect_all, vFp);

// wrap (so bridge all calls, just in case)
void wrapGtkListBoxClass(void* cl)
{
    my_GtkListBoxClass_t* class = (my_GtkListBoxClass_t*)cl;
    wrapGtkContainer3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkListBoxClass (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkListBoxClass(void* cl)
{
    my_GtkListBoxClass_t* class = (my_GtkListBoxClass_t*)cl;
    unwrapGtkContainer3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkListBoxClass (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkListBoxClass(void* cl)
{
    my_GtkListBoxClass_t* class = (my_GtkListBoxClass_t*)cl;
    bridgeGtkContainer3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkListBoxClass (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkListBoxInstance(my_GtkListBox_t* class)
{
    unwrapGtkContainer3Instance(&class->parent);
}
// autobridge
void bridgeGtkListBoxInstance(my_GtkListBox_t* class)
{
    bridgeGtkContainer3Instance(&class->parent);
}

// ----- GtkListBoxRowClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkListBoxRowClass, activate, void, (void *row), "p", row);

#define SUPERGO()       \
    GO(activate, vFpp);

// wrap (so bridge all calls, just in case)
void wrapGtkListBoxRowClass(void* cl)
{
    my_GtkListBoxRowClass_t* class = (my_GtkListBoxRowClass_t*)cl;
    wrapGtkBin3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkListBoxRowClass (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkListBoxRowClass(void* cl)
{
    my_GtkListBoxRowClass_t* class = (my_GtkListBoxRowClass_t*)cl;
    unwrapGtkBin3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkListBoxRowClass (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkListBoxRowClass(void* cl)
{
    my_GtkListBoxRowClass_t* class = (my_GtkListBoxRowClass_t*)cl;
    bridgeGtkBin3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkListBoxRowClass (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkListBoxRowInstance(my_GtkListBoxRow_t* class)
{
    unwrapGtkBin3Instance(&class->parent);
}
// autobridge
void bridgeGtkListBoxRowInstance(my_GtkListBoxRow_t* class)
{
    bridgeGtkBin3Instance(&class->parent);
}

// ----- GtkTable2Class ------
// wrap (so bridge all calls, just in case)
void wrapGtkTable2Class(void* cl)
{
    my_GtkTable2Class_t* class = (my_GtkTable2Class_t*)cl;
    wrapGtkContainer2Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkTable2Class(void* cl)
{
    my_GtkTable2Class_t* class = (my_GtkTable2Class_t*)cl;
    unwrapGtkContainer2Class(&class->parent_class);
}
// autobridge
void bridgeGtkTable2Class(void* cl)
{
    my_GtkTable2Class_t* class = (my_GtkTable2Class_t*)cl;
    bridgeGtkContainer2Class(&class->parent_class);
}

void unwrapGtkTable2Instance(my_GtkTable2_t* class)
{
    unwrapGtkContainer2Instance(&class->container);
}
// autobridge
void bridgeGtkTable2Instance(my_GtkTable2_t* class)
{
    bridgeGtkContainer2Instance(&class->container);
}
// ----- GtkFixed2Class ------

// wrap (so bridge all calls, just in case)
void wrapGtkFixed2Class(void* cl)
{
    my_GtkFixed2Class_t* class = (my_GtkFixed2Class_t*)cl;
    wrapGtkContainer2Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkFixed2Class(void* cl)
{
    my_GtkFixed2Class_t* class = (my_GtkFixed2Class_t*)cl;
    unwrapGtkContainer2Class(&class->parent_class);
}
// autobridge
void bridgeGtkFixed2Class(void* cl)
{
    my_GtkFixed2Class_t* class = (my_GtkFixed2Class_t*)cl;
    bridgeGtkContainer2Class(&class->parent_class);
}

void unwrapGtkFixed2Instance(my_GtkFixed2_t* class)
{
    unwrapGtkContainer2Instance(&class->parent);
}
// autobridge
void bridgeGtkFixed2Instance(my_GtkFixed2_t* class)
{
    bridgeGtkContainer2Instance(&class->parent);
}

// ----- GtkFixed3Class ------

// wrap (so bridge all calls, just in case)
void wrapGtkFixed3Class(void* cl)
{
    my_GtkFixed3Class_t* class = (my_GtkFixed3Class_t*)cl;
    wrapGtkContainer3Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkFixed3Class(void* cl)
{
    my_GtkFixed3Class_t* class = (my_GtkFixed3Class_t*)cl;
    unwrapGtkContainer3Class(&class->parent_class);
}
// autobridge
void bridgeGtkFixed3Class(void* cl)
{
    my_GtkFixed3Class_t* class = (my_GtkFixed3Class_t*)cl;
    bridgeGtkContainer3Class(&class->parent_class);
}

void unwrapGtkFixed3Instance(my_GtkFixed3_t* class)
{
    unwrapGtkContainer3Instance(&class->parent);
}
// autobridge
void bridgeGtkFixed3Instance(my_GtkFixed3_t* class)
{
    bridgeGtkContainer3Instance(&class->parent);
}

// ----- MetaFrames2Class ------

// wrap (so bridge all calls, just in case)
void wrapMetaFrames2Class(void* cl)
{
    my_MetaFrames2Class_t* class = (my_MetaFrames2Class_t*)cl;
    wrapGtkWindow2Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
void unwrapMetaFrames2Class(void* cl)
{
    my_MetaFrames2Class_t* class = (my_MetaFrames2Class_t*)cl;
    unwrapGtkWindow2Class(&class->parent_class);
}
// autobridge
void bridgeMetaFrames2Class(void* cl)
{
    my_MetaFrames2Class_t* class = (my_MetaFrames2Class_t*)cl;
    bridgeGtkWindow2Class(&class->parent_class);
}

void unwrapMetaFrames2Instance(my_MetaFrames2_t* class)
{
    unwrapGtkWindow2Instance(&class->parent);
}
// autobridge
void bridgeMetaFrames2Instance(my_MetaFrames2_t* class)
{
    bridgeGtkWindow2Instance(&class->parent);
}
// ----- GtkNotebook2Class ------
WRAPPER(GtkNotebook2Class, switch_page, void, (void* notebook, void* page, uint32_t page_num), "ppp", notebook, page, page_num);
WRAPPER(GtkNotebook2Class, select_page, int, (void* notebook, int move_focus), "pi", notebook, move_focus);
WRAPPER(GtkNotebook2Class, focus_tab, int, (void* notebook, int type), "pi", notebook, type);
WRAPPER(GtkNotebook2Class, change_current_page, int, (void* notebook, int offset), "pi", notebook, offset);
WRAPPER(GtkNotebook2Class, move_focus_out,void , (void* notebook, int direction), "pi", notebook, direction);
WRAPPER(GtkNotebook2Class, reorder_tab, int, (void* notebook, int direction, int move_to_last), "pii", notebook, direction, move_to_last);
WRAPPER(GtkNotebook2Class, insert_page, int, (void* notebook, void* child, void* tab_label, void* menu_label, int position), "ppppi", notebook, child, tab_label, menu_label, position);
WRAPPER(GtkNotebook2Class, create_window, void*, (void* notebook, void* page, int x, int y), "ppii", notebook, page, x, y);

#define SUPERGO()                   \
    GO(switch_page, vFppp);         \
    GO(select_page, iFpi);          \
    GO(focus_tab, iFpi);            \
    GO(change_current_page, iFpi);  \
    GO(move_focus_out, vFpi);       \
    GO(reorder_tab, iFpii);         \
    GO(insert_page, iFppppi);       \
    GO(create_window, pFppii);      \

// wrap (so bridge all calls, just in case)
void wrapGtkNotebook2Class(void* cl)
{
    my_GtkNotebook2Class_t* class = (my_GtkNotebook2Class_t*)cl;
    wrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkNotebook2Class (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkNotebook2Class(void* cl)
{
    my_GtkNotebook2Class_t* class = (my_GtkNotebook2Class_t*)cl;
    unwrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkNotebook2Class (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkNotebook2Class(void* cl)
{
    my_GtkNotebook2Class_t* class = (my_GtkNotebook2Class_t*)cl;
    bridgeGtkContainer2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkNotebook2Class (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkNotebook2Instance(my_GtkNotebook2_t* class)
{
    unwrapGtkContainer2Instance(&class->parent);
}
// autobridge
void bridgeGtkNotebook2Instance(my_GtkNotebook2_t* class)
{
    bridgeGtkContainer2Instance(&class->parent);
}

// ----- GtkCellRenderer2Class ------
WRAPPER(GtkCellRenderer2Class, get_size, void, (void* cell, void* widget, void* cell_area, int* x_offset, int* y_offset, int* width, int* height), "ppppppp", cell, widget, cell_area, x_offset, y_offset, width, height);
WRAPPER(GtkCellRenderer2Class, render, void, (void* cell, void* window, void* widget, void* background_area, void* cell_area, void* expose_area, int flags), "ppppppi", cell, window, widget, background_area, cell_area, expose_area, flags);
WRAPPER(GtkCellRenderer2Class, activate, int, (void* cell, void* event, void* widget, void* path, void* background_area, void* cell_area, int flags), "ppppppi", cell, event, widget, path, background_area, cell_area, flags);
WRAPPER(GtkCellRenderer2Class, start_editing, void*, (void* cell, void* event, void* widget, void* path, void* background_area, void* cell_area, int flags), "ppppppi", cell, event, widget, path, background_area, cell_area, flags);
WRAPPER(GtkCellRenderer2Class, editing_canceled, void, (void* cell), "p", cell);
WRAPPER(GtkCellRenderer2Class, editing_started, void, (void* cell, void* editable, void* path), "ppp", cell, editable, path);

#define SUPERGO()                   \
    GO(get_size, vFppppppp);        \
    GO(render, vFppppppi);          \
    GO(activate, iFppppppi);        \
    GO(start_editing, pFppppppi);   \
    GO(editing_canceled, vFp);      \
    GO(editing_started, vFppp);     \

// wrap (so bridge all calls, just in case)
void wrapGtkCellRenderer2Class(void* cl)
{
    my_GtkCellRenderer2Class_t* class = (my_GtkCellRenderer2Class_t*)cl;
    wrapGtkObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkCellRenderer2Class (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkCellRenderer2Class(void* cl)
{
    my_GtkCellRenderer2Class_t* class = (my_GtkCellRenderer2Class_t*)cl;
    unwrapGtkObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkCellRenderer2Class (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkCellRenderer2Class(void* cl)
{
    my_GtkCellRenderer2Class_t* class = (my_GtkCellRenderer2Class_t*)cl;
    bridgeGtkObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkCellRenderer2Class (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkCellRenderer2Instance(my_GtkCellRenderer2_t* class)
{
    unwrapGtkObjectInstance(&class->parent);
}
// autobridge
void bridgeGtkCellRenderer2Instance(my_GtkCellRenderer2_t* class)
{
    bridgeGtkObjectInstance(&class->parent);
}

// ----- GtkCellRendererText2Class ------
WRAPPER(GtkCellRendererText2Class, edited, void, (void* cell_renderer_text, void* path, void* new_text), "ppp", cell_renderer_text, path, new_text);

#define SUPERGO()                   \
    GO(edited, vFppp);              \

// wrap (so bridge all calls, just in case)
void wrapGtkCellRendererText2Class(void* cl)
{
    my_GtkCellRendererText2Class_t* class = (my_GtkCellRendererText2Class_t*)cl;
    wrapGtkCellRenderer2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkCellRendererText2Class (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkCellRendererText2Class(void* cl)
{
    my_GtkCellRendererText2Class_t* class = (my_GtkCellRendererText2Class_t*)cl;
    unwrapGtkCellRenderer2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkCellRendererText2Class (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkCellRendererText2Class(void* cl)
{
    my_GtkCellRendererText2Class_t* class = (my_GtkCellRendererText2Class_t*)cl;
    bridgeGtkCellRenderer2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkCellRendererText2Class (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkCellRendererText2Instance(my_GtkCellRendererText2_t* class)
{
    unwrapGtkCellRenderer2Instance(&class->parent);
}
// autobridge
void bridgeGtkCellRendererText2Instance(my_GtkCellRendererText2_t* class)
{
    bridgeGtkCellRenderer2Instance(&class->parent);
}

// ----- GDBusObjectManagerClientClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GDBusObjectManagerClient,interface_proxy_signal, void, (void* manager, void* object_proxy, void* interface_proxy, void* sender_name, void* signal_name, void* parameters), "pppppp", manager, object_proxy, interface_proxy, sender_name, signal_name, parameters);
WRAPPER(GDBusObjectManagerClient,interface_proxy_properties_changed, void, (void* manager, void* object_proxy, void* interface_proxy, void* changed_properties, void* invalidated_properties), "ppppp", manager, object_proxy, interface_proxy, changed_properties, invalidated_properties);

#define SUPERGO()                                       \
    GO(interface_proxy_signal, vFpppppp);               \
    GO(interface_proxy_properties_changed, vFppppp);    \


// wrap (so bridge all calls, just in case)
void wrapGDBusObjectManagerClientClass(void* cl)
{
    my_GDBusObjectManagerClientClass_t* class = (my_GDBusObjectManagerClientClass_t*)cl;
    wrapGObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GDBusObjectManagerClient (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGDBusObjectManagerClientClass(void* cl)
{
    my_GDBusObjectManagerClientClass_t* class = (my_GDBusObjectManagerClientClass_t*)cl;
    unwrapGObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GDBusObjectManagerClient (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGDBusObjectManagerClientClass(void* cl)
{
    my_GDBusObjectManagerClientClass_t* class = (my_GDBusObjectManagerClientClass_t*)cl;
    bridgeGObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GDBusObjectManagerClient (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGDBusObjectManagerClientInstance(my_GDBusObjectManagerClient_t* class)
{
    unwrapGObjectInstance(&class->parent);
}
// autobridge
void bridgeGDBusObjectManagerClientInstance(my_GDBusObjectManagerClient_t* class)
{
    bridgeGObjectInstance(&class->parent);
}

// ----- GDBusInterfaceSkeletonClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GDBusInterfaceSkeleton,get_info, void*, (void* interface_), "p", interface_);
WRAPPER_RET(GDBusInterfaceSkeleton,get_vtable, my_GDBusInterfaceVTable_t*, findFreeGDBusInterfaceVTable,(void* interface_), "p", interface_);
WRAPPER(GDBusInterfaceSkeleton,get_properties, void*, (void* interface_), "p", interface_);
WRAPPER(GDBusInterfaceSkeleton,flush, void, (void* interface_), "p", interface_);
WRAPPER(GDBusInterfaceSkeleton,g_authorize_method, int, (void* interface_, void* invocation), "pp", interface_, invocation);

#define SUPERGO()                   \
    GO(get_info, pFp);              \
    GO(get_vtable, pFp);            \
    GO(get_properties, pFp);        \
    GO(flush, vFp);                 \
    GO(g_authorize_method, iFpp);   \


// wrap (so bridge all calls, just in case)
void wrapGDBusInterfaceSkeletonClass(void* cl)
{
    my_GDBusInterfaceSkeletonClass_t* class = (my_GDBusInterfaceSkeletonClass_t*)cl;
    wrapGObjectClass(&class->parent);
    #define GO(A, W) class->A = reverse_##A##_GDBusInterfaceSkeleton (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGDBusInterfaceSkeletonClass(void* cl)
{
    my_GDBusInterfaceSkeletonClass_t* class = (my_GDBusInterfaceSkeletonClass_t*)cl;
    unwrapGObjectClass(&class->parent);
    #define GO(A, W)   class->A = find_##A##_GDBusInterfaceSkeleton (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGDBusInterfaceSkeletonClass(void* cl)
{
    my_GDBusInterfaceSkeletonClass_t* class = (my_GDBusInterfaceSkeletonClass_t*)cl;
    bridgeGObjectClass(&class->parent);
    #define GO(A, W) autobridge_##A##_GDBusInterfaceSkeleton (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGDBusInterfaceSkeletonInstance(my_GDBusInterfaceSkeleton_t* class)
{
    unwrapGObjectInstance(&class->parent);
}
// autobridge
void bridgeGDBusInterfaceSkeletonInstance(my_GDBusInterfaceSkeleton_t* class)
{
    bridgeGObjectInstance(&class->parent);
}

// ----- GtkButton2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkButton2, pressed, void,  (void* button), "p", button);
WRAPPER(GtkButton2, released, void, (void* button), "p", button);
WRAPPER(GtkButton2, clicked, void,  (void* button), "p", button);
WRAPPER(GtkButton2, enter, void,    (void* button), "p", button);
WRAPPER(GtkButton2, leave, void,    (void* button), "p", button);
WRAPPER(GtkButton2, activate, void, (void* button), "p", button);

#define SUPERGO()               \
    GO(pressed, vFp);           \
    GO(released, vFp);          \
    GO(clicked, vFp);           \
    GO(enter, vFp);             \
    GO(leave, vFp);             \
    GO(activate, vFp);          \


// wrap (so bridge all calls, just in case)
void wrapGtkButton2Class(void* cl)
{
    my_GtkButton2Class_t* class = (my_GtkButton2Class_t*)cl;
    wrapGtkBin2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkButton2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkButton2Class(void* cl)
{
    my_GtkButton2Class_t* class = (my_GtkButton2Class_t*)cl;
    unwrapGtkBin2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkButton2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkButton2Class(void* cl)
{
    my_GtkButton2Class_t* class = (my_GtkButton2Class_t*)cl;
    bridgeGtkBin2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkButton2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkButton2Instance(my_GtkButton2_t* class)
{
    unwrapGtkBin2Instance(&class->bin);
}
// autobridge
void bridgeGtkButton2Instance(my_GtkButton2_t* class)
{
    bridgeGtkBin2Instance(&class->bin);
}

// ----- GtkButton3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkButton3, pressed, void,  (void* button), "p", button);
WRAPPER(GtkButton3, released, void, (void* button), "p", button);
WRAPPER(GtkButton3, clicked, void,  (void* button), "p", button);
WRAPPER(GtkButton3, enter, void,    (void* button), "p", button);
WRAPPER(GtkButton3, leave, void,    (void* button), "p", button);
WRAPPER(GtkButton3, activate, void, (void* button), "p", button);

#define SUPERGO()               \
    GO(pressed, vFp);           \
    GO(released, vFp);          \
    GO(clicked, vFp);           \
    GO(enter, vFp);             \
    GO(leave, vFp);             \
    GO(activate, vFp);          \


// wrap (so bridge all calls, just in case)
void wrapGtkButton3Class(void* cl)
{
    my_GtkButton3Class_t* class = (my_GtkButton3Class_t*)cl;
    wrapGtkBin3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkButton3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkButton3Class(void* cl)
{
    my_GtkButton3Class_t* class = (my_GtkButton3Class_t*)cl;
    unwrapGtkBin3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkButton3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkButton3Class(void* cl)
{
    my_GtkButton3Class_t* class = (my_GtkButton3Class_t*)cl;
    bridgeGtkBin3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkButton3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkButton3Instance(my_GtkButton3_t* class)
{
    unwrapGtkBin3Instance(&class->bin);
}
// autobridge
void bridgeGtkButton3Instance(my_GtkButton3_t* class)
{
    bridgeGtkBin3Instance(&class->bin);
}

// ----- GtkComboBox2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkComboBox2, changed, void, (void* combo_box), "p", combo_box);
WRAPPER(GtkComboBox2, get_active_text, void*, (void* combo_box), "p", combo_box);

#define SUPERGO()               \
    GO(changed, vFp);           \
    GO(get_active_text, pFp);   \


// wrap (so bridge all calls, just in case)
void wrapGtkComboBox2Class(void* cl)
{
    my_GtkComboBox2Class_t* class = (my_GtkComboBox2Class_t*)cl;
    wrapGtkBin2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkComboBox2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkComboBox2Class(void* cl)
{
    my_GtkComboBox2Class_t* class = (my_GtkComboBox2Class_t*)cl;
    unwrapGtkBin2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkComboBox2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkComboBox2Class(void* cl)
{
    my_GtkComboBox2Class_t* class = (my_GtkComboBox2Class_t*)cl;
    bridgeGtkBin2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkComboBox2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkComboBox2Instance(my_GtkComboBox2_t* class)
{
    unwrapGtkBin2Instance(&class->parent);
}
// autobridge
void bridgeGtkComboBox2Instance(my_GtkComboBox2_t* class)
{
    bridgeGtkBin2Instance(&class->parent);
}

// ----- GtkToggleButton2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkToggleButton2, toggled, void, (void* toggle_button), "p", toggle_button);

#define SUPERGO()               \
    GO(toggled, vFp);           \


// wrap (so bridge all calls, just in case)
void wrapGtkToggleButton2Class(void* cl)
{
    my_GtkToggleButton2Class_t* class = (my_GtkToggleButton2Class_t*)cl;
    wrapGtkButton2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkToggleButton2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkToggleButton2Class(void* cl)
{
    my_GtkToggleButton2Class_t* class = (my_GtkToggleButton2Class_t*)cl;
    unwrapGtkButton2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkToggleButton2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkToggleButton2Class(void* cl)
{
    my_GtkToggleButton2Class_t* class = (my_GtkToggleButton2Class_t*)cl;
    bridgeGtkButton2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkToggleButton2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkToggleButton2Instance(my_GtkToggleButton2_t* class)
{
    unwrapGtkButton2Instance(&class->button);
}
// autobridge
void bridgeGtkToggleButton2Instance(my_GtkToggleButton2_t* class)
{
    bridgeGtkButton2Instance(&class->button);
}

// ----- GtkToggleButton3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkToggleButton3, toggled, void, (void* toggle_button), "p", toggle_button);

#define SUPERGO()               \
    GO(toggled, vFp);           \


// wrap (so bridge all calls, just in case)
void wrapGtkToggleButton3Class(void* cl)
{
    my_GtkToggleButton3Class_t* class = (my_GtkToggleButton3Class_t*)cl;
    wrapGtkButton3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkToggleButton3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkToggleButton3Class(void* cl)
{
    my_GtkToggleButton3Class_t* class = (my_GtkToggleButton3Class_t*)cl;
    unwrapGtkButton3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkToggleButton3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkToggleButton3Class(void* cl)
{
    my_GtkToggleButton3Class_t* class = (my_GtkToggleButton3Class_t*)cl;
    bridgeGtkButton3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkToggleButton2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkToggleButton3Instance(my_GtkToggleButton3_t* class)
{
    unwrapGtkButton3Instance(&class->parent);
}
// autobridge
void bridgeGtkToggleButton3Instance(my_GtkToggleButton3_t* class)
{
    bridgeGtkButton3Instance(&class->parent);
}

// ----- GtkCheckButton2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkCheckButton2, draw_indicator, void, (void* check_button, void* area), "pp", check_button, area);

#define SUPERGO()               \
    GO(draw_indicator, vFpp);   \


// wrap (so bridge all calls, just in case)
void wrapGtkCheckButton2Class(void* cl)
{
    my_GtkCheckButton2Class_t* class = (my_GtkCheckButton2Class_t*)cl;
    wrapGtkToggleButton2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkCheckButton2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkCheckButton2Class(void* cl)
{
    my_GtkCheckButton2Class_t* class = (my_GtkCheckButton2Class_t*)cl;
    unwrapGtkToggleButton2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkCheckButton2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkCheckButton2Class(void* cl)
{
    my_GtkCheckButton2Class_t* class = (my_GtkCheckButton2Class_t*)cl;
    bridgeGtkToggleButton2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkCheckButton2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkCheckButton2Instance(my_GtkCheckButton2_t* class)
{
    unwrapGtkToggleButton2Instance(&class->parent);
}
// autobridge
void bridgeGtkCheckButton2Instance(my_GtkCheckButton2_t* class)
{
    bridgeGtkToggleButton2Instance(&class->parent);
}

// ----- GtkCheckButton3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkCheckButton3, draw_indicator, void, (void* check_button, void* area), "pp", check_button, area);

#define SUPERGO()               \
    GO(draw_indicator, vFpp);   \


// wrap (so bridge all calls, just in case)
void wrapGtkCheckButton3Class(void* cl)
{
    my_GtkCheckButton3Class_t* class = (my_GtkCheckButton3Class_t*)cl;
    wrapGtkToggleButton3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkCheckButton3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkCheckButton3Class(void* cl)
{
    my_GtkCheckButton3Class_t* class = (my_GtkCheckButton3Class_t*)cl;
    unwrapGtkToggleButton3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkCheckButton3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkCheckButton3Class(void* cl)
{
    my_GtkCheckButton3Class_t* class = (my_GtkCheckButton3Class_t*)cl;
    bridgeGtkToggleButton3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkCheckButton3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkCheckButton3Instance(my_GtkCheckButton3_t* class)
{
    unwrapGtkToggleButton3Instance(&class->parent);
}
// autobridge
void bridgeGtkCheckButton3Instance(my_GtkCheckButton3_t* class)
{
    bridgeGtkToggleButton3Instance(&class->parent);
}
// ----- GtkGtkMenuButton3Class ------
// wrapper x86 -> natives of callbacks

#define SUPERGO() \

// wrap (so bridge all calls, just in case)
void wrapGtkMenuButton3Class(void* cl)
{
    my_GtkMenuButton3Class_t* class = (my_GtkMenuButton3Class_t*)cl;
    wrapGtkToggleButton3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkMenuButton3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkMenuButton3Class(void* cl)
{
    my_GtkMenuButton3Class_t* class = (my_GtkMenuButton3Class_t*)cl;
    unwrapGtkToggleButton3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkMenuButton3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkMenuButton3Class(void* cl)
{
    my_GtkMenuButton3Class_t* class = (my_GtkMenuButton3Class_t*)cl;
    bridgeGtkToggleButton3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkMenuButton3 (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

void unwrapGtkMenuButton3Instance(my_GtkMenuButton3_t* class)
{
    unwrapGtkToggleButton3Instance(&class->parent);
}
// autobridge
void bridgeGtkMenuButton3Instance(my_GtkMenuButton3_t* class)
{
    bridgeGtkToggleButton3Instance(&class->parent);
}
// ----- GtkEntry2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkEntry2, populate_popup, void,     (void* entry, void* menu), "pp", entry, menu);
WRAPPER(GtkEntry2, activate, void,           (void* entry), "p", entry);
WRAPPER(GtkEntry2, move_cursor, void,        (void* entry, int step, int count, int extend_selection), "piii", entry, step, count, extend_selection);
WRAPPER(GtkEntry2, insert_at_cursor, void,   (void* entry, void* str), "pp", entry, str);
WRAPPER(GtkEntry2, delete_from_cursor, void, (void* entry, size_t type, int count), "pLi", entry, type, count);
WRAPPER(GtkEntry2, backspace, void,          (void* entry), "p", entry);
WRAPPER(GtkEntry2, cut_clipboard, void,      (void* entry), "p", entry);
WRAPPER(GtkEntry2, copy_clipboard, void,     (void* entry), "p", entry);
WRAPPER(GtkEntry2, paste_clipboard, void,    (void* entry), "p", entry);
WRAPPER(GtkEntry2, toggle_overwrite, void,   (void* entry), "p", entry);
WRAPPER(GtkEntry2, get_text_area_size, void, (void* entry, void* x, void* y, void* width, void* height), "ppppp", entry, x, y, width, height);

#define SUPERGO()                   \
    GO(populate_popup, vFpp);       \
    GO(activate, vFp);              \
    GO(move_cursor, vFpiii);        \
    GO(insert_at_cursor, vFp);      \
    GO(delete_from_cursor, vFpii);  \
    GO(backspace, vFp);             \
    GO(cut_clipboard, vFp);         \
    GO(copy_clipboard, vFp);        \
    GO(paste_clipboard, vFp);       \
    GO(toggle_overwrite, vFp);      \
    GO(get_text_area_size, vFppppp);\

// wrap (so bridge all calls, just in case)
void wrapGtkEntry2Class(void* cl)
{
    my_GtkEntry2Class_t* class = (my_GtkEntry2Class_t*)cl;
    wrapGtkWidget2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkEntry2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkEntry2Class(void* cl)
{
    my_GtkEntry2Class_t* class = (my_GtkEntry2Class_t*)cl;
    unwrapGtkWidget2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkEntry2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkEntry2Class(void* cl)
{
    my_GtkEntry2Class_t* class = (my_GtkEntry2Class_t*)cl;
    bridgeGtkWidget2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkEntry2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkEntry2Instance(my_GtkEntry2_t* class)
{
    unwrapGtkWidget2Instance(&class->parent);
}
// autobridge
void bridgeGtkEntry2Instance(my_GtkEntry2_t* class)
{
    bridgeGtkWidget2Instance(&class->parent);
}

// ----- GtkSpinButton2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkSpinButton2, input, int,  (void* spin_button, void* new_value), "pp", spin_button, new_value);
WRAPPER(GtkSpinButton2, output, int, (void* spin_button), "p", spin_button);
WRAPPER(GtkSpinButton2, value_changed, void, (void* spin_button), "p", spin_button);
WRAPPER(GtkSpinButton2, change_value, void, (void* spin_button, int scroll), "pi", spin_button, scroll);
WRAPPER(GtkSpinButton2, wrapped, void, (void* spin_button), "p", spin_button);

#define SUPERGO()           \
    GO(input, iFpp);        \
    GO(output, iFp);        \
    GO(value_changed, vFp); \
    GO(change_value, vFpi); \
    GO(wrapped, vFp);       \

// wrap (so bridge all calls, just in case)
void wrapGtkSpinButton2Class(void* cl)
{
    my_GtkSpinButton2Class_t* class = (my_GtkSpinButton2Class_t*)cl;
    wrapGtkEntry2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkSpinButton2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkSpinButton2Class(void* cl)
{
    my_GtkSpinButton2Class_t* class = (my_GtkSpinButton2Class_t*)cl;
    unwrapGtkEntry2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkSpinButton2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkSpinButton2Class(void* cl)
{
    my_GtkSpinButton2Class_t* class = (my_GtkSpinButton2Class_t*)cl;
    bridgeGtkEntry2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkSpinButton2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkSpinButton2Instance(my_GtkSpinButton2_t* class)
{
    unwrapGtkEntry2Instance(&class->entry);
}
// autobridge
void bridgeGtkSpinButton2Instance(my_GtkSpinButton2_t* class)
{
    bridgeGtkEntry2Instance(&class->entry);
}

// ----- GtkProgress2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkProgress2, paint, void,          (void* progress), "p", progress);
WRAPPER(GtkProgress2, update, void,         (void* progress), "p", progress);
WRAPPER(GtkProgress2, act_mode_enter, void, (void* progress), "p", progress);

#define SUPERGO()           \
    GO(paint, vFp);         \
    GO(update, vFp);        \
    GO(act_mode_enter, vFp);\

// wrap (so bridge all calls, just in case)
void wrapGtkProgress2Class(void* cl)
{
    my_GtkProgress2Class_t* class = (my_GtkProgress2Class_t*)cl;
    wrapGtkWidget2Class(&class->parent);
    #define GO(A, W) class->A = reverse_##A##_GtkProgress2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkProgress2Class(void* cl)
{
    my_GtkProgress2Class_t* class = (my_GtkProgress2Class_t*)cl;
    unwrapGtkWidget2Class(&class->parent);
    #define GO(A, W)   class->A = find_##A##_GtkProgress2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkProgress2Class(void* cl)
{
    my_GtkProgress2Class_t* class = (my_GtkProgress2Class_t*)cl;
    bridgeGtkWidget2Class(&class->parent);
    #define GO(A, W) autobridge_##A##_GtkProgress2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkProgress2Instance(my_GtkProgress2_t* class)
{
    unwrapGtkWidget2Instance(&class->widget);
}
// autobridge
void bridgeGtkProgress2Instance(my_GtkProgress2_t* class)
{
    bridgeGtkWidget2Instance(&class->widget);
}

// ----- GtkProgressBar2Class ------
// no wrapper x86 -> natives of callbacks

#define SUPERGO()           \

// wrap (so bridge all calls, just in case)
void wrapGtkProgressBar2Class(void* cl)
{
    my_GtkProgressBar2Class_t* class = (my_GtkProgressBar2Class_t*)cl;
    wrapGtkProgress2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkProgressBar2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkProgressBar2Class(void* cl)
{
    my_GtkProgressBar2Class_t* class = (my_GtkProgressBar2Class_t*)cl;
    unwrapGtkProgress2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkProgressBar2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkProgressBar2Class(void* cl)
{
    my_GtkProgressBar2Class_t* class = (my_GtkProgressBar2Class_t*)cl;
    bridgeGtkProgress2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkProgressBar2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkProgressBar2Instance(my_GtkProgressBar2_t* class)
{
    unwrapGtkProgress2Instance(&class->parent);
}
// autobridge
void bridgeGtkProgressBar2Instance(my_GtkProgressBar2_t* class)
{
    bridgeGtkProgress2Instance(&class->parent);
}

// ----- GtkFrame2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkFrame2, compute_child_allocation, void, (void* frame, void* allocation), "pp", frame, allocation);

#define SUPERGO()                       \
    GO(compute_child_allocation, vFpp); \

// wrap (so bridge all calls, just in case)
void wrapGtkFrame2Class(void* cl)
{
    my_GtkFrame2Class_t* class = (my_GtkFrame2Class_t*)cl;
    wrapGtkBin2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkFrame2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkFrame2Class(void* cl)
{
    my_GtkFrame2Class_t* class = (my_GtkFrame2Class_t*)cl;
    unwrapGtkBin2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkFrame2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkFrame2Class(void* cl)
{
    my_GtkFrame2Class_t* class = (my_GtkFrame2Class_t*)cl;
    bridgeGtkBin2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkFrame2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkFrame2Instance(my_GtkFrame2_t* class)
{
    unwrapGtkBin2Instance(&class->bin);
}
// autobridge
void bridgeGtkFrame2Instance(my_GtkFrame2_t* class)
{
    bridgeGtkBin2Instance(&class->bin);
}

// ----- GtkMenuShell2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkMenuShell2,deactivate, void,      (void* menu_shell), "p", menu_shell);
WRAPPER(GtkMenuShell2,selection_done, void,  (void* menu_shell), "p", menu_shell);
WRAPPER(GtkMenuShell2,move_current, void,    (void* menu_shell, int direction),  "pi", menu_shell, direction);
WRAPPER(GtkMenuShell2,activate_current, void,(void* menu_shell, int force_hide), "pi", menu_shell, force_hide);
WRAPPER(GtkMenuShell2,cancel, void,          (void* menu_shell), "p", menu_shell);
WRAPPER(GtkMenuShell2,select_item, void,     (void* menu_shell, void* menu_item), "pp", menu_shell, menu_item);
WRAPPER(GtkMenuShell2,insert, void,          (void* menu_shell, void* child, int position), "ppi", menu_shell, child, position);
WRAPPER(GtkMenuShell2,get_popup_delay, int,  (void* menu_shell), "p", menu_shell);
WRAPPER(GtkMenuShell2,move_selected, int,    (void* menu_shell, int distance), "pi", menu_shell, distance);

#define SUPERGO()               \
    GO(deactivate, vFp);        \
    GO(selection_done, vFp);    \
    GO(move_current, vFpi);     \
    GO(activate_current, vFpi); \
    GO(cancel, vFp);            \
    GO(select_item, vFpp);      \
    GO(insert, vFppi);          \
    GO(get_popup_delay, iFp);   \
    GO(move_selected, iFpi);    \

// wrap (so bridge all calls, just in case)
void wrapGtkMenuShell2Class(void* cl)
{
    my_GtkMenuShell2Class_t* class = (my_GtkMenuShell2Class_t*)cl;
    wrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkMenuShell2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkMenuShell2Class(void* cl)
{
    my_GtkMenuShell2Class_t* class = (my_GtkMenuShell2Class_t*)cl;
    unwrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkMenuShell2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkMenuShell2Class(void* cl)
{
    my_GtkMenuShell2Class_t* class = (my_GtkMenuShell2Class_t*)cl;
    bridgeGtkContainer2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkMenuShell2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkMenuShell2Instance(my_GtkMenuShell2_t* class)
{
    unwrapGtkContainer2Instance(&class->container);
}
// autobridge
void bridgeGtkMenuShell2Instance(my_GtkMenuShell2_t* class)
{
    bridgeGtkContainer2Instance(&class->container);
}

// ----- GtkMenuBar2Class ------
// no wrapper x86 -> natives of callbacks

#define SUPERGO()           \

// wrap (so bridge all calls, just in case)
void wrapGtkMenuBar2Class(void* cl)
{
    my_GtkMenuBar2Class_t* class = (my_GtkMenuBar2Class_t*)cl;
    wrapGtkMenuShell2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkMenuBar2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkMenuBar2Class(void* cl)
{
    my_GtkMenuBar2Class_t* class = (my_GtkMenuBar2Class_t*)cl;
    unwrapGtkMenuShell2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkMenuBar2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkMenuBar2Class(void* cl)
{
    my_GtkMenuBar2Class_t* class = (my_GtkMenuBar2Class_t*)cl;
    bridgeGtkMenuShell2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkMenuBar2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkMenuBar2Instance(my_GtkMenuBar2_t* class)
{
    unwrapGtkMenuShell2Instance(&class->parent);
}
// autobridge
void bridgeGtkMenuBar2Instance(my_GtkMenuBar2_t* class)
{
    bridgeGtkMenuShell2Instance(&class->parent);
}

// ----- GtkTextView2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkTextView2, set_scroll_adjustments, void,   (void* text_view, void* hadjustment, void* vadjustment), "ppp", text_view, hadjustment, vadjustment);
WRAPPER(GtkTextView2, populate_popup, void,           (void* text_view, void* menu), "pp", text_view, menu);
WRAPPER(GtkTextView2, move_cursor, void,              (void* text_view, int step, int count, int extend_selection), "piii", text_view, step, count, extend_selection);
WRAPPER(GtkTextView2, page_horizontally, void,        (void* text_view, int count, int extend_selection), "pii", text_view, count, extend_selection);
WRAPPER(GtkTextView2, set_anchor, void,               (void* text_view), "p", text_view);
WRAPPER(GtkTextView2, insert_at_cursor, void,         (void* text_view, void* str), "pp", text_view, str);
WRAPPER(GtkTextView2, delete_from_cursor, void,       (void* text_view, int type, int count), "pii", text_view, type, count);
WRAPPER(GtkTextView2, backspace, void,                (void* text_view), "p", text_view);
WRAPPER(GtkTextView2, cut_clipboard, void,            (void* text_view), "p", text_view);
WRAPPER(GtkTextView2, copy_clipboard, void,           (void* text_view), "p", text_view);
WRAPPER(GtkTextView2, paste_clipboard, void,          (void* text_view), "p", text_view);
WRAPPER(GtkTextView2, toggle_overwrite, void,         (void* text_view), "p", text_view);
WRAPPER(GtkTextView2, move_focus, void,               (void* text_view, int direction), "pi", text_view, direction);

#define SUPERGO()                       \
    GO(set_scroll_adjustments, vFppp);  \
    GO(populate_popup, vFpp);           \
    GO(move_cursor, vFpiii);            \
    GO(page_horizontally, vFpii);       \
    GO(set_anchor, vFp);                \
    GO(insert_at_cursor, vFpp);         \
    GO(delete_from_cursor, vFpii);      \
    GO(backspace, vFp);                 \
    GO(cut_clipboard, vFp);             \
    GO(copy_clipboard, vFp);            \
    GO(paste_clipboard, vFp);           \
    GO(toggle_overwrite, vFp);          \
    GO(move_focus, vFpi);               \

// wrap (so bridge all calls, just in case)
void wrapGtkTextView2Class(void* cl)
{
    my_GtkTextView2Class_t* class = (my_GtkTextView2Class_t*)cl;
    wrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkTextView2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkTextView2Class(void* cl)
{
    my_GtkTextView2Class_t* class = (my_GtkTextView2Class_t*)cl;
    unwrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkTextView2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkTextView2Class(void* cl)
{
    my_GtkTextView2Class_t* class = (my_GtkTextView2Class_t*)cl;
    bridgeGtkContainer2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkTextView2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkTextView2Instance(my_GtkTextView2_t* class)
{
    unwrapGtkContainer2Instance(&class->parent);
}
// autobridge
void bridgeGtkTextView2Instance(my_GtkTextView2_t* class)
{
    bridgeGtkContainer2Instance(&class->parent);
}

// ----- GtkTextView3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkTextView3, populate_popup, void,           (void* text_view, void* menu), "pp", text_view, menu);
WRAPPER(GtkTextView3, move_cursor, void,              (void* text_view, int step, int count, int extend_selection), "piii", text_view, step, count, extend_selection);
WRAPPER(GtkTextView3, set_anchor, void,               (void* text_view), "p", text_view);
WRAPPER(GtkTextView3, insert_at_cursor, void,         (void* text_view, void* str), "pp", text_view, str);
WRAPPER(GtkTextView3, delete_from_cursor, void,       (void* text_view, int type, int count), "pii", text_view, type, count);
WRAPPER(GtkTextView3, backspace, void,                (void* text_view), "p", text_view);
WRAPPER(GtkTextView3, cut_clipboard, void,            (void* text_view), "p", text_view);
WRAPPER(GtkTextView3, copy_clipboard, void,           (void* text_view), "p", text_view);
WRAPPER(GtkTextView3, paste_clipboard, void,          (void* text_view), "p", text_view);
WRAPPER(GtkTextView3, toggle_overwrite, void,         (void* text_view), "p", text_view);
WRAPPER(GtkTextView3, create_buffer, void*,           (void* text_view), "p", text_view);
WRAPPER(GtkTextView3, draw_layer, void,               (void* text_view, int layer, void* cr), "pip", text_view, layer, cr);
WRAPPER(GtkTextView3, extend_selection, int,          (void* text_view, int granularity, void* location, void* start, void* end), "pippp", text_view, granularity, location, start, end);
WRAPPER(GtkTextView3, insert_emoji, void,             (void* text_view), "p", text_view);

#define SUPERGO()                       \
    GO(populate_popup, vFpp);           \
    GO(move_cursor, vFpiii);            \
    GO(set_anchor, vFp);                \
    GO(insert_at_cursor, vFpp);         \
    GO(delete_from_cursor, vFpii);      \
    GO(backspace, vFp);                 \
    GO(cut_clipboard, vFp);             \
    GO(copy_clipboard, vFp);            \
    GO(paste_clipboard, vFp);           \
    GO(toggle_overwrite, vFp);          \
    GO(create_buffer, pFp);             \
    GO(draw_layer, vFpip);              \
    GO(extend_selection, iFpippp);      \
    GO(insert_emoji, vFp);              \

// wrap (so bridge all calls, just in case)
void wrapGtkTextView3Class(void* cl)
{
    my_GtkTextView3Class_t* class = (my_GtkTextView3Class_t*)cl;
    wrapGtkContainer3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkTextView3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkTextView3Class(void* cl)
{
    my_GtkTextView3Class_t* class = (my_GtkTextView3Class_t*)cl;
    unwrapGtkContainer3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkTextView3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkTextView3Class(void* cl)
{
    my_GtkTextView3Class_t* class = (my_GtkTextView3Class_t*)cl;
    bridgeGtkContainer3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkTextView3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkTextView3Instance(my_GtkTextView3_t* class)
{
    unwrapGtkContainer3Instance(&class->parent);
}
// autobridge
void bridgeGtkTextView3Instance(my_GtkTextView3_t* class)
{
    bridgeGtkContainer3Instance(&class->parent);
}

// ----- GtkGrid3Class ------
// no wrapper x86 -> natives of callbacks

#define SUPERGO()           \

// wrap (so bridge all calls, just in case)
void wrapGtkGrid3Class(void* cl)
{
    my_GtkGrid3Class_t* class = (my_GtkGrid3Class_t*)cl;
    wrapGtkContainer3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkGrid3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkGrid3Class(void* cl)
{
    my_GtkGrid3Class_t* class = (my_GtkGrid3Class_t*)cl;
    unwrapGtkContainer3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkGrid3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkGrid3Class(void* cl)
{
    my_GtkGrid3Class_t* class = (my_GtkGrid3Class_t*)cl;
    bridgeGtkContainer3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkGrid3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkGrid3Instance(my_GtkGrid3_t* class)
{
    unwrapGtkContainer3Instance(&class->parent);
}
// autobridge
void bridgeGtkGrid3Instance(my_GtkGrid3_t* class)
{
    bridgeGtkContainer3Instance(&class->parent);
}

// ----- GtkEventControllerClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkEventController, set_widget, void, (void* controller, void* widget), "pp", controller, widget);
WRAPPER(GtkEventController, unset_widget, void, (void* controller), "p", controller);
WRAPPER(GtkEventController, handle_event, int, (void *controller, void *event, double x, double y), "ppdd", controller, event, x, y);
WRAPPER(GtkEventController, reset, void, (void* controller), "p", controller);
WRAPPER(GtkEventController, handle_crossing, void, (void *controller, void *crossing, double x, double y), "ppdd", controller, crossing, x, y);
WRAPPER(GtkEventController, filter_event, void, (void *controller, void *event), "pp", controller, event);

#define SUPERGO()               \
    GO(set_widget, vFpp);       \
    GO(unset_widget, vFp);      \
    GO(handle_event, iFppdd);   \
    GO(reset, vFp);             \
    GO(handle_crossing, vFppdd);\
    GO(filter_event, vFpp);

// wrap (so bridge all calls, just in case)
void wrapGtkEventControllerClass(void* cl)
{
    my_GtkEventControllerClass_t* class = (my_GtkEventControllerClass_t*)cl;
    wrapGObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkEventController(W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkEventControllerClass(void* cl)
{
    my_GtkEventControllerClass_t* class = (my_GtkEventControllerClass_t*)cl;
    unwrapGObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkEventController (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkEventControllerClass(void* cl)
{
    my_GtkEventControllerClass_t* class = (my_GtkEventControllerClass_t*)cl;
    bridgeGObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkEventController (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkEventControllerInstance(my_GtkEventController_t* class)
{
    unwrapGObjectInstance(&class->parent);
}
// autobridge
void bridgeGtkEventControllerInstance(my_GtkEventController_t* class)
{
    bridgeGObjectInstance(&class->parent);
}

// ----- GtkGestureClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkGesture, check, void, (void* gesture), "p", gesture);
WRAPPER(GtkGesture, begin, void, (void *gesture, void *sequence), "pp", gesture, sequence);
WRAPPER(GtkGesture, update, void, (void *gesture, void *sequence), "pp", gesture, sequence);
WRAPPER(GtkGesture, end, void, (void *gesture, void *sequence), "pp", gesture, sequence);
WRAPPER(GtkGesture, cancel, void, (void *gesture, void *sequence), "pp", gesture, sequence);
WRAPPER(GtkGesture, sequence_state_changed, void, (void *gesture, void *sequence, int state), "ppi", gesture, sequence, state);

#define SUPERGO()              \
    GO(check, vFp);            \
    GO(begin, vFpp);           \
    GO(update, vFpp);          \
    GO(end, vFpp);             \
    GO(cancel, vFpp);          \
    GO(sequence_state_changed, vFppi);

// wrap (so bridge all calls, just in case)
void wrapGtkGestureClass(void* cl)
{
    my_GtkGestureClass_t* class = (my_GtkGestureClass_t*)cl;
    wrapGtkEventControllerClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkGesture(W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkGestureClass(void* cl)
{
    my_GtkGestureClass_t* class = (my_GtkGestureClass_t*)cl;
    unwrapGtkEventControllerClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkGesture (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkGestureClass(void* cl)
{
    my_GtkGestureClass_t* class = (my_GtkGestureClass_t*)cl;
    bridgeGtkEventControllerClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkGesture (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkGestureInstance(my_GtkGesture_t* class)
{
    unwrapGtkEventControllerInstance(&class->parent);
}
// autobridge
void bridgeGtkGestureInstance(my_GtkGesture_t* class)
{
    bridgeGtkEventControllerInstance(&class->parent);
}

// ----- GtkGestureSingleClass ------
// wrap (so bridge all calls, just in case)
void wrapGtkGestureSingleClass(void* cl)
{
    my_GtkGestureSingleClass_t* class = (my_GtkGestureSingleClass_t*)cl;
    wrapGtkGestureClass(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkGestureSingleClass(void* cl)
{
    my_GtkGestureSingleClass_t* class = (my_GtkGestureSingleClass_t*)cl;
    unwrapGtkGestureClass(&class->parent_class);
}
// autobridge
void bridgeGtkGestureSingleClass(void* cl)
{
    my_GtkGestureSingleClass_t* class = (my_GtkGestureSingleClass_t*)cl;
    bridgeGtkGestureClass(&class->parent_class);
}

void unwrapGtkGestureSingleInstance(my_GtkGestureSingle_t* class)
{
    unwrapGtkGestureInstance(&class->parent);
}
// autobridge
void bridgeGtkGestureSingleInstance(my_GtkGestureSingle_t* class)
{
    bridgeGtkGestureInstance(&class->parent);
}
// ----- GtkGestureClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkGestureLongPress, pressed, void, (void *gesture, double x, double y), "pdd", gesture, x, y);
WRAPPER(GtkGestureLongPress, cancelled, void, (void *cancelled), "p", cancelled);

#define SUPERGO()              \
    GO(pressed, vFpdd);          \
    GO(cancelled, vFp);

// wrap (so bridge all calls, just in case)
void wrapGtkGestureLongPressClass(void* cl)
{
    my_GtkGestureLongPressClass_t* class = (my_GtkGestureLongPressClass_t*)cl;
    wrapGtkGestureSingleClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkGestureLongPress(W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
void unwrapGtkGestureLongPressClass(void* cl)
{
    my_GtkGestureLongPressClass_t* class = (my_GtkGestureLongPressClass_t*)cl;
    unwrapGtkGestureSingleClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkGestureLongPress (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
void bridgeGtkGestureLongPressClass(void* cl)
{
    my_GtkGestureLongPressClass_t* class = (my_GtkGestureLongPressClass_t*)cl;
    bridgeGtkGestureSingleClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkGestureLongPress (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

void unwrapGtkGestureLongPressInstance(my_GtkGestureLongPress_t* class)
{
    unwrapGtkGestureSingleInstance(&class->parent);
}
// autobridge
void bridgeGtkGestureLongPressInstance(my_GtkGestureLongPress_t* class)
{
    bridgeGtkGestureSingleInstance(&class->parent);
}

// ----- AtkObjectClass ------
// wrapper x86 -> natives of callbacks

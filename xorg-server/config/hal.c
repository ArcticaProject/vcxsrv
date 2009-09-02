/*
 * Copyright © 2007 Daniel Stone
 * Copyright © 2007 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Author: Daniel Stone <daniel@fooishbar.org>
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <dbus/dbus.h>
#include <hal/libhal.h>
#include <string.h>
#include <sys/select.h>

#include "input.h"
#include "inputstr.h"
#include "hotplug.h"
#include "config-backends.h"
#include "os.h"


#define LIBHAL_PROP_KEY "input.x11_options."
#define LIBHAL_XKB_PROP_KEY "input.xkb."


struct config_hal_info {
    DBusConnection *system_bus;
    LibHalContext *hal_ctx;
};

/* Used for special handling of xkb options. */
struct xkb_options {
    char* layout;
    char* model;
    char* rules;
    char* variant;
    char* options;
};


static void
remove_device(DeviceIntPtr dev)
{
    /* this only gets called for devices that have already been added */
    LogMessage(X_INFO, "config/hal: removing device %s\n", dev->name);

    /* Call PIE here so we don't try to dereference a device that's
     * already been removed. */
    OsBlockSignals();
    ProcessInputEvents();
    DeleteInputDeviceRequest(dev);
    OsReleaseSignals();
}

static void
device_removed(LibHalContext *ctx, const char *udi)
{
    DeviceIntPtr dev, next;
    char *value;

    value = xalloc(strlen(udi) + 5); /* "hal:" + NULL */
    if (!value)
        return;
    sprintf(value, "hal:%s", udi);

    for (dev = inputInfo.devices; dev; dev = next) {
	next = dev->next;
        if (dev->config_info && strcmp(dev->config_info, value) == 0)
            remove_device(dev);
    }
    for (dev = inputInfo.off_devices; dev; dev = next) {
	next = dev->next;
        if (dev->config_info && strcmp(dev->config_info, value) == 0)
            remove_device(dev);
    }

    xfree(value);
}

static void
add_option(InputOption **options, const char *key, const char *value)
{
    if (!value || *value == '\0')
        return;

    for (; *options; options = &(*options)->next)
        ;
    *options = xcalloc(sizeof(**options), 1);
    if (!*options) /* Yeesh. */
        return;
    (*options)->key = xstrdup(key);
    (*options)->value = xstrdup(value);
    (*options)->next = NULL;
}

static char *
get_prop_string(LibHalContext *hal_ctx, const char *udi, const char *name)
{
    char *prop, *ret;

    prop = libhal_device_get_property_string(hal_ctx, udi, name, NULL);
    LogMessageVerb(X_INFO, 10, "config/hal: getting %s on %s returned %s\n", name, udi, prop ? prop : "(null)");
    if (prop) {
        ret = xstrdup(prop);
        libhal_free_string(prop);
    }
    else {
        return NULL;
    }

    return ret;
}

static char *
get_prop_string_array(LibHalContext *hal_ctx, const char *udi, const char *prop)
{
    char **props, *ret, *str;
    int i, len = 0;

    props = libhal_device_get_property_strlist(hal_ctx, udi, prop, NULL);
    if (props) {
        for (i = 0; props[i]; i++)
            len += strlen(props[i]);

        ret = xcalloc(sizeof(char), len + i); /* i - 1 commas, 1 NULL */
        if (!ret) {
            libhal_free_string_array(props);
            return NULL;
        }

        str = ret;
        for (i = 0; props[i]; i++) {
            strcpy(str, props[i]);
            str += strlen(props[i]);
            *str++ = ',';
        }
        *(str-1) = '\0';

        libhal_free_string_array(props);
    }
    else {
        return NULL;
    }

    return ret;
}

static BOOL
device_is_duplicate(char *config_info)
{
    DeviceIntPtr dev;

    for (dev = inputInfo.devices; dev; dev = dev->next)
    {
        if (dev->config_info && (strcmp(dev->config_info, config_info) == 0))
            return TRUE;
    }

    for (dev = inputInfo.off_devices; dev; dev = dev->next)
    {
        if (dev->config_info && (strcmp(dev->config_info, config_info) == 0))
            return TRUE;
    }

    return FALSE;
}

static void
device_added(LibHalContext *hal_ctx, const char *udi)
{
    char *path = NULL, *driver = NULL, *name = NULL, *config_info = NULL;
    InputOption *options = NULL, *tmpo = NULL;
    DeviceIntPtr dev = NULL;
    DBusError error;
    struct xkb_options xkb_opts = {0};
    int rc;

    LibHalPropertySet *set = NULL;
	LibHalPropertySetIterator set_iter;
    char *psi_key = NULL, *tmp_val;


    dbus_error_init(&error);

    driver = get_prop_string(hal_ctx, udi, "input.x11_driver");
    if (!driver){
        /* verbose, don't tell the user unless they _want_ to see it */
        LogMessageVerb(X_INFO,7,"config/hal: no driver specified for device %s\n", udi);
        goto unwind;
    }

    path = get_prop_string(hal_ctx, udi, "input.device");
    if (!path) {
        LogMessage(X_WARNING,"config/hal: no driver or path specified for %s\n", udi);
        goto unwind;
    }

    name = get_prop_string(hal_ctx, udi, "info.product");
    if (!name)
        name = xstrdup("(unnamed)");

    options = xcalloc(sizeof(*options), 1);
    if (!options){
        LogMessage(X_ERROR, "config/hal: couldn't allocate space for input options!\n");
        goto unwind;
    }

    options->key = xstrdup("_source");
    options->value = xstrdup("server/hal");
    if (!options->key || !options->value) {
        LogMessage(X_ERROR, "config/hal: couldn't allocate first key/value pair\n");
        goto unwind;
    }

    /* most drivers use device.. not path. evdev uses both however, but the
     * path version isn't documented apparently. support both for now. */
    add_option(&options, "path", path);
    add_option(&options, "device", path);

    add_option(&options, "driver", driver);
    add_option(&options, "name", name);

    config_info = xalloc(strlen(udi) + 5); /* "hal:" and NULL */
    if (!config_info) {
        LogMessage(X_ERROR, "config/hal: couldn't allocate name\n");
        goto unwind;
    }
    sprintf(config_info, "hal:%s", udi);

    /* Check for duplicate devices */
    if (device_is_duplicate(config_info))
    {
        LogMessage(X_WARNING, "config/hal: device %s already added. Ignoring.\n", name);
        goto unwind;
    }

    /* ok, grab options from hal.. iterate through all properties
    * and lets see if any of them are options that we can add */
    set = libhal_device_get_all_properties(hal_ctx, udi, &error);

    if (!set) {
        LogMessage(X_ERROR, "config/hal: couldn't get property list for %s: %s (%s)\n",
               udi, error.name, error.message);
        goto unwind;
    }

    libhal_psi_init(&set_iter,set);
    while (libhal_psi_has_more(&set_iter)) {
        /* we are looking for supported keys.. extract and add to options */
        psi_key = libhal_psi_get_key(&set_iter);

        if (psi_key){

            /* normal options first (input.x11_options.<propname>) */
            if (!strncasecmp(psi_key, LIBHAL_PROP_KEY, sizeof(LIBHAL_PROP_KEY)-1)){
                char* tmp;

                /* only support strings for all values */
                tmp_val = get_prop_string(hal_ctx, udi, psi_key);

                if (tmp_val){

                    /* xkb needs special handling. HAL specs include
                     * input.xkb.xyz options, but the x11-input.fdi specifies
                     * input.x11_options.Xkbxyz options. By default, we use
                     * the former, unless the specific X11 ones are specified.
                     * Since we can't predict the order in which the keys
                     * arrive, we need to store them.
                     */
                    if ((tmp = strcasestr(psi_key, "xkb")) && strlen(tmp) >= 4)
                    {
                        if (!strcasecmp(&tmp[3], "layout"))
                        {
                            if (xkb_opts.layout)
                                xfree(xkb_opts.layout);
                            xkb_opts.layout = strdup(tmp_val);
                        } else if (!strcasecmp(&tmp[3], "model"))
                        {
                            if (xkb_opts.model)
                                xfree(xkb_opts.model);
                            xkb_opts.model = strdup(tmp_val);
                        } else if (!strcasecmp(&tmp[3], "rules"))
                        {
                            if (xkb_opts.rules)
                                xfree(xkb_opts.rules);
                            xkb_opts.rules = strdup(tmp_val);
                        } else if (!strcasecmp(&tmp[3], "variant"))
                        {
                            if (xkb_opts.variant)
                                xfree(xkb_opts.variant);
                            xkb_opts.variant = strdup(tmp_val);
                        } else if (!strcasecmp(&tmp[3], "options"))
                        {
                            if (xkb_opts.options)
                                xfree(xkb_opts.options);
                            xkb_opts.options = strdup(tmp_val);
                        }
                    } else
                    {
                        /* all others */
                        add_option(&options, psi_key + sizeof(LIBHAL_PROP_KEY)-1, tmp_val);
                        xfree(tmp_val);
                    }
                } else
                {
                    /* server 1.4 had xkb_options as strlist. */
                    if ((tmp = strcasestr(psi_key, "xkb")) &&
                        (strlen(tmp) >= 4) &&
                        (!strcasecmp(&tmp[3], "options")) &&
                        (tmp_val = get_prop_string_array(hal_ctx, udi, psi_key)))
                    {
                        if (xkb_opts.options)
                            xfree(xkb_opts.options);
                        xkb_opts.options = strdup(tmp_val);
                    }
                }
            } else if (!strncasecmp(psi_key, LIBHAL_XKB_PROP_KEY, sizeof(LIBHAL_XKB_PROP_KEY)-1)){
                char* tmp;

                /* only support strings for all values */
                tmp_val = get_prop_string(hal_ctx, udi, psi_key);

                if (tmp_val && strlen(psi_key) >= sizeof(LIBHAL_XKB_PROP_KEY)) {

                    tmp = &psi_key[sizeof(LIBHAL_XKB_PROP_KEY) - 1];

                    if (!strcasecmp(tmp, "layout"))
                    {
                        if (!xkb_opts.layout)
                            xkb_opts.layout = strdup(tmp_val);
                    } else if (!strcasecmp(tmp, "rules"))
                    {
                        if (!xkb_opts.rules)
                            xkb_opts.rules = strdup(tmp_val);
                    } else if (!strcasecmp(tmp, "variant"))
                    {
                        if (!xkb_opts.variant)
                            xkb_opts.variant = strdup(tmp_val);
                    } else if (!strcasecmp(tmp, "model"))
                    {
                        if (!xkb_opts.model)
                            xkb_opts.model = strdup(tmp_val);
                    } else if (!strcasecmp(tmp, "options"))
                    {
                        if (!xkb_opts.options)
                            xkb_opts.options = strdup(tmp_val);
                    }
                    xfree(tmp_val);
                } else
                {
                    /* server 1.4 had xkb options as strlist */
                    tmp_val = get_prop_string_array(hal_ctx, udi, psi_key);
                    if (tmp_val && strlen(psi_key) >= sizeof(LIBHAL_XKB_PROP_KEY))
                    {
                        tmp = &psi_key[sizeof(LIBHAL_XKB_PROP_KEY) - 1];
                        if (!strcasecmp(tmp, ".options") && (!xkb_opts.options))
                            xkb_opts.options = strdup(tmp_val);
                    }
                }
            }
        }

        /* psi_key doesn't need to be freed */
        libhal_psi_next(&set_iter);
    }


    /* Now add xkb options */
    if (xkb_opts.layout)
        add_option(&options, "xkb_layout", xkb_opts.layout);
    if (xkb_opts.rules)
        add_option(&options, "xkb_rules", xkb_opts.rules);
    if (xkb_opts.variant)
        add_option(&options, "xkb_variant", xkb_opts.variant);
    if (xkb_opts.model)
        add_option(&options, "xkb_model", xkb_opts.model);
    if (xkb_opts.options)
        add_option(&options, "xkb_options", xkb_opts.options);

    /* this isn't an error, but how else do you output something that the user can see? */
    LogMessage(X_INFO, "config/hal: Adding input device %s\n", name);
    if ((rc = NewInputDeviceRequest(options, &dev)) != Success) {
        LogMessage(X_ERROR, "config/hal: NewInputDeviceRequest failed (%d)\n", rc);
        dev = NULL;
        goto unwind;
    }

    for (; dev; dev = dev->next){
        if (dev->config_info)
            xfree(dev->config_info);
        dev->config_info = xstrdup(config_info);
    }

unwind:
    if (set)
        libhal_free_property_set(set);
    if (path)
        xfree(path);
    if (driver)
        xfree(driver);
    if (name)
        xfree(name);
    if (config_info)
        xfree(config_info);
    while (!dev && (tmpo = options)) {
        options = tmpo->next;
        xfree(tmpo->key);
        xfree(tmpo->value);
        xfree(tmpo);
    }

    if (xkb_opts.layout)
        xfree(xkb_opts.layout);
    if (xkb_opts.rules)
        xfree(xkb_opts.rules);
    if (xkb_opts.model)
        xfree(xkb_opts.model);
    if (xkb_opts.variant)
        xfree(xkb_opts.variant);
    if (xkb_opts.options)
        xfree(xkb_opts.options);

    dbus_error_free(&error);

    return;
}

static void
disconnect_hook(void *data)
{
    DBusError error;
    struct config_hal_info *info = data;

    if (info->hal_ctx) {
        if (dbus_connection_get_is_connected(info->system_bus)) {
            dbus_error_init(&error);
            if (!libhal_ctx_shutdown(info->hal_ctx, &error))
                LogMessage(X_WARNING, "config/hal: disconnect_hook couldn't shut down context: %s (%s)\n",
                        error.name, error.message);
            dbus_error_free(&error);
        }
        libhal_ctx_free(info->hal_ctx);
    }

    info->hal_ctx = NULL;
    info->system_bus = NULL;
}

static BOOL
connect_and_register(DBusConnection *connection, struct config_hal_info *info)
{
    DBusError error;
    char **devices;
    int num_devices, i;

    if (info->hal_ctx)
        return TRUE; /* already registered, pretend we did something */

    info->system_bus = connection;

    dbus_error_init(&error);

    info->hal_ctx = libhal_ctx_new();
    if (!info->hal_ctx) {
        LogMessage(X_ERROR, "config/hal: couldn't create HAL context\n");
        goto out_err;
    }

    if (!libhal_ctx_set_dbus_connection(info->hal_ctx, info->system_bus)) {
        LogMessage(X_ERROR, "config/hal: couldn't associate HAL context with bus\n");
        goto out_err;
    }
    if (!libhal_ctx_init(info->hal_ctx, &error)) {
        LogMessage(X_ERROR, "config/hal: couldn't initialise context: %s (%s)\n",
		   error.name ? error.name : "unknown error",
		   error.message ? error.message : "null");
        goto out_err;
    }
    if (!libhal_device_property_watch_all(info->hal_ctx, &error)) {
        LogMessage(X_ERROR, "config/hal: couldn't watch all properties: %s (%s)\n",
		   error.name ? error.name : "unknown error",
		   error.message ? error.message : "null");
        goto out_ctx;
    }
    libhal_ctx_set_device_added(info->hal_ctx, device_added);
    libhal_ctx_set_device_removed(info->hal_ctx, device_removed);

    devices = libhal_find_device_by_capability(info->hal_ctx, "input",
                                               &num_devices, &error);
    /* FIXME: Get default devices if error is set. */
    if (dbus_error_is_set(&error)) {
        LogMessage(X_ERROR, "config/hal: couldn't find input device: %s (%s)\n",
		   error.name ? error.name : "unknown error",
		   error.message ? error.message : "null");
        goto out_ctx;
    }
    for (i = 0; i < num_devices; i++)
        device_added(info->hal_ctx, devices[i]);
    libhal_free_string_array(devices);

    dbus_error_free(&error);

    return TRUE;

out_ctx:
    dbus_error_free(&error);

    if (!libhal_ctx_shutdown(info->hal_ctx, &error)) {
        LogMessage(X_WARNING, "config/hal: couldn't shut down context: %s (%s)\n",
                error.name ? error.name : "unknown error",
                error.message ? error.message : "null");
        dbus_error_free(&error);
    }

out_err:
    dbus_error_free(&error);

    if (info->hal_ctx) {
        libhal_ctx_free(info->hal_ctx);
    }

    info->hal_ctx = NULL;
    info->system_bus = NULL;

    return FALSE;
}


/**
 * Handle NewOwnerChanged signals to deal with HAL startup at X server runtime.
 *
 * NewOwnerChanged is send once when HAL shuts down, and once again when it
 * comes back up. Message has three arguments, first is the name
 * (org.freedesktop.Hal), the second one is the old owner, third one is new
 * owner.
 */
static DBusHandlerResult
ownerchanged_handler(DBusConnection *connection, DBusMessage *message, void *data)
{
    int ret = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

    if (dbus_message_is_signal(message,
                               "org.freedesktop.DBus",
                               "NameOwnerChanged")) {
        DBusError error;
        char *name, *old_owner, *new_owner;

        dbus_error_init(&error);
        dbus_message_get_args(message, &error,
                              DBUS_TYPE_STRING, &name,
                              DBUS_TYPE_STRING, &old_owner,
                              DBUS_TYPE_STRING, &new_owner,
                              DBUS_TYPE_INVALID);

        if (dbus_error_is_set(&error)) {
            ErrorF("[config/hal] failed to get NameOwnerChanged args: %s (%s)\n",
                   error.name, error.message);
        } else if (name && strcmp(name, "org.freedesktop.Hal") == 0) {

            if (!old_owner || !strlen(old_owner)) {
                DebugF("[config/hal] HAL startup detected.\n");
                if (connect_and_register(connection, (struct config_hal_info*)data))
                    dbus_connection_unregister_object_path(connection,
                                                     "/org/freedesktop/DBus");
                else
                    ErrorF("[config/hal] Failed to connect to HAL bus.\n");
            }

            ret = DBUS_HANDLER_RESULT_HANDLED;
        }
        dbus_error_free(&error);
    }

    return ret;
}

/**
 * Register a handler for the NameOwnerChanged signal.
 */
static BOOL
listen_for_startup(DBusConnection *connection, void *data)
{
    DBusObjectPathVTable vtable = { .message_function = ownerchanged_handler, };
    DBusError error;
    const char MATCH_RULE[] = "sender='org.freedesktop.DBus',"
                              "interface='org.freedesktop.DBus',"
                              "type='signal',"
                              "path='/org/freedesktop/DBus',"
                              "member='NameOwnerChanged'";
    int rc = FALSE;

    dbus_error_init(&error);
    dbus_bus_add_match(connection, MATCH_RULE, &error);
    if (!dbus_error_is_set(&error)) {
        if (dbus_connection_register_object_path(connection,
                                                  "/org/freedesktop/DBus",
                                                  &vtable,
                                                  data))
            rc = TRUE;
        else
            ErrorF("[config/hal] cannot register object path.\n");
    } else {
        ErrorF("[config/hal] couldn't add match rule: %s (%s)\n", error.name,
                error.message);
        ErrorF("[config/hal] cannot detect a HAL startup.\n");
    }

    dbus_error_free(&error);

    return rc;
}

static void
connect_hook(DBusConnection *connection, void *data)
{
    struct config_hal_info *info = data;

    if (listen_for_startup(connection, data) &&
        connect_and_register(connection, info))
        dbus_connection_unregister_object_path(connection,
                                               "/org/freedesktop/DBus");

    return;
}

static struct config_hal_info hal_info;
static struct config_dbus_core_hook hook = {
    .connect = connect_hook,
    .disconnect = disconnect_hook,
    .data = &hal_info,
};

int
config_hal_init(void)
{
    memset(&hal_info, 0, sizeof(hal_info));
    hal_info.system_bus = NULL;
    hal_info.hal_ctx = NULL;

    if (!config_dbus_core_add_hook(&hook)) {
        LogMessage(X_ERROR, "config/hal: failed to add D-Bus hook\n");
        return 0;
    }

    /* verbose message */
    LogMessageVerb(X_INFO,7,"config/hal: initialized");

    return 1;
}

void
config_hal_fini(void)
{
    config_dbus_core_remove_hook(&hook);
}

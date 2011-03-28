/*
 * Copyright © 2009 Julien Cristau
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
 * Author: Julien Cristau <jcristau@debian.org>
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <libudev.h>
#include <ctype.h>

#include "input.h"
#include "inputstr.h"
#include "hotplug.h"
#include "config-backends.h"
#include "os.h"

#define UDEV_XKB_PROP_KEY "xkb"

#define LOG_PROPERTY(path, prop, val)                                   \
    LogMessageVerb(X_INFO, 10,                                          \
                   "config/udev: getting property %s on %s "            \
                   "returned \"%s\"\n",                                 \
                   (prop), (path), (val) ? (val) : "(null)")
#define LOG_SYSATTR(path, attr, val)                                    \
    LogMessageVerb(X_INFO, 10,                                          \
                   "config/udev: getting attribute %s on %s "           \
                   "returned \"%s\"\n",                                 \
                   (attr), (path), (val) ? (val) : "(null)")

static struct udev_monitor *udev_monitor;

static void
device_added(struct udev_device *udev_device)
{
    const char *path, *name = NULL;
    char *config_info = NULL;
    const char *syspath;
    const char *tags_prop;
    const char *key, *value, *tmp;
    InputOption *options = NULL, *tmpo;
    InputAttributes attrs = {};
    DeviceIntPtr dev = NULL;
    struct udev_list_entry *set, *entry;
    struct udev_device *parent;
    int rc;

    path = udev_device_get_devnode(udev_device);

    syspath = udev_device_get_syspath(udev_device);

    if (!path || !syspath)
        return;

    if (!udev_device_get_property_value(udev_device, "ID_INPUT")) {
        LogMessageVerb(X_INFO, 10,
                       "config/udev: ignoring device %s without "
                       "property ID_INPUT set\n",
                       path);
        return;
    }

    options = calloc(sizeof(*options), 1);
    if (!options)
        return;

    options->key = strdup("_source");
    options->value = strdup("server/udev");
    if (!options->key || !options->value)
        goto unwind;

    parent = udev_device_get_parent(udev_device);
    if (parent) {
        const char *ppath = udev_device_get_devnode(parent);
        const char *product = udev_device_get_property_value(parent, "PRODUCT");
        const char *pnp_id = udev_device_get_sysattr_value(parent, "id");
        unsigned int usb_vendor, usb_model;

        name = udev_device_get_sysattr_value(parent, "name");
        LOG_SYSATTR(ppath, "name", name);
        if (!name) {
            name = udev_device_get_property_value(parent, "NAME");
            LOG_PROPERTY(ppath, "NAME", name);
        }

        if (pnp_id)
            attrs.pnp_id = strdup(pnp_id);
        LOG_SYSATTR(ppath, "id", pnp_id);

        /* construct USB ID in lowercase hex - "0000:ffff" */
        if (product && sscanf(product, "%*x/%4x/%4x/%*x", &usb_vendor, &usb_model) == 2) {
            if (asprintf(&attrs.usb_id, "%04x:%04x", usb_vendor, usb_model)
                == -1)
                attrs.usb_id = NULL;
            else
                LOG_PROPERTY(path, "PRODUCT", product);
        }
    }
    if (!name)
        name = "(unnamed)";
    else
        attrs.product = strdup(name);
    add_option(&options, "name", name);

    add_option(&options, "path", path);
    add_option(&options, "device", path);
    if (path)
        attrs.device = strdup(path);

    tags_prop = udev_device_get_property_value(udev_device, "ID_INPUT.tags");
    LOG_PROPERTY(path, "ID_INPUT.tags", tags_prop);
    attrs.tags = xstrtokenize(tags_prop, ",");

    if (asprintf(&config_info, "udev:%s", syspath) == -1) {
        config_info = NULL;
        goto unwind;
    }

    if (device_is_duplicate(config_info)) {
        LogMessage(X_WARNING, "config/udev: device %s already added. "
                              "Ignoring.\n", name);
        goto unwind;
    }

    set = udev_device_get_properties_list_entry(udev_device);
    udev_list_entry_foreach(entry, set) {
        key = udev_list_entry_get_name(entry);
        if (!key)
            continue;
        value = udev_list_entry_get_value(entry);
        if (!strncasecmp(key, UDEV_XKB_PROP_KEY,
                                sizeof(UDEV_XKB_PROP_KEY) - 1)) {
            LOG_PROPERTY(path, key, value);
            tmp = key + sizeof(UDEV_XKB_PROP_KEY) - 1;
            if (!strcasecmp(tmp, "rules"))
                add_option(&options, "xkb_rules", value);
            else if (!strcasecmp(tmp, "layout"))
                add_option(&options, "xkb_layout", value);
            else if (!strcasecmp(tmp, "variant"))
                add_option(&options, "xkb_variant", value);
            else if (!strcasecmp(tmp, "model"))
                add_option(&options, "xkb_model", value);
            else if (!strcasecmp(tmp, "options"))
                add_option(&options, "xkb_options", value);
        } else if (!strcmp(key, "ID_VENDOR")) {
            LOG_PROPERTY(path, key, value);
            attrs.vendor = strdup(value);
        } else if (!strcmp(key, "ID_INPUT_KEY")) {
            LOG_PROPERTY(path, key, value);
            attrs.flags |= ATTR_KEYBOARD;
        } else if (!strcmp(key, "ID_INPUT_MOUSE")) {
            LOG_PROPERTY(path, key, value);
            attrs.flags |= ATTR_POINTER;
        } else if (!strcmp(key, "ID_INPUT_JOYSTICK")) {
            LOG_PROPERTY(path, key, value);
            attrs.flags |= ATTR_JOYSTICK;
        } else if (!strcmp(key, "ID_INPUT_TABLET")) {
            LOG_PROPERTY(path, key, value);
            attrs.flags |= ATTR_TABLET;
        } else if (!strcmp(key, "ID_INPUT_TOUCHPAD")) {
            LOG_PROPERTY(path, key, value);
            attrs.flags |= ATTR_TOUCHPAD;
        } else if (!strcmp(key, "ID_INPUT_TOUCHSCREEN")) {
            LOG_PROPERTY(path, key, value);
            attrs.flags |= ATTR_TOUCHSCREEN;
        }
    }

    add_option(&options, "config_info", config_info);

    LogMessage(X_INFO, "config/udev: Adding input device %s (%s)\n",
               name, path);
    rc = NewInputDeviceRequest(options, &attrs, &dev);
    if (rc != Success)
        goto unwind;

 unwind:
    free(config_info);
    while ((tmpo = options)) {
        options = tmpo->next;
        free(tmpo->key);        /* NULL if dev != NULL */
        free(tmpo->value);      /* NULL if dev != NULL */
        free(tmpo);
    }

    free(attrs.usb_id);
    free(attrs.pnp_id);
    free(attrs.product);
    free(attrs.device);
    free(attrs.vendor);
    if (attrs.tags) {
        char **tag = attrs.tags;
        while (*tag) {
            free(*tag);
            tag++;
        }
        free(attrs.tags);
    }

    return;
}

static void
device_removed(struct udev_device *device)
{
    char *value;
    const char *syspath = udev_device_get_syspath(device);

    if (asprintf(&value, "udev:%s", syspath) == -1)
        return;

    remove_devices("udev", value);

    free(value);
}

static void
wakeup_handler(pointer data, int err, pointer read_mask)
{
    int udev_fd = udev_monitor_get_fd(udev_monitor);
    struct udev_device *udev_device;
    const char *action;

    if (err < 0)
        return;

    if (FD_ISSET(udev_fd, (fd_set *)read_mask)) {
        udev_device = udev_monitor_receive_device(udev_monitor);
        if (!udev_device)
            return;
        action = udev_device_get_action(udev_device);
        if (action) {
            if (!strcmp(action, "add"))
                device_added(udev_device);
            else if (!strcmp(action, "remove"))
                device_removed(udev_device);
            else if (!strcmp(action, "change")) {
                device_removed(udev_device);
                device_added(udev_device);
            }
        }
        udev_device_unref(udev_device);
    }
}

static void
block_handler(pointer data, struct timeval **tv, pointer read_mask)
{
}

int
config_udev_init(void)
{
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *device;

    udev = udev_new();
    if (!udev)
        return 0;
    udev_monitor = udev_monitor_new_from_netlink(udev, "udev");
    if (!udev_monitor)
        return 0;

    if (udev_monitor_enable_receiving(udev_monitor)) {
        ErrorF("config/udev: failed to bind the udev monitor\n");
        return 0;
    }

    enumerate = udev_enumerate_new(udev);
    if (!enumerate)
        return 0;
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);
    udev_list_entry_foreach(device, devices) {
        const char *syspath = udev_list_entry_get_name(device);
        struct udev_device *udev_device = udev_device_new_from_syspath(udev, syspath);
        device_added(udev_device);
        udev_device_unref(udev_device);
    }
    udev_enumerate_unref(enumerate);

    RegisterBlockAndWakeupHandlers(block_handler, wakeup_handler, NULL);
    AddGeneralSocket(udev_monitor_get_fd(udev_monitor));

    return 1;
}

void
config_udev_fini(void)
{
    struct udev *udev;

    if (!udev_monitor)
        return;

    udev = udev_monitor_get_udev(udev_monitor);

    RemoveGeneralSocket(udev_monitor_get_fd(udev_monitor));
    RemoveBlockAndWakeupHandlers(block_handler, wakeup_handler, NULL);
    udev_monitor_unref(udev_monitor);
    udev_monitor = NULL;
    udev_unref(udev);
}

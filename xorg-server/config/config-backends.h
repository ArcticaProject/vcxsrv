/*
 * Copyright © 2006-2007 Daniel Stone
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

#ifdef HAVE_DBUS
#include <dbus/dbus.h>

typedef void (*config_dbus_core_connect_hook)(DBusConnection *connection,
                                              void *data);
typedef void (*config_dbus_core_disconnect_hook)(void *data);

struct config_dbus_core_hook {
    config_dbus_core_connect_hook connect;
    config_dbus_core_disconnect_hook disconnect;
    void *data;

    struct config_dbus_core_hook *next;
};

int config_dbus_core_init(void);
void config_dbus_core_fini(void);
int config_dbus_core_add_hook(struct config_dbus_core_hook *hook);
void config_dbus_core_remove_hook(struct config_dbus_core_hook *hook);
#endif

#ifdef CONFIG_DBUS_API
int config_dbus_init(void);
void config_dbus_fini(void);
#endif

#ifdef CONFIG_HAL
int config_hal_init(void);
void config_hal_fini(void);
#endif

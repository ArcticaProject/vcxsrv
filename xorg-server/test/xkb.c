/**
 * Copyright © 2009 Red Hat, Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice (including the next
 *  paragraph) shall be included in all copies or substantial portions of the
 *  Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <xkb-config.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <math.h>
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include "misc.h"
#include "inputstr.h"
#include "opaque.h"
#include "property.h"
#define	XKBSRV_NEED_FILE_FUNCS
#include <xkbsrv.h>
#include "../xkb/xkbgeom.h"
#include <X11/extensions/XKMformat.h>
#include "xkbfile.h"
#include "../xkb/xkb.h"

#include <glib.h>

/**
 * Initialize an empty XkbRMLVOSet.
 * Call XkbGetRulesDflts to obtain the default ruleset.
 * Compare obtained ruleset with the built-in defaults.
 *
 * Result: RMLVO defaults are the same as obtained.
 */
static void xkb_get_rules_test(void)
{
    XkbRMLVOSet rmlvo = { NULL};
    XkbGetRulesDflts(&rmlvo);


    g_assert(rmlvo.rules);
    g_assert(rmlvo.model);
    g_assert(rmlvo.layout);
    g_assert(rmlvo.variant);
    g_assert(rmlvo.options);
    g_assert(strcmp(rmlvo.rules, XKB_DFLT_RULES) == 0);
    g_assert(strcmp(rmlvo.model, XKB_DFLT_MODEL) == 0);
    g_assert(strcmp(rmlvo.layout, XKB_DFLT_LAYOUT) == 0);
    g_assert(strcmp(rmlvo.variant, XKB_DFLT_VARIANT) == 0);
    g_assert(strcmp(rmlvo.options, XKB_DFLT_OPTIONS) == 0);
}

/**
 * Initialize an random XkbRMLVOSet.
 * Call XkbGetRulesDflts to obtain the default ruleset.
 * Compare obtained ruleset with the built-in defaults.
 * Result: RMLVO defaults are the same as obtained.
 */
static void xkb_set_rules_test(void)
{
    XkbRMLVOSet rmlvo = {
        .rules = "test-rules",
        .model = "test-model",
        .layout = "test-layout",
        .variant = "test-variant",
        .options = "test-options"
    };
    XkbRMLVOSet rmlvo_new = { NULL };

    XkbSetRulesDflts(&rmlvo);
    XkbGetRulesDflts(&rmlvo_new);

    /* XkbGetRulesDflts strdups the values */
    g_assert(rmlvo.rules != rmlvo_new.rules);
    g_assert(rmlvo.model != rmlvo_new.model);
    g_assert(rmlvo.layout != rmlvo_new.layout);
    g_assert(rmlvo.variant != rmlvo_new.variant);
    g_assert(rmlvo.options != rmlvo_new.options);

    g_assert(strcmp(rmlvo.rules, rmlvo_new.rules) == 0);
    g_assert(strcmp(rmlvo.model, rmlvo_new.model) == 0);
    g_assert(strcmp(rmlvo.layout, rmlvo_new.layout) == 0);
    g_assert(strcmp(rmlvo.variant, rmlvo_new.variant) == 0);
    g_assert(strcmp(rmlvo.options, rmlvo_new.options) == 0);
}


/**
 * Get the default RMLVO set.
 * Set the default RMLVO set.
 * Get the default RMLVO set.
 * Repeat the last two steps.
 *
 * Result: RMLVO set obtained is the same as previously set.
 */
static void xkb_set_get_rules_test(void)
{
/* This test failed before XkbGetRulesDftlts changed to strdup.
   We test this twice because the first time using XkbGetRulesDflts we obtain
   the built-in defaults. The unexpected free isn't triggered until the second
   XkbSetRulesDefaults.
 */
    XkbRMLVOSet rmlvo = { NULL };
    XkbRMLVOSet rmlvo_backup;

    XkbGetRulesDflts(&rmlvo);

    /* pass 1 */
    XkbSetRulesDflts(&rmlvo);
    XkbGetRulesDflts(&rmlvo);

    /* Make a backup copy */
    rmlvo_backup.rules = strdup(rmlvo.rules);
    rmlvo_backup.layout = strdup(rmlvo.layout);
    rmlvo_backup.model = strdup(rmlvo.model);
    rmlvo_backup.variant = strdup(rmlvo.variant);
    rmlvo_backup.options = strdup(rmlvo.options);

    /* pass 2 */
    XkbSetRulesDflts(&rmlvo);

    /* This test is iffy, because strictly we may be comparing against already
     * freed memory */
    g_assert(strcmp(rmlvo.rules, rmlvo_backup.rules) == 0);
    g_assert(strcmp(rmlvo.model, rmlvo_backup.model) == 0);
    g_assert(strcmp(rmlvo.layout, rmlvo_backup.layout) == 0);
    g_assert(strcmp(rmlvo.variant, rmlvo_backup.variant) == 0);
    g_assert(strcmp(rmlvo.options, rmlvo_backup.options) == 0);

    XkbGetRulesDflts(&rmlvo);
    g_assert(strcmp(rmlvo.rules, rmlvo_backup.rules) == 0);
    g_assert(strcmp(rmlvo.model, rmlvo_backup.model) == 0);
    g_assert(strcmp(rmlvo.layout, rmlvo_backup.layout) == 0);
    g_assert(strcmp(rmlvo.variant, rmlvo_backup.variant) == 0);
    g_assert(strcmp(rmlvo.options, rmlvo_backup.options) == 0);
}


int main(int argc, char** argv)
{
    g_test_init(&argc, &argv,NULL);
    g_test_bug_base("https://bugzilla.freedesktop.org/show_bug.cgi?id=");

    g_test_add_func("/xkb/set-get-rules", xkb_set_get_rules_test);
    g_test_add_func("/xkb/get-rules", xkb_get_rules_test);
    g_test_add_func("/xkb/set-rules", xkb_set_rules_test);

    return g_test_run();
}

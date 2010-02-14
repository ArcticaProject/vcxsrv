/*
 * Copyright (c) 2009 Dan Nicholson
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/* View/edit this file with tab stops set to 4 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <string.h>
#include "os.h"
#include "xf86Parser.h"
#include "xf86tokens.h"
#include "Configint.h"

extern LexRec val;

static
xf86ConfigSymTabRec InputClassTab[] =
{
    {ENDSECTION, "endsection"},
    {IDENTIFIER, "identifier"},
    {OPTION, "option"},
    {DRIVER, "driver"},
    {MATCH_PRODUCT, "matchproduct"},
    {MATCH_VENDOR, "matchvendor"},
    {MATCH_DEVICE_PATH, "matchdevicepath"},
    {MATCH_TAG, "matchtag"},
    {MATCH_IS_KEYBOARD, "matchiskeyboard"},
    {MATCH_IS_POINTER, "matchispointer"},
    {MATCH_IS_JOYSTICK, "matchisjoystick"},
    {MATCH_IS_TABLET, "matchistablet"},
    {MATCH_IS_TOUCHPAD, "matchistouchpad"},
    {MATCH_IS_TOUCHSCREEN, "matchistouchscreen"},
    {-1, ""},
};

#define CLEANUP xf86freeInputClassList

#define TOKEN_SEP "|"

XF86ConfInputClassPtr
xf86parseInputClassSection(void)
{
    int has_ident = FALSE;
    int token;

    parsePrologue(XF86ConfInputClassPtr, XF86ConfInputClassRec)

    while ((token = xf86getToken(InputClassTab)) != ENDSECTION) {
        switch (token) {
        case COMMENT:
            ptr->comment = xf86addComment(ptr->comment, val.str);
            break;
        case IDENTIFIER:
            if (xf86getSubToken(&(ptr->comment)) != STRING)
                Error(QUOTE_MSG, "Identifier");
            if (has_ident == TRUE)
                Error(MULTIPLE_MSG, "Identifier");
            ptr->identifier = val.str;
            has_ident = TRUE;
            break;
        case DRIVER:
            if (xf86getSubToken(&(ptr->comment)) != STRING)
                Error(QUOTE_MSG, "Driver");
            if (strcmp(val.str, "keyboard") == 0)
                ptr->driver = "kbd";
            else
                ptr->driver = val.str;
            break;
        case OPTION:
            ptr->option_lst = xf86parseOption(ptr->option_lst);
            break;
        case MATCH_PRODUCT:
            if (xf86getSubToken(&(ptr->comment)) != STRING)
                Error(QUOTE_MSG, "MatchProduct");
            ptr->match_product = xstrtokenize(val.str, TOKEN_SEP);
            break;
        case MATCH_VENDOR:
            if (xf86getSubToken(&(ptr->comment)) != STRING)
                Error(QUOTE_MSG, "MatchVendor");
            ptr->match_vendor = xstrtokenize(val.str, TOKEN_SEP);
            break;
        case MATCH_DEVICE_PATH:
            if (xf86getSubToken(&(ptr->comment)) != STRING)
                Error(QUOTE_MSG, "MatchDevicePath");
            ptr->match_device = xstrtokenize(val.str, TOKEN_SEP);
            break;
        case MATCH_TAG:
            if (xf86getSubToken(&(ptr->comment)) != STRING)
                Error(QUOTE_MSG, "MatchTag");
            ptr->match_tag = xstrtokenize(val.str, TOKEN_SEP);
            break;
        case MATCH_IS_KEYBOARD:
            if (xf86getSubToken(&(ptr->comment)) != STRING)
                Error(QUOTE_MSG, "MatchIsKeyboard");
            ptr->is_keyboard.set = xf86getBoolValue(&ptr->is_keyboard.val,
                                                    val.str);
            if (!ptr->is_keyboard.set)
                Error(BOOL_MSG, "MatchIsKeyboard");
            break;
        case MATCH_IS_POINTER:
            if (xf86getSubToken(&(ptr->comment)) != STRING)
                Error(QUOTE_MSG, "MatchIsPointer");
            ptr->is_pointer.set = xf86getBoolValue(&ptr->is_pointer.val,
                                                   val.str);
            if (!ptr->is_pointer.set)
                Error(BOOL_MSG, "MatchIsPointer");
            break;
        case MATCH_IS_JOYSTICK:
            if (xf86getSubToken(&(ptr->comment)) != STRING)
                Error(QUOTE_MSG, "MatchIsJoystick");
            ptr->is_joystick.set = xf86getBoolValue(&ptr->is_joystick.val,
                                                    val.str);
            if (!ptr->is_joystick.set)
                Error(BOOL_MSG, "MatchIsJoystick");
            break;
        case MATCH_IS_TABLET:
            if (xf86getSubToken(&(ptr->comment)) != STRING)
                Error(QUOTE_MSG, "MatchIsTablet");
            ptr->is_tablet.set = xf86getBoolValue(&ptr->is_tablet.val,
                                                  val.str);
            if (!ptr->is_tablet.set)
                Error(BOOL_MSG, "MatchIsTablet");
            break;
        case MATCH_IS_TOUCHPAD:
            if (xf86getSubToken(&(ptr->comment)) != STRING)
                Error(QUOTE_MSG, "MatchIsTouchpad");
            ptr->is_touchpad.set = xf86getBoolValue(&ptr->is_touchpad.val,
                                                    val.str);
            if (!ptr->is_touchpad.set)
                Error(BOOL_MSG, "MatchIsTouchpad");
            break;
        case MATCH_IS_TOUCHSCREEN:
            if (xf86getSubToken(&(ptr->comment)) != STRING)
                Error(QUOTE_MSG, "MatchIsTouchscreen");
            ptr->is_touchscreen.set = xf86getBoolValue(&ptr->is_touchscreen.val,
                                                       val.str);
            if (!ptr->is_touchscreen.set)
                Error(BOOL_MSG, "MatchIsTouchscreen");
            break;
        case EOF_TOKEN:
            Error(UNEXPECTED_EOF_MSG, NULL);
            break;
        default:
            Error(INVALID_KEYWORD_MSG, xf86tokenString ());
            break;
        }
    }

    if (!has_ident)
        Error(NO_IDENT_MSG, NULL);

#ifdef DEBUG
    printf("InputClass section parsed\n");
#endif

    return ptr;
}

void
xf86printInputClassSection (FILE * cf, XF86ConfInputClassPtr ptr)
{
    char **list;

    while (ptr) {
        fprintf(cf, "Section \"InputClass\"\n");
        if (ptr->comment)
            fprintf(cf, "%s", ptr->comment);
        if (ptr->identifier)
            fprintf(cf, "\tIdentifier      \"%s\"\n", ptr->identifier);
        if (ptr->driver)
            fprintf(cf, "\tDriver          \"%s\"\n", ptr->driver);
        if (ptr->match_product) {
            fprintf(cf, "\tMatchProduct    \"");
            for (list = ptr->match_product; *list; list++)
                fprintf(cf, "%s%s",
                        list == ptr->match_product ? "" : TOKEN_SEP,
                        *list);
            fprintf(cf, "\"\n");
        }
        if (ptr->match_vendor) {
            fprintf(cf, "\tMatchVendor     \"");
            for (list = ptr->match_vendor; *list; list++)
                fprintf(cf, "%s%s",
                        list == ptr->match_vendor ? "" : TOKEN_SEP,
                        *list);
            fprintf(cf, "\"\n");
        }
        if (ptr->match_device) {
            fprintf(cf, "\tMatchDevicePath \"");
            for (list = ptr->match_device; *list; list++)
                fprintf(cf, "%s%s",
                        list == ptr->match_device ? "" : TOKEN_SEP,
                        *list);
            fprintf(cf, "\"\n");
        }
        if (ptr->match_tag) {
            fprintf(cf, "\tMatchTag \"");
            for (list = ptr->match_tag; *list; list++)
                fprintf(cf, "%s%s",
                        list == ptr->match_tag ? "" : TOKEN_SEP,
                        *list);
            fprintf(cf, "\"\n");
        }
        if (ptr->is_keyboard.set)
            fprintf(cf, "\tIsKeyboard      \"%s\"\n",
                    ptr->is_keyboard.val ? "yes" : "no");
        if (ptr->is_pointer.set)
            fprintf(cf, "\tIsPointer       \"%s\"\n",
                    ptr->is_pointer.val ? "yes" : "no");
        if (ptr->is_joystick.set)
            fprintf(cf, "\tIsJoystick      \"%s\"\n",
                    ptr->is_joystick.val ? "yes" : "no");
        if (ptr->is_tablet.set)
            fprintf(cf, "\tIsTablet        \"%s\"\n",
                    ptr->is_tablet.val ? "yes" : "no");
        if (ptr->is_touchpad.set)
            fprintf(cf, "\tIsTouchpad      \"%s\"\n",
                    ptr->is_touchpad.val ? "yes" : "no");
        if (ptr->is_touchscreen.set)
            fprintf(cf, "\tIsTouchscreen   \"%s\"\n",
                    ptr->is_touchscreen.val ? "yes" : "no");
        xf86printOptionList(cf, ptr->option_lst, 1);
        fprintf(cf, "EndSection\n\n");
        ptr = ptr->list.next;
    }
}

void
xf86freeInputClassList (XF86ConfInputClassPtr ptr)
{
    XF86ConfInputClassPtr prev;
    char **list;

    while (ptr) {
        TestFree(ptr->identifier);
        TestFree(ptr->driver);
        if (ptr->match_product) {
            for (list = ptr->match_product; *list; list++)
                free(*list);
            free(ptr->match_product);
        }
        if (ptr->match_vendor) {
            for (list = ptr->match_vendor; *list; list++)
                free(*list);
            free(ptr->match_vendor);
        }
        if (ptr->match_device) {
            for (list = ptr->match_device; *list; list++)
                free(*list);
            free(ptr->match_device);
        }
        if (ptr->match_tag) {
            for (list = ptr->match_tag; *list; list++)
                free(*list);
            free(ptr->match_tag);
        }
        TestFree(ptr->comment);
        xf86optionListFree(ptr->option_lst);

        prev = ptr;
        ptr = ptr->list.next;
        free(prev);
    }
}

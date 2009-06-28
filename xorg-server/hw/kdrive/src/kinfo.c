/*
 * Copyright � 1999 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif
#include "kdrive.h"

KdCardInfo  *kdCardInfo;

KdCardInfo *
KdCardInfoAdd (KdCardFuncs  *funcs,
	       KdCardAttr   *attr,
	       void	    *closure)
{
    KdCardInfo	*ci, **prev;

    ci = (KdCardInfo *) xalloc (sizeof (KdCardInfo));
    if (!ci)
	return 0;
    bzero (ci, sizeof (KdCardInfo));
    for (prev = &kdCardInfo; *prev; prev = &(*prev)->next);
    *prev = ci;
    ci->cfuncs = funcs;
    ci->attr = *attr;
    ci->closure = closure;
    ci->screenList = 0;
    ci->selected = 0;
    ci->next = 0;
    return ci;
}

KdCardInfo *
KdCardInfoLast (void)
{
    KdCardInfo	*ci;

    if (!kdCardInfo)
	return 0;
    for (ci = kdCardInfo; ci->next; ci = ci->next);
    return ci;
}

void
KdCardInfoDispose (KdCardInfo *ci)
{
    KdCardInfo	**prev;

    for (prev = &kdCardInfo; *prev; prev = &(*prev)->next)
	if (*prev == ci)
	{
	    *prev = ci->next;
	    xfree (ci);
	    break;
	}
}

KdScreenInfo *
KdScreenInfoAdd (KdCardInfo *ci)
{
    KdScreenInfo    *si, **prev;
    int		    n;

    si = (KdScreenInfo *) xalloc (sizeof (KdScreenInfo));
    if (!si)
	return 0;
    bzero (si, sizeof (KdScreenInfo));
    for (prev = &ci->screenList, n = 0; *prev; prev = &(*prev)->next, n++);
    *prev = si;
    si->next = 0;
    si->card = ci;
    si->mynum = n;
    return si;
}

void
KdScreenInfoDispose (KdScreenInfo *si)
{
    KdCardInfo	    *ci = si->card;
    KdScreenInfo    **prev;

    for (prev = &ci->screenList; *prev; prev = &(*prev)->next) {
	if (*prev == si)
	{
	    *prev = si->next;
	    xfree (si);
	    if (!ci->screenList)
		KdCardInfoDispose (ci);
	    break;
	}
    }
}

KdPointerInfo *
KdNewPointer (void)
{
    KdPointerInfo *pi;
    int i;

    pi = (KdPointerInfo *)xcalloc(1, sizeof(KdPointerInfo));
    if (!pi)
        return NULL;

    pi->name = KdSaveString("Generic Pointer");
    pi->path = NULL;
    pi->inputClass = KD_MOUSE;
    pi->driver = NULL;
    pi->driverPrivate = NULL;
    pi->next = NULL;
    pi->options = NULL;
    pi->nAxes = 3;
    pi->nButtons = KD_MAX_BUTTON;
    for (i = 1; i < KD_MAX_BUTTON; i++)
        pi->map[i] = i;

    return pi;
}

void
KdFreePointer(KdPointerInfo *pi)
{
    InputOption *option, *prev = NULL;

    if (pi->name)
        xfree(pi->name);
    if (pi->path)
        xfree(pi->path);

    for (option = pi->options; option; option = option->next) {
        if (prev)
            xfree(prev);
        if (option->key)
            xfree(option->key);
        if (option->value)
            xfree(option->value);
        prev = option;
    }

    if (prev)
        xfree(prev);
    
    xfree(pi);
}
 
void
KdFreeKeyboard(KdKeyboardInfo *ki)
{
    if (ki->name)
        xfree(ki->name);
    if (ki->path)
        xfree(ki->path);
    ki->next = NULL;
    xfree(ki);
}

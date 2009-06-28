#include "xgl.h"
#include "xegl.h"

KdOsFuncs *kdOsFuncs;
Bool kdEmulateMiddleButton;
Bool kdRawPointerCoordinates;
Bool kdDontZap;
Bool kdDisableZaphod;
int kdScreenPrivateIndex;

static char *
KdParseFindNext (char *cur, char *delim, char *save, char *last)
{
    while (*cur && !strchr (delim, *cur))
    {
        *save++ = *cur++;
    }
    *save = 0;
    *last = *cur;
    if (*cur)
        cur++;
    return cur;
}

/*
 * Mouse argument syntax:
 *
 *  device,protocol,options...
 *
 *  Options are any of:
 *      1-5         n button mouse
 *      2button     emulate middle button
 *      {NMO}       Reorder buttons
 */
char *
KdSaveString (char *str)
{
    char    *n = (char *) xalloc (strlen (str) + 1);

    if (!n)
        return 0;
    strcpy (n, str);
    return n;
}

/*
 * Parse mouse information.  Syntax:
 *
 *  <device>,<nbutton>,<protocol>{,<option>}...
 *
 * options: {nmo}   pointer mapping (e.g. {321})
 *          2button emulate middle button
 *          3button dont emulate middle button
 */
void
KdParseMouse (char *arg)
{
    char        save[1024];
    char        delim;
    KdMouseInfo *mi;
    int         i;

    mi = KdMouseInfoAdd ();
    if (!mi)
        return;
    mi->name = 0;
    mi->prot = 0;
    mi->emulateMiddleButton = kdEmulateMiddleButton;
    mi->transformCoordinates = !kdRawPointerCoordinates;
    mi->nbutton = 3;
    for (i = 0; i < KD_MAX_BUTTON; i++)
        mi->map[i] = i + 1;

    if (!arg)
        return;
    if (strlen (arg) >= sizeof (save))
        return;
    arg = KdParseFindNext (arg, ",", save, &delim);
    if (!save[0])
        return;
    mi->name = KdSaveString (save);
    if (delim != ',')
        return;

    arg = KdParseFindNext (arg, ",", save, &delim);
    if (!save[0])
        return;

    if ('1' <= save[0] && save[0] <= '0' + KD_MAX_BUTTON && save[1] == '\0')
    {
        mi->nbutton = save[0] - '0';
        if (mi->nbutton > KD_MAX_BUTTON)
        {
            UseMsg ();
            return;
        }
    }

    if (!delim != ',')
        return;

    arg = KdParseFindNext (arg, ",", save, &delim);

    if (save[0])
        mi->prot = KdSaveString (save);

    while (delim == ',')
    {
        arg = KdParseFindNext (arg, ",", save, &delim);
        if (save[0] == '{')
        {
            char        *s = save + 1;
            i = 0;
            while (*s && *s != '}')
            {
                if ('1' <= *s && *s <= '0' + mi->nbutton)
                    mi->map[i] = *s - '0';
                else
                    UseMsg ();
                s++;
            }
        }
        else if (!strcmp (save, "2button"))
            mi->emulateMiddleButton = TRUE;
        else if (!strcmp (save, "3button"))
            mi->emulateMiddleButton = FALSE;
        else if (!strcmp (save, "rawcoord"))
            mi->transformCoordinates = FALSE;
        else if (!strcmp (save, "transform"))
            mi->transformCoordinates = TRUE;
        else
            UseMsg ();
    }
}

KdMouseInfo *kdMouseInfo;

KdMouseInfo *
KdMouseInfoAdd (void)
{
    KdMouseInfo *mi, **prev;

    mi = (KdMouseInfo *) xalloc (sizeof (KdMouseInfo));
    if (!mi)
        return 0;
    bzero (mi, sizeof (KdMouseInfo));
    for (prev = &kdMouseInfo; *prev; prev = &(*prev)->next);
    *prev = mi;
    return mi;
}

void
KdMouseInfoDispose (KdMouseInfo *mi)
{
    KdMouseInfo **prev;

    for (prev = &kdMouseInfo; *prev; prev = &(*prev)->next)
        if (*prev == mi)
        {
            *prev = mi->next;
            if (mi->name)
                xfree (mi->name);
            if (mi->prot)
                xfree (mi->prot);
            xfree (mi);
            break;
        }
}

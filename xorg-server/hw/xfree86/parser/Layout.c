/* 
 * 
 * Copyright (c) 1997  Metro Link Incorporated
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * Except as contained in this notice, the name of the Metro Link shall not be
 * used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization from Metro Link.
 * 
 */
/*
 * Copyright (c) 1997-2003 by The XFree86 Project, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the copyright holder(s)
 * and author(s) shall not be used in advertising or otherwise to promote
 * the sale, use or other dealings in this Software without prior written
 * authorization from the copyright holder(s) and author(s).
 */


/* View/edit this file with tab stops set to 4 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "xf86Parser.h"
#include "xf86tokens.h"
#include "Configint.h"
#include <string.h>

extern LexRec val;

static xf86ConfigSymTabRec LayoutTab[] =
{
	{ENDSECTION, "endsection"},
	{SCREEN, "screen"},
	{IDENTIFIER, "identifier"},
	{INACTIVE, "inactive"},
	{INPUTDEVICE, "inputdevice"},
	{OPTION, "option"},
	{-1, ""},
};

static xf86ConfigSymTabRec AdjTab[] =
{
	{RIGHTOF, "rightof"},
	{LEFTOF, "leftof"},
	{ABOVE, "above"},
	{BELOW, "below"},
	{RELATIVE, "relative"},
	{ABSOLUTE, "absolute"},
	{-1, ""},
};

#define CLEANUP xf86freeLayoutList

XF86ConfLayoutPtr
xf86parseLayoutSection (void)
{
	int has_ident = FALSE;
	int token;
	parsePrologue (XF86ConfLayoutPtr, XF86ConfLayoutRec)

	while ((token = xf86getToken (LayoutTab)) != ENDSECTION)
	{
		switch (token)
		{
		case COMMENT:
			ptr->lay_comment = xf86addComment(ptr->lay_comment, val.str);
			break;
		case IDENTIFIER:
			if (xf86getSubToken (&(ptr->lay_comment)) != STRING)
				Error (QUOTE_MSG, "Identifier");
			if (has_ident == TRUE)
				Error (MULTIPLE_MSG, "Identifier");
			ptr->lay_identifier = val.str;
			has_ident = TRUE;
			break;
		case INACTIVE:
			{
				XF86ConfInactivePtr iptr;

				iptr = xf86confcalloc (1, sizeof (XF86ConfInactiveRec));
				iptr->list.next = NULL;
				if (xf86getSubToken (&(ptr->lay_comment)) != STRING) {
					xf86conffree (iptr);
					Error (INACTIVE_MSG, NULL);
				}
				iptr->inactive_device_str = val.str;
				ptr->lay_inactive_lst = (XF86ConfInactivePtr)
					xf86addListItem ((glp) ptr->lay_inactive_lst, (glp) iptr);
			}
			break;
		case SCREEN:
			{
				XF86ConfAdjacencyPtr aptr;
				int absKeyword = 0;

				aptr = xf86confcalloc (1, sizeof (XF86ConfAdjacencyRec));
				aptr->list.next = NULL;
				aptr->adj_scrnum = -1;
				aptr->adj_where = CONF_ADJ_OBSOLETE;
				aptr->adj_x = 0;
				aptr->adj_y = 0;
				aptr->adj_refscreen = NULL;
				if ((token = xf86getSubToken (&(ptr->lay_comment))) == NUMBER)
					aptr->adj_scrnum = val.num;
				else
					xf86unGetToken (token);
				token = xf86getSubToken(&(ptr->lay_comment));
				if (token != STRING) {
					xf86conffree(aptr);
					Error (SCREEN_MSG, NULL);
				}
				aptr->adj_screen_str = val.str;

				token = xf86getSubTokenWithTab(&(ptr->lay_comment), AdjTab);
				switch (token)
				{
				case RIGHTOF:
					aptr->adj_where = CONF_ADJ_RIGHTOF;
					break;
				case LEFTOF:
					aptr->adj_where = CONF_ADJ_LEFTOF;
					break;
				case ABOVE:
					aptr->adj_where = CONF_ADJ_ABOVE;
					break;
				case BELOW:
					aptr->adj_where = CONF_ADJ_BELOW;
					break;
				case RELATIVE:
					aptr->adj_where = CONF_ADJ_RELATIVE;
					break;
				case ABSOLUTE:
					aptr->adj_where = CONF_ADJ_ABSOLUTE;
					absKeyword = 1;
					break;
				case EOF_TOKEN:
					xf86conffree(aptr);
					Error (UNEXPECTED_EOF_MSG, NULL);
					break;
				default:
					xf86unGetToken (token);
					token = xf86getSubToken(&(ptr->lay_comment));
					if (token == STRING)
						aptr->adj_where = CONF_ADJ_OBSOLETE;
					else
						aptr->adj_where = CONF_ADJ_ABSOLUTE;
				}
				switch (aptr->adj_where)
				{
				case CONF_ADJ_ABSOLUTE:
					if (absKeyword) 
						token = xf86getSubToken(&(ptr->lay_comment));
					if (token == NUMBER)
					{
						aptr->adj_x = val.num;
						token = xf86getSubToken(&(ptr->lay_comment));
						if (token != NUMBER) {
							xf86conffree(aptr);
							Error(INVALID_SCR_MSG, NULL);
						}
						aptr->adj_y = val.num;
					} else {
						if (absKeyword) {
							xf86conffree(aptr);
							Error(INVALID_SCR_MSG, NULL);
						} else
							xf86unGetToken (token);
					}
					break;
				case CONF_ADJ_RIGHTOF:
				case CONF_ADJ_LEFTOF:
				case CONF_ADJ_ABOVE:
				case CONF_ADJ_BELOW:
				case CONF_ADJ_RELATIVE:
					token = xf86getSubToken(&(ptr->lay_comment));
					if (token != STRING) {
						xf86conffree(aptr);
						Error(INVALID_SCR_MSG, NULL);
					}
					aptr->adj_refscreen = val.str;
					if (aptr->adj_where == CONF_ADJ_RELATIVE)
					{
						token = xf86getSubToken(&(ptr->lay_comment));
						if (token != NUMBER) {
							xf86conffree(aptr);
							Error(INVALID_SCR_MSG, NULL);
						}
						aptr->adj_x = val.num;
						token = xf86getSubToken(&(ptr->lay_comment));
						if (token != NUMBER) {
							xf86conffree(aptr);
							Error(INVALID_SCR_MSG, NULL);
						}
						aptr->adj_y = val.num;
					}
					break;
				case CONF_ADJ_OBSOLETE:
					/* top */
					aptr->adj_top_str = val.str;

					/* bottom */
					if (xf86getSubToken (&(ptr->lay_comment)) != STRING) {
						xf86conffree(aptr);
						Error (SCREEN_MSG, NULL);
					}
					aptr->adj_bottom_str = val.str;

					/* left */
					if (xf86getSubToken (&(ptr->lay_comment)) != STRING) {
						xf86conffree(aptr);
						Error (SCREEN_MSG, NULL);
					}
					aptr->adj_left_str = val.str;

					/* right */
					if (xf86getSubToken (&(ptr->lay_comment)) != STRING) {
						xf86conffree(aptr);
						Error (SCREEN_MSG, NULL);
					}
					aptr->adj_right_str = val.str;

				}
				ptr->lay_adjacency_lst = (XF86ConfAdjacencyPtr)
					xf86addListItem ((glp) ptr->lay_adjacency_lst, (glp) aptr);
			}
			break;
		case INPUTDEVICE:
			{
				XF86ConfInputrefPtr iptr;

				iptr = xf86confcalloc (1, sizeof (XF86ConfInputrefRec));
				iptr->list.next = NULL;
				iptr->iref_option_lst = NULL;
				if (xf86getSubToken (&(ptr->lay_comment)) != STRING) {
					xf86conffree(iptr);
					Error (INPUTDEV_MSG, NULL);
				}
				iptr->iref_inputdev_str = val.str;
				while ((token = xf86getSubToken (&(ptr->lay_comment))) == STRING)
				{
					iptr->iref_option_lst =
						xf86addNewOption (iptr->iref_option_lst, val.str, NULL);
				}
				xf86unGetToken (token);
				ptr->lay_input_lst = (XF86ConfInputrefPtr)
					xf86addListItem ((glp) ptr->lay_input_lst, (glp) iptr);
			}
			break;
		case OPTION:
			ptr->lay_option_lst = xf86parseOption(ptr->lay_option_lst);
			break;
		case EOF_TOKEN:
			Error (UNEXPECTED_EOF_MSG, NULL);
			break;
		default:
			Error (INVALID_KEYWORD_MSG, xf86tokenString ());
			break;
		}
	}

	if (!has_ident)
		Error (NO_IDENT_MSG, NULL);

#ifdef DEBUG
	printf ("Layout section parsed\n");
#endif

	return ptr;
}

#undef CLEANUP

void
xf86printLayoutSection (FILE * cf, XF86ConfLayoutPtr ptr)
{
	XF86ConfAdjacencyPtr aptr;
	XF86ConfInactivePtr iptr;
	XF86ConfInputrefPtr inptr;
	XF86OptionPtr optr;

	while (ptr)
	{
		fprintf (cf, "Section \"ServerLayout\"\n");
		if (ptr->lay_comment)
			fprintf (cf, "%s", ptr->lay_comment);
		if (ptr->lay_identifier)
			fprintf (cf, "\tIdentifier     \"%s\"\n", ptr->lay_identifier);

		for (aptr = ptr->lay_adjacency_lst; aptr; aptr = aptr->list.next)
		{
			fprintf (cf, "\tScreen     ");
			if (aptr->adj_scrnum >= 0)
				fprintf (cf, "%2d", aptr->adj_scrnum);
			else
				fprintf (cf, "  ");
			fprintf (cf, "  \"%s\"", aptr->adj_screen_str);
			switch(aptr->adj_where)
			{
			case CONF_ADJ_OBSOLETE:
				fprintf (cf, " \"%s\"", aptr->adj_top_str);
				fprintf (cf, " \"%s\"", aptr->adj_bottom_str);
				fprintf (cf, " \"%s\"", aptr->adj_right_str);
				fprintf (cf, " \"%s\"\n", aptr->adj_left_str);
				break;
			case CONF_ADJ_ABSOLUTE:
				if (aptr->adj_x != -1)
					fprintf (cf, " %d %d\n", aptr->adj_x, aptr->adj_y);
				else
					fprintf (cf, "\n");
				break;
			case CONF_ADJ_RIGHTOF:
				fprintf (cf, " RightOf \"%s\"\n", aptr->adj_refscreen);
				break;
			case CONF_ADJ_LEFTOF:
				fprintf (cf, " LeftOf \"%s\"\n", aptr->adj_refscreen);
				break;
			case CONF_ADJ_ABOVE:
				fprintf (cf, " Above \"%s\"\n", aptr->adj_refscreen);
				break;
			case CONF_ADJ_BELOW:
				fprintf (cf, " Below \"%s\"\n", aptr->adj_refscreen);
				break;
			case CONF_ADJ_RELATIVE:
				fprintf (cf, " Relative \"%s\" %d %d\n", aptr->adj_refscreen,
						 aptr->adj_x, aptr->adj_y);
				break;
			}
		}
		for (iptr = ptr->lay_inactive_lst; iptr; iptr = iptr->list.next)
			fprintf (cf, "\tInactive       \"%s\"\n", iptr->inactive_device_str);
		for (inptr = ptr->lay_input_lst; inptr; inptr = inptr->list.next)
		{
			fprintf (cf, "\tInputDevice    \"%s\"", inptr->iref_inputdev_str);
			for (optr = inptr->iref_option_lst; optr; optr = optr->list.next)
			{
				fprintf(cf, " \"%s\"", optr->opt_name);
			}
			fprintf(cf, "\n");
		}
		xf86printOptionList(cf, ptr->lay_option_lst, 1);
		fprintf (cf, "EndSection\n\n");
		ptr = ptr->list.next;
	}
}

static void
xf86freeAdjacencyList (XF86ConfAdjacencyPtr ptr)
{
	XF86ConfAdjacencyPtr prev;

	while (ptr)
	{
		TestFree (ptr->adj_screen_str);
		TestFree (ptr->adj_top_str);
		TestFree (ptr->adj_bottom_str);
		TestFree (ptr->adj_left_str);
		TestFree (ptr->adj_right_str);

		prev = ptr;
		ptr = ptr->list.next;
		xf86conffree (prev);
	}

}

static void
xf86freeInputrefList (XF86ConfInputrefPtr ptr)
{
	XF86ConfInputrefPtr prev;

	while (ptr)
	{
		TestFree (ptr->iref_inputdev_str);
		xf86optionListFree (ptr->iref_option_lst);
		prev = ptr;
		ptr = ptr->list.next;
		xf86conffree (prev);
	}

}

void
xf86freeLayoutList (XF86ConfLayoutPtr ptr)
{
	XF86ConfLayoutPtr prev;

	while (ptr)
	{
		TestFree (ptr->lay_identifier);
		TestFree (ptr->lay_comment);
		xf86freeAdjacencyList (ptr->lay_adjacency_lst);
		xf86freeInputrefList (ptr->lay_input_lst);
		prev = ptr;
		ptr = ptr->list.next;
		xf86conffree (prev);
	}
}

#define CheckScreen(str, ptr)\
if (str[0] != '\0') \
{ \
screen = xf86findScreen (str, p->conf_screen_lst); \
if (!screen) \
{ \
	xf86validationError (UNDEFINED_SCREEN_MSG, \
				   str, layout->lay_identifier); \
	return (FALSE); \
} \
else \
	ptr = screen; \
}

int
xf86validateLayout (XF86ConfigPtr p)
{
	XF86ConfLayoutPtr layout = p->conf_layout_lst;
	XF86ConfAdjacencyPtr adj;
	XF86ConfInactivePtr iptr;
	XF86ConfInputrefPtr inptr;
	XF86ConfScreenPtr screen;
	XF86ConfDevicePtr device;
	XF86ConfInputPtr input;

	while (layout)
	{
		adj = layout->lay_adjacency_lst;
		while (adj)
		{
			/* the first one can't be "" but all others can */
			screen = xf86findScreen (adj->adj_screen_str, p->conf_screen_lst);
			if (!screen)
			{
				xf86validationError (UNDEFINED_SCREEN_MSG,
							   adj->adj_screen_str, layout->lay_identifier);
				return (FALSE);
			}
			else
				adj->adj_screen = screen;

#if 0
			CheckScreen (adj->adj_top_str, adj->adj_top);
			CheckScreen (adj->adj_bottom_str, adj->adj_bottom);
			CheckScreen (adj->adj_left_str, adj->adj_left);
			CheckScreen (adj->adj_right_str, adj->adj_right);
#endif

			adj = adj->list.next;
		}
		iptr = layout->lay_inactive_lst;
		while (iptr)
		{
			device = xf86findDevice (iptr->inactive_device_str,
									p->conf_device_lst);
			if (!device)
			{
				xf86validationError (UNDEFINED_DEVICE_LAY_MSG,
						iptr->inactive_device_str, layout->lay_identifier);
				return (FALSE);
			}
			else
				iptr->inactive_device = device;
			iptr = iptr->list.next;
		}
		inptr = layout->lay_input_lst;
		while (inptr)
		{
			input = xf86findInput (inptr->iref_inputdev_str,
									p->conf_input_lst);
			if (!input)
			{
				xf86validationError (UNDEFINED_INPUT_MSG,
						inptr->iref_inputdev_str, layout->lay_identifier);
				return (FALSE);
			}
			else
				inptr->iref_inputdev = input;
			inptr = inptr->list.next;
		}
		layout = layout->list.next;
	}
	return (TRUE);
}

XF86ConfLayoutPtr
xf86findLayout (const char *name, XF86ConfLayoutPtr list)
{
	while (list)
	{
		if (xf86nameCompare (list->lay_identifier, name) == 0)
			return (list);
		list = list->list.next;
	}
	return (NULL);
}


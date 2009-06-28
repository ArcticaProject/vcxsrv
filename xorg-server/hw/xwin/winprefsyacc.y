%{
/*
 * Copyright (C) 1994-2000 The XFree86 Project, Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE XFREE86 PROJECT BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the XFree86 Project
 * shall not be used in advertising or otherwise to promote the sale, use
 * or other dealings in this Software without prior written authorization
 * from the XFree86 Project.
 *
 * Authors:     Earle F. Philhower, III
 */
/* $XFree86: $ */

#ifdef HAVE_XWIN_CONFIG_H
#include <xwin-config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "winprefs.h"

/* The following give better error messages in bison at the cost of a few KB */
#define YYERROR_VERBOSE 1

/* The global pref settings */
WINPREFS pref;

/* The working menu */  
static MENUPARSED menu;

/* Functions for parsing the tokens into out structure */
/* Defined at the end section of this file */

static void SetIconDirectory (char *path);
static void SetDefaultIcon (char *fname);
static void SetRootMenu (char *menu);
static void SetDefaultSysMenu (char *menu, int pos);
static void SetTrayIcon (char *fname);

static void OpenMenu(char *menuname);
static void AddMenuLine(char *name, MENUCOMMANDTYPE cmd, char *param);
static void CloseMenu(void);

static void OpenIcons(void);
static void AddIconLine(char *matchstr, char *iconfile);
static void CloseIcons(void);

static void OpenSysMenu(void);
static void AddSysMenuLine(char *matchstr, char *menuname, int pos);
static void CloseSysMenu(void);

static int yyerror (char *s);

extern void ErrorF (const char* /*f*/, ...);
extern char *yytext;
extern int yylex(void);

%}

%union {
  char *sVal;
  int iVal;
}

%token NEWLINE MENU LB RB ICONDIRECTORY DEFAULTICON ICONS DEFAULTSYSMENU
%token SYSMENU ROOTMENU SEPARATOR ATSTART ATEND EXEC ALWAYSONTOP DEBUG
%token RELOAD TRAYICON SILENTEXIT

%token <sVal> STRING
%type <iVal>  atspot

%%

input:	/* empty */
	| input line
	;

line:	NEWLINE
	| command
	;


newline_or_nada:	
	| NEWLINE newline_or_nada
	;

command:	defaulticon
	| icondirectory
	| menu
	| icons
	| sysmenu
	| rootmenu
	| defaultsysmenu
	| debug
	| trayicon
	| silentexit
	;

trayicon:	TRAYICON STRING NEWLINE { SetTrayIcon($2); free($2); }
	;

rootmenu:	ROOTMENU STRING NEWLINE { SetRootMenu($2); free($2); }
	;

defaultsysmenu:	DEFAULTSYSMENU STRING atspot NEWLINE { SetDefaultSysMenu($2, $3); free($2); }
	;

defaulticon:	DEFAULTICON STRING NEWLINE { SetDefaultIcon($2); free($2); }
	;

icondirectory:	ICONDIRECTORY STRING NEWLINE { SetIconDirectory($2); free($2); }
	;

menuline:	SEPARATOR NEWLINE newline_or_nada  { AddMenuLine("-", CMD_SEPARATOR, ""); }
	| STRING ALWAYSONTOP NEWLINE newline_or_nada  { AddMenuLine($1, CMD_ALWAYSONTOP, ""); free($1); }
	| STRING EXEC STRING NEWLINE newline_or_nada  { AddMenuLine($1, CMD_EXEC, $3); free($1); free($3); }
	| STRING MENU STRING NEWLINE newline_or_nada  { AddMenuLine($1, CMD_MENU, $3); free($1); free($3); }
	| STRING RELOAD NEWLINE newline_or_nada  { AddMenuLine($1, CMD_RELOAD, ""); free($1); }
	;

menulist:	menuline
	| menuline menulist
	;

menu:	MENU STRING LB { OpenMenu($2); free($2); } newline_or_nada menulist RB {CloseMenu();}
	;

iconline:	STRING STRING NEWLINE newline_or_nada { AddIconLine($1, $2); free($1); free($2); }
	;

iconlist:	iconline
	| iconline iconlist
	;

icons:	ICONS LB {OpenIcons();} newline_or_nada iconlist RB {CloseIcons();}
	;

atspot:	{ $$=AT_END; }
	| ATSTART { $$=AT_START; }
	| ATEND { $$=AT_END; }
	;

sysmenuline:	STRING STRING atspot NEWLINE newline_or_nada { AddSysMenuLine($1, $2, $3); free($1); free($2); }
	;

sysmenulist:	sysmenuline
	| sysmenuline sysmenulist
	;

sysmenu:	SYSMENU LB NEWLINE {OpenSysMenu();} newline_or_nada sysmenulist RB {CloseSysMenu();}
	;

silentexit:	SILENTEXIT NEWLINE { pref.fSilentExit = TRUE; }
	;

debug: 	DEBUG STRING NEWLINE { ErrorF("LoadPreferences: %s\n", $2); free($2); }
	;


%%
/*
 * Errors in parsing abort and print log messages
 */
static int
yyerror (char *s) 
{
  extern int yylineno; /* Handled by flex internally */

  ErrorF("LoadPreferences: %s line %d\n", s, yylineno);
  return 1;
}

/* Miscellaneous functions to store TOKENs into the structure */
static void
SetIconDirectory (char *path)
{
  strncpy (pref.iconDirectory, path, PATH_MAX);
  pref.iconDirectory[PATH_MAX] = 0;
}

static void
SetDefaultIcon (char *fname)
{
  strncpy (pref.defaultIconName, fname, NAME_MAX);
  pref.defaultIconName[NAME_MAX] = 0;
}

static void
SetTrayIcon (char *fname)
{
  strncpy (pref.trayIconName, fname, NAME_MAX);
  pref.trayIconName[NAME_MAX] = 0;
}

static void
SetRootMenu (char *menu)
{
  strncpy (pref.rootMenuName, menu, MENU_MAX);
  pref.rootMenuName[MENU_MAX] = 0;
}

static void
SetDefaultSysMenu (char *menu, int pos)
{
  strncpy (pref.defaultSysMenuName, menu, MENU_MAX);
  pref.defaultSysMenuName[MENU_MAX] = 0;
  pref.defaultSysMenuPos = pos;
}

static void
OpenMenu (char *menuname)
{
  if (menu.menuItem) free(menu.menuItem);
  menu.menuItem = NULL;
  strncpy(menu.menuName, menuname, MENU_MAX);
  menu.menuName[MENU_MAX] = 0;
  menu.menuItems = 0;
}

static void
AddMenuLine (char *text, MENUCOMMANDTYPE cmd, char *param)
{
  if (menu.menuItem==NULL)
    menu.menuItem = (MENUITEM*)malloc(sizeof(MENUITEM));
  else
    menu.menuItem = (MENUITEM*)
      realloc(menu.menuItem, sizeof(MENUITEM)*(menu.menuItems+1));

  strncpy (menu.menuItem[menu.menuItems].text, text, MENU_MAX);
  menu.menuItem[menu.menuItems].text[MENU_MAX] = 0;

  menu.menuItem[menu.menuItems].cmd = cmd;

  strncpy(menu.menuItem[menu.menuItems].param, param, PARAM_MAX);
  menu.menuItem[menu.menuItems].param[PARAM_MAX] = 0;

  menu.menuItem[menu.menuItems].commandID = 0;

  menu.menuItems++;
}

static void
CloseMenu (void)
{
  if (menu.menuItem==NULL || menu.menuItems==0)
    {
      ErrorF("LoadPreferences: Empty menu detected\n");
      return;
    }
  
  if (pref.menuItems)
    pref.menu = (MENUPARSED*)
      realloc (pref.menu, (pref.menuItems+1)*sizeof(MENUPARSED));
  else
    pref.menu = (MENUPARSED*)malloc (sizeof(MENUPARSED));
  
  memcpy (pref.menu+pref.menuItems, &menu, sizeof(MENUPARSED));
  pref.menuItems++;

  memset (&menu, 0, sizeof(MENUPARSED));
}

static void 
OpenIcons (void)
{
  if (pref.icon != NULL) {
    ErrorF("LoadPreferences: Redefining icon mappings\n");
    free(pref.icon);
    pref.icon = NULL;
  }
  pref.iconItems = 0;
}

static void
AddIconLine (char *matchstr, char *iconfile)
{
  if (pref.icon==NULL)
    pref.icon = (ICONITEM*)malloc(sizeof(ICONITEM));
  else
    pref.icon = (ICONITEM*)
      realloc(pref.icon, sizeof(ICONITEM)*(pref.iconItems+1));

  strncpy(pref.icon[pref.iconItems].match, matchstr, MENU_MAX);
  pref.icon[pref.iconItems].match[MENU_MAX] = 0;

  strncpy(pref.icon[pref.iconItems].iconFile, iconfile, PATH_MAX+NAME_MAX+1);
  pref.icon[pref.iconItems].iconFile[PATH_MAX+NAME_MAX+1] = 0;

  pref.icon[pref.iconItems].hicon = 0;

  pref.iconItems++;
}

static void 
CloseIcons (void)
{
}

static void
OpenSysMenu (void)
{
  if (pref.sysMenu != NULL) {
    ErrorF("LoadPreferences: Redefining system menu\n");
    free(pref.sysMenu);
    pref.sysMenu = NULL;
  }
  pref.sysMenuItems = 0;
}

static void
AddSysMenuLine (char *matchstr, char *menuname, int pos)
{
  if (pref.sysMenu==NULL)
    pref.sysMenu = (SYSMENUITEM*)malloc(sizeof(SYSMENUITEM));
  else
    pref.sysMenu = (SYSMENUITEM*)
      realloc(pref.sysMenu, sizeof(SYSMENUITEM)*(pref.sysMenuItems+1));

  strncpy (pref.sysMenu[pref.sysMenuItems].match, matchstr, MENU_MAX);
  pref.sysMenu[pref.sysMenuItems].match[MENU_MAX] = 0;

  strncpy (pref.sysMenu[pref.sysMenuItems].menuName, menuname, MENU_MAX);
  pref.sysMenu[pref.sysMenuItems].menuName[MENU_MAX] = 0;

  pref.sysMenu[pref.sysMenuItems].menuPos = pos;

  pref.sysMenuItems++;
}

static void
CloseSysMenu (void)
{
}


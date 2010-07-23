/*
 * Copyright (c) 2005 Alexander Gottwald
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the sale,
 * use or other dealings in this Software without prior written authorization.
 */
/* Definitions for various keyboard layouts from windows and their 
 * XKB settings.
 */

typedef struct 
{
    unsigned int winlayout;
    int winkbtype;
    char *xkbmodel;
    char *xkblayout;
    char *xkbvariant;
    char *xkboptions;
    char *layoutname;
} WinKBLayoutRec, *WinKBLayoutPtr;

/*
   This table is sorted by low byte of winlayout, then by next byte, etc.
*/

WinKBLayoutRec winKBLayouts[] = 
{
    {  0x404, -1, "pc105", "zh_TW",   NULL, NULL, "Chinese (Taiwan)"},
    {  0x405, -1, "pc105", "cz",      NULL, NULL, "Czech"},
    {0x10405, -1, "pc105", "cz_qwerty", NULL, NULL, "Czech (QWERTY)"},
    {  0x406, -1, "pc105", "dk",      NULL, NULL, "Danish"},
    {  0x407, -1, "pc105", "de",      NULL, NULL, "German (Germany)"},
    {0x10407, -1, "pc105", "de",      NULL, NULL, "German (Germany, IBM)"},
    {  0x807, -1, "pc105", "ch",      "de", NULL, "German (Switzerland)"},
    {  0x409, -1, "pc105", "us",      NULL, NULL, "English (USA)"},
    {0x10409, -1, "pc105", "dvorak",  NULL, NULL, "English (USA, Dvorak)"}, 
    {0x20409, -1, "pc105", "us_intl", NULL, NULL, "English (USA, International)"}, 
    {  0x809, -1, "pc105", "gb",      NULL, NULL, "English (United Kingdom)"},
    { 0x1009, -1, "pc105", "ca",      "fr", NULL, "French (Canada)"},
    {0x11009, -1, "pc105", "ca",      "multix", NULL, "Candian Multilingual Standard"},
    { 0x1809, -1, "pc105", "ie",      NULL, NULL, "Irish"},
    {  0x40a, -1, "pc105", "es",      NULL, NULL, "Spanish (Spain, Traditional Sort)"},
    {  0x80a, -1, "pc105", "latam",   NULL, NULL, "Latin American"},
    {  0x40b, -1, "pc105", "fi",      NULL, NULL, "Finnish"},
    {  0x40c, -1, "pc105", "fr",      NULL, NULL, "French (Standard)"},
    {  0x80c, -1, "pc105", "be",      NULL, NULL, "French (Belgian)"},
    {0x1080c, -1, "pc105", "be",      NULL, NULL, "Belgian (Comma)"},
    {  0xc0c, -1, "pc105", "ca",      "fr-legacy", NULL, "French (Canada) (Legacy)"},
    { 0x100c, -1, "pc105", "ch",      "fr", NULL, "French (Switzerland)"},
    {  0x40d, -1, "pc105", "il",      NULL, NULL, "Hebrew"},
    {  0x40e, -1, "pc105", "hu",      NULL, NULL, "Hungarian"},
    {  0x40f, -1, "pc105", "is",      NULL, NULL, "Icelandic"},
    {  0x410, -1, "pc105", "it",      NULL, NULL, "Italian"},
    {0x10410, -1, "pc105", "it",      NULL, NULL, "Italian (142)"},
    {0xa0000410,-1, "macbook79","it",   "mac",NULL, "Italiano (Apple)"},
    {  0x411,  7, "jp106", "jp",      NULL, NULL, "Japanese"},
    {  0x413, -1, "pc105", "nl",      NULL, NULL, "Dutch"},
    {  0x813, -1, "pc105", "be",      NULL, NULL, "Dutch (Belgian)"},  
    {  0x414, -1, "pc105", "no",      NULL, NULL, "Norwegian"},
    {  0x415, -1, "pc105", "pl",      NULL, NULL, "Polish (Programmers)"},
    {  0x416, -1, "pc105", "br",      NULL, NULL, "Portuguese (Brazil, ABNT)"},
    {0x10416, -1, "abnt2", "br",      NULL, NULL, "Portuguese (Brazil, ABNT2)"},
    {  0x816, -1, "pc105", "pt",      NULL, NULL, "Portuguese (Portugal)"},
    {  0x41a, -1, "pc105", "hr",      NULL, NULL, "Croatian"},
    {  0x41d, -1, "pc105", "se",      NULL, NULL, "Swedish (Sweden)"},
    {  0x424, -1, "pc105", "si",      NULL, NULL, "Slovenian"},
    {  0x425, -1, "pc105", "ee",      NULL, NULL, "Estonian"},
    {  0x452, -1, "pc105", "gb",      "intl", NULL, "United Kingdom (Extended)"},
    {     -1, -1, NULL,    NULL,      NULL, NULL, NULL}
};

/*
  See http://technet.microsoft.com/en-us/library/cc766503%28WS.10%29.aspx
  for a listing of input locale (keyboard layout) codes
*/    



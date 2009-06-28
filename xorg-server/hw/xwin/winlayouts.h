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

WinKBLayoutRec winKBLayouts[] = 
{
    {  0x405, -1, "pc105", "cz",      NULL, NULL, "Czech"},
    {0x10405, -1, "pc105", "cz_qwerty", NULL, NULL, "Czech (QWERTY)"},
    {  0x406, -1, "pc105", "dk",      NULL, NULL, "Danish"},
    {  0x407, -1, "pc105", "de",      NULL, NULL, "German (Germany)"},
    {0x10407, -1, "pc105", "de",      NULL, NULL, "German (Germany, IBM)"},
    {  0x807, -1, "pc105", "de_CH",   NULL, NULL, "German (Switzerland)"},
    {  0x409, -1, "pc105", "us",      NULL, NULL, "English (USA)"},
    {0x10409, -1, "pc105", "dvorak",  NULL, NULL, "English (USA, Dvorak)"}, 
    {0x20409, -1, "pc105", "us_intl", NULL, NULL, "English (USA, International)"}, 
    {  0x809, -1, "pc105", "gb",      NULL, NULL, "English (United Kingdom)"},
    { 0x1809, -1, "pc105", "ie",      NULL, NULL, "Irish"},
    {  0x40a, -1, "pc105", "es",      NULL, NULL, "Spanish (Spain, Traditional Sort)"},
    {  0x40b, -1, "pc105", "fi",      NULL, NULL, "Finnish"},
    {  0x40c, -1, "pc105", "fr",      NULL, NULL, "French (Standard)"},
    {  0x80c, -1, "pc105", "be",      NULL, NULL, "French (Belgian)"},
    {  0xc0c, -1, "pc105", "ca_enhanced", NULL, NULL, "French (Canada)"},
    { 0x100c, -1, "pc105", "fr_CH",   NULL, NULL, "French (Switzerland)"},
    {  0x40e, -1, "pc105", "hu",      NULL, NULL, "Hungarian"},
    {  0x410, -1, "pc105", "it",      NULL, NULL, "Italian"},
    {  0x411,  7, "jp106", "jp",      NULL, NULL, "Japanese"},
    {  0x813, -1, "pc105", "be",      NULL, NULL, "Dutch (Belgian)"},  
    {  0x414, -1, "pc105", "no",      NULL, NULL, "Norwegian"},
    {  0x416, -1, "pc105", "br",      NULL, NULL, "Portuguese (Brazil, ABNT)"},
    {0x10416, -1, "abnt2", "br",      NULL, NULL, "Portuguese (Brazil, ABNT2)"},
    {  0x816, -1, "pc105", "pt",      NULL, NULL, "Portuguese (Portugal)"},
    {  0x41d, -1, "pc105", "se",      NULL, NULL, "Swedish (Sweden)"},
    {     -1, -1, NULL,    NULL,      NULL, NULL, NULL}
};

/* Listing of language codes from MSDN */
/*
Support ID       XKB        Language
====================================================================
   ?    0x0000              Language Neutral
   ?    0x0400              Process or User Default Language
   ?    0x0800              System Default Language
        0x0401              Arabic (Saudi Arabia)
        0x0801              Arabic (Iraq)
        0x0c01              Arabic (Egypt)
        0x1001              Arabic (Libya)
        0x1401              Arabic (Algeria)
        0x1801              Arabic (Morocco)
        0x1c01              Arabic (Tunisia)
        0x2001              Arabic (Oman)
        0x2401              Arabic (Yemen)
        0x2801              Arabic (Syria)
        0x2c01              Arabic (Jordan)
        0x3001              Arabic (Lebanon)
        0x3401              Arabic (Kuwait)
        0x3801              Arabic (U.A.E.)
        0x3c01              Arabic (Bahrain)
        0x4001              Arabic (Qatar)
                            Arabic (102) AZERTY        				
        0x0402              Bulgarian
        0x0403              Catalan
        0x0404              Chinese (Taiwan)
        0x0804              Chinese (PRC)
        0x0c04              Chinese (Hong Kong SAR, PRC)
        0x1004              Chinese (Singapore)
        0x1404              Chinese (Macao SAR) (98/ME,2K/XP)
   X    0x0405  cz          Czech
   X            cz_qwerty   Czech (QWERTY)
                            Czech (Programmers)
   X    0x0406  dk          Danish
   X    0x0407  de          German (Standard)
   X    0x0807  de_CH       German (Switzerland)
        0x0c07              German (Austria)
        0x1007              German (Luxembourg)
        0x1407              German (Liechtenstein)
        0x0408              Greek
   X    0x0409  us          English (United States)
   X    0x0809  gb          English (United Kingdom)
        0x0c09              English (Australian)
        0x1009              English (Canadian)
        0x1409              English (New Zealand)
   X    0x1809  ie          English (Ireland)
        0x1c09              English (South Africa)
        0x2009              English (Jamaica)
        0x2409              English (Caribbean)
        0x2809              English (Belize)
        0x2c09              English (Trinidad)
        0x3009              English (Zimbabwe) (98/ME,2K/XP)
        0x3409              English (Philippines) (98/ME,2K/XP)
   X    0x040a  es          Spanish (Spain, Traditional Sort)
        0x080a              Spanish (Mexican)
        0x0c0a              Spanish (Spain, Modern Sort)
        0x100a              Spanish (Guatemala)
        0x140a              Spanish (Costa Rica)
        0x180a              Spanish (Panama)
        0x1c0a              Spanish (Dominican Republic)
        0x200a              Spanish (Venezuela)
        0x240a              Spanish (Colombia)
        0x280a              Spanish (Peru)
        0x2c0a              Spanish (Argentina)
        0x300a              Spanish (Ecuador)
        0x340a              Spanish (Chile)
        0x380a              Spanish (Uruguay)
        0x3c0a              Spanish (Paraguay)
        0x400a              Spanish (Bolivia)
        0x440a              Spanish (El Salvador)
        0x480a              Spanish (Honduras)
        0x4c0a              Spanish (Nicaragua)
        0x500a              Spanish (Puerto Rico)
   X    0x040b  fi          Finnish
                            Finnish (with Sami)
   X    0x040c  fr          French (Standard)
   X    0x080c  be          French (Belgian)
   .    0x0c0c              French (Canadian)
                            French (Canadian, Legacy)
                            Canadian (Multilingual)
   X    0x100c  fr_CH       French (Switzerland)
        0x140c              French (Luxembourg)
        0x180c              French (Monaco) (98/ME,2K/XP)
        0x040d              Hebrew
   X    0x040e  hu          Hungarian
   .    0x040f              Icelandic
   X    0x0410  it          Italian (Standard)
        0x0810              Italian (Switzerland)
   X    0x0411  jp          Japanese
        0x0412              Korean
        0x0812              Korean (Johab) (95,NT)
   .    0x0413              Dutch (Netherlands)
   X    0x0813  be          Dutch (Belgium)
   X    0x0414  no          Norwegian (Bokmal)
        0x0814              Norwegian (Nynorsk)
   .    0x0415              Polish
   X    0x0416  br          Portuguese (Brazil)
   X    0x0816  pt          Portuguese (Portugal)
   .    0x0418              Romanian
        0x0419              Russian
   .    0x041a              Croatian
   .    0x081a              Serbian (Latin)
   .    0x0c1a              Serbian (Cyrillic)
        0x101a              Croatian (Bosnia and Herzegovina)
        0x141a              Bosnian (Bosnia and Herzegovina)
        0x181a              Serbian (Latin, Bosnia, and Herzegovina)
        0x1c1a              Serbian (Cyrillic, Bosnia, and Herzegovina)
   .    0x041b              Slovak
   .    0x041c              Albanian
   X    0x041d  se          Swedish
        0x081d              Swedish (Finland)
        0x041e              Thai
        0x041f              Turkish
        0x0420              Urdu (Pakistan) (98/ME,2K/XP) 
        0x0820              Urdu (India)
        0x0421              Indonesian
        0x0422              Ukrainian
        0x0423              Belarusian
   .    0x0424              Slovenian
        0x0425              Estonian
        0x0426              Latvian
        0x0427              Lithuanian
        0x0827              Lithuanian (Classic) (98)
        0x0429              Farsi
        0x042a              Vietnamese (98/ME,NT,2K/XP)
        0x042b              Armenian. This is Unicode only. (2K/XP)
                            Armenian Eastern
                            Armenian Western
        0x042c              Azeri (Latin)
        0x082c              Azeri (Cyrillic)
        0x042d              Basque
        0x042f              Macedonian (FYROM)
        0x0430              Sutu
        0x0432              Setswana/Tswana (South Africa)
        0x0434              isiXhosa/Xhosa (South Africa)
        0x0435              isiZulu/Zulu (South Africa)
        0x0436              Afrikaans
        0x0437              Georgian. This is Unicode only. (2K/XP)
   .    0x0438              Faeroese
        0x0439              Hindi. This is Unicode only. (2K/XP)
        0x043a              Maltese (Malta)
        0x043b              Sami, Northern (Norway)
        0x083b              Sami, Northern (Sweden)
        0x0c3b              Sami, Northern (Finland)
        0x103b              Sami, Lule (Norway)
        0x143b              Sami, Lule (Sweden)
        0x183b              Sami, Southern (Norway)
        0x1c3b              Sami, Southern (Sweden)
        0x203b              Sami, Skolt (Finland)
        0x243b              Sami, Inari (Finland)
        0x043e              Malay (Malaysian)
        0x083e              Malay (Brunei Darussalam)
        0x0440              Kyrgyz. (XP)
        0x0441              Swahili (Kenya)
        0x0443              Uzbek (Latin)
        0x0843              Uzbek (Cyrillic)
        0x0444              Tatar (Tatarstan)
        0x0445              Bengali (India)
                            Bengali (Inscript)
        0x0446              Punjabi. This is Unicode only. (XP)
        0x0447              Gujarati. This is Unicode only. (XP)
        0x0449              Tamil. This is Unicode only. (2K/XP)
        0x044a              Telugu. This is Unicode only. (XP)
        0x044b              Kannada. This is Unicode only. (XP)
        0x044c              Malayalam (India)
        0x044e              Marathi. This is Unicode only. (2K/XP)
        0x044f              Sanskrit. This is Unicode only. (2K/XP)
        0x0450              Mongolian (XP)
        0x0452              Welsh (United Kingdom)
        0x0455              Burmese
        0x0456              Galician (XP)
        0x0457              Konkani. This is Unicode only. (2K/XP)
        0x045a              Syriac. This is Unicode only. (XP)
        0x0465              Divehi. This is Unicode only. (XP)
                            Divehi (Phonetic)
                            Divehi (Typewriter)
        0x046b              Quechua (Bolivia)
        0x086b              Quechua (Ecuador)
        0x0c6b              Quechua (Peru)
        0x046c              Sesotho sa Leboa/Northern Sotho (South Africa)
        0x007f              LOCALE_INVARIANT. See MAKELCID.
        0x0481              Maori (New Zealand)
*/    



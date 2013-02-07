/* DON'T REORDER!  The order is part of the cache signature. */
FC_OBJECT (FAMILY,		FcTypeString,	FcCompareFamily)
FC_OBJECT (FAMILYLANG,		FcTypeString,	NULL)
FC_OBJECT (STYLE,		FcTypeString,	FcCompareString)
FC_OBJECT (STYLELANG,		FcTypeString,	NULL)
FC_OBJECT (FULLNAME,		FcTypeString,	NULL)
FC_OBJECT (FULLNAMELANG,	FcTypeString,	NULL)
FC_OBJECT (SLANT,		FcTypeInteger,	FcCompareNumber)
FC_OBJECT (WEIGHT,		FcTypeInteger,	FcCompareNumber)
FC_OBJECT (WIDTH,		FcTypeInteger,	FcCompareNumber)
FC_OBJECT (SIZE,		FcTypeDouble,	NULL)
FC_OBJECT (ASPECT,		FcTypeDouble,	NULL)
FC_OBJECT (PIXEL_SIZE,		FcTypeDouble,	FcCompareSize)
FC_OBJECT (SPACING,		FcTypeInteger,	FcCompareNumber)
FC_OBJECT (FOUNDRY,		FcTypeString,	FcCompareString)
FC_OBJECT (ANTIALIAS,		FcTypeBool,	FcCompareBool)
FC_OBJECT (HINT_STYLE,		FcTypeInteger,	NULL)
FC_OBJECT (HINTING,		FcTypeBool,	NULL)
FC_OBJECT (VERTICAL_LAYOUT,	FcTypeBool,	NULL)
FC_OBJECT (AUTOHINT,		FcTypeBool,	NULL)
FC_OBJECT (GLOBAL_ADVANCE,	FcTypeBool,	NULL)	/* deprecated */
FC_OBJECT (FILE,		FcTypeString,	FcCompareFilename)
FC_OBJECT (INDEX,		FcTypeInteger,	NULL)
FC_OBJECT (RASTERIZER,		FcTypeString,	FcCompareString)
FC_OBJECT (OUTLINE,		FcTypeBool,	FcCompareBool)
FC_OBJECT (SCALABLE,		FcTypeBool,	NULL)
FC_OBJECT (DPI,			FcTypeDouble,	NULL)
FC_OBJECT (RGBA,		FcTypeInteger,	NULL)
FC_OBJECT (SCALE,		FcTypeDouble,	NULL)
FC_OBJECT (MINSPACE,		FcTypeBool,	NULL)
FC_OBJECT (CHAR_WIDTH,		FcTypeInteger,	NULL)
FC_OBJECT (CHAR_HEIGHT,		FcTypeInteger,	NULL)
FC_OBJECT (MATRIX,		FcTypeMatrix,	NULL)
FC_OBJECT (CHARSET,		FcTypeCharSet,	FcCompareCharSet)
FC_OBJECT (LANG,		FcTypeLangSet,	FcCompareLang)
FC_OBJECT (FONTVERSION,		FcTypeInteger,	FcCompareNumber)
FC_OBJECT (CAPABILITY,		FcTypeString,	NULL)
FC_OBJECT (FONTFORMAT,		FcTypeString,	NULL)
FC_OBJECT (EMBOLDEN,		FcTypeBool,	NULL)
FC_OBJECT (EMBEDDED_BITMAP,	FcTypeBool,	NULL)
FC_OBJECT (DECORATIVE,		FcTypeBool,	FcCompareBool)
FC_OBJECT (LCD_FILTER,		FcTypeInteger,	NULL)
FC_OBJECT (NAMELANG,		FcTypeString,	NULL)
FC_OBJECT (FONT_FEATURES,	FcTypeString,	NULL)
FC_OBJECT (PRGNAME,		FcTypeString,	NULL)
FC_OBJECT (HASH,		FcTypeString,	FcCompareString)
/* ^-------------- Add new objects here. */

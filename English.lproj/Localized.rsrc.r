data 'vers' (1, purgeable) {
	$"0120 8000 0000 0431 2E32 3020 312E 3230"            /* . �....1.20 1.20 */
	$"2028 6329 206E 616F 6B69 2069 696D 7572"            /*  (c) naoki iimur */
	$"612C 2031 3939 372D 3230 3035"                      /* a, 1997-2005 */
};

data 'vers' (2, purgeable) {
	$"0120 8000 0000 0431 2E32 3026 3235 3620"            /* . �....1.20&256  */
	$"636F 6C6F 7273 2070 6169 6E74 696E 6720"            /* colors painting  */
	$"746F 6F6C 2026 2069 636F 6E20 6564 6974"            /* tool & icon edit */
	$"6F72"                                               /* or */
};

data 'STR#' (128, "Undo", purgeable) {
	$"001F 0A43 616E 2774 2055 6E64 6F0A 556E"            /* ..�Can't Undo�Un */
	$"646F 2050 6169 6E74 0C55 6E64 6F20 4D61"            /* do Paint.Undo Ma */
	$"7271 7565 6509 556E 646F 204D 6F76 650B"            /* rquee�Undo Move. */
	$"556E 646F 2045 6666 6563 740D 556E 646F"            /* Undo Effect.Undo */
	$"2044 6573 656C 6563 740E 556E 646F 2044"            /*  Deselect.Undo D */
	$"7570 6C69 6361 7465 0855 6E64 6F20 4375"            /* uplicate.Undo Cu */
	$"740A 556E 646F 2050 6173 7465 0A55 6E64"            /* t�Undo Paste�Und */
	$"6F20 436C 6561 720D 556E 646F 2045 7863"            /* o Clear.Undo Exc */
	$"6861 6E67 650B 556E 646F 2052 6573 697A"            /* hange.Undo Resiz */
	$"650B 556E 646F 2052 6F74 6174 6511 556E"            /* e.Undo Rotate.Un */
	$"646F 2043 6861 6E67 6520 436F 6C6F 7212"            /* do Change Color. */
	$"556E 646F 2052 6570 6C61 6365 2043 6F6C"            /* Undo Replace Col */
	$"6F72 0955 6E64 6F20 466C 6970 0F55 6E64"            /* or�Undo Flip.Und */
	$"6F20 496E 7075 7420 5465 7874 1055 6E64"            /* o Input Text.Und */
	$"6F20 4368 616E 6765 204D 6173 6B0A 556E"            /* o Change Mask�Un */
	$"646F 2050 6169 6E74 1355 6E64 6F20 4164"            /* do Paint.Undo Ad */
	$"6420 4261 636B 6772 6F75 6E64 1355 6E64"            /* d Background.Und */
	$"6F20 4164 6420 466F 7265 6772 6F75 6E64"            /* o Add Foreground */
	$"1155 6E64 6F20 556E 696F 6E20 4C61 7965"            /* .Undo Union Laye */
	$"7273 1655 6E64 6F20 4465 6C65 7465 2046"            /* rs.Undo Delete F */
	$"6F72 6567 726F 756E 6416 556E 646F 2044"            /* oreground.Undo D */
	$"656C 6574 6520 4261 636B 6772 6F75 6E64"            /* elete Background */
	$"1E55 6E64 6F20 4368 616E 6765 2046 6F72"            /* .Undo Change For */
	$"6567 726F 756E 6420 4F70 6163 6974 7910"            /* eground Opacity. */
	$"556E 646F 2041 6464 2049 636F 6E28 7329"            /* Undo Add Icon(s) */
	$"1355 6E64 6F20 4465 6C65 7465 2049 636F"            /* .Undo Delete Ico */
	$"6E28 7329 1555 6E64 6F20 4368 616E 6765"            /* n(s).Undo Change */
	$"2049 636F 6E20 496E 666F 1255 6E64 6F20"            /*  Icon Info.Undo  */
	$"5061 7374 6520 4963 6F6E 2873 2915 556E"            /* Paste Icon(s).Un */
	$"646F 2043 6861 6E67 6520 4963 6F6E 204E"            /* do Change Icon N */
	$"616D 6513 556E 646F 2044 656C 6574 6520"            /* ame.Undo Delete  */
	$"4963 6F6E 2873 29"                                  /* Icon(s) */
};

data 'STR#' (129, "Redo", purgeable) {
	$"001F 000A 5265 646F 2050 6169 6E74 0C52"            /* ...�Redo Paint.R */
	$"6564 6F20 4D61 7271 7565 6509 5265 646F"            /* edo Marquee�Redo */
	$"204D 6F76 650B 5265 646F 2045 6666 6563"            /*  Move.Redo Effec */
	$"740D 5265 646F 2044 6573 656C 6563 740E"            /* t.Redo Deselect. */
	$"5265 646F 2044 7570 6C69 6361 7465 0852"            /* Redo Duplicate.R */
	$"6564 6F20 4375 740A 5265 646F 2050 6173"            /* edo Cut�Redo Pas */
	$"7465 0A52 6564 6F20 436C 6561 720D 5265"            /* te�Redo Clear.Re */
	$"646F 2045 7863 6861 6E67 650B 5265 646F"            /* do Exchange.Redo */
	$"2052 6573 697A 650B 5265 646F 2052 6F74"            /*  Resize.Redo Rot */
	$"6174 6511 5265 646F 2043 6861 6E67 6520"            /* ate.Redo Change  */
	$"436F 6C6F 7212 5265 646F 2052 6570 6C61"            /* Color.Redo Repla */
	$"6365 2043 6F6C 6F72 0952 6564 6F20 466C"            /* ce Color�Redo Fl */
	$"6970 0F52 6564 6F20 496E 7075 7420 5465"            /* ip.Redo Input Te */
	$"7874 1052 6564 6F20 4368 616E 6765 204D"            /* xt.Redo Change M */
	$"6173 6B0A 5265 646F 2050 6169 6E74 1352"            /* ask�Redo Paint.R */
	$"6564 6F20 4164 6420 4261 636B 6772 6F75"            /* edo Add Backgrou */
	$"6E64 1352 6564 6F20 4164 6420 466F 7265"            /* nd.Redo Add Fore */
	$"6772 6F75 6E64 1152 6564 6F20 556E 696F"            /* ground.Redo Unio */
	$"6E20 4C61 7965 7273 1652 6564 6F20 4465"            /* n Layers.Redo De */
	$"6C65 7465 2046 6F72 6567 726F 756E 6416"            /* lete Foreground. */
	$"5265 646F 2044 656C 6574 6520 4261 636B"            /* Redo Delete Back */
	$"6772 6F75 6E64 1E52 6564 6F20 4368 616E"            /* ground.Redo Chan */
	$"6765 2046 6F72 6567 726F 756E 6420 4F70"            /* ge Foreground Op */
	$"6163 6974 7910 5265 646F 2041 6464 2049"            /* acity.Redo Add I */
	$"636F 6E28 7329 1352 6564 6F20 4465 6C65"            /* con(s).Redo Dele */
	$"7465 2049 636F 6E28 7329 1552 6564 6F20"            /* te Icon(s).Redo  */
	$"4368 616E 6765 2049 636F 6E20 496E 666F"            /* Change Icon Info */
	$"1252 6564 6F20 5061 7374 6520 4963 6F6E"            /* .Redo Paste Icon */
	$"2873 2915 5265 646F 2043 6861 6E67 6520"            /* (s).Redo Change  */
	$"4963 6F6E 204E 616D 6513 5265 646F 2044"            /* Icon Name.Redo D */
	$"656C 6574 6520 4963 6F6E 2873 29"                   /* elete Icon(s) */
};

data 'STR#' (130, "Icon Type", purgeable) {
	$"000F 0620 6672 6F6D 200A 4C61 7267 6520"            /* ... from �Large  */
	$"3862 6974 0A4C 6172 6765 2034 6269 740A"            /* 8bit�Large 4bit� */
	$"4C61 7267 6520 3162 6974 0A4C 6172 6765"            /* Large 1bit�Large */
	$"204D 6173 6B0A 536D 616C 6C20 3862 6974"            /*  Mask�Small 8bit */
	$"0A53 6D61 6C6C 2034 6269 740A 536D 616C"            /* �Small 4bit�Smal */
	$"6C20 3142 6974 0A53 6D61 6C6C 204D 6173"            /* l 1Bit�Small Mas */
	$"6B0B 4C61 7267 6520 3332 6269 740F 4C61"            /* k.Large 32bit.La */
	$"7267 6520 3862 6974 206D 6173 6B0B 536D"            /* rge 8bit mask.Sm */
	$"616C 6C20 3332 6269 740F 536D 616C 6C20"            /* all 32bit.Small  */
	$"3862 6974 206D 6173 6B0F 5468 756D 626E"            /* 8bit mask.Thumbn */
	$"6169 6C20 3332 6269 7413 5468 756D 626E"            /* ail 32bit.Thumbn */
	$"6169 6C20 3862 6974 206D 6173 6B"                   /* ail 8bit mask */
};

data 'STR#' (131, "Suffix", purgeable) {
	$"0004 052E 7069 6374 042E 706E 6705 2E69"            /* ....pict..png..i */
	$"636F 6E04 2E69 636F"                                /* con..ico */
};

data 'STR#' (132, "untitled", purgeable) {
	$"0001 0875 6E74 6974 6C65 64"                        /* ...untitled */
};

data 'STR#' (133, "Confirm Save", purgeable) {
	$"0006 0743 6C6F 7369 6E67 0851 7569 7474"            /* ...Closing.Quitt */
	$"696E 6707 5069 6374 7572 6508 5265 736F"            /* ing.Picture.Reso */
	$"7572 6365 1249 636F 6E20 4661 6D69 6C79"            /* urce.Icon Family */
	$"2057 696E 646F 7706 466F 6C64 6572"                 /*  Window.Folder */
};

data 'STR#' (134, "Preferences File", purgeable) {
	$"0001 0F49 636F 6E50 6172 7479 2050 7265"            /* ...IconParty Pre */
	$"6673"                                               /* fs */
};

data 'STR#' (136, "About", purgeable) {
	$"0005 0B55 7365 6420 436F 756E 743A 0A55"            /* ...Used Count:�U */
	$"7365 6420 5469 6D65 3A0C 4672 6565 204D"            /* sed Time:.Free M */
	$"656D 6F72 793A 0620 7469 6D65 7303 206B"            /* emory:. times. k */
	$"62"                                                 /* b */
};

data 'STR#' (137, "New", purgeable) {
	$"0002 034E 6577 064E 6577 2E2E 2E"                   /* ...New.New... */
};

data 'STR#' (139, "icons from", purgeable) {
	$"0003 0B49 636F 6E73 2066 726F 6D20 0D49"            /* ...Icons from .I */
	$"4420 3D20 5E30 2066 726F 6D20 0F43 7573"            /* D = ^0 from .Cus */
	$"746F 6D20 6963 6F6E 206F 6620"                      /* tom icon of  */
};

data 'STR#' (140, "Export Icon(s)", purgeable) {
	$"0003 0A49 636F 6E20 4E61 6D65 3A04 2069"            /* ..�Icon Name:. i */
	$"643D 0549 636F 6E0D"                                /* d=.Icon. */
};

data 'STR#' (141, "Icon Name", purgeable) {
	$"0005 0875 6E74 6974 6C65 6400 0520 636F"            /* ...untitled.. co */
	$"7079 084E 6577 2049 636F 6E06 4372 6561"            /* py.New Icon.Crea */
	$"7465"                                               /* te */
};

data 'STR#' (142, "Hide Edges / Show Edges", purgeable) {
	$"0002 0A48 6964 6520 4564 6765 730A 5368"            /* ..�Hide Edges�Sh */
	$"6F77 2045 6467 6573"                                /* ow Edges */
};

data 'STR#' (143, "Preferences Tab", purgeable) {
	$"0002 2246 696C 650D 5061 696E 740D 546F"            /* .."File.Paint.To */
	$"6F6C 730D 504E 470D 5461 626C 6574 0D4F"            /* ols.PNG.Tablet.O */
	$"7468 6572 731B 4669 6C65 0D50 6169 6E74"            /* thers.File.Paint */
	$"0D54 6F6F 6C73 0D50 4E47 0D4F 7468 6572"            /* .Tools.PNG.Other */
	$"73"                                                 /* s */
};

data 'STR#' (144, "Temporary File", purgeable) {
	$"0001 0E49 636F 6E50 6172 7479 2074 656D"            /* ...IconParty tem */
	$"70"                                                 /* p */
};

data 'STR#' (145, "Create Resource", purgeable) {
	$"0002 0F52 6573 6F75 7263 6520 4669 6C65"            /* ...Resource File */
	$"203A 0D75 6E74 6974 6C65 642E 7273 7263"            /*  :.untitled.rsrc */
};

data 'STR#' (146, "Zoom", purgeable) {
	$"0005 0331 3030 0332 3030 0334 3030 0338"            /* ...100.200.400.8 */
	$"3030 0431 3630 30"                                  /* 00.1600 */
};

data 'STR#' (147, "Icon Name", purgeable) {
	$"0001 0B43 7573 746F 6D20 4963 6F6E"                 /* ...Custom Icon */
};

data 'STR#' (148, "Layer Menu", purgeable) {
	$"0005 0F48 6964 6520 4261 636B 6772 6F75"            /* ...Hide Backgrou */
	$"6E64 0F53 686F 7720 4261 636B 6772 6F75"            /* nd.Show Backgrou */
	$"6E64 0E41 6464 2046 6F72 6567 726F 756E"            /* nd.Add Foregroun */
	$"640E 4164 6420 4261 636B 6772 6F75 6E64"            /* d.Add Background */
	$"1746 6F72 6567 726F 756E 6420 2F20 4261"            /* .Foreground / Ba */
	$"636B 6772 6F75 6E64"                                /* ckground */
};

data 'STR#' (149, "Destroy 8.5 icons", purgeable) {
	$"0002 7E54 6869 7320 666F 6C64 6572 2866"            /* ..~This folder(f */
	$"696C 6529 2069 636F 6E20 636F 6E74 6169"            /* ile) icon contai */
	$"6E73 2033 3262 6974 7320 6963 6F6E 206F"            /* ns 32bits icon o */
	$"7220 3862 6974 7320 6D61 736B 2074 6861"            /* r 8bits mask tha */
	$"7420 4963 6F6E 5061 7274 7920 6361 6E20"            /* t IconParty can  */
	$"6E6F 7420 6564 6974 2E20 446F 2079 6F75"            /* not edit. Do you */
	$"2077 6973 6820 746F 2064 6573 7472 6F79"            /*  wish to destroy */
	$"2074 6865 7365 2069 636F 6E20 6461 7461"            /*  these icon data */
	$"3F69 5468 6973 2066 6F6C 6465 7228 6669"            /* ?iThis folder(fi */
	$"6C65 2920 6963 6F6E 2063 6F6E 7461 696E"            /* le) icon contain */
	$"7320 7468 6520 6963 6F6E 7320 666F 7220"            /* s the icons for  */
	$"4D61 634F 5320 382E 3520 6F72 204C 6174"            /* MacOS 8.5 or Lat */
	$"6572 2E20 446F 2079 6F75 2077 6973 6820"            /* er. Do you wish  */
	$"746F 2064 6573 7472 6F79 2074 6865 7365"            /* to destroy these */
	$"2069 636F 6E20 6461 7461 3F"                        /*  icon data? */
};

data 'STR#' (3000, "Help for Paint Window", purgeable) {
	$"0003 7E50 6169 6E74 2057 696E 646F 770D"            /* ..~Paint Window. */
	$"0D54 6865 2077 696E 646F 7720 666F 7220"            /* .The window for  */
	$"7061 696E 7469 6E67 2E20 5468 6973 2077"            /* painting. This w */
	$"696E 646F 7720 6973 2064 6973 706C 6179"            /* indow is display */
	$"6564 2077 6865 6E20 796F 7520 6F70 656E"            /* ed when you open */
	$"2050 4943 5420 6669 6C65 7320 6F72 2069"            /*  PICT files or i */
	$"636F 6E73 2066 726F 6D20 7468 6520 4963"            /* cons from the Ic */
	$"6F6E 2046 616D 696C 7920 5769 6E64 6F77"            /* on Family Window */
	$"2E95 5261 7469 6F20 6F66 2050 6169 6E74"            /* .�Ratio of Paint */
	$"2057 696E 646F 770D 0D44 6973 706C 6179"            /*  Window..Display */
	$"7320 7468 6520 7261 7469 6F20 6F66 2074"            /* s the ratio of t */
	$"6865 2050 6169 6E74 2057 696E 646F 772E"            /* he Paint Window. */
	$"2020 436C 6963 6B20 6865 7265 2C20 616E"            /*   Click here, an */
	$"6420 6469 7370 6C61 7973 2074 6865 2070"            /* d displays the p */
	$"6F70 7570 206D 656E 752E 2059 6F75 2063"            /* opup menu. You c */
	$"616E 2063 6861 6E67 6520 7468 6520 7261"            /* an change the ra */
	$"7469 6F20 6672 6F6D 2031 3030 2520 746F"            /* tio from 100% to */
	$"2031 3630 3025 2EC8 436F 6E64 6974 696F"            /*  1600%.�Conditio */
	$"6E20 6F66 2066 6F72 6567 726F 756E 642F"            /* n of foreground/ */
	$"6261 636B 6772 6F75 6E64 0D0D 4469 7370"            /* background..Disp */
	$"6C61 7973 2074 6865 2063 6F6E 6469 7469"            /* lays the conditi */
	$"6F6E 206F 6620 666F 7265 6772 6F75 6E64"            /* on of foreground */
	$"2F62 6163 6B67 726F 756E 642E 0D0D 436C"            /* /background...Cl */
	$"6963 6B20 6865 7265 2074 6F20 6368 616E"            /* ick here to chan */
	$"6765 2066 6F72 6567 726F 756E 642F 6261"            /* ge foreground/ba */
	$"636B 6772 6F75 6E64 206D 6F64 652E 2050"            /* ckground mode. P */
	$"7265 7373 206D 6F75 7365 2062 7574 746F"            /* ress mouse butto */
	$"6E20 6865 7265 2066 6F72 2061 2077 6869"            /* n here for a whi */
	$"6C65 2C20 616E 6420 6469 7370 6C61 7973"            /* le, and displays */
	$"2074 6865 2070 6F70 7570 206D 656E 752E"            /*  the popup menu. */
};

data 'STR#' (3001, "Help for Preview Window", purgeable) {
	$"0001 CB50 7265 7669 6577 2057 696E 646F"            /* ..�Preview Windo */
	$"770D 0D57 6865 6E20 7468 6520 5061 696E"            /* w..When the Pain */
	$"7420 5769 6E64 6F77 2069 7320 6163 7469"            /* t Window is acti */
	$"7665 2C20 6469 7370 6C61 7973 2074 6865"            /* ve, displays the */
	$"2070 6169 6E74 696E 6720 7069 6374 7572"            /*  painting pictur */
	$"6520 696E 206F 7269 6769 6E61 6C20 7369"            /* e in original si */
	$"7A65 2E0D 0D57 6865 6E20 7468 6520 4963"            /* ze...When the Ic */
	$"6F6E 204C 6973 7420 5769 6E64 6F77 206F"            /* on List Window o */
	$"7220 7468 6520 4963 6F6E 2046 616D 696C"            /* r the Icon Famil */
	$"7920 5769 6E64 6F77 2069 7320 6163 7469"            /* y Window is acti */
	$"7665 2C20 6469 7370 6C61 7973 2074 6865"            /* ve, displays the */
	$"2070 7265 7669 6577 206F 6620 7468 6520"            /*  preview of the  */
	$"7365 6C65 6374 6564 2069 636F 6E2E"                 /* selected icon. */
};

data 'STR#' (3002, "Help for Icon List Window", purgeable) {
	$"0001 9549 636F 6E20 4C69 7374 2057 696E"            /* ..�Icon List Win */
	$"646F 770D 0D44 6973 706C 6179 7320 7468"            /* dow..Displays th */
	$"6520 6C69 7374 206F 6620 6963 6F6E 7320"            /* e list of icons  */
	$"696E 2074 6865 2049 636F 6E20 4669 6C65"            /* in the Icon File */
	$"732E 0D59 6F75 2063 616E 2065 6469 7420"            /* s..You can edit  */
	$"7468 6520 6963 6F6E 732C 2061 6E64 2069"            /* the icons, and i */
	$"6D70 6F72 7428 6578 706F 7274 2920 6963"            /* mport(export) ic */
	$"6F6E 7320 6672 6F6D 2874 6F29 2046 696E"            /* ons from(to) Fin */
	$"6465 7220 6279 2064 7261 672D 6E2D 6472"            /* der by drag-n-dr */
	$"6F70 2068 6572 652E"                                /* op here. */
};

data 'STR#' (3003, "Help For Icon Family Window", purgeable) {
	$"0001 AA49 636F 6E20 4661 6D69 6C79 2057"            /* ..�Icon Family W */
	$"696E 646F 770D 0D44 6973 706C 6179 7320"            /* indow..Displays  */
	$"7468 6520 6963 6F6E 7320 6F66 2074 6865"            /* the icons of the */
	$"2049 636F 6E20 4C69 7374 2057 696E 646F"            /*  Icon List Windo */
	$"772C 206F 7220 7468 6520 6375 7374 6F6D"            /* w, or the custom */
	$"2069 636F 6E73 206F 6620 6669 6C65 732F"            /*  icons of files/ */
	$"666F 6C64 6572 732E 2059 6F75 2063 616E"            /* folders. You can */
	$"2063 6861 6E67 6520 7468 6520 6E61 6D65"            /*  change the name */
	$"206F 6620 7468 6520 6963 6F6E 732C 2061"            /*  of the icons, a */
	$"6E64 2076 6965 7720 616E 6420 6564 6974"            /* nd view and edit */
	$"2069 636F 6E20 696D 6167 6573 2E"                   /*  icon images. */
};

data 'STR#' (3004, "Help for Color Palettes", purgeable) {
	$"0001 C743 6F6C 6F72 2050 616C 6574 7465"            /* ..�Color Palette */
	$"0D0D 596F 7520 6361 6E20 7365 6C65 6374"            /* ..You can select */
	$"2074 6865 2066 6F72 6567 726F 756E 6428"            /*  the foreground( */
	$"5065 6E29 2063 6F6C 6F72 2074 6861 7420"            /* Pen) color that  */
	$"7573 6564 2069 6E20 7061 696E 7469 6E67"            /* used in painting */
	$"2077 6974 6820 5065 6E63 696C 2074 6F6F"            /*  with Pencil too */
	$"6C20 616E 6420 4275 636B 6574 2074 6F6F"            /* l and Bucket too */
	$"6C2E 2049 6620 796F 7520 636C 6963 6B20"            /* l. If you click  */
	$"7468 6973 2070 616C 6574 7465 2077 6974"            /* this palette wit */
	$"6820 6F70 7469 6F6E 2D6B 6579 2C20 796F"            /* h option-key, yo */
	$"7520 6361 6E20 7365 6C65 6374 2074 6865"            /* u can select the */
	$"2062 6163 6B67 726F 756E 6428 4572 6173"            /*  background(Eras */
	$"6572 2920 636F 6C6F 722E"                           /* er) color. */
};

data 'STR#' (3005, "Help for Tool Palette", purgeable) {
	$"0005 1150 656E 6369 6C20 746F 6F6C 205B"            /* ...Pencil tool [ */
	$"505D 0D0D 1145 7261 7365 7220 746F 6F6C"            /* P]...Eraser tool */
	$"205B 455D 0D0D 124D 6172 7175 6565 2074"            /*  [E]...Marquee t */
	$"6F6F 6C20 5B4D 5D0D 0D18 5370 6F69 7420"            /* ool [M]...Spoit  */
	$"746F 6F6C 205B 4981 5E6F 7074 696F 6E5D"            /* tool [I�^option] */
	$"0D0D 1142 7563 6B65 7420 746F 6F6C 205B"            /* ...Bucket tool [ */
	$"4B5D 0D0D"                                          /* K].. */
};

data 'STR#' (3006, "Help for Blend Palette", purgeable) {
	$"0006 8550 656E 6369 6C20 636F 6C6F 720D"            /* ..�Pencil color. */
	$"0D44 6973 706C 6179 7320 7468 6520 666F"            /* .Displays the fo */
	$"7265 6772 6F75 6E64 2850 656E 6369 6C29"            /* reground(Pencil) */
	$"2063 6F6C 6F72 2E20 4279 2063 6C69 636B"            /*  color. By click */
	$"696E 6720 7468 6973 2061 7265 612C 2079"            /* ing this area, y */
	$"6F75 2063 616E 2063 6861 6E67 6520 7468"            /* ou can change th */
	$"6520 666F 7265 6772 6F75 6E64 2063 6F6C"            /* e foreground col */
	$"6F72 2077 6974 6820 7468 6520 436F 6C6F"            /* or with the Colo */
	$"7250 6963 6B65 722E 8545 7261 7365 7220"            /* rPicker.�Eraser  */
	$"636F 6C6F 720D 0D44 6973 706C 6179 7320"            /* color..Displays  */
	$"7468 6520 6261 636B 6772 6F75 6E64 2845"            /* the background(E */
	$"7261 7365 7229 2063 6F6C 6F72 2E20 4279"            /* raser) color. By */
	$"2063 6C69 636B 696E 6720 7468 6973 2061"            /*  clicking this a */
	$"7265 612C 2079 6F75 2063 616E 2063 6861"            /* rea, you can cha */
	$"6E67 6520 7468 6520 6261 636B 6772 6F75"            /* nge the backgrou */
	$"6E64 2063 6F6C 6F72 2077 6974 6820 7468"            /* nd color with th */
	$"6520 436F 6C6F 7250 6963 6B65 722E 8545"            /* e ColorPicker.�E */
	$"7863 6861 6E67 6520 5065 6E63 696C 2F45"            /* xchange Pencil/E */
	$"7261 7365 7220 636F 6C6F 7273 0D0D 4279"            /* raser colors..By */
	$"2063 6C69 636B 696E 6720 7468 6973 2061"            /*  clicking this a */
	$"7265 612C 2079 6F75 2063 616E 2065 7863"            /* rea, you can exc */
	$"6861 6E67 6520 7468 6520 666F 7265 6772"            /* hange the foregr */
	$"6F75 6E64 2850 656E 6369 6C29 2063 6F6C"            /* ound(Pencil) col */
	$"6F72 2061 6E64 2074 6865 2062 6163 6B67"            /* or and the backg */
	$"726F 756E 6428 4572 6173 6572 2920 636F"            /* round(Eraser) co */
	$"6C6F 722E C943 6F6E 6469 7469 6F6E 206F"            /* lor.�Condition o */
	$"6620 426C 656E 6420 5061 6C65 7474 650D"            /* f Blend Palette. */
	$"286C 6F63 6B65 642F 756E 6C6F 636B 6564"            /* (locked/unlocked */
	$"290D 0D44 6973 706C 6179 7320 7468 6520"            /* )..Displays the  */
	$"636F 6E64 6974 696F 6E20 6F66 2074 6865"            /* condition of the */
	$"2042 6C65 6E64 2050 616C 6574 7465 2028"            /*  Blend Palette ( */
	$"6C6F 636B 6564 206F 7220 756E 6C6F 636B"            /* locked or unlock */
	$"6564 292E 2049 6620 7468 6520 426C 656E"            /* ed). If the Blen */
	$"6420 5061 6C65 7474 6520 6973 206C 6F63"            /* d Palette is loc */
	$"6B65 642C 2020 7468 6520 426C 656E 6420"            /* ked,  the Blend  */
	$"4172 6561 206F 6620 7468 6520 426C 656E"            /* Area of the Blen */
	$"6420 5061 6C65 7474 6520 7769 6C6C 206E"            /* d Palette will n */
	$"6F74 2062 6520 7570 6461 7465 642E AA4C"            /* ot be updated.�L */
	$"6967 6874 6572 2063 6F6C 6F72 732F 4461"            /* ighter colors/Da */
	$"726B 6572 2063 6F6C 6F72 730D 0D44 6973"            /* rker colors..Dis */
	$"706C 6179 7320 7468 6520 6461 726B 656E"            /* plays the darken */
	$"6564 2063 6F6C 6F72 2061 6E64 206C 6967"            /* ed color and lig */
	$"6874 656E 6564 2063 6F6C 6F72 206F 6620"            /* htened color of  */
	$"7468 6520 666F 7265 6772 6F75 6E64 2850"            /* the foreground(P */
	$"656E 6369 6C29 2063 6F6C 6F72 2E20 4279"            /* encil) color. By */
	$"2063 6C69 636B 696E 6720 7468 6973 2061"            /*  clicking this a */
	$"7265 612C 2079 6F75 2063 616E 2063 6861"            /* rea, you can cha */
	$"6E67 6520 7468 6520 666F 7265 6772 6F75"            /* nge the foregrou */
	$"6E64 2063 6F6C 6F72 2EDB 426C 656E 6420"            /* nd color.�Blend  */
	$"636F 6C6F 7273 2062 6574 7765 656E 2050"            /* colors between P */
	$"656E 6369 6C20 636F 6C6F 7220 616E 6420"            /* encil color and  */
	$"5072 6576 696F 7573 2050 656E 6369 6C20"            /* Previous Pencil  */
	$"636F 6C6F 720D 0D44 6973 706C 6179 7320"            /* color..Displays  */
	$"7468 6520 626C 656E 6465 6420 636F 6C6F"            /* the blended colo */
	$"7220 6265 7477 6565 6E20 7468 6520 666F"            /* r between the fo */
	$"7265 6772 6F75 6E64 2850 656E 6369 6C29"            /* reground(Pencil) */
	$"2063 6F6C 6F72 2061 6E64 2074 6865 2070"            /*  color and the p */
	$"7265 7669 6F75 7320 666F 7265 6772 6F75"            /* revious foregrou */
	$"6E64 2063 6F6C 6F72 2E20 4279 2063 6C69"            /* nd color. By cli */
	$"636B 696E 6720 7468 6973 2061 7265 612C"            /* cking this area, */
	$"2079 6F75 2063 616E 2063 6861 6E67 6520"            /*  you can change  */
	$"7468 6520 666F 7265 6772 6F75 6E64 2063"            /* the foreground c */
	$"6F6C 6F72 2E"                                       /* olor. */
};

data 'STR#' (3007, "Help for Title", purgeable) {
	$"0001 3154 6974 6C65 2028 666F 7220 4865"            /* ..1Title (for He */
	$"6C70 290D 0D53 686F 7773 2074 6865 2076"            /* lp)..Shows the v */
	$"6572 7369 6F6E 206F 6620 4963 6F6E 5061"            /* ersion of IconPa */
	$"7274 792E"                                          /* rty. */
};

data 'STR#' (3008, "Help for Info Palette", purgeable) {
	$"0001 1549 6E66 6F72 6D61 7469 6F6E 2050"            /* ...Information P */
	$"616C 6574 7465 0D0D"                                /* alette.. */
};

data 'STR#' (3009, "Help for Pattern Palette", purgeable) {
	$"0001 3C50 6174 7465 726E 2050 616C 6574"            /* ..<Pattern Palet */
	$"7465 0D0D 5365 6C65 6374 2074 6865 2070"            /* te..Select the p */
	$"6174 7465 726E 2075 7365 6420 6279 2074"            /* attern used by t */
	$"6865 2042 7563 6B65 7420 746F 6F6C 2E"              /* he Bucket tool. */
};

data 'STR#' (3010, "Help for Scroll bars", purgeable) {
	$"0002 0D53 6372 6F6C 6C20 6261 7273 0D0D"            /* ...Scroll bars.. */
	$"0D53 6372 6F6C 6C20 6261 7273 0D0D"                 /* .Scroll bars.. */
};

data 'STR#' (3011, "Help for Size box", purgeable) {
	$"0001 0A53 697A 6520 626F 780D 0D"                   /* ..�Size box.. */
};

data 'STR#' (150, "Dot Palette", purgeable) {
	$"0003 0D44 6F74 204D 6F64 6520 466F 6E74"            /* ...Dot Mode Font */
	$"0B44 6F74 204C 6962 7261 7279 0E3C 6E6F"            /* .Dot Library.<no */
	$"7420 7365 6C65 6374 6564 3E"                        /* t selected> */
};

data 'STR#' (3012, "Help for Dot Mode Palette", purgeable) {
	$"0005 1244 6F74 204C 6962 7261 7279 206E"            /* ...Dot Library n */
	$"616D 650D 0D18 5265 636F 7264 696E 6720"            /* ame...Recording  */
	$"646F 7420 636F 6D6D 616E 6473 0D0D 1849"            /* dot commands...I */
	$"6E70 7574 7469 6E67 2064 6F74 2063 6F6D"            /* nputting dot com */
	$"6D61 6E64 730D 0D18 446F 7420 4C69 6272"            /* mands...Dot Libr */
	$"6172 7920 706F 7075 7020 6D65 6E75 0D0D"            /* ary popup menu.. */
	$"1844 6F74 2043 6F6D 6D61 6E64 2070 6F70"            /* .Dot Command pop */
	$"7570 206D 656E 750D 0D"                             /* up menu.. */
};

data 'STR#' (152, "Support URL (Amatubu)", purgeable) {
	$"0001 3468 7474 703A 2F2F 6870 2E76 6563"            /* ..4http://hp.vec */
	$"746F 722E 636F 2E6A 702F 6175 7468 6F72"            /* tor.co.jp/author */
	$"732F 5641 3031 3033 3138 2F69 6E64 6578"            /* s/VA010318/index */
	$"2D65 2E68 746D 6C"                                  /* -e.html */
};

data 'STR#' (153, "not found", purgeable) {
	$"0001 0B3C 6E6F 7420 666F 756E 643E"                 /* ...<not found> */
};

data 'STR#' (154, "Pen Color / Eraser Color", purgeable) {
	$"0002 1153 656C 6563 7420 5065 6E20 436F"            /* ...Select Pen Co */
	$"6C6F 723A 1453 656C 6563 7420 4572 6173"            /* lor:.Select Eras */
	$"6572 2043 6F6C 6F72 3A"                             /* er Color: */
};

data 'STR#' (155, "Blend File", purgeable) {
	$"0004 1042 6C65 6E64 2046 696C 6520 4E61"            /* ...Blend File Na */
	$"6D65 3A0D 756E 7469 746C 6564 2E62 6C6E"            /* me:.untitled.bln */
	$"6416 496E 7075 7420 426C 656E 6420 4669"            /* d.Input Blend Fi */
	$"6C65 204E 616D 652E 1253 656C 6563 7420"            /* le Name..Select  */
	$"426C 656E 6420 4669 6C65 2E"                        /* Blend File. */
};

data 'STR#' (156, "Add Icon", purgeable) {
	$"0001 5054 6869 7320 6669 6C65 2068 6173"            /* ..PThis file has */
	$"206E 6F20 7265 736F 7572 6365 2066 6F72"            /*  no resource for */
	$"6B2E 204F 7065 6E69 6E67 2069 7420 7769"            /* k. Opening it wi */
	$"6C6C 2061 6464 206F 6E65 2E20 446F 2079"            /* ll add one. Do y */
	$"6F75 2077 6973 6820 746F 206F 7065 6E20"            /* ou wish to open  */
	$"6974 3F"                                            /* it? */
};

data 'STR#' (157, "Edit Dot Lib", purgeable) {
	$"0002 0F4E 6577 2044 6F74 2043 6F6D 6D61"            /* ...New Dot Comma */
	$"6E64 1045 6469 7420 446F 7420 436F 6D6D"            /* nd.Edit Dot Comm */
	$"616E 64"                                            /* and */
};

data 'STR#' (138, "for custom save", purgeable) {
	$"0001 0B46 696C 6520 6E61 6D65 203A"                 /* ...File name : */
};

data 'STR#' (158, "for NavServices", purgeable) {
	$"0001 00"                                            /* ... */
};

data 'STR#' (159, "Scroll bar name", purgeable) {
	$"0002 0C48 6F72 697A 2053 6372 6F6C 6C0B"            /* ...Horiz Scroll. */
	$"5665 7274 2053 6372 6F6C 6C"                        /* Vert Scroll */
};

data 'STR#' (160, "Select External Editor", purgeable) {
	$"0001 1C53 656C 6563 7420 7468 6520 4578"            /* ...Select the Ex */
	$"7465 726E 616C 2045 6469 746F 7220 3A"              /* ternal Editor : */
};

data 'STR#' (161, "suffix for icons", purgeable) {
	$"0003 0004 2E69 636F 052E 6963 6E73"                 /* .....ico..icns */
};

data 'STR#' (162, "Grid Color", purgeable) {
	$"0002 2053 656C 6563 7420 4772 6964 2043"            /* .. Select Grid C */
	$"6F6C 6F72 2028 7065 7220 3120 7069 7865"            /* olor (per 1 pixe */
	$"6C29 2E1E 5365 6C65 6374 2047 7269 6420"            /* l)..Select Grid  */
	$"436F 6C6F 7220 2866 6F72 2069 636F 6E73"            /* Color (for icons */
	$"292E"                                               /* ). */
};

data 'STR#' (3013, "Help for Favorite Palette", purgeable) {
	$"0001 1246 6176 6F72 6974 6520 5061 6C65"            /* ...Favorite Pale */
	$"7474 650D 0D"                                       /* tte.. */
};

data 'STR#' (135, "Title for PaintWindow", purgeable) {
	$"0001 075E 3020 285E 3129"                           /* ...^0 (^1) */
};

data 'PICT' (143, "Fast", purgeable) {
	$"0000 0000 0000 0004 0009 0011 02FF 0C00"            /* .........�...�.. */
	$"FFFE 0000 0048 0000 0048 0000 0000 0000"            /* ��...H...H...... */
	$"0004 0009 0000 0000 001E 0001 000A 0000"            /* ...�.........�.. */
	$"0000 0004 0009 0098 8002 0000 0000 0004"            /* .....�.��....... */
	$"0009 0000 0000 0000 0000 0048 0000 0048"            /* .�.........H...H */
	$"0000 0000 0001 0001 0001 0000 0000 0BB6"            /* ...............� */
	$"544C 0000 0000 0000 5BB8 0000 0001 0000"            /* TL......[�...... */
	$"0000 0000 0000 0001 FFFF FFFF FFFF 0000"            /* ........������.. */
	$"0000 0004 0009 0000 0000 0004 0009 0000"            /* .....�.......�.. */
	$"F87F 887F F87F 087F 00FF 6F7B 6F7B 6F7B"            /* �.�.�....�o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 0000 0000"            /* o{o{o{o{o{o{.... */
	$"0000 6F7B 0000 6F7B 6F7B 6F7B 6F7B 6F7B"            /* ..o{..o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 0000 0000 0000 0000"            /* o{o{o{o{........ */
	$"0000 0000 0000 0000 6F7B 6F7B 6F7B 6F7B"            /* ........o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 0000 0000"            /* o{o{o{o{o{o{.... */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"0000 6F7B 0000 0000 0000 0000 0000 0000"            /* ..o{............ */
	$"0000 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 0000"            /* ..o{o{o{o{o{o{.. */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 0000 6F7B"            /* o{o{o{o{o{o{..o{ */
	$"0000 0000 0000 6F7B 0000 6F7B 0000 6F7B"            /* ......o{..o{..o{ */
	$"0000 6F7B 0000 0000 0000 0000 0000 0000"            /* ..o{............ */
	$"0000 0000 0000 0000 0000 6F7B 6F7B 6F7B"            /* ..........o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 0000 6F7B"            /* o{o{o{o{o{o{..o{ */
	$"0000 6F7B 0000 0000 0000 0000 0000 0000"            /* ..o{............ */
	$"0000 6F7B 6F7B 6F7B 6F7B 6F7B 0000 0000"            /* ..o{o{o{o{o{.... */
	$"0000 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* ..o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 0000 6F7B"            /* o{o{o{o{o{o{..o{ */
	$"0000 6F7B 0000 6F7B 0000 6F7B 0000 6F7B"            /* ..o{..o{..o{..o{ */
	$"0000 6F7B 6F7B 6F7B 0000 0000 6F7B 0000"            /* ..o{o{o{....o{.. */
	$"6F7B 0000 0000 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{....o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"0000 6F7B 0000 6F7B 0000 6F7B 0000 6F7B"            /* ..o{..o{..o{..o{ */
	$"0000 6F7B 0000 0000 6F7B 6F7B 6F7B 0000"            /* ..o{....o{o{o{.. */
	$"6F7B 6F7B 6F7B 0000 0000 6F7B 6F7B 6F7B"            /* o{o{o{....o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 4E73 4E73 4E73"            /* o{o{o{o{o{NsNsNs */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 4E73 39CE 39CE"            /* o{o{o{o{o{Ns9�9� */
	$"4E73 4E73 4E73 4E73 4E73 4E73 4E73 4E73"            /* NsNsNsNsNsNsNsNs */
	$"4E73 4E73 4E73 4E73 4E73 4E73 4E73 4E73"            /* NsNsNsNsNsNsNsNs */
	$"4E73 4E73 4E73 4E73 4E73 4E73 4E73 4E73"            /* NsNsNsNsNsNsNsNs */
	$"4E73 4E73 4E73 4E73 4E73 4E73 4E73 4E73"            /* NsNsNsNsNsNsNsNs */
	$"4E73 4E73 4E73 4E73 4E73 0000 0000 0000"            /* NsNsNsNsNs...... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000"                           /* .......... */
};

data 'PICT' (144, "Slow", purgeable) {
	$"0000 0000 0000 0004 0007 0011 02FF 0C00"            /* .............�.. */
	$"FFFE 0000 0048 0000 0048 0000 0000 0000"            /* ��...H...H...... */
	$"0004 0007 0000 0000 001E 0001 000A 0000"            /* .............�.. */
	$"0000 0004 0007 0098 8002 0000 0000 0004"            /* .......��....... */
	$"0007 0000 0000 0000 0000 0048 0000 0048"            /* ...........H...H */
	$"0000 0000 0001 0001 0001 0000 0000 0BB6"            /* ...............� */
	$"51D8 0000 0000 0000 5CBE 0000 0001 0000"            /* Q�......\�...... */
	$"0000 0000 0000 0001 FFFF FFFF FFFF 0000"            /* ........������.. */
	$"0000 0004 0007 0000 0000 0004 0007 0000"            /* ................ */
	$"E0FF A0FF E0FF 20FF 00FF 6F7B 6F7B 6F7B"            /* ������ �.�o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 0000 0000"            /* o{o{o{o{o{o{.... */
	$"0000 6F7B 0000 6F7B 6F7B 6F7B 6F7B 6F7B"            /* ..o{..o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 0000 0000 0000 0000"            /* o{o{o{o{........ */
	$"0000 0000 0000 0000 6F7B 6F7B 6F7B 6F7B"            /* ........o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 0000 0000"            /* o{o{o{o{o{o{.... */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"0000 6F7B 0000 0000 0000 0000 0000 0000"            /* ..o{............ */
	$"0000 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 0000"            /* ..o{o{o{o{o{o{.. */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 0000 6F7B"            /* o{o{o{o{o{o{..o{ */
	$"0000 0000 0000 6F7B 0000 6F7B 0000 6F7B"            /* ......o{..o{..o{ */
	$"0000 6F7B 0000 0000 0000 0000 0000 0000"            /* ..o{............ */
	$"0000 0000 0000 0000 0000 6F7B 6F7B 6F7B"            /* ..........o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 0000 6F7B"            /* o{o{o{o{o{o{..o{ */
	$"0000 6F7B 0000 0000 0000 0000 0000 0000"            /* ..o{............ */
	$"0000 6F7B 6F7B 6F7B 6F7B 6F7B 0000 0000"            /* ..o{o{o{o{o{.... */
	$"0000 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* ..o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 0000 6F7B"            /* o{o{o{o{o{o{..o{ */
	$"0000 6F7B 0000 6F7B 0000 6F7B 0000 6F7B"            /* ..o{..o{..o{..o{ */
	$"0000 6F7B 6F7B 6F7B 0000 0000 6F7B 0000"            /* ..o{o{o{....o{.. */
	$"6F7B 0000 0000 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{....o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"0000 6F7B 0000 6F7B 0000 6F7B 0000 6F7B"            /* ..o{..o{..o{..o{ */
	$"0000 6F7B 0000 0000 6F7B 6F7B 6F7B 0000"            /* ..o{....o{o{o{.. */
	$"6F7B 6F7B 6F7B 0000 0000 6F7B 6F7B 6F7B"            /* o{o{o{....o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 4E73 4E73 4E73"            /* o{o{o{o{o{NsNsNs */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 4E73 39CE 39CE"            /* o{o{o{o{o{Ns9�9� */
	$"4E73 4E73 4E73 4E73 4E73 4E73 4E73 4E73"            /* NsNsNsNsNsNsNsNs */
	$"4E73 4E73 4E73 4E73 4E73 4E73 4E73 4E73"            /* NsNsNsNsNsNsNsNs */
	$"4E73 4E73 4E73 4E73 4E73 4E73 4E73 4E73"            /* NsNsNsNsNsNsNsNs */
	$"4E73 4E73 4E73 4E73 4E73 4E73 4E73 4E73"            /* NsNsNsNsNsNsNsNs */
	$"4E73 4E73 4E73 4E73 4E73 0000 0000 0000"            /* NsNsNsNsNs...... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000"                           /* .......... */
};

data 'PICT' (132, "tools", purgeable) {
	$"1514 0000 0000 0080 0023 0011 02FF 0C00"            /* .......�.#...�.. */
	$"FFFE 0000 0048 0000 0048 0000 0000 0000"            /* ��...H...H...... */
	$"0080 0023 0000 0000 00A1 01F2 0016 3842"            /* .�.#.....�.�..8B */
	$"494D 0000 0000 0000 0080 0023 4772 8970"            /* IM.......�.#Gr�p */
	$"68AF 626A 0001 000A 0000 0000 0080 0023"            /* h�bj...�.....�.# */
	$"009A 0000 00FF 808C 0000 0000 0080 0023"            /* .�...���.....�.# */
	$"0000 0004 0000 0000 0048 0000 0048 0000"            /* .........H...H.. */
	$"0010 0020 0003 0008 0000 0000 0000 0000"            /* ... ............ */
	$"0000 0000 0000 0000 0080 0023 0000 0000"            /* .........�.#.... */
	$"0080 0023 0040 06DE D8DE 03DE 2906 DED8"            /* .�.#.@.���.�).�� */
	$"DE03 DE29 06DE D8DE 03DE 2906 DED8 DE03"            /* �.�).���.�).���. */
	$"DE29 22EC D806 CA28 0D36 5E7A A2FA D8EB"            /* �)"��.�(.6^z���� */
	$"0304 0604 0102 02F9 03EC 2906 2610 0A0E"            /* .......�.�).&.�. */
	$"1419 20FA 2926 EDD8 07CA 284B 6B01 0112"            /* .. �)&��.�(Kk... */
	$"01FA D8EC 0306 0848 5062 4118 01FA 03ED"            /* .���...HPbA..�.� */
	$"2907 2611 4A5C 6343 1E05 FA29 29EE D803"            /* ).&.J\cC..�))��. */
	$"CA28 4B81 FE01 010B 01FA D8ED 0307 0848"            /* �(K��....���...H */
	$"8182 8249 0801 FA03 EE29 0827 114A 8982"            /* ���I..�.�).'.J�� */
	$"824B 0E05 FA29 2CEF D803 CB28 4B81 FE01"            /* �K..�),��.�(K��. */
	$"0210 0001 FAD8 EE03 0808 4781 8282 310D"            /* ....���...G���1. */
	$"3501 FA03 EF29 0927 114A 8982 8233 1234"            /* 5.�.�)�'.J���3.4 */
	$"05FA 292F F0D8 03CA 284B 9AFE 0103 1000"            /* .�)/��.�(K��.... */
	$"0001 FAD8 EF03 0908 4880 8282 310C 362F"            /* ..���.�.H���1.6/ */
	$"01FA 03F0 290A 2711 4A8A 8282 3412 342E"            /* .�.�)�'.J���4.4. */
	$"05FA 2932 F1D8 0BCA 294B A40E 0101 1100"            /* .�)2��.�)K�..... */
	$"0009 87FA D8F0 030A 0647 9F7B 8231 0C35"            /* .Ƈ���.�.G�{�1.5 */
	$"2E02 02FA 03F1 290B 2710 4AA3 7D82 3313"            /* ...�.�).'.J�}�3. */
	$"342D 071B FA29 34F2 D80C B00E 7EA4 0E01"            /* 4-..�)4��.�.~�.. */
	$"0615 0001 0772 C5FA D8F1 030A 0267 9F7B"            /* .....r����.�.g�{ */
	$"822E 0936 2402 04F9 03F2 290C 2308 6CA3"            /* �.�6$..�.�).#.l� */
	$"7D82 3210 3424 0718 26FA 293B F3D8 0EB0"            /* }�2.4$..&�);��.� */
	$"0D7E A40E 0106 1500 0006 5D72 72A5 FBD8"            /* .~�.......]rr��� */
	$"F203 0D02 679F 7B82 2E09 3524 0305 0606"            /* �...g�{�.�5$.... */
	$"05FB 03F3 290E 2208 6DA4 7D82 320F 3424"            /* .�.�).".m�}�2.4$ */
	$"0715 1818 20FB 293F F4D8 10AF 0D7F A30E"            /* .... �)?��.�..�. */
	$"0106 1500 0005 5D72 7273 72C5 FCD8 F303"            /* ......]rrsr����. */
	$"0A02 68B3 7D82 2E09 3524 0205 FD06 0004"            /* �.h�}�.�5$..�... */
	$"FC03 F429 0B22 076D B47F 8232 0F34 2407"            /* �.�).".m�.�2.4$. */
	$"15FD 1800 26FC 293E F5D8 0CAF 0E7E BB28"            /* .�..&�)>��.�.~�( */
	$"0104 2900 010E 5D73 FE72 00B1 FBD8 F503"            /* ..)...]s�r.����. */
	$"FF02 0767 BA71 7A1F 1036 16FB 0600 04FB"            /* �..g�qz..6.�...� */
	$"03F5 290B 2208 6DBB 777A 231A 3418 0B14"            /* .�).".m�wz#.4... */
	$"FD18 0022 FB29 41F6 D80C B00D 7FBB 2801"            /* �.."�)A��.�..�(. */
	$"0629 0004 116B 73FE 7200 B1FA D8F5 030B"            /* .)...ks�r.����.. */
	$"0268 BA72 7A16 1139 1504 0607 FE06 0004"            /* .h�rz..9....�... */
	$"FA03 F629 1022 086D BB77 7A1A 1A37 170A"            /* �.�).".m�wz..7.� */
	$"1718 1817 1822 FA29 3EF6 D80F 5145 8025"            /* ....."�)>��.QE�% */
	$"0109 3000 0412 6C72 7373 72B2 F9D8 F603"            /* .�0...lrssr����. */
	$"0F02 3888 717A 150C 3915 0406 0607 0706"            /* ..8�qz..9....... */
	$"04F9 03F6 290A 133C 8876 7A19 1637 170A"            /* .�.�)�.<�vz..7.� */
	$"17FD 1800 23F9 293B F6D8 0E36 8214 0109"            /* .�..#�);��.6�..� */
	$"3000 031E 6B72 7372 73B2 F8D8 F603 0701"            /* 0...krsrs����... */
	$"7C22 1017 0C35 0EFE 0603 0706 0604 F803"            /* |"...5.�......�. */
	$"F629 090E 7F25 141C 1634 110D 17FD 1800"            /* �)�..%...4...�.. */
	$"22F8 2937 F6D8 0738 BFFF 7414 0003 23FE"            /* "�)7��.8��t...#� */
	$"73FF 7200 B2F7 D8F6 030D 0DBF FF82 1136"            /* s�r.����...���.6 */
	$"0B06 0706 0706 0604 F703 F629 0718 C0FF"            /* ........�.�)..�� */
	$"8415 340E 0EFC 1800 22F7 2934 F6D8 0C01"            /* �.4..�.."�)4��.. */
	$"EFFF FF01 0323 7372 7372 73B2 F6D8 F603"            /* ���..#srsrs����. */
	$"0C01 EFFF FF01 0B06 0706 0706 0604 F603"            /* ..���.........�. */
	$"F629 0605 EFFF FF05 0E0F FC18 0023 F629"            /* �)..���...�..#�) */
	$"35F7 D807 6C2C FFEF AD04 3272 FE73 0179"            /* 5��.l,��.2r�s.y */
	$"BEF5 D8F7 030C 0216 FFEF A502 0706 0706"            /* ����....��..... */
	$"0706 04F5 03F7 2906 171D FFF0 A806 11FD"            /* ...�.�)...��..� */
	$"1801 1925 F529 33F8 D807 5114 0A0E 0129"            /* ...%�)3��.Q.�..) */
	$"6773 FE72 018C D2F4 D8F8 0307 0208 0405"            /* gs�r.�����...... */
	$"0104 0307 FD06 F303 F829 0C12 0806 0A05"            /* ....�.�.�)....�. */
	$"0E16 1818 1717 1C28 F429 30F8 D80A 0E01"            /* .......(�)0���.. */
	$"01BD D8BE 7273 7385 B8F2 D8F8 030A 0102"            /* .�ؾrss�����.�.. */
	$"0203 0304 0607 0705 04F2 03F8 290A 0705"            /* .........�.�)�.. */
	$"0525 2924 1718 181B 23F2 2925 F8D8 08B0"            /* .%)$....#�)%��.� */
	$"A2B0 D8CB 8598 ABCB F0D8 F403 0304 0605"            /* ���˅������..... */
	$"04EF 03F8 2908 2220 2229 271B 1E21 27F0"            /* .�.�)." ")'..!'� */
	$"2906 DED8 DE03 DE29 06DE D8DE 03DE 2906"            /* ).���.�).���.�). */
	$"DED8 DE03 DE29 06DE D8DE 03DE 2906 DED8"            /* ���.�).���.�).�� */
	$"DE03 DE29 06DE D8DE 03DE 2918 F1D8 02E9"            /* �.�).���.�).��.� */
	$"BBE9 F1D8 F103 026B BB6B F103 F129 0280"            /* �����..k�k�.�).� */
	$"BB80 F129 1EF2 D804 EABB EFAB E9F2 D8F2"            /* ���).��.������ */
	$"0304 6BBB 2B38 6BF2 03F2 2904 80BB 5048"            /* ..k�+8k�.�).��PH */
	$"80F2 292A F4D8 03DC EABB EFFE AB01 E9DC"            /* ��)*��.�����.�� */
	$"F4D8 F403 031E 6BBB 2BFE 3801 6B1E F403"            /* ���...k�+�8.k.�. */
	$"F429 033F 80BB 50FE 4801 803F F429 30F5"            /* �).?��P�H.�?�)0� */
	$"D80A DCEA BBEF B5CB CBB4 ABE9 DCF5 D8F5"            /* ������˴������ */
	$"030A 1E6B BB2B 2608 0826 386B 1EF5 03F5"            /* .�.k�+&..&8k.�.� */
	$"290A 4080 BB50 3C2B 2A3D 4880 40F5 2936"            /* )�@��P<+*=H�@�)6 */
	$"F6D8 0CDC EABB EFB5 CCD8 D8CB B5AB E9DC"            /* ��.������˵��� */
	$"F6D8 F603 0C1E 6BBB 2B26 0803 0308 2638"            /* ���...k�+&....&8 */
	$"6B1E F603 F629 0C3F 80BB 503D 2B29 292B"            /* k.�.�).?��P=+))+ */
	$"3D48 8040 F629 39F7 D805 DCE9 BBEF B5CB"            /* =H�@�)9��.���� */
	$"FDD8 04CB B5AB E9DC F7D8 F703 051E 6BBB"            /* ��.˵������...k� */
	$"2B26 08FD 0304 0826 386B 1EF7 03F7 2905"            /* +&.�...&8k.�.�). */
	$"4080 BB50 3C2B FD29 042B 3C48 8040 F729"            /* @��P<+�).+<H�@�) */
	$"39F8 D805 DCEA BBEF B5CB FBD8 04CC B5AB"            /* 9��.������.̵� */
	$"EADC F8D8 F803 051E 6BBB 2B26 08FB 0304"            /* �����...k�+&.�.. */
	$"0826 386B 1EF8 03F8 2905 3F80 BB50 3D2B"            /* .&8k.�.�).?��P=+ */
	$"FB29 042B 3D48 8040 F829 39F9 D805 DCE9"            /* �).+=H�@�)9��.�� */
	$"BBEF A2C4 F9D8 04CC ABAB E9DC F9D8 F903"            /* �����.̫������. */
	$"051E 6BBB 2B3A 0CF9 0304 0838 386B 1EF9"            /* ..k�+:.�...88k.� */
	$"03F9 2905 4080 BB50 482C F929 042B 4848"            /* .�).@��PH,�).+HH */
	$"8040 F929 39F9 D805 EABB 8871 7BC9 F9D8"            /* �@�)9��.껈q{��� */
	$"04CB AB87 3EDF F9D8 F903 056B BB88 963F"            /* .˫�>����..k���? */
	$"04F9 0304 0838 073E 31F9 03F9 2905 80BB"            /* .�...8.>1�.�).�� */
	$"8882 4527 F929 042A 4820 3E50 F929 3CF9"            /* ��E'�).*H >P�)<� */
	$"D805 BBFF FF88 A1C4 FAD8 05CC B587 5E35"            /* �.��������.̵�^5 */
	$"DCF9 D8F9 0305 BBFF FF88 3A0C FA03 0508"            /* ����..����:.�... */
	$"2607 0C36 1EF9 03F9 2905 BBFF FF88 472C"            /* &..6.�.�).����G, */
	$"FA29 052B 3D20 1B35 40F9 2942 F9D8 00BB"            /* �).+= .5@�)B��.� */
	$"FEEE 0288 B4CC FCD8 06CC B587 6428 04DC"            /* ��.�����.̵�d(.� */
	$"F9D8 F903 00BB FEEE 0288 2608 FC03 0608"            /* ���..���.�&.�... */
	$"2607 0C2B 0A1E F903 F929 00BB FEEE 0288"            /* &..+�.�.�).���.� */
	$"3C2B FC29 062B 3D20 1D29 0540 F929 45F9"            /* <+�).+= .).@�)E� */
	$"D800 88FD EE02 88B5 CCFE D807 CBB4 8764"            /* �.���.�����.˴�d */
	$"2A04 0464 F9D8 F903 0088 FDEE 0288 2609"            /* *..d���..���.�&� */
	$"FE03 0708 2607 0C2C 0A0A 0CF9 03F9 2900"            /* �...&..,��.�.�). */
	$"88FD EE02 883D 2BFE 2907 2B3D 201D 2B05"            /* ���.�=+�).+= .+. */
	$"051D F929 45F9 D800 88FC EE0D 88B5 C4C9"            /* ..�)E��.���.���� */
	$"C4B4 8764 2C04 0464 87B1 FAD8 F903 0088"            /* Ĵ�d,..d�����..� */
	$"FCEE 0D88 260C 040C 2607 0C2E 0A0A 0C07"            /* ��.�&...&...��.. */
	$"04FA 03F9 2900 88FC EE0D 883C 2C27 2C3D"            /* .�.�).���.�<,',= */
	$"201D 2D05 051D 2022 FA29 42F9 D801 6188"            /*  .-... "�)B��.a� */
	$"FCEE 0B88 A17B A187 642E 0404 6487 B1F9"            /* ��.��{��d...d��� */
	$"D8F9 0301 6188 FCEE 0B88 3A3E 3A07 0C30"            /* ��..a���.�:>:..0 */
	$"0A0A 0C07 04F9 03F9 2901 6188 FCEE 0B88"            /* ��...�.�).a���.� */
	$"4845 4720 1D2E 0505 1D20 22F9 293C F9D8"            /* HEG ..... "�)<�� */
	$"0164 7CFB EE09 8871 8764 3304 0464 87B1"            /* .d|��ƈq�d3..d�� */
	$"F8D8 F903 010C 7CFB EE09 8896 070C 340A"            /* ���...|��ƈ�..4� */
	$"0A0C 0704 F803 F929 011D 7CFB EE09 8882"            /* �...�.�)..|��ƈ� */
	$"201D 3305 051D 2022 F829 39F8 D801 047C"            /*  .3... "�)9��..| */
	$"FCEE 08DD 8864 3304 0464 87B1 F7D8 F803"            /* ��.݈d3..d�����. */
	$"010A 7DFC EE08 DD88 0C35 0A0A 0C07 04F7"            /* .�}��.݈.5��...� */
	$"03F8 2901 057C FCEE 08DD 881D 3405 051D"            /* .�)..|��.݈.4... */
	$"2022 F729 3CF8 D80E BE04 7C88 EEEE DDBB"            /*  "�)<��.�.|���ݻ */
	$"5533 0404 6487 B1F6 D8F8 030E 040A 7C88"            /* U3..d�����...�|� */
	$"EEEE DDBB 5535 0A0A 0C07 04F6 03F8 290E"            /* ��ݻU5��...�.�). */
	$"2405 7C88 EEEE DDBB 5534 0505 1D20 22F6"            /* $.|���ݻU4... "� */
	$"2936 F7D8 0C97 044A 8477 554E 2E04 0464"            /* )6��.�.J�wUN...d */
	$"87B1 F5D8 F703 0C05 0A4C 8477 554E 2F0A"            /* �����...�L�wUN/� */
	$"0A0C 0704 F503 F729 0C1E 054B 8477 554E"            /* �...�.�)...K�wUN */
	$"2E05 051D 2022 F529 24F6 D800 B1FA 0402"            /* .... "�)$��.��.. */
	$"6487 B1F4 D8F6 0300 04FA 0A02 0C07 04F4"            /* d�����...��....� */
	$"03F6 2900 22FA 0502 1D20 22F4 291C F5D8"            /* .�)."�... "�).�� */
	$"FEB1 01A4 B7FE B1F2 D8F5 03F9 04F2 03F5"            /* ��.�������.�.�.� */
	$"29FE 2201 2023 FE22 F229 06DE D8DE 03DE"            /* )�". #�"�).���.� */
	$"2906 DED8 DE03 DE29 06DE D8DE 03DE 2906"            /* ).���.�).���.�). */
	$"DED8 DE03 DE29 06DE D8DE 03DE 2906 DED8"            /* ���.�).���.�).�� */
	$"DE03 DE29 06DE D8DE 03DE 2906 DED8 DE03"            /* �.�).���.�).���. */
	$"DE29 36F7 D8FE FF00 D8FD FF00 D8FD FF00"            /* �)6����.���.���. */
	$"D8FE FFF9 D8F7 03FE FF00 03FD FF00 03FD"            /* ������.��..��..� */
	$"FF00 03FE FFF9 03F7 29FE FF00 29FD FF00"            /* �..���.�)��.)��. */
	$"29FD FF00 29FE FFF9 293F F8D8 0324 FF24"            /* )��.)���)?��.$�$ */
	$"D8FD 2400 D8FD 2400 D8FE 2400 FFF9 D8F8"            /* ��$.��$.��$.���� */
	$"0303 24FF 2403 FD24 0003 FD24 0003 FE24"            /* ..$�$.�$..�$..�$ */
	$"00FF F903 F829 0324 FF24 29FD 2400 29FD"            /* .��.�).$�$)�$.)� */
	$"2400 29FE 2400 FFF9 291E F8D8 0024 F2D8"            /* $.)�$.��).��.$�� */
	$"0024 F8D8 F803 0024 F203 0024 F803 F829"            /* .$���..$�..$�.�) */
	$"0024 F229 0024 F829 1EF7 D800 FFF2 D800"            /* .$�).$�).��.���. */
	$"FFF9 D8F7 0300 FFF2 0300 FFF9 03F7 2900"            /* ����..��..��.�). */
	$"FFF2 2900 FFF9 2924 F8D8 0124 FFF3 D801"            /* ��).��)$��.$���. */
	$"24FF F9D8 F803 0124 FFF3 0301 24FF F903"            /* $����..$��..$��. */
	$"F829 0124 FFF3 2901 24FF F929 24F8 D801"            /* �).$��).$��)$��. */
	$"24FF F3D8 0124 FFF9 D8F8 0301 24FF F303"            /* $���.$����..$��. */
	$"0124 FFF9 03F8 2901 24FF F329 0124 FFF9"            /* .$��.�).$��).$�� */
	$"291E F8D8 0024 F2D8 0024 F8D8 F803 0024"            /* ).��.$��.$���..$ */
	$"F203 0024 F803 F829 0024 F229 0024 F829"            /* �..$�.�).$�).$�) */
	$"1EF7 D800 FFF2 D800 FFF9 D8F7 0300 FFF2"            /* .��.���.����..�� */
	$"0300 FFF9 03F7 2900 FFF2 2900 FFF9 2924"            /* ..��.�).��).��)$ */
	$"F8D8 0124 FFF3 D801 24FF F9D8 F803 0124"            /* ��.$���.$����..$ */
	$"FFF3 0301 24FF F903 F829 0124 FFF3 2901"            /* ��..$��.�).$��). */
	$"24FF F929 24F8 D801 24FF F3D8 0124 FFF9"            /* $��)$��.$���.$�� */
	$"D8F8 0301 24FF F303 0124 FFF9 03F8 2901"            /* ��..$��..$��.�). */
	$"24FF F329 0124 FFF9 291E F8D8 0024 F2D8"            /* $��).$��).��.$�� */
	$"0024 F8D8 F803 0024 F203 0024 F803 F829"            /* .$���..$�..$�.�) */
	$"0024 F229 0024 F829 21F7 D800 FFF3 D801"            /* .$�).$�)!��.���. */
	$"24FF F9D8 F703 00FF F303 0124 FFF9 03F7"            /* $����..��..$��.� */
	$"2900 FFF3 2901 24FF F929 3CF8 D800 24FE"            /* ).��).$��)<��.$� */
	$"FF00 D8FD FF00 D8FD FF00 D8FE FFF9 D8F8"            /* �.���.���.������ */
	$"0300 24FE FF00 03FD FF00 03FD FF00 03FE"            /* ..$��..��..��..� */
	$"FFF9 03F8 2900 24FE FF00 29FD FF00 29FD"            /* ��.�).$��.)��.)� */
	$"FF00 29FE FFF9 2936 F8D8 FE24 00D8 FD24"            /* �.)���)6���$.��$ */
	$"00D8 FD24 00D8 FE24 F8D8 F803 FE24 0003"            /* .��$.��$���.�$.. */
	$"FD24 0003 FD24 0003 FE24 F803 F829 FE24"            /* �$..�$..�$�.�)�$ */
	$"0029 FD24 0029 FD24 0029 FE24 F829 06DE"            /* .)�$.)�$.)�$�).� */
	$"D8DE 03DE 2906 DED8 DE03 DE29 06DE D8DE"            /* ��.�).���.�).��� */
	$"03DE 2906 DED8 DE03 DE29 06DE D8DE 03DE"            /* .�).���.�).���.� */
	$"2906 DED8 DE03 DE29 06DE D8DE 03DE 291E"            /* ).���.�).���.�). */
	$"EAD8 04BD 5338 61BD FAD8 EA03 0404 0708"            /* ��.�S8a����..... */
	$"0704 FA03 EA29 0424 120E 1525 FA29 21EA"            /* ..�.�).$...%�)!� */
	$"D805 8810 8237 04BD FBD8 EA03 0505 189B"            /* �.�.�7.����....� */
	$"440A 04FB 03EA 2905 1B14 9C41 0524 FB29"            /* D�.�.�)...�A.$�) */
	$"21EA D805 604F CD82 046E FBD8 EA03 0507"            /* !��.`O͂.n���... */
	$"61F1 9B0A 07FB 03EA 2905 145F F59C 0517"            /* a��.�.�).._��.. */
	$"FB29 24EB D806 CB11 042A 0404 95FB D8EB"            /* �)$��.�..*..���� */
	$"0306 040A 0A35 0A0A 05FB 03EB 2906 2707"            /* ...��5��.�.�).'. */
	$"0532 0505 1DFB 2926 EDD8 0288 4011 FD04"            /* .2...�)&��.�@.�. */
	$"0111 CBFB D8ED 0302 0542 18FC 0A00 04FB"            /* ..����...B.��..� */
	$"03ED 2902 1B41 14FD 0501 0727 FB29 2DEE"            /* .�)..A.�...'�)-� */
	$"D803 A604 8F50 FD04 0255 BBC2 FCD8 EE03"            /* �.�.�P�..U�����. */
	$"0304 0AA6 5FFD 0A02 0804 04FC 03EE 2903"            /* ..¦_��....�.�). */
	$"2005 A65D FD05 0213 2425 FC29 35EF D808"            /*  .�]�...$%�)5��. */
	$"8B6E 0A04 0438 6D46 23FE 6500 CAFD D8EF"            /* �n�..8mF#�e.���� */
	$"030B 2063 0B0A 0A08 0608 0907 0706 FC03"            /* .. c.��...�...�. */
	$"EF29 0C33 6507 0505 0E17 100A 1615 1527"            /* �).3e......�...' */
	$"FD29 39F0 D809 889B FF6E 0404 6DD8 D881"            /* �)9��ƈ��n..m�؁ */
	$"FE65 00AD FDD8 F003 090C 83FF 6A0A 0A06"            /* �e.����.�.��j��. */
	$"0303 06FE 0700 05FD 03F0 290D 2287 FF6A"            /* ...�...�.�)."��j */
	$"0505 1729 291A 1616 1522 FD29 39F1 D80D"            /* ...))...."�)9��. */
	$"8C9D D1E0 FF44 046A 8290 6565 739F FCD8"            /* �����D.j��ees��� */
	$"F103 0D20 8CD1 E0FF 300A 0706 0507 0706"            /* �.. ����0�...... */
	$"05FC 03F1 290D 3390 D1E0 FF32 0516 1A1D"            /* .�.�).3����2.... */
	$"1516 181F FC29 36F2 D808 7E9E FF39 1AE1"            /* ....�)6��.~��9.� */
	$"6CA0 73FE 6500 9EFA D8F2 030C 1D96 FF36"            /* l�s�e.����...��6 */
	$"17E0 5505 0606 0707 05FA 03F2 290C 2F97"            /* .�U......�.�)./� */
	$"FF37 18E0 591F 1815 1615 1FFA 2939 F3D8"            /* �7.�Y......�)9�� */
	$"0D62 CEE1 FFD1 0C39 BD90 6565 976C A6FA"            /* .b����.9��ee�l�� */
	$"D8F3 030D 17C3 E0FF D108 1D03 0607 0705"            /* ��...����....... */
	$"0705 FA03 F329 0D25 C6E0 FFD1 0922 241D"            /* ..�.�).%�����"$. */
	$"1515 1E17 21FA 2933 F4D8 0B64 CE48 1AF0"            /* ....!�)3��.d�H.� */
	$"7D61 B889 6565 7AF7 D8F4 030B 25C3 4517"            /* }a��eez���..%�E. */
	$"EF69 1803 0607 0706 F703 F429 0B30 C646"            /* �i......�.�).0�F */
	$"18EF 6D25 241C 1516 19F7 2936 F5D8 0866"            /* .�m%$....�)6��.f */
	$"CFFF C20C 37AF C273 FE65 0097 F7D8 F503"            /* ���.7��s�e.����. */
	$"082C CFFF C108 1F03 0406 FE07 0005 F703"            /* .,���.....�...�. */
	$"F529 0836 CFFF C109 2421 2518 FE16 001E"            /* �).6����$!%.�... */
	$"F729 38F7 D80D CB57 EFA4 FF8D 71D8 AD6C"            /* �)8��.�W���qحl */
	$"6565 6CAD F6D8 F603 0C25 EFA2 FF7F 2203"            /* eel����..%��.". */
	$"0406 0707 0605 F603 F729 0D27 2EEF A3FF"            /* ......�.�).'.�� */
	$"8130 2921 1715 1516 22F6 2937 F8D8 08CB"            /* �0)!...."�)7��.� */
	$"58EF B30C 46A4 D190 FE65 0190 D1F5 D8F7"            /* X�.F�ѐ�e.����� */
	$"0307 2CEF B208 2F0F 0305 FE07 0005 F403"            /* ..,�./...�...�. */
	$"F829 0D27 33EF B209 3329 281D 1616 151D"            /* �).'3��3)(..... */
	$"28F5 2935 F9D8 0CCB 5BE1 F0BE 3AB3 C27A"            /* (�)5��.�[��:��z */
	$"6565 73BB F3D8 F803 0B3D E1F0 B416 0404"            /* ees����..=��... */
	$"0607 0706 04F3 03F9 290C 2643 E1F0 B61D"            /* .....�.�).&C��. */
	$"2325 1916 1618 24F3 2934 FAD8 0CCB 5BD2"            /* #%....$�)4��.�[� */
	$"1A4B 97D8 AD6C 6565 97D1 F2D8 F903 063D"            /* .K�حlee�����..= */
	$"D216 3A0E 0305 FE07 0005 F103 FA29 0C27"            /* �.:...�...�.�).' */
	$"42D2 173D 2729 2217 1616 1E28 F229 34FC"            /* B�.=')"....(�)4� */
	$"D80C BECB 5BEF AE31 A7D8 9765 6573 BBF0"            /* �.��[�1�ؗees�� */
	$"D8FA 030A 3DEF A514 0403 0507 0706 04F0"            /* ��.�=�........� */
	$"03FC 290C 2427 43EF A719 2129 1E15 1618"            /* .�).$'C�.!).... */
	$"24F0 2932 FDD8 0BCB 3A4D 9269 A4D8 D17A"            /* $�)2��.�:M�i���z */
	$"6565 97EE D8FC 030A 1847 9240 0F03 0306"            /* ee����.�.G�@.... */
	$"0607 05EE 03FD 290B 261A 4490 4829 2928"            /* ...�.�).&.D�H))( */
	$"1915 161E EE29 30FD D80A B149 D75F CAD8"            /* ....�)0��±I�_�� */
	$"D17A 6573 BBED D8FD 0303 0430 E055 FE03"            /* �zes����...0�U�. */
	$"0306 0706 04ED 03FD 290A 2230 D752 2629"            /* .....�.�)�"0�R&) */
	$"2819 1518 24ED 2928 FCD8 08CB 6756 D8D0"            /* (...$�)(��.�gV�� */
	$"7A65 90D1 ECD8 FB03 0634 1503 0306 0706"            /* ze�����..4...... */
	$"EB03 FC29 0826 3A1E 2927 1915 1D28 EC29"            /* �.�).&:.)'...(�) */
	$"21FB D805 BECB D882 65A5 EAD8 FB03 0504"            /* !��.��؂e����... */
	$"0303 0607 05EA 03FB 2905 2426 291A 1520"            /* .....�.�).$&)..  */
	$"EA29 17F9 D802 A56C C2E9 D8F9 0301 0407"            /* �).��.�l����.... */
	$"E803 F929 0220 1725 E929 15F9 D801 C29E"            /* �.�). .%�).��. */
	$"E8D8 F903 0104 05E8 03F9 2901 251F E829"            /* ���....�.�).%.�) */
	$"06DE D8DE 03DE 2924 F6D8 01CC B0FE 9901"            /* .���.�)$��.̰��. */
	$"ADC8 F0D8 F603 011F 5EFE 9501 6828 F003"            /* �����...^��.h(�. */
	$"F629 013E 6DFE 9601 7444 F029 2AF7 D808"            /* �).>m��.tD�)*��. */
	$"B899 99B1 C0B1 9D99 BCF1 D8F7 0308 4C95"            /* ������������..L� */
	$"955F 3A5F 8C95 43F1 03F7 2908 5F96 966E"            /* �_:_��C�.�)._��n */
	$"526E 8F96 59F1 292D F7D8 029D A9D4 FED8"            /* Rn��Y�)-��.����� */
	$"03D4 AD99 C0F2 D8F7 0302 8C71 0CFE 0303"            /* .ԭ�����..�q.�.. */
	$"0C68 953A F203 F729 0290 7B30 FE29 0330"            /* .h�:�.�).�{0�).0 */
	$"7596 52F2 292A F8D8 02CC 99C0 FBD8 02A5"            /* u�R�)*��.̙���.� */
	$"99D0 F3D8 F803 021F 953A FB03 027A 9515"            /* �����...�:�..z�. */
	$"F303 F829 023E 9652 FB29 0282 9637 F329"            /* �.�).>�R�).��7�) */
	$"2AF8 D802 BD99 D0FB D802 D099 B0F3 D8F8"            /* *��.�����.Й���� */
	$"0302 4395 15FB 0302 1595 5EF3 03F8 2902"            /* ..C�.�...�^�.�). */
	$"5996 36FB 2902 3796 6DF3 2924 F8D8 01AC"            /* Y�6�).7�m�)$��.� */
	$"99F9 D801 AD9C F3D8 F803 0167 95F9 0301"            /* ���.�����..g��.. */
	$"688C F303 F829 0174 96F9 2901 748F F329"            /* h��.�).t��).t��) */
	$"36F8 D80C C099 A7D8 6C30 3143 38A5 BC99"            /* 6��.����l01C8��� */
	$"D0F4 D8F8 030C 3A95 3A03 0B13 131F 1403"            /* ����..:�:....... */
	$"4395 15F4 03F8 290C 5296 4D29 1C18 1825"            /* C�.�.�).R�M)...% */
	$"1B20 5996 37F4 2933 F7D8 0B9F 8A27 1525"            /* . Y�7�)3��.��'.% */
	$"5647 1515 4599 C1F4 D8F7 03FF 8309 1215"            /* VG..E�����.���.. */
	$"2556 4715 1537 953A F403 F729 0B89 8415"            /* %VG..7�:�.�).��. */
	$"1525 5647 1515 3A96 52F4 2933 F7D8 0B7D"            /* .%VG..:�R�)3��.} */
	$"9115 3188 8897 B452 1580 B8F4 D8F7 030B"            /* �.1����R.�����.. */
	$"398D 1531 8888 97B4 5215 7D4C F403 F729"            /* 9�.1����R.}L�.�) */
	$"0B45 8E15 3188 8897 B452 157E 60F4 2935"            /* .E�.1����R.~`�)5 */
	$"F8D8 0CCB 0D8F 7182 8888 B6D9 D994 6E73"            /* ��.�.�q�����ٔns */
	$"F4D8 F703 0B00 716E 8288 88B6 D9D9 946E"            /* ���...qn�����ٔn */
	$"0BF4 03F8 290C 2703 776F 8288 88B6 D9D9"            /* .�.�).'.wo������ */
	$"946E 1EF4 2938 F8D8 0D6C 4BC8 9967 8897"            /* �n.�)8��.lKșg�� */
	$"D9D9 AC88 8858 BDF5 D8F8 030C 0203 2795"            /* �٬��X����....'� */
	$"6688 97D9 D9AC 8888 3AF4 03F8 290D 1510"            /* f���٬��:�.�)... */
	$"4496 6688 97D9 D9AC 8888 3F24 F529 3BF9"            /* D�f���٬��?$�);� */
	$"D809 CB0D 3396 7118 66D4 D9BB FE88 017F"            /* ���.3�q.f�ٻ��.. */
	$"51F5 D8F8 0308 0031 1957 1766 D4D9 BBFE"            /* Q���...1.W.f�ٻ� */
	$"8801 7F17 F503 F929 0926 0237 325D 1766"            /* �...�.�)�&.72].f */
	$"D4D9 BBFE 8801 7F21 F529 39F9 D808 6B3D"            /* �ٻ��..!�)9��.k= */
	$"0707 6227 0582 97FC 8801 52BD F6D8 F903"            /* ..b'.����.R����. */
	$"0801 0849 492B 0D05 8297 FC88 0143 02F6"            /* ...II+..����.C.� */
	$"03F9 2908 1412 4646 3A12 0582 97FC 8801"            /* .�)...FF:..����. */
	$"4624 F629 41FA D810 CA0D 3F07 0F2D CB6D"            /* F$�)A��.�.?..-�m */
	$"000D 1B44 4C66 7755 AFF6 D8F9 030F 002C"            /* ...DLfwU����..., */
	$"4948 760C 0100 081B 444C 6677 3E02 F603"            /* IHv.....DLfw>.�. */
	$"FA29 1026 0235 4647 8030 1500 091B 444C"            /* �).&.5FG�0..�.DL */
	$"6677 4221 F629 45FA D811 6C47 0707 090A"            /* fwB!�)E��.lG..�� */
	$"239C 518E 5A29 1C13 0032 83CC F7D8 FA03"            /* #�Q�Z)...2�����. */
	$"1102 0449 4977 9381 263A 0B05 0A0B 0D00"            /* ...IIw��&:..�... */
	$"050B 04F7 03FA 2911 1411 4646 7998 8A41"            /* ...�.�)...FFy��A */
	$"4623 140F 0E0E 000D 2128 F729 48FA D803"            /* F#......!(�)H��. */
	$"0D56 070E FE0A 0B19 0101 446C A25E 35D8"            /* .V..��....Dl�^5� */
	$"8221 76F8 D8FA 0303 0025 4957 FE93 0B59"            /* �!v���...%IW��.Y */
	$"5B5B 402F 1901 0103 0B14 0CF8 03FA 2903"            /* [[@/.......�.�). */
	$"0333 4657 FE98 0B5B 5757 4940 3412 0A29"            /* .3FW��.[WWI@4.�) */
	$"2016 1FF8 2948 FAD8 0618 0F07 090A 0A1B"            /*  ..�)H��....���. */
	$"FC01 0809 0BCA D8D8 2D15 45CC FAD8 FA03"            /* �..�.���-.E����. */
	$"0601 3849 8593 936C FC5B 0136 01FE 0303"            /* ..8I���l�[.6.�.. */
	$"1315 1104 FA03 FA29 0605 3746 8898 9870"            /* ....�.�)..7F���p */
	$"FC57 0834 0326 2929 1715 1A28 FA29 4AFA"            /* �W.4.&))...(�)J� */
	$"D806 5110 090A 0A1C 0DFC 0101 008E FED8"            /* �.Q.���..�...��� */
	$"036A 1515 39FA D8FA 0306 0125 6993 937B"            /* .j..9���...%i��{ */
	$"56FD 5B02 4A05 0FFE 0303 0D15 1512 FA03"            /* V�[.J..�......�. */
	$"FA29 0610 266A 9898 8155 FD57 0246 0528"            /* �)..&j���U�W.F.( */
	$"FE29 031D 1515 19FA 294A FAD8 0179 25FE"            /* �).....�)J��.y%� */
	$"0A00 1AFC 0101 0022 FDD8 009B FE15 0076"            /* �..�..."��.��..v */
	$"FBD8 FA03 0102 1CFE 9300 55FD 5B02 550B"            /* ���....��.U�[.U. */
	$"2BFD 0300 09FE 1500 0CFB 03FA 2901 1723"            /* +�..��...�.�)..# */
	$"FE98 0056 FD57 0252 0B2F FD29 0023 FE15"            /* ��.V�W.R./�).#�. */
	$"001F FB29 48FA D804 AF07 060A 12FC 0102"            /* ..�)H��.�..�.�.. */
	$"0005 BDFD D800 CCFE 1500 46FB D8FA 0304"            /* ..���.��..F���.. */
	$"0204 5C93 6EFC 5B02 1C20 0EFD 0300 04FE"            /* ..\�n�[.. .�...� */
	$"1500 11FB 03FA 2904 2105 5F98 71FC 5702"            /* ...�.�).!._�q�W. */
	$"1B1F 2FFD 2900 28FE 1500 1AFB 293F F9D8"            /* ../�).(�...�)?�� */
	$"0398 0901 08FD 01FF 0000 94FB D800 5EFE"            /* .��..�.�..���.^� */
	$"15FB D8F9 0303 0920 1242 FD5B 0222 111E"            /* .���..� .B�[.".. */
	$"FB03 000E FE15 FB03 F929 0324 2212 41FD"            /* �...�.�.�).$".A� */
	$"5702 2010 37FB 2900 1DFE 15FB 293F F8D8"            /* W. .7�)..�.�)?�� */
	$"08BE 5E17 0A01 0100 0A72 FAD8 03CC 4515"            /* .�^.�...�r��.�E. */
	$"76FB D8F8 0308 151E 0113 4450 1613 28FA"            /* v���......DP..(� */
	$"0303 0410 150C FB03 F829 0837 2F05 1441"            /* ......�.�).7/..A */
	$"4C15 1439 FA29 0328 1A15 1FFB 291E F5D8"            /* L..9�).(...�).�� */
	$"047E 0D00 1BCB EFD8 F503 0405 0600 2209"            /* .~...����....."� */
	$"EF03 F529 041B 0800 262C EF29 0EF3 D800"            /* �.�)....&,�).��. */
	$"CAED D8DE 03F3 2900 26ED 2906 DED8 DE03"            /* ����.�).&�).���. */
	$"DE29 00FF"                                          /* �).� */
};

data 'PICT' (139, "title", purgeable) {
	$"0000 0000 0000 003F 0023 0011 02FF 0C00"            /* .......?.#...�.. */
	$"FFFE 0000 0048 0000 0048 0000 0000 0000"            /* ��...H...H...... */
	$"003F 0023 0000 0000 001E 0001 000A 0000"            /* .?.#.........�.. */
	$"0000 003F 0023 0098 800A 0000 0000 003F"            /* ...?.#.���.....? */
	$"0023 0000 0000 0000 0000 0048 0000 0048"            /* .#.........H...H */
	$"0000 0000 0002 0001 0002 0000 0000 0BB7"            /* ...............� */
	$"51AC 0000 0000 0000 D09F 0000 0003 0000"            /* Q�......П...... */
	$"DEF7 0000 0000 0001 FFFF CE73 CE73 0002"            /* ��......���s�s.. */
	$"FFFF FFFF FFFF 0003 0000 0000 0000 0000"            /* ������.......... */
	$"0000 003F 0023 0000 0000 003F 0023 0000"            /* ...?.#.....?.#.. */
	$"05F9 5501 542A 05F9 5501 5403 0AFE 5506"            /* .�U.T*.�U.T.��U. */
	$"5000 1555 5557 640A FE55 0200 0001 FE55"            /* P..UUWd��U....�U */
	$"006E 0B02 5555 50FE 0003 1555 552D 0B02"            /* .n..UUP�...UU-.. */
	$"5555 40FE 0003 0555 5400 0A01 5555 FD00"            /* UU@�...UT.�.UU�. */
	$"0301 5554 0209 0155 54FC 0002 5554 0009"            /* ..UT.�.UT�..UT.� */
	$"0155 54FC 0002 5556 760B 0955 5000 AA80"            /* .UT�..UVv.�UP.�� */
	$"0000 1554 2E0B 0355 5002 AAFE 0002 1554"            /* ...T...UP.��...T */
	$"0009 0155 40FC 0002 0554 000B 0955 4000"            /* .�.U@�...T..�U@. */
	$"000A AAA0 0554 000B 0955 400A A82A 80A8"            /* .ª�.T..�U@¨*�� */
	$"0556 870B 0955 400A A02A 82A8 0554 000B"            /* .V�.�U@ *��.T.. */
	$"0955 402A A0AA 02A0 0554 090B 0955 402A"            /* �U@*��.�.T�.�U@* */
	$"80AA 0AA0 0554 180B 0955 40AA 82AA 0A80"            /* �� .T..�U@��� */
	$"0554 000B 0955 40AA 02AA AA00 0554 000B"            /* .T..�U@�.��..T.. */
	$"0955 5000 0AA0 0000 1556 A20B 0955 5000"            /* �UP. ...V�.�UP. */
	$"0AA0 0000 1556 830B 0955 5400 2A80 0000"            /*  ...V�.�UT.*�.. */
	$"5554 6509 0155 54FC 0002 5554 000A 0155"            /* UTe�.UT�..UT.�.U */
	$"55FD 0003 0155 5400 0B02 5555 40FE 0003"            /* U�...UT...UU@�.. */
	$"0555 5400 0B02 5555 50FE 0003 1555 5400"            /* .UT...UUP�...UT. */
	$"0AFE 5506 0000 0155 5554 000A FE55 0650"            /* ��U....UUT.��U.P */
	$"0015 5555 5400 05F9 5501 5400 05F9 5501"            /* ..UUT..�U.T..�U. */
	$"5400 0AFE 5502 5000 15FE 5500 320B 0255"            /* T.��U.P..�U.2..U */
	$"5554 FE00 0355 5557 D80B 0255 5540 FE00"            /* UT�..UUW�..UU@�. */
	$"0305 5554 000A 0155 55FD 0003 0155 5400"            /* ..UT.�.UU�...UT. */
	$"0901 5554 FC00 0255 5400 0901 5550 FC00"            /* �.UT�..UT.�.UP�. */
	$"0215 5408 0901 5540 FC00 0205 5400 0800"            /* ..T.�.U@�...T... */
	$"55FB 0002 0154 030B 0955 0002 8000 00A0"            /* U�...T..�U..�..� */
	$"0157 640B 0954 0002 8000 00A0 0055 6E0B"            /* .Wd.�T..�..�.Un. */
	$"0954 000A 0000 0280 0055 2D0B 0954 000A"            /* �T.�...�.U-.�T.� */
	$"0000 0280 0054 000B 0950 0028 0000 0A00"            /* ...�.T..�P.(..�. */
	$"0014 020B 0950 002A 80A8 0A2A 8014 000B"            /* ....�P.*���*�... */
	$"0950 00A0 A2AA 28A0 A016 760B 0950 00A0"            /* �P.���(��.v.�P.� */
	$"AA0A 28A0 A014 2E0B 0950 0282 8AA8 A282"            /* ��(��...�P.����� */
	$"8014 000B 0950 0282 A800 A282 8014 000B"            /* �...�P.��.���... */
	$"0950 0A0A 282A 8A0A 0014 000B 0054 FE0A"            /* �P��(*��.....T�� */
	$"05A2 8AA8 0056 870A 0054 FD00 0428 0000"            /* .���.V��.T�..(.. */
	$"5400 0A00 54FD 0004 2800 0054 090A 0055"            /* T.�.T�..(..T��.U */
	$"FD00 04A0 0001 5418 0A00 55FD 0004 A000"            /* �..�..T.�.U�..�. */
	$"0154 0009 0155 40FC 0002 0554 0009 0155"            /* .T.�.U@�...T.�.U */
	$"50FC 0002 1556 A209 0155 54FC 0002 5556"            /* P�...V��.UT�..UV */
	$"830A 0155 55FD 0003 0155 5465 0B02 5555"            /* ��.UU�...UTe..UU */
	$"40FE 0003 0555 5400 0B02 5555 54FE 0003"            /* @�...UT...UUT�.. */
	$"5555 5400 0AFE 5506 5000 1555 5554 0005"            /* UUT.��U.P..UUT.. */
	$"F955 0154 0005 F955 0157 FF00 00FF 6F7B"            /* �U.T..�U.W�..�o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B 6F7B"            /* o{o{o{o{o{o{o{o{ */
	$"6F7B 6F7B 6F7B 6F7B 6F7B 4E73 39CE 39CE"            /* o{o{o{o{o{Ns9�9� */
	$"4E73 4E73 4E73 4E73 4E73 4E73 4E73 4E73"            /* NsNsNsNsNsNsNsNs */
	$"4E73 4E73 4E73 4E73 4E73 4E73 4E73 4E73"            /* NsNsNsNsNsNsNsNs */
	$"4E73 4E73 4E73 4E73 4E73 4E73 4E73 4E73"            /* NsNsNsNsNsNsNsNs */
	$"4E73 4E73 4E73 4E73 4E73 4E73 4E73 4E73"            /* NsNsNsNsNsNsNsNs */
	$"4E73 4E73 4E73 4E73 4E73 0000 0000 0000"            /* NsNsNsNsNs...... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000"                           /* .......... */
};

data 'MENU' (128) {
	$"0080 0000 0000 0000 0000 FFFF FFFB 0114"            /* .�........����.. */
	$"1241 626F 7574 2049 636F 6E50 6172 7479"            /* .About IconParty */
	$"2E2E 2E00 0000 0001 2D00 0000 0000"                 /* ........-..... */
};

data 'MENU' (129) {
	$"0081 0000 0000 0000 0000 FFFF E807 0446"            /* .�........���..F */
	$"696C 6503 4E65 7700 4E00 0007 4F70 656E"            /* ile.New.N...Open */
	$"2E2E 2E00 4F00 0005 436C 6F73 6500 5700"            /* ....O...Close.W. */
	$"0004 5361 7665 0053 0000 0A53 6176 6520"            /* ..Save.S..�Save  */
	$"4173 2E2E 2E00 0000 0011 5361 7665 2053"            /* As........Save S */
	$"656C 6563 7469 6F6E 2E2E 2E00 0000 0006"            /* election........ */
	$"5265 7665 7274 0000 0000 012D 0000 0000"            /* Revert.....-.... */
	$"0E4F 7065 6E20 436C 6970 626F 6172 6400"            /* .Open Clipboard. */
	$"0000 0001 2D00 0000 000E 5072 6566 6572"            /* ....-.....Prefer */
	$"656E 6365 732E 2E2E 003B 0000 012D 0000"            /* ences....;...-.. */
	$"0000 0451 7569 7400 5100 0000"                      /* ...Quit.Q... */
};

data 'MENU' (131) {
	$"0083 0000 0000 0000 0000 FFE0 77F8 0745"            /* .�........��w�.E */
	$"6666 6563 7473 0B4C 6173 7420 4566 6665"            /* ffects.Last Effe */
	$"6374 0000 0000 012D 0000 0000 0446 696C"            /* ct.....-.....Fil */
	$"6C00 4600 0007 4C69 6768 7465 6E00 2D00"            /* l.F...Lighten.-. */
	$"0006 4461 726B 656E 003D 0000 0442 6C75"            /* ..Darken.=...Blu */
	$"7200 4200 000E 506F 696E 7469 6C6C 697A"            /* r.B...Pointilliz */
	$"652E 2E2E 0000 0000 0757 696E 642E 2E2E"            /* e........Wind... */
	$"0000 0000 0F49 6E70 7574 2046 6967 7572"            /* .....Input Figur */
	$"652E 2E2E 0000 0000 0A43 6F6C 6F72 204D"            /* e.......�Color M */
	$"6F64 6500 1B8A 0001 2D00 0000 0009 526F"            /* ode..�..-....�Ro */
	$"7461 7465 2E2E 2E00 0000 000F 466C 6970"            /* tate........Flip */
	$"2048 6972 6F7A 6F6E 7461 6C00 0000 000D"            /*  Hirozontal..... */
	$"466C 6970 2056 6572 7469 6361 6C00 0000"            /* Flip Vertical... */
	$"0001 2D00 0000 0006 4F70 6171 7565 0000"            /* ..-.....Opaque.. */
	$"0000 0B54 7261 6E73 7061 7265 6E74 0000"            /* ...Transparent.. */
	$"0000 0542 6C65 6E64 0000 0000 012D 0000"            /* ...Blend.....-.. */
	$"0000 1645 7863 6861 6E67 6520 466C 6F61"            /* ...Exchange Floa */
	$"7469 6E67 2041 7265 6100 0000 0000"                 /* ting Area..... */
};

data 'MENU' (133) {
	$"0085 0000 0000 0000 0000 FFFF FFFF 074F"            /* .�........����.O */
	$"7061 6369 7479 0D31 3030 2520 284F 7061"            /* pacity.100% (Opa */
	$"7175 6529 0031 1200 0520 2037 3525 0032"            /* que).1...  75%.2 */
	$"0000 0520 2035 3025 0033 0000 0520 2032"            /* ...  50%.3...  2 */
	$"3525 0034 0000 00"                                  /* 5%.4... */
};

data 'MENU' (135) {
	$"0087 0000 0000 0000 0000 FFFF FFB7 0950"            /* .�........�����P */
	$"656E 2043 6F6C 6F72 074C 6967 6874 656E"            /* en Color.Lighten */
	$"0000 0000 0644 6172 6B65 6E00 0000 0001"            /* .....Darken..... */
	$"2D00 0000 000C 5265 6365 6E74 2043 6F6C"            /* -.....Recent Col */
	$"6F72 0000 0000 1742 6C65 6E64 2077 6974"            /* or.....Blend wit */
	$"6820 5265 6365 6E74 2043 6F6C 6F72 0000"            /* h Recent Color.. */
	$"0000 012D 0000 0000 1253 656C 6563 7420"            /* ...-.....Select  */
	$"6672 6F6D 2053 6372 6565 6E00 0000 0000"            /* from Screen..... */
};

data 'MENU' (138) {
	$"008A 0000 0000 0000 0000 FFFF FFBF 0A43"            /* .�........�����C */
	$"6F6C 6F72 204D 6F64 6515 3231 3620 436F"            /* olor Mode.216 Co */
	$"6C6F 7273 2857 6562 2043 6F6C 6F72 2900"            /* lors(Web Color). */
	$"0000 0011 4170 706C 6520 4963 6F6E 2043"            /* ....Apple Icon C */
	$"6F6C 6F72 7300 0000 0009 3136 2043 6F6C"            /* olors....�16 Col */
	$"6F72 7300 0000 000D 3136 2047 7261 7973"            /* ors.....16 Grays */
	$"6861 6465 7300 0000 000F 426C 6163 6B20"            /* hades.....Black  */
	$"616E 6420 5768 6974 6500 0000 0001 2D00"            /* and White.....-. */
	$"0000 0016 3231 3620 436F 6C6F 7273 2077"            /* ....216 Colors w */
	$"6974 6820 4469 7468 6572 0000 0000 1D41"            /* ith Dither.....A */
	$"7070 6C65 2049 636F 6E20 436F 6C6F 7273"            /* pple Icon Colors */
	$"2077 6974 6820 4469 7468 6572 0000 0000"            /*  with Dither.... */
	$"1531 3620 436F 6C6F 7273 2077 6974 6820"            /* .16 Colors with  */
	$"4469 7468 6572 0000 0000 1931 3620 4772"            /* Dither.....16 Gr */
	$"6179 7368 6164 6573 2077 6974 6820 4469"            /* ayshades with Di */
	$"7468 6572 0000 0000 1B42 6C61 636B 2061"            /* ther.....Black a */
	$"6E64 2057 6869 7465 2077 6974 6820 4469"            /* nd White with Di */
	$"7468 6572 0000 0000 00"                             /* ther..... */
};

data 'MENU' (139) {
	$"008B 0000 0000 0000 0000 FFFF E407 0549"            /* .�........���..I */
	$"636F 6E73 1043 7265 6174 6520 4963 6F6E"            /* cons.Create Icon */
	$"2046 696C 6500 0000 000E 4372 6561 7465"            /*  File.....Create */
	$"2049 636F 6E2E 2E2E 0000 0000 012D 0000"            /*  Icon........-.. */
	$"0000 0B4E 6577 2049 636F 6E2E 2E2E 004B"            /* ...New Icon....K */
	$"0000 0C49 636F 6E20 496E 666F 2E2E 2E00"            /* ...Icon Info.... */
	$"4900 0001 2D00 0000 0011 496D 706F 7274"            /* I...-.....Import */
	$"2049 636F 6E28 7329 2E2E 2E00 0000 0011"            /*  Icon(s)........ */
	$"4578 706F 7274 2049 636F 6E28 7329 2E2E"            /* Export Icon(s).. */
	$"2E00 0000 0001 2D00 0000 000A 5072 6576"            /* ......-....�Prev */
	$"6965 7720 4247 001B 9A00 012D 0000 0000"            /* iew BG..�..-.... */
	$"114F 7065 6E20 7769 7468 2052 6573 4564"            /* .Open with ResEd */
	$"6974 0000 0000 00"                                  /* it..... */
};

data 'MENU' (140) {
	$"008C 0000 0000 0000 0000 FFFF FFFF 0E50"            /* .�........����.P */
	$"616C 6574 7465 7320 2873 7562 2907 5375"            /* alettes (sub).Su */
	$"7070 6F72 7400 0000 000D 546F 6F6C 7320"            /* pport.....Tools  */
	$"5061 6C65 7474 6500 0000 000B 496E 666F"            /* Palette.....Info */
	$"726D 6174 696F 6E00 0000 000E 436F 6C6F"            /* rmation.....Colo */
	$"7220 5061 6C65 7474 6531 0000 0000 0E43"            /* r Palette1.....C */
	$"6F6C 6F72 2050 616C 6574 7465 3200 0000"            /* olor Palette2... */
	$"000D 426C 656E 6420 5061 6C65 7474 6500"            /* ..Blend Palette. */
	$"0000 000F 5061 7474 6572 6E20 5061 6C65"            /* ....Pattern Pale */
	$"7474 6500 0000 0010 4661 766F 7269 7465"            /* tte.....Favorite */
	$"2050 616C 6574 7465 0000 0000 00"                   /*  Palette..... */
};

data 'MENU' (141) {
	$"008D 0000 0000 0000 0000 FFFF FFFF 045A"            /* .�........����.Z */
	$"6F6F 6D04 3130 3025 0000 0000 0432 3030"            /* oom.100%.....200 */
	$"2500 0000 0004 3430 3025 0000 0000 0438"            /* %.....400%.....8 */
	$"3030 2500 0000 0005 3136 3030 2500 0000"            /* 00%.....1600%... */
	$"0000"                                               /* .. */
};

data 'MENU' (142) {
	$"008E 0000 0000 0000 0000 FFFF FFFF 074F"            /* .�........����.O */
	$"7061 6369 7479 0D31 3030 2520 284F 7061"            /* pacity.100% (Opa */
	$"7175 6529 0000 1200 0520 2037 3525 0000"            /* que).....  75%.. */
	$"0000 0520 2035 3025 0000 0000 0520 2032"            /* ...  50%.....  2 */
	$"3525 0000 0000 00"                                  /* 5%..... */
};

data 'MENU' (143) {
	$"008F 0000 0000 0000 0000 FFFF FFFF 0A54"            /* .�........�����T */
	$"6974 6C65 506F 7075 7000"                           /* itlePopup. */
};

data 'MENU' (144) {
	$"0090 0000 0000 0000 0000 FFFF FFFF 045A"            /* .�........����.Z */
	$"6F6F 6D04 3130 3025 0000 0000 0432 3030"            /* oom.100%.....200 */
	$"2500 0000 0004 3430 3025 0000 0000 0438"            /* %.....400%.....8 */
	$"3030 2500 0000 0005 3136 3030 2500 0000"            /* 00%.....1600%... */
	$"0000"                                               /* .. */
};

data 'MENU' (130) {
	$"0082 0000 0000 0000 0000 FFE8 58A0 0445"            /* .�........��X�.E */
	$"6469 740A 4361 6E27 7420 556E 646F 005A"            /* dit�Can't Undo.Z */
	$"0000 012D 0000 0000 0343 7574 0058 0000"            /* ...-.....Cut.X.. */
	$"0443 6F70 7900 4300 0005 5061 7374 6500"            /* .Copy.C...Paste. */
	$"5600 0005 436C 6561 7200 0000 000A 5365"            /* V...Clear....�Se */
	$"6C65 6374 2041 6C6C 0041 0000 012D 0000"            /* lect All.A...-.. */
	$"0000 0944 7570 6C69 6361 7465 0044 0000"            /* ..�Duplicate.D.. */
	$"012D 0000 0000 0D49 6D61 6765 2053 697A"            /* .-.....Image Siz */
	$"652E 2E2E 0000 0000 0D49 6E70 7574 2054"            /* e........Input T */
	$"6578 742E 2E2E 0054 0000 012D 0000 0000"            /* ext....T...-.... */
	$"1641 6464 2042 6163 6B67 726F 756E 6420"            /* .Add Background  */
	$"284C 6179 6572 2900 0000 000C 556E 696F"            /* (Layer).....Unio */
	$"6E20 4C61 7965 7273 0000 0000 1144 656C"            /* n Layers.....Del */
	$"6574 6520 466F 7265 6772 6F75 6E64 0000"            /* ete Foreground.. */
	$"0000 1144 656C 6574 6520 4261 636B 6772"            /* ...Delete Backgr */
	$"6F75 6E64 0000 0000 012D 0000 0000 1246"            /* ound.....-.....F */
	$"6F72 6567 726F 756E 6420 4F70 6163 6974"            /* oreground Opacit */
	$"7900 1B89 000F 4869 6465 2042 6163 6B67"            /* y..�..Hide Backg */
	$"726F 756E 6400 0000 0000"                           /* round..... */
};

data 'MENU' (145) {
	$"0091 0000 0000 0000 0000 FFFF FFFF 0555"            /* .�........����.U */
	$"6E69 7473 0670 6978 656C 7300 0000 0001"            /* nits.pixels..... */
	$"2500 0000 0000"                                     /* %..... */
};

data 'MENU' (146) {
	$"0092 0000 0000 0080 0000 FFFF FFFF 0100"            /* .�.....�..����.. */
	$"0100 0000 0000 0100 0000 0000 0100 0000"            /* ................ */
	$"0000 0100 0000 0000 0100 0000 0000 0100"            /* ................ */
	$"0000 0000 0100 0000 0000 0100 0000 0000"            /* ................ */
	$"0100 0000 0000 0100 0000 0000 0100 0000"            /* ................ */
	$"0000 0100 0000 0000 0100 0000 0000 0100"            /* ................ */
	$"0000 0000 0100 0000 0000 0100 0000 0000"            /* ................ */
	$"00"                                                 /* . */
};

data 'MENU' (147) {
	$"0093 0000 0000 0080 0000 FFFF FFFF 0100"            /* .�.....�..����.. */
	$"0100 0000 1200 0100 0000 0000 0100 0000"            /* ................ */
	$"0000 0100 0000 0000 0100 0000 0000 0100"            /* ................ */
	$"0000 0000 0100 0000 0000 0100 0000 0000"            /* ................ */
	$"0100 0000 0000 0100 0000 0000 0100 0000"            /* ................ */
	$"0000 0100 0000 0000 0100 0000 0000 0100"            /* ................ */
	$"0000 0000 0100 0000 0000 0100 0000 0000"            /* ................ */
	$"00"                                                 /* . */
};

data 'MENU' (148) {
	$"0094 0000 0000 0080 0000 FFFF FFFF 0100"            /* .�.....�..����.. */
	$"0100 0000 0000 0100 0000 0000 0100 0000"            /* ................ */
	$"0000 0100 0000 0000 0100 0000 0000 0100"            /* ................ */
	$"0000 1200 0100 0000 0000 0100 0000 0000"            /* ................ */
	$"0100 0000 0000 0100 0000 0000 0100 0000"            /* ................ */
	$"0000 0100 0000 0000 0100 0000 0000 0100"            /* ................ */
	$"0000 0000 0100 0000 0000 0100 0000 0000"            /* ................ */
	$"00"                                                 /* . */
};

data 'MENU' (149) {
	$"0095 0000 0000 0000 0000 FFFF FFFB 0657"            /* .�........����.W */
	$"696E 646F 7704 5A6F 6F6D 001B 9000 012D"            /* indow.Zoom..�..- */
	$"0000 0000 0850 616C 6574 7465 7300 1B8C"            /* .....Palettes..� */
	$"0000"                                               /* .. */
};

data 'MENU' (150) {
	$"0096 0000 0000 0000 0000 FFFF FFFF 104F"            /* .�........����.O */
	$"7061 6369 7479 2028 4669 6775 7265 290D"            /* pacity (Figure). */
	$"3130 3025 2028 4F70 6171 7565 2900 0000"            /* 100% (Opaque)... */
	$"0005 2020 3735 2500 0000 0005 2020 3530"            /* ..  75%.....  50 */
	$"2500 0000 0005 2020 3235 2500 0000 0000"            /* %.....  25%..... */
};

data 'MENU' (151) {
	$"0097 0000 0000 0000 0000 FFFF FFFF 1A43"            /* .�........����.C */
	$"6F6C 6F72 2050 616C 6574 7465 2048 696C"            /* olor Palette Hil */
	$"6974 6520 2873 7562 2904 4E6F 6E65 0000"            /* ite (sub).None.. */
	$"0000 1848 696C 6974 6520 4170 706C 6520"            /* ...Hilite Apple  */
	$"4963 6F6E 2043 6F6C 6F72 7300 0000 0010"            /* Icon Colors..... */
	$"4869 6C69 7465 2031 3620 436F 6C6F 7273"            /* Hilite 16 Colors */
	$"0000 0000 1248 696C 6974 6520 5573 6564"            /* .....Hilite Used */
	$"2043 6F6C 6F72 7300 0000 0000"                      /*  Colors..... */
};

data 'MENU' (132) {
	$"0084 0000 0000 0000 0000 FFFF FFFF 0554"            /* .�........����.T */
	$"6F6F 6C73 0B50 656E 204F 7061 6369 7479"            /* ools.Pen Opacity */
	$"001B 8500 0950 656E 2043 6F6C 6F72 001B"            /* ..�.�Pen Color.. */
	$"8700 0850 656E 2053 697A 6500 1B93 000B"            /* �..Pen Size..�.. */
	$"4572 6173 6572 2053 697A 6500 1B94 000F"            /* Eraser Size..�.. */
	$"4772 6964 204F 7074 696F 6E73 2E2E 2E00"            /* Grid Options.... */
	$"0000 000A 4372 6F73 7320 4861 6972 0000"            /* ...�Cross Hair.. */
	$"0000 0F48 6967 686C 6967 6874 2043 6F6C"            /* ...Highlight Col */
	$"6F72 001B 9700 0D42 6C65 6E64 2050 616C"            /* or..�..Blend Pal */
	$"6574 7465 001B A900 0953 656C 6563 7469"            /* ette..�.�Selecti */
	$"6F6E 001B 9800 00"                                  /* on..�.. */
};

data 'MENU' (152) {
	$"0098 0000 0000 0000 0000 FFFF FFF7 0653"            /* .�........����.S */
	$"656C 6563 740B 4175 746F 2053 656C 6563"            /* elect.Auto Selec */
	$"7400 0000 000B 5365 6C65 6374 204E 6F6E"            /* t.....Select Non */
	$"6500 0000 0001 2D00 0000 000A 4869 6465"            /* e.....-....�Hide */
	$"2045 6467 6573 0048 0000 00"                        /*  Edges.H... */
};

data 'MENU' (153) {
	$"0099 0000 0000 0000 0000 FFFF FFFF 0946"            /* .�........�����F */
	$"696C 6520 5479 7065 0950 4943 5420 4669"            /* ile Type�PICT Fi */
	$"6C65 0031 0000 0850 4E47 2046 696C 6500"            /* le.1...PNG File. */
	$"3200 0017 4D61 6369 6E74 6F73 6820 4963"            /* 2...Macintosh Ic */
	$"6F6E 2028 466F 6C64 6572 2900 3300 000C"            /* on (Folder).3... */
	$"5769 6E64 6F77 7320 4963 6F6E 0034 0000"            /* Windows Icon.4.. */
	$"00"                                                 /* . */
};

data 'MENU' (154) {
	$"009A 0000 0000 0000 0000 FFFF FFFF 0A50"            /* .�........�����P */
	$"7265 7669 6577 2042 4705 5768 6974 6500"            /* review BG.White. */
	$"0000 000A 4772 6179 2028 4226 5729 0000"            /* ...�Gray (B&W).. */
	$"0000 0542 6C61 636B 0000 0000 0744 6573"            /* ...Black.....Des */
	$"6B74 6F70 0000 0000 00"                             /* ktop..... */
};

data 'MENU' (155) {
	$"009B 0000 0000 0000 0000 FFFF FFFF 1A43"            /* .�........����.C */
	$"6F6C 6F72 2050 616C 6574 7465 2048 696C"            /* olor Palette Hil */
	$"6974 6520 2870 6F70 2904 4E6F 6E65 0000"            /* ite (pop).None.. */
	$"0000 1848 696C 6974 6520 4170 706C 6520"            /* ...Hilite Apple  */
	$"4963 6F6E 2043 6F6C 6F72 7300 0000 0010"            /* Icon Colors..... */
	$"4869 6C69 7465 2031 3620 436F 6C6F 7273"            /* Hilite 16 Colors */
	$"0000 0000 1248 696C 6974 6520 5573 6564"            /* .....Hilite Used */
	$"2043 6F6C 6F72 7300 0000 0000"                      /*  Colors..... */
};

data 'MENU' (156) {
	$"009C 0000 0000 0000 0000 FFFF FEBB 2443"            /* .�........����$C */
	$"6F6E 7465 7874 7561 6C20 4D65 6E75 2066"            /* ontextual Menu f */
	$"6F72 2049 636F 6E20 4C69 7374 2057 696E"            /* or Icon List Win */
	$"646F 7709 4564 6974 2049 636F 6E00 0000"            /* dow�Edit Icon... */
	$"0001 2D00 0000 0008 4375 7420 4963 6F6E"            /* ..-.....Cut Icon */
	$"0000 0000 0943 6F70 7920 4963 6F6E 0000"            /* ....�Copy Icon.. */
	$"0000 0A43 6C65 6172 2049 636F 6E00 0000"            /* ..�Clear Icon... */
	$"0001 2D00 0000 000E 4475 706C 6963 6174"            /* ..-.....Duplicat */
	$"6520 4963 6F6E 0000 0000 012D 0000 0000"            /* e Icon.....-.... */
	$"0C49 636F 6E20 496E 666F 2E2E 2E00 0000"            /* .Icon Info...... */
	$"0011 4578 706F 7274 2049 636F 6E28 7329"            /* ..Export Icon(s) */
	$"2E2E 2E00 0000 0000"                                /* ........ */
};

data 'MENU' (157) {
	$"009D 0000 0000 0000 0000 FFFF FFDB 1F43"            /* .�........����.C */
	$"6F6E 7465 7874 7561 6C20 4D65 6E75 2066"            /* ontextual Menu f */
	$"6F72 2049 636F 6E20 4C69 7374 2032 0A50"            /* or Icon List 2�P */
	$"6173 7465 2049 636F 6E00 0000 0001 2D00"            /* aste Icon.....-. */
	$"0000 000B 4E65 7720 4963 6F6E 2E2E 2E00"            /* ....New Icon.... */
	$"0000 0011 496D 706F 7274 2049 636F 6E28"            /* ....Import Icon( */
	$"7329 2E2E 2E00 0000 0001 2D00 0000 0011"            /* s)........-..... */
	$"4F70 656E 2077 6974 6820 5265 7345 6469"            /* Open with ResEdi */
	$"7400 0000 0000"                                     /* t..... */
};

data 'MENU' (158) {
	$"009E 0000 0000 0000 0000 FFFF FFFF 0446"            /* .�........����.F */
	$"6F6E 7400"                                          /* ont. */
};

data 'MENU' (159) {
	$"009F 0000 0000 0000 0000 FFFF FFFF 0C5A"            /* .�........����.Z */
	$"6F6F 6D20 2870 6F70 7570 2904 3130 3025"            /* oom (popup).100% */
	$"0000 0000 0432 3030 2500 0000 0004 3430"            /* .....200%.....40 */
	$"3025 0000 0000 0438 3030 2500 0000 0005"            /* 0%.....800%..... */
	$"3136 3030 2500 0000 0000"                           /* 1600%..... */
};

data 'MENU' (160) {
	$"00A0 0000 0000 0000 0000 FFFF FFDF 0D4C"            /* .�........����.L */
	$"6179 6572 2028 706F 7075 7029 1541 6464"            /* ayer (popup).Add */
	$"2042 6163 6B67 726F 756E 6428 4C61 7965"            /*  Background(Laye */
	$"7229 0000 0000 0C55 6E69 6F6E 204C 6179"            /* r).....Union Lay */
	$"6572 7300 0000 0011 4465 6C65 7465 2046"            /* ers.....Delete F */
	$"6F72 6567 726F 756E 6400 0000 0011 4465"            /* oreground.....De */
	$"6C65 7465 2042 6163 6B67 726F 756E 6400"            /* lete Background. */
	$"0000 0001 2D00 0000 0012 466F 7265 6772"            /* ....-.....Foregr */
	$"6F75 6E64 204F 7061 6369 7479 001B 8800"            /* ound Opacity..�. */
	$"0F48 6964 6520 4261 636B 6772 6F75 6E64"            /* .Hide Background */
	$"0000 0000 00"                                       /* ..... */
};

data 'MENU' (136) {
	$"0088 0000 0000 0000 0000 FFFF FFFF 1246"            /* .�........����.F */
	$"6F72 6567 726F 756E 6420 4F70 6163 6974"            /* oreground Opacit */
	$"790D 3130 3025 2028 4F70 6171 7565 2900"            /* y.100% (Opaque). */
	$"0012 0003 3735 2500 0000 0003 3530 2500"            /* ....75%.....50%. */
	$"0000 0003 3235 2500 0000 0002 3025 0000"            /* ....25%.....0%.. */
	$"0000 00"                                            /* ... */
};

data 'MENU' (137) {
	$"0089 0000 0000 0000 0000 FFFF FFFF 1246"            /* .�........����.F */
	$"6F72 6567 726F 756E 6420 4F70 6163 6974"            /* oreground Opacit */
	$"790D 3130 3025 2028 4F70 6171 7565 2900"            /* y.100% (Opaque). */
	$"0012 0003 3735 2500 0000 0003 3530 2500"            /* ....75%.....50%. */
	$"0000 0003 3235 2500 0000 0002 3025 0000"            /* ....25%.....0%.. */
	$"0000 00"                                            /* ... */
};

data 'MENU' (161) {
	$"00A1 0063 0050 000B A5C8 FFFF FFFF 0B46"            /* .�.c.P..������.F */
	$"6967 7572 6520 5479 7065 0445 6467 6500"            /* igure Type.Edge. */
	$"0000 0010 4369 7263 6C65 202F 2045 6C6C"            /* ....Circle / Ell */
	$"6970 7365 0000 0000 0854 7269 616E 676C"            /* ipse.....Triangl */
	$"6500 0000 0011 5472 6961 6E67 6C65 2056"            /* e.....Triangle V */
	$"6572 7469 6361 6C00 0000 0007 4469 616D"            /* ertical.....Diam */
	$"6F6E 6400 0000 000C 4469 6167 6F6E 616C"            /* ond.....Diagonal */
	$"204C 2D52 0000 0000 0C44 6961 676F 6E61"            /*  L-R.....Diagona */
	$"6C20 522D 4C00 0000 0000"                           /* l R-L..... */
};

data 'MENU' (162) {
	$"00A2 0000 0000 0000 0000 FFFF FFFD 1344"            /* .�........����.D */
	$"6F74 204C 6962 7261 7279 2028 706F 7075"            /* ot Library (popu */
	$"7029 012D 0000 0000 0E4E 6577 204C 6962"            /* p).-.....New Lib */
	$"7261 7279 2E2E 2E00 0000 0000"                      /* rary........ */
};

data 'MENU' (163) {
	$"00A3 0000 0000 0000 0000 FFFF FFFF 1344"            /* .�........����.D */
	$"6F74 2043 6F6D 6D61 6E64 2028 706F 7075"            /* ot Command (popu */
	$"7029 00"                                            /* p). */
};

data 'MENU' (164) {
	$"00A4 0000 0000 0000 0000 FFFF FFFB 1852"            /* .�........����.R */
	$"6563 6F72 6465 6420 436F 6D6D 616E 6420"            /* ecorded Command  */
	$"2870 6F70 7570 290F 4578 6563 7574 6520"            /* (popup).Execute  */
	$"436F 6D6D 616E 6400 0000 0001 2D00 0000"            /* Command.....-... */
	$"0012 4164 6420 746F 2044 6F74 204C 6962"            /* ..Add to Dot Lib */
	$"7261 7279 0000 0000 00"                             /* rary..... */
};

data 'MENU' (165) {
	$"00A5 0000 0000 0000 0000 FFFF FFFF 1952"            /* .�........����.R */
	$"6563 6F72 6469 6E67 2043 6F6D 6D61 6E64"            /* ecording Command */
	$"2028 706F 7075 7029 0D43 6C65 6172 2043"            /*  (popup).Clear C */
	$"6F6D 6D61 6E64 0000 0000 0E52 6563 6F72"            /* ommand.....Recor */
	$"6420 436F 6D6D 616E 6400 0000 0000"                 /* d Command..... */
};

data 'MENU' (166) {
	$"00A6 0000 0000 0000 0000 FFFF FFFB 1654"            /* .�........����.T */
	$"7261 6E73 7061 7265 6E74 2042 6163 6B67"            /* ransparent Backg */
	$"726F 756E 6404 4E6F 6E65 0000 0000 012D"            /* round.None.....- */
	$"0000 0000 0557 6869 7465 0000 0000 0542"            /* .....White.....B */
	$"6C61 636B 0000 0000 1154 7261 6E73 7061"            /* lack.....Transpa */
	$"7265 6E74 2043 6F6C 6F72 0000 0000 1342"            /* rent Color.....B */
	$"6173 6564 206F 6E20 4C61 7374 2050 6978"            /* ased on Last Pix */
	$"656C 0000 0000 0950 656E 2043 6F6C 6F72"            /* el....�Pen Color */
	$"0000 0000 0C45 7261 7365 7220 436F 6C6F"            /* .....Eraser Colo */
	$"7200 0000 0000"                                     /* r..... */
};

data 'MENU' (167, "�N�����̏���-pop") {
	$"00A7 0000 0000 0000 0000 FFFF FFFF 0753"            /* .�........����.S */
	$"7461 7274 7570 044E 6F6E 6500 0000 0022"            /* tartup.None...." */
	$"4372 6561 7465 206E 6577 2070 6169 6E74"            /* Create new paint */
	$"2077 696E 646F 7720 6174 2073 7461 7274"            /*  window at start */
	$"7570 0000 0000 1B53 686F 7720 6F70 656E"            /* up.....Show open */
	$"2064 6961 6C6F 6720 6174 2073 7461 7274"            /*  dialog at start */
	$"7570 0000 0000 00"                                  /* up..... */
};

data 'MENU' (168) {
	$"00A8 0000 0000 0000 0000 FFFF FFFF 0F50"            /* .�........����.P */
	$"6169 6E74 2057 696E 646F 7720 4247 0557"            /* aint Window BG.W */
	$"6869 7465 0000 0000 0B54 7261 6E73 7061"            /* hite.....Transpa */
	$"7265 6E74 0000 0000 0C45 7261 7365 7220"            /* rent.....Eraser  */
	$"436F 6C6F 7200 0000 0000"                           /* Color..... */
};

data 'MENU' (169) {
	$"00A9 0000 0000 0000 0000 FFFF FFF7 1042"            /* .�........����.B */
	$"6C65 6E64 2046 696C 6520 2873 7562 2907"            /* lend File (sub). */
	$"4F70 656E 2E2E 2E00 0000 0007 5361 7665"            /* Open........Save */
	$"2E2E 2E00 0000 0001 2D00 0000 0004 4C6F"            /* ........-.....Lo */
	$"636B 0000 0000 00"                                  /* ck..... */
};

data 'MENU' (170) {
	$"00AA 0000 0000 0000 0000 FFFF FFF7 1042"            /* .�........����.B */
	$"6C65 6E64 2046 696C 6520 2870 6F70 2907"            /* lend File (pop). */
	$"4F70 656E 2E2E 2E00 0000 0007 5361 7665"            /* Open........Save */
	$"2E2E 2E00 0000 0001 2D00 0000 0004 4C6F"            /* ........-.....Lo */
	$"636B 0000 0000 00"                                  /* ck..... */
};

data 'MENU' (171) {
	$"00AB 0000 0000 0000 0000 FFFF FFFF 1244"            /* .�........����.D */
	$"6F74 204C 6962 204E 616D 6520 2870 6F70"            /* ot Lib Name (pop */
	$"2907 4564 6974 2E2E 2E00 0000 0000"                 /* ).Edit........ */
};

data 'MENU' (172, "�A�C�R�����X�g�R���e�N�X�g") {
	$"00AC 0000 0000 0000 0000 FFFF FFEF 2543"            /* .�........����%C */
	$"6F6E 7465 7874 7561 6C20 4D65 6E75 2066"            /* ontextual Menu f */
	$"6F72 2049 636F 6E46 616D 696C 7920 5769"            /* or IconFamily Wi */
	$"6E64 6F77 1345 6469 7420 2862 7920 4963"            /* ndow.Edit (by Ic */
	$"6F6E 5061 7274 7929 0000 0000 1745 6469"            /* onParty).....Edi */
	$"7420 6279 2045 7874 6572 6E61 6C20 6564"            /* t by External ed */
	$"6974 6F72 0000 0000 1944 726F 7020 6672"            /* itor.....Drop fr */
	$"6F6D 2045 7874 6572 6E61 6C20 4564 6974"            /* om External Edit */
	$"6F72 0000 0000 012D 0000 0000 0343 7574"            /* or.....-.....Cut */
	$"0000 0000 0443 6F70 7900 0000 0005 5061"            /* .....Copy.....Pa */
	$"7374 6500 0000 0005 436C 6561 7200 0000"            /* ste.....Clear... */
	$"0000"                                               /* .. */
};

data 'MENU' (173, "PNG Compression Level") {
	$"00AD 0000 0000 0000 0000 FFFF FFFF 1550"            /* .�........����.P */
	$"4E47 2043 6F6D 7072 6573 7369 6F6E 204C"            /* NG Compression L */
	$"6576 656C 1230 2028 6E6F 2063 6F6D 7072"            /* evel.0 (no compr */
	$"6573 7369 6F6E 2900 0000 0001 3100 0000"            /* ession).....1... */
	$"0001 3200 0000 0001 3300 0000 0001 3400"            /* ..2.....3.....4. */
	$"0000 0001 3500 0000 000B 3620 2864 6566"            /* ....5.....6 (def */
	$"6175 6C74 2900 0000 0001 3700 0000 0001"            /* ault).....7..... */
	$"3800 0000 0001 3900 0000 0000"                      /* 8.....9..... */
};

data 'MENU' (174, "�ۑ��`���i�A�C�R���j") {
	$"00AE 0000 0000 0000 0000 FFFF FFFF 1046"            /* .�........����.F */
	$"696C 6520 5479 7065 2028 6963 6F6E 2917"            /* ile Type (icon). */
	$"4D61 6369 6E74 6F73 6820 4963 6F6E 2028"            /* Macintosh Icon ( */
	$"466F 6C64 6572 2900 0000 0013 5769 6E64"            /* Folder).....Wind */
	$"6F77 7320 4963 6F6E 2028 2E69 636F 2900"            /* ows Icon (.ico). */
	$"0000 0014 4D61 634F 5320 5820 4963 6F6E"            /* ....MacOS X Icon */
	$"2028 2E69 636E 7329 0000 0000 00"                   /*  (.icns)..... */
};

data 'MENU' (175, "�O���b�h�`��P") {
	$"00AF 0000 0000 0000 0000 FFFF FFFF 0947"            /* .�........�����G */
	$"7269 6420 5479 7065 044E 6F6E 6500 0000"            /* rid Type.None... */
	$"0009 4772 6179 206C 696E 6500 0000 000B"            /* .�Gray line..... */
	$"446F 7474 6564 206C 696E 6500 0000 000A"            /* Dotted line....� */
	$"536F 6C69 6420 6C69 6E65 0000 0000 00"              /* Solid line..... */
};

data 'MENU' (176, "�O���b�h�`��A�C�R���p") {
	$"00B0 0000 0000 0000 0000 FFFF FFFF 1147"            /* .�........����.G */
	$"7269 6420 5370 6163 6520 2869 636F 6E29"            /* rid Space (icon) */
	$"044E 6F6E 6500 0000 0005 3332 2A33 3200"            /* .None.....32*32. */
	$"0000 0005 3136 2A31 3600 0000 0000"                 /* ....16*16..... */
};

data 'MENU' (177, "�O���b�h�̐F") {
	$"00B1 0000 0000 0000 0000 FFFF FFFF 0A47"            /* .�........�����G */
	$"7269 6420 436F 6C6F 7205 5768 6974 6500"            /* rid Color.White. */
	$"0000 0005 426C 6163 6B00 0000 0009 4375"            /* ....Black....�Cu */
	$"7374 6F6D 2E2E 2E00 0000 0000"                      /* stom........ */
};

data 'MENU' (178, "�O���b�h�̐F�A�C�R���p") {
	$"00B2 0000 0000 0000 0000 FFFF FFFF 1147"            /* .�........����.G */
	$"7269 6420 436F 6C6F 7220 2869 636F 6E29"            /* rid Color (icon) */
	$"0557 6869 7465 0000 0000 0542 6C61 636B"            /* .White.....Black */
	$"0000 0000 0943 7573 746F 6D2E 2E2E 0000"            /* ....�Custom..... */
	$"0000 00"                                            /* ... */
};

data 'MENU' (179, "Color Depth") {
	$"00B3 0000 0000 0000 0000 FFFF FFFF 0B43"            /* .�........����.C */
	$"6F6C 6F72 2044 6570 7468 0638 2062 6974"            /* olor Depth.8 bit */
	$"7300 0000 0007 3234 2062 6974 7300 0000"            /* s.....24 bits... */
	$"0000"                                               /* .. */
};

data 'DITL' (128, "About", purgeable) {
	$"000C 0000 0000 006C 00AF 007E 00D5 0402"            /* .......l.�.~.�.. */
	$"4F4B 0000 0000 0008 000B 0028 002B 8000"            /* OK.........(.+�. */
	$"0000 0000 0035 0021 0041 0071 880B 5573"            /* .....5.!.A.q�.Us */
	$"6564 2043 6F75 6E74 3A00 0000 0000 0042"            /* ed Count:......B */
	$"0021 004E 0071 880A 5573 6564 2054 696D"            /* .!.N.q��Used Tim */
	$"653A 0000 0000 004F 0021 005B 0071 880C"            /* e:.....O.!.[.q�. */
	$"4672 6565 204D 656D 6F72 793A 0000 0000"            /* Free Memory:.... */
	$"0035 0069 0041 00BE 8805 5E32 2089 F100"            /* .5.i.A.��.^2 ��. */
	$"0000 0000 0042 0069 004E 00BE 8800 0000"            /* .....B.i.N.��... */
	$"0000 004F 0069 005B 00BE 8800 0000 0000"            /* ...O.i.[.��..... */
	$"000C 003A 001C 00D0 8802 5E30 0000 0000"            /* ...:...Ј.^0.... */
	$"0030 001A 0061 00C5 C002 0082 0000 0000"            /* .0...a.��..�.... */
	$"001B 0099 002B 00CF 8802 5E31 0000 0000"            /* ...�.+.ψ.^1.... */
	$"0000 0000 0000 0000 0104 0001 0080 0000"            /* .............�.. */
	$"0000 006C 0047 007E 00A5 040E 5375 7070"            /* ...l.G.~.�..Supp */
	$"6F72 7420 5765 622E 2E2E"                           /* ort Web... */
};

data 'DITL' (137, "Confirm Revert", purgeable) {
	$"0004 0000 0000 0046 0106 005A 014B 0407"            /* .......F...Z.K.. */
	$"4469 7363 6172 6400 0000 0000 0046 00B6"            /* Discard......F.� */
	$"005A 00FB 0406 4361 6E63 656C 0000 0000"            /* .Z.�..Cancel.... */
	$"000B 0040 0041 014C 8835 446F 2079 6F75"            /* ...@.A.L�5Do you */
	$"2072 6561 6C6C 7920 7761 6E74 2074 6F20"            /*  really want to  */
	$"6469 7363 6172 6420 616C 6C20 6368 616E"            /* discard all chan */
	$"6765 7320 746F 205E 3020 225E 3122 3F00"            /* ges to ^0 "^1"?. */
	$"0000 0000 000D 0011 002D 0031 A002 0002"            /* .........-.1�... */
	$"0000 0000 0000 0000 0000 0000 0104 0001"            /* ................ */
	$"0081"                                               /* .� */
};

data 'DITL' (134, "Save", purgeable) {
	$"000F 0000 0000 00A1 00FC 00B5 014C 0404"            /* .......�.�.�.L.. */
	$"5361 7665 0000 0000 0082 00FC 0096 014C"            /* Save.....�.�.�.L */
	$"0406 4361 6E63 656C 0000 0000 0000 0000"            /* ..Cancel........ */
	$"0000 0000 8104 0001 E865 0000 0000 0008"            /* ....�...�e...... */
	$"00EB 0018 0151 0000 0000 0000 0020 00FC"            /* .�...Q....... .� */
	$"0034 014C 0405 456A 6563 7400 0000 0000"            /* .4.L..Eject..... */
	$"003C 00FC 0050 014C 0407 4465 736B 746F"            /* .<.�.P.L..Deskto */
	$"7000 0000 0000 001D 000C 007F 00E6 0000"            /* p............�.. */
	$"0000 0000 0006 000C 0019 00E6 0000 0000"            /* ...........�.... */
	$"0000 0077 00FA 0078 014E C002 000B 0000"            /* ...w.�.x.N�..... */
	$"0000 009D 000F 00AD 00E3 1000 0000 0000"            /* ...�...�.�...... */
	$"0088 000F 0098 00E3 880B 4669 6C65 204E"            /* .�...�.�.File N */
	$"616D 6520 3A00 0000 0000 0058 00FC 006C"            /* ame :......X.�.l */
	$"014C 8000 0000 0000 00BB 004F 00CF 00DD"            /* .L�......�.O.�.� */
	$"0702 0085 0000 0000 00BD 00FC 00CF 0144"            /* ...�.....�.�.�.D */
	$"0505 5370 6C69 7485 0000 0000 00BD 000F"            /* ..Split�.....�.. */
	$"00CD 004C 880B 4669 6C65 2054 7970 6520"            /* .�.L�.File Type  */
	$"3AD2 0000 0000 0000 0000 0000 0000 0104"            /* :�.............. */
	$"0001 0086"                                          /* ...� */
};

data 'DITL' (130, "Pointillize", purgeable) {
	$"000E 0000 0000 0077 00BE 0089 00F0 0402"            /* .......w.�.�.�.. */
	$"4F4B 0000 0000 0077 0083 0089 00B5 0406"            /* OK.....w.�.�.�.. */
	$"4361 6E63 656C 0000 0000 001C 0021 002E"            /* Cancel.......!.. */
	$"0077 0605 5768 6974 6500 0000 0000 002E"            /* .w..White....... */
	$"0021 0040 0077 0609 5065 6E20 436F 6C6F"            /* .!.@.w.�Pen Colo */
	$"7200 0000 0000 0040 0021 0052 0077 060D"            /* r......@.!.R.w.. */
	$"4F72 6967 696E 616C 2050 6963 7400 0000"            /* Original Pict... */
	$"0000 0051 002D 0063 0070 0507 4C69 6768"            /* ...Q.-.c.p..Ligh */
	$"7465 6E00 0000 0000 001C 00A5 002E 00DC"            /* ten........�...� */
	$"0604 4D61 6E79 0000 0000 002D 00A5 0040"            /* ..Many.....-.�.@ */
	$"00DC 0606 4E6F 726D 616C 0000 0000 003F"            /* .�..Normal.....? */
	$"00A5 0051 00DC 0603 4665 7700 0000 0000"            /* .�.Q.�..Few..... */
	$"0050 00A5 0062 00DC 0606 5370 6172 7365"            /* .P.�.b.�..Sparse */
	$"0000 0000 0014 0011 006A 0084 C002 0082"            /* .........j.��..� */
	$"0000 0000 000A 0017 001B 005F 880A 4261"            /* .....�....._��Ba */
	$"636B 6772 6F75 6E64 0000 0000 0014 0093"            /* ckground.......� */
	$"006A 00EE C002 0082 0000 0000 000A 009A"            /* .j.��..�.....�.� */
	$"001A 00E8 880A 446F 7420 416D 6F75 6E74"            /* ...��Dot Amount */
	$"0000 0000 0000 0000 0000 0000 0104 0001"            /* ................ */
	$"0082"                                               /* .� */
};

data 'DITL' (138, "Rotate", purgeable) {
	$"0004 0000 0000 0047 009C 005B 00D6 0402"            /* .......G.�.[.�.. */
	$"4F4B 0000 0000 0047 0058 005B 0092 0406"            /* OK.....G.X.[.�.. */
	$"4361 6E63 656C 0000 0000 0027 0057 0037"            /* Cancel.....'.W.7 */
	$"00CC 1001 30F1 0000 0000 000B 000F 001B"            /* .�..0�.......... */
	$"008A 8811 416E 676C 6520 2843 6C6F 636B"            /* .��.Angle (Clock */
	$"7769 7365 2900 0000 0000 0000 0000 0000"            /* wise)........... */
	$"0000 0104 0001 008A"                                /* .......� */
};

data 'DITL' (139, "Wind", purgeable) {
	$"000A 0000 0000 0052 0089 0066 00C3 0402"            /* .�.....R.�.f.�.. */
	$"4F4B 0000 0000 0052 0045 0066 007F 0406"            /* OK.....R.E.f.... */
	$"4361 6E63 656C 0000 0000 001C 001B 002E"            /* Cancel.......... */
	$"0057 0605 5269 6768 7400 0000 0000 002F"            /* .W..Right....../ */
	$"001B 0041 0057 0604 4C65 6674 0000 0000"            /* ...A.W..Left.... */
	$"001C 007B 002E 00B7 0606 4765 6E74 6C65"            /* ...{...�..Gentle */
	$"0000 0000 002F 007B 0041 00B7 0606 5374"            /* ...../.{.A.�..St */
	$"726F 6E67 0000 0000 0014 000E 0047 0061"            /* rong.........G.a */
	$"C002 0082 0000 0000 000A 0013 001A 004A"            /* �..�.....�.....J */
	$"8809 4469 7265 6374 696F 6E00 0000 0000"            /* ��Direction..... */
	$"0014 006E 0047 00C1 C002 0082 0000 0000"            /* ...n.G.��..�.... */
	$"000A 0073 001A 009E 8805 506F 7765 7200"            /* .�.s...��.Power. */
	$"0000 0000 0000 0000 0000 0000 0104 0001"            /* ................ */
	$"008B"                                               /* .� */
};

data 'DITL' (144, "Image Size", purgeable) {
	$"000F 0000 0000 00E6 008B 00FA 00C5 0402"            /* .......�.�.�.�.. */
	$"4F4B 0000 0000 00E6 0037 00FA 0081 0406"            /* OK.....�.7.�.�.. */
	$"4361 6E63 656C 0000 0000 0024 0049 0034"            /* Cancel.....$.I.4 */
	$"0071 1002 5E30 0000 0000 003D 0049 004D"            /* .q..^0.....=.I.M */
	$"0071 1002 5E31 0000 0000 0084 001E 0096"            /* .q..^1.....�...� */
	$"00B1 050F 5472 616E 7366 6F72 6D20 496D"            /* .�..Transform Im */
	$"6167 6500 0000 0000 0096 002B 00A8 0095"            /* age......�.+.�.� */
	$"050B 7769 7468 2044 6974 6865 7200 0000"            /* ..with Dither... */
	$"0000 0023 007B 0037 00C6 0702 0083 0000"            /* ...#.{.7.�...�.. */
	$"0000 003C 007B 0050 00C6 0702 0083 0000"            /* ...<.{.P.�...�.. */
	$"0000 0055 0022 0067 00BD 0515 436F 6E73"            /* ...U.".g.�..Cons */
	$"7472 6169 6E20 5072 6F70 6F72 7469 6F6E"            /* train Proportion */
	$"7300 0000 0000 00C4 001E 00D8 007F 0702"            /* s......�...�.... */
	$"008B 0000 0000 0024 001B 0034 0043 8805"            /* .�.....$...4.C�. */
	$"5769 6474 6800 0000 0000 000D 000E 001D"            /* Width........... */
	$"0079 8810 4E65 7720 496D 6167 6520 5369"            /* .y�.New Image Si */
	$"7A65 203A 0000 0000 0071 000E 0081 0059"            /* ze :.....q...�.Y */
	$"8808 4F70 7469 6F6E 203A 0000 0000 003D"            /* �.Option :.....= */
	$"001B 004D 0043 8806 4865 6967 6874 0000"            /* ...M.C�.Height.. */
	$"0000 0000 0000 0000 0000 0104 0001 0090"            /* ...............� */
	$"0000 0000 00B1 000E 00C1 0091 8813 4D61"            /* .....�...�.��.Ma */
	$"7267 696E 2042 6163 6B67 726F 756E 6420"            /* rgin Background  */
	$"3A00"                                               /* :. */
};

data 'DITL' (300, "preferences", purgeable) {
	$"0003 0000 0000 011A 0111 012E 0157 0404"            /* .............W.. */
	$"5361 7665 0000 0000 011A 00C0 012E 0106"            /* Save.......�.... */
	$"0405 4361 6365 6C00 0000 0000 0008 0000"            /* ..Cacel......... */
	$"0018 0140 0702 0081 0000 0000 0000 0000"            /* ...@...�........ */
	$"0000 0000 0104 0001 012C"                           /* ........., */
};

data 'DITL' (301, "File Prefs", purgeable) {
	$"0016 0000 0000 0039 0029 004D 0105 0702"            /* .......9.).M.... */
	$"008A 0000 0000 0069 00E5 0076 00FD 0403"            /* .�.....i.�.v.�.. */
	$"7365 7465 0000 0000 007B 00E5 0088 00FD"            /* sete.....{.�.�.� */
	$"0403 7365 7465 0000 0000 008D 00E5 009A"            /* ..sete.....�.�.� */
	$"00FD 0403 7365 7465 0000 0000 009F 00E5"            /* .�..sete.....�.� */
	$"00AC 00FD 0403 7365 7469 0000 0000 0066"            /* .�.�..seti.....f */
	$"0069 0076 00DE 880A 5369 6D70 6C65 5465"            /* .i.v.ވ�SimpleTe */
	$"7874 0000 0000 0078 0069 0088 00DF 8808"            /* xt.....x.i.�.߈. */
	$"636C 6970 3267 6966 0000 0000 008A 0069"            /* clip2gif.....�.i */
	$"009A 00DF 8809 4963 6F6E 5061 7274 7965"            /* .�.߈�IconPartye */
	$"0000 0000 009C 0069 00AC 00DF 8807 5265"            /* .....�.i.�.߈.Re */
	$"7345 6469 7461 0000 0000 00C4 0029 00D6"            /* sEdita.....�.).� */
	$"00FA 0516 5573 6520 4E61 7669 6761 7469"            /* .�..Use Navigati */
	$"6F6E 5365 7276 6963 6573 0000 0000 00D7"            /* onServices.....� */
	$"0029 00E9 00FA 0511 4164 6420 7072 6576"            /* .).�.�..Add prev */
	$"6965 7720 6963 6F6E 7300 0000 0000 0026"            /* iew icons......& */
	$"001A 0036 0088 8809 5D20 5374 6172 7475"            /* ...6.���] Startu */
	$"7067 0000 0000 0000 0000 0000 0000 0106"            /* pg.............. */
	$"0008 012D 0000 0000 0000 0066 0027 0076"            /* ...-.......f.'.v */
	$"0067 8806 5049 4354 8146 0000 0000 0078"            /* .g�.PICT�F.....x */
	$"0027 0088 0067 8805 4749 4681 4674 0000"            /* .'.�.g�.GIF�Ft.. */
	$"0000 0055 001A 0065 00AD 8816 5D20 4372"            /* ...U...e.��.] Cr */
	$"6561 746F 7220 4170 706C 6963 6174 696F"            /* eator Applicatio */
	$"6E73 0000 0000 0075 0020 0076 00E8 C002"            /* ns.....u. .v.��. */
	$"008A 0000 0000 0087 0020 0088 00E8 C002"            /* .�.....�. .�.��. */
	$"008A 0000 0000 009C 0027 00AC 0067 880A"            /* .�.....�.'.�.g�� */
	$"5265 736F 7572 6365 203A 0000 0000 00AB"            /* Resource :.....� */
	$"0020 00AC 00E8 C002 008A 0000 0000 008A"            /* . .�.��..�.....� */
	$"0027 009A 0067 8805 504E 4781 4620 0000"            /* .'.�.g�.PNG�F .. */
	$"0000 0099 0020 009A 00E8 C002 008A 0000"            /* ...�. .�.��..�.. */
	$"0000 00B3 001A 00C3 00AD 8808 5D20 4F74"            /* ...�...�.��.] Ot */
	$"6865 7273"                                          /* hers */
};

data 'DITL' (302, "Paint Prefs", purgeable) {
	$"000E 0000 0000 003D 0031 004D 0059 1000"            /* .......=.1.M.Y.. */
	$"0000 0000 003D 0076 004D 009E 1000 0000"            /* .....=.v.M.�.... */
	$"0000 003B 00C8 004F 011E 0702 0092 0000"            /* ...;.�.O.....�.. */
	$"0000 006B 00C8 007F 011E 0702 0082 0000"            /* ...k.�.......�.. */
	$"0000 0088 002B 009A 0113 051F 416C 7761"            /* ...�.+.�....Alwa */
	$"7973 2061 736B 2063 7265 6174 696E 6720"            /* ys ask creating  */
	$"6120 6E65 7720 696D 6167 658B 0000 0000"            /* a new image�.... */
	$"006B 002F 007F 0090 0702 008B 0000 0000"            /* .k./...�...�.... */
	$"00B7 002B 00CB 00DD 040F 3C6E 6F74 2073"            /* .�.+.�.�..<not s */
	$"7065 6369 6669 6564 3E73 0000 0000 00D1"            /* pecified>s.....� */
	$"002B 00E3 0122 0528 5573 6520 4578 7465"            /* .+.�.".(Use Exte */
	$"726E 616C 2045 6469 746F 7220 746F 2065"            /* rnal Editor to e */
	$"6469 7420 3332 6269 7473 2069 636F 6E73"            /* dit 32bits icons */
	$"0000 0000 0026 001B 0036 00A7 8814 5D20"            /* .....&...6.��.]  */
	$"4465 6661 756C 7420 496D 6167 6520 5369"            /* Default Image Si */
	$"7A65 0000 0000 003E 0062 004C 0071 8801"            /* ze.....>.b.L.q�. */
	$"7820 0000 0000 0056 00B4 0066 00FF 8806"            /* x .....V.�.f.��. */
	$"5D20 5A6F 6F6D 0000 0000 0000 0000 0000"            /* ] Zoom.......... */
	$"0000 0106 0008 012E 0000 0000 0000 0056"            /* ...............V */
	$"001B 0066 00A7 880C 5D20 4261 636B 6772"            /* ...f.��.] Backgr */
	$"6F75 6E64 0000 0000 00A1 001B 00B1 00A7"            /* ound.....�...�.� */
	$"8811 5D20 4578 7465 726E 616C 2045 6469"            /* �.] External Edi */
	$"746F 7292 0000 0000 0026 00B4 0036 0118"            /* tor�.....&.�.6.. */
	$"8807 5D20 4465 7074 6800"                           /* �.] Depth. */
};

data 'DITL' (303, "Tool Prefs", purgeable) {
	$"000F 0000 0000 0036 0027 0048 0113 0526"            /* .......6.'.H...& */
	$"4572 6173 6520 746F 2073 616D 6520 636F"            /* Erase to same co */
	$"6C6F 7220 2850 656E 6369 6C20 746F 6F6C"            /* lor (Pencil tool */
	$"206F 6E6C 7929 0000 0000 0047 0037 0058"            /*  only).....G.7.X */
	$"00FC 051D 4F6E 6C79 2077 6865 6E20 5065"            /* .�..Only when Pe */
	$"6E20 5369 7A65 2065 7175 616C 7320 312A"            /* n Size equals 1* */
	$"3100 0000 0000 00CE 0027 00E0 00D7 0519"            /* 1......�.'.�.�.. */
	$"4D61 736B 696E 6720 6279 2073 656C 6563"            /* Masking by selec */
	$"7469 6F6E 2061 7265 6100 0000 0000 00A8"            /* tion area......� */
	$"008B 00B6 00DB 0702 0086 0000 0000 00F7"            /* .�.�.�...�.....� */
	$"0027 0109 00A9 050D 4368 616E 6765 2043"            /* .'.�.�..Change C */
	$"7572 736F 7200 0000 0000 005A 0027 006C"            /* ursor......Z.'.l */
	$"013F 051F 4368 616E 6765 2043 7572 736F"            /* .?..Change Curso */
	$"7220 6163 636F 7264 696E 6720 746F 2053"            /* r according to S */
	$"697A 6500 0000 0000 006C 0037 007E 010A"            /* ize......l.7.~.� */
	$"0513 4368 616E 6765 2043 7572 736F 7220"            /* ..Change Cursor  */
	$"636F 6C6F 7200 0000 0000 007F 0027 0091"            /* color........'.� */
	$"0120 051C 5573 6520 4572 6173 6572 2074"            /* . ..Use Eraser t */
	$"6F6F 6C20 6279 2043 6D64 2B43 6C69 636B"            /* ool by Cmd+Click */
	$"0000 0000 0026 001A 0036 00A6 8816 5D20"            /* .....&...6.��.]  */
	$"5065 6E63 696C 202F 2045 7261 7365 7220"            /* Pencil / Eraser  */
	$"746F 6F6C 0000 0000 00BD 001A 00CD 007E"            /* tool.....�...�.~ */
	$"880E 5D20 4D61 7271 7565 6520 746F 6F6C"            /* �.] Marquee tool */
	$"0000 0000 0096 001A 00A6 0070 880A 5D20"            /* .....�...�.p��]  */
	$"446F 7420 746F 6F6C 0000 0000 00AD 00E2"            /* Dot tool.....�.� */
	$"00B1 00EB C002 008F 0000 0000 00A7 0026"            /* .�.��..�.....�.& */
	$"00B7 0068 880B 446F 7420 7370 6565 6420"            /* .�.h�.Dot speed  */
	$"3A00 0000 0000 00AD 007B 00B1 0082 4002"            /* :......�.{.�.�@. */
	$"0090 0000 0000 0000 0000 0000 0000 0106"            /* .�.............. */
	$"0008 012F 0000 0000 0000 00E7 001A 00F7"            /* .../.......�...� */
	$"007E 880C 5D20 5370 6F69 7420 546F 6F6C"            /* .~�.] Spoit Tool */
};

data 'DITL' (304, "PNG Prefs", purgeable) {
	$"000A 0000 0000 008B 0029 009D 00C4 050D"            /* .�.....�.).�.�.. */
	$"5573 6520 496E 7465 726C 6163 6500 0000"            /* Use Interlace... */
	$"0000 0037 0029 004B 00BF 0702 0089 0000"            /* ...7.).K.�...�.. */
	$"0000 00B4 0029 00C8 00F1 0702 008C 0000"            /* ...�.).�.�...�.. */
	$"0000 004E 0035 0060 0117 051F 5573 6520"            /* ...N.5.`....Use  */
	$"7472 616E 7370 6172 656E 7420 636F 6C6F"            /* transparent colo */
	$"7220 696E 2073 6176 696E 6700 0000 0000"            /* r in saving..... */
	$"0062 0035 0074 0117 0520 5573 6520 7472"            /* .b.5.t... Use tr */
	$"616E 7370 6172 656E 7420 636F 6C6F 7220"            /* ansparent color  */
	$"696E 206C 6F61 6469 6E67 0000 0000 00E0"            /* in loading.....� */
	$"0029 00F2 013D 051F 4F70 7469 6D69 7A65"            /* .).�.=..Optimize */
	$"2069 6D61 6765 2077 6974 6820 7573 6564"            /*  image with used */
	$"2063 6F6C 6F72 7372 0000 0000 0026 001A"            /*  colorsr.....&.. */
	$"0036 00C6 8818 5D20 5472 616E 7370 6172"            /* .6.ƈ.] Transpar */
	$"656E 7420 4261 636B 6772 6F75 6E64 0000"            /* ent Background.. */
	$"0000 0000 0000 0000 0000 0106 0008 0130"            /* ...............0 */
	$"0000 0000 0000 007A 001B 008A 0089 880B"            /* .......z...�.��. */
	$"5D20 496E 7465 726C 6163 653D 0000 0000"            /* ] Interlace=.... */
	$"00A3 001B 00B3 00AC 8813 5D20 436F 6D70"            /* .�...�.��.] Comp */
	$"7265 7373 696F 6E20 4C65 7665 6C00 0000"            /* ression Level... */
	$"0000 00CF 001B 00DF 0089 880E 5D20 4F70"            /* ...�...�.��.] Op */
	$"7469 6D69 7A61 7469 6F6E"                           /* timization */
};

data 'DITL' (305, "Tablet Prefs", purgeable) {
	$"0006 0000 0000 0039 002A 004B 00F7 0520"            /* .......9.*.K.�.  */
	$"5573 6520 5072 6573 7375 7265 2028 4566"            /* Use Pressure (Ef */
	$"6665 6374 2074 6F20 6F70 6163 6974 7929"            /* fect to opacity) */
	$"0000 0000 004B 003C 005D 00FA 851D 4967"            /* .....K.<.].��.Ig */
	$"6E6F 7265 2073 6574 7469 6E67 206F 6620"            /* nore setting of  */
	$"5065 6E20 4F70 6163 6974 7900 0000 0000"            /* Pen Opacity..... */
	$"0076 002A 0088 00AF 050A 5573 6520 4572"            /* .v.*.�.�.�Use Er */
	$"6173 6572 0000 0000 0088 003C 009A 00EA"            /* aser.....�.<.�.� */
	$"0513 5573 6520 4572 6173 6572 2050 7265"            /* ..Use Eraser Pre */
	$"7373 7572 6500 0000 0000 0026 001A 0036"            /* ssure......&...6 */
	$"007E 880E 5D20 5469 7020 5072 6573 7375"            /* .~�.] Tip Pressu */
	$"7265 0000 0000 0000 0000 0000 0000 0106"            /* re.............. */
	$"0008 0131 0000 0000 0000 0064 001A 0074"            /* ...1.......d...t */
	$"007E 8808 5D20 4572 6173 6572"                      /* .~�.] Eraser */
};

data 'DITL' (135, "New Image", purgeable) {
	$"000E 0000 0000 0094 00D8 00A8 011C 0403"            /* .......�.�.�.... */
	$"4E65 7700 0000 0000 0094 0088 00A8 00CC"            /* New......�.�.�.� */
	$"0406 4361 6E63 656C 0000 0000 0026 0022"            /* ..Cancel.....&." */
	$"0036 004A 1000 0000 0000 0026 0067 0036"            /* .6.J.......&.g.6 */
	$"008F 1000 0000 0000 0023 00BF 0037 0115"            /* .�.......#.�.7.. */
	$"0702 0092 0000 0000 006C 00BF 0080 0115"            /* ...�.....l.�.�.. */
	$"0702 0082 0000 0000 003D 0012 004F 00A2"            /* ...�.....=...O.� */
	$"050F 5265 6665 7220 436C 6970 626F 6172"            /* ..Refer Clipboar */
	$"648B 0000 0000 006C 0020 0080 0081 0702"            /* d�.....l. .�.�.. */
	$"008B 0000 0000 000E 000C 001E 0070 880C"            /* .�...........p�. */
	$"496D 6167 6520 5369 7A65 203A 0000 0000"            /* Image Size :.... */
	$"0027 0053 0035 0062 8801 786F 0000 0000"            /* .'.S.5.b�.xo.... */
	$"0058 00AB 0068 010F 8806 5A6F 6F6D 203A"            /* .X.�.h..�.Zoom : */
	$"0000 0000 0000 0000 0000 0000 0104 0001"            /* ................ */
	$"0087 0000 0000 0058 000C 0068 0070 880C"            /* .�.....X...h.p�. */
	$"4261 636B 6772 6F75 6E64 203A 0000 0000"            /* Background :.... */
	$"000E 00AB 001E 010F 8807 4465 7074 6820"            /* ...�....�.Depth  */
	$"3A00 0000 0000 0006 0005 008B 0125 8000"            /* :..........�.%�. */
};

data 'DITL' (131, "Icon Info", purgeable) {
	$"0007 0000 0000 0048 00A2 005C 00E4 0402"            /* .......H.�.\.�.. */
	$"4F4B 0000 0000 0048 0053 005C 0095 0406"            /* OK.....H.S.\.�.. */
	$"4361 6E63 656C 0000 0000 0015 0011 0035"            /* Cancel.........5 */
	$"0031 8000 0000 0000 0010 0073 0020 00B0"            /* .1�........s. .� */
	$"1002 5E30 0000 0000 0029 0073 0039 00E5"            /* ..^0.....).s.9.� */
	$"1002 5E31 0000 0000 0010 0043 0020 006C"            /* ..^1.......C. .l */
	$"8804 4944 203A 0000 0000 0029 0043 0039"            /* �.ID :.....).C.9 */
	$"006C 8806 4E61 6D65 203A 0000 0000 0000"            /* .l�.Name :...... */
	$"0000 0000 0000 0104 0001 0083"                      /* ...........� */
};

data 'DITL' (132, "Input Figure", purgeable) {
	$"0008 0000 0000 0067 0086 007B 00C0 0402"            /* .......g.�.{.�.. */
	$"4F4B 0000 0000 0067 003E 007B 0078 0406"            /* OK.....g.>.{.x.. */
	$"4361 6E63 656C 0000 0000 002D 0062 003D"            /* Cancel.....-.b.= */
	$"0098 1001 3100 0000 0000 0046 005D 005A"            /* .�..1......F.].Z */
	$"00BE 0702 0084 0000 0000 000D 005D 0021"            /* .�...�.......].! */
	$"00BE 0702 0088 0000 0000 002D 0012 003D"            /* .�...�.....-...= */
	$"0057 880C 4C69 6E65 2057 6964 7468 203A"            /* .W�.Line Width : */
	$"0000 0000 0047 0012 0057 0054 8809 4F70"            /* .....G...W.T��Op */
	$"6163 6974 7920 3A00 0000 0000 0000 0000"            /* acity :......... */
	$"0000 0000 0104 0001 0084 0000 0000 000F"            /* .........�...... */
	$"0012 001F 0054 8806 5479 7065 203A"                 /* .....T�.Type : */
};

data 'DITL' (306, "Other Prefs", purgeable) {
	$"0008 0000 0000 0036 0027 0048 0143 052C"            /* .......6.'.H.C., */
	$"4869 6C69 7465 2050 616C 6574 7465 2043"            /* Hilite Palette C */
	$"6F6C 6F72 7320 6279 2063 6861 6E67 696E"            /* olors by changin */
	$"6720 436F 6C6F 7220 4D6F 6465 0000 0000"            /* g Color Mode.... */
	$"004B 0027 005D 0126 051E 4163 7469 7661"            /* .K.'.].&..Activa */
	$"7465 2064 7261 672D 6E2D 6472 6F70 7065"            /* te drag-n-droppe */
	$"6420 7769 6E64 6F77 0000 0000 0060 0027"            /* d window.....`.' */
	$"0072 0126 0521 446F 6E27 7420 6372 6561"            /* .r.&.!Don't crea */
	$"7465 2064 6F75 6768 6E75 742D 7368 6170"            /* te doughnut-shap */
	$"6564 204D 6173 6B00 0000 0000 0075 0027"            /* ed Mask......u.' */
	$"0087 0147 0529 4170 706C 7920 636F 6E74"            /* .�.G.)Apply cont */
	$"696E 756F 7573 2049 4473 2077 6865 6E20"            /* inuous IDs when  */
	$"696D 706F 7274 696E 6720 6963 6F6E 7300"            /* importing icons. */
	$"0000 0000 008A 0027 009C 0126 0516 4164"            /* .....�.'.�.&..Ad */
	$"6420 466F 7265 6772 6F75 6E64 2028 4C61"            /* d Foreground (La */
	$"7965 7229 0000 0000 009F 0027 00B1 0126"            /* yer).....�.'.�.& */
	$"0524 436F 7079 2069 636F 6E20 6461 7461"            /* .$Copy icon data */
	$"2077 6865 6E20 636F 7079 696E 6720 5069"            /*  when copying Pi */
	$"6374 7572 6573 0000 0000 00B4 0027 00C6"            /* ctures.....�.'.� */
	$"0126 0516 5573 6520 4879 7065 7243 6172"            /* .&..Use HyperCar */
	$"6420 5061 7474 6572 6E73 0000 0000 0026"            /* d Patterns.....& */
	$"001A 0036 0074 880D 5D20 4F74 6865 7220"            /* ...6.t�.] Other  */
	$"5072 6566 7300 0000 0000 0000 0000 0000"            /* Prefs........... */
	$"0000 0106 0008 0132 0000"                           /* .......2.. */
};

data 'DITL' (133, "Input Text", purgeable) {
	$"0012 0000 0000 0145 0165 0159 019F 0402"            /* .......E.e.Y.�.. */
	$"4F4B 0000 0000 0145 011F 0159 0159 0406"            /* OK.....E...Y.Y.. */
	$"4361 6E63 656C 0000 0000 000E 0055 0022"            /* Cancel.......U." */
	$"0109 0702 0087 0000 0000 0029 0058 0039"            /* .�...�.....).X.9 */
	$"0087 1002 5E30 0000 0000 0042 0058 0052"            /* .�..^0.....B.X.R */
	$"0185 1002 5E31 0000 0000 006D 0036 007F"            /* .�..^1.....m.6.. */
	$"006D 0504 426F 6C64 0000 0000 006D 0095"            /* .m..Bold.....m.� */
	$"007F 00D5 0506 4974 616C 6963 0000 0000"            /* ...�..Italic.... */
	$"006D 00F0 007F 0147 0509 556E 6465 726C"            /* .m.�...G.�Underl */
	$"696E 6500 0000 0000 007F 0036 0091 007E"            /* ine........6.�.~ */
	$"0507 4F75 746C 696E 6500 0000 0000 007F"            /* ..Outline....... */
	$"0095 0091 00E0 0506 5368 6164 6F77 0000"            /* .�.�.�..Shadow.. */
	$"0000 007F 00F0 0091 0166 050C 416E 7469"            /* .....�.�.f..Anti */
	$"2D41 6C69 6173 6564 0000 0000 00B5 001F"            /* -Aliased.....�.. */
	$"012F 018D 8000 0000 0000 0029 0012 0039"            /* ./.��......)...9 */
	$"0050 8806 5369 7A65 203A 0000 0000 0042"            /* .P�.Size :.....B */
	$"0012 0052 0050 8806 5465 7874 203A 0000"            /* ...R.P�.Text :.. */
	$"0000 00A9 0011 013A 019A C002 0082 0000"            /* ...�...:.��..�.. */
	$"0000 0096 0013 00A6 0061 8809 5072 6576"            /* ...�...�.a��Prev */
	$"6965 7720 3A00 0000 0000 005B 0012 006B"            /* iew :......[...k */
	$"0050 8807 5374 796C 6520 3A00 0000 0000"            /* .P�.Style :..... */
	$"0000 0000 0000 0000 0104 0001 0085 0000"            /* .............�.. */
	$"0000 0010 0012 0020 0050 8806 466F 6E74"            /* ....... .P�.Font */
	$"203A"                                               /*  : */
};

data 'DITL' (401, "save", purgeable) {
	$"000B 0000 0000 00A1 00FC 00B5 014C 0404"            /* .......�.�.�.L.. */
	$"5361 7665 0000 0000 0082 00FC 0096 014C"            /* Save.....�.�.�.L */
	$"0406 4361 6E63 656C 0000 0000 0000 0000"            /* ..Cancel........ */
	$"0000 0000 8104 0001 E865 0000 0000 0008"            /* ....�...�e...... */
	$"00EB 0018 0151 0000 0000 0000 0020 00FC"            /* .�...Q....... .� */
	$"0034 014C 0405 456A 6563 7400 0000 0000"            /* .4.L..Eject..... */
	$"003C 00FC 0050 014C 0407 4465 736B 746F"            /* .<.�.P.L..Deskto */
	$"7000 0000 0000 001D 000C 007F 00E6 0000"            /* p............�.. */
	$"0000 0000 0006 000C 0019 00E6 0000 0000"            /* ...........�.... */
	$"0000 0077 00FA 0078 014E C002 000B 0000"            /* ...w.�.x.N�..... */
	$"0000 009D 000F 00AD 00E3 1000 0000 0000"            /* ...�...�.�...... */
	$"0088 000F 0098 00E3 880B 4669 6C65 206E"            /* .�...�.�.File n */
	$"616D 6520 3A00 0000 0000 0058 00FC 006C"            /* ame :......X.�.l */
	$"014C 8000"                                          /* .L�. */
};

data 'DITL' (400, "open", purgeable) {
	$"0009 0000 0000 008C 0100 00A0 0150 0404"            /* .�.....�...�.P.. */
	$"4F70 656E 0000 0000 006C 0100 0080 0150"            /* Open.....l...�.P */
	$"0406 4361 6E63 656C 0000 0000 0000 0000"            /* ..Cancel........ */
	$"0000 0000 8104 0001 E866 0000 0000 0008"            /* ....�...�f...... */
	$"00F6 0018 015A 0000 0000 0000 0024 0100"            /* .�...Z.......$.. */
	$"0038 0150 0405 456A 6563 7400 0000 0000"            /* .8.P..Eject..... */
	$"0040 0100 0054 0150 0407 4465 736B 746F"            /* .@...T.P..Deskto */
	$"7000 0000 0000 0022 000C 00A4 00EE 0000"            /* p......"...�.�.. */
	$"0000 0000 0008 000C 001B 00EE 0000 0000"            /* ...........�.... */
	$"0000 005F 00FF 0060 0151 C002 000B 0000"            /* ..._.�.`.Q�..... */
	$"0000 0000 0000 0000 0000 8104 0001 03E8"            /* ..........�....� */
};

data 'DITL' (500, "Save Dialog for Nav", purgeable) {
	$"0003 0000 0000 000A 004E 001E 00DC 0702"            /* .......�.N...�.. */
	$"0085 0000 0000 000C 00FF 001E 0148 0505"            /* .�.......�...H.. */
	$"5370 6C69 7454 0000 0000 000C 000E 001C"            /* SplitT.......... */
	$"004B 880B 4669 6C65 2054 7970 6520 3A00"            /* .K�.File Type :. */
	$"0000 0000 0000 0000 0000 0000 0106 0008"            /* ................ */
	$"01F4 0000"                                          /* .�.. */
};

data 'DITL' (136, "Confirm Destroy", purgeable) {
	$"0004 0000 0000 0046 0106 005A 014B 0407"            /* .......F...Z.K.. */
	$"4465 7374 726F 7900 0000 0000 0046 00B6"            /* Destroy......F.� */
	$"005A 00FB 0406 4361 6E63 656C 0000 0000"            /* .Z.�..Cancel.... */
	$"000B 0040 0041 014C 8802 5E30 0000 0000"            /* ...@.A.L�.^0.... */
	$"000D 0011 002D 0031 A002 0002 0000 0000"            /* .....-.1�....... */
	$"0000 0000 0000 0000 0104 0001 0088"                 /* .............� */
};

data 'DITL' (129, "Confirm Save", purgeable) {
	$"0005 0000 0000 0046 0106 005A 014B 0404"            /* .......F...Z.K.. */
	$"5361 7665 0000 0000 0046 00B6 005A 00FB"            /* Save.....F.�.Z.� */
	$"0406 4361 6E63 656C 0000 0000 0046 0049"            /* ..Cancel.....F.I */
	$"005A 008E 040A 446F 6E27 7420 5361 7665"            /* .Z.�.�Don't Save */
	$"0000 0000 000B 0040 0041 014C 8817 5361"            /* .......@.A.L�.Sa */
	$"7665 205E 3120 225E 3222 2062 6566 6F72"            /* ve ^1 "^2" befor */
	$"6520 5E30 3F00 0000 0000 000D 0011 002D"            /* e ^0?..........- */
	$"0031 A002 0002 0000 0000 0000 0000 0000"            /* .1�............. */
	$"0000 0104 0001 0081"                                /* .......� */
};

data 'DITL' (140, "Edit Custom Icon", purgeable) {
	$"0005 0000 0000 0046 0106 005A 014B 0404"            /* .......F...Z.K.. */
	$"4564 6974 0000 0000 0046 00B6 005A 00FB"            /* Edit.....F.�.Z.� */
	$"0406 4361 6E63 656C 0000 0000 0046 0049"            /* ..Cancel.....F.I */
	$"005A 00A5 0409 4164 6420 4963 6F6E 7300"            /* .Z.�.�Add Icons. */
	$"0000 0000 000B 0040 0041 014C 887E 4669"            /* .......@.A.L�~Fi */
	$"6C65 2022 5E30 2220 6861 7320 6E6F 2072"            /* le "^0" has no r */
	$"6573 6F75 7263 6520 666F 726B 2C20 6F72"            /* esource fork, or */
	$"2068 6173 206F 6E6C 7920 6375 7374 6F6D"            /*  has only custom */
	$"2069 636F 6E2E 2044 6F20 796F 7520 7769"            /*  icon. Do you wi */
	$"7368 2074 6F20 6564 6974 2861 6464 2920"            /* sh to edit(add)  */
	$"636F 7374 6F6D 2069 636F 6E20 6F72 2061"            /* costom icon or a */
	$"6464 206F 7468 6572 2069 636F 6E73 2074"            /* dd other icons t */
	$"6F20 7468 6973 2066 696C 653F 0000 0000"            /* o this file?.... */
	$"000D 0011 002D 0031 A002 0002 0000 0000"            /* .....-.1�....... */
	$"0000 0000 0000 0000 0104 0001 0081"                 /* .............� */
};

data 'DITL' (141, "Add Icon", purgeable) {
	$"0003 0000 0000 0046 0106 005A 014B 0403"            /* .......F...Z.K.. */
	$"4164 6400 0000 0000 0046 00B6 005A 00FB"            /* Add......F.�.Z.� */
	$"0406 4361 6E63 656C 0000 0000 000B 0040"            /* ..Cancel.......@ */
	$"0041 014C 8802 5E30 0000 0000 000D 0011"            /* .A.L�.^0........ */
	$"002D 0031 A002 0002"                                /* .-.1�... */
};

data 'DITL' (142, "New Library", purgeable) {
	$"0004 0000 0000 0047 0094 005B 00D3 0406"            /* .......G.�.[.�.. */
	$"4372 6561 7465 0000 0000 0047 004D 005B"            /* Create.....G.M.[ */
	$"008C 0406 4361 6E63 656C 0000 0000 0027"            /* .�..Cancel.....' */
	$"002A 0037 00D1 100C 5573 6572 204C 6962"            /* .*.7.�..User Lib */
	$"7261 7279 0000 0000 000B 000F 001B 008B"            /* rary...........� */
	$"8812 446F 7420 4C69 6272 6172 7920 4E61"            /* �.Dot Library Na */
	$"6D65 203A 0000 0000 0000 0000 0000 0000"            /* me :............ */
	$"0104 0001 008E"                                     /* .....� */
};

data 'DITL' (143, "New icon", purgeable) {
	$"001F 0000 0000 00EA 0113 00FE 0163 0402"            /* .......�...�.c.. */
	$"4F4B 0000 0000 00EA 00B5 00FE 0105 0406"            /* OK.....�.�.�.... */
	$"4361 6E63 656C 0000 0000 0016 0137 0036"            /* Cancel.......7.6 */
	$"0157 8000 0000 0000 0010 0047 0020 0084"            /* .W�........G. .� */
	$"1002 5E30 0000 0000 0029 0047 0039 011C"            /* ..^0.....).G.9.. */
	$"1002 5E31 0000 0000 0055 0024 0067 0060"            /* ..^1.....U.$.g.` */
	$"0504 6963 6C38 0000 0000 0069 0024 007B"            /* ..icl8.....i.$.{ */
	$"0060 0504 6963 6C34 0000 0000 007D 0024"            /* .`..icl4.....}.$ */
	$"008F 0060 0504 4943 4E23 0000 0000 0055"            /* .�.`..ICN#.....U */
	$"0062 0067 009E 0504 6963 7338 0000 0000"            /* .b.g.�..ics8.... */
	$"0069 0062 007B 009E 0504 6963 7334 0000"            /* .i.b.{.�..ics4.. */
	$"0000 007D 0062 008F 009E 0504 6963 7323"            /* ...}.b.�.�..ics# */
	$"0000 0000 0069 00A0 007B 00DC 0504 696C"            /* .....i.�.{.�..il */
	$"3332 0000 0000 007D 00A0 008F 00DC 0504"            /* 32.....}.�.�.�.. */
	$"6C38 6D6B 0000 0000 0069 00DE 007B 011A"            /* l8mk.....i.�.{.. */
	$"0504 6973 3332 0000 0000 007D 00DE 008F"            /* ..is32.....}.�.� */
	$"011A 0504 7338 6D6B 0000 0000 0069 011D"            /* ....s8mk.....i.. */
	$"007B 0159 0504 6974 3332 0000 0000 007D"            /* .{.Y..it32.....} */
	$"011D 008F 0159 0504 7438 6D6B 0000 0000"            /* ...�.Y..t8mk.... */
	$"009D 0024 00B1 0060 0403 416C 6C00 0000"            /* .�.$.�.`..All... */
	$"0000 00BB 0024 00CF 0060 0405 436C 6561"            /* ...�.$.�.`..Clea */
	$"7200 0000 0000 009D 0074 00B1 00BA 0407"            /* r......�.t.�.�.. */
	$"5379 7374 656D 3700 0000 0000 009D 00C3"            /* System7......�.� */
	$"00B1 0109 0405 4F53 382E 3500 0000 0000"            /* .�.�..OS8.5..... */
	$"009D 0113 00B1 0159 0404 4F53 2058 0000"            /* .�...�.Y..OS X.. */
	$"0000 00BB 0074 00CF 00BA 0406 4C20 6F6E"            /* ...�.t.�.�..L on */
	$"6C79 0000 0000 00BB 00C3 00CF 0109 0406"            /* ly.....�.�.�.�.. */
	$"5320 6F6E 6C79 0000 0000 00BB 0113 00CF"            /* S only.....�...� */
	$"0159 0406 5820 6F6E 6C79 0000 0000 0010"            /* .Y..X only...... */
	$"0017 0020 0040 8804 4944 203A 0000 0000"            /* ... .@�.ID :.... */
	$"0029 0017 0039 0040 8806 4E61 6D65 203A"            /* .)...9.@�.Name : */
	$"0000 0000 0000 0000 0000 0000 0104 0001"            /* ................ */
	$"008F 0000 0000 004C 0014 00DD 0167 C002"            /* .�.....L...�.g�. */
	$"0082 0000 0000 0041 001C 0051 007E 880B"            /* .�.....A...Q.~�. */
	$"4963 6F6E 206B 696E 6420 3A00 0000 0000"            /* Icon kind :..... */
	$"0059 00DE 0065 011E C002 009B 0000 0000"            /* .Y.�.e..�..�.... */
	$"0093 001A 0094 0169 C002 008A"                      /* .�...�.i�..� */
};

data 'DITL' (145, "Edit Dot Lib", purgeable) {
	$"0006 0000 0000 009E 00AC 00B2 00FC 0404"            /* .......�.�.�.�.. */
	$"5361 7665 0000 0000 0080 00AC 0094 00FC"            /* Save.....�.�.�.� */
	$"0406 4361 6E63 656C 0000 0000 0013 0012"            /* ..Cancel........ */
	$"00B3 0099 0000 0000 0000 0015 00AC 0029"            /* .�.�.........�.) */
	$"00FC 0403 4164 6400 0000 0000 0034 00AC"            /* .�..Add......4.� */
	$"0048 00FC 0404 4564 6974 0000 0000 0053"            /* .H.�..Edit.....S */
	$"00AC 0067 00FC 0406 4465 6C65 7465 0000"            /* .�.g.�..Delete.. */
	$"0000 0073 00A8 0074 0102 C002 008A"                 /* ...s.�.t..�..� */
};

data 'DITL' (146, "Edit Dot Command", purgeable) {
	$"0004 0000 0000 0057 00A3 006B 00EE 0404"            /* .......W.�.k.�.. */
	$"5361 7665 0000 0000 0057 004A 006B 0095"            /* Save.....W.J.k.� */
	$"0406 4361 6E63 656C 0000 0000 0025 001B"            /* ..Cancel.....%.. */
	$"0035 00EE 1000 0000 0000 0040 0021 004C"            /* .5.�.......@.!.L */
	$"00E4 8000 0000 0000 000A 000C 001A 00E7"            /* .�......�.....� */
	$"8821 446F 7420 436F 6D6D 616E 6420 2875"            /* �!Dot Command (u */
	$"7020 746F 2033 3120 6368 6172 6163 7465"            /* p to 31 characte */
	$"7273 2900"                                          /* rs). */
};

data 'DITL' (147, "Delete Command", purgeable) {
	$"0004 0000 0000 0046 0106 005A 014B 0406"            /* .......F...Z.K.. */
	$"4465 6C65 7465 0000 0000 0046 00B6 005A"            /* Delete.....F.�.Z */
	$"00FB 0406 4361 6E63 656C 0000 0000 000B"            /* .�..Cancel...... */
	$"0040 0041 014C 8830 446F 2079 6F75 2077"            /* .@.A.L�0Do you w */
	$"6973 6820 746F 2064 656C 6574 6520 7365"            /* ish to delete se */
	$"6C65 6374 6564 2064 6F74 2063 6F6D 6D61"            /* lected dot comma */
	$"6E64 2022 5E30 223F 0000 0000 000D 0011"            /* nd "^0"?........ */
	$"002D 0031 A002 0002 0000 0000 0000 0000"            /* .-.1�........... */
	$"0000 0000 0104 0001 0088"                           /* .........� */
};

data 'DITL' (501, "export icon for Nav", purgeable) {
	$"0001 0000 0000 000A 004E 001E 0104 0702"            /* .......�.N...... */
	$"008D 0000 0000 000C 000E 001C 004B 880A"            /* .�...........K�� */
	$"4669 6C65 2054 7970 653A"                           /* File Type: */
};

data 'DITL' (148, "Export icon (custom save)", purgeable) {
	$"000E 0000 0000 00A1 00FC 00B5 014C 0404"            /* .......�.�.�.L.. */
	$"5361 7665 0000 0000 0082 00FC 0096 014C"            /* Save.....�.�.�.L */
	$"0406 4361 6E63 656C 0000 0000 0000 0000"            /* ..Cancel........ */
	$"0000 0000 8104 0001 E865 0000 0000 0008"            /* ....�...�e...... */
	$"00EB 0018 0151 0000 0000 0000 0020 00FC"            /* .�...Q....... .� */
	$"0034 014C 0405 456A 6563 7400 0000 0000"            /* .4.L..Eject..... */
	$"003C 00FC 0050 014C 0407 4465 736B 746F"            /* .<.�.P.L..Deskto */
	$"7000 0000 0000 001D 000C 007F 00E6 0000"            /* p............�.. */
	$"0000 0000 0006 000C 0019 00E6 0000 0000"            /* ...........�.... */
	$"0000 0077 00FA 0078 014E C002 000B 0000"            /* ...w.�.x.N�..... */
	$"0000 009D 000F 00AD 00E3 1000 0000 0000"            /* ...�...�.�...... */
	$"0088 000F 0098 00E3 880B 4963 6F6E 206E"            /* .�...�.�.Icon n */
	$"616D 6520 3A00 0000 0000 0058 00FC 006C"            /* ame :......X.�.l */
	$"014C 8000 0000 0000 00BF 004F 00CF 00B3"            /* .L�......�.O.�.� */
	$"0702 008D 0000 0000 00C1 000F 00D1 004C"            /* ...�.....�...�.L */
	$"880B 4963 6F6E 2074 7970 6520 3A00 0000"            /* �.Icon type :... */
	$"0000 0000 0000 0000 0000 0104 0001 0086"            /* ...............� */
};

data 'DITL' (149, "Grid Settings", purgeable) {
	$"000B 0000 0000 00B6 00A9 00CA 00EA 0402"            /* .......�.�.�.�.. */
	$"4F4B 0000 0000 00B6 005C 00CA 009D 0406"            /* OK.....�.\.�.�.. */
	$"4361 6E63 656C 0000 0000 001F 0024 0033"            /* Cancel.......$.3 */
	$"00CE 0702 008E 0000 0000 0071 0024 0085"            /* .�...�.....q.$.� */
	$"00CE 0702 0090 0000 0000 0038 0024 004C"            /* .�...�.....8.$.L */
	$"00CE 0702 008F 0000 0000 0089 0024 009D"            /* .�...�.....�.$.� */
	$"00CE 0702 0091 0000 0000 003B 00CD 004B"            /* .�...�.....;.�.K */
	$"00DD 0000 0000 0000 008B 00CD 009B 00DD"            /* .�.......�.�.�.� */
	$"0000 0000 0000 0014 0011 0058 00ED C002"            /* ...........X.��. */
	$"0082 0000 0000 0066 0011 00AA 00ED C002"            /* .�.....f...�.��. */
	$"0082 0000 0000 000C 001C 001C 007C 8812"            /* .�...........|�. */
	$"4772 6964 2070 6572 2031 2070 6978 656C"            /* Grid per 1 pixel */
	$"203A 0000 0000 005E 001C 006E 0070 8810"            /*  :.....^...n.p�. */
	$"4772 6964 2066 6F72 2069 636F 6E73 203A"            /* Grid for icons : */
};

data 'CNTL' (128, "grid", purgeable) {
	$"00DF 00A2 00F7 0160 0000 0100 0046 0086"            /* .�.�.�.`.....F.� */
	$"03F0 0000 0000 0647 7269 6420 3A"                   /* .�.....Grid : */
};

data 'CNTL' (129, "tab", purgeable) {
	$"0000 0000 0010 0165 0001 0100 0006 0108"            /* .......e........ */
	$"4552 0000 0000 2146 696C 650D 5061 696E"            /* ER....!File.Pain */
	$"740D 546F 6F6C 0D50 4E47 0D54 6162 6C65"            /* t.Tool.PNG.Table */
	$"740D 4F74 6865 7273"                                /* t.Others */
};

data 'CNTL' (130, "zoom", purgeable) {
	$"005D 003A 0071 0090 0000 0100 0000 009F"            /* .].:.q.�.......� */
	$"03F0 0000 0000 045A 6F6F 6D"                        /* .�.....Zoom */
};

data 'CNTL' (131, "unit", purgeable) {
	$"0000 0000 0014 004B 0000 0100 0000 0091"            /* .......K.......� */
	$"03F0 0000 0000 0455 6E69 74"                        /* .�.....Unit */
};

data 'CNTL' (132, "opacity", purgeable) {
	$"005D 003A 0071 009B 0000 0100 0000 0096"            /* .].:.q.�.......� */
	$"03F0 0000 0000 074F 7061 6369 7479"                 /* .�.....Opacity */
};

data 'CNTL' (133, "file type", purgeable) {
	$"005D 003A 0071 00DC 0000 0100 0000 0099"            /* .].:.q.�.......� */
	$"03F0 0000 0000 0A46 696C 6520 5479 7065"            /* .�....�File Type */
	$"3A"                                                 /* : */
};

data 'CNTL' (134, "dot speed", purgeable) {
	$"0073 0013 0081 0063 0032 0100 0004 0000"            /* .s...�.c.2...... */
	$"4566 0000 0000 00"                                  /* Ef..... */
};

data 'CNTL' (135, "font", purgeable) {
	$"0000 0000 0014 00B4 0000 0100 0000 009E"            /* .......�.......� */
	$"03F4 464F 4E54 0646 6F6E 7420 3A"                   /* .�FONT.Font : */
};

data 'CNTL' (136, "figure type", purgeable) {
	$"0000 0000 0014 0061 0000 0100 0000 00A1"            /* .......a.......� */
	$"03F0 0000 0000 0654 7970 6520 3A"                   /* .�.....Type : */
};

data 'CNTL' (137, "transparent color for gif/png", purgeable) {
	$"0000 0000 0014 0096 0000 0100 0000 00A6"            /* .......�.......� */
	$"03F0 0000 0000 1854 7261 6E73 7061 7265"            /* .�.....Transpare */
	$"6E74 2062 6163 6B67 726F 756E 6420 3A"              /* nt background : */
};

data 'CNTL' (138, "startup", purgeable) {
	$"0000 0000 0014 00DC 0000 0100 0000 00A7"            /* .......�.......� */
	$"03F0 0000 0000 0953 7461 7274 7570 203A"            /* .�....�Startup : */
};

data 'CNTL' (139, "background", purgeable) {
	$"0000 0000 0014 0061 0000 0100 0000 00A8"            /* .......a.......� */
	$"03F0 0000 0000 0C42 6163 6B67 726F 756E"            /* .�.....Backgroun */
	$"6420 3A"                                            /* d : */
};

data 'CNTL' (140, "PNG Compression Level Popup", purgeable) {
	$"0000 0000 0014 00C8 0000 0100 0000 00AD"            /* .......�.......� */
	$"03F0 0000 0000 1143 6F6D 7072 6573 7369"            /* .�.....Compressi */
	$"6F6E 204C 6576 656C"                                /* on Level */
};

data 'CNTL' (141, "file type (icon)", purgeable) {
	$"005D 003A 0071 00F0 0000 0100 0000 00AE"            /* .].:.q.�.......� */
	$"03F0 0000 0000 0A46 696C 6520 7479 7065"            /* .�....�File type */
	$"3A"                                                 /* : */
};

data 'CNTL' (142, "Grid Type", purgeable) {
	$"005D 003A 0071 00E4 0000 0100 002D 00AF"            /* .].:.q.�.....-.� */
	$"03F0 0000 0000 0554 7970 653A"                      /* .�.....Type: */
};

data 'CNTL' (143, "Grid Color", purgeable) {
	$"005D 003A 0071 00E4 0000 0100 002D 00B1"            /* .].:.q.�.....-.� */
	$"03F0 0000 0000 0643 6F6C 6F72 3A"                   /* .�.....Color: */
};

data 'CNTL' (144, "Grid Type (icon)", purgeable) {
	$"005D 003A 0071 00E4 0000 0100 002D 00B0"            /* .].:.q.�.....-.� */
	$"03F0 0000 0000 0653 7061 6365 3A"                   /* .�.....Space: */
};

data 'CNTL' (145, "Grid Color (icon)", purgeable) {
	$"005D 003A 0071 00E4 0000 0100 002D 00B2"            /* .].:.q.�.....-.� */
	$"03F0 0000 0000 0643 6F6C 6F72 3A"                   /* .�.....Color: */
};

data 'CNTL' (146, "Color Depth", purgeable) {
	$"005D 003A 0071 0090 0000 0100 0000 00B3"            /* .].:.q.�.......� */
	$"03F0 0000 0000 0B43 6F6C 6F72 2044 6570"            /* .�.....Color Dep */
	$"7468"                                               /* th */
};

data 'DLOG' (128, "about", purgeable) {
	$"0051 0036 00D8 0115 0005 0000 0000 0000"            /* .Q.6.�.......... */
	$"0000 0080 0F41 626F 7574 2049 636F 6E50"            /* ...�.About IconP */
	$"6172 7479 300A"                                     /* arty0� */
};

data 'DLOG' (129, "confirm save", purgeable) {
	$"0050 004B 00B5 01A6 0005 0000 0000 0000"            /* .P.K.�.�........ */
	$"0000 0081 00"                                       /* ...�. */
};

data 'DLOG' (134, "save dialog", purgeable) {
	$"0000 0000 00DA 0159 0005 0000 0000 0000"            /* .....�.Y........ */
	$"0000 0086 1053 6176 6520 3A20 4963 6F6E"            /* ...�.Save : Icon */
	$"5061 7274 7900 0000"                                /* Party... */
};

data 'DLOG' (130, "pointillize", purgeable) {
	$"0078 0050 010D 014F 0005 0000 0000 0000"            /* .x.P...O........ */
	$"0000 0082 0B50 6F69 6E74 696C 6C69 7A65"            /* ...�.Pointillize */
};

data 'DLOG' (138, "rotate", purgeable) {
	$"0078 0050 00DD 0135 0005 0000 0000 0000"            /* .x.P.�.5........ */
	$"0000 008A 0652 6F74 6174 65"                        /* ...�.Rotate */
};

data 'DLOG' (139, "wind", purgeable) {
	$"0078 0050 00E8 0124 0005 0000 0000 0000"            /* .x.P.�.$........ */
	$"0000 008B 0457 696E 64"                             /* ...�.Wind */
};

data 'DLOG' (144, "resize", purgeable) {
	$"0078 0050 0180 0124 0005 0000 0000 0000"            /* .x.P.�.$........ */
	$"0000 0090 0A49 6D61 6765 2053 697A 65"              /* ...��Image Size */
};

data 'DLOG' (300, "preferences", purgeable) {
	$"0029 001A 0162 017F 0005 0000 0000 0000"            /* .)...b.......... */
	$"0000 012C 0B50 7265 6665 7265 6E63 6573"            /* ...,.Preferences */
	$"280A"                                               /* (� */
};

data 'DLOG' (135, "image size(new)", purgeable) {
	$"0078 0050 012C 017A 0005 0000 0000 0000"            /* .x.P.,.z........ */
	$"0000 0087 034E 6577"                                /* ...�.New */
};

data 'DLOG' (131, "icon info", purgeable) {
	$"0078 0050 00E2 0145 0005 0000 0000 0000"            /* .x.P.�.E........ */
	$"0000 0083 0949 636F 6E20 496E 666F"                 /* ...��Icon Info */
};

data 'DLOG' (132, "figure", purgeable) {
	$"0078 0050 0100 0122 0005 0000 0000 0000"            /* .x.P..."........ */
	$"0000 0084 0C49 6E70 7574 2046 6967 7572"            /* ...�.Input Figur */
	$"65"                                                 /* e */
};

data 'DLOG' (133, "text", purgeable) {
	$"0078 0050 01DF 01FE 0005 0000 0000 0000"            /* .x.P.�.�........ */
	$"0000 0085 0A49 6E70 7574 2054 6578 74"              /* ...��Input Text */
};

data 'DLOG' (400, "open", purgeable) {
	$"0000 0000 00C1 0159 0005 0000 0000 0000"            /* .....�.Y........ */
	$"0000 0190 0F8A 4A82 AD81 4649 636F 6E50"            /* ...�.�J���FIconP */
	$"6172 7479"                                          /* arty */
};

data 'DLOG' (401, "save", purgeable) {
	$"0000 0000 00C1 0159 0005 0000 0000 0000"            /* .....�.Y........ */
	$"0000 0191 0F95 DB91 B681 4649 636F 6E50"            /* ...�.�ۑ��FIconP */
	$"6172 7479"                                          /* arty */
};

data 'DLOG' (136, "destroy icons", purgeable) {
	$"0050 004B 00B5 01A6 0005 0000 0000 0000"            /* .P.K.�.�........ */
	$"0000 0088 00"                                       /* ...�. */
};

data 'DLOG' (137, "confirm revert", purgeable) {
	$"0050 004B 00B5 01A6 0005 0000 0000 0000"            /* .P.K.�.�........ */
	$"0000 0089 00"                                       /* ...�. */
};

data 'DLOG' (140, "edit custom icon?", purgeable) {
	$"0050 004B 00B5 01A6 0005 0000 0000 0000"            /* .P.K.�.�........ */
	$"0000 008C 00"                                       /* ...�. */
};

data 'DLOG' (141, "add icons", purgeable) {
	$"0050 004B 00B5 01A6 0005 0000 0000 0000"            /* .P.K.�.�........ */
	$"0000 008D 00"                                       /* ...�. */
};

data 'DLOG' (142, "new library", purgeable) {
	$"0078 0050 00DD 0135 0005 0000 0000 0000"            /* .x.P.�.5........ */
	$"0000 008E 0F4E 6577 2044 6F74 204C 6962"            /* ...�.New Dot Lib */
	$"7261 7279"                                          /* rary */
};

data 'DLOG' (143, "new icon", purgeable) {
	$"0078 0050 013D 0182 0005 0000 0000 0000"            /* .x.P.=.�........ */
	$"0000 008F 0949 636F 6E20 496E 666F"                 /* ...��Icon Info */
};

data 'DLOG' (145, "edit dot library", purgeable) {
	$"0078 0050 013E 0158 0005 0000 0100 0000"            /* .x.P.>.X........ */
	$"0000 0091 1045 6469 7420 446F 7420 4C69"            /* ...�.Edit Dot Li */
	$"6272 6172 79"                                       /* brary */
};

data 'DLOG' (146, "edit dot command", purgeable) {
	$"0078 0050 00F1 0151 0005 0000 0000 0000"            /* .x.P.�.Q........ */
	$"0000 0092 1045 6469 7420 446F 7420 436F"            /* ...�.Edit Dot Co */
	$"6D6D 616E 64"                                       /* mmand */
};

data 'DLOG' (147, "delete dot command", purgeable) {
	$"0050 004B 00B5 01A6 0005 0000 0000 0000"            /* .P.K.�.�........ */
	$"0000 0093 0083 300A"                                /* ...�.�0� */
};

data 'DLOG' (148, "save dialog(export icon)", purgeable) {
	$"0000 0000 00DE 0159 0005 0000 0000 0000"            /* .....�.Y........ */
	$"0000 0094 1745 7870 6F72 7420 4963 6F6E"            /* ...�.Export Icon */
	$"203A 2049 636F 6E50 6172 7479 0000"                 /*  : IconParty.. */
};

data 'DLOG' (149, "grid", purgeable) {
	$"0078 0050 014E 014D 0005 0000 0000 0000"            /* .x.P.N.M........ */
	$"0000 0095 0D47 7269 6420 7365 7474 696E"            /* ...�.Grid settin */
	$"6773"                                               /* gs */
};

data 'WIND' (129, "color palette 1", purgeable) {
	$"0016 0037 0055 01AC 0809 0000 0100 0000"            /* ...7.U.�.�...... */
	$"0003 00"                                            /* ... */
};

data 'WIND' (130, "color palette 2", purgeable) {
	$"0076 01C4 0119 01F5 0801 0000 0100 0000"            /* .v.�...�........ */
	$"0005 00"                                            /* ... */
};

data 'WIND' (131, "preview window", purgeable) {
	$"0024 01C4 0064 0224 0801 0000 0000 0000"            /* .$.�.d.$........ */
	$"0002 0750 7265 7669 6577"                           /* ...Preview */
};

data 'WIND' (132, "tool palette", purgeable) {
	$"0075 0002 00F5 0025 0801 0000 0100 0000"            /* .u...�.%........ */
	$"0003 00"                                            /* ... */
};

data 'WIND' (133, "blend palette", purgeable) {
	$"0076 01F9 00DE 021C 0801 0000 0100 0000"            /* .v.�.�.......... */
	$"0006 00"                                            /* ... */
};

data 'WIND' (134, "title", purgeable) {
	$"0024 0002 0063 0025 0801 0000 0100 0000"            /* .$...c.%........ */
	$"0007 00"                                            /* ... */
};

data 'WIND' (135, "info palette", purgeable) {
	$"0107 0002 0128 0025 0801 0000 0100 0000"            /* .....(.%........ */
	$"0008 00"                                            /* ... */
};

data 'WIND' (136, "icon list window", purgeable) {
	$"0078 003C 0138 012C 0000 0000 0100 0000"            /* .x.<.8.,........ */
	$"0009 0A4E 6577 2057 696E 646F 77"                   /* .��New Window */
};

data 'WIND' (128, "paint window", purgeable) {
	$"006E 002D 017E 01BD 0008 0000 0100 0000"            /* .n.-.~.�........ */
	$"0001 00"                                            /* ... */
};

data 'WIND' (137, "icon family window", purgeable) {
	$"0131 00C2 01B7 0185 000C 0000 0100 0000"            /* .1.�.�.�........ */
	$"000A 0449 636F 6E"                                  /* .�.Icon */
};

data 'WIND' (138, "pattern palette", purgeable) {
	$"0129 01C4 016C 0227 0801 0000 0100 0000"            /* .).�.l.'........ */
	$"0009 0750 6174 7465 726E"                           /* .�.Pattern */
};

data 'WIND' (139, "dot mode palette", purgeable) {
	$"0150 014D 0193 01B2 0801 0000 0100 0000"            /* .P.M.�.�........ */
	$"000B 0844 6F74 204D 6F64 65"                        /* ...Dot Mode */
};

data 'WIND' (140, "color palette 3", purgeable) {
	$"0150 01B3 018F 0256 0801 0000 0100 0000"            /* .P.�.�.V........ */
	$"0005 0F46 6176 6F72 6974 6520 436F 6C6F"            /* ...Favorite Colo */
	$"7273"                                               /* rs */
};


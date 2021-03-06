/* ------------------------------------------------------------ */
/*  PNGSupport.h                                                */
/*      PNGのエンコードルーチンのヘッダ                         */
/*                                                              */
/*                 2001.1.27 - 2001.1.27  naoki iimura        	*/
/* ------------------------------------------------------------ */

/* prototypes */
void	PNGEncode(const FSSpec *theFile,GWorldPtr theImage,RgnHandle saveRgn,
					short colorNum,short depth,Boolean interlaced,short transColorIndex,
					const RGBColor *transColor,short compLevel,OSType creator);
OSErr	PNGDecode(const FSSpec *theFile,GWorldPtr *theImage,GWorldPtr *maskImage,
					Boolean useTransColor);


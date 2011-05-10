/* ------------------------------------------------------------ */
/*  GifEncode-miGIF.c                                           */
/*     GIF保存ルーチン（GD 1.3による）                          */
/*                                                              */
/*                 1999.1.7 - 2000.6.3  naoki iimura            */
/* ------------------------------------------------------------ */


/* includes */
#include	<qdoffscreen.h>
#include	<Script.h>
#include	<TextUtils.h>
#include	<Resources.h>

#include	"Globals.h"
#include	"UsefulRoutines.h"
#include	"PreCarbonSupport.h"

/* definitions */
#define		sGifVersionString	135

#define		EOF				 -1

/* prototypes */
extern void	GifEncode(FSSpec *gifFileSpec,GWorldPtr theImage,RgnHandle saveRgn,
					Boolean interlaced,short transColorIndex);

static void	PutStr(Str255 str);
static void	PutShort(short s);
static void	PutChar(char c);

static void	Compress(int initCode,PixMapHandle pmh);
static void	MakePixelIndex(PixMapHandle portPix,RgnHandle selectedRgn,Boolean interlaced);
static void	BumpPixel(void);
static short	GetNextPixel(PixMapHandle pmh);

/* structures */
typedef struct {
	char	version[6];
	char	width[2];
	char	height[2];
	char	packed;
	char	bgIndex;
	char	aspectRatio;
} GIFHeader;

typedef struct {
	char	introducer;
	char	controlLabel;
	char	blockSize;
	char	packed;
	char	delayTime[2];
	char	transColorInd;
	char	terminator;
} ControlExtension;

typedef struct {
	char	imageSeparator;
	char	leftPosition[2];
	char	topPosition[2];
	char	imageWidth[2];
	char	imageHeight[2];
	char	packed;
} ImageData;

/* variables */
static Handle	gifData;
static Ptr		pixelIndex;
static short	gifFileRef;

static short	Left,Right,Top,Bottom;
static short	curx,cury;
static long		CountDown;
static short	Pass=0;
static short	Interlace;

/* public */

extern void GifEncode(FSSpec *gifFileSpec,GWorldPtr theImage,RgnHandle saveRgn,
						Boolean interlaced,short transColorIndex)
{
	OSErr	err;
	Str15	str;
	char	initCode;
	Rect	saveRect;
	short	width;
	short	height;
	Handle	pixels;
	long	count;
	short	i;
	CTabHandle	ctable;
	Ptr		color_palette,temp;
	
	FInfo			fndrInfo;
	long			filePos;
	PixMapHandle	savePix;
	
	savePix=GetGWorldPixMap(theImage);
	
	GetRegionBounds(saveRgn,&saveRect);
	width=saveRect.right-saveRect.left;
	height=saveRect.bottom-saveRect.top;
	
	/* ピクセルを得るためのデータ */
	Left=saveRect.left;
	Right=saveRect.right;
	Top=saveRect.top;
	Bottom=saveRect.bottom;
	CountDown=(long)width*(long)height;
	Pass=0;
	curx=saveRect.left;
	cury=saveRect.top;
	Interlace=interlaced;
	
	pixels=TempNewHandle(CountDown,&err);
	TempHLock(pixels,&err);
	pixelIndex=*pixels;
	
	MakePixelIndex(savePix,saveRgn,interlaced);
	
	/* ファイルがなければ作成し、開く */
	err=FSpGetFInfo(gifFileSpec,&fndrInfo);
	if (err==fnfErr)
		err=FSpCreate(gifFileSpec,gGIFCreator,kGIFFileType,smSystemScript);
	err=FSpOpenDF(gifFileSpec,fsWrPerm,&gifFileRef);
	
	/* header */
	/* signature&version */
	GetIndString(str,sGifVersionString,(interlaced ? 1 : 2));
	count=str[0];
	FSWrite(gifFileRef,&count,&str[1]);
	
	/* logical screen descriptor */
	/* screen width */
	PutShort(width);
	/* screen height */
	PutShort(height);
	/* <packed field> */
	PutChar(0xf7);
	/* background color index */
	PutChar(0x00);
	/* pixel aspect ratio */
	PutChar(0x00);
	
	/* global color table */
	ctable=(*savePix)->pmTable;
	count=3*256;
	color_palette = NewPtr(count);
	temp=color_palette;
	for (i=0; i<256; i++)
	{
		*temp++=((*ctable)->ctTable[i].rgb.red)>>8;
		*temp++=((*ctable)->ctTable[i].rgb.green)>>8;
		*temp++=((*ctable)->ctTable[i].rgb.blue)>>8;
	}
	err=FSWrite(gifFileRef,&count,color_palette);
	DisposePtr(color_palette);
	
	/* if transparent... */
	if ( transColorIndex >= 0 ) {
		/* graphic control extension */
		/* extention introducer */
	    PutChar('!');
	    /* graphic control label */
	    PutChar(0xf9);
	    /* block size */
	    PutChar(4);
	    /* <packed field> */
	    PutChar(0x01); /* transparent */
	    /* delay time */
	    PutShort(0);
	    /* transparent color table */
	    PutChar(transColorIndex & 0xff);
	    /* block terminator */
	    PutChar(0);
	}
	
	/* image data */
	/* image separator */
	PutChar(',');
	/* left position */
	PutShort(0);
	/* top position */
	PutShort(0);
	/* image width */
	PutShort(width);
	/* image height */
	PutShort(height);
	/* <packed field> */
	if (interlaced)
		PutChar(0x40);
	else
		PutChar(0x00); /*  0x40, if interlaced */
	
	/* init code size */
	initCode=8;
	PutChar(initCode);
	
	/* compress image data */
	Compress(initCode+1,savePix);
	
	/* end code */
	PutChar(0);
	
	/* gif trailer */
	PutChar(';');
	
	/* ファイルのサイズを設定 */
	err=GetFPos(gifFileRef,&filePos);
	err=SetEOF(gifFileRef,filePos);
	
	/* ファイルを閉じる */
	err=FSClose(gifFileRef);
	
	TempHUnlock(pixels,&err);
	TempDisposeHandle(pixels,&err);
}

void PutStr(Str255 str)
{
	long	count=str[0];
	FSWrite(gifFileRef,&count,&str[1]);
}

void PutShort(short s)
{
	PutChar(s&0xff);
	PutChar((s/256)&0xff);
}

void PutChar(char c)
{
	long count=1;
	FSWrite(gifFileRef,&count,&c);
}

void MakePixelIndex(PixMapHandle portPix,RgnHandle selectedRgn,Boolean interlaced)
{
	short	y;
	long	width;
	short	left,top,right,bottom;
	Ptr		temp,base;
	long	rowBytes;
	Rect	selectedRect;
	
	GetRegionBounds(selectedRgn,&selectedRect);
	left=selectedRect.left;
	top=selectedRect.top;
	right=selectedRect.right;
	bottom=selectedRect.bottom;
	width=right-left;
	
	rowBytes=MyGetPixRowBytes(portPix)& 0x3fff;
	
	temp=pixelIndex;
	
	LockPixels(portPix);
	base=MyGetPixBaseAddr(portPix);
	
	if (!interlaced)
		for (y=top; y<bottom; y++)
		{
			BlockMoveData(base+y*rowBytes+left,temp,width);
			temp+=width;
		}
	else /* interlaced */
	{
		for (y=top; y<bottom; y+=8)
		{
			BlockMoveData(base+y*rowBytes+left,temp,width);
			temp+=width;
		}
		for (y=top+4; y<bottom; y+=8)
		{
			BlockMoveData(base+y*rowBytes+left,temp,width);
			temp+=width;
		}
		for (y=top+2; y<bottom; y+=4)
		{
			BlockMoveData(base+y*rowBytes+left,temp,width);
			temp+=width;
		}
		for (y=top+1; y<bottom; y+=2)
		{
			BlockMoveData(base+y*rowBytes+left,temp,width);
			temp+=width;
		}
	}
	
	UnlockPixels(portPix);
}

static void BumpPixel(void)
{
	/* 右へ */
	++curx;
	
	/* 右端なら、左端に戻り、インタレースなら適切な、そうでなければ次の行へ移る */
	if (curx == Right)
	{
		curx=Left;
		
		if (!Interlace)
			++cury;
		else
		{
			switch (Pass)
			{
				case 0:
					cury += 8;
					if (cury >= Bottom)
					{
						++Pass;
						cury=Top+4;
					}
					break;
				
				case 1:
					cury += 8;
					if (cury >= Bottom)
					{
						++Pass;
						cury=Top+2;
					}
					break;
				
				case 2:
					cury +=4;
					if (cury >= Bottom)
					{
						++Pass;
						cury=Top+1;
					}
					break;
				
				case 3:
					cury +=2;
					break;
			}
		}
	}
}

static short GetNextPixel(PixMapHandle pmh)
{
	#pragma unused(pmh)
	
	if (CountDown == 0) return EOF;
	
	--CountDown;
	
	return (unsigned char)*pixelIndex++;
}


#define GIFBITS 12

/*-----------------------------------------------------------------------
 *
 * miGIF Compression - mouse and ivo's GIF-compatible compression
 *
 *          -run length encoding compression routines-
 *
 * Copyright (C) 1998 Hutchison Avenue Software Corporation
 *               http://www.hasc.com
 *               info@hasc.com
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  This software is provided "AS IS." The Hutchison Avenue 
 * Software Corporation disclaims all warranties, either express or implied, 
 * including but not limited to implied warranties of merchantability and 
 * fitness for a particular purpose, with respect to this code and accompanying
 * documentation. 
 * 
 * The miGIF compression routines do not, strictly speaking, generate files 
 * conforming to the GIF spec, since the image data is not LZW-compressed 
 * (this is the point: in order to avoid transgression of the Unisys patent 
 * on the LZW algorithm.)  However, miGIF generates data streams that any 
 * reasonably sane LZW decompresser will decompress to what we want.
 *
 * miGIF compression uses run length encoding. It compresses horizontal runs 
 * of pixels of the same color. This type of compression gives good results
 * on images with many runs, for example images with lines, text and solid 
 * shapes on a solid-colored background. It gives little or no compression 
 * on images with few runs, for example digital or scanned photos.
 *
 *                               der Mouse
 *                      mouse@rodents.montreal.qc.ca
 *            7D C8 61 52 5D E7 2D 39  4E F1 31 3E E8 B3 27 4B
 *
 *                             ivo@hasc.com
 *
 * The Graphics Interchange Format(c) is the Copyright property of
 * CompuServe Incorporated.  GIF(sm) is a Service Mark property of
 * CompuServe Incorporated.
 *
 */

static int rl_pixel;
static int rl_basecode;
static int rl_count;
static int rl_table_pixel;
static int rl_table_max;
static int just_cleared;
static int out_bits;
static int out_bits_init;
static int out_count;
static int out_bump;
static int out_bump_init;
static int out_clear;
static int out_clear_init;
static int max_ocodes;
static int code_clear;
static int code_eof;
static unsigned int obuf;
static int obits;
static unsigned char oblock[256];	/* 書き出し用ブロック */
static int oblen;					/* 書き出し用データ長 */


static const char *binformat(unsigned int v, int nbits)
{
	static char bufs[8][64];
	static int bhand = 0;
	unsigned int bit;
	int bno;
	char *bp;

	bhand --;
	if (bhand < 0) bhand = (sizeof(bufs)/sizeof(bufs[0]))-1;
	bp = &bufs[bhand][0];
	for (bno=nbits-1,bit=1U<<bno;bno>=0;bno--,bit>>=1)
	{
		*bp++ = (v & bit) ? '1' : '0';
		if (((bno&3) == 0) && (bno != 0)) *bp++ = '.';
	}
	*bp = '\0';
	return(&bufs[bhand][0]);
}

/* ファイル書き出し */
static void write_block(void)
{
	long	l;
	char	c;

	c=oblen;

	l=1;
	FSWrite(gifFileRef,&l,&c);
	l=(UInt16)oblen;
	FSWrite(gifFileRef,&l,&oblock[0]);
	oblen = 0;
}

/* １バイト追加 */
static void block_out(unsigned char c)
{
	oblock[oblen++] = c;
	if (oblen >= 255) write_block();
}

/* ブロック内容書き出し */
static void block_flush(void)
{
	if (oblen > 0) write_block();
}

static void output(int val)
{
	obuf |= val << obits;
	obits += out_bits;
	while (obits >= 8)
	{
		block_out(obuf&0xff);
		obuf >>= 8;
		obits -= 8;
	}
}

static void output_flush(void)
{
	if (obits > 0) block_out(obuf);
	block_flush();
}

static void did_clear(void)
{
	out_bits = out_bits_init;
	out_bump = out_bump_init;
	out_clear = out_clear_init;
	out_count = 0;
	rl_table_max = 0;
	just_cleared = 1;
}

static void output_plain(int c)
{
	just_cleared = 0;
	output(c);
	out_count ++;
	if (out_count >= out_bump)
	{
		out_bits ++;
		out_bump += 1 << (out_bits - 1);
	}
	if (out_count >= out_clear)
	{
		output(code_clear);
		did_clear();
	}
}

static unsigned int isqrt(unsigned int);/*__attribute__((__const__))*/
static unsigned int isqrt(unsigned int x)
{
	unsigned int r;
	unsigned int v;

	if (x < 2) return(x);
	for (v=x,r=1;v;v>>=2,r<<=1) ;
	while (1)
	{
		v = ((x / r) + r) / 2;
		if ((v == r) || (v == r+1)) return(r);
		r = v;
	}
}

static unsigned int compute_triangle_count(unsigned int count, unsigned int nrepcodes)
{
	unsigned int perrep;
	unsigned int cost;

	cost = 0;
	perrep = (nrepcodes * (nrepcodes+1)) / 2;
	while (count >= perrep)
	{
		cost += nrepcodes;
		count -= perrep;
	}
	if (count > 0)
	{
		unsigned int n;
		n = isqrt(count);
		while ((n*(n+1)) >= 2*count) n --;
		while ((n*(n+1)) < 2*count) n ++;
		cost += n;
	}
	return(cost);
}

static void max_out_clear(void)
{
	out_clear = max_ocodes;
}

static void reset_out_clear(void)
{
	out_clear = out_clear_init;
	if (out_count >= out_clear)
	{
		output(code_clear);
		did_clear();
	}
}

static void rl_flush_fromclear(int count)
{
	int n;

	max_out_clear();
	rl_table_pixel = rl_pixel;
	n = 1;
	while (count > 0)
	{
		if (n == 1)
		{
			rl_table_max = 1;
			output_plain(rl_pixel);
			count --;
		}
		else if (count >= n)
		{
			rl_table_max = n;
			output_plain(rl_basecode+n-2);
			count -= n;
		}
		else if (count == 1)
		{
			rl_table_max ++;
			output_plain(rl_pixel);
			count = 0;
		}
		else
		{
			rl_table_max ++;
			output_plain(rl_basecode+count-2);
			count = 0;
		}
		if (out_count == 0) n = 1; else n ++;
	}
	reset_out_clear();
}

static void rl_flush_clearorrep(int count)
{
	int withclr;

	withclr = 1 + compute_triangle_count(count,max_ocodes);
	if (withclr < count)
	{
		output(code_clear);
		did_clear();
		rl_flush_fromclear(count);
	}
	else
	{
	for (;count>0;count--) output_plain(rl_pixel);
	}
}

static void rl_flush_withtable(int count)
{
	int repmax;
	int repleft;
	int leftover;

	repmax = count / rl_table_max;
	leftover = count % rl_table_max;
	repleft = (leftover ? 1 : 0);
	if (out_count+repmax+repleft > max_ocodes)
	{
		repmax = max_ocodes - out_count;
		leftover = count - (repmax * rl_table_max);
		repleft = 1 + compute_triangle_count(leftover,max_ocodes);
	}
	if (1+compute_triangle_count(count,max_ocodes) < repmax+repleft)
	{
		output(code_clear);
		did_clear();
		rl_flush_fromclear(count);
		return;
	}
	max_out_clear();
	for (;repmax>0;repmax--) output_plain(rl_basecode+rl_table_max-2);
	if (leftover)
	{
		if (just_cleared)
			rl_flush_fromclear(leftover);
		else if (leftover == 1)
			output_plain(rl_pixel);
		else
			output_plain(rl_basecode+leftover-2);
	}
	reset_out_clear();
}

static void rl_flush(void)
{
	int table_reps;
	int table_extra;

	#pragma unused(table_extra,table_reps)

	if (rl_count == 1)
	{
		output_plain(rl_pixel);
		rl_count = 0;
		return;
	}
	if (just_cleared)
		rl_flush_fromclear(rl_count);
	else if ((rl_table_max < 2) || (rl_table_pixel != rl_pixel))
		rl_flush_clearorrep(rl_count);
	else
		rl_flush_withtable(rl_count);
	rl_count = 0;
}

static void Compress(int init_bits,PixMapHandle pmh)
{
	int c;

	obuf = 0;
	obits = 0;
	oblen = 0;
	code_clear = 1 << (init_bits - 1);
	code_eof = code_clear + 1;
	rl_basecode = code_eof + 1;
	out_bump_init = (1 << (init_bits - 1)) - 1;
	/* for images with a lot of runs, making out_clear_init larger will
	give better compression. */ 
	out_clear_init = (init_bits <= 3) ? 9 : (out_bump_init-1);
	out_bits_init = init_bits;
	max_ocodes = (1 << GIFBITS) - ((1 << (out_bits_init - 1)) + 3);
	did_clear();
	output(code_clear);
	rl_count = 0;
	while (1)
	{
		c = GetNextPixel(pmh);
		if ((rl_count > 0) && (c != rl_pixel)) rl_flush();
		if (c == EOF) break;
		if (rl_pixel == c)
		{
			rl_count ++;
		}
		else
		{
			rl_pixel = c;
			rl_count = 1;
		}
	}
	output(code_eof);
	output_flush();
}

/*-----------------------------------------------------------------------
 *
 * End of miGIF section  - See copyright notice at start of section.
 *
/*-----------------------------------------------------------------------*/
/* ------------------------------------------------------------ */
/*  PNGSupport.c                                                */
/*      PNGのエンコードルーチン                                 */
/*                                                              */
/*                 1999.11.9 - 2011.1.27  naoki iimura        	*/
/* ------------------------------------------------------------ */

#define		PNG_NO_STDIO
#define		MACOS

#ifdef __APPLE_CC__
#include	<Carbon/Carbon.h>
#else
#include	<MacTypes.h>
#include	<QDOffscreen.h>
#include	<Script.h>
#include	<TextUtils.h>
#include	<Resources.h>
#include	<Movies.h>
#endif

#include	<png.h>

#include	"Definition.h"
#include	"PNGSupport.h"
#include	"UsefulRoutines.h"
#include	"PreCarbonSupport.h"

/* prototypes */
static void	png_default_read_data(png_structp png_ptr,png_bytep data,png_size_t length);
static void	png_default_write_data(png_structp png_ptr,png_bytep data,png_size_t length);
static void	png_default_warning(png_structp png_ptr,png_const_charp message);
static void	png_default_error(png_structp png_ptr,png_const_charp message);

static pascal Boolean	MySearchTransProc(RGBColor *color,long *pos);

/* structures */
typedef struct MyPNGErrorRec {
	OSErr		err;
	GWorldPtr	cPort;
	GDHandle	cDevice;
} MyPNGErrorRec;

typedef struct MyPNGFileRec {
	short	refNum;
	short	vRefNum;
} MyPNGFileRec;

/* globals */
static RGBColor	gTransColor;


#define	PNGERR_RESID	4005
#define	PNGERR1	1
#define	PNGERR2	2
#define	PNGERR3	3
#define	PNGERR4	4
#define	PNGERR5	5
#define	PNGERR6	6
#define	PNGERR7	7
#define	PNGERR8	8


/* PNG保存 */
extern void PNGEncode(const FSSpec *theFile,GWorldPtr theImage,RgnHandle saveRgn,
					short colorNum,short depth,Boolean interlaced,short transColorIndex,
					const RGBColor *transColor,short compLevel,OSType creator)
{
	PixMapHandle	savePix;
	
	MyPNGErrorRec	myError;
	MyPNGFileRec	myFile;
	OSErr			err;
	png_structp		write_ptr;
	png_infop		write_info_ptr;
	
	Rect			saveRect;
	int				filters;
	png_uint_32		width,height;
	int				bit_depth,color_type;
	int				interlace_type;
	int				compression_type,filter_type;
	double			gamma;
	png_colorp		color_palette=NULL;
	CTabHandle		ctable;
	int				num_palette,i;
	png_bytep		trans=NULL;
	png_color_16p	trans16=NULL;
	
	int				y;
	Ptr				baseAddr;
	long			rowBytes;
	png_bytep		*row_bufp;
	
	FInfo			fndrInfo;
	long			filePos;
	
	GWorldPtr		cPort;
	GDHandle		cDevice;
	
	GetGWorld(&cPort,&cDevice);
	
	/* ファイルがなければ作成し、開く */
	err=FSpGetFInfo(theFile,&fndrInfo);
	if (err==fnfErr)
		err=FSpCreate(theFile,creator,kPNGFileType,smSystemScript);
	err=FSpOpenDF(theFile,fsWrPerm,&myFile.refNum);
	if (err!=noErr) return;
	
	/* PNG保存用の構造体を作る */
	write_ptr=png_create_write_struct(PNG_LIBPNG_VER_STRING,(png_voidp)NULL,
				png_default_error,png_default_warning);
	if (write_ptr == NULL)
	{
		FSClose(myFile.refNum);
		FSpDelete(theFile);
		return;
	}
	
	/* エラーハンドリングルーチン */
	myError.err=noErr;
	myError.cPort=cPort;
	myError.cDevice=cDevice;
	png_set_error_fn(write_ptr,(png_voidp)&myError,png_default_error,
				png_default_warning);
	
	write_info_ptr=png_create_info_struct(write_ptr);
	
	if (setjmp(write_ptr->jmpbuf))
	{
		ErrorAlertFromResource(PNGERR_RESID,PNGERR1);
		png_destroy_write_struct(&write_ptr,&write_info_ptr);
		FSClose(myFile.refNum);
		return;
	}
	
	/* 書き出しルーチン設定 */
	png_set_write_fn(write_ptr,(png_voidp)&myFile,png_default_write_data,NULL);
	
	/* プログレスルーチン */
	png_set_write_status_fn(write_ptr,NULL);
	
	/* ファイルの設定 */
	GetRegionBounds(saveRgn,&saveRect);
	width=saveRect.right-saveRect.left;
	height=saveRect.bottom-saveRect.top;
	
	if (depth <= 8)
	{
		bit_depth=depth;
		color_type=PNG_COLOR_TYPE_PALETTE; /* indexed color */
		filters = PNG_FILTER_NONE;
	}
	else
	{
		bit_depth=8; /* rgbごとに8ビット */
		color_type=PNG_COLOR_TYPE_RGB; /* rgb color */
		filters = PNG_ALL_FILTERS;
	}
	interlace_type=(interlaced ? PNG_INTERLACE_ADAM7 : PNG_INTERLACE_NONE);
	compression_type=PNG_COMPRESSION_TYPE_BASE;
	filter_type=PNG_FILTER_TYPE_BASE;
	
	/* フィルタ */
	png_set_filter(write_ptr,0,filters);
	
	/* 圧縮レベル */
	png_set_compression_level(write_ptr,compLevel);
	
	/* 大きさなど */
	png_set_IHDR(write_ptr,write_info_ptr,width,height,bit_depth,color_type,
				interlace_type,compression_type,filter_type);
	
	/*  */
	SetGWorld(theImage,0);
	savePix=GetGWorldPixMap(theImage);
	LockPixels(savePix);
	
	/* カラーパレット */
	if (color_type == PNG_COLOR_TYPE_PALETTE)
	{
		num_palette=colorNum;
		color_palette=(png_colorp)NewPtr(sizeof(png_color)*num_palette);
		
		ctable=(*savePix)->pmTable;
		HLock((Handle)ctable);
		for (i=0; i<num_palette; i++)
		{
			color_palette[i].red=((*ctable)->ctTable[i].rgb.red)>>8;
			color_palette[i].green=((*ctable)->ctTable[i].rgb.green)>>8;
			color_palette[i].blue=((*ctable)->ctTable[i].rgb.blue)>>8;
		}
		HUnlock((Handle)ctable);
		
		png_set_PLTE(write_ptr,write_info_ptr,color_palette,num_palette);
	}
	
	/* ガンマ */
	gamma=0.55555;
	png_set_gAMA(write_ptr,write_info_ptr,gamma);
	
	/* 透過色 */
	if (transColorIndex>=0)
	{
		switch (color_type)
		{
			case PNG_COLOR_TYPE_RGB:
				/* RGBの場合、2bytes * 3で透過色を指定 */
				trans16=(png_color_16p)NewPtr(sizeof(png_color_16));
				trans16->red=transColor->red;
				trans16->green=transColor->green;
				trans16->blue=transColor->blue;
				
				trans16->gray=0;
				trans16->index=0;
				
				SysBeep(0);
				
				png_set_tRNS(write_ptr,write_info_ptr,NULL,1,trans16);
				break;
			
			case PNG_COLOR_TYPE_PALETTE:
				{
					short	i;
					short	color_num=transColorIndex+1;
					
					trans=(png_bytep)NewPtr(color_num); /* 必要分 */
					for (i=0; i<transColorIndex; i++)
						trans[i]=255U;
					trans[i]=0;
					
					png_set_tRNS(write_ptr,write_info_ptr,trans,color_num,NULL);
				}
				break;
		}
	}
	
	/* 時刻（ファイルの修正日と一致させる） */
	{
		CInfoPBRec	cInfo;
		OSErr	err;
		DateTimeRec	dateTime;
		png_time	pTime;
		
		cInfo.hFileInfo.ioFDirIndex=0;
		/* これを０にすることによって、ioVRefNumで示されるボリュームリファレンス、
		   ioDirIDで示されるディレクトリからファイルを特定する */
		cInfo.hFileInfo.ioVRefNum=theFile->vRefNum;
		cInfo.hFileInfo.ioDirID=theFile->parID;
		cInfo.hFileInfo.ioNamePtr=(StringPtr)theFile->name;
		
		err=PBGetCatInfoSync(&cInfo);
		
		if (err==noErr)
		{
			SecondsToDate(cInfo.hFileInfo.ioFlMdDat,&dateTime);
			pTime.year=dateTime.year;
			pTime.month=dateTime.month;
			pTime.day=dateTime.day;
			pTime.hour=dateTime.hour;
			pTime.minute=dateTime.minute;
			pTime.second=dateTime.second;
			
			png_set_tIME(write_ptr,write_info_ptr,&pTime);
		}
	}
	
	/* テキスト */
	{
		png_text	text;
		
		text.compression=PNG_TEXT_COMPRESSION_NONE;
		text.key="Software";
		text.text="IconParty";
		text.text_length=strlen(text.text);
		png_set_text(write_ptr,write_info_ptr,&text,1);
	}
	
	/* 解像度（72dpi = 2834.6456693dpm (dots per meter)） */
	png_set_pHYs(write_ptr,write_info_ptr,2835,2835,PNG_RESOLUTION_METER);
	
	/* ヘッダを保存 */
	png_write_info(write_ptr,write_info_ptr);
	
	/* 4 bytes per 1 pixel -> 3 bytes per 1 pixel */
	if (color_type == PNG_COLOR_TYPE_RGB)
		png_set_filler(write_ptr,0,PNG_FILLER_BEFORE);

	/* データの位置を作成 */
//	LockPixels(savePix);
	baseAddr=MyGetPixBaseAddr(savePix); //+saveRect.left;
	rowBytes=MyGetPixRowBytes(savePix) & 0x3fff;
	//baseAddr+=saveRect.top * rowBytes;
	row_bufp=(png_bytep *)NewPtr(sizeof(png_bytep)*height);
	
	/* アドレスの設定 */
	for (y = 0; y < height; y++)
	{
		row_bufp[y]=(png_bytep)baseAddr;
//		png_write_row(write_ptr,row_buf);
		
		baseAddr+=rowBytes;
	}
	
	/* 書き込み */
	png_write_image(write_ptr,row_bufp);
	
	/* 保存終了 */
	png_write_end(write_ptr,write_info_ptr);
	
	UnlockPixels(savePix);
	SetGWorld(cPort,cDevice);
	
	/* 途中で確保したメモリを解放 */
	/* カラーテーブル */
	if (color_palette != NULL) DisposePtr((Ptr)color_palette);
	
	/* 透過色 */
	if (trans != NULL) DisposePtr((Ptr)trans);
	if (trans16 != NULL) DisposePtr((Ptr)trans16);
	
	/* row_bufp */
	DisposePtr((Ptr)row_bufp);
	
	/* 構造体自体を削除 */
	png_destroy_write_struct(&write_ptr,&write_info_ptr);
	
	/* ファイルのサイズを設定 */
	err=GetFPos(myFile.refNum,&filePos);
	err=SetEOF(myFile.refNum,filePos);
	
	/* ファイルを閉じる */
	err=FSClose(myFile.refNum);
	FlushVol(0,theFile->vRefNum);
}

#define	PNG_BYTES_TO_CHECK	8

/* PNG読み込み */
extern OSErr PNGDecode(const FSSpec *theFile,GWorldPtr *theImage,GWorldPtr *maskImage,Boolean useTransColor)
{
	MyPNGErrorRec	myError;
	MyPNGFileRec	myFile;
	OSErr			err;
	UInt8			buf[PNG_BYTES_TO_CHECK];
	long			size=PNG_BYTES_TO_CHECK;
	png_structp		read_ptr;
	png_infop		read_info_ptr,end_info_ptr;
	
	Rect			imageRect;
	png_uint_32		width,height;
	int				bit_depth,color_type;
	int				interlace_type,compression_type,filter_type;
	double			gamma;
	CTabHandle		ctable=NULL;
	int				num_palette;
	png_colorp		palette;
	short			i;
	short			pixelDepth=0;
	
	int				y;
	PixMapHandle	portPix;
	Ptr				baseAddr;
	long			rowBytes;
	png_bytep		*row_bufp=NULL;
	int				result;
	
	png_bytep		trans=NULL;
	int				num_trans;
	
	GWorldPtr		cPort;
	GDHandle		cDevice;
	
	GetGWorld(&cPort,&cDevice);
	
	/* ファイルを開く */
	err=FSpOpenDF(theFile,fsRdPerm,&myFile.refNum);
	if (err!=noErr) return err;
	
	/* シグネチャのチェック */
	err=FSRead(myFile.refNum,&size,buf);
	if (err!=noErr || size!=PNG_BYTES_TO_CHECK)
	{
		ErrorAlertFromResource(PNGERR_RESID,PNGERR2);
		FSClose(myFile.refNum);
		return err;
	}
	if (png_sig_cmp(buf,(png_size_t)0,(png_size_t)PNG_BYTES_TO_CHECK))
	{
		ErrorAlertFromResource(PNGERR_RESID,PNGERR3);
		FSClose(myFile.refNum);
		return unknownFormatErr; /* ま、なんでもいいんだけど */
	}
	
	/* 読み込み用の構造体 */
	read_ptr=png_create_read_struct(PNG_LIBPNG_VER_STRING,(png_voidp)NULL,
				png_default_error,png_default_warning);
	if (read_ptr == NULL)
	{
		FSClose(myFile.refNum);
		return memFullErr;
	}
	
	/* エラーハンドリングルーチン */
	myError.err=noErr;
	myError.cPort=cPort;
	myError.cDevice=cDevice;
	png_set_error_fn(read_ptr,(png_voidp)&myError,png_default_error,
				png_default_warning);
	
	/* 情報構造体 */
	read_info_ptr=png_create_info_struct(read_ptr);
	end_info_ptr=png_create_info_struct(read_ptr);
	
	/* setjmp */
	if (setjmp(read_ptr->jmpbuf))
	{
		ErrorAlertFromResource(PNGERR_RESID,PNGERR4);
		png_destroy_read_struct(&read_ptr,&read_info_ptr,&end_info_ptr);
		FSClose(myFile.refNum);
		return -2;
	}
	
	/* 読み込みルーチン設定 */
	png_set_read_fn(read_ptr,(png_voidp)&myFile,png_default_read_data);
	
	/* プログレスルーチン */
	png_set_write_status_fn(read_ptr,NULL);
	
	/* 読み込んであるバイト数を設定 */
	png_set_sig_bytes(read_ptr,PNG_BYTES_TO_CHECK);
	
	/* 設定を読み込む */
	png_read_info(read_ptr,read_info_ptr);
	
	/* widthなどを読み込む */
	result=png_get_IHDR(read_ptr,read_info_ptr,&width,&height,&bit_depth,&color_type,
						&interlace_type,&compression_type,&filter_type);
	if (!result)
	{
		FSClose(myFile.refNum);
		png_destroy_read_struct(&read_ptr,&read_info_ptr,&end_info_ptr);
		return -3;
	}
	
	/* 一応サイズのチェック */
	if (width > 32767 || height > 32767)
	{
		ErrorAlertFromResource(PNGERR_RESID,PNGERR5);
		FSClose(myFile.refNum);
		png_destroy_read_struct(&read_ptr,&read_info_ptr,&end_info_ptr);
		return -4;
	}
	
	SetRect(&imageRect,0,0,width,height);
	
	/* ガンマはどうしよう */
	png_get_gAMA(read_ptr,read_info_ptr,&gamma);
	
	/* カラータイプをチェックし、アルファチャンネルがある時は警告して削除 */
	if ((color_type & PNG_COLOR_MASK_ALPHA) != 0)
	{
		ErrorAlertFromResource(PNGERR_RESID,PNGERR6);
		png_set_strip_alpha(read_ptr);
		
		color_type -= PNG_COLOR_MASK_ALPHA;
	}
	
	/* 16bitの場合は8bitにする */
	if (bit_depth == 16) png_set_strip_16(read_ptr);
	
	/* カラータイプごとに場合わけし、カラーパレットを作成 */
	switch (color_type)
	{
		case PNG_COLOR_TYPE_PALETTE:
			/* パレット */
			/* カラーパレットを読み込む */
			pixelDepth=bit_depth;
			
			png_get_PLTE(read_ptr,read_info_ptr,&palette,&num_palette);
			
			/* カラーテーブルを作成する */
			ctable=(CTabHandle)NewHandle(sizeof(ColorTable)+sizeof(ColorSpec)*(num_palette-1));
			if (ctable == nil)
			{
				ErrorAlertFromResource(PNGERR_RESID,PNGERR7);
				FSClose(myFile.refNum);
				png_destroy_read_struct(&read_ptr,&read_info_ptr,&end_info_ptr);
				return memFullErr;
			}
			
			/* カラーテーブルに値を代入 */
			HLock((Handle)ctable);
			(*ctable)->ctSeed=UniqueID('clut');
			(*ctable)->ctFlags=0;
			(*ctable)->ctSize=num_palette-1;
			
			for (i=0; i<num_palette; i++)
			{
				(*ctable)->ctTable[i].value=i;
				(*ctable)->ctTable[i].rgb.red=((UInt16)palette[i].red)<<8;
				(*ctable)->ctTable[i].rgb.green=((UInt16)palette[i].green)<<8;
				(*ctable)->ctTable[i].rgb.blue=((UInt16)palette[i].blue)<<8;
			}
			HUnlock((Handle)ctable);
			
			if (useTransColor)
			{
				/* パレットの時だけ透過色を調べる */
				png_get_tRNS(read_ptr,read_info_ptr,&trans,&num_trans,NULL);
			}
			break;
		
		case PNG_COLOR_TYPE_GRAY:
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			/* グレースケール */
			/* カラーテーブルを作成する */
			pixelDepth=(bit_depth ==16 ? 8 : bit_depth);
			
			ctable=GetGrayscaleCTable(pixelDepth,false);
			if (ctable == nil)
			{
				ErrorAlertFromResource(PNGERR_RESID,PNGERR7);
				FSClose(myFile.refNum);
				png_destroy_read_struct(&read_ptr,&read_info_ptr,&end_info_ptr);
				return memFullErr;
			}
			break;
		
		case PNG_COLOR_TYPE_RGB:
		case PNG_COLOR_TYPE_RGB_ALPHA:
			/* RGB */
			/* カラーテーブルはいらない */
			pixelDepth=24;
			ctable=nil;
			png_set_filler(read_ptr,0,PNG_FILLER_BEFORE);
			break;
	}
	
	/* アップデート */
	png_read_update_info(read_ptr,read_info_ptr);
	
	/* カラーパレット、ビット数に従ってGWorldを作成 */
	err=NewGWorld(theImage,pixelDepth,&imageRect,ctable,0,useTempMem);
	if (err!=noErr)
	{
		ErrorAlertFromResource(PNGERR_RESID,PNGERR7);
		if (ctable!=nil) DisposeHandle((Handle)ctable);
		FSClose(myFile.refNum);
		png_destroy_read_struct(&read_ptr,&read_info_ptr,&end_info_ptr);
		return err;
	}
	
	/* アドレスの設定 */
	SetGWorld(*theImage,0);
	portPix=GetGWorldPixMap(*theImage);
	LockPixels(portPix);
	baseAddr=MyGetPixBaseAddr(portPix);
	rowBytes=MyGetPixRowBytes(portPix) & 0x3fff;
	
	/* row_bufを確保 */
	row_bufp=(png_bytep *)NewPtr(sizeof(png_bytep)*height);
	
	for (y=0; y<height; y++)
	{
		row_bufp[y]=(png_bytep)baseAddr;
		baseAddr+=rowBytes;
	}
	
	/* 読み込み */
	png_read_image(read_ptr,row_bufp);
	DisposePtr((Ptr)row_bufp);
	
	/* 読み込み終了 */
	png_read_end(read_ptr,end_info_ptr);
	
	UnlockPixels(portPix);
	
	/* 透過色の反映 */
	if (trans != NULL)
	{
		GWorldPtr	maskGWorld;
		ColorSearchUPP	csUPP;
		
		err=NewGWorld(&maskGWorld,1,&imageRect,nil,0,useTempMem);
		if (err==noErr)
		{
			SetGWorld(maskGWorld,0);
			LockPixels(GetGWorldPixMap(maskGWorld));
			EraseRect(&imageRect);
		//	PaintRect(&imageRect);	/* とりあえず全体がマスク */
			
			csUPP = NewColorSearchUPP(MySearchTransProc);
			AddSearch(csUPP);
			
			for (i=0; i<num_trans; i++)
			{
				if (trans[i]==0) /* 透明 */
				{
					gTransColor=(*ctable)->ctTable[i].rgb;
					CopyBits(GetPortBitMapForCopyBits(*theImage),
						GetPortBitMapForCopyBits(maskGWorld),
						&imageRect,&imageRect,srcOr,NULL);
				}
			}
			
			DelSearch(csUPP);
			DisposeColorSearchUPP(csUPP);
			
			InvertRect(&imageRect);
			
			UnlockPixels(GetGWorldPixMap(maskGWorld));
			
			SetGWorld(*theImage,0);
			ForeColor(whiteColor);
			CopyBits(GetPortBitMapForCopyBits(maskGWorld),
				GetPortBitMapForCopyBits(*theImage),
				&imageRect,&imageRect,notSrcOr,NULL);
			ForeColor(blackColor);
			
			*maskImage = maskGWorld;
		}
	}
	
	SetGWorld(cPort,cDevice);
	
	/* データの破棄 */
	png_destroy_read_struct(&read_ptr,&read_info_ptr,&end_info_ptr);
	DisposeHandle((Handle)ctable);
	
	/* ファイルを閉じる */
	FSClose(myFile.refNum);
	
	return noErr;
}

/* 色を探す */
static pascal Boolean MySearchTransProc(RGBColor *color,long *pos)
{
	if (EqualColor(color,&gTransColor))
		*pos = 1;
	else
		*pos = 0;
	return true;
}

/* 読み込みルーチン */
static void png_default_read_data(png_structp png_ptr,png_bytep data,png_size_t length)
{
	OSErr	err;
	long	llong=length;
	
	err=FSRead(((MyPNGFileRec *)png_ptr->io_ptr)->refNum,&llong,data);
	if (err!=noErr)
	{
		png_error(png_ptr,"Read Error");
	}
}

/* 書き出しルーチン */
static void png_default_write_data(png_structp png_ptr,png_bytep data,png_size_t length)
{
	OSErr	err;
	long	llong=length;
	short	refNum=((MyPNGFileRec *)png_ptr->io_ptr)->refNum;
	
	err=FSWrite(refNum,&llong,data);
	if (err!=noErr)
	{
		png_error(png_ptr,"Write Error");
	}
}

/* 警告ルーチン */
static void png_default_warning(png_structp png_ptr,png_const_charp message)
{
	#pragma unused(png_ptr,message)
	
	/* とりあえずなにもしない */
}

/* エラールーチン */
static void png_default_error(png_structp png_ptr,png_const_charp message)
{
	Str255	errStr,tempStr;
	MyPNGErrorRec	*myError = png_ptr->error_ptr;
	
	myError->err=-1;
	
	GetIndString(tempStr,PNGERR_RESID,PNGERR8);
	SetGWorld(myError->cPort,myError->cDevice);
	MyCToPStr(message,errStr);
	PStrCat(errStr,tempStr);
	
	ErrorAlert(tempStr);
}
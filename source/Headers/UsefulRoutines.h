/* ------------------------------------------------------------ */
/*  Useful Routines.h                                           */
/*     使えるルーチンのヘッダファイル                           */
/*                                                              */
/*                 1997.9.21 - 2001.1.27  naoki iimura		    */
/* ------------------------------------------------------------ */


#ifdef __APPLE_CC__
#include	<Carbon/Carbon.h>
#else
#include	<StandardFile.h>
#endif

/* prototypes */
void	ToolBoxInit(void);
void	ErrorAlert(const StringPtr errStr);
void	ErrorAlert2(const StringPtr errStr,short errCode);
void	ErrorAlertFromResource(short res_id,short num);

OSErr	MyConfirmDialog(const StringPtr prompt,const StringPtr okString,short *defaultSelection);

OSErr	GetApplSpec(OSType creator,FSSpec *applSpec);
OSErr	CreatorToAppl(OSType creator,Str255 applName);
pascal Boolean	FilterInvisFiles(CInfoPBPtr pb);
void	HiliteButton(DialogPtr dp,short item);
pascal Boolean	MyModalDialogFilter(DialogPtr theDialog,EventRecord *theEvent,short *theItemHit);
extern pascal void	MyActivate(DialogPtr theDialog,short item,Boolean activating,void *userData);

/* エイリアス解決 */
OSErr	ResolveAliasFileWithNoUI(FSSpec *fromFile,Boolean resolveAliasChains,Boolean *wasAliased);
OSErr	ResolveAliasFileWithNoUIMain(FSSpec *fromFile,Boolean resolveAliasChains);
OSErr	MyIsAliasFile(const FSSpec *fileFSSpec,Boolean *aliasFileFlag,Boolean *folderFlag);
OSErr	ResolveAliasWithNoUI(AliasHandle theAlias,FSSpec *theFile,Boolean resolveAliasChains);

/* ウィンドウのコントロールのアクティベート／ディアクティベート */
void	DeactivateWindowControl(WindowPtr theWindow);
void	ActivateWindowControl(WindowPtr theWindow);

/* ダイアログのアイテム操作補助 */
Handle	GetDialogItemHandle(DialogPtr theDialog,short itemNo);
void	GetDialogItemRect(DialogPtr theDialog,short itemNo,Rect *r);
short	GetDialogControlValue(DialogPtr theDialog,short itemNo);
void	SetDialogControlValue(DialogPtr theDialog,short itemNo,short value);
void	SetDialogControlMaximum(DialogPtr theDialog,short itemNo,short value);
void	SetDialogControlHilite(DialogPtr theDialog,short itemNo,short value);
void	SetDialogControlTitle(DialogPtr theDialog,short itemNo,Str255 title);
void	GetDialogItemText2(DialogPtr theDialog,short itemNo,Str255 string);
void	SetDialogItemText2(DialogPtr theDialog,short itemNo,Str255 string);
void	SetDialogItemToStaticText(DialogPtr theDialog,short itemNo);

/* 文字列の置き換え */
void	ReplaceString(Str255 base,Str255 subs,Str15 key);

/* RGBColor関係 */
Boolean	EqualColor(RGBColor *color1,RGBColor *color2);
short	RGBColorToIndex(RGBColor *color);
void	IndexToRGBColor(short index,RGBColor *color);
void	Set256RGBColor(RGBColor *color,short r,short g,short b);
void	SetRGBColor(RGBColor *color,unsigned short red,unsigned short green,unsigned short blue);

/* C文字列からPascal文字列へ変換 */
void	MyCToPStr(const char *src,Str255 dst);

/* leftが8の倍数でなくても大丈夫なCopyDeepMask */
void	SafeCopyDeepMask(const BitMap *srcBits,const BitMap *maskBits,const BitMap *dstBits,
						const Rect *srcRect,const Rect *maskRect,const Rect *dstRect,
						short mode,RgnHandle maskRgn);

/* テンポラリメモリを使ったPICT作成 */
OSErr	TempOpenCPicture(OpenCPicParams *params);
OSErr	TempClosePicture(PicHandle *resultPict);
pascal void	PutPicToTempMem(const void *p,SInt16 bytes);
OSErr	CheckPictureByte(void);

/* PICTの色数を調べる */
OSErr	GetPictureUniqueColors(PicHandle picture,long *uniqueColors);
OSErr	GetPictureMaxDepth(PicHandle picture,short *depth);

/* グレースケールのカラーテーブルを作成 */
CTabHandle	GetGrayscaleCTable(short depth,Boolean order);
OSErr	GetPixMapColors(PixMapHandle pmh,CTabHandle *ctab,short *colorNum);

/* クリエータとタイプからプロセスを検索 */
Boolean	FindProcessFromCreatorAndType(OSType creator,OSType type,ProcessSerialNumber *psn);

/* ダイアログのコントロールのアクティベート／ディアクティベート */
void	DeactivateDialogControl(DialogPtr theDialog);
void	ActivateDialogControl(DialogPtr theDialog);

/* 修正日を得る */
OSErr	FSpGetModDate(FSSpec *spec,unsigned long *modDate);

/* 拡張子関係 */
short	GetBodyLength(Str255 filename);
short	ChangeSuffix(Str255 filename,Str255 suffix);
short	GetBodyLength2(Str255 filename);
void	GetSuffix(Str255 filename,Str255 suffix);
OSErr	GetFileTypeFromSuffix(Str255 filename,OSType *fileType);

/* Finder関係？ */
OSErr	MakeOpenDocumentEvent(ProcessSerialNumber *targetPSN,FSSpec *theIconFile,AppleEvent *odocEvent);
OSErr	MakeUpdateEvent(const FSSpec *theFile,AppleEvent *result);
void	AEOpenFileWithApplication(FSSpec *theFile,FSSpec *theApplication);

/* FindFolderで指定するボリュームを得る */
SInt16  GetFindFolderVRefNum(void);

/* drag & drop */
pascal OSErr GetDropDirectory (DragReference dragRef, FSSpecPtr fssOut);

/* lists */
void	CancelSelect(ListHandle theList);

/* finder info */
OSErr	FSpGetFinderInfo(const FSSpec *theFile, FileInfo *info);

/* unicode */
void	FSpGetFileName(const FSSpec *theFile,Str255 filename);

/* Rect */
void OSSwapHostToBigRect(Rect *rect);

/* from MoreFilesExtras.h */
#ifdef __MOREFILESX__
void 
TruncPString(
  StringPtr          destination,
  ConstStr255Param   source,
  short              maxLength);
#endif

/* macros */
#define	PStrCpy(s, d)	{ BlockMove((s), (d), *(s) +1); }
#define	PStrCat(s, d)	{ BlockMove(&((s)[1]), &((d)[*(d) +1]), *(s)); *(d) += *(s); }
#define	CatChar(c, d)	{ ((d)[*(d) +1]) = (c); *(d) += 1; }

#define	LockGWorldPixels(x)		LockPixels(GetGWorldPixMap(x));
#define	UnlockGWorldPixels(x)	UnlockPixels(GetGWorldPixMap(x));

#define MySetGWorld(x)		SetGWorld(eWinRec->x,0);
#define MyLockPixels(x)		LockGWorldPixels(eWinRec->x);
#define	MyUnlockPixels(x)	UnlockGWorldPixels(eWinRec->x);

#define	min(a,b)	( a < b ? a : b )

#define	IPGetIconDepth(cm)	(cm)
#define	IPGetIconCTabHandle(cm)	(nil)
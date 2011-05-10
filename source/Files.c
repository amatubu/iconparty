/* ------------------------------------------------------------ */
/*  Files.c                                                     */
/*     ファイル処理                                             */
/*                                                              */
/*                 1997.1.28 - 2001.1.27  naoki iimura        	*/
/* ------------------------------------------------------------ */

/* includes */
#ifdef __APPLE_CC__
#include	<Carbon/Carbon.h>
#include	<QuickTime/QuickTime.h>
#else
#include	<Translation.h>
#include	<PictUtils.h>
#include	<StandardFile.h>
#include	<Scrap.h>
#include	<Sound.h>
#include	<ToolUtils.h>
#include	<Gestalt.h>
#include	<Folders.h>
#include	<Controls.h>
#include	<MacTypes.h>
#include	<QuickTimeComponents.h>
#ifndef scrollBarProc
#include	<ControlDefinitions.h>
#endif
#endif

#if __APPLE_CC__
#include	"MoreFilesX.h"
#else
#include	"MoreFilesExtras.h"
#endif

#include	"Globals.h"
#include	"FileRoutines.h"
#include	"MenuRoutines.h"
#include	"WindowExtensions.h"
#include	"UsefulRoutines.h"
#include	"IconParty.h"
#include	"NavigationServicesSupport.h"
#include	"PreCarbonSupport.h"
#include	"IconRoutines.h"
#include	"IconListWindow.h"
#include	"IconFamilyWindow.h"
#include	"WinIconSupport.h"
#include	"WindowRoutines.h"
#include	"Preferences.h"
#include	"PNGSupport.h"
#if !TARGET_API_MAC_CARBON
#include	"CustomSaveDialog.h"
#endif
#include	"EditRoutines.h"
#include	"ToolRoutines.h"
#include	"PaintRoutines.h"
#include	"UpdateCursor.h"


/* prototypes */
static short	InitPaintWinRec(WindowPtr theWindow);
static void	DisposePaintWinRec(WindowPtr theWindow);
static void	MakeUntitledFileName(OSType fileType,Str255 name);

static void	SaveAsPict(WindowPtr theWindow,const FSSpec *theFile);
static void	SaveAsPNG(WindowPtr theWindow,const FSSpec *theFile);

static short	GetRatioFromPict(FSSpec *spec);

#if !TARGET_API_MAC_CARBON
static pascal void	ActivatePaintWindow(WindowPtr eWindow,Boolean activateWindow);
#endif

static Boolean	IsFileOpened(FSSpec *theFile);
static OSErr	OpenPictFile(FSSpec *theFile);
static OSErr	OpenPNGFile(FSSpec *theFile);
static Boolean	IsBackAvailable(PicHandle picture,short *layerNum);
static pascal void	MyGetLayerInfoProc(short kind,short dataSize,Handle dataHandle);
static void	SpritPictAndLoad(PicHandle picture,PaintWinRec *eWinRec);
static pascal void	MyGetLayerDataProc(const BitMap *bitPtr,const Rect *srcRect,const Rect *dstRect,short mode,RgnHandle maskRgn);

static short	ConfigurePictSize(PaintWinPrefsRec *pRec);
static pascal Boolean CmdDFilter(DialogPtr dp,EventRecord *theEvent,short *item);

#if 0
static void	AESaveAsGIFWithClip2gif(FSSpec *theFile,WindowPtr theWindow,Boolean interlaced,short transColorIndex);
static OSErr	GetPSNOfClip2gif(ProcessSerialNumber *psn);
#endif

static void	SaveAsIcon(WindowPtr theWindow,FSSpec *theFile);
static void	SaveAsSplitIcon(WindowPtr theWindow,FSSpec *theFile);
static void	SaveAsWinIcon(WindowPtr theWindow,FSSpec *theFile);
static void	SaveAsSplitWinIcon(WindowPtr theWindow,FSSpec *theFile);

static void	AddPreviewIcon(WindowPtr theWindow,const FSSpec *theFile);

/* ドラッグ関連 */
static pascal short	MyPaintWinTrackingHandler(DragTrackingMessage theMessage,WindowPtr theWindow,
											void *handlerRefCon,DragReference theDrag);
static Boolean IsMyPaintWinTypeAvailable(DragReference theDrag);
static pascal short	MyPaintWinReceiveHandler(WindowPtr theWindow,void *handlerRefCon,
										DragReference theDrag);

/* gifencode.c */
extern void	GifEncode(FSSpec *spec,GWorldPtr theImage,RgnHandle saveRgn,
					Boolean interlaced,short transColorIndex);

#if CALL_NOT_IN_CARBON
/* StdGetFolder.c */
extern void StandardGetFolder(const short numTypes, ConstSFTypeListPtr typeList, StandardFileReply *reply);
#endif


#define	FILEERR_RESID	4010
#define	FILEERR1	1
#define	FILEERR2	2
#define	FILEERR3	3
#define	FILEERR4	4
#define	FILEERR5	5
#define	FILEERR6	6
#define	FILEERR7	7
#define	FILEERR8	8
#define	FILEERR9	9
#define	FILEERR10	10
#define	FILEERR11	11
#define	FILEERR12	12


/* 新しいアイコンを作成 */
void NewPaintWindow(void)
{
	WindowPtr	theWindow;
	PaintWinRec	*eWinRec;
	Str255	name;
	PaintWinPrefsRec pRec = gPaintWinPrefs;
	KeyMap	theKey;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	if (MaxBlock()<100*1024)
	{
		ErrorAlertFromResource(FILEERR_RESID,FILEERR1);
		return;
	}
	
	GetKeys(theKey);
	long     temp;
#if TARGET_RT_LITTLE_ENDIAN
	temp = EndianS32_BtoN (theKey[1].bigEndianValue);
#else
	temp = theKey[1];
#endif	
	if (gPaintWinPrefs.askSize ^ ((temp&0x00000004) !=0))
		if (ConfigurePictSize(&pRec)==cancel) return;
	
	theWindow=MakePaintWindow(&pRec.iconSize,pRec.ratio,pRec.colorMode);
	if (theWindow==nil)
	{
		ErrorAlertFromResource(FILEERR_RESID,FILEERR1);
		return;
	}
	
	eWinRec=GetPaintWinRec(theWindow);
	
	eWinRec->iconType.fileType=kPICTFileType;
	eWinRec->iconType.creatorCode=gPICTCreator;
	eWinRec->iconHasChanged=false;
	eWinRec->iconHasSaved=false;
	eWinRec->undoMode=umCannot;
	
	/* 背景を設定 */
	GetGWorld(&cPort,&cDevice);
	SetGWorld(eWinRec->currentMask,0);
	LockPixels(GetGWorldPixMap(eWinRec->currentMask));
	switch (pRec.background)
	{
		case 0: /* 白 */
			PaintRect(&pRec.iconSize);
			break;
		
		case 1: /* 透明 */
			EraseRect(&pRec.iconSize);
			eWinRec->isBackTransparent=true;
			break;
		
		case 2: /* 背景色 */
			if (!gBackColor.isTransparent) /* 背景色が透明色でない */
			{
				PaintRect(&pRec.iconSize);
				
				/* 背景色で塗る */
				SetGWorld(eWinRec->editDataPtr,0);
				LockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
				RGBBackColor(&gBackColor.rgb);
				EraseRect(&pRec.iconSize);
				UnlockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
			}
			else
			{
				EraseRect(&pRec.iconSize);
				eWinRec->isBackTransparent=true;
			}
			break;
	}
	UnlockPixels(GetGWorldPixMap(eWinRec->currentMask));
	SetGWorld(cPort,cDevice);
	
	MakeUntitledFileName(kPICTFileType,name);
	PStrCpy(name,eWinRec->saveFileSpec.name);
	
	SetEditWindowTitle(theWindow); /* ウィンドウタイトルの設定 */
	
	#if powerc
	if (gSystemVersion >= 0x0850)
	{
		OSErr	err;
		
		err=SetWindowProxyCreatorAndType(theWindow,gPICTCreator,kPICTFileType,kOnSystemDisk);
		err=SetWindowModified(theWindow,false);
	}
	#endif
	
	gUsedCount.newNum++;
	
	/* メニューを更新する */
	UpdateMenus();
	
	#if CALL_NOT_IN_CARBON
	{
		GrafPtr	port;
		
		GetPort(&port);
		SetPortWindowPort(gPreviewWindow);
		InvalRect(&eWinRec->iconSize);
		SetPort(port);
	}
	#else
	{
		Rect	r;
		
		GetWindowPortBounds(gPreviewWindow,&r);
		InvalWindowRect(gPreviewWindow,&r);
	}
	#endif
}



#define	dImageSize	135
enum {
	diNewImageSizeH=3,
	diNewImageSizeV,
	diNewImageColorMode,
	diNewImageSizeRatio,
	diNewImageReferClip,
	diNewImageBackground,
};

/* サイズダイアログ */
static short ConfigurePictSize(PaintWinPrefsRec *pRec)
{
	DialogPtr	dp;
	short		item;
	short		i;
	Str255		str[2];
	Str255		clipSize[2];
	long		dataSize=0L;
	PicHandle	clipPic;
	long		uniqueColors;
	short		defaultColorModeIndex,
				clipColorModeIndex;
	Boolean		referClip=gPaintWinPrefs.referClip;
	GrafPtr		port;
	ModalFilterUPP	mfUPP=NewModalFilterUPP(MyModalDialogFilter);
	#if TARGET_API_MAC_CARBON
	OSErr		err;
	ScrapRef	scrap;
	#else
	long		offset;
	#endif
	
	GetPort(&port);
	
	defaultColorModeIndex = (gPaintWinPrefs.colorMode == kNormal8BitColorMode ? 1 : 2);
	clipColorModeIndex = defaultColorModeIndex;
	
	/* PICTの存在をチェック */
	#if TARGET_API_MAC_CARBON
	do {
		err=GetCurrentScrap(&scrap);
		if (err!=noErr) break;
		err=GetScrapFlavorSize(scrap,kPICTFileType,&dataSize);
		if (err!=noErr) break;
	} while (false);
	#else
	dataSize=GetScrap(nil,kPICTFileType,&offset);
	#endif
	if (dataSize>0)
	{
		OSErr	err;
		
		clipPic=(PicHandle)TempNewHandle(dataSize,&err);
		if (err==noErr)
		{
			TempHLock((Handle)clipPic,&err);
			#if CALL_NOT_IN_CARBON
			dataSize=GetScrap((Handle)clipPic,kPICTFileType,&offset);
			#else
			err=GetScrapFlavorData(scrap,kPICTFileType,&dataSize,*clipPic);
			#endif
			TempHUnlock((Handle)clipPic,&err);
			
			NumToString((**clipPic).picFrame.right-(**clipPic).picFrame.left,clipSize[0]);
			NumToString((**clipPic).picFrame.bottom-(**clipPic).picFrame.top,clipSize[1]);
			
			err=GetPictureUniqueColors(clipPic,&uniqueColors);
			if (err==noErr) clipColorModeIndex = (uniqueColors <= 256 ? 1 : 2);
			
			TempDisposeHandle((Handle)clipPic,&err);
		}
		else /* PICTはあるが、読み込むメモリがない場合はないことにする */
			dataSize=0;
	}
	
	DeactivateFloatersAndFirstDocumentWindow();
	dp=GetNewDialog(dImageSize,nil,kFirstWindowOfClass);
	if (dp==nil)
	{
		SysBeep(0);
		return -1;
	}
	
	if (isAppearanceAvailable)
	{
		Rect	r;
		ControlHandle	c;
		
		GetDialogItemRect(dp,9,&r);
		c=NewControl(GetDialogWindow(dp),&r,"\p",true,0,0,0,160,0);
	}
	
	NumToString(pRec->iconSize.bottom,str[1]);
	NumToString(pRec->iconSize.right,str[0]);
	
	SetDialogControlValue(dp,diNewImageSizeRatio,pRec->ratio+1);
	
	SetDialogControlValue(dp,diNewImageReferClip,referClip);
	if (dataSize<=0)
		SetDialogControlHilite(dp,diNewImageReferClip,255);
	
	SetDialogControlValue(dp,diNewImageBackground,pRec->background);
	
	if (referClip && dataSize>0)
	{
		for (i=0; i<2; i++)
			SetDialogItemText2(dp,diNewImageSizeH+i,clipSize[i]);
		SetDialogControlValue(dp,diNewImageColorMode,clipColorModeIndex);
	}
	else
	{
		for (i=0; i<2; i++)
			SetDialogItemText2(dp,diNewImageSizeH+i,str[i]);
		SetDialogControlValue(dp,diNewImageColorMode,defaultColorModeIndex);
	}
	SelectDialogItemText(dp,diNewImageSizeH,0,255);
	
	ShowWindow(GetDialogWindow(dp));
	SetPortDialogPort(dp);
	
	SetDialogDefaultItem(dp,ok);
	SetDialogCancelItem(dp,cancel);
	
	item=3;
	while(item!=ok && item!=cancel)
	{
		ModalDialog(mfUPP,&item);
		
		switch (item)
		{
			case diNewImageReferClip:
				referClip=(unsigned char)~referClip;
				SetDialogControlValue(dp,diNewImageReferClip,referClip);
				if (referClip)
				{
					for (i=0; i<2; i++)
					{
						GetDialogItemText2(dp,i+diNewImageSizeH,str[i]);
						SetDialogItemText2(dp,i+diNewImageSizeH,clipSize[i]);
					}
					defaultColorModeIndex = GetDialogControlValue(dp,diNewImageColorMode);
					SetDialogControlValue(dp,diNewImageColorMode,clipColorModeIndex);
				}
				else
				{
					for (i=0; i<2; i++)
						SetDialogItemText2(dp,i+diNewImageSizeH,str[i]);
					SetDialogControlValue(dp,diNewImageColorMode,defaultColorModeIndex);
				}
				SelectDialogItemText(dp,diNewImageSizeH,0,255);
				break;
		}
	}
	
	if (item==ok)
	{
		long	l;
		
		GetDialogItemText2(dp,diNewImageSizeH,str[0]);
		StringToNum(str[0],&l);
		pRec->iconSize.right=(short)l;
		GetDialogItemText2(dp,diNewImageSizeV,str[1]);
		StringToNum(str[1],&l);
		pRec->iconSize.bottom=(short)l;
		
		if (GetDialogControlValue(dp,diNewImageColorMode) == 1)
			pRec->colorMode = kNormal8BitColorMode;
		else
			pRec->colorMode = k32BitColorMode;
		
		pRec->ratio=GetDialogControlValue(dp,diNewImageSizeRatio)-1;
		
		gPaintWinPrefs.referClip=referClip;
		
		pRec->background=GetDialogControlValue(dp,diNewImageBackground)-1;
	}
	
	DisposeDialog(dp);
	DisposeModalFilterUPP(mfUPP);
	ActivateFloatersAndFirstDocumentWindow();
	SetPort(port);
	UpdateMenus();
	
	return item;
}

/* 新しいアイコンファイル（アイコンリソースを含んだファイル）を作成 */
void NewIconFile(void)
{
	WindowPtr	iconWindow;
	IconListWinRec	*iWinRec;
	
	/* ウィンドウを作成 */
	iconWindow=MakeIconWindow(0);
	if (iconWindow==nil) return;
	
	/* データの初期化 */
	iWinRec=GetIconListRec(iconWindow);
	iWinRec->wasSaved=false;
	GetIndString(iWinRec->iconFileSpec.name,145,2);
	SetWTitle(iconWindow,iWinRec->iconFileSpec.name);
	
	ShowReferencedWindow(iconWindow);
	
	UpdateMenus();
}

/* アイコンファイルを作成 */
OSErr MakeIconFile(IconListWinRec *iWinRec)
{
	#if !TARGET_API_MAC_CARBON
	StandardFileReply	reply;
	#endif
	OSErr	err=noErr;
	NavReplyRecord	theReply;
	Boolean	sfReplacing=false;
	FSSpec	theFile;
	
	DeactivateFloatersAndFirstDocumentWindow();
	
	/* ファイル名を決めてファイル作成 */
	#if TARGET_API_MAC_CARBON
	if (isNavServicesAvailable)
	#else
	if (isNavServicesAvailable && useNavigationServices)
	#endif
	{
		GetIndString(theFile.name,145,2);
		err=NewIconFileWithNav(&theFile,&theReply);
		sfReplacing=false;
	}
	#if !TARGET_API_MAC_CARBON
	else
	{
		Str255	prompt,fileName;
		Point	where={-1,-1};
		ActivateYDUPP	aUPP=NewActivateYDProc(MyActivate);
		
		GetIndString(fileName,145,2);
		GetIndString(prompt,145,1);
		CustomPutFile(prompt,fileName,&reply,401,where,nil,nil,nil,aUPP,nil);
		if (reply.sfGood)
		{
			theFile = reply.sfFile;
			sfReplacing = reply.sfReplacing;
		}
		else
			err = userCanceledErr;
		
		DisposeRoutineDescriptor(aUPP);
	}
	#endif
	
	ActivateFloatersAndFirstDocumentWindow();
	if (err==noErr)
	{
		short	refNum;
		
		if (sfReplacing)
			err=FSpDelete(&theFile);
		
		FSpCreateResFile(&theFile,gIconCreator,kResourceFileType,smSystemScript);
		if (ResError()!=noErr)
		{
			ErrorAlertFromResource(FILEERR_RESID,FILEERR2);
			return ResError();
		}
		
		if (isNavServicesAvailable && useNavigationServices)
		{
			if (theReply.validRecord)
				err=NavCompleteSave(&theReply,kNavTranslateInPlace);
			err=NavDisposeReply(&theReply);
		}
		
		/* ファイルを開く */
		refNum=FSpOpenResFile(&theFile,fsRdWrPerm);
		if (refNum<=0) return ResError();
		
		iWinRec->iconFileSpec=theFile;
		iWinRec->refNum=refNum;
		iWinRec->wasSaved=true;
		
		return noErr;
	}
	else return userCanceledErr;
}

/* アイコンウィンドウの作成 */
WindowPtr MakePaintWindow(Rect *iconSize,short ratio,short colorMode)
{
	OSErr	err;
	WindowPtr	theWindow;
	PaintWinRec	**newEditRec;
	Point	windPos=kDefaultWinPos;
	Point	windOffset=kWinOffset;
	Rect	validRect;
	Rect	portRect;
	#if !TARGET_API_MAC_CARBON
	ActivateHandlerUPP	activateUPP;
	#endif
	
	GetRegionBounds(GetGrayRgn(),&validRect);
	UseResFile(gApplRefNum);
	
	/* 編集画面を作成 */
	err=GetNewWindowReference(&theWindow,kPaintWindowResID,kFirstWindowOfClass,nil);
	if (err!=noErr) return nil;
	
	SetExtWindowKind(theWindow,kWindowTypePaintWindow);
	
	/* ウィンドウの大きさ */
	SetPortWindowPort(theWindow);
	SetPaintWindowSize(theWindow,iconSize,ratio);
	SizeWindow(gPreviewWindow,iconSize->right,iconSize->bottom,true);
	
	SetPortWindowPort(gPreviewWindow);
	ClipRect(iconSize);
	
	/* ウィンドウの位置 */
	SetPortWindowPort(theWindow);
	GetWindowPortBounds(theWindow,&portRect);
	validRect.left=windPos.h;
	validRect.top=windPos.v;
	validRect.right-=portRect.right-80;
	validRect.bottom-=portRect.bottom-40;
	StackWindowPos(&windPos,windOffset,&validRect);
	MoveWindow(theWindow,windPos.h,windPos.v,false);
	
	/* 再びサイズを設定する */
	SetPaintWindowSize(theWindow,iconSize,ratio);
	
	newEditRec=(PaintWinRec **)NewHandleClear(sizeof(PaintWinRec));
	HLockHi((Handle)newEditRec);
	SetExtWRefCon(theWindow,(long)newEditRec);
	
	(**newEditRec).iconSize = *iconSize;
	(**newEditRec).ratio = ratio;
	(**newEditRec).colorMode = colorMode;
	(**newEditRec).iconDepth = IPGetIconDepth(colorMode);
	
	err=InitPaintWinRec(theWindow);
	if (err != noErr)
	{
		HUnlock((Handle)newEditRec);
		DisposeHandle((Handle)newEditRec);
		DisposeWindowReference(theWindow);
		
		return nil;
	}
	
	#if !TARGET_API_MAC_CARBON
	/* アクティベートルーチンの設定 */
	activateUPP=NewActivateHandlerProc((ProcPtr)ActivatePaintWindow);
	SetActivateHandlerProc(theWindow,activateUPP);
	(**newEditRec).dragHandlers.activateUPP=activateUPP;
	#endif
	
	/* ドラッグ関連 */
	if (isDragMgrPresent)
	{
		DragTrackingHandlerUPP	dragTrackUPP;
		DragReceiveHandlerUPP	dragReceiveUPP;
		
		dragTrackUPP=NewDragTrackingHandlerUPP(MyPaintWinTrackingHandler);
		if (dragTrackUPP != nil)
		{
			MyDragGlobalsRec	dragGlobals;
			
			dragGlobals.canAcceptDrag=false;
			dragGlobals.isInContent=false;
			(**newEditRec).dragHandlers.dragGlobals=dragGlobals;
			
			err=InstallTrackingHandler(dragTrackUPP,theWindow,&(**newEditRec).dragHandlers.dragGlobals);
			(**newEditRec).dragHandlers.trackUPP=dragTrackUPP;
			dragReceiveUPP=NewDragReceiveHandlerUPP(MyPaintWinReceiveHandler);
			if (dragReceiveUPP != nil)
			{
				err=InstallReceiveHandler(dragReceiveUPP,theWindow,&(**newEditRec).dragHandlers.dragGlobals);
				(**newEditRec).dragHandlers.receiveUPP=dragReceiveUPP;
			}
		}
	}
	
	ShowReferencedWindow(theWindow);
	#if TARGET_API_MAC_CARBON
	SetPortWindowPort(theWindow);
	MyInvalWindowPortBounds(theWindow);
	#endif
	
	return theWindow;
}

#if !TARGET_API_MAC_CARBON
/* ペイントウィンドウのアクティベートルーチン */
pascal void	ActivatePaintWindow(WindowPtr eWindow,Boolean activateWindow)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(eWindow);
	GrafPtr	port;
	
	GetPort(&port);
	SetPortWindowPort(eWindow);
	
	if (activateWindow)
	{
		/* アクティベートの時はスクロールバーを使用可能に */
		HiliteControl(eWinRec->hScroll,0);
		HiliteControl(eWinRec->vScroll,0);
	}
	else
	{
		/* デアクティベートの時はスクロールバーを使用不可能に */
		HiliteControl(eWinRec->hScroll,255);
		HiliteControl(eWinRec->vScroll,255);
		
		ResetRuler();
	}
	SetPort(port);
}
#endif

/* エディットウィンドウの各種初期化 */
short InitPaintWinRec(WindowPtr theWindow)
{
	OSErr	err;
	Rect	hScrollRect,vScrollRect;
	ControlHandle	hScroll,vScroll;
	PaintWinRec	*newEditRec=GetPaintWinRec(theWindow);
	Rect	iconSize=newEditRec->iconSize;
	GWorldPtr	offPort;
	Rect	portRect;
	Str255	cntlName;
	short	iconDepth = newEditRec->iconDepth;
	
	/* サイズ */
	newEditRec->iconEditSize=iconSize;
	newEditRec->iconEditSize.right<<=newEditRec->ratio;
	newEditRec->iconEditSize.bottom<<=newEditRec->ratio;
	
	newEditRec->editDataPtr=nil;
	newEditRec->tempDataPtr=nil;
	newEditRec->selectedDataPtr=nil;
	newEditRec->dispTempPtr=nil;
	newEditRec->currentMask=nil;
	newEditRec->pCurrentMask=nil;
	newEditRec->selectionMask=nil;
	
	newEditRec->eSelectedRgn=nil;
	newEditRec->ePrevSelectedRgn=nil;
	newEditRec->selectionPos=nil;
	newEditRec->prevSelectionPos=nil;
	newEditRec->updateRgn=nil;
	
	/* オフポートの初期化 */
	err=NewGWorld(&offPort,iconDepth,&iconSize,0,0,0);
	if (err!=noErr) goto errExit;
	newEditRec->editDataPtr=offPort;
	
	err=NewGWorld(&offPort,iconDepth,&iconSize,0,0,0);
	if (err!=noErr) goto errExit;
	newEditRec->tempDataPtr=offPort;
	
	err=NewGWorld(&offPort,iconDepth,&iconSize,0,0,0);
	if (err!=noErr) goto errExit;
	newEditRec->selectedDataPtr=offPort;
	
	err=NewGWorld(&offPort,iconDepth,&iconSize,0,0,0);
	if (err!=noErr) goto errExit;
	newEditRec->dispTempPtr=offPort;
	
	/* リージョンの初期化 */
	newEditRec->eSelectedRgn=NewRgn();
	newEditRec->ePrevSelectedRgn=NewRgn();
	newEditRec->selectionPos=NewRgn();
	newEditRec->prevSelectionPos=NewRgn();
	RectRgn(newEditRec->selectionPos,&newEditRec->iconSize);
	RectRgn(newEditRec->prevSelectionPos,&newEditRec->iconSize);
	
	newEditRec->updateRgn=NewRgn();
	
	/* 選択状態 */
	newEditRec->isSelected=false;
	newEditRec->selectedRect=newEditRec->iconSize;
	
	/* 前景、背景などの初期化 */
	newEditRec->foregroundGWorld=nil;
	newEditRec->foregroundMask=nil;
	
	err=NewGWorld(&offPort,1,&iconSize,0,0,useTempMem);
	if (err!=noErr) goto errExit;
	newEditRec->currentMask=offPort;
	
	err=NewGWorld(&offPort,1,&iconSize,0,0,useTempMem);
	if (err!=noErr) goto errExit;
	newEditRec->pCurrentMask=offPort;
	
	err=NewGWorld(&offPort,1,&iconSize,0,0,useTempMem);
	if (err!=noErr) goto errExit;
	newEditRec->selectionMask=offPort;
	
	newEditRec->backgroundGWorld=nil;
	
	/* 背景が透明かどうか */
	newEditRec->isBackTransparent=false;
	
	/* バックグラウンドモードなど */
	newEditRec->editBackground=gOtherPrefs.addForeground;
	newEditRec->dispBackground=true;
	newEditRec->foreTransparency=kForeTrans100;
	
	/* テンポラリファイル */
	newEditRec->tempFileRefNum=0;
	
	newEditRec->undoMode=umCannot;
	
	/* 描画点モード */
	newEditRec->isDotMode=false;
	SetPt(&newEditRec->dotPos,0,0);
	newEditRec->isInverted=false;
	newEditRec->lastInvertedTime=0;
	
	/* 画像領域の消去 */
	EraseOffPort(newEditRec);
	
	GetWindowPortBounds(theWindow,&portRect);
	
	/* スクロールバー */
	SetRect(&hScrollRect,portRect.left+kRatioWidth+kBackWidth-1,
				portRect.bottom-kScrollBarHeight,
				portRect.right+1-kScrollBarHeight,
				portRect.bottom+1);
	GetIndString(cntlName,159,1);
	hScroll=NewControl(theWindow,&hScrollRect,cntlName,true,0,0,0,
		(isAppearanceAvailable ? kControlScrollBarLiveProc : scrollBarProc),kHScroll);
	newEditRec->hScroll=hScroll;
	
	SetRect(&vScrollRect,portRect.right-kScrollBarWidth,
				portRect.top-1,
				portRect.right+1,
				portRect.bottom+1-kScrollBarHeight);
	GetIndString(cntlName,159,2);
	vScroll=NewControl(theWindow,&vScrollRect,cntlName,true,0,0,0,
		(isAppearanceAvailable ? kControlScrollBarLiveProc : scrollBarProc),kVScroll);
	newEditRec->vScroll=vScroll;
	
	SetScrollBarMax(theWindow);
	
	return noErr;
	
errExit:
	if (newEditRec->selectionMask!=nil) DisposeGWorld(newEditRec->selectionMask);
	if (newEditRec->pCurrentMask!=nil) DisposeGWorld(newEditRec->pCurrentMask);
	if (newEditRec->currentMask!=nil) DisposeGWorld(newEditRec->currentMask);
	if (newEditRec->editDataPtr!=nil) DisposeGWorld(newEditRec->editDataPtr);
	if (newEditRec->tempDataPtr!=nil) DisposeGWorld(newEditRec->tempDataPtr);
	if (newEditRec->selectedDataPtr!=nil) DisposeGWorld(newEditRec->selectedDataPtr);
	if (newEditRec->dispTempPtr!=nil) DisposeGWorld(newEditRec->dispTempPtr);
	
	if (newEditRec->updateRgn!=nil) DisposeRgn(newEditRec->updateRgn);
	if (newEditRec->prevSelectionPos!=nil) DisposeRgn(newEditRec->prevSelectionPos);
	if (newEditRec->selectionPos!=nil) DisposeRgn(newEditRec->selectionPos);
	if (newEditRec->ePrevSelectedRgn!=nil) DisposeRgn(newEditRec->ePrevSelectedRgn);
	if (newEditRec->eSelectedRgn!=nil) DisposeRgn(newEditRec->eSelectedRgn);
	
	return err;
}

/* エディットウィンドウのデータを破棄する */
void DisposePaintWinRec(WindowPtr theWindow)
{
	PaintWinRec	*editWinRec;
	
	editWinRec=GetPaintWinRec(theWindow);
	
	DisposeGWorld(editWinRec->editDataPtr);
	DisposeGWorld(editWinRec->tempDataPtr);
	DisposeGWorld(editWinRec->selectedDataPtr);
	DisposeGWorld(editWinRec->dispTempPtr);
	
	DisposeRgn(editWinRec->eSelectedRgn);
	DisposeRgn(editWinRec->ePrevSelectedRgn);
	DisposeRgn(editWinRec->selectionPos);
	DisposeRgn(editWinRec->prevSelectionPos);
	
	/* マスク関係 */
	DisposeGWorld(editWinRec->currentMask);
	DisposeGWorld(editWinRec->selectionMask);
	DisposeGWorld(editWinRec->pCurrentMask);
	
	/* バックグラウンド関係 */
	if (editWinRec->backgroundGWorld != nil) DisposeGWorld(editWinRec->backgroundGWorld);
	if (editWinRec->foregroundGWorld != nil) DisposeGWorld(editWinRec->foregroundGWorld);
	if (editWinRec->foregroundMask != nil) DisposeGWorld(editWinRec->foregroundMask);
	
	DisposeRgn(editWinRec->updateRgn);
	
	DisposeControl(editWinRec->hScroll);
	DisposeControl(editWinRec->vScroll);
}

static short	untitledPictNum=1,untitledPNGNum=1;

/* 名称未設定のファイル名を作成 */
void MakeUntitledFileName(OSType fileType,Str255 name)
{
	Str255 str;
	
	GetIndString(name,sUntitled,1);
	
	switch (fileType)
	{
		case kPICTFileType:
			if (untitledPictNum>1)
			{
				CatChar(' ',name);
				NumToString(untitledPictNum,str);
				PStrCat(str,name);
			}
			GetIndString(str,sFileSuffix,siPICTFile);
			untitledPictNum++;
			break;
		
		#if 0
		case kGIFFileType:
			if (untitledGifNum>1)
			{
				CatChar(' ',name);
				NumToString(untitledGifNum,str);
				PStrCat(str,name);
			}
			GetIndString(str,sFileSuffix,siGIFFile);
			untitledGifNum++;
			break;
		#endif
		
		case kPNGFileType:
			if (untitledPNGNum>1)
			{
				CatChar(' ',name);
				NumToString(untitledPNGNum,str);
				PStrCat(str,name);
			}
			GetIndString(str,sFileSuffix,siPNGFile);
			untitledPNGNum++;
			break;
	}
	PStrCat(str,name);
}

/* ペイントウィンドウを閉じる */
short ClosePaintWindow(WindowPtr theWindow,Boolean quitFlag)
{
	PaintWinRec	**eWinRec;
	OSErr	err=noErr;
	
	if (theWindow==nil) return 0;
	
	switch (GetExtWindowKind(theWindow))
	{
		case kWindowTypeIconListWindow:
			err=CloseIconFile(theWindow,quitFlag);
			UpdateMenus();
			ActivateFloatersAndFirstDocumentWindow();
			return err;
			break;
		
		case kWindowTypePaintWindow:
			eWinRec=(PaintWinRec **)GetExtWRefCon(theWindow);
			
			if ((**eWinRec).iconHasChanged) /* 変更されていれば保存する */
			{
				short	item;
				
				#if !TARGET_API_MAC_CARBON
				if (isNavServicesAvailable && useNavigationServices)
				#endif
				{
					/* フローティングウィンドウがあるとNavigation Servicesがフロントウィンドウを
					  誤認してしまう問題に対処。あまりきれいな方法ではないが、いちばんましかな */
					SuspendFloatingWindows();
					item=AskSaveWithNav((**eWinRec).saveFileSpec.name,
						(quitFlag ? kNavSaveChangesQuittingApplication : kNavSaveChangesClosingDocument));
					ResumeFloatingWindows();
				}
				#if !TARGET_API_MAC_CARBON
				else
				{
					Str255	docuKind;
					
					GetIndString(docuKind,133,3);
					item=SaveYN(docuKind,(**eWinRec).saveFileSpec.name,
						(quitFlag ? kSaveChangedQuitApplication : kSaveChangedCloseWindow));
				}
				#endif
				
				switch (item)
				{
					case ok:
						err=SavePaintWindow(false,false);
						break;
					
					case cancel:
						err=userCanceledErr;
						break;
					
					case kDialogItemDontSave:
						err=noErr;
						break;
					
					default:
						err=item;
				}
			}
			
			if (err==noErr)
			{
				/* ドラッグ関連 */
				if (isDragMgrPresent)
				{
					err=RemoveTrackingHandler((**eWinRec).dragHandlers.trackUPP,theWindow);
					err=RemoveReceiveHandler((**eWinRec).dragHandlers.receiveUPP,theWindow);
					
					#if CALL_NOT_IN_CARBON
					DisposeRoutineDescriptor((**eWinRec).dragHandlers.trackUPP);
					DisposeRoutineDescriptor((**eWinRec).dragHandlers.receiveUPP);
					#else
					DisposeDragTrackingHandlerUPP((**eWinRec).dragHandlers.trackUPP);
					DisposeDragReceiveHandlerUPP((**eWinRec).dragHandlers.receiveUPP);
					#endif
				}
				
				#if CALL_NOT_IN_CARBON
				DisposeRoutineDescriptor((**eWinRec).dragHandlers.activateUPP);
				#endif
				
				/* アイコンウィンドウに付随する変数を消去する */
				DisposePaintWinRec(theWindow);
				HUnlock((Handle)eWinRec);
				DisposeHandle((Handle)eWinRec);
				
				DisposeWindowReference(theWindow);
				
				UpdateMenus();
				ActivateFloatersAndFirstDocumentWindow();
			}
			return err;
			break;
		
		case kWindowTypeIconFamilyWindow:
			err=CloseFamilyWindow(theWindow,true,false);
			UpdateMenus();
			ActivateFloatersAndFirstDocumentWindow();
			return err;
			break;
	}
	return 0;
}

/* ペイントウィンドウの保存 */
short SavePaintWindow(Boolean selectionFlag,Boolean saveAsFlag)
{
	OSType	fType;
	OSType	fCreator;
	WindowPtr	theWindow;
	short	windowKind;
	PaintWinRec	*eWinRec;
	FSSpec	saveFileSpec;
	NavReplyRecord	navReply;
	Boolean	saveDialogFlag;
	Rect	r;
	long	num;
	OSErr	err;	
	
	theWindow=MyFrontNonFloatingWindow();
	if (theWindow==nil) return noErr;
	
	windowKind = GetExtWindowKind(theWindow);
	if (windowKind != kWindowTypePaintWindow) return -1;
	
	eWinRec=GetPaintWinRec(theWindow);
	saveDialogFlag=(!eWinRec->iconHasSaved || selectionFlag || saveAsFlag);
	
	/* 選択領域が存在し、そこだけを保存するのでなければ固定する */
	if (eWinRec->isSelected && !selectionFlag)
	{
		SetPortWindowPort(theWindow);
		FixSelection(theWindow);
	}
	
	/* バックグラウンドが表示されていれば、戻す */
	if (!selectionFlag && eWinRec->foregroundGWorld != nil)
		GoBackgroundMode(theWindow);
	
	fType=eWinRec->iconType.fileType;
	fCreator=eWinRec->iconType.creatorCode;
	if (selectionFlag)
		r = eWinRec->selectedRect;
	else
		r = eWinRec->iconSize;
	num = ((r.right - r.left-1)/32+1) * ((r.bottom - r.top-1)/32+1);
	
	/* 保存されていないあるいは選択領域のみの保存のときはファイル名などを決めてもらう */
	if (saveDialogFlag)
	{
		OSErr	err=noErr;
		PStrCpy(eWinRec->saveFileSpec.name,saveFileSpec.name);
		
		DeactivateFloatersAndFirstDocumentWindow();
		#if !TARGET_API_MAC_CARBON
		if (isNavServicesAvailable && useNavigationServices)
		#endif
			err=SaveFileWithNav(&saveFileSpec,&fType,&navReply,num);
		#if !TARGET_API_MAC_CARBON
		else
			if (!StandardSaveAs(&saveFileSpec,&fType,num))
				err=userCanceledErr;
		#endif
		ActivateFloatersAndFirstDocumentWindow();
		if (err!=noErr) return err;
	}
	else
		saveFileSpec=eWinRec->saveFileSpec;
	
	/* 実際の保存 */
	gUsedCount.saveNum++;
	
	switch (fType)
	{
		case kPICTFileType:
			SaveAsPict(theWindow,&saveFileSpec);
			break;
		
		case kPNGFileType:
			SaveAsPNG(theWindow,&saveFileSpec);
			break;
		
		case 'Icon':
			SaveAsIcon(theWindow,&saveFileSpec);
			break;
		
		case 'IcoS':
			SaveAsSplitIcon(theWindow,&saveFileSpec);
			break;
		
		case kIconFamilyType:
			SaveToFamily(theWindow);
			break;
		
		case 'wIco':
			SaveAsWinIcon(theWindow,&saveFileSpec);
			break;
		
		case 'wIcS':
			SaveAsSplitWinIcon(theWindow,&saveFileSpec);
			break;
	}
	
	#if !TARGET_API_MAC_CARBON
	if (isNavServicesAvailable && useNavigationServices && saveDialogFlag)
	#else
	if (saveDialogFlag)
	#endif
	{
		if (navReply.validRecord)
		{
			err=NavCompleteSave(&navReply,kNavTranslateInPlace);
		}
		err=NavDisposeReply(&navReply);
	}
	
	if (!selectionFlag) /* 選択範囲のみの保存でなければ */
	{
		eWinRec->iconType.fileType=fType;
		eWinRec->iconType.creatorCode=(fType==kPICTFileType ? gPICTCreator : gGIFCreator);
		
		eWinRec->saveFileSpec=saveFileSpec;
		
		eWinRec->iconHasSaved=true;
		SetEditWindowTitle(theWindow); /* ウィンドウタイトルの変更 */
		eWinRec->iconHasChanged=false;
		
		#if powerc
		if (gSystemVersion >= 0x0850)
		{
			FInfo	fileInfo;
			
			err=FSpGetFInfo(&saveFileSpec,&fileInfo);
			if (err!=noErr)
				err=RemoveWindowProxy(theWindow);
			else
				err=SetWindowProxyFSSpec(theWindow,&saveFileSpec);
			err=SetWindowModified(theWindow,false);
		}
		#endif
		
		UpdateSaveMenu();
	}
	return 0;
}

/* アイコンを開く */
void OpenIcon(void)
{
	UseResFile(gApplRefNum);
	
	DeactivateFloatersAndFirstDocumentWindow();
	#if !TARGET_API_MAC_CARBON
	if (NavServicesAvailable() && useNavigationServices)
	#else
	if (NavServicesAvailable())
	#endif
	{
		Str255	str;
		OSErr	err;
		
		GetIndString(str,138,1);
		
		err=OpenFileWithNav();
	}
	#if !TARGET_API_MAC_CARBON
	else
	{
		OSType	type[]={kPICTFileType,kResourceFileType,kPNGFileType,kWinIconFileType,kXIconFileType,kQTFileTypeQuickTimeImage};
		
		StandardFileReply	reply;
		FSSpec	spec;
		
		if (gQTVersion>=0x03000000)
			StandardGetFilePreview(nil,6,type,&reply);
		else if (gQTVersion>=0x0250000)
			StandardGetFilePreview(nil,5,type,&reply);
		else
			StandardGetFile(0,5,type,&reply);
		
		gIsDialog=false;
		
		if (reply.sfGood)
		{
			spec=reply.sfFile;
			
			HandleOpenDoc(&spec);
		}
	}
	#endif
	ActivateFloatersAndFirstDocumentWindow();
}
		
/* PICTとして保存 */
void SaveAsPict(WindowPtr theWindow,const FSSpec *theFile)
{
	OSErr	err;
	PicHandle	savePict;
	PaintWinRec *eWinRec=GetPaintWinRec(theWindow);
	short	fRef;
	
	savePict=GetSelectionPic(theWindow,true);
	if (savePict==nil)
	{
		ErrorAlertFromResource(FILEERR_RESID,FILEERR4);
		return;
	}
	
	err=SaveAsPictMain(savePict,theFile);
	if (err!=noErr)
	{
		KillPicture(savePict);
		ErrorAlertFromResource(FILEERR_RESID,FILEERR3);
		return;
	}
	
	/* 倍率情報を書き込む */
	fRef=FSpOpenResFile(theFile,fsRdWrPerm);
	if (fRef<0)
	{
		FSpCreateResFile(theFile,gPICTCreator,kPICTFileType,smSystemScript);
		fRef=FSpOpenResFile(theFile,fsRdWrPerm);
	}
	if (fRef>0)
	{	
		short	ratio=eWinRec->ratio;
		
		err=AddDataToPrefs(&ratio,sizeof(ratio),kIconPartyCreator,128,"\pmag");
		
		CloseResFile(fRef);
	}
	
	/* カスタムアイコンの保存 */
	if (gPreviewIconFlag)
		AddPreviewIcon(theWindow,theFile);
	
	err=FlushVol(0L,theFile->vRefNum);
	
	KillPicture(savePict);
}

/* PICT保存メイン */
OSErr SaveAsPictMain(PicHandle picture,const FSSpec *theFile)
{
	short	i,fRef;
	long	cnt,zero;
	OSErr	err;
	
	/* ファイルを作成し、保存 */
	err=FSpOpenDF(theFile,fsWrPerm,&fRef);
	if (err==fnfErr)
	{
		err=FSpCreate(theFile,gPICTCreator,kPICTFileType,smSystemScript);
		err=FSpOpenDF(theFile,fsWrPerm,&fRef);
	}
	else if (err!=noErr)
		return err;
	
	zero=0;
	cnt=4;
	for (i=0; i<128; i++)
	{
		err=FSWrite(fRef,&cnt,&zero); /* 0を512バイト書き込む */
		if (err!=noErr || cnt!=4)
		{
			FSClose(fRef);
			err=FSpDelete(theFile);
			return -1;
		}
	}
	cnt=GetHandleSize((Handle)picture);
	HLock((Handle)picture);
	err=FSWrite(fRef,&cnt,*picture);
	HUnlock((Handle)picture);
	err=SetEOF(fRef,cnt+512);
	err=FSClose(fRef);
	
	return noErr;
}	

/* プレビューアイコンの追加 */
void AddPreviewIcon(WindowPtr theWindow,const FSSpec *theFile)
{
	short iconSize[]={32,32,16,16};
	short iconDepth[]={8,1,8,1};
	ResType iconType[]={kLarge8BitData,kLarge1BitMask,kSmall8BitData,kSmall1BitMask};
	
	IconSuiteRef	iconSuite;
	OSErr			err;
	Handle			h;
	short			i;
	
	err=NewIconSuite(&iconSuite);
	
	for (i=0; i<4; i++)
	{
		h=GetSelectionIconHandle(theWindow,iconSize[i],iconDepth[i],true);
		err=AddIconToSuite(h,iconSuite,iconType[i]);
	}
	
	err=MakeFileWithIcon(theFile,iconSuite);
	
	err=DisposeIconSuite(iconSuite,true);
}

enum {
	kTransColorNoneIndex=-1,
	kTransColorTransIndex=256,
};

/* PNGとして保存 */
void SaveAsPNG(WindowPtr theWindow,const FSSpec *theFile)
{
	short		transColorIndex=0;		/* 透過色のインデックス */
	RGBColor	transColor;
	short		right,bottom;
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	OSErr		err;
	CTabHandle	ctab=NULL;
	short		colorNum=256,depth=eWinRec->iconDepth;
	
	RgnHandle		selectedRgn;
	Rect			selectedRect;
	PixMapHandle	portPix;
	PicHandle		picture;
	GWorldPtr		tempGWorld;
	GWorldPtr		cPort;
	GDHandle		cDevice;
	
	Boolean			paintWithTrans=false,isTransUsed=false;
	
	picture=GetSelectionPic(theWindow,true);
	if (picture == nil)
	{
		ErrorAlertFromResource(FILEERR_RESID,FILEERR4);
		return;
	}
	
	/* 透明部分があるかどうか */
	if (!eWinRec->backgroundGWorld) /* まず、背景がないこと */
	{
		if (eWinRec->isSelected)
		{
			MenuHandle	menu;
			CharParameter	c;
			
			menu=GetMenuHandle(mEffect);
			GetItemMark(menu,iOpaque,&c);
			
			isTransUsed = (c == 0);
		}
		else
		{
			RgnHandle	tempRgn=NewRgn(),tempRgn2=NewRgn();
			
			err=BitMapToRegion(tempRgn,GetPortBitMapForCopyBits(eWinRec->currentMask));
			if (err==noErr)
			{
				RectRgn(tempRgn2,&eWinRec->iconSize);
				isTransUsed = !EqualRgn(tempRgn,tempRgn2);
			}
			DisposeRgn(tempRgn);
			DisposeRgn(tempRgn2);
		}
	}
	
	GetGWorld(&cPort,&cDevice);
	
	/* 保存用のGWorld生成 */
	selectedRect=eWinRec->selectedRect;
	err=NewGWorld(&tempGWorld,eWinRec->iconDepth,&selectedRect,0,0,useTempMem);
	if (err!=noErr)
	{
		ErrorAlertFromResource(FILEERR_RESID,FILEERR4);
		return;
	}
	
	/* GWorldに絵をはりつける */
	portPix=GetGWorldPixMap(tempGWorld);
	SetGWorld(tempGWorld,0);
	
	LockPixels(portPix);
	EraseRect(&selectedRect);
	DrawPicture(picture,&selectedRect);
	
	/* 透過色 */
	switch (gPNGFilePrefs.transColor)
	{
		case kPNGTransNone:
			transColorIndex=kTransColorNoneIndex;
			break;
		
		case kPNGTransWhite:
			transColor=rgbWhiteColor;
			break;
		
		case kPNGTransBlack:
			transColor=rgbBlackColor;
			break;
		
		case kPNGTransBottomRight:
			/* 右下の色を調べる */
			right=selectedRect.right -1;
			bottom=selectedRect.bottom -1;
			
			LockPixels(portPix);
			GetCPixel(right,bottom,&transColor);
			UnlockPixels(portPix);
			break;
		
		case kPNGTransPenColor:
			/* 現在の選択色のインデックスを求める */
			if (gCurrentColor.isTransparent)
				if (eWinRec->backgroundGWorld)
					transColorIndex=kTransColorNoneIndex;
				else
					transColorIndex=kTransColorTransIndex;
			else
				transColor=gCurrentColor.rgb;
			break;
		
		case kPNGTransEraserColor:
			/* 背景色のインデックスを求める */
			if (gBackColor.isTransparent)
				if (eWinRec->backgroundGWorld)
					transColorIndex=kTransColorNoneIndex;
				else
					transColorIndex=kTransColorTransIndex;
			else
				transColor=gBackColor.rgb;
			break;
		
		case kPNGTransTransColor:
			if (eWinRec->backgroundGWorld)
				transColorIndex=kTransColorNoneIndex;
			else
				transColorIndex=kTransColorTransIndex;
			break;
	}
	
	/* 透明部分を強制的に透過させる */
	if (transColorIndex == 0 && gPNGFilePrefs.useTransColor.inSaving && isTransUsed)
	{
		paintWithTrans=true;
		
		RGBForeColor(&transColor);
		CopyBits(GetPortBitMapForCopyBits(eWinRec->currentMask),GetPortBitMapForCopyBits(tempGWorld),
			&selectedRect,&selectedRect,notSrcOr,nil);
		ForeColor(blackColor);
	}
	
	UnlockPixels(portPix);
	
	/* 色数などを調査し、最適化（8ビットのときだけ） */
	if (gPNGFilePrefs.optimizeImage && eWinRec->colorMode == kNormal8BitColorMode)
	{
		LockPixels(portPix);
		err=GetPixMapColors(portPix,&ctab,&colorNum);
		UnlockPixels(portPix);
		if (err==noErr)
		{
			GWorldFlags	flags;
			
			/* 透過色＝透明 */
			if (isTransUsed && (transColorIndex == kTransColorTransIndex || 
				(gPNGFilePrefs.useTransColor.inSaving && transColorIndex == kTransColorNoneIndex)))
			{
				if (colorNum < 256)
				{
					/* 色数が足りれば透過用の色を追加する */
					ColorSpec	*cspec;
					
					transColor.red=transColor.blue=transColor.green=0xfe00;
					
					SetHandleSize((Handle)ctab,sizeof(ColorTable)+sizeof(ColorSpec)*colorNum);
					cspec=&((*ctab)->ctTable[colorNum-1]);
					cspec[1]=cspec[0];
					cspec->rgb=transColor;
					cspec->value=colorNum;
					(*ctab)->ctSize=colorNum;
					transColorIndex = colorNum-1;
					
					CTabChanged(ctab);
					
					colorNum++;
					
					paintWithTrans=true;
				}
				else
					transColorIndex = kTransColorNoneIndex;
			}
			
			depth = (colorNum > 16 ? 8 : (colorNum > 4 ? 4 : (colorNum > 2 ? 2 : 1)));
			
			flags=UpdateGWorld(&tempGWorld,depth,&selectedRect,ctab,0,0);
			portPix=GetGWorldPixMap(tempGWorld);
			
			/* はりつけなおす */
			SetGWorld(tempGWorld,0);
			LockPixels(portPix);
			EraseRect(&selectedRect);
			DrawPicture(picture,&selectedRect);
			
			/* 透明部分を強制的に透過させる */
			if (paintWithTrans)
			{
				RGBForeColor(&transColor);
				CopyBits(GetPortBitMapForCopyBits(eWinRec->currentMask),
					GetPortBitMapForCopyBits(tempGWorld),
					&selectedRect,&selectedRect,notSrcOr,nil);
				ForeColor(blackColor);
			}
			
			UnlockPixels(portPix);
		}
		DisposeHandle((Handle)ctab);
	}
	
	TempDisposeHandle((Handle)picture,&err);
	
	/* 透過色を再マッピング */
	switch (eWinRec->colorMode)
	{
		case kNormal8BitColorMode:
			if (transColorIndex == 0)
			{
				if (RealColor(&transColor))
					transColorIndex = Color2Index(&transColor);
				else
					transColorIndex = kTransColorNoneIndex;
			}
			else if (transColorIndex == kTransColorTransIndex)
			{
				if (!gPNGFilePrefs.optimizeImage)
				{
					transColorIndex = kTransColorNoneIndex;
				}
			}
			break;
		
		case k32BitColorMode:
			/* とりあえず、32ビットでは、透過色=透明色は不可 */
			if (transColorIndex == kTransColorTransIndex)
				transColorIndex = kTransColorNoneIndex;
			break;
	}
	
	selectedRgn=NewRgn();
	CopyRgn(eWinRec->selectionPos,selectedRgn);
	if (eWinRec->isSelected) /* 選択範囲あり */
	{
		CopyRgn(eWinRec->selectionPos,selectedRgn);
		OffsetRgn(selectedRgn,-eWinRec->selectionOffset.h,-eWinRec->selectionOffset.v);
	}
	else
		RectRgn(selectedRgn,&eWinRec->iconSize);
	
	/* 実際の保存ルーチン */
	PNGEncode(theFile,tempGWorld,selectedRgn,colorNum,depth,gPNGFilePrefs.interlaced,
		transColorIndex,&transColor,gPNGFilePrefs.compLevel,gPNGCreator);
	
	/* プレビュー */
	if (gPreviewIconFlag)
		AddPreviewIcon(theWindow,theFile);
	
	SetGWorld(cPort,cDevice);
	DisposeRgn(selectedRgn);
	DisposeGWorld(tempGWorld);
}

#if 0
/* clip2gifを利用して保存する */
void AESaveAsGIFWithClip2gif(FSSpec *theFile,WindowPtr theWindow,Boolean interlaced,short transColorIndex)
{
	ProcessSerialNumber	psn;
	OSErr	err;
	AppleEvent		aeEvent={typeNull,NULL};
	AEAddressDesc	target={typeNull,NULL};
	AEDescList		rgbList={typeNull,NULL};
	PicHandle		picture;
	OSType			fileType=kGIFFileType;
	OSType			transType;
	
	err=GetPSNOfClip2gif(&psn);
	if (err!=noErr)
	{
		SysBeep(0);
		return;
	}
	
	/* 保存するPICTを作成 */
	picture=GetSelectionPic(theWindow,true);
	if (picture==nil)
	{
		SysBeep(0);
		return;
	}
	HLock((Handle)picture);
	
	/* ターゲット */
	err=AECreateDesc(typeProcessSerialNumber,&psn,sizeof(ProcessSerialNumber),&target);
	
	/* イベント作成 */
	err=AECreateAppleEvent(kAECoreSuite,kAESave,&target,kAutoGenerateReturnID,kAnyTransactionID,&aeEvent);
	
	/* PICTをdirect parameterに指定 */
	err=AEPutParamPtr(&aeEvent,keyDirectObject,kPICTFileType,*picture,GetHandleSize((Handle)picture));
	
	/* 保存形式(as) */
	err=AEPutParamPtr(&aeEvent,keyAEFileType,typeType,&fileType,sizeof(OSType));
	
	/* 保存先(in) */
	err=AEPutParamPtr(&aeEvent,keyAEFile,typeFSS,theFile,sizeof(FSSpec));
	
	/* インタレース */
	err=AEPutParamPtr(&aeEvent,'incd',typeBoolean,&interlaced,sizeof(Boolean));
	
	/* 透過色 */
	if (transColorIndex==-1) /* なし */
	{
		transType='no  ';
		err=AEPutParamPtr(&aeEvent,'trcy','etcy',&transType,sizeof(OSType));
	}
	else
	{
		long		l;
		RGBColor	transColor;
		
		IndexToRGBColor(transColorIndex,&transColor);
		
		/* リスト作成 */
		err=AECreateList(nil,0,false,&rgbList);
		
		l=(long)transColor.red;
		err=AEPutPtr(&rgbList,1,typeInteger,&l,sizeof(long));
		l=(long)transColor.green;
		err=AEPutPtr(&rgbList,2,typeInteger,&l,sizeof(long));
		l=(long)transColor.blue;
		err=AEPutPtr(&rgbList,3,typeInteger,&l,sizeof(long));
		
		err=AEPutParamDesc(&aeEvent,'trcy',&rgbList);
	}
	
	/* 送付 */
	err=AESend(&aeEvent,nil,kAEWaitReply+kAECanSwitchLayer+kAEAlwaysInteract,
				kAENormalPriority,kNoTimeOut,nil,nil);
	
	/* 破棄 */
	HUnlock((Handle)picture);
	DisposeHandle((Handle)picture);
	
	AEDisposeDesc(&aeEvent);
	AEDisposeDesc(&target);
	AEDisposeDesc(&rgbList);
}

/* clip2gifを探す */
OSErr GetPSNOfClip2gif(ProcessSerialNumber *psn)
{
	Boolean				found;
	LaunchParamBlockRec	launchParams;
	FSSpec				theClip2gifSpec;
	OSErr				err;
	
	/* clip2gifを探す */
	found=FindProcessFromCreatorAndType(kClip2gifCreator,kClip2gifType,psn);
	if (found) return noErr;
	
	/* 起動していない時はLaunchApplicationで起動し、そのPSNを返す */
	err=GetApplSpec(kClip2gifCreator,&theClip2gifSpec);
	if (err!=noErr)
	{
		isClip2gifAvailable=false;
		return err;
	}
	
	launchParams.launchBlockID=extendedBlock;
	launchParams.launchEPBLength=extendedBlockLen;
	launchParams.launchFileFlags=0;
	launchParams.launchControlFlags=launchContinue+launchNoFileFlags+launchDontSwitch;
	launchParams.launchAppSpec=&theClip2gifSpec;
	launchParams.launchAppParameters=nil;
	
	err=LaunchApplication(&launchParams);
	
	if (err==noErr) *psn=launchParams.launchProcessSN;
	return err;
}
#endif

/* アイコンとして保存 */
void SaveAsIcon(WindowPtr theWindow,FSSpec *theFile)
{
	short iconSize[]={32,32,32,16,16,16};
	short iconDepth[]={8,4,1,8,4,1};
	ResType iconType[]={kLarge8BitData,kLarge4BitData,kLarge1BitMask,kSmall8BitData,kSmall4BitData,kSmall1BitMask};
	
	IconSuiteRef	iconSuite;
	OSErr			err;
	Handle			h;
	short			i;
	
	err=NewIconSuite(&iconSuite);
	
	for (i=0; i<6; i++)
	{
		h=GetSelectionIconHandle(theWindow,iconSize[i],iconDepth[i],false);
		err=AddIconToSuite(h,iconSuite,iconType[i]);
	}
	
	err=MakeFolderWithIcon(theFile,iconSuite);
	
	err=DisposeIconSuite(iconSuite,true);
}

/* 分割アイコンとして保存 */
void SaveAsSplitIcon(WindowPtr theWindow,FSSpec *theFile)
{
	short		i,j;
	Rect		saveRect,splitRect,origRect;
	Boolean		saveSelected;
	short		width,height;
	long		num,count=1;
	Str31		temp,temp2;
	short		nLength;
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	
	if (eWinRec->isSelected)
		saveRect = eWinRec->selectedRect;
	else
		saveRect = eWinRec->iconSize;
	saveSelected = eWinRec->isSelected;
	origRect = eWinRec->iconSize;
	eWinRec->isSelected=false;
	
	/* 縦横と合計の数を計算 */
	width = (saveRect.right - saveRect.left-1) / 32+1;
	height = (saveRect.bottom - saveRect.top-1) / 32+1;
	num = width * height;
	
	/* ファイル（フォルダ）につける名前を作成(.###) */
	NumToString(num,temp);
	TruncPString(theFile->name,theFile->name,31-1-temp[0]);
	PStrCpy(theFile->name,temp2);
	for (i=1; i<=temp[0]; i++) temp[i]='0';
	CatChar('.',theFile->name);
	PStrCat(temp,theFile->name);
	nLength=theFile->name[0];
	
	for (j=0; j<height; j++) /* 横向きに作成 */
	{
		SetRect(&splitRect,saveRect.left,saveRect.top+j*32,saveRect.left+32,saveRect.top+j*32+32);
		
		for (i=0; i<width; i++)
		{
			/* 保存する場所を指定 */
			eWinRec->iconSize = eWinRec->selectedRect = splitRect;
			
			/* ファイル（フォルダ）の名前 */
			NumToString(count++,temp);
			BlockMoveData(&temp[1],&theFile->name[nLength-temp[0]+1],temp[0]);
			
			SaveAsIcon(theWindow,theFile);
			OffsetRect(&splitRect,32,0);
		}
	}
	
	/* 元に戻す */
	eWinRec->isSelected = saveSelected;
	eWinRec->iconSize = origRect;
	if (saveSelected)
		eWinRec->selectedRect = saveRect;
	else
		eWinRec->selectedRect = origRect;
	
	/* ファイル名は番号のない状態にしておく */
	PStrCpy(temp2,theFile->name);
}

/* Windowsアイコンとして保存 */
void SaveAsWinIcon(WindowPtr theWindow,FSSpec *theFile)
{
	Handle	h,h2;
	IconSuiteRef	iconSuite;
	OSErr	err;
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	short	width=eWinRec->iconSize.right - eWinRec->iconSize.left;
	short	height=eWinRec->iconSize.bottom - eWinRec->iconSize.top;
	
	/* IconSuiteを作成 */
	err=NewIconSuite(&iconSuite);
	if (err!=noErr)
	{
		ErrorAlertFromResource(FILEERR_RESID,FILEERR5);
		return;
	}
	
	if (width >= 32 || height >= 32) {
		/* 32*32 のアイコンデータを作成 */
		h=GetSelectionIconHandle(theWindow,32,8,false);
		h2=GetSelectionIconHandle(theWindow,32,1,false);
		if (h==nil || h2==nil)
		{
			if (h!=nil) DisposeHandle(h);
			if (h!=nil) DisposeHandle(h2);
			ErrorAlertFromResource(FILEERR_RESID,FILEERR6);
			return;
		}
		
		/* IconSuiteに設定 */
		err=AddIconToSuite(h,iconSuite,kLarge8BitData);
		err=AddIconToSuite(h2,iconSuite,kLarge1BitMask);
		if (err!=noErr)
		{
			ErrorAlertFromResource(FILEERR_RESID,FILEERR4);
			DisposeIconSuite(iconSuite,true);
			return;
		}
	} else {
		/* 16*16 のアイコンデータを作成 */
		h=GetSelectionIconHandle(theWindow,16,8,false);
		h2=GetSelectionIconHandle(theWindow,16,1,false);
		if (h==nil || h2==nil)
		{
			if (h!=nil) DisposeHandle(h);
			if (h!=nil) DisposeHandle(h2);
			ErrorAlertFromResource(FILEERR_RESID,FILEERR6);
			return;
		}
		
		/* IconSuiteに設定 */
		err=AddIconToSuite(h,iconSuite,kSmall8BitData);
		err=AddIconToSuite(h2,iconSuite,kSmall1BitMask);
		if (err!=noErr)
		{
			ErrorAlertFromResource(FILEERR_RESID,FILEERR4);
			DisposeIconSuite(iconSuite,true);
			return;
		}
	}
	
	err=MakeWinIconFromSuite(theFile,iconSuite);
	if (err!=noErr)
		ErrorAlertFromResource(FILEERR_RESID,FILEERR4);
	DisposeIconSuite(iconSuite,true);
}

/* 分割Winアイコンとして保存 */
void SaveAsSplitWinIcon(WindowPtr theWindow,FSSpec *theFile)
{
	short		i,j;
	Rect		saveRect,splitRect,origRect;
	Boolean		saveSelected;
	short		width,height;
	long		num,count=1;
	Str31		temp,temp2,suffix;
	short		nLength;
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	Boolean		b=true;
	
	if (eWinRec->isSelected)
		saveRect = eWinRec->selectedRect;
	else
		saveRect = eWinRec->iconSize;
	saveSelected = eWinRec->isSelected;
	origRect = eWinRec->iconSize;
	eWinRec->isSelected=false;
	
	/* 縦横と合計の数を計算 */
	width = (saveRect.right - saveRect.left-1) / 32+1;
	height = (saveRect.bottom - saveRect.top-1) / 32+1;
	num = width * height;
	
	/* ファイル（フォルダ）につける名前を作成(.###.ico) */
	GetIndString(suffix,131,5);
	for (i=1; i<=suffix[0]; i++)
		if (suffix[i] != theFile->name[theFile->name[0]-suffix[0]+i]) b=false;
	if (b) theFile->name[0]-=suffix[0];
	
	NumToString(num,temp);
	TruncPString(theFile->name,theFile->name,31-1-temp[0]-4);
	PStrCpy(theFile->name,temp2);
	for (i=1; i<=temp[0]; i++) temp[i]='0';
	CatChar('.',theFile->name);
	PStrCat(temp,theFile->name);
	nLength=theFile->name[0];
	PStrCat(suffix,theFile->name);
	PStrCat(suffix,temp2);
	
	for (j=0; j<height; j++) /* 横向きに作成 */
	{
		SetRect(&splitRect,saveRect.left,saveRect.top+j*32,saveRect.left+32,saveRect.top+j*32+32);
		
		for (i=0; i<width; i++)
		{
			/* 保存する場所を指定 */
			eWinRec->iconSize = eWinRec->selectedRect = splitRect;
			
			/* ファイル（フォルダ）の名前 */
			NumToString(count++,temp);
			BlockMoveData(&temp[1],&theFile->name[nLength-temp[0]+1],temp[0]);
			
			SaveAsWinIcon(theWindow,theFile);
			OffsetRect(&splitRect,32,0);
		}
	}
	
	/* 元に戻す */
	eWinRec->isSelected = saveSelected;
	eWinRec->iconSize = origRect;
	if (saveSelected)
		eWinRec->selectedRect = saveRect;
	else
		eWinRec->selectedRect = origRect;
	
	/* ファイル名は番号のない状態にしておく */
	PStrCpy(temp2,theFile->name);
}

/* PICTからの読み込み */
PicHandle LoadFromPict(FSSpec *picFileSpec)
{
	OSErr	err;
	short	fRef;
	long	pictSize;
	PicHandle	picture;
	
	err=FSpOpenDF(picFileSpec,fsRdPerm,&fRef);
	if (err!=noErr)
	{
		ErrorAlertFromResource(FILEERR_RESID,FILEERR7);
		return (PicHandle)0;
	}
	err=GetEOF(fRef,&pictSize);
	if (err!=noErr)
	{
		ErrorAlertFromResource(FILEERR_RESID,FILEERR8);
		FSClose(fRef);
		return (PicHandle)0;
	}
	
	err=SetFPos(fRef,fsFromStart,512); /* 512バイト読み飛ばす */
	if (err!=noErr)
	{
		ErrorAlertFromResource(FILEERR_RESID,FILEERR9);
		FSClose(fRef);
		return (PicHandle)0;
	}
	pictSize-=512;
	
	picture=(PicHandle)TempNewHandle(pictSize,&err);
	if (picture==nil || err!=noErr)
	{
		ErrorAlertFromResource(FILEERR_RESID,FILEERR10);
		FSClose(fRef);
		return (PicHandle)0;
	}
	TempHLock((Handle)picture,&err);
	err=FSRead(fRef,&pictSize,*picture);
	if (err!=noErr)
	{
		ErrorAlertFromResource(FILEERR_RESID,FILEERR9);
		FSClose(fRef);
		TempHUnlock((Handle)picture,&err);
		TempDisposeHandle((Handle)picture,&err);
		return (PicHandle)0L;
	}
	TempHUnlock((Handle)picture,&err);
	FSClose(fRef);
	
	return picture;
}

/* PICTファイルのリソースに倍率情報があれば利用 */
short GetRatioFromPict(FSSpec *spec)
{
	short	refNum;
	short	ratio=gPaintWinPrefs.ratio;
	OSErr	err;
	
	refNum=FSpOpenResFile(spec,fsRdPerm);
	if (refNum<=0) return ratio;
	
	UseResFile(refNum);
	// TODO little-endian
	err=LoadDataFromPrefs((Ptr)&ratio,sizeof(ratio),kIconPartyCreator,128);
#if TARGET_RT_LITTLE_ENDIAN
	if (err==noErr)
	{
		ratio = EndianS16_BtoL( ratio );
	}
#endif
	CloseResFile(refNum);
	UseResFile(gApplRefNum);
	
	return ratio;
}

/* PICTファイルを開く */
static OSErr OpenPictFile(FSSpec *spec)
{
	PicHandle	picture;
	Rect		picSize;
	WindowPtr	theWindow;
	PaintWinRec	*eWinRec;
	short	ratio;
	short	colorMode;
	short	maxDepth;
	OSErr	err;
	Boolean	isBackAvailable;
	short	layerNum;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	PictInfo	pictInfo;
	
	if (IsFileOpened(spec)) return noErr;
	
	GetGWorld(&cPort,&cDevice);
	
	if ((picture=LoadFromPict(spec)) ==nil) /* PICTファイルからの読み込み */
	{
		ErrorAlertFromResource(FILEERR_RESID,FILEERR11);
		return memFullErr;
	}
	
	/* 倍率情報をチェック */
	ratio=GetRatioFromPict(spec);
	
	/* 色数をチェック */
	err=GetPictureMaxDepth(picture,&maxDepth);
//	err=noErr;
//	maxDepth=8;
	if (err==noErr)
		colorMode = (maxDepth <= 8 ? kNormal8BitColorMode : k32BitColorMode);
	else
		colorMode=gPaintWinPrefs.colorMode;
	
	/* 左上の点を補正する */
	//	picSize=(**picture).picFrame;
	err=GetPictInfo(picture,&pictInfo,0,0,0,0);
	if (err!=noErr)
	{
		ErrorAlertFromResource(FILEERR_RESID, FILEERR8);
		return err;
	}
	picSize = pictInfo.sourceRect;
	OffsetRect(&picSize,-picSize.left,-picSize.top);
	
	/* 背景情報を含むかどうかを調べる */
	isBackAvailable=IsBackAvailable(picture,&layerNum);
	
	SetGWorld(cPort,cDevice);
	theWindow=MakePaintWindow(&picSize,ratio,colorMode);
	if (theWindow==nil)
	{
		/* ウィンドウを開けなかった時は読み込んだPICTを破棄 */
		ErrorAlertFromResource(FILEERR_RESID,FILEERR11);
		TempDisposeHandle((Handle)picture,&err);
		return memFullErr;
	}
	eWinRec=GetPaintWinRec(theWindow);
	eWinRec->iconSize=picSize;
	eWinRec->iconType.fileType=kPICTFileType;
	eWinRec->iconType.creatorCode=gPICTCreator;
	
	eWinRec->iconHasSaved=true;
	eWinRec->iconHasChanged=false;
	eWinRec->undoMode=umCannot;
	eWinRec->saveFileSpec=*spec;
	SetEditWindowTitle(theWindow);
	
	#if powerc
	if (gSystemVersion >= 0x0850)
	{
		err=SetWindowProxyFSSpec(theWindow,spec);
		err=SetWindowModified(theWindow,false);
	}
	#endif
	
	if (isBackAvailable)
	{
		switch (layerNum)
		{
			case 2:
				eWinRec->editBackground=false;	/* 前景編集モードになる */
				SpritPictAndLoad(picture,eWinRec);
				break;
			
			case 1:
				eWinRec->isBackTransparent=true;	/* 背景透明モード */
				SpritPictAndLoad(picture,eWinRec);
				break;
		}
	}
	else
	{
		/* 読み込んだ絵をオフポートに表示 */
		SetGWorld(eWinRec->editDataPtr,0);
		LockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
		DrawPicture(picture,&picSize);
		UnlockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
		
		RectRgn(eWinRec->updateRgn,&picSize);
		
		/* マスク */
		SetGWorld(eWinRec->currentMask,0);
		LockPixels(GetGWorldPixMap(eWinRec->currentMask));
		PaintRect(&picSize);
		UnlockPixels(GetGWorldPixMap(eWinRec->currentMask));
	}
	
	TempDisposeHandle((Handle)picture,&err);
	
	SetGWorld(cPort,cDevice);
	
	gUsedCount.openNum++;
	
	DispOffPort(theWindow);
	UpdateMenus();
	
	return noErr;
}

/* PNGファイルを開く */
static OSErr OpenPNGFile(FSSpec *theFile)
{
	Rect		picSize;
	WindowPtr	theWindow;
	PaintWinRec	*eWinRec;
	short		ratio;
	short		colorMode;
	OSErr		err;
	GWorldPtr	theImage,maskImage=NULL;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	GetGWorld(&cPort,&cDevice);
	
	if (IsFileOpened(theFile)) return noErr;
	
	/* PNGのデコード */
	err=PNGDecode(theFile,&theImage,&maskImage,gPNGFilePrefs.useTransColor.inLoading);
	if (err!=noErr) return err;
	
	/* 倍率情報をチェック */
	ratio=GetRatioFromPict(theFile);
	
	/* 色モード */
	if (GetPortDepth(theImage)>8)
		colorMode = k32BitColorMode;
	else
		colorMode = kNormal8BitColorMode;
	
	GetPortBounds(theImage,&picSize);
	
	theWindow=MakePaintWindow(&picSize,ratio,colorMode);
	if (theWindow==nil)
	{
		/* ウィンドウを開くことができない場合は用意したGWorldを破棄 */
		ErrorAlertFromResource(FILEERR_RESID,FILEERR11);
		DisposeGWorld(theImage);
		return memFullErr;
	}
	eWinRec=GetPaintWinRec(theWindow);
	eWinRec->iconSize=picSize;
	eWinRec->iconType.fileType=kPNGFileType;
	eWinRec->iconType.creatorCode=gPNGCreator;
	
	eWinRec->iconHasSaved=true;
	eWinRec->iconHasChanged=false;
	eWinRec->undoMode=umCannot;
	eWinRec->saveFileSpec=*theFile;
	SetEditWindowTitle(theWindow);
	
	#if powerc
	if (gSystemVersion >= 0x0850)
	{
		err=SetWindowProxyFSSpec(theWindow,theFile);
		err=SetWindowModified(theWindow,false);
	}
	#endif
	
	/* 読み込んだ絵を表示 */
	SetGWorld(eWinRec->editDataPtr,0);
	MyLockPixels(editDataPtr);
	CopyBits(GetPortBitMapForCopyBits(theImage),
		GetPortBitMapForCopyBits(eWinRec->editDataPtr),
		&picSize,&picSize,srcCopy+ditherCopy,nil);
	MyUnlockPixels(editDataPtr);
	
	RectRgn(eWinRec->updateRgn,&picSize);
	
	/* マスク */
	SetGWorld(eWinRec->currentMask,0);
	LockPixels(GetGWorldPixMap(eWinRec->currentMask));
	if (maskImage == NULL)
		PaintRect(&picSize);
	else
		CopyBits(GetPortBitMapForCopyBits(maskImage),
			GetPortBitMapForCopyBits(eWinRec->currentMask),
			&picSize,&picSize,srcCopy,nil);
	UnlockPixels(GetGWorldPixMap(eWinRec->currentMask));
	
	/* 読み込んだイメージを破棄 */
	DisposeGWorld(theImage);
	DisposeGWorld(maskImage);
	
	SetGWorld(cPort,cDevice);
	
	gUsedCount.openNum++;
	
	DispOffPort(theWindow);
	UpdateMenus();
	
	return noErr;
}

static Boolean	gAvailable;
static short	gLayerNum;

/* バックグラウンド情報が存在するかどうかをチェック */
Boolean IsBackAvailable(PicHandle picture,short *layerNum)
{
	GrafPtr		curPort;
	QDProcs		theQDProcs;		/* If we're using a GrafPort			*/
	CQDProcs	theCQDProcs;	/* If we're using a CGrafPort			*/
	PicHandle	dummyPICT;
	QDProcsPtr	tempProcs;
	PictInfo	pictInfo;
	OSErr		err;
	
	gAvailable=false;
	
	SetPortWindowPort(gPreviewWindow);
	GetPort(&curPort);
	
	/* まずは、コメントにレイヤ情報があるかどうかをチェック */
	tempProcs=(QDProcsPtr)GetPortGrafProcs(curPort);
	
	if (IsPortColor(curPort))				/* CGrafPort		*/
	{
		SetStdCProcs(&theCQDProcs);
		theCQDProcs.commentProc = NewQDCommentUPP(MyGetLayerInfoProc);
		SetPortGrafProcs(curPort,&theCQDProcs);
	}
	else											/* GrafPort		*/
	{
		SetStdProcs(&theQDProcs);
		theQDProcs.commentProc = NewQDCommentUPP(MyGetLayerInfoProc);
		SetPortGrafProcs(curPort,(CQDProcs *)&theQDProcs);
	}
	
	dummyPICT = OpenPicture(&(*picture)->picFrame);
	DrawPicture(picture, &(*picture)->picFrame);
	ClosePicture();
	KillPicture(dummyPICT);
	
	SetPortGrafProcs(curPort,(CQDProcs *)tempProcs);
	
	if (gAvailable)
	{
		*layerNum=gLayerNum;
		return gAvailable;
	}
	
	/* ない場合には、pictureの情報からビットマップの数を得る */
	err=GetPictInfo(picture,&pictInfo,0,0,0,0);
	if (err==noErr && pictInfo.bitMapCount+pictInfo.pixMapCount == 2 && 
		pictInfo.textCount+pictInfo.lineCount+pictInfo.rectCount+pictInfo.rRectCount+pictInfo.ovalCount+
		pictInfo.arcCount+pictInfo.polyCount+pictInfo.regionCount == 0)
	{
		gAvailable=true;
		*layerNum=2;
	}
	
	return gAvailable;
}

pascal void MyGetLayerInfoProc(short kind, short dataSize, Handle dataHandle)
{
	OSType		ownerApp;
	short		localPicComment;
	short		layerNum;
	
	if (kind==kCustomComment)
	{
		if (dataSize != 8) return;						/* Not ours?	*/
		
		BlockMoveData((Ptr) *dataHandle, &ownerApp, sizeof(ownerApp));
		BlockMoveData((Ptr) *dataHandle +sizeof(ownerApp), &localPicComment, sizeof(localPicComment));
		BlockMoveData((Ptr) *dataHandle +sizeof(ownerApp) +sizeof(localPicComment), &layerNum,sizeof(layerNum));
		
		if ((ownerApp != kIconPartyCreator) ||				/* Not ours?	*/
		    (localPicComment != kSubLayerInfo)) return;
		
		if (layerNum < 1 || layerNum > 2) return;
		gAvailable=true;
		gLayerNum=layerNum;
	}
}

static PaintWinRec	*gEWinRec;
static short		gLayerCount;
static short		gLayerTrans;

/* PICTを分割してそれぞれに保存 */
void SpritPictAndLoad(PicHandle picture,PaintWinRec *eWinRec)
{
	GrafPtr		curPort;
	QDProcs		theQDProcs;
	CQDProcs	theCQDProcs;
	PicHandle	dummyPICT;
	QDProcsPtr	tempProcs;
	
	/* グローバルを初期化 */
	gEWinRec=eWinRec;
	if (eWinRec->isBackTransparent)
		gLayerCount=1;
	else
		gLayerCount=0;
	gLayerTrans=kForeTrans100;
	
	SetPortWindowPort(gPreviewWindow);
	GetPort(&curPort);
	
	/* bitsProcを置き換えてCopyBitsを横取りする */
	//tempProcs=curPort->grafProcs;
	tempProcs=(QDProcsPtr)GetPortGrafProcs(curPort);
	
	if (IsPortColor(curPort))
	{
		SetStdCProcs(&theCQDProcs);
		theCQDProcs.bitsProc=NewQDBitsUPP(MyGetLayerDataProc);
		//curPort->grafProcs=(QDProcsPtr)&theCQDProcs;
		SetPortGrafProcs(curPort,&theCQDProcs);
	}
	else
	{
		SetStdProcs(&theQDProcs);
		theQDProcs.bitsProc=NewQDBitsUPP(MyGetLayerDataProc);
		//curPort->grafProcs=(QDProcsPtr)&theQDProcs;
		SetPortGrafProcs(curPort,(CQDProcs *)&theQDProcs);
	}
	
	dummyPICT=OpenPicture(&(*picture)->picFrame);
	DrawPicture(picture,&(*picture)->picFrame);
	ClosePicture();
	KillPicture(dummyPICT);
	
	//curPort->grafProcs=tempProcs;
	SetPortGrafProcs(curPort,(CQDProcs *)tempProcs);
	
	eWinRec->foreTransparency=gLayerTrans;
}

pascal void MyGetLayerDataProc(const BitMap *bitPtr,const Rect *srcRect,const Rect *dstRect,short mode,RgnHandle maskRgn)
{
	OSErr	err;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	/* 現在のグラフポートを保存 */
	GetGWorld(&cPort,&cDevice);
	
	switch (gLayerCount)
	{
		case 0:
			/* 背景を作成 */
			err=NewGWorld(&gEWinRec->backgroundGWorld,gEWinRec->iconDepth,&gEWinRec->iconSize,0,0,useTempMem);
			SetGWorld(gEWinRec->backgroundGWorld,0);
			LockPixels(GetGWorldPixMap(gEWinRec->backgroundGWorld));
			EraseRect(&gEWinRec->iconSize);
			UnlockPixels(GetGWorldPixMap(gEWinRec->backgroundGWorld));
			
			/* 描画 */
			CopyBits(bitPtr,GetPortBitMapForCopyBits(gEWinRec->backgroundGWorld),srcRect,dstRect,mode,maskRgn);
			break;
		
		case 1:
			/* 描画 */
			SetGWorld(gEWinRec->editDataPtr,0);
			CopyBits(bitPtr,GetPortBitMapForCopyBits(gEWinRec->editDataPtr),srcRect,dstRect,srcCopy,maskRgn);
			
			/* 透明度を調べる */
			if (mode==blend)
			{
				if (IsPortColor(cPort)) /* カラー */
				{
					RGBColor	opColor;
					
					GetPortOpColor(cPort,&opColor);
					
					gLayerTrans=kForeTrans0-opColor.red/0x4000;
				}
			}
			
			/* マスクを作成 */
			SetGWorld(gEWinRec->currentMask,0);
			LockPixels(GetGWorldPixMap(gEWinRec->currentMask));
			PaintRgn(maskRgn);
			UnlockPixels(GetGWorldPixMap(gEWinRec->currentMask));
			break;
	}
	
	SetGWorld(cPort,cDevice);
	
	gLayerCount++;
}
			
#define	kSaveChangesString	133

/* 変更を保存しますか？ */
short SaveYN(Str255 docuKind,Str255 docuName,short dialogFlag)
{
	DialogPtr	dp;
	short	item=4;
	ModalFilterUPP	upp;
	WindowPtr	theWindow=MyFrontNonFloatingWindow();
	Str15	beforeString;
	
	MySetCursor(0);
	upp=NewModalFilterUPP(CmdDFilter);
	
	if (dialogFlag==kSaveChangedCloseWindow)
		GetIndString(beforeString,kSaveChangesString,1);
	else
		GetIndString(beforeString,kSaveChangesString,2);
	
	DeactivateFloatersAndFirstDocumentWindow();
	ParamText(beforeString,docuKind,docuName,"\p");
	dp=GetNewDialog(129,nil,theWindow);
	SetDialogDefaultItem(dp,ok);
	SetDialogCancelItem(dp,cancel);
	
	if (dialogFlag==kSaveChangedCloseWindow)
	{
		SetPortWindowPort(MyFrontNonFloatingWindow());
		MoveDialogToParentWindow(dp);
	}
	else
	{
		SelectWindow(GetDialogWindow(dp));
		ShowWindow(GetDialogWindow(dp));
	}
	
	while(item < ok || item > kDialogItemDontSave)
	{
		ModalDialog(upp,&item);
	}
	DisposeDialog(dp);
	//DisposeRoutineDescriptor(upp);
	DisposeModalFilterUPP(upp);
	ActivateFloatersAndFirstDocumentWindow();
	
	return item;
}

/* 保存しますかダイアログのフィルタ */
pascal Boolean CmdDFilter(DialogPtr dp,EventRecord *theEvent,short *item)
{
	WindowPtr	theWindow;
	short		part;
	Boolean		eventHandled=false;
	char		key;
	
	switch (theEvent->what)
	{
		case keyDown:
			key=(char)(theEvent->message & charCodeMask);
			if (theEvent->message & cmdKey)
				if (key==kPeriod)
					key=kEscapeKey;
			
			switch (key)
			{
				case kReturnKey:
				case kEnterKey:
					*item=ok;
					HiliteButton(dp,ok);
					eventHandled=true;
					break;
				
				case kEscapeKey:
					*item=cancel;
					HiliteButton(dp,cancel);
					eventHandled=true;
					break;
				
				case 'd': /* cmd+D */
					*item=kDialogItemDontSave;
					HiliteButton(dp,kDialogItemDontSave);
					eventHandled=true;
					break;
			}
			break;
		
		case updateEvt:
			theWindow=(WindowPtr)theEvent->message;
			if (theWindow!=nil && theWindow!=GetDialogWindow(dp))
				DoUpdate(theEvent);
			break;
		
		case mouseDown:
			part=FindWindow(theEvent->where,&theWindow);
			if (part==inDrag && theWindow == GetDialogWindow(dp))
			{
				Rect	myScreenRect;
				
				GetRegionBounds(GetGrayRgn(),&myScreenRect);
				DragWindow(theWindow,theEvent->where,&myScreenRect);
				eventHandled=true;
			}
			break;
	}
	
	return eventHandled;
}

/* 終了 */
void Quit(void)
{
	WindowPtr	theWindow;
	short	err=noErr;
	
	while ((theWindow=MyFrontNonFloatingWindow())!=nil && err==noErr)
	{
		switch(GetExtWindowKind(theWindow))
		{
			case kWindowTypePaintWindow:
				err=ClosePaintWindow(theWindow,true);
				break;
			
			case kWindowTypeIconListWindow:
				err=CloseIconFile(theWindow,true);
				break;
			
			case kWindowTypeIconFamilyWindow:
				err=CloseFamilyWindow(theWindow,true,false);
				break;
		}
	}
	
	if (err==noErr)
		quit=true;
	else
		UpdateMenus();
}

/* クリップボードに入っている'PICT'を開く */
void OpenClipboard(void)
{
	WindowPtr	theWindow;
	PaintWinRec	*eWinRec;
	PicHandle	picture;
	Rect		picSize;
	short	ratio=gPaintWinPrefs.ratio;
	short	colorMode;
	short	maxDepth;
	long	dataSize;
	OSErr	err;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	#if TARGET_API_MAC_CARBON
	ScrapRef	scrap;
	{
		err=GetCurrentScrap(&scrap);
		if (err==noErr)
			err=GetScrapFlavorSize(scrap,'PICT',&dataSize);
		else
			dataSize=0;
	}
	if (err!=noErr)
	{
		SysBeep(0);
		return;
	}
	#else
	long	offset;
	
	dataSize=GetScrap(0,'PICT',&offset);
	#endif
	if (dataSize==0) return;
	
	GetGWorld(&cPort,&cDevice);
	
	picture=(PicHandle)TempNewHandle(dataSize,&err);
	if (err!=noErr)
	{
		ErrorAlertFromResource(FILEERR_RESID,FILEERR1);
		return;
	}
	
	#if TARGET_API_MAC_CARBON
	err=GetScrapFlavorData(scrap,'PICT',&dataSize,*picture);
	#else
	dataSize=GetScrap((Handle)picture,'PICT',&offset);
	#endif
	
	picSize.left=0;
	picSize.top=0;
	picSize.right=(**picture).picFrame.right-(**picture).picFrame.left;
	picSize.bottom=(**picture).picFrame.bottom-(**picture).picFrame.top;
			
	/* 色数をチェック */
	err=GetPictureMaxDepth(picture,&maxDepth);
	if (err==noErr)
		colorMode = (maxDepth <= 8 ? kNormal8BitColorMode : k32BitColorMode);
	else
		colorMode=gPaintWinPrefs.colorMode;
	
	theWindow=MakePaintWindow(&picSize,ratio,colorMode);
	if (theWindow==nil)
	{
		/* ウィンドウを開けなかった時は読み込んだPICTを破棄 */
		TempDisposeHandle((Handle)picture,&err);
		ErrorAlertFromResource(FILEERR_RESID,FILEERR1);
		return;
	}
	eWinRec=GetPaintWinRec(theWindow);
	eWinRec->iconSize=picSize;
	eWinRec->iconType.fileType='PICT';
	eWinRec->iconType.creatorCode=gPICTCreator;
	
	eWinRec->iconHasSaved=false;
	eWinRec->iconHasChanged=false;
	eWinRec->undoMode=umCannot;
	MakeUntitledFileName('PICT',eWinRec->saveFileSpec.name);
	
	SetEditWindowTitle(theWindow);
	
	/* 読み込んだ絵をオフポートに表示 */
	SetGWorld(eWinRec->editDataPtr,0);
	LockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
	DrawPicture(picture,&picSize);
	UnlockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
	
	TempDisposeHandle((Handle)picture,&err);
	
	SetGWorld(eWinRec->currentMask,0);
	LockPixels(GetGWorldPixMap(eWinRec->currentMask));
	PaintRect(&picSize);
	UnlockPixels(GetGWorldPixMap(eWinRec->currentMask));
	
	SetGWorld(cPort,cDevice);
	
	DispOffPort(theWindow);
	UpdateMenus();
}

/* ペイントウィンドウの復帰 */
void DoRevertPaintWindow(WindowPtr theWindow)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	short		item;
	Str255		docKind;
	FSSpec		theFile;
	OSErr		err;
	FInfo		fInfo;
	WindowPtr	fWindow;
	IconFamilyWinRec	*fWinRec;
	short		iconKind,selectedIcon;
	
	/* まず、確認 */
	GetIndString(docKind,133,3);
	item=RevertYN(docKind,eWinRec->saveFileSpec.name);
	if (item!=ok) return;
	
	/* 種類によって場合わけ */
	switch (eWinRec->iconType.fileType)
	{
		case 'PICT':
			theFile=eWinRec->saveFileSpec;
			err=FSpGetFInfo(&theFile,&fInfo);
			if (err!=noErr) /* ファイルが存在しないなどエラーが起こった場合はなにもしない */
			{
				SysBeep(0);
				break;
			}
			/* 変更フラグをリセットし、閉じる */
			eWinRec->iconHasChanged=false;
			err=ClosePaintWindow(theWindow,false);
			if (err!=noErr)
			{
				eWinRec->iconHasChanged=true;
				break;
			}
			
			err=OpenPictFile(&theFile);
			break;
		
		case kGIFFileType:
		case 'Icon':
		case 'wIco':
			SysBeep(0);
			break;
		
		case 'icns':
			fWindow=eWinRec->parentWindow;
			iconKind=eWinRec->iconKind;
			
			/* 変更フラグをリセットし、閉じる */
			eWinRec->iconHasChanged=false;
			err=ClosePaintWindow(theWindow,false);
			if (err!=noErr)
			{
				eWinRec->iconHasChanged=true;
				break;
			}
			
			/* ファミリウィンドウの選択中アイコンの情報を保存し、変更して開き直す */
			fWinRec=GetIconFamilyRec(fWindow);
			selectedIcon=fWinRec->selectedIcon;
			fWinRec->selectedIcon=iconKind;
			EditFamilyIcon(fWindow,kForceNone);
			break;
	}
			
}

/* 復帰確認 */
short RevertYN(Str255 docKind,Str255 docName)
{
	short	item;
	
	if (isNavServicesAvailable && useNavigationServices)
	{
		SuspendFloatingWindows();
		item=AskRevertWithNav(docName);
		ResumeFloatingWindows();
	}
	else
	{
		DialogPtr	dp;
		ModalFilterUPP	upp;
		WindowPtr	theWindow=MyFrontNonFloatingWindow();
		
		MySetCursor(0);
		upp=NewModalFilterUPP(MyModalDialogFilter);
		
		DeactivateFloatersAndFirstDocumentWindow();
		ParamText(docKind,docName,"\p","\p");
		dp=GetNewDialog(137,nil,theWindow);
		SetDialogDefaultItem(dp,ok);
		SetDialogCancelItem(dp,cancel);
		
		SetPortWindowPort(MyFrontNonFloatingWindow());
		MoveDialogToParentWindow(dp);
		
		item=3;
		while (item!=ok && item!=cancel)
			ModalDialog(upp,&item);
		
		DisposeDialog(dp);
		DisposeModalFilterUPP(upp);
		ActivateFloatersAndFirstDocumentWindow();
	}
	return item;
}

/* ウィンドウのタイトルを設定 */
void SetEditWindowTitle(WindowPtr theWindow)
{
	PaintWinRec	*eWinRec;
	Str255	title,temp;
	
	eWinRec=GetPaintWinRec(theWindow);
	
	switch (eWinRec->iconType.fileType)
	{
		case 'icns':
			GetIndString(title,130,eWinRec->iconKind+1+1);
			GetIndString(temp,130,1);
			PStrCat(temp,title);
			PStrCat(eWinRec->saveFileSpec.name,title);
			SetWTitle(theWindow,title);
			break;
		
		default:
			GetIndString(title,135,1);
			ReplaceString(title,eWinRec->saveFileSpec.name,"\p^0");
			NumToString(eWinRec->iconDepth,temp);
			ReplaceString(title,temp,"\p^1");
			SetWTitle(theWindow,title);
			break;
	}
}

/* アプリケーションが開かれたとき */
short HandleOpenApp(void)
{
	switch (gStartup+1) /* 初期設定によってなにをするかを変更 */
	{
		case iStartupNone:
			break;
		
		case iStartupNewPaintWindow:
			NewPaintWindow();
			break;
		
		case iStartupOpenDialog:
			OpenIcon();
			break;
		
		default:
			gStartup=0;
			break;
	}
	return noErr;
}

/* ドキュメントを指定して開かれたとき */
short HandleOpenDoc(FSSpec *spec)
{
	OSErr	err;
	WindowPtr	theWindow;
	PaintWinRec	*eWinRec;
	Rect		picSize;
	Boolean	wasAliased;
	KeyMap	theKeys;
	Boolean	cmdDown;
	OSType	fileType;
	#ifdef __MOREFILESX__
	FSRef	fsRef;
	FinderInfo	info;
	Boolean	isDirectory;
	#else
	FInfo	fndrInfo;
	DInfo	dirInfo;
	#endif
	
	GetKeys(theKeys);
	cmdDown=BitTst(theKeys,48);
	
	/* エイリアスを開いているかも知れないので、対応させる */
	err=ResolveAliasFileWithNoUI(spec,true,&wasAliased);
	if (err!=noErr)
	{
		ErrorAlertFromResource(FILEERR_RESID,FILEERR12);
		return err;
	}
	
	#ifdef __MOREFILESX__
	err = FSpMakeFSRef(spec,&fsRef);
	err = FSGetFinderInfo(&fsRef,&info,NULL,&isDirectory);
	if (err == noErr)
	{
		if (isDirectory)
		{
			OpenFolderIcon(spec,true);
			return noErr;
		}
	}
	else return err;
	#else
	err=FSpGetFInfo(spec,&fndrInfo);
	if (err!=noErr)
	{
		/* フォルダかも */
		err=FSpGetDInfo(spec,&dirInfo);
		if (err==noErr)
		{
			OpenFolderIcon(spec,true);
			return noErr;
		}
		else return err;
	}
	#endif
	
	if (cmdDown)
	{
		OpenFolderIcon(spec,false);
		
		return noErr;
	}
	
	/* Xのファイルへの対応 */
	#ifdef __MOREFILESX__
	fileType = info.file.fileType;
	#else
	fileType = fndrInfo.fdType;
	#endif
	if (fileType == 0 || fileType == 'TEXT')
	{
		err=GetFileTypeFromSuffix(spec->name,&fileType);
	}
	
	switch (fileType)
	{
		case kPICTFileType:
			err=OpenPictFile(spec);
			break;
		
		case kPNGFileType:
			err=OpenPNGFile(spec);
			break;
		
		case kXIconFileType:
			OpenXIconFile(spec);
			break;
		
		case kQTFileTypeJPEG:
		case kGIFFileType:
		case kQTFileTypePhotoShop:
		case kQTFileTypeBMP:
		case kQTFileTypeMacPaint:
		case kQTFileTypeSGIImage:
		case kQTFileTypeTargaImage:
		case 'BMP ':
		case 'GIF ':
		case 'PNG ':
			if (IsFileOpened(spec)) return noErr;
			
			/* QuickTime3のAPIを使って画像を読み込む */
			if (gQTVersion>=0x03000000)
			{
				short	refNum;
				long	size;
				ComponentInstance	gi;
				ComponentResult		result;
				ImageDescriptionHandle	desc;
				GWorldPtr	cPort;
				GDHandle	cDevice;
				short		depth,colorMode;
				
				err=GetGraphicsImporterForFile(spec,&gi);
				if (err!=noErr) break;
				
			//	result=GraphicsImportGetSourceRect(gi,&picSize);
				result=GraphicsImportGetImageDescription(gi,&desc);
				result=GraphicsImportGetNaturalBounds(gi,&picSize);
				depth = (**desc).depth;
				if (depth > 8)
					colorMode = k32BitColorMode;
				else
					colorMode = kNormal8BitColorMode;
				
				err=FSpOpenDF(spec,fsRdPerm,&refNum);
				if (err!=noErr) break;
				
				err=GetEOF(refNum,&size);
				if (err!=noErr) break;
				
				FSClose(refNum);
				
				theWindow=MakePaintWindow(&picSize,gPaintWinPrefs.ratio,colorMode);
				if (theWindow == nil)
				{
					DisposeHandle((Handle)desc);
					err=CloseComponent(gi);
					err=memFullErr;
					ErrorAlertFromResource(FILEERR_RESID,FILEERR11);
					break;
				}
				eWinRec=GetPaintWinRec(theWindow);
				
				GetGWorld(&cPort,&cDevice);
				SetGWorld(eWinRec->editDataPtr,0);
				
				LockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
				EraseRect(&eWinRec->iconSize);
				err=(OSErr)GraphicsImportSetGWorld(gi,eWinRec->editDataPtr,0);
				err+=(OSErr)GraphicsImportDraw(gi);
				UnlockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
				
				DisposeHandle((Handle)desc);
				err+=CloseComponent(gi);
				
				SetGWorld(cPort,cDevice);
				
				if (err!=noErr)
				{
					ClosePaintWindow(theWindow,false);
					break;
				}
				
				eWinRec->iconSize=picSize;
				#if 0
				if (fndrInfo.fdType == kGIFFileType)
				{
					eWinRec->iconType.fileType=kGIFFileType;
					eWinRec->iconType.creatorCode=gGIFCreator;
				}
				else
				#endif
				{
					eWinRec->iconType.fileType='PICT';
					eWinRec->iconType.creatorCode=gPICTCreator;
				}
				eWinRec->iconHasSaved=false;
				eWinRec->iconHasChanged=false;
				eWinRec->undoMode=umCannot;
				eWinRec->saveFileSpec=*spec;
				
				SetEditWindowTitle(theWindow);
				
				#if powerc
				if (gSystemVersion >= 0x0850)
				{
					err=SetWindowProxyFSSpec(theWindow,spec);
					err=SetWindowModified(theWindow,false);
				}
				#endif
				
				RectRgn(eWinRec->updateRgn,&picSize);
				SetGWorld(eWinRec->currentMask,0);
				LockPixels(GetGWorldPixMap(eWinRec->currentMask));
				PaintRect(&picSize);
				UnlockPixels(GetGWorldPixMap(eWinRec->currentMask));
				SetGWorld(cPort,cDevice);
				DispOffPort(theWindow);
				
				gUsedCount.openNum++;
				
				UpdateMenus();
			}
			else
			{
				/* Translate Managerが使用可能ならトランスレート、、、どうやるのかしら */
				FileTranslationSpec	howToTranslate;
				DocOpenMethod	howToOpen;
				FileType	nativeTypes[64]={'PICT',0L};
				long	response;
				
				err=Gestalt(gestaltTranslationAttr,&response);
				
				if (err != noErr || (response & (1L<<gestaltTranslationMgrExists))==0) return err;
				
				err=CanDocBeOpened(spec,kOnSystemDisk,kIconPartyCreator,nativeTypes,false,&howToOpen,&howToTranslate);
				if (err==noPrefAppErr)
				{
					ProcessInfoRec	pInfo;
					ProcessSerialNumber	psn;
					FSSpec			appSpec;
					
					pInfo.processInfoLength=sizeof(ProcessInfoRec);
					pInfo.processName=nil;
					pInfo.processAppSpec=&appSpec;
					
					err=GetCurrentProcess(&psn);
					err=GetProcessInformation(&psn,&pInfo);
					
					err=GetPathFromTranslationDialog(spec,pInfo.processAppSpec,nativeTypes,&howToOpen,&howToTranslate);
					
					if ((err == noErr) && (howToOpen == domTranslateFirst))
					{
						// Translate the file
//						err = TranslateFile(spec,
//						spec,
//						&howToTranslate);
					}
				}
			}
			break;
		
		case kResourceFileType:
			LoadFromIconFile(spec);
			
			UpdateMenus();
			break;
		
		case kTemporaryFileType:
			/* テンポラリファイル */
			return -1;
			break;
		
		case kWinIconFileType:
			OpenWinIconFile(spec);
			break;
		
		case kFolderType:
			/* フォルダ */
			OpenFolderIcon(spec,true);
			break;
		
		case kPreferencesFileType:
			/* 初期設定ファイル */
			OpenPrefsDialog();
			break;
		
		case kDotModeLibFileType:
			/* 描画点ライブラリ */
			if (spec->vRefNum == gDotLibVRefNum && spec->parID == gDotLibFolderID)
				SetDotLibMain(spec);
			break;
		
		case kBlendPaletteFileType:
			/* ブレンドパレット */
			LoadBlendPalette(spec);
			break;
		
		default:
			LoadFromIconFile(spec);
			
			UpdateMenus();
			break;
			
	}
	
	return noErr;
}

/* 画像ファイルがすでに開かれているかどうかをチェック */
Boolean IsFileOpened(FSSpec *theFile)
{
	WindowPtr	theWindow=MyFrontNonFloatingWindow();
	PaintWinRec	*eWinRec;
	
	/* 同じファイルがすでに開かれているかどうかをチェック */
	while (theWindow!=nil)
	{
		if (GetExtWindowKind(theWindow)==kWindowTypePaintWindow)
		{
			eWinRec=GetPaintWinRec(theWindow);
			if (eWinRec->parentWindow == nil && EqualFile(&eWinRec->saveFileSpec,theFile))
			{
				SelectReferencedWindow(theWindow);
				UpdateMenus();
				return true;
			}
		}
		theWindow=GetNextVisibleWindow(theWindow);
	}
	return false;
}

/* テンポラリファイルを作成する */
OSErr MakeTempFile(FSSpec *theTempFile,Boolean resourceFork)
{
	OSErr	err;
	short	vRefNum;
	long	dirID;
	Str255	tempFileName,str;
	
	/* テンポラリアイテムフォルダ */
	err=FindFolder(GetFindFolderVRefNum(),kTemporaryFolderType,true,&vRefNum,&dirID);
	if (err!=noErr) return err;
	
	/* テンポラリファイルの名前を決定する */
	GetIndString(tempFileName,144,1);
	NumToString(gTempFileNum,str);
	PStrCat(str,tempFileName);
	
	/* テンポラリファイルのFSSpecを作成 */
	err=FSMakeFSSpec(vRefNum,dirID,tempFileName,theTempFile);
	if (err==noErr)
	{
		/* すでに存在するなら削除 */
		err=FSpDelete(theTempFile);
	}
	if (err==fnfErr || err==noErr)
	{
		if (resourceFork)
			FSpCreateResFile(theTempFile,kIconPartyCreator,kTemporaryFileType,smSystemScript);
		else
			err=FSpCreate(theTempFile,kIconPartyCreator,kPICTFileType,smSystemScript);
		gTempFileNum++;
		err=noErr;
	}
	
	return err;
}

/* ファイルが一致するかどうか */
Boolean EqualFile(FSSpec *spec1,FSSpec *spec2)
{
	return ((spec1->vRefNum == spec2->vRefNum) && (spec1->parID == spec2->parID) &&
			(EqualString(spec1->name,spec2->name,false,true)));
}

/* 編集ウィンドウに関するDrag&Dropルーチン */
/* ドラッグをトラックする */
pascal short MyPaintWinTrackingHandler(DragTrackingMessage theMessage,WindowPtr theWindow,
										void *handlerRefCon,DragReference theDrag)
{
	MyDragGlobalsRec	*myGlobals=(MyDragGlobalsRec *)handlerRefCon;
	Point		mouse,localMouse;
	DragAttributes	attributes;
	RgnHandle	hiliteRgn;
	Rect		hiliteRect;
	
	GetDragAttributes(theDrag,&attributes);
	
	switch (theMessage) {
		case kDragTrackingEnterHandler:
			break;
		
		case kDragTrackingEnterWindow:
			myGlobals->isInContent=true;
			
			if (!(attributes & kDragInsideSenderWindow) && IsMyPaintWinTypeAvailable(theDrag))
			{
				myGlobals->canAcceptDrag=true;
				
				hiliteRgn=NewRgn();
				GetWindowPortBounds(theWindow,&hiliteRect);
				//hiliteRect=theWindow->portRect;
				hiliteRect.right-=kScrollBarWidth;
				hiliteRect.bottom-=kScrollBarHeight;
				RectRgn(hiliteRgn,&hiliteRect);
				ShowDragHilite(theDrag,hiliteRgn,true); /* 内側 */
				DisposeRgn(hiliteRgn);
			}
			break;
		
		case kDragTrackingInWindow:
			if (!myGlobals->canAcceptDrag)
				break;
			
			GetDragMouse(theDrag,&mouse,0L);
			localMouse=mouse;
			GlobalToLocal(&localMouse);
			
			break;
		
		case kDragTrackingLeaveWindow:
			if (myGlobals->canAcceptDrag && myGlobals->isInContent) {
				HideDragHilite(theDrag);
			}
			myGlobals->canAcceptDrag=false;
			break;
		
		case kDragTrackingLeaveHandler:
			break;
	}
	
	return (noErr);
}

/* なにがドラッグされているかをチェックする */
Boolean IsMyPaintWinTypeAvailable(DragReference theDrag)
{
	unsigned short	index;
	unsigned short	items;
	FlavorFlags		theFlags;
	ItemReference	theItem;
	OSErr			result;
	
	CountDragItems(theDrag,&items);
	
	if (items > 1) return false;
	
	for (index=1; index<=items; index++) {
		GetDragItemReferenceNumber(theDrag,index,&theItem);
		
		/* 'PICT' flavorの存在をチェック */
		result=GetFlavorFlags(theDrag,theItem,'PICT',&theFlags);
		if (result==noErr)
			continue;
		
		/* 'hfs ' flavorの存在をチェック */
		result=GetFlavorFlags(theDrag,theItem,'hfs ',&theFlags);
		if (result==noErr)
			continue;
		
		return false;
	}
	
	return true;
}

/* ドラッグを受け入れる */
pascal short MyPaintWinReceiveHandler(WindowPtr theWindow,void *handlerRefCon,
										DragReference theDrag)
{
	MyDragGlobalsRec	*myGlobals=(MyDragGlobalsRec *)handlerRefCon;
	Point			mouse;
	unsigned short	index;
	unsigned short	items;
	ItemReference	theItem;
	FlavorFlags		theFlags;
	Size			dataSize;
	PicHandle		dragPic;
	HFSFlavor		hfsFlavorData;
	OSErr			result;
	GrafPtr			port;
	
	if (!myGlobals->canAcceptDrag) return -2;
	myGlobals->isInContent=false;
	myGlobals->canAcceptDrag=false;
	
	GetDragMouse(theDrag,&mouse,0L);
	HideDragHilite(theDrag);
	
	CountDragItems(theDrag,&items);
	
	for (index=1; index<=items; index++)
	{
		GetDragItemReferenceNumber(theDrag,index,&theItem);
		
		result=GetFlavorFlags(theDrag,theItem,'PICT',&theFlags);
		if (result==noErr) /* 'PICT' */
		{
			PaintWinRec	*eWinRec;
			
			GetFlavorDataSize(theDrag,theItem,'PICT',&dataSize);
			dragPic=(PicHandle)NewHandle(dataSize);
			if (dragPic==nil)
			{
				/* メモリが足りない */
				return memFullErr;
			}
			
			HLock((Handle)dragPic);
			GetFlavorData(theDrag,theItem,'PICT',(char *)*dragPic,&dataSize,0L);
			HUnlock((Handle)dragPic);
			
			GetPort(&port);
			SetPortWindowPort(theWindow);
			
			/* PICTをウィンドウに置く */
			result=PutPictureIntoEditWin(dragPic,theWindow);
			
			if (result == noErr)
			{
				if (MyFrontNonFloatingWindow() == theWindow)
					SetUndoMode(umPaste);
				else
				{
					eWinRec=GetPaintWinRec(theWindow);
					eWinRec->undoMode=umPaste;
				}
				DispOffPort(theWindow);
			}
			else
			{
				if (MyFrontNonFloatingWindow() == theWindow)
					SetUndoMode(umCannot);
				else
				{
					eWinRec=GetPaintWinRec(theWindow);
					eWinRec->undoMode=umCannot;
				}
			}
			SetPort(port);
			
			DisposeHandle((Handle)dragPic);
			
			if (gOtherPrefs.activateDroppedWindow)
			{
				SelectReferencedWindow(theWindow);
				UpdateMenus();
			}
		}
		else
		{
			result=GetFlavorFlags(theDrag,theItem,'hfs ',&theFlags);
			if (result==noErr) /* 'hfs ' */
			{
				IconSuiteRef	iconSuite;
				OSErr			err;
				FInfo			fndrInfo;
				Str15			suffix;
				PaintWinRec		*eWinRec;
				
				GetFlavorDataSize(theDrag,theItem,'hfs ',&dataSize);
				GetFlavorData(theDrag,theItem,'hfs ',(char *)&hfsFlavorData,&dataSize,0L);
				
				GetSuffix(hfsFlavorData.fileSpec.name,suffix);
				
				err=FSpGetFInfo(&hfsFlavorData.fileSpec,&fndrInfo);
				if (fndrInfo.fdType == kWinIconFileType || EqualString(suffix,"\p.ico",false,true))
				{
					err=WinIconToIconSuite(&hfsFlavorData.fileSpec,&iconSuite);
					if (err!=noErr) continue;
				}
				else if (fndrInfo.fdType == kXIconFileType || EqualString(suffix,"\p.icns",false,true))
				{
					err=XIconToIconSuite(&hfsFlavorData.fileSpec,&iconSuite);
					if (err!=noErr) continue;
				}
				else
				{
					/* ファイルのアイコンを取得して表示 */
					iconSuite=GetFileIconSuite(&hfsFlavorData.fileSpec);
					if (iconSuite==nil) continue;
				}
				
				dragPic=IconToPicture(iconSuite,0);
				if (dragPic==nil)
				{
					dragPic=IconToPicture(iconSuite,1);
					if (dragPic==nil)
					{
						dragPic=IconToPicture(iconSuite,2);
						if (dragPic==nil)
							goto exit;
					}
				}
				
				GetPort(&port);
				SetPortWindowPort(theWindow);
				
				result=PutPictureIntoEditWin(dragPic,theWindow);
				
				if (result == noErr)
				{
					if (MyFrontNonFloatingWindow() == theWindow)
						SetUndoMode(umPaste);
					else
					{
						eWinRec=GetPaintWinRec(theWindow);
						eWinRec->undoMode=umPaste;
					}
					DispOffPort(theWindow);
				}
				else
				{
					if (MyFrontNonFloatingWindow() == theWindow)
						SetUndoMode(umCannot);
					else
					{
						eWinRec=GetPaintWinRec(theWindow);
						eWinRec->undoMode=umCannot;
					}
				}
				SetPort(port);
				
				KillPicture(dragPic);
exit:
				err=DisposeIconSuite(iconSuite,true);
				
				if (gOtherPrefs.activateDroppedWindow)
				{
					SelectReferencedWindow(theWindow);
					UpdateMenus();
				}
			}
		}
	}
	return noErr;
}

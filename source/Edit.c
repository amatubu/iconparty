/* ------------------------------------------------------------ */
/*  Edit.c                                                      */
/*     編集メニュー（クリップボード）の処理                     */
/*                                                              */
/*                 1997.1.28 - 2001.1.27  naoki iimura         	*/
/* ------------------------------------------------------------ */

/* includes */
#ifdef __APPLE_CC__
#include	<Carbon/Carbon.h>
#else
#include	<QDOffscreen.h>
#include	<Sound.h>
#include	<Scrap.h>
#include	<PictUtils.h>
#endif

#include	"WindowExtensions.h"
#include	"Globals.h"
#include	"IconParty.h"
#include	"FileRoutines.h"
#include	"MenuRoutines.h"
#include	"UsefulRoutines.h"
#include	"WindowRoutines.h"
#include	"IconRoutines.h"
#include	"IconListWindow.h"
#include	"IconFamilyWindow.h"
#include	"PreCarbonSupport.h"
#include	"PaintRoutines.h"
#include	"EditRoutines.h"
#include	"ToolRoutines.h"

/* prototypes */
static void	CopySelection(WindowPtr theWindow);
static Boolean CheckPictSize(Rect *iconSize,Rect *pictRect);

static Boolean	GetPictureRgn(PicHandle picture,RgnHandle rgn);
static pascal void	CustomPicProc(short kind, short dataSize, Handle dataHandle);

static Boolean	GetPictureMask(PicHandle picture,RgnHandle maskRgn);
static pascal void	MyGetMaskProc(const BitMap *bitPtr,const Rect *srcRect,const Rect *dstRect,short mode,RgnHandle maskRgn);

static void	ExchangeRgn(RgnHandle rgn1,RgnHandle rgn2);
static void	ExchangeOffPort(PaintWinRec *eWinRec);
static void	ExchangeMask(PaintWinRec *eWinRec);


extern WindowPtr	DotModePalette;


#define	EDITERR_RESID	4011
#define	EDITERR1	1
#define	EDITERR2	2


/* カット */
void DoCut(void)
{
	WindowPtr	theWindow=MyFrontNonFloatingWindow();
	PaintWinRec	*eWinRec;
	IconListWinRec	*iWinRec;
	IconFamilyWinRec	*fWinRec;
	
	if (theWindow==nil) return;
	
	switch (GetExtWindowKind(theWindow))
	{
		case kWindowTypePaintWindow:
			eWinRec=GetPaintWinRec(theWindow);
			
			if (!eWinRec->isSelected)
			{
				SysBeep(0);
				UpdateClipMenu();
				return;
			}
			
			CopySelection(theWindow);
			
			DoDelete(false);
			SetUndoMode(umCut);
			UpdatePasteMenu();
			
			eWinRec->iconHasChanged=true;
			if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
				UpdatePaletteCheck();
			break;
		
		case kWindowTypeIconListWindow:
			iWinRec=GetIconListRec(theWindow);
			
			if (IsMultiIconSelected(iWinRec) != 1)
			{
				SysBeep(0);
				UpdateClipMenu();
				return;
			}
			
			CopySelectedIcon(iWinRec);
			DeleteSelectedIcon(iWinRec);
			
			UpdatePasteMenu();
			UpdateClipMenu();
			UpdateIconMenu();
			break;
		
		case kWindowTypeIconFamilyWindow:
			fWinRec=GetIconFamilyRec(theWindow);
			if (fWinRec->selectedIcon>=0)
			{
				CopySelectedIconPicture(fWinRec);
				DeleteSelectedIconPicture(theWindow);
			}
			else if ((**fWinRec->iconNameTE).active)
			{
				#if CALL_NOT_IN_CARBON
				long	err=ZeroScrap();
				#else
				OSErr	err=ClearCurrentScrap();
				#endif
				
				if (err == noErr)
				{
					TECut(fWinRec->iconNameTE);
					TEToScrap();
					
					fWinRec->wasChanged=true;
					UpdateSaveMenu();
					UpdateClipMenu();
					UpdatePasteMenu();
				}
			}
			break;
	}
}

/* コピー */
void DoCopy(void)
{
	WindowPtr	theWindow=MyFrontNonFloatingWindow();
	PaintWinRec	*eWinRec;
	IconListWinRec	*iWinRec;
	IconFamilyWinRec	*fWinRec;
	
	if (theWindow==nil) return;
	
	switch (GetExtWindowKind(theWindow))
	{
		case kWindowTypePaintWindow:
			eWinRec=GetPaintWinRec(theWindow);
			
			if (!eWinRec->isSelected)
			{
				SysBeep(0);
				UpdateClipMenu();
				return;
			}
			
			CopySelection(theWindow);
		
			UpdatePasteMenu();
			break;
		
		case kWindowTypeIconListWindow:
			iWinRec=GetIconListRec(theWindow);
			if (IsMultiIconSelected(iWinRec) != 1)
			{
				SysBeep(0);
				UpdateClipMenu();
				return;
			}
			
			CopySelectedIcon(iWinRec);
			
			UpdatePasteMenu();
			break;
		
		case kWindowTypeIconFamilyWindow:
			fWinRec=GetIconFamilyRec(theWindow);
			if (fWinRec->selectedIcon>=0)
				CopySelectedIconPicture(fWinRec);
			else if ((**fWinRec->iconNameTE).active)
			{
				#if CALL_NOT_IN_CARBON
				long	err=ZeroScrap();
				#else
				OSErr	err=ClearCurrentScrap();
				#endif
				
				if (err==noErr)
				{
					TECopy(fWinRec->iconNameTE);
					TEToScrap();
				}
			}
			UpdatePasteMenu();
			break;
		
		default:
			SysBeep(0);
			break;
		
	}
}

/* ペースト */
void DoPaste(void)
{
	WindowPtr	theWindow=MyFrontNonFloatingWindow();
	PaintWinRec	*eWinRec;
	IconFamilyWinRec	*fWinRec;
	long	dataSize;
	OSErr	err,tempErr;
	PicHandle	loadedPic;
	#if TARGET_API_MAC_CARBON
	ScrapRef	scrap;
	#else
	long	offset;
	#endif
	
	if (theWindow==nil) return;
	
	switch (GetExtWindowKind(theWindow))
	{
		case kWindowTypePaintWindow:
			eWinRec=GetPaintWinRec(theWindow);
			
			#if CALL_NOT_IN_CARBON
			dataSize=GetScrap(nil,kPICTFileType,&offset);
			#else
			{
				err=GetCurrentScrap(&scrap);
				if (err==noErr)
					err=GetScrapFlavorSize(scrap,kPICTFileType,&dataSize);
				else
					dataSize=0;
			}
			#endif
			if (dataSize<=0) /* クリップボードにPICTがなければエラー */
			{
				SysBeep(0);
				UpdatePasteMenu();
				return;
			}
			
			loadedPic=(PicHandle)TempNewHandle(dataSize,&err);
			if (loadedPic==0 || err!=noErr)
			{
				ErrorAlertFromResource(EDITERR_RESID,EDITERR1);
				return;
			}
			
			TempHLock((Handle)loadedPic,&err);
			#if CALL_NOT_IN_CARBON
			dataSize=GetScrap((Handle)loadedPic,kPICTFileType,&offset);
			#else
			err=GetScrapFlavorData(scrap,kPICTFileType,&dataSize,*loadedPic);
			#endif
			TempHUnlock((Handle)loadedPic,&err);
			
			err=PutPictureIntoEditWin(loadedPic,theWindow);
			
			TempDisposeHandle((Handle)loadedPic,&tempErr);
			
			if (err==noErr)
			{
				SetUndoMode(umPaste);
				DispOffPort(theWindow);
			}
			else
				SetUndoMode(umCannot);
			
			if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
				UpdatePaletteCheck();
			break;
		
		case kWindowTypeIconListWindow:
			PasteIcon(theWindow);
			break;
		
		case kWindowTypeIconFamilyWindow:
			fWinRec=GetIconFamilyRec(theWindow);
			if (fWinRec->selectedIcon>=0)
				PasteToSelectedIcon(theWindow);
			else if ((**fWinRec->iconNameTE).active)
			{
				TEFromScrap();
				TEPaste(fWinRec->iconNameTE);
				
				UpdateClipMenu();
			}
			fWinRec->wasChanged=true;
			UpdateSaveMenu();
			break;
	}
}

/* pictureを編集ウィンドウ内に表示 */
OSErr PutPictureIntoEditWin(PicHandle picture,WindowPtr theWindow)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	Rect		pictRect;
	RgnHandle	maskRgn;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	GetGWorld(&cPort,&cDevice);
	
	if (eWinRec->isSelected) /* どこかが選択されているなら、固定する */
		FixSelection(theWindow);
	
	GoOffPort(theWindow);
	SetGWorld(eWinRec->selectedDataPtr,0);
	CopyBits(GetPortBitMapForCopyBits(eWinRec->editDataPtr),
		GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
		&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
	SetGWorld(cPort,cDevice);
	
	/* ピクチャのサイズを調べる（画面内に収まるかどうか） */
//	pictRect=(**picture).picFrame;
    QDGetPictureBounds(picture, &pictRect);
	if (CheckPictSize(&eWinRec->iconSize,&pictRect))
	{
		GWorldFlags	flags;
		
		flags=UpdateGWorld(&eWinRec->editDataPtr,eWinRec->iconDepth,&pictRect,0,0,0);
		if ((flags & gwFlagErr)!=0)
		{
			ErrorAlertFromResource(EDITERR_RESID,EDITERR2);
			SetGWorld(eWinRec->editDataPtr,0);
			CopyBits(GetPortBitMapForCopyBits(eWinRec->tempDataPtr),GetPortBitMapForCopyBits(eWinRec->editDataPtr),
					&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
			SetGWorld(cPort,cDevice);
			return (OSErr)flags;
		}
		flags=UpdateGWorld(&eWinRec->currentMask,1,&pictRect,0,0,0);
	}
	
	SetGWorld(eWinRec->editDataPtr,0);
	LockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
	EraseRect(&pictRect);
	DrawPicture(picture,&pictRect);
	UnlockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
	
	SetGWorld(cPort,cDevice);
	
	SetPt(&eWinRec->prevMovement,0,0);
	SetPt(&eWinRec->selectionOffset,0,0);
	eWinRec->selectedRect=pictRect;
	
	RectRgn(eWinRec->prevSelectionPos,&eWinRec->iconSize);
	SetEmptyRgn(eWinRec->ePrevSelectedRgn);
	
	/* ピクチャのリージョンを調べる */
	if (GetPictureRgn(picture,eWinRec->selectionPos))
    {
        Rect tempRect;
        QDGetPictureBounds(picture, &tempRect);
		MapRgn(eWinRec->selectionPos,&tempRect,&pictRect);
    }
	else
		RectRgn(eWinRec->selectionPos,&pictRect);
	
	CopyRgn(eWinRec->selectionPos,eWinRec->eSelectedRgn);
	MapRgn(eWinRec->eSelectedRgn,&eWinRec->iconSize,&eWinRec->iconEditSize);
	eWinRec->isSelected=true;
	
	UpdateClipMenu();
	
	eWinRec->showSelection=true;
	UpdateSelectionMenu();
	
	ToolSelect(kMarqueeTool);
	
	/* 変更フラグ */
	eWinRec->iconHasChanged=true;
	
	/* マスク */
	SetGWorld(eWinRec->selectionMask,0);
	CopyBits(GetPortBitMapForCopyBits(eWinRec->pCurrentMask),
			GetPortBitMapForCopyBits(eWinRec->selectionMask),
			&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
	
	/* マスクがあれば得る */
	maskRgn=NewRgn();
	if (GetPictureMask(picture,maskRgn))
	{
        Rect tempRect;
        
		SetGWorld(eWinRec->currentMask,0);
		LockPixels(GetGWorldPixMap(eWinRec->currentMask));
		#if CALL_NOT_IN_CARBON
		EraseRect(&eWinRec->currentMask->portRect);
		#else
		{
			Rect	portRect;
			
			EraseRect(GetPortBounds(eWinRec->currentMask,&portRect));
		}
		#endif
        QDGetPictureBounds(picture, &tempRect);
		MapRgn(maskRgn,&tempRect,&pictRect);
		PaintRgn(maskRgn);
		UnlockPixels(GetGWorldPixMap(eWinRec->currentMask));
	}
	else
	{
		SetGWorld(eWinRec->currentMask,0);
		LockPixels(GetGWorldPixMap(eWinRec->currentMask));
		EraseRect(&eWinRec->iconSize);
		PaintRgn(eWinRec->selectionPos);
		UnlockPixels(GetGWorldPixMap(eWinRec->currentMask));
	}
	SetGWorld(cPort,cDevice);
	DisposeRgn(maskRgn);
	
	CopyRgn(eWinRec->selectionPos,eWinRec->updateRgn);
	
	return noErr;
}

/* PICTの大きさをチェック（返り値は入り切らないかどうか） */
Boolean CheckPictSize(Rect *iconSize,Rect *pictRect)
{
	Rect	temp;
	
	SectRect(iconSize,pictRect,&temp);
	if (!EqualRect(pictRect,&temp))
	{
		if (EmptyRect(&temp))
		{
			OffsetRect(pictRect,-pictRect->left,-pictRect->top);
			SectRect(iconSize,pictRect,&temp);
			if (!EqualRect(pictRect,&temp))
				return true;
			else
				return false;
		}
		else
			return true;
	}
	return false;
}

static RgnHandle	gPictRgn;

/* ピクチャからリージョンを得る */
Boolean GetPictureRgn(PicHandle picture,RgnHandle rgn)
{
	GrafPtr	curPort;
	CQDProcs	theCQDProcs;	/* If we're using a CGrafPort			*/
	PicHandle	dummyPICT;
    Rect        picRect;
	#if CALL_NOT_IN_CARBON
	QDProcs		theQDProcs;		/* If we're using a GrafPort			*/
	QDProcsPtr	tempProcs;
	#else
	CQDProcsPtr	tempProcs;
	#endif
	
	gPictRgn=nil;
	
	SetPortWindowPort(gPreviewWindow);
	GetPort(&curPort);
	
	#if CALL_NOT_IN_CARBON
	tempProcs=curPort->grafProcs;
	#else
	tempProcs=GetPortGrafProcs(curPort);
	#endif
	
	#if CALL_NOT_IN_CARBON
	if (curPort->portBits.rowBytes < 0)				/* CGrafPort		*/
	{
	#endif
		SetStdCProcs(&theCQDProcs);
		theCQDProcs.commentProc = NewQDCommentUPP(CustomPicProc);
		#if CALL_NOT_IN_CARBON
		curPort->grafProcs = (QDProcsPtr) &theCQDProcs;
		#else
		SetPortGrafProcs(curPort,&theCQDProcs);
		#endif
	#if CALL_NOT_IN_CARBON
	}
	else											/* GrafPort		*/
	{
		SetStdProcs(&theQDProcs);
		theQDProcs.commentProc = NewQDCommentUPP(CustomPicProc);
		curPort->grafProcs = (QDProcsPtr) &theQDProcs;
	}
	#endif
	
/*	Open our dummy picture and draw into it so that our PicComment
	handler is called to parse the picture.  When finished, close the
	picture, kill it and remove our grafProcs.							*/

    QDGetPictureBounds(picture, &picRect);
	dummyPICT = OpenPicture(&picRect);
	DrawPicture(picture, &picRect);
	ClosePicture();
	KillPicture(dummyPICT);
	
	if (gPictRgn != nil)
	{
		CopyRgn(gPictRgn,rgn);
		DisposeRgn(gPictRgn);
	}
	
	#if CALL_NOT_IN_CARBON
	curPort->grafProcs = tempProcs;
	#else
	SetPortGrafProcs(curPort,tempProcs);
	#endif
	
	return (gPictRgn != nil);
}

/* コメントをチェック */
pascal void CustomPicProc(short kind, short dataSize, Handle dataHandle)
{
	long		ownerApp;
	short		localPicComment;
	Handle		theHandle;
	
	if (kind==kCustomComment)
	{
		if (dataSize < 6) return;						/* Not ours?	*/
		
		BlockMoveData((Ptr) *dataHandle, &ownerApp, 4);
		BlockMoveData((Ptr) *dataHandle +4, &localPicComment, 2);

		if ((ownerApp != kIconPartyCreator) ||				/* Not ours?	*/
		    (localPicComment != kSubSelection)) return;
		
		theHandle=NewHandle(dataSize-6);
		if (theHandle!=nil)
		{
			BlockMoveData((Ptr)*dataHandle +6,*theHandle,dataSize-6);
		
			gPictRgn=(RgnHandle)theHandle;
		}
	}
}

/* ピクチャからマスクを得る */
Boolean GetPictureMask(PicHandle picture,RgnHandle maskRgn)
{
	GrafPtr		curPort;
	CQDProcs	theCQDProcs;
	PicHandle	dummyPICT;
    Rect        picRect;
	#if CALL_NOT_IN_CARBON
	QDProcs		theQDProcs;
	QDProcsPtr	tempProcs;
	#else
	CQDProcsPtr	tempProcs;
	#endif
	PictInfo	pictInfo;
	OSErr		err;
	
	/* ピクチャの情報を調べ、bitmapが１つだけの場合のみ */
	err=GetPictInfo(picture,&pictInfo,0,0,0,0);
	if (err!=noErr || pictInfo.bitMapCount+pictInfo.pixMapCount == 0 ||
		pictInfo.textCount+pictInfo.lineCount+pictInfo.rectCount+pictInfo.rRectCount+pictInfo.ovalCount+
		pictInfo.arcCount+pictInfo.polyCount+pictInfo.regionCount != 0)
	{
		return false;
	}
	
	gPictRgn=nil;
	
	GetPort(&curPort);
	/* bitsProcを置き換えてCopyBitsを引っ掛ける */
	tempProcs=GetPortGrafProcs(curPort);
	
	#if CALL_NOT_IN_CARBON
	if (curPort->portBits.rowBytes < 0) /* CGrafPtr */
	{
	#endif
		SetStdCProcs(&theCQDProcs);
		theCQDProcs.bitsProc=NewQDBitsUPP(MyGetMaskProc);
		SetPortGrafProcs(curPort,&theCQDProcs);
	#if CALL_NOT_IN_CARBON
	}
	else /* GrafPtr */
	{
		SetStdProcs(&theQDProcs);
		theQDProcs.bitsProc=NewQDBitsUPP(MyGetMaskProc);
		curPort->grafProcs=(QDProcsPtr)&theQDProcs;
	}
	#endif
	
    QDGetPictureBounds(picture, &picRect);
	dummyPICT=OpenPicture(&picRect);
	DrawPicture(picture,&picRect);
	ClosePicture();
	KillPicture(dummyPICT);
	
	SetPortGrafProcs(curPort,tempProcs);
	
	/* リージョンが見つかっていればコピー */
	if (gPictRgn != nil)
	{
		CopyRgn(gPictRgn,maskRgn);
		DisposeRgn(gPictRgn);
	}
	
	return (gPictRgn != nil);
}

/* CopyBitsからマスクを得る */
pascal void MyGetMaskProc(const BitMap *bitPtr,const Rect *srcRect,const Rect *dstRect,short mode,RgnHandle maskRgn)
{
	#pragma unused(bitPtr,srcRect,dstRect,mode)
	
	if (maskRgn != nil)
	{
		if (gPictRgn == nil)
		{
			gPictRgn=NewRgn();
			CopyRgn(maskRgn,gPictRgn);
		}
		else
			UnionRgn(gPictRgn,maskRgn,gPictRgn);
	}
	else
	{
		if (gPictRgn == nil)
		{
			gPictRgn=NewRgn();
			RectRgn(gPictRgn,dstRect);
		}
		else
		{
			RgnHandle	tempRgn=NewRgn();
			
			RectRgn(tempRgn,dstRect);
			UnionRgn(gPictRgn,tempRgn,gPictRgn);
			DisposeRgn(tempRgn);
		}
	}
}

/* 選択範囲のPICTを得る */
PicHandle GetSelectionPic(WindowPtr theWindow,Boolean getBackground)
{
	PicHandle	pic=nil;
	OpenCPicParams	picParam;
	GrafPtr	port;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	PaintWinRec *eWinRec=GetPaintWinRec(theWindow);
	Rect	selectedRect=eWinRec->selectedRect;
	RgnHandle	selectedRgn;
	Handle	dataHandle,maskInfoH;
	long	rgnSize=0;
	OSType	creator=kIconPartyCreator;
	short	commentCode;
	Boolean	isMaskAvailable=false;
	OSErr	err;
	short	foreMode=srcCopy;
	
	if (!eWinRec->isSelected && getBackground)
		isMaskAvailable=(eWinRec->backgroundGWorld != nil || eWinRec->isBackTransparent);
	
	if (eWinRec->isSelected)
	{
		selectedRgn=NewRgn();
		CopyRgn(eWinRec->selectionPos,selectedRgn);
		OffsetRgn(selectedRgn,-eWinRec->selectionOffset.h,-eWinRec->selectionOffset.v);
		
		/* コメントにリージョンのデータを入れる */
		#if TARGET_API_MAC_CARBON
		rgnSize=GetHandleSize((Handle)selectedRgn);
		#else
		rgnSize=(**selectedRgn).rgnSize;
		#endif
		err=PtrToHand(&creator,&dataHandle,sizeof(OSType));
		commentCode=kSubSelection;
		err=PtrAndHand(&commentCode,dataHandle,sizeof(short));
		err=PtrAndHand(*(Handle)selectedRgn,dataHandle,rgnSize);
		DisposeRgn(selectedRgn);
	}
	
	/* 背景があるならその情報をコメントに入れておく */
	if (isMaskAvailable)
	{
		short	layerNum=(eWinRec->backgroundGWorld != nil ? 2 : 1);
		
		err=PtrToHand(&creator,&maskInfoH,sizeof(OSType));
		commentCode=kSubLayerInfo;
		err=PtrAndHand(&commentCode,maskInfoH,sizeof(short));
		err=PtrAndHand(&layerNum,maskInfoH,sizeof(short));
	}
	
	GetPort(&port);
	GetGWorld(&cPort,&cDevice);
	
	SetGWorld(eWinRec->editDataPtr,0);
	
	picParam.srcRect=selectedRect;
#if 0
	picParam.hRes=72L<<16;
	picParam.vRes=72L<<16;
#else
    picParam.hRes=Long2Fix(72L);
    picParam.vRes=Long2Fix(72L);
#endif
	picParam.version=-2;
	
	/* 記録開始 */
	#if TARGET_API_MAC_CARBON
		#define	USE_TEMP_MEM	0
	#else
		#define	USE_TEMP_MEM	1
	#endif
	
	#if USE_TEMP_MEM
	err=TempOpenCPicture(&picParam);
	if (err!=noErr)
	{
		SetGWorld(cPort,cDevice);
		SetPort(port);
		return nil;
	}
	#else
	pic=OpenCPicture(&picParam);
	#endif
	
	if (eWinRec->backgroundGWorld != nil && getBackground)
	{
		#if 1
		CopyBits(GetPortBitMapForCopyBits(eWinRec->backgroundGWorld),
			GetPortBitMapForCopyBits(eWinRec->editDataPtr),
			&selectedRect,&selectedRect,srcCopy,nil);
		#else
		{
			/* 2つにわけてCopyBitsする */
			Rect	tempRect = selectedRect;
			
			tempRect.right--;
			CopyBits(GetPortBitMapForCopyBits(eWinRec->backgroundGWorld),
				GetPortBitMapForCopyBits(eWinRec->editDataPtr),
				&tempRect,&tempRect,srcCopy,nil);
			
			tempRect.left = tempRect.right++;
			CopyBits(GetPortBitMapForCopyBits(eWinRec->backgroundGWorld),
				GetPortBitMapForCopyBits(eWinRec->editDataPtr),
				&tempRect,&tempRect,srcCopy,nil);
		}
		#endif
		
		#if USE_TEMP_MEM
		err=CheckPictureByte();
		#endif
		
		foreMode=SetForeBlend(eWinRec);
	}
	
	if (isMaskAvailable || eWinRec->isSelected)
	{
		selectedRgn=NewRgn();
		
		/* ビットマップのマスクをリージョン化しないとPicutureに記録されない */
		err=BitMapToRegion(selectedRgn,GetPortBitMapForCopyBits(eWinRec->currentMask));
		if (err!=noErr)
		{
			/* エラーが起きた場合はとりあえず画面全体をマスクだと思って保存 */
			SysBeep(0);
			CopyRgn(eWinRec->selectionPos,selectedRgn);
		}
	}
	else
		selectedRgn=nil;
	
	#if 1
	CopyBits(GetPortBitMapForCopyBits(eWinRec->editDataPtr),
		GetPortBitMapForCopyBits(eWinRec->editDataPtr),
		&selectedRect,&selectedRect,foreMode,selectedRgn);
	#else
	{
		/* 2つにわけてCopyBitsする */
		Rect	tempRect = selectedRect;
		
		tempRect.right--;
		CopyBits(GetPortBitMapForCopyBits(eWinRec->editDataPtr),
			GetPortBitMapForCopyBits(eWinRec->editDataPtr),
			&tempRect,&tempRect,foreMode,selectedRgn);
		
		tempRect.left = tempRect.right++;
		CopyBits(GetPortBitMapForCopyBits(eWinRec->editDataPtr),
			GetPortBitMapForCopyBits(eWinRec->editDataPtr),
			&tempRect,&tempRect,foreMode,selectedRgn);
	}
	#endif
	
	#if USE_TEMP_MEM
	err=CheckPictureByte();
	#endif
	
	if (selectedRgn != nil)
		DisposeRgn(selectedRgn);
	
	/* コメント */
	if (eWinRec->isSelected)
	{
		PicComment(kCustomComment,6+rgnSize,dataHandle);
		
		#if USE_TEMP_MEM
		err=CheckPictureByte();
		#endif
	}
	if (isMaskAvailable)
	{	PicComment(kCustomComment,8,maskInfoH);
		
		#if USE_TEMP_MEM
		err=CheckPictureByte();
		#endif
	}
	
	#if USE_TEMP_MEM
	err=TempClosePicture(&pic);
	#else
#if __BIG_ENDIAN__
	(**pic).picFrame=selectedRect;
#endif
	
	ClosePicture();
	#endif
	
	if (eWinRec->isSelected)
		DisposeHandle(dataHandle);
	if (isMaskAvailable)
		DisposeHandle(maskInfoH);
	
	SetGWorld(cPort,cDevice);
	SetPort(port);
	
	return (pic);
}

/* 選択領域をPICT／アイコンにしてコピー */
void CopySelection(WindowPtr theWindow)
{
	PicHandle	selectionPic;
	Handle	iconHandle;
	long	dataSize;
	long	err;
	#if TARGET_API_MAC_CARBON
	ScrapRef	scrap;
	#endif
	
	/* PICT をコピー */
	selectionPic=GetSelectionPic(theWindow,false);
	dataSize=GetHandleSize((Handle)selectionPic);
	if (dataSize==0)
	{
		SysBeep(0);
		return;
	}
	
	#if TARGET_API_MAC_CARBON
	do
	{
		err=ClearCurrentScrap();
		if (err!=noErr) break;;
		err=GetCurrentScrap(&scrap);
		if (err!=noErr) break;
		MoveHHi((Handle)selectionPic);
		err=MemError();
		if (err!=noErr) break;
		HLock((Handle)selectionPic);
		err=MemError();
		if (err!=noErr) break;
		err=PutScrapFlavor(scrap,kPICTFileType,0,GetHandleSize((Handle)selectionPic),*selectionPic);
	} while (false);
	#else
	err=ZeroScrap();
	
	err=PutScrap(dataSize,kPICTFileType,*selectionPic);
	if (err<0)
	{
		SysBeep(10);
		return;
	}
	#endif
	
	KillPicture(selectionPic);
	if (err!=noErr)
	{
		SysBeep(0);
		return;
	}
	
	if (gOtherPrefs.copyIconWithPicture)
	{
		short iconSize[]={32,32,32,16,16,16};
		short iconDepth[]={8,4,1,8,4,1};
		ResType iconType[]={kLarge8BitData,kLarge4BitData,kLarge1BitMask,kSmall8BitData,kSmall4BitData,kSmall1BitMask};
		short	i;
		
		/* アイコンのリソースをコピー */
		for (i=0; i<6; i++)
		{
			iconHandle=GetSelectionIconHandle(theWindow,iconSize[i],iconDepth[i],false);
			if (iconHandle!=nil)
			{
				dataSize=GetHandleSize((Handle)iconHandle);
				#if CALL_NOT_IN_CARBON
				err=PutScrap(dataSize,iconType[i],*iconHandle);
				#else
				err=PutScrapFlavor(scrap,iconType[i],0,dataSize,*iconHandle);
				#endif
				DisposeHandle(iconHandle);
			}
		}
	}
}

/* 消去 */
void DoDelete(Boolean optDown)
{
	WindowPtr	theWindow=MyFrontNonFloatingWindow();
	PaintWinRec	*eWinRec;
	IconListWinRec	*iWinRec;
	IconFamilyWinRec	*fWinRec;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	if (theWindow==nil) return;
	
	switch (GetExtWindowKind(theWindow))
	{
		case kWindowTypePaintWindow:
			eWinRec=GetPaintWinRec(theWindow);
			
			GetGWorld(&cPort,&cDevice);
			GoOffPort(theWindow);
			
			SetGWorld(eWinRec->editDataPtr,0);
			if (optDown) /* オプションキーが押されいれば選択色で塗りつぶす */
			{
				RGBForeColor(&gCurrentColor.rgb);
				
				LockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
				PaintRect(&eWinRec->selectedRect);
				UnlockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
				
				/* マスクの調整 */
				SetGWorld(eWinRec->currentMask,0);
				LockPixels(GetGWorldPixMap(eWinRec->currentMask));
				if (gCurrentColor.isTransparent) /* 透明色 */
					EraseRect(&eWinRec->selectedRect);
				else
				{
					RgnHandle	selectedRgn=NewRgn();
					
					CopyRgn(eWinRec->selectionPos,selectedRgn);
					#if CALL_NOT_IN_CARBON
					MapRgn(selectedRgn,&(**eWinRec->selectionPos).rgnBBox,&eWinRec->selectedRect);
					#else
					{
						Rect		r;
						
						GetRegionBounds(selectedRgn,&r);
						MapRgn(selectedRgn,&r,&eWinRec->selectedRect);
					}
					#endif
					PaintRgn(selectedRgn);
					DisposeRgn(selectedRgn);
				}
				UnlockPixels(GetGWorldPixMap(eWinRec->currentMask));
				
				SetGWorld(eWinRec->editDataPtr,0);
				ForeColor(blackColor);
				SetUndoMode(umEffect);
			}
			else /* 単なる削除 */
			{
				Rect	r;
				
				#if CALL_NOT_IN_CARBON
				r=eWinRec->editDataPtr->portRect;
				#else
				GetPortBounds(eWinRec->editDataPtr,&r);
				#endif
				if (!EqualRect(&r,&eWinRec->iconSize))
				{
					GWorldFlags	flags;
					
					flags=UpdateGWorld(&eWinRec->editDataPtr,eWinRec->iconDepth,&eWinRec->iconSize,0,0,0);
					flags=UpdateGWorld(&eWinRec->currentMask,1,&eWinRec->iconSize,0,0,0);
				}
				
				CopyBits(GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
						GetPortBitMapForCopyBits(eWinRec->editDataPtr),
						&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
				CopyRgn(eWinRec->selectionPos,eWinRec->prevSelectionPos);
				CopyRgn(eWinRec->eSelectedRgn,eWinRec->ePrevSelectedRgn);
				RectRgn(eWinRec->selectionPos,&eWinRec->iconSize);
				SetEmptyRgn(eWinRec->eSelectedRgn);
				eWinRec->selectedRect=eWinRec->iconSize;
				eWinRec->isSelected=false;
				
				/* マスクの調整 */
				SetGWorld(eWinRec->currentMask,0);
				LockPixels(GetGWorldPixMap(eWinRec->currentMask));
				CopyBits(GetPortBitMapForCopyBits(eWinRec->selectionMask),
						GetPortBitMapForCopyBits(eWinRec->currentMask),
						&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
				UnlockPixels(GetGWorldPixMap(eWinRec->currentMask));
				
				UpdateClipMenu();
				UpdateSelectionMenu();
				SetUndoMode(umDelete);
			}
			CopyRgn(eWinRec->selectionPos,eWinRec->updateRgn);
			
			SetGWorld(cPort,cDevice);
			DispOffPort(theWindow);
			
			if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
				UpdatePaletteCheck();
			break;
		
		case kWindowTypeIconListWindow:
			iWinRec=GetIconListRec(theWindow);
			
			if (!IsIconSelected(iWinRec))
				return;
			
			DeleteSelectedIcon(iWinRec);
			UpdateClipMenu();
			UpdateIconMenu();
			break;
		
		case kWindowTypeIconFamilyWindow:
			fWinRec=GetIconFamilyRec(theWindow);
			if (fWinRec->selectedIcon>=0)
			{
				DeleteSelectedIconPicture(theWindow);
			}
			else if ((**fWinRec->iconNameTE).active)
			{
				TEDelete(fWinRec->iconNameTE);
				
				fWinRec->wasChanged=true;
				UpdateSaveMenu();
				UpdateClipMenu();
			}
			break;
	}
}

/* 複製 */
void DoDuplicate(void)
{
	WindowPtr	theWindow=MyFrontNonFloatingWindow();
	PaintWinRec	*eWinRec;
	Point		offset;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	if (theWindow==nil) return;
	
	switch (GetExtWindowKind(theWindow))
	{
		case kWindowTypePaintWindow:
			eWinRec=GetPaintWinRec(theWindow);
			
			SetPt(&offset,3,3);
			
			GetGWorld(&cPort,&cDevice);
			GoOffPort(theWindow);
			
			CopyRgn(eWinRec->selectionPos,eWinRec->prevSelectionPos);
			CopyRgn(eWinRec->eSelectedRgn,eWinRec->ePrevSelectedRgn);
			
			SetGWorld(eWinRec->selectedDataPtr,0);
			CopyBits(GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
					GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
					&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
			
			/* マスク */
			SetGWorld(eWinRec->selectionMask,0);
			{
				Rect	r;
				
				GetRegionBounds(eWinRec->selectionPos,&r);
				CopyBits(GetPortBitMapForCopyBits(eWinRec->currentMask),
						GetPortBitMapForCopyBits(eWinRec->selectionMask),
						&eWinRec->selectedRect,&r,srcOr,nil);
			}
			
			/* リージョンをちょっとずらす */
			OffsetRgn(eWinRec->selectionPos,offset.h,offset.v);
			OffsetRgn(eWinRec->eSelectedRgn,offset.h<<eWinRec->ratio,offset.v<<eWinRec->ratio);
			
			/* 選択範囲の位置 */
			eWinRec->prevMovement=offset;
			AddPt(offset,&eWinRec->selectionOffset);
			
			/* 前回の選択範囲と今回の選択範囲が更新する場所 */
			UnionRgn(eWinRec->selectionPos,eWinRec->prevSelectionPos,eWinRec->updateRgn);
			
			eWinRec->showSelection=true;
			
			SetGWorld(cPort,cDevice);
			DispOffPort(theWindow);
			
			UpdateSelectionMenu();
			SetUndoMode(umDuplicate);
			if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
				UpdatePaletteCheck();
			break;
		
		case kWindowTypeIconListWindow:
			DuplicateSelectedIcon(theWindow);
			break;
		
		default:
			SysBeep(0);
			break;
	}
}

/* すべてを選択 */
void DoSelectAll(void)
{
	WindowPtr	theWindow=MyFrontNonFloatingWindow();
	
	if (theWindow==nil) return;
	
	switch (GetExtWindowKind(theWindow))
	{
		case kWindowTypePaintWindow:
			ToolSelect(kMarqueeTool);
			DoubleClickTool(kMarqueeTool);
			break;
		
		case kWindowTypeIconListWindow:
			SelectAllIcons(theWindow);
			break;
	}
}

/* 取り消し */
void DoUndo(void)
{
	short		mode;
	WindowPtr	theWindow=MyFrontNonFloatingWindow();
	OSErr		err;
	PaintWinRec	*eWinRec;
	short		undoMode;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	if (theWindow==nil) return;
	
	switch (GetExtWindowKind(theWindow))
	{
		case kWindowTypePaintWindow:
			ResetRuler();
			
			eWinRec=GetPaintWinRec(theWindow);
			undoMode=eWinRec->undoMode;
			mode=(undoMode<0?-undoMode:undoMode);/* 絶対値 */
			GetGWorld(&cPort,&cDevice);
			
			switch (mode)
			{
				case umCannot:
					{
						MenuHandle	menu=GetMenuHandle(mEdit);
						
						#if CALL_NOT_IN_CARBON
						DisableItem(menu,iUndo);
						#else
						DisableMenuItem(menu,iUndo);
						#endif
						return;
					}
					break;
				
				case umColorChange:
					if (gOtherPrefs.checkWhenColorChanged)
					{
						short	temp=gPrevPaletteCheck;
						
						gPrevPaletteCheck=gPaletteCheck;
						HandlePaletteChoice(temp+1);
					}
				case umDotPaint:
					if (mode==umDotPaint)
					{
						Point	tempPt;
						Str31	tempString;
						#if CALL_NOT_IN_CARBON
						GrafPtr	port;
						#endif
						
						ResetDot(theWindow);
						
						tempPt=eWinRec->firstDotPos;
						eWinRec->firstDotPos=eWinRec->dotPos;
						eWinRec->dotPos=tempPt;
						PStrCpy(eWinRec->prevDotCommand,tempString);
						PStrCpy(eWinRec->dotCommand,eWinRec->prevDotCommand);
						PStrCpy(tempString,eWinRec->dotCommand);
						
						#if CALL_NOT_IN_CARBON
						GetPort(&port);
						SetPortWindowPort(DotModePalette);
						InvalRect(&DotModePalette->portRect);
						SetPort(port);
						#else
						{
							Rect	r;
							
							GetWindowPortBounds(DotModePalette,&r);
							InvalWindowRect(DotModePalette,&r);
						}
						#endif
					}
				case umPaint:
				case umEffect:
					ExchangeOffPort(eWinRec);
					ExchangeMask(eWinRec);
					if (gPaletteCheck == kPaletteCheckUsed)
						UpdatePaletteCheck();
					break;
				
				case umReplaceColor:
					{
						MyColorRec	tempColor;
						
						tempColor=gCurrentColor;
						ChangeColor(&eWinRec->replacedColor,eWinRec->replacedTransparent);
						eWinRec->replacedColor=tempColor.rgb;
						eWinRec->replacedTransparent=tempColor.isTransparent;
						ExchangeOffPort(eWinRec);
						ExchangeMask(eWinRec);
						if (gPaletteCheck == kPaletteCheckUsed)
							UpdatePaletteCheck();
					}
					break;
				
				case umDeselect:
					if (!gToolPrefs.selectionMasking)
						ToolSelect(kMarqueeTool);
				case umSelect:
				case umPaste:
				case umCut:
				case umDelete:
				case umRotate:
				case umFlip:
				case umInputText:
					ExchangeMask(eWinRec);
					ExchangeOffPort(eWinRec);
					ExchangeRgn(eWinRec->selectionPos,eWinRec->prevSelectionPos);
					ExchangeRgn(eWinRec->eSelectedRgn,eWinRec->ePrevSelectedRgn);
					eWinRec->isSelected=!EmptyRgn(eWinRec->eSelectedRgn);
					if (mode==umRotate)
					{
						Rect	tempRect;
						
						tempRect=eWinRec->prevSelectedRect;
						eWinRec->prevSelectedRect=eWinRec->selectedRect;
						eWinRec->selectedRect=tempRect;
						
						if (!eWinRec->isSelected)
							RectRgn(eWinRec->updateRgn,&eWinRec->iconSize);
					}
					else
					{
						#if CALL_NOT_IN_CARBON
						eWinRec->selectedRect=(**eWinRec->selectionPos).rgnBBox;
						#else
						GetRegionBounds(eWinRec->selectionPos,&eWinRec->selectedRect);
						#endif
						OffsetRect(&eWinRec->selectedRect,-eWinRec->selectionOffset.h,-eWinRec->selectionOffset.v);
					}
					if (eWinRec->isSelected)
						UpdateTransparentMenu();
					UnionRgn(eWinRec->selectionPos,eWinRec->prevSelectionPos,eWinRec->updateRgn);
				if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
					UpdatePaletteCheck();
				break;
				
				case umDuplicate:
					if (undoMode>0) /* undo */
					{
						SetGWorld(eWinRec->selectedDataPtr,0);
						LockPixels(GetGWorldPixMap(eWinRec->selectedDataPtr));
						EraseRgn(eWinRec->prevSelectionPos);
						UnlockPixels(GetGWorldPixMap(eWinRec->selectedDataPtr));
						
						/* マスク */
						SetGWorld(eWinRec->selectionMask,0);
						LockPixels(GetGWorldPixMap(eWinRec->selectionMask));
						EraseRgn(eWinRec->prevSelectionPos);
						UnlockPixels(GetGWorldPixMap(eWinRec->selectionMask));
						SetGWorld(cPort,cDevice);
					}
					else /* redo */
					{
						SetGWorld(eWinRec->selectedDataPtr,0);
						{
							Rect	r;
							
							GetRegionBounds(eWinRec->selectionPos,&r);
							CopyBits(GetPortBitMapForCopyBits(eWinRec->editDataPtr),
									GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
									&r,&r,srcCopy,eWinRec->selectionPos);
							
							SetGWorld(eWinRec->selectionMask,0);
							CopyBits(GetPortBitMapForCopyBits(eWinRec->currentMask),
									GetPortBitMapForCopyBits(eWinRec->selectionMask),
									&eWinRec->selectedRect,&r,srcOr,eWinRec->selectionPos);
						}
						SetGWorld(cPort,cDevice);
					}
				case umMove:
					/* マスクの更新はなし */
					ExchangeRgn(eWinRec->selectionPos,eWinRec->prevSelectionPos);
					ExchangeRgn(eWinRec->eSelectedRgn,eWinRec->ePrevSelectedRgn);
					
					UnionRgn(eWinRec->selectionPos,eWinRec->prevSelectionPos,eWinRec->updateRgn);
					
					SetPt(&eWinRec->prevMovement,-eWinRec->prevMovement.h,-eWinRec->prevMovement.v);
					AddPt(eWinRec->prevMovement,&eWinRec->selectionOffset);
					
					if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
						UpdatePaletteCheck();
					break;
				
				case umExchange:
					ExchangeSelection(theWindow);
					undoMode=-undoMode;
					if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
						UpdatePaletteCheck();
					break;
				
				case umResize:
					{
						GWorldPtr	tempGWorld=eWinRec->editDataPtr;
						GWorldFlags	flags;
						GrafPtr		port;
						
						eWinRec->editDataPtr=eWinRec->tempDataPtr;
						eWinRec->tempDataPtr=tempGWorld;
						
						#if CALL_NOT_IN_CARBON
						eWinRec->iconSize=eWinRec->editDataPtr->portRect;
						#else
						GetPortBounds(eWinRec->editDataPtr,&eWinRec->iconSize);
						#endif
						SetGWorld(eWinRec->dispTempPtr,0);
						flags=UpdateGWorld(&eWinRec->dispTempPtr,0,&eWinRec->iconSize,0,0,0);
						SetGWorld(cPort,cDevice);
						
						RectRgn(eWinRec->selectionPos,&eWinRec->iconSize);
						eWinRec->selectedRect=eWinRec->iconSize;
						
						ExchangeMask(eWinRec);
						
						GetPort(&port);
						SetPortWindowPort(theWindow);
						UpdatePaintWindow(theWindow);
						
						SetPortWindowPort(gPreviewWindow);
						SizeWindow(gPreviewWindow,eWinRec->iconSize.right,eWinRec->iconSize.bottom,false);
						#if CALL_NOT_IN_CARBON
						ClipRect(&gPreviewWindow->portRect);
						InvalRect(&gPreviewWindow->portRect);
						#else
						{
							Rect	r;
							
							GetWindowPortBounds(gPreviewWindow,&r);
							ClipRect(&r);
							InvalWindowRect(gPreviewWindow,&r);
						}
						#endif
						SetPort(port);
						
						RectRgn(eWinRec->updateRgn,&eWinRec->iconSize);
					}
					if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
						UpdatePaletteCheck();
					break;
				
				case umMaskChange:
					ExchangeMask(eWinRec);
					CopyRgn(eWinRec->selectionPos,eWinRec->updateRgn);
					UpdateTransparentMenu();
					if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
						UpdatePaletteCheck();
					break;
				
				case umAddForeground:
					if (undoMode>0) /* undo */
					{
						/* イメージ（選択範囲内。完全ではないが） */
						SetGWorld(eWinRec->editDataPtr,0);
						MyLockPixels(editDataPtr);
						EraseRect(&eWinRec->iconSize);
						CopyBits(GetPortBitMapForCopyBits(eWinRec->backgroundGWorld),
								GetPortBitMapForCopyBits(eWinRec->editDataPtr),
								&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,eWinRec->selectionPos);
						MyUnlockPixels(editDataPtr);
						
						/* 選択範囲外 */
						SetGWorld(eWinRec->selectedDataPtr,0);
						MyLockPixels(selectedDataPtr);
						CopyBits(GetPortBitMapForCopyBits(eWinRec->backgroundGWorld),
								GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
								&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
						RGBBackColor(&gBackColor.rgb);
						EraseRgn(eWinRec->selectionPos);
						BackColor(whiteColor);
						MyUnlockPixels(selectedDataPtr);
						
						/* マスク */
						SetGWorld(eWinRec->currentMask,0);
						MyLockPixels(currentMask);
						PaintRgn(eWinRec->selectionPos);
						MyUnlockPixels(currentMask);
						
						/* 選択範囲外 */
						SetGWorld(eWinRec->selectionMask,0);
						MyLockPixels(selectionMask);
						PaintRect(&eWinRec->iconSize);
						if (gBackColor.isTransparent)
							EraseRgn(eWinRec->selectionPos);
						MyUnlockPixels(selectionMask);
						
						SetGWorld(cPort,cDevice);
						
						DisposeGWorld(eWinRec->backgroundGWorld);
						eWinRec->backgroundGWorld=nil;
						
						eWinRec->editBackground=gOtherPrefs.addForeground;
					}
					else /* redo */
					{
						err=NewGWorld(&eWinRec->backgroundGWorld,eWinRec->iconDepth,&eWinRec->iconSize,0,0,useTempMem);
						MySetGWorld(backgroundGWorld);
						MyLockPixels(backgroundGWorld);
						EraseRect(&eWinRec->iconSize);
						MyUnlockPixels(backgroundGWorld);
						
						CopyBits(GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
								GetPortBitMapForCopyBits(eWinRec->backgroundGWorld),
								&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
						
						SetGWorld(cPort,cDevice);
						EraseOffPort(eWinRec);
						
						eWinRec->editBackground=false;
					}
					RectRgn(eWinRec->updateRgn,&eWinRec->iconSize);
					UpdateBackInfo(theWindow);
					UpdateBGMenu();
					if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
						UpdatePaletteCheck();
					break;
				
				case umAddBackground:
					if (undoMode>0) /* undo */
					{
						/* イメージ（選択範囲内。完全ではないが） */
						SetGWorld(eWinRec->editDataPtr,0);
						MyLockPixels(editDataPtr);
						EraseRect(&eWinRec->iconSize);
						CopyBits(GetPortBitMapForCopyBits(eWinRec->foregroundGWorld),
								GetPortBitMapForCopyBits(eWinRec->editDataPtr),
								&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,eWinRec->selectionPos);
						MyUnlockPixels(editDataPtr);
						
						/* 選択範囲外 */
						SetGWorld(eWinRec->selectedDataPtr,0);
						MyLockPixels(selectedDataPtr);
						CopyBits(GetPortBitMapForCopyBits(eWinRec->foregroundGWorld),
								GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
								&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
						EraseRgn(eWinRec->selectionPos);
						MyUnlockPixels(selectedDataPtr);
						
						/* マスク */
						SetGWorld(eWinRec->currentMask,0);
						MyLockPixels(currentMask);
						EraseRect(&eWinRec->iconSize);
						CopyBits(GetPortBitMapForCopyBits(eWinRec->foregroundMask),
								GetPortBitMapForCopyBits(eWinRec->currentMask),
								&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,eWinRec->selectionPos);
						MyUnlockPixels(currentMask);
						
						/* 選択範囲外 */
						SetGWorld(eWinRec->selectionMask,0);
						MyLockPixels(selectionMask);
						CopyBits(GetPortBitMapForCopyBits(eWinRec->foregroundMask),
								GetPortBitMapForCopyBits(eWinRec->selectionMask),
								&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
						EraseRgn(eWinRec->selectionPos);
						MyUnlockPixels(selectionMask);
						
						SetGWorld(cPort,cDevice);
						
						/* 前景およびマスクを削除 */
						DisposeGWorld(eWinRec->foregroundGWorld);
						eWinRec->foregroundGWorld=nil;
						DisposeGWorld(eWinRec->foregroundMask);
						eWinRec->foregroundMask=nil;
						
						eWinRec->editBackground=gOtherPrefs.addForeground;
					}
					else /* redo */
					{
						/* 前景を作成 */
						err=NewGWorld(&eWinRec->foregroundGWorld,eWinRec->iconDepth,&eWinRec->iconSize,0,0,useTempMem);
						SetGWorld(eWinRec->foregroundGWorld,0);
						MyLockPixels(foregroundGWorld);
						EraseRect(&eWinRec->iconSize);
						MyUnlockPixels(foregroundGWorld);
						
						CopyBits(GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
								GetPortBitMapForCopyBits(eWinRec->foregroundGWorld),
								&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
						
						/* 前景マスクを作成 */
						err=NewGWorld(&eWinRec->foregroundMask,1,&eWinRec->iconSize,0,0,useTempMem);
						MySetGWorld(foregroundMask);
						MyLockPixels(foregroundMask);
						EraseRect(&eWinRec->iconSize);
						CopyBits(GetPortBitMapForCopyBits(eWinRec->currentMask),
								GetPortBitMapForCopyBits(eWinRec->foregroundMask),
								&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
						CopyBits(GetPortBitMapForCopyBits(eWinRec->selectionMask),
								GetPortBitMapForCopyBits(eWinRec->foregroundMask),
								&eWinRec->iconSize,&eWinRec->iconSize,srcOr,nil);
						MyUnlockPixels(foregroundMask);
						
						/* 画像を消去 */
						SetGWorld(cPort,cDevice);
						EraseOffPort(eWinRec);
						
						eWinRec->editBackground=true;
					}
					RectRgn(eWinRec->updateRgn,&eWinRec->iconSize);
					UpdateBackInfo(theWindow);
					UpdateBGMenu();
					if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
						UpdatePaletteCheck();
					break;
				
				case umUnionBackground:
				case umDeleteBackground:
					ExchangeMask(eWinRec);
					ExchangeOffPort(eWinRec);
					
					if (undoMode>0) /* undo */
					{
						err=NewGWorld(&eWinRec->backgroundGWorld,eWinRec->iconDepth,&eWinRec->iconSize,0,0,useTempMem);
						SetGWorld(eWinRec->backgroundGWorld,0);
						LockPixels(GetGWorldPixMap(eWinRec->backgroundGWorld));
						EraseRect(&eWinRec->iconSize);
						UnlockPixels(GetGWorldPixMap(eWinRec->backgroundGWorld));
						
						CopyBits(GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
								GetPortBitMapForCopyBits(eWinRec->backgroundGWorld),
								&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
						
						SetGWorld(cPort,cDevice);
						eWinRec->editBackground=false;
						eWinRec->dispBackground=true;
					}
					else /* redo */
					{
						DisposeGWorld(eWinRec->backgroundGWorld);
						eWinRec->backgroundGWorld=nil;
						eWinRec->editBackground=gOtherPrefs.addForeground;
					}
					RectRgn(eWinRec->updateRgn,&eWinRec->iconSize);
					UpdateBackInfo(theWindow);
					UpdateBGMenu();
					if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
						UpdatePaletteCheck();
					break;
				
				case umDeleteForeground:
					ExchangeMask(eWinRec);
					
					if (undoMode>0) /* undo */
					{
						err=NewGWorld(&eWinRec->backgroundGWorld,eWinRec->iconDepth,&eWinRec->iconSize,0,0,useTempMem);
						SetGWorld(eWinRec->backgroundGWorld,0);
						LockPixels(GetGWorldPixMap(eWinRec->backgroundGWorld));
						EraseRect(&eWinRec->iconSize);
						UnlockPixels(GetGWorldPixMap(eWinRec->backgroundGWorld));
						
						CopyBits(GetPortBitMapForCopyBits(eWinRec->tempDataPtr),
								GetPortBitMapForCopyBits(eWinRec->backgroundGWorld),
								&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
						
						SetGWorld(eWinRec->editDataPtr,0);
						CopyBits(GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
								GetPortBitMapForCopyBits(eWinRec->editDataPtr),
								&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
						
						SetGWorld(cPort,cDevice);
						eWinRec->editBackground=false;
						eWinRec->dispBackground=true;
					}
					else /* redo */
					{
						SetGWorld(eWinRec->editDataPtr,0);
						CopyBits(GetPortBitMapForCopyBits(eWinRec->backgroundGWorld),
								GetPortBitMapForCopyBits(eWinRec->editDataPtr),
								&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
						
						SetGWorld(cPort,cDevice);
						DisposeGWorld(eWinRec->backgroundGWorld);
						eWinRec->backgroundGWorld=nil;
						eWinRec->editBackground=gOtherPrefs.addForeground;
					}
					RectRgn(eWinRec->updateRgn,&eWinRec->iconSize);
					UpdateBackInfo(theWindow);
					UpdateBGMenu();
					if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
						UpdatePaletteCheck();
					break;
				
				case umChangeTransparency:
					{
						short		tempShort;
						
						tempShort=eWinRec->foreTransparency;
						eWinRec->foreTransparency=eWinRec->pForeTransparency;
						eWinRec->pForeTransparency=tempShort;
						RectRgn(eWinRec->updateRgn,&eWinRec->iconSize);
						UpdateBGMenu();
					}
					break;
			}
			
			SetGWorld(cPort,cDevice);
			DispOffPort(theWindow);
			
			/* 編集メニューなどの更新 */
			UpdateClipMenu();
			UpdateSelectionMenu();
			
			undoMode=-undoMode;
			
			gUsedCount.undoNum++;
			
			SetUndoMode(undoMode);
			break;
		
		case kWindowTypeIconFamilyWindow:
			DoUndoIconFamily(theWindow);
			break;
		
		case kWindowTypeIconListWindow:
			DoUndoIconList(theWindow);
	}
}

/* リージョンの交換 */
void ExchangeRgn(RgnHandle rgn1,RgnHandle rgn2)
{
	RgnHandle	tempRgn;
	
	tempRgn=NewRgn();
	CopyRgn(rgn1,tempRgn);
	CopyRgn(rgn2,rgn1);
	CopyRgn(tempRgn,rgn2);
	DisposeRgn(tempRgn);
}

/* オフポートの内容を交換 */
void ExchangeOffPort(PaintWinRec *eWinRec)
{
	GWorldPtr	tempGWorld;
	
	tempGWorld=eWinRec->tempDataPtr;
	eWinRec->tempDataPtr=eWinRec->editDataPtr;
	eWinRec->editDataPtr=tempGWorld;
}

/* マスクの内容を交換 */
void ExchangeMask(PaintWinRec *eWinRec)
{
	GWorldPtr	tempGWorld;
	
	tempGWorld=eWinRec->pCurrentMask;
	eWinRec->pCurrentMask=eWinRec->currentMask;
	eWinRec->currentMask=tempGWorld;
}
/* ------------------------------------------------------------ */
/*  Window.c                                                    */
/*     routines for windows                                     */
/*                                                              */
/*                 1997.1.28 - 2002.6.30  naoki iimura        	*/
/* ------------------------------------------------------------ */

/* includes */
#ifdef __APPLE_CC__
	//#include	<Carbon/Carbon.h>
#else
	#include	<Sound.h>
	#include	<Resources.h>
	#include	<Fonts.h>
	#include	<TextUtils.h>
	#include	<LowMem.h>
	#include	<ToolUtils.h>
	#include	<Controls.h>
	#include	<PictUtils.h>
	#ifndef kControlUpButtonPart
		#include	<ControlDefinitions.h>
	#endif
	#include	<ColorPicker.h>
#endif

#include	"Globals.h"
#include	"UsefulRoutines.h"
#include	"IconParty.h"
#include	"UpdateCursor.h"
#include	"MenuRoutines.h"
#include	"WindowExtensions.h"
#include	"TabletUtils.h"
#include	"WindowRoutines.h"
#include	"Preferences.h"
#include	"IconRoutines.h"
#include	"IconListWindow.h"
#include	"IconFamilyWindow.h"
#include	"DebugMode.h"
#include	"PreCarbonSupport.h"
#include	"EditRoutines.h"
#include	"PaintRoutines.h"
#include	"ToolRoutines.h"
#include	"DotPalette.h"

/* prototypes */
/* update paint and preview windows */
static void	RedrawPaintWindow(WindowPtr theWindow,RgnHandle updateRgn);
static void	RedrawPreviewWindow(WindowPtr frontWindow);
static GWorldPtr	MakePreviewImage(PaintWinRec *eWinRec);
static GWorldPtr	MakePaintImage(PaintWinRec *eWinRec,RgnHandle updateRgn);
static void	UpdatePatternPalette(void);
static void	UpdateFavoritePalette(WindowPtr theWindow);

/* click windows */
static void	ClickPaintWindow(WindowPtr theWindow,Point localPt,EventRecord *theEvent);
static void	ClickPaintWindowMain(WindowPtr theWindow,Point pt,long modifiers);
static void	ClickColorPalette1(Point pt,Boolean optDown,Boolean cmdDown);
static void	ClickColorPalette2(Point pt,Boolean optDown,Boolean cmdDown);
static void ClickFavoritePalette(Point mousePt,Boolean optDown,Boolean cmdDown);
static void	ClickToolPalette(Point pt,unsigned long clickTime);
static void	ClickBlendPalette(Point pt,Boolean optDown,Boolean cmdDown,Boolean ctrlDown);
static void	ClickInfoWindow(Point pt);
static void	ClickPatternPalette(Point localPt);
static void	ColorPalettePopup(Point localPt);

static void	DoBlendPop(Point localPt);

static void	LightenRect(const Rect *r);
static void	BlendColor(RGBColor *dst,RGBColor *src);

static void	DrawRatio(WindowPtr theWindow);
static void	DrawBackInfo(WindowPtr theWindow);
static void	BackgroundPop(WindowPtr theWindow,Point popPt);

/* floating windows (tools) */
static WindowPtr	GetNewFloatingWindow(short WINDid,short windowKind,long refCon);

/* color palettes */
static void	DrawPaletteCheck(short palNo,short palKind);
static CTabHandle	GetCheckPalette(short palKind,short *colorNum);
static Boolean ColorExistsInPal(RGBColor *color,CTabHandle ctab,short colorNum);
static void	HilitePattern(short n,Boolean hiliteFlag);

/*  */
static Boolean WindowExistsAtPt(Point globPt);

/* scroll paint windows */
static void	DoScrollPaintWindow(WindowPtr theWindow,Point localPt);
static pascal void	MyScrollHandler(ControlHandle theControl,short part);
static void	ContentScroll(WindowPtr theWindow,ControlHandle theControl,short delta);
static void	GrabScrollPaintWindow(WindowPtr theWindow,Point globPt);
static void DoScrollPaintWindowMain(WindowPtr theWindow,short hDelta,short vDelta);

/* resize paint windows */
static void	ResizePaintWindowMain(WindowPtr eWindow);
static void	SetScrollBarRect(WindowPtr eWindow);
static void	UpdatePaintClipRect(WindowPtr theWindow);

/* Drag & Drop */
static void	MyDoStartDragPaintWin(WindowPtr eWindow,EventRecord *theEvent);
static OSErr	MyDoAddPaintWinFlavors(WindowPtr eWindow,DragReference theDrag);
static OSErr	MyGetDragPaintWinRegion(WindowPtr eWindow,RgnHandle dragRegion,
								DragReference theDragRef);
static pascal short	MySendPaintWinDataProc(FlavorType theType,void *dragSendRefCon,
										ItemReference theItem,DragReference theDrag);

/* the window pointers */
WindowPtr	ColorPalette1;
WindowPtr	ColorPalette2;
static WindowPtr	TitleWindow;
static WindowPtr	InfoWindow;
WindowPtr	PatternPalette;
WindowPtr	DotModePalette;
WindowPtr	FavoritePalette;


#define	WINERR_RESID	4001
#define	WINERR1	1


/* Initialize tool windows */
void ToolWindowInit(void)
{
	OSErr	err;
	Point	pt;
	Boolean	vis;
	
	UseResFile(gApplRefNum);
	
	WriteStrToDebugFile("\pOpening tool palette... ");
	gToolPalette=GetNewFloatingWindow(kToolPaletteResID,kWindowTypeToolPalette,(long)nil);
	#ifdef DEBUG_MODE
		if (gToolPalette!=nil) WriteStrToDebugFile("\pok\r");
	#endif
	WriteStrToDebugFile("\pOpening color palette 1... ");
	ColorPalette1=GetNewFloatingWindow(kColorPalette1ResID,kWindowTypeColorPalette1,(long)nil);
	#ifdef DEBUG_MODE
		if (ColorPalette1!=nil) WriteStrToDebugFile("\pok\r");
	#endif
	WriteStrToDebugFile("\pOpening color palette 2... ");
	ColorPalette2=GetNewFloatingWindow(kColorPalette2ResID,kWindowTypeColorPalette2,(long)nil);
	#ifdef DEBUG_MODE
		if (ColorPalette2!=nil) WriteStrToDebugFile("\pok\r");
	#endif
	WriteStrToDebugFile("\pOpening blend palette... ");
	gBlendPalette=GetNewFloatingWindow(kBlendPaletteResID,kWindowTypeBlendPalette,(long)nil);
	#ifdef DEBUG_MODE
		if (gBlendPalette!=nil) WriteStrToDebugFile("\pok\r");
	#endif
	WriteStrToDebugFile("\pOpening title window... ");
	TitleWindow=GetNewFloatingWindow(kTitleWindowResID,kWindowTypeTitleWindow,(long)nil);
	#ifdef DEBUG_MODE
		if (TitleWindow!=nil) WriteStrToDebugFile("\pok\r");
	#endif
	WriteStrToDebugFile("\pOpening information palette... ");
	InfoWindow=GetNewFloatingWindow(kInfoWindowResID,kWindowTypeInfoWindow,(long)nil);
	#ifdef DEBUG_MODE
		if (InfoWindow!=nil) WriteStrToDebugFile("\pok\r");
	#endif
	WriteStrToDebugFile("\pOpening pattern palette... ");
	PatternPalette=GetNewFloatingWindow(kPatternPaletteWindowResID,kWindowTypePatternPalette,(long)nil);
	#ifdef DEBUG_MODE
		if (PatternPalette!=nil) WriteStrToDebugFile("\pok\r");
	#endif
	WriteStrToDebugFile("\pOpening dot mode palette... ");
	DotModePalette=GetNewFloatingWindow(kDotModePaletteWindowResID,kWindowTypeDotModePalette,(long)nil);
	#ifdef DEBUG_MODE
		if (DotModePalette!=nil) WriteStrToDebugFile("\pok\r");
	#endif
	WriteStrToDebugFile("\pOpening favorite palette... ");
	FavoritePalette=GetNewFloatingWindow(kFavoritePaletteResID,kWindowTypeFavoritePalette,(long)nil);
	#ifdef DEBUG_MODE
		if (FavoritePalette!=nil) WriteStrToDebugFile("\pok\r");
	#endif
	WriteStrToDebugFile("\pOpening preview window... ");
	gPreviewWindow=GetNewFloatingWindow(kPreviewWindowResID,kWindowTypePreviewWindow,(long)nil);
	#ifdef DEBUG_MODE
		if (gPreviewWindow!=nil) WriteStrToDebugFile("\pok\r");
	#endif
	
	if (gToolPalette==nil || ColorPalette1==nil || ColorPalette2==nil || gBlendPalette==nil ||
		TitleWindow==nil || InfoWindow==nil || PatternPalette==nil || DotModePalette==nil ||
		FavoritePalette==nil || gPreviewWindow==nil)
	{
		/* WIND resources are damaged */
 		ErrorAlertFromResource(WINERR_RESID,WINERR1);
		ExitToShell(); /* quit */
	}
	
	if (gPrefFileRefNum>0)
	{
		/* read window positions from preference file */
		UseResFile(gPrefFileRefNum);
		
		/* color palette 1 */
		err=LoadDataFromPrefs(&pt,sizeof(Point),'WPOS',kColorPalette1ResID);
		if (err==noErr && PtInRgn(pt,GetGrayRgn())) /* 画面内かどうか */
			MoveWindow(ColorPalette1,pt.h,pt.v,false);
		
		/* カラーパレット２ */
		err=LoadDataFromPrefs(&pt,sizeof(Point),'WPOS',kColorPalette2ResID);
		if (err==noErr && PtInRgn(pt,GetGrayRgn())) /* 画面内かどうか */
			MoveWindow(ColorPalette2,pt.h,pt.v,false);
		
		/* プレビュー画面 */
		err=LoadDataFromPrefs(&pt,sizeof(Point),'WPOS',kPreviewWindowResID);
		if (err==noErr && PtInRgn(pt,GetGrayRgn())) /* 画面内かどうか */
			MoveWindow(gPreviewWindow,pt.h,pt.v,false);
		
		/* ツールパレット */
		err=LoadDataFromPrefs(&pt,sizeof(Point),'WPOS',kToolPaletteResID);
		if (err==noErr && PtInRgn(pt,GetGrayRgn())) /* 画面内かどうか */
			MoveWindow(gToolPalette,pt.h,pt.v,false);
		
		/* ブレンドパレット */
		err=LoadDataFromPrefs(&pt,sizeof(Point),'WPOS',kBlendPaletteResID);
		if (err==noErr && PtInRgn(pt,GetGrayRgn())) /* 画面内かどうか */
			MoveWindow(gBlendPalette,pt.h,pt.v,false);
		
		/* タイトル画面 */
		err=LoadDataFromPrefs(&pt,sizeof(Point),'WPOS',kTitleWindowResID);
		if (err==noErr && PtInRgn(pt,GetGrayRgn())) /* 画面内かどうか */
			MoveWindow(TitleWindow,pt.h,pt.v,false);
		
		/* 情報ウィンドウ */
		err=LoadDataFromPrefs(&pt,sizeof(Point),'WPOS',kInfoWindowResID);
		if (err==noErr && PtInRgn(pt,GetGrayRgn())) /* 画面内かどうか */
			MoveWindow(InfoWindow,pt.h,pt.v,false);
		
		/* パターンパレット */
		err=LoadDataFromPrefs(&pt,sizeof(Point),'WPOS',kPatternPaletteWindowResID);
		if (err==noErr && PtInRgn(pt,GetGrayRgn()))
			MoveWindow(PatternPalette,pt.h,pt.v,false);
		
		/* 描画点モードパレット */
		err=LoadDataFromPrefs(&pt,sizeof(Point),'WPOS',kDotModePaletteWindowResID);
		if (err==noErr && PtInRgn(pt,GetGrayRgn()))
			MoveWindow(DotModePalette,pt.h,pt.v,false);
		
		/* お気に入りパレット */
		err=LoadDataFromPrefs(&pt,sizeof(Point),'WPOS',kFavoritePaletteResID);
		if (err==noErr && PtInRgn(pt,GetGrayRgn()))
			MoveWindow(FavoritePalette,pt.h,pt.v,false);
		
		/* 初期設定によって、表示／非表示を変更する */
		/* カラーパレット１ */
		err=LoadDataFromPrefs(&vis,sizeof(Boolean),'WVis',kColorPalette1ResID);
		if ((err==noErr && vis) || err!=noErr) 
			ShowHidePalette(ColorPalette1,true);
		
		/* カラーパレット２ */
		err=LoadDataFromPrefs(&vis,sizeof(Boolean),'WVis',kColorPalette2ResID);
		if ((err==noErr && vis) || err!=noErr) 
			ShowHidePalette(ColorPalette2,true);
		
		/* ツールパレット */
		err=LoadDataFromPrefs(&vis,sizeof(Boolean),'WVis',kToolPaletteResID);
		if ((err==noErr && vis) || err!=noErr) 
			ShowHidePalette(gToolPalette,true);
		
		/* ブレンドパレット */
		err=LoadDataFromPrefs(&vis,sizeof(Boolean),'WVis',kBlendPaletteResID);
		if ((err==noErr && vis) || err!=noErr) 
			ShowHidePalette(gBlendPalette,true);
		
		/* タイトル画面 */
		err=LoadDataFromPrefs(&vis,sizeof(Boolean),'WVis',kTitleWindowResID);
		if ((err==noErr && vis) || err!=noErr) 
			ShowHidePalette(TitleWindow,true);
		
		/* 情報画面 */
		err=LoadDataFromPrefs(&vis,sizeof(Boolean),'WVis',kInfoWindowResID);
		if ((err==noErr && vis) || err!=noErr) 
			ShowHidePalette(InfoWindow,true);
		
		/* お気に入りパレット */
		err=LoadDataFromPrefs(&vis,sizeof(Boolean),'WVis',kFavoritePaletteResID);
		if ((err==noErr && vis)) 
			ShowHidePalette(FavoritePalette,true);
		
		/* パターンパレット */
		if (gOtherPrefs.useHyperCardPalette)
			SizeWindow(PatternPalette,163,67,false);
		
		err=LoadDataFromPrefs(&vis,sizeof(Boolean),'WVis',kPatternPaletteWindowResID);
		if ((err==noErr && vis) || err!=noErr)
			ShowHidePalette(PatternPalette,true);
		
		UseResFile(gApplRefNum);
	}
	else
	{
		ShowHidePalette(ColorPalette1,true);
		ShowHidePalette(ColorPalette2,true);
		ShowHidePalette(gToolPalette,true);
		ShowHidePalette(gBlendPalette,true);
		ShowHidePalette(TitleWindow,true);
		ShowHidePalette(InfoWindow,true);
		ShowHidePalette(PatternPalette,true);
		ShowHidePalette(FavoritePalette,false);
	}
}

/* フローティングウィンドウを作成 */
WindowPtr GetNewFloatingWindow(short WINDid,short windowKind,long refCon)
{
	WindowPtr	theWindow;
	OSErr	err;
	
	err=GetNewWindowReference(&theWindow,WINDid,kFirstWindowOfClass,nil);
	if (err!=noErr)
	{
		#ifdef DEBUG_MODE
		Str255	log="\pfailed (error code = ",log2;
		
		NumToString(err,log2);
		PStrCat(log2,log);
		PStrCat("\p)\r",log);
		WriteStrToDebugFile(log);
		#endif
		
		return nil;
	}
	
	SetExtWindowKind(theWindow,windowKind);
	SetExtWRefCon(theWindow,refCon);
	
	return theWindow;
}

/* ウィンドウに拡張された種類を設定する */
void SetExtWindowKind(WindowPtr theWindow,short windowKind)
{
	#if TARGET_API_MAC_CARBON
	OSErr	err;
	
	err=SetWindowProperty(theWindow,kIconPartyCreator,'kind',sizeof(windowKind),&windowKind);
	#else
	WindowRefExtensions **extensionsH;
	
	extensionsH=(WindowRefExtensions **)GetWRefCon(theWindow);
	if (extensionsH != nil)
		(*extensionsH)->windowKind=windowKind;
	#endif
}

short GetExtWindowKind(WindowPtr theWindow)
{
	#if TARGET_API_MAC_CARBON
	OSErr	err;
	UInt32	dataSize;
	UInt32	actualSize;
	short	kind;
	
	err=GetWindowPropertySize(theWindow,kIconPartyCreator,'kind',&dataSize);
	if (err!=noErr || dataSize != sizeof(kind)) return -1;
	
	err=GetWindowProperty(theWindow,kIconPartyCreator,'kind',dataSize,&actualSize,&kind);
	if (err!=noErr) return err;
	
	return kind;
	#else
	WindowRefExtensions **extensionsH;
	
	if (theWindow==nil) return -1;
	
	extensionsH=(WindowRefExtensions **)GetWRefCon(theWindow);
	if (extensionsH != nil)
		return (*extensionsH)->windowKind;
	else
		return -1;
	#endif
}


/* track update event */
void DoUpdate(EventRecord *theEvent)
{
	WindowPtr	theWindow;
	GrafPtr		port;
	PaintWinRec	*eWinRec;
	PicHandle	pic;
	WindowPtr	frontWin;
	RgnHandle	imageRgn,tempClip;
	Rect		imageRect;
	
	theWindow=(WindowPtr)theEvent->message;
	BeginUpdate(theWindow);
	
	GetPort(&port);
	SetPortWindowPort(theWindow);
	
	switch (GetExtWindowKind(theWindow))
	{
		case kWindowTypePaintWindow:
			eWinRec=GetPaintWinRec(theWindow);
			
			tempClip=NewRgn();
			GetClip(tempClip);
			imageRgn=NewRgn();
			GetWindowPortBounds(theWindow,&imageRect);
			imageRect.right-=kScrollBarWidth;
			imageRect.bottom-=kScrollBarHeight;
			RectRgn(imageRgn,&imageRect);
			SectRgn(imageRgn,tempClip,imageRgn);
			SetClip(imageRgn);
			
			RedrawPaintWindow(theWindow,imageRgn);
			
			SetClip(tempClip);
			DisposeRgn(tempClip);
			DisposeRgn(imageRgn);
			
			/* サイズボックス */
			DrawGrowIcon(theWindow);
			
			/* スクロールバー */
			DrawControls(theWindow);
			
			/* 倍率 */
			DrawRatio(theWindow);
			
			/* バックグラウンド情報 */
			DrawBackInfo(theWindow);
			break;
		
		case kWindowTypePreviewWindow:
			ForeColor(blackColor);
			frontWin=MyFrontNonFloatingWindow();
			if (frontWin==nil) break;
			
			if (GetExtWindowKind(frontWin) == kWindowTypePaintWindow)
				RedrawPreviewWindow(frontWin);
			else
				UpdateIconPreview(frontWin);
			break;
		
		case kWindowTypeColorPalette1:
			pic=GetPicture(kColorPalette1PictureResID);
			GetWindowPortBounds(theWindow,&imageRect);
			DrawPicture(pic,&imageRect);
			if (gPaletteCheck>kPaletteCheckNone)
				DrawPaletteCheck(1,gPaletteCheck);
			HiliteSelectedColor(1,true);
			break;
		
		case kWindowTypeColorPalette2:
			pic=GetPicture(kColorPalette2PictureResID);
			GetWindowPortBounds(theWindow,&imageRect);
			DrawPicture(pic,&imageRect);
			if (gPaletteCheck>kPaletteCheckNone)
				DrawPaletteCheck(2,gPaletteCheck);
			HiliteSelectedColor(2,true);
			break;
		
		case kWindowTypeToolPalette:
			pic=GetPicture(kToolPalettePictureResID);
			GetWindowPortBounds(theWindow,&imageRect);
			DrawPicture(pic,&imageRect);
			DarkenRect(&gToolRect); /* hilite selected tool */
			break;
		
		case kWindowTypeBlendPalette:
			GetWindowPortBounds(theWindow,&imageRect);
			CopyBits(GetPortBitMapForCopyBits(gBlendPalettePtr),
				GetPortBitMapForCopyBits(GetWindowPort(theWindow)),
				&imageRect,&imageRect,srcCopy,0);
			break;
		
		case kWindowTypeTitleWindow:
			pic=GetPicture(kTitleWindowPictureResID);
			GetWindowPortBounds(theWindow,&imageRect);
			DrawPicture(pic,&imageRect);
			break;
		
		case kWindowTypeInfoWindow:
			RedrawInfo();
			break;
		
		case kWindowTypePatternPalette:
			UpdatePatternPalette();
			break;
		
		case kWindowTypeDotModePalette:
			UpdateDotModePalette();
			break;
		
		case kWindowTypeIconListWindow:
			UpdateIconWindow(theWindow);
			break;
		
		case kWindowTypeIconFamilyWindow:
			UpdateFamilyWindow(theWindow);
			break;
		
		case kWindowTypeFavoritePalette:
			UpdateFavoritePalette(theWindow);
			HiliteSelectedColor(3,true);
			break;
		
		default:
			break;
	}
	SetPort(port);
	
	EndUpdate(theWindow);
}

/* ペイントウィンドウの再描画 */
void RedrawPaintWindow(WindowPtr theWindow,RgnHandle updateRgn)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	GWorldPtr	eDispTempPtr;
	RgnHandle	tempRgn=NewRgn();
	
	eDispTempPtr=MakePaintImage(eWinRec,updateRgn);
	
	/* 実際のウィンドウに描画 */
	SetPortWindowPort(theWindow);
	GetClip(tempRgn);
	SetClip(updateRgn);
	CopyBits(GetPortBitMapForCopyBits(eDispTempPtr),
		GetPortBitMapForCopyBits(GetWindowPort(theWindow)),
		&eWinRec->iconEditSize,&eWinRec->iconEditSize,srcCopy,nil);
	
	if (eWinRec->isSelected && eWinRec->showSelection)
		DispSelectionMain(theWindow,true,false);
	
	DisposeGWorld(eDispTempPtr);
	SetClip(tempRgn);
	DisposeRgn(tempRgn);
}

/* ペイントウィンドウ用の画像を作成 */
GWorldPtr MakePaintImage(PaintWinRec *eWinRec,RgnHandle updateRgn)
{
	GWorldPtr	tempGWorld,eDispTempPtr;
	OSErr		err;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	GetGWorld(&cPort,&cDevice);
	
	/* 描画用のGWorld作成 */
	tempGWorld=MakePreviewImage(eWinRec);
	if (tempGWorld == nil) return nil;
	
	/* 実際の大きさに拡大したイメージを作成 */
	err=NewGWorld(&eDispTempPtr,eWinRec->iconDepth,&eWinRec->iconEditSize,0,0,useTempMem);
	if (err!=noErr)
	{
		if (eWinRec->backgroundGWorld != nil)
			DisposeGWorld(tempGWorld);
		return nil;
	}
	
	SetGWorld(eDispTempPtr,0);
	LockPixels(GetGWorldPixMap(eDispTempPtr));
	ForeColor(blackColor);
	BackColor(whiteColor);
	if (updateRgn != nil) SetClip(updateRgn);
	EraseRect(&eWinRec->iconEditSize);
	CopyBits(GetPortBitMapForCopyBits(tempGWorld),GetPortBitMapForCopyBits(eDispTempPtr),
		&eWinRec->iconSize,&eWinRec->iconEditSize,srcCopy,nil);
	
	/* draw grid */
	if (gToolPrefs.gridMode!=iGNone)
		DrawGridMain(eWinRec->ratio,&eWinRec->iconSize);
	
	#if 0
	/* draw selection */
	if (drawSelection && eWinRec->isSelected && eWinRec->showSelection)
	{
		PatHandle	gray;
		
		gray=GetPattern(128);
		PenNormal();
		PenMode(patXor);
		PenPat(*gray);
		FrameRgn(eWinRec->eSelectedRgn);
		PenNormal();
		ReleaseResource((Handle)gray);
	}
	#endif
	
	if (eWinRec->backgroundGWorld != nil)
		DisposeGWorld(tempGWorld);
	
	UnlockPixels(GetGWorldPixMap(eDispTempPtr));
	
	SetGWorld(cPort,cDevice);
	
	return eDispTempPtr;
}

/* プレビューウィンドウの再描画 */
void RedrawPreviewWindow(WindowPtr frontWindow)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(frontWindow);
	GWorldPtr	tempGWorld;
	
	tempGWorld=MakePreviewImage(eWinRec);
	
	/* display temporary->display */
	CopyBits(GetPortBitMapForCopyBits(tempGWorld),
		GetPortBitMapForCopyBits(GetWindowPort(gPreviewWindow)),
		&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,0);
	
	if (eWinRec->backgroundGWorld != nil)
		DisposeGWorld(tempGWorld);
	
	/* display selection */
	if (eWinRec->isSelected && eWinRec->showSelection)
		DispSelectionMain(frontWindow,false,true);
}

/* 前景、背景を合成したイメージを作成する */
GWorldPtr MakePreviewImage(PaintWinRec *eWinRec)
{
	OSErr		err;
	GWorldPtr	tempGWorld;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	GetGWorld(&cPort,&cDevice);
	
	if (eWinRec->backgroundGWorld != nil)
	{
		err=NewGWorld(&tempGWorld,eWinRec->iconDepth,&eWinRec->iconSize,0,0,useTempMem);
		if (err!=noErr) return nil;
		SetGWorld(tempGWorld,0);
		LockPixels(GetGWorldPixMap(tempGWorld));
		EraseRect(&eWinRec->iconSize);
		UnlockPixels(GetGWorldPixMap(tempGWorld));
		
		if (eWinRec->dispBackground)
			CopyBits(GetPortBitMapForCopyBits(eWinRec->backgroundGWorld),
				GetPortBitMapForCopyBits(tempGWorld),
				&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
		if (eWinRec->foreTransparency != kForeTrans0)
		{
			short	foreMode=SetForeBlend(eWinRec);
			
			if (!eWinRec->isSelected) /* 選択範囲なし */
				SafeCopyDeepMask(GetPortBitMapForCopyBits(eWinRec->editDataPtr),
					GetPortBitMapForCopyBits(eWinRec->currentMask),
					GetPortBitMapForCopyBits(tempGWorld),
					&eWinRec->iconSize,&eWinRec->iconSize,&eWinRec->iconSize,foreMode,nil);
			else
			{
				GWorldPtr	maskGWorld;
				Rect		r;
				
				GetRegionBounds(eWinRec->selectionPos,&r);
				
				err=NewGWorld(&maskGWorld,1,&eWinRec->iconSize,0,0,useTempMem);
				SetGWorld(maskGWorld,0);
				LockPixels(GetGWorldPixMap(maskGWorld));
				EraseRect(&eWinRec->iconSize);
				CopyBits(GetPortBitMapForCopyBits(eWinRec->currentMask),
					GetPortBitMapForCopyBits(maskGWorld),
					&eWinRec->selectedRect,&r,srcCopy,nil);
				CopyBits(GetPortBitMapForCopyBits(eWinRec->selectionMask),
					GetPortBitMapForCopyBits(maskGWorld),
					&eWinRec->iconSize,&eWinRec->iconSize,srcOr,nil);
				UnlockPixels(GetGWorldPixMap(maskGWorld));
				
				SetGWorld(tempGWorld,0);
				SafeCopyDeepMask(GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
					GetPortBitMapForCopyBits(maskGWorld),
					GetPortBitMapForCopyBits(tempGWorld),
					&eWinRec->iconSize,&eWinRec->iconSize,&eWinRec->iconSize,
					foreMode,nil);
				
				DisposeGWorld(maskGWorld);
			}
		}
	}
	else
		tempGWorld=eWinRec->dispTempPtr;
	
	SetGWorld(cPort,cDevice);
	
	return tempGWorld;
}

/* インフォメーションウィンドウの再描画 */
void RedrawInfo(void)
{
	Rect	r;
	GrafPtr	port;
	PicHandle	pic;
	
	GetPort(&port);
	SetPortWindowPort(InfoWindow);
	
	pic=GetPicture(kInfoWindow0pPictureResID+gBSelectedItem-1);
	GetWindowPortBounds(InfoWindow,&r);
	DrawPicture(pic,&r);
	ForeColor(blackColor);
	SetRect(&r,0x09-(gPenWidth>>1),0x09-(gPenHeight>>1),
		0x09+((gPenWidth+1)>>1),0x09+((gPenHeight+1)>>1));
	PaintRect(&r);
	SetRect(&r,0x1b-(gEraserWidth>>1),0x09-(gEraserHeight>>1),
		0x1b+((gEraserWidth+1)>>1),0x09+((gEraserHeight+1)>>1));
	PaintRect(&r);
	
	SetPort(port);
}

#define	kPatternPalettePicture	142
#define	kPatternNum	24
#define	kHyperCardPaletteNum	40

/* パターンパレットの再描画 */
void UpdatePatternPalette(void)
{
	short	i,x,y;
	Rect	r;
	Pattern	pat;
	short	w,id,patNum;
	
	GetWindowPortBounds(PatternPalette,&r);
	EraseRect(&r);
	
	if (gOtherPrefs.useHyperCardPalette)
	{
		w=10;
		id=201;
		patNum=kHyperCardPaletteNum;
	}
	else
	{
		w=6;
		id=200;
		patNum=kPatternNum;
	}
	
	PenNormal();
	for (i=0; i<=w; i++)
	{
		MoveTo(i*16+1,1);
		Line(0,64);
	}
	
	for (i=0; i<=4; i++)
	{
		MoveTo(1,i*16+1);
		Line(w*16,0);
	}
	
	RGBForeColor(&gCurrentColor.rgb);
	RGBBackColor(&gBackColor.rgb);
	
	for (i=0; i<patNum; i++)
	{
		x=(i % w)*(13+3)+3;
		y=(i / w)*(13+3)+3;
		SetRect(&r,x,y,x+13,y+13);
		GetIndPattern(&pat,id,i+1);
		FillRect(&r,&pat);
	}
	
	ForeColor(blackColor);
	BackColor(whiteColor);
	
	HilitePattern(gPatternNo,true);
}

/* パターンパレットのリサイズ */
void ResizePatternPalette(void)
{
	Rect	r;
	
	/* 初期設定変更前なので逆になる（わかりにくい） */
	if (gOtherPrefs.useHyperCardPalette)
	{
		SizeWindow(PatternPalette,99,67,false);
		gPatternNo=1;
	}
	else
	{
		SizeWindow(PatternPalette,163,67,false);
		gPatternNo=12;
	}
	SetPortWindowPort(PatternPalette);
	GetWindowPortBounds(PatternPalette,&r);
	EraseRect(&r);
	MyInvalWindowRect(PatternPalette,&r);
}

/* お気に入りパレットのアップデート */
void UpdateFavoritePalette(WindowPtr theWindow)
{
	ColorSpec	*cspec;
	short		colorNum,h,v;
	short		i;
	Rect		r;
	
	colorNum = (*gFavoriteColors)->ctSize + 1;
	GetWindowPortBounds(theWindow,&r);
	if (colorNum == 0)
	{
		EraseRect(&r);
	}
	else
	{
		cspec = &(*gFavoriteColors)->ctTable[0];
		
		ForeColor(whiteColor);
		FrameRect(&r);
		ForeColor(blackColor);
		
		h = (colorNum-1)%16 +1;
		v = (colorNum-1)/16;
		
		if (colorNum > 16) /* 2段以上になる */
		{
			for (i=0; i<= v; i++)
			{
				MoveTo(1,i*10+1);
				Line(16*10,0);
			}
			for (i=0; i<= 16; i++)
			{
				MoveTo(i*10+1,1);
				Line(0,v*10);
			}
		}
		if (v < 6)
		{
			MoveTo(1,v*10+1);
			Line(h*10,0);
			MoveTo(1,v*10+11);
			Line(h*10,0);
			for (i=0; i<= h; i++)
			{
				MoveTo(i*10+1,v*10+1);
				Line(0,10);
			}
		}
		
		/* 余白 */
		SetRect(&r,h*10+2,(v>0 ? v*10+2 : 1),16*10+2,v*10+1+11);
		EraseRect(&r);
		SetRect(&r,0,v*10+10+2,16*10+2,6*10+2);
		EraseRect(&r);
		
		/* パレット自体 */
		for (i=0; i< 256; i++)
		{
			if (i >= colorNum) break;
			SetRect(&r,(i%16)*10+2,(i/16)*10+2,(i%16)*10+11,(i/16)*10+11);
			RGBForeColor(&cspec[i].rgb);
			PaintRect(&r);
		}
		ForeColor(blackColor);
	}
}

/* ペイントウィンドウのマスク表示 */
void DispPaintMask(WindowPtr theWindow)
{
	PaintWinRec	*eWinRec;
	Rect		paintRect;
	RgnHandle	clipRgn;
	GrafPtr		port;
	KeyMap		theKeys;
	Rect		effectRect,tempRect;
	GWorldPtr	maskGWorld;
	OSErr		err;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	if (theWindow == nil || GetExtWindowKind(theWindow) != kWindowTypePaintWindow) return;
	
	/* 各データ初期化 */
	HideCursor();
	GetPort(&port);
	eWinRec=GetPaintWinRec(theWindow);
	GetWindowPortBounds(theWindow,&paintRect);
	paintRect.right-=kScrollBarWidth;
	paintRect.bottom-=kScrollBarHeight;
	
	effectRect=eWinRec->selectedRect;
	
	/* ペイント領域のみをクリッピング */
	SetPortWindowPort(theWindow);
	clipRgn=NewRgn();
	GetClip(clipRgn);
	ClipRect(&paintRect);
	
	err=NewGWorld(&maskGWorld,1,&eWinRec->iconSize,nil,nil,useTempMem);
	if (err==noErr)
	{
		GetGWorld(&cPort,&cDevice);
		SetGWorld(maskGWorld,0);
		LockGWorldPixels(maskGWorld);
		EraseRect(&eWinRec->iconSize);
		GetRegionBounds(eWinRec->selectionPos,&tempRect);
		CopyBits(GetPortBitMapForCopyBits(eWinRec->currentMask),
			GetPortBitMapForCopyBits(maskGWorld),
			&effectRect,&tempRect,srcCopy,eWinRec->selectionPos);
		if (eWinRec->isSelected)
			CopyBits(GetPortBitMapForCopyBits(eWinRec->selectionMask),
				GetPortBitMapForCopyBits(maskGWorld),
				&eWinRec->iconSize,&eWinRec->iconSize,srcOr,nil);
		UnlockGWorldPixels(maskGWorld);
		
		SetGWorld(cPort,cDevice);
		/* プレビュー */
		SetPortWindowPort(gPreviewWindow);
		CopyBits(GetPortBitMapForCopyBits(maskGWorld),
			GetPortBitMapForCopyBits(GetWindowPort(gPreviewWindow)),
			&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
		
		/* ウィンドウ */
		SetPortWindowPort(theWindow);
		CopyBits(GetPortBitMapForCopyBits(maskGWorld),
			GetPortBitMapForCopyBits(GetWindowPort(theWindow)),
			&eWinRec->iconSize,&eWinRec->iconEditSize,srcCopy,nil);
		
		DisposeGWorld(maskGWorld);
	}
	else
	{
		/* エラーが起こった場合は直接描画 */
		/* プレビュー */
		SetPortWindowPort(gPreviewWindow);
		EraseRect(&eWinRec->iconSize);
		GetRegionBounds(eWinRec->selectionPos,&tempRect);
		CopyBits(GetPortBitMapForCopyBits(eWinRec->currentMask),
			GetPortBitMapForCopyBits(GetWindowPort(gPreviewWindow)),
			&effectRect,&tempRect,srcCopy,eWinRec->selectionPos);
		if (eWinRec->isSelected)
			CopyBits(GetPortBitMapForCopyBits(eWinRec->selectionMask),
				GetPortBitMapForCopyBits(GetWindowPort(gPreviewWindow)),
				&eWinRec->iconSize,&eWinRec->iconSize,srcOr,nil);
		
		/* ウィンドウ */
		SetPortWindowPort(theWindow);
		MapRect(&tempRect,&eWinRec->iconSize,&eWinRec->iconEditSize);
		
		EraseRect(&paintRect);
		CopyBits(GetPortBitMapForCopyBits(eWinRec->currentMask),
			GetPortBitMapForCopyBits(GetWindowPort(theWindow)),
			&effectRect,&tempRect,srcCopy,nil);
		if (eWinRec->isSelected)
			CopyBits(GetPortBitMapForCopyBits(eWinRec->selectionMask),
				GetPortBitMapForCopyBits(GetWindowPort(theWindow)),
				&eWinRec->iconSize,&eWinRec->iconEditSize,srcOr,nil);
	}
	
	#if TARGET_API_MAC_CARBON
		/* ウィンドウの内容を強制アップデート */
		QDFlushPortBuffer(GetWindowPort(theWindow),nil);
		QDFlushPortBuffer(GetWindowPort(gPreviewWindow),nil);
	#endif
	
	GetKeys(theKeys);
	while (BitTst(theKeys,24))
		GetKeys(theKeys);
	
	SetClip(clipRgn);
	RedrawPaintWindow(theWindow,clipRgn);
	#if TARGET_API_MAC_CARBON
		QDFlushPortBuffer(GetWindowPort(theWindow),nil);
//		ValidWindowRgn(theWindow,clipRgn);
		RedrawPaintWindow(theWindow,clipRgn);
		QDFlushPortBuffer(GetWindowPort(theWindow),nil);
		ValidWindowRgn(theWindow,clipRgn);
	#endif
	DisposeRgn(clipRgn);
	SetPortWindowPort(gPreviewWindow);
	RedrawPreviewWindow(theWindow);
	#if TARGET_API_MAC_CARBON
		QDFlushPortBuffer(GetWindowPort(gPreviewWindow),nil);
//		ValidWindowRect(gPreviewWindow,&eWinRec->iconSize);
		RedrawPreviewWindow(theWindow);
		QDFlushPortBuffer(GetWindowPort(gPreviewWindow),nil);
		ValidWindowRect(gPreviewWindow,&eWinRec->iconSize);
	#endif
	SetPort(port);
	ShowCursor();
//	MyInvalWindowRect(theWindow,&paintRect);
//	SetClip(clipRgn);
//	DisposeRgn(clipRgn);
//	SetPortWindowPort(gPreviewWindow);
//	MyInvalWindowPortBounds(gPreviewWindow);
//	SetPort(port);
//	ShowCursor();
	
	FlushEvents(mDownMask+keyDownMask,0);
	return;
}

/* ウィンドウのズーム */
void MyZoomWindow(WindowPtr theWindow,short thePart,const EventRecord *theEvent)
{
	switch (GetExtWindowKind(theWindow))
	{
		case kWindowTypePaintWindow:
			if (TrackBox(theWindow,theEvent->where,thePart))
			{
				ResetRuler();
				ZoomPaintWindow(theWindow,thePart);
			}
			break;
		
		case kWindowTypeIconFamilyWindow:
			if (isIconServicesAvailable)
				if (TrackBox(theWindow,theEvent->where,thePart))
				{
					IconFamilyWinRec	*fWinRec=GetIconFamilyRec(theWindow);
					Rect	r;
					
					fWinRec->show32Icons=!fWinRec->show32Icons;
					if (fWinRec->show32Icons)
						SizeWindow(theWindow,
							fWinRec->largeWindowWidth,fWinRec->largeWindowHeight,true);
					else
						SizeWindow(theWindow,
							kFamilyWindowSmallWidth,kFamilyWindowSmallHeight,true);
					GetWindowPortBounds(theWindow,&r);
					SetPortWindowPort(theWindow);
					ClipRect(&r);
					//MyInvalWindowPortBounds(theWindow);
				}
			break;
	}
}

/* ウィンドウのクリック */
void ContentClick(EventRecord *theEvent)
{
	WindowPtr	theWindow;
	Point	mousePt;
	short	thePart;
	Boolean	optDown;
	Boolean	cmdDown;
	Boolean ctrlDown;
	
	mousePt=theEvent->where;
	thePart=FindWindow(mousePt,&theWindow);
	
	SetPortWindowPort(theWindow);
	
	GlobalToLocal(&mousePt);
	
	/* モディファイアキーの状態 */
	optDown=((theEvent->modifiers & optionKey)!=0);
	cmdDown=((theEvent->modifiers & cmdKey)!=0);
	ctrlDown=((theEvent->modifiers & controlKey)!=0);
	
	switch (GetExtWindowKind(theWindow))
	{
		case kWindowTypePaintWindow:
			ClickPaintWindow(theWindow,mousePt,theEvent);
			break;
		
		case kWindowTypePreviewWindow:
			/* 固定範囲選択機能？ */
			break;
		
		case kWindowTypeIconListWindow:
			ClickIconWindow(theWindow,mousePt,theEvent);
			break;
		
		case kWindowTypeIconFamilyWindow:
			ClickFamilyWindow(theWindow,mousePt,theEvent);
			break;
		
		case kWindowTypeColorPalette1:
			if (ctrlDown)
				ColorPalettePopup(mousePt);
			else
				ClickColorPalette1(mousePt,optDown,cmdDown);
			break;
		
		case kWindowTypeColorPalette2:
			if (ctrlDown)
				ColorPalettePopup(mousePt);
			else
				ClickColorPalette2(mousePt,optDown,cmdDown);
			break;
		
		case kWindowTypeToolPalette:
			ClickToolPalette(mousePt,theEvent->when);
			break;
		
		case kWindowTypeBlendPalette:
			ClickBlendPalette(mousePt,optDown,cmdDown,ctrlDown);
			break;
		
		case kWindowTypeTitleWindow:
			if (mousePt.h<0 || mousePt.v<0) break;
			About();
			break;
		
		case kWindowTypeInfoWindow:
			ClickInfoWindow(mousePt);
			break;
		
		case kWindowTypePatternPalette:
			ClickPatternPalette(mousePt);
			break;
		
		case kWindowTypeDotModePalette:
			ClickDotModePalette(mousePt);
			break;
		
		case kWindowTypeFavoritePalette:
			ClickFavoritePalette(mousePt,optDown,cmdDown);
			break;
		
		default:
			SysBeep(0);
			break;
	}
	
	SetPortWindowPort(MyFrontNonFloatingWindow());
}

/* ペイントウィンドウのクリック */
static void ClickPaintWindow(WindowPtr theWindow,Point localPt,EventRecord *theEvent)
{
	Rect	editRect;
	Boolean	optDown=((theEvent->modifiers & optionKey)!=0);
	Boolean	cmdDown=((theEvent->modifiers & cmdKey)!=0);
	Boolean	ctrlDown=((theEvent->modifiers & controlKey)!=0);
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	
	GetWindowPortBounds(theWindow,&editRect);
	editRect.right-=kScrollBarWidth;
	editRect.bottom-=kScrollBarHeight;
	if (isDragMgrPresent && PtInRect(localPt,&editRect) && 
		(gSelectedTool==kMarqueeTool || MyFrontNonFloatingWindow()!=theWindow))
	{
		eWinRec=GetPaintWinRec(theWindow);
		
		if (cmdDown && !optDown && PtInRgn(localPt,eWinRec->eSelectedRgn))
		{
			/* Dragを開始する */
			ResetRuler();
			MyDoStartDragPaintWin(theWindow,theEvent);
			return;
		}
	}
	
	if (MyFrontNonFloatingWindow()!=theWindow)
	{
		SelectReferencedWindow(theWindow);
		UpdateMenus();
		return;
	}
	
	if (PtInRect(localPt,&editRect))
	{
		ClickPaintWindowMain(theWindow,localPt,theEvent->modifiers);
	}
	else
	{
		/* change ratio or background popup */
		Rect	ratioRect,backRect;
		
		GetWindowPortBounds(theWindow,&ratioRect);
		SetRect(&ratioRect,ratioRect.left,
					ratioRect.bottom-kScrollBarHeight,
					ratioRect.left+kRatioWidth,
					ratioRect.bottom);
		SetRect(&backRect,ratioRect.left+kRatioWidth,
					ratioRect.top,
					ratioRect.right+kBackWidth,
					ratioRect.bottom);
		if (PtInRect(localPt,&ratioRect)) /* 倍率 */
		{
			Point	popPt;
			
			SetPt(&popPt,ratioRect.left,ratioRect.top);
			LocalToGlobal(&popPt);
			ChangeRatio(theWindow,popPt);
		}
		else if (PtInRect(localPt,&backRect)) /* バックグラウンド */
		{
			UInt32	time=TickCount()+GetDblTime();
			Point	popPt;
			
			if (!ctrlDown)
			{
				while (StillDown() && PtInRect(localPt,&backRect) && TickCount()<time)
					GetMouse(&localPt);
				if (!PtInRect(localPt,&backRect)) return; /* 範囲外に出た時はなにもしない */
				if (!StillDown())
				{
					GoBackgroundMode(theWindow);
					return;
				}
			}
			SetPt(&popPt,backRect.left,backRect.top+1);
			LocalToGlobal(&popPt);
			BackgroundPop(theWindow,popPt);
		}
		else /* スクロールバー */
			DoScrollPaintWindow(theWindow,localPt);
	}
}

/* ペイントウィンドウをクリック（メイン） */
void ClickPaintWindowMain(WindowPtr theWindow,Point pt,long modifiers)
{
	short	tool;
	Point	mousePt;
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	Boolean	cmdDown=((modifiers & cmdKey)!=0);
	Boolean optDown=((modifiers & optionKey)!=0);
	Boolean shiftDown=((modifiers & shiftKey)!=0);
	Boolean ctrlDown=((modifiers & controlKey)!=0);
	
	/* ドット描画モードなら、抜ける */
	if (eWinRec->isDotMode)
	{
		ResetDot(theWindow);
		eWinRec->isDotMode=false;
		HideReferencedWindow(DotModePalette);
		UpdateEffectMenu();
		return;
	}
	
	/* ルーラ表示がオンなら、表示されているルーラを消去 */
	ResetRuler();
	
	mousePt=pt;
	if (ctrlDown && !cmdDown)
	{
		/* コンテクストメニュー */
		MenuHandle	contextMenu;
		long		menuChoice=0;
		
		LocalToGlobal(&mousePt);
		contextMenu=GetMenu(131);
		InsertMenu(contextMenu,-1);
		
		if (isContextualMenuAvailable)
		{
			UInt32		selectionType;
			SInt16		menuID;
			UInt16		menuItem;
			OSStatus	err;
			err=ContextualMenuSelect(contextMenu,mousePt,false,kCMHelpItemNoHelp,"\p",nil,
										&selectionType,&menuID,&menuItem);
			
			if (err==noErr && selectionType==kCMMenuItemSelected)
				menuChoice=((long)menuID<<16)+menuItem;
		}
		else
			menuChoice=PopUpMenuSelectWFontSize(contextMenu,mousePt.v,mousePt.h+1,0,9);
		
		if (menuChoice!=0)
			HandleMenuChoice(menuChoice);
		
		SetPt(&mousePt,0,0);
		UpdateMouseCursor(mousePt);
		
		return;
	}
	
	if (optDown && cmdDown)
	{
		GrabScrollPaintWindow(theWindow,mousePt);
		return;
	}
	
	tool=gSelectedTool;
	
	/* タブレットの消しゴムを使っているときは消しゴム */
	if (isTabletAvailable && IsTabletInUse() && gTabletPrefs.useEraser && IsEraser())
	{
		tool=kEraserTool;
		
		if (eWinRec->isSelected && tool!=kMarqueeTool&& !gToolPrefs.selectionMasking)
			FixSelection(theWindow);
	}
	
	/* オプションを押していればスポイト */
	if (tool!=kMarqueeTool && optDown)
		tool=kSpoitTool;
	
	switch (tool)
	{
		case kPencilTool:
			SetPortWindowPort(theWindow);
			if (cmdDown)
				if (gToolPrefs.eraserByCmdKey && !ctrlDown) /* cmd+ctrl+pencil -> dot mode */
					DoErase(theWindow,mousePt,shiftDown);
				else
					DoDotPaint(theWindow,mousePt);
			else
				DoPaint(theWindow,mousePt,shiftDown);
			break;
		
		case kSpoitTool:
			DoSpoit(theWindow,mousePt);
			break;
		
		case kEraserTool:
			SetPortWindowPort(theWindow);
			DoErase(theWindow,mousePt,shiftDown);
			break;
		
		case kMarqueeTool:
			DoSelect(theWindow,mousePt,shiftDown,optDown,cmdDown);
			break;
		
		case kBucketTool:
			DoBucket(theWindow,mousePt,cmdDown);
			break;
		
		default:
			
			break;
	}
}

/* click at color palette 1 */
void ClickColorPalette1(Point mousePt,Boolean optDown,Boolean cmdDown)
{
	RGBColor	newColor;
	
	if (mousePt.h<0 || mousePt.v<0) return;
	
	if ((mousePt.h-1)%62 < 60 && mousePt.h < 372 && mousePt.v <= 60)
	{
		newColor.red=(5-(mousePt.h-1)/62)*0x3333U;
		newColor.green=(5-(mousePt.v-1)/10)*0x3333U;
		newColor.blue=(5-((mousePt.h-1)%62)/10)*0x3333U;
		
		if (cmdDown)
		{
			WindowPtr	theWindow=MyFrontNonFloatingWindow();
			
			if (theWindow != nil && GetExtWindowKind(theWindow) == kWindowTypePaintWindow)
				DoReplaceColor(theWindow,&gCurrentColor.rgb,gCurrentColor.isTransparent,&newColor,false);
			ChangeColor(&newColor,false);
		}
		else
		{
			if (optDown)
				ChangeBackColor(&newColor,false);
			else
			{
				UInt32	clickTime=GetDblTime()+TickCount();
				Rect	box;
				Point	newPt;
				EventRecord	followEvent;
				Boolean	isDoubleClick=false;
				
				ChangeColor(&newColor,false);
				
				/* ダブルクリック判定 */
				SetRect(&box,mousePt.h-2,mousePt.v-2,mousePt.h+2,mousePt.v+2);
				while(TickCount()<clickTime && !isDoubleClick)
				{
					GetMouse(&newPt);
					if (!PtInRect(newPt,&box)) break;
					isDoubleClick=EventAvail(mDownMask,&followEvent);
				}
				
				if (isDoubleClick)
				{
					FlushEvents(mDownMask,0);
					DoSelectColorArea(MyFrontNonFloatingWindow(),&newColor,false);
				}
			}
		}
	}
}

/* click at color palette 2 */
void ClickColorPalette2(Point mousePt,Boolean optDown,Boolean cmdDown)
{
	unsigned short	temp;
	RGBColor	newColor;
	Boolean		newTransparent=false;
	
	if (mousePt.v > 160 || mousePt.h<0 || mousePt.v<0) return;
	
	temp=(15-(mousePt.v-1)/10)*0x1111U;
	switch ((mousePt.h-1)/12)
	{
		case 0:
			if (temp == 0) /* 透明色 */
			{
				newColor=rgbWhiteColor;
				newTransparent=true;
			}
			else
			{
				newColor.red=temp;
				newColor.green=0;
				newColor.blue=0;
			}
			break;
		case 1:
			newColor.red=0;
			newColor.green=temp;
			newColor.blue=0;
			break;
		case 2:
			newColor.red=0;
			newColor.green=0;
			newColor.blue=temp;
			break;
		case 3:
			newColor.red=temp;
			newColor.green=temp;
			newColor.blue=temp;
			break;
		default:
			return;
			break;
	}
	
	if (cmdDown)
	{
		WindowPtr	theWindow=MyFrontNonFloatingWindow();
		
		if (theWindow != nil && GetExtWindowKind(theWindow) == kWindowTypePaintWindow)
			DoReplaceColor(theWindow,&gCurrentColor.rgb,gCurrentColor.isTransparent,&newColor,newTransparent);
		ChangeColor(&newColor,newTransparent);
	}
	else
	{
		if (optDown)
			ChangeBackColor(&newColor,newTransparent);
		else
		{
			UInt32	clickTime=GetDblTime()+TickCount();
			Rect	box;
			Point	newPt;
			EventRecord	followEvent;
			Boolean	isDoubleClick=false;
			
			ChangeColor(&newColor,newTransparent);
			
			/* ダブルクリック判定 */
			SetRect(&box,mousePt.h-2,mousePt.v-2,mousePt.h+2,mousePt.v+2);
			while(TickCount()<clickTime && !isDoubleClick)
			{
				GetMouse(&newPt);
				if (!PtInRect(newPt,&box)) break;
				isDoubleClick=EventAvail(mDownMask,&followEvent);
			}
			
			if (isDoubleClick)
			{
				FlushEvents(mDownMask,0);
				DoSelectColorArea(MyFrontNonFloatingWindow(),&newColor,newTransparent);
			}
		}
	}
}

/* click at favorite palette */
void ClickFavoritePalette(Point mousePt,Boolean optDown,Boolean cmdDown)
{
	RGBColor	newColor;
	short		i,colorNum;
	ColorSpec	*cspec;
	
	if (mousePt.h<0 || mousePt.v<0) return;
	
	colorNum = (*gFavoriteColors)->ctSize + 1;
	cspec = &((*gFavoriteColors)->ctTable[0]);
	
	i = (mousePt.h-1)/10 + ((mousePt.v-1)/10)*16;
	if (i>=colorNum)
	{
		/* 追加 */
		if (i >= 96) return; /* 最大値 */
		if (gCurrentColor.isTransparent) return; /* 透過色は追加できない */
		
		if (ColorExistsInPal(&gCurrentColor.rgb,gFavoriteColors,colorNum))
			return; /* すでに登録されている色は追加できない */
		
		/* １つ追加できるだけの領域を確保 */
		SetHandleSize((Handle)gFavoriteColors,sizeof(ColorTable)+sizeof(ColorSpec)*colorNum);
		(*gFavoriteColors)->ctSize ++;
		cspec = &((*gFavoriteColors)->ctTable[0]);
		cspec[colorNum].rgb = gCurrentColor.rgb;
		cspec[colorNum].value = colorNum;
		
		MyInvalWindowPortBounds(FavoritePalette);
		return;
	}
	
	if (cmdDown)
	{
		/* 削除 */
		if (i < colorNum-1)
			BlockMoveData(&cspec[i+1],&cspec[i],sizeof(ColorSpec)*(colorNum-i-1));
		SetHandleSize((Handle)gFavoriteColors,sizeof(ColorTable)+sizeof(ColorSpec)*(colorNum-1-1));
		(*gFavoriteColors)->ctSize --;
		
		MyInvalWindowPortBounds(FavoritePalette);
		return;
	}
	else
	{
		newColor = cspec[i].rgb;
		
		if (optDown)
			ChangeBackColor(&newColor,false);
		else
		{
			UInt32	clickTime=GetDblTime()+TickCount();
			Rect	box;
			Point	newPt;
			EventRecord	followEvent;
			Boolean	isDoubleClick=false;
			
			ChangeColor(&newColor,false);
			
			/* ダブルクリック判定 */
			SetRect(&box,mousePt.h-2,mousePt.v-2,mousePt.h+2,mousePt.v+2);
			while(TickCount()<clickTime && !isDoubleClick)
			{
				GetMouse(&newPt);
				if (!PtInRect(newPt,&box)) break;
				isDoubleClick=EventAvail(mDownMask,&followEvent);
			}
			
			if (isDoubleClick)
			{
				FlushEvents(mDownMask,0);
				DoSelectColorArea(MyFrontNonFloatingWindow(),&newColor,false);
			}
		}
	}
}

/* click at tool palette */
void ClickToolPalette(Point mousePt,UInt32 clickTime)
{
	short	toolTemp;
	short	newTool;
	Rect	box;
	Point	newPt;
	Boolean	isDoubleClick=false;
	EventRecord	followEvent;
	
	if (mousePt.h<0 || mousePt.v<0) return;
		
	toolTemp=gSelectedTool;
	newTool=(mousePt.v-2)/25;
	SetRect(&box,mousePt.h-2,mousePt.v-2,mousePt.h+2,mousePt.v+2);
	
	ToolSelect(newTool);
	
	/* double click? */
	while(TickCount()<clickTime+GetDblTime() && !isDoubleClick)
	{
		GetMouse(&newPt);
		if (!PtInRect(newPt,&box)) break;
		isDoubleClick=EventAvail(mDownMask,&followEvent);
	}
	
	if (isDoubleClick)
	{
		FlushEvents(mDownMask,0);
		DoubleClickTool(newTool);
		if (newTool==kEraserTool || newTool==kSpoitTool) ToolSelect(toolTemp);
	}
}

const static Rect	cColorRect[]={{2,2,16,16},{2,19,16,33}};
const static Rect	lockRect={18,1,26,34};
const static Rect	tColorRect[5]= {{28,2,42,16},{43,2,57,16},{58,2,72,16},
									{73,2,87,16},{88,2,102,16}};
const static Rect	bColorRect[5]= {{28,19,42,33},{43,19,57,33},{58,19,72,33},
									{73,19,87,33},{88,19,102,33}};
const static Rect	kExchangeRect={13,13,17,22};

/* ブレンドパレットのクリック */
void ClickBlendPalette(Point mousePt,Boolean optDown,Boolean cmdDown,Boolean ctrlDown)
{
	RGBColor	newColor;
	Point		pt={-1,-1};
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	if (mousePt.h<0 || mousePt.v<0) return;
	
	/* ロック */
	if (PtInRect(mousePt,&lockRect))
	{
		UInt32	time=TickCount()+GetDblTime();
		Point	popPt;
		
		if (!ctrlDown)
		{
			while (StillDown() && PtInRect(mousePt,&lockRect) && TickCount()<time)
				GetMouse(&mousePt);
			if (!PtInRect(mousePt,&lockRect)) return; /* 範囲外に出た時は何もしない */
			if (!StillDown()) /* ボタンが離されている場合はロックの切り替え */
			{
				HandleBlendPaletteChoice(iBlendLock);
				return;
			}
		}
		popPt=mousePt;
		DoBlendPop(popPt);
			
		return;
	}
	
	/* 交換 */
	if (PtInRect(mousePt,&kExchangeRect))
	{
		if (!EqualColor(&gCurrentColor.rgb,&gBackColor.rgb) || gCurrentColor.isTransparent != gBackColor.isTransparent)
		{
			ChangeColor(&gBackColor.rgb,gBackColor.isTransparent);
			ChangeBackColor(&gPrevColor.rgb,gPrevColor.isTransparent);
		}
		return;
	}
	
	/* 前景色 */
	if (PtInRect(mousePt,&cColorRect[0]))
	{
		Str255	prompt;
		Point	where={-1,-1};
		RGBColor	newColor;
		
		GetIndString(prompt,154,1);
		if (GetColor(where,prompt,&gCurrentColor.rgb,&newColor))
		{
			FixColor(&newColor);
			ChangeColor(&newColor,false);
		}
		return;
	}
	
	/* 背景色 */
	if (PtInRect(mousePt,&cColorRect[1]))
	{
		Str255	prompt;
		Point	where={-1,-1};
		RGBColor	newColor;
		
		GetIndString(prompt,154,2);
		if (GetColor(where,prompt,&gBackColor.rgb,&newColor))
		{
			FixColor(&newColor);
			ChangeBackColor(&newColor,false);
		}
		return;
	}
	
	/* それ以外の場合 */
	if (ctrlDown)
	{
		ColorPalettePopup(mousePt);
		return;
	}
	
	{
		short	i;
		
		for (i=0; i<5; i++)
		{
			if (PtInRect(mousePt,&tColorRect[i]))
			{
				SetPt(&pt,tColorRect[i].left,tColorRect[i].top);
				break;
			}
			else if (PtInRect(mousePt,&bColorRect[i]))
			{
				SetPt(&pt,bColorRect[i].left,bColorRect[i].top);
				break;
			}
		}
	}
	
	if (pt.h<0) return;	
	
	GetGWorld(&cPort,&cDevice);
	SetGWorld(gBlendPalettePtr,0);
	LockPixels(GetGWorldPixMap(gBlendPalettePtr));
	GetCPixel(pt.h,pt.v,&newColor);
	UnlockPixels(GetGWorldPixMap(gBlendPalettePtr));
	SetGWorld(cPort,cDevice);
	
	if (cmdDown)
	{
		WindowPtr	theWindow=MyFrontNonFloatingWindow();
		
		if (theWindow != nil && GetExtWindowKind(theWindow) == kWindowTypePaintWindow)
			DoReplaceColor(theWindow,&gCurrentColor.rgb,gCurrentColor.isTransparent,&newColor,false);
		ChangeColor(&newColor,false);
	}
	else
	{
		if (optDown)
			ChangeBackColor(&newColor,false);
		else
			ChangeColor(&newColor,false);
	}
}

/* 情報ウィンドウのクリック */
void ClickInfoWindow(Point mousePt)
{
	if (mousePt.h<0 || mousePt.v<0) return;
	
	if (mousePt.v<16)
	{
		if (mousePt.h<16)
			ChangePenSize();
		else
			ChangeEraserSize();
	}
	else
		ChangeTransparency();
}

/* パターンパレットのクリック */
void ClickPatternPalette(Point localPt)
{
	short	n;
	short	w=(gOtherPrefs.useHyperCardPalette ? 10 : 6);
	
	if (localPt.h<0 || localPt.v<0) return;
	
	n=1+(localPt.h/(13+3))+(localPt.v/(13+3))*w;
	
	if (n > w*4) return;
	
	HilitePattern(gPatternNo,false);
	HilitePattern(n,true);
	gPatternNo=n;
}

/* カラーパレットのコントロール＋クリック */
void ColorPalettePopup(Point localPt)
{
	MenuHandle	menu;
	long		selItem;
	short		item;
	Point		globPt=localPt;
	
	menu=GetMenu(155);
	InsertMenu(menu,-1);
	CheckMenuItem(menu,gPaletteCheck+1,true);
	
	LocalToGlobal(&globPt);
	selItem=PopUpMenuSelectWFontSize(menu,globPt.v,globPt.h,gPaletteCheck+1,9);
	
	CheckMenuItem(menu,gPaletteCheck+1,false);
	DeleteMenu(155);
	
	item=LoWord(selItem);
	if (item==0 || item == gPaletteCheck+1) return;
	
	HandlePaletteChoice(item);
}

/* ブレンドパレットのポップアップメニュー */
void DoBlendPop(Point localPt)
{
	MenuHandle	menu;
	long		selItem;
	short		item;
	
	LocalToGlobal(&localPt);
	
	menu=GetMenu(mBlendPalettePop);
	InsertMenu(menu,-1);
	
	CheckMenuItem(menu,iBlendLock,gBlendLocked);
	
	selItem=PopUpMenuSelectWFontSize(menu,localPt.v,localPt.h,0,9);
	if (selItem > 0)
	{
		item=LoWord(selItem);
		HandleBlendPaletteChoice(item);
	}
	DeleteMenu(mBlendPalettePop);
	ReleaseResource((Handle)menu);
}

/* ブレンドパレットの中身 */
void DrawBlend(void)
{
	RGBColor	tempColor,tempColor2;
	short	i;
	PicHandle	pic;
	RgnHandle	clipRgn=NewRgn();
	CIconHandle	cIconH;
	Rect		tempRect;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	const Rect	iconRect[]={{1,1,9,9},{1,18,9,26}};
	
	GetGWorld(&cPort,&cDevice);
	SetGWorld(gBlendPalettePtr,0);
	LockPixels(GetGWorldPixMap(gBlendPalettePtr));
	
	GetClip(clipRgn);
	if (gBlendLocked)
	{
		Rect	u;
		
		UnionRect(&cColorRect[0],&lockRect,&u);
		UnionRect(&cColorRect[1],&u,&u);
		ClipRect(&u);
	}
	GetPortBounds(gBlendPalettePtr,&tempRect);
	EraseRect(&tempRect);
	pic=GetPicture(152);
	DrawPicture(pic,&tempRect);
	
	RGBForeColor(&gCurrentColor.rgb);
	
	/* 前景色 */
	PaintRect(&cColorRect[0]);
	cIconH=GetCIcon(1001);
	PlotCIcon(&iconRect[0],cIconH);
	DisposeCIcon(cIconH);
	if (gCurrentColor.isTransparent) /* 透明色 */
	{
		Str15	tStr="\pT";
		
		ForeColor(blackColor);
		TextSize(9);
		MoveTo(cColorRect[0].right-StringWidth(tStr)-1,cColorRect[0].bottom-1);
		DrawString(tStr);
	}
	
	/* 背景色 */
	RGBForeColor(&gBackColor.rgb);
	PaintRect(&cColorRect[1]);
	cIconH=GetCIcon(1002);
	PlotCIcon(&iconRect[1],cIconH);
	DisposeCIcon(cIconH);
	if (gBackColor.isTransparent) /* 透明色 */
	{
		Str15	tStr="\pT";
		
		ForeColor(blackColor);
		TextSize(9);
		MoveTo(cColorRect[1].right-StringWidth(tStr)-1,cColorRect[1].bottom-1);
		DrawString(tStr);
	}
	
	/* 交換の絵 */
	pic=GetPicture(154);
	DrawPicture(pic,&kExchangeRect);
	
	/* ロックされているかどうか */
	if (gBlendLocked)
	{
		pic=GetPicture(153);
		DrawPicture(pic,&lockRect);
	}
	else
	{
		RGBForeColor(&gCurrentColor.rgb);
		for (i=0; i<5; i++)
			PaintRect(&tColorRect[i]);
		
		LightenRect(&tColorRect[0]);
		LightenRect(&tColorRect[0]);
		LightenRect(&tColorRect[1]);
		
		DarkenRect(&tColorRect[4]);
		DarkenRect(&tColorRect[4]);
		DarkenRect(&tColorRect[3]);
		
		tempColor=gCurrentColor.rgb;
		RGBForeColor(&tempColor);
		PaintRect(&bColorRect[0]);
		
		BlendColor(&tempColor,&gPrevColor.rgb);
		RGBForeColor(&tempColor);
		PaintRect(&bColorRect[2]);
		
		tempColor2=tempColor;
		BlendColor(&tempColor,&gCurrentColor.rgb);
		RGBForeColor(&tempColor);
		PaintRect(&bColorRect[1]);
		
		BlendColor(&tempColor2,&gPrevColor.rgb);
		RGBForeColor(&tempColor2);
		PaintRect(&bColorRect[3]);
		
		RGBForeColor(&gPrevColor.rgb);
		PaintRect(&bColorRect[4]);
		
		/* チェックマークをつける*/
		if (gPaletteCheck>0)
		{
			#if !TARGET_API_MAC_CARBON
			PicHandle	picture=GetPicture(131);
			CTabHandle	ctab;
			short		i;
			short		colorNum;
			
			ctab=GetCheckPalette(gPaletteCheck,&colorNum);
			if (ctab!=nil)
			{
				HLock((Handle)ctab);
				
				for (i=0; i<5; i++)
				{
					Point		pt;
					RGBColor	color;
					
					SetPt(&pt,tColorRect[i].left,tColorRect[i].top);
					GetCPixel(pt.h,pt.v,&color);
					if (ColorExistsInPal(&color,ctab,colorNum))
					{
						Rect	r=tColorRect[i];
						
						InsetRect(&r,5,5);
						DrawPicture(picture,&r);
					}
					
					SetPt(&pt,bColorRect[i].left,bColorRect[i].top);
					GetCPixel(pt.h,pt.v,&color);
					if (ColorExistsInPal(&color,ctab,colorNum))
					{
						Rect	r=bColorRect[i];
						
						InsetRect(&r,5,5);
						DrawPicture(picture,&r);
					}
				}
				
				HUnlock((Handle)ctab);
				if (gPaletteCheck == kPaletteCheckUsed)
					DisposeHandle((Handle)ctab);
				else
					DisposeCTable(ctab);
			}
			#endif
		}
	}
	
	SetClip(clipRgn);
	DisposeRgn(clipRgn);
	
	UnlockPixels(GetGWorldPixMap(gBlendPalettePtr));
	
	ForeColor(blackColor);
	
	SetGWorld(cPort,cDevice);
}

void DarkenRect(const Rect *rPtr)
{
	PenMode(subPin);
	OpColor(&rgbBlackColor);
	RGBForeColor(&rgbLBColor);
	
	PaintRect(rPtr);
	
	PenMode(srcCopy);
}

void LightenRect(const Rect *rPtr)
{
	PenMode(addPin);
	OpColor(&rgbWhiteColor);
	RGBForeColor(&rgbLBColor);
	
	PaintRect(rPtr);
	
	PenMode(srcCopy);
}

void BlendColor(RGBColor *dst,RGBColor *src)
{
	unsigned short	temp;
	
	temp=(dst->red >>1)+(src->red >>1);
	dst->red=temp;
	temp=(dst->green >>1)+(src->green >>1);
	dst->green=temp;
	temp=(dst->blue >>1)+(src->blue >>1);
	dst->blue=temp;
}

/* ブレンドパレット上でマウスカーソルの更新 */
void UpdateCursorInBlendPalette(Point localPt)
{
	Point		offsetPt={0,0};
	RgnHandle	arrowRgn=NewRgn();
	Rect		arrowRect;
	
	GetWindowPortBounds(gBlendPalette,&arrowRect);
	
	arrowRect.bottom=lockRect.bottom;
	
	/* グローバル座標変換用 */
	LocalToGlobal(&offsetPt);
	RectRgn(arrowRgn,&arrowRect);
	OffsetRgn(arrowRgn,offsetPt.h,offsetPt.v);
	
	if (PtInRect(localPt,&arrowRect)) /* ロックアイコン内 */
	{
		MySetCursor(0);
		CopyRgn(arrowRgn,gCurRgnHand);
	}
	else
	{
		RgnHandle	tempRgn=NewRgn();
		
		MySetCursor(gToolPrefs.changeSpoitCursor ? 139 : 128+kSpoitTool);
		GetWindowContentRgn(gBlendPalette,tempRgn);
		DiffRgn(tempRgn,arrowRgn,gCurRgnHand);
		DisposeRgn(tempRgn);
	}
	DisposeRgn(arrowRgn);
}

/* ブレンドパレット上でマウスカーソルの更新 */
void UpdateCursorInFavoritePalette(Point localPt,Boolean cmdDown)
{
	short		i,colorNum;
	ColorSpec	*cspec;
	
	colorNum = (*gFavoriteColors)->ctSize + 1;
	cspec = &((*gFavoriteColors)->ctTable[0]);
	
	i = (localPt.h-1)/10 + ((localPt.v-1)/10)*16;
	if (i>=colorNum)
		MySetCursor(128+kBucketTool);
	else
		if (cmdDown)
			MySetCursor(142);
		else
			MySetCursor((gToolPrefs.changeSpoitCursor ? 139 : 128+kSpoitTool));
	
	SetEmptyRgn(gCurRgnHand);
}

/* ルーラを描く（1.0b5大幅修正） */
void DrawRuler(Point pt)
{
	PatHandle	gray;
	Point		mousePt;
	GrafPtr		port;
	WindowPtr	theWindow,targetWin;
	short		part;
	Rect		winSize;
	short		ratio;
	RgnHandle	clipRgn,tempRgn;
	
	if (!IsFrontProcess()) return;
	
	#if !TARGET_API_MAC_CARBON
	if (isTSMgrAvailable)
	{
		part=FindServiceWindow(pt,&targetWin);
		if (targetWin!=nil)
		{
			ResetRuler();
			return;
		}
	}
	#endif
	
	theWindow=MyFrontNonFloatingWindow();
	part=FindWindow(pt,&targetWin);
	
	tempRgn=NewRgn();
	GetWindowUpdateRgn(theWindow,tempRgn);
	if (theWindow==nil || GetExtWindowKind(theWindow)!=kWindowTypePaintWindow || 
			!EmptyRgn(tempRgn))
	{
		SetPt(&gPrevRulerPt,-1,-1);
		SetEmptyRgn(gRulerRgn);
		DisposeRgn(tempRgn);
		return;
	}
	DisposeRgn(tempRgn);
	
	GetWindowPortBounds(theWindow,&winSize);
	winSize.right-=kScrollBarWidth;
	winSize.bottom-=kScrollBarHeight;
	ratio=(GetPaintWinRec(theWindow))->ratio;
	
	GetPort(&port);
	SetPortWindowPort(theWindow);
	mousePt=pt;
	GlobalToLocal(&mousePt);
	ToRealPos(&mousePt,ratio);
	ToGridPos(&mousePt,ratio);
	
	if (EqualPt(mousePt,gPrevRulerPt))
	{
		SetPort(port);
		return;
	}
	
	gray = GetPattern(128);
	PenPat(*gray);
	PenMode(patXor);
	
	if (theWindow == targetWin)
	{
		/* 新たなルーラを描く */
		if (PtInRect(mousePt,&winSize))
		{
			MoveTo(mousePt.h,0);
			Line(0,winSize.bottom-1);
			MoveTo(0,mousePt.v);
			Line(winSize.right-1,0);
		}
	}
	else
		SetPt(&mousePt,-1,-1);
	
	/* クリップを記憶しておく */
	clipRgn=NewRgn();
	GetClip(clipRgn);
	SetClip(gRulerRgn);
	
	/* 古いルーラを消す */
	if (PtInRect(gPrevRulerPt,&winSize))
	{
		MoveTo(gPrevRulerPt.h,0);
		Line(0,winSize.bottom-1);
		MoveTo(0,gPrevRulerPt.v);
		Line(winSize.right-1,0);
	}
	
	SetClip(clipRgn);
	GetPortVisibleRegion(GetWindowPort(theWindow),gRulerRgn);
	DisposeRgn(clipRgn);
	
	gPrevRulerPt=mousePt;
	
	PenNormal();
	
	SetPort(port);
}

/* ルーラ消去 */
void ResetRuler(void)
{
	Point	pt;
	
	if (gToolPrefs.showRuler)
	{
		SetPt(&pt,-1,-1);
		DrawRuler(pt);
	}
}

/* 編集ウィンドウに倍率表示 */
void DrawRatio(WindowPtr theWindow)
{
	short	ratio=(GetPaintWinRec(theWindow))->ratio;
	Str15	ratioStr;
	Rect	ratioRect;
	
	TextFont(applFont);
	TextSize(9);
	
	GetIndString(ratioStr,146,ratio+1);
	
	GetWindowPortBounds(theWindow,&ratioRect);
	ratioRect.top=ratioRect.bottom-14;
	ratioRect.right=ratioRect.left+kRatioWidth-1;
	
	EraseRect(&ratioRect);
	MoveTo(ratioRect.left+(kRatioWidth-StringWidth(ratioStr))/2,ratioRect.top+11);
	DrawString(ratioStr);
	
	MoveTo(ratioRect.right,ratioRect.top);
	LineTo(ratioRect.right,ratioRect.bottom);
}

/* 編集ウィンドウの倍率変更のポップアップメニューを表示 */
void ChangeRatio(WindowPtr theWindow,Point popPt)
{
	MenuHandle	popMenu;
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	short		ratio=eWinRec->ratio;
	long		selItem;
	short		item;
	
	popMenu=GetMenu(mMag);
	InsertMenu(popMenu,-1);
	CheckMenuItem(popMenu,ratio+1,true);
	
	selItem=PopUpMenuSelectWFontSize(popMenu,popPt.v,popPt.h,ratio+1,9);
	
	CheckMenuItem(popMenu,ratio+1,false);	
	DeleteMenu(mMag);
	
	item=LoWord(selItem);
	if (item==0 || item==ratio+1) return;
	
	/* 実際に変更する */
	ChangeRatioMain(theWindow,item-1);
}

/* 編集ウィンドウの倍率変更 */
void ChangeRatioMain(WindowPtr theWindow,short ratio)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	short		i;
	MenuHandle	menu=GetMenuHandle(mMag2);
	
	ResetDot(theWindow);
	
	/*　メニューのチェック */
	for (i=1; i<=5; i++)
		CheckMenuItem(menu,i,(i==ratio+1));
	
	/* 構造体のメンバを更新 */
	eWinRec->ratio=ratio;
	
	UpdatePaintWindow(theWindow);
}

/* ペイントウィンドウに背景情報表示 */
void DrawBackInfo(WindowPtr theWindow)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	Boolean		isBackMode=eWinRec->editBackground;
	Boolean		isBackAvailable=(eWinRec->backgroundGWorld!=nil || eWinRec->foregroundGWorld!=nil);
	PicHandle	backPic;
	Rect		backRect;
	
	backPic=GetPicture((isBackAvailable ? (isBackMode ? 146 : 145) : 147));
	GetWindowPortBounds(theWindow,&backRect);
	backRect.left+=kRatioWidth;
	backRect.top=backRect.bottom-14;
	backRect.right=backRect.left+kBackWidth-1;
	EraseRect(&backRect);
	DrawPicture(backPic,&backRect);
}

/* 背景情報の更新 */
void UpdateBackInfo(WindowPtr theWindow)
{
	GrafPtr	port;
	
	GetPort(&port);
	SetPortWindowPort(theWindow);
	DrawBackInfo(theWindow);
	SetPort(port);
}

/* バックグラウンドポップアップ */
void BackgroundPop(WindowPtr theWindow,Point popPt)
{
	MenuHandle	popMenu,subMenu;
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	Boolean		isBackAvailable=(eWinRec->backgroundGWorld != nil || eWinRec->foregroundGWorld != nil);
	long		selItem;
	short		menu,item;
	Str255		itemStr;
	short		i;
	
	/* メニューを用意 */
	popMenu=GetMenu(mBackground);
	InsertMenu(popMenu,-1);
	subMenu=GetMenu(mPopForeTrans);
	InsertMenu(subMenu,-1);
	
	/* 内容の初期化 */
	if (isBackAvailable)
	{
		MyEnableMenuItem(popMenu,iBUnionBack);
		MyEnableMenuItem(popMenu,iBDeleteFore);
		MyEnableMenuItem(popMenu,iBDeleteBack);
		MyEnableMenuItem(popMenu,iBHideBack);
		MyEnableMenuItem(popMenu,iBForeTransparency);
		
		GetIndString(itemStr,148,5);
		SetMenuItemText(popMenu,iBBackground,itemStr);
			
		GetIndString(itemStr,148,(eWinRec->dispBackground ? 1 : 2));
		SetMenuItemText(popMenu,iBHideBack,itemStr);
		
		for (i=iForeTrans100; i<=iForeTrans0; i++)
			CheckMenuItem(subMenu,i,(i-iForeTrans100+kForeTrans100 == eWinRec->foreTransparency));
	}
	else
	{
		GetIndString(itemStr,148,(gOtherPrefs.addForeground ? 3 : 4));
		SetMenuItemText(popMenu,iBBackground,itemStr);
		
		MyDisableMenuItem(popMenu,iBUnionBack);
		MyDisableMenuItem(popMenu,iBDeleteFore);
		MyDisableMenuItem(popMenu,iBDeleteBack);
		MyDisableMenuItem(popMenu,iBHideBack);
		MyDisableMenuItem(popMenu,iBForeTransparency);
	}
	
	selItem=PopUpMenuSelectWFontSize(popMenu,popPt.v,popPt.h,1,9);
	
	DeleteMenu(mBackground);
	DeleteMenu(mPopForeTrans);
	
	if (selItem==0) return;
	menu=HiWord(selItem);
	item=LoWord(selItem);
	
	switch (menu)
	{
		case mBackground:
			switch (item)
			{
				case iBBackground:
					GoBackgroundMode(theWindow);
					break;
				
				case iBUnionBack:
					UnionBackground(theWindow);
					break;
				
				case iBDeleteFore:
					DeleteForeground(theWindow);
					break;
				
				case iBDeleteBack:
					DeleteBackground(theWindow);
					break;
				
				case iBHideBack:
					ShowHideBackground(theWindow);
					break;
			}
			break;
		
		case mPopForeTrans:
			HandleForeTransparencyChoice(item);
			break;
	}
}

/* 編集ウィンドウの更新 */
void UpdatePaintWindow(WindowPtr theWindow)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	Rect		iconSize=eWinRec->iconSize;
	short		ratio=eWinRec->ratio,prevRatio=eWinRec->iconEditSize.right / iconSize.right;
	ControlHandle	hScroll=eWinRec->hScroll,vScroll=eWinRec->vScroll;
	short		width,height,prevWidth,prevHeight;
	short		hValue,vValue;
	Rect		tempRect;
	
	if (!prevRatio) prevRatio=1;
	
	SetRect(&eWinRec->iconEditSize,0,0,iconSize.right<<ratio,iconSize.bottom<<ratio);
	
	/* 編集画面の選択領域の更新 */
	if (eWinRec->isSelected)
	{
		CopyRgn(eWinRec->selectionPos,eWinRec->eSelectedRgn);
		MapRgn(eWinRec->eSelectedRgn,&eWinRec->iconSize,&eWinRec->iconEditSize);
	}
	if (!EmptyRgn(eWinRec->ePrevSelectedRgn))
	{
		CopyRgn(eWinRec->prevSelectionPos,eWinRec->ePrevSelectedRgn);
		MapRgn(eWinRec->ePrevSelectedRgn,&eWinRec->iconSize,&eWinRec->iconEditSize);
	}
	
	/* 変更前のウィンドウのサイズ */
	GetWindowPortBounds(theWindow,&tempRect);
	prevWidth=tempRect.right - tempRect.left - kScrollBarWidth;
	prevHeight=tempRect.bottom - tempRect.top - kScrollBarHeight;
	
	SetPaintWindowSize(theWindow,&iconSize,ratio);
	
	/* 変更後のウィンドウのサイズ */
	GetWindowPortBounds(theWindow,&tempRect);
	width=tempRect.right - tempRect.left - kScrollBarWidth;
	height=tempRect.bottom - tempRect.top - kScrollBarHeight;
	
	/* 変更前のスクロールされた量 */
	hValue=GetControlValue(hScroll);
	vValue=GetControlValue(vScroll);
	
	SetScrollBarMax(theWindow);
	SetControlValue(hScroll,(((prevWidth/2+hValue)<<ratio)/prevRatio)-width/2);
	SetControlValue(vScroll,(((prevHeight/2+vValue)<<ratio)/prevRatio)-height/2);
	SetOrigin(GetControlValue(hScroll),GetControlValue(vScroll));
	
	/* スクロールバーの位置とサイズを変更 */
	SetScrollBarRect(theWindow);
	
	GetWindowPortBounds(theWindow,&tempRect);
	ClipRect(&tempRect);
	MyInvalWindowRect(theWindow,&tempRect);
}

/* ペイントウィンドウサイズの変更 */
void SetPaintWindowSize(WindowPtr theWindow,Rect *iconSize,short ratio)
{
	short	width,height;
	Point	pt;
	Rect	tempRect,tempRect2;
	
	width=kScrollBarWidth+(iconSize->right<<ratio);
	height=kScrollBarHeight+(iconSize->bottom<<ratio);
	
	GetWindowPortBounds(theWindow,&tempRect);
	GetRegionBounds(GetGrayRgn(),&tempRect2);
	SetPt(&pt,width+tempRect.left,tempRect.top);
	LocalToGlobal(&pt);
	if (!PtInRgn(pt,GetGrayRgn()))
	{
		pt.h=tempRect2.right - 80;
		GlobalToLocal(&pt);
		width=pt.h - tempRect.left;
	}
	SetPt(&pt,tempRect.left,height+tempRect.top);
	LocalToGlobal(&pt);
	if (!PtInRgn(pt,GetGrayRgn()))
	{
		pt.v=tempRect2.bottom - 40;
		GlobalToLocal(&pt);
		height=pt.v - tempRect.top;
	}
	
	/* ウィンドウサイズを変更 */
	SizeWindow(theWindow,width,height,false);
}

/* スクロールバーの最大値を設定 */
void SetScrollBarMax(WindowPtr theWindow)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	short	width,height;
	Rect	tempRect;
	
	GetWindowPortBounds(theWindow,&tempRect);
	width=tempRect.right - tempRect.left;
	height=tempRect.bottom - tempRect.top;
	
	SetControlMaximum(eWinRec->hScroll,15+(eWinRec->iconSize.right<<eWinRec->ratio) - width);
	SetControlMaximum(eWinRec->vScroll,15+(eWinRec->iconSize.bottom<<eWinRec->ratio) - height);
}

/* 指定のフォントサイズでポップアップメニューを表示 */
long PopUpMenuSelectWFontSize(MenuHandle popMenu,short top,short left,short firstSelectedItem,short fontSize)
{
	#if !TARGET_API_MAC_CARBON
	short	sysFontSize;
	#endif
	long	result;
	
	#if !TARGET_API_MAC_CARBON
	#if TARGET_RT_MAC_CFM
	if (gSystemVersion < 0x0850)
	#endif
	{
		/* システムフォントを変更する */
		sysFontSize=LMGetSysFontSize();
		LMSetSysFontSize(fontSize);
		LMSetLastSPExtra(-1);
	}
	#if TARGET_RT_MAC_CFM
	else
		SetMenuFont(popMenu,systemFont,fontSize);
	#endif
	#else
		SetMenuFont(popMenu,systemFont,fontSize);
	#endif
	
	/* ポップアップメニューを表示する */
	result=PopUpMenuSelect(popMenu,top,left,firstSelectedItem);
	
	#if !TARGET_API_MAC_CARBON
	#if TARGET_RT_MAC_CFM
	if (gSystemVersion < 0x0850)
	#endif
	{
		/* システムフォントを元に戻す */
		LMSetSysFontSize(sysFontSize);
		LMSetLastSPExtra(-1);
	}
	#endif
	
	return result;
}

/* グリッドの描画（暫定バージョン） */
void DrawGrid(WindowPtr theWindow)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	short	ratio=eWinRec->ratio;
	Rect	iconSize=eWinRec->iconSize;
	
	DrawGridMain(ratio,&iconSize);
}

/* グリッド描画メイン */
void DrawGridMain(short ratio,Rect *iconSize)
{
	Pattern	gridPat;
	short	normalGrid=gToolPrefs.gridMode & 0x0f;
	short	iconGrid=(gToolPrefs.gridMode>>4) & 0x0f;
	
	if (normalGrid>1)
	{
		switch (ratio)
		{
			case 2:
				/* ４倍 */
				GetIndPattern(&gridPat,128,normalGrid);
				break;
			
			case 3:
			case 4:
				/* ８倍 */
				GetIndPattern(&gridPat,129,normalGrid);
				break;
		}
		if (ratio>=2)
		{
			Rect	iconEditSize=*iconSize;
			
			iconEditSize.right<<=ratio;
			iconEditSize.bottom<<=ratio;
			
			PenPat(&gridPat);
			RGBForeColor(&gToolPrefs.gridColor);
			PenMode(patOr);
			if (ratio > 3)
			{
				short a,h,w,i;
				
				h = iconEditSize.bottom;
				w = iconEditSize.right;
				a = 1<<ratio;
				
				for (i=0; i<=w; i+=a)
				{
					MoveTo(i,0);
					Line(0,h);
				}
				for (i=0; i<=h; i+=a)
				{
					MoveTo(0,i);
					Line(w,0);
				}
			}
			else
				PaintRect(&iconEditSize);
			ForeColor(blackColor);
			PenNormal();
		}
	}
	
	/* アイコン用のグリッド */
	if (iconGrid>0)
	{
		short	iconHeight=iconSize->bottom;
		short	iconWidth=iconSize->right;
		short	a,b,i;
		
		switch (ratio)
		{
			case 0:
			case 1:
				GetIndPattern(&gridPat,132,1);
				break;
			
			case 2:
				GetIndPattern(&gridPat,130,normalGrid);
				break;
			
			case 3:
			case 4:
				GetIndPattern(&gridPat,131,normalGrid);
				break;
		}
		
		PenPat(&gridPat);
		PenMode(patOr);
		RGBForeColor(&gToolPrefs.gridColor2);
		/* グリッドの描画 */
		a=5+1-iconGrid;
		b=a+ratio;
		for (i=1; i<=(iconWidth>>a); i++)
		{
			MoveTo((i<<b),0);
			Line(0,iconHeight<<ratio);
		}
		for (i=1; i<=(iconHeight>>a); i++)
		{
			MoveTo(0,(i<<b));
			Line(iconWidth<<ratio,0);
		}
		ForeColor(blackColor);
		PenNormal();
	}
}

/* ウィンドウ位置の保存 */
void SaveWindowPos(void)
{
	GrafPtr	port;
	Point	pt;
	Boolean	vis;
	OSErr	err;
	
	if (gPrefFileRefNum<=0) return;
	
	UseResFile(gPrefFileRefNum);
	
	GetPort(&port);
	
	/* カラーパレット（横）の位置 */
	SetPortWindowPort(ColorPalette1);
	SetPt(&pt,0,0);
	LocalToGlobal(&pt);
	err=AddDataToPrefs(&pt,sizeof(Point),'WPOS',kColorPalette1ResID,"\pColorPalette1 pos");
	
	/* カラーパレット（縦）の位置 */
	SetPortWindowPort(ColorPalette2);
	SetPt(&pt,0,0);
	LocalToGlobal(&pt);
	err=AddDataToPrefs(&pt,sizeof(Point),'WPOS',kColorPalette2ResID,"\pColorPalette2 pos");
	
	/* プレビュー画面の位置 */
	SetPortWindowPort(gPreviewWindow);
	SetPt(&pt,0,0);
	LocalToGlobal(&pt);
	err=AddDataToPrefs(&pt,sizeof(Point),'WPOS',kPreviewWindowResID,"\pPreviewWin pos");
	
	/* ツールパレットの位置 */
	SetPortWindowPort(gToolPalette);
	SetPt(&pt,0,0);
	LocalToGlobal(&pt);
	err=AddDataToPrefs(&pt,sizeof(Point),'WPOS',kToolPaletteResID,"\pToolPalette pos");
	
	/* ブレンドパレットの位置 */
	SetPortWindowPort(gBlendPalette);
	SetPt(&pt,0,0);
	LocalToGlobal(&pt);
	err=AddDataToPrefs(&pt,sizeof(Point),'WPOS',kBlendPaletteResID,"\pBlendPalette pos");
	
	/* タイトル画面の位置 */
	SetPortWindowPort(TitleWindow);
	SetPt(&pt,0,0);
	LocalToGlobal(&pt);
	err=AddDataToPrefs(&pt,sizeof(Point),'WPOS',kTitleWindowResID,"\pTitleWin pos");
	
	/* 情報ウィンドウの位置 */
	SetPortWindowPort(InfoWindow);
	SetPt(&pt,0,0);
	LocalToGlobal(&pt);
	err=AddDataToPrefs(&pt,sizeof(Point),'WPOS',kInfoWindowResID,"\pInfoWin pos");
	
	/* パターンパレットの位置 */
	SetPortWindowPort(PatternPalette);
	SetPt(&pt,0,0);
	LocalToGlobal(&pt);
	err=AddDataToPrefs(&pt,sizeof(Point),'WPOS',kPatternPaletteWindowResID,"\pPattern pos");
	
	/* 描画点パレットの位置 */
	SetPortWindowPort(DotModePalette);
	SetPt(&pt,0,0);
	LocalToGlobal(&pt);
	err=AddDataToPrefs(&pt,sizeof(Point),'WPOS',kDotModePaletteWindowResID,"\pDotModePalette pos");
	
	/* お気に入りパレットの位置 */
	SetPortWindowPort(FavoritePalette);
	SetPt(&pt,0,0);
	LocalToGlobal(&pt);
	err=AddDataToPrefs(&pt,sizeof(Point),'WPOS',kFavoritePaletteResID,"\pFavoritePalette pos");
	
	SetPort(port);
	
	/* ウィンドウが表示されているかどうかを記録する */
	/* カラーパレット１ */
	vis=IsWindowVisible(ColorPalette1);
	err=AddDataToPrefs(&vis,sizeof(Boolean),'WVis',kColorPalette1ResID,"\pColorPalette1 vis");
	
	/* カラーパレット２ */
	vis=IsWindowVisible(ColorPalette2);
	err=AddDataToPrefs(&vis,sizeof(Boolean),'WVis',kColorPalette2ResID,"\pColorPalette2 vis");
	
	/* ツールパレット */
	vis=IsWindowVisible(gToolPalette);
	err=AddDataToPrefs(&vis,sizeof(Boolean),'WVis',kToolPaletteResID,"\pToolPalette vis");
	
	/* ブレンドパレット */
	vis=IsWindowVisible(gBlendPalette);
	err=AddDataToPrefs(&vis,sizeof(Boolean),'WVis',kBlendPaletteResID,"\pBlendPalette vis");
	
	/* タイトル画面 */
	vis=IsWindowVisible(TitleWindow);
	err=AddDataToPrefs(&vis,sizeof(Boolean),'WVis',kTitleWindowResID,"\pTitleWin vis");
	
	/* 情報ウィンドウ */
	vis=IsWindowVisible(InfoWindow);
	err=AddDataToPrefs(&vis,sizeof(Boolean),'WVis',kInfoWindowResID,"\pInfoWin vis");
	
	/* パターンパレット */
	vis=IsWindowVisible(PatternPalette);
	err=AddDataToPrefs(&vis,sizeof(Boolean),'WVis',kPatternPaletteWindowResID,"\pPatternPalette vis");
	
	/* パターンパレット */
	vis=IsWindowVisible(FavoritePalette);
	err=AddDataToPrefs(&vis,sizeof(Boolean),'WVis',kFavoritePaletteResID,"\pFavoritePalette vis");
	
	UseResFile(gApplRefNum);
}

/* 各パレットの表示／非表示の切り替え */
void ShowHidePalette(WindowPtr theWindow,Boolean visible)
{
	MenuHandle	menu;
	
	menu=GetMenuHandle(mDisplay);
	
	if (visible)
		ShowReferencedWindow(theWindow);
	else
		HideReferencedWindow(theWindow);
	
	switch (GetExtWindowKind(theWindow))
	{
		case kWindowTypeColorPalette1:
			CheckMenuItem(menu,iDispColor1,visible);
			break;
		
		case kWindowTypeColorPalette2:
			CheckMenuItem(menu,iDispColor2,visible);
			break;
		
		case kWindowTypeToolPalette:
			CheckMenuItem(menu,iDispTool,visible);
			break;
		
		case kWindowTypeBlendPalette:
			CheckMenuItem(menu,iDispBlend,visible);
			break;
		
		case kWindowTypeTitleWindow:
			CheckMenuItem(menu,iDispTitle,visible);
			break;
		
		case kWindowTypeInfoWindow:
			CheckMenuItem(menu,iDispInfo,visible);
			break;
		
		case kWindowTypePatternPalette:
			CheckMenuItem(menu,iDispPattern,visible);
			break;
		
		case kWindowTypeFavoritePalette:
			CheckMenuItem(menu,iDispFavorite,visible);
			break;
	}
}

/* パレットの表示状態の変更（メニュー選択による） */
void ShowHidePalette2(short item)
{
	MenuHandle	menu;
	WindowPtr	theWindow = NULL;
	Boolean		vis;
	
	menu=GetMenuHandle(mDisplay);
	
	switch (item)
	{
		case iDispTitle:
			theWindow=TitleWindow;
			break;
		
		case iDispTool:
			theWindow=gToolPalette;
			break;
		
		case iDispInfo:
			theWindow=InfoWindow;
			break;
		
		case iDispColor1:
			theWindow=ColorPalette1;
			break;
		
		case iDispColor2:
			theWindow=ColorPalette2;
			break;
		
		case iDispBlend:
			theWindow=gBlendPalette;
			break;
		
		case iDispPattern:
			theWindow=PatternPalette;
			break;
		
		case iDispFavorite:
			theWindow=FavoritePalette;
			break;
	}
	
	vis=!IsWindowVisible(theWindow);
	if (vis)
		ShowReferencedWindow(theWindow);
	else
		HideReferencedWindow(theWindow);
	
	CheckMenuItem(menu,item,vis);
}

/* show all palettes */
void ShowAllPalettes(void)
{
	ShowHidePalette(TitleWindow,true);
	ShowHidePalette(gToolPalette,true);
	ShowHidePalette(InfoWindow,true);
	ShowHidePalette(ColorPalette1,true);
	ShowHidePalette(ColorPalette2,true);
	ShowHidePalette(gBlendPalette,true);
	ShowHidePalette(PatternPalette,true);
	ShowHidePalette(FavoritePalette,true);
}

/* カラーパレットのチェックのアップデート */
void UpdatePaletteCheck(void)
{
	GrafPtr	port;
	
	GetPort(&port);
	MyInvalWindowPortBounds(ColorPalette1);
	MyInvalWindowPortBounds(ColorPalette2);
	DrawBlend();
	MyInvalWindowPortBounds(gBlendPalette);
	SetPort(port);
}

/* 選択色のハイライト */
void HiliteSelectedColor(short palNo,Boolean hilite)
{
	short	r,g,b;
	GrafPtr	port;
	short	x,y;
	Rect	box;
	WindowPtr	theWindow = NULL;
	Boolean		notRealColor;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	r=gCurrentColor.rgb.red & 0x0f;
	g=gCurrentColor.rgb.green & 0x0f;
	b=gCurrentColor.rgb.blue & 0x0f;
	
	GetPort(&port);
	
	if (gCurrentColor.isTransparent)
		notRealColor = false;
	else
	{
		/* システム256におさまる色かどうか */
		GetGWorld(&cPort,&cDevice);
		SetGWorld(gBlendPalettePtr,0);
		notRealColor=!RealColor(&gCurrentColor.rgb);
		SetGWorld(cPort,cDevice);
	}
	
	switch (palNo)
	{
		case 1:
			/* カラーパレット１ */
			if (notRealColor || r%3+g%3+b%3>0 || gCurrentColor.isTransparent) return;
			SetPortWindowPort((theWindow = ColorPalette1));
			r/=3; g/=3; b/=3;
			
			x=310+50-r*62-b*10+1;
			y=50-g*10+1;
			SetRect(&box,x,y,x+11,y+11);
			break;
		
		case 2:
			/* カラーパレット２ */
			if (notRealColor || !((r==0 && (g==0 || b==0)) || (g==0 && b==0) || (r==b && r==g)))
				return;
			SetPortWindowPort((theWindow = ColorPalette2));
			if (gCurrentColor.isTransparent)
			{
				y=151;
				x=1;
			}
			else
			{
				if (r==0 || g==0)
				{
					y=150-(r+g+b)*10+1;
					x=(r+g+b==0?36:0)+(g>0?12:0)+(b>0?24:0)+1;
				}
				else
				{
					y=150-r*10+1;
					x=36+1;
				}
			}
			SetRect(&box,x,y,x+11,y+11);
			break;
		
		case 3:
			/* お気に入り */
			if (gCurrentColor.isTransparent) return;
			
			SetPortWindowPort((theWindow = FavoritePalette));
			{
				short	colorNum = (*gFavoriteColors)->ctSize + 1;
				short	i;
				ColorSpec	*cspec=&((*gFavoriteColors)->ctTable[0]);
				
				for (i=0; i<colorNum; i++)
				{
					if (EqualColor(&gCurrentColor.rgb,&cspec[i].rgb))
					{
						SetRect(&box,(i%16)*10+1,(i/16)*10+1,(i%16)*10+12,(i/16)*10+12);
						break;
					}
				}
				if (i == colorNum)
				{
					SetPort(port);
					return;
				}
			}
			break;
	}
	
	if (hilite)
	{
		ForeColor(whiteColor);
		FrameRect(&box);
		ForeColor(blackColor);
		InsetRect(&box,-1,-1);
		FrameRoundRect(&box,1,1);
	}
	else
	{
		InsetRect(&box,-1,-1);
		MyInvalWindowRect(theWindow,&box);
	}
	
	SetPort(port);
}

/* カラーパレットにドットを表示 */
void DrawPaletteCheck(short palNo,short palKind)
{
	CTabHandle	ctab;
	PicHandle	dotPic;
	short		i;
	ColorSpecPtr	colorList;
	short		r,g,b;
	Rect		box;
	short		x,y;
	short		colorNum;
	
	ctab=GetCheckPalette(palKind,&colorNum);
	
	if (ctab==nil) return;
	
	dotPic=GetPicture(131);
	
	HLock((Handle)ctab);
	colorList=(**ctab).ctTable;
	
	for (i=0; i<colorNum; i++)
	{
		r=colorList[i].rgb.red & 0x0f;
		g=colorList[i].rgb.green & 0x0f;
		b=colorList[i].rgb.blue & 0x0f;
		
		switch (palNo)
		{
			case 1:
				if (r%3+g%3+b%3>0) continue;
				r/=3; g/=3; b/=3;
				
				x=310+50-r*62-b*10+1+4;
				y=50-g*10+1+4;
				SetRect(&box,x,y,x+3,y+3);
				DrawPicture(dotPic,&box);
				break;
			
			case 2:
				if (!((r==0 && (g==0 || b==0)) || (g==0 && b==0) || (r==b && r==g)))
					continue;
				if (r==0 || g==0)
				{
					y=150-(r+g+b)*10+1+4;
					x=(r+g+b==0?36:0)+(g>0?12:0)+(b>0?24:0)+1+4;
				}
				else
				{
					y=150-r*10+1+4;
					x=36+1+4;
				}
				SetRect(&box,x,y,x+3,y+3);
				DrawPicture(dotPic,&box);
		}
	}
	
	/* カラーパレットを破棄 */
	HUnlock((Handle)ctab);
	if (gPaletteCheck == kPaletteCheckUsed)
		DisposeHandle((Handle)ctab);
	else
		DisposeCTable(ctab);
}

/* 指定のパレットを得る */
CTabHandle GetCheckPalette(short palKind,short *colorNum)
{
	CTabHandle	ctab=nil;
	
	switch (palKind)
	{
		case kPaletteCheckAIC:
			ctab=GetCTable(130);
			*colorNum=(**ctab).ctSize+1;
			if (*colorNum>34) *colorNum=34;
			break;
		
		case kPaletteCheck16:
			/* 16色。ROMから取り出す */
			ctab=GetCTable(131);
			*colorNum=(**ctab).ctSize+1;
			break;
		
		case kPaletteCheckUsed:
			/* 使用中の色。フロントウィンドウから取得 */
			{
				WindowPtr		theWindow=MyFrontNonFloatingWindow();
				PaintWinRec		*eWinRec;
				PixMapHandle	pmh;
				OSErr			err;
				GWorldPtr		cPort;
				GDHandle		cDevice;
				
				if (theWindow!=nil)
					if (GetExtWindowKind(theWindow) == kWindowTypePaintWindow)
					{
						eWinRec=GetPaintWinRec(theWindow);
						switch (eWinRec->colorMode)
						{
							case kNormal8BitColorMode:
								GetGWorld(&cPort,&cDevice);
								
								SetGWorld(eWinRec->dispTempPtr,0);
								pmh=GetGWorldPixMap(eWinRec->dispTempPtr);
								
								err=GetPixMapColors(pmh,&ctab,colorNum);
								
								SetGWorld(cPort,cDevice);
								break;
						}
					}
			}
			break;
		
		default:
			break;
	}
	
//	if (ctab != nil) HandToHand((Handle *)&ctab);
	return ctab;
}

/* ある色がパレットにあるかどうかをチェック */
Boolean ColorExistsInPal(RGBColor *color,CTabHandle ctab,short colorNum)
{
	short		i;
	Boolean		result=false;
	ColorSpecPtr	colorList;
	
	if (ctab==nil) return false;
	
	colorList=(**ctab).ctTable;
	for (i=0; i<colorNum; i++)
	{
		if (EqualColor(color,&colorList[i].rgb))
		{
			result=true;
			break;
		}
	}
	
	return result;
}

/* 選択中のパターンをハイライトさせる */
void HilitePattern(short n,Boolean hiliteFlag)
{
	#pragma unused(hiliteFlag)
	
	short	x,y;
	Rect	r;
	GrafPtr	port;
	short	w=(gOtherPrefs.useHyperCardPalette ? 10 : 6);
	
	GetPort(&port);
	SetPortWindowPort(PatternPalette);
	
	x=((n-1) % w)*(13+3);
	y=((n-1) / w)*(13+3);
	SetRect(&r,x,y,x+19,y+19);
	
	PenNormal();
	PenMode(patXor);
	ForeColor(blackColor);
	FrameRoundRect(&r,2,2);
	
	SetPort(port);
}

/* ウィンドウの位置を調整する */
void StackWindowPos(Point *startPt,Point offsetPt,Rect *validRect)
{
	Boolean		done=false;
	Point		temp=*startPt;
	
	while (!done)
	{
		done=!WindowExistsAtPt(*startPt);
		if (done) break;
		
		AddPt(offsetPt,startPt);
		if (startPt->v > validRect->bottom)
			startPt->v = validRect->top;
		if (startPt->v > validRect->right)
			startPt->v = validRect->left;
		
		if (EqualPt(*startPt,temp))
			break;
	}
}

/* 指定座標にウィンドウが存在するかどうか */
Boolean WindowExistsAtPt(Point globPt)
{
	WindowPtr	theWindow=MyFrontNonFloatingWindow();
	GrafPtr		port;
	Boolean		result=false;
	
	if (theWindow==nil) return false;
	GetPort(&port);
	while (theWindow != nil)
	{
		Point	pt;
		Rect	tempRect;
		
		GetWindowPortBounds(theWindow,&tempRect);
		SetPt(&pt,tempRect.left,tempRect.top);
		
		SetPortWindowPort(theWindow);
		LocalToGlobal(&pt);
		if (EqualPt(globPt,pt))
		{
			result=true;
			break;
		}
		theWindow=GetNextVisibleWindow(theWindow);
	}
	
	SetPort(port);
	
	return result;
}

/* ダイアログの位置を最全面のウィンドウの中央に移動 */
void MoveDialogToParentWindow(DialogPtr theDialog)
{
	GrafPtr		port;
	WindowPtr	theWindow=MyFrontNonFloatingWindow();
	Point		pt;
	Rect		dRect,wRect;
	Point		dialogBR;
	
	GetWindowPortBounds(GetDialogWindow(theDialog),&dRect);
	GetWindowPortBounds(theWindow,&wRect);
	GetPort(&port);
	
	/* まずウィンドウの位置を得る */
	SetPortWindowPort(theWindow);
	SetPt(&pt,wRect.left,wRect.top);
	LocalToGlobal(&pt);
	
	/* ウィンドウのサイズから、中央の位置を調べる */
	SetPt(&pt,
		pt.h+20,
		pt.v+20);
	
	SetPt(&dialogBR,pt.h+dRect.right,pt.v+dRect.bottom);
	if (PtInRgn(dialogBR,GetGrayRgn()))
		MoveWindow(GetDialogWindow(theDialog),pt.h,pt.v,true);
	SelectWindow(GetDialogWindow(theDialog));
	ShowWindow(GetDialogWindow(theDialog));
	
	SetPort(port);
}

/* 編集ウィンドウのリサイズ */
void ResizePaintWindow(WindowPtr eWindow,Point globPt)
{
	long	result;
	short	minH,maxH,minV,maxV;
	Rect	bBox;
	PaintWinRec	*eWinRec=GetPaintWinRec(eWindow);
	Rect	imageSize=eWinRec->iconSize;
	Rect	screenRect;
	
	GetRegionBounds(GetGrayRgn(),&screenRect);
	minH = min(imageSize.right +kScrollBarWidth,100);
	minV = min(imageSize.bottom +kScrollBarHeight,100);
	maxH = min((imageSize.right << eWinRec->ratio) + kScrollBarWidth,screenRect.right);
	maxV = min((imageSize.bottom << eWinRec->ratio) + kScrollBarHeight,screenRect.bottom);
	
	SetPortWindowPort(eWindow);
	SetRect(&bBox,minH,minV,maxH,maxV);
	
	result=GrowWindow(eWindow,globPt,&bBox);
	
	if (result!=0L)
	{
		SizeWindow(eWindow,LoWord(result),HiWord(result),true);
		ResizePaintWindowMain(eWindow);
	}
}

/* リサイズメイン */
void ResizePaintWindowMain(WindowPtr eWindow)
{
	short	windowWidth,windowHeight;
	Point	newOrigin;
	short	deltaH,deltaV;
	PaintWinRec	*eWinRec=GetPaintWinRec(eWindow);
	Rect	imageSize=eWinRec->iconSize,tempRect;
	
	GetWindowPortBounds(eWindow,&tempRect);
	windowWidth=tempRect.right - tempRect.left - kScrollBarWidth;
	windowHeight=tempRect.bottom - tempRect.top - kScrollBarHeight;
	
	HideControl(eWinRec->hScroll);
	HideControl(eWinRec->vScroll);
	
	SetControlMaximum(eWinRec->hScroll,(imageSize.right << eWinRec->ratio) - windowWidth);
	SetControlMaximum(eWinRec->vScroll,(imageSize.bottom << eWinRec->ratio) - windowHeight);
	
	SetPt(&newOrigin,tempRect.left,tempRect.top);
	deltaH = tempRect.right - (imageSize.right << eWinRec->ratio);
	if (deltaH > 0) newOrigin.h -= deltaH;
	deltaV = tempRect.bottom - (imageSize.bottom << eWinRec->ratio);
	if (deltaV > 0) newOrigin.v -= deltaV;
	if (newOrigin.h < 0) newOrigin.h = 0;
	if (newOrigin.v < 0) newOrigin.v = 0;
	SetOrigin(newOrigin.h,newOrigin.v);
	
	/* スクロールバーの値を設定しなおす (Thanks > Kageyama) */
	SetControlValue(eWinRec->hScroll,newOrigin.h);
	SetControlValue(eWinRec->vScroll,newOrigin.v);
	
	/* portRectを取り直す (Thanks > Kageyama) */
	GetWindowPortBounds(eWindow,&tempRect);
	
	ClipRect(&tempRect);
	MyInvalWindowRect(eWindow,&tempRect);
	
	SetScrollBarRect(eWindow);
	
	ShowControl(eWinRec->hScroll);
	ShowControl(eWinRec->vScroll);
}

/* スクロールバーの領域を設定する */
void SetScrollBarRect(WindowPtr eWindow)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(eWindow);
	Rect	scrollBarRect;
	
	GetWindowPortBounds(eWindow,&scrollBarRect);
	SetRect(&scrollBarRect,scrollBarRect.left+kRatioWidth+kBackWidth-1,
				scrollBarRect.bottom-kScrollBarHeight,
				scrollBarRect.right+1-kScrollBarWidth,
				scrollBarRect.bottom+1);
	SetControlBounds(eWinRec->hScroll,&scrollBarRect);
	GetWindowPortBounds(eWindow,&scrollBarRect);
	SetRect(&scrollBarRect,scrollBarRect.right-kScrollBarWidth,
				scrollBarRect.top-1,
				scrollBarRect.right+1,
				scrollBarRect.bottom+1-kScrollBarHeight);
	SetControlBounds(eWinRec->vScroll,&scrollBarRect);
}

/* グラブスクロール */
void GrabScrollPaintWindow(WindowPtr theWindow,Point globPt)
{
	PaintWinRec	*eWinRec;
	Point		startPt,mousePt,prevPt;
	GrafPtr		port;
	Rect		imageRect;
	short		hMax,vMax,hValue,vValue;
	Rect		validRect;
	Rect		editRect,tempRect;
	GWorldPtr	tempGWorld=nil;
	RgnHandle	selectedRgn = NULL;
	Boolean		showSelection;
	Pattern		gray1,gray2;
	Boolean		isPaintSize;
	RgnHandle	clipRgn;
	RgnHandle	upRgn;
	short		p = 0;
	
	if (theWindow == nil || GetExtWindowKind(theWindow) != kWindowTypePaintWindow) return;
	
	/* スクロールバーの状態 */
	eWinRec=GetPaintWinRec(theWindow);
	hMax=GetControlMaximum(eWinRec->hScroll);
	vMax=GetControlMaximum(eWinRec->vScroll);
	if (hMax+vMax == 0) /* スクロールできない時は何もしない */
		return;
	
	showSelection=(eWinRec->isSelected && eWinRec->showSelection);
	
	/* 描画に使用するイメージを得る */
	tempGWorld=MakePaintImage(eWinRec,nil);
	if (tempGWorld == nil)
	{
		tempGWorld=MakePreviewImage(eWinRec);
		if (tempGWorld == nil) return;
		isPaintSize=false;
	}
	else
		isPaintSize=true;
	
	SetRect(&validRect,0,0,hMax,vMax);
	hValue=GetControlValue(eWinRec->hScroll);
	vValue=GetControlValue(eWinRec->vScroll);
	
	/* 描画する場所 */
	tempRect=eWinRec->iconEditSize;
	OffsetRect(&tempRect,hValue,vValue);
	if (showSelection) // && !isPaintSize)
	{
		PatHandle	temp;
		
		selectedRgn=NewRgn();
		CopyRgn(eWinRec->eSelectedRgn,selectedRgn);
	//	OffsetRgn(selectedRgn,-hValue,-vValue);
		temp=GetPattern(128);
		gray1 = **temp;
		GetQDGlobalsGray(&gray2);
		
		p = (hValue + vValue) & 1;
	}
	
	/* set mouse cursor */
	MySetCursor(141);
	
	/* 描画可能位置を設定 */
	GetPort(&port);
	SetPortWindowPort(theWindow);
	clipRgn=NewRgn();
	GetClip(clipRgn);
	GetWindowPortBounds(theWindow,&imageRect);
	imageRect.right-=kScrollBarWidth;
	imageRect.bottom-=kScrollBarHeight;
	
	startPt=globPt;
	GlobalToLocal(&startPt);
	GetMouse(&startPt);
	startPt.v <<= eWinRec->ratio;
	startPt.h <<= eWinRec->ratio;
	startPt.h += hValue;
	startPt.v += vValue;
	
	mousePt=startPt;
	prevPt.h = hValue;
	prevPt.v = vValue;
	
	upRgn = NewRgn();
	
	while (StillDown())
	{
		GetMouse(&mousePt);
		
		mousePt.v <<= eWinRec->ratio;
		mousePt.h <<= eWinRec->ratio;
		
		SubPt(startPt,&mousePt);
		mousePt.h = -mousePt.h;
		mousePt.v = -mousePt.v;
		
		/* 動かせる範囲内に調整する */
		if (!PtInRect(mousePt,&validRect))
		{
			if (mousePt.h < 0) mousePt.h = 0;
			else if (mousePt.h > hMax) mousePt.h = hMax;
			if (mousePt.v < 0) mousePt.v = 0;
			else if (mousePt.v > vMax) mousePt.v = vMax;
		}
		
		if (EqualPt(prevPt,mousePt)) continue; /* 位置が変わっていなければ何もしない */
		
		/* 表示位置をずらす */
		editRect=tempRect;
		OffsetRect(&editRect,-mousePt.h,-mousePt.v);
		
		/* イメージを表示する */
		ClipRect(&imageRect);
		if (isPaintSize)
		{
			ScrollRect(&imageRect,-mousePt.h+prevPt.h,-mousePt.v+prevPt.v,upRgn);
			SetClip(upRgn);
			CopyBits(GetPortBitMapForCopyBits(tempGWorld),
				GetPortBitMapForCopyBits(GetWindowPort(theWindow)),
				&eWinRec->iconEditSize,&editRect,srcCopy,nil);
		}
		else
		{
			ScrollRect(&imageRect,-mousePt.h+prevPt.h,-mousePt.v+prevPt.v,upRgn);
			SetClip(upRgn);
			CopyBits(GetPortBitMapForCopyBits(tempGWorld),
				GetPortBitMapForCopyBits(GetWindowPort(theWindow)),
				&eWinRec->iconSize,&editRect,srcCopy,nil);
		}
		
		/* 選択範囲 */
		if (showSelection)
		{
			PenNormal();
			PenMode(patXor);
			
			if ((mousePt.h+mousePt.v+p) & 1)
				PenPat(&gray2);
			else
				PenPat(&gray1);
			
			OffsetRgn(selectedRgn,-mousePt.h+prevPt.h,-mousePt.v+prevPt.v);
			FrameRgn(selectedRgn);
			PenNormal();
		}
		prevPt=mousePt;
		
		/* スクロールバーの描画 */
		SetClip(clipRgn);
		SetControlValue(eWinRec->hScroll,mousePt.h);
		SetControlValue(eWinRec->vScroll,mousePt.v);
	}
	
	/* 動かされたぶんだけ動かす */
	SetOrigin(mousePt.h,mousePt.v);
	
	DisposeRgn(upRgn);
	
	FlushEvents(mDownMask,0);
	
	/* 確保したメモリの破棄 */
	if (isPaintSize || eWinRec->backgroundGWorld != nil)
		DisposeGWorld(tempGWorld);
	
	if (showSelection) // && !isPaintSize)
		DisposeRgn(selectedRgn);
	
	/* スクロールバーの位置を調整し、再描画させる */
	SetScrollBarRect(theWindow);
	GetWindowPortBounds(theWindow,&imageRect);
//	SetClip(clipRgn);
	DisposeRgn(clipRgn);
	UpdatePaintClipRect(theWindow);
	if (!isPaintSize && gToolPrefs.gridMode!=iGNone) /* redraw grid */
		MyInvalWindowRect(theWindow,&imageRect);
	SetPort(port);
}

void DoScrollPaintWindowByKeys(WindowPtr theWindow,short theChar,short modifiers)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	short	hDelta = 0,vDelta = 0;
	short	hMax = GetControlMaximum(eWinRec->hScroll);
	short	vMax = GetControlMaximum(eWinRec->vScroll);
	short	delta;
	Rect	tempRect;
	short	windowWidth,windowHeight;
	
	if (hMax+vMax == 0) return;
	
	if ((modifiers & cmdKey) != 0)
		delta = (hMax > vMax ? hMax : vMax);
	else
		delta = 16;
	
	GetWindowPortBounds(theWindow,&tempRect);
	windowWidth=tempRect.right - tempRect.left - kScrollBarWidth;
	windowHeight=tempRect.bottom - tempRect.top - kScrollBarHeight;
	
	switch (theChar)
	{
		case kLeftArrowCharCode: /* left arrow key */
			hDelta = -delta;
			break;
		
		case kRightArrowCharCode: /* right arrow key */
			hDelta = delta;
			break;
		
		case kUpArrowCharCode: /* up arrow key */
			vDelta = -delta;
			break;
		
		case kDownArrowCharCode: /* down arrow key */
			vDelta = delta;
			break;
		
		case kHomeCharCode:
			hDelta = -(windowWidth - 16);
			break;
		
		case kEndCharCode:
			hDelta = (windowWidth - 16);
			break;
		
		case kPageUpCharCode:
			vDelta = -(windowHeight - 16);
			break;
		
		case kPageDownCharCode:
			vDelta = (windowWidth - 16);
			break;
	}
	
	DoScrollPaintWindowMain(theWindow,hDelta,vDelta);
}

static void DoScrollPaintWindowMain(WindowPtr theWindow,short hDelta,short vDelta)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	short	hValue,hValue1,vValue,vValue1;
	Rect	r;
	RgnHandle	upRgn;
	
	hValue = GetControlValue(eWinRec->hScroll);
	hValue1 = hValue + hDelta;
	if (hValue1 < 0)
		hValue1 = 0;
	else if (hValue1 > GetControlMaximum(eWinRec->hScroll))
		hValue1 = GetControlMaximum(eWinRec->hScroll);
	
	vValue = GetControlValue(eWinRec->vScroll);
	vValue1 = vValue + vDelta;
	if (vValue1 < 0)
		vValue1 = 0;
	else if (vValue1 > GetControlMaximum(eWinRec->vScroll))
		vValue1 = GetControlMaximum(eWinRec->vScroll);
	
	hDelta = (hValue1 - hValue);
	vDelta = (vValue1 - vValue);
	
	if (hDelta != 0 || vDelta != 0)
	{
		SetControlValue(eWinRec->hScroll,hValue1);
		SetControlValue(eWinRec->vScroll,vValue1);
		
		GetWindowPortBounds(theWindow,&r);
		r.right -= kScrollBarWidth;
		r.bottom -= kScrollBarHeight;
		upRgn = NewRgn();
		ScrollRect(&r,-hDelta,-vDelta,upRgn);
		MyInvalWindowRgn(theWindow,upRgn);
		DisposeRgn(upRgn);
		
		SetOrigin(hValue1,vValue1);
		SetScrollBarRect(theWindow);
		DrawControls(theWindow);
		
		/* update clip region */
		UpdatePaintClipRect(theWindow);
	}
}

/* スクロール用画像 */
static GWorldPtr	gTempGWorld;
static Rect			gTempRect;
static short		gTempValue;
static Boolean		gIsPaintSize;
static Boolean		gShowSelection;
static RgnHandle	gTempSelectedRgn;
static short		gP;
static Pattern		gGray1,gGray2;

/* scroll paint window */
static void DoScrollPaintWindow(WindowPtr theWindow,Point localPt)
{
	short			part;
	ControlHandle	control;
	
	part=FindControl(localPt,theWindow,&control);
	if (part && control)
	{
		/* スクロールバー */
		PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
		short	value;
		ControlActionUPP	caUPP=NewControlActionUPP(MyScrollHandler);
		short	delta = 0;
		
		value=GetControlValue(control);
		gP = (value & 1);
		
		gShowSelection = (eWinRec->isSelected && eWinRec->showSelection);
		if (gShowSelection)
		{
			PatHandle	temp;
			
			gTempSelectedRgn = NewRgn();
			CopyRgn(eWinRec->eSelectedRgn,gTempSelectedRgn);
			
			temp=GetPattern(128);
			gGray1 = **temp;
			GetQDGlobalsGray(&gGray2);
		}
		
		switch(part)
		{
			case kControlUpButtonPart:
			case kControlDownButtonPart:
			case kControlPageUpPart:
			case kControlPageDownPart:
			case kControlIndicatorPart:
				if (part == kControlIndicatorPart && !isAppearanceAvailable)
				{
					part=TrackControl(control,localPt,nil);
					
					delta=GetControlValue(control)-value;
					if (delta != 0)
					{
						gTempGWorld = MakePaintImage(eWinRec,nil);
						if (gTempGWorld == NULL)
						{
							gIsPaintSize = false;
							gTempGWorld = MakePreviewImage(eWinRec);
						
						}
						else
							gIsPaintSize = true;
						
						
						
						gTempRect = eWinRec->iconEditSize;
						ContentScroll(theWindow,control,delta);
						DisposeGWorld(gTempGWorld);
						if (gShowSelection)
							DisposeRgn(gTempSelectedRgn);
						if (!gIsPaintSize && gToolPrefs.gridMode!=iGNone)
							MyInvalWindowPortBounds(theWindow);
					}
				}
				else
				{
					gTempGWorld = MakePaintImage(eWinRec,nil);
					if (gTempGWorld == NULL)
					{
						gIsPaintSize = false;
						gTempGWorld = MakePreviewImage(eWinRec);
					}
					else
						gIsPaintSize = true;
					
					gTempRect = eWinRec->iconEditSize;
					gTempValue = value;
					part=TrackControl(control,localPt,caUPP);
					DisposeGWorld(gTempGWorld);
			
					delta=GetControlValue(control)-value;
					
					if (gShowSelection)
						DisposeRgn(gTempSelectedRgn);
					if (!gIsPaintSize && gToolPrefs.gridMode!=iGNone)
						MyInvalWindowPortBounds(theWindow);
				}
				break;
		}
		if (delta != 0)
		{
			SetOrigin(GetControlValue(eWinRec->hScroll),
				GetControlValue(eWinRec->vScroll));
			SetScrollBarRect(theWindow);
			Draw1Control(control);
			
			/* clip regionの更新 */
			UpdatePaintClipRect(theWindow);
		}
		DisposeControlActionUPP(caUPP);
	}
}

/* スクロールバーのアクション */
pascal void MyScrollHandler(ControlHandle theControl,short part)
{
	short	moving=0,value,newValue;
	WindowPtr	theWindow;
	short	windowWidth;
	short	windowHeight;
	Rect	tempRect;
	
	theWindow=GetControlOwner(theControl);
	GetWindowPortBounds(theWindow,&tempRect);
	
	if (part == kControlIndicatorPart)
	{
		value=GetControlValue(theControl);
		ContentScroll(theWindow,theControl,value-gTempValue);
		gTempValue=value;
	}
	else
	{
		windowWidth=tempRect.right - tempRect.left - kScrollBarWidth;
		windowHeight=tempRect.bottom - tempRect.top - kScrollBarHeight;
		
		switch(part)
		{
			case kControlUpButtonPart:
				moving=-16;
				break;
			
			case kControlDownButtonPart:
				moving=16;
				break;
			
			case kControlPageUpPart:
				switch (GetControlReference(theControl))
				{
					case kHScroll:
						moving=-(windowWidth - 16);
						break;
					
					case kVScroll:
						moving=-(windowHeight - 16);
						break;
				}
				break;
			
			case kControlPageDownPart:
				switch (GetControlReference(theControl))
				{
					case kHScroll:
						moving=(windowWidth - 16);
						break;
					
					case kVScroll:
						moving=(windowHeight - 16);
						break;
				}
				break;
		}
		
		value=GetControlValue(theControl);
		newValue=value+moving;
		if (newValue > GetControlMaximum(theControl)) newValue=GetControlMaximum(theControl);
		else if (newValue < GetControlMinimum(theControl)) newValue=GetControlMinimum(theControl);
		
		if (value != newValue)
		{
			SetControlValue(theControl,newValue);
			ContentScroll(theWindow,theControl,newValue-value);
		}
	}
}

/* 内容をスクロールさせる処理 */
void ContentScroll(WindowPtr theWindow,ControlHandle theControl,short delta)
{
	RgnHandle	upRgn;
	Rect	scrollArea,tempRect;
	Point	offset;
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	
	upRgn=NewRgn();
	switch (GetControlReference(theControl))
	{
		case kHScroll:
			SetPt(&offset,delta,0);
			OffsetRect(&gTempRect,-delta,0);
			break;
		
		case kVScroll:
			SetPt(&offset,0,delta);
			OffsetRect(&gTempRect,0,-delta);
			break;
	}
	
	GetWindowPortBounds(theWindow,&tempRect);
//	SetOrigin(tempRect.left+offset.h,tempRect.top+offset.v);
//	GetWindowPortBounds(theWindow,&tempRect); /* スクロールによりPortBoundsが変わる */
	ClipRect(&tempRect);
	scrollArea=tempRect;
	scrollArea.right-=kScrollBarWidth;
	scrollArea.bottom-=kScrollBarHeight;
	ScrollRect(&scrollArea,-offset.h,-offset.v,upRgn);
	
	SetClip(upRgn);
	
//	RedrawPaintWindow(theWindow,upRgn);
	
	if (gIsPaintSize)
		CopyBits(GetPortBitMapForCopyBits(gTempGWorld),
			GetPortBitMapForCopyBits(GetWindowPort(theWindow)),
			&eWinRec->iconEditSize,&gTempRect,srcCopy,nil);
	else
		CopyBits(GetPortBitMapForCopyBits(gTempGWorld),
			GetPortBitMapForCopyBits(GetWindowPort(theWindow)),
			&eWinRec->iconSize,&gTempRect,srcCopy,nil);
	
	if (gShowSelection)
	{
		short p = GetControlValue(theControl);
		
		PenNormal();
		PenMode(patXor);
		
		if ((gP + p) & 1)
			PenPat(&gGray2);
		else
			PenPat(&gGray1);
		
		OffsetRgn(gTempSelectedRgn,-offset.h,-offset.v);
		FrameRgn(gTempSelectedRgn);
		PenNormal();
	}
	
	ClipRect(&tempRect);
	DisposeRgn(upRgn);
	
//	SetScrollBarRect(theWindow);
}

/* ズーム */
void ZoomPaintWindow(WindowPtr theWindow,short thePart)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	Rect		r,r2;
	RgnHandle	rgn;
	
	switch (thePart)
	{
		case inZoomIn:
			GetWindowUserState(theWindow,&r);
			SizeWindow(theWindow,r.right-r.left,r.bottom-r.top,false);
			ResizePaintWindowMain(theWindow);
			break;
		
		case inZoomOut:
			GetWindowUserState(theWindow,&r);
			
			SetPaintWindowSize(theWindow,&eWinRec->iconSize,eWinRec->ratio);
			ResizePaintWindowMain(theWindow);
			
			rgn=NewRgn();
			GetWindowContentRgn(theWindow,rgn);
			GetRegionBounds(rgn,&r2);
			DisposeRgn(rgn);
			GetWindowStandardState(theWindow,&r2);
			GetWindowUserState(theWindow,&r);
			break;
	}
}

/* ClipRectの更新 */
/* ポートは設定されていることを前提 */
void UpdatePaintClipRect(WindowPtr theWindow)
{
	Rect	clipRect;
	GrafPtr	port;
	
	GetPort(&port);
	SetPortWindowPort(theWindow);
	
	GetWindowPortBounds(theWindow,&clipRect);
//	clipRect.right -= kScrollBarWidth;
//	clipRect.bottom -= kScrollBarHeight;
	
	ClipRect(&clipRect);
	
	SetPort(port);
}

/* ドラッグを開始する（ドラッグのデータを作成） */
void MyDoStartDragPaintWin(WindowPtr eWindow,EventRecord *theEvent)
{
	DragReference   theDrag;
	RgnHandle       dragRegion = NULL;
	OSErr	err;
	DragSendDataUPP	dsdUPP;
	
	dsdUPP=NewDragSendDataUPP(MySendPaintWinDataProc);
	if (dsdUPP==0) return;
	
	err = NewDrag(&theDrag);
	if (err!=noErr)
		goto exit;
	
	err = MyDoAddPaintWinFlavors(eWindow, theDrag);
	if (err!=noErr)
		goto exit;
	
	dragRegion = NewRgn();
	err = MyGetDragPaintWinRegion(eWindow, dragRegion, theDrag);
	if (err!=noErr)
		goto exit;
	
	err = SetDragSendProc(theDrag, dsdUPP, eWindow);
	if (err!=noErr)
		goto exit;
	
	err = TrackDrag(theDrag, theEvent, dragRegion);
	
exit:
	DisposeRgn(dragRegion);
	DisposeDrag(theDrag);
	DisposeDragSendDataUPP(dsdUPP);
}

/* ドラッグデータにflavorを追加する */
OSErr MyDoAddPaintWinFlavors(WindowPtr eWindow,DragReference theDrag)
{
	#pragma unused(eWindow)
	OSErr	err;
	
	err=AddDragItemFlavor(theDrag,1,kPICTFileType,0L,0L,0);
	
	return err;
}

/* ドラッグリージョンの作成 */
OSErr MyGetDragPaintWinRegion(WindowPtr eWindow,RgnHandle dragRegion,
								DragReference theDragRef)
{
	#pragma unused(theDragRef)
	PaintWinRec		*eWinRec=GetPaintWinRec(eWindow);
	RgnHandle		tempRgn;
	Point			globalPoint={0,0};
	
	tempRgn=NewRgn();
	
	LocalToGlobal(&globalPoint);
	
	CopyRgn(eWinRec->eSelectedRgn,tempRgn);
	CopyRgn(tempRgn,dragRegion);
	InsetRgn(tempRgn,1,1);
	DiffRgn(dragRegion,tempRgn,dragRegion);
	
	OffsetRgn(dragRegion,globalPoint.h,globalPoint.v);
	
	DisposeRgn(tempRgn);
	return(noErr);
}

/* 要求されたflavorデータを提供する */
pascal short MySendPaintWinDataProc(FlavorType theType,void *dragSendRefCon,
									ItemReference theItem,DragReference theDrag)
{
	OSErr		err=noErr;
	WindowPtr	eWindow;
	PicHandle	picture;
	
	eWindow=dragSendRefCon;
	
	switch (theType) {
		case kPICTFileType:
			/* 渡すPICTデータを作成 */
			picture=GetSelectionPic(eWindow,false);
			HLock((Handle)picture);
			err=SetDragItemFlavorData(theDrag,theItem,kPICTFileType,*picture,
											GetHandleSize((Handle)picture),0L);
			HUnlock((Handle)picture);
			
			KillPicture(picture);
			break;
		
		default:
			err=badDragFlavorErr;
			break;
	}
	
	return err;
}
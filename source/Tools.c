/* ------------------------------------------------------------ */
/*  Tools.c                                                     */
/*     ツール処理                                               */
/*                                                              */
/*                 1997.1.28 - 2001.3.10  naoki iimura        	*/
/* ------------------------------------------------------------ */


#ifdef __APPLE_CC__
#include	<Carbon/Carbon.h>
#include	<QuickTime/QuickTime.h>
#else
#include	<ToolUtils.h>
#include	<ColorPicker.h>
#include	<LowMem.h>
#include	<Movies.h>
#endif

#include	"WindowExtensions.h"

#include	"Globals.h"
#include	"IconParty.h"
#include	"MenuRoutines.h"
#include	"WindowRoutines.h"
#include	"PreCarbonSupport.h"
#include	"PaintRoutines.h"
#include	"ToolRoutines.h"
#include	"UsefulRoutines.h"
#include	"UpdateCursor.h"

/* prototypes */
static Boolean	ChangePESize(short *width,short *height);

#if TARGET_API_MAC_CARBON
	extern pascal void PenSizeMDEFProc(short message,MenuHandle theMenu,Rect *menuRect,Point hitPt,short *whichItem);
#endif

extern WindowPtr	PatternPalette;


/* ツール選択 */
void ToolSelect(short tool)
{
	GrafPtr		port;
	WindowPtr	theWindow;
	PicHandle	pic;
	Rect		r;
	
	theWindow=MyFrontNonFloatingWindow();
	
	if (tool==gSelectedTool) return;
	
	if (tool>nTools || tool<0) return;
	
	/* 選択ツール以外を選んだとき、どこかが選択されていれば固定する */
	/* 選択領域でのマスキングが有効のときは、固定しない */
	if (theWindow!=nil && GetExtWindowKind(theWindow)==kWindowTypePaintWindow && !gToolPrefs.selectionMasking)
	{
		PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
		
		if (!EmptyRgn(eWinRec->eSelectedRgn) && (tool!=kMarqueeTool && tool!=kSpoitTool))
		{
			SetPortWindowPort(theWindow);
			FixSelection(theWindow);
			SetPortWindowPort(gToolPalette);
		}
	}
	
	gSelectedTool=tool;
	SetRect(&gToolRect,0x3,0x3+gSelectedTool*25,0x20,0x19+gSelectedTool*25);
	#if ENGLISH_VER
	InsetRect(&gToolRect,-1,-1);
	#endif
	
	GetPort(&port);
	SetPortWindowPort(gToolPalette);
	
	/* 選ばれたツールをハイライトさせる */
	pic=GetPicture(kToolPalettePictureResID);
	GetWindowPortBounds(gToolPalette,&r);
	DrawPicture(pic,&r);
	DarkenRect(&gToolRect);
	
	SetPort(port);
}

/* ツールパレットのダブルクリック */
void DoubleClickTool(short tool)
{
	WindowPtr	theWindow=MyFrontNonFloatingWindow();
	short		windowKind;
	PaintWinRec	*eWinRec;
	Rect		effectRect;
	Boolean		isBackMode;
	RgnHandle	selectRgn;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	GetGWorld(&cPort,&cDevice);
	
	switch (tool)
	{
		case kPencilTool:
			/* 鉛筆ツールをダブルクリック→ペンサイズの変更 */
			ChangePenSize();
			break;
		
		case kEraserTool:
			/* 消しゴムツールをダブルクリック→全画面消去 */
			if (theWindow==nil) return;
			
			windowKind=GetExtWindowKind(theWindow);
			if (windowKind!=kWindowTypePaintWindow) return;
			
			eWinRec=GetPaintWinRec(theWindow);
			isBackMode=(eWinRec->backgroundGWorld == nil);
			GoOffPort(theWindow);
			
			/* 消去する範囲 */
			if (EmptyRgn(eWinRec->eSelectedRgn))
				effectRect=eWinRec->iconSize;
			else
				effectRect=eWinRec->selectedRect;
			
			/* 消去 */
			SetGWorld(eWinRec->editDataPtr,0);
			LockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
			if (isBackMode)  /* 背景がない場合は背景色で塗る */
				RGBBackColor(&gBackColor.rgb);
			EraseRect(&effectRect);
			BackColor(whiteColor);
			UnlockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
			
			SetGWorld(eWinRec->currentMask,0);
			LockPixels(GetGWorldPixMap(eWinRec->currentMask));
			if (!isBackMode || gBackColor.isTransparent)  /* 背景がある、あるいは背景色が透明の時はマスクを消去 */
				EraseRgn(eWinRec->selectionPos);
			else /* それ以外の場合は選択範囲全体をマスク化 */
				PaintRgn(eWinRec->selectionPos);
			CopyRgn(eWinRec->selectionPos,eWinRec->updateRgn);
			
			SetGWorld(cPort,cDevice);
			DispOffPort(theWindow);
			
			UpdateTransparentMenu();
			SetUndoMode(umPaint);
			if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
				UpdatePaletteCheck();
			break;
		
		case kMarqueeTool:
			/* 選択ツールをダブルクリック→全画面選択 */
			if (theWindow==nil) return;
			
			windowKind=GetExtWindowKind(theWindow);
			if (windowKind != kWindowTypePaintWindow) return;
			
			eWinRec=GetPaintWinRec(theWindow);
			if (eWinRec->isSelected) /* 選択範囲があれば固定する */
				FixSelection(theWindow);
			
			GoOffPort(theWindow);
			
			selectRgn=NewRgn();
			RectRgn(selectRgn,&eWinRec->iconSize);
			
			DoSelectMain(theWindow,selectRgn);
			break;
		
		case kSpoitTool:
			/* スポイトツールをダブルクリック→画面の任意の場所から色をとる */
			GetDesktopColor();
			break;
	}
}

/* グリッドの変更 */
void ChangeGrid(short item)
{
	#pragma unused(item)
}

/* ペンサイズの変更 */
void ChangePenSize(void)
{
	short	prevPenHeight = gPenHeight,
			prevPenWidth = gPenWidth;
	
	if (ChangePESize(&gPenWidth,&gPenHeight))
	{
		/* メニューの更新 */
		MenuHandle	menu;
		
		menu = GetMenuHandle(mPenSize);
		CheckMenuItem(menu,(prevPenHeight-1)*4+prevPenWidth,false);
		CheckMenuItem(menu,(gPenHeight-1)*4+gPenWidth,true);
		
		/* 情報ウィンドウの再描画 */
		RedrawInfo();
		
		/* ツールの選択 */
		ToolSelect(kPencilTool);
	}
}

/* 消しゴムのサイズの変更 */
void ChangeEraserSize(void)
{
	short	prevEraserHeight = gEraserHeight,
			prevEraserWidth = gEraserWidth;
	
	if (ChangePESize(&gEraserWidth,&gEraserHeight))
	{
		/* メニューの更新 */
		MenuHandle	menu;
		
		menu = GetMenuHandle(mEraserSize);
		CheckMenuItem(menu,(prevEraserHeight-1)*4+prevEraserWidth,false);
		CheckMenuItem(menu,(gEraserHeight-1)*4+gEraserWidth,true);
		
		/* 情報ウィンドウの再描画 */
		RedrawInfo();
		
		/* ツールの選択 */
		ToolSelect(kEraserTool);
	}
}

/* ペン／消しゴムのサイズの変更メインルーチン */
Boolean ChangePESize(short *width,short *height)
{
	MenuHandle	menu;
	Point		mousePt;
	long		selItem;
	short		item;
	
	GetMouse(&mousePt);
	LocalToGlobal(&mousePt);
	
	#if TARGET_API_MAC_CARBON
	{
		OSErr	err;
		MenuDefSpec	defSpec;
		
		defSpec.defType = kMenuDefProcPtr;
		defSpec.u.defProc = NewMenuDefUPP(PenSizeMDEFProc);
		err=CreateCustomMenu(&defSpec,146,0,&menu);
		for (item=1; item<=16; item++)
			AppendMenuItemText(menu,"\p ");
	}
	#else
	menu=GetMenu(146);
	#endif
	InsertMenu(menu,-1);
	
	for (item=1; item<=16; item++)
		CheckMenuItem(menu,item,false);
	CheckMenuItem(menu,(*height-1)*4+*width,true);
	
	selItem=PopUpMenuSelect(menu,mousePt.v+1,mousePt.h+1,0);
	item=LoWord(selItem);
	
	DeleteMenu(146);
	
	if (item!=0)
	{
		*width=(item-1)%4+1;
		*height=(item-1)/4+1;
		
		return true;
	}
	return false;
}

/* 透明度の設定 */
void ChangeMode(short item)
{
	unsigned short	temp;
	
	if (item==1)
		gBlendMode=srcCopy;
	else
	{
		gBlendMode=blend;
		temp=(5-item)<<14;
		gBlendRatio.red=temp;
		gBlendRatio.green=temp;
		gBlendRatio.blue=temp;
	}
}

/* 透明度の変更 */
void ChangeTransparency(void)
{
	MenuHandle	menu;
	Point		mousePt;
	long		selItem;
	short		item;
	
	GetMouse(&mousePt);
	LocalToGlobal(&mousePt);
	
	menu=GetMenu(mTransp2);
	InsertMenu(menu,-1);
	
	for (item=1; item<=4; item++)
		CheckMenuItem(menu,item,false);
	CheckMenuItem(menu,gBSelectedItem,true);
	
	selItem=PopUpMenuSelect(menu,mousePt.v,mousePt.h,gBSelectedItem);
	item=LoWord(selItem);
	
	DeleteMenu(mTransp2);
	
	if (item!=0)
	{
		HandleBlendChoice(item);
		RedrawInfo();
	}
}

/* 選択色の変更 */
void ChangeColor(RGBColor *newColor,Boolean isTransparent)
{
	GrafPtr	port;
	Rect	r;
	
	if (EqualColor(newColor,&gCurrentColor.rgb) && isTransparent == gCurrentColor.isTransparent)
		return; /* 現在の選択色と同じ場合はなにもしない */
	
	/* カラーパレットのハイライト更新 */
	GetPort(&port);
	HiliteSelectedColor(1,false);
	HiliteSelectedColor(2,false);
	HiliteSelectedColor(3,false);
	gPrevColor=gCurrentColor;
	gCurrentColor.rgb=*newColor;
	gCurrentColor.isTransparent=isTransparent;
	HiliteSelectedColor(1,true);
	HiliteSelectedColor(2,true);
	HiliteSelectedColor(3,true);
	
	/* ブレンドパレット更新 */
	DrawBlend();
	SetPortWindowPort(gBlendPalette);
	GetWindowPortBounds(gBlendPalette,&r);
	CopyBits(GetPortBitMapForCopyBits(gBlendPalettePtr),GetPortBitMapForCopyBits(GetWindowPort(gBlendPalette)),
		&r,&r,srcCopy,nil);
	
	/* パターンパレット更新 */
	MyInvalWindowPortBounds(PatternPalette);
	
	SetPort(port);
}

/* 背景色の変更 */
void ChangeBackColor(RGBColor *newColor,Boolean isTransparent)
{
	GrafPtr	port;
	Rect	r;
	
	if (EqualColor(newColor,&gBackColor.rgb) && isTransparent == gBackColor.isTransparent) return;
	
	gBackColor.rgb=*newColor;
	gBackColor.isTransparent=isTransparent;
	
	GetPort(&port);
	/* ブレンドパレット更新 */
	DrawBlend();
	SetPortWindowPort(gBlendPalette);
	GetWindowPortBounds(gBlendPalette,&r);
	CopyBits(GetPortBitMapForCopyBits(gBlendPalettePtr),GetPortBitMapForCopyBits(GetWindowPort(gBlendPalette)),
		&r,&r,srcCopy,nil);
	
	/* パターンパレット更新 */
	MyInvalWindowPortBounds(PatternPalette);
	
	SetPort(port);
}

/* 画面上から色を得る 1.0b5追加 */
void GetDesktopColor(void)
{
	#if !TARGET_API_MAC_CARBON
	Point	mousePt;
	Boolean	result=true;
	RGBColor	newColor;
	MyColorRec	prevColor=gCurrentColor;
	GrafPtr	port;
	KeyMap	theKeys;
	
	MySetCursor(gToolPrefs.changeSpoitCursor ? 139 : 128+kSpoitTool);
	
	/* マウスクリックを読み飛ばす */
	while (Button()) ;
	
	GetPort(&port);
	SetPort(LMGetWMgrPort());
	
	while (!Button())
	{
		/* キーボードの状態を監視し、cmd+.なら終わる */
		GetKeys(theKeys);
		if (BitTst(theKeys,48) && BitTst(theKeys,40))
		{
			result=false;
			break;
		}
		
		/* マウス位置の色をとり、ブレンドパレット上で表示 */
		GetMouse(&mousePt);
		GetCPixel(mousePt.h,mousePt.v,&newColor);
		FixColor(&newColor);
		
		gCurrentColor.rgb=newColor;
		gCurrentColor.isTransparent=false;
		DrawBlend();
		SetPortWindowPort(gBlendPalette);
		CopyBits(GetPortBitMapForCopyBits(gBlendPalettePtr),GetPortBitMapForCopyBits(GetWindowPort(gBlendPalette)),
			&gBlendPalette->portRect,&gBlendPalette->portRect,srcCopy,nil);
		SetPort(LMGetWMgrPort());
	}
	FlushEvents(mDownMask+keyDownMask,0);
	
	SetPort(port);
	
	gCurrentColor=prevColor;
	if (result)
	{
		/* 色を設定 */
		ChangeColor(&newColor,false);
	}
	else
	{
		/* ブレンドパレットを元に戻す */
		DrawBlend();
		SetPort(gBlendPalette);
		InvalRect(&gBlendPalette->portRect);
		SetPort(port);
	}
	
	#else
	
	Point		mousePt;
	Boolean		result=true;
	RGBColor	newColor;
	MyColorRec	prevColor=gCurrentColor;
	GrafPtr		port;
	GWorldPtr	deskGWorld;
	GDHandle	mainDevice;
	CTabHandle	cTable;
	short		depth;	
	PixMapHandle	pixHandle = nil;
	BitMap		bitMap;
	Rect		screenBounds,portBounds;
	OSErr		err;
	EventRecord	theEvent;
	Boolean		isEventAvail;
	Boolean		quitFlag=true;
	WindowPtr	deskWindow=nil;
	Ptr			oldState;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	MySetCursor(gToolPrefs.changeSpoitCursor ? 139 : 128+kSpoitTool);
	
	/* マウスクリックを読み飛ばす */
	while (StillDown()) ;
	
	GetPort(&port);
	GetGWorld(&cPort,&cDevice);
	
	mainDevice = GetMainDevice();

	/* Store its current pixel depth. */
	depth = (**(**mainDevice).gdPMap).pixelSize;

	/* Make an identical copy of its pixmap's colortable. */
	cTable = (**(**mainDevice).gdPMap).pmTable;
	HandToHand( (Handle *)&cTable );
	
	// Get Resolution of screen
	GetQDGlobalsScreenBits(&bitMap);
	SetRect( &screenBounds, 0, 0, bitMap.bounds.right, bitMap.bounds.bottom);
	
	// Fill in a few of the PixMap's fields...
	// NewPixMap() is good for default initialization, simply modify
	// the new PixMap
	err=NewGWorld(&deskGWorld,depth,&screenBounds,cTable,nil,useTempMem);
	if (err!=noErr) return;
	
	SetGWorld(deskGWorld,0);
	pixHandle=GetGWorldPixMap(deskGWorld);
	LockPixels(pixHandle);

	CopyBits( (BitMap *)*(**mainDevice).gdPMap, (BitMap *) *pixHandle,
				&(**(**mainDevice).gdPMap).bounds, &(*pixHandle)->bounds, srcCopy, 0l );
	
	UnlockPixels(pixHandle);
	
	SetGWorld(cPort,cDevice);
	
	// Startup Full Screen Mode
	BeginFullScreen(&oldState,nil,0,0,&deskWindow,0,fullScreenDontChangeMenuBar);
	SetPortWindowPort(deskWindow);
	EraseRect(&screenBounds);
	CopyBits(GetPortBitMapForCopyBits(deskGWorld),
		GetPortBitMapForCopyBits(GetWindowPort(deskWindow)),
		&screenBounds,&screenBounds,srcCopy,nil);
//	QDFlushPortBuffer(GetWindowPort(deskWindow),nil);
	
	DisposeGWorld(deskGWorld);
	
	GetWindowPortBounds(gBlendPalette,&portBounds);
	
	FlushEvents(mDownMask+mUpMask+keyDownMask,0);
	while (quitFlag)
	{
		/* キーボードの状態を監視し、cmd+.なら終わる */
		isEventAvail=WaitNextEvent(mUpMask+keyDownMask,&theEvent,0,nil);
		if (isEventAvail)
		{
			switch (theEvent.what)
			{
				case keyDown:
					result=false;
					quitFlag=false;
					break;
				
				case mouseUp:
					quitFlag=false;
					break;
			}
		}
		
		/* マウス位置の色をとり、ブレンドパレット上で表示 */
		SetPortWindowPort(deskWindow);
		GetMouse(&mousePt);
		GetCPixel(mousePt.h,mousePt.v,&newColor);
		FixColor(&newColor);
		
		gCurrentColor.rgb=newColor;
		gCurrentColor.isTransparent=false;
		DrawBlend();
		SetPortWindowPort(gBlendPalette);
		CopyBits(GetPortBitMapForCopyBits(gBlendPalettePtr),
			GetPortBitMapForCopyBits(GetWindowPort(gBlendPalette)),
			&portBounds,&portBounds,srcCopy,nil);
	}
	FlushEvents(mDownMask+mUpMask+keyDownMask,0);
	
	EndFullScreen(oldState,(long)nil);
	
	SetPort(port);
	HideMenuBar();
	ShowMenuBar();
	
	gCurrentColor=prevColor;
	if (result)
	{
		/* 色を設定 */
		ChangeColor(&newColor,false);
	}
	else
	{
		/* ブレンドパレットを元に戻す */
		DrawBlend();
		SetPortWindowPort(gBlendPalette);
		MyInvalWindowPortBounds(gBlendPalette);
		SetPort(port);
	}
	
	#endif
}

/* 色を256色に補正（汚いやり方だ） */
void FixColor(RGBColor *c)
{
	long	l;
	short	colorMode=gPaintWinPrefs.colorMode;
	WindowPtr	theWindow=MyFrontNonFloatingWindow();
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	if (theWindow != nil && GetExtWindowKind(theWindow) == kWindowTypePaintWindow)
	{
		PaintWinRec	*eWinRec = GetPaintWinRec(theWindow);
		
		colorMode = eWinRec->colorMode;
	}
	
	switch (colorMode)
	{
		case kNormal8BitColorMode:
			GetGWorld(&cPort,&cDevice);
			SetGWorld(gBlendPalettePtr,0);
			l=Color2Index(c);
			Index2Color(l,c);
			SetGWorld(cPort,cDevice);
			break;
		
		case k32BitColorMode:
			break;
	}
}

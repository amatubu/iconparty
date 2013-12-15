/* ------------------------------------------------------------ */
/*  Paint.c                                                     */
/*     ペイント処理                                             */
/*                                                              */
/*                 1997.1.28 - 2001.3.10  naoki iimura        	*/
/* ------------------------------------------------------------ */

/* includes */
#ifdef __APPLE_CC__
#include	<Carbon/Carbon.h>
#else
#include	<sound.h>
#include	<Fonts.h>
#include	<NumberFormatting.h>
#include	<Resources.h>
#include	<ToolUtils.h>
#include	<Gestalt.h>
#endif

#include	"Globals.h"
#include	"IconParty.h"
#include	"FileRoutines.h"
#include	"MenuRoutines.h"
#include	"UsefulRoutines.h"
#include	"WindowExtensions.h"
#include	"TabletUtils.h"
#include	"WindowRoutines.h"
#include	"PreCarbonSupport.h"
#include	"Preferences.h"
#include	"PaintRoutines.h"
#include	"EditRoutines.h"
#include	"ToolRoutines.h"
#include	"UpdateCursor.h"

enum {
	kPencilMode=1,
	kEraserMode,
	kEraseMaskMode,
};

extern WindowPtr	DotModePalette;

#pragma options align=mac68k

typedef struct {
	GWorldPtr	maskGWorld;
	Boolean		srcIsTransparent;
} BucketDataRec;

/* リサイズ用構造体とグローバル */
typedef struct {
	Boolean	resizeImage;
	Boolean	dither;
	short	unit[2];
	Boolean	constRatio;
} ResizeDataRec;

enum {
	unitPixels=1,
	unitPercent,
};

static ResizeDataRec gResizeData={false,false,{unitPixels,unitPixels},false};

#pragma options align=reset

/* prototype */
static pascal Boolean	MyTransparentColorSearch(RGBColor *color,long *pos);

static Boolean	DragSelection(WindowPtr theWindow,Point mousePt,Point *result,Boolean shiftDown);

static pascal Boolean	SearchColorAreaProc(RGBColor *color,long *pos);

/* アイコン補助 */
static void	ReduceRect(Rect *result,short size);

/* マスク作成 */
static pascal Boolean	MySearchProc(RGBColor *color,long *pos);

static OSErr	PaintOffPort(WindowPtr theWindow,short mode,Point mousePt,Boolean shiftDown);

static void	TrackMarquee(PaintWinRec *eWinRec,Rect *startRect,Boolean shiftDown);
static void	SortRect(Rect *r,Boolean shiftDown);

static void	FixToGrid(Point *mousePt);
static void FixToGrid2(Point *mousePt);

static Boolean	PtInMask(Point pt,PaintWinRec *eWinRec);

static void	Pressure2Ratio(RGBColor *ratio);

static void	ChangeIconSizeMain(WindowPtr theWindow,Rect *newSize,short background);
static void	IsMarginExists(DialogPtr dp,short width,short height,ResizeDataRec *resizePrefs);

/* BitMap関連 */
static OSErr	NewBitMap(MyBitMapRec *bmpRec,Rect *rect);

/* リージョン */
static void	UnionRectRgn(RgnHandle dstRgn,Rect *srcRect);


#define PAINTERR_RESID	4006
#define	PAINTERR1	1
#define	PAINTERR2	2


/* 鉛筆ツールによる描画 */
void DoPaint(WindowPtr theWindow,Point pt,Boolean shiftDown)
{
	Point	mousePt;
	RGBColor	newColor;
	RGBColor	blendRatio2;
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	short	ratio=eWinRec->ratio;
	short	mode;
	OSErr	err;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	/* 描画に入る前にカーソルを更新 */
	SetPenCursor(kPencilTool);
	
	GetGWorld(&cPort,&cDevice);
	GoOffPort(theWindow);
	
	SetGWorld(eWinRec->editDataPtr,0);
	mousePt=pt;
	ToRealPos(&mousePt,ratio);
	if (gCurrentColor.isTransparent)
	{
		mode=kEraseMaskMode; /* マスクを消す */
		PenMode(srcCopy);
	}
	else
	{
		Boolean	usePressure=isTabletAvailable && gTabletPrefs.usePressure && IsTabletInUse();
		Boolean isEraserMode = (gToolPrefs.eraseSameColor && /* 同じ色 */
			 (!gToolPrefs.eraseSize11 || (gPenWidth==1 && gPenHeight==1)) && /* 1*1の場合のみ */
			 !usePressure && gBlendMode == srcCopy && /* タブレットの筆圧使用、あるいは、不透明度が100%以外か */
			 PtInMask(mousePt,eWinRec)); /* マスク内か */
		
		if (isEraserMode)
		{
			LockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
			if (eWinRec->isSelected)
				GetCPixel(mousePt.h-eWinRec->selectionOffset.h,mousePt.v-eWinRec->selectionOffset.v,&newColor);
			else
				GetCPixel(mousePt.h,mousePt.v,&newColor);
			UnlockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
		}
		/* 描画開始の点の色が選択色と同一ならば... */
		if (isEraserMode && EqualColor(&newColor,&gCurrentColor.rgb))
		{
			mode=kEraserMode; /* 消す */
			PenMode(srcCopy);
		}
		else
		{
			mode=kPencilMode;
			PenMode(gBlendMode);
			
			if (usePressure)
			{
				/* タブレット使用時 */
				Pressure2Ratio(&blendRatio2);
				OpColor(&blendRatio2);
				PenMode(blend);
			}
			else
				OpColor(&gBlendRatio);
		}
	}
	
	PenSize(gPenWidth,gPenHeight); /* 鉛筆ツールのサイズによってペンサイズを変える */
	
	SetGWorld(cPort,cDevice);
	err=PaintOffPort(theWindow,mode,mousePt,shiftDown);
	PenNormal();
	SetGWorld(cPort,cDevice);
	
	if (err==noErr)
	{
		UpdateTransparentMenu();
		SetUndoMode(umPaint);
		if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
			UpdatePaletteCheck();
	}
}

void Pressure2Ratio(RGBColor *ratio)
{
	unsigned short	pressure;
	
	pressure=GetPressure();
	SetRGBColor(ratio,pressure,pressure,pressure);
}

/* 消しゴムツールでの消去 */
void DoErase(WindowPtr theWindow,Point pt,Boolean shiftDown)
{
	Point	mousePt;
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	short	ratio=eWinRec->ratio;
	RGBColor	blendRatio2;
	OSErr	err;
	short	mode=kEraserMode;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	/* 描画に入る前にカーソルを更新 */
	SetPenCursor(kEraserTool);
	
	GetGWorld(&cPort,&cDevice);
	GoOffPort(theWindow);
	
	SetGWorld(eWinRec->editDataPtr,0);
	mousePt=pt;
	ToRealPos(&mousePt,ratio);
	if (gBackColor.isTransparent)
	{
		mode=kEraseMaskMode;
		PenMode(srcCopy);
	}
	else
	{
		if (isTabletAvailable && gTabletPrefs.useEraserPressure && IsTabletInUse() &&
				(eWinRec->backgroundGWorld == nil && !eWinRec->isBackTransparent))
		{
			/* タブレット使用時 */
			Pressure2Ratio(&blendRatio2);
			OpColor(&blendRatio2);
			PenMode(blend);
		}
		else
			PenMode(srcCopy);
	}
	
	PenSize(gEraserWidth,gEraserHeight); /* 消しゴムツールのサイズによってペンサイズを変える */
	
	SetGWorld(cPort,cDevice);
	err=PaintOffPort(theWindow,mode,mousePt,shiftDown);
	PenNormal();
	SetGWorld(cPort,cDevice);
	
	if (err==noErr)
	{
		UpdateTransparentMenu();
		SetUndoMode(umPaint);
		if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
			UpdatePaletteCheck();
	}
}

/* 描画点ツールでの描画 */
void DoDotPaint(WindowPtr theWindow,Point pt)
{
	Point	mousePt;
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	short	ratio=eWinRec->ratio;
	Point	offsetPt;
	
	SetPt(&offsetPt,-gPenWidth/2,-gPenHeight/2);
	mousePt=pt;
	ToRealPos(&mousePt,ratio);
	AddPt(offsetPt,&mousePt);
	
	/* 描画点モードに移行、データを初期化 */
	eWinRec->isDotMode=true;
	eWinRec->dotPos=mousePt;
	eWinRec->dotCommand[0]=0;
	eWinRec->isInverted=false;
	eWinRec->lastInvertedTime=TickCount();
	
	eWinRec->firstDotPos=mousePt;
	eWinRec->prevDotCommand[0]=0;
	
	ShowReferencedWindow(DotModePalette);
	
	UpdateEffectMenu();
}

/* 描画点ツールによる描画（メイン） */
void DoDotPaintMain(WindowPtr theWindow,Point pt)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	RGBColor	newColor;
	short		mode;
	Boolean		isBackMode=(eWinRec->backgroundGWorld==nil && !eWinRec->isBackTransparent);
	RgnHandle	maskRgn=NewRgn();
	Rect		redrawRect;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	GetGWorld(&cPort,&cDevice);
	ResetRuler();
	
	if (eWinRec->isSelected)
		SubPt(eWinRec->selectionOffset,&pt);
	
	/* 取り消しのモードが描画点による描画でなければバックアップ */
	if (eWinRec->undoMode != umDotPaint)
	{
		GoOffPort(theWindow);
		SetEmptyRgn(eWinRec->updateRgn);
		
		eWinRec->firstDotPos=pt;
		PStrCpy(eWinRec->dotCommand,eWinRec->prevDotCommand);
	}
	
	SetGWorld(eWinRec->editDataPtr,0);
	
	if (PtInRect(pt,&eWinRec->iconSize))
	{
		/* 透明かどうか */
		if (gCurrentColor.isTransparent)
		{
			mode=kEraseMaskMode;
			ForeColor(whiteColor);
			goto next;
		}
		else
		{
			Boolean isEraserMode = (gToolPrefs.eraseSameColor &&
				(!gToolPrefs.eraseSize11 || (gPenWidth==1 && gPenHeight==1)) &&
				gBlendMode == srcCopy && 
				PtInMask(pt,eWinRec));
			
			if (isEraserMode)
			{
				/* 描画しようとしている点の色を調べる */
				LockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
				GetCPixel(pt.h,pt.v,&newColor);
				UnlockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
			}
			/* 描画しようとしている色と同一なら…… */
			if (isEraserMode && EqualColor(&newColor,&gCurrentColor.rgb))
			{
				mode=kEraserMode; /* 消す */
				if (isBackMode)
					RGBForeColor(&gBackColor.rgb);
				else
					ForeColor(whiteColor);
				
				PenMode(srcCopy);
				goto next;
			}
		}
	}
	
	mode=kPencilMode;
	
	PenMode(gBlendMode);
	OpColor(&gBlendRatio);
	RGBForeColor(&gCurrentColor.rgb);
	
next:
	PenSize(gPenWidth,gPenHeight);
	MoveTo(pt.h,pt.v);
	Line(0,0);
	
	ForeColor(blackColor);
	PenNormal();
	
	SetRect(&redrawRect,pt.h,pt.v,pt.h+gPenWidth,pt.v+gPenHeight);
	SectRect(&redrawRect,&eWinRec->iconSize,&redrawRect);
	
	/* マスク処理 */
	SetGWorld(eWinRec->currentMask,0);
	LockPixels(GetGWorldPixMap(eWinRec->currentMask));
	if (mode==kEraseMaskMode || (mode==kEraserMode && !isBackMode))
		EraseRect(&redrawRect);
	else
		PaintRect(&redrawRect);
	
	if (eWinRec->isSelected)
		OffsetRect(&redrawRect,eWinRec->selectionOffset.h,eWinRec->selectionOffset.v);
	RectRgn(maskRgn,&redrawRect);
	UnionRgn(eWinRec->updateRgn,maskRgn,eWinRec->updateRgn);
	DisposeRgn(maskRgn);
	
	SetGWorld(cPort,cDevice);
	DispOffPort(theWindow);
	
	UpdateTransparentMenu();
	SetUndoMode(umDotPaint);
	if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
		UpdatePaletteCheck();
}

/* 選択ツールによる選択 */
void DoSelect(WindowPtr theWindow,Point pt,Boolean shiftDown,Boolean optDown,Boolean cmdDown)
{
	Point	mousePt;
	Point	result;
	Rect	startRect;
	MyBitMapRec	selectBM;
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	short	ratio=eWinRec->ratio;
	OSErr	err;
	Rect	tempRect;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	mousePt=pt;
	GetGWorld(&cPort,&cDevice);
	
	if (PtInRgn(mousePt,eWinRec->eSelectedRgn)) /* 選択範囲内でのドラッグならば */
	{
		if (optDown) /* オプションキーが押されていれば複製する */
		{
			SetGWorld(eWinRec->selectedDataPtr,0);
			CopyBits(GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
				GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
				&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,0);
			
			/* マスクも複製する */
			SetGWorld(eWinRec->selectionMask,0);
			GetRegionBounds(eWinRec->selectionPos,&tempRect);
			CopyBits(GetPortBitMapForCopyBits(eWinRec->currentMask),
				GetPortBitMapForCopyBits(eWinRec->selectionMask),
				&eWinRec->selectedRect,&tempRect,srcOr,nil);
			SetGWorld(cPort,cDevice);
		}
		
		/* 選択範囲を移動させる */
		ToRealPos(&mousePt,ratio);
		if (DragSelection(theWindow,mousePt,&result,shiftDown))
		{
			if (result.h!=0 || result.v!=0)
				MoveSelection(theWindow,result.h,result.v,false);
			else
				if (eWinRec->showSelection)
					DispSelection(theWindow);
		}
		else
		{
			/* クリックだけされた時 */
			RgnHandle	tempRgn=NewRgn();
			
			if (eWinRec->showSelection)
				DispSelection(theWindow);
			
			/* 選択領域が全体なら固定する */
			RectRgn(tempRgn,&eWinRec->iconSize);
			if (EqualRgn(eWinRec->selectionPos,tempRgn))
				FixSelection(theWindow);
			
			DisposeRgn(tempRgn);
		}
	}
	else /* 選択範囲内でなければ選択 */
	{
		RgnHandle	selectRgn;
		
		/* カーソル更新 */
		MySetCursor(128+kMarqueeTool);
		
		/* 選択範囲にマーキーを表示する処理 */
		ToRealPos(&mousePt,ratio);
		if (shiftDown)
			FixToGrid(&mousePt);
		Pt2Rect(mousePt,mousePt,&startRect);
		TrackMarquee(eWinRec,&startRect,shiftDown);
		
		if (eWinRec->isSelected) /* すでに選択範囲があれば固定する */
			FixSelection(theWindow);
		
		if (EmptyRect(&startRect)) /* どこも選択されなければ終わり */
			return;
		
		selectRgn=NewRgn();
		UpdatePSRgn(eWinRec);
		
		GoOffPort(theWindow);
		if (cmdDown) /* コマンドキーを押しながら選択したとき */
		{
			/* マスクパターンの作成 */
			err=CreateMask(eWinRec->editDataPtr,&startRect,&startRect,&selectBM);
			if (err==noErr)
			{
				err=BitMapToRegion(selectRgn,&selectBM.bmp);
				DisposeBitMap(&selectBM);
			}
			if (err!=noErr)
			{
				SysBeep(0);
				RectRgn(selectRgn,&startRect);
			}
			
			if (EmptyRgn(selectRgn)) /* どこも選択されなければ終わり */
			{
				DisposeRgn(selectRgn);
				SetEmptyRgn(eWinRec->eSelectedRgn);
				eWinRec->isSelected=false;
				eWinRec->selectedRect=eWinRec->iconSize;
				RectRgn(eWinRec->selectionPos,&eWinRec->iconSize);
				DispOffPort(theWindow);
				return;
			}
		}
		else /* 普通の選択 */
			RectRgn(selectRgn,&startRect);
		
		DoSelectMain(theWindow,selectRgn);
	}
}

/* 選択処理メイン（selectRgnはルーチン内で破棄） */
void DoSelectMain(WindowPtr theWindow,RgnHandle selectRgn)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	Boolean		isBackMode=(eWinRec->backgroundGWorld == nil && !eWinRec->isBackTransparent);
	Rect		tempRect;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	GetGWorld(&cPort,&cDevice);
	
	/* 選択領域リージョン初期化 */
	CopyRgn(selectRgn,eWinRec->selectionPos);
	DisposeRgn(selectRgn);
	GetRegionBounds(eWinRec->selectionPos,&eWinRec->selectedRect);
	SetPt(&eWinRec->selectionOffset,0,0);
	SetPt(&eWinRec->prevMovement,0,0);
	
	CopyRgn(eWinRec->selectionPos,eWinRec->eSelectedRgn);
	MapRgn(eWinRec->eSelectedRgn,&eWinRec->iconSize,&eWinRec->iconEditSize);
	
	/* 選択処理 */
	SetGWorld(eWinRec->selectedDataPtr,0);
	MyLockPixels(selectedDataPtr);
	EraseRect(&eWinRec->iconSize);
	CopyBits(GetPortBitMapForCopyBits(eWinRec->editDataPtr),
		GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
		&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,0);
	
	if (isBackMode) /* 背景がない場合は背景色で塗る */
		RGBBackColor(&gBackColor.rgb);
	EraseRgn(eWinRec->selectionPos);
	BackColor(whiteColor);
	UnlockPixels(GetGWorldPixMap(eWinRec->selectedDataPtr));
	
	eWinRec->isSelected=true;
	eWinRec->selectionBlend=srcCopy;
	eWinRec->showSelection=true;
	
	SetGWorld(eWinRec->dispTempPtr,0);
	CopyBits(GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
		GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
		&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
	GetRegionBounds(eWinRec->selectionPos,&tempRect);
	CopyBits(GetPortBitMapForCopyBits(eWinRec->editDataPtr),
		GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
		&tempRect,&tempRect,
		eWinRec->selectionBlend,eWinRec->selectionPos);
	
	CopyRgn(eWinRec->selectionPos,eWinRec->updateRgn);
	
	/* マスクの更新 */
	/* 選択範囲外 */
	SetGWorld(eWinRec->selectionMask,0);
	LockPixels(GetGWorldPixMap(eWinRec->selectionMask));
	EraseRect(&eWinRec->iconSize);
	CopyBits(GetPortBitMapForCopyBits(eWinRec->currentMask),
		GetPortBitMapForCopyBits(eWinRec->selectionMask),
		&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
	if (!isBackMode || gBackColor.isTransparent) /* 背景がある、あるいは背景色が透明の時はマスク削除 */
		EraseRgn(eWinRec->selectionPos);
	else /* それ以外の場合はマスク追加（？） */
		PaintRgn(eWinRec->selectionPos);
	UnlockPixels(GetGWorldPixMap(eWinRec->selectionMask));
	
	/* 選択範囲内 */
	SetGWorld(eWinRec->currentMask,0);
	LockPixels(GetGWorldPixMap(eWinRec->currentMask));
	EraseRect(&eWinRec->iconSize);
	CopyBits(GetPortBitMapForCopyBits(eWinRec->pCurrentMask),
		GetPortBitMapForCopyBits(eWinRec->currentMask),
		&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,eWinRec->selectionPos);
	UnlockPixels(GetGWorldPixMap(eWinRec->currentMask));
	
	SetGWorld(cPort,cDevice);
	DispSelection(theWindow);
	
	UpdateClipMenu();
	UpdateSelectionMenu();
	UpdateTransparentMenu();
	
	SetUndoMode(umSelect);
}

/* スポイトツールによる色選択 */
void DoSpoit(WindowPtr theWindow,Point pt)
{
	Point		mousePt=pt;
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	RGBColor	newColor;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	/* GWorld保存 */
	GetGWorld(&cPort,&cDevice);
	ToRealPos(&mousePt,eWinRec->ratio);
	
	if (PtInMask(mousePt,eWinRec) || (eWinRec->isSelected && RealPtInMask(mousePt,eWinRec->selectionMask)))
	{
		/* マスク内なら表示用テンポラリから色を得る */
		MySetGWorld(dispTempPtr);
		MyLockPixels(dispTempPtr);
		GetCPixel(mousePt.h,mousePt.v,&newColor);
		MyUnlockPixels(dispTempPtr);
	}
	else
	{
		/* マスク外 */
		if (eWinRec->backgroundGWorld == nil)
		{
			/* 背景がない→背景編集中 */
			newColor.red=newColor.blue=newColor.green = 0xffff;
			
			/* ここで透明色がとれるようにするかどうかは検討の余地がある */
		}
		else
		{
			if (!eWinRec->dispBackground) return; /* 背景非表示時はなにもしない */
			
			MySetGWorld(backgroundGWorld);
			MyLockPixels(backgroundGWorld);
			GetCPixel(mousePt.h,mousePt.v,&newColor);
			MyUnlockPixels(backgroundGWorld);
		}
	}
	/* GWorld復帰 */
	SetGWorld(cPort,cDevice);
	ChangeColor(&newColor,false);
}

/* バケツツールによる塗りつぶし */
void DoBucket(WindowPtr theWindow,Point pt,Boolean cmdDown)
{
	Point	mousePt;
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	short	ratio=eWinRec->ratio;
	Rect	effectRect=eWinRec->selectedRect;
	Boolean	srcIsTransparent;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	mousePt=pt;
	GetGWorld(&cPort,&cDevice);
	ToRealPos(&mousePt,ratio);
	if (eWinRec->isSelected)
		SubPt(eWinRec->selectionOffset,&mousePt);
	
	if (!PtInRect(mousePt,&effectRect)) return; /* クリックされたのが選択範囲外 */
	srcIsTransparent=!RealPtInMask(mousePt,eWinRec->currentMask);
	
	if (cmdDown) /* 置き換え */
	{
		if (srcIsTransparent) /* 透明色 */
		{
			if (gCurrentColor.isTransparent) return;
			
			DoReplaceColor(theWindow,&rgbWhiteColor,true,&gCurrentColor.rgb,gCurrentColor.isTransparent);
		}
		else /* 透明色でない */
		{
			RGBColor	newColor;
			
			/* 塗る場所の色を求める */
			SetGWorld(eWinRec->editDataPtr,0);
			LockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
			GetCPixel(mousePt.h,mousePt.v,&newColor);
			UnlockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
			SetGWorld(cPort,cDevice);
			
			if (!gCurrentColor.isTransparent && EqualColor(&newColor,&gCurrentColor.rgb)) return; /* 同じ色なら何もしない */
			
			DoReplaceColor(theWindow,&newColor,false,&gCurrentColor.rgb,gCurrentColor.isTransparent);
		}
		SetUndoMode(umEffect);
	}
	else /* バケツ */
	{
		OSErr	err;
		RgnHandle	fillRgn;
		MyBitMapRec	fillBM;
		Pattern	pat;
		
		/* 塗る範囲を求める */
		err=NewBitMap(&fillBM,&effectRect);
		if (err!=noErr)
		{
			ErrorAlertFromResource(PAINTERR_RESID,PAINTERR1);
			return;
		}
		
		if (srcIsTransparent) /* 透明色 */
		{
			/* マスクだけを考慮 */
			SetGWorld(eWinRec->currentMask,0);
			MyLockPixels(currentMask);
			SeedCFill(GetPortBitMapForCopyBits(eWinRec->currentMask),&fillBM.bmp,
				&effectRect,&effectRect,mousePt.h,mousePt.v,nil,0);
			MyUnlockPixels(currentMask);
		}
		else
		{
			RGBColor	newColor;
			Boolean		isWhite;
			
			SetGWorld(eWinRec->editDataPtr,0);
			LockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
			GetCPixel(mousePt.h,mousePt.v,&newColor);
			isWhite= (newColor.red == 0xffff && newColor.blue == 0xffff && newColor.green == 0xffff);
			if (isWhite) /* 白の時はマスクにより画像を反転し、透明部分と区別する */
			{
				CopyBits(GetPortBitMapForCopyBits(eWinRec->currentMask),
					GetPortBitMapForCopyBits(eWinRec->editDataPtr),
					&effectRect,&effectRect,srcXor,nil);
			}
			
			SeedCFill(GetPortBitMapForCopyBits(eWinRec->editDataPtr),&fillBM.bmp,
				&effectRect,&effectRect,mousePt.h,mousePt.v,nil,0);
			
			if (isWhite) /* 白 */
			{
				CopyBits(GetPortBitMapForCopyBits(eWinRec->currentMask),
					GetPortBitMapForCopyBits(eWinRec->editDataPtr),
					&effectRect,&effectRect,srcXor,nil);
			}
			
			UnlockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
		}
		SetGWorld(cPort,cDevice);
		
		/* もとまった範囲をリージョンにする */
		fillRgn=NewRgn();
		err=BitMapToRegion(fillRgn,&fillBM.bmp);
		if (err!=noErr)
		{
			ErrorAlertFromResource(PAINTERR_RESID,PAINTERR1);
			DisposeBitMap(&fillBM);
			DisposeRgn(fillRgn);
			return;
		}
		
		err=DisposeBitMap(&fillBM);
		
		GoOffPort(theWindow);
		
		/* 塗る */
		SetGWorld(eWinRec->editDataPtr,0);
		MyLockPixels(editDataPtr);
		RGBForeColor(&gCurrentColor.rgb);
		RGBBackColor(&gBackColor.rgb);
		GetIndPattern(&pat,(gOtherPrefs.useHyperCardPalette ? 201 : 200),gPatternNo);
		FillRgn(fillRgn,&pat);
		MyUnlockPixels(editDataPtr);
		
		/* 色を元に戻す */
		ForeColor(blackColor);
		BackColor(whiteColor);
		
		/* マスクに関する処理 */
		{
			Boolean	isBlackPat=(gOtherPrefs.useHyperCardPalette && gPatternNo==12) ||
								(!gOtherPrefs.useHyperCardPalette && gPatternNo==1),
					isWhitePat=(gOtherPrefs.useHyperCardPalette && gPatternNo==1) ||
								(!gOtherPrefs.useHyperCardPalette && gPatternNo==13);
			Boolean	isOpaque=(!gCurrentColor.isTransparent && !gBackColor.isTransparent) ||
								(!gCurrentColor.isTransparent && isBlackPat) ||
								(!gBackColor.isTransparent && isWhitePat),
					isTransparent=(gCurrentColor.isTransparent && gBackColor.isTransparent) ||
								(gCurrentColor.isTransparent && isBlackPat) ||
								(gBackColor.isTransparent && isWhitePat);
			
			SetGWorld(eWinRec->currentMask,0);
			LockPixels(GetGWorldPixMap(eWinRec->currentMask));
			if (isOpaque || isTransparent) /* 完全に透明 or 不透明 */
			{
				/* マスク処理（選択範囲内に限定していないからちょっとまずいかも） */
				if (isTransparent)
					EraseRgn(fillRgn);
				else
					PaintRgn(fillRgn);
			}
			else /* パターンが関係する */
			{
				ForeColor((gCurrentColor.isTransparent ? whiteColor : blackColor));
				BackColor((gBackColor.isTransparent ? whiteColor : blackColor));
				
				FillRgn(fillRgn,&pat);
			}
			UnlockPixels(GetGWorldPixMap(eWinRec->currentMask));
		}
		CopyRgn(fillRgn,eWinRec->updateRgn); /* 塗ったところが更新範囲 */
		if (eWinRec->isSelected) /* 選択範囲がある時は、実際の表示位置にずらす */
			OffsetRgn(eWinRec->updateRgn,eWinRec->selectionOffset.h,eWinRec->selectionOffset.v);
		DisposeRgn(fillRgn);
		
		SetGWorld(cPort,cDevice);
		DispOffPort(theWindow);
		
		UpdateTransparentMenu();
		SetUndoMode(umPaint);
	}
	if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
		UpdatePaletteCheck();
}

/* 透明色を考慮したバケツ塗り */
pascal Boolean MyTransparentColorSearch(RGBColor *color,long *pos)
{
	GWorldPtr	cPort;
	GDHandle	cDevice;
	MatchRec	*matchData;
	BucketDataRec	*bucketData;
	
	GetGWorld(&cPort,&cDevice);
	matchData=(MatchRec *)(**cDevice).gdRefCon;
	bucketData=(BucketDataRec *)matchData->matchData;
	
	if (bucketData->srcIsTransparent) /* 塗る場所が透明 */
		return RealPtInMask(*(Point*)pos,bucketData->maskGWorld);
	else
	{
		if (matchData->red == color->red && matchData->green == color->green && matchData->blue == color->blue)
			return !RealPtInMask(*(Point*)pos,bucketData->maskGWorld);
		else
			return true;
	}
}

/* 色の置き換え */
void DoReplaceColor(WindowPtr theWindow,RGBColor *srcColor,Boolean srcIsTransparent,
										RGBColor *dstColor,Boolean dstIsTransparent)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	Rect	effectRect=eWinRec->selectedRect;
	OSErr	err;
	GWorldPtr	tempGWorld;
	long	fgColor;
	CTabHandle	ctab;
	ColorSpec	*ctTable;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	/* 同じ色なら何もしない */
	if (EqualColor(srcColor,dstColor) && srcIsTransparent==dstIsTransparent) return;
	
	GetGWorld(&cPort,&cDevice);
	
	if (!srcIsTransparent)
	{
		err=NewGWorld(&tempGWorld,eWinRec->iconDepth,&effectRect,0,0,useTempMem);
		
		if (err!=noErr)
		{
			ErrorAlertFromResource(PAINTERR_RESID,PAINTERR1);
			return;
		}
	}

	GoOffPort(theWindow);
	
	if (!srcIsTransparent) /* 置き換え元が透明色でない */
	{
		SetGWorld(tempGWorld,0);
		
		LockPixels(GetGWorldPixMap(tempGWorld));
		EraseRect(&effectRect);
		
		CopyBits(GetPortBitMapForCopyBits(eWinRec->editDataPtr),
			GetPortBitMapForCopyBits(tempGWorld),
			&effectRect,&effectRect,srcCopy,nil);
		
		UnlockPixels(GetGWorldPixMap(tempGWorld));
		
		switch (eWinRec->colorMode)
		{
			case kNormal8BitColorMode:
				RGBForeColor(srcColor);
				fgColor=Color2Index(srcColor);
				ctab=(**GetGWorldPixMap(tempGWorld)).pmTable;
				ctTable=(**ctab).ctTable;
				ctTable[fgColor].rgb=*dstColor;
				CTabChanged(ctab);
				
				SetGWorld(eWinRec->editDataPtr,0);
				CopyBits(GetPortBitMapForCopyBits(tempGWorld),
					GetPortBitMapForCopyBits(eWinRec->editDataPtr),
					&effectRect,&effectRect,srcCopy,nil);
				break;
			
			case k32BitColorMode:
				break;
		}
		
		DisposeGWorld(tempGWorld);
		
		/* マスク更新 */
		if (!dstIsTransparent) /* 置き換え先が透明色以外 */
		{
			/*　透明色以外の色を透明色以外の色で置き換えるのだからマスクの変更はない */
		}
		else /* 置き換え先が透明色 */
		{
			if (EqualColor(srcColor,&rgbWhiteColor)) /* 置き換え元の色が白 */
			{
				OSErr		err;
				GWorldPtr	maskGWorld;
				
				err=CreateMask2(eWinRec->editDataPtr,&maskGWorld);
				if (err==noErr)
				{
					MySetGWorld(currentMask);
					CopyBits(GetPortBitMapForCopyBits(maskGWorld),
						GetPortBitMapForCopyBits(eWinRec->currentMask),
						&effectRect,&effectRect,srcCopy,nil);
					SetGWorld(cPort,cDevice);
					
					DisposeGWorld(maskGWorld);
				}
				else
				{
					SysBeep(0);
				}
			}
			else
			{
				OSErr		err;
				GWorldPtr	oldMask,newMask;
				
				err=CreateMask2(eWinRec->tempDataPtr,&oldMask);
				if (err==noErr)
				{
					err=CreateMask2(eWinRec->editDataPtr,&newMask);
					if (err==noErr)
					{
						SetGWorld(newMask,0);
						CopyBits(GetPortBitMapForCopyBits(oldMask),
							GetPortBitMapForCopyBits(newMask),
							&effectRect,&effectRect,notSrcXor,nil);
						MySetGWorld(currentMask);
						CopyBits(GetPortBitMapForCopyBits(newMask),
							GetPortBitMapForCopyBits(eWinRec->currentMask),
							&effectRect,&effectRect,notSrcBic,nil);
						
						DisposeGWorld(oldMask);
						DisposeGWorld(newMask);
					}
					else
					{
						SysBeep(0);
						DisposeGWorld(oldMask);
					}
				}
				else
					SysBeep(0);
			}
		}
	}
	else /* 置き換え元が透明色→全体を塗りつぶし、マスク内だけ元に戻す */
	{
		RgnHandle	selectedRgn=NewRgn();
		Rect		tempRect;
		
		MySetGWorld(editDataPtr);
		MyLockPixels(editDataPtr);
		RGBForeColor(dstColor);
		PaintRect(&effectRect);
		
		ForeColor(blackColor);
		
		SafeCopyDeepMask(GetPortBitMapForCopyBits(eWinRec->tempDataPtr),
			GetPortBitMapForCopyBits(eWinRec->currentMask),
			GetPortBitMapForCopyBits(eWinRec->editDataPtr),
			&effectRect,&effectRect,&effectRect,
			srcCopy,nil);
		
		MyUnlockPixels(editDataPtr);
		
		CopyRgn(eWinRec->selectionPos,selectedRgn);
		GetRegionBounds(eWinRec->selectionPos,&tempRect);
		MapRgn(selectedRgn,&tempRect,&effectRect);
		MySetGWorld(currentMask);
		MyLockPixels(currentMask);
		PaintRgn(selectedRgn);
		MyUnlockPixels(currentMask);
		DisposeRgn(selectedRgn);
	}
	
	CopyRgn(eWinRec->selectionPos,eWinRec->updateRgn);
	SetGWorld(cPort,cDevice);
	DispOffPort(theWindow);
	
	eWinRec->replacedColor=*srcColor;
	eWinRec->replacedTransparent=srcIsTransparent;
	gUsedCount.replaceNum++;
	UpdateTransparentMenu();
	SetUndoMode(umReplaceColor);
	if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
		UpdatePaletteCheck();
}

/* 自動選択 */
void DoAutoSelect(WindowPtr theWindow)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	Rect	tempRect;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	GetGWorld(&cPort,&cDevice);
	
	if (eWinRec->undoMode == umPaint)
	{
		RgnHandle	tempRgn=NewRgn();
		
		ResetDot(theWindow);
		GoOffPort(theWindow);
		
		if (eWinRec->isSelected) /* 選択されている */
		{
			if (eWinRec->showSelection)
				DispSelection(theWindow);
			
			DiffRgn(eWinRec->selectionPos,eWinRec->updateRgn,tempRgn);
			OffsetRgn(tempRgn,-eWinRec->selectionOffset.h,-eWinRec->selectionOffset.v);
			
			SetGWorld(eWinRec->selectedDataPtr,0);
			if (eWinRec->selectionBlend==blend)
				OpColor(&rgbGrayColor);
			GetRegionBounds(eWinRec->selectionPos,&tempRect);
			SafeCopyDeepMask(GetPortBitMapForCopyBits(eWinRec->tempDataPtr),
				GetPortBitMapForCopyBits(eWinRec->pCurrentMask),
				GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
				&eWinRec->selectedRect,&eWinRec->selectedRect,&tempRect,
				eWinRec->selectionBlend,eWinRec->selectionPos);
			
			SetGWorld(eWinRec->selectionMask,0);
			CopyBits(GetPortBitMapForCopyBits(eWinRec->pCurrentMask),
				GetPortBitMapForCopyBits(eWinRec->selectionMask),
				&eWinRec->selectedRect,&tempRect,srcOr,eWinRec->selectionPos);
		}
		else /* 選択されていない */
		{
			RectRgn(tempRgn,&eWinRec->iconSize);
			DiffRgn(tempRgn,eWinRec->updateRgn,tempRgn);
			
			SetGWorld(eWinRec->selectedDataPtr,0);
			CopyBits(GetPortBitMapForCopyBits(eWinRec->tempDataPtr),
				GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
				&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
			SetGWorld(eWinRec->selectionMask,0);
			CopyBits(GetPortBitMapForCopyBits(eWinRec->pCurrentMask),
				GetPortBitMapForCopyBits(eWinRec->selectionMask),
				&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
			
			SetPt(&eWinRec->selectionOffset,0,0);
			
			eWinRec->isSelected=true;
			eWinRec->selectionBlend=srcCopy;
		}
		
		/* 選択部分の更新 */
		SetGWorld(eWinRec->editDataPtr,0);
		MyLockPixels(editDataPtr);
		EraseRgn(tempRgn);
		MyUnlockPixels(editDataPtr);
		SetGWorld(eWinRec->currentMask,0);
		MyLockPixels(currentMask);
		EraseRgn(tempRgn);
		MyUnlockPixels(currentMask);
		DisposeRgn(tempRgn);
		
		/* 選択範囲の更新 */
		CopyRgn(eWinRec->updateRgn,eWinRec->selectionPos);
		GetRegionBounds(eWinRec->selectionPos,&eWinRec->selectedRect);
		if (eWinRec->isSelected)
			OffsetRect(&eWinRec->selectedRect,-eWinRec->selectionOffset.h,-eWinRec->selectionOffset.v);
		CopyRgn(eWinRec->updateRgn,eWinRec->eSelectedRgn);
		MapRgn(eWinRec->eSelectedRgn,&eWinRec->iconSize,&eWinRec->iconEditSize);
		RectRgn(eWinRec->prevSelectionPos,&eWinRec->iconSize);
		SetEmptyRgn(eWinRec->ePrevSelectedRgn);
		eWinRec->prevSelectedRect=eWinRec->iconSize;
		
		SetPt(&eWinRec->prevMovement,0,0);
		eWinRec->showSelection=true;
		
		SetGWorld(cPort,cDevice);
		DispOffPort(theWindow);
		ToolSelect(kMarqueeTool);
		SetUndoMode(umSelect);
		UpdateClipMenu();
		UpdateSelectionMenu();
		UpdateTransparentMenu();
	}
	else
	{
		if (eWinRec->isSelected) /* 選択されている */
		{
			MyBitMapRec	selectBM;
			OSErr	err;
			RgnHandle	tempRgn=NewRgn();
			
			err=CreateMask(eWinRec->editDataPtr,&eWinRec->selectedRect,&eWinRec->selectedRect,&selectBM);
			if (err==noErr)
			{
				err=BitMapToRegion(tempRgn,&selectBM.bmp);
				DisposeBitMap(&selectBM);
			}
			if (err!=noErr || EmptyRgn(tempRgn))
			{
				DisposeRgn(tempRgn);
				SysBeep(0);
				return;
			}
			
			/* 選択領域リージョン初期化 */
			OffsetRgn(tempRgn,eWinRec->selectionOffset.h,eWinRec->selectionOffset.v);
			SectRgn(tempRgn,eWinRec->selectionPos,tempRgn);
			if (EqualRgn(tempRgn,eWinRec->selectionPos))
			{
				DisposeRgn(tempRgn);
				return;
			}
			
			if (eWinRec->showSelection)
				DispSelection(theWindow);
			
			UpdatePSRgn(eWinRec);
			GoOffPort(theWindow);
			
			/* リージョンを設定する */
			CopyRgn(tempRgn,eWinRec->selectionPos);
			OffsetRgn(tempRgn,-eWinRec->selectionOffset.h,-eWinRec->selectionOffset.v);
			GetRegionBounds(tempRgn,&eWinRec->selectedRect);
			SetPt(&eWinRec->prevMovement,0,0);
			
			CopyRgn(eWinRec->selectionPos,eWinRec->eSelectedRgn);
			MapRgn(eWinRec->eSelectedRgn,&eWinRec->iconSize,&eWinRec->iconEditSize);
			
			eWinRec->showSelection=true;
			
			CopyRgn(eWinRec->selectionPos,eWinRec->updateRgn);
			
			/* マスクの更新 */
			/* 選択範囲内 */
			SetGWorld(eWinRec->currentMask,0);
			MyLockPixels(currentMask);
			EraseRect(&eWinRec->iconSize);
			CopyBits(GetPortBitMapForCopyBits(eWinRec->pCurrentMask),
				GetPortBitMapForCopyBits(eWinRec->currentMask),
				&eWinRec->selectedRect,&eWinRec->selectedRect,srcCopy,tempRgn);
			DisposeRgn(tempRgn);
			MyUnlockPixels(currentMask);
			
			CopyRgn(eWinRec->prevSelectionPos,eWinRec->updateRgn);
			SetGWorld(cPort,cDevice);
			DispOffPort(theWindow);
			
			SetUndoMode(umSelect);
			UpdateClipMenu();
			UpdateSelectionMenu();
			UpdateTransparentMenu();
		}
		else
		{
			MyBitMapRec	selectBM;
			OSErr	err;
			RgnHandle	tempRgn=NewRgn();
			
			err=CreateMask(eWinRec->editDataPtr,&eWinRec->iconSize,&eWinRec->iconSize,&selectBM);
			if (err==noErr)
			{
				err=BitMapToRegion(tempRgn,&selectBM.bmp);
				DisposeBitMap(&selectBM);
			}
			if (err!=noErr || EmptyRgn(tempRgn))
			{
				DisposeRgn(tempRgn);
				SysBeep(0);
				return;
			}
			
			UpdatePSRgn(eWinRec);
			GoOffPort(theWindow);
			
			DoSelectMain(theWindow,tempRgn);
		}
	}
}

/* 選択範囲の移動 */
void MoveSelection(WindowPtr theWindow,short dx,short dy,Boolean redrawFlag)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	
	GoOffPort(theWindow);
	
	CopyRgn(eWinRec->selectionPos,eWinRec->prevSelectionPos);
	CopyRgn(eWinRec->eSelectedRgn,eWinRec->ePrevSelectedRgn);
	
	OffsetRgn(eWinRec->selectionPos,dx,dy);
	OffsetRgn(eWinRec->eSelectedRgn,dx<<eWinRec->ratio,dy<<eWinRec->ratio);
	
	/* マスクに関しては、処理する必要なし */
	
	SetPt(&eWinRec->prevMovement,dx,dy);
	AddPt(eWinRec->prevMovement,&eWinRec->selectionOffset);
		
	UnionRgn(eWinRec->selectionPos,eWinRec->prevSelectionPos,eWinRec->updateRgn);
	
	if (redrawFlag)
		DispOffPort(theWindow);
	if (eWinRec->showSelection)
		DispSelection(theWindow);
	
	SetUndoMode(umMove);
	if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
		UpdatePaletteCheck();
}

/* 選択範囲の固定 */
void FixSelection(WindowPtr theWindow)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	GWorldFlags	flags;
	OSErr		err;
	Rect		tempRect;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	GetGWorld(&cPort,&cDevice);
	
	/* 枠を消す */
	if (eWinRec->showSelection)
		DispSelection(theWindow);
	
	err=GoOffPort(theWindow);
	if (err!=noErr)
	{
		ErrorAlertFromResource(PAINTERR_RESID,PAINTERR2);
		SetUndoMode(umCannot);
		return;
	}
	
	GetPortBounds(eWinRec->editDataPtr,&tempRect);
	if (!EqualRect(&tempRect,&eWinRec->iconSize))
	{
		flags=UpdateGWorld(&eWinRec->editDataPtr,eWinRec->iconDepth,&eWinRec->iconSize,0,0,0);
		flags=UpdateGWorld(&eWinRec->currentMask,1,&eWinRec->iconSize,0,0,0);
	}
	
	SetGWorld(eWinRec->editDataPtr,0);
	/* 画面表示用のテンポラリから取り出す（手抜きだな） */
	CopyBits(GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
		GetPortBitMapForCopyBits(eWinRec->editDataPtr),
		&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
	
	/* マスクの更新 */
	SetGWorld(eWinRec->currentMask,0);
	CopyBits(GetPortBitMapForCopyBits(eWinRec->selectionMask),
		GetPortBitMapForCopyBits(eWinRec->currentMask),
		&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
	GetRegionBounds(eWinRec->selectionPos,&tempRect);
	CopyBits(GetPortBitMapForCopyBits(eWinRec->pCurrentMask),
		GetPortBitMapForCopyBits(eWinRec->currentMask),
		&eWinRec->selectedRect,&tempRect,srcOr,nil);
	
	/* 選択範囲の更新 */
	CopyRgn(eWinRec->selectionPos,eWinRec->prevSelectionPos);
	CopyRgn(eWinRec->eSelectedRgn,eWinRec->ePrevSelectedRgn);
	RectRgn(eWinRec->selectionPos,&eWinRec->iconSize);
	SetEmptyRgn(eWinRec->eSelectedRgn);
	eWinRec->isSelected=false;
	eWinRec->selectedRect=eWinRec->iconSize;
	
	/* 取り消しのためにselectionMaskは残しておく */
	
	/* 前回の選択範囲が更新範囲 */
	CopyRgn(eWinRec->prevSelectionPos,eWinRec->updateRgn);
	
	SetGWorld(cPort,cDevice);
	UpdateClipMenu();
	UpdateSelectionMenu();
	UpdateTransparentMenu();
	
	SetUndoMode(umDeselect);
}

/* 選択範囲と背景を交換 */
void ExchangeSelection(WindowPtr theWindow)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	Rect	tempRect,tempRect2,tempRect3;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	/* どこも選択されていなければエラー */
	if (!eWinRec->isSelected)
	{
		SysBeep(0);
		UpdateClipMenu();
		return;
	}
	
	GetGWorld(&cPort,&cDevice);
	GoOffPort(theWindow);
	
	/* 背景がない部分のための処理 */
	tempRect=eWinRec->iconSize;
	GetRegionBounds(eWinRec->selectionPos,&tempRect2);
	OffsetRect(&tempRect,eWinRec->selectedRect.left-tempRect2.left,
				eWinRec->selectedRect.top-tempRect2.top);
	
	/* 交換する */
	SetGWorld(eWinRec->editDataPtr,0);
	ClipRect(&tempRect);
	CopyBits(GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
		GetPortBitMapForCopyBits(eWinRec->editDataPtr),
		&tempRect2,&eWinRec->selectedRect,
		srcCopy,nil);
	GetPortBounds(eWinRec->editDataPtr,&tempRect3);
	ClipRect(&tempRect3);
	
	SetGWorld(eWinRec->selectedDataPtr,0);
	CopyBits(GetPortBitMapForCopyBits(eWinRec->tempDataPtr),
		GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
		&eWinRec->selectedRect,&tempRect2,
		srcCopy,eWinRec->selectionPos);
	
	/* マスクの処理 */
	SetGWorld(eWinRec->currentMask,0);
	ClipRect(&tempRect);
	CopyBits(GetPortBitMapForCopyBits(eWinRec->selectionMask),
		GetPortBitMapForCopyBits(eWinRec->currentMask),
		&tempRect2,&eWinRec->selectedRect,
		srcCopy,nil);
	GetPortBounds(eWinRec->currentMask,&tempRect3);
	ClipRect(&tempRect3);
	
	SetGWorld(eWinRec->selectionMask,0);
	CopyBits(GetPortBitMapForCopyBits(eWinRec->pCurrentMask),
		GetPortBitMapForCopyBits(eWinRec->selectionMask),
		&eWinRec->selectedRect,&tempRect2,
		srcCopy,eWinRec->selectionPos);
	
	CopyRgn(eWinRec->selectionPos,eWinRec->updateRgn);
	
	SetGWorld(cPort,cDevice);
	DispOffPort(theWindow);
	
	UpdateTransparentMenu();
	SetUndoMode(umExchange);
	if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
		UpdatePaletteCheck();
}	

static RGBColor	gSearchColor;

/* 指定の色の領域を選択 */
void DoSelectColorArea(WindowPtr theWindow,RGBColor *color,Boolean isTransparent)
{
	PaintWinRec	*eWinRec;
	GWorldPtr	selectGWorld;
	OSErr		err;
	RgnHandle	selectRgn;
	ColorSearchUPP	csUPP;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	if (theWindow==nil) return;
	eWinRec=GetPaintWinRec(theWindow);
	
	if (eWinRec->isSelected) /* 選択範囲があれば固定する */
		FixSelection(theWindow);
	
	UpdatePSRgn(eWinRec);
	GetGWorld(&cPort,&cDevice);
	err=GoOffPort(theWindow);
	
	if (isTransparent) /* マスク外のみ */
	{
		RgnHandle	tempRgn;
		
		selectRgn=NewRgn();
		err=BitMapToRegion(selectRgn,GetPortBitMapForCopyBits(eWinRec->currentMask));
		if (err!=noErr)
		{
			DisposeRgn(selectRgn);
			DispOffPort(theWindow);
			SetUndoMode(umCannot);
			SysBeep(0);
			return;
		}
		
		tempRgn=NewRgn();
		RectRgn(tempRgn,&eWinRec->iconSize);
		DiffRgn(tempRgn,selectRgn,selectRgn);
		DisposeRgn(tempRgn);
	}
	else
	{
		err=NewGWorld(&selectGWorld,1,&eWinRec->iconSize,0,0,useTempMem);
		if (err!=noErr)
		{
			DispOffPort(theWindow);
			SetUndoMode(umCannot);
			SysBeep(0);
			return;
		}
		
		/* 指定した色だけをコピーする */
		SetGWorld(selectGWorld,0);
		LockPixels(GetGWorldPixMap(selectGWorld));
		gSearchColor=*color;
		csUPP = NewColorSearchUPP(SearchColorAreaProc);
		AddSearch(csUPP);
		CopyBits(GetPortBitMapForCopyBits(eWinRec->editDataPtr),GetPortBitMapForCopyBits(selectGWorld),
			&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
		DelSearch(csUPP);
		DisposeColorSearchUPP(csUPP);
		
		selectRgn=NewRgn();
		err=BitMapToRegion(selectRgn,GetPortBitMapForCopyBits(selectGWorld));
		if (err!=noErr)
		{
			SetEmptyRgn(selectRgn);
			SysBeep(0);
		}
		UnlockPixels(GetGWorldPixMap(selectGWorld));
		SetGWorld(cPort,cDevice);
		DisposeGWorld(selectGWorld);
	}
	
	if (EmptyRgn(selectRgn)) /* 選択領域が空であればなにもしない */
	{
		DisposeRgn(selectRgn);
		DispOffPort(theWindow);
		SetUndoMode(umCannot);
		
		UpdateClipMenu();
		UpdateSelectionMenu();
		UpdateTransparentMenu();
	}
	else
		DoSelectMain(theWindow,selectRgn);
}

/* 色領域選択の補助ルーチン（指定の色を黒くする） */
pascal Boolean SearchColorAreaProc(RGBColor *color,long *pos)
{
	if (EqualColor(color,&gSearchColor))
		*pos=1;
	else
		*pos=0;
	return true;
}

/* マスク変更（透明、不透明） */
void ChangeMask(WindowPtr theWindow,short mode)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	RgnHandle	maskRgn;
	OSErr		err;
	GWorldPtr	maskGWorld;
	Rect		tempRect;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	GetGWorld(&cPort,&cDevice);
	GoOffPort(theWindow);
	
	GetPortBounds(eWinRec->currentMask,&tempRect);
	switch (mode)
	{
		case srcCopy:
			/* 不透明 */
			maskRgn=NewRgn();
			CopyRgn(eWinRec->selectionPos,maskRgn);
			if (eWinRec->isSelected)
				OffsetRgn(maskRgn,-eWinRec->selectionOffset.h,-eWinRec->selectionOffset.v);
			
			MySetGWorld(currentMask);
			MyLockPixels(currentMask);
			EraseRect(&tempRect);
			PaintRgn(maskRgn);
			MyUnlockPixels(currentMask);
			DisposeRgn(maskRgn);
			SetGWorld(cPort,cDevice);
			break;
		
		case transparent:
			/* 透明 */
			err=CreateMask2(eWinRec->editDataPtr,&maskGWorld);
			if (err==noErr)
			{
				MySetGWorld(currentMask);
				MyLockPixels(currentMask);
				CopyBits(GetPortBitMapForCopyBits(maskGWorld),
					GetPortBitMapForCopyBits(eWinRec->currentMask),
					&tempRect,&tempRect,srcCopy,nil);
				MyUnlockPixels(currentMask);
				DisposeGWorld(maskGWorld);
				SetGWorld(cPort,cDevice);
			}
			else
				SysBeep(0);
			break;
	}
	
	CopyRgn(eWinRec->selectionPos,eWinRec->updateRgn);
	
	DispOffPort(theWindow);
	UpdateTransparentMenu();
	SetUndoMode(umMaskChange);
	if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
		UpdatePaletteCheck();
}

/* 選択範囲をアイコンに変換 */
/* rectMaskは、プレビューアイコン用に四角いマスクを作成するかどうか */
Handle GetSelectionIconHandle(WindowPtr theWindow,short size,short depth,Boolean rectMask)
{
	Handle	resHandle;
	GWorldPtr	iconDataPtr;
	PixMapHandle	iconDataPix;
	Ptr		pixPtr;
	Ptr		iconPtr;
	Rect	iconRect;
	Rect	realIconRect;
	OSErr	err;
	short	i;
	MyBitMapRec	maskBM;
	long	s;
	long	iconRowBytes=size*depth/8,rowBytes;
	Boolean	isMaskAvailable;
	GWorldPtr	tempGWorld;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	Rect	selectedRect=eWinRec->selectedRect;
	
	/* アイコン用の描画領域を作成 */
	SetRect(&iconRect,0,0,size,size);
	if (eWinRec->isSelected)
		isMaskAvailable=false;
	else
		isMaskAvailable=(eWinRec->backgroundGWorld != nil);
	
	GetGWorld(&cPort,&cDevice);
	
	if (isMaskAvailable)
	{
		SetGWorld(eWinRec->selectedDataPtr,0);
		LockPixels(GetGWorldPixMap(eWinRec->selectedDataPtr));
		EraseRect(&eWinRec->iconSize);
		UnlockPixels(GetGWorldPixMap(eWinRec->selectedDataPtr));
		CopyBits(GetPortBitMapForCopyBits(eWinRec->backgroundGWorld),
			GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
			&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
		
		if (eWinRec->foreTransparency != kForeTrans0)
		{
			short	foreMode=SetForeBlend(eWinRec);
			
			SafeCopyDeepMask(GetPortBitMapForCopyBits(eWinRec->editDataPtr),
				GetPortBitMapForCopyBits(eWinRec->currentMask),
				GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
				&eWinRec->iconSize,&eWinRec->iconSize,&eWinRec->iconSize,foreMode,nil);
		}
		SetGWorld(cPort,cDevice);
		
		tempGWorld=eWinRec->selectedDataPtr;
	}
	else
		tempGWorld=eWinRec->editDataPtr;
	
	realIconRect=selectedRect;
	ReduceRect(&realIconRect,size);
	
	err=NewGWorld(&iconDataPtr,depth,&iconRect,0,0,useTempMem);
	if (err<0)
	{
		ErrorAlertFromResource(PAINTERR_RESID,PAINTERR1);
		return (Handle)0;
	}
	iconDataPix=GetGWorldPixMap(iconDataPtr);
	
	/* アイコンのデータを収めるためのハンドルを作成 */
	resHandle=NewHandle(size*size/8*(depth==1?2:depth));
	if (resHandle==0)
	{
		DisposeGWorld(iconDataPtr);
		ErrorAlertFromResource(PAINTERR_RESID,PAINTERR1);
		return (Handle)0;
	}
	
	/* アイコンデータ用の描画領域にコピー */
	SetGWorld(iconDataPtr,0);
	
	LockPixels(iconDataPix);
	EraseRect(&iconRect);
	ForeColor(blackColor);
	BackColor(whiteColor);
	
	CopyBits(GetPortBitMapForCopyBits(tempGWorld),
		GetPortBitMapForCopyBits(iconDataPtr),
		&selectedRect,&realIconRect,(rectMask ? ditherCopy : srcCopy),0); /* リージョンについては保留 */
	
	if (rectMask)
	{
		IconSuiteRef	theIconSuite;
		
		FrameRect(&realIconRect);
		
		err=GetIconSuite(&theIconSuite,135,kSelectorAllAvailableData);
		err=PlotIconSuite(&iconRect,kAlignNone,kTransformNone,theIconSuite);
		err=DisposeIconSuite(theIconSuite,true);
	}
	
	/* 描画領域からアイコンのデータへコピー */
	HLock(resHandle);
	rowBytes=MyGetPixRowBytes(iconDataPix) & 0x3fff;
	iconPtr=*resHandle;
	pixPtr=MyGetPixBaseAddr(iconDataPix);
	
	for (i=0; i<size; i++)
	{
		BlockMoveData(pixPtr,iconPtr,iconRowBytes);
		iconPtr+=iconRowBytes;
		pixPtr+=rowBytes;
	}
	
	UnlockPixels(iconDataPix);
	
	SetGWorld(cPort,cDevice);
	
	DisposeGWorld(iconDataPtr);
	
	if (depth==kICNDepth) /* 白黒ならばマスクパターンを作成 */
	{
		if (rectMask)
		{
			Rect	logoRect;
			
			err=NewGWorld(&iconDataPtr,1,&iconRect,0,0,useTempMem);
			if (err!=0)
			{
				DisposeHandle(resHandle);
				return nil;
			}
			iconDataPix=GetGWorldPixMap(iconDataPtr);
			
			SetGWorld(iconDataPtr,0);
			
			LockPixels(iconDataPix);
			EraseRect(&iconRect);
			ForeColor(blackColor);
			
			PaintRect(&realIconRect);
			
			if (size == 32)
				SetRect(&logoRect,19,21,32,32);
			else
				SetRect(&logoRect,8,9,32,32);
			PaintRect(&logoRect);
			//UnlockPixels(iconDataPix);
			SetGWorld(cPort,cDevice);
			
			rowBytes=(**iconDataPix).rowBytes & 0x7fff;
			pixPtr=(**iconDataPix).baseAddr;
		}
		else
		{
			/* マスクパターン用の描画領域を作成 */
			err=NewGWorld(&iconDataPtr,8,&iconRect,0,0,useTempMem);
			if (err!=0)
			{
				DisposeHandle(resHandle);
				return nil;
			}
			iconDataPix=GetGWorldPixMap(iconDataPtr);
			
			SetGWorld(iconDataPtr,0);
			
			LockPixels(iconDataPix);
			EraseRect(&iconRect);
			ForeColor(blackColor);
			BackColor(whiteColor);
			
			CopyBits(GetPortBitMapForCopyBits(tempGWorld),
				GetPortBitMapForCopyBits(iconDataPtr),
				&selectedRect,&realIconRect,srcCopy,NULL); /* リージョンについては保留 */
			
			UnlockPixels(iconDataPix);
			
			SetGWorld(cPort,cDevice);
			
			/* マスクパターンを作成 */
			err=CreateMask(iconDataPtr,&iconRect,&iconRect,&maskBM);
			if (err!=noErr)
			{
				DisposeHandle(resHandle);
				DisposeGWorld(iconDataPtr);
				return nil;
			}
			
			rowBytes=maskBM.bmp.rowBytes & 0x7fff;
			pixPtr=maskBM.bmp.baseAddr;
			
			/* 背景がないときは、ペイントウィンドウのマスクを考慮する */
			while (eWinRec->backgroundGWorld == nil)
			{
				GWorldPtr		maskDataPtr;
				PixMapHandle	maskDataPix;
				Ptr				src,dst,temp1,temp2;
				short			srcRowBytes,dstRowBytes,j;
				
				err=NewGWorld(&maskDataPtr,1,&iconRect,0,0,useTempMem);
				if (err!=noErr) break; /* エラーの時は、なかったことにする */
				maskDataPix=GetGWorldPixMap(maskDataPtr);
				
				SetGWorld(maskDataPtr,0);
				
				LockPixels(maskDataPix);
				EraseRect(&iconRect);
				ForeColor(blackColor);
				BackColor(whiteColor);
				
				CopyBits(GetPortBitMapForCopyBits(eWinRec->currentMask),
					GetPortBitMapForCopyBits(maskDataPtr),
					&selectedRect,&realIconRect,srcCopy,NULL);
				
				src = (**maskDataPix).baseAddr;
				dst = pixPtr;
				srcRowBytes = (**maskDataPix).rowBytes & 0x7fff;
				dstRowBytes = rowBytes;
				
				for (i=0; i<size; i++)
				{
					temp1 = src;
					temp2 = dst;
					
					for (j=0; j<size/8; j++)
						*temp2++ &= *temp1++;
					
					src += srcRowBytes;
					dst += dstRowBytes;
				}
				
				UnlockPixels(maskDataPix);
				SetGWorld(cPort,cDevice);
				
				DisposeGWorld(maskDataPtr);
				
				break;
			}
		}
		
		for (i=0; i<size; i++)
		{
			BlockMoveData(pixPtr,iconPtr,size/8);
			iconPtr+=size/8;
			pixPtr+=rowBytes;
		}
		
		if (rectMask)
		{
			UnlockPixels(iconDataPix);
		}
		else
		{
			DisposeBitMap(&maskBM);
		}
		s=GetHandleSize(resHandle);
		
		DisposeGWorld(iconDataPtr);
	}
	
	HUnlock(resHandle);
	return resHandle;
}

/* 高さheight、幅widthのRectをsize*sizeの範囲に収まるように縮小する */
/* resultに結果を返す */
void ReduceRect(Rect *result,short size)
{
	short	destH,destW;
	short	height,width;
	
	height=result->bottom-result->top;
	width=result->right-result->left;
	
	if (height<=size && width<=size)
	{
		destH=height;
		destW=width;
	}
	else if (height>width)
	{
		destH=size;
		destW=size*width/height;
	}
	else
	{
		destH=size*height/width;
		destW=size;
	}
	
	SetRect(result,(size-destW)>>1,(size-destH)>>1,(size+destW+1)>>1,(size+destH+1)>>1);
}

/* 選択マスクを作成 */
OSErr CreateMask(GWorldPtr gWorld,Rect *srcRect,Rect *dstRect,MyBitMapRec *bmpRec)
{
	ColorSearchUPP	upp;
	OSErr		err;
	GWorldPtr	maskPtr;
	PixMapHandle	maskPix,pmh;
	Rect		gWorldRect;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	GetPortBounds(gWorld,&gWorldRect);
	pmh=GetGWorldPixMap(gWorld);
	
	#if 1
	/* マスク用のGWorldを作成 */
	err=NewGWorld(&maskPtr,1,dstRect,0,0,useTempMem);
	if (err!=noErr) return err;
	
	/* マスク用のBitMap作成 */
	err=NewBitMap(bmpRec,dstRect);
	if (err!=noErr)
	{
		DisposeGWorld(maskPtr);
		return err;
	}
	
	maskPix=GetGWorldPixMap(maskPtr);
	
	GetGWorld(&cPort,&cDevice);
	SetGWorld(maskPtr,0);
	LockPixels(maskPix);
	EraseRect(dstRect);
	
	upp=NewColorSearchUPP(MySearchProc);
	AddSearch(upp);
	
	(**(**pmh).pmTable).ctSeed=GetCTSeed();
	
	CopyBits(GetPortBitMapForCopyBits(gWorld),GetPortBitMapForCopyBits(maskPtr),
		srcRect,dstRect,srcCopy,0);
	DelSearch(upp);
	DisposeColorSearchUPP(upp);
	
	CalcCMask(GetPortBitMapForCopyBits(maskPtr),&bmpRec->bmp,dstRect,dstRect,
		&rgbBlackColor,0,0);
	#else
	err=NewGWorld(&maskPtr,8,dstRect,0,0,useTempMem);
	if (err!=noErr) return err;
	
	err=NewBitMap(bmpRec,dstRect);
	if (err!=noErr)
	{
		DisposeGWorld(maskPtr);
		return err;
	}
	
	maskPix=GetGWorldPixMap(maskPtr);
	
	GetGWorld(&cPort,&cDevice);
	SetGWorld(maskPtr,0);
	LockPixels(maskPix);
	EraseRect(dstRect);
	
	CopyBits(GetPortBitMapForCopyBits(gWorld),GetPortBitMapForCopyBits(maskPtr),
		srcRect,dstRect,srcCopy,nil);
	
	{
		CTabHandle	ctab=(*maskPix)->pmTable;
		ColorSpec	*ctTable=(*ctab)->ctTable;
		short		i;
		
		for (i=1; i<255; i++)
			ctTable[i].rgb=rgbBlackColor;
		CTabChanged(ctab);
	}
	
	CalcCMask(GetPortBitMapForCopyBits(maskPtr),&bmpRec->bmp,dstRect,dstRect,
		&rgbBlackColor,nil,0);
	#endif
	
	SetGWorld(cPort,cDevice);
	DisposeGWorld(maskPtr);
	
	return err;
}

/* レイヤーマスクを作成 */
OSErr CreateMask2(GWorldPtr gWorld,GWorldPtr *maskGWorld)
{
	ColorSearchUPP	upp;
	OSErr			err;
	PixMapHandle	maskPix;
	Rect			r;
	GWorldPtr		cPort;
	GDHandle		cDevice;
	
	GetPortBounds(gWorld,&r);
	GetGWorld(&cPort,&cDevice);
	
	/* マスク用GWorldを作成 */
	err=NewGWorld(maskGWorld,1,&r,0,0,useTempMem);
	if (err!=noErr) return err;
	
	maskPix=GetGWorldPixMap(*maskGWorld);
	SetGWorld(*maskGWorld,0);
	LockPixels(maskPix);
	EraseRect(&r);
	UnlockPixels(maskPix);
	
	upp=NewColorSearchUPP(MySearchProc);
	AddSearch(upp);
	
	CopyBits(GetPortBitMapForCopyBits(gWorld),
		GetPortBitMapForCopyBits(*maskGWorld),
		&r,&r,srcCopy,nil);
	DelSearch(upp);
	DisposeColorSearchUPP(upp);
	
	SetGWorld(cPort,cDevice);
	
	return noErr;
}

/* マスク作成の補助ルーチン（白以外の色を黒くする） */
pascal Boolean MySearchProc(RGBColor *color,long *pos)
{
	#pragma unused(pos)
	
	if (color->red!=0xffff || color->blue!=0xffff || color->green!=0xffff)
		*pos=1;
	else
		*pos=0;
	return true;
}

/* 選択領域のリージョンの更新 */
void UpdatePSRgn(PaintWinRec *eWinRec)
{
	CopyRgn(eWinRec->selectionPos,eWinRec->prevSelectionPos);
	CopyRgn(eWinRec->eSelectedRgn,eWinRec->ePrevSelectedRgn);
}

/* 現在の画面データを保存し、オフポートをカレントにする */
OSErr GoOffPort(WindowPtr theWindow)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	Rect	portSize,tempRect;
	OSErr	result=noErr;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	GetPortBounds(eWinRec->editDataPtr,&portSize);
	ResetDot(theWindow);
	
	GetGWorld(&cPort,&cDevice);
	
	GetPortBounds(eWinRec->tempDataPtr,&tempRect);
	if (!EqualRect(&portSize,&tempRect)) /* サイズが異なる */
	{
		GWorldFlags	flags,newFlags;
		
		/* マスクも変形 */
		flags=UpdateGWorld(&eWinRec->pCurrentMask,1,&portSize,0,0,0);
		
		MySetGWorld(tempDataPtr);
		flags=UpdateGWorld(&eWinRec->tempDataPtr,eWinRec->iconDepth,&portSize,0,0,0);
		if ((flags & gwFlagErr)!=0)
		{
			result=(OSErr)flags;
		}
		else if (eWinRec->undoMode==umResize)
		{
			/* リサイズ後ならテンポラリなどのサイズを更新 */
			MySetGWorld(selectedDataPtr);
			flags=UpdateGWorld(&eWinRec->selectedDataPtr,eWinRec->iconDepth,&portSize,0,0,0);
			
			MyLockPixels(selectedDataPtr);
			EraseRect(&portSize);
			MyUnlockPixels(selectedDataPtr);
			
			/* マスクも更新 */
			MySetGWorld(selectionMask);
			flags=UpdateGWorld(&eWinRec->selectionMask,1,&portSize,0,0,0);
			
			MyLockPixels(selectionMask);
			EraseRect(&portSize);
			MyUnlockPixels(selectionMask);
			
			if (eWinRec->backgroundGWorld != nil)
			{
				/* 背景も変形 */
				SetGWorld(eWinRec->backgroundGWorld,0);
				newFlags=0 + (eWinRec->resizeImage ? stretchPix + (eWinRec->resizeDither ? ditherPix : 0) : 0);
				flags=UpdateGWorld(&eWinRec->backgroundGWorld,eWinRec->iconDepth,&portSize,0,0,newFlags);
			}
		}
	}
	
	/* 絵のバックアップ */
	SetGWorld(eWinRec->tempDataPtr,0);
	CopyBits(GetPortBitMapForCopyBits(eWinRec->editDataPtr),
		GetPortBitMapForCopyBits(eWinRec->tempDataPtr),
		&portSize,&portSize,srcCopy,nil);
	
	/* マスクのバックアップ */
	SetGWorld(eWinRec->pCurrentMask,0);
	CopyBits(GetPortBitMapForCopyBits(eWinRec->currentMask),
		GetPortBitMapForCopyBits(eWinRec->pCurrentMask),
		&portSize,&portSize,srcCopy,nil);
	
	SetGWorld(cPort,cDevice);
	
	return result;
}

/* 鉛筆／消しゴムツールでの描画 */
OSErr PaintOffPort(WindowPtr theWindow,short mode,Point pt,Boolean shiftDown)
{
	RGBColor	newColor;
	Point	mousePt,prevPt;
	Rect	redrawRect,eRedrawRect;
	Rect	penRect;
	Boolean	moveHorizontal=false;
	short	dh,dv;
	short	width,height;
	PenState	pnState;
	Boolean		transparency;
	Boolean		usePressure = (isTabletAvailable && ((mode == kPencilMode && gTabletPrefs.usePressure) ||
								(mode == kEraserMode && gTabletPrefs.useEraserPressure)));
	RGBColor	blendRatio2;
	RgnHandle	tempRgn=NewRgn();
	Point	offsetPt={0,0};
	
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	Boolean	isBackMode=(eWinRec->backgroundGWorld==nil && !eWinRec->isBackTransparent);
	short	ratio=eWinRec->ratio;
	
	OSErr		err;
	GWorldPtr	tempGWorld;
	PixMapHandle	tempPix;
	Boolean		selectClip=eWinRec->isSelected;
	RgnHandle	eRgn=nil,clipRgn=NewRgn();
	Rect		imageRect,tempRect;
	
	RgnHandle	maskRgn=NewRgn(); /* マスク処理用 */
	GWorldPtr	maskGWorld;
	short		foreMode=srcCopy;
	GWorldPtr	foreTransGWorld;
	
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	GetGWorld(&cPort,&cDevice);
	
	GetWindowPortBounds(theWindow,&imageRect);
	
	switch (mode)
	{
		case kPencilMode:
			newColor=gCurrentColor.rgb;
			break;
		
		case kEraserMode:
			if (isBackMode)
				newColor=gBackColor.rgb;
			else
			{
				mode=kEraseMaskMode;
				newColor=rgbWhiteColor;
			}
			break;
		
		case kEraseMaskMode:
			newColor=rgbWhiteColor;
			break;
	}
	
	err=NewGWorld(&maskGWorld,1,&eWinRec->iconSize,0,0,useTempMem);
	if (err!=noErr)
	{
		SetGWorld(cPort,cDevice);
		DisposeRgn(tempRgn);
		DisposeRgn(maskRgn);
		DisposeRgn(clipRgn);
		SysBeep(0);
		SetUndoMode(umCannot);
		return err;
	}
	LockPixels(GetGWorldPixMap(maskGWorld));
	
	if (eWinRec->backgroundGWorld != nil)
	{
		switch (eWinRec->foreTransparency)
		{
			case kForeTrans100:
				foreTransGWorld=eWinRec->dispTempPtr;
			case kForeTrans0:
				break;
			
			default:
				err=NewGWorld(&foreTransGWorld,eWinRec->iconDepth,&eWinRec->iconSize,0,0,useTempMem);
				if (err!=noErr)
				{
					UnlockPixels(GetGWorldPixMap(maskGWorld));
					DisposeGWorld(maskGWorld);
					
					SetGWorld(cPort,cDevice);
					DisposeRgn(tempRgn);
					DisposeRgn(maskRgn);
					SysBeep(0);
					SetUndoMode(umCannot);
				}
				LockPixels(GetGWorldPixMap(foreTransGWorld));
		}
	}
	else
		foreTransGWorld=eWinRec->dispTempPtr;
	
	SetGWorld(cPort,cDevice);
	SetPortWindowPort(theWindow);
	/* 実際に表示可能な領域 */
	imageRect.right-=kScrollBarWidth;
	imageRect.bottom-=kScrollBarHeight;
	ClipRect(&imageRect);
	GetClip(clipRgn);
	
	SetGWorld(eWinRec->editDataPtr,0);
	
	mousePt=pt;
	
	/* ペンサイズを計算 */
	/* しかし、わざわざこんなことするくらいなら引き数でわたしてもいいような気がするが… */
	GetPenState(&pnState);
	width=(pnState.pnSize).h;
	height=(pnState.pnSize).v;
	SetPt(&offsetPt,-width/2,-height/2);
	AddPt(offsetPt,&mousePt);
	
	/* ペンの転送モード */
	transparency=(pnState.pnMode == blend);
	
	/* マスクの消去 */
	SetGWorld(maskGWorld,0);
	EraseRect(&eWinRec->iconSize);
	PenSize(width,height);
	
	/* 選択範囲の処理 */
	if (selectClip)
	{
		err=NewGWorld(&tempGWorld,eWinRec->iconDepth,&eWinRec->iconSize,0,0,useTempMem);
		if (err!=noErr)
		{
			if (eWinRec->backgroundGWorld != nil && (eWinRec->foreTransparency != kForeTrans100 &&
													eWinRec->foreTransparency != kForeTrans0))
			{
				UnlockPixels(GetGWorldPixMap(foreTransGWorld));
				DisposeGWorld(foreTransGWorld);
			}
			UnlockPixels(GetGWorldPixMap(maskGWorld));
			DisposeGWorld(maskGWorld);
			
			SetGWorld(cPort,cDevice);
			DisposeRgn(tempRgn);
			DisposeRgn(maskRgn);
			SysBeep(0);
			SetUndoMode(umCannot);
			return err;
		}
		
		eRgn=eWinRec->eSelectedRgn;
		SectRgn(clipRgn,eRgn,clipRgn);
	}
	else
	{
		tempGWorld=eWinRec->dispTempPtr;
	}
	SetGWorld(tempGWorld,0);
	tempPix=GetGWorldPixMap(tempGWorld);
	LockPixels(tempPix);
	
	SetPenState(&pnState);
	
	if (selectClip)
	{
		EraseRect(&eWinRec->iconSize);
		GetRegionBounds(eWinRec->selectionPos,&tempRect);
		CopyBits(GetPortBitMapForCopyBits(eWinRec->editDataPtr),
			GetPortBitMapForCopyBits(tempGWorld),
			&eWinRec->selectedRect,&tempRect,srcCopy,nil);
	}
	
	/* 色を設定 */
	RGBForeColor(&newColor);
	
	/* 筆圧処理 */
	if (transparency) {
		if (usePressure && IsTabletInUse())
		{
			/* タブレット使用時 */
			Pressure2Ratio(&blendRatio2);
			OpColor(&blendRatio2);
		}
		else {
			OpColor(&gBlendRatio);
        }
    }
	
	prevPt=mousePt;
	SetGWorld(cPort,cDevice);
	if (shiftDown) /* シフトが押されているときは縦または横にだけひく or 最後に描画された場所から直線を引く */
	{
		UInt32	t=TickCount()+60;
		
		/* マウスが動くまで待って、動いた方向によって縦線か横線かを決定 */
		while (EqualPt(mousePt,prevPt) && StillDown() && TickCount()<t)
		{
			GetMouse(&mousePt);
			ToRealPos(&mousePt,ratio);
			AddPt(offsetPt,&mousePt);
		}
		if (eWinRec->undoMode == umPaint && EqualPt(mousePt,prevPt))
		{
			SetGWorld(tempGWorld,0);
			RGBForeColor(&newColor);
			MoveTo(eWinRec->lastPaintPos.h,eWinRec->lastPaintPos.v);
			LineTo(mousePt.h,mousePt.v);
			ForeColor(blackColor);
			
			SetGWorld(maskGWorld,0);
			MoveTo(eWinRec->lastPaintPos.h,eWinRec->lastPaintPos.v);
			LineTo(mousePt.h,mousePt.v);
			
			SetRect(&redrawRect,mousePt.h,mousePt.v,eWinRec->lastPaintPos.h+width,eWinRec->lastPaintPos.v+height);
			SortRect(&redrawRect,false);
			goto dispStart;
		}
		dh=mousePt.h-prevPt.h;
		dv=mousePt.v-prevPt.v;
		dh=(dh>0?dh:-dh); /* 絶対値 */
		dv=(dv>0?dv:-dv);
		if (dh<dv)
			moveHorizontal=false;
		else
			moveHorizontal=true;
	}
	
	/* とりあえず最初にクリックされたところに点を打っておく */
	/* マスクに点を打つ */
	SetGWorld(maskGWorld,0);
	MoveTo(mousePt.h,mousePt.v);
	Line(0,0);
	
	SetGWorld(tempGWorld,0);
	RGBForeColor(&newColor);
	MoveTo(mousePt.h,mousePt.v);
	Line(0,0);
	ForeColor(blackColor);
	
	SetRect(&redrawRect,mousePt.h,mousePt.v,mousePt.h+width,mousePt.v+height);
dispStart:
	SectRect(&redrawRect,&eWinRec->iconSize,&redrawRect);
	if (selectClip)
	{
		SetGWorld(eWinRec->dispTempPtr,0);
		CopyBits(GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
			GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
			&redrawRect,&redrawRect,srcCopy,0);
		
		/* 選択領域と背景の重ね合せ */
		if (eWinRec->selectionBlend==blend) /* ブレンド */
			OpColor(&rgbGrayColor);
		CopyBits(GetPortBitMapForCopyBits(tempGWorld),
			GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
			&redrawRect,&redrawRect,eWinRec->selectionBlend,eWinRec->selectionPos);
	}
	prevPt=mousePt;
	
	/* 画面への描画 */
	/* プレビュー */
	ForeColor(blackColor);
	SetGWorld(cPort,cDevice);
	SetPortWindowPort(gPreviewWindow);
	if (mode==kEraseMaskMode)
	{
		if (eWinRec->backgroundGWorld != nil)
		{
			if (selectClip)
			{
				SetGWorld(eWinRec->dispTempPtr,0);
				CopyBits(GetPortBitMapForCopyBits(eWinRec->backgroundGWorld),
					GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
					&redrawRect,&redrawRect,srcCopy,eWinRec->selectionPos);
				SafeCopyDeepMask(GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
					GetPortBitMapForCopyBits(eWinRec->selectionMask),
					GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
					&redrawRect,&redrawRect,&redrawRect,srcCopy,eWinRec->selectionPos);
				
				SetGWorld(cPort,cDevice);
				SetPortWindowPort(gPreviewWindow);
				CopyBits(GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
					GetPortBitMapForCopyBits(GetWindowPort(gPreviewWindow)),
					&redrawRect,&redrawRect,srcCopy,eWinRec->selectionPos);
			}
			else
				CopyBits(GetPortBitMapForCopyBits(eWinRec->backgroundGWorld),
					GetPortBitMapForCopyBits(GetWindowPort(gPreviewWindow)),
					&redrawRect,&redrawRect,srcCopy,eWinRec->selectionPos);
		}
		else
		{
			CopyBits(GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
				GetPortBitMapForCopyBits(GetWindowPort(gPreviewWindow)),
				&redrawRect,&redrawRect,srcCopy,eWinRec->selectionPos);
		}
	}
	else
	{
		if (eWinRec->backgroundGWorld != nil)
		{
			switch (eWinRec->foreTransparency)
			{
				case kForeTrans100:
					break;
				
				case kForeTrans0:
					goto dispEnd2;
					break;
				
				default:
					SetGWorld(foreTransGWorld,0);
					CopyBits(GetPortBitMapForCopyBits(eWinRec->backgroundGWorld),
						GetPortBitMapForCopyBits(foreTransGWorld),
						&redrawRect,&redrawRect,srcCopy,eWinRec->selectionPos);
					foreMode=SetForeBlend(eWinRec);
					CopyBits(GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
						GetPortBitMapForCopyBits(foreTransGWorld),
						&redrawRect,&redrawRect,foreMode,eWinRec->selectionPos);
					
					SetGWorld(cPort,cDevice);
					SetPortWindowPort(gPreviewWindow);
			}
		}
		CopyBits(GetPortBitMapForCopyBits(foreTransGWorld),
			GetPortBitMapForCopyBits(GetWindowPort(gPreviewWindow)),
			&redrawRect,&redrawRect,srcCopy,eWinRec->selectionPos);
	}
	
	/* ペイント */
	eRedrawRect=redrawRect;
	MapRect(&eRedrawRect,&eWinRec->iconSize,&eWinRec->iconEditSize);
	SetPortWindowPort(theWindow);
//	SetClip(eRgn);
	SetClip(clipRgn);
	if (mode==kEraseMaskMode)
	{
		if (eWinRec->backgroundGWorld != nil)
			if (selectClip)
				CopyBits(GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
					GetPortBitMapForCopyBits(GetWindowPort(theWindow)),
					&redrawRect,&eRedrawRect,srcCopy,eRgn);
			else
				CopyBits(GetPortBitMapForCopyBits(eWinRec->backgroundGWorld),
					GetPortBitMapForCopyBits(GetWindowPort(theWindow)),
					&redrawRect,&eRedrawRect,srcCopy,eRgn);
		else
			CopyBits(GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
				GetPortBitMapForCopyBits(GetWindowPort(theWindow)),
				&redrawRect,&eRedrawRect,srcCopy,eRgn);
	}
	else
	{
		if (eWinRec->backgroundGWorld != nil)
		{
			switch (eWinRec->foreTransparency)
			{
				case kForeTrans0:
					goto dispEnd2;
					break;
				
				case kForeTrans100:
					break;
			}
		}
		CopyBits(GetPortBitMapForCopyBits(foreTransGWorld),
			GetPortBitMapForCopyBits(GetWindowPort(theWindow)),
			&redrawRect,&eRedrawRect,srcCopy,eRgn);
	}
dispEnd2:
	DrawGrid(theWindow);
	
	SetGWorld(tempGWorld,0);
	RGBForeColor(&newColor);

	while(StillDown())
	{
		SetGWorld(cPort,cDevice);
		GetMouse(&mousePt); /* 普通のグラフィックポートをカレントにしていないとマウスの位置が正確に求められない */
		SetGWorld(tempGWorld,0);
		ToRealPos(&mousePt,ratio);
		AddPt(offsetPt,&mousePt);
		
		if (shiftDown) { /* シフトが押されていれば、それなりの処理をする */
			if (moveHorizontal) {
				mousePt.v=prevPt.v;
			} else {
				mousePt.h=prevPt.h;
            }
        }
		
		if (EqualPt(mousePt,prevPt)) continue; /* 結果的に動いていなければ無視 */
		
		if (transparency && PtInRect(prevPt,&eWinRec->iconSize)) /* クリッピング */
		{
			/* 透明度が設定されているとき、重なった領域が濃くなってしまうのを防ぐ */
			/* 言葉で説明するのは難しい処理だな… */	
			SetRect(&penRect,prevPt.h,prevPt.v,prevPt.h+width,prevPt.v+height);
			RectRgn(tempRgn,&penRect);
			DiffRgn(eWinRec->selectionPos,tempRgn,tempRgn);
			SetClip(tempRgn);
			
			/* 筆圧処理 */
			if (usePressure && IsTabletInUse())
			{
				/* タブレット使用時 */
				Pressure2Ratio(&blendRatio2);
				OpColor(&blendRatio2);
			}
		}
		LineTo(mousePt.h,mousePt.v);
		
		/* マスクも描画 */
		SetGWorld(maskGWorld,0);
		MoveTo(prevPt.h,prevPt.v);
		LineTo(mousePt.h,mousePt.v);
		
		SetGWorld(tempGWorld,0);
		
		/* 画面に表示する領域を求める */
		Pt2Rect(mousePt,prevPt,&redrawRect);
		redrawRect.right+=width;
		redrawRect.bottom+=height;
		SectRect(&redrawRect,&eWinRec->iconSize,&redrawRect);
		
		/* 必要なら重ね合わせ処理をする */
		if (selectClip)
		{
			SetGWorld(eWinRec->dispTempPtr,0);
			CopyBits(GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
				GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
				&redrawRect,&redrawRect,srcCopy,0);
			
			if (eWinRec->selectionBlend==blend) /* ブレンド */
				OpColor(&rgbGrayColor);
			CopyBits(GetPortBitMapForCopyBits(tempGWorld),
				GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
				&redrawRect,&redrawRect,eWinRec->selectionBlend,eWinRec->selectionPos);
		}
		
		/* 画面に表示 */
		/* プレビューウィンドウの更新 */
		ForeColor(blackColor);
		SetGWorld(cPort,cDevice);
		SetPortWindowPort(gPreviewWindow);
		err=BitMapToRegion(maskRgn,GetPortBitMapForCopyBits(maskGWorld));
		SectRgn(maskRgn,eWinRec->selectionPos,maskRgn);
		if (mode==kEraseMaskMode)
		{
			if (eWinRec->backgroundGWorld != nil)
			{
				if (selectClip)
				{
					SetGWorld(eWinRec->dispTempPtr,0);
					CopyBits(GetPortBitMapForCopyBits(eWinRec->backgroundGWorld),
						GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
						&redrawRect,&redrawRect,srcCopy,eWinRec->selectionPos);
					SafeCopyDeepMask(GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
						GetPortBitMapForCopyBits(eWinRec->selectionMask),
						GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
						&redrawRect,&redrawRect,&redrawRect,srcCopy,maskRgn);
					
					SetGWorld(cPort,cDevice);
					SetPortWindowPort(gPreviewWindow);
					CopyBits(GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
						GetPortBitMapForCopyBits(GetWindowPort(gPreviewWindow)),
						&redrawRect,&redrawRect,srcCopy,maskRgn);
				}
				else
					CopyBits(GetPortBitMapForCopyBits(eWinRec->backgroundGWorld),
						GetPortBitMapForCopyBits(GetWindowPort(gPreviewWindow)),
						&redrawRect,&redrawRect,srcCopy,maskRgn);
			}
			else
				CopyBits(GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
					GetPortBitMapForCopyBits(GetWindowPort(gPreviewWindow)),
					&redrawRect,&redrawRect,srcCopy,maskRgn);
		}
		else
		{
			if (eWinRec->backgroundGWorld != nil) {
				switch (eWinRec->foreTransparency)
				{
					case kForeTrans0:
						goto dispEnd;
						break;
					
					case kForeTrans100:
						break;
					
					default:
						SetGWorld(foreTransGWorld,0);
						CopyBits(GetPortBitMapForCopyBits(eWinRec->backgroundGWorld),
							GetPortBitMapForCopyBits(foreTransGWorld),
							&redrawRect,&redrawRect,srcCopy,maskRgn);
						foreMode=SetForeBlend(eWinRec);
						CopyBits(GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
							GetPortBitMapForCopyBits(foreTransGWorld),
							&redrawRect,&redrawRect,foreMode,maskRgn);
						
						SetGWorld(cPort,cDevice);
						SetPortWindowPort(gPreviewWindow);
				}
			}
			
			CopyBits(GetPortBitMapForCopyBits(foreTransGWorld),
				GetPortBitMapForCopyBits(GetWindowPort(gPreviewWindow)),
				&redrawRect,&redrawRect,srcCopy,maskRgn);
		}
		
		eRedrawRect=redrawRect;
		MapRect(&eRedrawRect,&eWinRec->iconSize,&eWinRec->iconEditSize);
		MapRgn(maskRgn,&eWinRec->iconSize,&eWinRec->iconEditSize);
//		SectRgn(maskRgn,clipRgn,maskRgn);
		
		/* ペイントウィンドウの更新 */
		SetPortWindowPort(theWindow);
//		SetClip(maskRgn);
		if (mode==kEraseMaskMode)
		{
			if (eWinRec->backgroundGWorld != nil)
				if (selectClip)
					CopyBits(GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
						GetPortBitMapForCopyBits(GetWindowPort(theWindow)),
						&redrawRect,&eRedrawRect,srcCopy,maskRgn);
				else
					CopyBits(GetPortBitMapForCopyBits(eWinRec->backgroundGWorld),
						GetPortBitMapForCopyBits(GetWindowPort(theWindow)),
						&redrawRect,&eRedrawRect,srcCopy,maskRgn);
			else
				CopyBits(GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
					GetPortBitMapForCopyBits(GetWindowPort(theWindow)),
					&redrawRect,&eRedrawRect,srcCopy,maskRgn);
		}
		else
		{
			if (eWinRec->backgroundGWorld != nil)
			{
				switch (eWinRec->foreTransparency)
				{
					case kForeTrans0:
						goto dispEnd;
						break;
					
					case kForeTrans100:
						break;
				}
			}
			CopyBits(GetPortBitMapForCopyBits(foreTransGWorld),
				GetPortBitMapForCopyBits(GetWindowPort(theWindow)),
				&redrawRect,&eRedrawRect,srcCopy,maskRgn);
		}
dispEnd:
		DrawGrid(theWindow);
		
		SetGWorld(tempGWorld,0);
		
		RGBForeColor(&newColor);
		
		prevPt=mousePt;
	}
	
	ForeColor(blackColor);
	PenMode(srcCopy);
	
	ClipRect(&eWinRec->iconSize);
	SetGWorld(eWinRec->editDataPtr,0);
	if (selectClip)
	{
		/* 表示領域を求める */
		RectRgn(tempRgn,&eWinRec->iconSize);
		SectRgn(tempRgn,eWinRec->selectionPos,tempRgn);
		GetRegionBounds(eWinRec->selectionPos,&tempRect);
		MapRgn(tempRgn,&tempRect,&eWinRec->selectedRect);
		
		CopyBits(GetPortBitMapForCopyBits(tempGWorld),
			GetPortBitMapForCopyBits(eWinRec->editDataPtr),
			&tempRect,&eWinRec->selectedRect,srcCopy,
			tempRgn);
		
		UnlockPixels(tempPix);
		DisposeGWorld(tempGWorld);
	}
	else
	{
		CopyBits(GetPortBitMapForCopyBits(tempGWorld),
			GetPortBitMapForCopyBits(eWinRec->editDataPtr),
			&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
		
		UnlockPixels(tempPix);
	}
	
	DisposeRgn(tempRgn);
	
	/* GWorldを破棄 */
	if (eWinRec->backgroundGWorld != nil && (eWinRec->foreTransparency != kForeTrans0 && 
		eWinRec->foreTransparency != kForeTrans100))
	{
		UnlockPixels(GetGWorldPixMap(foreTransGWorld));
		DisposeGWorld(foreTransGWorld);
	}
	
	/* マスク処理 */
	err=BitMapToRegion(maskRgn,GetPortBitMapForCopyBits(maskGWorld));
	UnlockPixels(GetGWorldPixMap(maskGWorld));
	DisposeGWorld(maskGWorld);
	
	SectRgn(maskRgn,eWinRec->selectionPos,maskRgn); /* 選択範囲のみ */
	CopyRgn(maskRgn,eWinRec->updateRgn);
	
	if (eWinRec->isSelected)
	{
		GetRegionBounds(eWinRec->selectionPos,&tempRect);
		MapRgn(maskRgn,&tempRect,&eWinRec->selectedRect);
	}
	
	MySetGWorld(currentMask);
	MyLockPixels(currentMask);
	if (mode==kEraseMaskMode)
		EraseRgn(maskRgn);
	else
		PaintRgn(maskRgn);
	MyUnlockPixels(currentMask);
	DisposeRgn(maskRgn);
	
	SetGWorld(cPort,cDevice);
	SetPortWindowPort(theWindow);
	GetWindowPortBounds(theWindow,&tempRect);
	ClipRect(&tempRect);
	DisposeRgn(clipRgn);
	
	eWinRec->lastPaintPos = mousePt;
	
	DispOffPort(theWindow);
	
	return noErr;
}

/* オフポートの画像を表示 */
void DispOffPort(WindowPtr theWindow)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	RgnHandle	tempRgn;
	Rect		tempRect;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	GetGWorld(&cPort,&cDevice);
	
	SetGWorld(eWinRec->editDataPtr,0);
	ForeColor(blackColor);
	SetGWorld(eWinRec->dispTempPtr,0);
	
	if (eWinRec->isSelected) /* 選択されている時 */
	{
		CopyBits(GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
			GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
			&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
		
		if (eWinRec->selectionBlend==blend) /* ブレンド */
			OpColor(&rgbGrayColor);
		GetRegionBounds(eWinRec->selectionPos,&tempRect);
		SafeCopyDeepMask(GetPortBitMapForCopyBits(eWinRec->editDataPtr),
			GetPortBitMapForCopyBits(eWinRec->currentMask),
			GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
			&eWinRec->selectedRect,&eWinRec->selectedRect,&tempRect,
			eWinRec->selectionBlend,eWinRec->selectionPos);
	}
	else
	{
		CopyBits(GetPortBitMapForCopyBits(eWinRec->editDataPtr),
			GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
			&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
	}
	
	SetGWorld(cPort,cDevice);
	
	if (MyFrontNonFloatingWindow() == theWindow)
	{
		/* Preview */
		SetPortWindowPort(gPreviewWindow);
		MyInvalWindowRgn(gPreviewWindow,eWinRec->updateRgn);
	}
	
	SetPortWindowPort(theWindow);
	ForeColor(blackColor);
	tempRgn=NewRgn();
	CopyRgn(eWinRec->updateRgn,tempRgn);
	
	MapRgn(tempRgn,&eWinRec->iconSize,&eWinRec->iconEditSize);
	MyInvalWindowRgn(theWindow,tempRgn);
	
	DisposeRgn(tempRgn);

	if (gToolPrefs.showRuler)
		SetPt(&gPrevRulerPt,-1,-1);
}

/* 選択領域の枠を描画 */
void DispSelection(WindowPtr theWindow)
{
	DispSelectionMain(theWindow,true,true);
}

/* 領域表示メイン */
void DispSelectionMain(WindowPtr theWindow,Boolean dispPaint,Boolean dispPreview)
{
	PatHandle	gray;
	
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	Rect		editRect;
	
	gray=GetPattern(128);
	
	if (dispPreview && MyFrontNonFloatingWindow() == theWindow)
	{
		SetPortWindowPort(gPreviewWindow);
		PenMode(patXor);
		PenPat(*gray);
		FrameRgn(eWinRec->selectionPos);
		PenNormal();
	}
	if (dispPaint)
	{
		RgnHandle	tempRgn=NewRgn();
		
		SetPortWindowPort(theWindow);
		GetWindowPortBounds(theWindow,&editRect);
		editRect.right-=kScrollBarWidth;
		editRect.bottom-=kScrollBarHeight;
		GetClip(tempRgn);
		ClipRect(&editRect);
		PenMode(patXor);
		PenPat(*gray);
		FrameRgn(eWinRec->eSelectedRgn);
		PenNormal();
		SetClip(tempRgn);
		DisposeRgn(tempRgn);
	}
	
	ReleaseResource((Handle)gray);
}

/* 選択領域のドラッグ（返り値は、実際にドラッグされたかどうか） */
Boolean DragSelection(WindowPtr theWindow,Point pt,Point *resultPt,Boolean shiftDown)
{
	Point	mousePt;
	Point	prevMousePt;
	RgnHandle	dragRgn;
	RgnHandle	prevDragRgn;
	GrafPtr	port;
	KeyMap	theKeys;
	Boolean		horizFlag=true;
	Boolean		vertFlag=true;
	Point	offsetPt;
	
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	short	ratio=eWinRec->ratio;
	
	GWorldPtr	eDispTempPtr=(GWorldPtr)0L; /* 編集画面のグリッド用 */
	PixMapHandle	eDispTempPix=NULL;
	OSErr		err;
	
	Rect	imageRect,tempRect;
	Boolean	result=false;
	
	GWorldPtr	tempGWorld;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	#define		abs(x)	((x>0 ? x : -x))
	
	if (eWinRec->backgroundGWorld != nil)
		err=NewGWorld(&tempGWorld,eWinRec->iconDepth,&eWinRec->iconSize,0,0,useTempMem);
	else
		tempGWorld=eWinRec->dispTempPtr;
	
	GetPort(&port);
	GetGWorld(&cPort,&cDevice);
	SetPortWindowPort(theWindow);
	
	GetWindowPortBounds(theWindow,&imageRect);
	imageRect.right-=kScrollBarWidth;
	imageRect.bottom-=kScrollBarHeight;
	ClipRect(&imageRect);
	
	GetKeys(theKeys);
	if (BitTst(theKeys,61)) /* オプションキー */
		MySetCursor(kCursorHandOnPlus);
	else
		MySetCursor(kCursorHandOn);
	
	/* マウスが動くまで待つ */
	if (eWinRec->showSelection)
		DispSelection(theWindow);
	GetMouse(&prevMousePt);
	mousePt=prevMousePt;
	while (EqualPt(prevMousePt,mousePt) && StillDown())
	{
		GetMouse(&mousePt);
	}
	if (EqualPt(prevMousePt,mousePt))
	{
		/* 動かされていない */
		SetPt(resultPt,0,0);
		goto exit;
	}
	
	/* 左右、上下に移動できるか */
	if (shiftDown)
	{
		SubPt(mousePt,&prevMousePt);
		if (abs(prevMousePt.h) > abs(prevMousePt.v))
			vertFlag=false;
		else
			horizFlag=false;
	}
	
	prevMousePt=pt;
	
	prevDragRgn=NewRgn();
	
	dragRgn=eWinRec->selectionPos;
	CopyRgn(eWinRec->selectionPos,prevDragRgn);
	
	GetRegionBounds(eWinRec->selectionPos,&tempRect);
	SetPt(&offsetPt,tempRect.left,tempRect.top);
	SubPt(pt,&offsetPt);
	
	if (gToolPrefs.gridMode!=iGNone)
	{
		err=NewGWorld(&eDispTempPtr,eWinRec->iconDepth,&eWinRec->iconEditSize,0,0,useTempMem);
		eDispTempPix=GetGWorldPixMap(eDispTempPtr);
		LockPixels(eDispTempPix);
		
		SetGWorld(eDispTempPtr,0);
		EraseRect(&eWinRec->iconEditSize);
		SetGWorld(cPort,cDevice);
	}
	
	while (StillDown())
	{
		GetMouse(&mousePt);
		GetKeys(theKeys);
		ToRealPos(&mousePt,ratio);
		long temp;
#if TARGET_RT_LITTLE_ENDIAN
		temp = EndianS32_BtoN (theKeys[1].bigEndianValue);
#else
		temp = theKeys[1];
#endif	
		if ((temp & 0x00000001) != 0 && !shiftDown)
		{
			AddPt(offsetPt,&mousePt);
			FixToGrid2(&mousePt);
			SubPt(offsetPt,&mousePt);
		}
		
		if (!EqualPt(prevMousePt,mousePt))
		{
			SubPt(mousePt,&prevMousePt);
			OffsetRgn(dragRgn,-prevMousePt.h*horizFlag,-prevMousePt.v*vertFlag);
			
			/* 背景の再描画 */
			UnionRgn(prevDragRgn,dragRgn,prevDragRgn);
			SetGWorld(eWinRec->dispTempPtr,0);
			GetRegionBounds(prevDragRgn,&tempRect);
			CopyBits(GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
				GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
				&tempRect,&tempRect,srcCopy,prevDragRgn);
			
			/* 選択領域の再描画 */
			if (eWinRec->selectionBlend==blend)
				OpColor(&rgbGrayColor);
			GetRegionBounds(dragRgn,&tempRect);
			SafeCopyDeepMask(GetPortBitMapForCopyBits(eWinRec->editDataPtr),
				GetPortBitMapForCopyBits(eWinRec->currentMask),
				GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
				&eWinRec->selectedRect,&eWinRec->selectedRect,&tempRect,
				eWinRec->selectionBlend,nil);
			
			/* プレビュー画面の再描画 */
			if (eWinRec->backgroundGWorld != nil)
			{
				GWorldPtr	maskGWorld;
				
				err=NewGWorld(&maskGWorld,1,&eWinRec->iconSize,0,0,useTempMem);
				SetGWorld(maskGWorld,0);
				LockPixels(GetGWorldPixMap(maskGWorld));
				EraseRect(&eWinRec->iconSize);
				GetRegionBounds(eWinRec->selectionPos,&tempRect);
				CopyBits(GetPortBitMapForCopyBits(eWinRec->currentMask),
					GetPortBitMapForCopyBits(maskGWorld),
					&eWinRec->selectedRect,&tempRect,srcCopy,nil);
				CopyBits(GetPortBitMapForCopyBits(eWinRec->selectionMask),
					GetPortBitMapForCopyBits(maskGWorld),
					&eWinRec->iconSize,&eWinRec->iconSize,srcOr,nil);
				UnlockPixels(GetGWorldPixMap(maskGWorld));
				
				SetGWorld(tempGWorld,0);
				/* 背景表示 */
				if (eWinRec->dispBackground)
				{
					GetRegionBounds(prevDragRgn,&tempRect);
					CopyBits(GetPortBitMapForCopyBits(eWinRec->backgroundGWorld),
						GetPortBitMapForCopyBits(tempGWorld),
						&tempRect,&tempRect,srcCopy,prevDragRgn);
				}
				
				/* 前景表示 */
				if (eWinRec->foreTransparency != kForeTrans0)
				{
					SafeCopyDeepMask(GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
						GetPortBitMapForCopyBits(maskGWorld),
						GetPortBitMapForCopyBits(tempGWorld),
						&eWinRec->iconSize,&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
				}
				
				DisposeGWorld(maskGWorld);
			}
			
			SetGWorld(cPort,cDevice);
			SetPortWindowPort(gPreviewWindow);
			
			GetRegionBounds(prevDragRgn,&tempRect);
			CopyBits(GetPortBitMapForCopyBits(tempGWorld),
				GetPortBitMapForCopyBits(GetWindowPort(gPreviewWindow)),
				&tempRect,&tempRect,srcCopy,prevDragRgn);
			
			/* 編集画面の再描画 */
			MapRgn(prevDragRgn,&eWinRec->iconSize,&eWinRec->iconEditSize);
			
			if (eDispTempPtr==nil)
			{
				SetPortWindowPort(theWindow);
				SetClip(prevDragRgn);
				CopyBits(GetPortBitMapForCopyBits(tempGWorld),
					GetPortBitMapForCopyBits(GetWindowPort(theWindow)),
					&eWinRec->iconSize,&eWinRec->iconEditSize,srcCopy,prevDragRgn);
				DrawGrid(theWindow);
			}
			else
			{
				SetGWorld(eDispTempPtr,0);
				SetClip(prevDragRgn);
				CopyBits(GetPortBitMapForCopyBits(tempGWorld),
					GetPortBitMapForCopyBits(eDispTempPtr),
					&eWinRec->iconSize,&eWinRec->iconEditSize,srcCopy,prevDragRgn);
				
				DrawGridMain(eWinRec->ratio,&eWinRec->iconSize);
				
				SetGWorld(cPort,cDevice);
				SetPortWindowPort(theWindow);
				CopyBits(GetPortBitMapForCopyBits(eDispTempPtr),
					GetPortBitMapForCopyBits(GetWindowPort(theWindow)),
					&eWinRec->iconEditSize,&eWinRec->iconEditSize,srcCopy,prevDragRgn);
			}
			
			CopyRgn(dragRgn,prevDragRgn);
			
			prevMousePt=mousePt;
		}
	}
	
	SubPt(pt,&prevMousePt);
	if (!horizFlag) prevMousePt.h=0;
	if (!vertFlag) prevMousePt.v=0;
	*resultPt=prevMousePt;
	
	DisposeRgn(prevDragRgn);
	
	if (eDispTempPtr!=nil)
	{
		UnlockPixels(eDispTempPix);
		DisposeGWorld(eDispTempPtr);
	}
	
	OffsetRgn(eWinRec->selectionPos,-resultPt->h,-resultPt->v);
	
	result=true;
	
exit:
	if (eWinRec->backgroundGWorld != nil)
		DisposeGWorld(tempGWorld);
	
	SetGWorld(cPort,cDevice);
	SetPortWindowPort(theWindow);
	GetWindowPortBounds(theWindow,&tempRect);
	ClipRect(&tempRect);
	SetPort(port);
	
	return result;
}

#define	dResizeIcon		144
enum {
	diWidth=3,
	diHeight,
	diResizeImage,
	diResizeDither,
	diWidthUnit,
	diHeightUnit,
	diConstRatio,
	diResizeBackground,
};

/* 画像サイズの変更 */
void ChangeIconSize(WindowPtr theWindow)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	DialogPtr	dp;
	short	width=eWinRec->iconSize.right,height=eWinRec->iconSize.bottom;
	long	newWidth,newHeight;
	Str255	widthStr,heightStr;
	short	item;
	ResizeDataRec	newResizeData;
	short	tempUnit;
	short	a[2],b[2];
	short	background=0;
	ModalFilterUPP	mfUPP=NewModalFilterUPP(MyModalDialogFilter);
	
	/* バックグラウンドが存在する時は使用不可 */
	if (eWinRec->backgroundGWorld!=nil || eWinRec->foregroundGWorld!=nil)
	{
		SysBeep(0);
		UpdateEffectMenu();
		return;
	}
	
	DeactivateFloatersAndFirstDocumentWindow();
	dp=GetNewDialog(dResizeIcon,nil,kFirstWindowOfClass);
	
	/* 各アイテムの初期化 */
	SetDialogControlValue(dp,diResizeImage,gResizeData.resizeImage);
	if (!gResizeData.resizeImage)
		SetDialogControlHilite(dp,diResizeDither,255);
	SetDialogControlValue(dp,diResizeDither,gResizeData.dither);
	
	SetDialogControlValue(dp,diWidthUnit,gResizeData.unit[0]);
	SetDialogControlValue(dp,diHeightUnit,gResizeData.unit[1]);
	SetDialogControlValue(dp,diConstRatio,gResizeData.constRatio);
	
	SetDialogControlValue(dp,diResizeBackground,(eWinRec->isBackTransparent ? 
								2 : gPaintWinPrefs.background+1));
	SetDialogControlHilite(dp,diResizeBackground,255);
	
	newResizeData=gResizeData;
	
	a[0]=height;
	a[1]=100;
	b[0]=width;
	b[1]=100;
	
	NumToString(b[gResizeData.unit[0]-1],widthStr);
	NumToString(a[gResizeData.unit[1]-1],heightStr);
	
	SetDialogItemText2(dp,diWidth,widthStr);
	SetDialogItemText2(dp,diHeight,heightStr);
	
	SetDialogDefaultItem(dp,ok);
	SetDialogCancelItem(dp,cancel);
	
	SelectDialogItemText(dp,diWidth,0,widthStr[0]);
	ShowWindow(GetDialogWindow(dp));
	
	item=3;
	while (item!=ok && item!=cancel)
	{
		ModalDialog(mfUPP,&item);
		
		switch (item)
		{
			case diConstRatio:
				newResizeData.constRatio=!newResizeData.constRatio;
				SetDialogControlValue(dp,diConstRatio,newResizeData.constRatio);
			case diWidth:
				if (newResizeData.constRatio)
				{
					/* 縦横比一定なら */
					GetDialogItemText2(dp,diWidth,widthStr);
					StringToNum(widthStr,&newWidth);
					newHeight=newWidth*a[newResizeData.unit[1]-1]/b[newResizeData.unit[0]-1];
					NumToString(newHeight,heightStr);
					SetDialogItemText2(dp,diHeight,heightStr);
				}
				IsMarginExists(dp,width,height,&newResizeData);
				break;
				
			case diHeight:
				if (newResizeData.constRatio)
				{
					GetDialogItemText2(dp,diHeight,heightStr);
					StringToNum(heightStr,&newHeight);
					newWidth=newHeight*b[newResizeData.unit[0]-1]/a[newResizeData.unit[1]-1];
					NumToString(newWidth,widthStr);
					SetDialogItemText2(dp,diWidth,widthStr);
				}
				IsMarginExists(dp,width,height,&newResizeData);
				break;
			
			case diWidthUnit:
				tempUnit=newResizeData.unit[0];
				newResizeData.unit[0]=GetDialogControlValue(dp,diWidthUnit);
				GetDialogItemText2(dp,diWidth,widthStr);
				StringToNum(widthStr,&newWidth);
				newWidth=newWidth*b[newResizeData.unit[0]-1]/b[tempUnit-1];
				NumToString(newWidth,widthStr);
				SetDialogItemText2(dp,diWidth,widthStr);
				SelectDialogItemText(dp,diWidth,0,widthStr[0]);
				break;
			
			case diHeightUnit:
				tempUnit=newResizeData.unit[1];
				newResizeData.unit[1]=GetDialogControlValue(dp,diHeightUnit);
				GetDialogItemText2(dp,diHeight,heightStr);
				StringToNum(heightStr,&newHeight);
				newHeight=newHeight*a[newResizeData.unit[1]-1]/a[tempUnit-1];
				NumToString(newHeight,heightStr);
				SetDialogItemText2(dp,diHeight,heightStr);
				SelectDialogItemText(dp,diHeight,0,heightStr[0]);
				break;
			
			case diResizeImage:
				newResizeData.resizeImage=!newResizeData.resizeImage;
				SetDialogControlValue(dp,diResizeImage,newResizeData.resizeImage);
				SetDialogControlHilite(dp,diResizeDither,newResizeData.resizeImage ? 0 : 255);
				IsMarginExists(dp,width,height,&newResizeData);
				break;
			
			case diResizeDither:
				newResizeData.dither=!newResizeData.dither;
				SetDialogControlValue(dp,diResizeDither,newResizeData.dither);
				break;
		}
	}
	
	if (item==ok)
	{
		GetDialogItemText2(dp,diWidth,widthStr);
		StringToNum(widthStr,&newWidth);
		if (newResizeData.unit[0]==unitPercent) newWidth=newWidth*width/100;
		
		GetDialogItemText2(dp,diHeight,heightStr);
		StringToNum(heightStr,&newHeight);
		if (newResizeData.unit[1]==unitPercent) newHeight=newHeight*height/100;
		
		background=GetDialogControlValue(dp,diResizeBackground)-1;
		
		gResizeData=newResizeData;
	}
	
	DisposeDialog(dp);
	DisposeModalFilterUPP(mfUPP);
	ActivateFloatersAndFirstDocumentWindow();
	
	if (item==cancel) return;
	if (newWidth <= 0 || newWidth > 16383 || newHeight <= 0 || newHeight > 16383)
	{
		SysBeep(0);
		return;
	}
	
	if (width != newWidth || height != newHeight)
	{
		Rect	newSize;
		
		SetRect(&newSize,0,0,newWidth,newHeight);
		ChangeIconSizeMain(theWindow,&newSize,background);
		if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
			UpdatePaletteCheck();
	}
}

/* リサイズ後のイメージに余白ができるかどうかを判定する */
void IsMarginExists(DialogPtr dp,short width,short height,ResizeDataRec *resizePrefs)
{
	long	newHeight,newWidth;
	Str255	str;
	Boolean	result=false;
	
	/* イメージを変型するなら関係ない */
	if (!resizePrefs->resizeImage)
	{
		/* 横幅が大きくなっているか */
		GetDialogItemText2(dp,diWidth,str);
		StringToNum(str,&newWidth);
		switch (resizePrefs->unit[0])
		{
			case unitPixels:
				result=(newWidth > width);
				break;
			
			case unitPercent:
				result=(newWidth > 100);
				break;
		}
		
		/* そうでなければ縦が大きくなっているか */
		if (!result)
		{
			GetDialogItemText2(dp,diHeight,str);
			StringToNum(str,&newHeight);
			switch(resizePrefs->unit[1])
			{
				case unitPixels:
					result=(newHeight > height);
					break;
				
				case unitPercent:
					result=(newHeight > 100);
					break;
			}
		}
	}
	
	SetDialogControlHilite(dp,diResizeBackground,(result ? 0 : 255));
}

/* 実際のリサイズ */
void ChangeIconSizeMain(WindowPtr theWindow,Rect *newSize,short background)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	GWorldFlags	newFlags;
	GrafPtr		port;
	Rect		copySize;
	short		tMode=srcCopy;
	OSErr		err=noErr;
	
	long		reqMemSize=(long)newSize->right*(long)newSize->bottom - (long)eWinRec->iconSize.right*(long)eWinRec->iconSize.bottom;
	Handle		tempH[4]={nil,nil,nil,nil};
	short		i;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	/* 残りメモリチェック */
	if (reqMemSize>0)
	{
		for (i=0; i<4; i++)
		{
			tempH[i]=NewHandle(reqMemSize);
			if (tempH[i]==nil)
			{
				err=memFullErr;
				break;
			}
		}
		for (i=0; i<4; i++)
			DisposeHandle(tempH[i]);
	}
	
	if (err!=noErr)
	{
		ErrorAlertFromResource(PAINTERR_RESID,PAINTERR1);
		return;
	}
	
	/* 選択領域がある場合はまず固定 */
	if (eWinRec->isSelected)
		FixSelection(theWindow);
	GetGWorld(&cPort,&cDevice);
	GoOffPort(theWindow);
	
	/* GWorldのサイズを変更 */
	newFlags=UpdateGWorld(&eWinRec->editDataPtr,eWinRec->iconDepth,newSize,0,0,0);
	if ((newFlags & gwFlagErr)!=0)
	{
		Rect	oldRect;
		
		GetPortBounds(eWinRec->tempDataPtr,&oldRect);
		
		SetGWorld(eWinRec->editDataPtr,0);
		CopyBits(GetPortBitMapForCopyBits(eWinRec->tempDataPtr),
			GetPortBitMapForCopyBits(eWinRec->editDataPtr),
			&oldRect,&oldRect,srcCopy,nil);
		SetGWorld(cPort,cDevice);
		DispOffPort(theWindow);
		if (eWinRec->undoMode == umDeselect) DoUndo();
		SetUndoMode(umCannot);
		
		ErrorAlertFromResource(PAINTERR_RESID,PAINTERR1);
		return;
	}
	newFlags=UpdateGWorld(&eWinRec->currentMask,1,newSize,0,0,0);
	if ((newFlags & gwFlagErr)!=0)
	{
		Rect	oldRect;
		
		GetPortBounds(eWinRec->tempDataPtr,&oldRect);
		
		newFlags=UpdateGWorld(&eWinRec->editDataPtr,eWinRec->iconDepth,&oldRect,0,0,0);
		
		SetGWorld(eWinRec->editDataPtr,0);
		CopyBits(GetPortBitMapForCopyBits(eWinRec->tempDataPtr),
			GetPortBitMapForCopyBits(eWinRec->editDataPtr),
			&oldRect,&oldRect,srcCopy,nil);
		
		SetGWorld(eWinRec->currentMask,0);
		CopyBits(GetPortBitMapForCopyBits(eWinRec->pCurrentMask),
			GetPortBitMapForCopyBits(eWinRec->currentMask),
			&oldRect,&oldRect,srcCopy,nil);
		
		SetGWorld(cPort,cDevice);
		DispOffPort(theWindow);
		if (eWinRec->undoMode == umDeselect) DoUndo();
		SetUndoMode(umCannot);
		
		ErrorAlertFromResource(PAINTERR_RESID,PAINTERR1);
		return;
	}
	
	/* コピーするサイズなどを計算 */
	if (gResizeData.resizeImage)
	{
		copySize=*newSize;
		if (gResizeData.dither)
			tMode+=ditherCopy;
	}
	else
		copySize=eWinRec->iconSize;
	
	/* リサイズ、コピーする */
	SetGWorld(eWinRec->editDataPtr,0);
	LockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
	
	/* 余白を塗る */
	if (background == 2)
		RGBBackColor(&gBackColor.rgb);
	EraseRect(newSize);
	BackColor(whiteColor);
	CopyBits(GetPortBitMapForCopyBits(eWinRec->tempDataPtr),
		GetPortBitMapForCopyBits(eWinRec->editDataPtr),
		&eWinRec->iconSize,&copySize,tMode,nil);
	UnlockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
	
	SetGWorld(eWinRec->dispTempPtr,0);
	newFlags=UpdateGWorld(&eWinRec->dispTempPtr,eWinRec->iconDepth,newSize,0,0,0);
	
	LockPixels(GetGWorldPixMap(eWinRec->dispTempPtr));
	
	/* 余白を塗る */
	if (background == 2)
		RGBBackColor(&gBackColor.rgb);
	EraseRect(newSize);
	BackColor(whiteColor);
	CopyBits(GetPortBitMapForCopyBits(eWinRec->editDataPtr),
		GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
		newSize,newSize,srcCopy,nil);
	UnlockPixels(GetGWorldPixMap(eWinRec->dispTempPtr));
	
	SetGWorld(cPort,cDevice);
	
	/* 選択範囲更新 */
	RectRgn(eWinRec->selectionPos,newSize);
	SetEmptyRgn(eWinRec->eSelectedRgn);
	eWinRec->isSelected=false;
	eWinRec->selectedRect=eWinRec->iconSize;
	
	eWinRec->resizeImage=gResizeData.resizeImage;
	eWinRec->resizeDither=gResizeData.dither;
	
	/* マスク */
	if (gResizeData.resizeImage) /* イメージを変形 */
	{
		GWorldPtr	maskGWorld;
		
		MySetGWorld(currentMask);
		CopyBits(GetPortBitMapForCopyBits(eWinRec->pCurrentMask),
			GetPortBitMapForCopyBits(eWinRec->currentMask),
			&eWinRec->iconSize,&copySize,srcCopy,nil);
		err=CreateMask2(eWinRec->editDataPtr,&maskGWorld);
		if (err!=noErr)
			SysBeep(0);
		else
		{
			MySetGWorld(currentMask);
			CopyBits(GetPortBitMapForCopyBits(maskGWorld),
				GetPortBitMapForCopyBits(eWinRec->currentMask),
				&copySize,&copySize,srcOr,nil);
			
			DisposeGWorld(maskGWorld);
		}
		SetGWorld(cPort,cDevice);
	}
	else /* イメージを変型しない */
	{
		MySetGWorld(currentMask);
		MyLockPixels(currentMask);
		
		if (background == 1 || (background == 2 && gBackColor.isTransparent))
			EraseRect(newSize);
		else
			PaintRect(newSize);
		CopyBits(GetPortBitMapForCopyBits(eWinRec->pCurrentMask),
			GetPortBitMapForCopyBits(eWinRec->currentMask),
			&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
		
		MyUnlockPixels(currentMask);
		SetGWorld(cPort,cDevice);
	}
	
	GetPort(&port);
	SetPortWindowPort(theWindow);
	eWinRec->iconSize=*newSize;
	eWinRec->selectedRect=*newSize;
	UpdatePaintWindow(theWindow);
	
	SetPortWindowPort(gPreviewWindow);
	SizeWindow(gPreviewWindow,newSize->right,newSize->bottom,false);
	ClipRect(&eWinRec->iconSize);
	MyInvalWindowRect(gPreviewWindow,&eWinRec->iconSize);
	SetPort(port);
	
	SetUndoMode(umResize);
}

/* バックグラウンド編集モードへ */
void GoBackgroundMode(WindowPtr theWindow)
{
	PaintWinRec	*eWinRec;
	OSErr		err;
	RgnHandle	tempRgn;
	short		mode=umCannot;
	Rect		tempRect;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	ResetRuler();
	eWinRec=GetPaintWinRec(theWindow);
	
	GetGWorld(&cPort,&cDevice);
	GoOffPort(theWindow);
	
	/* 画面からはみ出ている部分を破棄する */
	if (eWinRec->isSelected)
	{
		GWorldFlags	flags;
		
		tempRgn=NewRgn();
		RectRgn(tempRgn,&eWinRec->iconSize);
		SectRgn(tempRgn,eWinRec->selectionPos,eWinRec->selectionPos);
		DisposeRgn(tempRgn);
		
		/* 画面表示用のリージョンも更新 */
		CopyRgn(eWinRec->selectionPos,eWinRec->eSelectedRgn);
		MapRgn(eWinRec->eSelectedRgn,&eWinRec->iconSize,&eWinRec->iconEditSize);
		
		/* イメージ、マスクの変形 */
		flags=UpdateGWorld(&eWinRec->editDataPtr,eWinRec->iconDepth,&eWinRec->iconSize,0,0,0);
		flags=UpdateGWorld(&eWinRec->currentMask,1,&eWinRec->iconSize,0,0,0);
		
		/* イメージ、マスクの描き戻し */
		SetGWorld(eWinRec->editDataPtr,0);
		MyLockPixels(editDataPtr);
		EraseRect(&eWinRec->iconSize);
		GetRegionBounds(eWinRec->selectionPos,&tempRect);
		CopyBits(GetPortBitMapForCopyBits(eWinRec->tempDataPtr),
			GetPortBitMapForCopyBits(eWinRec->editDataPtr),
			&eWinRec->selectedRect,&tempRect,srcCopy,nil);
		MyUnlockPixels(editDataPtr);
		SetGWorld(eWinRec->currentMask,0);
		MyLockPixels(currentMask);
		EraseRect(&eWinRec->iconSize);
		CopyBits(GetPortBitMapForCopyBits(eWinRec->pCurrentMask),
			GetPortBitMapForCopyBits(eWinRec->currentMask),
			&eWinRec->selectedRect,&tempRect,srcCopy,nil);
		MyUnlockPixels(currentMask);
		
		/* その他の変数を調整 */
		eWinRec->selectedRect=tempRect;
		SetPt(&eWinRec->selectionOffset,0,0);
	}
	
	if (eWinRec->editBackground) /* 背景編集時 */
	{
		/* 背景GWorldを作成 */
		err=NewGWorld(&eWinRec->backgroundGWorld,eWinRec->iconDepth,&eWinRec->iconSize,0,0,useTempMem);
		SetGWorld(eWinRec->backgroundGWorld,0);
		LockPixels(GetGWorldPixMap(eWinRec->backgroundGWorld));
		EraseRect(&eWinRec->iconSize);
		UnlockPixels(GetGWorldPixMap(eWinRec->backgroundGWorld));
		
		/* 編集中の画像を背景にコピー */
		CopyBits(GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
			GetPortBitMapForCopyBits(eWinRec->backgroundGWorld),
			&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
		
		if (eWinRec->foregroundGWorld == nil) /* 前景が存在しなければ */
		{
			EraseOffPort(eWinRec);
			/* マスク作成 */
			err=NewGWorld(&eWinRec->foregroundMask,1,&eWinRec->iconSize,0,0,useTempMem);
			MySetGWorld(foregroundMask);
			MyLockPixels(foregroundMask);
			EraseRect(&eWinRec->iconSize);
			MyUnlockPixels(foregroundMask);
			
			eWinRec->foreTransparency=kForeTrans100;
			
			mode=umAddForeground;
		}
		else
		{
			/* 前景を編集可能に */
			SetGWorld(eWinRec->editDataPtr,0);
			CopyBits(GetPortBitMapForCopyBits(eWinRec->foregroundGWorld),
				GetPortBitMapForCopyBits(eWinRec->editDataPtr),
				&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
			SetGWorld(eWinRec->selectedDataPtr,0);
			LockPixels(GetGWorldPixMap(eWinRec->selectedDataPtr));
			CopyBits(GetPortBitMapForCopyBits(eWinRec->foregroundGWorld),
				GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
				&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
			EraseRgn(eWinRec->selectionPos);
			UnlockPixels(GetGWorldPixMap(eWinRec->selectedDataPtr));
			
			/* 前景を破棄 */
			DisposeGWorld(eWinRec->foregroundGWorld);
			eWinRec->foregroundGWorld=nil;
		}
		
		eWinRec->dispBackground=true;
		
		/* マスク */
		MySetGWorld(currentMask);
		MyLockPixels(currentMask);
		EraseRect(&eWinRec->iconSize);
		CopyBits(GetPortBitMapForCopyBits(eWinRec->foregroundMask),
			GetPortBitMapForCopyBits(eWinRec->currentMask),
			&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,eWinRec->selectionPos);
		MyUnlockPixels(currentMask);
		
		MySetGWorld(selectionMask);
		MyLockPixels(selectionMask);
		EraseRect(&eWinRec->iconSize);
		CopyBits(GetPortBitMapForCopyBits(eWinRec->foregroundMask),
			GetPortBitMapForCopyBits(eWinRec->selectionMask),
			&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
		EraseRgn(eWinRec->selectionPos);
		MyUnlockPixels(selectionMask);
		DisposeGWorld(eWinRec->foregroundMask);
		eWinRec->foregroundMask=nil;
	}
	else /* 前景編集時 */
	{
		/* 前景GWorldを作成 */
		err=NewGWorld(&eWinRec->foregroundGWorld,eWinRec->iconDepth,&eWinRec->iconSize,0,0,useTempMem);
		SetGWorld(eWinRec->foregroundGWorld,0);
		MyLockPixels(foregroundGWorld);
		EraseRect(&eWinRec->iconSize);
		MyUnlockPixels(foregroundGWorld);
		
		/* 編集中の画像を前景にコピー */
		CopyBits(GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
			GetPortBitMapForCopyBits(eWinRec->foregroundGWorld),
			&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
		
		/* マスクをコピー */
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
		
		if (eWinRec->backgroundGWorld == nil) /* 背景が存在しなければ */
		{
			EraseOffPort(eWinRec);
			eWinRec->foreTransparency=kForeTrans100;
			
			mode=umAddBackground;
		}
		else
		{
			/* 背景を編集可能に */
			SetGWorld(eWinRec->editDataPtr,0);
			CopyBits(GetPortBitMapForCopyBits(eWinRec->backgroundGWorld),
				GetPortBitMapForCopyBits(eWinRec->editDataPtr),
				&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
			SetGWorld(eWinRec->selectedDataPtr,0);
			LockPixels(GetGWorldPixMap(eWinRec->selectedDataPtr));
			CopyBits(GetPortBitMapForCopyBits(eWinRec->backgroundGWorld),
				GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
				&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
			RGBBackColor(&gBackColor.rgb);
			EraseRgn(eWinRec->selectionPos);
			BackColor(whiteColor);
			UnlockPixels(GetGWorldPixMap(eWinRec->selectedDataPtr));
			
			/* 背景を破棄 */
			DisposeGWorld(eWinRec->backgroundGWorld);
			eWinRec->backgroundGWorld=nil;
		}
		
		/* マスク */
		MySetGWorld(currentMask);
		MyLockPixels(currentMask);
		PaintRgn(eWinRec->selectionPos);
		MyUnlockPixels(currentMask);
		
		MySetGWorld(selectionMask);
		MyLockPixels(selectionMask);
		PaintRect(&eWinRec->iconSize);
		if (gBackColor.isTransparent)
			EraseRgn(eWinRec->selectionPos);
		MyUnlockPixels(selectionMask);
	}
	
	RectRgn(eWinRec->updateRgn,&eWinRec->iconSize);
	
	SetGWorld(cPort,cDevice);
	DispOffPort(theWindow);
	
	eWinRec->editBackground=!eWinRec->editBackground;
	UpdateBGMenu();
	UpdateEffectMenu();
	UpdateTransparentMenu();
	UpdateBackInfo(theWindow);
	SetUndoMode(mode);
	if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
		UpdatePaletteCheck();
}

/* バックグラウンドを統合 */
void UnionBackground(WindowPtr theWindow)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	short		foreMode;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	if (eWinRec->backgroundGWorld == nil && eWinRec->foregroundGWorld == nil) /* バックグラウンドが存在しない */
	{
		UpdateBGMenu();
		return;
	}
	
	GetGWorld(&cPort,&cDevice);
	
	/* とりあえず、固定する */
	if (eWinRec->isSelected)
		FixSelection(theWindow);
	
	/* 背景表示時には前景表示にする */
	if (eWinRec->foregroundGWorld != nil)
		GoBackgroundMode(theWindow);
	
	/* 前景を取り消し用バッファにバックアップ */
	GoOffPort(theWindow);
	
	/* 背景は選択領域外にバックアップ */
	SetGWorld(eWinRec->selectedDataPtr,0);
	CopyBits(GetPortBitMapForCopyBits(eWinRec->backgroundGWorld),
		GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
		&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
	
	if (eWinRec->foreTransparency != kForeTrans0)
	{
		SetGWorld(eWinRec->backgroundGWorld,0);
		foreMode=SetForeBlend(eWinRec);
		
		/* 次に、背景にコピーして合成 */
		SafeCopyDeepMask(GetPortBitMapForCopyBits(eWinRec->dispTempPtr),
			GetPortBitMapForCopyBits(eWinRec->currentMask),
			GetPortBitMapForCopyBits(eWinRec->backgroundGWorld),
			&eWinRec->iconSize,&eWinRec->iconSize,&eWinRec->iconSize,foreMode,nil);
	}
	
	/* 背景から編集画面にコピー */
	SetGWorld(eWinRec->editDataPtr,0);
	CopyBits(GetPortBitMapForCopyBits(eWinRec->backgroundGWorld),
		GetPortBitMapForCopyBits(eWinRec->editDataPtr),
		&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
	
	/* 背景を破棄 */
	DisposeGWorld(eWinRec->backgroundGWorld);
	eWinRec->backgroundGWorld=nil;
	eWinRec->editBackground=gOtherPrefs.addForeground;
	
	/* 背景が透明かどうかのフラグをリセット */
	eWinRec->isBackTransparent=false;
	
	/* マスク */
	MySetGWorld(currentMask);
	MyLockPixels(currentMask);
	PaintRect(&eWinRec->iconSize);
	MyUnlockPixels(currentMask);
	
	RectRgn(eWinRec->updateRgn,&eWinRec->iconSize);
	
	SetGWorld(cPort,cDevice);
	DispOffPort(theWindow);
	
	UpdateBGMenu();
	UpdateEffectMenu();
	UpdateBackInfo(theWindow);
	SetUndoMode(umUnionBackground);
	if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
		UpdatePaletteCheck();
}

/* 背景の削除 */
void DeleteBackground(WindowPtr theWindow)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	if (eWinRec->backgroundGWorld == nil && eWinRec->foregroundGWorld == nil) /* バックグラウンドが存在しない */
	{
		UpdateBGMenu();
		return;
	}
	
	GetGWorld(&cPort,&cDevice);
	
	/* とりあえず、固定する */
	if (eWinRec->isSelected)
		FixSelection(theWindow);
	
	/* 背景表示時には前景表示にする */
	if (eWinRec->foregroundGWorld != nil)
		GoBackgroundMode(theWindow);
	
	/* 前景をバックアップ */
	GoOffPort(theWindow);
	
	/* 背景をバックアップ */
	SetGWorld(eWinRec->selectedDataPtr,0);
	CopyBits(GetPortBitMapForCopyBits(eWinRec->backgroundGWorld),
		GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
		&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
	
	/* 背景を削除 */
	DisposeGWorld(eWinRec->backgroundGWorld);
	eWinRec->backgroundGWorld=nil;
	eWinRec->editBackground=gOtherPrefs.addForeground;
	
	RectRgn(eWinRec->updateRgn,&eWinRec->iconSize);
	
	SetGWorld(cPort,cDevice);
	DispOffPort(theWindow);
	
	UpdateBGMenu();
	UpdateEffectMenu();
	UpdateBackInfo(theWindow);
	SetUndoMode(umDeleteBackground);
	if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
		UpdatePaletteCheck();
}

/* 前景の削除 */
void DeleteForeground(WindowPtr theWindow)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	GWorldPtr	tempGWorld;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	if (eWinRec->backgroundGWorld == nil && eWinRec->foregroundGWorld == nil) /* バックグラウンドが存在しない */
	{
		UpdateBGMenu();
		return;
	}
	
	GetGWorld(&cPort,&cDevice);
	
	/* とりあえず、固定する */
	if (eWinRec->isSelected)
		FixSelection(theWindow);
	
	/* 前景表示時には背景表示にする */
	if (eWinRec->backgroundGWorld != nil)
	{
		OSErr	err;
		
		/* マスクをバックアップする */
		err=NewGWorld(&tempGWorld,eWinRec->iconDepth,&eWinRec->iconSize,0,0,useTempMem);
		SetGWorld(tempGWorld,0);
		CopyBits(GetPortBitMapForCopyBits(eWinRec->currentMask),
			GetPortBitMapForCopyBits(tempGWorld),
			&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
		SetGWorld(cPort,cDevice);
		
		GoBackgroundMode(theWindow);
		
		/* マスクを元に戻す */
		SetGWorld(eWinRec->currentMask,0);
		CopyBits(GetPortBitMapForCopyBits(tempGWorld),
			GetPortBitMapForCopyBits(eWinRec->currentMask),
			&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
		SetGWorld(cPort,cDevice);
		DisposeGWorld(tempGWorld);
	}
	
	/* 背景をバックアップ */
	GoOffPort(theWindow);
	
	/* 前景をバックアップ */
	SetGWorld(eWinRec->selectedDataPtr,0);
	CopyBits(GetPortBitMapForCopyBits(eWinRec->foregroundGWorld),
		GetPortBitMapForCopyBits(eWinRec->selectedDataPtr),
		&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
	
	/* 前景を削除 */
	DisposeGWorld(eWinRec->foregroundGWorld);
	eWinRec->foregroundGWorld=nil;
	eWinRec->editBackground=gOtherPrefs.addForeground;
	
	/* マスク */
	SetGWorld(eWinRec->currentMask,0);
	MyLockPixels(currentMask);
	PaintRect(&eWinRec->iconSize);
	MyUnlockPixels(currentMask);
	RectRgn(eWinRec->updateRgn,&eWinRec->iconSize);
	
	SetGWorld(cPort,cDevice);
	DispOffPort(theWindow);
	
	UpdateBGMenu();
	UpdateEffectMenu();
	UpdateBackInfo(theWindow);
	SetUndoMode(umDeleteForeground);
	if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
		UpdatePaletteCheck();
}

/* 背景の表示／非表示を切り替え */
void ShowHideBackground(WindowPtr theWindow)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	Rect		r;
	
	if (eWinRec->backgroundGWorld == nil)
	{
		UpdateBGMenu();
		return;
	}
	
	eWinRec->dispBackground=!eWinRec->dispBackground;
	GetWindowPortBounds(theWindow,&r);
	r.right-=kScrollBarWidth;
	r.bottom-=kScrollBarHeight;
	SetPortWindowPort(theWindow);
	MyInvalWindowRect(theWindow,&r);
	SetPortWindowPort(gPreviewWindow);
	MyInvalWindowPortBounds(gPreviewWindow);
	SetPortWindowPort(theWindow);
	
	UpdateBGMenu();
}

/* 選択中の枠を表示 */
void TrackMarquee(PaintWinRec *eWinRec,Rect *previewRect,Boolean shiftDown)
{
	PatHandle	gray;
	Point	mousePt;
	Rect 	startRect;
	Rect	nowRect,prevRect;
	Rect	pNowRect,pPrevRect;
	GrafPtr	port;
	WindowPtr	theWindow;
	short	ratio=eWinRec->ratio;
	Rect	imageRect,tempRect;
	
	gray=GetPattern(128);
	
	theWindow=MyFrontNonFloatingWindow();
	
	/* ペンパターンの設定 */
	GetPort(&port);
	SetPortWindowPort(theWindow);
	
	/* 表示可能領域 */
	GetWindowPortBounds(theWindow,&imageRect);
	imageRect.right-=kScrollBarWidth;
	imageRect.bottom-=kScrollBarHeight;
	ClipRect(&imageRect);
	
	PenNormal();
	PenMode(patXor);
	PenPat(*gray);
	SetPortWindowPort(gPreviewWindow);
	PenNormal();
	PenMode(patXor);
	PenPat(*gray);
	
	startRect=*previewRect;
	MapRect(&startRect,&eWinRec->iconSize,&eWinRec->iconEditSize);
	nowRect=startRect;
	pNowRect=*previewRect;
	SetPortWindowPort(theWindow);
	FrameRect(&nowRect);
	SetPortWindowPort(gPreviewWindow);
	FrameRect(&pNowRect);
	SetPortWindowPort(theWindow);
	prevRect=nowRect;
	pPrevRect=pNowRect;
	
	while (StillDown())
	{
		GetMouse(&mousePt);
		ToRealPos(&mousePt,ratio);
		if (shiftDown)
			FixToGrid(&mousePt);
		SetRect(&pNowRect,previewRect->left,previewRect->top,mousePt.h,mousePt.v);
		SortRect(&pNowRect,shiftDown);
		if (!EqualRect(&pNowRect,&pPrevRect))
		{
			SectRect(&pNowRect,&eWinRec->iconSize,&pNowRect);
			nowRect=pNowRect;
			MapRect(&nowRect,&eWinRec->iconSize,&eWinRec->iconEditSize);
			FrameRect(&prevRect);
			FrameRect(&nowRect);
			SetPortWindowPort(gPreviewWindow);
			FrameRect(&pPrevRect);
			FrameRect(&pNowRect);
			SetPortWindowPort(theWindow);
			prevRect=nowRect;
			pPrevRect=pNowRect;
		}
	}
	SetPortWindowPort(theWindow);
	FrameRect(&prevRect);
	
	GetWindowPortBounds(theWindow,&tempRect);
	ClipRect(&tempRect);
	PenNormal();
	
	SetPortWindowPort(gPreviewWindow);
	FrameRect(&pPrevRect);
	PenNormal();
	SetPort(port);
	*previewRect=pPrevRect;
}

/* sort rectangle */
/* shiftが押されているときは、グリッドの分だけ補正する */
void SortRect(Rect *dragRect,Boolean shiftDown)
{
	short	temp;
	short	h=1;
	
	/* 1.20b12修正 */
	if (shiftDown) {
		if (((gToolPrefs.gridMode>>4)& 0x0f) == 1) { /* 32*32 */
			h = 32;
		} else {
			h = 16;
        }
    }
	
	if (dragRect->bottom<dragRect->top)
	{
		temp=dragRect->bottom;
		dragRect->bottom=dragRect->top+h;
		dragRect->top=temp;
	}
	if (dragRect->right<dragRect->left)
	{
		temp=dragRect->right;
		dragRect->right=dragRect->left+h;
		dragRect->left=temp;
	}
}

/* 指定された点がマスク内かどうかを調べる */
Boolean PtInMask(Point pt,PaintWinRec *eWinRec)
{
	if (eWinRec->isSelected) /* 選択範囲がある */
		SubPt(eWinRec->selectionOffset,&pt);
	
	return RealPtInMask(pt,eWinRec->currentMask);
}

/* 指定された点がマスク内かどうか調べる２ */
Boolean RealPtInMask(Point pt,GWorldPtr maskGWorld)
{
	PixMapHandle	pmh;
	long	rowBytes;
	Ptr		baseAddr;
	Boolean	result;
	short	left,top;
	Rect	tempRect;
	
	GetPortBounds(maskGWorld,&tempRect);
	if (!PtInRect(pt,&tempRect)) return false;
	
	pmh=GetGWorldPixMap(maskGWorld);
	LockPixels(pmh);
	rowBytes=MyGetPixRowBytes(pmh) & 0x3fff;
	baseAddr=MyGetPixBaseAddr(pmh);
	
	left=tempRect.left;
	top=tempRect.top;
	result=BitTst(baseAddr+rowBytes * (pt.v - top),(pt.h - left));
	UnlockPixels(pmh);
	return result;
}

/* 前景の不透明度によってブレンド割り合いを決定する */
short SetForeBlend(PaintWinRec *eWinRec)
{
	short	foreMode;
	UInt16	t[]={0xc000,0x8000,0x4000,0x0000},tt;
	RGBColor	foreTransparency;
	
	switch(eWinRec->foreTransparency)
	{
		case kForeTrans100:
			foreMode=srcCopy;
			break;
		
		default:
			foreMode=blend;
			tt=t[eWinRec->foreTransparency-kForeTrans75];
			SetRGBColor(&foreTransparency,tt,tt,tt);
			OpColor(&foreTransparency);
	}
	
	return foreMode;
}

/* 座標を実際の位置に変換する */
void ToRealPos(Point *mousePt,short ratio)
{
	mousePt->h>>=ratio;
	mousePt->v>>=ratio;
}

/* 座標を画面上の位置に変換する */
void ToGridPos(Point *mousePt,short ratio)
{
	mousePt->h<<=ratio;
	mousePt->v<<=ratio;
}

/* グリッドにそった位置に動かす */
void FixToGrid(Point *mousePt)
{
	switch ((gToolPrefs.gridMode>>4)& 0x0f)
	{
		case 0:
		case 2:
			mousePt->h&=0xFFF0;
			mousePt->v&=0xFFF0;
			break;
		
		case 1:
			mousePt->h&=0xFFE0;
			mousePt->v&=0xFFE0;
			break;
	}
}

/* グリッドにそった位置に動かす */
/* 四捨五入する版 */
void FixToGrid2(Point *mousePt)
{
	switch ((gToolPrefs.gridMode>>4)& 0x0f)
	{
		case 0:
		case 2:
			mousePt->h = ((mousePt->h + 8) & 0xFFF0);
			mousePt->v = ((mousePt->v + 8) & 0xFFF0);
			break;
		
		case 1:
			mousePt->h = ((mousePt->h + 16) & 0xFFE0);
			mousePt->v = ((mousePt->v + 16) & 0xFFE0);
			break;
	}
}

/* BitMapを作成 */
OSErr NewBitMap(MyBitMapRec *bmpRec,Rect *rect)
{
	long	size;
	short	rowBytes;
	Ptr		baseAddr;
	OSErr	err;
	Ptr		temp;
	
	/* サイズチェック */
	if (EmptyRect(rect)) return paramErr;
	rowBytes=((rect->right-rect->left+31)>>5)<<2;
	size=(long)rowBytes*(long)(rect->bottom-rect->top);
	
	/* メモリ確保 */
	bmpRec->bmpDataH=TempNewHandle(size,&err);
	if (err!=noErr) return err;
	TempHLock(bmpRec->bmpDataH,&err);
	
	/* データをクリア */
	baseAddr=*bmpRec->bmpDataH;
	temp=baseAddr;
	for ( ; size>0; size--)
		*temp++=0;
	
	bmpRec->bmp.baseAddr=baseAddr;
	bmpRec->bmp.rowBytes=rowBytes;
	bmpRec->bmp.bounds=*rect;
	
	return noErr;
}

/* BitMapを破棄 */
OSErr DisposeBitMap(MyBitMapRec *bmpRec)
{
	OSErr	err;
	
	TempHUnlock(bmpRec->bmpDataH,&err);
	TempDisposeHandle(bmpRec->bmpDataH,&err);
	
	return err;
}

/* リージョンとRectの結合 */
void UnionRectRgn(RgnHandle dstRgn,Rect *srcRect)
{
	RgnHandle	tempRgn=NewRgn();
	
	RectRgn(tempRgn,srcRect);
	UnionRgn(dstRgn,tempRgn,dstRgn);
	DisposeRgn(tempRgn);
}

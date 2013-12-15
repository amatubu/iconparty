/*
 *  UpdataCursor.c
 *  IconParty
 *
 *  Created by naoki iimura on Sun Jul 14 2002.
 *  Copyright (c) 1997-2002 naoki iimura. All rights reserved.
 *
 */

#if __APPLE_CC__
#include <Carbon/Carbon.h>
#endif

#include "UpdateCursor.h"
#include "Globals.h"
#include "PreCarbonSupport.h"
#include "PaintRoutines.h"
#include "WindowRoutines.h"
#include "IconFamilyWindow.h"
#include "TabletUtils.h"
#include "UsefulRoutines.h"


static OSErr	SetPenCursorMain(short width,short height,RGBColor *crsrColor);


extern WindowPtr	ColorPalette1,ColorPalette2;

typedef struct {
	short	width;
	short	height;
	RGBColor	color;
} MyCursorRec;

static short	gCrsrID;
MyCursorRec		gCursData={0,0,{0,0,0}};



/* マウスカーソルの変更 */
void UpdateMouseCursor(Point mousePt)
{
	WindowPtr	theWindow;
	short	thePart;
	KeyMap	theKeys;
	Point	localPt;
	GrafPtr	port;
	Rect	editRect;
	
	#if !TARGET_API_MAC_CARBON
	if (isTSMgrAvailable)
	{
		thePart=FindServiceWindow(mousePt,&theWindow); /* 1.0b5追加 */
		if (theWindow!=nil) /* TSMフローティングウィンドウ上 */
		{
			MakeArrowCursorRgn();
			gCrsrID = 0;
			SetTSMCursor(mousePt);
			ResetRuler();
			return;
		}
	}
	#endif
	
	thePart=FindWindow(mousePt,&theWindow);
	if (theWindow==nil) /* ウィンドウ上でない */
	{
		MakeArrowCursorRgn();
		MySetCursor(0);
		return;
	}
	
	GetPort(&port);
	SetPortWindowPort(theWindow);
	localPt=mousePt;
	GlobalToLocal(&localPt);
	
	if (gToolPrefs.showRuler)
		DrawRuler(mousePt);
	
	if (thePart!=inContent || (localPt.h<0 || localPt.v<0))
	{
		MakeArrowCursorRgn();
		MySetCursor(0);
	}
	else
	{
		if (!IsWindowHilited(theWindow)) /* アクティブでないウィンドウ上では矢印カーソル */
		{
			MakeArrowCursorRgn();
			MySetCursor(0);
		}
		else
		{
			Boolean	cmdDown,optDown,ctrlDown;
			
			switch (GetExtWindowKind(theWindow))
			{
				case kWindowTypePaintWindow:
	//				InvertDot(theWindow);
					
					GetWindowPortBounds(theWindow,&editRect);
					editRect.right-=kScrollBarWidth;
					editRect.bottom-=kScrollBarHeight;
					if (!PtInRect(localPt,&editRect)) /* ペイント領域でなければ矢印カーソル */
					{
						RgnHandle	tempRgn=NewRgn();
						Point	offset={0,0};
						
						LocalToGlobal(&offset);
						MakeArrowCursorRgn();
						RectRgn(tempRgn,&editRect);
						OffsetRgn(tempRgn,offset.h,offset.v);
						DiffRgn(gCurRgnHand,tempRgn,gCurRgnHand);
						DisposeRgn(tempRgn);
						MySetCursor(0);
						break;
					}
					
					SetEmptyRgn(gCurRgnHand);
					
					GetKeys(theKeys);
					cmdDown=BitTst(theKeys,48);
					optDown=BitTst(theKeys,61);
					ctrlDown=BitTst(theKeys,60);
					
					if (cmdDown && optDown) /* グラブスクロール */
					{
						MySetCursor(140);
						break;
					}
					
					if (gSelectedTool==kMarqueeTool) /* 選択ツール */
					{
						if (isTabletAvailable && IsEraser() && gTabletPrefs.useEraser)
							SetPenCursor(kEraserTool);
						else
						{
							RgnHandle	eSelectedRgn=(GetPaintWinRec(theWindow))->eSelectedRgn;
							
							if (PtInRgn(localPt,eSelectedRgn)) /* 選択領域内 */
							{
								if (optDown) /* オプションキー */
									MySetCursor(kCursorHandOffPlus);
								else
									MySetCursor(kCursorHandOff);
							}
							else
								MySetCursor(128+kMarqueeTool);
						}
					}
					else /* 選択ツール以外 */
					{
						short	tool;
						
						if (optDown) /* オプションキー → スポイト */
							tool=kSpoitTool;
						else if (isTabletAvailable && gTabletPrefs.useEraser && IsEraser())
							tool=kEraserTool;
						else
							tool=gSelectedTool;
						
						switch (tool)
						{
							case kPencilTool:
								if (cmdDown)
								{
									if (gToolPrefs.eraserByCmdKey && !ctrlDown)
										SetPenCursor(kEraserTool);
									else
										MySetCursor(137);
								}
								else
									SetPenCursor(kPencilTool);
								break;
							
							case kEraserTool:
								SetPenCursor(kEraserTool);
								break;
							
							case kSpoitTool:
								MySetCursor((gToolPrefs.changeSpoitCursor ? 139 : 128+kSpoitTool));
								break;
							
							default:
								MySetCursor(128+gSelectedTool);
						}
					}
					break;
				
				case kWindowTypeColorPalette1:
				case kWindowTypeColorPalette2:
					GetWindowContentRgn(theWindow,gCurRgnHand);
					MySetCursor(gToolPrefs.changeSpoitCursor ? 139 : 128+kSpoitTool);
					break;
				
				case kWindowTypeBlendPalette:
					UpdateCursorInBlendPalette(localPt);
					break;
				
				case kWindowTypeIconFamilyWindow:
					UpdateCursorInFamilyWindow(theWindow,localPt);
					break;
				
				case kWindowTypeFavoritePalette:
					GetKeys(theKeys);
					cmdDown=BitTst(theKeys,48);
					UpdateCursorInFavoritePalette(localPt,cmdDown);
					break;
				
				case kWindowTypePreviewWindow:
				case kWindowTypeToolPalette:
				case kWindowTypeTitleWindow:
				default:
					MakeArrowCursorRgn();
					MySetCursor(0);
					break;
			}
		}
	}
	
	SetPort(port);
}

/* 矢印カーソルにするリージョンを計算 */
void MakeArrowCursorRgn(void)
{
	RgnHandle	tempRgn=NewRgn();
	
	/* まず、画面全体 */
	CopyRgn(GetGrayRgn(),gCurRgnHand);
	
	/* カラーパレット、ブレンドパレットのコンテントリージョンを除く */
	if (IsWindowVisible(ColorPalette1))
	{
		GetWindowContentRgn(ColorPalette1,tempRgn);
		DiffRgn(gCurRgnHand,tempRgn,gCurRgnHand);
	}
	if (IsWindowVisible(ColorPalette2))
	{
		GetWindowContentRgn(ColorPalette2,tempRgn);
		DiffRgn(gCurRgnHand,tempRgn,gCurRgnHand);
	}
	if (IsWindowVisible(gBlendPalette))
	{
		GetWindowContentRgn(gBlendPalette,tempRgn);
		DiffRgn(gCurRgnHand,tempRgn,gCurRgnHand);
	}
	
	DisposeRgn(tempRgn);
}

/* 鉛筆ツール、消しゴムツールのマウスカーソルを作成して設定 */
void SetPenCursor(short tool)
{
	if (gToolPrefs.changePencilCursor)
	{
		short	ratio=0;
		WindowPtr	theWindow=MyFrontNonFloatingWindow();
		OSErr	err;
		
		if (theWindow!=nil && GetExtWindowKind(theWindow)==kWindowTypePaintWindow)
			ratio=(GetPaintWinRec(theWindow))->ratio;
		
		switch (tool)
		{
			case kPencilTool:
				err=SetPenCursorMain(gPenWidth<<ratio,gPenHeight<<ratio,&gCurrentColor.rgb);
				if (err!=noErr)
					MySetCursor(128+kPencilTool);
				break;
			
			case kEraserTool:
				err=SetPenCursorMain(gEraserWidth<<ratio,gEraserHeight<<ratio,&gBackColor.rgb);
				if (err!=noErr)
					MySetCursor(128+kEraserTool);
				break;
			
			default:
				MySetCursor(0);
		}
	}
	else
	{
		switch (tool)
		{
			case kPencilTool:
				MySetCursor(128+kPencilTool);
				break;
			
			case kEraserTool:
				MySetCursor(128+kEraserTool);
				break;
			
			default:
				MySetCursor(0);
		}
	}
}

/* マウスカーソルの作成メイン */
OSErr SetPenCursorMain(short width,short height,RGBColor *crsrColor)
{
	GWorldPtr		cursGWorld;
	PixMapHandle	pmh;
	OSErr			err;
	Rect			cursRect={0,0,16,16},r;
	Cursor			curs;
	short			i;
	Ptr				baseAddr;
	long			rowBytes;
	GWorldPtr		cPort;
	GDHandle		cDevice;
	
	if (width <= 0 || width > 16 || height <= 0 || height > 16) return paramErr;
	
	if (gCrsrID==-1)
	{
		if (gCursData.width == width && gCursData.height == height && EqualColor(&gCursData.color,crsrColor))
			return noErr;
	}
	else
		gCrsrID=-1;
	
	gCursData.width=width;
	gCursData.height=height;
	gCursData.color=*crsrColor;
	
	GetGWorld(&cPort,&cDevice);
	
	err=NewGWorld(&cursGWorld,1,&cursRect,0,0,useTempMem);
	if (err!=noErr) return err;
	
	pmh=GetGWorldPixMap(cursGWorld);
	LockPixels(pmh);
	baseAddr=MyGetPixBaseAddr(pmh);
	rowBytes=MyGetPixRowBytes(pmh) & 0x3fff;
	
	/* GWorldにカーソルにする絵を描画 */
	SetGWorld(cursGWorld,0);
	EraseRect(&cursRect);
	SetRect(&r,0,0,width,height);
	PaintRect(&r);
	
	/* カーソルにコピー */
	for (i=0; i<16; i++)
	{
		curs.data[i]=*(short *)baseAddr;
		baseAddr+=rowBytes;
		
		curs.mask[i]=0;
	}
	
	SetGWorld(cPort,cDevice);
	UnlockPixels(pmh);
	DisposeGWorld(cursGWorld);
	
	if (gToolPrefs.useColorCursor) /* カラーカーソル */
	{
		CCrsrHandle		ccurs;
		short			j;
		UInt32			*ul;
		
		ccurs=GetCCursor(128);
		(**(**(**ccurs).crsrMap).pmTable).ctTable[1].rgb=*crsrColor;
		CTabChanged((**(**ccurs).crsrMap).pmTable);
		HLock((**ccurs).crsrData);
		ul=(UInt32 *)*(**ccurs).crsrData;
		
		for (i=0; i<16; i++)
		{
			*ul=0;
			for (j=0; j<16; j++)
				*ul+=OSSwapHostToBigInt32(((UInt32)((OSSwapBigToHostInt16(curs.data[i]) >> j) & 1)) << (j*2));
			ul++;
		}
		
		HUnlock((**ccurs).crsrData);
		BlockMoveData(&curs.data[0],&(**ccurs).crsr1Data[0],16*16/8);
		BlockMoveData(&curs.data[0],&(**ccurs).crsrMask[0],16*16/8);
		SetPt(&(**ccurs).crsrHotSpot,width/2,height/2);
		SetCCursor(ccurs);
		DisposeCCursor(ccurs);
	}
	else
	{
		/* カーソルの中心 */
		SetPt(&curs.hotSpot,width/2,height/2);
		
		SetCursor(&curs);
	}
	
	return noErr;
}

/* カーソルの変更 */
void MySetCursor(short id)
{
	CursHandle	curs;
	
	if (id==gCrsrID) return;
	
	gCrsrID=id;
	
	switch (id)
	{
		case 0:
			InitCursor();
			break;
		
		case -1:
			break;
		
		default:
			curs=GetCursor(id);
			SetCursor(*curs);
	}
}
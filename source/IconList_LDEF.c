/* ------------------------------------------------------------ */
/*  IconList_LDEF.c                                             */
/*     アイコンリストウィンドウ用のLDEF                         */
/*                                                              */
/*                 1997.12.30 - 2001.12.31  naoki iimura        */
/* ------------------------------------------------------------ */

/* includes */
#ifdef __APPLE_CC__
#include	<Carbon/Carbon.h>
#else
#include	<Icons.h>
#include	<NumberFormatting.h>
#include	<LowMem.h>
#include	<Resources.h>
#include	<Gestalt.h>
#endif

#if TARGET_API_MAC_CARBON
#include	"Globals.h"
#endif
#include	"Definition.h"
#include	"IconRoutines.h"
#include	"MenuRoutines.h"
#include	"PreCarbonSupport.h"

#define		isBlack(color) (color.red == 0 && color.green == 0 && color.blue == 0)

/* prototypes */
#if TARGET_API_MAC_CARBON
	pascal void MyIconListLDEFProc(short message,Boolean selected,Rect *cellRect,Cell theCell,
									short dataOffset,short dataLen,ListHandle theList);
#endif

static void	MyLDEFInit(ListHandle theList);
static void	MyLDEFDraw(Boolean selected,Rect *cellRect,Cell theCell,
					short dataOffset,short dataLen,ListHandle theList);
//static void	MyLDEFHighlight(Rect *cellRect,Cell theCell,
//					short dataOffset,short dataLen,ListHandle theList);
static void	MyLDEFClose(ListHandle theList);

static OSErr	MyLDEFGetIconSuite(IconSuiteRef *iconSuite,IconListRec *iconListRec,IconListDataRec *data);

/* 実際のルーチン */
/* MyLDEF：LDEFのメインルーチン。送られてくるメッセージによって処理をわける */
#if TARGET_API_MAC_CARBON
pascal void MyIconListLDEFProc(short message,Boolean selected,Rect *cellRect,Cell theCell,
								short dataOffset,short dataLen,ListHandle theList)
#else
pascal void	main(short message,Boolean selected,Rect *cellRect,Cell theCell,short dataOffset,
					short dataLen,ListHandle theList)
#endif
{
	switch (message)
	{
		case lInitMsg:
			MyLDEFInit(theList);
			break;
		
		case lDrawMsg:
		case lHiliteMsg:
			MyLDEFDraw(selected,cellRect,theCell,dataOffset,dataLen,theList);
			break;
		
//			MyLDEFHighlight(selected,cellRect,theCell,dataOffset,dataLen,theList);
//			break;
		
		case lCloseMsg:
			MyLDEFClose(theList);
			break;
	}
}

/* リストの初期化 */
/* このLDEFではやることがない */
static void MyLDEFInit(ListHandle theList)
{
	#pragma unused(theList)
}

/* セルの描画 */
/* セルの内容をとりだし、描画する */
static void MyLDEFDraw(Boolean selected,Rect *cellRect,Cell theCell,short dataOffset,short dataLen,
					ListHandle theList)
{
	#pragma unused(dataOffset,theCell)
	GrafPtr		savedPort;
	RgnHandle	savedClip;
	PenState	savedPenState;
	IconListRec	*iconListRec;
	
	Rect	iconRect={0,0,32,32},frameRect;
	
	if (dataLen==0) return;
	
	/* ポート、クリップリージョン、ペン状態を保存 */
	GetPort(&savedPort);
	SetPort(GetListPort(theList));
	savedClip=NewRgn();
	GetClip(savedClip);
	ClipRect(cellRect);
	GetPenState(&savedPenState);
	PenNormal();
	TextSize(9);
	
	/* アプリケーションの参照番号などの情報を得る */
	iconListRec=(IconListRec *)GetListRefCon(theList);
	
	if (dataLen==sizeof(IconListDataRec *))
	{
		Str15	idStr;
		
		IconSuiteRef	iconSuite;
		OSErr	err;
		Rect	idRect;
		RgnHandle	frameRgn=NewRgn(),iconRgn=NewRgn();
		Boolean	smallFlag;
		Handle	h;
		IconListDataRec	*cellData;
		
		LGetCell(&cellData,&dataLen,theCell,theList);
		
		/* とりあえずアイコンを読み込む */
		err=MyLDEFGetIconSuite(&iconSuite,iconListRec,cellData);
		err=GetIconFromSuite(&h,iconSuite,kLarge1BitMask);
		smallFlag=(h==nil);
		
		/* アイコンの表示位置 */
		if (smallFlag)
			SetRect(&iconRect,cellRect->left+12+8,cellRect->top+8+8,
					cellRect->left+12+8+16,cellRect->top+8+8+16);
		else
			SetRect(&iconRect,cellRect->left+12,cellRect->top+8,
					cellRect->left+32+12,cellRect->top+32+8);
		frameRect=iconRect;
		InsetRect(&frameRect,-3,-3);
		RectRgn(frameRgn,&frameRect);
		
		err=IconSuiteToRgn(iconRgn,&iconRect,kAlignNone,iconSuite);
		DiffRgn(frameRgn,iconRgn,frameRgn);
		EraseRgn(frameRgn);
		DisposeRgn(frameRgn);
		DisposeRgn(iconRgn);
		
		idRect=*cellRect;
		idRect.top+=32+8+2;
		
		#if 1 /* OffScreen版 */
		{
			GWorldPtr	tempGWorld;
			GWorldPtr	currGWorld;
			GDHandle	currDevice;
			OSErr		err;
			
			GetGWorld(&currGWorld,&currDevice);
			err=NewGWorld(&tempGWorld,(iconListRec->isIconServicesAvailable ? 24 : 8),
				cellRect,0,0,useTempMem);
			SetGWorld(tempGWorld,0);
			LockPixels(GetGWorldPixMap(tempGWorld));
			EraseRect(cellRect);
			
			TextSize(9);
			TextFont(applFont);
			
			/* idを表示 */
			NumToString(cellData->resID,idStr);
			if (cellData->resType == kIconFamilyType)
				idStr[++idStr[0]]='i';
			else if (smallFlag)
				idStr[++idStr[0]]='s';
			MoveTo(cellRect->left+28-(StringWidth(idStr)>>1),cellRect->top+5+32+5+12);
			
			if (selected) /* 選択されていればハイライトさせる */
			{
				RGBColor	hiliteColor,tempColor;
				
				err=PlotIconSuite(&iconRect,kAlignNone,kTransformSelected,iconSuite);
				
				PenMode(srcXor);
				PenSize(2,2);
				FrameRect(&frameRect);
				PenNormal();
				
				TextMode(srcCopy);
				GetBackColor(&tempColor);
				LMGetHiliteRGB(&hiliteColor);
				RGBBackColor(&hiliteColor);
				if (isBlack(hiliteColor))
				{
					ForeColor(whiteColor);
					DrawString(idStr);
					ForeColor(blackColor);
				}
				else
				{
					DrawString(idStr);
				}
				RGBBackColor(&tempColor);
			}
			else
			{
				err=PlotIconSuite(&iconRect,kAlignNone,kTransformNone,iconSuite);
				TextMode(srcCopy);
				DrawString(idStr);
			}
			
			SetGWorld(currGWorld,currDevice);
			CopyBits(GetPortBitMapForCopyBits(tempGWorld),GetPortBitMapForCopyBits(currGWorld),
				cellRect,cellRect,srcCopy,NULL);
			
			UnlockPixels(GetGWorldPixMap(tempGWorld));
			DisposeGWorld(tempGWorld);
		}
		#else
		
		/* idを表示 */
		NumToString(cellData->resID,idStr);
		if (cellData->resType == kIconFamilyType)
			idStr[++idStr[0]]='i';
//		if (smallFlag)
//			idStr[++idStr[0]]='s';
		else
		{
			Str15	idStr2;
			
			BlockMoveData(&idStr[0],&idStr2[0],idStr[0]+1);
			idStr2[++idStr2[0]]='i';
//			idStr2[++idStr2[0]]='s';
			
			MoveTo(cellRect->left+28-(StringWidth(idStr2)>>1),cellRect->top+5+32+5+12);
			ForeColor(whiteColor);
			DrawString(idStr2);
			ForeColor(blackColor);
		}
		MoveTo(cellRect->left+28-(StringWidth(idStr)>>1),cellRect->top+5+32+5+12);
		
		if (selected) /* 選択されていればハイライトさせる */
		{
			RGBColor	hiliteColor,tempColor;
			
			err=PlotIconSuite(&iconRect,kAlignNone,kTransformSelected,iconSuite);
			
			PenMode(srcXor);
			PenSize(2,2);
			FrameRect(&frameRect);
			PenNormal();
			
			TextMode(srcCopy);
			GetBackColor(&tempColor);
			LMGetHiliteRGB(&hiliteColor);
			RGBBackColor(&hiliteColor);
			if (isBlack(hiliteColor))
			{
				ForeColor(whiteColor);
				DrawString(idStr);
				ForeColor(blackColor);
			}
			else
			{
				DrawString(idStr);
			}
			RGBBackColor(&tempColor);
		}
		else
		{
			err=PlotIconSuite(&iconRect,kAlignNone,kTransformNone,iconSuite);
			TextMode(srcCopy);
			DrawString(idStr);
		}
		#endif
		
		err=DisposeIconSuite(iconSuite,true);
		
		UseResFile(iconListRec->gApplRefNum);
	}

	/* ポート、クリップリージョン、ペン状態を元に戻す */
	SetPort(savedPort);
	SetClip(savedClip);
	DisposeRgn(savedClip);
	SetPenState(&savedPenState);
}

#if 0
/* セルをハイライトさせる */	
static void MyLDEFHighlight(Rect *cellRect,Cell theCell,short dataOffset,short dataLen,ListHandle theList)
{
	#pragma unused(theCell,dataOffset,theList)
	Rect	iconRect;
	
	if (dataLen==0) return;
	
	SetRect(&iconRect,cellRect->left+12,cellRect->top+8,
		cellRect->left+32+12,cellRect->top+32+8);
	
	InsetRect(&iconRect,-3,-3);
	
	PenMode(srcXor);
	PenSize(2,2);
	
	FrameRect(&iconRect);
	
	PenMode(srcCopy);
	PenSize(1,1);
}
#endif

/* リストを消去する */
/* リストのデータとして確保したメモリを解放 */
static void	MyLDEFClose(ListHandle theList)
{
	Cell	aCell;
	Ptr		dataPtr;
	short	dataLength;
	ListBounds	r;
	
	SetPt(&aCell,0,0);
	GetListDataBounds(theList,&r);
	if (PtInRect(aCell,&r))
	{
		do
		{
			dataLength=sizeof(Ptr);
			LGetCell(&dataPtr,&dataLength,aCell,theList);
			if (dataLength==sizeof(Ptr))
				DisposePtr(dataPtr);
		} while (LNextCell(true,true,&aCell,theList));
	}
	
	DisposePtr((Ptr)GetListRefCon(theList));
}

/* 指定IDのアイコンを取得 */
static OSErr MyLDEFGetIconSuite(IconSuiteRef *iconSuite,IconListRec *iconListRec,IconListDataRec *data)
{
	OSErr	err;
	Handle	iconData;
	
	UseResFile(iconListRec->tempRefNum);
	
	if (data->resType == kIconFamilyType)
	{
		/* 'icns' */
		IconFamilyHandle	iconFamily;
		
		iconFamily = (IconFamilyHandle)Get1Resource(kIconFamilyType,data->resID);
		if (iconFamily == nil)
		{
			UseResFile(iconListRec->refNum);
			iconFamily = (IconFamilyHandle)Get1Resource(kIconFamilyType,data->resID);
		}
		
		err=IconFamilyToIconSuite(iconFamily,kSelectorMy32Data,iconSuite);
		ReleaseResource((Handle)iconFamily);
	}
	else
	{
		iconData=Get1Resource(kLarge1BitMask,data->resID);
		if (iconData == nil)
		{
			iconData=Get1Resource(kSmall1BitMask,data->resID);
			if (iconData==nil)
			{
				UseResFile(iconListRec->refNum);
			}
		}
		
		err=Get1IconSuite(iconSuite,data->resID,
			(iconListRec->isIconServicesAvailable ? kSelectorMy32Data : kSelectorMyData));
	}
	
	UseResFile(iconListRec->gApplRefNum);
	
	return err;
}

#if !TARGET_API_MAC_CARBON
/* カレントリソースの中からアイコンを取得 */
OSErr Get1IconSuite(IconSuiteRef *theIconSuite,short theResID,IconSelectorValue selector)
{
	OSErr	err;
	ResType	resList[]={	kLarge1BitMask,kLarge4BitData,kLarge8BitData,kLarge32BitData,kLarge8BitMask,0,0,0,
						kSmall1BitMask,kSmall4BitData,kSmall8BitData,kSmall32BitData,kSmall8BitMask,0,0,0,
						 kMini1BitMask, kMini4BitData, kMini8BitData,0,0,0,0,0,
						 kHuge1BitMask, kHuge4BitData, kHuge8BitData, kHuge32BitData, kHuge8BitMask,0,0,0};
	
	short	i,j=0;
	Handle	h;
	
	err=NewIconSuite(theIconSuite);
	if (err!=noErr) return err;
	
	for (i=0; i<32 && err==noErr; i++)
	{
		if ((selector & (1L << i)) != 0)
		{
			h=Get1Resource(resList[i],theResID);
			if (h != nil)
			{
				err=AddIconToSuite(h,*theIconSuite,resList[i]);
				j++;
			}
		}
	}
	
	return err;
}
#endif
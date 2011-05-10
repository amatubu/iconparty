/* ------------------------------------------------------------ */
/*  PenSize_MDEF.c                                              */
/*     ペンサイズメニューを実現するためのMDEF                   */
/*                                                              */
/*                 1999.1.10 - 2000.10.29  naoki iimura       	*/
/* ------------------------------------------------------------ */

/* includes */
#ifdef __APPLE_CC__
#include	<Carbon/Carbon.h>
#else
#include	<Appearance.h>
#include	<Gestalt.h>
#endif

/* definitions */
#define	kGestaltMask	1L
#define	kItemWidth	16
#define	kItemHeight	16
#define	kRows		4
#define	kColumns		4
#define	kItemNum		kRows*kRolumns

/* prototypes */
static void GetItemRect(Rect *r,Rect *menuRect,short item);
static void DrawItem(Rect *r,short i,short j);

#if TARGET_API_MAC_CARBON
	pascal void PenSizeMDEFProc(short message,MenuHandle theMenu,Rect *menuRect,Point hitPt,short *whichItem);
#endif

#if TARGET_API_MAC_CARBON
pascal void PenSizeMDEFProc(short message,MenuHandle theMenu,Rect *menuRect,Point hitPt,short *whichItem)
#else
pascal void main(short message,MenuHandle theMenu,Rect *menuRect,Point hitPt,short *whichItem)
#endif
{
	short	i,j;
	short	currentItem;
	Rect	r;
	OSErr	err;
	long	response;
	#if TARGET_API_MAC_CARBON
	MenuTrackingData	*trackingData;
	HiliteMenuItemData	*hiliteData;
	#endif
	
	switch (message)
	{
		case kMenuDrawMsg:
			/* メニューの描画 */
			err=Gestalt(gestaltAppearanceAttr,&response);
			if (err==noErr && (response & (kGestaltMask << gestaltAppearanceExists)))
			{
				/* Appearance Managerのバージョンが1.0.1以降かどうか */
				err=Gestalt(gestaltAppearanceVersion,&response);
				if (err==noErr && response>=0x0101)
					#if powerc
						if (DrawThemeMenuBackground != nil)
					#endif
					err=DrawThemeMenuBackground(menuRect,kThemeMenuTypePopUp);
			}
			
			/* チェック */
			for (i=0; i<kRows; i++)
				for (j=0; j<kColumns; j++)
				{
					short	markChar;
					
					GetItemRect(&r,menuRect,i*kColumns+j);
					DrawItem(&r,i,j);
					GetItemMark(theMenu,i*kColumns+j+1,&markChar);
					if (markChar != noMark)
					{
						InsetRect(&r,2,2);
						PenMode(srcXor);
						FrameRect(&r);
						PenNormal();
					}
				}
			break;
		
		#if TARGET_API_MAC_CARBON
		case kMenuFindItemMsg:
			trackingData=(MenuTrackingData *)whichItem;
			
			currentItem=0;
			if (PtInRect(hitPt,menuRect))
			{
				for (i=0; i<kRows; i++)
					for (j=0; j<kColumns; j++)
					{
						GetItemRect(&r,menuRect,i*kColumns+j);
						if (PtInRect(hitPt,&r))
						{
							currentItem=i*kColumns+j+1;
							break;
						}
					}
			}
			trackingData->itemSelected = currentItem;
			trackingData->itemUnderMouse = currentItem;
			break;
		
		case kMenuHiliteItemMsg:
			hiliteData=(HiliteMenuItemData *)whichItem;
			
			if (hiliteData->previousItem != 0)
			{
				GetItemRect(&r,menuRect,hiliteData->previousItem-1);
				InsetRect(&r,1,1);
				PenMode(srcXor);
				FrameRect(&r);
				PenNormal();
			}
			if (hiliteData->newItem != 0)
			{
				GetItemRect(&r,menuRect,hiliteData->newItem-1);
				InsetRect(&r,1,1);
				PenMode(srcXor);
				FrameRect(&r);
				PenNormal();
			}
			break;
		#else
		case mChooseMsg:
			/* メニューの選択（反転） */
			currentItem=0;
			if (PtInRect(hitPt,menuRect))
			{
				for (i=0; i<kRows; i++)
					for (j=0; j<kColumns; j++)
					{
						GetItemRect(&r,menuRect,i*kColumns+j);
						if (PtInRect(hitPt,&r))
						{
							currentItem=i*kColumns+j+1;
							break;
						}
					}
			}
			if (currentItem!=*whichItem)
			{
				if (currentItem!=0)
				{
					GetItemRect(&r,menuRect,currentItem-1);
					InsetRect(&r,1,1);
					PenMode(srcXor);
					FrameRect(&r);
					PenNormal();
				}
				if (*whichItem!=0)
				{
					GetItemRect(&r,menuRect,*whichItem-1);
					InsetRect(&r,1,1);
					PenMode(srcXor);
					FrameRect(&r);
					PenNormal();
				}
				*whichItem=currentItem;
			}
			break;
		#endif
		
		case kMenuSizeMsg:
			/* メニューのサイズを設定 */
			#if TARGET_API_MAC_CARBON
			SetMenuWidth(theMenu,kRows*kItemWidth);
			SetMenuHeight(theMenu,kColumns*kItemHeight);
			#else
			(**theMenu).menuWidth=kRows*kItemWidth;
			(**theMenu).menuHeight=kColumns*kItemHeight;
			#endif
			break;
		
		case kMenuPopUpMsg:
			/* ポップアップのRectを計算 */
			/* theMenu		メニューへのハンドル
			   whichItem	最初に選ばれているアイテム
			   hitPt		ポップアップメニューの左上
			   menuRect		ポップアップのRectを入れて返す */
			/* なぜかhitPtのhとvは入れ代わっている */
			SetRect(menuRect,hitPt.v,hitPt.h,hitPt.v+kRows*kItemWidth,hitPt.h+kColumns*kItemHeight);
			break;
		
		case kMenuThemeSavvyMsg:
			*whichItem=kThemeSavvyMenuResponse;
			break;
	}
}

static void GetItemRect(Rect *r,Rect *menuRect,short item)
{
	short	row=item/kColumns,column=item%kColumns;
	
	SetRect(r,menuRect->left+column*kItemWidth,menuRect->top+row*kItemHeight,
			menuRect->left+(column+1)*kItemWidth,menuRect->top+(row+1)*kItemHeight);
}

static void DrawItem(Rect *r,short i,short j)
{
	Rect	penRect;
	short	cx=(r->left+r->right)/2,cy=(r->top+r->bottom)/2;
	
	SetRect(&penRect,cx-(j+1)/2,cy-(i+1)/2,cx+(j+2)/2,cy+(i+2)/2);
	PaintRect(&penRect);
}
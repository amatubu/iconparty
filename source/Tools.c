/* ------------------------------------------------------------ */
/*  Tools.c                                                     */
/*     �c�[������                                               */
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


/* �c�[���I�� */
void ToolSelect(short tool)
{
	GrafPtr		port;
	WindowPtr	theWindow;
	PicHandle	pic;
	Rect		r;
	
	theWindow=MyFrontNonFloatingWindow();
	
	if (tool==gSelectedTool) return;
	
	if (tool>nTools || tool<0) return;
	
	/* �I���c�[���ȊO��I�񂾂Ƃ��A�ǂ������I������Ă���ΌŒ肷�� */
	/* �I��̈�ł̃}�X�L���O���L���̂Ƃ��́A�Œ肵�Ȃ� */
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
	
	/* �I�΂ꂽ�c�[�����n�C���C�g������ */
	pic=GetPicture(kToolPalettePictureResID);
	GetWindowPortBounds(gToolPalette,&r);
	DrawPicture(pic,&r);
	DarkenRect(&gToolRect);
	
	SetPort(port);
}

/* �c�[���p���b�g�̃_�u���N���b�N */
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
			/* ���M�c�[�����_�u���N���b�N���y���T�C�Y�̕ύX */
			ChangePenSize();
			break;
		
		case kEraserTool:
			/* �����S���c�[�����_�u���N���b�N���S��ʏ��� */
			if (theWindow==nil) return;
			
			windowKind=GetExtWindowKind(theWindow);
			if (windowKind!=kWindowTypePaintWindow) return;
			
			eWinRec=GetPaintWinRec(theWindow);
			isBackMode=(eWinRec->backgroundGWorld == nil);
			GoOffPort(theWindow);
			
			/* ��������͈� */
			if (EmptyRgn(eWinRec->eSelectedRgn))
				effectRect=eWinRec->iconSize;
			else
				effectRect=eWinRec->selectedRect;
			
			/* ���� */
			SetGWorld(eWinRec->editDataPtr,0);
			LockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
			if (isBackMode)  /* �w�i���Ȃ��ꍇ�͔w�i�F�œh�� */
				RGBBackColor(&gBackColor.rgb);
			EraseRect(&effectRect);
			BackColor(whiteColor);
			UnlockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
			
			SetGWorld(eWinRec->currentMask,0);
			LockPixels(GetGWorldPixMap(eWinRec->currentMask));
			if (!isBackMode || gBackColor.isTransparent)  /* �w�i������A���邢�͔w�i�F�������̎��̓}�X�N������ */
				EraseRgn(eWinRec->selectionPos);
			else /* ����ȊO�̏ꍇ�͑I��͈͑S�̂��}�X�N�� */
				PaintRgn(eWinRec->selectionPos);
			CopyRgn(eWinRec->selectionPos,eWinRec->updateRgn);
			
			SetGWorld(cPort,cDevice);
			DispOffPort(theWindow);
			
			UpdateTransparentMenu();
			SetUndoMode(umPaint);
			if (gPaletteCheck == kPaletteCheckUsed) /* �p���b�g�̃`�F�b�N���A�b�v�f�[�g */
				UpdatePaletteCheck();
			break;
		
		case kMarqueeTool:
			/* �I���c�[�����_�u���N���b�N���S��ʑI�� */
			if (theWindow==nil) return;
			
			windowKind=GetExtWindowKind(theWindow);
			if (windowKind != kWindowTypePaintWindow) return;
			
			eWinRec=GetPaintWinRec(theWindow);
			if (eWinRec->isSelected) /* �I��͈͂�����ΌŒ肷�� */
				FixSelection(theWindow);
			
			GoOffPort(theWindow);
			
			selectRgn=NewRgn();
			RectRgn(selectRgn,&eWinRec->iconSize);
			
			DoSelectMain(theWindow,selectRgn);
			break;
		
		case kSpoitTool:
			/* �X�|�C�g�c�[�����_�u���N���b�N����ʂ̔C�ӂ̏ꏊ����F���Ƃ� */
			GetDesktopColor();
			break;
	}
}

/* �O���b�h�̕ύX */
void ChangeGrid(short item)
{
	#pragma unused(item)
}

/* �y���T�C�Y�̕ύX */
void ChangePenSize(void)
{
	short	prevPenHeight = gPenHeight,
			prevPenWidth = gPenWidth;
	
	if (ChangePESize(&gPenWidth,&gPenHeight))
	{
		/* ���j���[�̍X�V */
		MenuHandle	menu;
		
		menu = GetMenuHandle(mPenSize);
		CheckMenuItem(menu,(prevPenHeight-1)*4+prevPenWidth,false);
		CheckMenuItem(menu,(gPenHeight-1)*4+gPenWidth,true);
		
		/* ���E�B���h�E�̍ĕ`�� */
		RedrawInfo();
		
		/* �c�[���̑I�� */
		ToolSelect(kPencilTool);
	}
}

/* �����S���̃T�C�Y�̕ύX */
void ChangeEraserSize(void)
{
	short	prevEraserHeight = gEraserHeight,
			prevEraserWidth = gEraserWidth;
	
	if (ChangePESize(&gEraserWidth,&gEraserHeight))
	{
		/* ���j���[�̍X�V */
		MenuHandle	menu;
		
		menu = GetMenuHandle(mEraserSize);
		CheckMenuItem(menu,(prevEraserHeight-1)*4+prevEraserWidth,false);
		CheckMenuItem(menu,(gEraserHeight-1)*4+gEraserWidth,true);
		
		/* ���E�B���h�E�̍ĕ`�� */
		RedrawInfo();
		
		/* �c�[���̑I�� */
		ToolSelect(kEraserTool);
	}
}

/* �y���^�����S���̃T�C�Y�̕ύX���C�����[�`�� */
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

/* �����x�̐ݒ� */
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

/* �����x�̕ύX */
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

/* �I��F�̕ύX */
void ChangeColor(RGBColor *newColor,Boolean isTransparent)
{
	GrafPtr	port;
	Rect	r;
	
	if (EqualColor(newColor,&gCurrentColor.rgb) && isTransparent == gCurrentColor.isTransparent)
		return; /* ���݂̑I��F�Ɠ����ꍇ�͂Ȃɂ����Ȃ� */
	
	/* �J���[�p���b�g�̃n�C���C�g�X�V */
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
	
	/* �u�����h�p���b�g�X�V */
	DrawBlend();
	SetPortWindowPort(gBlendPalette);
	GetWindowPortBounds(gBlendPalette,&r);
	CopyBits(GetPortBitMapForCopyBits(gBlendPalettePtr),GetPortBitMapForCopyBits(GetWindowPort(gBlendPalette)),
		&r,&r,srcCopy,nil);
	
	/* �p�^�[���p���b�g�X�V */
	MyInvalWindowPortBounds(PatternPalette);
	
	SetPort(port);
}

/* �w�i�F�̕ύX */
void ChangeBackColor(RGBColor *newColor,Boolean isTransparent)
{
	GrafPtr	port;
	Rect	r;
	
	if (EqualColor(newColor,&gBackColor.rgb) && isTransparent == gBackColor.isTransparent) return;
	
	gBackColor.rgb=*newColor;
	gBackColor.isTransparent=isTransparent;
	
	GetPort(&port);
	/* �u�����h�p���b�g�X�V */
	DrawBlend();
	SetPortWindowPort(gBlendPalette);
	GetWindowPortBounds(gBlendPalette,&r);
	CopyBits(GetPortBitMapForCopyBits(gBlendPalettePtr),GetPortBitMapForCopyBits(GetWindowPort(gBlendPalette)),
		&r,&r,srcCopy,nil);
	
	/* �p�^�[���p���b�g�X�V */
	MyInvalWindowPortBounds(PatternPalette);
	
	SetPort(port);
}

/* ��ʏォ��F�𓾂� 1.0b5�ǉ� */
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
	
	/* �}�E�X�N���b�N��ǂݔ�΂� */
	while (Button()) ;
	
	GetPort(&port);
	SetPort(LMGetWMgrPort());
	
	while (!Button())
	{
		/* �L�[�{�[�h�̏�Ԃ��Ď����Acmd+.�Ȃ�I��� */
		GetKeys(theKeys);
		if (BitTst(theKeys,48) && BitTst(theKeys,40))
		{
			result=false;
			break;
		}
		
		/* �}�E�X�ʒu�̐F���Ƃ�A�u�����h�p���b�g��ŕ\�� */
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
		/* �F��ݒ� */
		ChangeColor(&newColor,false);
	}
	else
	{
		/* �u�����h�p���b�g�����ɖ߂� */
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
	
	/* �}�E�X�N���b�N��ǂݔ�΂� */
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
		/* �L�[�{�[�h�̏�Ԃ��Ď����Acmd+.�Ȃ�I��� */
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
		
		/* �}�E�X�ʒu�̐F���Ƃ�A�u�����h�p���b�g��ŕ\�� */
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
	
	EndFullScreen(oldState,nil);
	
	SetPort(port);
	HideMenuBar();
	ShowMenuBar();
	
	gCurrentColor=prevColor;
	if (result)
	{
		/* �F��ݒ� */
		ChangeColor(&newColor,false);
	}
	else
	{
		/* �u�����h�p���b�g�����ɖ߂� */
		DrawBlend();
		SetPortWindowPort(gBlendPalette);
		MyInvalWindowPortBounds(gBlendPalette);
		SetPort(port);
	}
	
	#endif
}

/* �F��256�F�ɕ␳�i�����������j */
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
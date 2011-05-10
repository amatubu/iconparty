/* ------------------------------------------------------------ */
/*  EffectRoutines.c                                            */
/*     ���ʏ����Ȃ�                                             */
/*                                                              */
/*                 2001.11.17 - 2001.11.17  naoki iimura       	*/
/* ------------------------------------------------------------ */

/* includes */
#ifdef __APPLE_CC__
#include	<Carbon/Carbon.h>
#endif

#include	"Globals.h"
#include	"UsefulRoutines.h"
#include	"EffectRoutines.h"
#include	"IconParty.h"
#include	"MenuRoutines.h"
#include	"WindowRoutines.h"
#include	"PaintRoutines.h"
#include	"EditRoutines.h"
#include	"ToolRoutines.h"
#include	"Preferences.h"
#include	"PreCarbonSupport.h"
#include	"UpdateCursor.h"


#pragma options align=mac68k

/* ��] */
static short	gAngle=90;

/* �_�` */
typedef struct DotPictureRec {
	short	dotPicBack;
	Boolean	dotPicLight;
	short	dotVolume;
} DotPictureRec;

static DotPictureRec	dotPicturePrefs={0,false,0};

/* �� */
typedef struct WindRec {
	short	direction;
	short	power;
} WindRec;

static WindRec	windPrefs={0,0};

/* �������͗p */
typedef struct TypeStringRec {
	short	fontID;
	short	size;
	short	face;
	Str255	string;
	Boolean	antialias;
	short	menuItem;
} TypeStringRec;

static TypeStringRec	typeStringPrefs={0,12,normal,"\ptext",false,1};

/* ����� */
enum {
	kShapeEdge=1,
	kShapeOval,
	kShapeTriangle,
	kShapeRTriangle,
	kShapeLozenge,		/* �Ђ��` */
	kShapeDiagonal1,	/* �Ίp�� */
	kShapeDiagonal2,
};

typedef struct EdgePrefsRec
{
	short	pixels;
	short	transparency;
	short	kind;
} EdgePrefsRec;

static EdgePrefsRec	edgePrefs={1,1,kShapeEdge};

#pragma options align=reset

static void	DoEdgingMain(PaintWinRec *eWinRec,EdgePrefsRec *prefs);

static void	FlipVerticalRgn(RgnHandle rgn,Rect *r);
static void	FlipHorizontalRgn(RgnHandle rgn,Rect *r);

/* ���ʂȂǂ̐ݒ� */
static short	ConfigureDotPicture(void);
static short	ConfigureRotate(void);
static short	ConfigureWind(void);
static short	ConfigureEdging(EdgePrefsRec *newPrefs);
static short	ConfigureTypeString(TypeStringRec *prefs);
static void	TypeStringTEUpdate(TypeStringRec *prefs,TEHandle hTE);
static pascal Boolean	MyDispTEFilter(DialogPtr theDialog,EventRecord *theEvent,short *theItemHit);

static void	GetSinCos(short angle,long *sine,long *cosine);


#define	EFFECTERR_RESID	4006 /* = PAINTERR_RESID */
#define	EFFECTERR1	1
#define	EFFECTERR2	2


/* �I��͈͂��Â����� */
void DoDarken(WindowPtr theWindow)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	RgnHandle	selectedRgn=NewRgn();
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	GetGWorld(&cPort,&cDevice);
	ResetRuler();
	GoOffPort(theWindow);
	
	if (eWinRec->isSelected) /* �I��͈͂��� */
	{
		CopyRgn(eWinRec->selectionPos,selectedRgn);
		OffsetRgn(selectedRgn,-eWinRec->selectionOffset.h,-eWinRec->selectionOffset.v);
	}
	else
		RectRgn(selectedRgn,&eWinRec->iconSize);
	
	SetGWorld(eWinRec->editDataPtr,0);
	PenMode(subPin);
	OpColor(&rgbBlackColor);
	RGBForeColor(&rgbLBColor);
	
	LockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
	PaintRgn(selectedRgn);
	UnlockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
	
	ForeColor(blackColor);
	PenMode(srcCopy);
	
	/* �}�X�N���� */
	SetGWorld(eWinRec->currentMask,0);
	LockPixels(GetGWorldPixMap(eWinRec->currentMask));
	PaintRgn(selectedRgn);
	UnlockPixels(GetGWorldPixMap(eWinRec->currentMask));
	DisposeRgn(selectedRgn);
	
	CopyRgn(eWinRec->selectionPos,eWinRec->updateRgn);
	
	SetGWorld(cPort,cDevice);
	DispOffPort(theWindow);
	
	gUsedCount.darkenNum++;
	UpdateTransparentMenu();
	SetUndoMode(umEffect);
	if (gPaletteCheck == kPaletteCheckUsed) /* �p���b�g�̃`�F�b�N���A�b�v�f�[�g */
		UpdatePaletteCheck();
}

/* �I��͈͂𖾂邭���� */
void DoLighten(WindowPtr theWindow)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	GetGWorld(&cPort,&cDevice);
	ResetRuler();
	GoOffPort(theWindow);
	
	SetGWorld(eWinRec->editDataPtr,0);
	PenMode(addPin);
	OpColor(&rgbWhiteColor);
	RGBForeColor(&rgbLBColor);
	
	LockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
	PaintRect(&eWinRec->selectedRect);
	UnlockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
	
	CopyRgn(eWinRec->selectionPos,eWinRec->updateRgn);
	
	ForeColor(blackColor);
	PenMode(srcCopy);
	
	SetGWorld(cPort,cDevice);
	DispOffPort(theWindow);
	
	gUsedCount.lightenNum++;
	UpdateTransparentMenu();
	SetUndoMode(umEffect);
	if (gPaletteCheck == kPaletteCheckUsed) /* �p���b�g�̃`�F�b�N���A�b�v�f�[�g */
		UpdatePaletteCheck();
}

/* �I��͈͂�h��Ԃ� */
void DoFill(WindowPtr theWindow)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	Pattern		pat;
	RgnHandle	effectRgn=NewRgn();
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	GetGWorld(&cPort,&cDevice);
	GoOffPort(theWindow);
	
	if (eWinRec->isSelected)
	{
		CopyRgn(eWinRec->selectionPos,effectRgn);
		OffsetRgn(effectRgn,-eWinRec->selectionOffset.h,-eWinRec->selectionOffset.v);
	}
	else
		RectRgn(effectRgn,&eWinRec->iconSize);
	
	SetGWorld(eWinRec->editDataPtr,0);
	PenMode(gBlendMode);
	OpColor(&gBlendRatio);
	RGBForeColor(&gCurrentColor.rgb);
	RGBBackColor(&gBackColor.rgb);
	GetIndPattern(&pat,(gOtherPrefs.useHyperCardPalette ? 201 : 200),gPatternNo);
	
	LockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
	PenPat(&pat);
	PaintRgn(effectRgn); /* FillRect���ƕs�����x�����f����Ȃ� */
	UnlockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
	
	PenNormal();
	PenMode(srcCopy);
	ForeColor(blackColor);
	BackColor(whiteColor);
	
	/* �}�X�N���� */
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
		
		if (isOpaque || isTransparent) /* ���S�ɓ��� or �s���� */
		{
			SetGWorld(eWinRec->currentMask,0);
			LockPixels(GetGWorldPixMap(eWinRec->currentMask));
			if (isTransparent)
				EraseRect(&eWinRec->selectedRect);
			else
				PaintRgn(effectRgn);
			UnlockPixels(GetGWorldPixMap(eWinRec->currentMask));
		}
		else /* �p�^�[�����֌W���� */
		{
			SetGWorld(eWinRec->currentMask,0);
			LockPixels(GetGWorldPixMap(eWinRec->currentMask));
			
			ForeColor((gCurrentColor.isTransparent ? whiteColor : blackColor));
			BackColor((gBackColor.isTransparent ? whiteColor : blackColor));
			
			FillRgn(effectRgn,&pat);
			
			ForeColor(blackColor);
			BackColor(whiteColor);
			UnlockPixels(GetGWorldPixMap(eWinRec->currentMask));
		}
	}
	DisposeRgn(effectRgn);
	
	CopyRgn(eWinRec->selectionPos,eWinRec->updateRgn);
	
	SetGWorld(cPort,cDevice);
	DispOffPort(theWindow);
	
	gUsedCount.fillNum++;
	UpdateTransparentMenu();
	SetUndoMode(umEffect);
	if (gPaletteCheck == kPaletteCheckUsed) /* �p���b�g�̃`�F�b�N���A�b�v�f�[�g */
		UpdatePaletteCheck();
}

/* �I��͈͂ɃA���`�G�C���A�X������������ */
void DoAntialias(WindowPtr theWindow)
{
	Rect	dstRect;
	RGBColor	antiRatio[4]={{0,0,0},{0x8000,0x8000,0x8000},{0x5555,0x5555,0x5555},{0x4000,0x4000,0x4000}};
	short	mode[4]={srcCopy,blend,blend,blend};
	short	dh[4]={1,-1,0,0},dv[4]={0,0,-1,1};
	short	i;
		
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	Rect		effectRect=eWinRec->selectedRect;
	Rect		portRect;
	GWorldPtr	tempGWorld;
	OSErr		err;
	GWorldPtr	maskGWorld;
	Rect		tempRect;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	GetGWorld(&cPort,&cDevice);
	ResetRuler();
	
	/* �����ł�32768�F�Ōv�Z���� */
	GetPortBounds(eWinRec->editDataPtr,&portRect);
	err=NewGWorld(&tempGWorld,24,&portRect,0,0,useTempMem);
	
	if (err!=noErr)
	{
		ErrorAlertFromResource(EFFECTERR_RESID,EFFECTERR1);
		return;
	}
	
	GoOffPort(theWindow);
	
	/* �㉺���E�ɂ��炵�Ȃ���K���Ȋ����ł܂����킹�� */
	SetGWorld(tempGWorld,0);
	
	LockPixels(GetGWorldPixMap(tempGWorld));
	EraseRect(&portRect);
	
	for (i=0; i<4; i++)
	{
		OpColor(&antiRatio[i]);
		dstRect=effectRect;
		OffsetRect(&dstRect,dh[i],dv[i]);
		
		CopyBits(GetPortBitMapForCopyBits(eWinRec->tempDataPtr),
			GetPortBitMapForCopyBits(tempGWorld),
			&effectRect,&dstRect,mode[i],0);
	}
	
	OpColor(&antiRatio[1]);
	
	CopyBits(GetPortBitMapForCopyBits(eWinRec->tempDataPtr),
		GetPortBitMapForCopyBits(tempGWorld),
		&effectRect,&effectRect,blend,0);
	
	SetGWorld(eWinRec->editDataPtr,0);
	
	/* �Ō��256�F�ɖ߂� */
	CopyBits(GetPortBitMapForCopyBits(tempGWorld),
		GetPortBitMapForCopyBits(eWinRec->editDataPtr),
		&effectRect,&effectRect,srcCopy,0);
	
	UnlockPixels(GetGWorldPixMap(tempGWorld));
	DisposeGWorld(tempGWorld);
	
	/* �}�X�N���� */
	err=CreateMask2(eWinRec->editDataPtr,&maskGWorld);
	if (err!=noErr)
	{
		SysBeep(0);
		MySetGWorld(currentMask);
		MyLockPixels(currentMask);
		GetPortBounds(eWinRec->currentMask,&tempRect);
		PaintRect(&tempRect);
		MyUnlockPixels(currentMask);
		SetGWorld(cPort,cDevice);
	}
	else
	{
		MySetGWorld(currentMask);
		CopyBits(GetPortBitMapForCopyBits(maskGWorld),
			GetPortBitMapForCopyBits(eWinRec->currentMask),
			&effectRect,&effectRect,srcOr,nil);
		SetGWorld(cPort,cDevice);
		
		DisposeGWorld(maskGWorld);
	}
	
	/* �I��͈͑S�̂��X�V */
	CopyRgn(eWinRec->selectionPos,eWinRec->updateRgn);
	SetGWorld(cPort,cDevice);
	DispOffPort(theWindow);
	
	gUsedCount.antialiasNum++;
	UpdateTransparentMenu();
	SetUndoMode(umEffect);
	if (gPaletteCheck == kPaletteCheckUsed) /* �p���b�g�̃`�F�b�N���A�b�v�f�[�g */
		UpdatePaletteCheck();
}

/* �_�`�p�̒萔�^�ϐ� */
#define		dDotPicture	130
enum {
	bgNone=3,
	bgCurrent,
	bgSource,
	bgLighten,
	dotMany,
	dotNormal,
	dotFew,
	dotFewer
};

/* �_�`�i�H�j */
void DoDotPicture(WindowPtr theWindow)
{
	RGBColor	dotOpColor[]=  {{0x8000,0x8000,0x8000},
								{0x4000,0x4000,0x4000}};
	short	dotX,dotY;
	short	i;
	Rect	selectRect;
	short	left,top;
	short	right,bottom;
	
	GWorldPtr	randomGWorld;
	PixMapHandle	pmh;
	Ptr		baseAddr;
	long	rowBytes;
	Ptr		addrX;
	long	outBits;
	short	bitCount;
	short	dx;
	
	OSErr	err;
	Rect	dstRect;
	
	short	dh[]={-1,0,1,-1,1,-1,0,1};
	short	dv[]={-1,-1,-1,0,0,1,1,1};
	short	colorSet[]={1,0,1,0,0,1,0,1};
	
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	Rect		effectRect=eWinRec->selectedRect,tempRect;
	GWorldPtr	maskGWorld;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	/* �ݒ���Ăяo�� */
	if (ConfigureDotPicture()<0) return;
	
	/* �v�Z�p�̕ϐ��̏����� */
	selectRect=effectRect;
	left=selectRect.left;
	top=selectRect.top;
	right=selectRect.right;
	bottom=selectRect.bottom;
	
	/* �����_���h�b�g�̃p�^�[���쐬 */
	err=NewGWorld(&randomGWorld,1,&effectRect,0,0,useTempMem);
	if (err!=noErr)
	{
		ErrorAlertFromResource(EFFECTERR_RESID,EFFECTERR1);
		return;
	}
	GetGWorld(&cPort,&cDevice);
	SetGWorld(randomGWorld,0);
	pmh=GetGWorldPixMap(randomGWorld);
	LockPixels(pmh);
	EraseRect(&effectRect);
	SetGWorld(cPort,cDevice);
	
	baseAddr=MyGetPixBaseAddr(pmh);
	rowBytes=MyGetPixRowBytes(pmh) & 0x3fff;
	
	for (dotY=top; dotY<bottom; dotY++) /* �I��͈͂̏ォ�牺�܂Ń��[�v���� */
	{
		dotX=right+(dotY%3)*3+(Random()%3); /* �E������_��ł��Ă��� */
		while (dotX>=right) dotX-=(dotPicturePrefs.dotVolume<<1)+4+Random()%3;
		
		/* �_�̈ʒu�̃A�h���X���v�Z */
		addrX=baseAddr+(dotY-top)*rowBytes+((dotX-left) >> 3)+1;
		bitCount=7-((dotX-left) & 0x07);
		outBits=1L<<bitCount;
		
		while (true)
		{
			dx=(dotPicturePrefs.dotVolume<<1)+4+Random()%3;
			dotX-=dx;
			if (dotX<left) break; /* ���͂��ɒB����܂ŌJ��Ԃ� */
			bitCount+=dx;
			outBits+=1L<<bitCount;
			while (bitCount>=8)
			{
				*--addrX=outBits&0xff; /* �W�r�b�g�i�P�o�C�g�j�P�ʂŏ�������ł��� */
				outBits>>=8;
				bitCount-=8;
			}
		}
		if (bitCount>0) *--addrX=outBits&0xff; /* �c�����r�b�g���������� */
		if (addrX<baseAddr)
		{
			SysBeep(0);
		}
	}
	
	/* ���C�����[�`�� */
	GoOffPort(theWindow);
	
	SetGWorld(eWinRec->editDataPtr,0);
	LockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
	
	/* �w�i�̕`�� */
	switch (dotPicturePrefs.dotPicBack+bgNone)
	{
		case bgNone:
			/* �w�i�Ȃ� */
			EraseRect(&effectRect);
			break;
		
		case bgCurrent:
			/* ���݂̑I��F */
			RGBForeColor(&gCurrentColor.rgb);
			PaintRect(&effectRect);
			ForeColor(blackColor);
			break;
		
		case bgSource:
			/* ���݂̊G */
			if (dotPicturePrefs.dotPicLight) /* ��������Ȃ� */
			{
				/* 50���̊����Ŕ��œh�� */
				PenMode(blend);
				OpColor(&rgbGrayColor);
				ForeColor(whiteColor);
				PaintRect(&effectRect);
				ForeColor(blackColor);
				PenMode(srcCopy);
			}
			break;
	}
	
	UnlockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
	
	/* �_�`����������ł����i�����_���h�b�g�Ń}�X�N���Ȃ��炸�炵�ĕ`��j */
	SafeCopyDeepMask(GetPortBitMapForCopyBits(eWinRec->tempDataPtr),
		GetPortBitMapForCopyBits(randomGWorld),
		GetPortBitMapForCopyBits(eWinRec->editDataPtr),
		&effectRect,&effectRect,&effectRect,srcCopy,nil);
	
	for (i=0; i<8; i++)
	{
		dstRect=effectRect;
		OffsetRect(&dstRect,dh[i],dv[i]);
		
		OpColor(&dotOpColor[colorSet[i]]);
		SafeCopyDeepMask(GetPortBitMapForCopyBits(eWinRec->tempDataPtr),
			GetPortBitMapForCopyBits(randomGWorld),
			GetPortBitMapForCopyBits(eWinRec->editDataPtr),
			&effectRect,&effectRect,&dstRect,blend,nil);
	}
	
	DisposeGWorld(randomGWorld);
	
	/* �}�X�N���� */
	err=CreateMask2(eWinRec->editDataPtr,&maskGWorld);
	if (err!=noErr)
	{
		SysBeep(0);
		MySetGWorld(currentMask);
		MyLockPixels(currentMask);
		GetPortBounds(eWinRec->currentMask,&tempRect);
		PaintRect(&tempRect);
		MyUnlockPixels(currentMask);
		SetGWorld(cPort,cDevice);
	}
	else
	{
		MySetGWorld(currentMask);
		CopyBits(GetPortBitMapForCopyBits(maskGWorld),
			GetPortBitMapForCopyBits(eWinRec->currentMask),
			&effectRect,&effectRect,srcOr,nil);
		SetGWorld(cPort,cDevice);
		
		DisposeGWorld(maskGWorld);
	}
	
	CopyRgn(eWinRec->selectionPos,eWinRec->updateRgn);
	
	DispOffPort(theWindow);
	
	gUsedCount.dotPictureNum++;
	UpdateTransparentMenu();
	SetUndoMode(umEffect);
	if (gPaletteCheck == kPaletteCheckUsed) /* �p���b�g�̃`�F�b�N���A�b�v�f�[�g */
		UpdatePaletteCheck();
}

/* ���̂��߂̒萔�^�ϐ� */
#define	dWind	139

enum{
	dRight=3,
	dLeft,
	pWeak,
	pStrong,
};

/* ���i�������j */
void DoWind(WindowPtr theWindow)
{
	RGBColor	rgbDarkGrayColor={0x5000,0x5000,0x5000};
	RgnHandle	windRgn;
	
	Rect	selectRect,tempRect;
	short	top,left,bottom,right;
	Rect	windRect;
	Rect	dstRect;
	
	short	i;
	short	dotY;
	short	wd[2]={1,-1};
	
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	Rect		effectRect=eWinRec->selectedRect;
	OSErr		err;
	GWorldPtr	maskGWorld;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	/* �ݒ�ύX���[�`�� */
	if (ConfigureWind()<0) return;
	
	GetGWorld(&cPort,&cDevice);
	
	selectRect=effectRect;
	top=selectRect.top;
	left=selectRect.left;
	bottom=selectRect.bottom;
	right=selectRect.right;
	
	windRgn=NewRgn();
	OpenRgn();
	PenSize(1,1);
	
	for (dotY=top; dotY<bottom; dotY++)
	{
		if (Random()%4==0) /* �K���ȂƂ���ɍ��������Ђ��Ă��� */
		{
			SetRect(&windRect,left,dotY,right,dotY+1);
			FrameRect(&windRect);
			dotY+=2;
		}
	}
	
	CloseRgn(windRgn);
	
	GoOffPort(theWindow);
	
	/* ���ɂ��炵�Ȃ���d�˂ĕ`�� */
	SetGWorld(eWinRec->editDataPtr,0);
	dstRect=effectRect;
	OffsetRect(&dstRect,(4+windPrefs.power)*wd[windPrefs.direction],0);
	OpColor(&rgbDarkGrayColor);
	CopyBits(GetPortBitMapForCopyBits(eWinRec->tempDataPtr),
		GetPortBitMapForCopyBits(eWinRec->editDataPtr),
		&effectRect,&dstRect,blend,windRgn);
	
	for (i=0; i<4+windPrefs.power; i++)
	{
		OffsetRect(&dstRect,-wd[windPrefs.direction],0);
		CopyBits(GetPortBitMapForCopyBits(eWinRec->tempDataPtr),
			GetPortBitMapForCopyBits(eWinRec->editDataPtr),
			&effectRect,&dstRect,blend,windRgn);
	}
	
	DisposeRgn(windRgn);
	
	/* �}�X�N���� */
	err=CreateMask2(eWinRec->editDataPtr,&maskGWorld);
	if (err!=noErr)
	{
		SysBeep(0);
		MySetGWorld(currentMask);
		MyLockPixels(currentMask);
		GetPortBounds(eWinRec->currentMask,&tempRect);
		PaintRect(&tempRect);
		MyUnlockPixels(currentMask);
		SetGWorld(cPort,cDevice);
	}
	else
	{
		MySetGWorld(currentMask);
		CopyBits(GetPortBitMapForCopyBits(maskGWorld),
			GetPortBitMapForCopyBits(eWinRec->currentMask),
			&effectRect,&effectRect,srcOr,nil);
		SetGWorld(cPort,cDevice);
		
		DisposeGWorld(maskGWorld);
	}
	
	CopyRgn(eWinRec->selectionPos,eWinRec->updateRgn);
	
	DispOffPort(theWindow);
	
	gUsedCount.windNum++;
	UpdateTransparentMenu();
	SetUndoMode(umEffect);
	if (gPaletteCheck == kPaletteCheckUsed) /* �p���b�g�̃`�F�b�N���A�b�v�f�[�g */
		UpdatePaletteCheck();
}

/* ��] */
void DoRotate(WindowPtr theWindow)
{
	long	sine; /* �p�x�̃T�C���i32768�{���Ă���j */
	long	cosine; /* �p�x�̃R�T�C���i����j */
	
	Rect	selectRect;
	short	left;
	short	right;
	short	top;
	short	bottom;
	short	centerX; /* �I��͈͂̒��S�_�̍��W�̂Q�{ */
	short	centerY;
	PixMapHandle	pmh;
	long	editRowBytes,tempRowBytes;
	Ptr		editBaseAddr,tempBaseAddr;
	
	short	x,y;
	long	px,py;
	short	prevRealX,prevRealY;
	Point	prevRealPt;
	
	OSErr	err;
	RgnHandle	selectedRgn;
	Rect	newSelectRect;
	Rect	tempRect;
	GWorldPtr	rotateGWorld,maskGWorld;
	long	rotateRowBytes,maskRowBytes;
	Ptr		rotateBaseAddr,maskBaseAddr;
	
	short	editTop,tempTop,editLeft,tempLeft;
	short	newAngle;
	
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	Boolean	isBackMode=(eWinRec->backgroundGWorld == nil && !eWinRec->isBackTransparent);
	
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	/* �ݒ胋�[�`���̌Ăяo�� */
	newAngle=ConfigureRotate();
	if (newAngle<=0) return;
	
	GetGWorld(&cPort,&cDevice);
	
	if (eWinRec->undoMode==umRotate)
	{
		newAngle+=eWinRec->prevRotate;
		if (newAngle>360) newAngle-=360;
		
		DoUndo();
	}
	
	eWinRec->prevRotate=newAngle;
	
	GetSinCos(newAngle,&sine,&cosine); /* �p�x�ɑ΂���T�C���^�R�T�C�����v�Z */
	
	GoOffPort(theWindow);
	
	/* �v�Z�p�̕ϐ��̏����� */
	selectRect=eWinRec->selectedRect;
	if (!eWinRec->isSelected) /* �I�𒆂łȂ� */
	{
		SetGWorld(eWinRec->selectedDataPtr,0);
		LockPixels(GetGWorldPixMap(eWinRec->selectedDataPtr));
		if (isBackMode) /* �w�i���Ȃ��ꍇ�͔w�i�F�œh�� */
			RGBBackColor(&gBackColor.rgb);
		EraseRect(&selectRect);
		BackColor(whiteColor);
		UnlockPixels(GetGWorldPixMap(eWinRec->selectedDataPtr));
	}
	
	left=selectRect.left*2;
	right=selectRect.right*2;
	top=selectRect.top*2;
	bottom=selectRect.bottom*2;
	centerX=(left+right)/2;
	centerY=(top+bottom)/2;
	left-=centerX;
	right-=centerX;
	top-=centerY;
	bottom-=centerY;
	
	/* ��]��̗̈�ƂȂ�Rect�����߂� */
	if (newAngle < 90)
	{
		/* �����̓_�����[�ɂȂ� */
		newSelectRect.left=((left*cosine-bottom*sine+16384)/32768+centerX)/2;
		/* ����̓_����[�ɂȂ� */
		newSelectRect.top=((left*sine+top*cosine+16384)/32768+centerY)/2;
		/* �E��̓_���E�[�ɂȂ� */
		newSelectRect.right=((right*cosine-top*sine+16384)/32768+centerX)/2;
		/* �E���̓_�����[�ɂȂ� */
		newSelectRect.bottom=((right*sine+bottom*cosine+16384)/32768+centerY)/2;
	}
	else if (newAngle < 180)
	{
		/* �E���̓_�����[�ɂȂ� */
		newSelectRect.left=((right*cosine-bottom*sine+16384)/32768+centerX)/2;
		/* �����̓_����[�ɂȂ� */
		newSelectRect.top=((left*sine+bottom*cosine+16384)/32768+centerY)/2;
		/* ����̓_���E�[�ɂȂ� */
		newSelectRect.right=((left*cosine-top*sine+16384)/32768+centerX)/2;
		/* �E��̓_�����[�ɂȂ� */
		newSelectRect.bottom=((right*sine+top*cosine+16384)/32768+centerY)/2;
	}
	else if (newAngle < 270)
	{
		/* �E�オ���[ */
		newSelectRect.left=((right*cosine-top*sine+16384)/32768+centerX)/2;
		/* �E������[ */
		newSelectRect.top=((right*sine+bottom*cosine+16384)/32768+centerY)/2;
		/* �������E�[ */
		newSelectRect.right=((left*cosine-bottom*sine+16384)/32768+centerX)/2;
		/* ���オ���[ */
		newSelectRect.bottom=((left*sine+top*cosine+16384)/32768+centerY)/2;
	}
	else
	{
		/* ���オ���[ */
		newSelectRect.left=((left*cosine-top*sine+16384)/32768+centerX)/2;
		/* �E�オ��[ */
		newSelectRect.top=((right*sine+top*cosine+16384)/32768+centerY)/2;
		/* �E�����E�[ */
		newSelectRect.right=((right*cosine-bottom*sine+16384)/32768+centerX)/2;
		/* ���������[ */
		newSelectRect.bottom=((left*sine+bottom*cosine+16384)/32768+centerY)/2;
	}
	
	/* �O�̂��߁A1�h�b�g�L���Ă��� */
	InsetRect(&newSelectRect,-1,-1);
	
	/* �I��͈͂����݂̉摜���݂͂������ǂ����𒲂ׂ� */
	GetPortBounds(eWinRec->editDataPtr,&tempRect);
	SectRect(&tempRect,&newSelectRect,&tempRect);
	if (!EqualRect(&newSelectRect,&tempRect))
	{
		GWorldFlags	flags;
		
		flags=UpdateGWorld(&eWinRec->editDataPtr,eWinRec->iconDepth,&newSelectRect,0,0,0);
		if ((flags & gwFlagErr)!=0) /* �G���[ */
		{
			SetGWorld(cPort,cDevice);
			ErrorAlertFromResource(EFFECTERR_RESID,EFFECTERR1);
			
			SetGWorld(eWinRec->editDataPtr,0);
			CopyBits(GetPortBitMapForCopyBits(eWinRec->tempDataPtr),
				GetPortBitMapForCopyBits(eWinRec->editDataPtr),
				&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
			SetGWorld(cPort,cDevice);
			SetUndoMode(umCannot);
			return;
		}
		
		flags=UpdateGWorld(&eWinRec->currentMask,1,&newSelectRect,0,0,0);
		if ((flags & gwFlagErr)!=0) /* �G���[ */
		{
			flags=UpdateGWorld(&eWinRec->editDataPtr,eWinRec->iconDepth,&eWinRec->iconSize,0,0,0);
			
			SetGWorld(eWinRec->currentMask,0);
			CopyBits(GetPortBitMapForCopyBits(eWinRec->pCurrentMask),
				GetPortBitMapForCopyBits(eWinRec->currentMask),
				&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
			
			SetGWorld(cPort,cDevice);
			ErrorAlertFromResource(EFFECTERR_RESID,EFFECTERR1);
			
			SetGWorld(eWinRec->editDataPtr,0);
			CopyBits(GetPortBitMapForCopyBits(eWinRec->tempDataPtr),
				GetPortBitMapForCopyBits(eWinRec->editDataPtr),
				&eWinRec->iconSize,&eWinRec->iconSize,srcCopy,nil);
			SetGWorld(cPort,cDevice);
			SetUndoMode(umCannot);
			return;
		}
	}
	
	/* �摜��ł̑I��̈� */
	selectedRgn=NewRgn();
	CopyRgn(eWinRec->selectionPos,selectedRgn);
	OffsetRgn(selectedRgn,-eWinRec->selectionOffset.h,-eWinRec->selectionOffset.v);
	
	/* �V�����I��̈�𓾂邽�߂�GWorld */
	err=NewGWorld(&rotateGWorld,1,&newSelectRect,0,0,useTempMem);
	if (err!=noErr)
	{
		SetGWorld(cPort,cDevice);
		ErrorAlertFromResource(EFFECTERR_RESID,EFFECTERR1);
		
		SetUndoMode(umCannot);
		return;
	}
	pmh=GetGWorldPixMap(rotateGWorld);
	SetGWorld(rotateGWorld,0);
	LockPixels(pmh);
	EraseRect(&newSelectRect);
	rotateRowBytes=MyGetPixRowBytes(pmh) & 0x3fff;
	rotateBaseAddr=MyGetPixBaseAddr(pmh);
	
	/* �V�����}�X�N */
	maskGWorld=eWinRec->currentMask;
	pmh=GetGWorldPixMap(maskGWorld);
	SetGWorld(maskGWorld,0);
	LockPixels(pmh);
	EraseRect(&newSelectRect);
	maskRowBytes=MyGetPixRowBytes(pmh) & 0x3fff;
	maskBaseAddr=MyGetPixBaseAddr(pmh);
	
	pmh=GetGWorldPixMap(eWinRec->editDataPtr);
	SetGWorld(eWinRec->editDataPtr,0);
	LockPixels(pmh);
	EraseRect(&newSelectRect);
	editRowBytes=MyGetPixRowBytes(pmh) & 0x3fff;
	editBaseAddr=MyGetPixBaseAddr(pmh);
	
	pmh=GetGWorldPixMap(eWinRec->tempDataPtr);
	SetGWorld(eWinRec->tempDataPtr,0);
	LockPixels(pmh);
	tempRowBytes=MyGetPixRowBytes(pmh) & 0x3fff;
	tempBaseAddr=MyGetPixBaseAddr(pmh);
	
	/* �V�����̈�̂��߂ɐݒ肵�Ȃ��� */
	left=newSelectRect.left;
	top=newSelectRect.top;
	right=newSelectRect.right;
	bottom=newSelectRect.bottom;
	
	GetPortBounds(eWinRec->tempDataPtr,&tempRect);
	tempLeft=tempRect.left;
	tempTop=tempRect.top;
	GetPortBounds(eWinRec->editDataPtr,&tempRect);
	editLeft=tempRect.left;
	editTop=tempRect.top;
	
	for (y=top; y<bottom; y++)
	{
		/* �ꎟ�ϊ���p���ĉ�]�O�̓_�����߂� */
		px=(left*2-centerX+1)*cosine+(y*2-centerY+1)*sine;
		py=-(left*2-centerX+1)*sine+(y*2-centerY+1)*cosine;
		
		for (x=left; x<right; x++)
		{
			prevRealX=(((px+(px>0 ? 16384L : -16384L))/32768L)+centerX-1)/2;
			prevRealY=(((py+(py>0 ? 16384L : -16384L))/32768L)+centerY-1)/2;
			SetPt(&prevRealPt,prevRealX,prevRealY);
			
			if (PtInRgn(prevRealPt,selectedRgn)) /* ��]�O�̓_���I��͈͓��Ȃ�� */
			{
				switch (eWinRec->colorMode)
				{
					case kNormal8BitColorMode:
						*(editBaseAddr+(x-editLeft)+(y-editTop)*editRowBytes)=
							*(tempBaseAddr+(prevRealX-tempLeft)+
							(prevRealY-tempTop)*tempRowBytes);
						break;
					
					case k32BitColorMode:
						*(UInt32 *)(editBaseAddr+(x-editLeft)*4+(y-editTop)*editRowBytes)=
							*(UInt32 *)(tempBaseAddr+(prevRealX-tempLeft)*4+
							(prevRealY-tempTop)*tempRowBytes);
						break;
				}
				*(rotateBaseAddr+(x-left)/8+(y-top)*rotateRowBytes)|=
					1<<(7-(x-left)%8);
			}
			
			/* �}�X�N */
			if (RealPtInMask(prevRealPt,eWinRec->pCurrentMask))
				*(maskBaseAddr+(x-editLeft)/8+(y-editTop)*maskRowBytes)|=
					1<<(7-(x-editLeft)%8);
			
			/* ���̓_�ֈړ� */
			px+=cosine*2;
			py-=sine*2;
		}
	}
	
	UnlockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
	UnlockPixels(GetGWorldPixMap(eWinRec->tempDataPtr));
	
	/* �V�����I��̈� */
	err=BitMapToRegion(selectedRgn,GetPortBitMapForCopyBits(rotateGWorld));
	UnlockPixels(GetGWorldPixMap(rotateGWorld));
	DisposeGWorld(rotateGWorld);
	
	/* �I��͈͊O�͑I������Ă��Ȃ���Ԃ����]���ꂽ�ꍇ�̂ݍX�V */
	if (!eWinRec->isSelected)
	{
		MySetGWorld(selectionMask);
		MyLockPixels(selectionMask);
		if (isBackMode && !gBackColor.isTransparent)
			PaintRect(&eWinRec->iconSize);
		else
			EraseRect(&eWinRec->iconSize);
		MyUnlockPixels(selectionMask);
		
		/* �V�����I�����ꂽ���̓u�����h���[�h�����ɖ߂� */
		eWinRec->selectionBlend=srcCopy;
		{
			MenuHandle	menu;
			
			menu=GetMenuHandle(mEffect);
			CheckMenuItem(menu,iBlend,false);
		}
	}
	
	eWinRec->prevSelectedRect=eWinRec->selectedRect;
	GetRegionBounds(selectedRgn,&eWinRec->selectedRect);
	if (eWinRec->isSelected)
		OffsetRgn(selectedRgn,eWinRec->selectionOffset.h,eWinRec->selectionOffset.v);
	
	CopyRgn(eWinRec->selectionPos,eWinRec->prevSelectionPos);
	CopyRgn(eWinRec->eSelectedRgn,eWinRec->ePrevSelectedRgn);
	CopyRgn(selectedRgn,eWinRec->selectionPos);
	CopyRgn(selectedRgn,eWinRec->eSelectedRgn);
	MapRgn(eWinRec->eSelectedRgn,&eWinRec->iconSize,&eWinRec->iconEditSize);
	eWinRec->isSelected=true;
	
	DisposeRgn(selectedRgn);
	
	/* ����̑I��͈͂ƑO��̑I��͈͂��X�V�͈� */
	UnionRgn(eWinRec->selectionPos,eWinRec->prevSelectionPos,eWinRec->updateRgn);
	
	eWinRec->showSelection=true;
	
	SetGWorld(cPort,cDevice);
	DispOffPort(theWindow);
	
	if (!gToolPrefs.selectionMasking && gSelectedTool!=kMarqueeTool)
		ToolSelect(kMarqueeTool);
	
	UpdateClipMenu();
	UpdateSelectionMenu();
	gUsedCount.rotateNum++;
	UpdateTransparentMenu();
	SetUndoMode(umRotate);
	if (gPaletteCheck == kPaletteCheckUsed) /* �p���b�g�̃`�F�b�N���A�b�v�f�[�g */
		UpdatePaletteCheck();
}

/* �}�`���� */
void DoEdging(WindowPtr theWindow)
{
	EdgePrefsRec	newPrefs;
	unsigned short	transR;
	RGBColor	transRGB;
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	Rect		effectRect=eWinRec->selectedRect;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	if (ConfigureEdging(&newPrefs) != noErr) return;
	
	GetGWorld(&cPort,&cDevice);
	GoOffPort(theWindow);
	
	SetGWorld(eWinRec->editDataPtr,0);
	MyLockPixels(editDataPtr);
	RGBForeColor(&gCurrentColor.rgb);
	PenSize(newPrefs.pixels,newPrefs.pixels);
	
	DoEdgingMain(eWinRec,&newPrefs);
	
	MyUnlockPixels(editDataPtr);
	PenNormal();
	ForeColor(blackColor);
	
	/* �s�����x��100%�łȂ��Ƃ��͂���Ȃ�̏��� */
	if (newPrefs.transparency != 1)
	{
		transR=(newPrefs.transparency-1)<<14;
		SetRGBColor(&transRGB,transR,transR,transR);
		OpColor(&transRGB);
		
		CopyBits(GetPortBitMapForCopyBits(eWinRec->tempDataPtr),
			GetPortBitMapForCopyBits(eWinRec->editDataPtr),
			&effectRect,&effectRect,blend,nil);
	}
	
	/* �}�X�N���� */
	MySetGWorld(currentMask);
	MyLockPixels(currentMask);
	if (gCurrentColor.isTransparent)
		ForeColor(whiteColor);
	PenSize(newPrefs.pixels,newPrefs.pixels);
	DoEdgingMain(eWinRec,&newPrefs);
	ForeColor(blackColor);
	PenNormal();
	MyUnlockPixels(currentMask);
	
	CopyRgn(eWinRec->selectionPos,eWinRec->updateRgn);
	SetGWorld(cPort,cDevice);
	DispOffPort(theWindow);
	gUsedCount.edgeNum++;
	UpdateTransparentMenu();
	SetUndoMode(umEffect);
	if (gPaletteCheck == kPaletteCheckUsed) /* �p���b�g�̃`�F�b�N���A�b�v�f�[�g */
		UpdatePaletteCheck();
}

/* �}�`���̓��C���i�`��̂ݒS���j */
void DoEdgingMain(PaintWinRec *eWinRec,EdgePrefsRec *prefs)
{
	RgnHandle	effectRgn;
	Rect		effectRect=eWinRec->selectedRect,tempRect;
	
	switch(prefs->kind)
	{
		case kShapeEdge:
			/* ����� */
			effectRgn=NewRgn();
			CopyRgn(eWinRec->selectionPos,effectRgn);
			GetRegionBounds(eWinRec->selectionPos,&tempRect);
			MapRgn(effectRgn,&tempRect,&effectRect);
			FrameRgn(effectRgn);
			DisposeRgn(effectRgn);
			break;
		
		case kShapeOval:
			/* ���~ */
			FrameOval(&effectRect);
			break;
		
		case kShapeTriangle:
			/* �O�p�`�B�`�𐮂��邽�߁A��`�Ƃ��ĕ`�� */
			MoveTo((effectRect.left+effectRect.right-prefs->pixels)/2,effectRect.top);
			LineTo(effectRect.left,effectRect.bottom-prefs->pixels);
			LineTo(effectRect.right-prefs->pixels,effectRect.bottom-prefs->pixels);
			LineTo((effectRect.left+effectRect.right-prefs->pixels+1)/2,effectRect.top);
			LineTo((effectRect.left+effectRect.right-prefs->pixels)/2,effectRect.top);
			break;
		
		case kShapeRTriangle:
			/* �t�O�p�`�B�`�𐮂��邽�߁A��`�Ƃ��ĕ`�� */
			MoveTo((effectRect.left+effectRect.right-prefs->pixels)/2,effectRect.bottom-prefs->pixels);
			LineTo(effectRect.left,effectRect.top);
			LineTo(effectRect.right-prefs->pixels,effectRect.top);
			LineTo((effectRect.left+effectRect.right-prefs->pixels+1)/2,effectRect.bottom-prefs->pixels);
			LineTo((effectRect.left+effectRect.right-prefs->pixels)/2,effectRect.bottom-prefs->pixels);
			break;
		
		case kShapeLozenge:
			/* �Ђ��`�B�`�𐮂��邽�߁A���p�`�Ƃ��ĕ`�� */
			MoveTo((effectRect.left+effectRect.right-prefs->pixels)/2,effectRect.top);
			LineTo(effectRect.left,(effectRect.top+effectRect.bottom-prefs->pixels)/2);
			LineTo(effectRect.left,(effectRect.top+effectRect.bottom-prefs->pixels+1)/2);
			LineTo((effectRect.left+effectRect.right-prefs->pixels)/2,effectRect.bottom-prefs->pixels);
			LineTo((effectRect.left+effectRect.right-prefs->pixels+1)/2,effectRect.bottom-prefs->pixels);
			LineTo(effectRect.right-prefs->pixels,(effectRect.top+effectRect.bottom-prefs->pixels+1)/2);
			LineTo(effectRect.right-prefs->pixels,(effectRect.top+effectRect.bottom-prefs->pixels)/2);
			LineTo((effectRect.left+effectRect.right-prefs->pixels+1)/2,effectRect.top);
			LineTo((effectRect.left+effectRect.right-prefs->pixels)/2,effectRect.top);
			break;
		
		case kShapeDiagonal1:
			/* �Ίp���i�_�j */
			MoveTo(effectRect.left,effectRect.top);
			LineTo(effectRect.right-prefs->pixels,effectRect.bottom-prefs->pixels);
			break;
		
		case kShapeDiagonal2:
			/* �Ίp���i�^�j */
			MoveTo(effectRect.right-prefs->pixels,effectRect.top);
			LineTo(effectRect.left,effectRect.bottom-prefs->pixels);
			break;
	}
}

/* ���E���] */
void FlipHorizontal(WindowPtr theWindow)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	Rect		effectRect=eWinRec->selectedRect;
	short		width,i;
	Rect		srcRect,dstRect;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	GetGWorld(&cPort,&cDevice);
	GoOffPort(theWindow);
	
	SetGWorld(eWinRec->editDataPtr,0);
	width=(effectRect.right - effectRect.left);
	SetRect(&srcRect,effectRect.left,effectRect.top,effectRect.left+1,effectRect.bottom);
	SetRect(&dstRect,effectRect.right-1,effectRect.top,effectRect.right,effectRect.bottom);
	
	for (i=0; i<width; i++)
	{
		CopyBits(GetPortBitMapForCopyBits(eWinRec->tempDataPtr),
			GetPortBitMapForCopyBits(eWinRec->editDataPtr),
			&srcRect,&dstRect,srcCopy,nil);
		srcRect.left++; srcRect.right++;
		dstRect.left--; dstRect.right--;
	}
	
	CopyRgn(eWinRec->selectionPos,eWinRec->prevSelectionPos);
	CopyRgn(eWinRec->eSelectedRgn,eWinRec->ePrevSelectedRgn);
	
	/* �I��͈͂𔽓]������*/
	if (eWinRec->isSelected)
	{
		Rect	tempRect;
		
		GetRegionBounds(eWinRec->selectionPos,&tempRect);
		FlipHorizontalRgn(eWinRec->selectionPos,&tempRect);
		
		CopyRgn(eWinRec->selectionPos,eWinRec->eSelectedRgn);
		MapRgn(eWinRec->eSelectedRgn,&eWinRec->iconSize,&eWinRec->iconEditSize);
		
		UnionRgn(eWinRec->selectionPos,eWinRec->prevSelectionPos,eWinRec->updateRgn);
	}
	else
		CopyRgn(eWinRec->selectionPos,eWinRec->updateRgn);
	
	/* �}�X�N�����] */
	SetRect(&srcRect,effectRect.left,effectRect.top,effectRect.left+1,effectRect.bottom);
	SetRect(&dstRect,effectRect.right-1,effectRect.top,effectRect.right,effectRect.bottom);
	
	SetGWorld(eWinRec->currentMask,0);
	for (i=0; i<width; i++)
	{
		CopyBits(GetPortBitMapForCopyBits(eWinRec->pCurrentMask),
			GetPortBitMapForCopyBits(eWinRec->currentMask),
			&srcRect,&dstRect,srcCopy,nil);
		
		srcRect.left++; srcRect.right++;
		dstRect.left--; dstRect.right--;
	}
	
	SetGWorld(cPort,cDevice);
	DispOffPort(theWindow);
	
	gUsedCount.flipNum++;
	SetUndoMode(umFlip);
	if (gPaletteCheck == kPaletteCheckUsed) /* �p���b�g�̃`�F�b�N���A�b�v�f�[�g */
		UpdatePaletteCheck();
}

/* ���[�W�����̍��E���] */
void FlipHorizontalRgn(RgnHandle rgn,Rect *r)
{
	RgnHandle	tempRgn=NewRgn(),resultRgn=NewRgn();
	Rect		srcRect;
	short		i,width;
	
	srcRect=*r;
	width=srcRect.right-srcRect.left;
	srcRect.right=srcRect.left+1;
	
	for (i=0; i<width; i++)
	{
		RectRgn(tempRgn,&srcRect);
		SectRgn(rgn,tempRgn,tempRgn);
		OffsetRgn(tempRgn,width-1-(i<<1),0);
		UnionRgn(resultRgn,tempRgn,resultRgn);
		
		OffsetRect(&srcRect,1,0);
	}
	DisposeRgn(tempRgn);
	CopyRgn(resultRgn,rgn);
	DisposeRgn(resultRgn);
}

/* �㉺���] */
void FlipVertical(WindowPtr theWindow)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	Rect		effectRect=eWinRec->selectedRect;
	short		height,width,i,left,top;
	Rect		srcRect,dstRect,tempRect;
	PixMapHandle	srcPMH,dstPMH;
	Ptr			srcAddr,dstAddr;
	long		srcRowBytes,dstRowBytes;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	GetGWorld(&cPort,&cDevice);
	GoOffPort(theWindow);
	
	height=(effectRect.bottom - effectRect.top);
	width=(effectRect.right - effectRect.left) * ( ( eWinRec->colorMode == k32BitColorMode ) ? 4 : 1 );
	
	GetPortBounds(eWinRec->editDataPtr,&tempRect);
	left=tempRect.left;
	top=tempRect.top;
	
	SetGWorld(eWinRec->editDataPtr,0);
	
	/* PixMap�𒼐ڂ����� */
	srcPMH=GetGWorldPixMap(eWinRec->tempDataPtr);
	LockPixels(srcPMH);
	srcRowBytes=MyGetPixRowBytes(srcPMH) & 0x3fff;
	srcAddr=MyGetPixBaseAddr(srcPMH) + srcRowBytes * (effectRect.top - top) + (effectRect.left - left);
	dstPMH=GetGWorldPixMap(eWinRec->editDataPtr);
	LockPixels(dstPMH);
	dstRowBytes=MyGetPixRowBytes(srcPMH) & 0x3fff;
	dstAddr=MyGetPixBaseAddr(dstPMH) + dstRowBytes * (effectRect.bottom-1 - top) + (effectRect.left - left);
	
	for (i=0; i<height; i++)
	{
		BlockMoveData(srcAddr,dstAddr,width);
		srcAddr+=srcRowBytes;
		dstAddr-=dstRowBytes;
	}
	
	UnlockPixels(srcPMH);
	UnlockPixels(dstPMH);
	
	CopyRgn(eWinRec->selectionPos,eWinRec->prevSelectionPos);
	CopyRgn(eWinRec->eSelectedRgn,eWinRec->ePrevSelectedRgn);
	
	/* �I��͈͂𔽓]������*/
	if (eWinRec->isSelected)
	{
		GetRegionBounds(eWinRec->selectionPos,&tempRect);
		FlipVerticalRgn(eWinRec->selectionPos,&tempRect);
		
		CopyRgn(eWinRec->selectionPos,eWinRec->eSelectedRgn);
		MapRgn(eWinRec->eSelectedRgn,&eWinRec->iconSize,&eWinRec->iconEditSize);
		
		UnionRgn(eWinRec->selectionPos,eWinRec->prevSelectionPos,eWinRec->updateRgn);
	}
	else
		CopyRgn(eWinRec->selectionPos,eWinRec->updateRgn);
	
	/* �}�X�N�����] */
	SetRect(&srcRect,effectRect.left,effectRect.top,effectRect.right,effectRect.top+1);
	SetRect(&dstRect,effectRect.left,effectRect.bottom-1,effectRect.right,effectRect.bottom);
	
	SetGWorld(eWinRec->currentMask,0);
	for (i=0; i<height; i++)
	{
		CopyBits(GetPortBitMapForCopyBits(eWinRec->pCurrentMask),
			GetPortBitMapForCopyBits(eWinRec->currentMask),
			&srcRect,&dstRect,srcCopy,nil);
		
		srcRect.top++; srcRect.bottom++;
		dstRect.top--; dstRect.bottom--;
	}
	
	SetGWorld(cPort,cDevice);
	DispOffPort(theWindow);
	
	gUsedCount.flipNum++;
	SetUndoMode(umFlip);
	if (gPaletteCheck == kPaletteCheckUsed) /* �p���b�g�̃`�F�b�N���A�b�v�f�[�g */
		UpdatePaletteCheck();
}

/* ���[�W�����̏㉺���] */
void FlipVerticalRgn(RgnHandle rgn,Rect *r)
{
	RgnHandle	tempRgn=NewRgn(),resultRgn=NewRgn();
	Rect		srcRect;
	short		i,height;
	
	srcRect=*r;
	height=srcRect.bottom-srcRect.top;
	srcRect.bottom=srcRect.top+1;
	
	for (i=0; i<height; i++)
	{
		RectRgn(tempRgn,&srcRect);
		SectRgn(rgn,tempRgn,tempRgn);
		OffsetRgn(tempRgn,0,height-1-(i<<1));
		UnionRgn(resultRgn,tempRgn,resultRgn);
		
		OffsetRect(&srcRect,0,1);
	}
	DisposeRgn(tempRgn);
	CopyRgn(resultRgn,rgn);
	DisposeRgn(resultRgn);
}

/* �����̓��� */
void TypeString(WindowPtr theWindow)
{
	Rect	stringRect;
	FontInfo	fontInfo;
	TypeStringRec	prefs=typeStringPrefs;
	OpenCPicParams	picParam;
	PicHandle	picture;
	short	width;
	OSErr		err,tempErr;
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	ResetRuler();
	MySetCursor(0);
	if (ConfigureTypeString(&prefs) != ok) return;
	
	typeStringPrefs=prefs;
	
	/* �܂��A�����̉����A�c�̍������`�F�b�N */
	TextFont(prefs.fontID);
	TextSize(prefs.size);
	TextFace(prefs.face);
	TextMode(srcOr);
	width=StringWidth(prefs.string)+2+((prefs.face & italic) ? prefs.size/10 : 0)+((prefs.face & underline ) ? 2 : 0);
	
	GetFontInfo(&fontInfo);
	SetRect(&stringRect,0,0,width,fontInfo.leading+fontInfo.ascent+fontInfo.descent);
	
	GetGWorld(&cPort,&cDevice);
	
	if (!prefs.antialias) /* �A���`�G�C���A�X�������Ȃ��ꍇ */
	{
		GWorldPtr	tempGWorld=nil,maskGWorld;
		RgnHandle	maskRgn=nil;
		CTabHandle	ctable=NULL;
		RgnHandle	clipRgn=NULL,tempRgn=NULL;
		
		if (gSystemVersion < 0x0850)
		{
			clipRgn=NewRgn();
			GetClip(clipRgn);
			tempRgn=NewRgn();
			RectRgn(tempRgn,&stringRect);
			SetClip(tempRgn);
			DisposeRgn(tempRgn);
		}
		else /* 8.5�ȍ~�̏ꍇ�͂Ȃ߂炩�ȕ����̐ݒ�ɔ����A */
		{
			/* 2bits GWorld��ɕ`�� */
			/* �J���[�p���b�g���쐬 */
			ctable=(CTabHandle)NewHandle(sizeof(ColorTable)+sizeof(ColorSpec)*(4-1));
			if (ctable==nil)
			{
				ErrorAlertFromResource(EFFECTERR_RESID,EFFECTERR1);
				return;
			}
			
			(*ctable)->ctSeed=UniqueID('clut');
			(*ctable)->ctFlags=0;
			(*ctable)->ctSize=4-1;
			
			(*ctable)->ctTable[0].value=0;
			(*ctable)->ctTable[0].rgb=rgbBlackColor;
			(*ctable)->ctTable[1].value=1;
			(*ctable)->ctTable[1].rgb=gCurrentColor.rgb;
			(*ctable)->ctTable[2].value=2;
			(*ctable)->ctTable[2].rgb=gBackColor.rgb;
			(*ctable)->ctTable[3].value=3;
			(*ctable)->ctTable[3].rgb=rgbWhiteColor;
			
			err=NewGWorld(&tempGWorld,2,&stringRect,ctable,0,useTempMem);
			if (err!=noErr)
			{
				ErrorAlertFromResource(EFFECTERR_RESID,EFFECTERR1);
				DisposeHandle((Handle)ctable);
				return;
			}
			
			SetGWorld(tempGWorld,0);
			
			TextFont(prefs.fontID);
			TextSize(prefs.size);
			TextFace(prefs.face);
			TextMode(srcOr);
			
			/*�@�}�X�N�쐬 */
			if (gCurrentColor.isTransparent && gBackColor.isTransparent) /* �������� */
			{
				/* ��̃��[�W���� */
				maskRgn=NewRgn();
				SetEmptyRgn(maskRgn);
			}
			else if (!gCurrentColor.isTransparent && !gBackColor.isTransparent) /* �����s���� */
			{
				/* �S�́Bnil�̂܂܂ł��� */
				maskRgn=nil;
			}
			else
			{
				/* �ꕔ�BGWorld�o�R�ō�� */
				err=NewGWorld(&maskGWorld,1,&stringRect,nil,0,useTempMem);
				if (err==noErr)
				{
					SetGWorld(maskGWorld,0);
					LockPixels(GetGWorldPixMap(maskGWorld));
					
					/* �F */
					if (gCurrentColor.isTransparent) /* �O�i�F������ */
					{
						ForeColor(whiteColor);
						BackColor(blackColor);
					}
					else
					{
						ForeColor(blackColor);
						BackColor(whiteColor);
					}
					
					/* �w�i */
					EraseRect(&stringRect);
					
					/* �e�L�X�g�t�H���g�Ȃ� */
					TextFont(prefs.fontID);
					TextSize(prefs.size);
					TextFace(prefs.face);
					TextMode(srcOr);
					
					MoveTo(0,fontInfo.ascent);
					DrawString(prefs.string);
					
					ForeColor(blackColor);
					BackColor(whiteColor);
					
					UnlockPixels(GetGWorldPixMap(maskGWorld));
					
					/* ���[�W������ */
					maskRgn=NewRgn();
					err=BitMapToRegion(maskRgn,GetPortBitMapForCopyBits(maskGWorld));
					if (err!=noErr)
					{
						SysBeep(0);
						DisposeRgn(maskRgn);
						maskRgn=nil;
					}
					
					SetGWorld(tempGWorld,0);
					DisposeGWorld(maskGWorld);
				}
				else
				{
					SysBeep(0);
					maskRgn=nil;
				}
			}
		}
		
		/* �s�N�`���L�^�J�n */
		picParam.srcRect=stringRect;
		picParam.hRes=72L<<16;
		picParam.vRes=72L<<16;
		picParam.version=-2;
		
		if (gSystemVersion < 0x0850)
//			picture=OpenCPicture(&picParam);
			err=TempOpenCPicture(&picParam);
		
		RGBForeColor(&gBackColor.rgb);
		PaintRect(&stringRect);
		
		RGBForeColor(&gCurrentColor.rgb);
		MoveTo(0,fontInfo.ascent);
		DrawString(prefs.string);
		
		ForeColor(blackColor);
		BackColor(whiteColor);
		
		if (gSystemVersion >= 0x0850)
		{
//			picture=OpenCPicture(&picParam);
			err=TempOpenCPicture(&picParam);
			if (err!=noErr)
			{
				if (maskRgn!=nil) DisposeRgn(maskRgn);
				SetGWorld(cPort,cDevice);
				DisposeGWorld(tempGWorld);
				DisposeHandle((Handle)ctable);
				ErrorAlertFromResource(EFFECTERR_RESID,EFFECTERR1);
				return;
			}
			
			CopyBits(GetPortBitMapForCopyBits(tempGWorld),
				GetPortBitMapForCopyBits(tempGWorld),
				&stringRect,&stringRect,srcCopy,maskRgn);
			
			if (QDError()!=noErr)
			{
				TempClosePicture(nil);
				if (maskRgn!=nil) DisposeRgn(maskRgn);
				SetGWorld(cPort,cDevice);
				DisposeGWorld(tempGWorld);
				DisposeHandle((Handle)ctable);
				ErrorAlertFromResource(EFFECTERR_RESID,EFFECTERR1);
				return;
			}
		}
		
		/* �␳ */
		err=CheckPictureByte();
		
	//	(**picture).picFrame=stringRect;
	//	ClosePicture();
		err=TempClosePicture(&picture);
		
		if (gSystemVersion < 0x0850)
		{
			SetClip(clipRgn);
			DisposeRgn(clipRgn);
		}
		else
		{
			if (maskRgn!=nil) DisposeRgn(maskRgn);
			DisposeGWorld(tempGWorld);
			DisposeHandle((Handle)ctable);
		}
		
		SetGWorld(cPort,cDevice);
	}
	else /* �A���`�G�C���A�X��������ꍇ */
	{
		GWorldPtr	tempGWorld;
		PixMapHandle	tempPix;
		GWorldPtr	currentPort;
		GDHandle	currentDevice;
		Rect		tempRect;
		
		#define		kAntiDepth	4
		
		/* �̈���c��kAntiDepth�{�ɍL���� */
		stringRect.right+=2;
		stringRect.bottom++;
		SetRect(&tempRect,0,0,stringRect.right*kAntiDepth,stringRect.bottom*kAntiDepth);
		
		GetGWorld(&currentPort,&currentDevice);
		err=NewGWorld(&tempGWorld,eWinRec->iconDepth,&tempRect,0,0,useTempMem);
		if (err!=noErr)
		{
			ErrorAlertFromResource(EFFECTERR_RESID,EFFECTERR1);
			return;
		}
		
		tempPix=GetGWorldPixMap(tempGWorld);
		SetGWorld(tempGWorld,0);
		TextFont(prefs.fontID);
		TextSize(prefs.size *kAntiDepth);
		TextFace(prefs.face);
		TextMode(srcOr);
		
		LockPixels(tempPix);
		RGBForeColor(&gBackColor.rgb);
		PaintRect(&tempRect);
		
		RGBForeColor(&gCurrentColor.rgb);
		MoveTo(0,fontInfo.ascent*kAntiDepth+kAntiDepth/2);
		DrawString(prefs.string);
		
		ForeColor(blackColor);
		UnlockPixels(tempPix);
		
		/* �s�N�`���L�^�J�n */
		picParam.srcRect=stringRect;
		picParam.hRes=72L<<16;
		picParam.vRes=72L<<16;
		picParam.version=-2;
		
//		picture=OpenCPicture(&picParam);
		err=TempOpenCPicture(&picParam);
		
		CopyBits(GetPortBitMapForCopyBits(tempGWorld),
			GetPortBitMapForCopyBits(tempGWorld),
			&tempRect,&stringRect,srcCopy+ditherCopy,0);
		
//		(**picture).picFrame=stringRect;
//		ClosePicture();
		/* �␳ */
		err=CheckPictureByte();
		err=TempClosePicture(&picture);
		
		if ((err=QDError())!=noErr)
		{
		//	ClosePicture();
			KillPicture(picture);
			picture=nil;
			ErrorAlertFromResource(EFFECTERR_RESID,EFFECTERR1);
			SetGWorld(currentPort,currentDevice);
			DisposeGWorld(tempGWorld);
			return;
		}
		
		SetGWorld(cPort,cDevice);
		DisposeGWorld(tempGWorld);
	}
	
	TextMode(srcCopy);
	TextFont(applFont);
	TextFace(normal);
	
	err=PutPictureIntoEditWin(picture,theWindow);
//	KillPicture(picture);
	TempDisposeHandle((Handle)picture,&tempErr);
	
	if (err==noErr)
	{
		SetUndoMode(umInputText);
		UpdateTransparentMenu();
		DispOffPort(theWindow);
		if (gPaletteCheck == kPaletteCheckUsed) /* �p���b�g�̃`�F�b�N���A�b�v�f�[�g */
			UpdatePaletteCheck();
	}
	else
	{
		SetUndoMode(umCannot);
	}
}

#define		kTypeStringDialogID		133
enum {
	diFontPopup=3,
	diFontSize,
	diText,
	diBold,
	diItalic,
	diUnderline,
	diOutline,
	diShadow,
	diAntialias,
	diPreview
};
struct MyTypeStringRec {
	TypeStringRec	*prefs;
	TEHandle		hTE;
};

/* �������͂̐ݒ�ύX */
short ConfigureTypeString(TypeStringRec *prefs)
{
	DialogPtr	dp;
	short		item;
	Rect		box;
	Str255		str;
	long		l;
	TEHandle	hTE;
	GrafPtr		port;
	MenuHandle	menu;
	short		mItem;
	ModalFilterUPP	mfUPP=NewModalFilterUPP(MyDispTEFilter);
	struct MyTypeStringRec	myData;
	WindowRefExtensions	**myExtensions;
	
	#if TARGET_API_MAC_CARBON
	ItemCount	count;
	FMFontStyle	style;
	#endif
	
	GetPort(&port);
	DeactivateFloatersAndFirstDocumentWindow();
	dp=GetNewDialog(kTypeStringDialogID,nil,kFirstWindowOfClass);
	SetPortDialogPort(dp);
	SetDialogDefaultItem(dp,ok);
	SetDialogCancelItem(dp,cancel);
	
	/* ��ɃT�C�Y�`�F�b�N */
	if (prefs->size < 0) prefs->size=12;
	
	/* �e�A�C�e���̏����� */
	menu=GetControlPopupMenuHandle((ControlHandle)GetDialogItemHandle(dp,diFontPopup));
	#if TARGET_API_MAC_CARBON
	CreateStandardFontMenu(menu,0,190,kNilOptions,&count);
	mItem=CountMenuItems(menu);
	SetDialogControlMaximum(dp,diFontPopup,mItem);
	#endif
	SetDialogControlValue(dp,diFontPopup,prefs->menuItem);
	#if TARGET_API_MAC_CARBON
	GetFontFamilyFromMenuSelection(menu,prefs->menuItem,&prefs->fontID,&style); /* style�͂ǂ�����H */
	#else
	GetMenuItemText(menu,prefs->menuItem,str);
	GetFNum(str,&prefs->fontID);
	#endif
	
	NumToString(prefs->size,str);
	SetDialogItemText2(dp,diFontSize,str);
	
	SetDialogItemText2(dp,diText,prefs->string);
	
	SetDialogControlValue(dp,diBold,(prefs->face & bold) ? 1 : 0);
	SetDialogControlValue(dp,diItalic,(prefs->face & italic) ? 1 : 0);
	SetDialogControlValue(dp,diUnderline,(prefs->face & underline) ? 1 : 0);
	SetDialogControlValue(dp,diOutline,(prefs->face & outline) ? 1 : 0);
	SetDialogControlValue(dp,diShadow,(prefs->face & shadow) ? 1 : 0);
	
	SetDialogControlValue(dp,diAntialias,prefs->antialias ? 1 : 0);
	
	/* �v���r���[ */
	GetDialogItemRect(dp,diPreview,&box);
	TextFont(prefs->fontID);
	TextSize(prefs->size);
	TextFace(prefs->face);
	hTE=TENew(&box,&box);
	TESetText(&prefs->string[1],prefs->string[0],hTE);
	TextFont(systemFont);
	TextSize(0);
	TextFace(normal);
	
	SelectDialogItemText(dp,diText,0,prefs->string[0]);
	ShowWindow(GetDialogWindow(dp));
	TEUpdate(&box,hTE);
	
	myExtensions=(WindowRefExtensions **)NewHandleClear(sizeof(WindowRefExtensions));
	HLockHi((Handle)myExtensions);
	(*myExtensions)->windowKind=kTypeStringDialogID;
	(*myExtensions)->userRefCon=(long)&myData;
	myData.prefs=prefs;
	myData.hTE=hTE;
	SetWRefCon(GetDialogWindow(dp),(long)myExtensions);
	
	item=cancel+1;
	while (item!=ok && item!=cancel)
	{
		ModalDialog(mfUPP,&item);
		
		switch (item)
		{
			case diFontSize:
				GetDialogItemText2(dp,diFontSize,str);
				StringToNum(str,&l);
				
				if (l>0 && l<32768)
				{
					prefs->size=l;
					
					TypeStringTEUpdate(prefs,hTE);
				}
				else
				{
					NumToString(prefs->size,str);
					SetDialogItemText2(dp,diFontSize,str);
				}
				break;
			
			case diFontPopup:
				mItem=GetDialogControlValue(dp,diFontPopup);
				#if TARGET_API_MAC_CARBON
				GetFontFamilyFromMenuSelection(menu,mItem,&prefs->fontID,&style);
				#else
				GetMenuItemText(menu,mItem,str);
				GetFNum(str,&prefs->fontID);
				#endif
				
				TypeStringTEUpdate(prefs,hTE);
				break;
			
			case diText:
				GetDialogItemText2(dp,diText,str);
				
				GetDialogItemRect(dp,diPreview,&box);
				TESetText(&str[1],str[0],hTE);
				EraseRect(&box);
				TEUpdate(&box,hTE);
				break;
			
			case diBold:
				prefs->face^=bold;
				SetDialogControlValue(dp,diBold,(prefs->face & bold) ? 1 : 0);
				
				TypeStringTEUpdate(prefs,hTE);
				break;
			
			case diItalic:
				prefs->face^=italic;
				SetDialogControlValue(dp,diItalic,(prefs->face & italic) ? 1 : 0);
				
				TypeStringTEUpdate(prefs,hTE);
				break;
			
			case diUnderline:
				prefs->face^=underline;
				SetDialogControlValue(dp,diUnderline,(prefs->face & underline) ? 1 : 0);
				
				TypeStringTEUpdate(prefs,hTE);
				break;
			
			case diOutline:
				prefs->face^=outline;
				SetDialogControlValue(dp,diOutline,(prefs->face & outline) ? 1 : 0);
				
				TypeStringTEUpdate(prefs,hTE);
				break;
			
			case diShadow:
				prefs->face^=shadow;
				SetDialogControlValue(dp,diShadow,(prefs->face & shadow) ? 1 : 0);
				
				TypeStringTEUpdate(prefs,hTE);
				break;
			
			case diAntialias:
				prefs->antialias=!prefs->antialias;
				SetDialogControlValue(dp,diAntialias,prefs->antialias ? 1 : 0);
				break;
		}
	}
	
	if (item==ok)
	{
		/* �w�肳�ꂽ���e�𓾂� */
		/* ���j���[ */
		mItem=GetDialogControlValue(dp,diFontPopup);
		prefs->menuItem=mItem;
		
		#if TARGET_API_MAC_CARBON
		GetFontFamilyFromMenuSelection(menu,mItem,&prefs->fontID,&style);
		#else
		GetMenuItemText(menu,mItem,str);
		GetFNum(str,&prefs->fontID);
		#endif
		
		/* �t�H���g�T�C�Y */
		GetDialogItemText2(dp,diFontSize,str);
		StringToNum(str,&l);
		prefs->size=l;
		
		/* ������ */
		GetDialogItemText2(dp,diText,prefs->string);
	}
	
	TEDispose(hTE);
	DisposeDialog(dp);
	HUnlock((Handle)myExtensions);
	DisposeHandle((Handle)myExtensions);
	DisposeModalFilterUPP(mfUPP);
	ActivateFloatersAndFirstDocumentWindow();
	SetPort(port);
	
	return item;
}

/* �������̓_�C�A���O�̃A�b�v�f�[�g */
pascal Boolean MyDispTEFilter(DialogPtr theDialog,EventRecord *theEvent,short *theItemHit)
{
	if (theEvent->what == updateEvt && GetDialogFromWindow((WindowPtr)theEvent->message) == theDialog)
	{
		GrafPtr	port;
		WindowRefExtensions	**myExtensions;
		struct MyTypeStringRec	*myData;
		
		GetPort(&port);
		SetPortDialogPort(theDialog);
		BeginUpdate(GetDialogWindow(theDialog));
		DrawDialog(theDialog);
		
		myExtensions=(WindowRefExtensions **)GetWRefCon(GetDialogWindow(theDialog));
		myData=(struct MyTypeStringRec *)(*myExtensions)->userRefCon;
		TypeStringTEUpdate(myData->prefs,myData->hTE);
		
		EndUpdate(GetDialogWindow(theDialog));
		SetPort(port);
		
		return true;
	}
	
	return MyModalDialogFilter(theDialog,theEvent,theItemHit);
}

/* TE���A�b�v�f�[�g */
void TypeStringTEUpdate(TypeStringRec *prefs,TEHandle hTE)
{
	Rect	box;
	FontInfo	info;
	
	TextSize(prefs->size);
	TextFont(prefs->fontID);
	TextFace(prefs->face);
	GetFontInfo(&info);
	(**hTE).txSize=prefs->size;
	(**hTE).txFont=prefs->fontID;
	(**hTE).txFace=prefs->face;
	(**hTE).lineHeight=info.ascent+info.descent+info.leading;
	(**hTE).fontAscent=info.ascent;
	box=(**hTE).viewRect;
	EraseRect(&box);
	TECalText(hTE);
	TEUpdate(&box,hTE);
	TextFont(systemFont);
	TextSize(0);
	TextFace(normal);
}

/* �^����ꂽ�p�x����T�C���^�R�T�C�����v�Z�i���Ă����Ă��e�[�u������Ăяo�����������ǁj */
void GetSinCos(short angle,long *sine,long *cosine)
{
	Handle	resHandle;
	unsigned short	*s;
	
	UseResFile(gApplRefNum);
	
	resHandle=Get1Resource('sine',129);
	if (resHandle==0)
	{
		SysBeep(0);
		return;
	}
	
	HLock(resHandle);
	s=(unsigned short *)*resHandle;
	
	/* �p�x���O�`�X�O�x�͈̔͂ł͂Ȃ��ꍇ�̏����Ȃ� */
	if (angle<=90)
	{
		*sine=s[angle];
		*cosine=s[90-angle];
	}
	else if (angle<=180)
	{
		*sine=s[180-angle];
		*cosine=-(long)s[angle-90];
	}
	else if (angle<=270)
	{
		*sine=-(long)s[angle-180];
		*cosine=-(long)s[270-angle];
	}
	else /* 270<angle<360 */
	{
		*sine=-(long)s[360-angle];
		*cosine=s[angle-270];
	}
	
	HUnlock(resHandle);
	ReleaseResource(resHandle);
}


/* ��]�̐ݒ胋�[�`���̂��߂̒萔 */
#define		dRotate	138
enum {
	angleStr=3
};

/* ��]�̐ݒ� */
short ConfigureRotate(void)
{
	DialogPtr	dp;
	short	item;
	Str255	str;
	long	num;
	short	result;
	ModalFilterUPP	mfUPP=NewModalFilterUPP(MyModalDialogFilter);
	
	DeactivateFloatersAndFirstDocumentWindow();
	dp=GetNewDialog(dRotate,nil,kFirstWindowOfClass);
	
	NumToString(gAngle,str);
	SetDialogItemText2(dp,angleStr,str);
	
	SelectDialogItemText(dp,angleStr,0,str[0]);
	
	SetDialogDefaultItem(dp,ok);
	SetDialogCancelItem(dp,cancel);
	ShowWindow(GetDialogWindow(dp));
	
	item=3;
	while (item!=ok && item!=cancel)
		ModalDialog(mfUPP,&item);
	
	GetDialogItemText2(dp,angleStr,str);
	DisposeDialog(dp);
	DisposeModalFilterUPP(mfUPP);
	ActivateFloatersAndFirstDocumentWindow();
	
	if (item==ok)
	{
		StringToNum(str,&num);
		result=num;
		
		/* ���͂��ꂽ�p�x���O�`�R�U�O�͈̔͂ɒ������� */
		while (result<0)
			result+=360;
		while (result>=360)
			result-=360;
		
		gAngle=result;
		return result;
	}
	return userCanceledErr;
}

/* �_�`�̐ݒ� */
short ConfigureDotPicture(void)
{
	DialogPtr	dp;
	short	item;
	
	DotPictureRec	prevDotPrefs;
	ModalFilterUPP	mfUPP=NewModalFilterUPP(MyModalDialogFilter);
	
	DeactivateFloatersAndFirstDocumentWindow();
	dp=GetNewDialog(dDotPicture,nil,kFirstWindowOfClass);
	
	/* �R���g���[���̒l��ݒ� */
	SetDialogControlValue(dp,bgNone+dotPicturePrefs.dotPicBack,1);
	if (dotPicturePrefs.dotPicBack!=bgSource-bgNone)
		SetDialogControlHilite(dp,bgLighten,255);
	SetDialogControlValue(dp,bgLighten,dotPicturePrefs.dotPicLight ? 1 : 0);
	SetDialogControlValue(dp,dotMany+dotPicturePrefs.dotVolume,1);
	
	prevDotPrefs=dotPicturePrefs;
	
	SetDialogDefaultItem(dp,ok);
	SetDialogCancelItem(dp,cancel);
	ShowWindow(GetDialogWindow(dp));
	
	item=3;
	while (item!=ok && item!=cancel)
	{
		ModalDialog(mfUPP,&item);
		if (item>=bgNone && item <=bgSource)
		{
			if (item-bgNone==prevDotPrefs.dotPicBack) continue;
			SetDialogControlValue(dp,bgNone+prevDotPrefs.dotPicBack,0);
			prevDotPrefs.dotPicBack=item-bgNone;
			SetDialogControlValue(dp,item,1);
			
			SetDialogControlHilite(dp,bgLighten,((item==bgSource)?0:255));
		}
		else if (item==bgLighten)
		{
			prevDotPrefs.dotPicLight=!prevDotPrefs.dotPicLight;
			SetDialogControlValue(dp,bgLighten,prevDotPrefs.dotPicLight ? 1 : 0);
		}
		else if (item>=dotMany && item<=dotFewer)
		{
			if (item-dotMany==prevDotPrefs.dotVolume) continue;
			SetDialogControlValue(dp,dotMany+prevDotPrefs.dotVolume,0);
			prevDotPrefs.dotVolume=item-dotMany;
			SetDialogControlValue(dp,item,1);
		}
	}
	
	DisposeDialog(dp);
	DisposeModalFilterUPP(mfUPP);
	ActivateFloatersAndFirstDocumentWindow();
	
	if (item==ok)
	{
		dotPicturePrefs=prevDotPrefs;
		
		return noErr;
	}
	return userCanceledErr;
}

/* ���̐ݒ� */
short ConfigureWind(void)
{
	DialogPtr	dp;
	short	item;
	WindRec	prevWindPrefs;
	ModalFilterUPP	mfUPP=NewModalFilterUPP(MyModalDialogFilter);
	
	DeactivateFloatersAndFirstDocumentWindow();
	dp=GetNewDialog(dWind,nil,kFirstWindowOfClass);
	
	/* �R���g���[���̒l��ݒ� */
	SetDialogControlValue(dp,dRight+windPrefs.direction,1);
	SetDialogControlValue(dp,pWeak+windPrefs.power,1);
	
	prevWindPrefs=windPrefs;
	
	SetDialogDefaultItem(dp,ok);
	SetDialogCancelItem(dp,cancel);
	ShowWindow(GetDialogWindow(dp));
	
	item=3;
	while (item!=ok && item!=cancel)
	{
		ModalDialog(mfUPP,&item);
		switch (item)
		{
			case dRight:
			case dLeft:
				if (item-dRight==prevWindPrefs.direction) break;
				SetDialogControlValue(dp,dRight+prevWindPrefs.direction,0);
				prevWindPrefs.direction=item-dRight;
				SetDialogControlValue(dp,item,1);
				break;
			
			case pWeak:
			case pStrong:
				if (item-pWeak==prevWindPrefs.power) break;
				SetDialogControlValue(dp,pWeak+prevWindPrefs.power,0);
				prevWindPrefs.power=item-pWeak;
				SetDialogControlValue(dp,item,1);
				break;
		}
	}
	
	DisposeDialog(dp);
	DisposeModalFilterUPP(mfUPP);
	ActivateFloatersAndFirstDocumentWindow();
	
	if (item==ok)
	{
		windPrefs=prevWindPrefs;
		
		return noErr;
	}
	return userCanceledErr;
}

#define		dEdging	132
enum {
	diPixels=3,
	diTransparency,
	diKind,
};

/* �����̐ݒ� */
short ConfigureEdging(EdgePrefsRec *newPrefs)
{
	DialogPtr	dp;
	short		item;
	Str255		pixelsStr;
	ModalFilterUPP	mfUPP=NewModalFilterUPP(MyModalDialogFilter);
	OSErr		result=noErr;
	
	DeactivateFloatersAndFirstDocumentWindow();
	dp=GetNewDialog(dEdging,nil,kFirstWindowOfClass);
	
	/* �e�A�C�e���̓��e��ݒ� */
	NumToString(edgePrefs.pixels,pixelsStr);
	SetDialogItemText2(dp,diPixels,pixelsStr);
	if (gCurrentColor.isTransparent)
	{
		/* �O�i�F�������F�̏ꍇ�A�s�����x�̎w��͖����Ƃ��� */
		SetDialogControlValue(dp,diTransparency,1);
		SetDialogControlHilite(dp,diTransparency,255);
	}
	else
		SetDialogControlValue(dp,diTransparency,edgePrefs.transparency);
	SetDialogControlValue(dp,diKind,edgePrefs.kind);
	
	SetDialogDefaultItem(dp,ok);
	SetDialogCancelItem(dp,cancel);
	
	SelectDialogItemText(dp,diPixels,0,pixelsStr[0]);
	ShowWindow(GetDialogWindow(dp));
	
	item=3;
	while (item!=ok && item!=cancel)
		ModalDialog(mfUPP,&item);
	
	if (item==ok)
	{
		long	temp;
		
		GetDialogItemText2(dp,diPixels,pixelsStr);
		StringToNum(pixelsStr,&temp);
		if (temp < 0 || temp > 32767)
		{
			SysBeep(0);
			result=paramErr;
		}
		else
		{
			edgePrefs.pixels=newPrefs->pixels=temp;
			newPrefs->transparency=GetDialogControlValue(dp,diTransparency);
			if (!gCurrentColor.isTransparent)
				edgePrefs.transparency=newPrefs->transparency;
			edgePrefs.kind=newPrefs->kind=GetDialogControlValue(dp,diKind);
		}
	}
	else
		result=userCanceledErr;
	
	DisposeDialog(dp);
	DisposeModalFilterUPP(mfUPP);
	ActivateFloatersAndFirstDocumentWindow();
	
	return result;
}

/* �F���̕ύX�i����̂��H�@�Ǝv�������ǁA�f�B�U������ƌ��\���������j */
void ChangeColorMode(WindowPtr theWindow,short mode,Boolean dither)
{
	GWorldPtr	newModePtr;
	OSErr		err=noErr;
	CTabHandle	ctab=NULL;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	Rect	iconSize=eWinRec->iconSize;
	
	GetGWorld(&cPort,&cDevice);
	
	switch (mode)
	{
		case i216Color:
			/* 216�F�J���[�iweb�p�j */
			ctab=GetCTable(129);
			err=NewGWorld(&newModePtr,8,&iconSize,ctab,0,useTempMem);
			break;
		
		case iAppleIconColor:
			/* AppleIconColor�iIcon�p�j */
			ctab=GetCTable(130);
			err=NewGWorld(&newModePtr,8,&iconSize,ctab,0,useTempMem);
			break;
		
		case i16Color:
			err=NewGWorld(&newModePtr,4,&iconSize,0,0,useTempMem);
			break;
		
		case i16Gray:
			/* 16�F�O���[ */
			/* �J���[�e�[�u�����g�p */
			ctab=GetCTable(128);
			err=NewGWorld(&newModePtr,4,&iconSize,ctab,0,useTempMem);
			break;
		
		case iMonochrome:
			err=NewGWorld(&newModePtr,1,&iconSize,0,0,useTempMem);
			break;
	}
	
	if (err!=noErr)
	{
		ErrorAlertFromResource(EFFECTERR_RESID,EFFECTERR1);
		return;
	}
	
	GoOffPort(theWindow);
	SetGWorld(newModePtr,0);
	
	CopyBits(GetPortBitMapForCopyBits(eWinRec->tempDataPtr),
		GetPortBitMapForCopyBits(newModePtr),
		&iconSize,&iconSize,srcCopy+(dither ? ditherCopy : 0),0); /* �f�B�U�R�s�[���� */
	
	SetGWorld(eWinRec->editDataPtr,0);
	CopyBits(GetPortBitMapForCopyBits(newModePtr),
		GetPortBitMapForCopyBits(eWinRec->editDataPtr),
		&iconSize,&iconSize,srcCopy,0);
	
	if (mode==i16Gray || mode==i216Color || mode==iAppleIconColor)
		DisposeCTable(ctab); /* �J���[�e�[�u����j�� */
	
	DisposeGWorld(newModePtr);
	
	CopyRgn(eWinRec->selectionPos,eWinRec->updateRgn);
	
	SetGWorld(cPort,cDevice);
	DispOffPort(theWindow);
	
	gUsedCount.colorChangeNum++;
	SetUndoMode(umColorChange);
	if (gPaletteCheck == kPaletteCheckUsed) /* �p���b�g�̃`�F�b�N���A�b�v�f�[�g */
		UpdatePaletteCheck();
}


/* �ݒ�̓ǂݍ��� */
void LoadPaintPrefs(void)
{
	OSErr	err;
	
	err=LoadDataFromPrefs(&gAngle,sizeof(short),'pprf',128);
	err=LoadDataFromPrefs(&dotPicturePrefs,sizeof(DotPictureRec),'pprf',129);
	err=LoadDataFromPrefs(&windPrefs,sizeof(WindRec),'pprf',130);
	err=LoadDataFromPrefs(&typeStringPrefs,sizeof(TypeStringRec),'pprf',131);
	err=LoadDataFromPrefs(&edgePrefs,sizeof(EdgePrefsRec),'pprf',132);
}

/* �ݒ�̕ۑ� */
void SavePaintPrefs(void)
{
	OSErr	err;
	
	err=AddDataToPrefs(&gAngle,sizeof(short),'pprf',128,"\protate prefs");
	err=AddDataToPrefs(&dotPicturePrefs,sizeof(DotPictureRec),'pprf',129,"\pdot picture prefs");
	err=AddDataToPrefs(&windPrefs,sizeof(WindRec),'pprf',130,"\pwind prefs");
	err=AddDataToPrefs(&typeStringPrefs,sizeof(TypeStringRec),'pprf',131,"\ptype string prefs");
	err=AddDataToPrefs(&edgePrefs,sizeof(EdgePrefsRec),'pprf',132,"\pedge prefs");
}
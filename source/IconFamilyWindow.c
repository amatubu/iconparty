/* ------------------------------------------------------------ */
/*  IconFamilyWindow.c                                          */
/*     routines for IconFamily windows                          */
/*                                                              */
/*                 2001.9.25 - 2002.7.1  naoki iimura        	*/
/* ------------------------------------------------------------ */

/* includes */
#ifdef __APPLE_CC__
#include	<Carbon/Carbon.h>
#else
#endif

#include	"Definition.h"
#include	"WindowRoutines.h"
#include	"FileRoutines.h"
#include	"MenuRoutines.h"
#include	"IPIconSupport.h"
#include	"UsefulRoutines.h"
#include	"IconRoutines.h"
#include	"IconFamilyWindow.h"
#include	"IconParty.h"
#include	"PreCarbonSupport.h"
#include	"ExternalEditorSupport.h"
#include	"NavigationServicesSupport.h"
#include	"PaintRoutines.h"
#include	"EditRoutines.h"
#include	"IconListWindow.h"
#include	"WinIconSupport.h"
#include	"UpdateCursor.h"

#ifdef __APPLE_CC__
#include	"MoreFilesX.h"
#else
#include	"MoreFilesExtras.h"
#endif


/* ファミリウィンドウの処理 */
#if !TARGET_API_MAC_CARBON
static pascal void	ActivateFamilyWindow(WindowPtr fWindow,Boolean activateWindow);
#endif
static void	DisplayFamilyIcon(IconFamilyWinRec *fWinRec,short i);

static Handle	PictureToIcon(PicHandle picture,short iconKind);
static void	MyDrawPicture(PicHandle picture,const Rect *r);

#if 0
static void	InitIconFamilyWinRec(WindowPtr fWindow,FSSpec *theFile,OSType iconType,
	const IPIconRec *ipIcon,const StringPtr iconName);
#endif
static void	SetIconFamilyWinTitle(WindowPtr fWindow);


/* contextual menu */
static void MyIconFamilyContextMenu(Point globPt,WindowPtr fWindow);

/* drag & drop */
static pascal short	MyIconFamilyTrackingHandler(DragTrackingMessage theMessage,WindowPtr theWindow,
											void *handlerRefCon,DragReference theDrag);
static Boolean	IsMyIconFamilyTypeAvailable(DragReference theDrag);
static pascal short	MyIconFamilyReceiveHandler(WindowPtr theWindow,void *handlerRefCon,
											DragReference theDrag);
static void	MyDoStartDragIconFamily(WindowPtr fWindow,EventRecord *theEvent);
static OSErr	MyDoAddIconFamilyFlavors(WindowPtr fWindow,DragReference theDrag);
static OSErr	MyGetDragIconFamilyRegion(WindowPtr fWindow,RgnHandle dragRegion,
									DragReference theDragRef);
static pascal short	MySendIconFamilyDataProc(FlavorType theType,void *dragSendRefCon,
											ItemReference theItem,DragReference theDrag);


#define	ICONERR_RESID	4008
#define	ICONERR1	1
#define	ICONERR2	2
#define	ICONERR3	3
#define	ICONERR4	4
#define	ICONERR5	5
#define	ICONERR6	6


/* constants */
const short gIconSize[]={32,32,32,32,16,16,16,16,32,32,16,16,128,128};
const short gIconDepth[]={8,4,1,1,8,4,1,1,32,8,32,8,32,8};
const short gIconUseDepth[]={8,8,8,8,8,8,8,8,32,32,32,32,32,32};
const ResType gIconType[]={kLarge8BitData,kLarge4BitData,kLarge1BitMask,kLarge1BitMask,
								kSmall8BitData,kSmall4BitData,kSmall1BitMask,kSmall1BitMask,
								kLarge32BitData,kLarge8BitMask,kSmall32BitData,kSmall8BitMask,
								kThumbnail32BitData,kThumbnail8BitMask};

const static Rect	familyNameTERect={0x09,0x2d,0x15,0xb1};

const Rect	familyIconRect[] = {
						{0x24,0x0b,0x44,0x2b},{0x24,0x39,0x44,0x59},
						{0x24,0x67,0x44,0x87},{0x24,0x95,0x44,0xb5},
						{0x61,0x13,0x71,0x23},{0x61,0x41,0x71,0x51},
						{0x61,0x6f,0x71,0x7f},{0x61,0x9d,0x71,0xad},
						{0x24,0xcd,0x44,0xed},{0x24,0xfb,0x44,0x11b},
						{0x61,0xd5,0x71,0xe5},{0x61,0x103,0x71,0x113},
						{0x90,0x0b,0x110,0x8b},{0x90,0x9b,0x110,0x11b}};

/* macros */
#define	GetIconKindNum(fWinRec)	(8+(fWinRec->show32Icons ? 4+(isThumbnailIconsAvailable ? 2 : 0) : 0))
#define	GetIconDataSize(k)	gIconSize[k]*(long)gIconSize[k]/8*gIconDepth[k]* \
							((k==kL1Data || k==kL1Mask || k==kS1Data || k==kS1Mask) ? 2 : 1)


/* create a new IconFamily window */
void NewIconFamilyWindow(void)
{
	MyIconResRec		newIcon;
	IPIconSelector		selector;
	FSSpec				theFile;
	IconFamilyWinRec	*fWinRec;
	WindowPtr			fWindow;
	IPIconRec			ipIcon;
	Boolean				is32Exist;
	Point				windPos=kDefaultWinPos;
	Str255				name;
	OSErr				err;
	short				iconTemplate;
	
	/* kind of icon */
	newIcon.resID = kCustomIconResource; /* custom icon */
	GetIndString(newIcon.resName,147,1);
	newIcon.attrs = 0; /* no attributes */
	
	if (NewIconDialog(&newIcon,&selector,&iconTemplate,NULL) != noErr) return;
	
	err=GetIPIconFromTemplate(&ipIcon,&selector,iconTemplate);
	if (err != noErr)
	{
		SysBeep(0);
		return;
	}
	
	err=IPIconHas32Icons(&ipIcon,&is32Exist);
	if (err!=noErr) {
		SysBeep(0);
		return;
	}
	
	if (!isIconServicesAvailable)
	{
		IconActionUPP	detachIconUPP;
		
		/* detach icons from resource */
		detachIconUPP=NewIconActionUPP(DetachIcon);
		err=ForEachIconDo(ipIcon.iconSuite,GetMySelector(),detachIconUPP,nil);
		DisposeIconActionUPP(detachIconUPP);
	}
	
	GetIndString(theFile.name,141,1);
	GetIndString(name,147,1);
	
	/* create a new window */
	fWindow = MakeFamilyWindow(windPos,is32Exist,&theFile,0L,&ipIcon,name);
	if (fWindow == NULL)
	{
		DisposeIPIcon(&ipIcon);
		SysBeep(0);
		return;
	}
	
	fWinRec=GetIconFamilyRec(fWindow);
	fWinRec->isSaved = false; /* not saved */
	UpdateSaveMenu();
}

/* open icon of folder */
void OpenFolderIcon(FSSpec *theFolderSpec,Boolean isFolder)
{
	WindowPtr	fWindow;
	IconFamilyWinRec	*fWinRec;
	IconListWinRec	*iWinRec;
	
	OSErr			err;
	IPIconRec		ipIcon;
	Boolean			is32Exist=false;		/* true, if 32bits icon exists */
	short			alertMode=0;			/* アイコン編集に関してなんらかのアラートを出す必要がある */
	Str255			name;
	
	Point			windPos=kDefaultWinPos;
	
	/* 開こうとしているアイコンがすでに開かれていないかどうかをチェック */
	fWindow=MyFrontNonFloatingWindow();
	while (fWindow!=nil)
	{
		if (GetExtWindowKind(fWindow)==kWindowTypeIconFamilyWindow)
		{
			fWinRec=GetIconFamilyRec(fWindow);
			if (fWinRec->parentWindow==nil && EqualFile(&fWinRec->theIconSpec,theFolderSpec)) break;
		}
		else if (!isFolder && GetExtWindowKind(fWindow)==kWindowTypeIconListWindow)
		{
			iWinRec=GetIconListRec(fWindow);
			if (EqualFile(&iWinRec->iconFileSpec,theFolderSpec)) break;
		}
		fWindow=GetNextVisibleWindow(fWindow);
	}
	if (fWindow!=nil) /* already opened */
	{
		SelectReferencedWindow(fWindow);
		UpdateMenus();
		return;
	}
	
	/* get IPIcon from file(folder) */
	err=GetFileIPIcon(theFolderSpec,&ipIcon,isFolder,&is32Exist,&alertMode);
	if (err!=noErr)
	{
		ErrorAlertFromResource(ICONERR_RESID,ICONERR2);
		return;
	}
	if (alertMode)
	{
		Str255	message;
		
		switch (alertMode)
		{
			case kDeleteIconFamilyMode:
				GetIndString(message,149,2);
				break;
			
			case kAddResourceForkMode:
				GetIndString(message,156,1);
				break;
			
			case kDeleteIconMode:
				GetIndString(message,149,1);
				break;
		}
		if (MyConfirmDialog(message,NULL,NULL)==noErr) alertMode = 0;
	}
	if (alertMode)
	{
		err=DisposeIPIcon(&ipIcon);
		return;
	}
	
	/* create IconFamily window */
	UseResFile(gApplRefNum);
	GetIndString(name,147,1);
	fWindow=MakeFamilyWindow(windPos,is32Exist,theFolderSpec,kFolderIconType,&ipIcon,name);
	if (fWindow==nil) return;
	
	fWinRec=GetIconFamilyRec(fWindow);
	fWinRec->isFolder=isFolder;
}

/* open icns(icons for MacOS X) */
void OpenXIconFile(FSSpec *theFile)
{
	WindowPtr	fWindow;
	IconFamilyWinRec	*fWinRec;
	IPIconRec	ipIcon;
	
	OSErr		err;
	
	Point			windPos=kDefaultWinPos;
	Boolean			is32Exist;
	
	if (!isIconServicesAvailable) return;
	
	/* 開こうとしているアイコンがすでに開かれていないかどうかをチェック */
	fWindow=MyFrontNonFloatingWindow();
	while (fWindow!=nil)
	{
		if (GetExtWindowKind(fWindow)==kWindowTypeIconFamilyWindow)
		{
			fWinRec=GetIconFamilyRec(fWindow);
			if (fWinRec->parentWindow==nil && EqualFile(&fWinRec->theIconSpec,theFile)) break;
		}
		fWindow=GetNextVisibleWindow(fWindow);
	}
	if (fWindow!=nil)
	{
		SelectReferencedWindow(fWindow);
		UpdateMenus();
		return;
	}
	
	/* アイコンを開く */
	err=XIconToIPIcon(theFile,&ipIcon);
	if (err!=noErr)
	{
		ErrorAlertFromResource(ICONERR_RESID,ICONERR1);
		return;
	}
		
	err=IPIconHas32Icons(&ipIcon,&is32Exist);
	
	/* ファミリウィンドウを作成 */
	UseResFile(gApplRefNum);
	fWindow=MakeFamilyWindow(windPos,is32Exist,theFile,kXIconFileType,&ipIcon,theFile->name);
	if (fWindow==nil)
	{
		DisposeIPIcon(&ipIcon);
		
		ErrorAlertFromResource(ICONERR_RESID,ICONERR4);
		return;
	};
}

OSErr XIconToIconSuite(FSSpec *theFile,IconSuiteRef *iconSuite)
{
	OSErr	err;
	IconFamilyHandle	theIconFamily;
	short		refNum;
	long		count;
	
	if (gSystemVersion >= 0x0850)
	{
		/* アイコンを開く */
		err=FSpOpenDF(theFile,fsRdPerm,&refNum);
		if (err!=noErr) return err;
		
		err=GetEOF(refNum,&count);
		if (err!=noErr)
		{
			FSClose(refNum);
			return err;
		}
		theIconFamily=(IconFamilyHandle)NewHandle(count);
		HLock((Handle)theIconFamily);
		err=FSRead(refNum,&count,*theIconFamily);
		HUnlock((Handle)theIconFamily);
		
		err=FSClose(refNum);
		
		err=IconFamilyToIconSuite(theIconFamily,GetMySelector(),iconSuite);
		DisposeHandle((Handle)theIconFamily);
		return err;
	}
	else
		return -1;
}

/* icoを開く */
void OpenWinIconFile(FSSpec *theFile)
{
	WindowPtr	fWindow;
	IconFamilyWinRec	*fWinRec;
	
	OSErr		err;
	IPIconRec	ipIcon;
	
	Point			windPos=kDefaultWinPos;
	
	/* 開こうとしているアイコンがすでに開かれていないかどうかをチェック */
	fWindow=MyFrontNonFloatingWindow();
	while (fWindow!=nil)
	{
		if (GetExtWindowKind(fWindow)==kWindowTypeIconFamilyWindow)
		{
			fWinRec=GetIconFamilyRec(fWindow);
			if (fWinRec->parentWindow==nil && EqualFile(&fWinRec->theIconSpec,theFile)) break;
		}
		fWindow=GetNextVisibleWindow(fWindow);
	}
	if (fWindow!=nil)
	{
		SelectReferencedWindow(fWindow);
		UpdateMenus();
		return;
	}
	
	/* アイコンを開く */
	err=WinIconToIconSuite(theFile,&ipIcon.iconSuite);
	if (err!=noErr) return;
	ipIcon.it32Data = ipIcon.t8mkData = NULL;
	
	/* ファミリウィンドウを作成 */
	UseResFile(gApplRefNum);
	fWindow=MakeFamilyWindow(windPos,false,theFile,kWinIconType,&ipIcon,theFile->name);
//	if (fWindow==nil) return;
	
//	InitIconFamilyWinRec(fWindow,theFile,kWinIconType,&ipIcon,theFile->name);
}

/* アイコンファミリウィンドウを作成 */
WindowPtr MakeFamilyWindow(Point windPos,Boolean is32Exist,FSSpec *theFile,
	OSType iconType,const IPIconRec *ipIcon,const StringPtr iconName)
{
	IconFamilyWinRec	**fWinHandle,*fWinRec;
	WindowPtr			fWindow;
	OSErr				err;
	Point				windOffset=kWinOffset;
	Rect				validRect,tempRect;
	#if !TARGET_API_MAC_CARBON
	ActivateHandlerUPP	activateUPP;
	#endif
	long	total,contig;
	
	TEHandle	hTE;
	OSType			supType[1];
	TSMDocumentID	docID;
	TSMTERecHandle	docTSMRecH;
	
	/* 0.90b7変更部分 */
	/* メモリチェック。2000はアイコン領域、1024はその他 */
	PurgeSpace(&total,&contig);
	if (total < sizeof(IconFamilyWinRec)+2000+1024+512) return 0L;
	
	UseResFile(gApplRefNum);
	
	err=GetNewWindowReference(&fWindow,kIconFamilyWindowResID,kFirstWindowOfClass,nil);
	if (err!=noErr || fWindow==nil) return nil;
	
	SetExtWindowKind(fWindow,kWindowTypeIconFamilyWindow);
	
	fWinHandle=(IconFamilyWinRec **)NewHandle(sizeof(IconFamilyWinRec));
	if (fWinHandle == nil)
	{
		DisposeWindowReference(fWindow);
		return nil;
	}
	
	HLockHi((Handle)fWinHandle);
	SetExtWRefCon(fWindow,(long)fWinHandle);
	fWinRec=*fWinHandle;
	
	/* アクティベートルーチンの設定 */
	#if !TARGET_API_MAC_CARBON
	activateUPP=NewActivateHandlerProc((ProcPtr)ActivateFamilyWindow);
	SetActivateHandlerProc(fWindow,activateUPP);
	fWinRec->dragHandlers.activateUPP=activateUPP;
	#endif
	
	/* ドラッグ受け入れルーチンの設定 */
	if (isDragMgrPresent)
	{
		DragTrackingHandlerUPP	dragTrackUPP;
		DragReceiveHandlerUPP	dragReceiveUPP;
		
		dragTrackUPP=NewDragTrackingHandlerUPP(MyIconFamilyTrackingHandler);
		if (dragTrackUPP != nil)
		{
			MyDragGlobalsRec	*dragGlobals;
			
			dragGlobals=&fWinRec->dragHandlers.dragGlobals;
			dragGlobals->canAcceptDrag=false;
			dragGlobals->isInContent=false;
			
			err=InstallTrackingHandler(dragTrackUPP,fWindow,dragGlobals);
			fWinRec->dragHandlers.trackUPP=dragTrackUPP;
			dragReceiveUPP=NewDragReceiveHandlerUPP(MyIconFamilyReceiveHandler);
			if (dragReceiveUPP != nil)
			{
				err=InstallReceiveHandler(dragReceiveUPP,fWindow,dragGlobals);
				fWinRec->dragHandlers.receiveUPP=dragReceiveUPP;
			}
		}
	}
	
	/* 初期設定 */
	fWinRec->selectedIcon=-1;
	fWinRec->wasChanged=false;
	fWinRec->show32Icons=false;
	
	fWinRec->undoData.undoMode=umCannot;
	fWinRec->undoData.iconKind=-1;
	fWinRec->undoData.iconData=nil;
	
	fWinRec->largeWindowWidth = kFamilyWindowSmallWidth;
	fWinRec->largeWindowHeight = kFamilyWindowSmallHeight;
	
	/* ウィンドウの大きさ */
	if (isIconServicesAvailable)
	{
		/* 編集するアイコンの種類によって場合わけ */
		switch (iconType)
		{
			case kFolderIconType: /* アイコン付きフォルダ */
			case kXIconFileType: /* X用アイコン */
			case 0L:
				if (isIconServicesAvailable)
				{
					if (isThumbnailIconsAvailable)
						fWinRec->largeWindowHeight = kFamilyWindowThumbnailHeight;
					else
						fWinRec->largeWindowHeight = kFamilyWindowLargeHeight;
					fWinRec->largeWindowWidth = kFamilyWindowLargeWidth;
				}
				break;
			
			case kWinIconType: /* Windows用アイコン */
				break;
			
			case kLarge1BitMask:
				if (isIconServicesAvailable)
				{
					fWinRec->largeWindowHeight = kFamilyWindowLargeHeight;
					fWinRec->largeWindowWidth = kFamilyWindowLargeWidth;
				}
				break;
		}
		
		if (is32Exist)
		{
			fWinRec->show32Icons=true;
			
			SizeWindow(fWindow,fWinRec->largeWindowWidth,fWinRec->largeWindowHeight,true);
			GetWindowPortBounds(fWindow,&tempRect);
			SetPortWindowPort(fWindow);
			ClipRect(&tempRect);
		}
	}
	
	/* ウィンドウの表示可能な位置 */
	GetRegionBounds(GetGrayRgn(),&validRect);
	validRect.left=windPos.h;
	validRect.top=windPos.v;
	GetWindowPortBounds(fWindow,&tempRect);
	validRect.right-=tempRect.right;
	validRect.bottom-=tempRect.bottom;
	
	StackWindowPos(&windPos,windOffset,&validRect);
	MoveWindow(fWindow,windPos.h,windPos.v,false);
	
	/* 名前のTE */
	SetPortWindowPort(fWindow);
	TextSize(9);
	hTE=TENew(&familyNameTERect,&familyNameTERect);
	(**hTE).txSize=9;
	TEFeatureFlag(teFOutlineHilite,TEBitSet,hTE);
	TEAutoView(true,hTE);
	fWinRec->iconNameTE=hTE;
	
	/* インライン入力 */
	if (isTSMTEAvailable)
	{
		TEFeatureFlag(teFInlineInput,TEBitSet,hTE);
		
		supType[0]=kTSMTEInterfaceType;
		err=NewTSMDocument(1,supType,&docID,(long)&docTSMRecH);
		fWinRec->docID=docID;
		fWinRec->docTSMRecH=docTSMRecH;
		(**docTSMRecH).textH=hTE;
		(**docTSMRecH).preUpdateProc=nil;
		(**docTSMRecH).postUpdateProc=nil;
		(**docTSMRecH).updateFlag=0;
		(**docTSMRecH).refCon=0;
	}
	
	fWinRec->parentWindow = NULL;
	if (theFile != NULL)
		fWinRec->theIconSpec=*theFile;
	else
		PStrCpy(iconName,fWinRec->theIconSpec.name);
	fWinRec->iconType=iconType;
	fWinRec->isSaved=true;
	fWinRec->isFolder=false;
	fWinRec->ipIcon=*ipIcon;
	fWinRec->iconID=kCustomIconResource;
	PStrCpy(iconName,fWinRec->iconName);
	
	/* 名前を入れ、選択 */
	TEActivate(fWinRec->iconNameTE);
	if (isTSMTEAvailable)
		ActivateTSMDocument(fWinRec->docID);
	TESetText(&fWinRec->iconName[1],fWinRec->iconName[0],fWinRec->iconNameTE);
	TESetSelect(0,fWinRec->iconName[0],fWinRec->iconNameTE);
	
	/* タイトルの設定 */
	SetIconFamilyWinTitle(fWindow);
	
	ShowReferencedWindow(fWindow);
	
	UpdateMenus();
	
	return fWindow;
}

#if !TARGET_API_MAC_CARBON
/* アイコンファミリウィンドウのアクティベートルーチン */
pascal void	ActivateFamilyWindow(WindowPtr fWindow,Boolean activateWindow)
{
	IconFamilyWinRec	*fWinRec=GetIconFamilyRec(fWindow);
	GrafPtr	port;
	
	GetPort(&port);
	SetPortWindowPort(fWindow);
	
	if (activateWindow)
	{
		/* アクティベートの時はTEがアクティブだった時だけアクティベート */
		if (fWinRec->wasNameTEActive)
		{
			TEActivate(fWinRec->iconNameTE);
			if (isTSMTEAvailable)
				ActivateTSMDocument(fWinRec->docID);
		}
	}
	else
	{
		/* デアクティベートの時はTEがアクティブだったかどうかを記録 */
		fWinRec->wasNameTEActive=((**fWinRec->iconNameTE).active !=0);
		TEDeactivate(fWinRec->iconNameTE);
		if (isTSMTEAvailable)
			DeactivateTSMDocument(fWinRec->docID);
	}
}
#endif

#if 0
/* アイコンファミリウィンドウ構造体の初期化 */
void InitIconFamilyWinRec(WindowPtr fWindow,FSSpec *theFile,OSType iconType,
	const IPIconRec *ipIcon,const StringPtr iconName)
{
	IconFamilyWinRec	*fWinRec=GetIconFamilyRec(fWindow);
	
	fWinRec->parentWindow = NULL;
	fWinRec->theIconSpec=*theFile;
	fWinRec->iconType=iconType;
	fWinRec->isSaved=true;
	fWinRec->isFolder=false;
	fWinRec->ipIcon=*ipIcon;
	fWinRec->iconID=kCustomIconResource;
	PStrCpy(iconName,fWinRec->iconName);
	
	/* 名前を入れ、選択 */
	TEActivate(fWinRec->iconNameTE);
	if (isTSMTEAvailable)
		ActivateTSMDocument(fWinRec->docID);
	TESetText(&fWinRec->iconName[1],fWinRec->iconName[0],fWinRec->iconNameTE);
	TESetSelect(0,fWinRec->iconName[0],fWinRec->iconNameTE);
	
	/* タイトルの設定 */
	SetIconFamilyWinTitle(fWindow);
	
	ShowReferencedWindow(fWindow);
	
	UpdateMenus();
}
#endif

/* アイコンファミリウィンドウのタイトル */
void SetIconFamilyWinTitle(WindowPtr fWindow)
{
	IconFamilyWinRec	*fWinRec=GetIconFamilyRec(fWindow);
	Str255	title;
	
	switch (fWinRec->iconType)
	{
		case kFolderIconType: /* アイコン付きフォルダ */
			GetIndString(title,sFamilyWinTitle,3);
			PStrCat(fWinRec->theIconSpec.name,title);
			SetWTitle(fWindow,title);
			break;
		
		case kXIconFileType: /* X用アイコン or IconFamily */
		case kWinIconType: /* Win用アイコン */
		case kLarge1BitMask: 
			SetWTitle(fWindow,fWinRec->theIconSpec.name);
			break;
		
		default:
			GetIndString(title,sFamilyWinTitle,4);
			SetWTitle(fWindow,title);
	}
}	

/* アイコンファミリウィンドウを閉じる */
OSErr CloseFamilyWindow(WindowPtr theWindow,Boolean saveFlag,Boolean deleteFlag)
{
	IconFamilyWinRec	**fWinRec;
	OSErr	err=noErr;
	WindowPtr	eWindow,nextWindow;
	PaintWinRec	*eWinRec;
	Str255	newName;
	Handle	newNameH;
	short	l;
	
	if (theWindow==nil) return noErr;
	
	/* このウィンドウを親に持つペイントウィンドウがないかどうかをチェック */
	eWindow=MyFrontNonFloatingWindow();
	while (eWindow != nil)
	{
		nextWindow=GetNextVisibleWindow(eWindow);
		if (GetExtWindowKind(eWindow) == kWindowTypePaintWindow)
		{
			eWinRec=GetPaintWinRec(eWindow);
			if (eWinRec->iconType.fileType == kIconFamilyType && eWinRec->parentWindow == theWindow)
			{
				SelectReferencedWindow(eWindow);
				if (deleteFlag)
				{
					if (eWinRec->iconHasChanged)
					{
						SysBeep(0);
						return userCanceledErr;
					}
				}
				err=ClosePaintWindow(eWindow,false);
				if (err != noErr)
					return err;
			}
		}
		eWindow=nextWindow;
	}
	
	if (CloseXIcon(theWindow)!=noErr)
		ErrorAlertFromResource(ICONERR_RESID,ICONERR6);
	
	fWinRec=(IconFamilyWinRec **)GetExtWRefCon(theWindow);
	
	if (saveFlag)
	{
		if ((**fWinRec).parentWindow!=nil)
		{
			/* TEの内容を取り出す */
			newNameH=TEGetText((**fWinRec).iconNameTE);
			l=(**(**fWinRec).iconNameTE).teLength;
			if (l>255) l=255;
			newName[0]=l;
			BlockMoveData(*newNameH,&newName[1],l);
			
			if (!EqualString((**fWinRec).iconName,newName,true,true))
			{
				PStrCpy(newName,(**fWinRec).iconName);
				(**fWinRec).wasChanged=true;
			}
		}
		
		if ((**fWinRec).wasChanged)
		{
			if ((**fWinRec).parentWindow==nil)
			{
				short	item;
				
				if (isNavServicesAvailable && useNavigationServices)
				{
					SuspendFloatingWindows();
					item=AskSaveWithNav((**fWinRec).theIconSpec.name,kNavSaveChangesClosingDocument);
					ResumeFloatingWindows();
				}
				else
				{
					Str255	kind;
					
					GetIndString(kind,133,6);
					item=SaveYN(kind,(**fWinRec).theIconSpec.name,kSaveChangedCloseWindow);
				}
				
				/* 保存確認と保存ルーチン */
				switch (item)
				{
					case ok:
						SaveFamilyIcon(theWindow,false,false);
						err=noErr;
						break;
					
					case cancel:
						err=userCanceledErr;
						break;
					
					case kDialogItemDontSave:
						err=noErr;
						break;
				}
			}
			else
			{
				SaveFamilyIcon(theWindow,false,false);
				err=noErr;
			}
		}
	}
	
	if (err == noErr)
	{
		/* 取り消し用ハンドルの破棄 */
		DisposeHandle((**fWinRec).undoData.iconData);
		
		/* ドラッグ受け入れルーチンの削除 */
		if (isDragMgrPresent)
		{
			err=RemoveTrackingHandler((**fWinRec).dragHandlers.trackUPP,theWindow);
			DisposeDragTrackingHandlerUPP((**fWinRec).dragHandlers.trackUPP);
			err=RemoveReceiveHandler((**fWinRec).dragHandlers.receiveUPP,theWindow);
			DisposeDragReceiveHandlerUPP((**fWinRec).dragHandlers.receiveUPP);
		}
		#if !TARGET_API_MAC_CARBON
		DisposeRoutineDescriptor((**fWinRec).dragHandlers.activateUPP);
		#endif
		
		DisposeIPIcon(&(*fWinRec)->ipIcon);
		
		if (isTSMTEAvailable)
			err=DeleteTSMDocument((**fWinRec).docID);
		TEDispose((**fWinRec).iconNameTE);
		HUnlock((Handle)fWinRec);
		DisposeHandle((Handle)fWinRec);
		
		DisposeWindowReference(theWindow);
	}
	return err;
}

/* アイコンファミリウィンドウのアップデート */
void UpdateFamilyWindow(WindowPtr theWindow)
{
	IconFamilyWinRec	*fWinRec;
	PicHandle	backPic;
	short		i;
    Rect        picRect;
	
	if (theWindow==nil) return;
	
	fWinRec=GetIconFamilyRec(theWindow);
	
	UseResFile(gApplRefNum);
	backPic=GetPicture(kIconFamilyPictureResID);
    QDGetPictureBounds(backPic, &picRect);
	DrawPicture(backPic,&picRect);
	
	for (i=0; i<GetIconKindNum(fWinRec); i++)
		DisplayFamilyIcon(fWinRec,i);
	
	TEUpdate(&familyNameTERect,fWinRec->iconNameTE);
	
	UseResFile(gApplRefNum);
}

/* ファミリウィンドウのクリック処理 */
/* mousePtはローカル座標 */
void ClickFamilyWindow(WindowPtr fWindow,Point mousePt,EventRecord *theEvent)
{
	IconFamilyWinRec	*fWinRec=GetIconFamilyRec(fWindow);
	short	i,iconKind=-1;
	short	prevSelection=fWinRec->selectedIcon;
	Boolean	shiftDown=((theEvent->modifiers & shiftKey) != 0);
	UInt32	time;
	Point	pt;
	Rect	dRect={-2,-2,3,3}; /* ドラッグの判定用 */
	
	SetPortWindowPort(fWindow);
	
	OffsetRect(&dRect,mousePt.h,mousePt.v);
	
	/* クリックされたアイコンを調べる */
	for (i=0; i<GetIconKindNum(fWinRec); i++)
	{
		if (PtInRect(mousePt,&familyIconRect[i]))
		{
			iconKind=i;
			break;
		}
	}
	
	if (iconKind >= 0)
	{
		/* ポップアップメニュー判定 */
		if ((theEvent->modifiers & controlKey) != 0)
		{
			SetPortWindowPort(fWindow);
			fWinRec->selectedIcon=iconKind;
			DisplayFamilyIcon(fWinRec,iconKind);
			DisplayFamilyIcon(fWinRec,prevSelection);
			pt=mousePt;
			LocalToGlobal(&pt);
			MyIconFamilyContextMenu(pt,fWindow);
			return;
		}
		
		time=TickCount()+GetDblTime();
		/* ポップアップメニュー判定 */
		if (isDragMgrPresent || MyFrontNonFloatingWindow()==fWindow)
		{
			GetMouse(&pt);
			while (StillDown() && PtInRect(pt,&dRect))
			{
				GetMouse(&pt);
				if (TickCount() > time)
				{
					SetPortWindowPort(fWindow);
					fWinRec->selectedIcon=iconKind;
					DisplayFamilyIcon(fWinRec,iconKind);
					DisplayFamilyIcon(fWinRec,prevSelection);
					LocalToGlobal(&pt);
					MyIconFamilyContextMenu(pt,fWindow);
					return;
				}
			}
		}
		/* ドラッグ判定 */
		if (isDragMgrPresent)
		{
			if (StillDown())
			{
				if (fWinRec->selectedIcon != iconKind)
				{
					SetPortWindowPort(fWindow);
					fWinRec->selectedIcon=iconKind;
					DisplayFamilyIcon(fWinRec,iconKind);
					DisplayFamilyIcon(fWinRec,prevSelection);
				}
				/* ドラッグ処理 */
				MyDoStartDragIconFamily(fWindow,theEvent);
				return;
			}
		}
		else
		{
			GetMouse(&pt);
			while (StillDown()) ;
		}
		
		/* フロントウィンドウでなかった時は選択して終わり */
		if (MyFrontNonFloatingWindow()!=fWindow)
		{
			SelectReferencedWindow(fWindow);
			UpdateMenus();
			return;
		}
		
		/* 選択アイコンが変更されたらハイライトを更新 */
		if (fWinRec->selectedIcon != iconKind)
		{
			SetPortWindowPort(fWindow);
			fWinRec->selectedIcon=iconKind;
			DisplayFamilyIcon(fWinRec,iconKind);
			DisplayFamilyIcon(fWinRec,prevSelection);
			
			SetPortWindowPort(gPreviewWindow);
			UpdateIconPreview(fWindow);
			SetPortWindowPort(fWindow);
		}
		
		/* ダブルクリック判定 */
		while (TickCount() < time && PtInRect(pt,&dRect))
		{
			EventRecord	followEvent;
			
			GetMouse(&pt);
			if (EventAvail(mDownMask,&followEvent))
			{
				FlushEvents(mDownMask+mUpMask,0);
				
				/* アイコン編集 */
				EditFamilyIcon(fWindow,kForceNone);
				return;
			}
		}
		if (prevSelection != fWinRec->selectedIcon)
		{
			UpdateClipMenu();
			UpdatePasteMenu();
			UpdateIconMenu();
		}
		FlushEvents(mDownMask,0);
	}
	else
	{
		if (MyFrontNonFloatingWindow()!=fWindow)
		{
			SelectReferencedWindow(fWindow);
			UpdateMenus();
			return;
		}
	}
	
	/* TEの内部かどうかをチェック */
	if (PtInRect(mousePt,&familyNameTERect))
	{
		if (!(**fWinRec->iconNameTE).active)
		{
			TEActivate(fWinRec->iconNameTE);
			if (isTSMTEAvailable)
				ActivateTSMDocument(fWinRec->docID);
		}
		TEClick(mousePt,shiftDown,fWinRec->iconNameTE);
		
		/* クリップボードメニュー更新のため、マイナスの値
		   ハイライト更新のため、-1と違う値 */
		fWinRec->selectedIcon = -2;
		
		UpdateClipMenu();
		UpdatePasteMenu();
		UpdateIconMenu();
	}
	else
	{
		/* 内部でなければ、TEをディアクティベート */
		if ((**fWinRec->iconNameTE).active)
		{
			TEDeactivate(fWinRec->iconNameTE);
			if (isTSMTEAvailable)
				DeactivateTSMDocument(fWinRec->docID);
		}
	}
	
	/* 選択アイコンが変更されたらハイライトを更新 */
	if (fWinRec->selectedIcon != iconKind)
	{
		SetPortWindowPort(fWindow);
		fWinRec->selectedIcon=iconKind;
		DisplayFamilyIcon(fWinRec,iconKind);
		DisplayFamilyIcon(fWinRec,prevSelection);
	}
}

/* 次のアイコンの選択 */
void SelectNextIcon(IconFamilyWinRec *fWinRec,Boolean shiftDown)
{
	short	prevSelectedIcon=fWinRec->selectedIcon;
	
	if (prevSelectedIcon < 0)
	{
		if (!(**fWinRec->iconNameTE).active)
		{
			TEActivate(fWinRec->iconNameTE);
			if (isTSMTEAvailable)
				ActivateTSMDocument(fWinRec->docID);
			TESetSelect(0,(**fWinRec->iconNameTE).teLength,fWinRec->iconNameTE);
			TEUpdate(&familyNameTERect,fWinRec->iconNameTE);
		}
		else
		{
			TEDeactivate(fWinRec->iconNameTE);
			if (isTSMTEAvailable)
				DeactivateTSMDocument(fWinRec->docID);
			TEUpdate(&familyNameTERect,fWinRec->iconNameTE);
			
			fWinRec->selectedIcon=(shiftDown ? GetIconKindNum(fWinRec)-1 : kL8Data);
			DisplayFamilyIcon(fWinRec,fWinRec->selectedIcon);
		}
	}
	else
	{
		fWinRec->selectedIcon+=(shiftDown ? -1 : 1);
		if (fWinRec->selectedIcon >= GetIconKindNum(fWinRec)) fWinRec->selectedIcon=-1;
		if (fWinRec->selectedIcon < 0)
		{
			TEActivate(fWinRec->iconNameTE);
			if (isTSMTEAvailable)
				ActivateTSMDocument(fWinRec->docID);
			TESetSelect(0,(**fWinRec->iconNameTE).teLength,fWinRec->iconNameTE);
			TEUpdate(&familyNameTERect,fWinRec->iconNameTE);
		}
		else
		{
			DisplayFamilyIcon(fWinRec,fWinRec->selectedIcon);
		}
		DisplayFamilyIcon(fWinRec,prevSelectedIcon);
	}
	UpdateClipMenu();
	UpdatePasteMenu();
	UpdateIconMenu();
	RedrawIconPreview();
}

/* 選択アイコンの変更 */
void SelectNextIcon2(IconFamilyWinRec *fWinRec,short dx,short dy)
{
	short	prevSelectedIcon=fWinRec->selectedIcon;
	short	newSelectedIcon=fWinRec->selectedIcon;
	
	if (prevSelectedIcon < 0) /* アイコンが選択されていなければ何もしない */
		return;
	
	newSelectedIcon += dx + dy*4;
	/* 範囲外の場合は何もしない */
	if (newSelectedIcon < kL8Data || newSelectedIcon >= GetIconKindNum(fWinRec)) return;
	
	fWinRec->selectedIcon = newSelectedIcon;
	DisplayFamilyIcon(fWinRec,fWinRec->selectedIcon);
	DisplayFamilyIcon(fWinRec,prevSelectedIcon);
	
	UpdateClipMenu();
	UpdateIconMenu();
	RedrawIconPreview();
}

/* アイコンデータの表示（選択されていれば枠も） */
void DisplayFamilyIcon(IconFamilyWinRec *fWinRec,short i)
{
	PicHandle	iconPic;
	Rect	iconRect;
	
	if (i<0) return;
	
	iconRect=familyIconRect[i];
	InsetRect(&iconRect,-2,-2);
	if (i != fWinRec->selectedIcon)
		EraseRect(&iconRect);
	
	iconPic=IPIconToPicture(&fWinRec->ipIcon,i);
	if (iconPic != NULL)
	{
		MyDrawPicture(iconPic,&familyIconRect[i]);
		KillPicture(iconPic);
	}
	else
	{
		Pattern	pat;
		
		GetQDGlobalsGray(&pat);
		PenPat(&pat);
		PaintRect(&iconRect);
		PenNormal();
	}
	
	if (i == fWinRec->selectedIcon)
		FrameRect(&iconRect);
}

/* アイコンデータの保存 */
/* saveToFileは、リストウィンドウの子ウィンドウのときのみ有効 */
/* saveAsFlagは、名前を付けて保存のとき（親がいるときは独立させる） */
void SaveFamilyIcon(WindowPtr fWindow,Boolean saveToFile,Boolean saveAsFlag)
{
	OSErr	err=noErr;
	IconFamilyWinRec	*fWinRec=GetIconFamilyRec(fWindow);
	
	if (fWinRec->parentWindow==nil || saveAsFlag)
	{
		/* 親のリストウィンドウがない→独立したファミリウィンドウ */
		NavReplyRecord	theReply;
		Boolean			newSaveFlag=false;
		
		if (!fWinRec->isSaved || saveAsFlag) /* 未保存か、名前を付けて保存 */
		{
			err=SaveAsIconDialog(fWinRec->theIconSpec.name,
				&fWinRec->theIconSpec,&fWinRec->iconType,&theReply);
			if (err!=noErr) return;
			
			newSaveFlag = true;
			fWinRec->isFolder = (fWinRec->iconType == kFolderIconType);
			
			/* 親がいるときは独立させる */
			if (saveAsFlag)
				fWinRec->parentWindow = nil;
		}
		
		/* 実際の保存 */
		if (fWinRec->isFolder)
			err=MakeFolderWithIPIcon(&fWinRec->theIconSpec,&fWinRec->ipIcon);
		else
		{
			switch (fWinRec->iconType)
			{
				case kFolderIconType:
					err=MakeFileWithIPIcon(&fWinRec->theIconSpec,&fWinRec->ipIcon);
					break;
				
				case kWinIconType:
					err=MakeWinIconFromSuite(&fWinRec->theIconSpec,fWinRec->ipIcon.iconSuite);
					break;
				
				case kXIconFileType:
					err=MakeXIconWithIPIcon(&fWinRec->theIconSpec,&fWinRec->ipIcon);
					break;
			}
		}
		
		/* 新規保存でNavigation Services使用時はCompleteSaveを呼ぶ */
		if (newSaveFlag && isNavServicesAvailable && useNavigationServices)
		{
			OSErr dummy;
			if (theReply.validRecord && err == noErr)
				err = NavCompleteSave(&theReply,kNavTranslateInPlace);
			dummy = NavDisposeReply(&theReply);
		}
		
		if (err!=noErr)
			ErrorAlertFromResource(ICONERR_RESID,ICONERR5);
		else
		{
			/* エラーなく保存できれば変更フラグをリセットし、保存済みフラグをたてる */
			fWinRec->wasChanged=false;
			fWinRec->isSaved=true;
		}
		
		/* 新しく保存された場合は、タイトルバーを修正 */
		if (newSaveFlag)
			SetIconFamilyWinTitle(fWindow);
	}
	else
	{
		MyIconResRec	newIcon;
		WindowPtr		iconWindow=fWinRec->parentWindow;
		IconListWinRec	*iWinRec=GetIconListRec(iconWindow);
		GrafPtr			port;
		IconActionUPP	iconActionUPP;
		
		newIcon.resID=fWinRec->iconID;
		PStrCpy(fWinRec->iconName,newIcon.resName);
		newIcon.attrs=resPurgeable;
		
		/* テンポラリファイルへ書き出す */
		UseResFile(iWinRec->tempRefNum);
		
		if (fWinRec->iconType == kIconFamilyType)
		{
			IconSuiteRef		iconSuite;
			IconFamilyHandle	iconFamily;
			
			iconActionUPP=NewIconActionUPP(DeleteIcon);
			err = Get1IconSuite(&iconSuite,newIcon.resID,GetMySelector());
			err = ForEachIconDo(iconSuite,GetMySelector(),iconActionUPP,NULL);
			err = DisposeIconSuite(iconSuite,true);
			
			err = IPIconToIconFamily(&fWinRec->ipIcon,&iconFamily);
			SaveDataToResource(*iconFamily,GetHandleSize((Handle)iconFamily),
				kIconFamilyType,newIcon.resID,newIcon.resName,newIcon.attrs);
		}
		else
		{
			Handle	resHandle;
			
			iconActionUPP=NewIconActionUPP(AddIconToFile);
			err=ForEachIconDo(fWinRec->ipIcon.iconSuite,GetMySelector(),
				iconActionUPP,&newIcon);
			
			resHandle = Get1Resource(kIconFamilyType,newIcon.resID);
			RemoveResource(resHandle);
		}
		DisposeIconActionUPP(iconActionUPP);
		
		SaveDataToResource(NULL,0,kUpdateIconInfoType,
			newIcon.resID,newIcon.resName,newIcon.attrs);
		
		UpdateResFile(iWinRec->tempRefNum);
		UseResFile(gApplRefNum);
		
		/* アイコンリストウィンドウのアップデート */
		GetPort(&port);
		SetPortWindowPort(iconWindow);
		MyInvalWindowPortBounds(iconWindow);
		SetPort(port);
		
		fWinRec->wasChanged=false;
		iWinRec->wasChanged=true;
		
		if (saveToFile)
			SaveIconFile(iconWindow);
	}
	UpdateSaveMenu();
}

/* アイコンファミリウィンドウの復帰 */
void DoRevertIconFamilyWindow(WindowPtr theWindow)
{
	IconFamilyWinRec	*fWinRec=GetIconFamilyRec(theWindow);
	short		item;
	Str255		docKind,docName;
	OSErr		err;
	
	/* まず、確認 */
	GetIndString(docKind,133,5);
	GetWTitle(theWindow,docName);
	item=RevertYN(docKind,docName);
	if (item!=ok) return;
	
	if (fWinRec->parentWindow != nil) /* リストウィンドウを親に持つ */
	{
		WindowPtr	iWindow=fWinRec->parentWindow;
		IconListWinRec	*iWinRec=GetIconListRec(iWindow);
		short	selNum=IsMultiIconSelected(iWinRec);
		Cell	*cellList=nil;
		Cell	theCell;
		
		/* 選択するセル */
		if (!IDToCell(fWinRec->iconID,&theCell,iWinRec->iconList))
		{
			SysBeep(0);
			return;
		}
		
		/* 選択されているセルを保存しておく */
		if (selNum>0)
		{
			Cell	dummy={0,0};
			short	i=0;
			
			cellList=(Cell *)NewPtr(sizeof(Cell)*selNum);
			while (LGetSelect(true,&dummy,iWinRec->iconList))
			{
				LSetSelect(false,dummy,iWinRec->iconList);
				cellList[i++]=dummy;
				LNextCell(true,true,&dummy,iWinRec->iconList);
			}
			LSetDrawingMode(false,iWinRec->iconList);
		}
		
		/* セルを選択 */
		LSetSelect(true,theCell,iWinRec->iconList);
		
		/* 保存せずにウィンドウを閉じる */
		err=CloseFamilyWindow(theWindow,false,false);
		if (err==noErr)
			OpenSelectedIcon(iWindow);
		
		/* 選択セルを元に戻す */
		LSetSelect(false,theCell,iWinRec->iconList);
		while (selNum>0)
		{
			LSetSelect(true,cellList[--selNum],iWinRec->iconList);
		}
		DisposePtr((Ptr)cellList);
		LSetDrawingMode(true,iWinRec->iconList);
	}
	else /* ファイルかフォルダ */
	{
		FSSpec		theFile=fWinRec->theIconSpec;
		Boolean		wasFolder=fWinRec->isFolder;
		
		#ifdef __MOREFILESX__
		FSRef		fsRef;
		FinderInfo	info;
		
		err = FSpMakeFSRef(&theFile,&fsRef);
		err = FSGetFinderInfo(&fsRef,&info,NULL,NULL);
		
		#else
		if (wasFolder) /* フォルダ */
		{
			DInfo		dInfo;
			
			err=FSpGetDInfo(&theFile,&dInfo);
		}
		else /* ファイル */
		{
			FInfo		fInfo;
			
			err=FSpGetFInfo(&theFile,&fInfo);
		}
		#endif
		
		if (err!=noErr)
		{
			SysBeep(0);
			return;
		}
		
		/* 保存せずに閉じて開き直す */
		err=CloseFamilyWindow(theWindow,false,false);
		if (err==noErr)
			OpenFolderIcon(&theFile,wasFolder);
	}
}

/* マウスカーソルのアップデート */
/* ptはローカル座標 */
void UpdateCursorInFamilyWindow(WindowPtr theWindow,Point pt)
{
	#pragma unused(theWindow)
	
	if (PtInRect(pt,&familyNameTERect))
	{
		MySetCursor(iBeamCursor);
		RectRgn(gCurRgnHand,&familyNameTERect);
	}
	else
	{
		RgnHandle	tempRgn=NewRgn();
		Point		tempPt={0,0};
		
		MySetCursor(0);
		MakeArrowCursorRgn();
		RectRgn(tempRgn,&familyNameTERect);
		LocalToGlobal(&tempPt);
		OffsetRgn(tempRgn,tempPt.h,tempPt.v);
		DiffRgn(gCurRgnHand,tempRgn,gCurRgnHand);
		DisposeRgn(tempRgn);
	}
}

/* アイコンデータをクリップボードにコピー */
void CopySelectedIconPicture(IconFamilyWinRec *fWinRec)
{
	PicHandle	picture;
	long		err,dataSize;
	
	/* まず、選んでいるアイコンのPICTを得る */
	picture=IPIconToPicture(&fWinRec->ipIcon,fWinRec->selectedIcon);
	if (picture == nil)
	{
		SysBeep(0);
		return;
	}
	
	/* クリップボードに入れる */
	dataSize=GetHandleSize((Handle)picture);
	#if TARGET_API_MAC_CARBON
	do
	{
		ScrapRef	scrap;
		
		err=ClearCurrentScrap();
		if (err!=noErr) break;;
		err=GetCurrentScrap(&scrap);
		if (err!=noErr) break;
		MoveHHi((Handle)picture);
		err=MemError();
		if (err!=noErr) break;
		HLock((Handle)picture);
		err=MemError();
		if (err!=noErr) break;
		err=PutScrapFlavor(scrap,kPICTFileType,0,GetHandleSize((Handle)picture),*picture);
		if (err!=noErr) break;
		HUnlock((Handle)picture);
	} while (false);
	#else
	err=ZeroScrap();
	
	err=PutScrap(dataSize,kPICTFileType,*picture);
	#endif
	
	KillPicture(picture);
}

/* アイコンデータを削除 */
void DeleteSelectedIconPicture(WindowPtr fWindow)
{
	Handle	temp,temp2;
	OSErr	err;
	short	tempKind=-1;
	Rect	updateRect;
	IconFamilyWinRec *fWinRec=GetIconFamilyRec(fWindow);
	WindowPtr	eWindow;
	PaintWinRec	*eWinRec;
	
	/* 削除されるデータのチェック */
	err=GetDataFromIPIcon(&temp,&fWinRec->ipIcon,fWinRec->selectedIcon);
	if (temp == nil)
	{
		SysBeep(0);
		return;
	}
	
	/* 削除されてもアイコンが成り立つかどうかをチェック */
	if (gIconType[fWinRec->selectedIcon] == kLarge1BitMask)
	{
		err=GetIconFromSuite(&temp2,fWinRec->ipIcon.iconSuite,kSmall1BitMask);
		if (temp2 == nil)
		{
			SysBeep(0);
			return;
		}
	}
	else if (gIconType[fWinRec->selectedIcon] == kSmall1BitMask)
	{
		err=GetIconFromSuite(&temp2,fWinRec->ipIcon.iconSuite,kLarge1BitMask);
		if (temp2 == nil)
		{
			SysBeep(0);
			return;
		}
	}
	
	/* 削除しようとしているアイコンが編集中でないかどうかを確認 */
	eWindow=MyFrontNonFloatingWindow();
	while (eWindow != nil)
	{
		if (GetExtWindowKind(eWindow) == kWindowTypePaintWindow)
		{
			eWinRec=GetPaintWinRec(eWindow);
			if (eWinRec->iconType.fileType == kIconFamilyType && eWinRec->parentWindow == fWindow &&
					eWinRec->iconKind == fWinRec->selectedIcon)
			{
				if (eWinRec->iconHasChanged)
				{
					SysBeep(0);
					SelectReferencedWindow(eWindow);
					return;
				}
				else
				{
					SelectReferencedWindow(eWindow);
					err=ClosePaintWindow(eWindow,false);
					if (err!=noErr)
						return;
					else
						break;
				}
			}
		}
		eWindow=GetNextVisibleWindow(eWindow);
	}
	
	/* 実際の削除 */
	fWinRec->undoData.iconData=temp;
	HandToHand(&fWinRec->undoData.iconData);
	err=SetDataToIPIcon(nil,&fWinRec->ipIcon,fWinRec->selectedIcon);
	DisposeHandle(temp);
	fWinRec->undoData.iconKind=fWinRec->selectedIcon;
	fWinRec->wasChanged=true;
	
	/* 再描画領域 */
	updateRect=familyIconRect[fWinRec->selectedIcon];
	InsetRect(&updateRect,-2,-2);
	MyInvalWindowRect(fWindow,&updateRect);
	if (fWinRec->selectedIcon == kL1Data || fWinRec->selectedIcon == kS1Data)
		tempKind=fWinRec->selectedIcon+1;
	else if (fWinRec->selectedIcon == kL1Mask || fWinRec->selectedIcon == kS1Mask)
		tempKind=fWinRec->selectedIcon-1;
	if (tempKind >= 0)
	{
		updateRect=familyIconRect[tempKind];
		InsetRect(&updateRect,-2,-2);
		MyInvalWindowRect(fWindow,&updateRect);
	}
	
	/* 更新 */
	fWinRec->undoData.undoMode=umDeleteIconData;
	UpdateUndoMenu();
	UpdateSaveMenu();
	UpdateClipMenu();
	UpdateIconMenu();
}

/* クリップボードの内容をアイコンデータに */
void PasteToSelectedIcon(WindowPtr fWindow)
{
	PicHandle	picture;
	long		dataSize;
	OSErr		err;
	WindowPtr	eWindow;
	PaintWinRec	*eWinRec;
	IconFamilyWinRec *fWinRec=GetIconFamilyRec(fWindow);
	#if TARGET_API_MAC_CARBON
	ScrapRef	scrap;
	#else
	long		offset;
	#endif
	
	if (fWinRec->selectedIcon < 0) return;
	
	/* ペーストしようとしているアイコンが編集中でないかどうかを確認 */
	eWindow=MyFrontNonFloatingWindow();
	while (eWindow != nil)
	{
		if (GetExtWindowKind(eWindow) == kWindowTypePaintWindow)
		{
			eWinRec=GetPaintWinRec(eWindow);
			if (eWinRec->iconType.fileType == kIconFamilyType && eWinRec->parentWindow == fWindow && 
					eWinRec->iconKind == fWinRec->selectedIcon)
			{
				SysBeep(0);
				return;
			}
			else break;
		}
		eWindow=GetNextVisibleWindow(eWindow);
	}
	
	#if TARGET_API_MAC_CARBON
	err=GetCurrentScrap(&scrap);
	if (err==noErr)
		err=GetScrapFlavorSize(scrap,kPICTClipType,&dataSize);
	else
		dataSize=0;
	#else
	dataSize=GetScrap(nil,kPICTFileType,&offset);
	#endif
	if (dataSize == 0) return;
	
	picture=(PicHandle)TempNewHandle(dataSize,&err);
	if (err!=noErr)
	{
		ErrorAlertFromResource(ICONERR_RESID,ICONERR3);
		return;
	}
	
	/* クリップボードから読み込む */
	#if TARGET_API_MAC_CARBON
	err=GetScrapFlavorData(scrap,kPICTFileType,&dataSize,*picture);
	#else
	TempHLock((Handle)picture,&err);
	dataSize=GetScrap((Handle)picture,kPICTFileType,&offset);
	TempHUnlock((Handle)picture,&err);
	#endif
	
	SavePictureToIconData(picture,fWinRec,true);
	
	TempDisposeHandle((Handle)picture,&err);
	RedrawIconPreview();
}

/* Pictureのデータをアイコンに設定 */
void SavePictureToIconData(PicHandle picture,IconFamilyWinRec *fWinRec,Boolean convertMaskFlag)
{
	Handle		dataHandle,iconData;
	Rect		updateRect;
	OSErr		err;
	long		offset;
	short		iconKind=fWinRec->selectedIcon,tempKind=fWinRec->selectedIcon;
	WindowPtr	fWindow=GetWindowFromPort((**fWinRec->iconNameTE).inPort);
	
	if (!convertMaskFlag)
		if (iconKind == kL1Mask) tempKind=kL1Data;
		else if (iconKind == kS1Mask) tempKind=kS1Data;
	
	/* アイコンデータに変換 */
	dataHandle=PictureToIcon(picture,tempKind);
	if (dataHandle == nil)
	{
		ErrorAlertFromResource(ICONERR_RESID,ICONERR4);
		return;
	}
	
	err=GetDataFromIPIcon(&iconData,&fWinRec->ipIcon,iconKind);
	
	offset=0;
	if (iconKind == kL1Mask)
		offset=32*32/8;
	if (iconKind == kS1Mask)
		offset=16*16/8;
	
	if (fWinRec->undoData.iconData != nil)
		DisposeHandle(fWinRec->undoData.iconData);
	
	if (iconData == nil)
	{
		long	iconDataSize=GetIconDataSize(iconKind);
		short	tempKind=-1;
		
		/* アイコンデータがない場合には作成しないといけない */
		iconData=NewHandleClear(iconDataSize);
		err=SetDataToIPIcon(iconData,&fWinRec->ipIcon,iconKind);
		
		fWinRec->undoData.iconData=nil;
		if (iconKind == kL1Data || iconKind == kS1Data)
			tempKind=iconKind+1;
		else if (iconKind == kL1Mask || iconKind == kS1Mask)
			tempKind=iconKind-1;
		
		if (tempKind >= 0)
		{
			updateRect=familyIconRect[tempKind];
			InsetRect(&updateRect,-2,-2);
			MyInvalWindowRect(fWindow,&updateRect);
		}
	}
	else
	{
		fWinRec->undoData.iconData=iconData;
		HandToHand(&fWinRec->undoData.iconData);
	}
	fWinRec->undoData.iconKind=fWinRec->selectedIcon;
	
	BlockMoveData(*dataHandle,*iconData+offset,GetHandleSize(dataHandle));
	
	updateRect=familyIconRect[fWinRec->selectedIcon];
	InsetRect(&updateRect,-2,-2);
	MyInvalWindowRect(fWindow,&updateRect);
	
	DisposeHandle(dataHandle);
	
	/* 変更フラグをセット */
	fWinRec->wasChanged=true;
	
	/* 取り消し処理用 */
	fWinRec->undoData.undoMode=umPasteIconData;
	UpdateUndoMenu();
	UpdateSaveMenu();
	UpdateClipMenu();
	UpdateIconMenu();
}

pascal void MyBitsProc(const BitMap *bitPtr,const Rect *srcRect,const Rect *dstRect,
	short mode,RgnHandle maskRgn);

GWorldPtr	gT;

/* Pictureからアイコンに変換 */
Handle PictureToIcon(PicHandle picture,short iconKind)
{
	short	iconSize=gIconSize[iconKind],iconDepth=gIconDepth[iconKind];
	Rect	pictRect,iconRect;
	GWorldPtr	iconGWorld,tempGWorld;
	PixMapHandle	iconPix=NULL;
	Handle	dataHandle;
	OSErr	err;
	short	i;
	Ptr		src,dst;
	long	rowBytes;
	long	iconRowBytes=iconSize*iconDepth/8;
	CTabHandle	ctab=nil;
	MyBitMapRec	myBmp;
	Boolean	maskFlag=(iconKind == kL1Mask || iconKind == kS1Mask);
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	SetRect(&iconRect,0,0,iconSize,iconSize);
	QDGetPictureBounds(picture,&pictRect);
	
	dataHandle=NewHandle(iconSize*iconRowBytes);
	if (dataHandle == nil) return nil;
	
	/* まずオフポートを作成し、PICTをその中に表示する */
	GetGWorld(&cPort,&cDevice);
	err=NewGWorld(&tempGWorld,k32ARGBPixelFormat,&pictRect,0,0,useTempMem);
	if (err != noErr) return nil;
	
	LockPixels(GetGWorldPixMap(tempGWorld));
	SetGWorld(tempGWorld,0);
	EraseRect(&pictRect);
	#if 1
	DrawPicture(picture,&pictRect);
	#else
	{
		CQDProcs	*savedProcs,procs;
		
		err=NewGWorld(&gT,k32ARGBPixelFormat,&pictRect,0,0,useTempMem);
		
		LockPixels(GetGWorldPixMap(gT));
		SetGWorld(gT,0);
		EraseRect(&pictRect);
		SetGWorld(tempGWorld,0);
		
		/* bitsProc置き換え */
		savedProcs=GetPortGrafProcs(tempGWorld);
		SetStdCProcs(&procs);
		procs.bitsProc=NewQDBitsUPP(MyBitsProc);
		SetPortCGrafProcs(tempGWorld,&procs);
		
		DrawPicture(picture,&pictRect);
		
		/* bitsProcを元に戻す */
		SetPortCGrafProcs(tempGWorld,savedProcs);
		DisposeQDBitsUPP(procs.bitsProc);
		
		CopyBits(GetPortBitMapForCopyBits(gT),
			GetPortBitMapForCopyBits(tempGWorld),
			&pictRect,&pictRect,srcCopy,nil);
		
		UnlockPixels(GetGWorldPixMap(gT));
		DisposeGWorld(gT);
	}
	#endif
	
	UnlockPixels(GetGWorldPixMap(tempGWorld));
	
	if (maskFlag)
	{
		err=CreateMask(tempGWorld,&pictRect,&iconRect,&myBmp);
		src=myBmp.bmp.baseAddr;
		rowBytes=myBmp.bmp.rowBytes;
	}
	else
	{
		if (iconKind == kL8Mask || iconKind == kS8Mask || iconKind == kT8Mask)
			ctab=GetGrayscaleCTable(iconDepth,true);
		
		err=NewGWorld(&iconGWorld,iconDepth,&iconRect,ctab,0,useTempMem);
		SetGWorld(iconGWorld,0);
		iconPix=GetGWorldPixMap(iconGWorld);
		LockPixels(iconPix);
		
		EraseRect(&iconRect);
		CopyBits(GetPortBitMapForCopyBits(tempGWorld),
			GetPortBitMapForCopyBits(iconGWorld),
			&pictRect,&iconRect,srcCopy,nil); /* dither? */
		
		src=MyGetPixBaseAddr(iconPix);
		rowBytes=MyGetPixRowBytes(iconPix) & 0x3fff;
	}
	
	SetGWorld(cPort,cDevice);
	DisposeGWorld(tempGWorld);
	
	/* データ化 */
	HLock(dataHandle);
	dst=*dataHandle;
	for (i=0; i<iconSize; i++)
	{
		BlockMoveData(src,dst,iconRowBytes);
		src+=rowBytes;
		dst+=iconRowBytes;
	}
	HUnlock(dataHandle);
	
	if (maskFlag)
		err=DisposeBitMap(&myBmp);
	else
	{
		UnlockPixels(iconPix);
		DisposeGWorld(iconGWorld);
		if (ctab != nil) DisposeHandle((Handle)ctab);
	}
	
	return dataHandle;
}

/* bitsProc */
pascal void MyBitsProc(const BitMap *bitPtr,const Rect *srcRect,const Rect *dstRect,
	short mode,RgnHandle maskRgn)
{
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	GetGWorld(&cPort,&cDevice);
	
	SetGWorld(gT,0);
	CopyBits(bitPtr,GetPortBitMapForCopyBits(gT),
		srcRect,dstRect,mode,maskRgn);
	
	SetGWorld(cPort,cDevice);
}

/* アイコンからデータを取り出してPictureに変換 */
PicHandle IconToPicture(IconSuiteRef theIconSuite,short iconKind)
{
	ResType	iconType;
	short	iconSize,iconDepth;
	long	offset=0;
	OSErr	err;
	GWorldPtr	iconGWorld;
	PixMapHandle	iconPix;
	Handle	dataHandle;
	Ptr		src,dst;
	PicHandle	picture;
	OpenCPicParams	picParam;
	Rect	iconRect;
	short	rowBytes,iconRowBytes;
	short	i;
	CTabHandle	ctab=nil;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	if (iconKind < 0) return nil;
	
	iconType=gIconType[iconKind];
	iconSize=gIconSize[iconKind];
	iconDepth=gIconDepth[iconKind];
	
	switch (iconKind)
	{
		case kL1Mask:
			offset=32*32/8;
			break;
		
		case kS1Mask:
			offset=16*16/8;
			break;
		
		case kL8Mask:
		case kS8Mask:
		case kT8Mask:
			ctab=GetGrayscaleCTable(iconDepth,true);
			break;
	}
	
	SetRect(&iconRect,0,0,iconSize,iconSize);
	
	err=GetIconFromSuite(&dataHandle,theIconSuite,iconType);
	if (err != noErr || dataHandle == nil) return nil; /* アイコンがなければnilを返す */
	if (GetHandleSize(dataHandle) != iconSize*(long)iconSize*(iconDepth>1 ? iconDepth : 2)/8) return nil;
	
	GetGWorld(&cPort,&cDevice);
	err=NewGWorld(&iconGWorld,iconDepth,&iconRect,ctab,NULL,0);
	if (err != noErr) return nil;
	
	HLock(dataHandle);
	
	SetGWorld(iconGWorld,0);
	iconPix=GetGWorldPixMap(iconGWorld);
	LockPixels(iconPix);
	rowBytes=MyGetPixRowBytes(iconPix) & 0x3fff;
	iconRowBytes=iconSize*iconDepth/8;
	EraseRect(&iconRect);
	
	src=*dataHandle+offset;
	dst=MyGetPixBaseAddr(iconPix);
	for (i=0; i<iconSize; i++)
	{
		BlockMoveData(src,dst,iconRowBytes);
		src+=iconRowBytes;
		dst+=rowBytes;
	}
	HUnlock(dataHandle);
	
	picParam.srcRect=iconRect;
	picParam.hRes=72L<<16;
	picParam.vRes=72L<<16;
	picParam.version=-2;
	
	picture=OpenCPicture(&picParam);
	ClipRect(&iconRect);
	EraseRect(&iconRect);
	CopyBits(GetPortBitMapForCopyBits(iconGWorld),GetPortBitMapForCopyBits(iconGWorld),
		&iconRect,&iconRect,srcCopy,nil);
#if __BIG_ENDIAN__
	(**picture).picFrame=iconRect;
#endif
	ClosePicture();
	
	UnlockPixels(iconPix);
	
	SetGWorld(cPort,cDevice);
	DisposeGWorld(iconGWorld);
	if (ctab != NULL) DisposeHandle((Handle)ctab);
	
	return picture;
}

/* アイコンの絵を描く。ない場合はグレーで塗る */
void MyDrawPicture(PicHandle picture,const Rect *r)
{
	if (picture!=nil)
		DrawPicture(picture,r);
	else
	{
		Rect	rr=*r;
		Pattern	pat;
		
		GetQDGlobalsLightGray(&pat);
		InsetRect(&rr,-2,-2);
		PenPat(&pat);
		PaintRect(&rr);
		PenNormal();
	}
}

/* open the selected icon (by PaintWindow) */
void EditFamilyIcon(WindowPtr fWindow,short forceFlag)
{
	IconFamilyWinRec	*fWinRec=GetIconFamilyRec(fWindow);
	WindowPtr	eWindow;
	PaintWinRec	*eWinRec;
	PicHandle	picture;
	Rect		picSize;
	Boolean		useExternalEditor=false;
	short		colorMode;
	Handle		dataHandle;
	OSErr		err;
	
	/* データがあるかどうかを確認 */
	err=GetDataFromIPIcon(&dataHandle,&fWinRec->ipIcon,fWinRec->selectedIcon);
	if (err!=noErr || dataHandle == NULL) return;
	
	/* まずウィンドウが先に開かれていないかどうかをチェック */
	eWindow=MyFrontNonFloatingWindow();
	while (eWindow != nil)
	{
		if (GetExtWindowKind(eWindow) == kWindowTypePaintWindow)
		{
			eWinRec=GetPaintWinRec(eWindow);
			if (eWinRec->iconType.fileType == kIconFamilyType && eWinRec->parentWindow == fWindow &&
					eWinRec->iconKind == fWinRec->selectedIcon)
			{
				SelectReferencedWindow(eWindow);
				UpdateMenus();
				return;
			}
		}
		eWindow=GetNextVisibleWindow(eWindow);
	}
	
	/* 外部エディタで開かれているかどうか */
	if (IsOpenedByExternalEditor(fWindow,fWinRec->selectedIcon,kXIconOpenMode))
		return; /* 取りあえずの処理 */
	
	switch (forceFlag)
	{
		case kForceNone:
			useExternalEditor = (gIconUseDepth[fWinRec->selectedIcon] == 32 && 
				isExternalEditorAvailable && gUseExternalEditor);
			break;
		
		case kForceInternalEditor:
			useExternalEditor = false;
			break;
		
		case kForceExternalEditor:
			useExternalEditor = isExternalEditorAvailable;
			break;
	}
	
	/* 32bitアイコンなどで、外部エディタで開く設定ならば委託 */
	if (useExternalEditor)
		OpenByExternalEditor(fWindow);
	else
	{
		GWorldPtr	cPort;
		GDHandle	cDevice;
		
		/* そうでなければ自前で開く */
		picture=IPIconToPicture(&fWinRec->ipIcon,fWinRec->selectedIcon);
		if (picture == nil)
		{
			ErrorAlertFromResource(ICONERR_RESID,ICONERR4);
			return;
		};
		
		/* color mode */
		if (gIconUseDepth[fWinRec->selectedIcon] == 32)
			colorMode = k32BitColorMode;
		else
			colorMode = kNormal8BitColorMode;
		
//		picSize=(**picture).picFrame;
        QDGetPictureBounds(picture,&picSize);
		eWindow=MakePaintWindow(&picSize,
			(gIconSize[fWinRec->selectedIcon] == 128 ? 2 : 3),colorMode);
		if (eWindow == nil)
		{
			KillPicture(picture);
			ErrorAlertFromResource(ICONERR_RESID,ICONERR4);
			return;
		}
		eWinRec=GetPaintWinRec(eWindow);
		eWinRec->iconSize=picSize;
		eWinRec->iconType.fileType=kIconFamilyType;
		
		eWinRec->iconHasSaved=true;
		eWinRec->iconHasChanged=false;
		eWinRec->undoMode=umCannot;
		if (fWinRec->iconName[0] == 0)
			GetIndString(eWinRec->saveFileSpec.name,132,1);
		else
			PStrCpy(fWinRec->iconName,eWinRec->saveFileSpec.name);
		
		eWinRec->parentWindow=fWindow;
		eWinRec->iconKind=fWinRec->selectedIcon;
		SetEditWindowTitle(eWindow);
		
		/* draw picture in offport */
		GetGWorld(&cPort,&cDevice);
		SetGWorld(eWinRec->editDataPtr,0);
		LockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
		DrawPicture(picture,&picSize);
		UnlockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
		
		KillPicture(picture);
		
		/* mask */
		SetGWorld(eWinRec->currentMask,0);
		LockPixels(GetGWorldPixMap(eWinRec->currentMask));
		PaintRect(&picSize);
		UnlockPixels(GetGWorldPixMap(eWinRec->currentMask));
		
		SetGWorld(cPort,cDevice);
		
		DispOffPort(eWindow);
		UpdateMenus();
	}
}

/* アイコンをファミリウィンドウへ保存 */
void SaveToFamily(WindowPtr eWindow)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(eWindow);
	PicHandle	savePict;
	WindowPtr	fWindow;
	IconFamilyWinRec	*fWinRec;
	GrafPtr		port;
	
	savePict=GetSelectionPic(eWindow,true);
	
	fWindow=eWinRec->parentWindow;
	fWinRec=GetIconFamilyRec(fWindow);
	fWinRec->selectedIcon=eWinRec->iconKind;
	
	SavePictureToIconData(savePict,fWinRec,gOtherPrefs.maskAutoConvert);
	
	KillPicture(savePict);
	
	GetPort(&port);
	SetPortWindowPort(fWindow);
	MyInvalWindowPortBounds(fWindow);
	SetPort(port);
}

/* 空のデータを追加 */
void CreateEmptyIconData(WindowPtr fWindow)
{
	IconFamilyWinRec	*fWinRec=GetIconFamilyRec(fWindow);
	Handle		iconData;
	Rect		updateRect;
	OSErr		err;
	short		iconKind=fWinRec->selectedIcon;
	
	/* アイコンが選択されていなければ無効 */
	if (iconKind < 0) return;
	
	/* アイコンデータが存在すれば無効 */
	err=GetDataFromIPIcon(&iconData,&fWinRec->ipIcon,iconKind);
	if (iconData != NULL) return;
	
	/* 取り消し用のデータをクリア */
	if (fWinRec->undoData.iconData != nil)
		DisposeHandle(fWinRec->undoData.iconData);
	
	/* アイコンデータの作成 */
	{
		long	iconDataSize=GetIconDataSize(iconKind);
		short	tempKind=-1;
		
		iconData=NewHandleClear(iconDataSize);
		err=SetDataToIPIcon(iconData,&fWinRec->ipIcon,iconKind);
		
		/* 1ビットマスクの場合、更新する場所が多い */
		fWinRec->undoData.iconData=nil;
		if (iconKind == kL1Data || iconKind == kS1Data)
			tempKind=iconKind+1;
		else if (iconKind == kL1Mask || iconKind == kS1Mask)
			tempKind=iconKind-1;
		
		if (tempKind >= 0)
		{
			updateRect=familyIconRect[tempKind];
			InsetRect(&updateRect,-2,-2);
			MyInvalWindowRect(fWindow,&updateRect);
		}
	}
	fWinRec->undoData.iconKind=fWinRec->selectedIcon;
	
	/* ウィンドウの更新 */
	updateRect=familyIconRect[fWinRec->selectedIcon];
	InsetRect(&updateRect,-2,-2);
	MyInvalWindowRect(fWindow,&updateRect);
	
	/* 変更フラグをセット */
	fWinRec->wasChanged=true;
	
	/* 取り消し処理用 */
	fWinRec->undoData.undoMode=umAddIconData;
	UpdateUndoMenu();
	UpdateSaveMenu();
	UpdateClipMenu();
	UpdateIconMenu();
	RedrawIconPreview();
}

/* アイコンファミリウィンドウでのundo処理 */
void DoUndoIconFamily(WindowPtr fWindow)
{
	IconFamilyWinRec	*fWinRec=GetIconFamilyRec(fWindow);
	OSErr	err;
	Handle	temp,temp2;
	short	undoMode=fWinRec->undoData.undoMode;
	short	mode=( undoMode>0 ? undoMode : -undoMode );
	
	switch (mode)
	{
		case umCannot:
			{
				MenuHandle	menu;
				
				menu=GetMenuHandle(mEdit);
				MyDisableMenuItem(menu,iUndo);
				return;
			}
			break;
		
		case umPasteIconData:
		case umDeleteIconData:
		case umAddIconData:
			err=GetDataFromIPIcon(&temp,&fWinRec->ipIcon,fWinRec->undoData.iconKind);
			if (temp == nil)
			{
				err=SetDataToIPIcon(fWinRec->undoData.iconData,&fWinRec->ipIcon,fWinRec->undoData.iconKind);
//				DisposeHandle(fWinRec->undoData.iconData);
				fWinRec->undoData.iconData=nil;
			}
			else if (fWinRec->undoData.iconData == nil)
			{
				fWinRec->undoData.iconData=temp;
				HandToHand(&fWinRec->undoData.iconData);
				err=SetDataToIPIcon(nil,&fWinRec->ipIcon,fWinRec->undoData.iconKind);
				DisposeHandle(temp);
			}
			else
			{
				temp2=temp;
				HandToHand(&temp2);
				BlockMoveData(*fWinRec->undoData.iconData,*temp,GetHandleSize(temp));
				BlockMoveData(*temp2,*fWinRec->undoData.iconData,GetHandleSize(temp));
				DisposeHandle(temp2);
			}
			break;
	}
	
	fWinRec->undoData.undoMode = -undoMode;
	fWinRec->selectedIcon = fWinRec->undoData.iconKind;
	
	MyInvalWindowPortBounds(fWindow);
	
	if (!fWinRec->wasChanged)
	{
		fWinRec->wasChanged=true;
		UpdateSaveMenu();
	}
	
	gUsedCount.undoNum++;
	
	UpdateClipMenu();
	UpdateIconMenu();
	UpdateUndoMenu();
	RedrawIconPreview();
}

/* アイコンを選択していない場合のメニュー */
void MyIconFamilyContextMenu(Point globPt,WindowPtr fWindow)
{
	enum {
		piEdit=1,
		piEditByExternalEditor,
		piDropFromExternalEditor,
		piCut=5,
		piCopy,
		piPaste,
		piClear,
		piAdd=10,
	};
	
	MenuHandle	menu=GetMenu(172);
	long	selItem;
	long	dataSize;
	IconFamilyWinRec	*fWinRec=GetIconFamilyRec(fWindow);
	OSErr	err;
	
	InsertMenu(menu,-1);
	/* クリップボードの確認→ペースト */
	#if TARGET_API_MAC_CARBON
	{
		ScrapRef	scrap;
		OSErr		err;
		
		err=GetCurrentScrap(&scrap);
		err=GetScrapFlavorSize(scrap,kPICTClipType,&dataSize);
	}
	#else
	{
		long	offset;
		
		dataSize=GetScrap(0,kPICTFileType,&offset);
	}
	#endif
	if (dataSize<=0) MyDisableMenuItem(menu,piPaste);
	else MyEnableMenuItem(menu,piPaste);
	
	/* 選択アイコンの確認 */
	{
		Handle	dataHandle;
		OSErr	err;
		
		err=GetDataFromIPIcon(&dataHandle,&fWinRec->ipIcon,fWinRec->selectedIcon);
		
		/* 選択アイコンあり */
		if (err == noErr && dataHandle != nil)
		{
			MyEnableMenuItem(menu,piEdit);
			MyEnableMenuItem(menu,piCopy);
			MyEnableMenuItem(menu,piCut);
			MyEnableMenuItem(menu,piClear);
			
			/* 外部エディタ存在の確認 */
			if (isExternalEditorAvailable)
			{
				MyEnableMenuItem(menu,piEditByExternalEditor);
				if (IsOpenedByExternalEditor(fWindow,fWinRec->selectedIcon,0))
					MyEnableMenuItem(menu,piDropFromExternalEditor);
				else
					MyDisableMenuItem(menu,piDropFromExternalEditor);
			}
			else
			{
				MyDisableMenuItem(menu,piEditByExternalEditor);
				MyDisableMenuItem(menu,piDropFromExternalEditor);
			}
			
			MyDisableMenuItem(menu,piAdd);
		}
		else
		{
			MyDisableMenuItem(menu,piEdit);
			MyDisableMenuItem(menu,piEditByExternalEditor);
			MyDisableMenuItem(menu,piDropFromExternalEditor);
			MyDisableMenuItem(menu,piCopy);
			MyDisableMenuItem(menu,piCut);
			MyDisableMenuItem(menu,piClear);
			
			MyEnableMenuItem(menu,piAdd);
		}
	}
	
	selItem=PopUpMenuSelectWFontSize(menu,globPt.v,globPt.h,0,9);
	if (selItem==0L) return;
	
	DeleteMenu(172);
	
	switch (LoWord(selItem))
	{
		case piEdit:
			EditFamilyIcon(fWindow,kForceInternalEditor);
			break;
		
		case piEditByExternalEditor:
			EditFamilyIcon(fWindow,kForceExternalEditor);
			break;
		
		case piDropFromExternalEditor:
			err=DropFromExternalEditor(fWindow,fWinRec->selectedIcon);
			break;
		
		case piCopy:
			CopySelectedIconPicture(fWinRec);
			break;
		
		case piCut:
			CopySelectedIconPicture(fWinRec);
			DeleteSelectedIconPicture(fWindow);
			break;
		
		case piPaste:
			PasteToSelectedIcon(fWindow);
			break;
		
		case piClear:
			DeleteSelectedIconPicture(fWindow);
			break;
		
		case piAdd:
			CreateEmptyIconData(fWindow);
			break;
	}
}

/* ファミリウィンドウへのドラッグ処理 */
pascal short MyIconFamilyTrackingHandler(DragTrackingMessage theMessage,WindowPtr theWindow,
										void *handlerRefCon,DragReference theDrag)
{
	#pragma unused(theWindow)
	MyDragGlobalsRec	*myGlobals=(MyDragGlobalsRec *)handlerRefCon;
	Point		mouse,localMouse;
	DragAttributes	attributes;
	
	GetDragAttributes(theDrag,&attributes);
	
	switch (theMessage) {
		case kDragTrackingEnterHandler:
			break;
		
		case kDragTrackingEnterWindow:
			myGlobals->canAcceptDrag=IsMyIconFamilyTypeAvailable(theDrag);
			myGlobals->isInContent=true;
			
			if (myGlobals->canAcceptDrag)
			{
			}
			break;
		
		case kDragTrackingInWindow:
			if (!myGlobals->canAcceptDrag)
				break;
			
			GetDragMouse(theDrag,&mouse,0L);
			localMouse=mouse;
			GlobalToLocal(&localMouse);
			
			//if (attributes & kDragHasLeftSenderWindow)
			{
				short	i;
				Boolean	isInIconRect=false;
				IconFamilyWinRec	*fWinRec=GetIconFamilyRec(theWindow);
				
				for (i=0; i<GetIconKindNum(fWinRec); i++)
				{
					if (PtInRect(localMouse,&familyIconRect[i]))
					{
						RgnHandle	hiliteRgn;
						Rect		hiliteRect;
						
						hiliteRgn=NewRgn();
						hiliteRect=familyIconRect[i];
						InsetRect(&hiliteRect,-2,-2);
						RectRgn(hiliteRgn,&hiliteRect);
						ShowDragHilite(theDrag,hiliteRgn,true); /* 内側 */
						DisposeRgn(hiliteRgn);
						isInIconRect=true;
						break;
					}
				}
				
				if (!isInIconRect)
					HideDragHilite(theDrag);
			}
			break;
		
		case kDragTrackingLeaveWindow:
			if (myGlobals->canAcceptDrag && myGlobals->isInContent) {
				HideDragHilite(theDrag);
			}
			myGlobals->canAcceptDrag=myGlobals->isInContent=false;
			break;
		
		case kDragTrackingLeaveHandler:
			break;
	}
	
	return (noErr);
}

/* 受け入れ可能なものかどうかのチェック */
Boolean IsMyIconFamilyTypeAvailable(DragReference theDrag)
{
	short			index;
	unsigned short	items;
	FlavorFlags		theFlags;
	ItemReference	theItem;
	OSErr			result;
	
	CountDragItems(theDrag,&items);
	
	/* １つだけ受け入れ可能 */
	if (items > 1) return false;
	
	for (index=1; index<=1; index++) {
		GetDragItemReferenceNumber(theDrag,index,&theItem);
		
		/* 'PICT' flavorの存在をチェック */
		result=GetFlavorFlags(theDrag,theItem,kPICTFileType,&theFlags);
		if (result==noErr)
			continue;
		
		return false;
	}
	
	return true;
}

/* ファミリウィンドウへのドラッグの受け入れ */
pascal short MyIconFamilyReceiveHandler(WindowPtr theWindow,void *handlerRefCon,
										DragReference theDrag)
{
	MyDragGlobalsRec	*myGlobals=(MyDragGlobalsRec *)handlerRefCon;
	Point			mouse;
	short			index;
	unsigned short	items;
	ItemReference	theItem;
	FlavorFlags		theFlags;
	Size			dataSize;
	PicHandle		dragPic;
	OSErr			result=noErr;
	
	if (!myGlobals->canAcceptDrag) return -2;
	myGlobals->isInContent=false;
	myGlobals->canAcceptDrag=false;
	
	GetDragMouse(theDrag,&mouse,0L);
	HideDragHilite(theDrag);
	
	CountDragItems(theDrag,&items);
	
	for (index=1; index<=items; index++)
	{
		GetDragItemReferenceNumber(theDrag,index,&theItem);
		
		result=GetFlavorFlags(theDrag,theItem,kPICTFileType,&theFlags);
		if (result==noErr)
		{
			short	i,iconKind=-1;
			Point	localMouse=mouse;
			IconFamilyWinRec	*fWinRec=GetIconFamilyRec(theWindow);
			
			GlobalToLocal(&localMouse);
			
			GetFlavorDataSize(theDrag,theItem,kPICTFileType,&dataSize);
			dragPic=(PicHandle)NewHandle(dataSize);
			if (dragPic==nil)
			{
				/* メモリが足りない */
				return memFullErr;
			}
			
			HLock((Handle)dragPic);
			GetFlavorData(theDrag,theItem,kPICTFileType,(char *)*dragPic,&dataSize,0L);
			HUnlock((Handle)dragPic);
			
			/* どこにドラッグされたのかをチェック */
			for (i=0; i<GetIconKindNum(fWinRec); i++)
			{
				if (PtInRect(localMouse,&familyIconRect[i]))
				{
					iconKind=i;
					break;
				}
			}
			if (iconKind>=0)
			{
				WindowPtr	eWindow;
				PaintWinRec	*eWinRec;
				
				if (fWinRec->selectedIcon>=0)
				{
					Rect	prevSelectedRect=familyIconRect[fWinRec->selectedIcon];
					
					InsetRect(&prevSelectedRect,-2,-2);
					MyInvalWindowRect(theWindow,&prevSelectedRect);
				}
				
				/* ドロップしようとしているアイコンが編集中でないかどうかを確認 */
				eWindow=MyFrontNonFloatingWindow();
				while (eWindow != nil)
				{
					if (GetExtWindowKind(eWindow) == kWindowTypePaintWindow)
					{
						eWinRec=GetPaintWinRec(eWindow);
						if (eWinRec->iconType.fileType == kXIconFileType && eWinRec->parentWindow == theWindow &&
								eWinRec->iconKind == iconKind)
						{
							SysBeep(0);
							DisposeHandle((Handle)dragPic);
							return paramErr;
						}
						else break;
					}
					eWindow=GetNextVisibleWindow(eWindow);
				}
				
				fWinRec->selectedIcon=iconKind;
				SavePictureToIconData(dragPic,fWinRec,true);
				
				result=noErr;
			}
			else
				result=-1;
			
			DisposeHandle((Handle)dragPic);
		}
/*		else
		{
			result=GetFlavorFlags(theDrag,theItem,'hfs ',&theFlags);
			if (result==noErr)
			{
				GetFlavorDataSize(theDrag,theItem,'hfs ',&dataSize);
				GetFlavorData(theDrag,theItem,'hfs ',(char *)&hfsFlavorData,&dataSize,0L);
				*/
				/* ファイルのアイコンを取得してリストに追加する */
/*				AddFileIconToIconList(theWindow,&hfsFlavorData.fileSpec);
			}
		}*/
	}
	/* ウィンドウをアクティブにする設定なら、する */
	if (gOtherPrefs.activateDroppedWindow)
	{
		SelectReferencedWindow(theWindow);
		UpdateMenus();
	}
	
	/* ドロップされたウィンドウがフロントウィンドウならプレビューを更新 */
	if (MyFrontNonFloatingWindow() == theWindow)
		RedrawIconPreview();
	
	return result;
}

/* ドラッグを開始する（ドラッグのデータを作成） */
void MyDoStartDragIconFamily(WindowPtr fWindow,EventRecord *theEvent)
{
	IconFamilyWinRec	*fWinRec=GetIconFamilyRec(fWindow);
	
	DragReference   theDrag;
	RgnHandle       dragRegion=NULL;
	OSErr			err;
	DragSendDataUPP	dsdUPP;
	
	dsdUPP=NewDragSendDataUPP(MySendIconFamilyDataProc);
	if (dsdUPP==0) return;
	
	err = NewDrag(&theDrag);
	if (err!=noErr)
		goto exit;
	
	err = MyDoAddIconFamilyFlavors(fWindow, theDrag);
	if (err!=noErr)
		goto exit;
	
	dragRegion = NewRgn();
	err = MyGetDragIconFamilyRegion(fWindow, dragRegion, theDrag);
	if (err!=noErr)
		goto exit;
	
	err = SetDragSendProc(theDrag, dsdUPP, fWinRec);
	if (err!=noErr)
		goto exit;
	
	err = TrackDrag(theDrag, theEvent, dragRegion);
	
exit:
	DisposeRgn(dragRegion);
	DisposeDrag(theDrag);
	DisposeDragSendDataUPP(dsdUPP);
}

/* ドラッグデータにflavorを追加する */
OSErr MyDoAddIconFamilyFlavors(WindowPtr fWindow,DragReference theDrag)
{
	#pragma unused(fWindow)
	OSErr	err;
	
	err=AddDragItemFlavor(theDrag,1,kPICTFileType,0L,0L,0);
	
	return err;
}

/* ドラッグリージョンの作成 */
OSErr MyGetDragIconFamilyRegion(WindowPtr fWindow,RgnHandle dragRegion,
							DragReference theDragRef)
{
	#pragma unused(theDragRef)
	IconFamilyWinRec	*fWinRec=GetIconFamilyRec(fWindow);
	RgnHandle		tempRgn;
	Point			globalPoint={0,0};
	
	tempRgn=NewRgn();
	
	LocalToGlobal(&globalPoint);
	
	RectRgn(tempRgn,&familyIconRect[fWinRec->selectedIcon]);
	CopyRgn(tempRgn,dragRegion);
	InsetRgn(tempRgn,1,1);
	DiffRgn(dragRegion,tempRgn,dragRegion);
	
	OffsetRgn(dragRegion,globalPoint.h,globalPoint.v);
	
	DisposeRgn(tempRgn);
	return(noErr);
}

/* 要求されたflavorデータを提供する */
pascal short MySendIconFamilyDataProc(FlavorType theType,void *dragSendRefCon,
										ItemReference theItem,DragReference theDrag)
{
	#pragma unused(theItem)
	OSErr	err=noErr;
	IconFamilyWinRec	*fWinRec;
	PicHandle	picture;
	
	fWinRec=dragSendRefCon;
	
	switch (theType) {
		case kPICTFileType:
			/* 渡すPICTデータを作成 */
			picture=IPIconToPicture(&fWinRec->ipIcon,fWinRec->selectedIcon);
			HLock((Handle)picture);
			err=SetDragItemFlavorData(theDrag,theItem,kScrapFlavorTypePicture,*picture,
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


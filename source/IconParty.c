/* ------------------------------------------------------------ */
/*  IconParty.c                                                 */
/*     初期化、イベント処理など（メインルーチン）               */
/*                                                              */
/*                 1997.1.11 - 2001.1.27  naoki iimura        	*/
/* ------------------------------------------------------------ */

/* includes */
#ifdef __APPLE_CC__
#include	<Carbon/Carbon.h>
#else
#include	<QDOffscreen.h>
#include	<sound.h>
#include	<Drag.h>
#include	<Appearance.h>
#include	<TSMTE.h>
#include	<Navigation.h>
#include	<Balloons.h>
#include	<Resources.h>
#include	<Gestalt.h>
#include	<LowMem.h>
#include	<TextUtils.h>
#include	<Script.h>
#include	<ToolUtils.h>
#endif

#ifdef __APPLE_CC__
#include	<MoreFilesX.h>
#else
#include	<MoreFilesExtras.h>
#endif

#include	"WindowExtensions.h"

#include	"Globals.h"
#include	"AESupport.h"
#include	"IconParty.h"
#include	"MenuRoutines.h"
#include	"FileRoutines.h"
#include	"UsefulRoutines.h"
#include	"TabletUtils.h"
#include	"DebugMode.h"
#include	"PreCarbonSupport.h"
#include	"IconRoutines.h"
#include	"IconListWindow.h"
#include	"IconFamilyWindow.h"
#include	"WindowRoutines.h"
#include	"Preferences.h"
#include	"ExternalEditorSupport.h"
#include	"EditRoutines.h"
#include	"PaintRoutines.h"
#include	"ToolRoutines.h"
#include	"UpdateCursor.h"

/* prototypes */
static void	CurRgnInit(void);
static void	InitConstants(void);
static void	InitGlobals(void);
static void	GWorldInit(void);
static void	DotLibInit(void);

static void	MainLoop(void);
static void	DoIdleEvent(EventRecord *theEvent);
static void	HandleMouseDown(EventRecord *theEvent);
static void	HandleKeyDown(EventRecord *theEvent);

static void	ResetSysSettings(void);

static void	TitlePopup(WindowPtr theWindow,Point popPos);
static void	FSpAERevealFile(FSSpec *spec);

static void	InvertDot(WindowPtr theWindow);
static void	InvertDotMain(PaintWinRec *eWinRec);
static void	MoveDot(WindowPtr theWindow,short dx,short dy);

static void	PutCommand(Str255 command,char newCommand);
static void	CatChar2(char c,Str255 string);
static void	MoveDotMain(Point *pt,short dx,short dy,Rect *r);

#if !TARGET_API_MAC_CARBON
static void	UpdateBalloonHelp(Point globPt);
static void	MyShowBalloon(const Rect *r,short id,short index);
#endif

/* Window.cでの変数だったかな */
extern WindowPtr	DotModePalette;


#define	IPERR_RESID	4009
#define	IPERR1	1
#define	IPERR2	2
#define	IPERR3	3


/* main */
#ifdef __APPLE_CC__
int main(void)
#else
void main(void)
#endif
{
	Boolean	needToUnload=false;
	OSErr	err=noErr;
	
	#if !TARGET_API_MAC_CARBON
	SetApplLimit(GetApplLimit()-16384);
	
	MaxApplZone();
	#endif
	ToolBoxInit();
	{
		#if TARGET_API_MAC_CARBON
		MoreMasterPointers(10);
		#else
		short	i;
		
		for (i=0; i<10; i++)
			MoreMasters();
		#endif
	}
	
	CreateDebugFile();
	
	gApplRefNum=CurResFile();		/* アプリケーションのリファレンス */
	#if TARGET_API_MAC_CARBON && 0
	{
		Str255	temp;
		Handle	h;
		short	c;
		
		h=Get1Resource('WIND',128);
		if (h==nil) ErrorAlert("\pGet1Resource() failed.");
		
		h=GetResource('WIND',128);
		if (h==nil) ErrorAlert("\pGetResource() failed.");
		
		NumToString(gApplRefNum,temp);
		ErrorAlert(temp);
		
		UseResFile(gApplRefNum);
		c=CountResources('WIND');
		NumToString(c,temp);
		ErrorAlert(temp);
		
		h=Get1Resource('WIND',128);
		if (h==nil) ErrorAlert("\pGet1Resource() failed.");
		
		h=GetResource('WIND',128);
		if (h==nil) ErrorAlert("\pGetResource() failed.");
	}
	#endif
	InitConstants(); WriteStrToDebugFile("\pInitConstants.\r");
	InitGlobals(); WriteStrToDebugFile("\pInitGlobals.\r");
	
	#if TARGET_API_MAC_CARBON
	if (gSystemVersion >= 0x0810 && gCarbonLibVersion >= 0x0102) /* 1.0.2以降の追加APIが多いため */
	#else
	if (gSystemVersion >= 0x0700)
	#endif
	{
		LoadPrefFile(); WriteStrToDebugFile("\pLoadPrefFile.\r");
		
		if (isAppearanceAvailable) err=RegisterAppearanceClient();
		if (isContextualMenuAvailable) InitContextualMenus();
		
		MenuBarInit(); WriteStrToDebugFile("\pMenuBarInit.\r");
		ToolWindowInit(); WriteStrToDebugFile("\pToolWindowInit.\r");
		DotLibInit(); WriteStrToDebugFile("\pDotLibInit.\r");
		AEInit(); WriteStrToDebugFile("\pAEInit.\r");
		CurRgnInit(); WriteStrToDebugFile("\pCurRgnInit.\r");
		
		GWorldInit(); WriteStrToDebugFile("\pGWorldInit.\r");
		
		ChangeSysSettings(); WriteStrToDebugFile("\pChangeSysSettings.\r");
		
		#if !TARGET_API_MAC_CARBON
		if (isTSMTEAvailable) InitTSMAwareApplication();
		#endif
		#if TARGET_API_MAC_CARBON
			needToUnload = (NavLoad() == noErr);
		#else
		if (isNavServicesAvailable && useNavigationServices) needToUnload = (NavLoad() == noErr);
		#endif
		
		/* 乱数の初期化 */
		#if TARGET_API_MAC_CARBON
		{
			UInt32	time;
			
			GetDateTime(&time);
			SetQDGlobalsRandomSeed((SInt32)time);
		}
		#else
		GetDateTime((unsigned long*)(&qd.randSeed));
		#endif
		MainLoop();
		
		ResetSysSettings();
		
		#if !TARGET_API_MAC_CARBON
		if (isTSMTEAvailable) CloseTSMAwareApplication();
		#endif
		if (isNavServicesAvailable && needToUnload) NavUnload();
		
		/* 確保したメモリを解放 */
		DisposeGWorld(gBlendPalettePtr);
		DisposeRgn(gRulerRgn);
		DisposeRgn(gCurRgnHand);
		
		/* 終了前に初期設定を保存 */
		if (gPrefFileRefNum>0)
			SavePrefFile();
		if (gDotLibRefNum>0)
			CloseResFile(gDotLibRefNum);
	}
	else
	#if TARGET_API_MAC_CARBON
		ErrorAlertFromResource(IPERR_RESID,IPERR3);
	#else
		ErrorAlertFromResource(IPERR_RESID,IPERR1);
	#endif
	
	CloseDebugFile();
	
	#ifdef __APPLE_CC__
	return 0;
	#endif
}

/* マウスカーソル更新のためのリージョンの初期化 */
void CurRgnInit(void)
{
	gCurRgnHand=NewRgn();
}

/* 定数の初期化 */
void InitConstants(void)
{
	SetRGBColor(&rgbBlackColor,0x0000U,0x0000U,0x0000U);
	SetRGBColor(&rgbWhiteColor,0xffffU,0xffffU,0xffffU);
	SetRGBColor(&rgbLBColor,0x2222U,0x2222U,0x2222U);
	SetRect(&gToolRect,0x03,0x03,0x20,0x19);
	#if ENGLISH_VER
	InsetRect(&gToolRect,-1,-1);
	#endif
}

/* グローバル変数の初期化 */
void InitGlobals(void)
{
	OSErr	err;
	long	response;
	FSSpec	theFile;
	
	gIsDialog=false;
	gFrontWindow=nil;
	
	gBlendRatio=rgbBlackColor;
	gBlendMode=srcCopy;
	gBSelectedItem=i0percent;			/* 0%:不透明 */
	gCurrentColor.rgb=rgbBlackColor;	/* 黒 */
	gCurrentColor.isTransparent=false;	/* 透明色ではない */
	gPrevColor.rgb=rgbBlackColor;		/* 黒 */
	gPrevColor.isTransparent=false;		/* 透明色ではない */
	gBackColor.rgb=rgbWhiteColor;		/* 白 */
	gBackColor.isTransparent=false;		/* 透明色ではない */
	gSelectedTool=kPencilTool;				/* 鉛筆ツール */
	gPatternNo=1;						/* １番目のパターン（黒） */
	gPenWidth=1;						/* ペンの幅＝１ */
	gPenHeight=1;						/* ペンの高さ＝１ */
	gEraserWidth=2;						/* 消しゴムの幅＝２ */
	gEraserHeight=2;					/* 消しゴムの高さ＝２ */
	SetPt(&gPrevRulerPt,-1,-1);			/* ルーラーの表示されている場所（表示されていない） */
	gBlendLocked=false;					/* ブレンドパレットはロックされていない */
	
	/* タブレットが使用可能かどうかをチェック */
	isTabletAvailable=GetTablet();
	
	SetRGBColor(&rgbGrayColor,0x8000U,0x8000U,0x8000U);
	
	/* テンポラリファイルの番号は1から */
	gTempFileNum=1;
	
	/* ドラッグマネージャが使用可能かどうか */
	isDragMgrPresent= ( (Gestalt(gestaltDragMgrAttr, &response) == noErr) &&
			 ((response & (1L << gestaltDragMgrPresent)) != 0) );
	
	/* アピアランスマネージャが使用可能かどうか */
	err=Gestalt(gestaltAppearanceAttr,&response);
	isAppearanceAvailable=(err==noErr && (response & (kGestaltMask << gestaltAppearanceExists)));
	#if powerc
		if (RegisterAppearanceClient == nil) isAppearanceAvailable=false;
	#endif
	
	/* コンテクストメニューが使用可能かどうか */
	#if powerc
		isContextualMenuAvailable=(InitContextualMenus != nil);
	#else
		err=Gestalt(gestaltContextualMenuAttr,&response);
		isContextualMenuAvailable=(err==noErr &&
				(response & (kGestaltMask << gestaltContextualMenuTrapAvailable)));
	#endif
	
	/* Text Services Managerが使用可能かどうか */
	err=Gestalt(gestaltTSMgrVersion,&response);
	isTSMgrAvailable=(err==noErr);
	
	/* インライン入力ができるかどうか */
	err=Gestalt(kTSMTESignature,&response);
	isTSMTEAvailable=(err==noErr && (response &(kGestaltMask << gestaltTSMTE)));
	
	/* NavigationServicesが使用可能かどうか */
	isNavServicesAvailable=NavServicesAvailable();
	if (isNavServicesAvailable) gNavLibraryVersion=NavLibraryVersion();
	
	/* ヘルプマネージャが使用可能かどうか */
	err=Gestalt(gestaltHelpMgrAttr,&response);
	isHelpMgrAvailable=(err==noErr && (response & (kGestaltMask << gestaltHelpMgrPresent)));
	
	/* QuickTimeのバージョン */
	err=Gestalt(gestaltQuickTimeVersion,&response);
	gQTVersion=(err==noErr ? response : 0L);
	
	/* システムのバージョン */
	err=Gestalt(gestaltSystemVersion,&gSystemVersion);
	if (err!=noErr) gSystemVersion=0;
	
	#if TARGET_API_MAC_CARBON
	err=Gestalt(gestaltCarbonVersion,&gCarbonLibVersion);
	if (err!=noErr) gCarbonLibVersion=0;
	#endif
	
	/* OS Xかどうか */
	err=Gestalt(gestaltMenuMgrAttr,&response);
	if (err==noErr) isOSX=((response & gestaltMenuMgrAquaLayoutMask) != 0);
	else isOSX=false;
	
	/* IconServicesをチェック */
	err=Gestalt(gestaltIconUtilitiesAttr,&response);
	if (err==noErr)
	{
		isIconServicesAvailable=((response & (kGestaltMask << gestaltIconUtilitiesHasIconServices)) != 0);
		is32BitIconsAvailable=((response & (kGestaltMask << gestaltIconUtilitiesHas32BitIcons)) != 0);
		isThumbnailIconsAvailable=(gSystemVersion >= 0x0910);
	}
	else
	{
		isIconServicesAvailable=false;
		is32BitIconsAvailable=false;
		isThumbnailIconsAvailable=false;
	}
	
	/* キーの繰り返し状態を取得しておく */
	gKeyThreshStore.keyThresh=LMGetKeyThresh();
	gKeyThreshStore.keyRepThresh=LMGetKeyRepThresh();
	
	/* clip2gif、ResEditを探す */
	err=GetApplSpec(kResEditCreator,&theFile);
	isResEditAvailable=(err==noErr);
	
	err=GetApplSpec(kClip2gifCreator,&theFile);
	isClip2gifAvailable=(err==noErr);
	
	/* ルーラ */
	gRulerRgn=NewRgn();
}

/* 描画点ライブラリの初期化 */
void DotLibInit(void)
{
	OSErr	err;
	ProcessSerialNumber	psn;
	ProcessInfoRec		processInfo;
	FSSpec	applSpec,folderSpec,theFile;
	Boolean	isDirectory;
	
	/* 描画点ライブラリフォルダを探す */
	/* まずアプリケーションの位置を得る */
	err=GetCurrentProcess(&psn);
	processInfo.processInfoLength=sizeof(ProcessInfoRec);
	processInfo.processName=nil;
	processInfo.processAppSpec=&applSpec;
	err=GetProcessInformation(&psn,&processInfo);
	
	GetIndString(applSpec.name,150,2);
	err=FSMakeFSSpec(applSpec.vRefNum,applSpec.parID,applSpec.name,&folderSpec);
	if (err==fnfErr)
		isDotLibAvailable=false;
	else if (err==noErr)
	{
		#ifdef __MOREFILESX__
		FSRef	fsRef;
		
		err = FSpMakeFSRef(&folderSpec,&fsRef);
		err = FSGetNodeID(&fsRef,&gDotLibFolderID,&isDirectory);
		#else
		err=FSpGetDirectoryID(&folderSpec,&gDotLibFolderID,&isDirectory);
		#endif
		if (err==noErr)
		{
			isDotLibAvailable=true;
			gDotLibVRefNum=folderSpec.vRefNum;
		}
		else isDotLibAvailable=false;
	}
	
	/* 最後に開いていたライブラリがあればそれを開く */
	if (gDotLibName[0]>0 && isDotLibAvailable)
	{
		err=FSMakeFSSpec(gDotLibVRefNum,gDotLibFolderID,gDotLibName,&theFile);
		if (err==noErr)
			gDotLibRefNum=FSpOpenResFile(&theFile,fsRdWrPerm);
		else
			gDotLibName[0]=0;
	}
	if (gDotLibName[0]==0)
	{
		gDotLibRefNum=-1;
		GetIndString(gDotLibName,150,3);
	}
}

/* オフポートの初期化 */
void GWorldInit(void)
{
	OSErr	err;
	Rect	r;
	
	/* オフポートの作成 */
	err=NewGWorld(&gBlendPalettePtr,8,GetWindowPortBounds(gBlendPalette,&r),0,0,useTempMem);
	
	if (err!=noErr) /* if failed then quit */
	{
		ErrorAlertFromResource(IPERR_RESID,IPERR2);
		ExitToShell();
		quit=true;
		return;
	}
	
	DrawBlend();
}

/* オフポートの画像を消去 */
void EraseOffPort(PaintWinRec *eWinRec)
{
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	GetGWorld(&cPort,&cDevice);
	
	SetGWorld(eWinRec->editDataPtr,0);
	LockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
	EraseRect(&eWinRec->iconSize);
	UnlockPixels(GetGWorldPixMap(eWinRec->editDataPtr));
	
	SetGWorld(eWinRec->tempDataPtr,0);
	LockPixels(GetGWorldPixMap(eWinRec->tempDataPtr));
	EraseRect(&eWinRec->iconSize);
	UnlockPixels(GetGWorldPixMap(eWinRec->tempDataPtr));
	
	SetGWorld(eWinRec->selectedDataPtr,0);
	LockPixels(GetGWorldPixMap(eWinRec->selectedDataPtr));
	EraseRect(&eWinRec->iconSize);
	UnlockPixels(GetGWorldPixMap(eWinRec->selectedDataPtr));
	
	SetGWorld(eWinRec->dispTempPtr,0);
	LockPixels(GetGWorldPixMap(eWinRec->dispTempPtr));
	EraseRect(&eWinRec->iconSize);
	UnlockPixels(GetGWorldPixMap(eWinRec->dispTempPtr));
	
	/* マスク */
	MySetGWorld(currentMask);
	MyLockPixels(currentMask);
	EraseRect(&eWinRec->iconSize);
	MyUnlockPixels(currentMask);
	
	MySetGWorld(pCurrentMask);
	MyLockPixels(pCurrentMask);
	EraseRect(&eWinRec->iconSize);
	MyUnlockPixels(pCurrentMask);
	
	MySetGWorld(selectionMask);
	MyLockPixels(selectionMask);
	EraseRect(&eWinRec->iconSize);
	MyUnlockPixels(selectionMask);
	
	SetGWorld(cPort,cDevice);
}

/* システムの設定を変更する */
void ChangeSysSettings(void)
{
	LMSetKeyThresh(gToolPrefs.dotDrawPrefs.keyThresh);
	LMSetKeyRepThresh(gToolPrefs.dotDrawPrefs.keyRepThresh);
}

/* システムの設定を元に戻す */
void ResetSysSettings(void)
{
	LMSetKeyThresh(gKeyThreshStore.keyThresh);
	LMSetKeyRepThresh(gKeyThreshStore.keyRepThresh);
}

/* メインイベントループ */
void MainLoop(void)
{
	EventRecord	event;
	
	quit=false;
	
	while(!quit)
	{
		if (WaitNextEvent(everyEvent,&event,kSleep,gCurRgnHand))
			DoEvent(&event);
		else
			DoIdleEvent(&event);
	}
}

/* イベント処理 */
void DoEvent(EventRecord *theEvent)
{
	char	theChar;
	WindowPtr	theWindow;
	long	menuChoice;
	
	#if !TARGET_API_MAC_CARBON
	if (!gIsDialog)
	{
		theWindow=MyFrontNonFloatingWindow();
		if (theWindow != nil)
		{
			if (GetExtWindowKind(theWindow) == kWindowTypeIconFamilyWindow)
				if (isTSMTEAvailable && TSMEvent(theEvent))
				{
					SetTSMCursor(theEvent->where);
					return;
				}
		}
	}
	#endif
	
	switch (theEvent->what)
	{
		case mouseDown:
			HandleMouseDown(theEvent);
			break;
		
		case mouseUp:
			break;
		
		case keyDown:
		case autoKey:
			if (gSystemVersion >= 0x0800)
			{
				KeyMap	theKey;
				
				GetKeys(theKey);
				long temp;
#if TARGET_RT_LITTLE_ENDIAN
				temp = EndianS32_BtoN (theKey[1].bigEndianValue);
#else
				temp = theKey[1];
#endif	
				if (temp&0x00000004) theEvent->modifiers |= optionKey;
				
				menuChoice=MenuEvent(theEvent);
			}
			else
			{
				Ptr		kCHRPtr;
				UInt32	state = 0;
				
				kCHRPtr = ( Ptr )GetScriptManagerVariable( smKCHRCache );
				theChar = (char)KeyTranslate( kCHRPtr, (UInt16)(theEvent->message & keyCodeMask)>>8,
											 &state )&charCodeMask;
				
				if ((theEvent->modifiers &cmdKey)!=0)
				{
					menuChoice=MenuKey(theChar);
				}
				else menuChoice=0;
			}
			if (menuChoice != 0)
			{
				UseResFile(gApplRefNum);
				HandleMenuChoice(menuChoice);
				break;
			}
			else //if (!(theEvent->modifiers & cmdKey))
			{
				/* キーダウンに対する処理 */
				HandleKeyDown(theEvent);
			}
			break;
		
		case updateEvt:
			DoUpdate(theEvent);
			break;
		
		case osEvt:
			switch((unsigned long)(theEvent->message &osEvtMessageMask) >> 24)
			{
				/* 他のアプリケーションから戻ってきたときにクリップボードの状態をチェック */
				/* ペーストメニューに反映する */
				case suspendResumeMessage:
					if (theEvent->message & resumeFlag) /* resume */
					{
						ResumeFloatingWindows();
						//if (theEvent->message & convertClipboardFlag) /* 本当はこうするべきだが */
							UpdatePasteMenu();
						ChangeSysSettings();
						ResetRuler();
					}
					else /* suspend */
					{
						SuspendFloatingWindows();
						ResetSysSettings();
						
						theWindow = MyFrontNonFloatingWindow();
						if (GetExtWindowKind(theWindow) == kWindowTypePaintWindow)
						{
							ResetDot(theWindow);
						}
					}
					break;
				
				/* マウスの位置によってマウスカーソルの形を変える */
				case mouseMovedMessage:
					UpdateMouseCursor(theEvent->where);
					break;
			}
			break;
		
		case kHighLevelEvent:
			AEProcessAppleEvent(theEvent);
			break;
		
		default:
			break;
	}
}

/* アイドルイベント（イベントがないとき）の処理 */
void DoIdleEvent(EventRecord *theEvent)
{
	WindowPtr	theWindow=MyFrontNonFloatingWindow();
	IconFamilyWinRec	*fWinRec;
	
	/* マウスカーソルの形を変える */
	UpdateMouseCursor(theEvent->where);
	#if !TARGET_API_MAC_CARBON
	UpdateBalloonHelp(theEvent->where);
	#endif
	
	if (theWindow==nil) return;
	
	switch (GetExtWindowKind(theWindow))
	{
		case kWindowTypeIconFamilyWindow:
			/* カーソルの点滅 */
			fWinRec=GetIconFamilyRec(theWindow);
			
			if ((**fWinRec->iconNameTE).active)
				TEIdle(fWinRec->iconNameTE);
			
			break;
		
		case kWindowTypePaintWindow:
			InvertDot(theWindow);
			break;
	}
	
	/* 外部エディタで編集中のアイコンを更新 */
	UpdateXIconList();
}

/* ドット点滅処理 */
void InvertDot(WindowPtr theWindow)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	
	if (eWinRec->isDotMode)
	{
		if (TickCount() - eWinRec->lastInvertedTime > GetCaretTime())
		{
			GrafPtr	port;
			
			GetPort(&port);
			SetPortWindowPort(theWindow);
			InvertDotMain(eWinRec);
			SetPort(port);
		}
	}
}

/* ドット点滅メイン */
void InvertDotMain(PaintWinRec *eWinRec)
{
	Rect	r;
	
	SetRect(&r,eWinRec->dotPos.h << eWinRec->ratio,eWinRec->dotPos.v << eWinRec->ratio,
			(eWinRec->dotPos.h+gPenWidth) << eWinRec->ratio,(eWinRec->dotPos.v+gPenHeight)<< eWinRec->ratio);
	
	InvertRect(&r);
	
	eWinRec->isInverted=(UInt8)!eWinRec->isInverted;
	eWinRec->lastInvertedTime=TickCount();
}

/* ドット消去処理 */
void ResetDot(WindowPtr theWindow)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	
	if (eWinRec->isDotMode && eWinRec->isInverted)
	{
		GrafPtr	port;
		
		GetPort(&port);
		SetPortWindowPort(theWindow);
		InvertDotMain(eWinRec);
		SetPort(port);
	}
}

/* 描画点モードのキー処理 */
Boolean HandleDotKey(short eventKind,char theChar,WindowPtr theWindow)
{
	Boolean	result=false;
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	
	switch (theChar)
	{
		case '5':
		case 'k':
		case 'K':
			/* 描画 */
			if (eventKind == keyDown)
			{
				ResetDot(theWindow);
				
				DoDotPaintMain(theWindow,eWinRec->dotPos);
				PutCommand(eWinRec->dotCommand,'5');
			}
			result=true;
			break;
		
		case '1':
			MoveDot(theWindow,-1,1);
			PutCommand(eWinRec->dotCommand,'1');
			result=true;
			break;
		
		case '2':
		case ',':
			MoveDot(theWindow,0,1);
			PutCommand(eWinRec->dotCommand,'2');
			result=true;
			break;
		
		case '3':
			MoveDot(theWindow,1,1);
			PutCommand(eWinRec->dotCommand,'3');
			result=true;
			break;
		
		case '4':
		case 'j':
		case 'J':
			MoveDot(theWindow,-1,0);
			PutCommand(eWinRec->dotCommand,'4');
			result=true;
			break;
		
		case '6':
		case 'l':
		case 'L':
			MoveDot(theWindow,1,0);
			PutCommand(eWinRec->dotCommand,'6');
			result=true;
			break;
		
		case '7':
			MoveDot(theWindow,-1,-1);
			PutCommand(eWinRec->dotCommand,'7');
			result=true;
			break;
		
		case '8':
		case 'i':
		case 'I':
			MoveDot(theWindow,0,-1);
			PutCommand(eWinRec->dotCommand,'8');
			result=true;
			break;
		
		case '9':
			MoveDot(theWindow,1,-1);
			PutCommand(eWinRec->dotCommand,'9');
			result=true;
			break;
		
		case '.':
			/* 描画点モードから抜ける */
			ResetDot(theWindow);
			eWinRec->isDotMode=false;
			HideReferencedWindow(DotModePalette);
			UpdateEffectMenu();
			result=true;
			break;
		
		case '0':
			/* 記録 */
			if (eWinRec->dotCommand[0]!=0) /* 入力されている */
			{
				GrafPtr	port;
				
				PStrCpy(eWinRec->dotCommand,gDotCommand);
				eWinRec->dotCommand[0]=0;
				
				GetPort(&port);
				SetPortWindowPort(DotModePalette);
				UpdateRecordedCommand(gDotCommand);
				UpdateInputCommand(eWinRec->dotCommand);
				SetPort(port);
			}
			break;
		
		case 0x1b:
			/* clear -- 入力中コマンドを消去する */
			if (eWinRec->dotCommand[0]!=0)
			{
				GrafPtr	port;
				
				eWinRec->dotCommand[0]=0;
				
				GetPort(&port);
				SetPortWindowPort(DotModePalette);
				UpdateInputCommand(eWinRec->dotCommand);
				SetPort(port);
			}
			break;
		
		case 0x0d:
			/* return -- 記録されているコマンドを再生する */
			if (gDotCommand[0]!=0)
			{
				ExecuteCommand(theWindow,gDotCommand);
			}
			break;
	}
	
	return result;
}

/* ドットを動かす */
void MoveDot(WindowPtr theWindow,short dx,short dy)
{
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	
	/* まずドットを消す */
	ResetDot(theWindow);
	
	/* 新しい位置を計算 */
	MoveDotMain(&eWinRec->dotPos,dx,dy,&eWinRec->iconSize);
}

/* コマンドを記録 */
void PutCommand(Str255 command,char newCommand)
{
	short	commandLength=command[0];
	char	lastCommand;
	short	twoCommands;
	GrafPtr	port;
	
	/* なにも記録されていない時はつなげるだけ */
	if (commandLength==0)
	{
		command[0]=1;
		command[1]=newCommand;
	}
	else
	{
		lastCommand=command[commandLength];
		twoCommands=(((short)lastCommand)<<8)+newCommand;
		
		switch (twoCommands)
		{
			case '19': case '91': case '37': case '73': case '46': case '64': case '28': case '82':
				command[0]--;
				break;
			
			case '26': case '62':
				command[commandLength]='3';
				break;
			
			case '24': case '42':
				command[commandLength]='1';
				break;
			
			case '48': case '84':
				command[commandLength]='7';
				break;
			
			case '68': case '86':
				command[commandLength]='9';
				break;
			
			case '16': case '61': case '34': case '43':
				command[commandLength]='2';
				break;
			
			case '38': case '83': case '29': case '92':
				command[commandLength]='6';
				break;
			
			case '67': case '76': case '49': case '94':
				command[commandLength]='8';
				break;
			
			case '18': case '81': case '27': case '72':
				command[commandLength]='4';
				break;
			
			case '51':
				command[commandLength]='z';
				break;
			
			case '52':
				command[commandLength]='x';
				break;
			
			case '53':
				command[commandLength]='c';
				break;
			
			case '54':
				command[commandLength]='a';
				break;
			
			case '56':
				command[commandLength]='d';
				break;
			
			case '57':
				command[commandLength]='q';
				break;
			
			case '58':
				command[commandLength]='w';
				break;
			
			case '59':
				command[commandLength]='e';
				break;
			
			case 'z9': case 'e1': case 'c7': case 'q3': case 'a6': case 'd4': case 'x8': case 'w2':
				command[commandLength]='5';
				break;
			
			case 'x6': case 'd2':
				command[commandLength]='c';
				break;
			
			case 'x4': case 'a2':
				command[commandLength]='z';
				break;
			
			case 'a8': case 'w4':
				command[commandLength]='q';
				break;
			
			case 'd8': case 'w6':
				command[commandLength]='e';
				break;
			
			case 'z6': case 'd1': case 'c4': case 'a3':
				command[commandLength]='x';
				break;
			
			case 'c8': case 'w3': case 'x9': case 'e2':
				command[commandLength]='d';
				break;
			
			case 'd7': case 'q6': case 'a9': case 'e4':
				command[commandLength]='w';
				break;
			
			case 'z8': case 'w1': case 'x7': case 'q2':
				command[commandLength]='a';
				break;
			
			default:
				CatChar2(newCommand,command);
		}
	}
	
	GetPort(&port);
	SetPortWindowPort(DotModePalette);
	UpdateInputCommand(command);
	SetPort(port);
}

/* 文字を文字列につなげる */
void CatChar2(char c,Str255 string)
{
	if (string[0]==kDotCommandMaxLength)
	{
		BlockMove(&string[2],&string[1],kDotCommandMaxLength-1);
		string[kDotCommandMaxLength]=c;
	}
	else
	{
		string[0]++;
		string[string[0]]=c;
	}
}

/* コマンドの実行 */
void ExecuteCommand(WindowPtr theWindow,Str31 command)
{
	short	i;
	char	c;
	PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
	Point	pt=eWinRec->dotPos;
	
	ResetDot(theWindow);
	
	for (i=1; i<=command[0]; i++)
	{
		c=command[i];
		switch (c)
		{
			case '5':
				/* 描画 */
				DoDotPaintMain(theWindow,pt);
				break;
			
			case 'z':
				DoDotPaintMain(theWindow,pt);
			case '1':
				MoveDotMain(&pt,-1,1,&eWinRec->iconSize);
				break;
			
			case 'x':
				DoDotPaintMain(theWindow,pt);
			case '2':
				MoveDotMain(&pt,0,1,&eWinRec->iconSize);
				break;
			
			case 'c':
				DoDotPaintMain(theWindow,pt);
			case '3':
				MoveDotMain(&pt,1,1,&eWinRec->iconSize);
				break;
			
			case 'a':
				DoDotPaintMain(theWindow,pt);
			case '4':
				MoveDotMain(&pt,-1,0,&eWinRec->iconSize);
				break;
			
			case 'd':
				DoDotPaintMain(theWindow,pt);
			case '6':
				MoveDotMain(&pt,1,0,&eWinRec->iconSize);
				break;
			
			case 'q':
				DoDotPaintMain(theWindow,pt);
			case '7':
				MoveDotMain(&pt,-1,-1,&eWinRec->iconSize);
				break;
			
			case 'w':
				DoDotPaintMain(theWindow,pt);
			case '8':
				MoveDotMain(&pt,0,-1,&eWinRec->iconSize);
				break;
			
			case 'e':
				DoDotPaintMain(theWindow,pt);
			case '9':
				MoveDotMain(&pt,1,-1,&eWinRec->iconSize);
				break;
		}
	}
	
	eWinRec->dotPos=pt;
}

/* 新しい位置にドットを移動させる */
void MoveDotMain(Point *pt,short dx,short dy,Rect *r)
{
	Point	newPt=*pt;
	Rect	newRect=*r;
	
	newRect.left-=gPenWidth-1;
	newRect.top-=gPenHeight-1;
	
	newPt.h+=dx*gPenWidth;
	newPt.v+=dy*gPenHeight;
	if (PtInRect(newPt,&newRect))
		*pt=newPt;
}

#if !TARGET_API_MAC_CARBON
/* バルーンヘルプ処理 */
void UpdateBalloonHelp(Point globPt)
{
	GrafPtr		port;
	WindowPtr	theWindow;
	short		part;
	Rect		r;
	Point		localPt;
	ControlHandle	theControl;
	short		index;
	
	enum {
		kPaintWindowHelpID=3000,
		kPreviewWindowHelpID,
		kIconListWindowHelpID,
		kIconFamilyWindowHelpID,
		kColorPaletteHelpID,
		kToolPaletteHelpID,
		kBlendPaletteHelpID,
		kTitleWindowHelpID,
		kInfoWindowHelpID,
		kPatternPaletteHelpID,
		kScrollBarHelpID,
		kSizeBoxHelpID,
		kDotModePaletteHelpID,
		kFavoritePaletteHelpID,
	};
	
	enum {
		kPaintContentIndex=1,
		kPaintRatioIndex,
		kPaintBGIndex,
	};
	
	enum {
		kContentIndex=1,
	};
	
	enum {
		kPencilToolIndex=1,
		kEraserToolIndex,
		kMarqueeToolIndex,
		kSpoitToolIndex,
		kBucketToolIndex,
	};
	
	enum {
		kBlendCurrentIndex=1,
		kBlendBackIndex,
		kBlendExchangeIndex,
		kBlendLockIndex,
		kBlendLightDarkIndex,
		kBlendPreviousIndex,
	};
	
	enum {
		kScrollBarEnableIndex=1,
		kScrollBarDisableIndex,
	};
	
	enum {
		kSizeBoxIndex=1,
	};
	
	enum {
		kDotLibNameIndex=1,
		kRecordedCmdIndex,
		kInputCmdIndex,
		kDotLibPopupIndex,
		kDotCmdPopupIndex,
	};
	
	/* ヘルプマネージャが使用可能でない、あるいはバルーンヘルプがオフの場合はなにもしない */
	if (!isHelpMgrAvailable) return;
	if (!HMGetBalloons()) return;
	
	part=FindWindow(globPt,&theWindow);
	if (theWindow==nil) /* ウィンドウ内でなければなにもしない */
	{
		MyShowBalloon(nil,-1,0);
		return;
	}
	
	if (part != inContent && part != inGrow)
	{
		MyShowBalloon(nil,-1,0);
		return;
	}
	
	GetPort(&port);
	SetPort(theWindow);
	
	localPt=globPt;
	GlobalToLocal(&localPt);
	
	if (part==inGrow)
	{
		r=theWindow->portRect;
		r.left=r.right-kScrollBarWidth;
		r.top=r.bottom-kScrollBarHeight;
		MyShowBalloon(&r,kSizeBoxHelpID,kContentIndex);
	}
	else
	{	
		switch (GetExtWindowKind(theWindow))
		{
			case kWindowTypePaintWindow:
				r=theWindow->portRect;
				r.right-=kScrollBarWidth;
				r.bottom-=kScrollBarHeight;
				if (PtInRect(localPt,&r)) /* ペイント領域内 */
					MyShowBalloon(&r,kPaintWindowHelpID,kPaintContentIndex);
				else
				{
					r=theWindow->portRect;
					r.right=r.left+kRatioWidth;
					r.top=r.bottom-kScrollBarHeight;
					if (PtInRect(localPt,&r)) /* 倍率領域内 */
						MyShowBalloon(&r,kPaintWindowHelpID,kPaintRatioIndex);
					else
					{
						r.right+=kBackWidth;
						r.left+=kRatioWidth;
						if (PtInRect(localPt,&r)) /* 背景情報内 */
							MyShowBalloon(&r,kPaintWindowHelpID,kPaintBGIndex);
						else /* スクロールバー */
						{
							part=FindControl(localPt,theWindow,&theControl);
							
							r.left+=kBackWidth;
							r.right=theWindow->portRect.right-kScrollBarWidth;
							if (PtInRect(localPt,&r)) /* 水平スクロールバー */
								MyShowBalloon(&r,kScrollBarHelpID,(part && theControl) ? 
												kScrollBarEnableIndex : kScrollBarDisableIndex);
							else
							{
								r=theWindow->portRect;
								r.left=r.right-kScrollBarWidth;
								r.bottom-=kScrollBarHeight;
								MyShowBalloon(&r,kScrollBarHelpID,(part && theControl) ?
												kScrollBarEnableIndex : kScrollBarDisableIndex);
							}
						}
					}
				}
				break;
			
			case kWindowTypePreviewWindow:
				MyShowBalloon(&theWindow->portRect,kPreviewWindowHelpID,kContentIndex);
				break;
			
			case kWindowTypeIconListWindow:
				r=theWindow->portRect;
				r.right-=kScrollBarWidth;
				if (PtInRect(localPt,&r))
					MyShowBalloon(&r,kIconListWindowHelpID,kContentIndex);
				else
				{
					part=FindControl(localPt,theWindow,&theControl);
					r.left=r.right;
					r.right+=kScrollBarWidth;
					MyShowBalloon(&r,kScrollBarHelpID,(part && theControl) ? 
									kScrollBarEnableIndex : kScrollBarDisableIndex);
				}
				break;
			
			case kWindowTypeIconFamilyWindow:
				MyShowBalloon(&theWindow->portRect,kIconFamilyWindowHelpID,kContentIndex);
				break;
			
			case kWindowTypeColorPalette1:
			case kWindowTypeColorPalette2:
				MyShowBalloon(&theWindow->portRect,kColorPaletteHelpID,kContentIndex);
				break;
			
			case kWindowTypeToolPalette:
				index=(localPt.v-2)/25;
				r=theWindow->portRect;
				r.top=index*25+2;
				r.bottom=index*25+25+2;
				MyShowBalloon(&r,kToolPaletteHelpID,index+1);
				break;
			
			case kWindowTypeBlendPalette:
				SetRect(&r,13,13,22,17);
				if (PtInRect(localPt,&r))
				{
					MyShowBalloon(&r,kBlendPaletteHelpID,kBlendExchangeIndex);
					break;
				}
				SetRect(&r,2,2,16,16);
				if (PtInRect(localPt,&r))
				{
					MyShowBalloon(&r,kBlendPaletteHelpID,kBlendCurrentIndex);
					break;
				}
				SetRect(&r,19,2,33,16);
				if (PtInRect(localPt,&r))
				{
					MyShowBalloon(&r,kBlendPaletteHelpID,kBlendBackIndex);
					break;
				}
				SetRect(&r,1,18,34,26);
				if (PtInRect(localPt,&r))
				{
					MyShowBalloon(&r,kBlendPaletteHelpID,kBlendLockIndex);
					break;
				}
				r=theWindow->portRect;
				if (localPt.v > 16)
				{
					if (localPt.h < 16)
					{
						r.top=0x16;
						r.right=0x11;
						MyShowBalloon(&r,kBlendPaletteHelpID,kBlendLightDarkIndex);
					}
					else if (localPt.h >=19)
					{
						r.top=0x16;
						r.left=0x13;
						MyShowBalloon(&r,kBlendPaletteHelpID,kBlendPreviousIndex);
					}
				}
				else
					MyShowBalloon(nil,-1,0);
				break;
			
			case kWindowTypeTitleWindow:
				MyShowBalloon(&theWindow->portRect,kTitleWindowHelpID,kContentIndex);
				break;
			
			case kWindowTypeInfoWindow:
				MyShowBalloon(&theWindow->portRect,kInfoWindowHelpID,kContentIndex);
				break;
			
			case kWindowTypePatternPalette:
				MyShowBalloon(&theWindow->portRect,kPatternPaletteHelpID,kContentIndex);
				break;
			
			case kWindowTypeDotModePalette:
				SetRect(&r,0x05,0x03,0x4f,0x0e);
				if (PtInRect(localPt,&r))
				{
					MyShowBalloon(&r,kDotModePaletteHelpID,kDotLibNameIndex);
					break;
				}
				SetRect(&r,0x05,0x13,0x4f,0x1b);
				if (PtInRect(localPt,&r))
				{
					MyShowBalloon(&r,kDotModePaletteHelpID,kRecordedCmdIndex);
					break;
				}
				SetRect(&r,0x05,0x22,0x60,0x3f);
				if (PtInRect(localPt,&r))
				{
					MyShowBalloon(&r,kDotModePaletteHelpID,kInputCmdIndex);
					break;
				}
				SetRect(&r,0x55,0x03,0x60,0x0e);
				if (PtInRect(localPt,&r))
				{
					MyShowBalloon(&r,kDotModePaletteHelpID,kDotLibPopupIndex);
					break;
				}
				SetRect(&r,0x55,0x12,0x60,0x1d);
				if (PtInRect(localPt,&r))
				{
					MyShowBalloon(&r,kDotModePaletteHelpID,kDotCmdPopupIndex);
					break;
				}
				break;
			
			case kWindowTypeFavoritePalette:
				MyShowBalloon(&theWindow->portRect,kFavoritePaletteHelpID,kContentIndex);
				break;
			
			default:
				MyShowBalloon(nil,-1,0);
		}
	}
	SetPort(port);
}

static short	gBalloonID=-1,gBalloonIndex;	

/* バルーンヘルプの表示 */
void MyShowBalloon(const Rect *r,short id,short index)
{
	HMMessageRecord	aHelpMsg;
	Point			tip;
	OSErr			err;
	Rect			rr;
	
	typedef struct {
		Point	topLeft;
		Point	botRight;
	} Rect2;
	
	if (gBalloonID==id && gBalloonIndex==index) return;
	
	if (id < 0)
	{
		gBalloonID=-1;
	}
	else
	{
		rr=*r;
		
		aHelpMsg.hmmHelpType=khmmStringRes;
		aHelpMsg.u.hmmStringRes.hmmResID=id;
		aHelpMsg.u.hmmStringRes.hmmIndex=index;
		
		LocalToGlobal(&((Rect2 *)&rr)->topLeft);
		LocalToGlobal(&((Rect2 *)&rr)->botRight);
		
		SetPt(&tip,(rr.right + rr.left) / 2, (rr.bottom + rr.top) / 2);
		
		err=HMShowBalloon(&aHelpMsg,tip,&rr,nil,0,0,kHMRegularWindow);
		if (err==noErr)
		{
			gBalloonID=id;
			gBalloonIndex=index;
		}
		else
			gBalloonID=-1;
	}
}
#endif

/* マウスダウンイベントの処理 */
void HandleMouseDown(EventRecord *theEvent)
{
	WindowPtr	theWindow;
	short		thePart;
	long		menuChoice;
	Boolean		isFront;
	short		windowKind;
	
	theWindow=MyFrontNonFloatingWindow();
	if (GetExtWindowKind(theWindow) == kWindowTypePaintWindow)
	{
		ResetDot(theWindow);
	}
	
	thePart=FindWindow(theEvent->where,&theWindow);
	
	switch (thePart)
	{
		case inMenuBar:
			MySetCursor(0);
			menuChoice=MenuSelect(theEvent->where);
			HandleMenuChoice(menuChoice);
			break;
		
		#if !TARGET_API_MAC_CARBON
		case inSysWindow:
			SystemClick(theEvent,theWindow);
			break;
		#endif
		
		case inContent:
			ContentClick(theEvent);
			break;
		
		#if powerc
		case inProxyIcon:
			if (gSystemVersion >= 0x0850)
			{
				OSErr	err;
				
				err=TrackWindowProxyDrag(theWindow,theEvent->where);
				if (err==errUserWantsToDragWindow)
				{
				}
			}
			break;
		#endif
		
		case inDrag:
			isFront=(MyFrontNonFloatingWindow()==theWindow || GetWindowKind(theWindow) == kApplicationFloaterKind);
			windowKind=GetExtWindowKind(theWindow);
			
			#if powerc
			if (gSystemVersion >= 0x0850)
			{
				OSErr	err;
				SInt32	menuResult;
				Boolean	found=false;
				
				if (IsWindowPathSelectClick(theWindow,theEvent))
				{
					err=WindowPathSelect(theWindow,NULL,&menuResult);
					if (LoWord(menuResult) > 1)
					{
						ProcessSerialNumber	psn;
						/* Finderを探す */
						found=FindProcessFromCreatorAndType(kFinderCreator,kFinderType,&psn);
						if (found)
							err=SetFrontProcess(&psn);
					}
				}
			}
			else
			#endif
			{
				if (isFront && (theEvent->modifiers &cmdKey)!=0 && 
						(windowKind == kWindowTypePaintWindow || windowKind == kWindowTypeIconListWindow))
				{
					/* ポップアップメニューかもしんない */
					Point	mousePt=theEvent->where;
					short	fontID=GetPortTextFont(GetWindowPort(theWindow));
					short	titleWidth;
					Str255	title;
					Rect	r;
					short	windowWidth=GetWindowPortBounds(theWindow,&r)->right;
					
					SetPortWindowPort(theWindow);
					GlobalToLocal(&mousePt);
					TextFont(0);
					GetWTitle(theWindow,title);
					titleWidth=StringWidth(title);
					TextFont(fontID);
					
					if (mousePt.v<0 && ( mousePt.h > (windowWidth-titleWidth)/2 &&
										 mousePt.h < (windowWidth+titleWidth)/2 ))
					{
						Point	popPos;
						
						SetPt(&popPos,(windowWidth-titleWidth)/2-20,-17);
						LocalToGlobal(&popPos);
						
						MySetCursor(0);
						TitlePopup(theWindow,popPos);
						
						break;
					}
				}
			}
			if (windowKind == kWindowTypePaintWindow)
			{
				if ((theEvent->modifiers & controlKey)!=0)
				{
					GrafPtr	port;
					
					/* 倍率ポップアップメニュー */
					GetPort(&port);
					SetPortWindowPort(theWindow);
					
					MySetCursor(0);
					ChangeRatio(theWindow,theEvent->where);
					
					SetPort(port);
					break;
				}
			}
			{
				Rect	myScreenRect;
				
				GetRegionBounds(GetGrayRgn(),&myScreenRect);
				DragReferencedWindow(theWindow,theEvent->where,&myScreenRect);
			}
			
			/* ウィンドウを動かしたあとには、一番前のウィンドウが変化したりするから、メニューの更新が必要 */
			if (!isFront)
				UpdateMenus();
			break;
		
		case inGoAway:
			if (TrackGoAway(theWindow,theEvent->where))
			{
				switch (GetExtWindowKind(theWindow))
				{
					case kWindowTypePaintWindow:
						ClosePaintWindow(theWindow,false);
						UpdateMenus(); /* ウィンドウを閉じるといちばん前のウィンドウが変化するからメニューを更新 */
						break;
					
					case kWindowTypeIconListWindow:
						CloseIconFile(theWindow,false);
						UpdateMenus();
						break;
					
					case kWindowTypeIconFamilyWindow:
						CloseFamilyWindow(theWindow,true,false);
						UpdateMenus();
						break;
					
					case kWindowTypeDotModePalette:
						HideReferencedWindow(theWindow);
						{
							PaintWinRec	*eWinRec;
							
							theWindow=MyFrontNonFloatingWindow();
							if (theWindow==nil || GetExtWindowKind(theWindow)!=kWindowTypePaintWindow) break;
							
							eWinRec=GetPaintWinRec(theWindow);
							ResetDot(theWindow);
							eWinRec->isDotMode=false;
							UpdateEffectMenu();
						}
						break;
					
					default:
						ShowHidePalette(theWindow,false);
						break;
				}
			}
			break;
		
		case inGrow:
			switch (GetExtWindowKind(theWindow))
			{
				case kWindowTypeIconListWindow:
					ResizeIconWindow(theWindow,theEvent->where);
					break;
				
				case kWindowTypePaintWindow:
					ResetRuler();
					ResizePaintWindow(theWindow,theEvent->where);
					break;
			}
			break;
		
		case inZoomIn:
		case inZoomOut:
			MyZoomWindow(theWindow,thePart,theEvent);
			break;
	}
}

/* キーダウンイベントの処理 */
void HandleKeyDown(EventRecord *theEvent)
{
	short	dx[4]={-1,1,0,0},dy[4]={0,0,1,-1};
	WindowPtr	theWindow;
	SInt16	keyCode,theChar;
	PaintWinRec	*eWinRec;
	short	moveRatio;
	IconListWinRec	*iWinRec;
	IconFamilyWinRec	*fWinRec;
	Boolean	optDown=(Boolean)((theEvent->modifiers & optionKey) !=0);
	Boolean shiftDown=(Boolean)((theEvent->modifiers & shiftKey) !=0);
	
	keyCode = (SInt16)( theEvent->message & keyCodeMask ) >> 8;
	theChar = (SInt16)( theEvent->message & charCodeMask );
	
	theWindow=MyFrontNonFloatingWindow();
	
	if (theWindow!=nil)
	{
		switch (GetExtWindowKind(theWindow))
		{
			case kWindowTypeIconFamilyWindow:
				/* アイコンファミリウィンドウ */
				fWinRec=GetIconFamilyRec(theWindow);
				if (theChar == 0x09)
				{
					/* タブ */
					SelectNextIcon(fWinRec,shiftDown);
					return;
					break;
				}
				else if ((**fWinRec->iconNameTE).active)
				{
					if (theChar != kReturnCharCode && theChar != kEnterCharCode)
					{
						switch (theChar)
						{
							case kLeftArrowCharCode:
							case kRightArrowCharCode:
							case kUpArrowCharCode:
							case kDownArrowCharCode:
								TEKey(theChar,fWinRec->iconNameTE);
								break;
							
							default:
								TEKey(theChar,fWinRec->iconNameTE);
								fWinRec->wasChanged=true;
								UpdateSaveMenu();
								break;
						}
						UpdateClipMenu();
						return;
					}
					return;
				}
				else
				{
					switch (theChar)
					{
						case kReturnCharCode:
						case kEnterCharCode:
							/* リターン or エンター */
							EditFamilyIcon(theWindow,kForceNone);
							return;
							break;
						
						case kLeftArrowCharCode:
						case kRightArrowCharCode:
						case kUpArrowCharCode:
						case kDownArrowCharCode:
							/* カーソルキー */
							SelectNextIcon2(fWinRec,dx[keyCode-0x7b],dy[keyCode-0x7b]);
							return;
							break;
						
						case 0x08:
							/* デリートキー */
							DeleteSelectedIconPicture(theWindow);
							break;
					}
				}
				break;
			
			case kWindowTypeIconListWindow:
				/* アイコンリストウィンドウ */
				iWinRec=GetIconListRec(theWindow);
				if (IsIconSelected(iWinRec))
				{
					switch (theChar)
					{
						case kBackspaceCharCode:
							/* デリートキー */
							DoDelete(false);
							return;
							break;
						
						case kReturnCharCode:
						case kEnterCharCode:
							/* リターン or エンターキー */
							OpenSelectedIcon(theWindow);
							return;
							break;
					}
				}
				switch (theChar)
				{
						case kLeftArrowCharCode:
						case kRightArrowCharCode:
						case kUpArrowCharCode:
						case kDownArrowCharCode:
							/* カーソルキー */
							MoveSelectedIcon(theWindow,dx[keyCode-0x7b],dy[keyCode-0x7b]);
							return;
							break;
				}
				break;
			
			case kWindowTypePaintWindow:
				/* ペイントウィンドウ */
				eWinRec=GetPaintWinRec(theWindow);
				
				if (optDown && (keyCode==0x1f || theChar=='o'))	/* opt+o */
				{
					DispPaintMask(theWindow);
					return;
				}
				if (eWinRec->isDotMode)
				{
					if (HandleDotKey(theEvent->what,theChar,theWindow)) return;
				}
				if (eWinRec->isSelected) /* if anywhere is selected */
				{
					switch (theChar)
					{
						case kBackspaceCharCode: /* delete key */
							DoDelete(optDown);
							return;
							break;
						
						case kLeftArrowCharCode: /* cursor keys */
						case kRightArrowCharCode:
						case kUpArrowCharCode:
						case kDownArrowCharCode:
							/* まず、選択の印を消す */
							if (eWinRec->showSelection)
								DispSelection(theWindow);
							
							moveRatio=(optDown ? (shiftDown ? 4 : 3) : 0);
							/* 動かす。動かしたあとは再描画 */
							MoveSelection(theWindow,dx[keyCode-0x7b]<<moveRatio,dy[keyCode-0x7b]<<moveRatio,true);
							return;
							break;
						
						case kEnterCharCode: /* enter key */
							FixSelection(theWindow);
							return;
							break;
						
						case 't':
						case 'T':
							HandleEffectChoice(iTransparent);
							return;
							break;
						
						case 'o':
						case 'O':
							HandleEffectChoice(iOpaque);
							return;
							break;
						
						case 'b':
						case 'B':
							HandleEffectChoice(iBlend);
							return;
							break;
					}
				}
				else /* not selected */
				{
					switch (theChar)
					{
						case kLeftArrowCharCode: /* cursor keys */
						case kRightArrowCharCode:
						case kUpArrowCharCode:
						case kDownArrowCharCode:
							/* スクロール。動かしたあとは再描画 */
							DoScrollPaintWindowByKeys(theWindow,theChar,theEvent->modifiers);
							return;
							break;
					}
				}
				switch (theChar)
				{
					case kHomeCharCode:
					case kEndCharCode:
					case kPageUpCharCode:
					case kPageDownCharCode:
						DoScrollPaintWindowByKeys(theWindow,theChar,theEvent->modifiers);
						return;
						break;
				}
				switch (theChar)
				{
					case 'A':
					case 'a':
						HandleEffectChoice(iAntialias);
						return;
						break;
					
					case 'f':
					case 'F':
						HandleEffectChoice(iFill);
						return;
						break;
					
					case 'h':
					case 'H':
						HandleEffectChoice(iFlipHorizontal);
						return;
						break;
					
					case 'v':
					case 'V':
						HandleEffectChoice(iFlipVertical);
						return;
						break;
					
					case 'd':
					case 'D':
						HandlePenColorChoice(iPenDarken);
						return;
						break;
					
					case 'l':
					case 'L':
						HandlePenColorChoice(iPenLighten);
						return;
						break;
					
					case 's':
					case 'S':
						DoAutoSelect(theWindow);
						break;
					
					case 'z':
					case 'Z':
						DoUndo();
						return;
						break;
				}
				switch (theChar)
				{
					case '1':
						if (shiftDown)
							HandleColorModeChoice(iMonoD);
						else
							HandleColorModeChoice(iMonochrome);
						return;
						break;
					
					case '3':
						if (shiftDown)
							HandleColorModeChoice(iAppleIconColorD);
						else
							HandleColorModeChoice(iAppleIconColor);
						return;
						break;
					
					case '4':
						if (optDown)
							if (shiftDown)
								HandleColorModeChoice(i16GrayD);
							else
								HandleColorModeChoice(i16Gray);
						else
							if (shiftDown)
								HandleColorModeChoice(i16ColorD);
							else
								HandleColorModeChoice(i16Color);
						return;
						break;
					
					case '6':
						if (shiftDown)
							HandleColorModeChoice(i216ColorD);
						else
							HandleColorModeChoice(i216Color);
						return;
						break;
				}
				break;
		}
	}
	
	/* ツール選択など、ウィンドウに依存しないもの */
	switch(theChar)
	{
		/* ツール */
		case 'p':
		case 'P':
			if (gSelectedTool == kPencilTool)
				ToolSelect(kEraserTool);
			else
				ToolSelect(kPencilTool);
			return;
			break;
		
		case 'm':
		case 'M':
			ToolSelect(kMarqueeTool);
			return;
			break;
		
		case 'e':
		case 'E':
			if (gSelectedTool == kEraserTool)
				ToolSelect(kPencilTool);
			else
				ToolSelect(kEraserTool);
			return;
			break;
		
		case 'k':
		case 'K':
			ToolSelect(kBucketTool);
			return;
			break;
		
		case 'i':
		case 'I':
			ToolSelect(kSpoitTool);
			return;
			break;
		
		/* ペンの色関連 */
		case 0x1e: /* 上 */
			HandlePenColorChoice(iPenLighten);
			return;
			break;
		
		case 0x1f: /* 下 */
			HandlePenColorChoice(iPenDarken);
			return;
			break;
		
		case 0x09: /* tab key */
			if (optDown)
				HandlePenColorChoice(iPrevBlend);
			else
				HandlePenColorChoice(iPrevColor);
			return;
			break;
	}
}

/* 編集ウィンドウのデータから状態を取り出す */
void UpdateEditWinData(void)
{
	WindowPtr	theWindow=MyFrontNonFloatingWindow();
	PaintWinRec	*eWinRec;
	Rect	r;
	
	/* アクティブウィンドウなければプレビューを隠す */
	if (theWindow==nil)
	{
		HideReferencedWindow(gPreviewWindow);
		HideReferencedWindow(DotModePalette);
		
		if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
			UpdatePaletteCheck();
		return;
	}
	
	if (GetExtWindowKind(theWindow)!=kWindowTypePaintWindow)
	{
		SetPortWindowPort(gPreviewWindow);
		SizeWindow(gPreviewWindow,kIconPreviewWidth,kIconPreviewHeight,true);
		GetWindowPortBounds(gPreviewWindow,&r);
		ClipRect(&r);
		MyInvalWindowPortBounds(gPreviewWindow);
		ShowReferencedWindow(gPreviewWindow);
		SetPortWindowPort(theWindow);
		
		HideReferencedWindow(DotModePalette);
		
		if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
			UpdatePaletteCheck();
		return;
	}
	
	ShowReferencedWindow(gPreviewWindow);
	
	/* ルーラ */
	ResetRuler();
	
	eWinRec=GetPaintWinRec(theWindow);
	
	/* プレビューも更新 */
	SetPortWindowPort(gPreviewWindow);
	SizeWindow(gPreviewWindow,eWinRec->iconSize.right,eWinRec->iconSize.bottom,true);
	ClipRect(&eWinRec->iconSize);
	MyInvalWindowPortBounds(gPreviewWindow);
	SetPortWindowPort(theWindow);
	
	/* 描画点モードならパレットを表示 */
	if (eWinRec->isDotMode)
		ShowReferencedWindow(DotModePalette);
	else
		HideReferencedWindow(DotModePalette);
	
	/* 選択領域があるなら選択ツールに変える */
	if (!EmptyRgn(eWinRec->eSelectedRgn) && !gToolPrefs.selectionMasking)
		ToolSelect(kMarqueeTool);
	
	if (gPaletteCheck == kPaletteCheckUsed)
		UpdatePaletteCheck();
	else
	{
		/* アイコンを開いたものであれば、必要に応じてカラーパレットの印を変更 */
		if (eWinRec->iconType.fileType == 'icns')
		{
			short	newPaletteCheck=gPaletteCheck;
			
			if (eWinRec->iconKind == kL8Data || eWinRec->iconKind == kS8Data)
				newPaletteCheck=kPaletteCheckAIC;
			else if (eWinRec->iconKind == kL4Data || eWinRec->iconKind == kS4Data)
				newPaletteCheck=kPaletteCheck16;
			
			if (gPaletteCheck != newPaletteCheck)
			{
				HandlePaletteChoice(newPaletteCheck+1);
			}
		}
	}
}

/* タイトルバーのポップアップ */
void TitlePopup(WindowPtr theWindow,Point popPos)
{
	FSSpec		spec,tempSpec;
	MenuHandle	popMenu;
	OSErr		err=noErr;
	short		item=1;
	long		selItem;
	Boolean		savedFlag;
	short		windowKind;
	
	if ((windowKind=GetExtWindowKind(theWindow))==kWindowTypePaintWindow)
	{
		PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
		
		if (eWinRec->iconType.fileType == 'icns') return;
		
		spec=eWinRec->saveFileSpec;
		savedFlag=eWinRec->iconHasSaved;
	}
	else if (windowKind==kWindowTypeIconListWindow)
	{
		IconListWinRec	*iWinRec=GetIconListRec(theWindow);
		
		spec=iWinRec->iconFileSpec;
		savedFlag=iWinRec->wasSaved;
	}
	else
		return;
	
	tempSpec=spec;
	
	popMenu=GetMenu(mTitlePopup);
	
	if (savedFlag)
	{
		while (spec.parID!=fsRtParID && err==noErr)
		{
			AppendMenu(popMenu,"\p ");
			SetMenuItemText(popMenu,item++,spec.name);
			err=FSMakeFSSpec(spec.vRefNum,spec.parID,0,&spec);
		}
		if (err==noErr)
		{
			AppendMenu(popMenu,"\p ");
			SetMenuItemText(popMenu,item++,spec.name);
		}
	}
	else
		AppendMenu(popMenu,"\pNot Saved Yet");
	
	InsertMenu(popMenu,-1);
	
	selItem=PopUpMenuSelect(popMenu,popPos.v,popPos.h,1);
	
	item=LoWord(selItem);
	
	if (item>1)
	{
		spec=tempSpec;
		while (item-->2)
		{
			err=FSMakeFSSpec(spec.vRefNum,spec.parID,0,&spec);
		}
		
		FSpAERevealFile(&spec);
	}			
	
	DeleteMenu(mTitlePopup);
	DisposeMenu(popMenu);
}

/* 指定ファイルをFinder上で表示 */
void FSpAERevealFile(FSSpec *spec)
{
	AliasHandle	fileAlias,parentAlias;
	FSSpec		parentSpec;
	
	ProcessSerialNumber	psn;
	Boolean		found;
	OSErr		err;
	
	AEDesc		target={typeNull,NULL};
	AppleEvent	aeEvent={typeNull,NULL};
	AEDesc		parentDesc={typeNull,NULL};
	AEDescList	fileList={typeNull,NULL};
	
	/* Finderを探す */
	found=FindProcessFromCreatorAndType(kFinderCreator,kFinderType,&psn);
	if (!found) return;
	
	/* Finderをターゲットに指定 */
	err=AECreateDesc(typeProcessSerialNumber,&psn,sizeof(ProcessSerialNumber),&target);
	
	/* 送付するAppleイベントを作成 */
	err=AECreateAppleEvent(kAEFinderEvents,kAERevealSelection,&target,kAutoGenerateReturnID,
							kAnyTransactionID,&aeEvent);
	
	/* Finder上で表示するファイル（フォルダ）のリストを作成 */
	err=AECreateList(nil,0,false,&fileList);
	
	/* リストにファイルを追加 */
	err=NewAlias(nil,spec,&fileAlias);
	if (err==noErr)
	{
		HLock((Handle)fileAlias);
#if __AL_USE_OPAQUE_RECORD__
		Size aliasSize = GetAliasSize( fileAlias );
#else
		unsigned short aliasSize = (*fileAlias)->aliasSize;
#endif
		err=AEPutPtr(&fileList,1,typeAlias,(Ptr)*fileAlias,aliasSize);
		HUnlock((Handle)fileAlias);
		DisposeHandle((Handle)fileAlias);
	}
	
	/* リストをAppleイベントのkeySelectionパラメータに設定する */
	err=AEPutParamDesc(&aeEvent,keySelection,&fileList);
	
	/* ファイルの親フォルダのデスクリプタを作成 */
	err=FSMakeFSSpec(spec->vRefNum,spec->parID,0,&parentSpec);
	err=NewAlias(nil,&parentSpec,&parentAlias);
	if (err==noErr)
	{
		HLock((Handle)parentAlias);
#if __AL_USE_OPAQUE_RECORD__
		Size aliasSize = GetAliasSize( parentAlias );
#else
		unsigned short aliasSize = (*parentAlias)->aliasSize;
#endif
		err=AECreateDesc(typeAlias,(Ptr)*parentAlias,aliasSize,&parentDesc);
		HUnlock((Handle)parentAlias);
		DisposeHandle((Handle)parentAlias);
	}
	
	/* 親フォルダのデスクリプタをkeyDirectObjectパラメータに設定する */
	err=AEPutParamDesc(&aeEvent,keyDirectObject,&parentDesc);
	
	/* Appleイベントを送付する */
	err=AESend(&aeEvent,nil,kAENoReply+kAECanSwitchLayer+kAEAlwaysInteract,
				kAENormalPriority,kNoTimeOut,nil,nil);
	
	/* 確保したデスクリプタを破棄する */
	err=AEDisposeDesc(&target);
	err=AEDisposeDesc(&aeEvent);
	err=AEDisposeDesc(&fileList);
	err=AEDisposeDesc(&parentDesc);
	
	err=SetFrontProcess(&psn);
}

/* ファイルを得る */
OSErr   GetFile(FSSpec *theSpec,long index,Boolean *type)
{
	CInfoPBRec	pb;
	DirInfo		*fiPtr = &pb.dirInfo;
	OSErr		err;
	
	fiPtr->ioCompletion	= nil;
	fiPtr->ioVRefNum	= theSpec->vRefNum;
	fiPtr->ioNamePtr	= theSpec->name;
	fiPtr->ioDrDirID	= theSpec->parID;
	fiPtr->ioFDirIndex	= (short)index;
	
	err = PBGetCatInfoSync(&pb);
	
	if(err == noErr) {
		FSMakeFSSpec( pb.hFileInfo.ioVRefNum,pb.hFileInfo.ioFlParID,pb.hFileInfo.ioNamePtr, &(*theSpec) );
        if( pb.hFileInfo.ioFlAttrib & 0x10 ){	/* フォルダだったら */
        	*type=true;
        }else{									/* ファイルだったら */
        	*type=false;
        }
	}
	
	return err;
}

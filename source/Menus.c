/* ------------------------------------------------------------ */
/*  Menus.c                                                     */
/*     routines for menus                                       */
/*                                                              */
/*                 1997.1.28 - 2002.7.4  naoki iimura        	*/
/* ------------------------------------------------------------ */

/* includes */
#ifdef __APPLE_CC__
#include	<Carbon/Carbon.h>
#else
#include	<QDOffscreen.h>
#include	<Icons.h>
#include	<Sound.h>
#include	<ToolUtils.h>
#include	<Devices.h>
#include	<LowMem.h>
#include	<Resources.h>
#include	<TextUtils.h>
#include	<NumberFormatting.h>
#include	<Scrap.h>
#endif

#include	"WindowExtensions.h"

#include	"Globals.h"
#include	"IconParty.h"
#include	"MenuRoutines.h"
#include	"FileRoutines.h"
#include	"UsefulRoutines.h"
#include	"NavigationServicesSupport.h"
#include	"IconRoutines.h"
#include	"IconListWindow.h"
#include	"IconFamilyWindow.h"
#include	"IPIconSupport.h"
#include	"WindowRoutines.h"
#include	"PreCarbonSupport.h"
#include	"Preferences.h"
#include	"EditRoutines.h"
#include	"ToolRoutines.h"
#include	"PaintRoutines.h"
#include	"EffectRoutines.h"


/* prototypes */
static void	HandlePenSizeChoice(short item);
static void	HandleEraserSizeChoice(short item);
static void	HandleWindowChoice(short item);
static void	HandleMagChoice(short item);
static void	HandleSelectionChoice(short item);

static void	UpdateWindowMenu(void);

static void	MySetDialogItemText(DialogPtr theDialog,short item,Str255 string,short fontSize,short just);
#if !TARGET_API_MAC_CARBON
static void	MyNumToTimeString(unsigned long time,Str255 str);
static pascal Boolean	AboutFilter(DialogPtr dp,EventRecord *theEvent,short *item);
#endif

/* ブレンドパレット関係。本来ならここに置くべきではないが */
static void	SaveBlendPalette(FSSpec *theFile);

/* -> ICSupport.c */
extern Boolean	IsICInstalled(void);
extern OSErr	ICLaunchURLString(Str255 urlStr);

/* -> UpdateCheck.c */
extern OSStatus IPUpdateCheck(void);


#if TARGET_API_MAC_CARBON
	extern pascal void PenSizeMDEFProc(short message,MenuHandle theMenu,Rect *menuRect,Point hitPt,short *whichItem);
#endif

/* globals */
static short	eWinNum=0,iWinNum=0,fWinNum=0;

#if TARGET_API_MAC_CARBON
static short	helpMenuID,updateMenuIndex;
static short	prefMenuID,prefMenuIndex;
#endif

extern WindowPtr	ColorPalette1,ColorPalette2;


#define	MENUERR_RESID	4007
#define	MENUERR1	1
#define	MENUERR2	2


/* メニューバーの初期化 */
void MenuBarInit(void)
{
	Handle		menuBar;
	MenuHandle	menu;
	
	menuBar=GetNewMBar(128);
	SetMenuBar(menuBar);
	
	#if !TARGET_API_MAC_CARBON
	/* アップルメニュー */
	menu=GetMenuHandle(mApple);
	AppendResMenu(menu,'DRVR');
	#endif
	
	#if TARGET_API_MAC_CARBON
	/* ファイルメニューの終了をメンテナンス */
	if (isOSX)
	{
		MenuItemIndex i;
		MenuHandle outMenu;
		
		menu=GetMenuHandle(mFile);
		DeleteMenuItem(menu,iQuit);
		DeleteMenuItem(menu,iQuit-1);
		
		EnableMenuCommand(NULL,'pref');
		
		GetIndMenuItemWithCommandID(NULL,kHICommandPreferences,1,&outMenu,&i);
		SetItemCmd(outMenu,i,';');
		
		prefMenuID = GetMenuID(outMenu);
		prefMenuIndex = i;
	}
	#endif
	
	/* 階層メニュー */
	menu=GetMenu(mTransp);
	InsertMenu(menu,-1);
	
	menu=GetMenu(mGrid);
	InsertMenu(menu,-1);
	
	menu=GetMenu(mPenColor);
	InsertMenu(menu,-1);
	#if 0
	DisableMenuItem(menu,iFromDesktop);
	#endif
	
	menu=GetMenu(mColorMode);
	InsertMenu(menu,-1);
	
	/* ペンサイズメニュー */
	#if TARGET_API_MAC_CARBON
	{
		OSErr	err;
		MenuDefSpec	defSpec;
		short	item;
		
		defSpec.defType = kMenuDefProcPtr;
		defSpec.u.defProc = NewMenuDefUPP(PenSizeMDEFProc);
		err=CreateCustomMenu(&defSpec,mPenSize,0,&menu);
		for (item=1; item<=16; item++)
			AppendMenuItemText(menu,"\p ");
		InsertMenu(menu,-1);
		CheckMenuItem(menu,(gPenHeight-1)*4+gPenWidth,true);
		
		err=CreateCustomMenu(&defSpec,mEraserSize,0,&menu);
		for (item=1; item<=16; item++)
			AppendMenuItemText(menu,"\p ");
		InsertMenu(menu,-1);
		CheckMenuItem(menu,(gEraserHeight-1)*4+gEraserWidth,true);
	}
	#else
	menu=GetMenu(mPenSize);
	InsertMenu(menu,-1);
	
	menu=GetMenu(mEraserSize);
	InsertMenu(menu,-1);
	#endif
	
	menu=GetMenu(mDisplay);
	InsertMenu(menu,-1);
	
	menu=GetMenu(mMag2);
	InsertMenu(menu,-1);
	
	menu=GetMenu(mColorPalette);
	InsertMenu(menu,-1);
	
	menu=GetMenu(mSelection);
	InsertMenu(menu,-1);
	
	menu=GetMenu(mListBackground);
	InsertMenu(menu,-1);
	CheckMenuItem(menu,gListBackground,true);
	#if TARGET_API_MAC_CARBON
	if (isOSX) MyDisableMenuItem(menu,iBackDesktop);
	#endif
	
	menu=GetMenu(mForeTransparency);
	InsertMenu(menu,-1);
	
	menu=GetMenu(mBlendPaletteSub);
	InsertMenu(menu,-1);
	
	UpdateToolMenu();
	
	/* 新規メニューをいじる */
	UpdateNewMenu();
	
	UpdatePasteMenu();
	
	/* ツールの選択範囲 */
	menu=GetMenuHandle(mTool);
	MyDisableMenuItem(menu,iSelection);
	
	/* ウィンドウの倍率 */
	menu=GetMenuHandle(mWindow);
	MyDisableMenuItem(menu,iMag);
	
	if (gSystemVersion >= 0x0800)
	{
		/* ファイルメニュー */
		menu=GetMenuHandle(mFile);
		SetItemCmd(menu,iSaveAs,'S');
		SetMenuItemModifiers(menu,iSaveAs,kMenuShiftModifier);
		SetItemCmd(menu,iOpenClipboard,'O');
		SetMenuItemModifiers(menu,iOpenClipboard,kMenuShiftModifier);
		
		/* 編集メニュー */
		menu=GetMenuHandle(mEdit);
		SetItemCmd(menu,iBackground,'B');
		SetMenuItemModifiers(menu,iBackground,kMenuShiftModifier);
		SetItemCmd(menu,iHideBackground,'H');
		SetMenuItemModifiers(menu,iHideBackground,kMenuShiftModifier);
		
		/* アイコンメニュー */
		menu=GetMenuHandle(mIcon);
		SetItemCmd(menu,iNewIconFile,'N');
		SetMenuItemModifiers(menu,iNewIconFile,kMenuShiftModifier);
		SetItemCmd(menu,iNewIcon,'N');
		SetMenuItemModifiers(menu,iNewIcon,kMenuOptionModifier);
		SetItemCmd(menu,iImportIcon,'I');
		SetMenuItemModifiers(menu,iImportIcon,kMenuShiftModifier);
		SetItemCmd(menu,iExportIcon,'E');
		SetMenuItemModifiers(menu,iExportIcon,kMenuShiftModifier);
		
		/* 色モードメニュー（サブメニュー） */
/*		menu=GetMenuHandle(mColorMode);
		SetItemCmd(menu,i216Color,'6');
		SetMenuItemModifiers(menu,i216Color,kMenuNoCommandModifier);
		SetItemCmd(menu,iAppleIconColor,'3');
		SetMenuItemModifiers(menu,iAppleIconColor,kMenuNoCommandModifier);
		SetItemCmd(menu,i16Color,'4');
		SetMenuItemModifiers(menu,i16Color,kMenuNoCommandModifier);
		SetItemCmd(menu,i16Gray,'4');
		SetMenuItemModifiers(menu,i16Gray,kMenuNoCommandModifier+kMenuOptionModifier);
		SetItemCmd(menu,iMonochrome,'1');
		SetMenuItemModifiers(menu,iMonochrome,kMenuNoCommandModifier);
		SetItemCmd(menu,i216ColorD,'6');
		SetMenuItemModifiers(menu,i216ColorD,kMenuNoCommandModifier+kMenuShiftModifier);
		SetItemCmd(menu,iAppleIconColorD,'3');
		SetMenuItemModifiers(menu,iAppleIconColorD,kMenuNoCommandModifier+kMenuShiftModifier);
		SetItemCmd(menu,i16ColorD,'4');
		SetMenuItemModifiers(menu,i16ColorD,kMenuNoCommandModifier+kMenuShiftModifier);
		SetItemCmd(menu,i16GrayD,'4');
		SetMenuItemModifiers(menu,i16GrayD,kMenuNoCommandModifier+kMenuOptionModifier+kMenuShiftModifier);
		SetItemCmd(menu,iMonoD,'1');
		SetMenuItemModifiers(menu,iMonoD,kMenuNoCommandModifier+kMenuShiftModifier); */
		
		/* 効果メニュー */
/*		menu=GetMenuHandle(mEffect);
		SetItemCmd(menu,iOpaque,'O');
		SetMenuItemModifiers(menu,iOpaque,kMenuNoCommandModifier);
		SetItemCmd(menu,iTransparent,'T');
		SetMenuItemModifiers(menu,iTransparent,kMenuNoCommandModifier);
		SetItemCmd(menu,iBlend,'B');
		SetMenuItemModifiers(menu,iBlend,kMenuNoCommandModifier);
		SetItemCmd(menu,iFlipHorizontal,'H');
		SetMenuItemModifiers(menu,iFlipHorizontal,kMenuNoCommandModifier);
		SetItemCmd(menu,iFlipVertical,'V');
		SetMenuItemModifiers(menu,iFlipVertical,kMenuNoCommandModifier); */
		
		#if TARGET_API_MAC_CARBON
		/* 選択範囲メニュー */
		menu=GetMenuHandle(mSelection);
		if (isOSX)
		{
			SetItemCmd(menu,iHideSelection,'H');
			SetMenuItemModifiers(menu,iHideSelection,kMenuOptionModifier);
		}
		
		/* help menu */
		if (gCarbonLibVersion >= 0x0110 && IsICInstalled())
		{
			OSStatus		err;
			MenuItemIndex	firstCustomIndex;
			Str255			string;
			
			err=HMGetHelpMenu(&menu,&firstCustomIndex);
			
			helpMenuID = GetMenuID(menu);
			updateMenuIndex = firstCustomIndex;
			
			GetIndString(string,5001,1);
			AppendMenu(menu,string);
		}
		#endif
/*		SetItemCmd(menu,iAutoSelect,'S');
		SetMenuItemModifiers(menu,iAutoSelect,kMenuNoCommandModifier); */
		
	}
	
	DrawMenuBar();
}

/* メニューの選択 */
void HandleMenuChoice(long menuChoice)
{
	short menu,item;
	
	if (menuChoice !=0)
	{
		menu=HiWord(menuChoice);
		item=LoWord(menuChoice);
		
		switch(menu)
		{
			case mApple:
				HandleAppleChoice(item);
				break;
			
			case mFile:
				HandleFileChoice(item);
				break;
			
			case mEdit:
				HandleEditChoice(item);
				break;
			
			case mEffect:
				HandleEffectChoice(item);
				break;
			
			case mTool:
				HandleToolChoice(item);
				break;
			
			case mTransp:
				HandleBlendChoice(item);
				break;
			
			case mGrid:
				HandleGridChoice(item);
				break;
			
			case mPenColor:
				HandlePenColorChoice(item);
				break;
			
			case mPenSize:
				HandlePenSizeChoice(item);
				break;
			
			case mEraserSize:
				HandleEraserSizeChoice(item);
				break;
			
			case mColorMode:
				HandleColorModeChoice(item);
				break;
			
			case mIcon:
				HandleIconChoice(item);
				break;
			
			case mWindow:
				HandleWindowChoice(item);
				break;
			
			case mMag2:
				HandleMagChoice(item);
				break;
			
			case mDisplay:
				HandleDispChoice(item);
				break;
			
			case mColorPalette:
				HandlePaletteChoice(item);
				break;
			
			case mSelection:
				HandleSelectionChoice(item);
				break;
			
			case mListBackground:
				HandleListBackgroundChoice(item);
				break;
			
			case mForeTransparency:
				HandleForeTransparencyChoice(item);
				break;
			
			case mBlendPaletteSub:
				HandleBlendPaletteChoice(item);
				break;
			
			default:
				#if TARGET_API_MAC_CARBON
				if (menu == helpMenuID && item == updateMenuIndex)
				{
					IPUpdateCheck();
				}
				else if (menu == prefMenuID && item == prefMenuIndex)
				{
					OpenPrefsDialog();
				}
				#endif
				break;
		}
		HiliteMenu(0);
	}
}

/* アップルメニュー */
void HandleAppleChoice(short item)
{
	#if !TARGET_API_MAC_CARBON
	MenuHandle	appleMenu;
	Str255		accName;
	short		accNumber;
	#endif
	
	switch (item)
	{
		case iAbout:
			About();
			break;
		
		default:
			/* デスクアクセサリ（アップルメニュー項目）の呼び出し */
			#if !TARGET_API_MAC_CARBON
			appleMenu=GetMenuHandle(mApple);
			GetMenuItemText(appleMenu,item,accName);
			accNumber=OpenDeskAcc(accName);
			#endif
			break;
	}
}

enum {
	kAboutIconIndex	= 2,
	kAboutNumTextIndex,
	#if !TARGET_API_MAC_CARBON
	kAboutTimeTextIndex,
	kAboutMemoryTextIndex,
	#endif
	kAboutNumValueIndex,
	#if !TARGET_API_MAC_CARBON
	kAboutTimeValueIndex,
	kAboutMemoryValueIndex,
	#endif
	kAboutApplicationNameIndex,
	kAboutApplicationVersionIndex,
	kAboutSupportWebIndex,
};

/* アバウトダイアログの表示 */
void About(void)
{
	DialogPtr	dp;
	short		item=0;
	GrafPtr		port;
	Rect		box;
	WindowPtr	theWindow;
	VersRecHndl	vers;
	Str255		numStr,str2;
	#if !TARGET_API_MAC_CARBON
	ProcessInfoRec	processInfo;
	ProcessSerialNumber	psn;
	unsigned long	oldTime;
	#endif
	
	GetPort(&port);
	
	SuspendFloatingWindows();
	
	/* バージョン */
	UseResFile(gApplRefNum);
	vers=(VersRecHndl)GetResource('vers',1);
	
	ParamText(LMGetCurApName(),(*vers)->shortVersion,"\p","\p");
	
	dp=GetNewDialog(128,nil,kFirstWindowOfClass);
	SetDialogDefaultItem(dp,ok);
	
	/* ICが使用可能かどうか */
	if (!IsICInstalled())
		HideDialogItem(dp,kAboutSupportWebIndex);
	
	/* アイテムの位置を調整 */
	/* 使用回数 */
	NumToString(1+(long)gUsedCount.usedCount,numStr); /* 1回分補正（1.0b11） */
	GetIndString(str2,136,4);
	PStrCat(str2,numStr);
	MySetDialogItemText(dp,kAboutNumValueIndex,numStr,9,teJustRight);
	
	#if !TARGET_API_MAC_CARBON
	/* プロセスの情報 */
	GetCurrentProcess(&psn);
	processInfo.processInfoLength=sizeof(ProcessInfoRec);
	processInfo.processName=nil;
	processInfo.processLocation=nil;
	processInfo.processAppSpec=nil;
	GetProcessInformation(&psn,&processInfo);
	
	/* 使用時間 */
	oldTime=gUsedCount.usedTime+processInfo.processActiveTime/60;
	MyNumToTimeString(oldTime,numStr);
	MySetDialogItemText(dp,kAboutTimeValueIndex,numStr,9,teJustRight);
	
	/* 残りメモリ */
	NumToString((long)processInfo.processFreeMem/1024,numStr);
	CatChar(' ',numStr);
	CatChar('/',numStr);
	CatChar(' ',numStr);
	NumToString((long)processInfo.processSize/1024,str2);
	PStrCat(str2,numStr);
	GetIndString(str2,136,5);
	PStrCat(str2,numStr);
	MySetDialogItemText(dp,kAboutMemoryValueIndex,numStr,9,teJustRight);
	#endif
	
	ShowWindow(GetDialogWindow(dp));
	SetPortDialogPort(dp);
//	TextSize(9);
	
	while (item != ok)
	{
		EventRecord	theEvent;
		
		if (WaitNextEvent(everyEvent,&theEvent,0,nil))
			if (IsDialogEvent(&theEvent))
			{
				char		theChar;
				Handle		theIconHnd;
				GrafPtr		port;
				Boolean		needDev=true;
				
				switch (theEvent.what)
				{
					case keyDown:
						theChar=(char)theEvent.message & charCodeMask;
						
						switch (theChar)
						{
							case 0x03:
							case 0x0d:
								HiliteButton(dp,ok);
								item=ok;
								break;
						}
						needDev=false;
						break;
					
					case updateEvt:
						GetPort(&port);
						theWindow=(WindowPtr)theEvent.message;
						SetPortWindowPort(theWindow);
						BeginUpdate(theWindow);
						
						DrawDialog(dp);
						
						UseResFile(gApplRefNum);
						
						/* アイコン */
						GetDialogItemRect(dp,kAboutIconIndex,&box);
						GetIconSuite(&theIconHnd, 128, svAllLargeData);
						SetPortDialogPort(dp);
						PlotIconSuite(&box,atNone,ttNone,theIconHnd);
						
						DisposeIconSuite(theIconHnd,true);
						
						EndUpdate(theWindow);
						SetPort(port);
						needDev=false;
						break;
					
					case osEvt:
						switch((unsigned long)(theEvent.message &osEvtMessageMask) >> 24)
						{
							case suspendResumeMessage:
								if (theEvent.message & resumeFlag)
									ActivateDialogControl(dp);
								else
									DeactivateDialogControl(dp);
								MyInvalWindowPortBounds(GetDialogWindow(dp));
								break;
						}
						break;
				}
				
				if (needDev)
				{
					if (DialogSelect(&theEvent,&dp,&item))
					{
						if (item == kAboutSupportWebIndex) /* サポートwebボタン */
						{
							Str255	urlStr;
							
							GetIndString(urlStr,152,1);
							ICLaunchURLString(urlStr);
						}
					}
				}
			}
			else
			{
				short	part;
				
				switch (theEvent.what)
				{
					case mouseDown:
						part=FindWindow(theEvent.where,&theWindow);
						if (theWindow != GetDialogWindow(dp))
						{
							SysBeep(0);
							break;
						}
						switch (part)
						{
							case inDrag:
								{
									Rect	screenRect;
									
									GetRegionBounds(GetGrayRgn(),&screenRect);
									DragWindow(theWindow,theEvent.where,&screenRect);
								}
								break;
						}
						break;
						
					case updateEvt:
						DoEvent(&theEvent);
						break;
				}
			}
		#if !TARGET_API_MAC_CARBON
		else
		{
			unsigned long	newTime;
			
			GetProcessInformation(&psn,&processInfo);
			newTime=gUsedCount.usedTime+processInfo.processActiveTime/60;
			
			if (newTime > oldTime)
			{
				MyNumToTimeString(gUsedCount.usedTime+processInfo.processActiveTime/60,numStr);
				SetDialogItemText2(dp,kAboutTimeValueIndex,numStr);
				oldTime=newTime;
			}
		}
		#endif
	}
	DisposeDialog(dp);
	ReleaseResource((Handle)vers);
	
	ResumeFloatingWindows();
	
	SetPort(port);
}

/* ダイアログのアイテムにテキストを設定 */
void MySetDialogItemText(DialogPtr theDialog,short item,Str255 string,short fontSize,short just)
{
	#if TARGET_API_MAC_CARBON
	#pragma unused(fontSize,just)
	#endif
	short	itemType;
	Handle	h;
	Rect	box;
	#if !TARGET_API_MAC_CARBON
	short	prevSize;
	short	prevFont;
	GrafPtr	port;
	#endif
	
	/* まずテキストを設定 */
	GetDialogItem(theDialog,item,&itemType,&h,&box);
	SetDialogItemText(h,string);
	
	#if !TARGET_API_MAC_CARBON
	switch (just)
	{
		case teJustLeft:
			break;
		
		case teJustCenter:
		case teJustRight:
			GetPort(&port);
			prevSize=GetPortTextSize(port);
			prevFont=GetPortTextFont(port);
			TextSize(fontSize);
			TextFont(systemFont);
			
			if (just == teJustRight)
			{
				box.left=box.right-StringWidth(string)-2;
				box.right+=5;
			}
			else
			{
				short	center=(box.right-box.left-StringWidth(string))>>1;
				
				box.left=box.left+center;
				box.right=box.right-center;
			}
			
			SetDialogItem(theDialog,item,itemType,h,&box);
			
			TextSize(prevSize);
			TextFont(prevFont);
			break;
	}
	#endif
}

#if !TARGET_API_MAC_CARBON
/* 数値を時間文字列に変換 */
void MyNumToTimeString(unsigned long time,Str255 str)
{
	unsigned long	hour;
	short	minute,second;
	
	hour=time/60/60;
	time %= 60*60;
	minute=(short)(time/60);
	second=(short)(time%60);
	
	NumToString((long)hour,str);
	CatChar(':',str);
	CatChar('0'+(UInt8)(minute/10),str);
	CatChar('0'+(UInt8)(minute%10),str);
	CatChar(':',str);
	CatChar('0'+(UInt8)(second/10),str);
	CatChar('0'+(UInt8)(second%10),str);
}

/* アバウトダイアログのモーダルフィルタ */
pascal Boolean AboutFilter(DialogPtr dp,EventRecord *theEvent,short *item)
{
	#pragma unused(dp,item)
	
	switch (theEvent->what)
	{}
	return false;
}
#endif

/* ファイルメニュー */
void HandleFileChoice(short item)
{
	WindowPtr	theWindow;
	short		windowKind = -1;
	
	theWindow = MyFrontNonFloatingWindow();
	if (theWindow != nil) windowKind = GetExtWindowKind(theWindow);
	
	switch (item)
	{
		case iNew:
			NewPaintWindow();
			break;
		
		case iOpen:
			OpenIcon();
			break;
		
		case iClose:
			if (theWindow != nil) ClosePaintWindow(theWindow,false);
			break;
		
		case iSave:
			if (theWindow == nil) break;
			switch (windowKind)
			{
				case kWindowTypePaintWindow:
					SavePaintWindow(false,false);
					break;
				
				case kWindowTypeIconListWindow:
					SaveIconFile(theWindow);
					break;
				
				case kWindowTypeIconFamilyWindow:
					SaveFamilyIcon(theWindow,false,false);
					break;
			}
			break;
		
		case iSaveAs:
			if (theWindow == nil) break;
			switch (windowKind)
			{
				case kWindowTypePaintWindow:
					SavePaintWindow(false,true);
					break;
				
				case kWindowTypeIconListWindow:
					break;
				
				case kWindowTypeIconFamilyWindow:
					SaveFamilyIcon(theWindow,false,true);
					break;
			}
			break;
		
		case iSaveSelection:
			if (theWindow == nil) break;
			SavePaintWindow(true,true);
			break;
		
		case iRevert:
			if (theWindow==nil) break;
			switch (windowKind)
			{
				case kWindowTypePaintWindow:
					DoRevertPaintWindow(theWindow);
					break;
				
				case kWindowTypeIconListWindow:
					DoRevertIconFile(theWindow);
					break;
				
				case kWindowTypeIconFamilyWindow:
					DoRevertIconFamilyWindow(theWindow);
					break;
			}
			break;
		
		case iOpenClipboard:
			OpenClipboard();
			break;
		
		case iPreferences:
			OpenPrefsDialog();
			break;
		
		case iQuit:
			Quit();
			break;
		
		default:
			break;
	}
}

/* 編集メニュー */
void HandleEditChoice(short item)
{
	WindowPtr	theWindow;
	
	switch (item)
	{
		case iUndo:
			DoUndo();
			break;
		
		case iCut:
			DoCut();
			break;
		
		case iCopy:
			DoCopy();
			break;
		
		case iPaste:
			DoPaste();
			break;
		
		case iClear:
			DoDelete(false);
			break;
		
		case iSelectAll:
			DoSelectAll();
			break;
		
		case iDuplicate:
			DoDuplicate();
			break;
		
		case iIconSize:
			theWindow=MyFrontNonFloatingWindow();
			if (theWindow!=nil && GetExtWindowKind(theWindow)==kWindowTypePaintWindow)
				ChangeIconSize(theWindow);
			break;
		
		case iTypeString:
			theWindow=MyFrontNonFloatingWindow();
			if (theWindow!=nil && GetExtWindowKind(theWindow)==kWindowTypePaintWindow)
				TypeString(theWindow);
			break;
		
		case iBackground:
			theWindow=MyFrontNonFloatingWindow();
			if (theWindow!=nil && GetExtWindowKind(theWindow)==kWindowTypePaintWindow)
				GoBackgroundMode(theWindow);
			break;
		
		case iUnionBackground:
			theWindow=MyFrontNonFloatingWindow();
			if (theWindow!=nil && GetExtWindowKind(theWindow)==kWindowTypePaintWindow)
				UnionBackground(theWindow);
			break;
		
		case iDeleteForeground:
			theWindow=MyFrontNonFloatingWindow();
			if (theWindow!=nil && GetExtWindowKind(theWindow)==kWindowTypePaintWindow)
				DeleteForeground(theWindow);
			break;
		
		case iDeleteBackground:
			theWindow=MyFrontNonFloatingWindow();
			if (theWindow!=nil && GetExtWindowKind(theWindow)==kWindowTypePaintWindow)
				DeleteBackground(theWindow);
			break;
		
		case iHideBackground:
			theWindow=MyFrontNonFloatingWindow();
			if (theWindow!=nil && GetExtWindowKind(theWindow)==kWindowTypePaintWindow)
				ShowHideBackground(theWindow);
			break;
	}
}

/* 効果メニュー */
void HandleEffectChoice(short item)
{
	WindowPtr	theWindow=MyFrontNonFloatingWindow();
	MenuHandle	menu;
	
	if (theWindow==nil) return;
	
	if (GetExtWindowKind(theWindow)==kWindowTypePaintWindow)
	{
		PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
		
		switch (item)
		{
			case iFill:
				DoFill(theWindow);
				break;
			
			case iDarken:
				DoDarken(theWindow);
				break;
			
			case iLighten:
				DoLighten(theWindow);
				break;
			
			case iAntialias:
				DoAntialias(theWindow);
				break;
			
			case iDotPicture:
				DoDotPicture(theWindow);
				break;
			
			case iWind:
				DoWind(theWindow);
				break;
			
			case iEdge:
				DoEdging(theWindow);
				break;
			
			case iExchange:
				ExchangeSelection(theWindow);
				break;
			
			case iRotate:
				DoRotate(theWindow);
				break;
			
			case iFlipVertical:
				FlipVertical(theWindow);
				break;
			
			case iFlipHorizontal:
				FlipHorizontal(theWindow);
				break;
			
			case iOpaque:
				ChangeMask(theWindow,srcCopy);
				break;
			
			case iTransparent:
				ChangeMask(theWindow,transparent);
				break;
			
			case iBlend:
				{
					menu=GetMenuHandle(mEffect);
					if (eWinRec->selectionBlend==srcCopy)
					{
						eWinRec->selectionBlend=blend;
						CheckMenuItem(menu,iBlend,true);
					}
					else
					{
						eWinRec->selectionBlend=srcCopy;
						CheckMenuItem(menu,iBlend,false);
					}
					CopyRgn(eWinRec->selectionPos,eWinRec->updateRgn);
					DispOffPort(theWindow);
					if (gPaletteCheck == kPaletteCheckUsed) /* パレットのチェックをアップデート */
						UpdatePaletteCheck();
				}
				break;
		}
	}
}

/* ツールメニュー */
void HandleToolChoice(short item)
{
	MenuHandle	menu;
	
	switch (item)
	{
		case iPenSize:
			break;
		
		case iEraserSize:
			break;
		
		case iGrid:
			ConfigureGrid();
			break;
		
		case iRuler:
			ResetRuler();
			gToolPrefs.showRuler=!gToolPrefs.showRuler;
			
			menu=GetMenuHandle(mTool);
			CheckMenuItem(menu,iRuler,gToolPrefs.showRuler);
			break;
		
		case iColorPalette:
			break;
		
		case iBlendPalette:
			break;
	}
}

/* 透明度メニュー（サブメニュー） */
void HandleBlendChoice(short item)
{
	MenuHandle	bMenu;
	
	if (item==gBSelectedItem) return;
	
	/* 選ばれているメニューにチェックマークをつける */
	bMenu=GetMenuHandle(mTransp);
	if (bMenu==nil)
	{
		SysBeep(0);
		return;
	}
	CheckMenuItem(bMenu,gBSelectedItem,false);
	CheckMenuItem(bMenu,item,true);
	gBSelectedItem=item;
	
	ChangeMode(gBSelectedItem);
	
	RedrawInfo();
}

/* ペンサイズメニュー（サブメニュー） */
void HandlePenSizeChoice(short item)
{
	MenuHandle	menu;
	
	if (item == (gPenHeight-1)*4+gPenWidth) return;
	
	menu=GetMenuHandle(mPenSize);
	
	CheckMenuItem(menu,(gPenHeight-1)*4+gPenWidth,false);
	CheckMenuItem(menu,item,true);
	
	gPenWidth=(item-1)%4+1;
	gPenHeight=(item-1)/4+1;
	
	RedrawInfo();
}

void HandleEraserSizeChoice(short item)
{
	MenuHandle	menu;
	
	if (item == (gEraserHeight-1)*4+gEraserWidth) return;
	
	menu=GetMenuHandle(mEraserSize);
	
	CheckMenuItem(menu,(gEraserHeight-1)*4+gEraserWidth,false);
	CheckMenuItem(menu,item,true);
	
	gEraserWidth=(item-1)%4+1;
	gEraserHeight=(item-1)/4+1;
	
	RedrawInfo();
}

/* グリッドメニュー（サブメニュー） */
void HandleGridChoice(short item)
{
	MenuHandle	gMenu;
	WindowPtr	theWindow;
	GrafPtr		port;
	short		iconGrid;
	short		normalGrid;
	
	gMenu=GetMenuHandle(mGrid);
	if (gMenu==0)
	{
		SysBeep(0);
		return;
	}
	
	iconGrid=(gToolPrefs.gridMode>>4)  &0x0f;
	normalGrid=gToolPrefs.gridMode & 0x0f;
	
	if (item==iGIcon32 || item==iGIcon16)
	{
		if (item-iGIcon32+1==iconGrid)
		{
			CheckMenuItem(gMenu,item,false);
			gToolPrefs.gridMode&=0x0f;
		}
		else
		{
			if (iconGrid >0)
			{
				CheckMenuItem(gMenu,iconGrid +iGIcon32-1,false);
			}
			CheckMenuItem(gMenu,item,true);
			gToolPrefs.gridMode=(gToolPrefs.gridMode & 0x0f)+((item-iGIcon32+1)<<4);
		}
	}
	else
	{
		if (item==normalGrid) return;
		
		CheckMenuItem(gMenu,normalGrid,false);
		CheckMenuItem(gMenu,item,true);
		gToolPrefs.gridMode=(gToolPrefs.gridMode & 0xf0)+item;
	}
	
	/* 表示されているすべてのエディットウィンドウにグリッドを表示する */
	GetPort(&port);
	
	theWindow=MyFrontNonFloatingWindow();
	
	while(theWindow!=nil && GetExtWindowKind(theWindow)==kWindowTypePaintWindow)
	{
		MyInvalWindowPortBounds(theWindow);
		theWindow=GetNextVisibleWindow(theWindow);
	}
	SetPort(port);
}

/* ペンの色メニュー（サブメニュー） */
void HandlePenColorChoice(short item)
{
	MyColorRec	newColor;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	newColor.isTransparent=false;
	
	GetGWorld(&cPort,&cDevice);
	SetGWorld(gBlendPalettePtr,0);
	LockPixels(GetGWorldPixMap(gBlendPalettePtr));
	
	switch (item)
	{
		/* すごくいいかげんな処理だな…。ま、動けばいいのだけど。 */
		case iPenLighten:
			GetCPixel(0x01,0x2b,&newColor.rgb);
			break;
		
		case iPenDarken:
			GetCPixel(0x01,0x4d,&newColor.rgb);
			break;
		
		case iPrevColor:
			newColor=gPrevColor;
			break;
		
		case iPrevBlend:
			GetCPixel(0x14,0x3c,&newColor.rgb);
			break;
		
		case iFromDesktop:
			UnlockPixels(GetGWorldPixMap(gBlendPalettePtr));
			SetGWorld(cPort,cDevice);
			GetDesktopColor();
			return;
			break;
	}
	
	UnlockPixels(GetGWorldPixMap(gBlendPalettePtr));
	SetGWorld(cPort,cDevice);
	ChangeColor(&newColor.rgb,newColor.isTransparent);
}

/* カラーモードメニュー（サブメニュー） */
void HandleColorModeChoice(short item)
{
	WindowPtr theWindow=MyFrontNonFloatingWindow();
	
	if (theWindow==nil) return;
	
	if (GetExtWindowKind(theWindow)==kWindowTypePaintWindow)
	{
		if (item>iMonochrome)
		{
			item-=i216ColorD-1;
			ChangeColorMode(theWindow,item,true); /* ディザあり */
		}
		else
			ChangeColorMode(theWindow,item,false); /* ディザなし */
		
		if (gOtherPrefs.checkWhenColorChanged)
		{
			gPrevPaletteCheck=gPaletteCheck;
			
			switch (item)
			{
				case iAppleIconColor:
					HandlePaletteChoice(kPaletteCheckAIC+1);
					break;
				
				case i16Color:
					HandlePaletteChoice(kPaletteCheck16+1);
					break;
			}
		}
	}
}

/* アイコンメニュー */
void HandleIconChoice(short item)
{
	WindowPtr	theWindow=MyFrontNonFloatingWindow();
	IconListWinRec	*iWinRec;
	short		winKind=0;
	
	if (theWindow != NULL)
		winKind = GetExtWindowKind(theWindow);
	
	switch (item)
	{
		case iNewIconFile:
			NewIconFile();
			break;
		
		case iNewIcon:
			NewIconFamilyWindow();
			break;
		
		case iAddIcon:
			if (theWindow == NULL) return;
			switch (winKind)
			{
				case kWindowTypeIconListWindow:
					AddNewIcon(theWindow);
					break;
				
				case kWindowTypeIconFamilyWindow:
					CreateEmptyIconData(theWindow);
					break;
			}
			break;
		
		case iIconInfo:
			if (theWindow == NULL) return;
			if (winKind==kWindowTypeIconListWindow)
			{
				iWinRec=GetIconListRec(theWindow);
				if (IsMultiIconSelected(iWinRec) == 1)
					SelectedIconInfo(iWinRec);
			}
			break;
		
		case iImportIcon:
			if (theWindow == NULL) return;
			if (winKind==kWindowTypeIconListWindow)
				ImportIconFromFile(theWindow);
			break;
		
		case iExportIcon:
			if (theWindow == NULL) return;
			if (winKind==kWindowTypeIconListWindow)
			{
				iWinRec=GetIconListRec(theWindow);
				if (IsMultiIconSelected(iWinRec) == 1)
					ExportIconToFolder(theWindow);
			}
			break;
		
		case iOpenWithResEdit:
			if (theWindow == NULL) return;
			if (winKind==kWindowTypeIconListWindow)
				OpenWithResEdit(theWindow);
			break;
	}
}

/* ウィンドウメニュー */
void HandleWindowChoice(short item)
{
	WindowPtr	theWindow=MyFrontNonFloatingWindow();
	
	item-=4;
	if (item<=iWinNum)
	{
		/* アイコンリスト */
		while (theWindow!=nil)
		{
			if (GetExtWindowKind(theWindow)==kWindowTypeIconListWindow)
			{
				item--;
				if (item==0) break;
			}
			theWindow=GetNextWindow(theWindow);
		}
	}
	else
	{
		item-=(iWinNum>0 ? iWinNum : -1)+1;
		
		if (item<=fWinNum)
		{
			/* アイコンファミリウィンドウ */
			while (theWindow!=nil)
			{
				if (GetExtWindowKind(theWindow)==kWindowTypeIconFamilyWindow)
				{
					item--;
					if (item==0) break;
				}
				theWindow=GetNextWindow(theWindow);
			}
		}
		else
		{
			item-=(fWinNum>0 ? fWinNum : -1)+1;
			
			/* ペイントウィンドウ */
			while (theWindow!=nil)
			{
				if (GetExtWindowKind(theWindow)==kWindowTypePaintWindow)
				{
					item--;
					if (item==0) break;
				}
				theWindow=GetNextWindow(theWindow);
			}
		}
	}
	if (theWindow!=nil && theWindow!=MyFrontNonFloatingWindow())
	{
		SelectReferencedWindow(theWindow);
		UpdateMenus();
	}
}

/* 倍率メニュー */
void HandleMagChoice(short item)
{
	WindowPtr	theWindow=MyFrontNonFloatingWindow();
	
	if (GetExtWindowKind(theWindow)==kWindowTypePaintWindow)
	{
		GrafPtr		port;
		
		GetPort(&port);
		SetPortWindowPort(theWindow);
		ChangeRatioMain(theWindow,item-1);
		SetPort(port);
	}
}

/* 表示メニュー */
void HandleDispChoice(short item)
{
	if (item == iShowAllPalettes) /* show all palettes */
		ShowAllPalettes();
	else
		ShowHidePalette2(item);
}

/* カラーパレットメニュー（サブメニュー） */
void HandlePaletteChoice(short item)
{
	MenuHandle	menu;
	GrafPtr		port;
	
	if (item == gPaletteCheck+1) return;
	
	menu=GetMenuHandle(mColorPalette);
	CheckMenuItem(menu,gPaletteCheck+1,false);
	CheckMenuItem(menu,item,true);
	
	gPaletteCheck=item-1;
	
	GetPort(&port);
	MyInvalWindowPortBounds(ColorPalette1);
	MyInvalWindowPortBounds(ColorPalette2);
	DrawBlend();
	MyInvalWindowPortBounds(gBlendPalette);
	SetPort(port);
}

/* ブレンドパレットメニュー（サブメニュー） */
void HandleBlendPaletteChoice(short item)
{
	MenuHandle	menu;
	GrafPtr		port;
	FSSpec		theFile;
	NavReplyRecord	theReply;
	Str255		prompt,filename;
	OSErr		err;
	OSType		typeList[]={kBlendPaletteFileType};
	
	switch (item)
	{
		case iBlendLoad:
			/* 読み込み */
			GetIndString(prompt,155,4);
			
			DeactivateFloatersAndFirstDocumentWindow();
			#if !TARGET_API_MAC_CARBON
			if (isNavServicesAvailable && useNavigationServices)
			#endif
			{
				err=ChooseFileWithNav(1,typeList,prompt,&theFile);
				if (err!=noErr)
				{
					ActivateFloatersAndFirstDocumentWindow();
					break;
				}
			}
			#if !TARGET_API_MAC_CARBON
			else
			{
				StandardFileReply	reply;
				Point	where={-1,-1};
				ActivateYDUPP	aUPP=NewActivateYDProc(MyActivate);
				
				CustomGetFile(nil,1,typeList,&reply,400,where,nil,nil,nil,aUPP,nil);
				DisposeRoutineDescriptor(aUPP);
				if (!reply.sfGood)
				{
					ActivateFloatersAndFirstDocumentWindow();
					break;
				}
				
				theFile=reply.sfFile;
			}
			#endif
			ActivateFloatersAndFirstDocumentWindow();
			
			LoadBlendPalette(&theFile);
			break;
		
		case iBlendSave:
			/* 保存 */
			GetIndString(filename,155,2);
			
			DeactivateFloatersAndFirstDocumentWindow();
			#if !TARGET_API_MAC_CARBON
			if (isNavServicesAvailable && useNavigationServices)
			#endif
			{
				GetIndString(prompt,155,3);
				err=SaveBlendPaletteWithNav(&theFile,&theReply,prompt,filename);
				if (err!=noErr)
				{
					ActivateFloatersAndFirstDocumentWindow();
					break;
				}
			}
			#if !TARGET_API_MAC_CARBON
			else
			{
				StandardFileReply	reply;
				Point	where={-1,-1};
				ActivateYDUPP	aUPP=NewActivateYDProc(MyActivate);
				
				GetIndString(prompt,155,1);
				CustomPutFile(prompt,filename,&reply,401,where,nil,nil,nil,aUPP,nil);
				
				DisposeRoutineDescriptor(aUPP);
				
				if (!reply.sfGood)
				{
					ActivateFloatersAndFirstDocumentWindow();
					break;
				}
				
				theFile=reply.sfFile;
				if (reply.sfReplacing)
					err=FSpDelete(&theFile);
				
			}
			#endif
			ActivateFloatersAndFirstDocumentWindow();
			
			FSpCreateResFile(&theFile,kIconPartyCreator,kBlendPaletteFileType,smSystemScript);
			if (ResError()!=noErr)
			{
				ErrorAlertFromResource(MENUERR_RESID,MENUERR1);
				return;
			}
			
			if (isNavServicesAvailable && useNavigationServices)
			{
				if (theReply.validRecord)
					err=NavCompleteSave(&theReply,kNavTranslateInPlace);
				err=NavDisposeReply(&theReply);
			}
			
			/* ファイルを開く */
			SaveBlendPalette(&theFile);
			break;
		
		case iBlendLock:
			/* ロック */
			GetPort(&port);
			gBlendLocked = !gBlendLocked;
			DrawBlend();
			MyInvalWindowPortBounds(gBlendPalette);
			
			menu=GetMenuHandle(mBlendPaletteSub);
			CheckMenuItem(menu,iBlendLock,gBlendLocked);
			break;
	}
}

/* ブレンドパレット読み込み */
void LoadBlendPalette(FSSpec *theFile)
{
	short		refNum;
	CTabHandle	ctab;
	ColorSpecPtr	cspec;
	
	/* ファイルを開く */
	refNum=FSpOpenResFile(theFile,fsRdPerm);
	if (refNum<=0)
	{
		ErrorAlertFromResource(MENUERR_RESID,MENUERR2);
		return;
	}
	
	UseResFile(refNum);
	
	/* カラーテーブルを読み込む */
	ctab=(CTabHandle)Get1Resource('clut',128);
	HLock((Handle)ctab);
	cspec=(**ctab).ctTable;
	
	{
		/* 現在の選択色を保存 */
		MyColorRec	tempColor1=gCurrentColor,tempColor2=gPrevColor;
		
		gBlendLocked=false;
		ChangeColor(&cspec[1].rgb,false);
		ChangeColor(&cspec[0].rgb,false);
		HandleBlendPaletteChoice(iBlendLock);
		ChangeColor(&tempColor2.rgb,tempColor2.isTransparent);
		ChangeColor(&tempColor1.rgb,tempColor1.isTransparent);
	}
	
	HUnlock((Handle)ctab);
	ReleaseResource((Handle)ctab);
	CloseResFile(refNum);
	UseResFile(gApplRefNum);
}

/* ブレンドパレット保存 */
void SaveBlendPalette(FSSpec *theFile)
{
	short		refNum;
	CTabHandle	ctab;
	ColorSpecPtr	cspec;
	
	refNum=FSpOpenResFile(theFile,fsWrPerm);
	if (refNum<=0)
	{
		ErrorAlertFromResource(MENUERR_RESID,MENUERR1);
		return;
	}
	
	UseResFile(refNum);
	
	/* 色を保存するためにカラーテーブルを作成 */
	ctab=(CTabHandle)NewHandle(sizeof(ColorTable)+sizeof(ColorSpec));
	(**ctab).ctSeed=UniqueID('clut');
	(**ctab).ctFlags=0;
	(**ctab).ctSize=1;
	
	cspec=(**ctab).ctTable;
	cspec[0].value=0;
	cspec[0].rgb=gCurrentColor.rgb;
	cspec[1].value=1;
	cspec[1].rgb=gPrevColor.rgb;
	
	AddResource((Handle)ctab,'clut',128,"\pblend colors");
	WriteResource((Handle)ctab);
	ReleaseResource((Handle)ctab);
	
	CloseResFile(refNum);
	UseResFile(gApplRefNum);
}

/* 選択範囲メニュー */
void HandleSelectionChoice(short item)
{
	WindowPtr	theWindow=MyFrontNonFloatingWindow();
	PaintWinRec	*eWinRec;
	
	if (GetExtWindowKind(theWindow)!=kWindowTypePaintWindow)
	{
		UpdateSelectionMenu();
		return;
	}
	
	eWinRec=GetPaintWinRec(theWindow);
	if (item != iAutoSelect && !eWinRec->isSelected)
	{
		UpdateSelectionMenu();
		return;
	}
	
	switch (item)
	{
		case iAutoSelect:
			DoAutoSelect(theWindow);
			break;
		
		case iCancelSelect:
			FixSelection(theWindow);
			UpdateSelectionMenu();
			UpdateClipMenu();
			SetUndoMode(umDeselect);
			break;
		
		case iHideSelection:
			eWinRec->showSelection = !eWinRec->showSelection;
			DispSelection(theWindow);
			UpdateSelectionMenu();
			break;
	}
}

/* プレビューの背景メニュー（サブメニュー） */
void HandleListBackgroundChoice(short item)
{
	MenuHandle	menu;
	
	if (item == gListBackground) return;
	
	menu=GetMenuHandle(mListBackground);
	CheckMenuItem(menu,item,true);
	CheckMenuItem(menu,gListBackground,false);
	gListBackground=item;
	
	/* プレビューを更新 */
	MyInvalWindowPortBounds(gPreviewWindow);
}

/* 前景の不透明度メニュー（サブメニュー） */
void HandleForeTransparencyChoice(short item)
{
	WindowPtr	theWindow;
	PaintWinRec	*eWinRec;
	Rect		r;
	short		newTrans;
	
	theWindow=MyFrontNonFloatingWindow();
	if (theWindow==nil || GetExtWindowKind(theWindow)!=kWindowTypePaintWindow) return;
	
	eWinRec=GetPaintWinRec(theWindow);
	newTrans=(item-iForeTrans100+kForeTrans100);
	if (newTrans==eWinRec->foreTransparency) return;
	
	eWinRec->pForeTransparency=eWinRec->foreTransparency; /* バックアップ */
	eWinRec->foreTransparency=newTrans;
	
	GetWindowPortBounds(theWindow,&r);
	r.right-=kScrollBarWidth;
	r.bottom-=kScrollBarHeight;
	RectRgn(eWinRec->updateRgn,&eWinRec->iconSize);
	MyInvalWindowPortBounds(theWindow);
	MyInvalWindowPortBounds(gPreviewWindow);
	SetPortWindowPort(theWindow);
	UpdateBGMenu();
	SetUndoMode(umChangeTransparency);
}

/* メニューのアップデート処理 */
void UpdateMenus(void)
{
	UpdateEffectMenu();
	UpdateTransparentMenu();
	UpdateClipMenu();
	UpdateBGMenu();
	UpdatePasteMenu();
	UpdateSaveMenu();
	UpdateSelectionMenu();
	
	UpdateEditWinData();
	UpdateUndoMenu();
	UpdateWindowMenu();
	UpdateIconMenu();
}

/* 新規メニューを設定によって変更 */
void UpdateNewMenu(void)
{
	MenuHandle	menu;
	Str255		itemStr;
	
	menu=GetMenuHandle(mFile);
	GetIndString(itemStr,137,(gPaintWinPrefs.askSize ? 2 : 1));
	SetMenuItemText(menu,iNew,itemStr);
}

/* 選択範囲のあるなしによって */
/* カット／コピー／消去／複製／選択範囲を保存メニューをアップデート */
/* 効果メニューの重ね合せに関するメニューも */
void UpdateClipMenu(void)
{
	MenuHandle	menu;
	WindowPtr	theWindow;
	RgnHandle	eSelectedRgn;
	IconListWinRec	*iWinRec;
	IconFamilyWinRec	*fWinRec;
	
	theWindow=MyFrontNonFloatingWindow();
	
	if (theWindow==nil) return;
	
	switch (GetExtWindowKind(theWindow))
	{
		case kWindowTypePaintWindow:
			eSelectedRgn=(GetPaintWinRec(theWindow))->eSelectedRgn;
			
			if (EmptyRgn(eSelectedRgn)) /* どこも選択されていない */
			{
				menu=GetMenuHandle(mEdit);
				MyDisableMenuItem(menu,iCut);
				MyDisableMenuItem(menu,iCopy);
				MyDisableMenuItem(menu,iClear);
				MyDisableMenuItem(menu,iDuplicate);
				
				menu=GetMenuHandle(mEffect);
				MyDisableMenuItem(menu,iOpaque);
				MyDisableMenuItem(menu,iTransparent);
				MyDisableMenuItem(menu,iBlend);
				MyDisableMenuItem(menu,iExchange);
				
				menu=GetMenuHandle(mFile);
				MyDisableMenuItem(menu,iSaveSelection);
			}
			else
			{
				menu=GetMenuHandle(mEdit);
				MyEnableMenuItem(menu,iCut);
				MyEnableMenuItem(menu,iCopy);
				MyEnableMenuItem(menu,iClear);
				MyEnableMenuItem(menu,iDuplicate);
				
				menu=GetMenuHandle(mEffect);
				MyEnableMenuItem(menu,iOpaque);
				MyEnableMenuItem(menu,iTransparent);
				MyEnableMenuItem(menu,iBlend);
				MyEnableMenuItem(menu,iExchange);
				
				menu=GetMenuHandle(mFile);
				MyEnableMenuItem(menu,iSaveSelection);
			}
			break;
		
		case kWindowTypeIconListWindow:
			menu=GetMenuHandle(mFile);
			MyDisableMenuItem(menu,iSaveSelection);
			
			menu=GetMenuHandle(mEdit);
			
			iWinRec=GetIconListRec(theWindow);
			if (IsMultiIconSelected(iWinRec) == 1)
			{
				MyEnableMenuItem(menu,iCut);
				MyEnableMenuItem(menu,iCopy);
				MyEnableMenuItem(menu,iClear);
				MyEnableMenuItem(menu,iDuplicate);
			}
			else
			{
				MyDisableMenuItem(menu,iCut);
				MyDisableMenuItem(menu,iCopy);
				MyDisableMenuItem(menu,iClear);
				MyDisableMenuItem(menu,iDuplicate);
			}	
			break;
		
		case kWindowTypeIconFamilyWindow:
			menu=GetMenuHandle(mFile);
			MyDisableMenuItem(menu,iSaveSelection);
			
			fWinRec=GetIconFamilyRec(theWindow);
			
			menu=GetMenuHandle(mEdit);
			MyDisableMenuItem(menu,iDuplicate);
			if (fWinRec->selectedIcon >= 0)
			{
				Handle	dataHandle;
				OSErr	err;
				
				err=GetDataFromIPIcon(&dataHandle,&fWinRec->ipIcon,fWinRec->selectedIcon);
				
				/* 選択アイコンあり */
				if (err==noErr && dataHandle != nil)
				{
					MyEnableMenuItem(menu,iCopy);
					
					if (fWinRec->selectedIcon != 0)
					{
						MyEnableMenuItem(menu,iCut);
						MyEnableMenuItem(menu,iClear);
					}
					break;
				}
			}
			else
			{
				if ((**fWinRec->iconNameTE).active)
				{
					if ((**fWinRec->iconNameTE).selEnd-(**fWinRec->iconNameTE).selStart > 0)
					{
						MyEnableMenuItem(menu,iCut);
						MyEnableMenuItem(menu,iCopy);
						MyEnableMenuItem(menu,iClear);
						break;
					}
				}
			}
			MyDisableMenuItem(menu,iCut);
			MyDisableMenuItem(menu,iCopy);
			MyDisableMenuItem(menu,iClear);
			break;
	}
}


/* クリップボードの内容によってペーストメニューを更新 */
void UpdatePasteMenu(void)
{
	MenuHandle	menu;
	long	dataSize=0;
	WindowPtr	theWindow;
	IconFamilyWinRec	*fWinRec;
	#if TARGET_API_MAC_CARBON
	OSErr	err;
	ScrapRef	scrap;
	#else
	long	offset;
	#endif
	
	menu=GetMenuHandle(mFile);
	#if TARGET_API_MAC_CARBON
	err=GetCurrentScrap(&scrap);
	if (err==noErr)
    {
        OSErr tempErr=GetScrapFlavorSize(scrap,kPICTClipType,&dataSize);
    }
	else
    {
        dataSize=0;
    }
	#else
	
	dataSize=GetScrap(0,kPICTClipType,&offset);
	#endif
	
	if (dataSize>0)
		MyEnableMenuItem(menu,iOpenClipboard);
	else
		MyDisableMenuItem(menu,iOpenClipboard);
	
	menu=GetMenuHandle(mEdit);
	
	theWindow=MyFrontNonFloatingWindow();
	if (theWindow==nil) return;
	
	switch (GetExtWindowKind(theWindow))
	{
		case kWindowTypePaintWindow:
			/* dataSizeは先ほど調べたものを流用 */
			break;
		
		case kWindowTypeIconListWindow:
			#if TARGET_API_MAC_CARBON
			if (err==noErr) err=GetScrapFlavorSize(scrap,kLarge1BitMask,&dataSize);
			if (dataSize==0) err=GetScrapFlavorSize(scrap,kXIconClipType,&dataSize);
			#else
			dataSize=GetScrap(0,kLarge1BitMask,&offset);
			if (dataSize<=0 && gSystemVersion>=0x0850)
				dataSize=GetScrap(0,kXIconFileType,&offset);
			#endif
			break;
		
		case kWindowTypeIconFamilyWindow:
			fWinRec=GetIconFamilyRec(theWindow);
			if (fWinRec->selectedIcon >= 0)
			{}
			else if ((**fWinRec->iconNameTE).active)
            {
			#if TARGET_API_MAC_CARBON
				if (err==noErr)
                {
                    OSErr tempErr=GetScrapFlavorSize(scrap,kTextClipType,&dataSize);
                }
			#else
				dataSize=GetScrap(0,kTextClipType,&offset);
			#endif
            }
			else
            {
				dataSize=0;
            }
			break;
		
		default:
			dataSize=0;
	}
	
	if (dataSize<=0)
		MyDisableMenuItem(menu,iPaste);
	else
		MyEnableMenuItem(menu,iPaste);
}

/* ツールメニューを更新（グリッド、ルーラーモード） */
void UpdateToolMenu(void)
{
	MenuHandle	menu;
		
	menu=GetMenuHandle(mGrid);
	
	CheckMenuItem(menu,iGIcon32,((gToolPrefs.gridMode & 0x10)!=0));
	CheckMenuItem(menu,iGIcon16,((gToolPrefs.gridMode & 0x20)!=0));
	CheckMenuItem(menu,gToolPrefs.gridMode & 0x0f,true);
	
	menu=GetMenuHandle(mTool);
	
	CheckMenuItem(menu,iRuler,gToolPrefs.showRuler);
	
	menu=GetMenuHandle(mColorPalette);
	CheckMenuItem(menu,gPaletteCheck+1,true);
}

/* 効果メニュー、編集メニューなどの更新 */
/* ウィンドウが表示されているかどうかによって更新する */
void UpdateEffectMenu(void)
{
	MenuHandle	menu;
	WindowPtr	theWindow;
	PaintWinRec	*eWinRec;
	short		windowKind;
	
	theWindow=MyFrontNonFloatingWindow();
	
	/* ウィンドウが開かれていない */
	if (theWindow==nil)
	{
		menu=GetMenuHandle(mEdit);
		MyDisableMenuItem(menu,0);
		
		menu=GetMenuHandle(mEffect);
		MyDisableMenuItem(menu,0);
		
		menu=GetMenuHandle(mFile);
		MyDisableMenuItem(menu,iClose);
		MyDisableMenuItem(menu,iSaveAs);
		
		DrawMenuBar();
		return;
	}
	
	windowKind = GetExtWindowKind(theWindow);
	switch (windowKind)
	{
		case kWindowTypePaintWindow:
			menu=GetMenuHandle(mFile);
			MyEnableMenuItem(menu,iClose);
			MyEnableMenuItem(menu,iSaveAs);
			
			menu=GetMenuHandle(mEdit);
			MyEnableMenuItem(menu,0);
			MyEnableMenuItem(menu,iSelectAll);
			
			/* 背景がある時はサイズ変更不可（暫定的処置） */
			eWinRec=GetPaintWinRec(theWindow);
			if (eWinRec->backgroundGWorld || eWinRec->foregroundGWorld)
				MyDisableMenuItem(menu,iIconSize);
			else
				MyEnableMenuItem(menu,iIconSize);
			MyEnableMenuItem(menu,iTypeString);
			
			menu=GetMenuHandle(mEffect);
			MyEnableMenuItem(menu,0);
			
			/* 描画点モードでは色変換の"6"などのショートカットを使用不可 */
/*			menu=GetMenuHandle(mColorMode);
			if (eWinRec->isDotMode)
			{
				SetItemCmd(menu,i216Color,0);
				SetItemCmd(menu,i16Color,0);
				SetItemCmd(menu,iAppleIconColor,0);
				SetItemCmd(menu,iMonochrome,0);
			}
			else
			{
				SetItemCmd(menu,i216Color,'6');
				SetItemCmd(menu,i16Color,'4');
				SetItemCmd(menu,iAppleIconColor,'3');
				SetItemCmd(menu,iMonochrome,'1');
			} */
			break;
		
		case kWindowTypeIconListWindow:
		case kWindowTypeIconFamilyWindow:
			menu=GetMenuHandle(mFile);
			MyEnableMenuItem(menu,iClose);
			if (windowKind == kWindowTypeIconFamilyWindow)
				MyEnableMenuItem(menu,iSaveAs);
			else
				MyDisableMenuItem(menu,iSaveAs);
			
			menu=GetMenuHandle(mEdit);
			MyEnableMenuItem(menu,0);
			if (windowKind == kWindowTypeIconListWindow)
				MyEnableMenuItem(menu,iSelectAll);
			else
				MyDisableMenuItem(menu,iSelectAll);
			MyDisableMenuItem(menu,iIconSize);
			MyDisableMenuItem(menu,iTypeString);
			
			menu=GetMenuHandle(mEffect);
			MyDisableMenuItem(menu,0);
			break;
		
	}
	
	DrawMenuBar();
}

/* undoメニューのアップデート */
void UpdateUndoMenu(void)
{
	WindowPtr	theWindow=MyFrontNonFloatingWindow();
	MenuHandle	menu;
	short		mode=umCannot;
	Str255		modeStr;
	
	menu=GetMenuHandle(mEdit);
	
	if (theWindow!=nil)
	{
		switch (GetExtWindowKind(theWindow))
		{
			case kWindowTypePaintWindow:
				mode=(GetPaintWinRec(theWindow))->undoMode;
				break;
			
			case kWindowTypeIconListWindow:
				mode=(GetIconListRec(theWindow))->undoData.undoMode;
				break;
			
			case kWindowTypeIconFamilyWindow:
				mode=(GetIconFamilyRec(theWindow))->undoData.undoMode;
				break;
		}
	}
	
	UseResFile(gApplRefNum);
	if (mode<0)
		GetIndString(modeStr,129,-mode+1);
	else
		GetIndString(modeStr,128,mode+1);
	SetMenuItemText(menu,iUndo,modeStr);
			
	if (mode==0)
		MyDisableMenuItem(menu,iUndo);
	else
		MyEnableMenuItem(menu,iUndo);
}

/* ウィンドウメニューのアップデート */
void UpdateWindowMenu(void)
{
	MenuHandle	menu=GetMenuHandle(mWindow);
	WindowPtr	theWindow=MyFrontNonFloatingWindow();
	WindowPtr	frontWindow=theWindow;
	PaintWinRec	*eWinRec;
	short		i;
	short		numItems;
	Str255		title;
	short		item;
	
	if (theWindow==nil)
		MyDisableMenuItem(menu,iMag);
	else
	{
		switch (GetExtWindowKind(theWindow))
		{
			case kWindowTypePaintWindow:
				MyEnableMenuItem(menu,iMag);
				
				menu=GetMenuHandle(mMag2);
				eWinRec=GetPaintWinRec(theWindow);
				for (i=0; i<4; i++)
					CheckMenuItem(menu,i+1,i==eWinRec->ratio);
				break;
			
			default:
				MyDisableMenuItem(menu,iMag);
				break;
		}
	}
	menu=GetMenuHandle(mWindow);
	
	/* まず、ウィンドウリストを消す */
	numItems=CountMenuItems(menu);
	while(numItems>3)
		DeleteMenuItem(menu,numItems--);
	
	/* ウィンドウの数を初期化 */
	eWinNum=0;
	iWinNum=0;
	fWinNum=0;
	
	/* 開かれているウィンドウリストを追加する */
	while (theWindow!=nil)
	{
		switch (GetExtWindowKind(theWindow))
		{
			case kWindowTypeIconListWindow:
				/* アイコンリストウィンドウ */
				iWinNum++;
				if (iWinNum==1)
					InsertMenuItem(menu,"\p-",3);
				
				item=3+iWinNum;
				InsertMenuItem(menu,"\p ",item);
				GetWTitle(theWindow,title);
				if (title[1]=='-')
				{
					BlockMoveData(&title[1],&title[2],title[0]++);
					title[1]='\r';
				}
				SetMenuItemText(menu,item+1,title);
				if (theWindow==frontWindow)
					CheckMenuItem(menu,item+1,true);
				break;
			
			case kWindowTypeIconFamilyWindow:
				/* ファミリウィンドウ */
				fWinNum++;
				item=3+1+(iWinNum==0 ? -1 : iWinNum);
				if (fWinNum==1)
					InsertMenuItem(menu,"\p-",item);
				
				InsertMenuItem(menu,"\p ",item+fWinNum);
				GetWTitle(theWindow,title);
				if (title[1]=='-')
				{
					BlockMoveData(&title[1],&title[2],title[0]++);
					title[1]='\r';
				}
				SetMenuItemText(menu,item+fWinNum+1,title);
				if (theWindow==frontWindow)
					CheckMenuItem(menu,item+fWinNum+1,true);
				break;
			
			case kWindowTypePaintWindow:
				eWinNum++;
				item=3+2+(iWinNum==0 ? -1 : iWinNum) + (fWinNum==0 ? -1 : fWinNum);
				if (eWinNum==1)
					InsertMenuItem(menu,"\p-",item);
				
				InsertMenuItem(menu,"\p ",item+eWinNum);
				GetWTitle(theWindow,title);
				if (title[1]=='-')
				{
					BlockMoveData(&title[1],&title[2],title[0]++);
					title[1]='\r';
				}
				SetMenuItemText(menu,item+eWinNum+1,title);
				if (theWindow==frontWindow)
					CheckMenuItem(menu,item+eWinNum+1,true);
				break;
		}
		theWindow=GetNextWindow(theWindow);
	}
}

/* アイコンメニューのアップデート */
void UpdateIconMenu(void)
{
	MenuHandle	menu=GetMenuHandle(mIcon);
	WindowPtr	theWindow=MyFrontNonFloatingWindow();
	short		winKind;
	
	if (theWindow!=nil)
	{
		if ((winKind = GetExtWindowKind(theWindow))==kWindowTypeIconListWindow)
		{
			IconListWinRec	*iWinRec=GetIconListRec(theWindow);
			short		num;
			
			MyEnableMenuItem(menu,iAddIcon);
			MyEnableMenuItem(menu,iImportIcon);
			if (isResEditAvailable)
				MyEnableMenuItem(menu,iOpenWithResEdit);
			else
				MyDisableMenuItem(menu,iOpenWithResEdit);
			
			num=IsMultiIconSelected(iWinRec);
			if (num == 1) /* １つだけ選ばれている時のみ有効 */
			{
				MyEnableMenuItem(menu,iIconInfo);
				MyEnableMenuItem(menu,iExportIcon);
			}
			else
			{
				MyDisableMenuItem(menu,iIconInfo);
				MyDisableMenuItem(menu,iExportIcon);
			}
			
			return;
		}
		if (winKind == kWindowTypeIconFamilyWindow)
		{
			IconFamilyWinRec	*fWinRec=GetIconFamilyRec(theWindow);
			Handle	dataHandle;
			OSErr	err;
			
			err = GetDataFromIPIcon(&dataHandle,&fWinRec->ipIcon,fWinRec->selectedIcon);
			if (err == noErr && dataHandle == NULL)
				MyEnableMenuItem(menu,iAddIcon);
			else
				MyDisableMenuItem(menu,iAddIcon);
		}
		else
			MyDisableMenuItem(menu,iAddIcon);
	}
	else
		MyDisableMenuItem(menu,iAddIcon);
	
	MyDisableMenuItem(menu,iImportIcon);
	MyDisableMenuItem(menu,iIconInfo);
	MyDisableMenuItem(menu,iExportIcon);
	MyDisableMenuItem(menu,iOpenWithResEdit);
}

/* 選択範囲メニューのアップデート */
void UpdateSelectionMenu(void)
{
	WindowPtr	theWindow=MyFrontNonFloatingWindow();
	MenuHandle	menu;
	Str255		itemStr;
	
	if (theWindow!=nil && GetExtWindowKind(theWindow) == kWindowTypePaintWindow)
	{
		PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
		
		menu=GetMenuHandle(mTool);
		MyEnableMenuItem(menu,iSelection);
		
		menu=GetMenuHandle(mSelection);
		MyEnableMenuItem(menu,iAutoSelect);
		
		if (eWinRec->isSelected)
		{
			MyEnableMenuItem(menu,iCancelSelect);
			MyEnableMenuItem(menu,iHideSelection);
			GetIndString(itemStr,142,(eWinRec->showSelection ? 1 : 2));
			SetMenuItemText(menu,iHideSelection,itemStr);
		}
		else
		{
			MyDisableMenuItem(menu,iCancelSelect);
			MyDisableMenuItem(menu,iHideSelection);
			GetIndString(itemStr,142,1);
			SetMenuItemText(menu,iHideSelection,itemStr);
		}
	}
	else
	{
		/* 選択範囲サブメニューそのものを使用不可能にする */
		menu=GetMenuHandle(mTool);
		MyDisableMenuItem(menu,iSelection);
	}
}

/* 取消メニュー、undo modeの設定 */
/* modeが負ならredo mode */
void SetUndoMode(short mode)
{
	PaintWinRec	*eWinRec;
	WindowPtr	theWindow;
	
	if (mode<=-undoModeNum || mode>=undoModeNum) return;
	
	/* ウィンドウの情報を得る */
	theWindow=MyFrontNonFloatingWindow();
	if (theWindow!=nil)
	{
		switch (GetExtWindowKind(theWindow))
		{
			case kWindowTypePaintWindow:
				eWinRec=GetPaintWinRec(theWindow);
				
				eWinRec->undoMode=mode;
				
				if (mode!=0 && mode!=umSelect && mode!=-umSelect)
				{
					eWinRec->iconHasChanged=true;
					#if powerc
					if (gSystemVersion >= 0x0850)
						SetWindowModified(theWindow,true);
					#endif
					UpdateSaveMenu();
				}
				break;
			
			default:
				break;
		}
	}
	
	UpdateUndoMenu();
}

/* 保存メニューの更新 */
void UpdateSaveMenu(void)
{
	MenuHandle	menu;
	Boolean		iconHasChanged=false,iconHasSaved=false;
	WindowPtr	theWindow;
	
	theWindow=MyFrontNonFloatingWindow();
	if (theWindow!=nil)
	{
		switch (GetExtWindowKind(theWindow))
		{
			case kWindowTypePaintWindow:
				iconHasChanged=(*GetPaintWinRec(theWindow)).iconHasChanged;
				iconHasSaved=(*GetPaintWinRec(theWindow)).iconHasSaved;
				switch ((*GetPaintWinRec(theWindow)).iconType.fileType)
				{
					case kGIFFileType:
					case kFolderIconType:
					case kWinIconType:
						/* GIF、アイコンつきフォルダ、Windowsアイコンについては、
						  読み込むルーチンがないため復帰不可能 */
						iconHasSaved=false;
						break;
				}
				break;
			
			case kWindowTypeIconListWindow:
				iconHasChanged=(*GetIconListRec(theWindow)).wasChanged;
				iconHasSaved=(*GetIconListRec(theWindow)).wasSaved;
				break;
			
			case kWindowTypeIconFamilyWindow:
				iconHasChanged=(*GetIconFamilyRec(theWindow)).wasChanged;
				iconHasSaved=(*GetIconFamilyRec(theWindow)).isSaved;
				
				/* 未保存の場合は、変更フラグが立っていることにして「保存」メニューを使用可に */
				if (!iconHasSaved) iconHasChanged = true;
				break;
		}
	}
	
	menu=GetMenuHandle(mFile);
	
	if (iconHasChanged)
	{
		MyEnableMenuItem(menu,iSave);
		if (iconHasSaved) MyEnableMenuItem(menu,iRevert);
		else MyDisableMenuItem(menu,iRevert);
	}
	else /* アイコンの内容が変化していなければ保存しない */
	{
		MyDisableMenuItem(menu,iSave);
		MyDisableMenuItem(menu,iRevert);
	}
}

/* バックグラウンドメニューの更新 */
void UpdateBGMenu(void)
{
	MenuHandle	menu;
	WindowPtr	theWindow=MyFrontNonFloatingWindow();
	Str255		itemStr;
	
	menu=GetMenuHandle(mEdit);
	
	if (theWindow != nil && GetExtWindowKind(theWindow)==kWindowTypePaintWindow)
	{
		PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
		
		MyEnableMenuItem(menu,iBackground);
		
		if (eWinRec->backgroundGWorld != nil || eWinRec->foregroundGWorld != nil)
		{
			short		i;
			
			MyEnableMenuItem(menu,iUnionBackground);
			MyEnableMenuItem(menu,iHideBackground);
			MyEnableMenuItem(menu,iForeTransparency);
			
			GetIndString(itemStr,148,5);
			SetMenuItemText(menu,iBackground,itemStr);
			
			MyEnableMenuItem(menu,iDeleteBackground);
			MyEnableMenuItem(menu,iDeleteForeground);
			
			GetIndString(itemStr,148,(eWinRec->dispBackground ? 1 : 2));
			SetMenuItemText(menu,iHideBackground,itemStr);
			
			menu=GetMenuHandle(mForeTransparency);
			for (i=iForeTrans100; i<=iForeTrans0; i++)
				CheckMenuItem(menu,i,(i-iForeTrans100+kForeTrans100 == eWinRec->foreTransparency));
		}
		else
		{
			GetIndString(itemStr,148,(gOtherPrefs.addForeground ? 3 : 4));
			SetMenuItemText(menu,iBackground,itemStr);
			
			MyDisableMenuItem(menu,iUnionBackground);
			MyDisableMenuItem(menu,iDeleteForeground);
			MyDisableMenuItem(menu,iDeleteBackground);
			MyDisableMenuItem(menu,iHideBackground);
			MyDisableMenuItem(menu,iForeTransparency);
		}
	}
	else
	{
		GetIndString(itemStr,148,(gOtherPrefs.addForeground ? 3 : 4));
		SetMenuItemText(menu,iBackground,itemStr);
		
		MyDisableMenuItem(menu,iBackground);
		MyDisableMenuItem(menu,iForeTransparency);
	}
}

/* 透明／不透明メニューの更新 */
void UpdateTransparentMenu(void)
{
	MenuHandle	menu;
	WindowPtr	theWindow=MyFrontNonFloatingWindow();
	Boolean		isOpaque=false,isTransparent=false;
	Boolean		isBlend=false;
	
	menu=GetMenuHandle(mEffect);
	
	if (theWindow != nil && GetExtWindowKind(theWindow)==kWindowTypePaintWindow)
	{
		PaintWinRec	*eWinRec=GetPaintWinRec(theWindow);
		
		if (eWinRec->isSelected)
		{
			OSErr	err;
			RgnHandle	maskRgn;
			
			/* マスクと選択範囲が一致する場合は不透明 */
			maskRgn=NewRgn();
			MyLockPixels(currentMask);
			err=BitMapToRegion(maskRgn,GetPortBitMapForCopyBits(eWinRec->currentMask));
			if (err==noErr)
			{
				OffsetRgn(maskRgn,eWinRec->selectionOffset.h,eWinRec->selectionOffset.v);
				SectRgn(maskRgn,eWinRec->selectionPos,maskRgn);
				if (EqualRgn(maskRgn,eWinRec->selectionPos))
					isOpaque=true;
			}
			MyUnlockPixels(currentMask);
			
			/* マスクと白以外の部分が一致する場合は透明 */
			if (err==noErr && !isOpaque)
			{
				GWorldPtr	maskGWorld;
				RgnHandle	maskRgn2;
				
				err=CreateMask2(eWinRec->editDataPtr,&maskGWorld);
				if (err==noErr)
				{
					maskRgn2=NewRgn();
					err=BitMapToRegion(maskRgn2,GetPortBitMapForCopyBits(maskGWorld));
					if (err==noErr)
					{
						OffsetRgn(maskRgn2,eWinRec->selectionOffset.h,eWinRec->selectionOffset.v);
						SectRgn(maskRgn2,eWinRec->selectionPos,maskRgn2);
						if (EqualRgn(maskRgn,maskRgn2))
							isTransparent=true;
					}
					DisposeRgn(maskRgn2);
					DisposeGWorld(maskGWorld);
				}
			}
			DisposeRgn(maskRgn);
			
			/* ブレンドモードかどうか */
			isBlend = (eWinRec->selectionBlend == blend);
		}
	}
	
	CheckMenuItem(menu,iOpaque,isOpaque);
	CheckMenuItem(menu,iTransparent,isTransparent);
	CheckMenuItem(menu,iBlend,isBlend);
}
/* ------------------------------------------------------------ */
/*  DotPalette.c                                                */
/*     描画点パレット処理                                       */
/*                                                              */
/*                 2001.11.4 - 2001.11.4  naoki iimura        	*/
/* ------------------------------------------------------------ */

/* includes */
#ifdef __APPLE_CC__
#include	<Carbon/Carbon.h>
#else
#endif

#include	"Globals.h"
#include	"MenuRoutines.h"
#include	"UsefulRoutines.h"
#include	"IconParty.h"
#include	"WindowRoutines.h"
#include	"Preferences.h"
#include	"PaintRoutines.h"
#include	"DotPalette.h"
#include	"PreCarbonSupport.h"
#include	"UpdateCursor.h"


static void	DoDotLibPop(short h,short v);
static void	DoDotCmdPop(short h,short v);
static void	DoRecDotPop(short h,short v);
static void	DoInputDotPop(short h,short v);
static void	DoLibNamePop(short h,short v);

static void	AddCmdToLib(void);

static short	AddNewDotLib(FSSpec *theFile);
static void	AddNewDotLibMain(FSSpec *theFile);

static void	DoEditLibrary(void);
static pascal Boolean	EditLibDialogFilter(DialogPtr theDialog,EventRecord *theEvent,short *theItemHit);
static Boolean	MyLClick(Point localPt,UInt32 time,ListHandle theList);
static void	DoEditSelectedCommand(ListHandle theList);
static short	DoEditDotCommand(Str255 command,ListHandle theList);
static pascal Boolean	EditCmdDialogFilter(DialogPtr theDialog,EventRecord *theEvent,short *theItemHit);
static OSErr	CheckCommandStr(Str255 command);
static void	DoDeleteSelectedCommand(ListHandle theList);
static OSErr	DoDeleteDotCommand(Str255 command,ListHandle theList);
static pascal Boolean	DeleteCmdDialogFilter(DialogPtr theDialog,EventRecord *theEvent,short *theItemHit);

static void	UpdateSelectedLib(Str31 libName);
static void	UpdateDotCmdPop(void);
static short	CountCmdNum(void);

static long	PopUpMenuSelectDotFont(MenuHandle menu,short v,short h,short item);


#define	DOTERR_RESID	4002
#define	DOTERR1	1
#define	DOTERR2	2
#define	DOTERR3	3


extern WindowPtr	DotModePalette;


/* 描画点パレットのクリック */
void ClickDotModePalette(Point localPt)
{
	Rect	r;
	
	/* ライブラリポップアップ */
	SetRect(&r,0x55,0x03,0x60,0x0e);
	if (PtInRect(localPt,&r))
		DoDotLibPop(0x55,0x03);
	else
	{
		/* コマンドポップアップ */
		SetRect(&r,0x55,0x12,0x60,0x1d);
		if (PtInRect(localPt,&r))
			DoDotCmdPop(0x55,0x12);
		else
		{
			/* 記録コマンドポップアップ */
			SetRect(&r,0x05,19,0x4f,0x1b);
			if (PtInRect(localPt,&r))
				DoRecDotPop(localPt.h,localPt.v);
			else
			{
				/* 入力中コマンドポップアップ */
				SetRect(&r,0x05,0x22,0x60,0x3f);
				if (PtInRect(localPt,&r))
					DoInputDotPop(localPt.h,localPt.v);
				else
				{
					/* ライブラリ名 */
					SetRect(&r,0x05,0x03,0x4f,0x0e);
					if (PtInRect(localPt,&r))
						DoLibNamePop(localPt.h,localPt.v);
				}
			}
		}
	}
}

/* ライブラリポップアップ */
void DoDotLibPop(short h,short v)
{
	MenuHandle	menu;
	FSSpec		theSpec;
	long		index=1;
	Boolean		isDirectory;
	long		selItem;
	OSErr		err;
	FInfo	fileInfo;
	short		itemNum=0;
	Point		popPt;
	short		item,checkItem=1;
	Boolean	cmdDown;
	KeyMap	theKeys;
	
	/* ライブラリフォルダが存在しない場合はなにもしない */
	if (!isDotLibAvailable) return;
	
	/* ライブラリフォルダにあるライブラリファイルのリストを作り、メニュー化 */
	menu=GetMenu(mDotLibPop);
	
	theSpec.vRefNum=gDotLibVRefNum;
	theSpec.parID=gDotLibFolderID;
	
	while (GetFile(&theSpec,index++,&isDirectory)==noErr)
	{
		if (!isDirectory)
		{
			err=FSpGetFInfo(&theSpec,&fileInfo);
			if (fileInfo.fdType==kDotModeLibFileType)
			{
				InsertMenuItem(menu,"\p ",itemNum++);
				SetMenuItemText(menu,itemNum,theSpec.name);
				if (EqualString(theSpec.name,gDotLibName,false,true))
				{
					CheckMenuItem(menu,itemNum,true);
					checkItem=itemNum;
				}
			}
		}
	}
	
	SetPt(&popPt,h,v);
	LocalToGlobal(&popPt);
	
	InsertMenu(menu,-1);
	selItem=PopUpMenuSelectWFontSize(menu,popPt.v,popPt.h,checkItem,9);
	
	GetKeys(theKeys);
	cmdDown=BitTst(theKeys,48);
	
	item=LoWord(selItem);
	if (item>0)
	{
		Str255	name;
		
		if (item > itemNum)
		{
			if (AddNewDotLib(&theSpec)==ok)
				AddNewDotLibMain(&theSpec);
		}
		else
		{
			/* ライブラリファイルのspecを得る */
			GetMenuItemText(menu,item,name);
			err=FSMakeFSSpec(gDotLibVRefNum,gDotLibFolderID,name,&theSpec);
			if (err==noErr)
				SetDotLibMain(&theSpec);
		}
	}
	DeleteMenu(mDotLibPop);
	ReleaseResource((Handle)menu);
}

/* 描画点ライブラリの作成 */
short AddNewDotLib(FSSpec *theFile)
{
	Str255		name;
	DialogPtr	theDialog;
	short		item=3;
	ModalFilterUPP	mfUPP=NewModalFilterUPP(MyModalDialogFilter);
	OSErr		err;
	
	MySetCursor(0);
	DeactivateFloatersAndFirstDocumentWindow();
	theDialog=GetNewDialog(142,nil,kFirstWindowOfClass);
	SetDialogDefaultItem(theDialog,ok);
	SetDialogCancelItem(theDialog,cancel);
	SelectDialogItemText(theDialog,3,0,255);
	ShowWindow(GetDialogWindow(theDialog));
	
	while (item!=ok && item!=cancel)
	{
		ModalDialog(mfUPP,&item);
		
		if (item==ok)
		{
			GetDialogItemText2(theDialog,3,name);
			if (name[0]>31)
			{
				ErrorAlertFromResource(DOTERR_RESID,DOTERR1);
				item=3;
				continue;
			}
			ReplaceString(name,"\p:","\p-");
			err=FSMakeFSSpec(gDotLibVRefNum,gDotLibFolderID,name,theFile);
			if (err==noErr)
			{
				ErrorAlertFromResource(DOTERR_RESID,DOTERR2);
				item=3;
				continue;
			}
			else if (err!=fnfErr)
			{
				ErrorAlertFromResource(DOTERR_RESID,DOTERR3);
				item=cancel;
			}
		}
	}
	
	DisposeDialog(theDialog);
	DisposeModalFilterUPP(mfUPP);
	ActivateFloatersAndFirstDocumentWindow();
	
	return item;
}

/* 描画点ライブラリの作成（メイン） */
void AddNewDotLibMain(FSSpec *theFile)
{
	short	num=0;
	short	refNum;
	
	FSpCreateResFile(theFile,kIconPartyCreator,kDotModeLibFileType,smSystemScript);
	if (ResError())
	{
		ErrorAlertFromResource(DOTERR_RESID,DOTERR3);
		return;
	}
	
	/* 中身が空の'STR#'リソースを作る */
	refNum=FSpOpenResFile(theFile,fsWrPerm);
	if (refNum>0)
	{
		UseResFile(refNum);
		AddDataToPrefs(&num,sizeof(num),'STR#',128,"\p");
		CloseResFile(refNum);
	}
	
	SetDotLibMain(theFile);
}

/* 描画点ライブラリの指定 */
void SetDotLibMain(FSSpec *theFile)
{
	short	refNum;
	GrafPtr	port;
	
	refNum=FSpOpenResFile(theFile,fsRdWrPerm);
	
	if (refNum > 0)
	{
		if (refNum == gDotLibRefNum) return;
		
		/* ライブラリファイルが開かれていれば閉じる */
		if (gDotLibRefNum > 0) CloseResFile(gDotLibRefNum);
		gDotLibRefNum=refNum;
		
		UseResFile(gApplRefNum);
		PStrCpy(theFile->name,gDotLibName);
		
		GetPort(&port);
		SetPortWindowPort(DotModePalette);
		UpdateSelectedLib(gDotLibName);
		UpdateDotCmdPop();
		SetPort(port);
	}
	else
		SysBeep(0);
}

/* コマンドポップアップ */
void DoDotCmdPop(short h,short v)
{
	MenuHandle	menu;
	short		cmdNum;
	short		i;
	Str255		command;
	Point		popPt;
	long		selItem;
	short		item,checkItem=0;
	KeyMap		theKeys;
	Boolean		cmdDown;
	
	/* ライブラリファイルが開かれていない、あるいは中身が空の場合は何もしない */
	if ((cmdNum=CountCmdNum())<=0) return;
	
	/* ライブラリファイルに含まれるコマンド列を取り出し、メニュー化 */
	UseResFile(gDotLibRefNum);
	
	menu=GetMenu(mDotCmdPop);
	
	for (i=1; i<=cmdNum; i++)
	{
		GetIndString(command,128,i);
		InsertMenuItem(menu,command,i-1);
		if (EqualString(command,gDotCommand,true,true))
		{
			CheckMenuItem(menu,i,true);
			checkItem=i;
		}
	}
	
	UseResFile(gApplRefNum);
	SetPt(&popPt,h,v);
	LocalToGlobal(&popPt);
	
	InsertMenu(menu,-1);
	selItem=PopUpMenuSelectDotFont(menu,popPt.v,popPt.h,checkItem);
	
	GetKeys(theKeys);
	cmdDown=BitTst(theKeys,48);
	
	item=LoWord(selItem);
	
	if (item>0)
	{
		if (cmdDown)
		{
			/* コマンド削除 */
			Handle	h;
			Size	s;
			Ptr		p,p2;
			
			UseResFile(gDotLibRefNum);
			h=Get1Resource('STR#',128);
			s=GetHandleSize(h);
			HLock(h);
			p=(*h)+sizeof(short);
			for (i=1; i<=cmdNum; i++)
			{
				p2=p+p[0]+1;
				if (i==item)
				{
					BlockMoveData(p2,p,s+(*h)-p2);
					break;
				}
				p=p2;
			}
			(**(short **)h)--;
			HUnlock(h);
			SetHandleSize(h,s-p[0]-1);
			ChangedResource(h);
			WriteResource(h);
			UseResFile(gApplRefNum);
			
			if (cmdNum==1) UpdateDotCmdPop();
		}
		else
		{
			/* 選ばれたコマンドを得る */
			Str255	cmd;
			GrafPtr	port;
			
			GetMenuItemText(menu,item,cmd);
			if (cmd[0]>30)
			{
				cmd[0]=30; /* 長すぎる時は短くする */
				SysBeep(0);
			}
			PStrCpy(cmd,gDotCommand);
			
			GetPort(&port);
			SetPortWindowPort(DotModePalette);
			UpdateRecordedCommand(gDotCommand);
			SetPort(port);
		}
	}
	
	DeleteMenu(mDotCmdPop);
	ReleaseResource((Handle)menu);
}

/* 記録コマンドポップアップ */
void DoRecDotPop(short h,short v)
{
	MenuHandle	menu;
	long		selItem;
	short		item;
	Point		popPt;
	WindowPtr	theWindow=MyFrontNonFloatingWindow();
	
	/* コマンドがなにも記録されていない時はなにもしない */
	if (gDotCommand[0]==0) return;
	if (theWindow==nil) return;
	if (GetExtWindowKind(theWindow) != kWindowTypePaintWindow) return;
	
	menu=GetMenu(mDotRecPop);
	SetPt(&popPt,h,v);
	LocalToGlobal(&popPt);
	
	if (CountCmdNum()<0)
		MyDisableMenuItem(menu,iAddCmdToLib);
	
	InsertMenu(menu,-1);
	selItem=PopUpMenuSelectWFontSize(menu,popPt.v,popPt.h,0,9);
	
	item=LoWord(selItem);
	switch (item)
	{
		case iInputCmd:
			ExecuteCommand(theWindow,gDotCommand);
			break;
		
		case iAddCmdToLib:
			AddCmdToLib();
			break;
	}
	DeleteMenu(mDotRecPop);
	ReleaseResource((Handle)menu);
}

/* 記録中コマンドをライブラリに追加 */
void AddCmdToLib(void)
{
	short	cmdNum;
	short	i;
	Boolean	dup=false;
	Str255	cmd;
	Handle	h;
	
	cmdNum=CountCmdNum();
	if (cmdNum<0)
	{
		SysBeep(0);
		return;
	}
	
	UseResFile(gDotLibRefNum);
	
	/* 重複チェック */
	for (i=1; i<=cmdNum; i++)
	{
		GetIndString(cmd,128,i);
		if (EqualString(cmd,gDotCommand,true,true))
		{
			dup=true;
			break;
		}
	}
	if (dup)
	{
		SysBeep(0);
		UseResFile(gApplRefNum);
		return;
	}
	
	/* 追加処理 */
	h=Get1Resource('STR#',128);
	if (h==nil)
	{
		h=NewHandleClear(sizeof(short));
		AddResource(h,'STR#',128,"\p");
	}
	PtrAndHand(&gDotCommand[0],h,gDotCommand[0]+1);
	HLock(h);
	(**(short **)h)++;
	HUnlock(h);
	
	ChangedResource(h);
	WriteResource(h);
	UpdateResFile(gDotLibRefNum);
	
	UseResFile(gApplRefNum);
	
	if (cmdNum==0) UpdateDotCmdPop();
}

/* 入力中コマンドポップアップ */
void DoInputDotPop(short h,short v)
{
	MenuHandle	menu;
	long		selItem;
	short		item;
	Point		popPt;
	WindowPtr	theWindow=MyFrontNonFloatingWindow();
	PaintWinRec	*eWinRec;
	
	/* 入力されているコマンドがない場合はなにもしない */
	if (theWindow==nil) return;
	if (GetExtWindowKind(theWindow) != kWindowTypePaintWindow) return;
	eWinRec=GetPaintWinRec(theWindow);
	if (eWinRec->dotCommand[0]==0) return;
	
	menu=GetMenu(mDotInpPop);
	SetPt(&popPt,h,v);
	LocalToGlobal(&popPt);
	
	InsertMenu(menu,-1);
	selItem=PopUpMenuSelectWFontSize(menu,popPt.v,popPt.h,0,9);
	
	item=LoWord(selItem);
	switch (item)
	{
		case iClearCmd:
			HandleDotKey(keyDown,0x1b,theWindow);
			break;
		
		case iRecordCmd:
			HandleDotKey(keyDown,'0',theWindow);
			break;
	}
	DeleteMenu(mDotInpPop);
	ReleaseResource((Handle)menu);
}

/* ライブラリ名ポップアップメニュー */
void DoLibNamePop(short h,short v)
{
	MenuHandle	menu;
	long		selItem;
	short		item;
	Point		popPt;
	short		cmdNum;
	
	/* ライブラリファイルが開かれていない、あるいは中身が空の場合は何もしない */
	if ((cmdNum=CountCmdNum())<=0) return;
	
	SetPt(&popPt,h,v);
	/* ダブルクリック判定 */
	{
		Rect	box;
		Point	newPt;
		Boolean	isDoubleClick=false;
		UInt32	clickTime=TickCount()+GetDblTime();
		EventRecord	followEvent;
		
		SetRect(&box,popPt.h-2,popPt.v-2,popPt.h+2,popPt.v+2);
		while(TickCount()<clickTime && !isDoubleClick)
		{
			GetMouse(&newPt);
			if (!PtInRect(newPt,&box)) break;
			isDoubleClick=EventAvail(mDownMask,&followEvent);
		}
		if (isDoubleClick) /* ダブルクリックは描画点モード */
		{
			FlushEvents(mDownMask,0);
			DoEditLibrary();
			return;
		}
	}
	
	menu=GetMenu(mDotLibNamePop);
	LocalToGlobal(&popPt);
	
	InsertMenu(menu,-1);
	selItem=PopUpMenuSelectWFontSize(menu,popPt.v,popPt.h,0,9);
	
	item=LoWord(selItem);
	switch (item)
	{
		case iEditLib:
			DoEditLibrary();
			break;
	}
	DeleteMenu(mDotLibNamePop);
	ReleaseResource((Handle)menu);
}

#define	kEditLibDialogID	145
enum {
	kDotCmdListIndex=3,
	kAddDotCmdIndex=4,
	kEditDotCmdIndex=5,
	kDeleteDotCmdIndex=6,
};

/* 描画点ライブラリの編集 */
void DoEditLibrary(void)
{
	DialogPtr	theDialog;
	ListHandle	theList;
	Rect		listView,dataBounds;
	Point		cellSize;
	short		item=3;
	short		i,cmdNum;
	ModalFilterUPP	mfUPP=NewModalFilterUPP(EditLibDialogFilter);
	GrafPtr		port;
	Str255		fontName;
	short		fNum;
	Str255		dotCmd;
	Cell		theCell;
	
	GetPort(&port);
	MySetCursor(0);
	DeactivateFloatersAndFirstDocumentWindow();
	theDialog=GetNewDialog(kEditLibDialogID,nil,kFirstWindowOfClass);
	SetDialogDefaultItem(theDialog,ok);
	SetDialogCancelItem(theDialog,cancel);
	
	/* リスト */
	GetDialogItemRect(theDialog,kDotCmdListIndex,&listView);
	listView.right-=kScrollBarWidth; /* scroll bar */
	SetRect(&dataBounds,0,0,1,(cmdNum=CountCmdNum()));
	SetPt(&cellSize,listView.right - listView.left,10);
	
	SetPortDialogPort(theDialog);
	GetIndString(fontName,150,1);
	#if TARGET_API_MAC_CARBON
	fNum=FMGetFontFamilyFromName(fontName);
	#else
	GetFNum(fontName,&fNum);
	#endif
	TextFont(fNum);
	TextSize(12);
	
	theList=LNew(&listView,&dataBounds,cellSize,0,GetDialogWindow(theDialog),false,true,false,true);
	SetListSelectionFlags(theList,lOnlyOne+lNoNilHilite);
	SetWRefCon(GetDialogWindow(theDialog),(long)theList);
	
	/* リストに項目を追加 */
	UseResFile(gDotLibRefNum);
	for (i=1; i<=cmdNum; i++)
	{
		GetIndString(dotCmd,128,i);
		SetPt(&theCell,0,i-1);
		LSetCell(&dotCmd[1],dotCmd[0],theCell,theList);
	}
	UseResFile(gApplRefNum);
	
	LSetDrawingMode(true,theList);
	
	ShowWindow(GetDialogWindow(theDialog));
	SetPortDialogPort(theDialog);
	MyInvalWindowPortBounds(GetDialogWindow(theDialog));
	
	while (item != ok && item != cancel)
	{
		ModalDialog(mfUPP,&item);
		
		switch (item)
		{
			case kAddDotCmdIndex:
				dotCmd[0]=0;
				if (DoEditDotCommand(dotCmd,theList) == ok)
				{
					/* アイテムの追加 */
					GetListDataBounds(theList,&dataBounds);
					cmdNum = dataBounds.bottom;
					LAddRow(1,cmdNum,theList);
					SetPt(&theCell,0,cmdNum++);
					LSetCell(&dotCmd[1],dotCmd[0],theCell,theList);
					CancelSelect(theList);
					LSetSelect(true,theCell,theList);
					LScroll(0,cmdNum,theList);
				}
				break;
			
			case kEditDotCmdIndex:
				DoEditSelectedCommand(theList);
				break;
			
			case kDeleteDotCmdIndex:
				DoDeleteSelectedCommand(theList);
				break;
		}
	}
	
	if (item == ok)
	{
		/* 保存する */
		Handle	resHandle;
		short	dataSize;
		OSErr	err;
		
		UseResFile(gDotLibRefNum);
		resHandle=Get1Resource('STR#',128);
		SetHandleSize(resHandle,0);
		
		/* アイテム数 */
		GetListDataBounds(theList,&dataBounds);
		cmdNum = dataBounds.bottom;
		err=PtrAndHand(&cmdNum,resHandle,sizeof(short));
		
		/* 各アイテム */
		theCell.h=0;
		for (i=0; i<cmdNum; i++)
		{
			theCell.v=i;
			dataSize=255;
			LGetCell(&dotCmd[1],&dataSize,theCell,theList);
			dotCmd[0]=dataSize;
			err=PtrAndHand(&dotCmd[0],resHandle,dotCmd[0]+1);
		}
		
		ChangedResource(resHandle);
		WriteResource(resHandle);
		ReleaseResource(resHandle);
		
		UseResFile(gApplRefNum);
	}
	
	LDispose(theList);
	DisposeModalFilterUPP(mfUPP);
	DisposeDialog(theDialog);
	ActivateFloatersAndFirstDocumentWindow();
	SetPort(port);
}

/* 選択コマンドの編集 */
void DoEditSelectedCommand(ListHandle theList)
{
	Cell	theCell;
	short	dataSize;
	Str255	dotCmd;
	
	SetPt(&theCell,0,0);
	if (LGetSelect(true,&theCell,theList)) /* 選択されている */
	{
		dataSize=255;
		LGetCell(&dotCmd[1],&dataSize,theCell,theList);
		dotCmd[0]=dataSize;
		if (DoEditDotCommand(dotCmd,theList) == ok) /* 変更された */
			LSetCell(&dotCmd[1],dotCmd[0],theCell,theList);
	}
	else SysBeep(0);
}

/* 選択コマンドの削除 */
void DoDeleteSelectedCommand(ListHandle theList)
{
	Cell	theCell;
	short	dataSize;
	Str255	dotCmd;
	
	SetPt(&theCell,0,0);
	if (LGetSelect(true,&theCell,theList)) /* 選択されている */
	{
		dataSize=255;
		LGetCell(&dotCmd[1],&dataSize,theCell,theList);
		dotCmd[0]=dataSize;
		if (DoDeleteDotCommand(dotCmd,theList) == noErr) /* 削除ok */
			LDelRow(1,theCell.v,theList);
	}
	else SysBeep(0);
}

/* ライブラリ編集ダイアログのフィルタ */
pascal Boolean EditLibDialogFilter(DialogPtr theDialog,EventRecord *theEvent,short *theItemHit)
{
	Boolean		eventHandled=false;
	WindowPtr	theWindow;
	short		part;
	char		key;
	ListHandle	theList;
	Rect		box;
	
	switch (theEvent->what)
	{
		case updateEvt:
			theWindow=(WindowPtr)theEvent->message;
			if (theWindow == GetDialogWindow(theDialog))
			{
				Boolean	active;
				GrafPtr	port;
				RgnHandle	rgn=NewRgn();
				
				GetPort(&port);
				theList=(ListHandle)GetWRefCon(GetDialogWindow(theDialog));
				active=GetListActive(theList);
				
				SetPortDialogPort(theDialog);
				BeginUpdate(GetDialogWindow(theDialog));
				
				DrawDialog(theDialog);
				if (isAppearanceAvailable)
					SetThemeWindowBackground(theWindow,kThemeBrushWhite,false);
				GetPortVisibleRegion(GetWindowPort(theWindow),rgn);
				LUpdate(rgn,theList);
				
				GetDialogItemRect(theDialog,kDotCmdListIndex,&box);
				if (isAppearanceAvailable)
				{
					SetThemeWindowBackground(theWindow,(active ? kThemeBrushModelessDialogBackgroundActive :
														kThemeBrushModelessDialogBackgroundInactive),false);
					
//					InsetRect(&box,1,1);
					DrawThemeListBoxFrame(&box,active ? kThemeStateActive : kThemeStateInactive);
//					InsetRect(&box,-1,-1);
					DrawThemeFocusRect(&box,active ? kThemeStateActive : kThemeStateInactive);
				}
				else
				{
					InsetRect(&box,-1,-1);
					FrameRect(&box);
				}
				EndUpdate(GetDialogWindow(theDialog));
				SetPort(port);
			}
			else if (theWindow!=nil && GetWindowKind(theWindow)!=kDialogWindowKind)
				DoUpdate(theEvent);
			break;
		
		case keyDown:
		case autoKey:
			key=theEvent->message & charCodeMask;
			if ((theEvent->modifiers & cmdKey)!=0) {
				if (key==kPeriod)
					key=kEscapeKey;
				else if (key == 'K')
				{
					*theItemHit = kAddDotCmdIndex;
					HiliteButton(theDialog,*theItemHit);
					eventHandled=true;
				}
            }
			
			switch (key)
			{
				case kReturnKey:
				case kEnterKey:
					*theItemHit=ok;
					HiliteButton(theDialog,ok);
					eventHandled=true;
					break;
				
				case kEscapeKey:
					*theItemHit=cancel;
					HiliteButton(theDialog,cancel);
					eventHandled=true;
					break;
				
				case kDeleteKey: /* DELETE */
					*theItemHit=kDeleteDotCmdIndex;
					HiliteButton(theDialog,*theItemHit);
					eventHandled=true;
					break;
			}
			break;
		
		case mouseDown:
			part=FindWindow(theEvent->where,&theWindow);
			if (theWindow==GetDialogWindow(theDialog))
			{
				if (part==inDrag)
				{
					Rect	myScreenRect;
					
					GetRegionBounds(GetGrayRgn(),&myScreenRect);
					DragWindow(theWindow,theEvent->where,&myScreenRect);
					eventHandled=true;
				}
				else if (part == inContent)
				{
					Point	localPt;
					
					GetDialogItemRect(theDialog,kDotCmdListIndex,&box);
					localPt=theEvent->where;
					GlobalToLocal(&localPt);
					if (PtInRect(localPt,&box))
					{
						theList=(ListHandle)GetWRefCon(GetDialogWindow(theDialog));
						GetListViewBounds(theList,&box);
						if (PtInRect(localPt,&box)) /* リスト領域 */
						{
							Boolean	doubleClicked;
							
							doubleClicked=MyLClick(localPt,theEvent->when,theList);
							FlushEvents(mDownMask,0);
							if (doubleClicked)
							{
								*theItemHit = kEditDotCmdIndex; /* 編集 */
								HiliteButton(theDialog,*theItemHit);
							}
						}
						else /* スクロールバー領域 */
						{
							if (isAppearanceAvailable)
								SetThemeWindowBackground(theWindow,kThemeBrushWhite,false);
							LClick(localPt,0,theList);
							if (isAppearanceAvailable)
								SetThemeWindowBackground(theWindow,kThemeBrushModelessDialogBackgroundActive,false);
						}
						eventHandled=true;
					}
				}
			}
			break;
	}
	
	return eventHandled;
}

/* 描画コマンドリストクリック処理（ドラッグ、ダブルクリックサポート） */
Boolean MyLClick(Point localPt,UInt32 time,ListHandle theList)
{
	Cell	theCell;
	Rect	listRect,dataBounds,visible;
	Rect	upScrollRect,downScrollRect;
	Point	pt,cellSize;
	Rect	dRect;
	short	v,h1,h2,pv;
	Handle	dataHandle;
	short	size;
	short	dataNum;
	UInt32	ticks;
	short	cellHeight;
	WindowPtr	theWindow=GetWindowFromPort(GetListPort(theList));
	Pattern	pat;
	
	GetListViewBounds(theList,&listRect);
	GetListDataBounds(theList,&dataBounds);
	GetListCellSize(theList,&cellSize);
	GetListVisibleCells(theList,&visible);
	dataNum=dataBounds.bottom;
	cellHeight=cellSize.v;
	
	SetPt(&theCell,0,(localPt.v-listRect.top)/cellHeight+visible.top);
	if (theCell.v >= dataNum) /* データが存在しない領域 */
	{
		if (isAppearanceAvailable)
			SetThemeWindowBackground(theWindow,kThemeBrushWhite,false);
		CancelSelect(theList);
		if (isAppearanceAvailable)
			SetThemeWindowBackground(theWindow,kThemeBrushModelessDialogBackgroundActive,false);
		return false;
	}
	if (!LGetSelect(false,&theCell,theList)) /* 選択されていなければ選択 */
	{
		if (isAppearanceAvailable)
			SetThemeWindowBackground(theWindow,kThemeBrushWhite,false);
		CancelSelect(theList);
		LSetSelect(true,theCell,theList);
		if (isAppearanceAvailable)
			SetThemeWindowBackground(theWindow,kThemeBrushModelessDialogBackgroundActive,false);
	}
	
	#if TARGET_API_MAC_CARBON
	GetQDGlobalsGray(&pat);
	#else
	pat=qd.gray;
	#endif
	
	/* ダブルクリック判定 */
	ticks=time+GetDblTime();
	SetRect(&dRect,localPt.h-2,localPt.v-2,localPt.h+3,localPt.v+3);
	pt=localPt;
	while (StillDown() && PtInRect(pt,&dRect) && TickCount()<ticks)
		GetMouse(&pt);
	if (StillDown())
	{
		/* ドラッグ */
		upScrollRect=listRect;
		upScrollRect.bottom=upScrollRect.top;
		upScrollRect.top-=cellHeight;
		downScrollRect=listRect;
		downScrollRect.top=downScrollRect.bottom;
		downScrollRect.bottom+=cellHeight;
		
		PenPat(&pat);
		PenMode(srcXor);
		h1=listRect.left+1;
		h2=listRect.right-1;
		pv=-1;
		
		while (StillDown())
		{
			GetMouse(&pt);
			if (PtInRect(pt,&listRect))
			{
				/* リスト領域 */
				v=(pt.v-listRect.top+cellHeight/2)/cellHeight;
				if (v>dataNum) v=dataNum;
				v=v*cellHeight+listRect.top-1;
				if (pv != v)
				{
					PenPat(&pat);
					PenMode(srcXor);
					MoveTo(h1,v);
					LineTo(h2,v);
					MoveTo(h1,pv);
					LineTo(h2,pv);
					pv=v;
				}
			}
			else
			{
				/* その他の領域の場合、前の線を消す */
				PenPat(&pat);
				PenMode(srcXor);
				MoveTo(h1,pv);
				LineTo(h2,pv);
				pv=-1;
				
				if (PtInRect(pt,&upScrollRect))
				{
					if (isAppearanceAvailable)
						SetThemeWindowBackground(theWindow,kThemeBrushWhite,false);
					LScroll(0,-1,theList);
					if (isAppearanceAvailable)
						SetThemeWindowBackground(theWindow,kThemeBrushModelessDialogBackgroundActive,false);
					Delay(5,&ticks);
				}
				else if (PtInRect(pt,&downScrollRect))
				{
					if (isAppearanceAvailable)
						SetThemeWindowBackground(theWindow,kThemeBrushWhite,false);
					LScroll(0,1,theList);
					if (isAppearanceAvailable)
						SetThemeWindowBackground(theWindow,kThemeBrushModelessDialogBackgroundActive,false);
					Delay(5,&ticks);
				}
			}
		}
		/* 最後に、線を消す */
		MoveTo(h1,pv);
		LineTo(h2,pv);
		
		if (pv >= 0)
		{
			/* データを移動する必要がある */
			v=(pv+1-listRect.top)/cellHeight+visible.top;
			
			/* 移動してなければなにもしない */
			if (v==theCell.v || v==theCell.v+1)
			{
				PenNormal();
				return false;
			}
			
			LSetDrawingMode(false,theList);
			
			/* リストの交換 */
			size=255;
			LGetCell(&dataHandle,&size,theCell,theList);
			LDelRow(1,theCell.v,theList);
			if (theCell.v<v) v--;
			
			LAddRow(1,v,theList);
			theCell.v=v;
			LSetCell(&dataHandle,size,theCell,theList);
			LSetSelect(true,theCell,theList);
			
			LSetDrawingMode(true,theList);
			MyInvalWindowRect(theWindow,&listRect);
		}
		
		PenNormal();
		return false;
	}
	else
	{
		EventRecord	followEvent;
		
		/* ダブルクリック判定 */
		ticks=TickCount();
		while (TickCount()-ticks<GetDblTime() && PtInRect(pt,&dRect))
		{
			GetMouse(&pt);
			if (EventAvail(mDownMask,&followEvent)) return true;
		}
		return false;
	}
}

#define	kEditCmdDialogID	146
enum {
	kDotCmdInputBoxIndex=3,
	kDotCmdDisplayBoxIndex=4
};

/* 描画点コマンドの編集、追加 */
short DoEditDotCommand(Str255 command,ListHandle theList)
{
	Str255	newCommand;
	DialogPtr	theDialog;
	short		item;
	Str255		tempStr;
	GrafPtr	port;
	Str255		fontName;
	short		fNum;
	Rect		box;
	ModalFilterUPP	mfUPP=NewModalFilterUPP(EditCmdDialogFilter);
	WindowPtr	theWindow=GetWindowFromPort(GetListPort(theList));
	
	GetPort(&port);
	{
		LActivate(false,theList);
		DeactivateWindowControl(theWindow);
		MyInvalWindowPortBounds(theWindow);
	}
	PStrCpy(command,newCommand);
	theDialog=GetNewDialog(kEditCmdDialogID,nil,kFirstWindowOfClass);
	SetDialogDefaultItem(theDialog,ok);
	SetDialogCancelItem(theDialog,cancel);
	
	/* コマンド */
	SetDialogItemText2(theDialog,kDotCmdInputBoxIndex,newCommand);
	SelectDialogItemText(theDialog,kDotCmdInputBoxIndex,0,newCommand[0]);
	SetPortDialogPort(theDialog);
	GetIndString(fontName,150,1);
	#if TARGET_API_MAC_CARBON
	fNum=FMGetFontFamilyFromName(fontName);
	#else
	GetFNum(fontName,&fNum);
	#endif
	TextFont(fNum);
	TextSize(12);
	GetDialogItemRect(theDialog,kDotCmdDisplayBoxIndex,&box);
	TETextBox(&newCommand[1],newCommand[0],&box,teJustLeft);
	TextFont(0);
	
	/* タイトル */
	GetIndString(tempStr,157,(newCommand[0] ? 2 : 1));
	SetWTitle(GetDialogWindow(theDialog),tempStr);
	ShowWindow(GetDialogWindow(theDialog));
	
	item=3;
	while (item != ok && item != cancel)
	{
		ModalDialog(mfUPP,&item);
		
		switch (item)
		{
			case kDotCmdInputBoxIndex:
				GetDialogItemText2(theDialog,kDotCmdInputBoxIndex,tempStr);
				if (!EqualString(tempStr,newCommand,true,true))
				{
					if (CheckCommandStr(tempStr)==noErr)
					{
						PStrCpy(tempStr,newCommand);
						MyInvalWindowRect(GetDialogWindow(theDialog),&box);
					}
					else
					{
						SysBeep(0);
						PStrCpy(newCommand,tempStr);
						SetDialogItemText2(theDialog,kDotCmdInputBoxIndex,tempStr);
					}
				}
				break;
			
			case ok:
				if (!EqualString(newCommand,command,true,true)) /* 変更されている */
				{
					Cell	theCell={0,0};
					
					if (LSearch(&newCommand[1],newCommand[0],nil,&theCell,theList)) /* 同じものがある */
					{
						SysBeep(0);
						item = 3;
					}
					else PStrCpy(newCommand,command);
				}
				else item = cancel;
				break;
		}
	}
	
	DisposeDialog(theDialog);
	DisposeModalFilterUPP(mfUPP);
	{
		LActivate(true,theList);
		ActivateWindowControl(theWindow);
		MyInvalWindowPortBounds(theWindow);
	}
	
	SetPort(port);
	return item;
}

/* コマンド編集ダイアログのフィルタ */
pascal Boolean EditCmdDialogFilter(DialogPtr theDialog,EventRecord *theEvent,short *theItemHit)
{
	Boolean		eventHandled=false;
	WindowPtr	theWindow;
	short		part;
	char		key;
	Rect		box;
	
	switch (theEvent->what)
	{
		case updateEvt:
			theWindow=(WindowPtr)theEvent->message;
			if (theWindow == GetDialogWindow(theDialog))
			{
				Str255	dotCmd,fontName;
				short	fNum;
				GrafPtr	port;
				
				GetPort(&port);
				SetPortDialogPort(theDialog);
				BeginUpdate(GetDialogWindow(theDialog));
				
				DrawDialog(theDialog);
				
				GetDialogItemText2(theDialog,kDotCmdInputBoxIndex,dotCmd);
//				SetPort(theDialog);
				GetIndString(fontName,150,1);
				#if TARGET_API_MAC_CARBON
				fNum=FMGetFontFamilyFromName(fontName);
				#else
				GetFNum(fontName,&fNum);
				#endif
				TextFont(fNum);
				TextSize(12);
				GetDialogItemRect(theDialog,kDotCmdDisplayBoxIndex,&box);
				TETextBox(&dotCmd[1],dotCmd[0],&box,teJustLeft);
				TextFont(0);
				EndUpdate(GetDialogWindow(theDialog));
				SetPort(port);
			}
			else if (theWindow == GetNextWindow(GetDialogWindow(theDialog)))
				eventHandled=EditLibDialogFilter(GetDialogFromWindow(theWindow),theEvent,theItemHit);
			else if (theWindow!=nil && GetWindowKind(theWindow)!=kDialogWindowKind)
				DoUpdate(theEvent);
			break;
		
		case keyDown:
		case autoKey:
			key=theEvent->message & charCodeMask;
			if ((theEvent->modifiers & cmdKey)!=0)
				if (key==kPeriod)
					key=kEscapeKey;
			
			switch (key)
			{
				case kReturnKey:
				case kEnterKey:
					*theItemHit=ok;
					HiliteButton(theDialog,ok);
					eventHandled=true;
					break;
				
				case kEscapeKey:
					*theItemHit=cancel;
					HiliteButton(theDialog,cancel);
					eventHandled=true;
					break;
			}
			break;
		
		case mouseDown:
			part=FindWindow(theEvent->where,&theWindow);
			if (theWindow==GetDialogWindow(theDialog))
			{
				if (part==inDrag)
				{
					Rect	myScreenRect;
					
					GetRegionBounds(GetGrayRgn(),&myScreenRect);
					DragWindow(theWindow,theEvent->where,&myScreenRect);
					eventHandled=true;
				}
			}
			break;
	}
	
	return eventHandled;
}

/* コマンド文字列のチェック */
OSErr CheckCommandStr(Str255 command)
{
	short	i;
	
	if (command[0] > 31) return paramErr; /* 31文字以内 */
	
	for (i=1; i<= command[0]; i++)
	{
		switch (command[i])
		{
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case 'q':
			case 'w':
			case 'e':
			case 'a':
			case 'd':
			case 'z':
			case 'x':
			case 'c':
				break;
			
			default:
				return paramErr;
		}
	}
	return noErr;
}

#define kDeleteCmdDialogID	147

/* 描画点コマンドの編集、追加 */
OSErr DoDeleteDotCommand(Str255 command,ListHandle theList)
{
	GrafPtr		port;
	ModalFilterUPP	mfUPP=NewModalFilterUPP(DeleteCmdDialogFilter);
	WindowPtr	theWindow=GetWindowFromPort(GetListPort(theList));
	Str255		prompt;
	DialogPtr	theDialog;
	short		item;
	
	GetPort(&port);
	{
		LActivate(false,theList);
		DeactivateWindowControl(theWindow);
		MyInvalWindowPortBounds(theWindow);
	}
	
	GetIndString(prompt,150,4);
	ReplaceString(prompt,command,"\p^0");
	
	ParamText(prompt,"\p","\p","\p");
	theDialog=GetNewDialog(150,nil,kFirstWindowOfClass);
	SetDialogDefaultItem(theDialog,ok);
	SetDialogCancelItem(theDialog,cancel);
	
	ShowWindow(GetDialogWindow(theDialog));
	
	item=3;
	while (item != ok && item != cancel)
		ModalDialog(mfUPP,&item);
	
	DisposeDialog(theDialog);
	DisposeModalFilterUPP(mfUPP);
	
	{
		LActivate(true,theList);
		ActivateWindowControl(theWindow);
		MyInvalWindowPortBounds(theWindow);
	}
	
	SetPort(port);
	return (item == ok ? noErr : userCanceledErr);
}

/* コマンド削除ダイアログのフィルタ */
pascal Boolean DeleteCmdDialogFilter(DialogPtr theDialog,EventRecord *theEvent,short *theItemHit)
{
	Boolean		eventHandled=false;
	WindowPtr	theWindow;
	short		part;
	char		key;
	
	switch (theEvent->what)
	{
		case updateEvt:
			theWindow=(WindowPtr)theEvent->message;
			if (theWindow == GetNextWindow(GetDialogWindow(theDialog)))
				eventHandled=EditLibDialogFilter(GetDialogFromWindow(theWindow),theEvent,theItemHit);
			else if (GetWindowKind(theWindow)!=kDialogWindowKind && theWindow!=nil)
				DoUpdate(theEvent);
			break;
		
		case keyDown:
		case autoKey:
			key=theEvent->message & charCodeMask;
			if ((theEvent->modifiers & cmdKey)!=0)
				if (key==kPeriod)
					key=kEscapeKey;
			
			switch (key)
			{
				case kReturnKey:
				case kEnterKey:
					*theItemHit=ok;
					HiliteButton(theDialog,ok);
					eventHandled=true;
					break;
				
				case kEscapeKey:
					*theItemHit=cancel;
					HiliteButton(theDialog,cancel);
					eventHandled=true;
					break;
			}
			break;
		
		case mouseDown:
			part=FindWindow(theEvent->where,&theWindow);
			if (theWindow==GetDialogWindow(theDialog))
			{
				if (part==inDrag)
				{
					Rect	myScreenRect;
					
					GetRegionBounds(GetGrayRgn(),&myScreenRect);
					DragWindow(theWindow,theEvent->where,&myScreenRect);
					eventHandled=true;
				}
			}
			break;
	}
	
	return eventHandled;
}

/* ドットフォントでポップアップメニューを表示 */
long PopUpMenuSelectDotFont(MenuHandle menu,short v,short h,short item)
{
	Str255	fontName;
	short	fontID;
	#if !TARGET_API_MAC_CARBON
	short	sysFontSize;
	short	sysFontID;
	#endif
	long	result;
	
	GetIndString(fontName,150,1);
	#if TARGET_API_MAC_CARBON
	fontID=FMGetFontFamilyFromName(fontName);
	#else
	GetFNum(fontName,&fontID);
	#endif
	
	#if !TARGET_API_MAC_CARBON
	#if TARGET_RT_MAC_CFM
	if (gSystemVersion < 0x0850)
	#endif
	{
		/* システムフォントを変更する */
		sysFontSize=LMGetSysFontSize();
		sysFontID=LMGetSysFontFam();
		LMSetSysFontSize(12);
		LMSetSysFontFam(fontID);
		LMSetLastSPExtra(-1);
	}
	#if TARGET_RT_MAC_CFM
	else
	#endif /* TARGET_RT_MAC_CFM */
	#endif /* !TARGET_API_MAC_CARBON */
	#if TARGET_API_MAC_CARBON || TARGET_RT_MAC_CFM
		SetMenuFont(menu,fontID,12);
	#endif
	
	/* ポップアップメニューを表示する */
	result=PopUpMenuSelect(menu,v,h,item);
	
	/* システムフォントを元に戻す */
	#if !TARGET_API_MAC_CARBON
	#if TARGET_RT_MAC_CFM
	if (gSystemVersion < 0x0850)
	#endif
	{
		LMSetSysFontSize(sysFontSize);
		LMSetSysFontFam(sysFontID);
		LMSetLastSPExtra(-1);
	}
	#endif
	
	return result;
}

const static Rect	gDotLibCmdPopRect={0x12,0x54,0x1e,0x61};

/* 描画点パレットの再描画 */
void UpdateDotModePalette(void)
{
	WindowPtr	theWindow;
	PaintWinRec	*eWinRec;
	PicHandle	backPic;
    Rect        picRect;
	
	theWindow=MyFrontNonFloatingWindow();
	if (theWindow==nil || GetExtWindowKind(theWindow)!=kWindowTypePaintWindow)
	{
		HideReferencedWindow(DotModePalette);
		return;
	}
	
	eWinRec=GetPaintWinRec(theWindow);
	if (!eWinRec->isDotMode)
	{
		HideReferencedWindow(DotModePalette);
		return;
	}
	
	backPic=GetPicture(148);
    QDGetPictureBounds(backPic, &picRect);
	DrawPicture(backPic,&picRect);
	
	if (CountCmdNum()<=0) /* データがない */
	{
		PicHandle	nPic;
		
		nPic=GetPicture(133);
		DrawPicture(nPic,&gDotLibCmdPopRect);
	}
	
	UpdateSelectedLib(gDotLibName);
	UpdateRecordedCommand(gDotCommand);
	UpdateInputCommand(eWinRec->dotCommand);
}

/* コマンド数をチェックする */
short CountCmdNum(void)
{
	Handle	hand;
	short	cmdNum;
	
	if (gDotLibRefNum <= 0) return -1;
	
	UseResFile(gDotLibRefNum);
	hand=Get1Resource('STR#',128);
	if (hand==nil)
	{
		UseResFile(gApplRefNum);
		return -1;
	}
	if (GetHandleSize(hand)<sizeof(short))
	{
		ReleaseResource(hand);
		UseResFile(gApplRefNum);
		return -1;
	}
	cmdNum=**(short **)hand;
	ReleaseResource(hand);
	UseResFile(gApplRefNum);
	
	return cmdNum;
}

/* コマンドポップアップをアップデートさせる */
void UpdateDotCmdPop(void)
{
	GrafPtr	port;
	
	GetPort(&port);
	SetPortWindowPort(DotModePalette);
	MyInvalWindowRect(DotModePalette,&gDotLibCmdPopRect);
	SetPort(port);
}

/* ライブラリ名をアップデートさせる */
void UpdateSelectedLib(Str31 libName)
{
	Str31	temp;
	Rect	r;
	RgnHandle	clipRgn=NewRgn();
	
	GetClip(clipRgn);
	
	TextMode(srcCopy);
	TextFont(applFont);
	TextSize(9);
	
	SetRect(&r,0x05,0x03,0x4f,0x0E);
	ClipRect(&r);
	PStrCpy(libName,temp);
	TruncString(0x4f-0x05,temp,truncEnd);
	OffsetRect(&r,0,-1);
	TETextBox(&temp[1],temp[0],&r,teJustLeft);
	
	SetClip(clipRgn);
	DisposeRgn(clipRgn);
}

/* 記録コマンドをアップデートさせる */
void UpdateRecordedCommand(Str31 command)
{
	Str255	fontName;
	short	fNum;
	Rect	r;
	Str31	temp;
	
	GetIndString(fontName,150,1);
	#if TARGET_API_MAC_CARBON
	fNum=FMGetFontFamilyFromName(fontName);
	#else
	GetFNum(fontName,&fNum);
	#endif
	
	TextMode(srcCopy);
	TextFont(fNum);
	TextSize(12);
	
	SetRect(&r,0x05,19,0x4f,0x1b);
	PStrCpy(command,temp);
	TruncString(0x4f-0x05,temp,truncEnd);
	TETextBox(&temp[1],temp[0],&r,teJustLeft);
}

/* 入力中コマンドだけをアップデートさせる */
void UpdateInputCommand(Str31 command)
{
	Str255	fontName;
	short	fNum;
	Rect	r;
	
	GetIndString(fontName,150,1);
	#if TARGET_API_MAC_CARBON
	fNum=FMGetFontFamilyFromName(fontName);
	#else
	GetFNum(fontName,&fNum);
	#endif
	
	TextMode(srcCopy);
	TextFont(fNum);
	TextSize(12);
	
	SetRect(&r,0x05,0x22,0x60,0x3f);
	TETextBox(&command[1],command[0],&r,teJustLeft);
}


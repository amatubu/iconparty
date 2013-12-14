/* ------------------------------------------------------------ */
/*  IconListWindow.c                                            */
/*     アイコンリストウィンドウ処理                             */
/*                                                              */
/*                 2001.9.25 - 2001.9.25  naoki iimura        	*/
/* ------------------------------------------------------------ */

/* includes */
#ifdef __APPLE_CC__
#include	<Carbon/Carbon.h>
#else
#include	<FinderRegistry.h>
#endif

#ifdef __APPLE_CC__
#include	"MoreFilesX.h"
#else
#include	"MoreFilesExtras.h"
#endif

#include	"Definition.h"
#include	"IconRoutines.h"
#include	"WindowRoutines.h"
#include	"FileRoutines.h"
#include	"MenuRoutines.h"
#include	"UsefulRoutines.h"
#include	"NavigationServicesSupport.h"
#include	"PreCarbonSupport.h"
#include	"CustomSaveDialog.h"
#include	"WinIconSupport.h"
#include	"IconFamilyWindow.h"
#include	"IconListWindow.h"
#include	"IPIconSupport.h"


/* アイコンリストウィンドウ関係 */
static WindowPtr	OpenIconWindow(FSSpec *spec);
static short	CountIcons(FSSpec *theFile);
#if !TARGET_API_MAC_CARBON
static pascal void	ActivateIconWindow(WindowPtr iconWindow,Boolean activateWindow);
#endif

static ListHandle	InitIconList(WindowPtr iconWindow,short refNum);
static void	SortIconList(ListHandle iconList);

static void	MyDrawGrowIcon(WindowPtr theWindow);
static Boolean	LClick2(Point mousePt,short modifier,ListHandle theList,Rect *box,EventRecord *theEvent);

static pascal Boolean	MyDispIconFilter(DialogPtr theDialog,EventRecord *theEvent,short *theItemHit);

/* ResEditに開いてもらう処理 */
static void	AEOpenFileWithResEdit(FSSpec *theIconFile);
#if 0
static void	AEOpenFileInFinderWithResEdit(FSSpec *theIconFile);
#endif

/* データ検索 */
static short	MyUniqueID(ListHandle iconList);
static short	MyContinuousUniqID(ListHandle iconList,short num);
static pascal short	MyIDSearch(Ptr cellDataPtr,Ptr searchDataPtr,short cellDataLen,short searchDataLen);

static void	AddFileIconToIconList(WindowPtr iconWindow,FSSpec *theFile,short id,Boolean redrawFlag);
static void	AddIPIconToList(WindowPtr iconWindow,const IPIconRec *ipIcon,ResType iconType,
	Str255 iconName,short id,Boolean redrawFlag);

/* コンテクストメニュー */
static void	MyIconListContextMenu(Point globPt,WindowPtr iconWindow);
static void	MyIconListContextMenu2(Point globPt,WindowPtr iconWindow);

static void	UpdateIconList(IconListWinRec *iWinRec,short command,short idNum,IconListDataRec *iconList);
static void	AddIconToIconList(IconListWinRec *iWinRec,IconListDataRec *newIcon,Boolean redrawFlag);

/* スクロール */
static void	LScrollToCell(Cell theCell,ListHandle iconList);

/* drag & drop */
static pascal short	MyIconListTrackingHandler(DragTrackingMessage theMessage,WindowPtr theWindow,
											void *handlerRefCon,DragReference theDrag);
static Boolean	IsMyIconListTypeAvailable(DragReference theDrag);
static pascal short	MyIconListReceiveHandler(WindowPtr theWindow,void *handlerRefCon,
											DragReference theDrag);

static void	MyDoStartDragIcon(WindowPtr iconWindow,EventRecord *theEvent);
static OSErr	MyDoAddIconFlavors(WindowPtr theWindow,DragReference theDrag);
static OSErr	MyGetDragIconRegion(WindowPtr theWindow,RgnHandle dragRegion,
							DragReference theDragRef);
static pascal short	MySendIconDataProc(FlavorType tyeType,void *dragSendRefCon,
									ItemReference theItem,DragReference theDrag);

static OSErr	AddDragItemFlavorTypePromiseHFS(DragReference dragRef, ItemReference itemRef,
										OSType fileType, OSType fileCreator,
										UInt16 fdFlags, FlavorType promisedFlavor);


#if !TARGET_API_MAC_CARBON
/* -> StdGetFolder.c */
extern void StandardGetFolder(const short numTypes, ConstSFTypeListPtr typeList, StandardFileReply *reply);
#endif

#if TARGET_API_MAC_CARBON
/* -> IconList_LDEF.c */
extern pascal void MyIconListLDEFProc(short message,Boolean selected,Rect *cellRect,Cell theCell,
									short dataOffset,short dataLen,ListHandle theList);
#endif


#define	ICONERR_RESID	4008
#define	ICONERR1	1
#define	ICONERR2	2
#define	ICONERR3	3
#define	ICONERR4	4
#define	ICONERR5	5
#define	ICONERR6	6


/* アイコンファイルからの読み込み */
void LoadFromIconFile(FSSpec *spec)
{
	WindowPtr	theWindow;
	IconListWinRec	*iWinRec;
	IconFamilyWinRec	*fWinRec;
	short		iconNum;
	Str255		str;
	
	/* 同じファイルがすでに開かれていないかどうかをチェック */
	theWindow=MyFrontNonFloatingWindow();
	while (theWindow!=nil)
	{
		if (GetExtWindowKind(theWindow)==kWindowTypeIconListWindow)
		{
			iWinRec=GetIconListRec(theWindow);
			if (EqualFile(&iWinRec->iconFileSpec,spec)) break;
		}
		else if (GetExtWindowKind(theWindow)==kWindowTypeIconFamilyWindow)
		{
			fWinRec=GetIconFamilyRec(theWindow);
			if (fWinRec->parentWindow == nil && EqualFile(&fWinRec->theIconSpec,spec)) break;
		}
			
		theWindow=GetNextVisibleWindow(theWindow);
	}
	if (theWindow != nil)
	{
		SelectReferencedWindow(theWindow);
		UpdateMenus();
		return;
	}
	
	if ((iconNum=CountIcons(spec)) > 0)
	{
		theWindow=OpenIconWindow(spec);
		
		if (theWindow!=nil)
		{
			FSpGetFileName(spec,str);
			SetWTitle(theWindow,str);
		}
	}
	else if (iconNum == 0)
	{
		/* カスタムアイコンを開く */
		OpenFolderIcon(spec,false);
	}
}

/* アイコンの数をカウント */
short CountIcons(FSSpec *theFile)
{
	short	refNum;
	OSErr	err;
	short	result1,result2,result3;
	Handle	theHandle;
	ResType	theType;
	short	id;
	Str255	resName;
	
	refNum=FSpOpenResFile(theFile,fsRdWrPerm);
	if (refNum <= 0)
	{
		if ((err=ResError())==eofErr)
		{
			/* リソースフォークがない */
			return noErr;
		}
		else
		{
			ErrorAlertFromResource(ICONERR_RESID,ICONERR1);
			return err;
		}
	}
	UseResFile(refNum); /* 実際には必要ないのだが */
	
	/* 大アイコン */
	result1=Count1Resources(kLarge1BitMask);
	if (result1 == 1)
	{
		/* カスタムアイコンか？ */
		theHandle=Get1IndResource(kLarge1BitMask,1);
		if (theHandle == nil) result1=0;
		else
		{
			GetResInfo(theHandle,&id,&theType,resName);
			if (id == kCustomIconResource) result1=0;
			ReleaseResource(theHandle);
		}
	}
	
	/* 小アイコン */
	result2=Count1Resources(kSmall1BitMask);
	if (result2 == 1)
	{
		/* カスタムアイコンか？ */
		theHandle=Get1IndResource(kSmall1BitMask,1);
		if (theHandle == nil) result2 = 0;
		else
		{
			GetResInfo(theHandle,&id,&theType,resName);
			if (id == kCustomIconResource) result2 = 0;
			ReleaseResource(theHandle);
		}
	}
	
	if (isIconServicesAvailable)
	{
		/* アイコンファミリ */
		result3 = Count1Resources(kIconFamilyType);
		if (result3 == 1)
		{
			/* カスタムアイコンか？ */
			theHandle = Get1IndResource(kIconFamilyType,1);
			if (theHandle == nil) result3 = 0;
			else
			{
				GetResInfo(theHandle,&id,&theType,resName);
				if (id == kCustomIconResource) result3 = 0;
				ReleaseResource(theHandle);
			}
		}
	}
	else result3 = 0;
	
	CloseResFile(refNum);
	
	return result1+result2+result3;
}

/* アイコンの保存 */
OSErr SaveIconFile(WindowPtr theWindow)
{
	short	num;
	short	i;
	IconActionUPP	delIconUPP=NewIconActionUPP(DeleteIcon);
	OSErr	err;
	IconListWinRec *iWinRec=GetIconListRec(theWindow);
	
	/* 保存されていなければファイルを作成 */
	if (!iWinRec->wasSaved)
	{
		err=MakeIconFile(iWinRec);
		if (err!=noErr) return err;
		
		SetWTitle(theWindow,iWinRec->iconFileSpec.name);
	}
	
	/* 削除情報リソースを元に削除 */
	UseResFile(iWinRec->tempRefNum);
	num=Count1Resources(kDeleteIconInfoType);
	for (i=1; i<=num; i++)
	{
		Handle	resHandle;
		short	id;
		ResType	theType;
		Str255	name;
		IconSuiteRef	iconSuite;
		
		UseResFile(iWinRec->tempRefNum);
		resHandle=Get1IndResource(kDeleteIconInfoType,i);
		GetResInfo(resHandle,&id,&theType,name);
		ReleaseResource(resHandle);
		
		UseResFile(iWinRec->refNum);
		err=Get1IconSuite(&iconSuite,id,kSelectorMyData);
		err=ForEachIconDo(iconSuite,kSelectorMyData,delIconUPP,nil);
		
		resHandle=Get1Resource(kIconFamilyType,id);
		RemoveResource(resHandle);
	}
	num=Count1Resources(kUpdateIconInfoType);
	for (i=1; i<=num; i++)
	{
		Handle	resHandle;
		short	id;
		ResType	theType;
		Str255	name;
		IconSuiteRef	iconSuite;
		
		UseResFile(iWinRec->tempRefNum);
		resHandle=Get1IndResource(kUpdateIconInfoType,i);
		GetResInfo(resHandle,&id,&theType,name);
		ReleaseResource(resHandle);
		
		UseResFile(iWinRec->refNum);
		err=Get1IconSuite(&iconSuite,id,kSelectorMyData);
		err=ForEachIconDo(iconSuite,kSelectorMyData,delIconUPP,nil);
		
		resHandle=Get1Resource(kIconFamilyType,id);
		RemoveResource(resHandle);
	}
	DisposeIconActionUPP(delIconUPP);
	
	/* テンポラリファイルのアイコンを保存する */
	CopyIcons(iWinRec->tempRefNum,iWinRec->refNum);
	
	UseResFile(gApplRefNum);
	UpdateResFile(iWinRec->refNum);
	
	/* テンポラリファイルを作りなおす */
	CloseResFile(iWinRec->tempRefNum);
	FSpDelete(&iWinRec->tempFileSpec);
	FSpCreateResFile(&iWinRec->tempFileSpec,kIconPartyCreator,kTemporaryFileType,smSystemScript);
	iWinRec->tempRefNum=FSpOpenResFile(&iWinRec->tempFileSpec,fsRdWrPerm);
	
	UseResFile(gApplRefNum);
	
	iWinRec->wasChanged=false;
	iWinRec->undoData.undoMode=umCannot; /* 保存後の取り消しは不可とする */
	UpdateUndoMenu();
	UpdateSaveMenu();
	
	return noErr;
}

/* アイコンファイル（リソースファイル）を閉じる */
OSErr CloseIconFile(WindowPtr theWindow,Boolean quitFlag)
{
	short	refNum;
	IconListWinRec	**iWinRec;
	OSErr	err=noErr;
	WindowPtr	fWindow,nextWin;
	
	iWinRec=(IconListWinRec **)GetExtWRefCon(theWindow);
	
	/* このウィンドウを親に持つファミリウィンドウが存在しないかどうかをチェック */
	fWindow=GetNextVisibleWindow(theWindow);
	while (fWindow != nil && err == noErr)
	{
		nextWin=GetNextVisibleWindow(fWindow);
		if (GetExtWindowKind(fWindow)==kWindowTypeIconFamilyWindow)
		{
			IconFamilyWinRec	*fWinRec=GetIconFamilyRec(fWindow);
			
			if (fWinRec->parentWindow==theWindow)
				err=CloseFamilyWindow(fWindow,true,false);
		}
		fWindow=nextWin;
	}
	if (err!=noErr) return err;
	
	if ((**iWinRec).wasChanged)
	{
		Str255	docuKind;
		short	item;
		
		if (isNavServicesAvailable && useNavigationServices)
		{
			/* フローティングウィンドウがあるとNavigation Servicesがフロントウィンドウを
			  誤認してしまう問題に対処。あまりきれいな方法ではないが、いちばんましかな */
			SuspendFloatingWindows();
			item=AskSaveWithNav((**iWinRec).iconFileSpec.name,
				(quitFlag ? kNavSaveChangesQuittingApplication : kNavSaveChangesClosingDocument));
			ResumeFloatingWindows();
		}
		else
		{
			GetIndString(docuKind,133,4);
			item=SaveYN(docuKind,(**iWinRec).iconFileSpec.name,kSaveChangedCloseWindow);
		}
		switch (item)
		{
			case cancel:
				return userCanceledErr;
			
			case ok:
				err=SaveIconFile(theWindow);
				if (err==userCanceledErr) return err;
				break;
			
			case kDialogItemDontSave:
				break;
		}
	}
	
	/* 保存されていればファイルを閉じる */
	if ((**iWinRec).wasSaved)
	{
		refNum=(**iWinRec).refNum;
		CloseResFile(refNum);
	}
	
	/* テンポラリファイルも閉じ、消去する */
	CloseResFile((**iWinRec).tempRefNum);
	err=FSpDelete(&(**iWinRec).tempFileSpec);
	
	/* 取り消し用のIDリストも破棄 */
	DisposePtr((Ptr)(**iWinRec).undoData.iconList);
	
	/* ドラッグ関連のルーチンの破棄 */
	if (isDragMgrPresent)
	{
		err=RemoveTrackingHandler((**iWinRec).dragHandlers.trackUPP,theWindow);
		DisposeDragTrackingHandlerUPP((**iWinRec).dragHandlers.trackUPP);
		err=RemoveReceiveHandler((**iWinRec).dragHandlers.receiveUPP,theWindow);
		DisposeDragReceiveHandlerUPP((**iWinRec).dragHandlers.receiveUPP);
	}
	#if !TARGET_API_MAC_CARBON
	DisposeRoutineDescriptor((**iWinRec).dragHandlers.activateUPP);
	#endif
	
	LDispose((**iWinRec).iconList);
	HUnlock((Handle)iWinRec);
	DisposeHandle((Handle)iWinRec);
	
	DisposeWindowReference(theWindow);
	
	UpdateMenus();
	
	return noErr;
}

/* アイコンファイルの復帰 */
void DoRevertIconFile(WindowPtr theWindow)
{
	IconListWinRec	*iWinRec=GetIconListRec(theWindow);
	short		item;
	Str255		docKind;
	FSSpec		theFile;
	OSErr		err;
	FInfo		fInfo;
	
	/* まず、確認 */
	GetIndString(docKind,133,4);
	item=RevertYN(docKind,iWinRec->iconFileSpec.name);
	if (item!=ok) return;
	
	/* ファイルを閉じて開き直す */
	theFile=iWinRec->iconFileSpec;
	err=FSpGetFInfo(&theFile,&fInfo);
	if (err!=noErr)
	{
		SysBeep(0);
		return;
	}
	
	iWinRec->wasChanged=false;
	err=CloseIconFile(theWindow,false);
	if (err!=noErr)
	{
		iWinRec->wasChanged=true;
		return;
	}
	
	LoadFromIconFile(&theFile);
}

/* アイコンリストウィンドウを作成 */
/* refNum : リソースファイルのリファレンスナンバ */
WindowPtr MakeIconWindow(short refNum)
{
	OSErr		err;
	WindowPtr	iconWindow;
	IconListWinRec	**iWinRec;
	#if !TARGET_API_MAC_CARBON
	ActivateHandlerUPP	activateIconWindowUPP;
	#endif
	Point		windPos=kDefaultWinPos;
	Point		windOffset=kWinOffset;
	Rect		validRect,tempRect;
	
	GetRegionBounds(GetGrayRgn(),&validRect);
	UseResFile(gApplRefNum);
	
	/* ウィンドウのデータをおさめる領域を確保 */
	iWinRec=(IconListWinRec **)NewHandle(sizeof(IconListWinRec));
	if (MemError()) return nil;
	HLockHi((Handle)iWinRec);
	
	/* ウィンドウを作成 */
	err=GetNewWindowReference(&iconWindow,kIconListWindowResID,kFirstWindowOfClass,nil);
	if (iconWindow==nil || err!=noErr)
	{
		HUnlock((Handle)iWinRec);
		DisposeHandle((Handle)iWinRec);
		SysBeep(0);
		return nil;
	}
	SetExtWindowKind(iconWindow,kWindowTypeIconListWindow);
	SetExtWRefCon(iconWindow,(long)iWinRec);
	
	/* ウィンドウの移動 */
	SetPortWindowPort(iconWindow);
	validRect.left=windPos.h;
	validRect.top=windPos.v;
	GetWindowPortBounds(iconWindow,&tempRect);
	validRect.right-=tempRect.right;
	validRect.bottom-=tempRect.bottom;
	StackWindowPos(&windPos,windOffset,&validRect);
	MoveWindow(iconWindow,windPos.h,windPos.v,false);
	
	/* アクティベートルーチンの設定 */
	#if !TARGET_API_MAC_CARBON
	activateIconWindowUPP=NewActivateHandlerProc((ProcPtr)ActivateIconWindow);
	SetActivateHandlerProc(iconWindow,activateIconWindowUPP);
	(**iWinRec).dragHandlers.activateUPP=activateIconWindowUPP;
	#endif
	
	/* ドラッグ受け入れルーチンの設定 */
	if (isDragMgrPresent)
	{
		DragTrackingHandlerUPP	dragTrackUPP;
		DragReceiveHandlerUPP	dragReceiveUPP;
		
		dragTrackUPP=NewDragTrackingHandlerUPP(MyIconListTrackingHandler);
		if (dragTrackUPP != nil)
		{
			MyDragGlobalsRec	*dragGlobals=&(**iWinRec).dragHandlers.dragGlobals;
			
			dragGlobals->canAcceptDrag=false;
			dragGlobals->isInContent=false;
			
			err=InstallTrackingHandler(dragTrackUPP,iconWindow,dragGlobals);
			(**iWinRec).dragHandlers.trackUPP=dragTrackUPP;
			dragReceiveUPP=NewDragReceiveHandlerUPP(MyIconListReceiveHandler);
			if (dragReceiveUPP != nil)
			{
				err=InstallReceiveHandler(dragReceiveUPP,iconWindow,dragGlobals);
				(**iWinRec).dragHandlers.receiveUPP=dragReceiveUPP;
			}
		}
	}
	
	/* テンポラリファイルを作る */
	err=MakeTempFile(&(*iWinRec)->tempFileSpec,true);
	if (err==noErr)
	{
		(**iWinRec).tempRefNum=FSpOpenResFile(&(**iWinRec).tempFileSpec,fsRdWrPerm);
	}
	UseResFile(gApplRefNum);
	
	/* 各種データを初期化 */
	(**iWinRec).iconList=InitIconList(iconWindow,refNum);
	(**iWinRec).wasChanged=false;
	
	(**iWinRec).undoData.undoMode=umCannot;
	(**iWinRec).undoData.iconNum=0;
	(**iWinRec).undoData.iconList=NULL;
	
	((IconListRec *)(**(**iWinRec).iconList).refCon)->tempRefNum=(**iWinRec).tempRefNum;
	
	return iconWindow;
}

/* アイコンファイルのウィンドウを開く */
WindowPtr OpenIconWindow(FSSpec *spec)
{
	WindowPtr	iconWindow;
	IconListWinRec	*iWinRec;
	short		refNum;
	
	refNum=FSpOpenResFile(spec,fsRdWrPerm);
	if (refNum<=0) return nil;
	
	iconWindow=MakeIconWindow(refNum);
	if (iconWindow==nil)
	{
		CloseResFile(refNum);
		return nil;
	}
	
	/* データの初期化 */
	iWinRec=GetIconListRec(iconWindow);
	iWinRec->refNum=refNum;
	iWinRec->iconFileSpec=*spec;
	iWinRec->wasSaved=true;
	
	ShowReferencedWindow(iconWindow);
	
	return iconWindow;
}

#if !TARGET_API_MAC_CARBON
/* アイコンウィンドウのアクティベートルーチン */
pascal void	ActivateIconWindow(WindowPtr iconWindow,Boolean activateWindow)
{
	IconListWinRec	*iconRec;
	
	SetPortWindowPort(iconWindow);
	
	iconRec=GetIconListRec(iconWindow);
	
	LActivate(activateWindow, iconRec->iconList);
	MyDrawGrowIcon(iconWindow);
}
#endif

/* アイコンリストの初期化 */
/* refNum=0ならば、アイコンなし */
ListHandle InitIconList(WindowPtr iconWindow,short refNum)
{
	short	width,height;
	short	i,j,k;
	Handle	resHandle;
	short	id;
	Str255	name;
	ResType	resType;
	Point	cell;
	ListHandle	iconList;
	IconListRec	*iconListPtr;
	IconListDataRec	*iconDataPtr;
	
	Rect	iconView;
	Rect	dataBounds={0,0,0,1};		/* リストの大きさ */
	Point	cellSize={kIconListHeight,kIconListWidth};
	Rect	tempRect;
	
	/* アイコン表示用のリストを作成 */
	GetWindowPortBounds(iconWindow,&iconView);
	iconView.right-=kScrollBarWidth;	/* スクロールバーの分だけ縮小する */
	
	width=(iconView.right-iconView.left)/kIconListWidth;
	dataBounds.right=width;
	
	#if TARGET_API_MAC_CARBON
	{
		ListDefSpec	defSpec;
		OSErr		err;
		
		defSpec.defType=kListDefUserProcType;
		defSpec.u.userProc=NewListDefUPP(MyIconListLDEFProc);
		err=CreateCustomList(&iconView,&dataBounds,cellSize,&defSpec,iconWindow,false,true,false,true,&iconList);
	}
	#else
	iconList=LNew(&iconView,&dataBounds,cellSize,128,iconWindow,false,true,false,true);
	#endif
	SetListSelectionFlags(iconList,lNoNilHilite); /* 空セルをハイライトしない */
	GetControlBounds(GetListVerticalScrollBar(iconList),&tempRect);
	tempRect.bottom-=kScrollBarWidth;
	SetControlBounds(GetListVerticalScrollBar(iconList),&tempRect);
	
	/* リスト表示に使うデータを設定 */
	iconListPtr=(IconListRec *)NewPtr(sizeof(IconListRec));
	iconListPtr->refNum=refNum;
	iconListPtr->gApplRefNum=gApplRefNum;
	iconListPtr->background=&gListBackground;
	
	iconListPtr->isIconServicesAvailable=isIconServicesAvailable;
	
	SetListRefCon(iconList,(long)iconListPtr);
	
	if (refNum > 0)
	{
		short	iconNum,iconNum2,iconNum3=0;
		short	*idList;
		ResType	*typeList=NULL;
		Boolean	tempBool;
		
		UseResFile(refNum);
		iconNum=Count1Resources(kLarge1BitMask);
		iconNum2=Count1Resources(kSmall1BitMask);
		if (isIconServicesAvailable)
			iconNum3=Count1Resources(kIconFamilyType);
		tempBool = (isIconServicesAvailable && iconNum3 > 0);
		
		idList=(short *)NewPtr(sizeof(short)*(iconNum+iconNum2+iconNum3));
		
		k=0;
		if (iconNum > 0)
		{
			/* まず、'ICN#'の調査 */
			for (i=1; i<=iconNum; i++)
			{
				resHandle=Get1IndResource(kLarge1BitMask,i);
				GetResInfo(resHandle,&id,&resType,name);
				ReleaseResource(resHandle);
				
				if (id != kCustomIconResource)
				{
					for (j=0; j<k; j++)
					{
						if (idList[j] == id) break;
						if (idList[j] > id)
						{
							BlockMoveData(&idList[j],&idList[j+1],sizeof(short)*(k-j));
							idList[j]=id;
							k++;
							break;
						}
					}
					if (j==k)
					{
						idList[j]=id;
						k++;
					}
				}
			}
		}
		
		if (iconNum2 > 0)
		{
			/* 次に、'ics#'の調査 */	
			for (i=1; i<=iconNum2; i++)
			{
				resHandle=Get1IndResource(kSmall1BitMask,i);
				GetResInfo(resHandle,&id,&resType,name);
				ReleaseResource(resHandle);
				
				if (id != kCustomIconResource)
				{
					for (j=0; j<k; j++)
					{
						if (idList[j] == id) break;
						if (idList[j] > id)
						{
							BlockMoveData(&idList[j],&idList[j+1],sizeof(short)*(k-j));
							idList[j]=id;
							k++;
							break;
						}
					}
					if (j==k)
					{
						idList[j]=id;
						k++;
					}
				}
			}
		}
		
		if (tempBool)
		{
			typeList=(ResType *)NewPtr(sizeof(ResType)*(k+iconNum3));
			for (j=0; j<k; j++)
				typeList[j] = kLarge1BitMask;
			
			/* 最後に、'icns'の調査 */
			for (i=1; i<=iconNum3; i++)
			{
				resHandle=Get1IndResource(kIconFamilyType,i);
				GetResInfo(resHandle,&id,&resType,name);
				ReleaseResource(resHandle);
				
				if (id != kCustomIconResource)
				{
					for (j=0; j<k; j++)
					{
						if (idList[j] == id)
						{
							typeList[j] = kIconFamilyType;
							break;
						}
						if (idList[j] > id)
						{
							BlockMoveData(&idList[j],&idList[j+1],sizeof(short)*(k-j));
							BlockMoveData(&typeList[j],&typeList[j+1],sizeof(ResType)*(k-j));
							idList[j] = id;
							typeList[j] = kIconFamilyType;
							k++;
							break;
						}
					}
					if (j==k)
					{
						idList[j] = id;
						typeList[j] = kIconFamilyType;
						k++;
					}
				}
			}
		}
		
		iconListPtr->iconNum=iconNum=k;
		height=(k+width-1)/width;
		
		k=0;
		for (j=0; j<height; j++)
		{
			LAddRow(1,j,iconList);
			
			for (i=0; i<width; i++)
			{
				SetPt(&cell,i,j);
				
				iconDataPtr=(IconListDataRec *)NewPtr(sizeof(IconListDataRec));
				iconDataPtr->resID = idList[k];
				if (tempBool)
					iconDataPtr->resType = typeList[k];
				else
					iconDataPtr->resType = kLarge1BitMask;
				k++;
				
				LSetCell(&iconDataPtr,sizeof(IconListDataRec *),cell,iconList);
				
				if (k >= iconNum) break;
			}
			if (k >= iconNum) break;
		}
		
		DisposePtr((Ptr)idList);
		if (tempBool)
			DisposePtr((Ptr)typeList);
		
		/* ID順にソート */
	//	SortIconList(iconList);
	}
	else
		iconListPtr->iconNum=0;
	
	LSetDrawingMode(true, iconList);
	
	UseResFile(gApplRefNum);
	
	return iconList;
}

/* リストのソーティング */
void SortIconList(ListHandle iconList)
{
	short	i,j;
	short	iconNum;
	IconListDataRec		*iconDataPtr1;
	IconListDataRec		*iconDataPtr2;
	IconListDataRec		**temp;
	IconListRec		*iconListPtr;
	
	iconListPtr=(IconListRec *)GetListRefCon(iconList);
	iconNum=iconListPtr->iconNum;
	
	temp=(IconListDataRec **)(*(**iconList).cells);
	
	for (i=1; i<iconNum; i++)
	{
		iconDataPtr1=temp[i];
		
		for (j=i-1; j>=0; j--)
		{
			iconDataPtr2=temp[j];
			
			if (iconDataPtr1->resID < iconDataPtr2->resID)
				temp[j+1]=iconDataPtr2;
			else
				break;
		}
		temp[j+1]=iconDataPtr1;
	}
	
	CancelSelect(iconList);
}

/* アイコンウィンドウのアップデート */
void UpdateIconWindow(WindowPtr iconWindow)
{
	IconListWinRec	iconRec;
	RgnHandle	rgn=NewRgn();
	
	iconRec=*GetIconListRec(iconWindow);
	GetPortVisibleRegion(GetWindowPort(iconWindow),rgn);
	LUpdate(rgn,iconRec.iconList);
	DisposeRgn(rgn);
	
	/* グローボックスの表示 */
	MyDrawGrowIcon(iconWindow);
}

/* グローボックスの表示 */
void MyDrawGrowIcon(WindowPtr theWindow)
{
	Rect		r;
	RgnHandle	temp=NewRgn();
	
	GetWindowPortBounds(theWindow,&r);
	GetClip(temp);
	SetRect(&r,r.right-kScrollBarWidth,r.bottom-kScrollBarHeight,r.right+1,r.bottom+1);
	ClipRect(&r);
	
	DrawGrowIcon(theWindow);
	
	SetClip(temp);
	DisposeRgn(temp);
}

/* アイコンウィンドウのクリック */
void ClickIconWindow(WindowPtr iconWindow,Point mousePt,EventRecord *theEvent) /* mousePtはローカル座標 */
{
	IconListWinRec	*iconRec;
	Rect	box={7,11,55,45};
	
	iconRec=GetIconListRec(iconWindow);
	
	if (LClick2(mousePt,theEvent->modifiers,iconRec->iconList,&box,theEvent))
		/* ダブルクリック処理 */
		OpenSelectedIcon(iconWindow);
	else
	{
		UpdateClipMenu();
		UpdateIconMenu();
	}
}

/* アイコンのクリック判定 */
Boolean	LClick2(Point mousePt,short modifier,ListHandle theList,Rect *box,EventRecord *theEvent)
{
	#pragma unused(modifier)
	Point	pt;
	Boolean isFront=(GetListWindow(theList) == MyFrontNonFloatingWindow());
	Cell	selectedCell={0,0};
	Boolean	wasSelected;
	Boolean	shiftDown=(modifier & shiftKey)!=0,cmdDown=(modifier & cmdKey)!=0;
	Point	size;
	Rect	viewBounds;
	ListBounds	visible;
	EventRecord	followEvent;
	UInt32	time;
	
	wasSelected=LGetSelect(true,&selectedCell,theList);
	
	GetListCellSize(theList,&size);
	pt.h=mousePt.h % size.h;
	pt.v=mousePt.v % size.v;
	
	GetListViewBounds(theList,&viewBounds);
	GetListVisibleCells(theList,&visible);
	if (!PtInRect(mousePt,&viewBounds))
	{
		if (!isFront)
			goto setFront;
		
		/* スクロールバー */
		return LClick(mousePt,modifier,theList);
	}
	if (PtInRect(pt,box))
	{
		Cell	theCell;
		Rect	dRect={-2,-2,3,3}; /* ドラッグ、ダブルクリックの判定用 */
		
		theCell.h=(mousePt.h-viewBounds.left)/size.h;
		theCell.v=(mousePt.v-viewBounds.top)/size.v+visible.top;
		
		OffsetRect(&dRect,mousePt.h,mousePt.v);
		
		if (isFront)
		{
			/* フロントウィンドウなら、まず選択する */
			if (shiftDown) /* shift+click */
			{
				Cell	topLeftCell={32767,32767},bottomRightCell={0,0},tempCell={0,0};
				
				LSetSelect(true,theCell,theList);
				
				while (LGetSelect(true,&tempCell,theList))
				{
					if (tempCell.h < topLeftCell.h) topLeftCell.h=tempCell.h;
					if (tempCell.v < topLeftCell.v) topLeftCell.v=tempCell.v;
					if (tempCell.h > bottomRightCell.h) bottomRightCell.h=tempCell.h;
					if (tempCell.v > bottomRightCell.v) bottomRightCell.v=tempCell.v;
					
					LNextCell(true,true,&tempCell,theList);
				}
				
				for (tempCell.h=topLeftCell.h; tempCell.h<=bottomRightCell.h; tempCell.h++)
					for (tempCell.v=topLeftCell.v; tempCell.v<=bottomRightCell.v; tempCell.v++)
						LSetSelect(true,tempCell,theList);
			}
			else if (!cmdDown) /* normal click */
			{
				if (!LGetSelect(false,&theCell,theList)) /* 選択されていない→アイコンがない */
					CancelSelect(theList);
			}
			if (!LGetSelect(false,&theCell,theList))
			{
				LSetSelect(true,theCell,theList);
				SetPortWindowPort(gPreviewWindow);
				UpdateIconPreview(GetListWindow(theList));
				SetPort(GetListPort(theList));
			}
		}
		
		if ((modifier & controlKey )!=0)
		{
			if (!isFront)
				goto setFront;
			
			pt=mousePt;
			LocalToGlobal(&pt);
			if (LGetSelect(false,&theCell,theList))
				MyIconListContextMenu(pt,GetListWindow(theList));
			else
				MyIconListContextMenu2(pt,GetListWindow(theList));
			return false;
		}
		
		time=TickCount()+GetDblTime();
		GetMouse(&pt);
		while (StillDown() && PtInRect(pt,&dRect))
		{
			/* */
			GetMouse(&pt);
			if (TickCount() > time)
			{
				LocalToGlobal(&pt);
				if (LGetSelect(false,&theCell,theList))
					MyIconListContextMenu(pt,GetListWindow(theList));
				else
					MyIconListContextMenu2(pt,GetListWindow(theList));
				return false;
			}
		}
		if (StillDown())
		{
			if (!isFront)
			{
				if (!LGetSelect(false,&theCell,theList))
					CancelSelect(theList);
				else
				{
					LSetSelect(true,theCell,theList);
					SetPortWindowPort(gPreviewWindow);
					UpdateIconPreview(GetListWindow(theList));
					SetPort(GetListPort(theList));
				}
			}

			/* ドラッグ処理 */
			if (LGetSelect(false,&theCell,theList) && isDragMgrPresent)
			{
				MyDoStartDragIcon(GetListWindow(theList),theEvent);
				return false;
			}
		}
		else
		{
			Boolean	result=false;
			
			time=TickCount()+GetDblTime();
			if (!isFront)
				goto setFront;
			
			/* ダブルクリック判定 */
			while (TickCount() < time && PtInRect(pt,&dRect) && !result)
			{
				GetMouse(&pt);
				result=EventAvail(mDownMask,&followEvent);
			}
			
			FlushEvents(mDownMask+mUpMask,0);
			
			return result;
		}
	}
	else
	{
		Rect	dRect={-2,-2,3,3};
		
		if (!isFront)
			goto setFront;
		
		CancelSelect(theList);
		RedrawIconPreview();
		if ((modifier & controlKey) != 0)
		{
			LocalToGlobal(&mousePt);
			MyIconListContextMenu2(mousePt,GetListWindow(theList));
			return false;
		}
		time=TickCount()+GetDblTime();
		GetMouse(&pt);
		OffsetRect(&dRect,mousePt.h,mousePt.v);
		while (StillDown() && PtInRect(pt,&dRect))
		{
			GetMouse(&pt);
			if(TickCount() > time)
			{
				LocalToGlobal(&pt);
				MyIconListContextMenu2(pt,GetListWindow(theList));
				return false;
			}
		}
		return false;
	}
setFront:
	SelectReferencedWindow(GetListWindow(theList));
	UpdateMenus();
	
	return false;
}

/* アイコンウィンドウのリサイズ */
void ResizeIconWindow(WindowPtr iconWindow,Point mousePt)
{
	long	result;
	Rect	bBox;
	
	GetRegionBounds(GetGrayRgn(),&bBox);
	bBox.left=240;
	bBox.top=192;
	
	result=GrowWindow(iconWindow,mousePt,&bBox);
	
	if (result!=0L)
	{
		IconListWinRec	*iWinRec=GetIconListRec(iconWindow);
		short	iconNum=IsMultiIconSelected(iWinRec);
		
		SizeWindow(iconWindow,LoWord(result),HiWord(result),true);
		
		if (IsIconSelected(iWinRec) == noErr)
		{
			IconListDataRec	*iconList=(IconListDataRec *)NewPtr(sizeof(IconListDataRec)*iconNum);
			short	i=0;
			Cell	theCell={0,0};
			short	dataSize=sizeof(IconListDataRec *);
			IconListDataRec	*iconData;
			
			while (LGetSelect(true,&theCell,iWinRec->iconList))
			{
				LGetCell(&iconData,&dataSize,theCell,iWinRec->iconList);
				iconList[i++]=*iconData;
				LNextCell(true,true,&theCell,iWinRec->iconList);
			}
			
			UpdateIconList(iWinRec,kSelectIconCommand,iconNum,iconList);
			
			DisposePtr((Ptr)iconList);
		}
		else
			UpdateIconList(iWinRec,kCommandNone,0,nil);
	}
}

/* 選択アイコンを動かす */
void MoveSelectedIcon(WindowPtr iconWindow,short dh,short dv)
{
	IconListWinRec	*iWinRec=GetIconListRec(iconWindow);
	Cell		theCell={0,0},newCell={0,0};
	IconListRec	*iconListPtr=(IconListRec *)GetListRefCon(iWinRec->iconList);
	short		iconNum=iconListPtr->iconNum,iconPos;
	Rect		dataBounds;
	short		cNum;
	Boolean	cellChanged=false;
	
	GetListDataBounds(iWinRec->iconList,&dataBounds);
	cNum=dataBounds.right;
	
	if (LGetSelect(true,&theCell,iWinRec->iconList))
	{
		/* 選択セルあり */
		SetPt(&newCell,theCell.h+dh,theCell.v+dv);
		if (newCell.h>=cNum)
			SetPt(&newCell,0,newCell.v+1);
		if (newCell.h<0)
			SetPt(&newCell,cNum-1,newCell.v-1);
		iconPos=newCell.v*cNum+newCell.h;
		if (iconPos < iconNum && iconPos >= 0 && PtInRect(newCell,&dataBounds))
		{
			LSetSelect(false,theCell,iWinRec->iconList);
			LSetSelect(true,newCell,iWinRec->iconList);
			LScrollToCell(newCell,iWinRec->iconList);
			
			cellChanged=true;
		}
	}
	else
	{
		if (dh+dv > 0)
		{
			SetPt(&newCell,0,0);
			LSetSelect(true,newCell,iWinRec->iconList);
			LScrollToCell(newCell,iWinRec->iconList);
		}
		else
		{
			SetPt(&newCell,iconNum % cNum -1,iconNum / cNum);
			LSetSelect(true,newCell,iWinRec->iconList);
			LScrollToCell(newCell,iWinRec->iconList);
		}
		cellChanged=true;
		UpdateClipMenu();
		UpdateIconMenu();
	}
	
	if (cellChanged)
	{
		RedrawIconPreview();
	}
}

/* アイコンリソースを開く */
void OpenSelectedIcon(WindowPtr iconWindow)
{
	IconListWinRec	*iWinRec;
	WindowPtr	fWindow;
	IconFamilyWinRec	*fWinRec;
	IconActionUPP	detachIconUPP;
	
	IPIconRec	ipIcon;
	OSErr	err;
	Str255	iconName;
	
	Point	windPos={0,0};
	Point	windOffset=kWinOffset;
	Cell	theCell={0,0};
	Boolean	is32Exist;
	
	iWinRec=GetIconListRec(iconWindow);
	
	/* 親ウィンドウの位置を得てそこからずらす */
	SetPortWindowPort(iconWindow);
	LocalToGlobal(&windPos);
	AddPt(windOffset,&windPos);
	
	while (LGetSelect(true,&theCell,iWinRec->iconList))
	{
		short	dataLen=sizeof(IconListDataRec *);
		IconListDataRec	*data;
		Boolean		isEditable=true;
		
		LGetCell(&data,&dataLen,theCell,iWinRec->iconList);
		
		/* 開こうとしているアイコンがすでに開かれていないかどうかをチェック */
		fWindow=GetNextVisibleWindow(iconWindow);
		while(fWindow!=nil)
		{
			if (GetExtWindowKind(fWindow)==kWindowTypeIconFamilyWindow)
			{
				fWinRec=GetIconFamilyRec(fWindow);
				if (fWinRec->parentWindow==iconWindow && fWinRec->iconID==data->resID) break;
			}
			fWindow=GetNextVisibleWindow(fWindow);
		}
		if (fWindow!=nil)
		{
			SelectReferencedWindow(fWindow);
			UpdateMenus();
			return;
		}
		
		/* アイコンを得る */
		err=MyGetIPIcon(iWinRec,&ipIcon,data,iconName,&isEditable);
		
		if (!isIconServicesAvailable || data->resType != kIconFamilyType)
		{
			/* 読み込んだデータをリソースから切り離す */
			detachIconUPP=NewIconActionUPP(DetachIcon);
			err=ForEachIconDo(ipIcon.iconSuite,GetMySelector(),detachIconUPP,nil);
			DisposeIconActionUPP(detachIconUPP);
		}
		else if (!isEditable)
		{
			Str255	prompt;
			
			GetIndString(prompt,149,3);
			if (MyConfirmDialog(prompt,NULL,NULL) != noErr)
			{
				LNextCell(true,true,&theCell,iWinRec->iconList);
				continue;
			}
		}
		
		/* 32bitアイコンがあるかどうかを調べる */
		if (isIconServicesAvailable)
			err=IPIconHas32Icons(&ipIcon,&is32Exist);
		else
			is32Exist=false;
		
		UseResFile(gApplRefNum);
		fWindow=MakeFamilyWindow(windPos,is32Exist,NULL,data->resType,&ipIcon,iconName);
		if (fWindow==nil) return;
		
		fWinRec=GetIconFamilyRec(fWindow);
		fWinRec->parentWindow=iconWindow;
		fWinRec->iconID=data->resID;
		
		LNextCell(true,true,&theCell,iWinRec->iconList);
	}
	
	UpdateMenus();
}

/* 白紙アイコンを追加する */
void AddNewIcon(WindowPtr iconWindow)
{
	IPIconRec		ipIcon;
	IconListWinRec	*iWinRec=GetIconListRec(iconWindow);
	MyIconResRec	newIcon;
	IPIconSelector	selector;
	OSErr			err;
	short			iconTemplate;
	
	UseResFile(gApplRefNum);
	GetIndString(newIcon.resName,141,1);
	newIcon.resID=MyUniqueID(iWinRec->iconList);
	newIcon.attrs=resPurgeable;
	
	if (NewIconDialog(&newIcon,&selector,&iconTemplate,iWinRec->iconList) != noErr) return;
	
	err=GetIPIconFromTemplate(&ipIcon,&selector,iconTemplate);
	if (err != noErr)
	{
		SysBeep(0);
		return;
	}
	
	DisposePtr((Ptr)iWinRec->undoData.iconList);
	iWinRec->undoData.iconList=(IconListDataRec *)NewPtr(sizeof(IconListDataRec *));
	iWinRec->undoData.iconNum=0;
	
	AddIPIconToList(iconWindow,&ipIcon,(ResType)NULL,newIcon.resName,newIcon.resID,true);
}

/* ファイルからのアイコンの取り込み */
void ImportIconFromFile(WindowPtr iconWindow)
{
	IconListWinRec		*iWinRec=NULL;
	short			id=128;
	Boolean			wasImported=false;
	
	DeactivateFloatersAndFirstDocumentWindow();
	#if !TARGET_API_MAC_CARBON
	if (isNavServicesAvailable && useNavigationServices)
	#endif
	{
		OSErr	err;
		NavReplyRecord	theReply;
		
		err=SelectObjectDialog(NewNavEventUPP(MyEventProc),"\p",&theReply);
		if (err==noErr)
		{
			long		i,numFiles;
			AEKeyword	keyword;
			DescType	typeCode;
			Size		actualSize;
			FSSpec		theFile;
			
			/* 選択されたオブジェクトの数 */
			err=AECountItems(&theReply.selection,&numFiles);
			
			iWinRec=GetIconListRec(iconWindow);
			DisposePtr((Ptr)iWinRec->undoData.iconList);
			iWinRec->undoData.iconList=(IconListDataRec *)NewPtr(sizeof(IconListDataRec)*numFiles);
			iWinRec->undoData.iconNum=0;
			
			if (gOtherPrefs.continuousIDs)
				id=MyContinuousUniqID(iWinRec->iconList,numFiles);
			
			for (i=1; i<=numFiles; i++)
			{
				err=AEGetNthPtr(&theReply.selection,i,typeFSS,&keyword,&typeCode,(Ptr)&theFile,sizeof(theFile),&actualSize);
				
				if (!gOtherPrefs.continuousIDs) id=MyUniqueID(iWinRec->iconList);
				
				AddFileIconToIconList(iconWindow,&theFile,id++,(i==numFiles));
			}
			
			wasImported=true;
		}
		err=NavDisposeReply(&theReply);
	}
	#if !TARGET_API_MAC_CARBON
	else
	{
		StandardFileReply	reply;
		
		StandardGetFolder(-1,nil,&reply);
		if (reply.sfGood)
		{
			iWinRec=GetIconListRec(iconWindow);
			DisposePtr((Ptr)iWinRec->undoData.iconList);
			iWinRec->undoData.iconList=(IconListDataRec *)NewPtr(sizeof(IconListDataRec));
			iWinRec->undoData.iconNum=0;
			
			AddFileIconToIconList(iconWindow,&reply.sfFile,0,true);
			
			wasImported=true;
		}
	}
	#endif
	ActivateFloatersAndFirstDocumentWindow();
	
	if (wasImported)
	{
		iWinRec->wasChanged=true;
		UpdateSaveMenu();
		UpdateIconMenu();
		RedrawIconPreview();
		
		iWinRec->undoData.undoMode=umAddIcon;
		UpdateUndoMenu();
	}
}

/* アイコンをフォルダにする */
void ExportIconToFolder(WindowPtr iconWindow)
{
	IconListWinRec		*iWinRec=GetIconListRec(iconWindow);
	OSErr			err;
	IconListDataRec	*data;
	FSSpec			theFile;
	OSType			iconType;
	Str255			iconName,str;
	IPIconRec		ipIcon;
	NavReplyRecord	theReply;
	
	err=GetSelectedIconData(iWinRec,&data);
	if (err!=noErr) return;
	
	/* アイコンを取得しておく */
	err=MyGetIPIcon(iWinRec,&ipIcon,data,iconName,NULL);
	if (err!=noErr) return;
	
	/* フォルダの名前を決める */
	/* フォルダ名には「:」は使用できない。また、フォルダ名は31バイトまで */
	ReplaceString(iconName,"\p-","\p:");
	if (iconName[0]==0)
	{
		PStrCpy(iWinRec->iconFileSpec.name,iconName);
		GetIndString(str,140,2);
		PStrCat(str,iconName);
		NumToString(data->resID,str);
		PStrCat(str,iconName);
	}
	TruncPString(iconName,iconName,31);
	
	UseResFile(gApplRefNum);
	
	err=SaveAsIconDialog(iconName,&theFile,&iconType,&theReply);
	
	if (err==noErr)
	{
		/* フォルダを作成 */
		err=MyGetIPIcon(iWinRec,&ipIcon,data,iconName,NULL);
		switch (iconType)
		{
			case kFolderIconType:
				err=MakeFolderWithIPIcon(&theFile,&ipIcon);
				break;
			
			case kWinIconType:
				err=MakeWinIconFromSuite(&theFile,ipIcon.iconSuite);
				break;
			
			case kXIconFileType:
				err=MakeXIconWithIPIcon(&theFile,&ipIcon);
				break;
		}
		
		if (isNavServicesAvailable && useNavigationServices)
		{
			if (theReply.validRecord)
				err=NavCompleteSave(&theReply,kNavTranslateInPlace);
			err=NavDisposeReply(&theReply);
		}
	}
	err=DisposeIPIcon(&ipIcon);
}

/* ファイルのアイコンをアイコンリストに追加 */
void AddFileIconToIconList(WindowPtr iconWindow,FSSpec *theFile,short id,Boolean redrawFlag)
{
	IPIconRec		ipIcon;
	OSErr			err;
	Str15			suffix;
	Boolean			isDirectory = false;
	OSType			fileType;
	#ifdef __MOREFILESX__
	FSRef			fsRef;
	FinderInfo		info;
	#else
	FInfo			fndrInfo;
	DInfo			dirInfo;
	#endif
	
	GetSuffix(theFile->name,suffix);
	
	#ifdef __MOREFILESX__
	err = FSpMakeFSRef(theFile,&fsRef);
	err = FSGetFinderInfo(&fsRef,&info,NULL,&isDirectory);
	
	if (!isDirectory) {
		fileType = info.file.fileType;
	}
	#else
	err=FSpGetFInfo(theFile,&fndrInfo);
	if (err==fnfErr)
	{
		err=FSpGetDInfo(theFile,&dirInfo);
		isDirectory = (err == noErr);
	}
	else
	{
		if (err == noErr)
		{
			fileType = fndrInfo.fdType;
		}
	}
	if (err!=noErr) return;
	
	#endif
	
	if (isDirectory)
	{
		fileType = 'fold';
	}
	else
	{
		if (EqualString(suffix,"\p.ico",false,true))
			fileType = kWinIconFileType;
		else if (EqualString(suffix,"\p.icns",false,true))
			fileType = kXIconFileType;
	}
	
	switch (fileType)
	{
		case kWinIconFileType:
			/* windows icons */
			err=WinIconToIconSuite(theFile,&ipIcon.iconSuite);
			if (err!=noErr) return;
			
			ipIcon.it32Data = NULL;
			ipIcon.t8mkData = NULL;
			break;
		
		case kXIconFileType:
			/* mac os x icons */
			err=XIconToIPIcon(theFile,&ipIcon);
			if (err!=noErr) return;
			break;
		
		default:
			{
				Boolean		is32Exist;
				short		alertMode;
				
				err=GetFileIPIcon(theFile,&ipIcon,isDirectory,&is32Exist,&alertMode);
				if (err!=noErr)
				{
					SysBeep(0);
					return;
				}
			}
	}
	
	/* アイコンが有効なものかどうか */
	if (!IsValidIconSuite(ipIcon.iconSuite))
	{
		err=DisposeIPIcon(&ipIcon);
		return;
	}
	
	AddIPIconToList(iconWindow,&ipIcon,(ResType)NULL,theFile->name,id,redrawFlag);
	
	gUsedCount.importNum++;
}

/* IPIconを追加する */
void AddIPIconToList(WindowPtr iconWindow,const IPIconRec *ipIcon,ResType iconType,
	Str255 iconName,short id,Boolean redrawFlag)
{
	IconListWinRec	*iWinRec=GetIconListRec(iconWindow);
	MyIconResRec	newIcon;
	OSErr			err;
	IconListDataRec	*iconDataPtr;
	
	if (id == 0) /* IDの指定なし→未使用のIDを得る */
		newIcon.resID=MyUniqueID(iWinRec->iconList);
	else
		newIcon.resID=id;
	
	PStrCpy(iconName,newIcon.resName);
	newIcon.attrs=resPurgeable;
	
	UseResFile(iWinRec->tempRefNum);
	
	/* アイコンを追加する */
	if (iconType == kIconFamilyType ||
		(iconType == (ResType)NULL && ((isIconServicesAvailable && gOtherPrefs.importAsFamilyIcon) || 
		(isThumbnailIconsAvailable && IsIPIconHasThumbnailIcon(ipIcon)))))
	{
		IconFamilyHandle	iconFamily;
		
		err = IPIconToIconFamily(ipIcon,&iconFamily);
        if (err == noErr && iconFamily != nil) {
            HLock((Handle)iconFamily);
            SaveDataToResource(*iconFamily,GetHandleSize((Handle)iconFamily),kIconFamilyType,
                newIcon.resID,newIcon.resName,newIcon.attrs);
            HUnlock((Handle)iconFamily);
            DisposeHandle((Handle)iconFamily);
        } else {
            return;
        }
		
		iconType = kIconFamilyType;
	}
	else
	{
		IconActionUPP	addIconUPP=NewIconActionUPP(AddIconToFile);
		err=ForEachIconDo(ipIcon->iconSuite,GetMySelector(),addIconUPP,&newIcon);
		DisposeIconActionUPP(addIconUPP);
		
		iconType = kLarge1BitMask;
	}
	
	DisposeIPIcon(ipIcon);
	
	CreateUpdateInfo(id);
	
	UseResFile(gApplRefNum);
	
	/* 取り消し用のデータ */
	iWinRec->undoData.iconList[iWinRec->undoData.iconNum].resID = newIcon.resID;
	iWinRec->undoData.iconList[iWinRec->undoData.iconNum].resType = iconType;
	iWinRec->undoData.iconNum++;
	
	/* リストの更新 */
	iconDataPtr=(IconListDataRec *)NewPtr(sizeof(IconListDataRec));
	iconDataPtr->resID = newIcon.resID;
	iconDataPtr->resType = iconType;
	
	AddIconToIconList(iWinRec,iconDataPtr,redrawFlag);
	
	if (redrawFlag)
	{
		UpdateResFile(iWinRec->tempRefNum);
		
		iWinRec->wasChanged=true;
		UpdateSaveMenu();
		RedrawIconPreview();
		
		iWinRec->undoData.undoMode=umAddIcon;
		UpdateUndoMenu();
	}
}

/* 選択中のアイコンをクリップボードにコピー */
void CopySelectedIcon(IconListWinRec *iWinRec)
{
	IconListDataRec	*data;
	IPIconRec	ipIcon;
	OSErr	err;
	Str255	name;
	
	err=GetSelectedIconData(iWinRec,&data);
	if (err!=noErr) return;
	
	err=MyGetIPIcon(iWinRec,&ipIcon,data,name,NULL);
	if (err!=noErr) return;
	
	err=IPIconToClip(&ipIcon);
	if (err!=noErr) return;
	
	err=DisposeIPIcon(&ipIcon);
	
	UseResFile(gApplRefNum);
	UpdatePasteMenu();
}

/* 選択アイコンの削除 */
void DeleteSelectedIcon(IconListWinRec *iWinRec)
{
	short		id;
	OSErr	err;
	WindowPtr	fWindow,iconWindow=GetListWindow(iWinRec->iconList); /* いやな方法だ */
	Cell	theCell={0,0};
	short	iconNum,i=0;
	IconListDataRec	*tempList;
	
	iconNum=IsMultiIconSelected(iWinRec);
	tempList=(IconListDataRec *)NewPtr(sizeof(IconListDataRec)*iconNum);
	
	/* まず、削除可能かどうかを調べる */
	while (LGetSelect(true,&theCell,iWinRec->iconList))
	{
		short	dataLen=sizeof(IconListDataRec *);
		IconListDataRec	*iconDataPtr;
		
		LGetCell(&iconDataPtr,&dataLen,theCell,iWinRec->iconList);
		id=iconDataPtr->resID;
		tempList[i++]=*iconDataPtr; /* 取り消し用IDリスト */
		
		UseResFile(gApplRefNum);
		
		/* そのアイコンが開かれていないかどうかをチェック */
		fWindow=MyFrontNonFloatingWindow();
		while (fWindow!=nil)
		{
			if (GetExtWindowKind(fWindow)==kWindowTypeIconFamilyWindow)
			{
				IconFamilyWinRec	*fWinRec=GetIconFamilyRec(fWindow);
				
				if (fWinRec->parentWindow==iconWindow && fWinRec->iconID==id)
				{
					err=CloseFamilyWindow(fWindow,true,true); /* 削除フラグ付き */
					if (err!=noErr)
					{
						DisposePtr((Ptr)tempList);
						return;
					}
					else break;
				}
			}
			fWindow=GetNextVisibleWindow(fWindow);
		}
		
		LNextCell(true,true,&theCell,iWinRec->iconList);
	}
	
	/* 実際の削除 */
	DisposePtr((Ptr)iWinRec->undoData.iconList);
	iWinRec->undoData.iconList=tempList;
	
	/* 削除情報リソースの作成 */
	UseResFile(iWinRec->tempRefNum);
	for (i=0; i<iconNum; i++)
		CreateDeleteInfo(iWinRec->undoData.iconList[i].resID);
	
	iWinRec->undoData.iconNum=iconNum;
	UpdateIconList(iWinRec,kDeleteIconCommand,iWinRec->undoData.iconNum,iWinRec->undoData.iconList);
	
	iWinRec->wasChanged=true;
	UpdateSaveMenu();
	RedrawIconPreview();
	
	/* 取消用 */
	iWinRec->undoData.undoMode=umDeleteIcon;
	
	UpdateResFile(iWinRec->tempRefNum);
	UpdateUndoMenu();
}

static IconSuiteRef	mIconSuite;

/* アイコンの情報 */
void SelectedIconInfo(IconListWinRec *iWinRec)
{
	MyIconResRec	iconInfo;
	DialogPtr		dp;
	short			item;
	OSErr			err;
	IPIconRec		ipIcon;
	GrafPtr			port;
	Str255			str;
	long			temp;
	Cell			theCell={0,0};
	short			dataSize=sizeof(IconListDataRec *);
	IconListDataRec	*iconDataPtr;
	ModalFilterUPP	mfUPP=NewModalFilterUPP(MyDispIconFilter);
	
	/* まずアイコンの情報を得る */
	LGetSelect(true,&theCell,iWinRec->iconList);
	LGetCell(&iconDataPtr,&dataSize,theCell,iWinRec->iconList);
	iconInfo.resID=iconDataPtr->resID;
	
	err=MyGetIPIcon(iWinRec,&ipIcon,iconDataPtr,iconInfo.resName,NULL);
	if (err!=noErr)
	{
		SysBeep(0);
		return;
	}
	
	/* ダイアログを表示し、必要な情報を入れておく */
	DeactivateFloatersAndFirstDocumentWindow();
	GetPort(&port);
	UseResFile(gApplRefNum);
	dp=GetNewDialog(131,nil,kFirstWindowOfClass);
	SetPortDialogPort(dp);
	
	NumToString(iconInfo.resID,str);
	SetDialogItemText2(dp,4,str);
	SetDialogItemText2(dp,5,iconInfo.resName);
	
	SetDialogDefaultItem(dp,ok);
	SetDialogCancelItem(dp,cancel);
	
	mIconSuite=ipIcon.iconSuite;
	
	SelectDialogItemText(dp,4,0,str[0]);
	ShowWindow(GetDialogWindow(dp));
	
	item=3;
	while (item!=ok && item!=cancel)
	{
		ModalDialog(mfUPP,&item);
		
		if (item == ok)
		{
			GetDialogItemText2(dp,4,str);
			StringToNum(str,&temp);
			if (temp == kCustomIconResource)
			{
				/* カスタムアイコンのIDにはできない */
				SysBeep(0);
				item=3;
				SelectDialogItemText(dp,4,0,str[0]);
				continue;
			}
			if (temp != iconInfo.resID)
			{
				/* すでに同じIDのものがあるかもしれないからそれをチェック */
				Cell	theCell={0,0};
				
				if (IDToCell(temp,&theCell,iWinRec->iconList))
				{
					SysBeep(0);
					item=3;
					SelectDialogItemText(dp,4,0,str[0]);
				}
			}
		}
			
	}
	
	if (item==ok)
	{
		/* 取り消し用に元のデータを保存しておく */
		DisposePtr((Ptr)iWinRec->undoData.iconList);
		iWinRec->undoData.iconList=(IconListDataRec *)NewPtr(sizeof(IconListDataRec)*2);
		iWinRec->undoData.iconNum=2;
		iWinRec->undoData.iconList[1].resID=iconInfo.resID;
		iWinRec->undoData.iconList[1].resType=iconDataPtr->resType;
		PStrCpy(iconInfo.resName,iWinRec->undoData.name);
		
		/* ダイアログから必要な情報を得る */
		GetDialogItemText2(dp,5,iconInfo.resName);
		
		/* もしそのアイコンが開かれていればそのウィンドウのデータを更新 */
		{
			WindowPtr	fWindow=MyFrontNonFloatingWindow();
			WindowPtr	iconWindow=GetListWindow(iWinRec->iconList);
			
			while (fWindow != nil)
			{
				if (GetExtWindowKind(fWindow)==kWindowTypeIconFamilyWindow)
				{
					IconFamilyWinRec	*fWinRec=GetIconFamilyRec(fWindow);
					
					if (fWinRec->parentWindow == iconWindow && fWinRec->iconID == iconInfo.resID)
					{
						Str15	idStr;
						Str255	title;
						GrafPtr	port;
						
						fWinRec->iconID=temp;
						PStrCpy(iconInfo.resName,fWinRec->iconName);
						
						/* TE */
						TESetText(&fWinRec->iconName[1],fWinRec->iconName[0],fWinRec->iconNameTE);
						TESetSelect(0,fWinRec->iconName[0],fWinRec->iconNameTE);
						GetPort(&port);
						SetPortWindowPort(fWindow);
						MyInvalWindowRect(fWindow,&(**fWinRec->iconNameTE).viewRect);
						SetPort(port);
						
						/* タイトル */
						GetIndString(title,sFamilyWinTitle,2);
						NumToString(temp,idStr);
						ReplaceString(title,idStr,"\p^0");
						PStrCat(iWinRec->iconFileSpec.name,title);
						SetWTitle(fWindow,title);
						break;
					}
				}
				fWindow=GetNextVisibleWindow(fWindow);
			}
		}
		
		UseResFile(iWinRec->tempRefNum);
		
		/* IDが変わるときは、削除情報を作成 */
		if (iconInfo.resID != temp)
			CreateDeleteInfo(iconInfo.resID);
		
		/* アイコン自体の更新 */
		/* すでにテンポラリファイルに存在する場合、存在しない場合を考慮する必要あり */
		if (iconDataPtr->resType == kIconFamilyType)
		{
			Handle		resHandle;
			
			resHandle = Get1Resource(kIconFamilyType,iconInfo.resID);
			
			if (resHandle == NULL) /* テンポラリファイルに存在しない */
			{
				UseResFile(iWinRec->refNum);
				resHandle = Get1Resource(kIconFamilyType,iconInfo.resID);
				DetachResource(resHandle);
				
				UseResFile(iWinRec->tempRefNum);
				AddResource(resHandle,kIconFamilyType,temp,iconInfo.resName);
			}
			else
				SetResInfo(resHandle,temp,iconInfo.resName);
			
			ChangedResource(resHandle);
			ReleaseResource(resHandle);
			iconInfo.resID=temp;
		}
		else
		{
			IconActionUPP	iconActionUPP;
			
			if (IsIconChanged(iconInfo.resID)) /* アイコンが変更されている（→テンポラリファイルに存在） */
			{
				/* 情報変更のみ */
				iconActionUPP=NewIconActionUPP(ChangeIconInfo);
				iconInfo.resID=temp;
				iconInfo.attrs=resPurgeable;
				err=ForEachIconDo(ipIcon.iconSuite,GetMySelector(),iconActionUPP,&iconInfo);
				DisposeIconActionUPP(iconActionUPP);
			}
			else
			{
				/* リソースから切り離して追加する */
				iconActionUPP=NewIconActionUPP(DetachIcon);
				err=ForEachIconDo(ipIcon.iconSuite,GetMySelector(),iconActionUPP,NULL);
				DisposeIconActionUPP(iconActionUPP);
				
				iconActionUPP=NewIconActionUPP(AddIconToFile);
				iconInfo.resID=temp;
				iconInfo.attrs=resPurgeable;
				err=ForEachIconDo(ipIcon.iconSuite,GetMySelector(),iconActionUPP,&iconInfo);
				DisposeIconActionUPP(iconActionUPP);
			}
		}
		
		CreateUpdateInfo(temp);
		
		UpdateResFile(iWinRec->tempRefNum);
		
		/* アイコンを破棄 */
		err=DisposeIPIcon(&ipIcon);
		
		dataSize=sizeof(IconListDataRec *);
		iconDataPtr->resID=iconInfo.resID;
		LSetCell(&iconDataPtr,dataSize,theCell,iWinRec->iconList);
		
		LSetDrawingMode(false,iWinRec->iconList);
		SortIconList(iWinRec->iconList);
		
		IDToCell(iconInfo.resID,&theCell,iWinRec->iconList);
		LSetSelect(true,theCell,iWinRec->iconList);
		LScrollToCell(theCell,iWinRec->iconList);
		
		LSetDrawingMode(true,iWinRec->iconList);
		
		UseResFile(gApplRefNum);
	
		iWinRec->wasChanged=true;
		
		iWinRec->undoData.undoMode=umChangeIconInfo;
		iWinRec->undoData.iconList[0].resID=temp;
		iWinRec->undoData.iconList[0].resType=iconDataPtr->resType;
	}
	
	DisposeDialog(dp);
	DisposeModalFilterUPP(mfUPP);
	ActivateFloatersAndFirstDocumentWindow();
	
	SetPort(port);
	
	/* ウィンドウとメニューのアップデート */
	if (item == ok)
	{
		WindowPtr	iWindow=GetListWindow(iWinRec->iconList);
		Rect		viewBounds;
		
		GetListViewBounds(iWinRec->iconList,&viewBounds);
		UpdateSaveMenu();
		UpdateUndoMenu();
		MyInvalWindowRect(iWindow,&viewBounds);
	}
}

/* アイコンを表示するフィルタ */
pascal Boolean MyDispIconFilter(DialogPtr theDialog,EventRecord *theEvent,short *theItemHit)
{
	if (theEvent->what == updateEvt && GetDialogFromWindow((WindowPtr)theEvent->message) == theDialog)
	{
		GrafPtr	port;
		Rect	box;
		OSErr	err;
		
		GetPort(&port);
		SetPortDialogPort(theDialog);
		BeginUpdate(GetDialogWindow(theDialog));
		DrawDialog(theDialog);
		GetDialogItemRect(theDialog,3,&box);
		err=PlotIconSuite(&box,kAlignNone,kTransformNone,mIconSuite);
		EndUpdate(GetDialogWindow(theDialog));
		SetPort(port);
		
		return true;
	}
	
	return MyModalDialogFilter(theDialog,theEvent,theItemHit);
}

/* 選択アイコンの複製 */
void DuplicateSelectedIcon(WindowPtr iconWindow)
{
	IconListWinRec	*iWinRec=GetIconListRec(iconWindow);
	IPIconRec	ipIcon;
	OSErr		err;
	MyIconResRec	newIcon;
	Str255		str;
	IconListDataRec	*data;
	
	err=GetSelectedIconData(iWinRec,&data);
	GetIndString(str,141,3);
	
	err=MyGetIPIcon(iWinRec,&ipIcon,data,newIcon.resName,NULL);
	PStrCat(str,newIcon.resName);
	
	DisposePtr((Ptr)iWinRec->undoData.iconList);
	iWinRec->undoData.iconList=(IconListDataRec *)NewPtr(sizeof(IconListDataRec));
	iWinRec->undoData.iconNum=0;
	
	AddIPIconToList(iconWindow,&ipIcon,data->resType,newIcon.resName,0,true);
}

/* アイコンのペースト */
void PasteIcon(WindowPtr iconWindow)
{
	IconListWinRec	*iWinRec=GetIconListRec(iconWindow);
	MyIconResRec	newIcon;
	IPIconRec		ipIcon;
	IconSuiteRef	iconSuite;
	OSErr	err;
	long	dataSize;
	
	#if TARGET_API_MAC_CARBON
	ScrapRef	scrap;
	
	err=GetCurrentScrap(&scrap);
	err=GetScrapFlavorSize(scrap,kIconFamilyType,&dataSize);
	if (dataSize>0)
	{
		IconFamilyHandle	iconFamily=(IconFamilyHandle)NewHandle(dataSize);
		
		HLock((Handle)iconFamily);
		err=GetScrapFlavorData(scrap,kIconFamilyType,&dataSize,*iconFamily);
		HUnlock((Handle)iconFamily);
		err=IconFamilyToIPIcon(iconFamily,&ipIcon);
		DisposeHandle((Handle)iconFamily);
	}
	else
	{
		Handle	dataHandle;
		
		err=NewIconSuite(&iconSuite);
		
		err=GetScrapFlavorSize(scrap,kLarge1BitMask,&dataSize);
		if (dataSize>0)
		{
			dataHandle=NewHandle(dataSize);
			err=GetScrapFlavorData(scrap,kLarge1BitMask,&dataSize,*dataHandle);
			err=AddIconToSuite(dataHandle,iconSuite,kLarge1BitMask);
			
			err=GetScrapFlavorSize(scrap,kLarge8BitData,&dataSize);
			if (dataSize>0)
			{
				dataHandle=NewHandle(dataSize);
				err=GetScrapFlavorData(scrap,kLarge8BitData,&dataSize,*dataHandle);
				err=AddIconToSuite(dataHandle,iconSuite,kLarge8BitData);
			}
			err=GetScrapFlavorSize(scrap,kLarge4BitData,&dataSize);
			if (dataSize>0)
			{
				dataHandle=NewHandle(dataSize);
				err=GetScrapFlavorData(scrap,kLarge4BitData,&dataSize,*dataHandle);
				err=AddIconToSuite(dataHandle,iconSuite,kLarge4BitData);
			}
		}
		err=GetScrapFlavorSize(scrap,kSmall1BitMask,&dataSize);
		if (dataSize>0)
		{
			dataHandle=NewHandle(dataSize);
			err=GetScrapFlavorData(scrap,kSmall1BitMask,&dataSize,*dataHandle);
			err=AddIconToSuite(dataHandle,iconSuite,kSmall1BitMask);
			
			err=GetScrapFlavorSize(scrap,kSmall8BitData,&dataSize);
			if (dataSize>0)
			{
				dataHandle=NewHandle(dataSize);
				err=GetScrapFlavorData(scrap,kSmall8BitData,&dataSize,*dataHandle);
				err=AddIconToSuite(dataHandle,iconSuite,kSmall8BitData);
			}
			err=GetScrapFlavorSize(scrap,kSmall4BitData,&dataSize);
			if (dataSize>0)
			{
				dataHandle=NewHandle(dataSize);
				err=GetScrapFlavorData(scrap,kSmall4BitData,&dataSize,*dataHandle);
				err=AddIconToSuite(dataHandle,iconSuite,kSmall4BitData);
			}
		}
		
		ipIcon.iconSuite = iconSuite;
		ipIcon.it32Data = NULL;
		ipIcon.t8mkData = NULL;
	}
	#else
	long	offset;
	
	if (isIconServicesAvailable && (dataSize=GetScrap(0,kIconFamilyType,&offset))>0)
	{
		IconFamilyHandle	iconFamily=(IconFamilyHandle)NewHandle(dataSize);
		
		HLock((Handle)iconFamily);
		dataSize=GetScrap((Handle)iconFamily,kIconFamilyType,&offset);
		HUnlock((Handle)iconFamily);
		err=IconFamilyToIPIcon(iconFamily,&ipIcon);
		
		DisposeHandle((Handle)iconFamily);
	}
	else
	{
		Handle	dataHandle;
		
		err=NewIconSuite(&iconSuite);
		
		dataSize=GetScrap(0,kLarge1BitMask,&offset);
		if (dataSize>0)
		{
			dataHandle=NewHandle(dataSize);
			dataSize=GetScrap(dataHandle,kLarge1BitMask,&offset);
			err=AddIconToSuite(dataHandle,iconSuite,kLarge1BitMask);
			
			dataSize=GetScrap(0,kLarge8BitData,&offset);
			if (dataSize>0)
			{
				dataHandle=NewHandle(dataSize);
				dataSize=GetScrap(dataHandle,kLarge8BitData,&offset);
				err=AddIconToSuite(dataHandle,iconSuite,kLarge8BitData);
			}
			dataSize=GetScrap(0,kLarge4BitData,&offset);
			if (dataSize>0)
			{
				dataHandle=NewHandle(dataSize);
				dataSize=GetScrap(dataHandle,kLarge4BitData,&offset);
				err=AddIconToSuite(dataHandle,iconSuite,kLarge4BitData);
			}
		}
		dataSize=GetScrap(0,kSmall1BitMask,&offset);
		if (dataSize>0)
		{
			dataHandle=NewHandle(dataSize);
			dataSize=GetScrap(dataHandle,kSmall1BitMask,&offset);
			err=AddIconToSuite(dataHandle,iconSuite,kSmall1BitMask);
			
			dataSize=GetScrap(0,kSmall8BitData,&offset);
			if (dataSize>0)
			{
				dataHandle=NewHandle(dataSize);
				dataSize=GetScrap(dataHandle,kSmall8BitData,&offset);
				err=AddIconToSuite(dataHandle,iconSuite,kSmall8BitData);
			}
			dataSize=GetScrap(0,kSmall4BitData,&offset);
			if (dataSize>0)
			{
				dataHandle=NewHandle(dataSize);
				dataSize=GetScrap(dataHandle,kSmall4BitData,&offset);
				err=AddIconToSuite(dataHandle,iconSuite,kSmall4BitData);
			}
		}
		
		ipIcon.iconSuite = iconSuite;
		ipIcon.it32Data = NULL;
		ipIcon.t8mkData = NULL;
	}
	#endif
	
	GetIndString(newIcon.resName,141,1);
	
	/* 取り消し用データ */
	DisposePtr((Ptr)iWinRec->undoData.iconList);
	iWinRec->undoData.iconList=(IconListDataRec *)NewPtr(sizeof(IconListDataRec));
	iWinRec->undoData.iconNum=0;
	
	AddIPIconToList(iconWindow,&ipIcon,(ResType)NULL,newIcon.resName,0,true);
}

/* アイコンをすべて選択 */
void SelectAllIcons(WindowPtr iconWindow)
{
	IconListWinRec	*iWinRec=GetIconListRec(iconWindow);
	Cell		theCell={0,0};
	Rect		dataBounds;
	
	GetListDataBounds(iWinRec->iconList,&dataBounds);
	if (PtInRect(theCell,&dataBounds))
	{
		do {
			LSetSelect(true,theCell,iWinRec->iconList);
		} while (LNextCell(true,true,&theCell,iWinRec->iconList));
	}
}

/* アイコンリストウィンドウの取り消し処理 */
void DoUndoIconList(WindowPtr iWindow)
{
	IconListWinRec	*iWinRec=GetIconListRec(iWindow);
	short	undoMode=iWinRec->undoData.undoMode;
	MyIconResRec	iconInfo;
	IconListDataRec	*iconDataPtr;
	IPIconRec		ipIcon;
	OSErr	err;
	Str255	iconName;
	IconListDataRec	temp;
	Cell			theCell={0,0};
	short			dataSize=sizeof(IconListDataRec *);
	short	i;
	
	switch (undoMode)
	{
		case umCannot:
			{
				MenuHandle	menu;
				
				menu=GetMenuHandle(mEdit);
				MyDisableMenuItem(menu,iUndo);
				return;
			}
			break;
		
		case umAddIcon:
		case -umDeleteIcon:
			/* 削除情報リソースを追加するのみ */
			UseResFile(iWinRec->tempRefNum);
			for (i=0; i<iWinRec->undoData.iconNum; i++)
				CreateDeleteInfo(iWinRec->undoData.iconList[i].resID);
			UpdateResFile(iWinRec->tempRefNum);
				
			UseResFile(gApplRefNum);
			
			UpdateIconList(iWinRec,kDeleteIconCommand,iWinRec->undoData.iconNum,
					iWinRec->undoData.iconList);
			break;
		
		case -umAddIcon:
		case umDeleteIcon:
			/* 削除情報リソースを削除 */
			UseResFile(iWinRec->tempRefNum);
			for (i=0; i<iWinRec->undoData.iconNum; i++)
				RemoveDeleteInfo(iWinRec->undoData.iconList[i].resID);
			UpdateResFile(iWinRec->tempRefNum);
			
			UseResFile(gApplRefNum);
			
			for (i=0; i<iWinRec->undoData.iconNum; i++)
			{
				iconDataPtr=(IconListDataRec *)NewPtr(sizeof(IconListDataRec));
				*iconDataPtr=iWinRec->undoData.iconList[i];
				
				AddIconToIconList(iWinRec,iconDataPtr,true);
			}
			break;
		
		case umChangeIconInfo:
		case -umChangeIconInfo:
			/* 情報の変更 */
			{
				short	id0,id1;
				
				id0 = iWinRec->undoData.iconList[0].resID;
				id1 = iWinRec->undoData.iconList[1].resID;
				/* id0 変更後
				   id1 変更前 */
				
				err=MyGetIPIcon(iWinRec,&ipIcon,&iWinRec->undoData.iconList[0],iconName,NULL);
				
				UseResFile(iWinRec->tempRefNum);
				if (id0 != id1) /* IDが異なる */
				{
					CreateDeleteInfo(iWinRec->undoData.iconList[0].resID);
					CreateUpdateInfo(iWinRec->undoData.iconList[1].resID);
				}
				
				if (iWinRec->undoData.iconList[0].resType == kIconFamilyType)
				{
					Handle	resHandle;
					
					resHandle = Get1Resource(kIconFamilyType,id0);
					SetResInfo(resHandle,id1,iWinRec->undoData.name);
					ChangedResource(resHandle);
					ReleaseResource(resHandle);
				}
				else
				{
					IconActionUPP	iconActionUPP;
					
					iconInfo.resID=id1;
					PStrCpy(iWinRec->undoData.name,iconInfo.resName);
					iconInfo.attrs=resPurgeable;
					
					iconActionUPP=NewIconActionUPP(ChangeIconInfo);
					err=ForEachIconDo(ipIcon.iconSuite,GetMySelector(),iconActionUPP,&iconInfo);
					DisposeIconActionUPP(iconActionUPP);
					
					UpdateResFile(iWinRec->tempRefNum);
				}
				
				/* アイコンを破棄 */
				err=DisposeIPIcon(&ipIcon);
				
				UseResFile(gApplRefNum);
				
				dataSize=sizeof(IconListDataRec *);
				IDToCell(id0,&theCell,iWinRec->iconList);
				LGetCell(&iconDataPtr,&dataSize,theCell,iWinRec->iconList);
				iconDataPtr->resID=id1;
				
				LSetDrawingMode(false,iWinRec->iconList);
				SortIconList(iWinRec->iconList);
				
				IDToCell(id1,&theCell,iWinRec->iconList);
				LSetSelect(true,theCell,iWinRec->iconList);
				LScrollToCell(theCell,iWinRec->iconList);
				
				LSetDrawingMode(true,iWinRec->iconList);
				
				/* 取り消し用のデータを入れ替える */
				temp=iWinRec->undoData.iconList[0];
				iWinRec->undoData.iconList[0]=iWinRec->undoData.iconList[1];
				iWinRec->undoData.iconList[1]=temp;
				PStrCpy(iconName,iWinRec->undoData.name);
			}
			break;
	}
	
	iWinRec->undoData.undoMode = -undoMode;
	
	MyInvalWindowPortBounds(iWindow);
	
	if (!iWinRec->wasChanged)
	{
		iWinRec->wasChanged=true;
		UpdateSaveMenu();
	}
	
	gUsedCount.undoNum++;
	
	UpdateClipMenu();
	UpdateUndoMenu();
	UpdateIconMenu();
	RedrawIconPreview();
}

/* アイコンが選択されているかどうか */
Boolean IsIconSelected(IconListWinRec *iWinRec)
{
	ListHandle	iconList;
	Cell		theCell={0,0};
	
	iconList=iWinRec->iconList;
	return (LGetSelect(true,&theCell,iconList));
}

/* 複数アイコンが選択されているかどうか */
short IsMultiIconSelected(IconListWinRec *iWinRec)
{
	ListHandle	iconList;
	Cell		theCell={0,0};
	short		result=0;
	
	iconList=iWinRec->iconList;
	while (LGetSelect(true,&theCell,iconList))
	{
		result++;
		LNextCell(true,true,&theCell,iconList);
	}
	
	return result;
}

/* 選択されているアイコンのデータを得る */
OSErr GetSelectedIconData(IconListWinRec *iWinRec,IconListDataRec **data)
{
	ListHandle	iconList;
	Cell		theCell={0,0};
	short		dataLen=sizeof(IconListDataRec *);
	
	iconList=iWinRec->iconList;
	if (!LGetSelect(true,&theCell,iconList)) return -1;
	
	LGetCell(data,&dataLen,theCell,iconList);
	return noErr;
}

/* アイコンリストを更新する */
void UpdateIconList(IconListWinRec *iWinRec,short command,short idNum,IconListDataRec *iconList)
{
	WindowPtr	iconWindow=GetListWindow(iWinRec->iconList);
	GrafPtr		port;
	ListHandle	newIconList;
	Rect		viewRect;
	Rect		dataBounds;
	Point		cellSize={kIconListHeight,kIconListWidth};
	IconListRec	*iconListPtr=(IconListRec *)GetListRefCon(iWinRec->iconList),*newListPtr;
	short		width,height;
	short		i,j,k;
	Cell		cell;
	IconListDataRec	*iconDataPtr,*newDataPtr;
	IconListDataRec	**temp;
	short		iconNum;
	Rect		tempRect;
	
	GetWindowPortBounds(iconWindow,&viewRect);
	viewRect.right-=kScrollBarWidth;
	
	width=(viewRect.right)/kIconListWidth;
	height=(iconListPtr->iconNum+width-1)/width;
	SetRect(&dataBounds,0,0,width,0);
	
	#if TARGET_API_MAC_CARBON
	{
		ListDefSpec	defSpec;
		OSErr		err;
		
		defSpec.defType=kListDefUserProcType;
		defSpec.u.userProc=NewListDefUPP(MyIconListLDEFProc);
		err=CreateCustomList(&viewRect,&dataBounds,cellSize,&defSpec,iconWindow,false,true,false,true,&newIconList);
	}
	#else
	newIconList=LNew(&viewRect,&dataBounds,cellSize,128,iconWindow,false,true,false,true);
	#endif
	SetListSelectionFlags(newIconList,lNoNilHilite);
	GetControlBounds(GetListVerticalScrollBar(newIconList),&tempRect);
	tempRect.bottom-=kScrollBarHeight;
	SetControlBounds(GetListVerticalScrollBar(newIconList),&tempRect);
	
	temp=(IconListDataRec **)(*(**iWinRec->iconList).cells);
	iconNum=iconListPtr->iconNum;
	k=0;
	for (j=0; j<height; j++)
	{
		LAddRow(1,j,newIconList);
		
		for (i=0; i<width; i++)
		{
			iconDataPtr=temp[k++];
			if (command == kDeleteIconCommand)
			{
				short	l;
				Boolean	flag=true;
				
				while (flag && k<=iconNum)
				{
					Boolean	flag2=false;
					
					for (l=0; l<idNum; l++)
					{
						if (iconDataPtr->resID == iconList[l].resID)
						{
							flag2=true;
							iconListPtr->iconNum--;
							if (k>=iconNum) goto exit;
							iconDataPtr=temp[k++];
							break;
						}
					}
					flag=flag2;
				}
			}
			SetPt(&cell,i,j);
			newDataPtr=(IconListDataRec *)NewPtr(sizeof(IconListDataRec));
			BlockMoveData(iconDataPtr,newDataPtr,sizeof(IconListDataRec));
			LSetCell(&newDataPtr,sizeof(IconListDataRec *),cell,newIconList);
			
			if (k >= iconNum) goto exit;
		}
	}
exit:
	if (iconListPtr->iconNum < iconNum) /* アイコンの数が減っている */
	{
		short	newHeight=(iconListPtr->iconNum+width-1)/width;
		
		if (newHeight != height)
			LDelRow(height-newHeight,newHeight,newIconList);
	}
	
	newListPtr=(IconListRec *)NewPtr(sizeof(IconListRec));
	BlockMoveData(iconListPtr,newListPtr,sizeof(IconListRec));
	(**newIconList).refCon=(long)newListPtr;
	
	/* 古いリストを破棄し、新しいものをつなぐ */
	LDispose(iWinRec->iconList);
	iWinRec->iconList=newIconList;
	LSetDrawingMode(true,iWinRec->iconList);
	
	/* アイコンを選択するなら選択 */
	if (command == kSelectIconCommand)
	{
		Cell	theCell;
		short	i;
		
		for (i=0; i<idNum; i++)
		{
			IDToCell(iconList[i].resID,&theCell,iWinRec->iconList);
			LSetSelect(true,theCell,iWinRec->iconList);
		}
		LScrollToCell(theCell,iWinRec->iconList);
	}
	
	GetPort(&port);
	SetPortWindowPort(iconWindow);
	GetWindowPortBounds(iconWindow,&tempRect);
	EraseRect(&tempRect);
	MyInvalWindowRect(iconWindow,&tempRect);
	SetPort(port);
}

/* リストにアイコンの追加 */
void AddIconToIconList(IconListWinRec *iWinRec,IconListDataRec *newIcon,Boolean redrawFlag)
{
	GrafPtr		port;
	ListHandle	iconList=iWinRec->iconList;
	WindowPtr	iconWindow=GetListWindow(iconList);
	Cell		newCell;
	IconListRec	*iconListPtr=(IconListRec *)GetListRefCon(iconList);
	short		iconNum=iconListPtr->iconNum;
	Rect		dataBounds;
	short		width;
	
	GetListDataBounds(iconList,&dataBounds);
	width=dataBounds.right;
	LSetDrawingMode(false,iconList);
	SetPt(&newCell,iconNum % width,iconNum / width);
	if (newCell.h == 0)
		LAddRow(1,newCell.v,iconList);
	
	LSetCell(&newIcon,sizeof(IconListDataRec *),newCell,iconList);
	iconListPtr->iconNum++;
	SortIconList(iconList);
	
	if (redrawFlag)
	{
		short	i,id;
		Rect	tempRect;
		
		/* 取り消し処理用データをもとに、登録されたものすべてを選択する */
		for (i=0; i<iWinRec->undoData.iconNum; i++)
		{
			id=iWinRec->undoData.iconList[i].resID;
			IDToCell(id,&newCell,iconList);
			LSetSelect(true,newCell,iconList);
		}
		
		IDToCell(newIcon->resID,&newCell,iconList);
		LScrollToCell(newCell,iconList);
		
		LSetDrawingMode(true,iconList);
		
		GetPort(&port);
		SetPortWindowPort(iconWindow);
		GetWindowPortBounds(iconWindow,&tempRect);
		EraseRect(&tempRect);
		MyInvalWindowRect(iconWindow,&tempRect);
		SetPort(port);
	}
}

/* アイコンリストウィンドウのコンテクストメニュー風メニュー */
void MyIconListContextMenu(Point globPt,WindowPtr iconWindow)
{
	enum {
		piEdit=1,
		piCut=3,
		piCopy,
		piDelete,
		piDuplicate=7,
		piInfo=9,
		piExport
	};
	
	MenuHandle	menu=GetMenu(156);
	long		selItem;
	IconListWinRec	*iWinRec=GetIconListRec(iconWindow);
	Boolean		isMultiIconSelected=(IsMultiIconSelected(iWinRec) > 1);
	
	/* メニューアイテムの初期化 */
	if (isMultiIconSelected)
	{
		MyDisableMenuItem(menu,piCut);
		MyDisableMenuItem(menu,piCopy);
		MyDisableMenuItem(menu,piDuplicate);
		MyDisableMenuItem(menu,piInfo);
		MyDisableMenuItem(menu,piExport);
	}
	else
	{
		MyEnableMenuItem(menu,piCut);
		MyEnableMenuItem(menu,piCopy);
		MyEnableMenuItem(menu,piDuplicate);
		MyEnableMenuItem(menu,piInfo);
		MyEnableMenuItem(menu,piExport);
	}
	
	InsertMenu(menu,-1);
	selItem=PopUpMenuSelectWFontSize(menu,globPt.v,globPt.h,0,9);
	if (selItem==0L) return;
	
	DeleteMenu(156);
	ReleaseResource((Handle)menu);
	
	switch(LoWord(selItem))
	{
		case piEdit:
			OpenSelectedIcon(iconWindow);
			break;
		
		case piCut:
			CopySelectedIcon(iWinRec);
		case piDelete:
			DeleteSelectedIcon(iWinRec);
			break;
		
		case piCopy:
			CopySelectedIcon(iWinRec);
			break;
		
		case piDuplicate:
			DuplicateSelectedIcon(iconWindow);
			break;
		
		case piInfo:
			SelectedIconInfo(iWinRec);
			break;
		
		case piExport:
			ExportIconToFolder(iconWindow);
			break;
	}
}

/* アイコンファミリウィンドウのメニュー */
void MyIconListContextMenu2(Point globPt,WindowPtr iconWindow)
{
	enum {
		piPaste=1,
		piAdd=3,
		piImport,
		piResEdit=6
	};
	
	MenuHandle	menu=GetMenu(157);
	long	selItem;
	long	dataSize;
	
	InsertMenu(menu,-1);
	#if TARGET_API_MAC_CARBON
	{
		ScrapRef	scrap;
		OSErr		err;
		
		err=GetCurrentScrap(&scrap);
		err=GetScrapFlavorSize(scrap,kLarge1BitMask,&dataSize);
	}
	#else
	{
		long		offset;
		
		dataSize=GetScrap(0,kLarge1BitMask,&offset);
	}
	#endif
	if (dataSize<=0) MyDisableMenuItem(menu,piPaste);
	else MyEnableMenuItem(menu,piPaste);
	
	selItem=PopUpMenuSelectWFontSize(menu,globPt.v,globPt.h,0,9);
	if (selItem==0L) return;
	
	DeleteMenu(157);
	
	switch (LoWord(selItem))
	{
		case piPaste:
			PasteIcon(iconWindow);
			break;
		
		case piAdd:
			AddNewIcon(iconWindow);
			break;
		
		case piImport:
			ImportIconFromFile(iconWindow);
			break;
		
		case piResEdit:
			OpenWithResEdit(iconWindow);
			break;
	}
}

/* IDからセルを探す（見つかればtrueを返す） */
Boolean IDToCell(short id,Cell *theCell,ListHandle iconList)
{
	ListSearchUPP	lsUPP=NewListSearchUPP(MyIDSearch);
	Boolean		result;
	
	/* セルの初期化 */
	SetPt(theCell,0,0);
	
	result=LSearch(&id,sizeof(short),lsUPP,theCell,iconList);
	
	DisposeListSearchUPP(lsUPP);
	
	return result;
}

/* 指定セルが見える場所までスクロールさせる */
void LScrollToCell(Cell theCell,ListHandle iconList)
{
	short	dh,dv;
	Rect	r;
	
	/* すでに見えている場合はなにもしない */
	GetListVisibleCells(iconList,&r);
	if (PtInRect(theCell,&r)) return;
	
	dv=(theCell.v < r.top ? theCell.v-r.top :
		(theCell.v > r.bottom-1 ? theCell.v-r.bottom+1 : 0));
	dh=(theCell.h < r.left ? theCell.h-r.left :
		(theCell.h > r.right-1 ? theCell.h-r.right : 0));
	
	LScroll(dh,dv,iconList);
}

/* 128から順に未使用のIDを探す */
short MyUniqueID(ListHandle iconList)
{
	return MyContinuousUniqID(iconList,1);
}

/* 128から順に、連続してnum個あいている部分を探す */
short MyContinuousUniqID(ListHandle iconList,short num)
{
	short	uniqID=127,temp=127;
	short	i=0;
	IconListDataRec **iconDataList;
	IconListRec	*iconListPtr;
	short		iconNum;
	
	iconListPtr=(IconListRec *)GetListRefCon(iconList);
	iconNum=iconListPtr->iconNum;
	
	iconDataList=(IconListDataRec **)(*(**iconList).cells);
	
	while (temp - uniqID <= num && i<iconNum)
	{
		uniqID=temp;
		if (iconDataList[i++]->resID > temp)
			temp=iconDataList[i-1]->resID;
	}
	
	if (temp - uniqID > num)
		uniqID++;
	else
		uniqID=temp+1;
	
	return uniqID;
}

/* リスト内のデータ検索（一致する時falseを返す） */
pascal short MyIDSearch(Ptr cellDataPtr,Ptr searchDataPtr,short cellDataLen,short searchDataLen)
{
	IconListDataRec	*cellData;
	short	searchID;
	
	if (cellDataLen != sizeof(IconListDataRec *) || searchDataLen != sizeof(short)) return true;
	cellData=*(IconListDataRec **)cellDataPtr;
	searchID=*(short *)searchDataPtr;
	
	if (cellData->resID == searchID)
		return false;
	else
		return true;
}

/* ドラッグをトラックする */
pascal short MyIconListTrackingHandler(DragTrackingMessage theMessage,WindowPtr theWindow,
										void *handlerRefCon,DragReference theDrag)
{
	MyDragGlobalsRec	*myGlobals=(MyDragGlobalsRec *)handlerRefCon;
	Point		mouse,localMouse;
	DragAttributes	attributes;
	RgnHandle	hiliteRgn;
	Rect		hiliteRect;
	
	GetDragAttributes(theDrag,&attributes);
	
	switch (theMessage) {
		case kDragTrackingEnterHandler:
			break;
		
		case kDragTrackingEnterWindow:
			myGlobals->isInContent=true;
			
			if (!(attributes & kDragInsideSenderWindow))
			{
				myGlobals->canAcceptDrag=IsMyIconListTypeAvailable(theDrag);
				
				if (myGlobals->canAcceptDrag)
				{
					hiliteRgn=NewRgn();
					GetWindowPortBounds(theWindow,&hiliteRect);
					hiliteRect.right-=kScrollBarWidth;
					RectRgn(hiliteRgn,&hiliteRect);
					ShowDragHilite(theDrag,hiliteRgn,true); /* 内側 */
					DisposeRgn(hiliteRgn);
				}
			}
			break;
		
		case kDragTrackingInWindow:
			if (!myGlobals->canAcceptDrag)
				break;
			
			GetDragMouse(theDrag,&mouse,0L);
			localMouse=mouse;
			GlobalToLocal(&localMouse);
			
			if (attributes & kDragHasLeftSenderWindow) {
			}
			break;
		
		case kDragTrackingLeaveWindow:
			if (myGlobals->canAcceptDrag && myGlobals->isInContent) {
				HideDragHilite(theDrag);
			}
			myGlobals->canAcceptDrag=false;
			break;
		
		case kDragTrackingLeaveHandler:
			break;
	}
	
	return (noErr);
}

/* なにがドラッグされているかをチェックする */
Boolean IsMyIconListTypeAvailable(DragReference theDrag)
{
	short			index;
	unsigned short	items;
	FlavorFlags		theFlags;
	ItemReference	theItem;
	OSErr			result;
	
	CountDragItems(theDrag,&items);
	
	for (index=1; index<=items; index++) {
		GetDragItemReferenceNumber(theDrag,index,&theItem);
		
		/* 'hfs ' flavorの存在をチェック */
		result=GetFlavorFlags(theDrag,theItem,kDragFlavorTypeHFS,&theFlags);
		if (result==noErr) continue;
		
		#if !TARGET_API_MAC_CARBON
		/* 'suit' flavorの存在をチェック */
		result=GetFlavorFlags(theDrag,theItem,'suit',&theFlags);
		if (result==noErr) continue;
		#endif
		
		/* 'icns' flavorの存在チェック */
		if (isIconServicesAvailable)
		{
			result=GetFlavorFlags(theDrag,theItem,kIconFamilyType,&theFlags);
			if (result==noErr) continue;
		}
		
		return false;
	}
	
	return true;
}

/* ドラッグの受け入れ */
pascal short MyIconListReceiveHandler(WindowPtr theWindow,void *handlerRefCon,
										DragReference theDrag)
{
	MyDragGlobalsRec	*myGlobals=(MyDragGlobalsRec *)handlerRefCon;
	Point			mouse;
	short			index;
	unsigned short	items;
	ItemReference	theItem;
	FlavorFlags		theFlags;
	Size			dataSize;
	HFSFlavor		hfsFlavorData;
	OSErr			result;
	short			id=128;
	IconListWinRec	*iWinRec=GetIconListRec(theWindow);
	
	if (!myGlobals->canAcceptDrag) return -2;
	myGlobals->isInContent=false;
	myGlobals->canAcceptDrag=false;
	
	GetDragMouse(theDrag,&mouse,0L);
	HideDragHilite(theDrag);
	
	CountDragItems(theDrag,&items);
	
	if (gOtherPrefs.continuousIDs)
		id=MyContinuousUniqID(iWinRec->iconList,items);
	
	DisposePtr((Ptr)iWinRec->undoData.iconList);
	iWinRec->undoData.iconList=(IconListDataRec *)NewPtr(sizeof(IconListDataRec)*items);
	iWinRec->undoData.iconNum=0; /* まず、0に初期化 */
	
	for (index=1; index<=items; index++)
	{
		GetDragItemReferenceNumber(theDrag,index,&theItem);
		
		result=GetFlavorFlags(theDrag,theItem,kDragFlavorTypeHFS,&theFlags);
		if (result==noErr)
		{
			GetFlavorDataSize(theDrag,theItem,kDragFlavorTypeHFS,&dataSize);
			GetFlavorData(theDrag,theItem,kDragFlavorTypeHFS,(char *)&hfsFlavorData,&dataSize,0L);
			
			/* IDを記録 */
			if (!gOtherPrefs.continuousIDs) id=MyUniqueID(iWinRec->iconList);
			
			/* ファイルのアイコンを取得してリストに追加する */
			AddFileIconToIconList(theWindow,&hfsFlavorData.fileSpec,id++,(index == items));
		}
		else
		{
			if (isIconServicesAvailable)
			{
				result = GetFlavorFlags(theDrag,theItem,kIconFamilyType,&theFlags);
				if (result == noErr)
				{
					IconFamilyHandle	iconFamily;
					IPIconRec			ipIcon;
					
					/* IconFamilyHandleを取り出す */
					GetFlavorDataSize(theDrag,theItem,kIconFamilyType,&dataSize);
					iconFamily=(IconFamilyHandle)NewHandle(dataSize);
					HLock((Handle)iconFamily);
					GetFlavorData(theDrag,theItem,kIconFamilyType,(char *)*iconFamily,&dataSize,0L);
					HUnlock((Handle)iconFamily);
					
					/* IDを記録 */
					if (!gOtherPrefs.continuousIDs) id=MyUniqueID(iWinRec->iconList);
					
					IconFamilyToIPIcon(iconFamily,&ipIcon);
					DisposeHandle((Handle)iconFamily);
					
					AddIPIconToList(theWindow,&ipIcon,(ResType)NULL,"\p",id++,(index == items));
					
					continue;
				}
			}
			#if !TARGET_API_MAC_CARBON
			result=GetFlavorFlags(theDrag,theItem,'suit',&theFlags);
			if (result==noErr)
			{
				MyIconListDragRec	dragRec;
				IPIconRec			ipIcon;
				
				GetFlavorDataSize(theDrag,theItem,'suit',&dataSize);
				GetFlavorData(theDrag,theItem,'suit',(char *)&dragRec,&dataSize,0L);
				
				/* IDを記録 */
				if (!gOtherPrefs.continuousIDs) id=MyUniqueID(iWinRec->iconList);
				
				ipIcon.iconSuite = dragRec.iconSuite;
				ipIcon.it32Data = NULL;
				ipIcon.t8mkData = NULL;
				
				AddIPIconToList(theWindow,&ipIcon,NULL,dragRec.iconName,id++,(index == items));
			}
			#endif
		}
	}
			
	/* ウィンドウをアクティブにする設定なら、する */
	if (gOtherPrefs.activateDroppedWindow)
	{
		SelectReferencedWindow(theWindow);
		UpdateMenus();
	}
	else
		UpdateIconMenu();
	return noErr;
}

/* ドラッグを開始する（ドラッグのデータを作成） */
void MyDoStartDragIcon(WindowPtr iconWindow,EventRecord *theEvent)
{
	IconListWinRec		*iWinRec=GetIconListRec(iconWindow);
	
	DragReference   theDrag;
	RgnHandle       dragRegion=NULL;
	OSErr	err;
	DragSendDataUPP	dsdUPP;
	
	iWinRec->cmdDown=((theEvent->modifiers & cmdKey)!=0);
	
	dsdUPP=NewDragSendDataUPP(MySendIconDataProc);
	if (dsdUPP==0) return;
	
	err = NewDrag(&theDrag);
	if (err!=noErr)
		goto exit;
	
	err = MyDoAddIconFlavors(iconWindow, theDrag);
	if (err!=noErr)
		goto exit;
	
	dragRegion = NewRgn();
	err = MyGetDragIconRegion(iconWindow, dragRegion, theDrag);
	if (err!=noErr)
		goto exit;
	
	err = SetDragSendProc(theDrag, dsdUPP, iWinRec);
	if (err!=noErr)
		goto exit;
	
	err = TrackDrag(theDrag, theEvent, dragRegion);
	
exit:
	DisposeRgn(dragRegion);
	DisposeDrag(theDrag);
	DisposeDragSendDataUPP(dsdUPP);
}

/* ドラッグデータにflavorを追加する */
OSErr MyDoAddIconFlavors(WindowPtr iconWindow,DragReference theDrag)
{
	OSErr	err=noErr;
	IconListWinRec	*iWinRec=GetIconListRec(iconWindow);
	Cell	theCell={0,0};
	short	i=0;
	
	while (LGetSelect(true,&theCell,iWinRec->iconList))
	{
		i++;
		
		/* PromiseHFSを追加 */
		err=AddDragItemFlavorTypePromiseHFS(theDrag,i,kDragPseudoFileTypeDirectory,'MACS',0L,'fssP');
		if (err==noErr)
		{
			#if !TARGET_API_MAC_CARBON
			err=AddDragItemFlavor(theDrag,i,'suit',0L,0L,0);
			if (err==noErr && isIconServicesAvailable)
			#else
			if (err==noErr)
			#endif
				err=AddDragItemFlavor(theDrag,i,kIconFamilyType,0L,0L,0);
		}
		LNextCell(true,true,&theCell,iWinRec->iconList);
	}
	
	return err;
}

/* ドラッグリージョンの作成 */
OSErr MyGetDragIconRegion(WindowPtr iconWindow,RgnHandle dragRegion,
							DragReference theDragRef)
{
	#pragma unused(theDragRef)
	IconListWinRec		*iWinRec=GetIconListRec(iconWindow);
	RgnHandle		tempRgn;
	Point			globalPoint={0,0};
	Rect			cellRect;
	Cell			theCell={0,0};
	
	tempRgn=NewRgn();
	
	LocalToGlobal(&globalPoint);
	
	while(LGetSelect(true,&theCell,iWinRec->iconList))
	{
		LRect(&cellRect,theCell,iWinRec->iconList);
		
		SetRect(&cellRect,cellRect.left+12,cellRect.top+8,
			cellRect.left+32+12,cellRect.top+32+8);
	
		RectRgn(tempRgn,&cellRect);
		UnionRgn(tempRgn,dragRegion,dragRegion);
		
		LNextCell(true,true,&theCell,iWinRec->iconList);
	}
	
	CopyRgn(dragRegion,tempRgn);
	InsetRgn(tempRgn,1,1);
	DiffRgn(dragRegion,tempRgn,dragRegion);
	
	OffsetRgn(dragRegion,globalPoint.h,globalPoint.v);
	
	DisposeRgn(tempRgn);
	return(noErr);
}

/* 要求されたflavorデータを提供する */
pascal short MySendIconDataProc(FlavorType theType,void *dragSendRefCon,
								ItemReference theItem,DragReference theDrag)
{
	OSErr	err=noErr;
	FSSpec	spec;
	IconListWinRec	*iWinRec;
	
	Cell	theCell={0,0};
	short	dataLen=sizeof(IconListDataRec *);
	IconListDataRec	*iconDataPtr;
	short	id,i=1;
	
	IPIconRec	ipIcon;
	Str255	iconName;
	MyIconListDragRec	dragRec;
	
	iWinRec=dragSendRefCon;
	
	while (LGetSelect(true,&theCell,iWinRec->iconList))
	{
		if (i==theItem) break;
		LNextCell(true,true,&theCell,iWinRec->iconList);
		i++;
	}
	LGetCell(&iconDataPtr,&dataLen,theCell,iWinRec->iconList);
	id=iconDataPtr->resID;
	
	switch (theType) {
		case 'fssP':
			/* ファイルがドラッグされた位置をべる */
			err=GetDropDirectory(theDrag,&spec);
			if (err==noErr)
			{
				long	droppedDirID;
				Boolean	isDirectory;
				short	length;
				#ifdef __MOREFILESX__
				FSRef	fsRef;
				
				err = FSpMakeFSRef(&spec,&fsRef);
				err = FSGetNodeID(&fsRef,&droppedDirID,&isDirectory);
				#else
				
				err=FSpGetDirectoryID(&spec,&droppedDirID,&isDirectory);
				#endif
				if (err==noErr)
				{
					Str255	str;
					FSSpec	theIconFile;
					short	vRefNum;
					long	dirID;
					short	count=0;
					short	l;
					
					/* ごみ箱かどうかチェック */
					err=FindFolder(GetFindFolderVRefNum(),kTrashFolderType,kCreateFolder,&vRefNum,&dirID);
					if (vRefNum==spec.vRefNum && droppedDirID==dirID)
					{
						/* ごみ箱 */
						/* 汚い方法だが…… */
						if (theItem == 1) DeleteSelectedIcon(iWinRec);
						break;
					}
					
					/* アイコンを取得しておく */
					err=MyGetIPIcon(iWinRec,&ipIcon,iconDataPtr,iconName,NULL);
					
					/* フォルダあるいはアイコンの名前を決める */
					if (iconName[0]==0)
					{
						UseResFile(gApplRefNum);
						PStrCpy(iWinRec->iconFileSpec.name,iconName);
						GetIndString(str,140,2);
						PStrCat(str,iconName);
						NumToString(id,str);
						PStrCat(str,iconName);
						UseResFile(iWinRec->tempRefNum);
					}
					
					length=(iWinRec->cmdDown ? 27 : 31);
					
					TruncPString(iconName,iconName,length);
					ReplaceString(iconName,"\p-","\p:");
					l=iconName[0];
					
					if (iWinRec->cmdDown)
						PStrCat("\p.ico",iconName);
					
					UseResFile(gApplRefNum);
					
					do
					{
						err=FSMakeFSSpec(spec.vRefNum,droppedDirID,iconName,&theIconFile);
						if (err==fnfErr)
						{
							if (iWinRec->cmdDown)
								err=MakeWinIconFromSuite(&theIconFile,ipIcon.iconSuite);
							else
								err=MakeFolderWithIPIcon(&theIconFile,&ipIcon);
							break;
						}
						else
						{
							iconName[0]=l;
							NumToString(++count,str);
							TruncPString(iconName,iconName,length-str[0]-1);
							CatChar(' ',iconName);
							PStrCat(str,iconName);
							if (iWinRec->cmdDown)
								PStrCat("\p.ico",iconName);
						}
					} while (err==noErr);
					err=DisposeIPIcon(&ipIcon);
				}
			}
			break;
		
		#if !TARGET_API_MAC_CARBON
		case 'suit':
			/* IconSuiteを渡す */
			{
				IconActionUPP	detachIconUPP;
				
				err=MyGetIPIcon(iWinRec,&ipIcon,iconDataPtr,dragRec.iconName,NULL);
				dragRec.iconSuite = ipIcon.iconSuite;
				
				detachIconUPP=NewIconActionUPP(DetachIcon);
				err=ForEachIconDo(dragRec.iconSuite,GetMySelector(),detachIconUPP,nil);
				DisposeIconActionUPP(detachIconUPP);
				err=DisposeIPIcon(&ipIcon);
				
				err=SetDragItemFlavorData(theDrag,theItem,'suit',&dragRec,sizeof(MyIconListDragRec),0L);
			}
			break;
		#endif
		
		case kIconFamilyType:
			/* IconFamilyHandleを渡す */
			if (isIconServicesAvailable)
			{
				IconFamilyHandle	iconFamily;
				
				err=MyGetIPIcon(iWinRec,&ipIcon,iconDataPtr,dragRec.iconName,NULL);
				err=IPIconToIconFamily(&ipIcon,&iconFamily);
				err=DisposeIPIcon(&ipIcon);
				
				HLock((Handle)iconFamily);
				err=SetDragItemFlavorData(theDrag,theItem,kIconFamilyType,*iconFamily,
					GetHandleSize((Handle)iconFamily),0L);
				HUnlock((Handle)iconFamily);
				
				DisposeHandle((Handle)iconFamily);
			}
			break;
		
		default:
			err=badDragFlavorErr;
			break;
	}
	
	return err;
}

/* PromiseHFSFlavorを追加 */
OSErr AddDragItemFlavorTypePromiseHFS(DragReference dragRef, ItemReference itemRef,
										OSType fileType, OSType fileCreator,
										UInt16 fdFlags, FlavorType promisedFlavor)
{
	OSErr err = noErr;
	
	PromiseHFSFlavor phfs;
	
	phfs.fileType = fileType;
	phfs.fileCreator = fileCreator;
	phfs.fdFlags = fdFlags;
	phfs.promisedFlavor = promisedFlavor;
	
	if (!(err = AddDragItemFlavor(dragRef, itemRef, flavorTypePromiseHFS,
									&phfs, sizeof(phfs), flavorNotSaved)))
	{
		err = AddDragItemFlavor(dragRef, itemRef, promisedFlavor, nil, 0, flavorNotSaved);
	}

	return err;
}

/* ResEditに託す */
void OpenWithResEdit(WindowPtr iWindow)
{
	IconListWinRec	*iWinRec=GetIconListRec(iWindow);
	FSSpec	theIconFile=iWinRec->iconFileSpec;
	
	if (CloseIconFile(iWindow,false) != noErr) return;
	
	AEOpenFileWithResEdit(&theIconFile);
	
	UpdateMenus();
}

/* ResEditでファイルを開いてもらう */
void AEOpenFileWithResEdit(FSSpec *theIconFile)
{
	ProcessSerialNumber	psn;
	OSErr		err;
	Boolean		found;
	
	AppleEvent	aeEvent={typeNull,NULL};
	
	/* ResEditが起動しているかどうかを探す */
	found = FindProcessFromCreatorAndType(kResEditCreator,kResEditType,&psn);
	if (found)
	{
		/* 見つかった場合は見つかったアプリケーションにイベントを送信 */
		err=MakeOpenDocumentEvent(&psn,theIconFile,&aeEvent);
		if (err==noErr)
			/* Appleイベントを送付する */
			err=AESend(&aeEvent,nil,kAENoReply+kAECanSwitchLayer+kAEAlwaysInteract,
						kAENormalPriority,kNoTimeOut,nil,nil);
	
		/* 確保したデスクリプタを破棄する */
		err=AEDisposeDesc(&aeEvent);
	}
	else
	{
		/* 見つからない場合はLaunchApplicationで起動させる */
		FSSpec				resEditSpec;
		
		/* ResEditを探す */
		err=GetApplSpec(kResEditCreator,&resEditSpec);
		if (err!=noErr) return;
		
		AEOpenFileWithApplication(theIconFile,&resEditSpec);
	}
	
	if (found)
		err=SetFrontProcess(&psn);
}

#if 0
/* Finderにodocイベントを送り、ResEditで開いてもらう */
void AEOpenFileInFinderWithResEdit(FSSpec *theIconFile)
{
	AliasHandle	fileAlias;
	FSSpec		resEditSpec;
	
	ProcessSerialNumber	psn;
	OSErr		err;
	
	AppleEvent	aeEvent={typeNull,NULL};
	AEDesc		resEditDesc={typeNull,NULL};
	
	/* Finderを探す */
	if (!FindProcessFromCreatorAndType(kFinderCreator,kFinderType,&psn)) return;
	
	/* odocイベントを作成 */
	err=MakeOpenDocumentEvent(&psn,theIconFile,&aeEvent);
	
	/* ResEditを探す */
	err=GetApplSpec(kResEditCreator,&resEditSpec);
	err=NewAlias(nil,&resEditSpec,&fileAlias);
	if (err==noErr)
	{
		HLock((Handle)fileAlias);
		err=AECreateDesc(typeAlias,(Ptr)*fileAlias,(*fileAlias)->aliasSize,&resEditDesc);
		HUnlock((Handle)fileAlias);
		DisposeHandle((Handle)fileAlias);
	}
	
	/* 親フォルダのデスクリプタをkeyAEUsingに設定する */
	err=AEPutParamDesc(&aeEvent,keyAEUsing,&resEditDesc);
	
	/* Appleイベントを送付する */
	err=AESend(&aeEvent,nil,kAENoReply+kAEAlwaysInteract,
				kAENormalPriority,kNoTimeOut,nil,nil);//+kAECanSwitchLayer
	
	/* 確保したデスクリプタを破棄する */
	err=AEDisposeDesc(&resEditDesc);
	err=AEDisposeDesc(&aeEvent);
}

#endif
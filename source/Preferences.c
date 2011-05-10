/* ------------------------------------------------------------ */
/*  Preferences.c                                               */
/*     初期設定ファイルの処理                                   */
/*                                                              */
/*                 1997.1.28 - 2001.2.3  naoki iimura        	*/
/* ------------------------------------------------------------ */

/* includes */
#ifdef __APPLE_CC__
#include	<Carbon/Carbon.h>
#else
#include	<Navigation.h>
#include	<Folders.h>
#include	<TextUtils.h>
#include	<Resources.h>
#include	<StandardFile.h>
#endif

#include	"Globals.h"
#include	"IconParty.h"
#include	"MenuRoutines.h"
#include	"UsefulRoutines.h"
#include	"Preferences.h"
#include	"WindowRoutines.h"
#include	"PaintRoutines.h"
#include	"EffectRoutines.h"
#include	"PreCarbonSupport.h"
#include	"NavigationServicesSupport.h"
#include	"UpdateCursor.h"

/* prototypes */
static void	OpenPrefFile(void);
static short	SavePrefFileMain(void);
static void	ClosePrefFile(void);

static OSErr	RemoveDataFromPrefs(ResType type,short id);

static OSErr	GetCreatorFromFile(OSType *creator);

static void	UpdateBGSetting(void);
static OSErr SetExternalApplication(FSSpec *theFile);
#if !TARGET_API_MAC_CARBON
static pascal Boolean MyGetFileModalFilter(DialogPtr theDialog,EventRecord *theEvent,short *itemHit,void *yourDataPtr);
#endif
static pascal Boolean MyConfigureGridFilter(DialogPtr theDialog,EventRecord *theEvent,short *theItemHit);


#define	PREFERR_RESID	4004
#define	PREFERR1	1
#define	PREFERR2	2
#define	PREFERR3	3
#define	PREFERR4	4


/* variables */
static short	prevTab=kFilePrefsTab;


/* 初期設定を開く */
void OpenPrefFile(void)
{
	Str255	prefFileName;
	OSErr	err;
	FSSpec	prefFileSpec;
	short	vRefNum;
	long	dirID;
	
	/* 「初期設定」フォルダのディレクトリIDを得る */
	err=FindFolder(GetFindFolderVRefNum(),kPreferencesFolderType,kCreateFolder,&vRefNum,&dirID);
	if (err!=noErr)
	{
		ErrorAlertFromResource(PREFERR_RESID,PREFERR1);
		return;
	}
	
	GetIndString(prefFileName,sPrefFileString,1);
	err=FSMakeFSSpec(vRefNum,dirID,prefFileName,&prefFileSpec);
	if (err==fnfErr) /* 初期設定ファイルがなければ作成する */
	{
		FSpCreateResFile(&prefFileSpec,kIconPartyCreator,kPreferencesFileType,smSystemScript);
	}
	
	gPrefFileRefNum=FSpOpenResFile(&prefFileSpec,fsRdWrPerm);
}

/* 初期設定の読み込み */
void LoadPrefFile(void)
{
	OSErr	err;
	AliasHandle	theAlias;
	
	/* グリッドモード */
	gToolPrefs.gridMode=2*16+1; /* none */
		/* bit 0~3 : 1=none,2=gray,3=dot,4=white */
		/* bit 4~7 : 0=none,1=icon32, 2=icon16 */
	gToolPrefs.gridColor=rgbBlackColor;
	gToolPrefs.gridColor2=rgbBlackColor;	/* 32*32 or 16*16 grid */
	
	/* ルーラーモード */
	gToolPrefs.showRuler=false; /* ルーラーなし */
	
	/* パレットのチェック */
	gPaletteCheck=kPaletteCheckNone;
	
	/* PNGファイルの設定 */
	gPNGFilePrefs.interlaced=true;		/* インタレース */
	gPNGFilePrefs.transColor=0;			/* 透過色なし */
	gPNGFilePrefs.useClip2gif=false;	/* clip2gifは使用しない */
	gPNGFilePrefs.compLevel=6;			/* 圧縮レベル6 */
	
	gPNGFilePrefs.optimizeImage=true;	/* 色数にあわせて最適化 */
	gPNGFilePrefs.useTransColor.inSaving=false;
	gPNGFilePrefs.useTransColor.inLoading=false;
	
	/* ペイント設定 */
	gToolPrefs.eraseSameColor=true;
	gToolPrefs.eraseSize11=true;
	gToolPrefs.selectionMasking=true; /* v1.1変更 */
	gToolPrefs.dotDrawPrefs=gKeyThreshStore;
	gToolPrefs.changeSpoitCursor=false;
	gToolPrefs.changePencilCursor=false;
	gToolPrefs.useColorCursor=false;
	gToolPrefs.eraserByCmdKey=false;
	
	/* クリエータコード */
	gPICTCreator=kIconPartyCreator;	/* IconParty */
	gGIFCreator=kIconPartyCreator;	/* IconParty */
	if (isOSX)
		gIconCreator=kIconPartyCreator;
	else
		gIconCreator=kResEditCreator; /* ResEdit */
	gPNGCreator=kIconPartyCreator;
	
	/* 起動時動作設定 */
	gStartup=1; /* 新規ウィンドウを表示 */
	
	/* プレビューアイコンを追加 */
	gPreviewIconFlag=0; /* 追加しない */
	
	/* タブレット設定 */
	gTabletPrefs.usePressure=false; /* 1.1b2変更 */
	gTabletPrefs.overrideTransparency=true;
	gTabletPrefs.useEraser=false; /* 1.1b2変更 */
	gTabletPrefs.useEraserPressure=false;
	
	SetRect(&gPaintWinPrefs.iconSize,0,0,96,64);
	gPaintWinPrefs.ratio=2;	/* ４倍 */
	gPaintWinPrefs.askSize=false;	/* サイズは毎回聞かない */
	gPaintWinPrefs.referClip=false; /* don't refer clipboard */
	gPaintWinPrefs.background=0; /* 白 */
	gPaintWinPrefs.colorMode = kNormal8BitColorMode;
	
	/* アイコンリストの背景 */
	gListBackground=iBackWhite;
	
	#if TARGET_API_MAC_CARBON
	useNavigationServices=true;
	#else
	useNavigationServices=false;
	#endif
	
	/* その他の設定 */
	gOtherPrefs.checkWhenColorChanged=false;
	gOtherPrefs.activateDroppedWindow=false;
	gOtherPrefs.maskAutoConvert=false;
	gOtherPrefs.continuousIDs=false;
	gOtherPrefs.addForeground=true; /* v1.1変更 */
	gOtherPrefs.copyIconWithPicture=true;
	gOtherPrefs.useHyperCardPalette=false;
	
	gOtherPrefs.importAsFamilyIcon=false; /* デフォルトでは、ばらばらに取り込む */
	
	/* 使用回数 */
	gUsedCount.usedCount=0;
	gUsedCount.usedTime=0;
	gUsedCount.newNum=gUsedCount.openNum=gUsedCount.saveNum=0;
	gUsedCount.fillNum=gUsedCount.lightenNum=gUsedCount.darkenNum=gUsedCount.antialiasNum=gUsedCount.dotPictureNum-0;
	gUsedCount.windNum=gUsedCount.edgeNum=gUsedCount.colorChangeNum=gUsedCount.rotateNum=gUsedCount.flipNum=0;
	gUsedCount.undoNum=0;
	gUsedCount.importNum=0;
	gUsedCount.exportNum=0;
	gUsedCount.showCountNum=0;
	
	/* 描画点関係 */
	gDotCommand[0]=0;
	gDotLibName[0]=0;
	
	/* 外部エディタ関係 */
	isExternalEditorAvailable=false;
	gUseExternalEditor=(gSystemVersion >= 0x0850);
	
	OpenPrefFile();
	
	if (gPrefFileRefNum<=0)
	{
		ErrorAlertFromResource(PREFERR_RESID,PREFERR2);
		gPrefFileRefNum=-1;
		return;
	}
	UseResFile(gPrefFileRefNum);
	
	/* パレットのチェックの読み込み */
	err=LoadDataFromPrefs(&gPaletteCheck,sizeof(gPaletteCheck),'cAIC',128);
	#if TARGET_API_MAC_CARBON
	if (isOSX && gPaletteCheck == kPaletteCheckUsed)
		gPaletteCheck = kPaletteCheckNone;
	#endif
	
	/* リストの背景の読み込み */
	err=LoadDataFromPrefs(&gListBackground,sizeof(gListBackground),'lBak',128);
	if (isOSX && gListBackground == iBackDesktop) gListBackground=iBackWhite;
	
	/* PNG設定の読み込み */
	err=LoadDataFromPrefs(&gPNGFilePrefs,sizeof(gPNGFilePrefs),'gSet',128);
	gPNGFilePrefs.useClip2gif=false; /* clip2gifは使用しない */
	
	/* ペイント設定の読み込み */
	err=LoadDataFromPrefs(&gToolPrefs,sizeof(gToolPrefs),'pPrf',128);
	
	/* grid mode */
	gToolPrefs.gridMode &= 0x0037;
	if ((gToolPrefs.gridMode & 0x07) > 4) gToolPrefs.gridMode &= 0x31;
	if ((gToolPrefs.gridMode & 0x30) > 2) gToolPrefs.gridMode &= 0x27;
	
	/* 各ファイルのクリエータコードの読み込み */
	err=LoadDataFromPrefs(&gPICTCreator,sizeof(OSType),'DCrt',128);
	err=LoadDataFromPrefs(&gGIFCreator,sizeof(OSType),'DCrt',129);
	err=LoadDataFromPrefs(&gIconCreator,sizeof(OSType),'DCrt',130);
	err=LoadDataFromPrefs(&gPNGCreator,sizeof(OSType),'DCrt',131);
	
	/* 起動時動作の読み込み */
	err=LoadDataFromPrefs(&gStartup,sizeof(short),'Stup',128);
	
	/* プレビューアイコンを追加するか */
	err=LoadDataFromPrefs(&gPreviewIconFlag,sizeof(short),'fPrf',128);
	
	/* タブレット設定の読み込み */
	err=LoadDataFromPrefs(&gTabletPrefs,sizeof(gTabletPrefs),'tPrf',128);
	
	/* デフォルトサイズ、倍率の読み込み */
	err=LoadDataFromPrefs(&gPaintWinPrefs,sizeof(gPaintWinPrefs),'PWpf',128);
	switch (gPaintWinPrefs.colorMode)
	{
		case kNormal8BitColorMode:
		case k32BitColorMode:
			break;
		
		default:
			gPaintWinPrefs.colorMode = kNormal8BitColorMode;
	}
	
	/* 互換性を保つため */
	err=LoadDataFromPrefs(&gToolPrefs.gridMode,sizeof(gToolPrefs.gridMode),'gMod',128);
	if (err==noErr)
	{
		RemoveDataFromPrefs('gMod',128);
		gToolPrefs.gridMode = ((gToolPrefs.gridMode & 0x0c) << 2) + gToolPrefs.gridMode & 0x03;
	}
	err=LoadDataFromPrefs(&gToolPrefs.showRuler,sizeof(gToolPrefs.showRuler),'rMod',128);
	if (err==noErr) RemoveDataFromPrefs('rMod',128);
	
	err=LoadDataFromPrefs(&gPaintWinPrefs.iconSize,sizeof(gPaintWinPrefs.iconSize),'iSiz',128);
	if (err!=resNotFound) RemoveDataFromPrefs('iSiz',128);
	err=LoadDataFromPrefs(&gPaintWinPrefs.ratio,sizeof(gPaintWinPrefs.ratio),'eRat',128);
	if (err!=resNotFound) RemoveDataFromPrefs('eRat',128);
	
	err=LoadDataFromPrefs(&gPaintWinPrefs.askSize,sizeof(gPaintWinPrefs.askSize),'aSiz',128);
	if (err!=resNotFound) RemoveDataFromPrefs('aSiz',128);
	err=LoadDataFromPrefs(&gPaintWinPrefs.referClip,sizeof(gPaintWinPrefs.referClip),'rClp',128);
	if (err!=resNotFound) RemoveDataFromPrefs('rClp',128);
	
	#if !TARGET_API_MAC_CARBON
	err=LoadDataFromPrefs(&useNavigationServices,sizeof(useNavigationServices),'uNav',128);
	#endif
	
	/* その他の設定 */
	err=LoadDataFromPrefs(&gOtherPrefs,sizeof(gOtherPrefs),'oPrf',128);
	if (gOtherPrefs.useHyperCardPalette)
		gPatternNo=12;
	if (!isIconServicesAvailable)
		gOtherPrefs.importAsFamilyIcon=false;
	
	/* 使用回数の読込 */
	err=LoadDataFromPrefs(&gUsedCount,sizeof(gUsedCount),'uCnt',128);
	gUsedCount.usedCount++;
	
	/* ペイント関係 */
	LoadPaintPrefs();
	
	/* 描画点記録コマンド */
	err=LoadDataFromPrefs(gDotCommand,sizeof(gDotCommand),'STR ',128);
	err=LoadDataFromPrefs(gDotLibName,sizeof(gDotLibName),'STR ',129);
	
	/* 外部エディタ */
	theAlias=(AliasHandle)Get1Resource(rAliasType,128);
	if (theAlias!=nil)
	{
		Boolean	changed;
		
		err=ResolveAlias(nil,theAlias,&gExternalEditor,&changed);
		if (err==noErr)
		{
			isExternalEditorAvailable=true;
			if (changed)
			{
				ChangedResource((Handle)theAlias);
				WriteResource((Handle)theAlias);
			}
			ReleaseResource((Handle)theAlias);
		}
		else
		{
			RemoveResource((Handle)theAlias);
			DisposeHandle((Handle)theAlias);
		}
	}
	err=LoadDataFromPrefs(&gUseExternalEditor,sizeof(gUseExternalEditor),'uExE',128);
	
	/* お気に入りパレット */
	gFavoriteColors=(CTabHandle)Get1Resource('clut',128);
	
	UseResFile(gApplRefNum);
	
	if (gFavoriteColors == NULL)
	{
		gFavoriteColors=(CTabHandle)Get1Resource('clut',130);
		DetachResource((Handle)gFavoriteColors);
		SetHandleSize((Handle)gFavoriteColors,sizeof(ColorTable)+sizeof(ColorSpec)*(34-1));
		(*gFavoriteColors)->ctSize = 34-1;
	}
	else
		DetachResource((Handle)gFavoriteColors);
}

/* 設定の保存 */
void SavePrefFile(void)
{
	short	err;
	
	err=SavePrefFileMain();
	if (err!=noErr) /* エラーがあればその旨を表示 */
	{
		ErrorAlertFromResource(PREFERR_RESID,PREFERR3);
	}
}

/* 設定の保存（メイン） */
short SavePrefFileMain(void)
{
	OSErr	err;
	#if !TARGET_API_MAC_CARBON
	ProcessInfoRec	processInfo;
	ProcessSerialNumber	psn;
	#endif
	
	if (gPrefFileRefNum<=0)
		return -1;
	
	UseResFile(gPrefFileRefNum);
	
	/* リストのバックグラウンド */
	err=AddDataToPrefs(&gListBackground,sizeof(gListBackground),'lBak',128,"\pbackground of icon list");
	
	/* Apple Icon Colorの表示 */
	err=AddDataToPrefs(&gPaletteCheck,sizeof(gPaletteCheck),'cAIC',128,"\pcolor palette check");
	
	/* PNGファイル設定 */
	err=AddDataToPrefs(&gPNGFilePrefs,sizeof(gPNGFilePrefs),'gSet',128,"\ppng file prefs");
	
	/* ペイント初期設定 */
	err=AddDataToPrefs(&gToolPrefs,sizeof(gToolPrefs),'pPrf',128,"\ptool prefs");
	
	/* 書類のクリエータ */
	err=AddDataToPrefs(&gPICTCreator,sizeof(OSType),'DCrt',128,"\ppict creator");
	err=AddDataToPrefs(&gGIFCreator,sizeof(OSType),'DCrt',129,"\pgif creator");
	err=AddDataToPrefs(&gIconCreator,sizeof(OSType),'DCrt',130,"\picon creator");
	err=AddDataToPrefs(&gPNGCreator,sizeof(OSType),'DCrt',131,"\ppng creator");
	
	/* 起動時動作設定 */
	err=AddDataToPrefs(&gStartup,sizeof(short),'Stup',128,"\pstartup setting");
	
	/* プレビューアイコンを追加するか */
	err=AddDataToPrefs(&gPreviewIconFlag,sizeof(short),'fPrf',128,"\padd preview icon");
	
	/* タブレット設定 */
	err=AddDataToPrefs(&gTabletPrefs,sizeof(gTabletPrefs),'tPrf',128,"\ptablet prefs");
	
	/* デフォルトサイズと倍率 */
	err=AddDataToPrefs(&gPaintWinPrefs,sizeof(gPaintWinPrefs),'PWpf',128,"\ppaint window prefs");
	
	#if !TARGET_API_MAC_CARBON
	err=AddDataToPrefs(&useNavigationServices,sizeof(useNavigationServices),'uNav',128,"\puse navigation services");
	#endif
	
	err=AddDataToPrefs(&gOtherPrefs,sizeof(gOtherPrefs),'oPrf',128,"\pother Prefs");
	
	#if !TARGET_API_MAC_CARBON
	GetCurrentProcess(&psn);
	processInfo.processInfoLength=sizeof(ProcessInfoRec);
	processInfo.processName=nil;
	processInfo.processLocation=nil;
	processInfo.processAppSpec=nil;
	GetProcessInformation(&psn,&processInfo);
	gUsedCount.usedTime+=processInfo.processActiveTime/60;
	#endif
	
	err=AddDataToPrefs(&gUsedCount,sizeof(gUsedCount),'uCnt',128,"\pcount of use");
	
	/* ペイント関係 */
	SavePaintPrefs();
	
	/* 描画点記録コマンド */
	err=AddDataToPrefs(gDotCommand,gDotCommand[0]+1,'STR ',128,"\pdot mode command");
	if (gDotLibRefNum > 0)
		err=AddDataToPrefs(gDotLibName,gDotLibName[0]+1,'STR ',129,"\pdot lib name");
	
	/* 外部エディタ */
	if (isExternalEditorAvailable)
	{
		AliasHandle	theAlias;
		
		err=NewAlias(nil,&gExternalEditor,&theAlias);
		if (err==noErr)
		{
			HLock((Handle)theAlias);
			err=AddDataToPrefs(*theAlias,GetHandleSize((Handle)theAlias),rAliasType,128,"\pexternal editor");
			HUnlock((Handle)theAlias);
			DisposeHandle((Handle)theAlias);
		}
		else
			RemoveDataFromPrefs(rAliasType,128);
	}
	else
		RemoveDataFromPrefs(rAliasType,128);
	err=AddDataToPrefs(&gUseExternalEditor,sizeof(gUseExternalEditor),'uExE',128,"\puse external editor");
	
	/* お気に入りパレット */
	err=AddDataToPrefs(*gFavoriteColors,GetHandleSize((Handle)gFavoriteColors),'clut',128,"\pfavorite colors");
	
	UseResFile(gApplRefNum);
	
	if (quit==true)
	{
		ClosePrefFile();
	}
	
	return err;
}

/* データを初期設定に追加する */
OSErr AddDataToPrefs(void *dataPtr,long dataSize,ResType type,short id,Str255 resName)
{
	Handle	resHandle;
	
	resHandle=Get1Resource(type,id);
	if (resHandle==nil)
	{
		resHandle=NewHandle(dataSize);
		AddResource(resHandle,type,id,resName);
		SetResAttrs(resHandle,resPurgeable);
	}
	else
	{
		if (GetHandleSize(resHandle) != dataSize)
			SetHandleSize(resHandle,dataSize);
	}
	
	BlockMoveData(dataPtr,*resHandle,dataSize);
	
	ChangedResource(resHandle);
//	WriteResource(resHandle);
	ReleaseResource(resHandle);
	
	return noErr;
}

/* データを初期設定から読み込む */
OSErr LoadDataFromPrefs(void *dataPtr,long dataSize,ResType type,short id)
{
	Handle	resHandle;
	
	resHandle=Get1Resource(type,id);
	if (resHandle==nil) return resNotFound;
	
	if (dataSize > GetHandleSize(resHandle)) dataSize=GetHandleSize(resHandle);
	BlockMoveData(*resHandle,dataPtr,dataSize);
	ReleaseResource(resHandle);
	
	return noErr;
}

/* リソースを削除する */
OSErr RemoveDataFromPrefs(ResType type,short id)
{
	Handle	resHandle;
	
	resHandle=Get1Resource(type,id);
	if (resHandle==nil) return resNotFound;
	
	RemoveResource(resHandle);
	DisposeHandle(resHandle);
	
	return noErr;
}

/* 初期設定ファイルを閉じる */
void ClosePrefFile(void)
{
	if (gPrefFileRefNum<=0) return;
	
	/* 閉じる前にウィンドウの位置を記録 */
	SaveWindowPos();
	
	CloseResFile(gPrefFileRefNum);
	FlushVol(0,0);
}

/* 初期設定ダイアログの表示 */
void OpenPrefsDialog(void)
{
	DialogPtr	dp;
	Handle		ditlHandle;
	Handle		tabHandle;
	Rect		itemRect;
	short		startItem[6],itemNum[6],tTabNum[6]={1,2,3,4,5,6};
	short		i,j;
	GrafPtr		port;
	
	short	tabNum=1,temp;
	short	item,item2=0;
	
	short	prevStartup;
	ResType	info[4];
	Str255	applName;
	OSType	creator;
	Boolean	prevAsk;
	Boolean	prevUN;
	Boolean	prevPI;
	Str255	str;
	long	l;
	ToolPrefsRec	prevPaintPrefs;
	PNGPrefsRec		prevPNGPrefs;
	TabletPrefsRec	prevTabletPrefs;
	OtherPrefs	prevOtherPrefs;
	Boolean	tempBool;
	Boolean	tempExternalEditorAvailable,tempUseExternalEditor;
	FSSpec	tempExternalEditor,tempSpec;
	
	short	keyThreshValue[]={0x0048,0x0028,0x0018,0x0010,0x000c};
	short	keyRepThreshValue[]={0x0090,0x000c,0x0008,0x0004,0x0002};
	short	value;
	
	ModalFilterUPP	mfUPP=NewModalFilterUPP(MyModalDialogFilter);
	
	UseResFile(gApplRefNum);
	
	DeactivateFloatersAndFirstDocumentWindow();
	dp=GetNewDialog(300,nil,kFirstWindowOfClass);
	if (dp==nil)
	{
		ErrorAlertFromResource(PREFERR_RESID,PREFERR4);
		return;
	}
	
	GetPort(&port);
	SetPortDialogPort(dp);
	MySetCursor(0);
	
	/* タブの調整（タブレットのある／なし） */
	tabNum=prevTab;
	tabHandle=GetDialogItemHandle(dp,3);
	if (!isTabletAvailable)
	{
		Str255	title;
		
		SetControlMaximum((ControlHandle)tabHandle,5);
		GetIndString(title,143,2);
		SetControlTitle((ControlHandle)tabHandle,title);
		
		SetControlValue((ControlHandle)tabHandle,(tabNum > 5 ? 5 : tabNum));
	}
	else
		SetControlValue((ControlHandle)tabHandle,tabNum);
	
	/* アイテムの追加 */
	for (i=0; i<6; i++)
	{
		startItem[i]=CountDITL(dp);
		ditlHandle=Get1Resource('DITL',300+i+1);
		AppendDITL(dp,ditlHandle,overlayDITL);
		ReleaseResource(ditlHandle);
		itemNum[i]=CountDITL(dp)-startItem[i];
		if (i!=prevTab-1)
			for (j=0; j<itemNum[i]; j++)
				HideDialogItem(dp,startItem[i]+j+1);
	}
	
	/* ファイル設定初期化 */
	/* 起動時の処理 */
	prevStartup=gStartup;
	SetDialogControlValue(dp,startItem[0]+diStartupPrefs,prevStartup+1);
	
	/* クリエータ */
	info[0]=gPICTCreator;
	info[1]=gGIFCreator;
	info[2]=gPNGCreator;
	info[3]=gIconCreator;
	
	for (i=0; i<4; i++)
	{
		CreatorToAppl(info[i],applName);
		SetDialogItemText2(dp,startItem[0]+i+diPICTCreatorName,applName);
	}
	
	/* Navigation Services */
	prevUN=useNavigationServices;
	SetDialogControlValue(dp,startItem[0]+diUseNavServices,useNavigationServices);
	
	if (!NavServicesAvailable())
		SetDialogControlHilite(dp,startItem[0]+diUseNavServices,itemInactive);
	
	/* プレビューアイコン追加 */
	prevPI=gPreviewIconFlag;
	SetDialogControlValue(dp,startItem[0]+diPreviewIconFlag,prevPI);
	
	/* ペイント設定 */
	/* アイコンの大きさ */
	NumToString(gPaintWinPrefs.iconSize.right,str);
	SetDialogItemText2(dp,startItem[1]+diImageWidth,str);
	NumToString(gPaintWinPrefs.iconSize.bottom,str);
	SetDialogItemText2(dp,startItem[1]+diImageHeight,str);
	
	/* カラーモード */
	if (gPaintWinPrefs.colorMode == kNormal8BitColorMode)
		SetDialogControlValue(dp,startItem[1]+diImageColorMode,1);
	else
		SetDialogControlValue(dp,startItem[1]+diImageColorMode,2);
	
	/* 倍率 */
	SetDialogControlValue(dp,startItem[1]+diImageRatio,gPaintWinPrefs.ratio+1);
	
	/* サイズを毎回聞くかどうか */
	prevAsk=gPaintWinPrefs.askSize;
	SetDialogControlValue(dp,startItem[1]+diAskSize,prevAsk);
	
	/* 背景 */
	SetDialogControlValue(dp,startItem[1]+diBackground,gPaintWinPrefs.background+1);
	
	/* 外部エディタ */
	tempExternalEditor=gExternalEditor;
	tempExternalEditorAvailable=isExternalEditorAvailable;
	tempUseExternalEditor=gUseExternalEditor;
	SetDialogControlHilite(dp,startItem[1]+diUseExternalEditor,itemInactive);
	if (isExternalEditorAvailable)
	{
		SetDialogControlTitle(dp,startItem[1]+diExternalEditor,tempExternalEditor.name);
		if (gSystemVersion >= 0x0850)
			SetDialogControlHilite(dp,startItem[1]+diUseExternalEditor,itemActive);
	}
	SetDialogControlValue(dp,startItem[1]+diUseExternalEditor,tempUseExternalEditor);
	
	/* ツール設定初期化 */
	prevPaintPrefs=gToolPrefs;
	
	SetDialogControlValue(dp,startItem[2]+diEraseSameColor,prevPaintPrefs.eraseSameColor);
	SetDialogControlValue(dp,startItem[2]+diErase11,prevPaintPrefs.eraseSize11);
	if (!prevPaintPrefs.eraseSameColor)
		SetDialogControlHilite(dp,startItem[2]+diErase11,255);
	SetDialogControlValue(dp,startItem[2]+diChangePencilCursor,prevPaintPrefs.changePencilCursor);
	SetDialogControlValue(dp,startItem[2]+diUseColorCursor,prevPaintPrefs.useColorCursor);
	if (!prevPaintPrefs.changePencilCursor)
		SetDialogControlHilite(dp,startItem[2]+diUseColorCursor,255);
	SetDialogControlValue(dp,startItem[2]+diEraserByCmdKey,prevPaintPrefs.eraserByCmdKey);
	
	SetDialogControlValue(dp,startItem[2]+diSelectionMaskingOn,prevPaintPrefs.selectionMasking);
	
	value=0;
	for (i=0; i<5; i++)
	{
		if (gToolPrefs.dotDrawPrefs.keyRepThresh >= keyRepThreshValue[i])
		{
			value=i;
			break;
		}
	}
	SetDialogControlValue(dp,startItem[2]+diKeyThreshControl,value);
	
	SetDialogControlValue(dp,startItem[2]+diChangeSpoitCursor,prevPaintPrefs.changeSpoitCursor);
	
	/* PNG設定初期化 */
	prevPNGPrefs=gPNGFilePrefs;
	
	switch (prevPNGPrefs.transColor)
	{
		case kPNGTransNone:
			item2=iTransNone;
			break;
		case kPNGTransWhite:
			item2=iTransWhite;
			break;
		case kPNGTransBlack:
			item2=iTransBlack;
			break;
		case kPNGTransBottomRight:
			item2=iTransBottomRight;
			break;
		case kPNGTransPenColor:
			item2=iTransCurrent;
			break;
		case kPNGTransEraserColor:
			item2=iTransBack;
			break;
		case kPNGTransTransColor:
			item2=iTransTrans;
			break;
	}
	SetDialogControlValue(dp,startItem[3]+diPNGTransColor,item2);
	SetDialogControlValue(dp,startItem[3]+diPNGInteraced,prevPNGPrefs.interlaced);
	SetDialogControlValue(dp,startItem[3]+diPNGUseTransSaving,prevPNGPrefs.useTransColor.inSaving);
	SetDialogControlValue(dp,startItem[3]+diPNGUseTransLoading,prevPNGPrefs.useTransColor.inLoading);
	SetDialogControlValue(dp,startItem[3]+diPNGOptimizeImage,prevPNGPrefs.optimizeImage);
	
	#if 0
	SetDialogControlValue(dp,startItem[3]+diGIFUseclip2gif,prevPNGPrefs.useClip2gif);
	HideDialogItem(dp,startItem[3]+diGIFUseclip2gif);
	#endif
	
	SetDialogControlValue(dp,startItem[3]+diPNGCompLevel,prevPNGPrefs.compLevel+1);
	
	/* タブレット設定初期化 */
	prevTabletPrefs=gTabletPrefs;
	
	SetDialogControlValue(dp,startItem[4]+diTabletUsePressure,prevTabletPrefs.usePressure);
	SetDialogControlValue(dp,startItem[4]+diTabletOverrideTransparency,prevTabletPrefs.overrideTransparency);
	if (!prevTabletPrefs.usePressure)
		SetDialogControlHilite(dp,startItem[4]+diTabletOverrideTransparency,255);
	SetDialogControlValue(dp,startItem[4]+diTabletUseEraser,prevTabletPrefs.useEraser);
	SetDialogControlValue(dp,startItem[4]+diTabletUseEraserPressure,prevTabletPrefs.useEraserPressure);
	if (!prevTabletPrefs.useEraser)
		SetDialogControlHilite(dp,startItem[4]+diTabletUseEraserPressure,255);
	
	/* その他の設定初期化 */
	prevOtherPrefs=gOtherPrefs;
	
	SetDialogControlValue(dp,startItem[5]+diCheckWhenColorChanged,prevOtherPrefs.checkWhenColorChanged);
	SetDialogControlValue(dp,startItem[5]+diActivateDroppedWindow,prevOtherPrefs.activateDroppedWindow);
	SetDialogControlValue(dp,startItem[5]+diMaskAutoConvert,prevOtherPrefs.maskAutoConvert);
	SetDialogControlValue(dp,startItem[5]+diContinuousID,prevOtherPrefs.continuousIDs);
	SetDialogControlValue(dp,startItem[5]+diAddForeground,prevOtherPrefs.addForeground);
	SetDialogControlValue(dp,startItem[5]+diCopyIconWithPicture,prevOtherPrefs.copyIconWithPicture);
	SetDialogControlValue(dp,startItem[5]+diUseHyperCardPalette,prevOtherPrefs.useHyperCardPalette);
	SetDialogControlValue(dp,startItem[5]+diImportAsFamilyIcon,prevOtherPrefs.importAsFamilyIcon);
	
	if (!isIconServicesAvailable)
		SetDialogControlHilite(dp,startItem[5]+diImportAsFamilyIcon,255);
	
	/* ダイアログ表示 */
	SetDialogDefaultItem(dp,ok);
	SetDialogCancelItem(dp,cancel);
	ShowWindow(GetDialogWindow(dp));
	
	if (tabNum==kPaintPrefsTab)
		SelectDialogItemText(dp,startItem[1]+diImageWidth,0,3);
	
	item=3;
	while (item!=ok && item!=cancel)
	{
		ModalDialog(mfUPP,&item);
		
		switch (item)
		{
			case ok:
			case cancel:
				break;
			
			case 3:
				/* タブ */
				temp=GetControlValue((ControlHandle)tabHandle);
				if (!isTabletAvailable && temp==5) temp=6;
				if (temp!=tabNum)
				{
					for (j=0; j<itemNum[tabNum-1]; j++)
						HideDialogItem(dp,startItem[tTabNum[tabNum-1]-1]+j+1);
					
					tabNum=temp;
					
					for (j=0; j<itemNum[tabNum-1]; j++)
						ShowDialogItem(dp,startItem[tTabNum[tabNum-1]-1]+j+1);
					
					if (tabNum==kPaintPrefsTab)
						SelectDialogItemText(dp,startItem[1]+diImageWidth,0,3);
				}
				break;
			
			default:
				item2=item-startItem[tTabNum[tabNum-1]-1];
				
				switch (tabNum)
				{
					case kFilePrefsTab:
						/* ファイル設定 */
						switch (item2)
						{
							case diStartupPrefs:
								prevStartup=GetDialogControlValue(dp,item)-1;
								break;
							
							case diSetPICTCreator:
							case diSetGIFCreator:
							case diSetIconCreator:
							case diSetPNGCreator:
								/* クリエータ設定ボタン */
								if (GetCreatorFromFile(&creator)==noErr)
								{
									CreatorToAppl(creator,applName);
									info[item2-diSetPICTCreator]=creator;
									SetDialogItemText2(dp,item2-diSetPICTCreator+diPICTCreatorName+startItem[0],
													applName);
									MyInvalWindowRect(GetDialogWindow(dp),&itemRect);
								}
								if (!prevPaintPrefs.eraseSameColor)
									SetDialogControlHilite(dp,startItem[2]+diErase11,255);
								if (!prevPaintPrefs.changePencilCursor)
									SetDialogControlHilite(dp,startItem[2]+diUseColorCursor,255);
								if (!prevTabletPrefs.usePressure)
									SetDialogControlHilite(dp,startItem[4]+diTabletOverrideTransparency,255);
								if (!prevTabletPrefs.useEraser)
									SetDialogControlHilite(dp,startItem[4]+diTabletUseEraserPressure,255);
								if (!NavServicesAvailable())
									SetDialogControlHilite(dp,startItem[0]+diUseNavServices,255);
								SetDialogControlHilite(dp,ok,0);
								SetDialogControlHilite(dp,cancel,0);
								break;
							
							#if !TARGET_API_MAC_CARBON
							case diUseNavServices:
								/* Navigation Servicesを使用するかどうか */
								prevUN=(UInt8)!prevUN;
								SetDialogControlValue(dp,item,prevUN);
								break;
							#endif
							
							case diPreviewIconFlag:
								/* プレビューアイコンを追加するかどうか */
								prevPI=(UInt8)!prevPI;
								SetDialogControlValue(dp,item,prevPI);
								break;
						}
						break;
					
					case kPaintPrefsTab:
						/* ペイント設定 */
						switch (item2)
						{
							case diAskSize:
								/* 起動時にサイズを聞くかどうか */
								prevAsk=(UInt8)!prevAsk;
								SetDialogControlValue(dp,item,prevAsk);
								break;
							
							case diExternalEditor:
								/* 外部エディタ */
								if (SetExternalApplication(&tempSpec)==noErr)
								{
									tempExternalEditorAvailable=true;
									tempExternalEditor=tempSpec;
									
									FSpGetFileName(&tempSpec,str);
									SetDialogControlTitle(dp,item,str);
									if (gSystemVersion >= 0x0850)
										SetDialogControlHilite(dp,startItem[1]+diUseExternalEditor,itemActive);
								}
								break;
							
							case diUseExternalEditor:
								/* 外部エディタをデフォルトで使用するか */
								tempUseExternalEditor=!tempUseExternalEditor;
								SetDialogControlValue(dp,item,tempUseExternalEditor);
								break;
						}
						break;
					
					case kToolPrefsTab:
						/* ツール設定 */
						switch (item2)
						{
							case diEraseSameColor:
								/* 同じ色の場合消す */
								prevPaintPrefs.eraseSameColor=(UInt8)!prevPaintPrefs.eraseSameColor;
								SetDialogControlValue(dp,item,prevPaintPrefs.eraseSameColor);
								SetDialogControlHilite(dp,startItem[2]+diErase11,(prevPaintPrefs.eraseSameColor ? 
														itemActive : itemInactive));
								break;
							
							case diErase11:
								/* 1*1の場合のみ */
								prevPaintPrefs.eraseSize11=(UInt8)!prevPaintPrefs.eraseSize11;
								SetDialogControlValue(dp,item,prevPaintPrefs.eraseSize11);
								break;
							
							case diSelectionMaskingOn:
								/* 選択領域でマスキング */
								prevPaintPrefs.selectionMasking=(UInt8)!prevPaintPrefs.selectionMasking;
								SetDialogControlValue(dp,item,prevPaintPrefs.selectionMasking);
								break;
							
							case diChangeSpoitCursor:
								/* スポイトツールのカーソルを変更 */
								prevPaintPrefs.changeSpoitCursor=(UInt8)!prevPaintPrefs.changeSpoitCursor;
								SetDialogControlValue(dp,item,prevPaintPrefs.changeSpoitCursor);
								break;
							
							case diChangePencilCursor:
								/* 鉛筆ツールのカーソルを太さによって変更 */
								prevPaintPrefs.changePencilCursor=(UInt8)!prevPaintPrefs.changePencilCursor;
								SetDialogControlValue(dp,item,prevPaintPrefs.changePencilCursor);
								SetDialogControlHilite(dp,startItem[2]+diUseColorCursor,(prevPaintPrefs.changePencilCursor ? 
															itemActive : itemInactive));
								break;
							
							case diUseColorCursor:
								/* 色付きカーソルを使用するか */
								prevPaintPrefs.useColorCursor=(UInt8)!prevPaintPrefs.useColorCursor;
								SetDialogControlValue(dp,item,prevPaintPrefs.useColorCursor);
								break;
							
							case diEraserByCmdKey:
								/* コマンドキー＋クリック、ドラッグで消しゴムツール */
								prevPaintPrefs.eraserByCmdKey=(UInt8)!prevPaintPrefs.eraserByCmdKey;
								SetDialogControlValue(dp,item,prevPaintPrefs.eraserByCmdKey);
								break;
						}
						break;
					
					case kPNGPrefsTab:
						/* PNG設定 */
						switch (item2)
						{
							case diPNGTransColor:
								/* 透明色 */
								item2=GetDialogControlValue(dp,item);
								switch (item2)
								{
									case iTransNone: /* none */
										prevPNGPrefs.transColor=kPNGTransNone;
										break;
									case iTransWhite: /* white */
										prevPNGPrefs.transColor=kPNGTransWhite;
										break;
									case iTransBlack: /* black */
										prevPNGPrefs.transColor=kPNGTransBlack;
										break;
									case iTransBottomRight: /* bottom right */
										prevPNGPrefs.transColor=kPNGTransBottomRight;
										break;
									case iTransCurrent: /* current color */
										prevPNGPrefs.transColor=kPNGTransPenColor;
										break;
									case iTransBack: /* back color */
										prevPNGPrefs.transColor=kPNGTransEraserColor;
										break;
									case iTransTrans: /* transparent color */
										prevPNGPrefs.transColor=kPNGTransTransColor;
										break;
								}
								break;
							
							case diPNGInteraced:
								/* インタレース */
								prevPNGPrefs.interlaced=(UInt8)!prevPNGPrefs.interlaced;
								SetDialogControlValue(dp,item,prevPNGPrefs.interlaced);
								break;
							
							case diPNGUseTransSaving:
								/* 保存時に透明部分を透過指定 */
								prevPNGPrefs.useTransColor.inSaving=!prevPNGPrefs.useTransColor.inSaving;
								SetDialogControlValue(dp,item,prevPNGPrefs.useTransColor.inSaving);
								break;
							
							case diPNGUseTransLoading:
								/* 読み込み時に透過部分を透明化 */
								prevPNGPrefs.useTransColor.inLoading=!prevPNGPrefs.useTransColor.inLoading;
								SetDialogControlValue(dp,item,prevPNGPrefs.useTransColor.inLoading);
								break;
							
							case diPNGOptimizeImage:
								/* 使用されている色数にあわせて最適化 */
								prevPNGPrefs.optimizeImage=!prevPNGPrefs.optimizeImage;
								SetDialogControlValue(dp,item,prevPNGPrefs.optimizeImage);
								break;
							
							#if 0
							case diGIFUseclip2gif:
								/* clip2gifを使用 */
								prevPNGPrefs.useClip2gif=(UInt8)!prevPNGPrefs.useClip2gif;
								SetDialogControlValue(dp,startItem[3]+diGIFUseclip2gif,prevPNGPrefs.useClip2gif);
								break;
							#endif
						}
						break;
					
					case kTabletPrefsTab:
						/* タブレット設定 */
						switch (item2)
						{
							case diTabletUsePressure:
								/* 筆圧を使用 */
								tempBool=(UInt8)!prevTabletPrefs.usePressure;
								SetDialogControlValue(dp,item,tempBool);
								SetDialogControlHilite(dp,startItem[4]+diTabletOverrideTransparency,tempBool ? 0 : 255);
								prevTabletPrefs.usePressure=tempBool;
								break;
							
							case diTabletOverrideTransparency:
								tempBool=(UInt8)!prevTabletPrefs.overrideTransparency;
								SetDialogControlValue(dp,item,tempBool);
								prevTabletPrefs.overrideTransparency=tempBool;
								break;
							
							case diTabletUseEraser:
								/* 消しゴムを使用 */
								tempBool=(UInt8)!prevTabletPrefs.useEraser;
								SetDialogControlValue(dp,item,tempBool);
								SetDialogControlHilite(dp,startItem[4]+diTabletUseEraserPressure,tempBool ? 0 : 255);
								prevTabletPrefs.useEraser=tempBool;
								break;
							
							case diTabletUseEraserPressure:
								/* 消しゴムの筆圧を使用 */
								tempBool=(UInt8)!prevTabletPrefs.useEraserPressure;
								SetDialogControlValue(dp,item,tempBool);
								prevTabletPrefs.useEraserPressure=tempBool;
								break;
						}
						break;
					
					case kOtherPrefsTab:
						/* その他の設定 */
						switch (item2)
						{
							case diCheckWhenColorChanged:
								/* 色数変更後に印をつける */
								tempBool=(UInt8)!prevOtherPrefs.checkWhenColorChanged;
								SetDialogControlValue(dp,item,tempBool);
								prevOtherPrefs.checkWhenColorChanged=tempBool;
								break;
							
							case diActivateDroppedWindow:
								/* ドロップされたウィンドウをアクティブに */
								tempBool=(UInt8)!prevOtherPrefs.activateDroppedWindow;
								SetDialogControlValue(dp,item,tempBool);
								prevOtherPrefs.activateDroppedWindow=tempBool;
								break;
							
							case diMaskAutoConvert:
								/* マスクを自動変換 */
								tempBool=(UInt8)!prevOtherPrefs.maskAutoConvert;
								SetDialogControlValue(dp,item,tempBool);
								prevOtherPrefs.maskAutoConvert=tempBool;
								break;
							
							case diContinuousID:
								/* 連続ID */
								tempBool=(UInt8)!prevOtherPrefs.continuousIDs;
								SetDialogControlValue(dp,item,tempBool);
								prevOtherPrefs.continuousIDs=tempBool;
								break;
							
							case diAddForeground:
								/* 前景を追加 */
								tempBool=(UInt8)!prevOtherPrefs.addForeground;
								SetDialogControlValue(dp,item,tempBool);
								prevOtherPrefs.addForeground=tempBool;
								break;
							
							case diCopyIconWithPicture:
								/* アイコンもコピー */
								tempBool=(UInt8)!prevOtherPrefs.copyIconWithPicture;
								SetDialogControlValue(dp,item,tempBool);
								prevOtherPrefs.copyIconWithPicture=tempBool;
								break;
							
							case diUseHyperCardPalette:
								/* HyperCardのパレットを使用 */
								tempBool=(UInt8)!prevOtherPrefs.useHyperCardPalette;
								SetDialogControlValue(dp,item,tempBool);
								prevOtherPrefs.useHyperCardPalette=tempBool;
								break;
							
							case diImportAsFamilyIcon:
								/* 'icns'としてアイコンを取り込み */
								tempBool=(UInt8)!prevOtherPrefs.importAsFamilyIcon;
								SetDialogControlValue(dp,item,tempBool);
								prevOtherPrefs.importAsFamilyIcon=tempBool;
								break;
						}
						break;
				}
				break;
		}
	}
	
	switch (item)
	{
		case ok:
			/* ファイル設定の保存 */
			/* 起動時設定の保存 */
			gStartup=prevStartup;
			
			/* クリエータ */
			gPICTCreator=info[0];
			gGIFCreator=info[1];
			gPNGCreator=info[2];
			gIconCreator=info[3];
			
			/* Navigation Services */
			useNavigationServices=prevUN;
			
			/* プレビューアイコン */
			gPreviewIconFlag=prevPI;
			
			/* ペイント設定 */
			/* アイコンサイズ、倍率保存 */
			GetDialogItemText2(dp,startItem[1]+diImageWidth,str);
			StringToNum(str,&l);
			gPaintWinPrefs.iconSize.right=(short)l;
			GetDialogItemText2(dp,startItem[1]+diImageHeight,str);
			StringToNum(str,&l);
			gPaintWinPrefs.iconSize.bottom=(short)l;
			
			temp = GetDialogControlValue(dp,startItem[1]+diImageColorMode);
			if (temp == 1)
				gPaintWinPrefs.colorMode = kNormal8BitColorMode;
			else
				gPaintWinPrefs.colorMode = k32BitColorMode;
			
			gPaintWinPrefs.ratio=GetDialogControlValue(dp,startItem[1]+diImageRatio)-1;
			
			gPaintWinPrefs.askSize=prevAsk;
			
			gPaintWinPrefs.background=GetDialogControlValue(dp,startItem[1]+diBackground)-1;
			UpdateNewMenu();
			
			/* 外部エディタ */
			gExternalEditor=tempExternalEditor;
			isExternalEditorAvailable=tempExternalEditorAvailable;
			gUseExternalEditor=tempUseExternalEditor;
			
			/* ツール設定の保存 */
			gToolPrefs=prevPaintPrefs;
			
			value=GetDialogControlValue(dp,startItem[2]+diKeyThreshControl);
			
			gToolPrefs.dotDrawPrefs.keyThresh=keyThreshValue[value];
			gToolPrefs.dotDrawPrefs.keyRepThresh=keyRepThreshValue[value];
			
			ChangeSysSettings();
			
			/* PNG設定の保存 */
			gPNGFilePrefs=prevPNGPrefs;
			gPNGFilePrefs.compLevel=GetDialogControlValue(dp,startItem[3]+diPNGCompLevel)-1;
			
			/* タブレット設定の保存 */
			gTabletPrefs=prevTabletPrefs;
			
			/* その他の設定の保存 */
			if (gOtherPrefs.useHyperCardPalette != prevOtherPrefs.useHyperCardPalette)
				ResizePatternPalette();
			gOtherPrefs=prevOtherPrefs;
			break;
		
		case cancel:
			break;
	}
	
	prevTab=tabNum;
	
	DisposeDialog(dp);
	DisposeModalFilterUPP(mfUPP);
	ActivateFloatersAndFirstDocumentWindow();
	
	if (item==ok)
		UpdateBGSetting();
	
	SetPort(port);
}

/* アプリケーション選択ダイアログを表示し、選ばれたアプリケーションを得る */
OSErr SetExternalApplication(FSSpec *theFile)
{
	FSSpec	spec;
	OSErr	err=noErr;
	OSType	fileType[]={'APPL'};
	short	numTypes=1;
	#if !TARGET_API_MAC_CARBON
	ModalFilterYDUPP	mfydUPP=NewModalFilterYDUPP(MyGetFileModalFilter);
	#endif
	DialogPtr	theDialog=GetDialogFromWindow(FrontWindow());
	Str255	message;
	
	DeactivateFloatersAndFirstDocumentWindow();
	
	/* ダイアログのアイテムをDeacivateする */
	DeactivateDialogControl(theDialog);
	DrawDialog(theDialog);
	
	#if !TARGET_API_MAC_CARBON
	if (useNavigationServices)
	{
	#endif
		GetIndString(message,160,1);
		err=ChooseFileWithNav(numTypes,fileType,message,&spec);
	#if !TARGET_API_MAC_CARBON
	}
	else
	{
		StandardFileReply	reply;
		Point	pt={-1,-1};
		
		CustomGetFile(nil,numTypes,fileType,&reply,400,pt,nil,mfydUPP,nil,nil,theDialog);
		if (reply.sfGood)
			spec=reply.sfFile;
		else
			err=userCanceledErr;
		DisposeRoutineDescriptor(mfydUPP);
	}
	#endif
	if (err==noErr)
		*theFile=spec;
	else err=userCanceledErr;
	
	ActivateFloatersAndFirstDocumentWindow();
	
	/* ダイアログのアイテムをActivateする */
	ActivateDialogControl(theDialog);
	DrawDialog(theDialog);
	
	return err;
}

/* ファイル選択ダイアログを表示し、選ばれたファイルのクリエータコードを得る */
OSErr GetCreatorFromFile(OSType *creator)
{
	FSSpec	spec;
	FInfo	info;
	OSErr	err=noErr;
	#if !TARGET_API_MAC_CARBON
	ModalFilterYDUPP	mfydUPP=NewModalFilterYDUPP(MyGetFileModalFilter);
	#endif
	DialogPtr	theDialog=GetDialogFromWindow(FrontWindow());
	
	DeactivateFloatersAndFirstDocumentWindow();
	
	/* ダイアログのアイテムをDeacivateする */
	DeactivateDialogControl(theDialog);
	DrawDialog(theDialog);
	
	#if !TARGET_API_MAC_CARBON
	if (useNavigationServices)
	#endif
		err=ChooseFileWithNav(0,NULL,NULL,&spec);
	#if !TARGET_API_MAC_CARBON
	else
	{
		StandardFileReply	reply;
		Point	pt={-1,-1};
		
		CustomGetFile(nil,-1,nil,&reply,400,pt,nil,mfydUPP,nil,nil,theDialog);
		if (reply.sfGood)
			spec=reply.sfFile;
		else
			err=userCanceledErr;
		DisposeRoutineDescriptor(mfydUPP);
	}
	#endif
	if (err==noErr)
	{
		/* 選択されたファイルのクリエータコードを求め表示する */
		err=FSpGetFInfo(&spec,&info);
		if (err==noErr)
			*creator=info.fdCreator;
	}
	else err=userCanceledErr;
	
	ActivateFloatersAndFirstDocumentWindow();
	
	/* ダイアログのアイテムをActivateする */
	ActivateDialogControl(theDialog);
	DrawDialog(theDialog);
	
	return err;
}

/* 各ペイントウィンドウの設定を更新する */
void UpdateBGSetting(void)
{
	WindowPtr	theWindow=MyFrontNonFloatingWindow();
	PaintWinRec	*eWinRec;
	
	while (theWindow != nil)
	{
		if (GetExtWindowKind(theWindow)==kWindowTypePaintWindow)
		{
			eWinRec=GetPaintWinRec(theWindow);
			if (eWinRec->foregroundGWorld == nil && eWinRec->backgroundGWorld == nil)
				eWinRec->editBackground=gOtherPrefs.addForeground;
		}
		theWindow=GetNextVisibleWindow(theWindow);
	}
	
	UpdateBGMenu();
}

#if !TARGET_API_MAC_CARBON
/* ファイル選択ダイアログのフィルタ */
pascal Boolean MyGetFileModalFilter(DialogPtr theDialog,EventRecord *theEvent,short *itemHit,void *yourDataPtr)
{
	#pragma unused(itemHit,yourDataPtr)
	Boolean		eventHandled=false;
	DialogPtr	updateDialog,parentDialog=(DialogPtr)yourDataPtr;
	GrafPtr		port;
	
	switch (theEvent->what)
	{
		case updateEvt:
			updateDialog=(DialogPtr)theEvent->message;
			if (updateDialog != nil)
			{
				if (updateDialog == parentDialog)
				{
					GetPort(&port);
					SetPortDialogPort(parentDialog);
					BeginUpdate(GetDialogWindow(parentDialog));
					DrawDialog(parentDialog);
					EndUpdate(GetDialogWindow(parentDialog));
					SetPort(port);
				}
				else if (updateDialog != theDialog)
					DoEvent(theEvent);
			}
			break;
	}
	
	return eventHandled;
}
#endif

enum {
	kGridTypeItem = 3,
	kIconGridWidthItem,
	kGridColorItem,
	kIconGridColorItem,
	kGridColorSampleItem,
	kIconGridColorSampleItem,
};

enum {
	kGridColorWhite = 1,
	kGridColorBlack,
	kGridColorCustom,
};

static RGBColor	gGridColor[2];

/* グリッド設定 */
void ConfigureGrid(void)
{
	DialogPtr	dp;
	GrafPtr		port;
	short		item;
	ModalFilterUPP	mfUPP=NewModalFilterUPP(MyConfigureGridFilter);
	short		gridType;
	short		iconGrid;
	short		colorType;
	Point		where = {-1,-1};
	Str255	prompt;
	RGBColor	newColor;
	Rect		r;
	
	UseResFile(gApplRefNum);
	
	DeactivateFloatersAndFirstDocumentWindow();
	dp=GetNewDialog(149,nil,kFirstWindowOfClass);
	if (dp==nil)
	{
		ErrorAlertFromResource(PREFERR_RESID,PREFERR4);
		return;
	}
	
	GetPort(&port);
	SetPortDialogPort(dp);
	MySetCursor(0);
	
	/* ポップアップなどの調整 */
	gridType = gToolPrefs.gridMode & 0x0f;
	iconGrid = (gToolPrefs.gridMode>>4) & 0x0f;
	
	SetDialogControlValue(dp,kGridTypeItem,gridType);
	SetDialogControlValue(dp,kIconGridWidthItem,iconGrid+1);
	
	if (EqualColor(&gToolPrefs.gridColor,&rgbBlackColor))
		colorType = kGridColorBlack;
	else if (EqualColor(&gToolPrefs.gridColor,&rgbWhiteColor))
		colorType = kGridColorWhite;
	else
		colorType = kGridColorCustom;
	SetDialogControlValue(dp,kGridColorItem,colorType);
	
	if (EqualColor(&gToolPrefs.gridColor2,&rgbBlackColor))
		colorType = kGridColorBlack;
	else if (EqualColor(&gToolPrefs.gridColor2,&rgbWhiteColor))
		colorType = kGridColorWhite;
	else
		colorType = kGridColorCustom;
	SetDialogControlValue(dp,kIconGridColorItem,colorType);
	
	gGridColor[0] = gToolPrefs.gridColor;
	gGridColor[1] = gToolPrefs.gridColor2;
	
	/* ダイアログ表示 */
	SetDialogDefaultItem(dp,ok);
	SetDialogCancelItem(dp,cancel);
	ShowWindow(GetDialogWindow(dp));
	
	item=3;
	while (item!=ok && item!=cancel)
	{
		ModalDialog(mfUPP,&item);
		
		switch (item)
		{
			case kGridColorItem:
			case kIconGridColorItem:
				switch (GetDialogControlValue(dp,item))
				{
					case kGridColorCustom:
						GetIndString(prompt,162,item-kGridColorItem+1);
						if (!GetColor(where,prompt,&gGridColor[item-kGridColorItem],&newColor))
							newColor = gGridColor[item-kGridColorItem];
						break;
					
					case kGridColorWhite:
						newColor = rgbWhiteColor;
						break;
					
					case kGridColorBlack:
						newColor = rgbBlackColor;
						break;
				}
				if (!EqualColor(&newColor,&gGridColor[item-kGridColorItem]))
				{
					gGridColor[item-kGridColorItem] = newColor;
					GetDialogItemRect(dp,item-kGridColorItem+kGridColorSampleItem,&r);
					MyInvalWindowRect(GetDialogWindow(dp),&r);
				}
				break;
			
			case kGridColorSampleItem:
			case kIconGridColorSampleItem:
				GetIndString(prompt,162,item-kGridColorSampleItem+1);
				if (GetColor(where,prompt,&gGridColor[item-kGridColorSampleItem],&newColor))
					if (!EqualColor(&newColor,&gGridColor[item-kGridColorSampleItem]))
					{
						gGridColor[item-kGridColorSampleItem] = newColor;
						GetDialogItemRect(dp,item,&r);
						MyInvalWindowRect(GetDialogWindow(dp),&r);
					}
				break;
		}
	}
	
	/* 設定を保存する */
	if (item == ok)
	{
		gToolPrefs.gridMode = ((GetDialogControlValue(dp,kIconGridWidthItem)-1) << 4) + 
								GetDialogControlValue(dp,kGridTypeItem);
		gToolPrefs.gridColor = gGridColor[0];
		gToolPrefs.gridColor2 = gGridColor[1];
	}
	
	DisposeDialog(dp);
	DisposeModalFilterUPP(mfUPP);
	ActivateFloatersAndFirstDocumentWindow();
	
	/* 画面の書き換え */
	if (item == ok)
	{
		WindowPtr theWindow=MyFrontNonFloatingWindow();
		
		while(theWindow!=nil && GetExtWindowKind(theWindow)==kWindowTypePaintWindow)
		{
			MyInvalWindowPortBounds(theWindow);
			theWindow=GetNextVisibleWindow(theWindow);
		}
	}
	
	SetPort(port);
}

pascal Boolean MyConfigureGridFilter(DialogPtr theDialog,EventRecord *theEvent,short *theItemHit)
{
	WindowPtr	theWindow;
	short		i;
	Rect		r;
	
	switch (theEvent->what)
	{
		case updateEvt:
			theWindow=(WindowPtr)theEvent->message;
			if (theWindow==GetDialogWindow(theDialog))
			{
				for (i=0; i<2; i++)
				{
					GetDialogItemRect(theDialog,kGridColorSampleItem+i,&r);
					FrameRect(&r);
					InsetRect(&r,1,1);
					RGBForeColor(&gGridColor[i]);
					PaintRect(&r);
					ForeColor(blackColor);
				}
			}
			break;
	}
	
	return MyModalDialogFilter(theDialog,theEvent,theItemHit);
}
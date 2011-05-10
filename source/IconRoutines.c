/* ------------------------------------------------------------ */
/*  IconRoutines.c                                              */
/*     アイコン処理                                             */
/*                                                              */
/*                 1998.12.2 - 2001.2.3  naoki iimura        	*/
/* ------------------------------------------------------------ */

/* includes */
#ifdef __APPLE_CC__
#include	<Carbon/Carbon.h>
#else
#include	<TSMTE.h>
#include	<FinderRegistry.h>
#include	<Sound.h>
#include	<Scrap.h>
#include	<ToolUtils.h>
#include	<Folders.h>
#endif

#ifdef __APPLE_CC__
#include	"MoreFilesX.h"
#else
#include	"MoreDesktopMgr.h"
#include	"MoreFilesExtras.h"
#include	"FullPath.h"
#endif

#include	"Globals.h"
#include	"UsefulRoutines.h"
#include	"IconParty.h"
#include	"IconRoutines.h"
#include	"WindowExtensions.h"
#include	"MenuRoutines.h"
#include	"FileRoutines.h"
#include	"WindowRoutines.h"
#include	"NavigationServicesSupport.h"
#include	"ExternalEditorSupport.h"
#include	"EditRoutines.h"
#include	"PaintRoutines.h"
#include	"IPIconSupport.h"
#include	"IconListWindow.h"
#if !TARGET_API_MAC_CARBON
#include	"CustomSaveDialog.h"
#include	"Find_icon.h"
#endif
#include	"PreCarbonSupport.h"


/* プロトタイプ */

/* 新規アイコン関連 */
static void	AddNewIconReset(DialogPtr dp);
static pascal Boolean NewIconUpdateFilter(DialogPtr theDialog,EventRecord *theEvent,short *theItemHit);


extern const OSType gIconType[];


/* アイコンデータをファイルに保存 */
pascal OSErr AddIconToFile(ResType theType,Handle *theIcon,void *yourDataPtr)
{
	MyIconResRec *newIcon=(MyIconResRec *)yourDataPtr;
	
	if (*theIcon != nil)
		SaveDataToResource(**theIcon,GetHandleSize(*theIcon),theType,newIcon->resID,
							newIcon->resName,newIcon->attrs);
	else
	{
		Handle	h=Get1Resource(theType,newIcon->resID);
		
		if (h!=nil)
		{
			RemoveResource(h);
			DisposeHandle(h);
		}
	}
	
	return noErr;
}

/* アイコン付きのフォルダを作成 */
OSErr MakeFolderWithIcon(FSSpec *theFolder,IconSuiteRef iconSuite)
{
	IPIconRec	ipIcon;
	
	ipIcon.iconSuite = iconSuite;
	ipIcon.it32Data = NULL;
	ipIcon.t8mkData = NULL;
	
	return MakeFolderWithIPIcon(theFolder,&ipIcon);
}

/* アイコンつきファイルの作成 */
OSErr MakeFileWithIcon(const FSSpec *theFile,IconSuiteRef iconSuite)
{
	IPIconRec	ipIcon;
	
	ipIcon.iconSuite = iconSuite;
	ipIcon.it32Data = NULL;
	ipIcon.t8mkData = NULL;
	
	return MakeFileWithIPIcon(theFile,&ipIcon);
}

/* 'icns'リソースを削除 */
void DeleteIconFamilyResource(void)
{
	Handle	h;
	
	h=Get1Resource(kIconFamilyType,kCustomIconResource);
	if (h!=nil)
	{
		RemoveResource(h);
		DisposeHandle(h);
	}
}

/* ファイルのアイコンをIconSuiteとして得る */
IconSuiteRef GetFileIconSuite(FSSpec *theFile)
{
	OSErr		err;
	IconSuiteRef	iconSuite;
	
	#if !TARGET_API_MAC_CARBON
	if (gSystemVersion>=0x0850)
	#endif
	{
		/* 8.5以上であればIconServicesのルーチンを使う */
		IconRef		iconRef;
		short		label;
		IconFamilyHandle	iconFamily;
		
		err=GetIconRefFromFile(theFile,&iconRef,&label);
		err=IconRefToIconFamily(iconRef,GetMySelector(),&iconFamily);
		err=IconFamilyToIconSuite(iconFamily,GetMySelector(),&iconSuite);
		err=ReleaseIconRef(iconRef);
		DisposeHandle((Handle)iconFamily);
		
		return iconSuite;
	}
	#if !TARGET_API_MAC_CARBON
	else
	{
		/* 0.90b7変更部分 */
		/* それ以前なら、Find_iconを使用 */
		err=Find_icon(theFile,NULL,kSelectorMyData,&iconSuite);
		
		if (err==noErr) return iconSuite;
		else return (IconSuiteRef)0L;
	}
	#endif
	return nil;
}

/* アイコンをリソースから切り離す */
pascal OSErr DetachIcon(ResType theType,Handle *iconData,void *yourDataPtr)
{
	#pragma unused(theType,yourDataPtr)
	DetachResource(*iconData);
	HNoPurge(*iconData);
	
	return noErr;
}

/* IconSuiteからクリップボードへ */
OSErr IconSuiteToClip(IconSuiteRef iconSuite)
{
	OSErr	err;
	IconActionUPP	copyIconDataUPP=NewIconActionUPP(CopyIconData);
	IconFamilyHandle	iconFamily;
	
	#if TARGET_API_MAC_CARBON
	ScrapRef	scrap;
	
	err=ClearCurrentScrap();
	err=GetCurrentScrap(&scrap);
	err=ForEachIconDo(iconSuite,kSelectorMyData,copyIconDataUPP,scrap);
	
	err=IconSuiteToIconFamily(iconSuite,kSelectorMy32Data,&iconFamily);
	if (err==noErr)
	{
		err=PutScrapFlavor(scrap,kIconFamilyType,0,GetHandleSize((Handle)iconFamily),*iconFamily);
		DisposeHandle((Handle)iconFamily);
	}
	#else
	long	result;
	
	result=ZeroScrap();
	err=ForEachIconDo(iconSuite,kSelectorMyData,copyIconDataUPP,nil);
	
	if (gSystemVersion >= 0x0850)
	{
		err=IconSuiteToIconFamily(iconSuite,kSelectorMy32Data,&iconFamily);
		if (err==noErr)
		{
			result=PutScrap(GetHandleSize((Handle)iconFamily),kIconFamilyType,*iconFamily);
			DisposeHandle((Handle)iconFamily);
		}
	}
	#endif
	
	DisposeIconActionUPP(copyIconDataUPP);
	
	return err;
}

/* アイコンデータをクリップボードにコピー */
pascal OSErr CopyIconData(ResType theType,Handle *theIcon,void *yourDataPtr)
{
	#if !TARGET_API_MAC_CARBON
	#pragma unused(yourDataPtr)
	
	long	result;
	#else
	ScrapRef	scrap=(ScrapRef)yourDataPtr;
	OSErr		err;
	#endif
	long	dataSize;
	
	if (*theIcon != NULL)
	{
		dataSize=GetHandleSize(*theIcon);
		#if TARGET_API_MAC_CARBON
		err=PutScrapFlavor(scrap,theType,0,dataSize,**theIcon);
		#else
		result=PutScrap(dataSize,theType,**theIcon);
		#endif
	}
	
	return noErr;
}

/* アイコンの削除 */
pascal OSErr DeleteIcon(ResType theType,Handle *theIcon,void *yourDataPtr)
{
	#pragma unused(theType,yourDataPtr)
	
	if (*theIcon != NULL)
		RemoveResource(*theIcon);
	
	return noErr;
}

/* アイコンの情報の変更 */
pascal OSErr ChangeIconInfo(ResType theType,Handle *theIcon,void *yourDataPtr)
{
	#pragma unused(theType)
	MyIconResRec	*newIcon=(MyIconResRec *)yourDataPtr;
	ResType	type;
	short	id;
	Str255	name;
	
	if (*theIcon != NULL)
	{
		GetResInfo(*theIcon,&id,&type,name);
		SetResInfo(*theIcon,newIcon->resID,newIcon->resName);
	}
	
	return noErr;
}

/* 削除情報を作成 */
void CreateDeleteInfo(short resID)
{
	SaveDataToResource(NULL,0,kDeleteIconInfoType,resID,"\p",resPurgeable);
}

/* 更新情報を作成 */
void CreateUpdateInfo(short resID)
{
	SaveDataToResource(NULL,0,kUpdateIconInfoType,resID,"\p",resPurgeable);
	
	RemoveDeleteInfo(resID);
}

/* 削除情報を破棄 */
void RemoveDeleteInfo(short resID)
{
	Handle	resHandle;
	
	resHandle = Get1Resource(kDeleteIconInfoType,resID);
	if (resHandle != NULL)
		RemoveResource(resHandle);
}

/* 更新されているかどうか（テンポラリファイルにアイコンがあるかどうか） */
Boolean IsIconChanged(short resID)
{
	Handle	resHandle;
	
	resHandle = Get1Resource(kUpdateIconInfoType,resID);
	if (resHandle != NULL)
	{
		ReleaseResource(resHandle);
		return true;
	}
	return false;
}

/* アイコンプレビューを書き換えさせる */
void RedrawIconPreview(void)
{
	GrafPtr	port;
	
	GetPort(&port);
	SetPortWindowPort(gPreviewWindow);
	MyInvalWindowPortBounds(gPreviewWindow);
	SetPort(port);
}

/* アイコンプレビューのアップデート */
void UpdateIconPreview(WindowPtr frontWin)
{
	IconListWinRec	*iWinRec;
	IconFamilyWinRec	*fWinRec;
	IconListDataRec	*data;
	IPIconRec	ipIcon;
	OSErr		err;
	
	switch (GetExtWindowKind(frontWin))
	{
		case kWindowTypeIconListWindow:
			iWinRec=GetIconListRec(frontWin);
			if (GetSelectedIconData(iWinRec,&data)==noErr)
			{
				Str255	iconName;
				
				if (MyGetIPIcon(iWinRec,&ipIcon,data,iconName,NULL)==noErr)
				{
					DrawIPIconPreview(&ipIcon);
					DisposeIPIcon(&ipIcon);
					return;
				}
			}
			DrawIPIconPreview(NULL);
			break;
		
		case kWindowTypeIconFamilyWindow:
			fWinRec=GetIconFamilyRec(frontWin);
			if (fWinRec->selectedIcon>=0)
			{
				Handle	h;
				
				/* まず、選ばれているアイコンのデータを取得 */
				err=GetDataFromIPIcon(&h,&fWinRec->ipIcon,fWinRec->selectedIcon);
				if (err!=noErr || h == NULL)
				{
					DrawIPIconPreview(NULL);
					return;
				}
				
				/* まず、選ばれているアイコンを追加 */
				if (fWinRec->selectedIcon != kT32Data && fWinRec->selectedIcon != kT8Mask)
					err=NewIconSuite(&ipIcon.iconSuite);
				else
					ipIcon.iconSuite = NULL;
				ipIcon.it32Data = NULL;
				ipIcon.t8mkData = NULL;
				
				err=SetDataToIPIcon(h,&ipIcon,fWinRec->selectedIcon);
				
				/* 次に、他のデータを追加 */
				switch (fWinRec->selectedIcon)
				{
					case kT32Data:
					case kT8Mask:
						if (gSystemVersion >= 0x0850)
						{
							if (fWinRec->selectedIcon == kT32Data)
							{
								err=GetDataFromIPIcon(&h,&fWinRec->ipIcon,kT8Mask);
								err=SetDataToIPIcon(h,&ipIcon,kT8Mask);
							}
							else
							{
								err=GetDataFromIPIcon(&h,&fWinRec->ipIcon,kT32Data);
								err=SetDataToIPIcon(h,&ipIcon,kT32Data);
							}
						}
						break;
					
					case kL32Data:
					case kL8Mask:
						if (gSystemVersion >= 0x0850)
						{
							if (fWinRec->selectedIcon == kL32Data)
							{
								err=GetDataFromIPIcon(&h,&fWinRec->ipIcon,kL8Mask);
								err=SetDataToIPIcon(h,&ipIcon,kL8Mask);
							}
							else
							{
								err=GetDataFromIPIcon(&h,&fWinRec->ipIcon,kL4Data);
								if (h != NULL)
									err=SetDataToIPIcon(h,&ipIcon,kL4Data);
								err=GetDataFromIPIcon(&h,&fWinRec->ipIcon,kL8Data);
								if (h != NULL)
									err=SetDataToIPIcon(h,&ipIcon,kL8Data);
								err=GetDataFromIPIcon(&h,&fWinRec->ipIcon,kL32Data);
								if (h != NULL)
									err=SetDataToIPIcon(h,&ipIcon,kL32Data);
							}
						}
					case kL8Data:
					case kL4Data:
						err=GetDataFromIPIcon(&h,&fWinRec->ipIcon,kL1Data);
						err=SetDataToIPIcon(h,&ipIcon,kL1Data);
					case kL1Data:
					case kL1Mask:
						break;
					
					case kS32Data:
					case kS8Mask:
						if (gSystemVersion >= 0x0850)
						{
							if (fWinRec->selectedIcon == kS32Data)
							{
								err=GetDataFromIPIcon(&h,&fWinRec->ipIcon,kS8Mask);
								err=SetDataToIPIcon(h,&ipIcon,kS8Mask);
							}
							else
							{
								err=GetDataFromIPIcon(&h,&fWinRec->ipIcon,kS4Data);
								if (h != NULL)
									err=SetDataToIPIcon(h,&ipIcon,kS4Data);
								err=GetDataFromIPIcon(&h,&fWinRec->ipIcon,kS8Data);
								if (h != NULL)
									err=SetDataToIPIcon(h,&ipIcon,kS8Data);
								err=GetDataFromIPIcon(&h,&fWinRec->ipIcon,kS32Data);
								if (h != NULL)
									err=SetDataToIPIcon(h,&ipIcon,kS32Data);
							}
						}
					case kS8Data:
					case kS4Data:
						err=GetDataFromIPIcon(&h,&fWinRec->ipIcon,kS1Data);
						err=SetDataToIPIcon(h,&ipIcon,kS1Data);
						break;
				}
				DrawIPIconPreview(&ipIcon);
				DisposeIconSuite(ipIcon.iconSuite,false);
				return;
			}
			DrawIPIconPreview(nil);
			break;
	}
}

/* アイコンファイルからアイコンを読み込む。テンポラリファイルの内容も考慮 */
/* iconName : アイコンの名前が返る
   isEditableIconFamily : IconPartyで編集可能なアイコンかどうかが返る。NULL可 */
OSErr MyGetIPIcon(IconListWinRec *iWinRec,IPIconRec *ipIcon,IconListDataRec *data,
	Str255 iconName,Boolean *isEditableIconFamily)
{
	OSErr	err;
	Handle	iconData;
	ResType	theType;
	short	id;
	
	UseResFile(iWinRec->tempRefNum);
	
	if (data->resType == kIconFamilyType)
	{
		IconFamilyHandle	iconFamily;
		
		iconFamily = (IconFamilyHandle)Get1Resource(kIconFamilyType,data->resID);
		if (iconFamily == nil)
		{
			UseResFile(iWinRec->refNum);
			iconFamily = (IconFamilyHandle)Get1Resource(kIconFamilyType,data->resID);
		}
		
		if (isEditableIconFamily != NULL)
			*isEditableIconFamily = IsEditableIconFamily(iconFamily);
		
		err=IconFamilyToIPIcon(iconFamily,ipIcon);
		GetResInfo((Handle)iconFamily,&id,&theType,iconName);
		ReleaseResource((Handle)iconFamily);
	}
	else
	{
		iconData=Get1Resource(kLarge1BitMask,data->resID);
		if (iconData==nil)
		{
			iconData=Get1Resource(kSmall1BitMask,data->resID);
			if (iconData==nil)
			{
				UseResFile(iWinRec->refNum);
				iconData=Get1Resource(kLarge1BitMask,data->resID);
			}
		}
		err=Get1IconSuite(&ipIcon->iconSuite,data->resID,GetMySelector());
		ipIcon->it32Data=NULL;
		ipIcon->t8mkData=NULL;
		GetResInfo(iconData,&id,&theType,iconName);
	}
	
	UseResFile(gApplRefNum);
	
	return err;
}

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

/* データをリソースに書き出す */
void SaveDataToResource(void *dataPtr,long dataSize,ResType type,short id,Str255 resName,short attrs)
{
	Handle	resHandle;
	
	resHandle=Get1Resource(type,id);
	if (resHandle!=nil)
	{
		RemoveResource(resHandle);
		DisposeHandle(resHandle);
	}
	
	PtrToHand(dataPtr,&resHandle,dataSize);
	AddResource(resHandle,type,id,resName);
	SetResAttrs(resHandle,attrs);
	ChangedResource(resHandle);
	ReleaseResource(resHandle);
}

/* アイコンのデータをコピーする */
void CopyIcons(short srcRefNum,short dstRefNum)
{
	short			i,j,iconNum;
	short			curResFile=CurResFile();
	ResType			iconTypeList[]={kLarge8BitData,kLarge4BitData,kLarge1BitMask,
									kSmall8BitData,kSmall4BitData,kSmall1BitMask,
									kLarge32BitData,kLarge8BitMask,
									kSmall32BitData,kSmall8BitMask,
									kIconFamilyType};
	
	for (j=0; j<(isIconServicesAvailable ? 11 : 6); j++)
	{
		UseResFile(srcRefNum);
		iconNum=Count1Resources(iconTypeList[j]);
		
		for (i=1; i<=iconNum; i++)
		{
			Handle	resHandle,temp;
			short	resID;
			ResType	resType;
			Str255	resName;
			short	attrs;
			
			UseResFile(srcRefNum);
			resHandle=Get1IndResource(iconTypeList[j],i);
			GetResInfo(resHandle,&resID,&resType,resName);
			attrs=GetResAttrs(resHandle);
			
			/* 削除情報があるものについてはコピーしない */
			temp=Get1Resource(kDeleteIconInfoType,resID);
			if (temp != nil)
			{
				ReleaseResource(temp);
				continue;
			}
			
			UseResFile(dstRefNum);
			SaveDataToResource(*resHandle,GetHandleSize(resHandle),resType,resID,resName,resPurgeable);
		}
	}
	
	UseResFile(curResFile);
}

/* アイコンに必要なデータがあるかどうか */
Boolean IsValidIconSuite(IconSuiteRef iconSuite)
{
	Handle	h;
	OSErr	err;
	
	err=GetIconFromSuite(&h,iconSuite,kLarge1BitMask);
	if (h == nil)
		err=GetIconFromSuite(&h,iconSuite,kSmall1BitMask);
			if (h == nil) return false;
	return true;
}

/* 32ビットアイコンが存在しているかどうか */
Boolean Is32Exist(IconSuiteRef iconSuite)
{
	Boolean	is32Exist=false;
	Handle	iconData;
	short	i;
	OSErr	err;
	
	for (i=kL32Data; i<=kS8Mask; i++)
	{
		err=GetIconFromSuite(&iconData,iconSuite,gIconType[i]);
		if (iconData != nil)
		{
			is32Exist=true;
			break;
		}
	}
	
	return is32Exist;
}

/* IconPartyで編集できないアイコンが含まれていないかどうか */
/* true -> 含まれていない 
   false -> 含まれている */
Boolean	IsEditableIconFamily(IconFamilyHandle iconFamily)
{
	Handle	h;
	ResType	iconType[]={kHuge1BitMask,kHuge4BitData,kHuge8BitData,
						kHuge32BitData,kHuge8BitMask,
						kMini1BitMask,kMini4BitData,kMini8BitData};
	short	i;
	OSErr	err;
	Boolean	result = true;
	
	h=NewHandle(0);
	
	for (i=0; i<8; i++)
	{
		err=GetIconFamilyData(iconFamily,iconType[i],h);
		if (err==noErr && GetHandleSize(h)>0)
		{
			result = false;
			break;
		}
	}
	DisposeHandle(h);
	
	return result;
}

/* 新規アイコン関連 */
enum {
	kNewIconIconIndex=3,
	kNewIconIDIndex,
	kNewIconNameIndex,
	
	kNewIconL8Index,
	kNewIconL4Index,
	kNewIconL1Index,
	kNewIconS8Index,
	kNewIconS4Index,
	kNewIconS1Index,
	
	kNewIconL32Index,
	kNewIconL8MIndex,
	kNewIconS32Index,
	kNewIconS8MIndex,
	
	kNewIconT32Index,
	kNewIconT8MIndex,
	
	kNewIconSelectAllIndex,
	kNewIconSelectNoneIndex,
	
	kNewIconForKT7Index,
	kNewIconFor85Index,
	kNewIconForXIndex,
	
	kNewIconLargeOnlyIndex,
	kNewIconSmallOnlyIndex,
	kNewIconThumbnailOnlyIndex,
	
	kNewIconTemplateIndex,
};

static IPIconSelector	gSelector={kSelectorMyData,false,false};

static IconSelectorValue	gSelectorList[]= {
								kSelectorLarge8Bit,kSelectorLarge4Bit,kSelectorLarge1Bit,
								kSelectorSmall8Bit,kSelectorSmall4Bit,kSelectorSmall1Bit,
								kSelectorLarge32Bit,kSelectorLarge8BitMask,
								kSelectorSmall32Bit,kSelectorSmall8BitMask};

/* icon templates */
OSType gTemplateIconType[] = {
					0,0,
					kGenericDocumentIcon,
					kGenericApplicationIcon,
					kGenericFolderIcon };
short gTemplateIconResource[] = {
					0,0,
					kGenericDocumentIconResource,
					kGenericApplicationIconResource,
					kGenericFolderIconResource };

static short gSelectedTemplate = 0;


/* 新規アイコンのダイアログ */
OSErr NewIconDialog(MyIconResRec *newIcon,IPIconSelector *selector,
	short *iconTemplate,ListHandle iconList)
{
	DialogPtr		dp;
	short			item;
	Str255			str;
	long			temp;
	ModalFilterUPP	mfUPP=NewModalFilterUPP(NewIconUpdateFilter);
	Boolean			tempBool;
	Rect			r,iconRect;
	GrafPtr			port;
	
	DeactivateFloatersAndFirstDocumentWindow();
	InitCursor();
	
	dp=GetNewDialog(143,nil,kFirstWindowOfClass);
	
	/* タイトルおよび変更ボタンの名前を変更 */
	GetIndString(str,141,4);
	SetWTitle(GetDialogWindow(dp),str);
	GetIndString(str,141,5);
	SetDialogControlTitle(dp,ok,str);
	
	GetPort(&port);
	SetPortDialogPort(dp);
	
	/* アイコンファミリ作成時は、IDと名前を変更不可 */
	if (newIcon->resID == kCustomIconResource)
	{
		SetDialogItemToStaticText(dp,kNewIconIDIndex);
		SetDialogItemToStaticText(dp,kNewIconNameIndex);
		
		/* テキストのハイライトを消すため */
		HideDialogItem(dp,kNewIconIDIndex);
		ShowDialogItem(dp,kNewIconIDIndex);
	}
	
	NumToString(newIcon->resID,str);
	SetDialogItemText2(dp,kNewIconIDIndex,str);
	SetDialogItemText2(dp,kNewIconNameIndex,newIcon->resName);
	
	if (newIcon->resID != kCustomIconResource)
		SelectDialogItemText(dp,kNewIconIDIndex,0,str[0]);
	
	/* アイコン表示アイテムを隠す */
	//HideDialogItem(dp,kNewIconIconIndex);
	
	GetDialogItemRect(dp,kNewIconIconIndex,&iconRect);
	r = iconRect;
	InsetRect(&r,-2,-2);
	FrameRect(&r);
	
	/* template */
	SetDialogControlValue(dp,kNewIconTemplateIndex,gSelectedTemplate + 1);
	
	/* ok,cancelにショートカットを割り当て */
	SetDialogDefaultItem(dp,ok);
	SetDialogCancelItem(dp,cancel);
	
	/* OS8.5以前ならil32などを無効に */
	if (!isThumbnailIconsAvailable)
	{
		if (!is32BitIconsAvailable)
		{
			for (item=kNewIconL32Index; item<=kNewIconS8MIndex; item++)
				SetDialogControlHilite(dp,item,255);
			SetDialogControlHilite(dp,kNewIconFor85Index,255);
		}
			
		for (item=kNewIconT32Index; item<=kNewIconT8MIndex; item++)
			SetDialogControlHilite(dp,item,255);
		
		SetDialogControlHilite(dp,kNewIconForXIndex,255);
		SetDialogControlHilite(dp,kNewIconThumbnailOnlyIndex,255);
	}
	
	/* チェックボックスにチェック */
	AddNewIconReset(dp);
	
	ShowWindow(GetDialogWindow(dp));
	
	item=3;
	while (item != ok && item != cancel)
	{
		ModalDialog(mfUPP,&item);
		
		switch (item)
		{
			case ok:
				/* 同じIDのアイコンがあるかも知れないからそれをチェック */
				/* アイコンファミリ作成でないときのみ */
				if (newIcon->resID != kCustomIconResource)
				{
					Cell	theCell={0,0};
					
					GetDialogItemText2(dp,kNewIconIDIndex,str);
					StringToNum(str,&temp);
					if (temp == kCustomIconResource)
					{
						SysBeep(0);
						item=3;
						SelectDialogItemText(dp,kNewIconIDIndex,0,str[0]);
						continue;
					}
					
					if (IDToCell(temp,&theCell,iconList))
					{
						SysBeep(0);
						item=3;
						SelectDialogItemText(dp,kNewIconIDIndex,0,str[0]);
					}
				}
				break;
			
			case kNewIconL8Index:
			case kNewIconL4Index:
			case kNewIconL32Index:
			case kNewIconL8MIndex:
				/* icl8 / icl4 / il32 / l8mk */
				tempBool=GetDialogControlValue(dp,item);
				SetDialogControlValue(dp,item,1-tempBool);
				gSelector.selector ^= gSelectorList[item-kNewIconL8Index];
				if (!tempBool) /* icl8、icl4、il32、l8mkありの時はICN#もあり */
				{
					SetDialogControlValue(dp,kNewIconL1Index,1);
					gSelector.selector |= kSelectorLarge1Bit;
				}
				MyInvalWindowRect(GetDialogWindow(dp),&iconRect);
				break;
			
			case kNewIconL1Index:
				/* ICN# */
				tempBool=GetDialogControlValue(dp,item);
				if (tempBool && !GetDialogControlValue(dp,kNewIconS1Index) &&
					!GetDialogControlValue(dp,kNewIconT8MIndex)) break;
				
				SetDialogControlValue(dp,item,1-tempBool);
				gSelector.selector ^= gSelectorList[item-kNewIconL8Index];
				if (tempBool) /* ICN#なしの時はicl8、icl4、il32、l8mkもなし */
				{
					SetDialogControlValue(dp,kNewIconL8Index,0);
					SetDialogControlValue(dp,kNewIconL4Index,0);
					SetDialogControlValue(dp,kNewIconL32Index,0);
					SetDialogControlValue(dp,kNewIconL8MIndex,0);
					gSelector.selector &= ~(kSelectorLarge8Bit+kSelectorLarge4Bit+
											kSelectorLarge32Bit+kSelectorLarge8BitMask);
				}
				MyInvalWindowRect(GetDialogWindow(dp),&iconRect);
				break;
			
			case kNewIconS8Index:
			case kNewIconS4Index:
			case kNewIconS32Index:
			case kNewIconS8MIndex:
				/* ics8 / ics4 / is32 / s8mk */
				tempBool=GetDialogControlValue(dp,item);
				SetDialogControlValue(dp,item,1-tempBool);
				gSelector.selector ^= gSelectorList[item-kNewIconL8Index];
				if (!tempBool) /* ics8、ics4、il32、s8mkありの時はics#もあり */
				{
					SetDialogControlValue(dp,kNewIconS1Index,1);
					gSelector.selector |= kSelectorSmall1Bit;
				}
				MyInvalWindowRect(GetDialogWindow(dp),&iconRect);
				break;
			
			case kNewIconS1Index:
				/* ics# */
				tempBool=GetDialogControlValue(dp,item);
				if (tempBool && !GetDialogControlValue(dp,kNewIconL1Index) &&
					!GetDialogControlValue(dp,kNewIconT8MIndex)) break;
				
				SetDialogControlValue(dp,item,1-tempBool);
				gSelector.selector ^= gSelectorList[item-kNewIconL8Index];
				if (tempBool) /* ics#なしの時はics4、ics8、is32、s8mkもなし */
				{
					SetDialogControlValue(dp,kNewIconS8Index,0);
					SetDialogControlValue(dp,kNewIconS4Index,0);
					SetDialogControlValue(dp,kNewIconS32Index,0);
					SetDialogControlValue(dp,kNewIconS8MIndex,0);
					gSelector.selector &= ~(kSelectorSmall8Bit+kSelectorSmall4Bit+
											kSelectorSmall32Bit+kSelectorSmall8BitMask);
				}
				MyInvalWindowRect(GetDialogWindow(dp),&iconRect);
				break;
			
			case kNewIconT32Index:
			case kNewIconT8MIndex:
				/* it32 / t8mk */
				tempBool = GetDialogControlValue(dp,item);
				if (tempBool && !GetDialogControlValue(dp,kNewIconL1Index) &&
					!GetDialogControlValue(dp,kNewIconS1Index)) break;
				
				SetDialogControlValue(dp,kNewIconT32Index,1-tempBool);
				SetDialogControlValue(dp,kNewIconT8MIndex,1-tempBool);
				
				gSelector.it32 = gSelector.t8mk = 1-tempBool;
				//MyInvalWindowRect(GetDialogWindow(dp),&iconRect);
				break;
			
			case kNewIconSelectAllIndex:
				/* select all */
				gSelector.selector = GetMySelector();
				if (gSystemVersion >= 0x0850)
					gSelector.it32 = gSelector.t8mk = true;
				else
					gSelector.it32 = gSelector.t8mk = false;
				
				AddNewIconReset(dp);
				break;
			
			case kNewIconSelectNoneIndex:
				/* select none */
				gSelector.selector = 0;
				gSelector.it32 = gSelector.t8mk = false;
				
				AddNewIconReset(dp);
				break;
			
			case kNewIconForKT7Index:
				/* for KT7 */
				gSelector.selector = kSelectorMyData;
				gSelector.it32 = gSelector.t8mk = false;
				
				AddNewIconReset(dp);
				break;
			
			case kNewIconFor85Index:
				/* for OS8.5 */
				gSelector.selector = kSelectorLarge8Bit + kSelectorLarge1Bit + 
					kSelectorSmall8Bit + kSelectorSmall1Bit + 
					kSelectorLarge32Bit + kSelectorLarge8BitMask + 
					kSelectorSmall32Bit + kSelectorSmall8BitMask;
				gSelector.it32 = gSelector.t8mk = false;
				
				AddNewIconReset(dp);
				break;
			
			case kNewIconForXIndex:
				/* for X */
				gSelector.selector = kSelectorLarge8Bit + kSelectorLarge1Bit + 
					kSelectorSmall8Bit + kSelectorSmall1Bit + 
					kSelectorLarge32Bit + kSelectorLarge8BitMask + 
					kSelectorSmall32Bit + kSelectorSmall8BitMask;
				gSelector.it32 = gSelector.t8mk = true;
				
				AddNewIconReset(dp);
				break;
			
			case kNewIconLargeOnlyIndex:
				/* Large icon only */
				gSelector.selector = kSelectorLarge8Bit + kSelectorLarge4Bit + 
					kSelectorLarge1Bit + kSelectorLarge32Bit + kSelectorLarge8BitMask;
				gSelector.it32 = gSelector.t8mk = false;
				
				AddNewIconReset(dp);
				break;
			
			case kNewIconSmallOnlyIndex:
				/* small icon only */
				gSelector.selector = kSelectorSmall8Bit + kSelectorSmall4Bit + 
					kSelectorSmall1Bit + kSelectorSmall32Bit + kSelectorSmall8BitMask;
				gSelector.it32 = gSelector.t8mk = false;
				
				AddNewIconReset(dp);
				break;
			
			case kNewIconThumbnailOnlyIndex:
				gSelector.selector = 0;
				gSelector.it32 = gSelector.t8mk = true;
				
				AddNewIconReset(dp);
				break;
			
			case kNewIconTemplateIndex:
				/* update icon */
				{
					short newValue = GetDialogControlValue(dp,kNewIconTemplateIndex)-1;
					if (newValue != gSelectedTemplate)
					{
						gSelectedTemplate = newValue;
						
						MyInvalWindowRect(GetDialogWindow(dp),&iconRect);
					}
				}
				break;
		}
	}
	
	SetPort(port);
	
	if (item==ok)
	{
		newIcon->resID=temp;
		GetDialogItemText2(dp,kNewIconNameIndex,newIcon->resName);
		
		*selector = gSelector;
		
		*iconTemplate = gSelectedTemplate;
	}
	DisposeModalFilterUPP(mfUPP);
	DisposeDialog(dp);
	ActivateFloatersAndFirstDocumentWindow();
	
	return (item == ok ? noErr : userCanceledErr);
}

/* ダイアログのフィルタ（アイコンのアップデート） */
pascal Boolean NewIconUpdateFilter(DialogPtr theDialog,EventRecord *theEvent,short *theItemHit)
{
	Boolean		eventHandled = false;
	WindowPtr	theWindow;
	GrafPtr		port;
	Rect		r;
	OSErr		err;
	
	switch (theEvent->what)
	{
		case updateEvt:
			theWindow=(WindowPtr)theEvent->message;
			if (theWindow!=nil && theWindow==GetDialogWindow(theDialog))
			{
				/* update dialog */
				GetPort(&port);
				SetPortWindowPort(theWindow);
				BeginUpdate(theWindow);
				
				DrawDialog(theDialog);
				GetDialogItemRect(theDialog,kNewIconIconIndex,&r);
				EraseRect(&r);
				
				if (gSelectedTemplate > 0)
				{
					IPIconRec	ipIcon;
					
					err = GetIPIconFromTemplate(&ipIcon,&gSelector,gSelectedTemplate);
					if (err == noErr)
					{
						err = PlotIconSuite(&r,kAlignNone,kTransformNone,
							ipIcon.iconSuite);
						err = DisposeIPIcon(&ipIcon);
					}
				}
				
				/* frame of icon */
				InsetRect(&r,-2,-2);
				FrameRect(&r);
				
				EndUpdate(theWindow);
				SetPort(port);
				
				*theItemHit = -1;
				eventHandled = true;
			}
			break;
	}
	
	if (!eventHandled)
		eventHandled = MyModalDialogFilter(theDialog,theEvent,theItemHit);
	
	return eventHandled;
}

/* get IPIcon from templates */
OSErr GetIPIconFromTemplate(IPIconRec *ipIcon,const IPIconSelector *selector,short iconTemplate)
{
	OSErr	err;
	
	if (iconTemplate == 0) /* not use template */
	{
		UseResFile(gApplRefNum);
		err=Get1IPIcon(ipIcon,2000,selector);
		
		return err;
	}
	
	if (isThumbnailIconsAvailable)
	{
		IconRef				theIconRef;
		IconFamilyHandle	theIconFamily;
		OSErr				igErr;
		
		err = GetIconRef(kOnSystemDisk,kSystemIconsCreator,
				gTemplateIconType[iconTemplate],&theIconRef);
		if (err!=noErr) return err;
		
		err = IconRefToIconFamily(theIconRef,kSelectorAllAvailableData,&theIconFamily); // 1.20b14
		if (err!=noErr)
		{
			igErr = ReleaseIconRef(theIconRef);
			return err;
		}
		
		err = IconFamilyToIPIconWithSelector(theIconFamily,selector,ipIcon);
		DisposeHandle((Handle)theIconFamily);
		
		return err;
	}
	else
	{
		IconSuiteRef		theIconSuite;
		
		err = GetIconSuite(&theIconSuite,
				gTemplateIconResource[iconTemplate],
				selector->selector);
		if (err!=noErr) return err;
		
		ipIcon->iconSuite = theIconSuite;
		ipIcon->it32Data = NULL;
		ipIcon->t8mkData = NULL;
		
		return noErr;
	}
}

/* 追加ダイアログのチェックを初期化 */
void AddNewIconReset(DialogPtr dp)
{
	short	item;
	Rect	iconRect;
	
	for (item=kNewIconL8Index; item<=kNewIconS8MIndex; item++)
		SetDialogControlValue(dp,item,
			(gSelector.selector & gSelectorList[item-kNewIconL8Index])!=0);
	
	SetDialogControlValue(dp,kNewIconT32Index,gSelector.it32);
	SetDialogControlValue(dp,kNewIconT8MIndex,gSelector.t8mk);
	
	/* update icon */
	GetDialogItemRect(dp,kNewIconIconIndex,&iconRect);
	MyInvalWindowRect(GetDialogWindow(dp),&iconRect);
}

/* アイコン保存 */
OSErr SaveAsIconDialog(Str255 filename,FSSpec *theFile,OSType *iconType,NavReplyRecord *theReply)
{
	OSErr	err;
	
	InitCursor();
	DeactivateFloatersAndFirstDocumentWindow();
	
	#if !TARGET_API_MAC_CARBON
	if (isNavServicesAvailable && useNavigationServices)
	#endif
	{
		err=ExportIconWithNav(filename,theFile,theReply,iconType);
	}
	#if !TARGET_API_MAC_CARBON
	else
	{
//		Point			where={-1,-1};
//		ActivateYDUPP	aUPP=NewActivateYDProc(MyActivate);
		StandardFileReply	reply;
		
		*iconType = 'Icon';
		ExportIconDialog(theFile->name,&reply,iconType);
//		CustomPutFile(str,iconName,&reply,148,where,nil,nil,nil,aUPP,nil);
//		DisposeRoutineDescriptor(aUPP);
		
		if (reply.sfGood && reply.sfReplacing)
		{
			err=FSpDelete(&reply.sfFile);
			if (err!=noErr) return err;
		}
		err=(reply.sfGood ? noErr : userCanceledErr);
		if (err==noErr) *theFile=reply.sfFile;
	}
	#endif
	ActivateFloatersAndFirstDocumentWindow();
	
	return err;
}

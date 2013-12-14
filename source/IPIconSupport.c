/* ------------------------------------------------------------ */
/*  IPIconSupport.c                                             */
/*     support routines for IPIcon (IconParty original format)  */
/*                                                              */
/*                 2001.7.4 - 2002.7.1  naoki iimura        	*/
/* ------------------------------------------------------------ */

/* includes */
#if __APPLE_CC__
#include	<MoreFilesX.h>
#else
#include	<MoreFilesExtras.h>
#endif

#include	"Definition.h"
#include	"Globals.h"
#include	"IconRoutines.h"
#include	"UsefulRoutines.h"
#include	"MenuRoutines.h"
#include	"PreCarbonSupport.h"
#if !TARGET_API_MAC_CARBON
#include	"Find_icon.h"
#endif

#include	"IPIconSupport.h"


static void	UpdateFinderIcon(const FSSpec *theFile);

static OSErr	FolderIconCheck(const FSSpec *theFolderSpec,short *alertMode);
static OSErr	FileIconCheck(const FSSpec *theIconFile,short *alertMode);


const extern short		gIconSize[];
const extern short		gIconDepth[];
const extern ResType	gIconType[];

const static Rect iconPreviewRect[]={
				{0x07,0x08,0x27,0x28},{0x07,0x2e,0x27,0x4e},{0x17,0x53,0x27,0x63},{0x17,0x67,0x27,0x77},
				{0x3f,0x08,0x5f,0x28},{0x3f,0x2e,0x5f,0x4e},{0x4f,0x53,0x5f,0x63},{0x4f,0x67,0x5f,0x77},
				{0x77,0x08,0x97,0x28},{0x77,0x2e,0x97,0x4e},{0x87,0x53,0x97,0x63},{0x87,0x67,0x97,0x77},
				{0x17,0x00,0x97,0x80}};


/* convert IconFamily to IPIcon */
OSErr	IconFamilyToIPIcon(IconFamilyHandle theIconFamily,IPIconRec *ipIcon)
{
	IPIconSelector	ipSelector;
	
	ipSelector.selector = GetMySelector();
	ipSelector.it32 = ipSelector.t8mk = isThumbnailIconsAvailable;
	
	return IconFamilyToIPIconWithSelector(theIconFamily,&ipSelector,ipIcon);
}

OSErr IconFamilyToIPIconWithSelector(IconFamilyHandle theIconFamily,
	const IPIconSelector *ipSelector,IPIconRec *ipIcon)
{
	OSErr		err=noErr;
	Handle		dataHandle;
	long		dataSize;
	
	if (ipSelector->selector != 0)
		err=IconFamilyToIconSuite(theIconFamily,ipSelector->selector,&ipIcon->iconSuite);
	else
		ipIcon->iconSuite = NULL;
	
	if (err==noErr)
	{
		if (isThumbnailIconsAvailable)
		{
			/* the others */
			if (ipSelector->it32)
			{
				dataHandle=NewHandle(0);
				err=GetIconFamilyData(theIconFamily,kThumbnail32BitData,dataHandle);
				if (err==noErr)
				{
					dataSize = GetHandleSize(dataHandle);
					if (dataSize > 0)
					{
						ipIcon->it32Data = dataHandle;
						HandToHand(&ipIcon->it32Data);
					}
					else
						ipIcon->it32Data = NULL;
				}
				else if (err == noIconDataAvailableErr) // 1.20b14
				{
					err = noErr;
					ipIcon->it32Data = NULL;
				}
				
				DisposeHandle(dataHandle);
			}
			else
				ipIcon->it32Data = NULL;
			
			if (ipSelector->t8mk)
			{
				dataHandle=NewHandle(0);
				err=GetIconFamilyData(theIconFamily,kThumbnail8BitMask,dataHandle);
				if (err==noErr)
				{
					dataSize = GetHandleSize(dataHandle);
					if (dataSize > 0)
					{
						ipIcon->t8mkData = dataHandle;
						HandToHand(&ipIcon->t8mkData);
					}
					else
						ipIcon->t8mkData = NULL;
				}
				else if (err == noIconDataAvailableErr) // 1.20b14
				{
					err = noErr;
					ipIcon->t8mkData = NULL;
				}
				
				DisposeHandle(dataHandle);
			}
			else
				ipIcon->t8mkData = NULL;
		}
		else
		{
			ipIcon->it32Data = NULL;
			ipIcon->t8mkData = NULL;
		}
	}
	return err;
}

/* convert IPIcon to IconFamily */
OSErr	IPIconToIconFamily(const IPIconRec *ipIcon,IconFamilyHandle *iconFamily)
{
	OSErr	err=noErr;
	short	curRefNum = CurResFile();
	
	if (ipIcon->iconSuite == NULL)
	{
		UseResFile(gApplRefNum);
		*iconFamily=(IconFamilyHandle)Get1Resource(kXIconClipType,2002);
		DetachResource((Handle)*iconFamily);
		HUnlock((Handle)*iconFamily);
		UseResFile(curRefNum);
	}
	else
	{
		err=IconSuiteToIconFamily(ipIcon->iconSuite,kSelectorMy32Data,iconFamily);
		if (err==memFullErr)
		{
			UseResFile(gApplRefNum);
			*iconFamily=(IconFamilyHandle)Get1Resource(kXIconClipType,2002);
			DetachResource((Handle)*iconFamily);
			HUnlock((Handle)*iconFamily);
			err=noErr;
			UseResFile(curRefNum);
		}
	}
	if (err==noErr && isThumbnailIconsAvailable)
	{
		/* thumbnail */
		if (ipIcon->it32Data != NULL)
			err=SetIconFamilyData(*iconFamily,kThumbnail32BitData,ipIcon->it32Data);
		if (ipIcon->t8mkData != NULL)
			err=SetIconFamilyData(*iconFamily,kThumbnail8BitMask,ipIcon->t8mkData);
	}
	
	return err;
}

/* convert icns(icons for MacOS X) to IPIcon */
OSErr	XIconToIPIcon(const FSSpec *theFile,IPIconRec *ipIcon)
{
	OSErr	err;
	IconFamilyHandle	theIconFamily;
	short		refNum;
	long		count;
	
	if (isIconServicesAvailable)
	{
		/* open icns file */
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
		
		/* convert IconFamily to IPIcon */
		err=IconFamilyToIPIcon(theIconFamily,ipIcon);
		
		DisposeHandle((Handle)theIconFamily);
		return err;
	}
	else
		return -1;
}

/* get IPIcon from file icon */
OSErr GetFileIPIcon(const FSSpec *theFile,IPIconRec *ipIcon,Boolean isFolder,Boolean *is32Exist,short *alertMode)
{
	#if TARGET_API_MAC_CARBON
	#pragma unused(isFolder)
	#endif
	OSErr		err=noErr;
	
	if (isFolder)
		err=FolderIconCheck(theFile,alertMode);
	else
		err=FileIconCheck(theFile,alertMode);
	
	#if !TARGET_API_MAC_CARBON
	if (isIconServicesAvailable)
	#endif
	{
		/* use IconServices when runinng on 8.5 or later */
		IconRef		iconRef;
		short		label;
		IconFamilyHandle	iconFamily;
		
		err=GetIconRefFromFile(theFile,&iconRef,&label);
		if (err!=noErr) return err;
		
		//err=IconRefToIconFamily(iconRef,GetMySelector(),&iconFamily);
		err=IconRefToIconFamily(iconRef,kSelectorAllAvailableData,&iconFamily); // 1.20b14
		ReleaseIconRef(iconRef);
		if (err!=noErr) return err;
		
		err=IconFamilyToIPIcon(iconFamily,ipIcon);
		DisposeHandle((Handle)iconFamily);
		
		if (err==noErr)
			err=IPIconHas32Icons(ipIcon,is32Exist);
		return err;
	}
	#if !TARGET_API_MAC_CARBON
	else
	{
		/* use Find_icon when running on older than 8.5 */
		err=Find_icon(theFile,NULL,kSelectorMyData,&ipIcon->iconSuite);
		ipIcon->it32Data=NULL;
		ipIcon->t8mkData=NULL;
		
		return err;
	}
	#endif
}

/* create folder with IPIcon */
OSErr MakeFolderWithIPIcon(const FSSpec *theFolder,const IPIconRec *ipIcon)
{
	OSErr		err;
	long		dirID;
	FSSpec		theIconFile;
	Str15		iconFileName;
	#ifdef __MOREFILESX__
	FSRef		fsRef;
	
	#endif
	
	/* create a folder */
	err=FSpDirCreate(theFolder,smSystemScript,&dirID);
	if (err==dupFNErr || err==dirNFErr)
	{
		Boolean	isDirectory;
		
		#ifdef __MOREFILESX__
		err = FSpMakeFSRef(theFolder,&fsRef);
		err = FSGetNodeID(&fsRef,&dirID,&isDirectory);
		#else
		err=FSpGetDirectoryID(theFolder,&dirID,&isDirectory);
		#endif
		if (!isDirectory) return -1;
	}
	if (err!=noErr) return err;
	
	#ifdef __MOREFILESX__
	err = FSpMakeFSRef(theFolder,&fsRef);
	#endif
	
	/* create icon file */
	GetIndString(iconFileName,140,3);
	err=FSMakeFSSpec(theFolder->vRefNum,dirID,iconFileName,&theIconFile);
	if (err==fnfErr)
	{
		FInfo	fndrInfo;
		
		FSpCreateResFile(&theIconFile,kFinderCreator,'icon',smSystemScript);
		err=FSpGetFInfo(&theIconFile,&fndrInfo);
		fndrInfo.fdFlags |= kIsInvisible;
		err=FSpSetFInfo(&theIconFile,&fndrInfo);
	}
	
	/* save icon data */
	if (err==noErr)
	{
		short	refNum;
		IconActionUPP	addIconUPP;
		MyIconResRec	newIcon;
		#ifndef __MOREFILESX__
		DInfo			dInfo;
		#endif
		
		newIcon.resID=kCustomIconResource;
		GetIndString(newIcon.resName,141,2);
		newIcon.attrs=0;
		
		refNum=FSpOpenResFile(&theIconFile,fsWrPerm);
		UseResFile(refNum);
		
		/* save icon family(separated icons) */
		if (ipIcon->iconSuite != NULL)
		{
			addIconUPP = NewIconActionUPP(AddIconToFile);
			err=ForEachIconDo(ipIcon->iconSuite,kSelectorMyData,addIconUPP,&newIcon);
			DisposeIconActionUPP(addIconUPP);
		}
		
		/* save icns(single icon) */
		if (isIconServicesAvailable)
		{
			IconFamilyHandle	iconFamily;
			
			err=IPIconToIconFamily(ipIcon,&iconFamily);
			if (err==noErr)
			{
				SaveDataToResource(*iconFamily,GetHandleSize((Handle)iconFamily),
					kIconFamilyType,newIcon.resID,newIcon.resName,newIcon.attrs);
				DisposeHandle((Handle)iconFamily);
			}
		}
		else
			/* delete icns resource */
			DeleteIconFamilyResource();
		
		CloseResFile(refNum);
		UseResFile(gApplRefNum);
		
		/* set flag for custom icons */
		#ifdef __MOREFILESX__
		err = FSSetHasCustomIcon(&fsRef);
		err = FSClearHasBeenInited(&fsRef);
		#else
		err=FSpGetDInfo(theFolder,&dInfo);
		dInfo.frFlags |= kHasCustomIcon;
		dInfo.frFlags &= ~kHasBeenInited;
		err=FSpSetDInfo(theFolder,&dInfo);
		#endif
		
		gUsedCount.exportNum++;
	}
	
	FlushVol(0L,theFolder->vRefNum);
	
	/* update folder icon */
	UpdateFinderIcon(theFolder);
	
	return err;
}

/* create file with IPIcon */
OSErr MakeFileWithIPIcon(const FSSpec *theFile,const IPIconRec *ipIcon)
{
	OSErr		err;
	FInfo		fndrInfo;
	short		refNum;
	IconActionUPP	addIconUPP;
	MyIconResRec	newIcon;
	
	/* create a file */
	err=FSpGetFInfo(theFile,&fndrInfo);
	if (err==fnfErr)
		FSpCreateResFile(theFile,kResEditCreator,kResourceFileType,smSystemScript);
	if (err!=fnfErr && err!=noErr) return err;
	
	/* open the file */
	refNum=FSpOpenResFile(theFile,fsWrPerm);
	if (refNum < 0) /* could not open -> the file has no resource fork */
	{
		FSpCreateResFile(theFile,fndrInfo.fdCreator,fndrInfo.fdType,smSystemScript);
		refNum=FSpOpenResFile(theFile,fsWrPerm);
	}
	if (refNum < 0) /* could not open -> error */
		return refNum;
	
	UseResFile(refNum);
	
	/* information for icon */
	newIcon.resID=kCustomIconResource;
	GetIndString(newIcon.resName,141,2);
	newIcon.attrs=0;
	
	/* save icon family(separated icons) */
	if (ipIcon->iconSuite != NULL)
	{
		addIconUPP=NewIconActionUPP(AddIconToFile);
		err=ForEachIconDo(ipIcon->iconSuite,GetMySelector(),addIconUPP,&newIcon);
		DisposeIconActionUPP(addIconUPP);
	}
	
	/* save icns(single icon) */
	if (isIconServicesAvailable)
	{
		IconFamilyHandle	iconFamily;
		
		err=IPIconToIconFamily(ipIcon,&iconFamily);
		if (err==noErr)
		{
			SaveDataToResource(*iconFamily,GetHandleSize((Handle)iconFamily),
				kIconFamilyType,newIcon.resID,newIcon.resName,newIcon.attrs);
			DisposeHandle((Handle)iconFamily);
		}
	}
	else
		/* delete icns resource */
		DeleteIconFamilyResource();
	
	CloseResFile(refNum);
	UseResFile(gApplRefNum);
	
	/* set flag for custom icons */
	err=FSpGetFInfo(theFile,&fndrInfo);
	fndrInfo.fdFlags |= kHasCustomIcon;
	fndrInfo.fdFlags &= ~kHasBeenInited;
	err=FSpSetFInfo(theFile,&fndrInfo);
	
	FlushVol(0L,theFile->vRefNum);
	
	/* update file icon */
	UpdateFinderIcon(theFile);
	
	return err;
}

/* update icons in Finder */
void UpdateFinderIcon(const FSSpec *theFile)
{
	OSErr	err;
	
	/* update icons */
	if (isOSX)
	{
		AppleEvent		aeEvent={typeNull,NULL};
		
		/* send AppleEvnet that class=kAEFinderSuite and id=kAESync to Finder */
		err=MakeUpdateEvent(theFile,&aeEvent);
		if (err!=noErr) return;
		
		err=AESend(&aeEvent,nil,kAENoReply+kAEAlwaysInteract+kAECanSwitchLayer,
				kAENormalPriority,kNoTimeOut,nil,nil);
		
		AEDisposeDesc(&aeEvent);
	}
	else
	{
		if (isIconServicesAvailable)
			FlushIconRefsByVolume(theFile->vRefNum);
		else
		{
			/* change the modification data of the parent folder */
			FSSpec	parentFolder;
			
			err=FSMakeFSSpec(theFile->vRefNum,theFile->parID,NULL,&parentFolder);
			if (err==noErr)
			{
				#ifdef __MOREFILESX__
				FSRef	fsRef;
				
				err = FSpMakeFSRef(&parentFolder,&fsRef);
				err = FSBumpDate(&fsRef);
				
				#else
				err=FSpBumpDate(&parentFolder);
				#endif
			}
		}
	}
}

/* create icns(icon for MacOS X) with IPIcon */
OSErr MakeXIconWithIPIcon(const FSSpec *theFile,const IPIconRec *ipIcon)
{
	OSErr	err;
	FInfo	fndrInfo;
	short	refNum;
	IconFamilyHandle	iconFamily;
	long	count;
	
	if (!isIconServicesAvailable) return -1;
	
	/* convert IPIcon to icns */
	err=IPIconToIconFamily(ipIcon,&iconFamily);
	if (err!=noErr) return err;
	
	/* create a file */
	err=FSpGetFInfo(theFile,&fndrInfo);
	if (err==fnfErr)
		err=FSpCreate(theFile,kIconPartyCreator,kXIconFileType,smSystemScript);
	if (err!=noErr) return err;
	
	/* open the file */
	err=FSpOpenDF(theFile,fsWrPerm,&refNum);
	if (err!=noErr) return err;
	
	/* save icns */
	HLock((Handle)iconFamily);
	count=GetHandleSize((Handle)iconFamily);
	err=FSWrite(refNum,&count,*iconFamily);
	err=SetEOF(refNum,count);
	HUnlock((Handle)iconFamily);
	DisposeHandle((Handle)iconFamily);
	
	/* close the file */
	err=FSClose(refNum);
	
	return noErr;
}

/* get icon data from IPIcon */
OSErr	GetDataFromIPIcon(Handle *dataHandle,const IPIconRec *ipIcon,short iconKind)
{
	OSErr	err=noErr;
	
	*dataHandle = NULL;
	
	if (iconKind == kT32Data)
		*dataHandle = ipIcon->it32Data;
	else if (iconKind == kT8Mask)
		*dataHandle = ipIcon->t8mkData;
	else
	{
		if (ipIcon->iconSuite != NULL)
			err=GetIconFromSuite(dataHandle,ipIcon->iconSuite,gIconType[iconKind]);
	}
	
	return err;
}

/* set icon data to IPIcon */
OSErr	SetDataToIPIcon(Handle dataHandle,IPIconRec *ipIcon,short iconKind)
{
	OSErr	err=noErr;
	
	if (iconKind == kT32Data)
		ipIcon->it32Data = dataHandle;
	else if (iconKind == kT8Mask)
		ipIcon->t8mkData = dataHandle;
	else
	{
		if (ipIcon->iconSuite == NULL)
			err = NewIconSuite(&ipIcon->iconSuite);
		
		if (err!=noErr) return err;
		
		err=AddIconToSuite(dataHandle,ipIcon->iconSuite,gIconType[iconKind]);
	}
	
	return err;
}

extern Rect	familyIconRect[];

/* convert IPIcon to Picture */
PicHandle IPIconToPicture(const IPIconRec *ipIcon,short iconKind)
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
	Rect	temp;
	long	rowBytes,iconRowBytes;
	short	i;
	CTabHandle	ctab=nil;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	/* invalid icon kind */
	if (iconKind < 0) return nil;
	
	iconType=gIconType[iconKind];
	iconSize=gIconSize[iconKind];
	iconDepth=gIconDepth[iconKind];
	
	SetRect(&iconRect,0,0,iconSize,iconSize);
	temp=iconRect;
	temp.right++;
	
	err=GetDataFromIPIcon(&dataHandle,ipIcon,iconKind);
	
	if (err != noErr || dataHandle == nil) return nil; /* return nil when there are no icons */
	if (GetHandleSize(dataHandle) != iconSize*(long)iconSize*(iconDepth>1 ? iconDepth : 2)/8) return nil;
	
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
	
	GetGWorld(&cPort,&cDevice);
	err=NewGWorld(&iconGWorld,iconDepth,&temp,ctab,0,useTempMem);
	if (ctab != NULL) DisposeHandle((Handle)ctab);
	if (err != noErr) return nil;
	
	HLock(dataHandle);
	
	SetGWorld(iconGWorld,0);
	iconPix=GetGWorldPixMap(iconGWorld);
	LockPixels(iconPix);
	rowBytes=MyGetPixRowBytes(iconPix) & 0x3fff;
	EraseRect(&iconRect);
	
	src=*dataHandle+offset;
	dst=MyGetPixBaseAddr(iconPix);
	iconRowBytes=iconSize*iconDepth/8;
	for (i=0; i<iconSize; i++)
	{
		BlockMoveData(src,dst,iconRowBytes);
		src+=iconRowBytes;
		dst+=rowBytes;
	}
	HUnlock(dataHandle);
	UnlockPixels(iconPix);
	
	picParam.srcRect=iconRect;
	picParam.hRes=72L<<16;
	picParam.vRes=72L<<16;
	picParam.version=-2;
	picParam.reserved1=0;
	picParam.reserved2=0;
	
	picture=OpenCPicture(&picParam);
	
	ForeColor(blackColor);
	BackColor(whiteColor);
	
	ClipRect(&iconRect);
	EraseRect(&iconRect);
	#if 1
	CopyBits(GetPortBitMapForCopyBits(iconGWorld),GetPortBitMapForCopyBits(iconGWorld),
		&iconRect,&iconRect,srcCopy,nil);
	#else
	{
		IconFamilyHandle	iconFamily;
		IconRef				iconRef;
		
		err=IPIconToIconFamily(ipIcon,&iconFamily);
		err=RegisterIconRefFromIconFamily(kIconPartyCreator,'TEMP',iconFamily,&iconRef);
		DisposeHandle((Handle)iconFamily);
		
		err=PlotIconRef(&iconRect,kAlignNone,kTransformNone,kIconServicesNormalUsageFlag,
				iconRef);
		err=ReleaseIconRef(iconRef);
	}
	#endif
#if __BIG_ENDIAN__
	(**picture).picFrame=iconRect;
#endif
	ClosePicture();
	
	SetGWorld(cPort,cDevice);
	
	#if 0
	CopyBits(GetPortBitMapForCopyBits(iconGWorld),GetPortBitMapForCopyBits(GetWindowPort(MyFrontNonFloatingWindow())),
		&iconRect,&familyIconRect[iconKind],srcCopy,nil);
	
	DrawPicture(picture,&familyIconRect[iconKind]);
	#endif
	
	DisposeGWorld(iconGWorld);
	
	return picture;
}

/* IPIconに32bitデータあるいは8bitマスクが存在するかどうか */
OSErr	IPIconHas32Icons(const IPIconRec *ipIcon,Boolean *is32Exist)
{
	OSErr	err=noErr;
	Handle	dataHandle=NULL;
	short	iconList32[]={kL32Data,kL8Mask,kS32Data,kS8Mask,kT32Data,kT8Mask};
	short	i;
	
    if (ipIcon->it32Data || ipIcon->t8mkData) {
        *is32Exist = true;
        return noErr;
    }
    
	for (i=0; i<6; i++)
	{
		err=GetDataFromIPIcon(&dataHandle,ipIcon,iconList32[i]);
		if (err != noErr) return err;
		if (dataHandle)
		{
			*is32Exist = true;
			return noErr;
		}
	}
	
	*is32Exist = false;
	return noErr;
}

/* IPIconにThumbnailアイコンが存在するかどうか */
OSErr	IPIconHasThumbnailIcon(const IPIconRec *ipIcon,Boolean *isThumbnailExist)
{
	*isThumbnailExist = (ipIcon->it32Data != NULL || ipIcon->t8mkData != NULL);
	return noErr;
}

/* IPIconにThumbnailアイコンが存在するかどうか */
Boolean IsIPIconHasThumbnailIcon(const IPIconRec *ipIcon)
{
	Boolean	result = false;
	OSErr	err;
	
	err = IPIconHasThumbnailIcon(ipIcon,&result);
	
	return result;
}

/* IPIconの破棄 */
OSErr	DisposeIPIcon(const IPIconRec *ipIcon)
{
	OSErr	err=noErr;
	
	if (ipIcon->iconSuite != NULL)
		err=DisposeIconSuite(ipIcon->iconSuite,true);
	
	if (ipIcon->it32Data != NULL)
		DisposeHandle(ipIcon->it32Data);
	if (ipIcon->t8mkData != NULL)
		DisposeHandle(ipIcon->t8mkData);
	
	return err;
}

/* フォルダアイコンをチェック */
OSErr FolderIconCheck(const FSSpec *theFolderSpec,short *alertMode)
{
	OSErr	err;
	long	dirID;
	FSSpec	theIconFile;
	Str15	iconFileName;
	Boolean	isDirectory;
	#ifdef __MOREFILESX__
	FSRef	fsRef;
	FinderInfo	info;
	
	err = FSpMakeFSRef(theFolderSpec,&fsRef);
	if (err != noErr) return err;
	#else
	DInfo	dirInfo;
	#endif
	
	/* まず、カスタムアイコンフラグが立っているかどうかを調べる */
	/* ここでエラーが発生する場合はおそらくフォルダアイコン編集も不可能なのでそのまま返る */
	#ifdef __MOREFILESX__
	err = FSGetFinderInfo(&fsRef,&info,NULL,NULL);
	#else
	err=FSpGetDInfo(theFolderSpec,&dirInfo);
	#endif
	if (err!=noErr) return err;
	
	/* カスタムアイコンフラグが立っていなければ問題なし */
	#ifdef __MOREFILESX__
	if ((info.folder.finderFlags & kHasCustomIcon) == 0) return noErr;
	#else
	if ((dirInfo.frFlags & kHasCustomIcon) == 0) return noErr;
	#endif
	
	/* 立っている場合は、カスタムアイコンをチェック */
	#ifdef __MOREFILESX__
	err = FSGetNodeID(&fsRef,&dirID,&isDirectory);
	#else
	err=FSpGetDirectoryID(theFolderSpec,&dirID,&isDirectory);
	#endif
	if (err!=noErr) return err;
	
	GetIndString(iconFileName,140,3);
	err=FSMakeFSSpec(theFolderSpec->vRefNum,dirID,iconFileName,&theIconFile);
	if (err==fnfErr) /* アイコンファイルが見つからない場合は、フラグが間違っているわけだから修正 */
	{
		#ifdef __MOREFILESX__
		err = FSClearHasCustomIcon(&fsRef);
		#else
		err=FSpClearHasCustomIcon(theFolderSpec);
		#endif
		return noErr;
	}
	else if (err!=noErr) /* それ以外のエラーなら編集できないだろうから戻る */
		return err;
	
	return FileIconCheck(&theIconFile,alertMode);
}

/* ファイルのアイコンをチェック */
OSErr FileIconCheck(const FSSpec *theIconFile,short *alertMode)
{
	OSErr	err;
	short	refNum;
	IconFamilyHandle	iconFamily;
	
	refNum=FSpOpenResFile(theIconFile,fsRdWrPerm);
	if (refNum <=0) /* 開けない場合は追加確認 */
	{
		if ((err=ResError())==eofErr)
		{
			*alertMode = kAddResourceForkMode;
			return noErr;
		}
		return err;
	}
	
	/* 'icns'リソースを探す */
	UseResFile(refNum);
	iconFamily=(IconFamilyHandle)Get1Resource(kXIconFileType,kCustomIconResource);
	if (iconFamily==nil) /* 'icns'リソースがない場合は問題なし */
	{
		CloseResFile(refNum);
		UseResFile(gApplRefNum);
		return noErr;
	}
	
	if (!isIconServicesAvailable)
	{
		ReleaseResource((Handle)iconFamily);
		
		*alertMode = kDeleteIconFamilyMode;
	}
	else
		if (!IsEditableIconFamily(iconFamily))
			*alertMode = kDeleteIconMode;
	
	CloseResFile(refNum);
	UseResFile(gApplRefNum);
	
	return noErr;
}

/* アイコンプレビューのアップデート */
void DrawIPIconPreview(const IPIconRec *ipIcon)
{
	PicHandle	previewPict;
	Rect		r;
	Pattern		pat;
	Boolean		drawLarge=false,drawSmall=false,drawThumbnail=false;
	Handle		h;
	OSErr		err;
	IconSuiteRef	iconSuite=NULL;
	IconRef		iconRef;
    Rect        picRect;
	
	/* プレビューの大きさ */
	GetWindowPortBounds(gPreviewWindow,&r);
	
	/* まず背景 */
	switch (gListBackground)
	{
		case iBackWhite:
			EraseRect(&r);
			break;
		
		case iBackGray:
			GetIndPattern(&pat,0,4);
			FillRect(&r,&pat);
			break;
		
		case iBackBlack:
			PaintRect(&r);
			break;

		case iBackDesktop:
			#if !TARGET_API_MAC_CARBON
			{
				PixPatHandle	ppat;
				
				UseResFile(0);
	//			ppat=GetPixPat(16);
				ppat=LMGetDeskCPat();
				if (ppat==nil)
				{
					LMGetDeskPattern(&pat);
					FillRect(&r,&pat);
				}
				else
				{
					FillCRect(&r,ppat);
			//		DisposePixPat(ppat);
				}
				UseResFile(gApplRefNum);
			}
			#else
			EraseRect(&r);
			#endif
			break;

        default:
			EraseRect(&r);
			break;
}
	
	if (ipIcon!=NULL)
	{
		err=GetDataFromIPIcon(&h,ipIcon,kL1Data);
		drawLarge=(h!=nil);
		err=GetDataFromIPIcon(&h,ipIcon,kS1Data);
		drawSmall=(h!=nil);
		err=GetDataFromIPIcon(&h,ipIcon,kT32Data);
		drawThumbnail=(h!=nil);
	}
	
	if (drawThumbnail)
		previewPict = GetPicture(142);
	else
		previewPict = GetPicture(141);
    QDGetPictureBounds(previewPict, &picRect);
	DrawPicture(previewPict,&picRect);
	
	if (ipIcon==NULL) return;
	
	if (!isThumbnailIconsAvailable || !drawThumbnail)
	{
		iconSuite = ipIcon->iconSuite;
	}
	else
	{
		IconFamilyHandle	iconFamily;
		
		err=IPIconToIconFamily(ipIcon,&iconFamily);
		err=RegisterIconRefFromIconFamily(kIconPartyCreator,'TEMP',iconFamily,&iconRef);
	//	DisposeHandle((Handle)iconFamily);
	}
	
	if (isThumbnailIconsAvailable && drawThumbnail)
	{
		err=PlotIconRef(&iconPreviewRect[12],kAlignNone,kTransformNone,kIconServicesNormalUsageFlag,
					iconRef);
		err=ReleaseIconRef(iconRef);
	}
	else
	{
		if (iconSuite != NULL)
		{
			PlotIconSuite(&iconPreviewRect[0],kAlignNone,kTransformNone,iconSuite);
			PlotIconSuite(&iconPreviewRect[1],kAlignNone,kTransformSelected,iconSuite);
			
			if (drawLarge)
			{
				PlotIconSuite(&iconPreviewRect[4],kAlignNone,kTransformOpen,iconSuite);
				PlotIconSuite(&iconPreviewRect[5],kAlignNone,kTransformSelectedOpen,iconSuite);
				
				PlotIconSuite(&iconPreviewRect[8],kAlignNone,kTransformOffline,iconSuite);
				PlotIconSuite(&iconPreviewRect[9],kAlignNone,kTransformSelectedOffline,iconSuite);
			}
			PlotIconSuite(&iconPreviewRect[2],kAlignNone,kTransformNone,iconSuite);
			PlotIconSuite(&iconPreviewRect[3],kAlignNone,kTransformSelected,iconSuite);
			
			if (drawSmall)
			{
				PlotIconSuite(&iconPreviewRect[6],kAlignNone,kTransformOpen,iconSuite);
				PlotIconSuite(&iconPreviewRect[7],kAlignNone,kTransformSelectedOpen,iconSuite);
				
				PlotIconSuite(&iconPreviewRect[10],kAlignNone,kTransformOffline,iconSuite);
				PlotIconSuite(&iconPreviewRect[11],kAlignNone,kTransformSelectedOffline,iconSuite);
			}
		}
	}
}

/* IPIcon -> clipboard */
OSErr IPIconToClip(const IPIconRec *ipIcon)
{
	OSErr	err;
	IconFamilyHandle	iconFamily;
	IconActionUPP	copyIconDataUPP=NewIconActionUPP(CopyIconData);
	
	#if TARGET_API_MAC_CARBON
	{
		ScrapRef	scrap;
		
		err=ClearCurrentScrap();
		err=GetCurrentScrap(&scrap);
		err=ForEachIconDo(ipIcon->iconSuite,kSelectorMyData,copyIconDataUPP,scrap);
		
		err=IPIconToIconFamily(ipIcon,&iconFamily);
		if (err==noErr)
		{
			err=PutScrapFlavor(scrap,kIconFamilyType,0,GetHandleSize((Handle)iconFamily),*iconFamily);
			DisposeHandle((Handle)iconFamily);
		}
	}
	#else
	{
		long	result;
		
		result=ZeroScrap();
		err=ForEachIconDo(ipIcon->iconSuite,kSelectorMyData,copyIconDataUPP,nil);
		
		if (gSystemVersion >= 0x0850)
		{
			err=IPIconToIconFamily(ipIcon,&iconFamily);
			if (err==noErr)
			{
				result=PutScrap(GetHandleSize((Handle)iconFamily),kIconFamilyType,*iconFamily);
				DisposeHandle((Handle)iconFamily);
			}
		}
	}
	#endif
		
	DisposeIconActionUPP(copyIconDataUPP);
	
	return err;
}

/* カレントリソースから指定されたアイコンをIPIconとして得る */
OSErr Get1IPIcon(IPIconRec *ipIcon,short resID,const IPIconSelector *selector)
{
	OSErr	err;
	IconFamilyHandle	iconFamily;
	
	if (isIconServicesAvailable)
	{
		iconFamily = (IconFamilyHandle)Get1Resource(kIconFamilyType,resID);
		if (iconFamily != NULL)
		{
			/* IconSuiteにおさめる部分 */
			err = IconFamilyToIPIconWithSelector(iconFamily,selector,ipIcon);
			ReleaseResource((Handle)iconFamily);
			
			return err;
		}
	}
	
	err = Get1IconSuite(&ipIcon->iconSuite,resID,selector->selector);
	ipIcon->it32Data = ipIcon->t8mkData = NULL;
	
	return err;
}
/* ------------------------------------------------------------ */
/*  ExternalEditorSupport.c                                     */
/*      外部エディタのサポートルーチン                          */
/*                                                              */
/*                 2001.1.27 - 2000.1.27  naoki iimura        	*/
/* ------------------------------------------------------------ */

/* includes */
#ifdef __APPLE_CC__
#include	<Carbon/Carbon.h>
#else
#include	<Sound.h>
#include	<Script.h>
#endif

#include	"Definition.h"

#include	"ExternalEditorSupport.h"
#include	"UsefulRoutines.h"
#include	"FileRoutines.h"
#include	"IconRoutines.h"
#include	"IconFamilyWindow.h"
#include	"IPIconSupport.h"
#include	"PreCarbonSupport.h"

/* structures */
/* 外部エディタ編集用 */
struct MyXIconRec {
	FSSpec				tempFile;		/* 一時ファイル */
	UInt32				modDate;		/* ファイルの修正日時 */
	WindowPtr			parentWindow;	/* 親ウィンドウ（ファミリウィンドウ） */
	SInt16				iconKind;		/* アイコンの種類 */
	struct MyXIconRec	*next;
};
typedef struct MyXIconRec	MyXIconRec;

/* globals */
static MyXIconRec	*gXIconList=nil;


/* prototypes */
static OSErr	LoadXIcon(MyXIconRec *xIconPtr);
static OSErr	OpenByExternalEditorMain(FSSpec *tempFile);
static OSErr	OpenByExternalEditor2(MyXIconRec *xIconPtr);


/* routines */
/* 外部エディタで編集中のアイコンリストを更新 */
void UpdateXIconList(void)
{
	UInt32		modDate;
	MyXIconRec	*xIconPtr=gXIconList,
				*prevIconPtr=nil;
	OSErr		err;
	
	while (xIconPtr != nil)
	{
		err=FSpGetModDate(&xIconPtr->tempFile,&modDate);
		if (err==fnfErr)
		{
			if (prevIconPtr == nil)
				gXIconList=xIconPtr->next;
			else
				prevIconPtr->next=xIconPtr->next;
			DisposePtr((Ptr)xIconPtr);
		}
		else if (err==noErr)
		{
			if (modDate > xIconPtr->modDate)
				if (LoadXIcon(xIconPtr)==noErr)
					xIconPtr->modDate=modDate;
		}
		prevIconPtr=xIconPtr;
		xIconPtr=xIconPtr->next;
	}
}

/* 外部エディタで編集中のアイコンを読み込む */
OSErr LoadXIcon(MyXIconRec *xIconPtr)
{
	PicHandle	picture;
	IconFamilyWinRec	*fWinRec;
	GrafPtr		port;
	
	picture=LoadFromPict(&xIconPtr->tempFile);
	if (picture == nil) return -1;
	
	fWinRec=GetIconFamilyRec(xIconPtr->parentWindow);
	fWinRec->selectedIcon=xIconPtr->iconKind;
	
	SavePictureToIconData(picture,fWinRec,gOtherPrefs.maskAutoConvert);
	
	KillPicture(picture);
	
	GetPort(&port);
	SetPortWindowPort(xIconPtr->parentWindow);
	MyInvalWindowPortBounds(xIconPtr->parentWindow);
	SetPort(port);
	
	return noErr;
}

/* 外部エディタでアイコンを開く */
OSErr OpenByExternalEditor(WindowPtr fWindow)
{
	IconFamilyWinRec	*fWinRec;
	MyXIconRec			*xIconPtr;
	PicHandle			picture;
	FSSpec				tempFile;
	OSErr				err,igErr;
	
	fWinRec=GetIconFamilyRec(fWindow);
	picture=IPIconToPicture(&fWinRec->ipIcon,fWinRec->selectedIcon);
	if (picture==nil) return memFullErr;
	
	/* テンポラリファイルの作成 */
	err=MakeTempFile(&tempFile,false); /* data fork */
	if (err!=noErr)
	{
		KillPicture(picture);
		return err;
	}
	
	/* PICTデータの保存 */
	err=SaveAsPictMain(picture,&tempFile);
	if (err!=noErr)
	{
		KillPicture(picture);
		igErr=FSpDelete(&tempFile);
		return err;
	}
	KillPicture(picture);
	
	/* データを入れる */
	xIconPtr=(MyXIconRec *)NewPtr(sizeof(MyXIconRec));
	xIconPtr->tempFile=tempFile;
	err=FSpGetModDate(&tempFile,&xIconPtr->modDate);
	if (err!=noErr) xIconPtr->modDate=0;
	xIconPtr->parentWindow=fWindow;
	xIconPtr->iconKind=fWinRec->selectedIcon;
	
	/* つなぎなおし */
	xIconPtr->next=gXIconList;
	gXIconList=xIconPtr;
	
	return OpenByExternalEditorMain(&tempFile);
}

/* AppleEventを送付してファイルを開く */
OSErr OpenByExternalEditorMain(FSSpec *tempFile)
{
	ProcessSerialNumber	psn;
	OSErr				err=noErr;
	ProcessInfoRec		procInfo;
	Boolean				found=false;
	FSSpec				theFile;
	AppleEvent			aeEvent={typeNull,NULL};
	
	psn.highLongOfPSN=0;
	psn.lowLongOfPSN=kNoProcess;
	
	procInfo.processInfoLength=sizeof(ProcessInfoRec);
	procInfo.processName=nil;
	procInfo.processAppSpec=&theFile;
	procInfo.processLocation=nil;
	
	while(GetNextProcess(&psn)==noErr)
	{
		if (GetProcessInformation(&psn,&procInfo)==noErr)
		{
			if (EqualFile(&gExternalEditor,procInfo.processAppSpec))
			{
				found=true;
				break;
			}
		}
	}
	if (found)
	{
		err=MakeOpenDocumentEvent(&psn,tempFile,&aeEvent);
		if (err==noErr)
			/* Appleイベントを送付する */
			err=AESend(&aeEvent,nil,kAENoReply+kAECanSwitchLayer+kAEAlwaysInteract,
						kAENormalPriority,kNoTimeOut,nil,nil);
		err=AEDisposeDesc(&aeEvent);
		
		err=SetFrontProcess(&psn);
	}
	else
		AEOpenFileWithApplication(tempFile,&gExternalEditor);
	
	return err;
}

/* 外部エディタで編集中かどうかを調べる */
Boolean IsOpenedByExternalEditor(WindowPtr fWindow,short iconKind,short mode)
{
	MyXIconRec	*xIconPtr=gXIconList;
	Boolean		found=false;
	OSErr		err;
	
	while (xIconPtr != nil)
	{
		if (xIconPtr->parentWindow == fWindow && xIconPtr->iconKind == iconKind)
		{
			found=true;
			break;
		}
		xIconPtr=xIconPtr->next;
	}
	
	if (found)
		switch (mode)
		{
			case kXIconOpenMode:
				err=OpenByExternalEditor2(xIconPtr);
				if (err!=noErr) SysBeep(0);
				break;
		}
	
	return found;
}

/* すでに開かれているアイコンをさらに開く */
OSErr OpenByExternalEditor2(MyXIconRec *xIconPtr)
{
	IconFamilyWinRec	*fWinRec;
	PicHandle			picture;
	OSErr				err,igErr;
	FInfo				fileInfo;
	
	err=FSpGetFInfo(&xIconPtr->tempFile,&fileInfo);
	if (err==fnfErr)
	{
		fWinRec=GetIconFamilyRec(xIconPtr->parentWindow);
		picture=IPIconToPicture(&fWinRec->ipIcon,fWinRec->selectedIcon);
		if (picture==nil) return memFullErr;
		
		/* テンポラリファイルの作成 */
		err=FSpCreate(&xIconPtr->tempFile,kIconPartyCreator,kPICTFileType,smSystemScript);
		if (err!=noErr)
		{
			KillPicture(picture);
			return err;
		}
		
		/* PICTデータの保存 */
		err=SaveAsPictMain(picture,&xIconPtr->tempFile);
		if (err!=noErr)
		{
			KillPicture(picture);
			igErr=FSpDelete(&xIconPtr->tempFile);
			return err;
		}
		KillPicture(picture);
	}
	
	/* データを更新 */
	err=FSpGetModDate(&xIconPtr->tempFile,&xIconPtr->modDate);
	if (err!=noErr) xIconPtr->modDate=0;
	
	return OpenByExternalEditorMain(&xIconPtr->tempFile);
}

/* 外部エディタで編集中のアイコンを閉じる */
OSErr CloseXIcon(WindowPtr fWindow)
{
	MyXIconRec	*xIconPtr=gXIconList;
	OSErr		err=noErr;
	
	while (xIconPtr != nil)
	{
		if (xIconPtr->parentWindow == fWindow)
		{
			err=FSpDelete(&xIconPtr->tempFile);
			if (err!=noErr) return err;
		}
		xIconPtr=xIconPtr->next;
	}
	
	return err;
}

/* 外部エディタで編集中のアイコンを切り離す */
OSErr DropFromExternalEditor(WindowPtr fWindow,short selectedIcon)
{
	MyXIconRec	*xIconPtr=gXIconList;
	OSErr		err=noErr;
	
	while (xIconPtr != nil)
	{
		if (xIconPtr->parentWindow == fWindow && xIconPtr->iconKind == selectedIcon)
		{
			err=FSpDelete(&xIconPtr->tempFile);
			return err;
		}
		xIconPtr=xIconPtr->next;
	}
	
	return err;
}
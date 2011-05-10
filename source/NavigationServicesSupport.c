/* ------------------------------------------------------------ */
/*  NavigationServicesSupport.c                                 */
/*     NavigationService関連ルーチン                            */
/*                                                              */
/*                 1998.12.5 - 2001.1.27  naoki iimura        	*/
/* ------------------------------------------------------------ */

/* includes */
#ifdef __APPLE_CC__
	#include	<Carbon/Carbon.h>
	#include	<QuickTime/QuickTime.h>
#else
	#include	<Controls.h>
	#include	<QuickTimeComponents.h>
	#ifndef ControlEditTextSelectionRec
		#include	<ControlDefinitions.h>
	#endif
#endif

#ifdef __APPLE_CC__
#include	"MoreFilesX.h"
#else
#include	"MoreFilesExtras.h"
#endif
#include	"Globals.h"
#include	"IconParty.h"
#include	"NavigationServicesSupport.h"
#include	"UsefulRoutines.h"
#include	"PreCarbonSupport.h"

/* prototypes */
static pascal Boolean myChooseAppFilterProc(
		AEDesc* theItem,
		void* info, 
		NavCallBackUserData callBackUD, 
		NavFilterModes filterMode );
static pascal void 	MySaveEventProc(
		const NavEventCallbackMessage callBackSelector, 
		NavCBRecPtr callBackParms, 
		NavCallBackUserData callBackUD);
static pascal void	MyExportIconEventProc(
		const NavEventCallbackMessage callBackSelector,
		NavCBRecPtr callBackParms,
		NavCallBackUserData callBackUD);

static OSStatus	SendOpenAE(AEDescList list);

static Handle NewOpenHandle(OSType applicationSignature, short numTypes, OSType typeList[]);

OSType	gFileTypeList[]={'PICT',kPNGFileType,'Icon','wIco','IcoS','wIcS'};
enum {
	kFileTypePICT=1,
	kFileTypePNG,
	kFileTypeIcon,
	kFileTypeWinIcon,
};

/* ファイルオープンダイアログ */
OSErr OpenFileWithNav(void)
{
	NavReplyRecord		theReply;
	NavDialogOptions	dialogOptions;
	OSErr				err = noErr;
	NavTypeListHandle	typeList = NULL;
	NavEventUPP			eventUPP = NewNavEventUPP(MyEventProc);
	OSType				fileType[]={'\0\0\0\0',kPICTFileType,kResourceFileType,kPNGFileType,
									kXIconFileType,kWinIconFileType,
									kQTFileTypeJPEG,
									kGIFFileType,kQTFileTypeQuickTimeImage};
	short				numTypes;
	
	InitCursor();
	
	/* デフォルトの設定を読み込む */
	err=NavGetDefaultDialogOptions(&dialogOptions);
	dialogOptions.dialogOptionFlags |= kNavSelectAllReadableItem+kNavNoTypePopup;
	
	/* アプリケーションの名前を設定する */
	BlockMoveData(LMGetCurApName(),dialogOptions.clientName,LMGetCurApName()[0]+1);
	
	/* ファイルタイプのリスト */
	numTypes=(gQTVersion >= 0x03000000 ? 8 : 5);
	numTypes ++;
	typeList=(NavTypeListHandle)NewOpenHandle(kIconPartyCreator,numTypes,fileType);
	
	err=NavGetFile(nil,&theReply,&dialogOptions,eventUPP,nil,nil,typeList,nil);
	
	DisposeNavEventUPP(eventUPP);
	
	gIsDialog=false;
	
	if (theReply.validRecord && err == noErr)
	{
		err=SendOpenAE(theReply.selection);
		
		err=NavDisposeReply(&theReply);
	}
	
	DisposeHandle((Handle)typeList);
	
	return err;
}

/* ファイル選択ダイアログ */
OSErr ChooseFileWithNav(short numTypes,OSType *typeList,Str255 prompt,FSSpec *theFile)
{
	NavReplyRecord		theReply;
	NavDialogOptions	dialogOptions;
	OSErr				err=noErr;
	NavTypeListHandle	openList=NULL;
	NavEventUPP			eventUPP=NewNavEventUPP(MyEventProc);
	NavObjectFilterUPP	filterUPP=NULL;
	
	InitCursor();
	
	/* デフォルトの設定を読み込む */
	err=NavGetDefaultDialogOptions(&dialogOptions);
	
	dialogOptions.dialogOptionFlags &= ~kNavAllowMultipleFiles;
	if ( gNavLibraryVersion >= 0x02008000 )
		dialogOptions.dialogOptionFlags += kNavSupportPackages;
	
	/* アプリケーションの名前を設定 */
	BlockMoveData(LMGetCurApName(),dialogOptions.clientName,LMGetCurApName()[0]+1);
	if (prompt)
		BlockMoveData(prompt,dialogOptions.message,*prompt+1);
	
	/* ファイルタイプのリスト */
	if (numTypes)
	{
		if (typeList[0] == 'APPL')
			filterUPP = NewNavObjectFilterUPP( myChooseAppFilterProc );
		else
		{
			openList=(NavTypeListHandle)NewOpenHandle(kIconPartyCreator,numTypes,typeList);
			if (openList)
				HLock((Handle)openList);
		}
	}
	
	err=NavChooseFile(nil,&theReply,&dialogOptions,eventUPP,nil,filterUPP,openList,nil);
	
	DisposeNavEventUPP(eventUPP);
	if (openList)
	{
		HUnlock((Handle)openList);
		DisposeHandle((Handle)openList);
	}
	
	if (theReply.validRecord && err == noErr)
	{
		AEKeyword	keyword;
		DescType	typeCode;
		Size		actualSize;
		
		err=AEGetNthPtr(&theReply.selection,1,typeFSS,&keyword,&typeCode,theFile,sizeof(FSSpec),&actualSize);
		
		NavDisposeReply(&theReply);
	}
	
	return err;
}

static pascal Boolean myChooseAppFilterProc(AEDesc* theItem, void* info, NavCallBackUserData callBackUD, NavFilterModes filterMode )
{
#pragma unused ( callBackUD, filterMode )

	Boolean 		display = false;
	NavFileOrFolderInfo* 	theInfo = (NavFileOrFolderInfo*)info;
	
	if ( theItem->descriptorType == typeFSS )
		if ( theInfo->isFolder )
		{
			if ( theInfo->fileAndFolder.folderInfo.folderType == '????' &&
				( theInfo->fileAndFolder.folderInfo.folderCreator == '????' ))
				display = true;	// it's a normal folder so allow it
			else
				if ( theInfo->fileAndFolder.folderInfo.folderType == 'APPL' )
					display = true;	// it's a package application so allow it
		}
		else
			// it's a file:
			if ( theInfo->fileAndFolder.fileInfo.finderInfo.fdType == 'APPL' )
				display = true;
				
	return display;
}

static OSType	lIconTypeList[] = {'Icon','wIco','icns'};

OSErr ExportIconWithNav(Str255 filename,FSSpec *theFile,NavReplyRecord *theReply,OSType *iconType)
{
	OSErr				err=noErr;
	NavDialogOptions	dialogOptions;
	NavEventUPP			eventUPP=NewNavEventUPP(MyExportIconEventProc);
	NavExportIconDataRec	userData;
	
	InitCursor();
	
	NavGetDefaultDialogOptions(&dialogOptions);
	
	dialogOptions.dialogOptionFlags &= !kNavAllowStationery;
	dialogOptions.dialogOptionFlags |= kNavDontAddTranslateItems;
	dialogOptions.dialogOptionFlags |= kNavNoTypePopup;
	
	userData.selItem = 1;
	userData.bodySelected = false;
	
	PStrCpy(filename,dialogOptions.savedFileName);
	BlockMoveData(LMGetCurApName(),dialogOptions.clientName,LMGetCurApName()[0]+1);
	
	err=NavPutFile(nil,theReply,&dialogOptions,eventUPP,'Icon',kIconPartyCreator,&userData);
	DisposeNavEventUPP(eventUPP);
	
	if (theReply->validRecord)
	{
		/* 保存 */
		AEKeyword	keyword;
		DescType	typeCode;
		Size		actualSize;
		
		/* 保存ファイルのFSSpecを得る */
		err=AEGetNthPtr(&theReply->selection,1,typeFSS,&keyword,&typeCode,
						theFile,sizeof(FSSpec),&actualSize);
		
		if (theReply->replacing)
		{
			err=FSpDelete(theFile);
		}
		
		*iconType = lIconTypeList[userData.selItem - 1];
	}
	else
		err=userCanceledErr;
	
	return err;
}

OSErr NewIconFileWithNav(FSSpec *theFile,NavReplyRecord *theReply)
{
	OSErr	err=noErr;
	NavDialogOptions	dialogOptions;
	NavEventUPP			eventUPP=NewNavEventUPP(MyEventProc);
	
	InitCursor();
	
	NavGetDefaultDialogOptions(&dialogOptions);
	
	dialogOptions.dialogOptionFlags |= kNavNoTypePopup;
	
	BlockMoveData(theFile->name,dialogOptions.savedFileName,theFile->name[0]+1);
	BlockMoveData(LMGetCurApName(),dialogOptions.clientName,LMGetCurApName()[0]+1);
	
	err=NavPutFile(nil,theReply,&dialogOptions,eventUPP,kResourceFileType,gIconCreator,nil);
	DisposeNavEventUPP(eventUPP);
	
	if (theReply->validRecord)
	{
		/* 保存 */
		AEKeyword	keyword;
		DescType	typeCode;
		Size		actualSize;
		
		/* 保存ファイルのFSSpecを得る */
		err=AEGetNthPtr(&theReply->selection,1,typeFSS,&keyword,&typeCode,
						theFile,sizeof(FSSpec),&actualSize);
		
		if (theReply->replacing)
		{
			err=FSpDelete(theFile);
		}
	}
	else
		err=userCanceledErr;
	
	return err;
}

OSErr SaveBlendPaletteWithNav(FSSpec *theFile,NavReplyRecord *theReply,Str255 prompt,Str255 filename)
{
	OSErr	err=noErr;
	NavDialogOptions	dialogOptions;
	NavEventUPP			eventUPP=NewNavEventUPP(MyEventProc);
	
	InitCursor();
	
	NavGetDefaultDialogOptions(&dialogOptions);
	
	dialogOptions.dialogOptionFlags |= kNavNoTypePopup;
	
	BlockMoveData(filename,dialogOptions.savedFileName,filename[0]+1);
	BlockMoveData(prompt,dialogOptions.message,prompt[0]+1);
	BlockMoveData(LMGetCurApName(),dialogOptions.clientName,LMGetCurApName()[0]+1);
	
	err=NavPutFile(nil,theReply,&dialogOptions,eventUPP,kResourceFileType,gIconCreator,nil);
	
	DisposeNavEventUPP(eventUPP);
	
	if (theReply->validRecord)
	{
		/* 保存 */
		AEKeyword	keyword;
		DescType	typeCode;
		Size		actualSize;
		
		/* 保存ファイルのFSSpecを得る */
		err=AEGetNthPtr(&theReply->selection,1,typeFSS,&keyword,&typeCode,
						theFile,sizeof(FSSpec),&actualSize);
		
		if (theReply->replacing)
			err=FSpDelete(theFile);
	}
	else
		err=userCanceledErr;
	
	return err;
}

/* メインの保存ダイアログ */
OSErr SaveFileWithNav(FSSpec *theFile,OSType *fileType,NavReplyRecord *theReply,long splitNum)
{
	OSErr	err=noErr;
	NavDialogOptions	dialogOptions;
	NavEventUPP			eventUPP=NewNavEventUPP(MySaveEventProc);
	NavSaveDataRec		saveData;
	short	i;
	
	InitCursor();
	
	NavGetDefaultDialogOptions(&dialogOptions);
	dialogOptions.dialogOptionFlags |= kNavNoTypePopup;
	
	BlockMoveData(theFile->name,dialogOptions.savedFileName,theFile->name[0]+1);
	BlockMoveData(LMGetCurApName(),dialogOptions.clientName,LMGetCurApName()[0]+1);
	
	saveData.selItem=1;
	saveData.splitFlag=false;
	for (i=0; i<4; i++)
		if (*fileType == gFileTypeList[i])
		{
			saveData.selItem=i+1;
			break;
		}
	if (*fileType == 'IcoS')
	{
		saveData.selItem=kFileTypeIcon;
		saveData.splitFlag=true;
	}
	else if (*fileType == 'wIcS')
	{
		saveData.selItem=kFileTypeWinIcon;
		saveData.splitFlag=true;
	}
	saveData.splitNum=splitNum;
	NumToString(splitNum,saveData.numStr);
	for (i=1; i<saveData.numStr[0]; i++) saveData.numStr[i]='0';
	saveData.numStr[i]='1';
	saveData.bodySelected=false;
	
	err=NavPutFile(nil,theReply,&dialogOptions,eventUPP,*fileType,kIconPartyCreator,&saveData);
	DisposeNavEventUPP(eventUPP);
	
	if (err==noErr && theReply->validRecord)
	{
		/* 保存 */
		AEKeyword	keyword;
		DescType	typeCode;
		Size		actualSize;
		
		*fileType=gFileTypeList[saveData.selItem-1];
		if (saveData.splitFlag)
			if (*fileType == 'Icon')
				*fileType = 'IcoS';
			else if (*fileType == 'wIco')
				*fileType = 'wIcS';
			else
				saveData.splitFlag = false;
		
		/* 保存されるファイルのFSSpecを得る */
		err=AEGetNthPtr(&theReply->selection,1,typeFSS,&keyword,&typeCode,
						theFile,sizeof(FSSpec),&actualSize);
		if (saveData.splitFlag)
		{
			/* ファイル名から番号を取り除く */
			if (theFile->name[0] >= saveData.numStr[0]+1)
			{
				if (*fileType == 'IcoS')
				{
					if (theFile->name[theFile->name[0]-saveData.numStr[0]] == '.')
					{
						Boolean	b=true;
						UInt8	*p1=&theFile->name[theFile->name[0]-saveData.numStr[0]+1],
								*p2=&saveData.numStr[1];
						short	k;
						
						for (k=saveData.numStr[0]; k>0; k--)
							if (*p1++ != *p2++) b=false;
						if (b) theFile->name[0]-=saveData.numStr[0]+1;
					}
				}
				else /* Winアイコンの場合は番号と拡張子を取り除く */
				{
					Boolean	b=true;
					Str15	suffix;
					short	k;
					UInt8	*p1,*p2;
					
					GetIndString(suffix,131,kFileTypeWinIcon);
					p1=&theFile->name[theFile->name[0]-suffix[0]+1];
					p2=&suffix[1];
					
					for (k=suffix[0]; k>0; k--)
						if (*p1++ != *p2++) b=false;
					if (b)
					{
						theFile->name[0]-=suffix[0];
						
						if (theFile->name[0] >= saveData.numStr[0]+1 && theFile->name[theFile->name[0]-saveData.numStr[0]] == '.')
						{
							p1=&theFile->name[theFile->name[0]-saveData.numStr[0]+1];
							p2=&saveData.numStr[1];
							
							for (k=saveData.numStr[0]; k>0; k--)
								if (*p1++ != *p2++) b=false;
							if (b) theFile->name[0]-=saveData.numStr[0]+1;
						}
					}
				}
			}
		}
		
		if (theReply->replacing && !saveData.splitFlag)
		{
			/* ファイルを削除 */
			err=FSpDelete(theFile);
		}
	}
	else
		err=userCanceledErr;
	
	return err;
}

/* 保存確認のダイアログ */
short AskSaveWithNav(Str255 fileName,short action)
{
	OSErr	err=noErr;
	NavDialogOptions	dialogOptions;
	NavEventUPP			eventUPP=NewNavEventUPP(MyEventProc);
	NavAskSaveChangesResult		reply;
	Str255	title;
	
	InitCursor();
	
	NavGetDefaultDialogOptions(&dialogOptions);
	
	/* アプリケーションの名前を設定 */
	BlockMoveData(LMGetCurApName(),dialogOptions.clientName,LMGetCurApName()[0]+1);
	
	/* タイトル */
	GetIndString(title,158,1);
	BlockMoveData(title,dialogOptions.windowTitle,title[0]+1);
	
	/* 保存するファイルの名前 */
	BlockMoveData(fileName,dialogOptions.savedFileName,fileName[0]+1);
	
	err=NavAskSaveChanges(&dialogOptions,action,&reply,eventUPP,nil);
	DisposeNavEventUPP(eventUPP);
	
	if (err == noErr)
	{
		return (short)reply;
	}
	else return err;
}

/* 破棄確認のダイアログ */
short AskRevertWithNav(Str255 fileName)
{
	OSErr	err=noErr;
	NavDialogOptions	dialogOptions;
	NavEventUPP			eventUPP=NewNavEventUPP(MyEventProc);
	NavAskDiscardChangesResult	reply;
	
	InitCursor();
	
	NavGetDefaultDialogOptions(&dialogOptions);
	
	/* アプリケーション名 */
	BlockMoveData(LMGetCurApName(),dialogOptions.clientName,LMGetCurApName()[0]+1);
	
	/* ファイル名 */
	BlockMoveData(fileName,dialogOptions.savedFileName,fileName[0]+1);
	
	err=NavAskDiscardChanges(&dialogOptions,&reply,eventUPP,nil);
	
	DisposeNavEventUPP(eventUPP);
	
	if (err==noErr)
		return (short)reply;
	else return err;
}

/* StandardGetFile形式のファイルリストに変換 */
static Handle NewOpenHandle(OSType applicationSignature, short numTypes, OSType typeList[])
{
	Handle hdl = NULL;
	
	if ( numTypes > 0 )
	{
	
		hdl = NewHandle(sizeof(NavTypeList) + numTypes * sizeof(OSType));
	
		if ( hdl != NULL )
		{
			NavTypeListHandle open		= (NavTypeListHandle)hdl;
			
			(*open)->componentSignature = applicationSignature;
			(*open)->osTypeCount		= numTypes;
			BlockMoveData(typeList, (*open)->osType, numTypes * sizeof(OSType));
		}
	}
	
	return hdl;
}

/* 選ばれたファイルを開く */
static OSStatus SendOpenAE(AEDescList list)
{
	OSStatus		err;
	AEAddressDesc	theAddress;
	AppleEvent		dummyReply;
	AppleEvent		theEvent;
	
	theAddress.descriptorType	= typeNull;
	theAddress.dataHandle		= NULL;

	do {
		ProcessSerialNumber psn;
	
		err = GetCurrentProcess(&psn);
		if ( err != noErr) break;
		
		err =AECreateDesc(typeProcessSerialNumber, &psn, 
						sizeof(ProcessSerialNumber), &theAddress);
		if ( err != noErr) break;
			
		dummyReply.descriptorType	= typeNull;
		dummyReply.dataHandle		= NULL;

		err = AECreateAppleEvent(kCoreEventClass, kAEOpenDocuments, 
						&theAddress, kAutoGenerateReturnID, kAnyTransactionID, &theEvent);
		if ( err != noErr) break;
		
		err = AEPutParamDesc(&theEvent, keyDirectObject, &list);
		if ( err != noErr) break;
		
		err = AESend(&theEvent, &dummyReply, kAENoReply, kAENormalPriority, 
						kAEDefaultTimeout, NULL, NULL);
		if ( err != noErr) break;
		
		AEDisposeDesc(&theAddress);
		AEDisposeDesc(&theEvent);
		AEDisposeDesc(&dummyReply);
			
	} while (false);
	
	return err;
}

/* オブジェクト選択ダイアログ */
OSErr	SelectObjectDialog(NavEventUPP eventProc,Str255 str,NavReplyRecord *theReply)
{
	NavDialogOptions	dialogOptions;
	OSErr				theErr;
	NavEventUPP			eventUPP=eventProc;
	
	InitCursor();
	
	theErr=NavGetDefaultDialogOptions(&dialogOptions);
	
	BlockMoveData(str,dialogOptions.message,str[0]+1);
	
	theErr=NavChooseObject(nil,theReply,&dialogOptions,eventUPP,nil,nil);
	DisposeNavEventUPP(eventUPP);
	
	return theErr;
}

/* フォルダ選択ダイアログ */
OSErr	SelectFolderDialog(NavEventUPP eventProc,Str255 str,FSSpec *folderSpec)
{
	NavReplyRecord		theReply;
	NavDialogOptions	dialogOptions;
	OSErr				theErr;
	NavEventUPP			eventUPP=eventProc;
	
	InitCursor();
	
	theErr=NavGetDefaultDialogOptions(&dialogOptions);
	
	BlockMoveData(str,dialogOptions.message,*str+1);
	
	theErr=NavChooseFolder(NULL,&theReply,&dialogOptions,eventUPP,NULL,NULL);
	DisposeNavEventUPP(eventUPP);
	
	if (theReply.validRecord && theErr==noErr)
	{
		AEKeyword	keyword;
		DescType	typeCode;
		Size		actualSize;
		
		theErr=AEGetNthPtr(&theReply.selection,1,typeFSS,&keyword,&typeCode,
						folderSpec,sizeof(FSSpec),&actualSize);
		
		NavDisposeReply(&theReply);
	}
	
	return theErr;
}

/* イベント処理 */
/* アップデート処理のみ */
pascal void MyEventProc(const NavEventCallbackMessage callBackSelector, 
						NavCBRecPtr callBackParms, 
						NavCallBackUserData callBackUD)
{
	#pragma unused(callBackUD)
	switch (callBackSelector)
	{
		case kNavCBEvent:
			switch (callBackParms->eventData.eventDataParms.event->what)
			{
				case updateEvt:
					if (callBackParms->eventData.eventDataParms.event->message == 
							(long)callBackParms->window)
						DrawDialog(GetDialogFromWindow(callBackParms->window));
					else
						DoEvent(callBackParms->eventData.eventDataParms.event);
					break;
			}
			break;
	}
}

static Handle	gDitlList;
enum {
	diNavFileType = 1,
	diNavSplitCheck
};

pascal void MySaveEventProc(const NavEventCallbackMessage callBackSelector, 
						NavCBRecPtr callBackParms, 
						NavCallBackUserData callBackUD)
{
	Str255	fileName,suffix;
	OSErr	err;
	ControlEditTextSelectionRec	selection;
	short	firstControl;
	Rect	customRect;
	short	itemHit,myItemHit,newSelItem;
	NavSaveDataRec	*userData;
	Point	pt;
	Handle	h;
	
	switch (callBackSelector)
	{
		case kNavCBEvent:
			switch (callBackParms->eventData.eventDataParms.event->what)
			{
				case updateEvt:
					if (callBackParms->eventData.eventDataParms.event->message == (long)callBackParms->window)
						DrawControls(callBackParms->window);
					else
						DoEvent(callBackParms->eventData.eventDataParms.event);
					break;
				
				case mouseDown:
					userData=(NavSaveDataRec *)callBackUD;
					pt=callBackParms->eventData.eventDataParms.event->where;
					GlobalToLocal(&pt);
					itemHit=FindDialogItem(GetDialogFromWindow(callBackParms->window),pt);
					err=NavCustomControl(callBackParms->context,kNavCtlGetFirstControlID,&firstControl);
					
					myItemHit=itemHit-firstControl+1;
					switch (myItemHit)
					{
						case diNavFileType:
							newSelItem=GetDialogControlValue(GetDialogFromWindow(callBackParms->window),diNavFileType+firstControl);
							if (userData->selItem != newSelItem)
							{
								userData->selItem=newSelItem;
								
								/* ファイル名を更新 */
								err=NavCustomControl(callBackParms->context,kNavCtlGetEditFileName,fileName);
								GetIndString(suffix,131,newSelItem);
								selection.selStart=0;
								selection.selEnd=ChangeSuffix(fileName,suffix);
								
								err=NavCustomControl(callBackParms->context,kNavCtlSetEditFileName,fileName);
								
								/* 分割できる時はチェックボックスを有効に */
								if (userData->splitNum > 1)
								{
									if  (newSelItem == kFileTypeIcon)
									{
										SetDialogControlHilite(GetDialogFromWindow(callBackParms->window),diNavSplitCheck+firstControl,0);
										if (userData->splitFlag)
										{
											TruncPString(fileName,fileName,31-1-userData->numStr[0]);
											CatChar('.',fileName);
											PStrCat(userData->numStr,fileName);
											err=NavCustomControl(callBackParms->context,kNavCtlSetEditFileName,fileName);
										}
									}
									else if (newSelItem == kFileTypeWinIcon)
									{
										SetDialogControlHilite(GetDialogFromWindow(callBackParms->window),diNavSplitCheck+firstControl,0);
										if (userData->splitFlag)
										{
											TruncPString(fileName,fileName,31-1-userData->numStr[0]);
											fileName[0]=GetBodyLength(fileName);
											CatChar('.',fileName);
											PStrCat(userData->numStr,fileName);
											PStrCat(suffix,fileName);
											err=NavCustomControl(callBackParms->context,kNavCtlSetEditFileName,fileName);
										}
									}
									else
										SetDialogControlHilite(GetDialogFromWindow(callBackParms->window),diNavSplitCheck+firstControl,255);
								}
								else
									SetDialogControlHilite(GetDialogFromWindow(callBackParms->window),diNavSplitCheck+firstControl,255);
								if (gNavLibraryVersion >= 0x0110)
									err=NavCustomControl(callBackParms->context,kNavCtlSelectEditFileName,&selection);
							}
							break;
						
						case diNavSplitCheck:
							newSelItem=GetDialogControlValue(GetDialogFromWindow(callBackParms->window),diNavFileType+firstControl);
							if (newSelItem != kFileTypeIcon && newSelItem != kFileTypeWinIcon)
								break;
							
							userData->splitFlag = !userData->splitFlag;
							SetDialogControlValue(GetDialogFromWindow(callBackParms->window),diNavSplitCheck+firstControl,userData->splitFlag);
							err=NavCustomControl(callBackParms->context,kNavCtlGetEditFileName,fileName);
							if (userData->splitFlag)
							{
								if (newSelItem == kFileTypeIcon)
								{
									TruncPString(fileName,fileName,31-1-userData->numStr[0]);
									selection.selStart=0;
									selection.selEnd=GetBodyLength(fileName);
									CatChar('.',fileName);
									PStrCat(userData->numStr,fileName);
								}
								else
								{
									TruncPString(fileName,fileName,31-1-userData->numStr[0]-4);
									fileName[0]=GetBodyLength(fileName);
									CatChar('.',fileName);
									PStrCat(userData->numStr,fileName);
									GetIndString(suffix,131,newSelItem);
									PStrCat(suffix,fileName);
									selection.selStart=0;
									selection.selEnd=GetBodyLength(fileName);
								}
							}
							else
							{
								if (newSelItem == kFileTypeIcon)
								{
									GetIndString(suffix,131,newSelItem);
									selection.selStart=0;
									selection.selEnd=ChangeSuffix(fileName,suffix);
								}
								else
								{
									fileName[0]=GetBodyLength(fileName);
									fileName[0]=GetBodyLength2(fileName);
									GetIndString(suffix,131,newSelItem);
									PStrCat(suffix,fileName);
									selection.selStart=0;
									selection.selEnd=GetBodyLength(fileName);
								}
							}
							err=NavCustomControl(callBackParms->context,kNavCtlSetEditFileName,fileName);
							if (gNavLibraryVersion >= 0x0110)
								err=NavCustomControl(callBackParms->context,kNavCtlSelectEditFileName,&selection);
							break;
					}
					break;
			}
			break;
		
		case kNavCBStart:
			/* ダイアログ作成時 */
			/* ファイル名の選択 */
			err=NavCustomControl(callBackParms->context,kNavCtlGetEditFileName,fileName);
			selection.selStart=0;
			selection.selEnd=GetBodyLength(fileName);
			if (gNavLibraryVersion >= 0x0110)
				err=NavCustomControl(callBackParms->context,kNavCtlSelectEditFileName,&selection);
			
			/* DITLを追加 */
			gDitlList=GetResource('DITL',500);
			HNoPurge(gDitlList);
			err=NavCustomControl(callBackParms->context,kNavCtlAddControlList,gDitlList);
			
			userData=(NavSaveDataRec *)callBackUD;
			err=NavCustomControl(callBackParms->context,kNavCtlGetFirstControlID,&firstControl);
			h=GetDialogItemHandle(GetDialogFromWindow(callBackParms->window),diNavFileType+firstControl);
			
			#if 0
			/* ポップアップメニューのGIF部分をいじる */
			{
				MenuHandle		menu;
				Str255			string;
				
				menu=GetControlPopupMenuHandle((ControlHandle)h);
				GetIndString(string,151,(gPNGFilePrefs.useClip2gif ? 2 : 1));
				SetMenuItemText(menu,2,string);
			}
			#endif
			
			/* メニューを選択 */
			SetControlValue((ControlHandle)h,userData->selItem);
			
			/* 分割するチェックの初期値を変更 */
			SetDialogControlValue(GetDialogFromWindow(callBackParms->window),diNavSplitCheck+firstControl,userData->splitFlag);
			
			if (userData->splitNum>1 && (userData->selItem == kFileTypeIcon || userData->selItem == kFileTypeWinIcon))
				SetDialogControlHilite(GetDialogFromWindow(callBackParms->window),diNavSplitCheck+firstControl,0);
			else
				SetDialogControlHilite(GetDialogFromWindow(callBackParms->window),diNavSplitCheck+firstControl,255);
			break;
		
		case kNavCBCustomize:
			/* カスタムコントロール領域の確保 */
			customRect=callBackParms->customRect;
			if (customRect.right - customRect.left < 400 && customRect.bottom - customRect.top < 40)
			{
				customRect.right=customRect.left+400;
				customRect.bottom=customRect.top+40;
				callBackParms->customRect=customRect;
			}
			break;
		
		case kNavCBTerminate:
			ReleaseResource(gDitlList);
			break;
	}
}

pascal void MyExportIconEventProc(const NavEventCallbackMessage callBackSelector, 
						NavCBRecPtr callBackParms, 
						NavCallBackUserData callBackUD)
{
	Str255	fileName,suffix;
	OSErr	err;
	ControlEditTextSelectionRec	selection;
	short	firstControl;
	Rect	customRect;
	short	itemHit,myItemHit,newSelItem;
	NavExportIconDataRec	*userData;
	Point	pt;
	Handle	h;
	
	switch (callBackSelector)
	{
		case kNavCBEvent:
			switch (callBackParms->eventData.eventDataParms.event->what)
			{
				case updateEvt:
					if (callBackParms->eventData.eventDataParms.event->message == (long)callBackParms->window)
						DrawControls(callBackParms->window);
					else
						DoEvent(callBackParms->eventData.eventDataParms.event);
					break;
				
				case mouseDown:
					userData=(NavExportIconDataRec *)callBackUD;
					pt=callBackParms->eventData.eventDataParms.event->where;
					GlobalToLocal(&pt);
					itemHit=FindDialogItem(GetDialogFromWindow(callBackParms->window),pt);
					err=NavCustomControl(callBackParms->context,kNavCtlGetFirstControlID,&firstControl);
					
					myItemHit=itemHit-firstControl+1;
					switch (myItemHit)
					{
						case diNavFileType:
							newSelItem=GetDialogControlValue(GetDialogFromWindow(callBackParms->window),diNavFileType+firstControl);
							if (userData->selItem != newSelItem)
							{
								userData->selItem=newSelItem;
								
								/* ファイル名を更新 */
								err=NavCustomControl(callBackParms->context,kNavCtlGetEditFileName,fileName);
								GetIndString(suffix,161,newSelItem);
								selection.selStart=0;
								selection.selEnd=ChangeSuffix(fileName,suffix);
								
								err=NavCustomControl(callBackParms->context,kNavCtlSetEditFileName,fileName);
								
								if (gNavLibraryVersion > 0x0110)
									err=NavCustomControl(callBackParms->context,kNavCtlSelectEditFileName,&selection);
							}
							break;
					}	
			}
			break;
		
		case kNavCBStart:
			/* ダイアログ作成時 */
			/* ファイル名の選択 */
			userData=(NavExportIconDataRec *)callBackUD;
			err=NavCustomControl(callBackParms->context,kNavCtlGetEditFileName,fileName);
			selection.selStart=0;
			selection.selEnd=GetBodyLength(fileName);
			if (gNavLibraryVersion > 0x0110)
				err=NavCustomControl(callBackParms->context,kNavCtlSelectEditFileName,&selection);
			
			/* DITLを追加 */
			gDitlList=GetResource('DITL',501);
			HNoPurge(gDitlList);
			err=NavCustomControl(callBackParms->context,kNavCtlAddControlList,gDitlList);
			
			/* メニューを選択 */
			err=NavCustomControl(callBackParms->context,kNavCtlGetFirstControlID,&firstControl);
			h=GetDialogItemHandle(GetDialogFromWindow(callBackParms->window),diNavFileType+firstControl);
			SetControlValue((ControlHandle)h,userData->selItem);
			{
				MenuHandle		menu;
				
				menu=GetControlPopupMenuHandle((ControlHandle)h);
				if (gSystemVersion < 0x0850)
					MyDisableMenuItem(menu,3);
			}
			break;
		
		case kNavCBCustomize:
			/* カスタムコントロール領域の確保 */
			customRect=callBackParms->customRect;
			if (customRect.right - customRect.left < 400 && customRect.bottom - customRect.top < 40)
			{
				customRect.right=customRect.left+400;
				customRect.bottom=customRect.top+40;
				callBackParms->customRect=customRect;
			}
			break;
		
		case kNavCBTerminate:
			ReleaseResource(gDitlList);
			break;
	}
}
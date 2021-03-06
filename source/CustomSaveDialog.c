/* ------------------------------------------------------------ */
/*  CustomSaveDialog.c                                          */
/*     保存ダイアログの処理                                     */
/*                                                              */
/*                 1997.1.28 - 2001.1.27  naoki iimura        	*/
/* ------------------------------------------------------------ */

#include	<StandardFile.h>
#include	<TextUtils.h>
#include	<Controls.h>
#ifndef PopupPrivateData
#include	<ControlDefinitions.h>
#endif

#include	"MoreFilesExtras.h"
#include	"Globals.h"
#include	"UsefulRoutines.h"
#include	"CustomSaveDialog.h"
#include	"PreCarbonSupport.h"


#if !TARGET_API_MAC_CARBON

/* structures */
typedef struct {
	OSType	selType;
	Boolean	splitFlag;
	long	splitNum;
	Str15	numStr;
	Boolean	bodySelected;
} SFData,*SFDataPtr;

/* definitions */
#define	dPutFileDialog	134
#define	diFileType		13
#define	diSplitCheck	14
enum {
	kFileTypePICT=1,
	kFileTypePNG,
	kFileTypeIcon,
	kFileTypeWinIcon,
};

/* prototypes */
static pascal short	DialogHook(short item,DialogPtr dlg,Ptr userData);
static pascal Boolean	ModalFilter(DialogPtr theDialog,EventRecord *theEvent,short *itemHit,Ptr userData);
static pascal short	ExportIconDialogHook(short item,DialogPtr dlg,Ptr userData);
static pascal Boolean	ExportIconModalFilter(DialogPtr theDialog,EventRecord *theEvent,short *itemHit,Ptr userData);
extern pascal void	MyActivate(DialogPtr theDialog,short item,Boolean activating,void *userData);

extern void	DoUpdate(EventRecord *theEvent);


extern OSType	gFileTypeList[];


/* 保存 */
Boolean StandardSaveAs(FSSpec *spec,OSType *fType,long splitNum)
{
	StandardFileReply	rep;
	SFData	sfUserData;
	OSErr	err;
	Point	where={-1,-1};
	DlgHookYDUPP	dlUPP=NewDlgHookYDProc(DialogHook);
	ModalFilterYDUPP	mfUPP=NewModalFilterYDProc(ModalFilter);
	ActivateYDUPP	aUPP=NewActivateYDProc(MyActivate);
	Str255	prompt;
	short	i;
	
	GetIndString(prompt,138,1);
	
	/* 構造体初期化 */
	sfUserData.splitFlag=false;
	if (*fType == 'IcoS')
	{
		sfUserData.selType='Icon';
		sfUserData.splitFlag=true;
	}
	else if (*fType == 'wIcS')
	{
		sfUserData.selType='wIco';
		sfUserData.splitFlag=true;
	}
	else
	{
		sfUserData.selType=*fType;
	}
	sfUserData.splitNum=splitNum;
	NumToString(splitNum,sfUserData.numStr);
	for (i=1; i<sfUserData.numStr[0]; i++) sfUserData.numStr[i]='0';
	sfUserData.numStr[i]='1';
	sfUserData.bodySelected=false;
	
	CustomPutFile(prompt,spec->name,&rep,dPutFileDialog,where,
		dlUPP,mfUPP,nil,aUPP,&sfUserData);
	
	DisposeRoutineDescriptor(dlUPP);
	DisposeRoutineDescriptor(mfUPP);
	DisposeRoutineDescriptor(aUPP);
	
	if (rep.sfGood)
	{
		*spec=rep.sfFile;
		*fType=sfUserData.selType;
		if (sfUserData.splitFlag)
			if (*fType == 'Icon')
				*fType = 'IcoS';
			else if (*fType == 'wIco')
				*fType = 'wIcS';
			else
				sfUserData.splitFlag = false;
		
		if (sfUserData.splitFlag)
		{
			/* ファイル名から番号を取り除く */
			if (spec->name[0] >= sfUserData.numStr[0]+1)
			{
				if (*fType == 'IcoS')
				{
					if (spec->name[spec->name[0]-sfUserData.numStr[0]] == '.')
					{
						Boolean	b=true;
						UInt8	*p1=&spec->name[spec->name[0]-sfUserData.numStr[0]+1],
								*p2=&sfUserData.numStr[1];
						short	k;
						
						for (k=sfUserData.numStr[0]; k>0; k--)
							if (*p1++ != *p2++) b=false;
						if (b) spec->name[0]-=sfUserData.numStr[0]+1;
					}
				}
				else /* Winアイコンの場合は番号と拡張子を取り除く */
				{
					Boolean	b=true;
					Str15	suffix;
					short	k;
					UInt8	*p1,*p2;
					
					GetIndString(suffix,131,kFileTypeWinIcon);
					p1=&spec->name[spec->name[0]-suffix[0]+1];
					p2=&suffix[1];
					
					for (k=suffix[0]; k>0; k--)
						if (*p1++ != *p2++) b=false;
					if (b)
					{
						spec->name[0]-=suffix[0];
						
						if (spec->name[0] >= sfUserData.numStr[0]+1 && spec->name[spec->name[0]-sfUserData.numStr[0]] == '.')
						{
							p1=&spec->name[spec->name[0]-sfUserData.numStr[0]+1];
							p2=&sfUserData.numStr[1];
							
							for (k=sfUserData.numStr[0]; k>0; k--)
								if (*p1++ != *p2++) b=false;
							if (b) spec->name[0]-=sfUserData.numStr[0]+1;
						}
					}
				}
			}
		}
		
		if (rep.sfReplacing && !sfUserData.splitFlag)
			err=FSpDelete(spec);
	}
	
	return rep.sfGood;
}

/* 保存ダイアログのフック */
pascal short DialogHook(short item,DialogPtr dlg,Ptr userData)
{
	SFDataPtr	sfUserData;
	Handle	rbControlHandle;
	Str255	filename;
	short	bodyLength;
	short	selType;
	Str15	suffix;
	short	selItem,i;
	
	if (GetWRefCon(dlg)!=sfMainDialogRefCon)
		return item;
	
	sfUserData=(SFDataPtr)userData;
	
	switch (item)
	{
		case sfHookFirstCall:
			rbControlHandle=GetDialogItemHandle(dlg,diFileType);
			
			#if 0
			/* ポップアップメニューのGIF部分をいじる */
			{
				PopupPrivateDataHandle	ppdh=
							(PopupPrivateDataHandle)(**(ControlHandle)rbControlHandle).contrlData;
				MenuHandle		menu;
				Str255			string;
				
				menu=(**ppdh).mHandle;
				GetIndString(string,151,(gPNGFilePrefs.useClip2gif ? 2 : 1));
				SetMenuItemText(menu,2,string);
			}
			#endif
			
			/* ファイルタイプによってポップアップメニューの初期値を変更 */
			selItem=1;
			for (i=0; i<4; i++)
				if (sfUserData->selType == gFileTypeList[i])
				{
					selItem=i+1;
					break;
				}
			
			SetControlValue((ControlHandle)rbControlHandle,selItem);
			
			/* 分割するチェックの初期値を変更 */
			SetDialogControlValue(dlg,diSplitCheck,sfUserData->splitFlag);
			
			if (sfUserData->splitNum>1 && (selItem == kFileTypeIcon || selItem == kFileTypeWinIcon))
				SetDialogControlHilite(dlg,diSplitCheck,0);
			else
				SetDialogControlHilite(dlg,diSplitCheck,255);
			break;
		
		case sfHookNullEvent:
			if (!sfUserData->bodySelected)
			{
				GetDialogItemText2(dlg,sfItemFileNameTextEdit,filename);
				bodyLength=GetBodyLength(filename);
				SelectDialogItemText(dlg,sfItemFileNameTextEdit,0,bodyLength);
				
				sfUserData->bodySelected=true;
			}
			break;
		
		case sfItemOpenButton:
			selItem=GetDialogControlValue(dlg,diFileType);
			sfUserData->selType=gFileTypeList[selItem-1];
			break;
		
		case sfItemCancelButton:
			break;
		
		case diFileType:
			selType=GetDialogControlValue(dlg,diFileType);
			
			GetDialogItemText2(dlg,sfItemFileNameTextEdit,filename);
			
			GetIndString(suffix,131,selType);
			ChangeSuffix(filename,suffix);
			SetDialogItemText2(dlg,sfItemFileNameTextEdit,filename);
			
			/* 分割できる時はチェックボックスを有効に */
			if (sfUserData->splitNum > 1)
			{
				if (selType == kFileTypeIcon)
				{
					SetDialogControlHilite(dlg,diSplitCheck,0);
					if (sfUserData->splitFlag)
					{
						TruncPString(filename,filename,31-1-sfUserData->numStr[0]);
						CatChar('.',filename);
						PStrCat(sfUserData->numStr,filename);
						SetDialogItemText2(dlg,sfItemFileNameTextEdit,filename);
					}
				}
				else if (selType == kFileTypeWinIcon)
				{
					SetDialogControlHilite(dlg,diSplitCheck,0);
					if (sfUserData->splitFlag)
					{
						TruncPString(filename,filename,31-1-sfUserData->numStr[0]);
						filename[0]=GetBodyLength(filename);
						CatChar('.',filename);
						PStrCat(sfUserData->numStr,filename);
						PStrCat(suffix,filename);
						SetDialogItemText2(dlg,sfItemFileNameTextEdit,filename);
					}
				}
				else
					SetDialogControlHilite(dlg,diSplitCheck,255);
			}
			else
				SetDialogControlHilite(dlg,diSplitCheck,255);
			
			/* ファイル名が選択されていない状態にする→次のNull Evtで選択させる */
			sfUserData->bodySelected=false;
			break;
		
		case diSplitCheck:
			sfUserData->splitFlag=!sfUserData->splitFlag;
			SetDialogControlValue(dlg,diSplitCheck,sfUserData->splitFlag);
			GetDialogItemText2(dlg,sfItemFileNameTextEdit,filename);
			selType=GetDialogControlValue(dlg,diFileType);
			if (sfUserData->splitFlag)
			{
				if (selType == kFileTypeIcon)
				{
					TruncPString(filename,filename,31-1-sfUserData->numStr[0]);
					CatChar('.',filename);
					PStrCat(sfUserData->numStr,filename);
				}
				else
				{
					TruncPString(filename,filename,31-1-sfUserData->numStr[0]-4);
					filename[0]=GetBodyLength(filename);
					CatChar('.',filename);
					PStrCat(sfUserData->numStr,filename);
					GetIndString(suffix,131,selType);
					PStrCat(suffix,filename);
				}
			}
			else
			{
				if (selType == kFileTypeIcon)
				{
					GetIndString(suffix,131,selType);
					ChangeSuffix(filename,suffix);
				}
				else
				{
					filename[0]=GetBodyLength(filename);
					filename[0]=GetBodyLength2(filename);
					GetIndString(suffix,131,selType);
					PStrCat(suffix,filename);
				}
			}
			SetDialogItemText2(dlg,sfItemFileNameTextEdit,filename);
			sfUserData->bodySelected=false;
			break;
	}
	
	return item;
}

/* モーダルフィルタ（ショートカットによりファイルタイプを選択） */
pascal Boolean ModalFilter(DialogPtr theDialog,EventRecord *theEvent,short *itemHit,Ptr userData)
{
	#pragma unused (userData)
	Boolean	eventHandled=false;
	WindowPtr	theWindow;
	
	if (GetWRefCon(theDialog) != sfMainDialogRefCon)
		return false;
	
	switch (theEvent->what)
	{
		case updateEvt:
			theWindow=(WindowPtr)theEvent->message;
			if (theWindow!=nil && theWindow!=GetDialogWindow(theDialog))
				DoUpdate(theEvent);
			break;
		
		case keyDown:
		case autoKey:
			if ((theEvent->modifiers & cmdKey)!=0)
			{
				char	theChar;
				
				theChar=theEvent->message & charCodeMask;
				if (theChar >= '1' && theChar <= '4')
				{
					SetDialogControlValue(theDialog,diFileType,theChar-'1'+1);
					*itemHit=diFileType;
					eventHandled=true;
				}
			}
			break;
	}
	
	return eventHandled;
}

/* アイコン書き出し */
void ExportIconDialog(Str31 iconName,StandardFileReply *reply,OSType *fType)
{
	SFData	sfUserData;
	OSErr	err;
	Point	where={-1,-1};
	DlgHookYDUPP	dlUPP=NewDlgHookYDProc(ExportIconDialogHook);
	ModalFilterYDUPP	mfUPP=NewModalFilterYDProc(ExportIconModalFilter);
	ActivateYDUPP	aUPP=NewActivateYDProc(MyActivate);
	Str255	prompt;
	
	GetIndString(prompt,140,1);
	
	/* 構造体初期化 */
	sfUserData.selType = *fType;
	sfUserData.bodySelected=false;
	
	CustomPutFile(prompt,iconName,reply,148,where,
		dlUPP,mfUPP,nil,aUPP,&sfUserData);
	
	DisposeRoutineDescriptor(dlUPP);
	DisposeRoutineDescriptor(mfUPP);
	DisposeRoutineDescriptor(aUPP);
	
	if (reply->sfGood)
	{
		*fType=sfUserData.selType;
		
		if (reply->sfReplacing)
			err=FSpDelete(&reply->sfFile);
	}
}

static OSType	lIconTypeList[] = {'Icon','wIco','icns'};

pascal short ExportIconDialogHook(short item,DialogPtr dlg,Ptr userData)
{
	SFDataPtr	sfUserData;
	Handle	rbControlHandle;
	Str255	filename;
	short	bodyLength;
	short	selType;
	Str15	suffix;
	short	selItem,i;
	
	if (GetWRefCon(dlg)!=sfMainDialogRefCon)
		return item;
	
	sfUserData=(SFDataPtr)userData;
	
	switch (item)
	{
		case sfHookFirstCall:
			rbControlHandle=GetDialogItemHandle(dlg,diFileType);
			
			/* ファイルタイプによってポップアップメニューの初期値を変更 */
			selItem=1;
			for (i=0; i<3; i++)
				if (sfUserData->selType == lIconTypeList[i])
				{
					selItem=i+1;
					break;
				}
			
			{
				MenuHandle		menu;
				
				menu=GetControlPopupMenuHandle((ControlHandle)rbControlHandle);
				if (gSystemVersion < 0x0850)
					MyDisableMenuItem(menu,3);
			}
			SetControlValue((ControlHandle)rbControlHandle,selItem);
			break;
		
		case sfHookNullEvent:
			if (!sfUserData->bodySelected)
			{
				GetDialogItemText2(dlg,sfItemFileNameTextEdit,filename);
				bodyLength=GetBodyLength(filename);
				SelectDialogItemText(dlg,sfItemFileNameTextEdit,0,bodyLength);
				
				sfUserData->bodySelected=true;
			}
			break;
		
		case sfItemOpenButton:
			selItem=GetDialogControlValue(dlg,diFileType);
			sfUserData->selType=lIconTypeList[selItem-1];
			break;
		
		case sfItemCancelButton:
			break;
		
		case diFileType:
			selType=GetDialogControlValue(dlg,diFileType);
			
			GetDialogItemText2(dlg,sfItemFileNameTextEdit,filename);
			
			GetIndString(suffix,161,selType);
			ChangeSuffix(filename,suffix);
			SetDialogItemText2(dlg,sfItemFileNameTextEdit,filename);
			
			/* ファイル名が選択されていない状態にする→次のNull Evtで選択させる */
			sfUserData->selType=selType;
			sfUserData->bodySelected=false;
			break;
	}
	
	return item;
}

/* モーダルフィルタ（ショートカットによりファイルタイプを選択） */
pascal Boolean ExportIconModalFilter(DialogPtr theDialog,EventRecord *theEvent,short *itemHit,Ptr userData)
{
	#pragma unused (userData)
	Boolean	eventHandled=false;
	WindowPtr	theWindow;
	
	if (GetWRefCon(theDialog) != sfMainDialogRefCon)
		return false;
	
	switch (theEvent->what)
	{
		case updateEvt:
			theWindow=(WindowPtr)theEvent->message;
			if (theWindow!=nil && theWindow!=GetDialogWindow(theDialog))
				DoUpdate(theEvent);
			break;
		
		case keyDown:
		case autoKey:
			if ((theEvent->modifiers & cmdKey)!=0)
			{
				char	theChar;
				
				theChar=theEvent->message & charCodeMask;
				if (theChar >= '1' && theChar <= '3')
				{
					SetDialogControlValue(theDialog,diFileType,theChar-'1'+1);
					*itemHit=diFileType;
					eventHandled=true;
				}
			}
			break;
	}
	
	return eventHandled;
}
#endif


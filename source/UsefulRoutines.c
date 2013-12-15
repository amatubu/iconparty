/* ------------------------------------------------------------ */
/*  Useful Routines.c                                           */
/*     使えるルーチン（苦笑）                                   */
/*                                                              */
/*                 1997.1.11 - 2001.2.3  naoki iimura        	*/
/* ------------------------------------------------------------ */

/* includes */
#ifdef __APPLE_CC__
#include	<Carbon/Carbon.h>
#include	<QuickTime/QuickTime.h>
#else
#include	<Fonts.h>
#include	<NumberFormatting.h>
#include	<StringCompare.h>
#include	<TextUtils.h>
#include	<Controls.h>
#ifndef kControlButtonPart
#include	<ControlDefinitions.h>
#endif
#include	<Resources.h>
#include	<QuickTimeComponents.h>
#include	<PictUtils.h>
#include	<FinderRegistry.h>
#endif

#ifdef __APPLE_CC__
#include	"MoreFilesX.h"
#else
#include	"MoreFilesExtras.h"
#endif

#include	"Globals.h"
#include	"UsefulRoutines.h"
#include	"PreCarbonSupport.h"

/* local routine */
static pascal void MyGetDepthProc(const BitMap *bitPtr,const Rect *srcRect,const Rect *dstRect,
	short mode,RgnHandle maskRgn);


/* -> IconParty.c */
extern void MySetCursor(short id);

/* -> window.c */
extern void	DoUpdate(EventRecord *theEvent);

/* ツールボックスの初期化 */
void ToolBoxInit(void)
{
	#if !TARGET_API_MAC_CARBON
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	#endif
	InitCursor();
}

/* エラー表示 */
void ErrorAlert(const StringPtr errStr)
{
	DeactivateFloatersAndFirstDocumentWindow();
	
	ParamText(errStr,"\p","\p","\p");
	
	Alert(201,0);
	
	ActivateFloatersAndFirstDocumentWindow();
}

/* エラー表示2（エラーコードつき） */
void ErrorAlert2(const StringPtr errStr,short errCode)
{
	Str255	errCodeStr;
	
	DeactivateFloatersAndFirstDocumentWindow();
	
	NumToString(errCode,errCodeStr);
	ParamText(errStr,errCodeStr,"\p","\p");
	
	Alert(202,0);
	
	ActivateFloatersAndFirstDocumentWindow();
}

/* エラー表示（リソースから） */
void ErrorAlertFromResource(short res_id,short num)
{
	Str255	errStr;
	
	GetIndString(errStr,res_id,num);
	ErrorAlert(errStr);
}

/* ユーザに是非を問う */
OSErr MyConfirmDialog(const StringPtr prompt,const StringPtr okString,
	short *defaultSelection)
{
	short		dialogID;
	DialogPtr	theDialog;
	short		item;
	ModalFilterUPP	mfUPP;
	
	if (defaultSelection == NULL)
		dialogID = 150;
	else
	{
		if (*defaultSelection == ok) return noErr;
		if (*defaultSelection == cancel) return userCanceledErr;
		
		dialogID = 151;
	}
	
	MySetCursor(0);
//	DeactivateFloatersAndFirstDocumentWindow();
	
	ParamText(prompt,"\p","\p","\p");
	theDialog = GetNewDialog(dialogID,NULL,kFirstWindowOfClass);
	if (okString) SetDialogControlTitle(theDialog,ok,okString);
	
	SetDialogDefaultItem(theDialog,ok);
	SetDialogCancelItem(theDialog,ok);
	ShowWindow(GetDialogWindow(theDialog));
	
	item = 3;
	mfUPP=NewModalFilterUPP(MyModalDialogFilter);
	while (item != ok && item != cancel)
	{
		ModalDialog(mfUPP,&item);
		
		if (item == 3 && defaultSelection != NULL)
		{
			SetDialogControlValue(theDialog,3,!GetDialogControlValue(theDialog,3));
		}
	}
	
	DisposeDialog(theDialog);
	DisposeModalFilterUPP(mfUPP);
//	ActivateFloatersAndFirstDocumentWindow();
	
	return (item == ok ? noErr : userCanceledErr);
}

/* クリエータコードからアプリケーションのパスを求める */
OSErr GetApplSpec(OSType creator,FSSpec *applSpec)
{
	short 			i = 0;
	OSErr			err;
	HVolumeParam	vparamRec;
	DTPBRec			paramBlock;
	Boolean			boolFoundIt=false;
	
	vparamRec.ioCompletion = NULL;
	vparamRec.ioNamePtr = NULL;
	applSpec->name[0]=0;
	
	do {
			vparamRec.ioVRefNum = 0;
			vparamRec.ioVolIndex = ++i;
		
			err = PBHGetVInfoSync( (HParmBlkPtr)&vparamRec );
//														grab desktop database
			if ( err == noErr ) 
			{
				paramBlock.ioNamePtr = NULL;
				paramBlock.ioVRefNum = vparamRec.ioVRefNum;
				err = PBDTGetPath( &paramBlock );
//														search desktop for app
				if ( err == noErr ) 
				{
					paramBlock.ioIndex = 0;	/* 作成日が一番最近のアプリケーションを探す */
					paramBlock.ioNamePtr = applSpec->name;
					paramBlock.ioFileCreator = creator;
					paramBlock.ioCompletion = NULL;
					err = PBDTGetAPPLSync( &paramBlock );
					if ( err == noErr )
						boolFoundIt = true;
				}
				err = noErr;
			}
		} while ( !boolFoundIt && ( err == noErr ) );
//														store data in fsspec
	if ( boolFoundIt ) 
	{
		err = noErr;	/* いらんか */
		applSpec->vRefNum = paramBlock.ioVRefNum;
		applSpec->parID = paramBlock.ioAPPLParID;
	}
	else
		err = afpItemNotFound;
	
	return err;
}

/* クリエータコードからアプリケーション名を求める */
OSErr CreatorToAppl(OSType creator,Str255 applName)
{
	OSErr	err;
	FSSpec	applSpec;
	
	err=GetApplSpec(creator,&applSpec);
	
	if (err == noErr)
	{
		PStrCpy(applSpec.name,applName);
	}
	else
		GetIndString(applName,153,1);
	
	return err;
}

/* ファイルオープンダイアログのフィルタ（不可視ファイルを非表示） */
pascal Boolean FilterInvisFiles(CInfoPBPtr pb)
{
	if ((pb->hFileInfo.ioFlFndrInfo.fdFlags) & fInvisible)
		return true;
	else
		return false;
}

/* エイリアスの元ファイルを探す。ユーザインタラクションを許可しない */
OSErr ResolveAliasFileWithNoUI(FSSpec *fromFile,Boolean resolveAliasChains,Boolean *wasAliased)
{
	OSErr	err;
	Boolean	isFolder;
	
	err=MyIsAliasFile(fromFile,wasAliased,&isFolder);
	if (*wasAliased)
	{
		err=ResolveAliasFileWithNoUIMain(fromFile,resolveAliasChains);
	}
	
	return err;
}

/* ユーザインタラクションなしでエイリアスファイルを解決する */
OSErr ResolveAliasFileWithNoUIMain(FSSpec *fromFile,Boolean resolveAliasChains)
{
	OSErr	err;
	short	refNum;
	AliasHandle	theAlias;
	Boolean	needsUpdate,wasChanged;
	FSSpec	theSpec;
	Boolean	isAlias,isFolder;
	short	count=1;
	
	refNum=FSpOpenResFile(fromFile,fsRdPerm);
	if (refNum <= 0) return ResError();
	
	theAlias=(AliasHandle)Get1Resource(rAliasType,0);
	if (theAlias == nil) return -1;
	
	err=MatchAlias(nil,kARMNoUI+kARMMultVols+kARMSearch,theAlias,&count,&theSpec,&needsUpdate,nil,nil);
	if (err==noErr)
	{
		*fromFile=theSpec;
		if (needsUpdate)
			err=UpdateAlias(nil,&theSpec,theAlias,&wasChanged);
	}
	CloseResFile(refNum);
	
	if (err==noErr && resolveAliasChains)
	{
		err=MyIsAliasFile(fromFile,&isAlias,&isFolder);
		if (err==noErr && isAlias)
			err=ResolveAliasFileWithNoUIMain(fromFile,resolveAliasChains);
	}
	
	return err;
}

/* エイリアスかどうかをチェック */
OSErr MyIsAliasFile(const FSSpec *fileFSSpec,Boolean *aliasFileFlag,Boolean *folderFlag)
{
	OSErr	err;
	#ifdef __MOREFILESX__
	FSRef	fsRef;
	FinderInfo	info;
	Boolean	isDirectory;
	#else
	FInfo	fInfo;
	#endif
	
	*aliasFileFlag=false;
	*folderFlag=false;
	
	#ifdef __MOREFILESX__
	err = FSpMakeFSRef(fileFSSpec,&fsRef);
	err = FSGetFinderInfo(&fsRef,&info,NULL,&isDirectory);
	
	if (err == noErr) {
		if (!isDirectory)
			*aliasFileFlag = ((info.file.finderFlags & kIsAlias) != 0);
		*folderFlag = isDirectory;
	}
	
	#else
	err=FSpGetFInfo(fileFSSpec,&fInfo);
	if (err==noErr)
	{
		*aliasFileFlag = (fInfo.fdFlags & kIsAlias) != 0;
	}
	else
	{
		DInfo	dInfo;
		
		err=FSpGetDInfo(fileFSSpec,&dInfo);
		if (err==noErr) *folderFlag=true;
	}
	#endif
	
	return err;
}

/* ユーザインタラクションなしでエイリアスを解決する */
OSErr ResolveAliasWithNoUI(AliasHandle theAlias,FSSpec *theFile,Boolean resolveAliasChains)
{
	OSErr	err;
	Boolean	needsUpdate,wasChanged;
	FSSpec	theSpec;
	Boolean	isAlias,isFolder;
	short	count=1;
	
	err=MatchAlias(nil,kARMNoUI+kARMMultVols+kARMSearch,theAlias,&count,&theSpec,&needsUpdate,nil,nil);
	if (err==noErr)
	{
		*theFile=theSpec;
		if (needsUpdate)
			err=UpdateAlias(nil,&theSpec,theAlias,&wasChanged);
	}
	
	if (err==noErr && resolveAliasChains)
	{
		err=MyIsAliasFile(&theSpec,&isAlias,&isFolder);
		if (err==noErr && isAlias)
			err=ResolveAliasFileWithNoUIMain(&theSpec,resolveAliasChains);
	}
	
	return err;
}

/* ボタンをハイライトさせる */
void HiliteButton(DialogPtr dp,short item)
{
	short	itemType;
	Rect	box;
	Handle	itemHandle;
	unsigned long	tilticks;
	
	GetDialogItem(dp,item,&itemType,&itemHandle,&box);
	if (itemType!=ctrlItem+btnCtrl && itemType!=ctrlItem+resCtrl) return;
	
	HiliteControl((ControlHandle)itemHandle,kControlButtonPart);
	Delay(kMyButtonDelay,&tilticks);
	HiliteControl((ControlHandle)itemHandle,false);
}

/* 文字列の置き換え */
void ReplaceString(Str255 base,Str255 subs,Str15 key)
{
	Handle	baseH,subsH;
	OSErr	err;
	short	result;
	
	err=PtrToHand(&base[1],&baseH,*base);
	err=PtrToHand(&subs[1],&subsH,*subs);
	
	result=ReplaceText(baseH,subsH,key);
	
	if (result>0)
	{
		*base=GetHandleSize(baseH);
		BlockMoveData(*baseH,&base[1],*base);
	}
	
	DisposeHandle(baseH);
	DisposeHandle(subsH);
}

/* check 2 colors are equal or not */
Boolean EqualColor(RGBColor *color1,RGBColor *color2)
{
	return (color1->red==color2->red)&&(color1->green==color2->green)&&
		(color1->blue==color2->blue);
}

/* RGBColorからインデックスに変換 */
short RGBColorToIndex(RGBColor *color)
{
	short	r=0xf & (short)color->red,g=0xf & (short)color->green,b=0xf & (short)color->blue;
	
	if (r==0 && g==0 && b==0) return 255; /* 黒 */
	if (r%3==0 && g%3==0 && b%3==0) /* 216色 */
		return (5-b/3)+(5-g/3)*6+(5-r/3)*36;
	else
		if (g==0) /* red or blue */
			if (b==0) return 215+9-(r*2/3); /* red */
			else return 235+9-(b*2/3); /* blue */
		else /* green or gray */
			if (r==0) return 225+9-(g*2/3); /* green */
			else return 245+9-(r*2/3); /* gray */
}

/* インデックスからRGBColorに変換 */
void IndexToRGBColor(short index,RGBColor *color)
{
	if (index < 215)
		Set256RGBColor(color,(5-index/36)*3,(5-(index/6)%6)*3,(5-index%6)*3); /* 216 */
	else if (index < 225)
		Set256RGBColor(color,14-(index-215)*3/2,0,0); /* red */
	else if (index < 235)
		Set256RGBColor(color,0,14-(index-225)*3/2,0); /* green */
	else if (index < 245)
		Set256RGBColor(color,0,0,14-(index-235)*3/2); /* blue */
	else if (index < 255)
		Set256RGBColor(color,14-(index-245)*3/2,14-(index-245)*3/2,14-(index-245)*3/2); /* gray */
	else
		Set256RGBColor(color,0,0,0); /* black */
}

/* 色を設定 */
void Set256RGBColor(RGBColor *color,short r,short g,short b)
{
	color->red=0x1111U*r;
	color->green=0x1111U*g;
	color->blue=0x1111U*b;
}

/* set color */
void SetRGBColor(RGBColor *color,unsigned short red,unsigned short green,unsigned short blue)
{
	color->red=red;
	color->green=green;
	color->blue=blue;
}

/* モーダルダイアログのフィルタ */
pascal Boolean MyModalDialogFilter(DialogPtr theDialog,EventRecord *theEvent,short *theItemHit)
{
	Boolean		eventHandled=false;
	WindowPtr	theWindow;
	short		part;
	char		key;
	
	switch (theEvent->what)
	{
		case updateEvt:
			theWindow=(WindowPtr)theEvent->message;
			if (theWindow!=nil && GetWindowKind(theWindow)!=kDialogWindowKind)
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
				
				case kUpArrowKey:
				case kDownArrowKey:
					{
						SInt16	itemNo=GetDialogKeyboardFocusItem(theDialog);
						Handle	theItemHandle;
						Str255	theText;
						SInt16	i;
						SInt16	itemType;
						Rect	r;
						Boolean	numOK=true;
						SInt32	number;
						
						/* エディットテキストが選択されていない時は何もしない */
						if (itemNo <= 0) break;
						
						/* 本当にエディットテキストかどうかを判定する */
						GetDialogItem(theDialog,itemNo,&itemType,&theItemHandle,&r);
						if (itemType != kEditTextDialogItem) break;
						
						/* 文字が入力されているかどうかを判定する */
						GetDialogItemText(theItemHandle,theText);
						if (theText[0]==0) break;
						
						/* 文字が数字だけかどうかを判定する */
						for (i=1; i<=theText[0]; i++)
						{
							if (theText[i] < '0' || theText[i] > '9') {numOK=false; break;}
						}
						if (!numOK) break;
						
						/* 数字を得る */
						StringToNum(theText,&number);
						
						/* 矢印キーの方向により、値を変更 */
						number+=(key == kUpArrowKey ? 1 : -1)*((theEvent->modifiers & shiftKey) ? 10 : 1);
						if (number < 0) number=0;
						
						NumToString(number,theText);
						SetDialogItemText(theItemHandle,theText);
						SelectDialogItemText(theDialog,itemNo,0,theText[0]);
						
						*theItemHit=itemNo;
						eventHandled=true;
					}
					break;
			}
			break;
		
		case mouseDown:
			part=FindWindow(theEvent->where,&theWindow);
			if (part==inDrag && theWindow==GetDialogWindow(theDialog))
			{
				Rect	myScreenRect;
				
				GetRegionBounds(GetGrayRgn(),&myScreenRect);
				DragWindow(theWindow,theEvent->where,&myScreenRect);
				eventHandled=true;
			}
			break;
		
		case nullEvent:
			break;
		
		case osEvt:
			if ((unsigned long)(theEvent->message & osEvtMessageMask) >> 24 == suspendResumeMessage)
			{
				if (theEvent->message & resumeFlag) /* resume */
				{
					ActivateDialogControl(theDialog);
					ActivateFloatersAndFirstDocumentWindow();
				}
				else
				{
					DeactivateFloatersAndFirstDocumentWindow();
					DeactivateDialogControl(theDialog);
				}
				MyInvalWindowPortBounds(GetDialogWindow(theDialog));
			}
			break;
		
	}
	
	return eventHandled;
}

#if !TARGET_API_MAC_CARBON
/* カスタムセーブダイアログのアクティベートルーチン */
pascal void MyActivate(DialogPtr theDialog,short item,Boolean activating,void *userData)
{
	#pragma unused(activating,userData)
	
	Rect	box;
	RgnHandle	rgn;
	
	if (GetWRefCon(GetDialogWindow(theDialog))!=sfMainDialogRefCon)
		return;
	
	switch (item)
	{
		case sfItemFileListUser:
			break;
		
		case sfItemFileNameTextEdit:
			GetDialogItemRect(theDialog,item,&box);
			rgn=NewRgn();
			RectRgn(rgn,&box);
			UpdateDialog(theDialog,rgn);
			DisposeRgn(rgn);
			break;
	}
}
#endif

/* ダイアログのコントロールをDeactivate */
void DeactivateDialogControl(DialogPtr theDialog)
{
	ControlHandle	theControl;
	short			i;
	short			itemType;
	Handle			itemHandle;
	Rect			itemRect;
	short			itemNum;
	UInt16			h;
	
	itemNum=CountDITL(theDialog);
	for (i=1; i<=itemNum; i++)
	{
		GetDialogItem(theDialog,i,&itemType,&itemHandle,&itemRect);
		switch (itemType)
		{
			case kButtonDialogItem:
			case kCheckBoxDialogItem:
			case kRadioButtonDialogItem:
			case kResourceControlDialogItem:
				theControl=(ControlHandle)itemHandle;
				h=GetControlHilite(theControl);
				if (h != 255)
				{
					SetDialogItem(theDialog,i,itemType+kItemDisableBit,itemHandle,&itemRect);
					HiliteControl(theControl,255);
				}
				break;
		}
	}
}

/* ダイアログのコントロールをActivate */
void ActivateDialogControl(DialogPtr theDialog)
{
	ControlHandle	theControl;
	short			i;
	short			itemType;
	Handle			itemHandle;
	Rect			itemRect;
	short			itemNum;
	UInt16			h;
	
	itemNum=CountDITL(theDialog);
	for (i=1; i<=itemNum; i++)
	{
		GetDialogItem(theDialog,i,&itemType,&itemHandle,&itemRect);
		switch (itemType)
		{
			case kButtonDialogItem+kItemDisableBit:
			case kCheckBoxDialogItem+kItemDisableBit:
			case kRadioButtonDialogItem+kItemDisableBit:
			case kResourceControlDialogItem+kItemDisableBit:
				theControl=(ControlHandle)itemHandle;
				h=GetControlHilite(theControl);
				if (h == 255)
				{
					SetDialogItem(theDialog,i,itemType-kItemDisableBit,itemHandle,&itemRect);
					HiliteControl(theControl,0);
				}
				break;
		}
	}
}

/* ウィンドウのコントロールをDeactivate */
void DeactivateWindowControl(WindowPtr theWindow)
{
	ControlHandle	theControl;
	OSErr			err;
	
	if (theWindow==nil) return;
	
	err=MyGetRootControl(theWindow,&theControl);
	if (err!=noErr) return;
	
	#if TARGET_API_MAC_CARBON
	{
		UInt16	index,count;
		ControlHandle	outControl;
		
		err=CountSubControls(theControl,&count);
		for (index = 1; index <= count; index++)
		{
			err=GetIndexedSubControl(theControl,index,&outControl);
			DeactivateControl(outControl);
		}
	}
	#else
	while (theControl != nil)
	{
		if (isAppearanceAvailable)
			DeactivateControl(theControl);
		else
			HiliteControl(theControl,255);
		
		theControl=(*theControl)->nextControl;
	}
	#endif
}

/* ウィンドウのコントロールをActivate */
void ActivateWindowControl(WindowPtr theWindow)
{
	ControlHandle	theControl;
	OSErr			err;
	
	if (theWindow==nil) return;
	
	err=MyGetRootControl(theWindow,&theControl);
	if (err!=noErr) return;
	
	#if TARGET_API_MAC_CARBON
	{
		UInt16	index,count;
		ControlHandle	outControl;
		
		err=CountSubControls(theControl,&count);
		for (index = 1; index <= count; index++)
		{
			err=GetIndexedSubControl(theControl,index,&outControl);
			ActivateControl(outControl);
		}
	}
	#else
	while (theControl != nil)
	{
		if (isAppearanceAvailable)
			ActivateControl(theControl);
		else
			HiliteControl(theControl,0);
		
		theControl=(*theControl)->nextControl;
	}
	#endif
}

/* ダイアログからコントロールを取り出す */
Handle GetDialogItemHandle(DialogPtr theDialog,short itemNo)
{
	short	itemType;
	Rect	box;
	Handle	h;
	
	GetDialogItem(theDialog,itemNo,&itemType,&h,&box);
	return h;
}

/* ダイアログのアイテムの位置を得る */
void GetDialogItemRect(DialogPtr theDialog,short itemNo,Rect *r)
{
	short	itemType;
	Handle	h;
	
	GetDialogItem(theDialog,itemNo,&itemType,&h,r);
}

/* ダイアログのコントロールから値を取り出す */
short GetDialogControlValue(DialogPtr theDialog,short itemNo)
{
	return GetControlValue((ControlHandle)GetDialogItemHandle(theDialog,itemNo));
}

/* ダイアログのコントロールに値を設定する */
void SetDialogControlValue(DialogPtr theDialog,short itemNo,short value)
{
	SetControlValue((ControlHandle)GetDialogItemHandle(theDialog,itemNo),value);
}

/* ダイアログのコントロールの最大値を設定する */
void SetDialogControlMaximum(DialogPtr theDialog,short itemNo,short value)
{
	SetControlMaximum((ControlHandle)GetDialogItemHandle(theDialog,itemNo),value);
}

/* ダイアログのコントロールのハイライトを設定する */
void SetDialogControlHilite(DialogPtr theDialog,short itemNo,short value)
{
	HiliteControl((ControlHandle)GetDialogItemHandle(theDialog,itemNo),value);
}

/* ダイアログのコントロールのタイトルを設定する */
void SetDialogControlTitle(DialogPtr theDialog,short itemNo,Str255 title)
{
	SetControlTitle((ControlHandle)GetDialogItemHandle(theDialog,itemNo),title);
}

/* ダイアログのアイテムからテキストを取り出す */
void GetDialogItemText2(DialogPtr theDialog,short itemNo,Str255 string)
{
	GetDialogItemText(GetDialogItemHandle(theDialog,itemNo),string);
}

/* ダイアログのアイテムにテキストを設定する */
void SetDialogItemText2(DialogPtr theDialog,short itemNo,Str255 string)
{
	SetDialogItemText(GetDialogItemHandle(theDialog,itemNo),string);
}

/* ダイアログのエディットテキストをスタティックテキストに変更する */
void SetDialogItemToStaticText(DialogPtr theDialog,short itemNo)
{
	Handle	h;
	Rect	box;
	short	itemType;
	
	GetDialogItem(theDialog,itemNo,&itemType,&h,&box);
	if (itemType == editText)
	SetDialogItem(theDialog,itemNo,statText,h,&box);
}

/* C文字列からPascal文字列へ変換 */
void MyCToPStr(const char *src,Str255 dst)
{
	short	count;
	char	*dstP=(char *)&dst[1];
	
	for (count=0; count <= 255 && *src != 0; count++)
		*dstP++=*src++;
	
	if (count > 255) dst[0]=255;
	else dst[0]=count;
}

/* CopyDeepMask安全バージョン */
void SafeCopyDeepMask(const BitMap *srcBits,const BitMap *maskBits,const BitMap *dstBits,
						const Rect *srcRect,const Rect *maskRect,const Rect *dstRect,
						short mode,RgnHandle maskRgn)
{
	short	offset;
	
	if ((offset = dstRect->left & 3) != 0)
	{
		Rect		tSrcRect=*srcRect,tMaskRect=*maskRect,tDstRect=*dstRect;
		RgnHandle	tempRgn;
		
		tSrcRect.left-=offset;
		tMaskRect.left-=offset;
		tDstRect.left-=offset;
		
		if (maskRgn == nil)
		{
			tempRgn=NewRgn();
			RectRgn(tempRgn,dstRect);
		}
		else tempRgn=maskRgn;
		
		CopyDeepMask(srcBits,maskBits,dstBits,&tSrcRect,&tMaskRect,&tDstRect,mode,tempRgn);
		
		if (maskRgn == nil)
			DisposeRgn(tempRgn);
	}
	else
		CopyDeepMask(srcBits,maskBits,dstBits,srcRect,maskRect,dstRect,mode,maskRgn);
}

#define	kExtraGrowSize	1024	/* 1k bytes */

static OSErr	mError;
static Size		mPictureSize;
static CQDProcs	mProcs,*mSavedProcs;
static Size		mHandleSize;
static Handle	mPictureHnd;
static PicHandle	mPicture;
static RgnHandle	mClipRgn;

/* テンポラリメモリ使用のOpenCPicture */
OSErr TempOpenCPicture(OpenCPicParams *params)
{
	Picture	dummyPict;
	OSErr	err;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	/* メモリ確保 */
	mPictureHnd=TempNewHandle(sizeof(Picture) + kExtraGrowSize,&err);
	if (err!=noErr)
		return err;
	
	/* 変数初期化 */
	mError=noErr;
	mPictureSize=0L;
	mHandleSize=sizeof(Picture) + kExtraGrowSize;
	dummyPict.picSize=0;
	dummyPict.picFrame=params->srcRect;
	
	/* ヘッダ部分の書き込み */
    OSSwapHostToBigRect(&(dummyPict.picFrame));
	BlockMoveData(&dummyPict,*mPictureHnd,sizeof(Picture));
	mPictureSize=sizeof(Picture);
	
	/* PutPictureの置き換え */
	GetGWorld(&cPort,&cDevice);
	mSavedProcs=GetPortGrafProcs(cPort);
	SetStdCProcs(&mProcs);
	mProcs.putPicProc=NewQDPutPicUPP(PutPicToTempMem);
	SetPortCGrafProcs(cPort,&mProcs);
	
	/* クリップリージョンの記録 */
	mClipRgn=NewRgn();
	GetClip(mClipRgn);
	
	mPicture=OpenCPicture(params);
	mError=QDError();
	if (mError!=noErr)
	{
		TempDisposeHandle(mPictureHnd,&err);
		ClosePicture(); /* いらんのかも */
		
		SetPortCGrafProcs(cPort,mSavedProcs);
		DisposeQDPutPicUPP(mProcs.putPicProc);
	}
	return mError;
}

OSErr CheckPictureByte(void)
{
	if (mError == noErr && (mPictureSize & 1))
	{
		char	c;
		
		if (mPictureSize == mHandleSize)
		{
			SetHandleSize(mPictureHnd,mPictureSize+1);
			mError=MemError();
			if (mError!=noErr) return mError;
		}
		
		c=0;
		BlockMoveData(&c,*mPictureHnd+mPictureSize,sizeof(char));
		mPictureSize++;
	}
	
	return mError;
}

OSErr TempClosePicture(PicHandle *resultPict)
{
	OSErr	err;
	GWorldPtr	cPort;
	GDHandle	cDevice;
	
	/* 1バイト補正 */
	mError=CheckPictureByte();
	
	/* ピクチャ記録終了 */
	ClosePicture();
	
	/* PutPictureを元に戻す */
	GetGWorld(&cPort,&cDevice);
	SetPortCGrafProcs(cPort,mSavedProcs);
	DisposeQDPutPicUPP(mProcs.putPicProc);
	DisposeHandle((Handle)mPicture);
	
	/* クリップリージョンを元に戻す */
	SetClip(mClipRgn);
	DisposeRgn(mClipRgn);
	
	if (mError!=noErr)
	{
		TempDisposeHandle(mPictureHnd,&err);
		*resultPict = nil;
	}
	else if (mPictureSize != mHandleSize)
	{
		SetHandleSize(mPictureHnd,mPictureSize);
		*resultPict = (PicHandle)mPictureHnd;
	}
	
	return mError;
}

pascal void PutPicToTempMem(const void *p,SInt16 bytes)
{
	const SInt32    needSize = mPictureSize + bytes;
	if ( mError )  return;      // errorが起こったら後は skip
	
	if (mHandleSize < needSize) {
		// Grow回数を減らして高速化するため大きめで挑戦
		SetHandleSize(mPictureHnd,needSize + kExtraGrowSize);
		mError = MemError();
		if ( mError ) {
			// ぴったりの大きさで再挑戦
			SetHandleSize(mPictureHnd,needSize);
			mError = MemError();
			if ( mError )  return;
		}

		mHandleSize = GetHandleSize(mPictureHnd);
	}

	BlockMoveData(p,*mPictureHnd + mPictureSize,bytes);
	mPictureSize += bytes;
}

/* グレースケールのカラーテーブルを得る */
/* order = trueの時、降順 */
CTabHandle GetGrayscaleCTable(short depth,Boolean order)
{
	CTabHandle	ctab;
	ColorSpec	*cspec;
	short		num_palette=1<<depth;
	
	ctab=(CTabHandle)NewHandle(sizeof(ColorTable)+sizeof(ColorSpec)*(num_palette-1));
	if (ctab != nil)
	{
		short	i;
		UInt16	c;
		
		/* カラーテーブルに値を代入 */
		HLock((Handle)ctab);
		(*ctab)->ctSeed=UniqueID('clut');
		(*ctab)->ctFlags=0;
		(*ctab)->ctSize=num_palette-1;
		
		cspec = &(*ctab)->ctTable[0];
		for (i=0; i<num_palette; i++)
		{
			c=0xFFFFUL*i/(num_palette-1);
			if (order) c^=0xFFFF;
			
			cspec[i].value=i;
			cspec[i].rgb.red=c;
			cspec[i].rgb.green=c;
			cspec[i].rgb.blue=c;
		}
		HUnlock((Handle)ctab);
	}
	return ctab;
}

/* PixMapで使われている色を調べる（256色限定） */
OSErr GetPixMapColors(PixMapHandle pmh,CTabHandle *ctab,short *colorNum)
{
	short	x,y,i;
	UInt8	*colors,*dataAddr,*temp;
	short	rowBytes;
	Rect	bounds;
	CTabHandle	ctable,ori_ctable;
	ColorSpec	*cspec,*ori_cspec;
	short	num;
	short	num_palette;
	
	/* 256色以外の場合はエラー */
	if ((*pmh)->pixelSize != 8) return paramErr;
	
	/* 色リストの初期化 */
	num_palette = 256;
	colors = (UInt8 *)NewPtrClear(num_palette);
	if (colors == nil)
		return memFullErr;
	
	LockPixels(pmh);
	
	/* アドレスなどの初期化 */
	temp = dataAddr = (UInt8 *)(*pmh)->baseAddr;
	rowBytes = (*pmh)->rowBytes & 0x3fff;
	bounds = (*pmh)->bounds;
	
	/* 使われている色を調べる */
	for (y = bounds.top; y < bounds.bottom; y++)
	{
		for (x = bounds.left; x < bounds.right; x++)
		{
			colors[*dataAddr++] = 1;
		}
		temp += rowBytes;
		dataAddr = temp;
	}
	
	UnlockPixels(pmh);
	
	/* 使われている色でカラーテーブルを作成 */
	num = 0;
	ctable = (CTabHandle)NewHandle(sizeof(ColorTable)+sizeof(ColorSpec)*(num_palette-1));
	if (ctable == nil)
	{
		DisposePtr((Ptr)colors);
		return memFullErr;
	}
	
	ori_ctable = (*pmh)->pmTable;
	HLock((Handle)ctable);
	HLock((Handle)ori_ctable);
	cspec = &(*ctable)->ctTable[0];
	ori_cspec = &(*ori_ctable)->ctTable[0];
	(*ctable)->ctSeed = UniqueID('clut');
	(*ctable)->ctFlags = 0;
	for (i=0; i<num_palette; i++)
	{
		if (colors[i])
		{
			cspec[num].value=num;
			cspec[num].rgb = ori_cspec[i].rgb;
			
			num++;
		}
	}
	
	(*ctable)->ctSize = num - 1;
	HUnlock((Handle)ctable);
	HUnlock((Handle)ori_ctable);
	DisposePtr((Ptr)colors);
	
	SetHandleSize((Handle)ctable,sizeof(ColorTable)+sizeof(ColorSpec)*(num-1));
	
	*ctab = ctable;
	*colorNum = num;
	
	return noErr;
}

/* Pictureの色数を調べる */
OSErr GetPictureUniqueColors(PicHandle picture,long *uniqueColors)
{
	PictInfo	pictInfo;
	OSErr		err;
	
	err=GetPictInfo(picture,&pictInfo,0,0,0,0);
	if (err==noErr)
		*uniqueColors = pictInfo.uniqueColors;
	return err;
}

static short gMaxDepth;

/* PICTで使われているBitMap(PixMap)の最大深度を調べる */
OSErr GetPictureMaxDepth(PicHandle picture,short *depth)
{
	GrafPtr	curPort;
	QDProcs		theQDProcs;
	CQDProcs	theCQDProcs;
	PicHandle	dummyPICT;
	QDProcsPtr	tempProcs;
    Rect        picRect;
	
	SetPortWindowPort(gPreviewWindow);
	GetPort(&curPort);
	
	gMaxDepth = 0;
	
	/* bitsProcを置き換えてCopyBitsを横取りする */
	tempProcs=(QDProcsPtr)GetPortGrafProcs(curPort);
	
	if (IsPortColor(curPort))
	{
		SetStdCProcs(&theCQDProcs);
		theCQDProcs.bitsProc=NewQDBitsUPP(MyGetDepthProc);
		SetPortGrafProcs(curPort,&theCQDProcs);
	}
	else
	{
		SetStdProcs(&theQDProcs);
		theQDProcs.bitsProc=NewQDBitsUPP(MyGetDepthProc);
		SetPortGrafProcs(curPort,(CQDProcs *)&theQDProcs);
	}
	
    QDGetPictureBounds(picture, &picRect);
	dummyPICT=OpenPicture(&picRect);
	DrawPicture(picture,&picRect);
	ClosePicture();
	KillPicture(dummyPICT);
	
	/* bitsProcを元に戻す */
	SetPortGrafProcs(curPort,(CQDProcs *)tempProcs);
	SetPort(curPort);
	
	*depth = gMaxDepth;
	return noErr;
}

/* 渡されたBitMap(PixMap)の深度を調べる */
static pascal void MyGetDepthProc(const BitMap *bitPtr,const Rect *srcRect,const Rect *dstRect,
	short mode,RgnHandle maskRgn)
{
	#pragma unused(srcRect,dstRect,mode,maskRgn)
	PixMapPtr aPixMap;
	short tempRB;
	
	tempRB = (*bitPtr).rowBytes;
	if (tempRB < 0) {
		if ((tempRB<<1) < 0)
			aPixMap = (PixMapPtr) bitPtr;
		else
			aPixMap = (PixMapPtr) bitPtr;
		if ((*aPixMap).pixelSize > gMaxDepth)	/* deepest pixmap so far? */
			gMaxDepth = (*aPixMap).pixelSize;
	}
	else {
		/* It's just a BitMap */
		if (1 > gMaxDepth)
			gMaxDepth = 1;
	}
}

/* プロセスの検索 */
Boolean FindProcessFromCreatorAndType(OSType creator,OSType type,ProcessSerialNumber *psn)
{
	ProcessInfoRec	procInfo;
	Boolean			found=false;
	
	psn->highLongOfPSN=0;
	psn->lowLongOfPSN=kNoProcess;
	
	procInfo.processInfoLength=sizeof(ProcessInfoRec);
	procInfo.processName=nil;
	procInfo.processAppSpec=nil;
	procInfo.processLocation=nil;
	
	while (GetNextProcess(psn)==noErr)
	{
		if (GetProcessInformation(psn,&procInfo)==noErr)
		{
			if (procInfo.processType==type && procInfo.processSignature==creator)
			{
				found=true;
				break;
			}
		}
	}
	return found;
}

/* ファイルの修正日を得る */
OSErr FSpGetModDate(FSSpec *spec,unsigned long *modDate)
{
	CInfoPBRec	cInfo;
	OSErr	err;
	
	cInfo.hFileInfo.ioFDirIndex=0;
	/* これを０にすることによって、ioVRefNumで示されるボリュームリファレンス、
	   ioDirIDで示されるディレクトリからファイルを特定する */
	cInfo.hFileInfo.ioVRefNum=spec->vRefNum;
	cInfo.hFileInfo.ioDirID=spec->parID;
	cInfo.hFileInfo.ioNamePtr=(StringPtr)spec->name;
	
	err=PBGetCatInfoSync(&cInfo);
	
	if (err!=noErr) return err;
	else
	{
		*modDate=cInfo.hFileInfo.ioFlMdDat;
		return noErr;
	}
}

/* 拡張子関係 */
/* ファイル名の拡張子以外の部分を得る */
short GetBodyLength(Str255 filename)
{
	short	i;
	Boolean	n=false;
	UInt8	c;
	
	for (i=filename[0]; i>0; i--)
	{
		c = filename[i];
		if (c=='.' && n) break;
		if (c<'0' || c>'9') n=true;
	}
	if (i>0)
		return i-1;
	else
		return filename[0];
}

/* ファイル名の拡張子以外の部分を得る */
short GetBodyLength2(Str255 filename)
{
	short	i;
	UInt8	c;
	
	for (i=filename[0]; i>0; i--)
	{
		c = filename[i];
		if (c<'0' || c>'9' || c=='.') break;
	}
	if (i>0)
		if (c=='.')
			return i-1;
		else
			return i;
	else
		return filename[0];
}

/* ファイル名の拡張子を変更する */
short ChangeSuffix(Str255 filename,Str255 suffix)
{
	short	bodyLength;
	
	filename[0]=GetBodyLength(filename);
	bodyLength=GetBodyLength2(filename);
	TruncPString(filename,filename,min(bodyLength,31-suffix[0]));
	
	PStrCat(suffix,filename);
	
	return bodyLength;
}

/* 拡張子を得る */
void GetSuffix(Str255 filename,Str255 suffix)
{
	short	bodyLength;
	
	bodyLength=GetBodyLength(filename);
	suffix[0] = filename[0]-bodyLength;
	BlockMoveData(&filename[bodyLength+1],&suffix[1],suffix[0]);
}

/* 拡張子からファイルタイプに変換 */
OSErr GetFileTypeFromSuffix(Str255 filename,OSType *fileType)
{
	Str255	suffix;
	Str15	suffixList[] = {"\p.pict","\p.pct","\p.png","\p.icns","\p.jpeg","\p.jpg","\p.gif",
							"\p.psd","\p.bmp","\p.rsrc","\p.ico"};
	OSType	typeList[] = {kPICTFileType,kPICTFileType,kPNGFileType,kXIconFileType,kQTFileTypeJPEG,
							kQTFileTypeJPEG,kGIFFileType,kQTFileTypePhotoShop,kQTFileTypeBMP,
							kResourceFileType,kWinIconFileType};
	const short	num = 11;
	short	i;
	
	GetSuffix(filename,suffix);
	
	for (i=0; i<num; i++)
	{
		if (EqualString(suffix,suffixList[i],false,true))
			break;
	}
	
	if (i < num) *fileType = typeList[i];
	
	return noErr;
}

/* 指定のアプリケーションでファイルを開く */
void AEOpenFileWithApplication(FSSpec *theFile,FSSpec *theApplication)
{
	#if TARGET_RT_MAC_MACHO
	{
		/* Launch Serviceを使う */
		FSRef	appRef;
		FSRef	itemRef;
		OSErr	err;
		LSLaunchFSRefSpec	launchSpec;
		
		err = FSpMakeFSRef(theApplication,&appRef);
		if (err != noErr) return;
		
		err = FSpMakeFSRef(theFile,&itemRef);
		if (err != noErr) return;
		
		launchSpec.appRef = &appRef;
		launchSpec.numDocs = 1;
		launchSpec.itemRefs = &itemRef;
		launchSpec.passThruParams = NULL;
		launchSpec.launchFlags = kLSLaunchDefaults;
		launchSpec.asyncRefCon = NULL;
		
		err = LSOpenFromRefSpec(&launchSpec,NULL);
	}
	#else
	{
		ProcessSerialNumber	psn;
		LaunchParamBlockRec	launchParams;
		AEDesc				launchParamDesc={typeNull,NULL};
		OSErr				err;
		AppleEvent			aeEvent={typeNull,NULL};
		
		/* なにかのProcessSerialNumberが必要なのでとりあえず自分自身のものを使う */
		GetCurrentProcess(&psn);
	
		err=MakeOpenDocumentEvent(&psn,theFile,&aeEvent);
		if (err==noErr)
		{
			/* できたAppleイベントをtypeAppParametersタイプのデスクリプタに変換 */
			err=AECoerceDesc(&aeEvent,typeAppParameters,&launchParamDesc);
			if (err==noErr)
			{
				/* launchParamsにパラメータを設定 */
				launchParams.launchBlockID		= extendedBlock;
				launchParams.launchEPBLength	= extendedBlockLen;
				launchParams.launchFileFlags	= 0;
				launchParams.launchControlFlags	= launchContinue+launchNoFileFlags;
				launchParams.launchAppSpec		= theApplication;
				
				#if TARGET_API_MAC_CARBON
				{
					Size	dataSize;
					
					dataSize=AEGetDescDataSize(&launchParamDesc);
					launchParams.launchAppParameters=(AppParametersPtr)NewPtr(dataSize);
					err=AEGetDescData(&launchParamDesc,launchParams.launchAppParameters,dataSize);
				}
				#else
				HLock(launchParamDesc.dataHandle);
				launchParams.launchAppParameters= (AppParametersPtr)*(launchParamDesc.dataHandle);
				#endif
				
				err = LaunchApplication(&launchParams);
				
				#if TARGET_API_MAC_CARBON
				DisposePtr((Ptr)launchParams.launchAppParameters);
				#else
				HUnlock((Handle)launchParamDesc.dataHandle);
				#endif
				err=AEDisposeDesc(&launchParamDesc);
			}
		}
	}
	#endif
}

/* 'odoc'イベントを作る */
OSErr MakeOpenDocumentEvent(ProcessSerialNumber *targetPSN,FSSpec *theIconFile,AppleEvent *odocEvent)
{
	OSErr		err;
	AppleEvent	aeEvent={typeNull,NULL};
	AEDesc		target={typeNull,NULL};
	AEDescList	fileList={typeNull,NULL};
	AliasHandle	fileAlias=nil;
	
	/* ターゲットアプリケーションのデスクリプタを作成 */
	err=AECreateDesc(typeProcessSerialNumber,targetPSN,sizeof(ProcessSerialNumber),&target);
	
	/* Appleイベントを作成 */
	err=AECreateAppleEvent(kCoreEventClass,kAEOpenDocuments,&target,kAutoGenerateReturnID,
							kAnyTransactionID,&aeEvent);
	
	/* 開くファイルのリストを作成 */
	err=AECreateList(nil,0,false,&fileList);
	
	/* リストにファイルを追加 */
	err=NewAlias(nil,theIconFile,&fileAlias);
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
	
	/* 作成したファイルリストをAppleEventのkeyDirectObjectパラメータに設定 */
	err=AEPutParamDesc(&aeEvent,keyDirectObject,&fileList);
	
	/* できたイベントを返す */
	*odocEvent=aeEvent;
	
	/* 作成したデスクリプタを破棄 */
	err=AEDisposeDesc(&target);
	err=AEDisposeDesc(&fileList);
	
	return err;
}

/* 'fupd'イベントを作る */
OSErr MakeUpdateEvent(const FSSpec *theFile,AppleEvent *result)
{
	OSErr		err;
	AppleEvent	aeEvent={typeNull,NULL};
	AEDesc		target={typeNull,NULL};
	AEDesc		fileDesc={typeNull,NULL};
	AliasHandle	fileAlias=NULL;
	ProcessSerialNumber	psn;
	
	/* Finderを探す */
	if (!FindProcessFromCreatorAndType(kFinderCreator,kFinderType,&psn)) return -1;
	
	/* ターゲットアプリケーションのデスクリプタを作成 */
	err=AECreateDesc(typeProcessSerialNumber,&psn,sizeof(ProcessSerialNumber),&target);
	if (err!=noErr) return err;
	
	/* Appleイベントを作成 */
	err=AECreateAppleEvent(kAEFinderSuite,kAESync,&target,kAutoGenerateReturnID,
							kAnyTransactionID,&aeEvent);
	if (err!=noErr) return err;
	
	/* 更新するファイルのデスクリプタを作成 */
	err=NewAlias(NULL,theFile,&fileAlias);
	if (err!=noErr) return err;
	
	HLock((Handle)fileAlias);
#if __AL_USE_OPAQUE_RECORD__
	Size aliasSize = GetAliasSize( fileAlias );
#else
	unsigned short aliasSize = (*fileAlias)->aliasSize;
#endif
	err=AECreateDesc(typeAlias,(Ptr)*fileAlias,aliasSize,&fileDesc);
	HUnlock((Handle)fileAlias);
	DisposeHandle((Handle)fileAlias);
	
	/* 更新するファイルのデスクリプタをkeyDirectObjectに指定 */
	err=AEPutParamDesc(&aeEvent,keyDirectObject,&fileDesc);
	
	/* できたイベントを返す */
	*result = aeEvent;
	
	/* 作成したデスクリプタを破棄 */
	AEDisposeDesc(&target);
	AEDisposeDesc(&fileDesc);
	
	return err;
}

/* System Folderのあるボリュームリファレンスを得る */
/* thanks > Mr. Hideaki Iimori */
SInt16  GetFindFolderVRefNum(void)
{
	SInt16      vRefNum = kOnSystemDisk;
	SInt32      response;
	OSStatus    err = Gestalt(gestaltFindFolderAttr,&response);
	if (err == noErr) {
		if ((response & (1L << gestaltFolderMgrSupportsDomains)) != 0) {
			vRefNum = kUserDomain;
		}
		else if ((response & (1L << gestaltFolderMgrSupportsExtendedCalls)) != 0) {
			vRefNum = kOnAppropriateDisk;
		}
	}
	
	return vRefNum;
}

/* Drag & Drop関連 */
/* ドラッグされた位置を得る */
pascal OSErr GetDropDirectory (DragReference dragRef, FSSpecPtr fssOut)
{
	OSErr err = noErr;
	
	AEDesc dropLocAlias = { typeNull, nil };
	
	if (!(err = GetDropLocation (dragRef, &dropLocAlias)))
	{
		if (dropLocAlias.descriptorType != typeAlias)
			err = paramErr;
		else
		{
			AEDesc dropLocFSS = { typeNull, nil };
			if (!(err = AECoerceDesc(&dropLocAlias, typeFSS, &dropLocFSS)))
			{
				#if TARGET_API_MAC_CARBON
                FSSpecPtr dummyFSSPtr;
				err = AEGetDescData(&dropLocFSS,&dummyFSSPtr,sizeof(FSSpec));
				BlockMoveData(dummyFSSPtr,fssOut,sizeof(FSSpec));
				#else
				BlockMoveData(*(dropLocFSS.dataHandle),fssOut,sizeof(FSSpec));
				
				#endif
				err = AEDisposeDesc (&dropLocFSS);
			}
		}
		
		if (dropLocAlias.dataHandle)
		{
			OSErr err2 = AEDisposeDesc (&dropLocAlias);
			if (!err) err = err2;
		}
	}
	
	return err;
}

/* セルの選択を解除する */
void CancelSelect(ListHandle theList)
{
	Cell	theCell={0,0};
	
	while (LGetSelect(true,&theCell,theList))
		LSetSelect(false,theCell,theList);
}

/* FSSpecからFinderInfoを得る */
OSErr FSpGetFinderInfo(const FSSpec *theFile, FileInfo *info) {
	FSRef			fileRef;
	OSErr			err;

	/* まず、FSRefに変換 */
	err = FSpMakeFSRef(theFile,&fileRef);
	if (err != noErr) return err;
	
	FSCatalogInfo catInfo;
	err = FSGetCatalogInfo(&fileRef, kFSCatInfoFinderInfo, &catInfo, NULL, NULL, NULL);
	if ( err == noErr )
		info = (FileInfo*)catInfo.finderInfo;

	return err;
}

/* Unicode関連 */
/* FSSpecからファイル名を得る（Unicode対応） */
void FSpGetFileName(const FSSpec *theFile,Str255 filename)
{
	#if !TARGET_API_MAC_CARBON
		PStrCpy(theFile->name,filename);
	#else
		FSRef			fileRef;
		OSErr			err;
		HFSUniStr255	name;
		CFStringRef		strRef;
		
		/* まず、FSRefに変換 */
		err = FSpMakeFSRef(theFile,&fileRef);
		if (err != noErr) return ;
		
		/* FSGetCatalogInfoでUnicodeのファイル名を得る */
		err = FSGetCatalogInfo(&fileRef,kFSCatInfoNone,NULL,&name,NULL,NULL);
		
		/* Unicode文字列をCFStringに変換 */
		strRef = CFStringCreateWithCharacters(kCFAllocatorDefault,
			name.unicode,name.length);
		
		/* CFStringをPascal文字列に変換 */
		#if 0
		{
			short length = CFStringGetLength(strRef);
			if (length > 255) length = 255;
			filename[0] = CFStringGetBytes(strRef,CFRangeMake(0,length),
				kTextEncodingMacJapanese,'^',false,&filename[1],255,NULL);
		}
		#else
		CFStringGetPascalString(strRef,filename,255,kTextEncodingMacJapanese);
		#endif
		
		CFRelease(strRef);
	#endif
}

/* RectをLittle Endianに変換 */
void OSSwapHostToBigRect(Rect *rect)
{
    rect->top = OSSwapHostToBigInt16(rect->top);
    rect->left = OSSwapHostToBigInt16(rect->left);
    rect->bottom = OSSwapHostToBigInt16(rect->bottom);
    rect->right = OSSwapHostToBigInt16(rect->right);
}

/* from MoreFilesExtras.c */
#ifdef __MOREFILESX__
pascal	void	TruncPString(StringPtr destination,
							 ConstStr255Param source,
							 short maxLength)
{
	short	charType;
	
	if ( source != NULL && destination != NULL )	/* don't do anything stupid */
	{
		if ( source[0] > maxLength )
		{
			/* Make sure the string isn't truncated in the middle of */
			/* a multi-byte character. */
			while (maxLength != 0)
			{
				/* Note: CharacterByteType's textOffset parameter is zero-based from the textPtr parameter */
				charType = CharacterByteType((Ptr)&source[1], maxLength - 1, smSystemScript);
				if ( (charType == smSingleByte) || (charType == smLastByte) )
					break;	/* source[maxLength] is now a valid last character */ 
				--maxLength;
			}
		}
		else
		{
			maxLength = source[0];
		}
		/* Set the destination string length */
		destination[0] = maxLength;
		/* and copy maxLength characters (if needed) */
		if ( source != destination )
		{
			while ( maxLength != 0 )
			{
				destination[maxLength] = source[maxLength];
				--maxLength;
			}
		}
	}
}
#endif
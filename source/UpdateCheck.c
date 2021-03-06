/* ------------------------------------------------------------ */
/*  UpdateCheck.c                                               */
/*     URL Access Managerを使ってアップデートのチェック         */
/*                                                              */
/*                 2001.6.3 - 2001.6.17  naoki iimura     	  	*/
/* ------------------------------------------------------------ */

/* includes */
#if !TARGET_API_MAC_CARBON
#ifdef __APPLE_CC__
#include	<Carbon/Carbon.h>
#else
#include	<URLAccess.h>
#endif
#else
#include	<InternetConfig.h>
#endif

#include	<stdio.h>
#include	<string.h>

#include	"Definition.h"
#include	"UsefulRoutines.h"
#include	"Preferences.h"
#include	"PreCarbonSupport.h"

/* 実験用 */
#define		LOCAL_TEST	0
#define		USE_TE		1


typedef struct IPUpdateInfoRec {
	Str31	version;		/* バージョン */
	Str31	date;			/* 日付 */
	Str255	downloadURL;	/* ダウンロード用URL */
	char	*info;			/* その他の情報（変更点） */
} IPUpdateInfoRec;

/* ヘッダ */
extern OSStatus	IPUpdateCheck(void);

static OSStatus	IPDownloadCatalog(Str255 url_string,Handle *catalog);
static OSStatus	IPGetCatalogInfo(Handle catalog,IPUpdateInfoRec *ip_info);
static OSStatus IPDisplayInfo(const IPUpdateInfoRec *ip_info);

static pascal Boolean IPUpdateDialogFilter(DialogPtr theDialog,EventRecord *theEvent,
	short *theItemHit);
static pascal void MyUpdateTEScrollHandler(ControlHandle theControl,short part);

static OSErr IPGet1Line(UInt8 *ptr,Size *pos,Size max,Str255 line);
static UInt32	IPVersionStrToNum(Str31 versionStr);


/* 外部ファイル */
extern Boolean	IsICInstalled(void);
extern OSErr	ICLaunchURLString(Str255 url_string);


#define	UPDATEERR_RESID	4003
#define	UPDATEERR1	1


/* アップデートが存在するかどうかを確認 */
extern OSStatus IPUpdateCheck(void)
{
	OSStatus	err;
	Handle		catalog;
	IPUpdateInfoRec	ip_info;
	Str255		url_string;
	VersRecHndl	vers;
	UInt32		v1,v2;
	
	/* URL Access Managerがサポートされているかどうか */
	if (!URLAccessAvailable()) return -1;
	
	/* URL */
	GetIndString(url_string,163,1);
	
	#if LOCAL_TEST	/* テスト時は、TEXTリソースから読み込む */
	catalog=Get1Resource('TEXT',128);
	#else
	/* カタログファイルのダウンロード */
	err=IPDownloadCatalog(url_string,&catalog);
	if (err!=noErr) return err;
	#endif
	
	/* カタログの解析 */
	err=IPGetCatalogInfo(catalog,&ip_info);
	#if LOCAL_TEST
	ReleaseResource(catalog);
	#else
	DisposeHandle(catalog);
	#endif
	if (err!=noErr)
		return err;
	
	/* バージョンチェック */
	vers=(VersRecHndl)GetResource('vers',1);
	v1 = IPVersionStrToNum((*vers)->shortVersion);
	v2 = IPVersionStrToNum(ip_info.version);
	
	if (v2 > v1) /* 新しいバージョンがある */
	{
		err=IPDisplayInfo(&ip_info);
		if (err!=noErr) return err;
		
		err=ICLaunchURLString(ip_info.downloadURL);
		return err;
	}
	
	ErrorAlertFromResource(UPDATEERR_RESID,UPDATEERR1);
	return noErr;
}

/* カタログファイル（っていうかテキストだけど）をダウンロード */
static OSStatus IPDownloadCatalog(Str255 url_string,Handle *catalog)
{
	URLReference	urlRef;
	char			c[256];
	Handle			destHandle=NULL;
	URLSystemEventUPP	eventProc=NULL;
	OSStatus		err,igErr;
	long			dataSize;
	
	/* C文字列に変換 */
	BlockMoveData(&url_string[1],c,url_string[0]);
	c[url_string[0]]=0x0;
#if 0
    CFStringRef url = CFStringCreateWithCString( kCFAllocatorDefault, c, kCFStringEncodingASCII);
    CFURLRef myURL = CFURLCreateWithString(kCFAllocatorDefault, url, NULL);
    CFHTTPMessageRef myRequest =
        CFHTTPMessageCreateRequest(kCFAllocatorDefault, CFSTR("GET"), myURL,
                               kCFHTTPVersion1_1);

    CFReadStreamRef myReadStream = CFReadStreamCreateForHTTPRequest(kCFAllocatorDefault, myRequest);
    while ( !CFReadStreamOpen(myReadStream) );
    CFHTTPMessageRef myResponse = (CFHTTPMessageRef)CFReadStreamCopyProperty(myReadStream, kCFStreamPropertyHTTPResponseHeader);
    CFStringRef myStatusLine = CFHTTPMessageCopyResponseStatusLine(myResponse);
    
    CFDataRef mySerializedRequest = CFHTTPMessageCopySerializedMessage(myResponse);
    CFIndex length = CFDataGetLength(mySerializedRequest);
    destHandle=NewHandle(length);
    CFDataGetBytes(mySerializedRequest, CFRangeMake(0, length), (UInt8 *)*destHandle);
    
    return noErr;
#else
	err=URLNewReference((const char*)c,&urlRef);
	if (err!=noErr) return err;
	
	destHandle=NewHandle(0); /* とりあえず０バイトのハンドル */
	
	err=URLDownload(urlRef,NULL,destHandle,kURLDisplayProgressFlag,eventProc,NULL);
	if (err!=noErr)
	{
		igErr=URLDisposeReference(urlRef);
		return err;
	}
	
	dataSize=GetHandleSize(destHandle);
	SetHandleSize(destHandle,dataSize+1);
	(*destHandle)[dataSize]=0x0;
	*catalog=destHandle;
	
	return err;
#endif
}

/* ダウンロードしたカタログを分析 */
static OSStatus IPGetCatalogInfo(Handle catalog,IPUpdateInfoRec *ip_info)
{
	OSStatus	err=noErr;
	long		pos,len;
	
	HLock(catalog);
	
	#if TARGET_API_MAC_CARBON
	{
		UInt8	*ptr=(UInt8 *)*catalog;
		Size	len=GetHandleSize(catalog);
		
		pos=0;
		err=IPGet1Line(ptr,&pos,len,ip_info->version);
		err=IPGet1Line(ptr,&pos,len,ip_info->date);
		err=IPGet1Line(ptr,&pos,len,ip_info->downloadURL);
	}
	#else
	{
		char	v[32],d[32],u[256];
		
		sscanf(*catalog,"%s\r%s\r%s\r",v,d,u);
		
		MyCToPStr(v,ip_info->version);
		MyCToPStr(d,ip_info->date);
		MyCToPStr(u,ip_info->downloadURL);
	}
	#endif
	
	pos=ip_info->version[0]+ip_info->date[0]+ip_info->downloadURL[0]+3;
	len=GetHandleSize(catalog)-pos;
	ip_info->info=NewPtr(len+1);
	BlockMoveData(&((*catalog)[pos]),ip_info->info,len);
	ip_info->info[len]=0x0;
	
	HUnlock(catalog);
	
	return err;
}

/* アップデート情報を表示して、ダウンロードするかどうかを確認 */
static OSStatus IPDisplayInfo(const IPUpdateInfoRec *ip_info)
{
	OSStatus	err=noErr;
	DialogPtr	dp;
	short		item=3;
	ModalFilterUPP	mfUPP=NewModalFilterUPP(IPUpdateDialogFilter);
	GrafPtr		port;
	Rect		r;
	#if USE_TE
	TEHandle	hTE;
	ControlHandle	theControl;
	#else
	UInt32		delay1=5,delay2=5;
	UInt16		amount=1;
	Boolean		auto_scroll=true;
	#endif
	
	GetPort(&port);
	
	AddDataToPrefs(ip_info->info,GetPtrSize(ip_info->info),'TEXT',129,"\p");
	
	ParamText(ip_info->version,ip_info->date,"\p","\p");
	dp=GetNewDialog(1001,NULL,kFirstWindowOfClass);
	
	SetPortDialogPort(dp);
	GetDialogItemRect(dp,3,&r);
	#if USE_TE
	{
		Rect	r2;
		short	teHeight,max;
		
		/* create TE */
		r.right -= kScrollBarWidth;
		r2 = r;
		InsetRect(&r2,1,1);
		
		TextFont(applFont);
		TextSize(10);
		hTE=TENew(&r2,&r);
		TESetText(ip_info->info,GetPtrSize(ip_info->info),hTE);
		TextFont(systemFont);
		TextSize(0);
		
		/* create scroll bar */
		r.left = r.right;
		r.right += kScrollBarWidth;
		
		teHeight = ((*hTE)->lineHeight)*((*hTE)->nLines);
		max = teHeight - (r2.bottom - r2.top);
		if (max < 0) max = 0;
		
		theControl=NewControl(GetDialogWindow(dp),&r,"\p",true,0,0,max,kControlScrollBarLiveProc,(SRefCon)NULL);
		
		/* use hTE in callback routine */
		SetWRefCon(GetDialogWindow(dp),(long)hTE);
	}
	#else
		theControl=NewControl(GetDialogWindow(dp),&r,"\p",true,129,0,1,kControlScrollTextBoxProc,NULL);
		
		err=SetControlData(theControl,0,kControlScrollTextBoxDelayBeforeAutoScrollTag,sizeof(delay1),&delay1);
		err=SetControlData(theControl,0,kControlScrollTextBoxDelayBetweenAutoScrollTag,sizeof(delay2),&delay2);
		err=SetControlData(theControl,0,kControlScrollTextBoxAutoScrollAmountTag,sizeof(UInt16),&amount);
	#endif
	
	ShowWindow(GetDialogWindow(dp));
	DrawDialog(dp);
	#if USE_TE
	TEUpdate(&r,hTE);
	#else
	#endif
	DrawControls(GetDialogWindow(dp));
	
	while (item != ok && item != cancel)
	{
		ModalDialog(mfUPP,&item);
	}
	
	DisposeDialog(dp);
	DisposeModalFilterUPP(mfUPP);
	
	#if USE_TE
	TEDispose(hTE);
	#endif
	
	err=(item == ok ? noErr : userCanceledErr);
	
	SetPort(port);
	
	return err;
}

static pascal Boolean IPUpdateDialogFilter(DialogPtr theDialog,EventRecord *theEvent,short *theItemHit)
{
	Boolean		eventHandled=false;
	WindowPtr	theWindow;
	short		part;
	
	switch (theEvent->what)
	{
		case nullEvent:
			IdleControls(GetDialogWindow(theDialog));
			break;
		
		case mouseDown:
			part = FindWindow(theEvent->where,&theWindow);
			if (theWindow == GetDialogWindow(theDialog))
			{
				if (part==inDrag)
				{
					Rect	myScreenRect;
					
					GetRegionBounds(GetGrayRgn(),&myScreenRect);
					DragWindow(theWindow,theEvent->where,&myScreenRect);
					eventHandled=true;
				}
				#if USE_TE
				else if (part == inContent)
				{
					ControlHandle	theControl;
					GrafPtr		port;
					Point		localPt;
					ControlActionUPP	caUPP=NewControlActionUPP(MyUpdateTEScrollHandler);
					
					GetPort(&port);
					SetPortWindowPort(theWindow);
					localPt = theEvent->where;
					GlobalToLocal(&localPt);
					
					part = FindControl(localPt,theWindow,&theControl);
					switch (part)
					{
						case kControlUpButtonPart:
						case kControlDownButtonPart:
						case kControlPageUpPart:
						case kControlPageDownPart:
						case kControlIndicatorPart:
							part=TrackControl(theControl,localPt,caUPP);
							
							eventHandled = true;
							break;
					}
					SetPort(port);
					
					DisposeControlActionUPP(caUPP);
				}
				#else
				else if (part == inContent)
				{
					ControlHandle	theControl;
					GrafPtr		port;
					Point		localPt;
					
					GetPort(&port);
					SetPortWindowPort(theWindow);
					localPt = theEvent->where;
					GlobalToLocal(&localPt);
					
					part = FindControl(localPt,theWindow,&theControl);
					switch (part)
					{
						case kControlUpButtonPart:
						case kControlDownButtonPart:
						case kControlPageUpPart:
						case kControlPageDownPart:
						case kControlIndicatorPart:
							part=TrackControl(theControl,localPt,nil);
							eventHandled = true;
							break;
					}
					SetPort(port);
				}
				#endif
			}
			break;
		
		#if USE_TE
		case updateEvt:
			if (GetDialogFromWindow((WindowPtr)theEvent->message) == theDialog)
			{
				GrafPtr		port;
				TEHandle	hTE;
				Rect		box;
				
				GetPort(&port);
				SetPortDialogPort(theDialog);
				BeginUpdate(GetDialogWindow(theDialog));
				DrawDialog(theDialog);
				DrawControls(GetDialogWindow(theDialog));
				
				hTE = (TEHandle)GetWRefCon(GetDialogWindow(theDialog));
				box = (**hTE).viewRect;
				TEUpdate(&box,hTE);
				
				box.right += kScrollBarWidth;
				InsetRect(&box,-1,-1);
				FrameRect(&box);
				
				EndUpdate(GetDialogWindow(theDialog));
				SetPort(port);
				
				eventHandled = true;
			}
		break;
		#endif
	}
	
	if (!eventHandled)
		eventHandled = MyModalDialogFilter(theDialog,theEvent,theItemHit);
	
	return eventHandled;
}

/* scroll TE */
pascal void MyUpdateTEScrollHandler(ControlHandle theControl,short part)
{
	short	moving=0,value,newValue;
	WindowPtr	theWindow;
	TEHandle	hTE;
	Rect	tempRect;
	short	teHeight;
	
	theWindow=GetControlOwner(theControl);
	hTE = (TEHandle)GetWRefCon(theWindow);
	tempRect = (*hTE)->viewRect;
	
	if (part == kControlIndicatorPart)
	{
		value = tempRect.top - ((*hTE)->destRect).top;
		newValue=GetControlValue(theControl);
		TEScroll(0,value-newValue,hTE);
	}
	else
	{
		teHeight=tempRect.bottom - tempRect.top - kScrollBarHeight;
		
		switch(part)
		{
			case kControlUpButtonPart:
				moving=-16;
				break;
			
			case kControlDownButtonPart:
				moving=16;
				break;
			
			case kControlPageUpPart:
				moving=-(teHeight - 16);
				break;
			
			case kControlPageDownPart:
				moving=(teHeight - 16);
				break;
		}
		
		value=GetControlValue(theControl);
		newValue=value+moving;
		if (newValue > GetControlMaximum(theControl)) newValue=GetControlMaximum(theControl);
		else if (newValue < GetControlMinimum(theControl)) newValue=GetControlMinimum(theControl);
		
		if (value != newValue)
		{
			SetControlValue(theControl,newValue);
			TEScroll(0,value-newValue,hTE);
		}
	}
}

/* １行取り出す */
static OSErr IPGet1Line(UInt8 *ptr,Size *pos,Size max,Str255 line)
{
	char	c=0;
	short	lineLength=0;
	Size	prevPos=*pos;
	
	*line=0;
	while(*pos<max && (c=ptr[(*pos)++])!='\r')
		lineLength++;
	
	BlockMove(&ptr[prevPos],line+1,lineLength);
	*line=lineLength;
	
	return noErr;
}

/* バージョンを数値化 */
/* 		1.00	->	0x10000
		1.10	->	0x11000
		1.19	->	0x11900
		1.19.1	->	0x11910
		1.20a1	->	0x11E01 (0x12000 - 0x200 + 0x1)
		1.20b1	->	0x11F01 (0x12000 - 0x100 + 0x1)
		1.20b10	->	0x11F10 (0x12000 - 0x100 + 0x10
		1.20	->	0x12000 */
static UInt32 IPVersionStrToNum(Str31 versionStr)
{
	short	length=versionStr[0];
	UInt8	*c=&versionStr[1];
	UInt32	result=0,temp;
	short	keta;
	
	/* 整数部分 */
	while (*c >= '0' && *c <= '9' && length > 0)
	{
		result <<= 4;
		result += *c - '0';
		length --;
		c++;
	}
	
	result <<= 16;
	
	if (length>0)
	{
		keta = 12;
		if (*c == '.')
		{
			/* 小数部分 */
			temp = 0;
			c++;
			length --;
			while (*c >= '0' && *c <= '9' && length > 0 && keta >= 0)
			{
				temp += (*c - '0') << keta;
				keta -= 4;
				length--;
				c++;
			}
			
			result += temp;
		}
		if (*c == '.' && length>0 && keta >= 0)
		{
			/* 小数部分 */
			temp = 0;
			c++;
			length --;
			while (*c >= '0' && *c <= '9' && length > 0 && keta >= 0)
			{
				temp += (*c - '0') << keta;
				keta -= 4;
				length--;
				c++;
			}
			
			result += temp;
		}
		if (*c == 'a' || *c == 'b')
		{
			/* アルファ、ベータ */
			result += (*c-'c')<<8;
			temp = 0;
			c++;
			length --;
			while (*c >= '0' && *c <= '9' && length > 0)
			{
				temp <<= 4;
				temp += (*c - '0');
				length --;
				c ++;
			}
			
			result += temp;
		}
	}
	
	return result;
}
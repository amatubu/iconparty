/* ------------------------------------------------------------ */
/*  AE Install.c                                                */
/*      アップルイベントハンドラのインストール処理など          */
/*                                                              */
/*                 1997.1.11 - 2000.1.4  naoki iimura         	*/
/* ------------------------------------------------------------ */


/* includes */
#ifdef __APPLE_CC__
#include	<Carbon/Carbon.h>
#else
#include	<AppleEvents.h>
#include	<Files.h>
#include	<Gestalt.h>
#include	<Errors.h>
#endif

#include	"UsefulRoutines.h"
#include	"AESupport.h"
#include	"FileRoutines.h"

/* definitions */
#define		kGestaltMask	1L


/* prototypes */
static void	AEInstallHandlers(void);

static pascal OSErr	DoOpenApp(const AppleEvent *event,AppleEvent *reply,long refcon);
static pascal OSErr	DoOpenDoc(const AppleEvent *event,AppleEvent *reply,long refcon);
static pascal OSErr	DoPrintDoc(const AppleEvent *event,AppleEvent *reply,long refcon);
static pascal OSErr	DoQuitApp(const AppleEvent *event,AppleEvent *reply,long refcon);
static OSErr	CheckForRequiredParams(const AppleEvent *event);


#define	AEERR_RESID	4012
#define	AEERR1	1
#define	AEERR2	2
#define	AEERR3	3
#define	AEERR4	4
#define	AEERR5	5
#define	AEERR6	6


/* アップルイベントの初期化 */
void AEInit(void)
{
	OSErr	err;
	long	feature;
	
	/* アップルイベントが使用可能かどうかチェック */
	err=Gestalt(gestaltAppleEventsAttr,&feature);
	
	if (err!=noErr)
	{
		ErrorAlertFromResource(AEERR_RESID,AEERR1);
		return;
	}
	if (!(feature&(kGestaltMask << gestaltAppleEventsPresent)))
	{
		ErrorAlertFromResource(AEERR_RESID,AEERR2);
		return;
	}
	
	AEInstallHandlers();
}

/*アップルイベントハンドラのインストール */
void AEInstallHandlers(void)
{
	OSErr	err;
	
	err=AEInstallEventHandler(kCoreEventClass,kAEOpenApplication,NewAEEventHandlerUPP(DoOpenApp),0L,false);
	if (err!=noErr)
	{
		ErrorAlertFromResource(AEERR_RESID,AEERR3);
	}
	err=AEInstallEventHandler(kCoreEventClass,kAEOpenDocuments,NewAEEventHandlerUPP(DoOpenDoc),0L,false);
	if (err!=noErr)
	{
		ErrorAlertFromResource(AEERR_RESID,AEERR4);
	}
	err=AEInstallEventHandler(kCoreEventClass,kAEPrintDocuments,NewAEEventHandlerUPP(DoPrintDoc),0L,false);
	if (err!=noErr)
	{
		ErrorAlertFromResource(AEERR_RESID,AEERR5);
	}
	err=AEInstallEventHandler(kCoreEventClass,kAEQuitApplication,NewAEEventHandlerUPP(DoQuitApp),0L,false);
	if (err!=noErr)
	{
		ErrorAlertFromResource(AEERR_RESID,AEERR6);
	}
}

/* oappアップルイベントハンドラ */
pascal OSErr DoOpenApp(const AppleEvent *event,AppleEvent *reply,long refcon)
{
	#pragma unused(event,reply,refcon)
	
	#if TARGET_API_MAC_CARBON
	return noErr;
	#else
	return HandleOpenApp();
	#endif
}

/* odocアップルイベントハンドラ */
pascal OSErr DoOpenDoc(const AppleEvent *event,AppleEvent *reply,long refcon)
{
	#pragma unused(reply,refcon)
	
	OSErr	err;
	FSSpec	spec;
	OSType	type;
	long	i,numDocs;
	DescType	returnedType;
	AEKeyword	keywd;
	Size	actualSize;
	AEDescList	docList={typeNull,nil};
	
	err=AEGetParamDesc(event,keyDirectObject,typeAEList,&docList);
	
	err=CheckForRequiredParams(event);
	if (err)
	{
		err=AEDisposeDesc(&docList);
		return err;
	}
	
	err=AECountItems(&docList,&numDocs);
	if (err)
	{
		err=AEDisposeDesc(&docList);
		return err;
	}
	
	for (i=1; i<=numDocs; i++)
	{
		err=AEGetNthPtr(&docList,i,typeFSS,&keywd,&returnedType,(Ptr)&spec,sizeof(spec),&actualSize);
		err=AEGetNthPtr(&docList,i,typeType,&keywd,&returnedType,(Ptr)&type,sizeof(type),&actualSize);
		
		/* ファイルを開く */
		err=HandleOpenDoc(&spec);
	}
	err=AEDisposeDesc(&docList);
	
	return err;
}

OSErr CheckForRequiredParams(const AppleEvent *event)
{
	DescType	returnedType;
	Size	actualSize;
	OSErr	err;
	
	err=AEGetAttributePtr(event,keyMissedKeywordAttr,typeWildCard,&returnedType,nil,0,&actualSize);
	
	if (err==errAEDescNotFound)
		return noErr;
	else
		if (err==noErr)
			return errAEParamMissed;
		else
			return err;
}

pascal OSErr DoPrintDoc(const AppleEvent *event,AppleEvent *reply,long refcon)
{
	#pragma unused(event,reply,refcon)
	
	return noErr;
}

pascal OSErr DoQuitApp(const AppleEvent *event,AppleEvent *reply,long refcon)
{
	#pragma unused(event,reply,refcon)
	
	Quit();
	
	return noErr;
}
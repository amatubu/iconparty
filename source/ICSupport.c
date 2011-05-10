/* ------------------------------------------------------------ */
/*  ICSupport.c                                                 */
/*     InternetConfigのサポート                                 */
/*                                                              */
/*                 1999.11.25 - 1999.11.25  naoki iimura       	*/
/* ------------------------------------------------------------ */

/* includes */
#ifdef __APPLE_CC__
#include	<Carbon/Carbon.h>
#else
#if !TARGET_API_MAC_CARBON
#include	"ICTypes.h"
#include	"ICKeys.h"
#include	"ICAPI.h"
#include	"ICComponentSelectors.h"
#else
#include	<InternetConfig.h>
#endif
#include	<Gestalt.h>
#endif

#include	"Definition.h"

extern Boolean	IsICInstalled(void);
extern OSErr	ICLaunchURLString(Str255 urlStr);

/* InternetConfigがインストールされているかどうかを調べる */
extern Boolean IsICInstalled(void)
{
	long	response;
	
	return (Gestalt(gestaltComponentMgr,&response) == noErr);
}

extern OSErr	ICLaunchURLString(Str255 urlStr)
{
	#if !TARGET_API_MAC_CARBON
	ICError		err,igErr;
	#else
	OSErr		err,igErr;
	#endif
	ICInstance	inst;
	long		selStart,selEnd;
	
	if (!IsICInstalled()) return -1;
	
	err=ICStart(&inst,kIconPartyCreator);
	if (err!=noErr) return err;
	
	#if !TARGET_API_MAC_CARBON
	err=ICFindConfigFile(inst,0,nil);
	if (err!=noErr)
		goto errExit;
	#endif
	
	selStart=0;
	selEnd=urlStr[0];
	err=ICLaunchURL(inst,"\p",(char *)&urlStr[1],urlStr[0],&selStart,&selEnd);
	
	#if !TARGET_API_MAC_CARBON
errExit:
	#endif
	igErr=ICStop(inst);
	
	return err;
}
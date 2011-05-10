/* ------------------------------------------------------------ */
/*  NavigationServicesSupport.h                                 */
/*     NavigationServices関連のヘッダファイル                   */
/*                                                              */
/*                 98.12.5 - 99.9.7  naoki iimura               */
/* ------------------------------------------------------------ */

/* includes */
#ifdef __APPLE_CC__
#include	<Carbon/Carbon.h>
#else
#include	<Navigation.h>
#include	<LowMem.h>
#include	<TextUtils.h>
#endif

/* definitions */
#define kOpenRsrcType			'open'
#define kOpenRsrcID				1000

/* structures */
typedef struct NavSaveDataRec {
	short	selItem;
	Boolean	splitFlag;
	long	splitNum;
	Str15	numStr;
	Boolean	bodySelected;
} NavSaveDataRec;

typedef struct NavExportIconDataRec {
	short	selItem;
	Boolean	bodySelected;
} NavExportIconDataRec;

/* prototypes */
OSErr	OpenFileWithNav(void);
OSErr	ChooseFileWithNav(short numTypes,OSType *typeList,Str255 prompt,FSSpec *theFile);
OSErr	NewIconFileWithNav(FSSpec *theFile,NavReplyRecord *theReply);
OSErr	ExportIconWithNav(Str255 filename,FSSpec *theFile,NavReplyRecord *theReply,OSType *iconType);
OSErr	SaveBlendPaletteWithNav(FSSpec *theFile,NavReplyRecord *theReply,Str255 prompt,Str255 filename);
OSErr	SaveFileWithNav(FSSpec *theFile,OSType *fileType,NavReplyRecord *theReply,long num);
short	AskSaveWithNav(Str255 saveFileName,short action);
short	AskRevertWithNav(Str255 saveFileName);

OSErr	SelectObjectDialog(NavEventUPP eventProc,Str255 str,NavReplyRecord *theReply);
OSErr	SelectFolderDialog(NavEventUPP eventProc,Str255 str,FSSpec *folderSpec);

pascal void MyEventProc(const NavEventCallbackMessage callBackSelector, 
						NavCBRecPtr callBackParms, 
						NavCallBackUserData callBackUD);

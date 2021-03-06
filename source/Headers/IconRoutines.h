/* ------------------------------------------------------------ */
/*  IconRoutines.h                                              */
/*     アイコン処理用ヘッダファイル                             */
/*                                                              */
/*                 1998.12.2 - 2001.1.21  naoki iimura		    */
/* ------------------------------------------------------------ */


#pragma once

/* includes */
#ifdef __APPLE_CC__
#include	<Carbon/Carbon.h>
#else
#include	<Navigation.h>
#endif
#include	"IPIconSupport.h"

/* definitions */
#define		kIconListWidth	56
#define		kIconListHeight	64		

#define		TEBitSet	1

#define		kIconFamilyPictureResID		140
#define		sFamilyWinTitle	139
#define		kSelectorMyData	kSelectorLarge1Bit | kSelectorLarge4Bit | kSelectorLarge8Bit | \
							kSelectorSmall1Bit | kSelectorSmall4Bit | kSelectorSmall8Bit
#define		kSelectorMy32Data	kSelectorMyData | \
							kSelectorLarge32Bit | kSelectorLarge8BitMask | \
							kSelectorSmall32Bit | kSelectorSmall8BitMask

enum {
	kL8Data=0,kL4Data,kL1Data,kL1Mask,kS8Data,kS4Data,kS1Data,kS1Mask,
	kL32Data,kL8Mask,kS32Data,kS8Mask,kT32Data,kT8Mask,
};

enum {
	kCommandNone=0,
	kSelectIconCommand,
	kDeleteIconCommand,
};

enum {
	kForceNone=0,
	kForceInternalEditor,
	kForceExternalEditor,
};

enum {
	kDeleteIconFamilyMode = 1,
	kAddResourceForkMode = 2,
	kDeleteIconMode = 3,
};

enum {
	kFamilyWindowSmallWidth = 195,
	kFamilyWindowSmallHeight = 134,
	
	kFamilyWindowLargeWidth = 295,
	kFamilyWindowLargeHeight = 134,
	
	kFamilyWindowThumbnailHeight = 292,
};

#define	GetFamilyWindowHeight()	(isThumbnailIconsAvailable ? kFamilyWindowThumbnailHeight : \
									kFamilyWindowLargeHeight)


/* 構造体 */
typedef struct IconListRec {
	short	gApplRefNum;	/* アプリケーションの参照番号 */
	short	refNum;			/* アイコンファイルの参照番号 */
	short	tempRefNum;		/* テンポラリファイルの参照番号 */
	short	iconNum;		/* アイコンの数 */
	short	*background;	/* 背景の種類 */
	
	Boolean	isIconServicesAvailable;
} IconListRec;

typedef struct MyIconResRec
{
	short	resID;
	Str255	resName;
	short	attrs;
} MyIconResRec;

#pragma options align=mac68k

typedef struct MyIconListDragRec {
	IconSuiteRef	iconSuite;
	Str255			iconName;
} MyIconListDragRec;

#pragma options align=reset


/* prototypes */
/* アイコンリストウィンドウ */
OSErr	MakeFolderWithIcon(FSSpec *theFolder,IconSuiteRef iconSuite);
OSErr	MakeFileWithIcon(const FSSpec *theFile,IconSuiteRef iconSuite);

/* アイコン保存 */
pascal OSErr	AddIconToFile(ResType theType,Handle *theIcon,void *yourDataPtr);
void	SaveDataToResource(void *dataPtr,long dataSize,ResType type,short id,
	Str255 resName,short attrs);
void	DeleteIconFamilyResource(void);

void	RedrawIconPreview(void);
void	UpdateIconPreview(WindowPtr frontWin);

OSErr	MyGetIPIcon(IconListWinRec *iWinRec,IPIconRec *ipIcon,IconListDataRec *data,
	Str255 iconName,Boolean *isEditableIconFamily);
void	CopyIcons(short srcRefNum,short dstRefNum);

pascal OSErr	CopyIconData(ResType theType,Handle *theIcon,void *yourDataPtr);
pascal OSErr	DetachIcon(ResType theType,Handle *iconData,void *yourDataPtr);
pascal OSErr	DeleteIcon(ResType theType,Handle *theIcon,void *yourDataPtr);
pascal OSErr	ChangeIconInfo(ResType theType,Handle *theIcon,void *yourDataPtr);

void	CreateDeleteInfo(short resID);
void	CreateUpdateInfo(short resID);
void	RemoveDeleteInfo(short resID);

Boolean	IsIconChanged(short resID);

Boolean	Is32Exist(IconSuiteRef iconSuite);
Boolean	IsEditableIconFamily(IconFamilyHandle iconFamily);

IconSuiteRef	GetFileIconSuite(FSSpec *theFile);
OSErr	XIconToIconSuite(FSSpec *theFile,IconSuiteRef *iconSuite);
OSErr	Get1IconSuite(IconSuiteRef *theIconSuite,short theResID,IconSelectorValue selector);
Boolean	IsValidIconSuite(IconSuiteRef theIconSuite);
OSErr	IconSuiteToClip(IconSuiteRef iconSuite);

OSErr	NewIconDialog(MyIconResRec *newIcon,IPIconSelector *selector,short *iconTemplate,
	ListHandle iconList);
OSErr	GetIPIconFromTemplate(IPIconRec *ipIcon,const IPIconSelector *selector,short iconTemplate);
OSErr	SaveAsIconDialog(Str255 filename,FSSpec *theFile,OSType *iconType,NavReplyRecord *theReply);


#define	GetMySelector()	(gSystemVersion >= 0x0850 ? kSelectorMy32Data : kSelectorMyData)

#define	GetIconListRec(w)	(*(IconListWinRec **)GetExtWRefCon(w))
#define	GetIconFamilyRec(w)	(*(IconFamilyWinRec **)GetExtWRefCon(w))

/* ------------------------------------------------------------ */
/*  Definition.h                                                */
/*     定数、構造体などの定義                                   */
/*                                                              */
/*                 97.1.24 - 99.5.24  naoki iimura              */
/* ------------------------------------------------------------ */

#pragma once
#ifndef	_IconParty_
	#define		_IconParty_ 
#endif

/* includes */
#ifdef __APPLE_CC__
#include	<Carbon/Carbon.h>
#else
#include	<TSMTE.h>
#include	<Lists.h>
#include	<MacTypes.h>
#endif
#include	"WindowExtensions.h"

/* difinitions */
#ifndef LONG_MAX
#define		LONG_MAX	2147483647L
#endif
#define		kSleep		6

#define		kGestaltMask	1L

/* definitions */
#define 	kMyButtonDelay				8
#define 	kReturnKey					(char)0x0D	
#define 	kEnterKey					(char)0x03	
#define 	kEscapeKey					(char)0x1B	
#define 	kPeriod						'.'
#define		kUpArrowKey					(char)0x1E
#define		kDownArrowKey				(char)0x1F
#define		kDeleteKey					(char)0x08

enum { kFinderType=OSSwapHostToBigInt32('FNDR'), kFinderCreator=OSSwapHostToBigInt32('MACS')};
enum { kResEditType=OSSwapHostToBigInt32('APPL'), kResEditCreator=OSSwapHostToBigInt32('RSED')};
enum { kClip2gifType=OSSwapHostToBigInt32('APPL'), kClip2gifCreator=OSSwapHostToBigInt32('c2gf')};
enum { kIconPartyCreator=OSSwapHostToBigInt32('Ipty')};
enum { kGraphicConverterCreator=OSSwapHostToBigInt32('GKON')};
enum {
	kPICTFileType=OSSwapHostToBigInt32('PICT'),
	kGIFFileType=OSSwapHostToBigInt32('GIFf'),
	kPNGFileType=OSSwapHostToBigInt32('PNGf'),
	kResourceFileType=OSSwapHostToBigInt32('rsrc'),
	kDotModeLibFileType=OSSwapHostToBigInt32('iPat'),
	kBlendPaletteFileType=OSSwapHostToBigInt32('iBld'),
	kTemporaryFileType=OSSwapHostToBigInt32('temp'),
	kWinIconFileType=OSSwapHostToBigInt32('ICO '),
	kXIconFileType=OSSwapHostToBigInt32('icns'),
	kPreferencesFileType=OSSwapHostToBigInt32('pref'),
	kFolderType=OSSwapHostToBigInt32('fold'),
	
	kDeleteIconInfoType=OSSwapHostToBigInt32('iDel'),
	kUpdateIconInfoType=OSSwapHostToBigInt32('iUpd'),
};

#define		kRatioWidth			30
#define		kBackWidth			13
#define		kScrollBarWidth		15
#define		kScrollBarHeight	15

#define		kBaseResID	128

#define		kDotCommandMaxLength	30

#if TARGET_API_MAC_CARBON
#define		kDefaultWinPos	{113,42}
#else
#define		kDefaultWinPos	{110,45}
#endif
#define		kWinOffset		{30,20}

#define 	kCustomComment	100
#define 	kSubSelection	200
#define		kSubLayerInfo	201

/* WIND resource ID */
enum {
	kPaintWindowResID=kBaseResID,
	kColorPalette1ResID,
	kColorPalette2ResID,
	kPreviewWindowResID,
	kToolPaletteResID,
	kBlendPaletteResID,
	kTitleWindowResID,
	kInfoWindowResID,
	kIconListWindowResID,
	kIconFamilyWindowResID,
	kPatternPaletteWindowResID,
	kDotModePaletteWindowResID,
	kFavoritePaletteResID,
};


/* Window refCon */
enum {
	kWindowTypePaintWindow=kBaseResID,
	kWindowTypePreviewWindow,
	kWindowTypeToolPalette,
	kWindowTypeColorPalette1,
	kWindowTypeColorPalette2,
	kWindowTypeBlendPalette,
	kWindowTypeTitleWindow,
	kWindowTypeInfoWindow,
	kWindowTypePatternPalette,
	kWindowTypeDotModePalette,
	kWindowTypeIconListWindow,
	kWindowTypeIconFamilyWindow,
	kWindowTypeFavoritePalette,
};

enum {
	kFirstPaletteRefCon = kWindowTypeFavoritePalette,
	kLastPaletteRefCon = kWindowTypeFavoritePalette,
};


/* Pictures */
enum {
	kColorPalette1PictureResID=kBaseResID,
	kColorPalette2PictureResID,
	kToolPalettePictureResID=132,
	kBlendPalettePictureResID,
	kInfoWindow0pPictureResID,
	kTitleWindowPictureResID=139,
	kDotModePalettePictureResID=131,
};



/* Cursors */
#define		kCursorHandOff		133
#define		kCursorHandOn		134
#define		kCursorHandOffPlus	135
#define		kCursorHandOnPlus	136


/* 保存しますかのダイアログ */
#define		kDialogItemDontSave	3

/* 保存しますかダイアログのフラグ */
#define		kSaveChangedCloseWindow		1
#define		kSaveChangedQuitApplication	2


/* Strings */
enum {
	sUndoString=kBaseResID,
	sRedoString,
	sFileKind,
	sFileSuffix,
	sUntitled,
	sLoadAlertString,
	sPrefFileString,
	sGifVersionString,
	sChangeSizeName,
	sIconIDString
};

/* file suffix */
enum {
	siPICTFile=1,
	siPNGFile,
	siIconFile,
	siWIconFile,
};

/* Tools */
enum {
	kPencilTool=0,
	kEraserTool,
	kMarqueeTool,
	kSpoitTool,
	kBucketTool
};

#define		nTools		kBucketTool

/* パレットのチェック */
enum {
	kPaletteCheckNone=0,
	kPaletteCheckAIC,
	kPaletteCheck16,
	kPaletteCheckUsed,
};

/* Undo mode */
enum {
	umCannot=0,
	umPaint,
	umSelect,
	umMove,
	umEffect,
	umDeselect,
	umDuplicate,
	umCut,
	umPaste,
	umDelete,
	umExchange,
	umResize,
	umRotate,
	umColorChange,
	umReplaceColor,
	umFlip,
	umInputText,
	umMaskChange,
	umDotPaint,
	
	umAddBackground,
	umAddForeground,
	umUnionBackground,
	umDeleteForeground,
	umDeleteBackground,
	umChangeTransparency,
	
	umAddIcon,
	umDeleteIcon,
	umChangeIconInfo,
	
	umPasteIconData,
	umChangeIconName,
	umDeleteIconData,
	umAddIconData,
	
	undoModeNum
};

#define	kIconPreviewWidth	0x7f
#define	kIconPreviewHeight	0xad

/* icon color */
enum {
	kicl8Depth=8,
	kicl4Depth=4,
	kICNDepth=1
};

/* scroll bar */
enum {
	kHScroll=1,
	kVScroll=2,
};

/* foreground transparency */
enum {
	kForeTrans100,
	kForeTrans75,
	kForeTrans50,
	kForeTrans25,
	kForeTrans0
};

/* PNGのための定数 */
enum {
	kPNGTransNone=0,
	kPNGTransWhite,
	kPNGTransBottomRight,
	kPNGTransPenColor,
	kPNGTransBlack,
	kPNGTransEraserColor,
	kPNGTransTransColor,
};

/* 色モードの定数 */
enum {
	kNormal8BitColorMode = k8IndexedPixelFormat,
	k32BitColorMode = k24RGBPixelFormat,
};

/* structures */
/* ドラッグのグローバル */
typedef struct MyDragGlobalsRec
{
	Boolean	canAcceptDrag;
	Boolean	isInContent;
} MyDragGlobalsRec;

/* BitMap */
typedef struct MyBitMapRec {
	BitMap	bmp;
	Handle	bmpDataH;
} MyBitMapRec;

/* Drag&Drop、アクティベートなどのハンドラ */
typedef struct DragHandlerRec {
	DragTrackingHandlerUPP	trackUPP;
	DragReceiveHandlerUPP	receiveUPP;
	
	MyDragGlobalsRec	dragGlobals;
	
	ActivateHandlerUPP	activateUPP;
} DragHandlerRec;

/* IconPartyでアイコンを扱う内部形式（thumbnail対応） */
typedef struct IPIconRec {
	IconSuiteRef	iconSuite;
	Handle			it32Data;
	Handle			t8mkData;
} IPIconRec;

/* アイコンリストウィンドウでの取り消し処理用データ */
typedef struct IconListDataRec {
	short	resID;
	ResType	resType;
} IconListDataRec;

typedef struct IconFileUndoRec
{
	short	undoMode;				/* 取り消しモード */
	
	short	iconNum;				/* 取り消し用のアイコンの数 */
	IconListDataRec	*iconList;		/* 取り消し用のIDリスト */
	
	Str255	name;					/* 取り消し用のアイコンの名前（情報変更時） */
} IconFileUndoRec;

/* アイコンリストウィンドウに付属する構造体 */
typedef struct IconListWinRec
{
	ListHandle	iconList;			/* アイコンIDのリスト */
	short	refNum;					/* アイコンファイルのリファレンスナンバ */
	FSSpec	iconFileSpec;			/* アイコンファイルの保存場所 */
	short	tempRefNum;				/* テンポラリファイルのリファレンスナンバ */
	FSSpec	tempFileSpec;			/* テンポラリファイル */
	Boolean	wasSaved;				/* 保存されているかどうか */
	Boolean	wasChanged;				/* 変更されているかどうか */
	
	Boolean	cmdDown;				/* Finderへのドロップ時、Winアイコンを書き出すフラグ */
	
	IconFileUndoRec	undoData;		/* 取り消し用のデータ */
	
	DragHandlerRec	dragHandlers;	/* ドラッグ用 */
} IconListWinRec;

/* アイコンファミリウィンドウでの取り消し処理用データ */
typedef struct IconFamilyUndoRec
{
	short	undoMode;				/* 取り消しモード */
	
	short	iconKind;				/* 取り消し用のアイコンの種類 */
	Handle	iconData;				/* 取り消し用のアイコンデータ */
} IconFamilyUndoRec;

#pragma mark __IconFamilyWinRec__
/* アイコンファミリウィンドウに付属する構造体 */
typedef struct IconFamilyWinRec
{
	WindowPtr	parentWindow;		/* 親ウィンドウ（アイコンリストウィンドウ）へのポインタ */
	OSType		iconType;			/* アイコンタイプ */
	 /* parentWindow == nil時、
	 		'icns'		X用アイコン
	 		'wIco'		Win用アイコン
	 		'Icon'		アイコン付きフォルダ
	 	parentWindow != nil時、
	 		'icns'		アイコンファミリリソース
	 		'ICN#'		アイコンリソース */
	FSSpec		theIconSpec;		/* アイコンつきフォルダ／ファイルの位置（parentWindow==nil時） */
	Boolean		isSaved;			/* 保存されているかどうか */
	Boolean		isFolder;			/* フォルダかファイルかを示すフラグ */
//	Handle		iconSuite;			/* アイコンのデータ自体 */
	IPIconRec	ipIcon;				/* アイコンのデータ */
	short		iconID;				/* アイコンのID */
//	ResType		resType;			/* 元のアイコンタイプ（'ICN#'か'icns'） */
	Str255		iconName;			/* アイコンの名前 */
	short		selectedIcon;		/* 選択中のアイコン */
	TEHandle	iconNameTE;			/* アイコンの名前を編集するためのTE */
	Boolean		wasNameTEActive;	/* アイコンの名前TEがアクティブだったかどうか */
	TSMDocumentID	docID;			/* インライン入力用 */
	TSMTERecHandle	docTSMRecH;		/* インライン入力用 */
	Boolean		wasChanged;			/* 変更されたかどうかのフラグ */
	
	Boolean		show32Icons;		/* 8.5用のアイコンを表示するかどうか */
	Boolean		showXIcons;			/* X用のアイコンを表示するかどうか */
	
	short		largeWindowWidth;	
	short		largeWindowHeight;	
	
	IconFamilyUndoRec	undoData;	/* 取り消し用データ */
	
	DragHandlerRec	dragHandlers;	/* ドラッグ用 */
} IconFamilyWinRec;

typedef struct IconInfoRec
{
	OSType	fileType;
	OSType	creatorCode;
} IconInfoRec;

#pragma mark __PaintWinRec__
/* 編集ウィンドウに付属する構造体 */
typedef struct PaintWinRec
{
	Rect		iconSize;			/* アイコンの大きさ */
	Rect		iconEditSize;		/* 編集領域の大きさ */
	short		ratio;				/* 倍率（2^ratio倍） */
	Rect		editRect;			/* 編集ウィンドウの描画可能領域 */
	ControlHandle	hScroll;		/* 水平スクロールバー */
	ControlHandle	vScroll;		/* 垂直スクロールバー */
	
	short		colorMode;			/* 色モード（8 or 32） */
	short		iconDepth;			/* 色深度（8 or 32） */
	
	GWorldPtr	editDataPtr;		/* 編集用 */
	GWorldPtr	tempDataPtr;		/* 取消用 */
	GWorldPtr	dispTempPtr;		/* 表示用 */
	GWorldPtr	selectedDataPtr;	/* 選択領域外 */
	
	GWorldPtr	foregroundGWorld;	/* 前景 */
	GWorldPtr	foregroundMask;		/* 前景のマスク */
	GWorldPtr	currentMask;		/* 編集画面のマスク */
	GWorldPtr	pCurrentMask;		/* 前回の編集画面のマスク */
	GWorldPtr	selectionMask;		/* 選択領域外のマスク */
	GWorldPtr	backgroundGWorld;	/* 背景 */
	GWorldPtr	backgroundMask;		/* 背景のマスク（不使用） */
	
	Boolean		isBackTransparent;	/* 背景が透明かどうか（レイヤーが存在しない場合のみ） */
	
	Boolean		editBackground;		/* バックグラウンドを編集 */
	Boolean		dispBackground;		/* バックグラウンドを表示 */
	short		foreTransparency;	/* 前景の不透明度 */
	short		pForeTransparency;	/* 前回の前景の不透明度 */
	
	Boolean		resizeImage;		/* 直前のリサイズの設定 */
	Boolean		resizeDither;
	
	RgnHandle	eSelectedRgn;		/* ペイントウィンドウでの選択領域 */ 
	RgnHandle	ePrevSelectedRgn;	/* 前回の選択領域 */
	RgnHandle	selectionPos;		/* 実際の選択位置 */
	RgnHandle	prevSelectionPos;	/* 前回の選択位置 */
	Rect		selectedRect;		/* 選択領域（実際） */
	Rect		prevSelectedRect;	/* 前回の選択領域 */
	Point		selectionOffset;	/* 選択領域の実際の位置とのずれ */
	Point		prevMovement;		
	short		prevRotate;			/* 前回回転された角度 */
	
	Boolean		isSelected;			/* 選択状態かどうか */
	
	RGBColor	replacedColor;
	Boolean		replacedTransparent;
	
	RgnHandle	updateRgn;			/* 再描画する領域 */
	
	short		selectionBlend;		/* 選択領域と背景の重ね合せ */
	Boolean		showSelection;		/* 選択枠を表示するかどうか */
	
	Boolean		iconHasChanged;		/* アイコンが変更されているかどうか */
	Boolean		iconHasSaved;		/* アイコンが保存されているかどうか */
	
	IconInfoRec	iconType;			/* 保存する際のファイルタイプ／アイコンのタイプ */
	FSSpec		saveFileSpec;		/* アイコンを保存する場所 */
	WindowPtr	parentWindow;		/* 親ウィンドウ（ファミリウィンドウ）のポインタ */
	short		iconKind;
	
	FSSpec		tempFileSpec;
	short		tempFileRefNum;		/* テンポラリファイル */
	
	/* 描画点モード用 */
	Boolean		isDotMode;			/* 描画点モードかどうか */
	Point		dotPos;				/* 描画点の位置 */
	Boolean		isInverted;			/* 描画点が反転されているかどうか */
	UInt32		lastInvertedTime;	/* 描画点が最後に点滅した時間 */
	Str31		dotCommand;			/* 記録されている描画点コマンド */
	
	Point		firstDotPos;		/* 描画点モードに入った時の位置 */
	Str31		prevDotCommand;		/* 記録されている描画点コマンド（取り消し用） */
	
	/* シフトライン用 */
	Point	lastPaintPos;			/* 鉛筆、消しゴムで最後に描画した場所 */
	
	short	undoMode;				/* 取り消しのモード */
	
	DragHandlerRec	dragHandlers;	/* ドラッグ用 */
} PaintWinRec;
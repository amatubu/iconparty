/* ------------------------------------------------------------ */
/*  Globals.h                                                   */
/*     グローバル変数リスト                                     */
/*                                                              */
/*                 97.1.28 - 99.5.24  naoki iimura              */
/* ------------------------------------------------------------ */

#ifdef __APPLE_CC__
#include	<Carbon/Carbon.h>
#else
#include	<QDOffscreen.h>
#endif
#if !TARGET_API_MAC_CARBON
#include	"Wacom.h"
#endif
#include	"Definition.h"

#pragma once
#ifdef	GLOBAL_DEFINE
#define	Extern	/* define */
#else
#define	Extern	extern
#endif


/* theWindow pointers */
Extern WindowPtr	gPreviewWindow;
Extern WindowPtr	gToolPalette;
Extern WindowPtr	gBlendPalette;

/* gworlds */
//Extern GWorldPtr	currentPort;
//Extern GDHandle		currentDevice;

Extern Boolean quit;

/* colors */
typedef struct MyColorRec {
	RGBColor	rgb;
	Boolean		isTransparent;
} MyColorRec;

Extern MyColorRec	gCurrentColor;
Extern MyColorRec	gPrevColor;
Extern MyColorRec	gBackColor;
Extern RGBColor	rgbBlackColor;
Extern RGBColor	rgbWhiteColor;
Extern RGBColor	rgbLBColor;

/* blend mode */
Extern RGBColor	gBlendRatio;
Extern short	gBlendMode;
Extern short	gBSelectedItem;

/* tools */
Extern short	gSelectedTool;
Extern Rect		gToolRect;

/* cursor */
Extern RgnHandle	gCurRgnHand;

/* pen&eraser size */
Extern short	gPenWidth;
Extern short	gPenHeight;
Extern short	gEraserWidth;
Extern short	gEraserHeight;

/* dot command */
Extern Str31	gDotCommand;

/* パターン */
Extern short	gPatternNo;

/* blend palette */
Extern GWorldPtr	gBlendPalettePtr;
Extern Boolean		gBlendLocked;

/* ruler */
Extern Point	gPrevRulerPt;
Extern RgnHandle	gRulerRgn;

/* パレットの種類 */
Extern short	gPaletteCheck;
Extern short	gPrevPaletteCheck;

#pragma options align=mac68k

#pragma mark _PNGPrefsRec
/* PNG設定 */
typedef struct PNGPrefsRec
{
	short	transColor;		/* 透過色 */
	Boolean	interlaced;		/* インタレース */
	
	Boolean	useClip2gif;	/* (不使用） */
	
	short	compLevel;		/* 圧縮レベル */
	
	Boolean	optimizeImage;	/* 使用されている色数にあわせて最適化 */
	struct {
		Boolean	inSaving;
		Boolean	inLoading;
	} useTransColor;
} PNGPrefsRec;

Extern PNGPrefsRec	gPNGFilePrefs;

#pragma mark _OtherPrefs
/* その他の設定 */
typedef struct OtherPrefs
{
	Boolean	checkWhenColorChanged;	/* 色数変更時にカラーパレットに印 */
	Boolean	activateDroppedWindow;	/* ドロップされたウィンドウをアクティブに */
	Boolean maskAutoConvert;		/* マスク保存時に囲まれた部分をマスク化 */
	Boolean	continuousIDs;			/* 複数アイコン追加時に連続するIDを付加 */
	Boolean	addForeground;			/* レイヤーで、前景を追加 */
	Boolean	copyIconWithPicture;	/* ペイントウィンドウでコピー時にアイコン情報もコピー */
	Boolean	useHyperCardPalette;	/* パターンパレットでHyperCard互換パレットを使用 */
	
	Boolean	importAsFamilyIcon;		/* 'icns'として取り込む */
} OtherPrefs;

Extern OtherPrefs	gOtherPrefs;

typedef struct KeyThreshRec
{
	short	keyThresh;		/* 繰り返すまでの遅れ */
	short	keyRepThresh;	/* 繰り返しの速度 */
} KeyThreshRec;

#pragma mark _ToolPrefsRec
/* paint prefs */
typedef struct ToolPrefsRec
{
	Boolean			eraseSameColor;			/* 同じ色の場合消す */
	Boolean 		eraseSize11;			/* 1*1の場合のみ */
	Boolean 		selectionMasking;		/* 選択範囲でマスキング */
	
	KeyThreshRec	dotDrawPrefs;			/* 描画点ツールの設定 */
	
	Boolean			changeSpoitCursor;		/* スポイトツールのカーソルを変更 */
	Boolean			changePencilCursor;		/* 鉛筆ツールのカーソルを変更 */
	
	Boolean 		useColorCursor;			/* カーソルに色を使用 */
	Boolean			eraserByCmdKey;			/* コマンドキーで消しゴムツール */
	
	short			gridMode;				/* グリッド */
	Boolean			showRuler;				/* ルーラー */
	RGBColor		gridColor;				/* グリッドの色 */
	RGBColor		gridColor2;				/* 32*32 or 16*16 のグリッドの色 */
} ToolPrefsRec;

Extern KeyThreshRec	gKeyThreshStore;
Extern ToolPrefsRec gToolPrefs;

/* アプリケーションのファイル参照番号 */
Extern short	gApplRefNum;
Extern short	gPrefFileRefNum;

/* 描画点ツール用 */
Extern long		gDotLibFolderID;
Extern short	gDotLibVRefNum;
Extern Boolean	isDotLibAvailable;
Extern Str31	gDotLibName;
Extern short	gDotLibRefNum;

/* デフォルトのクリエータ */
Extern OSType	gPICTCreator;
Extern OSType	gGIFCreator;
Extern OSType	gIconCreator;
Extern OSType	gPNGCreator;

/* 起動時の処理 */
Extern short	gStartup; /* ０：なにもしない、１：新規ウィンドウ、２：オープンダイアログ */

/* プレビューアイコンフラグ */
Extern short	gPreviewIconFlag;	/* ０：追加しない、１：追加する */

/* タブレット処理関連 */
Extern Boolean	isTabletAvailable;

#pragma mark _TabletPrefsRec
/* タブレット設定 */
typedef struct TabletPrefsRec
{
	Boolean	usePressure;			/* 筆圧を使用 */
	Boolean	overrideTransparency;	/* 筆圧使用時にペンの透明度を無視 */
	Boolean	useEraser;				/* 消しゴムを使用 */
	Boolean useEraserPressure;		/* 消しゴムの筆圧を使用 */
} TabletPrefsRec;

Extern TabletPrefsRec	gTabletPrefs;

Extern RGBColor	rgbGrayColor;

#pragma mark _PaintWinPrefsRec
/* 画像サイズ */
typedef struct {
	Rect	iconSize;	/* デフォルトの大きさ */
	short	ratio;	/* デフォルトの倍率 */
	Boolean	askSize;		/* 作成時にサイズを指定 */
	Boolean	referClip;		/* サイズ指定時にクリップボードを参照 */
	short	background;		/* 作成時の背景 */
	short	colorMode;	/* 色モード */
} PaintWinPrefsRec;

Extern PaintWinPrefsRec	gPaintWinPrefs;

/* プレビューの背景 */
Extern short	gListBackground;

Extern CTabHandle	gFavoriteColors;

/* 起動回数など */
typedef struct UsedCountRec
{
	unsigned long	usedCount;	/* 使用回数 */
	unsigned long	usedTime;	/* 使用時間（秒単位） */
	
	unsigned long	newNum;
	unsigned long	openNum;
	unsigned long	saveNum;
	
	unsigned long	fillNum;
	unsigned long	lightenNum;
	unsigned long	darkenNum;
	unsigned long	antialiasNum;
	unsigned long	dotPictureNum;
	unsigned long	windNum;
	unsigned long	edgeNum;
	unsigned long	colorChangeNum;
	unsigned long	rotateNum;
	unsigned long	flipNum;
	unsigned long	replaceNum;
	
	unsigned long	undoNum;
	
	unsigned long	importNum;
	unsigned long	exportNum;
	
	unsigned long	showCountNum;
} UsedCountRec;

#pragma options align=reset

Extern UsedCountRec	gUsedCount;

Extern Boolean		gIsDialog;
Extern WindowPtr	gFrontWindow;

/* テンポラリファイルの番号 */
Extern short	gTempFileNum;

/* システム関連 */
Extern Boolean	isDragMgrPresent;
Extern Boolean	isAppearanceAvailable;
Extern Boolean	isContextualMenuAvailable;
Extern Boolean	isTSMTEAvailable;
Extern Boolean	isTSMgrAvailable;
Extern Boolean	isNavServicesAvailable;
Extern Boolean	isHelpMgrAvailable;

Extern long		gQTVersion;
Extern long		gSystemVersion;
Extern UInt32	gNavLibraryVersion;
#if TARGET_API_MAC_CARBON
Extern SInt32	gCarbonLibVersion;
#endif
Extern Boolean	isOSX;

Extern Boolean	useNavigationServices;

Extern Boolean	isClip2gifAvailable;
Extern Boolean	isResEditAvailable;
Extern Boolean	isExternalEditorAvailable;
Extern FSSpec	gExternalEditor;
Extern Boolean	gUseExternalEditor;

Extern Boolean	isIconServicesAvailable;
Extern Boolean	is32BitIconsAvailable;
Extern Boolean	isThumbnailIconsAvailable;
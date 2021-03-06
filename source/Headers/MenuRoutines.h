/* ------------------------------------------------------------ */
/*  MenuRoutines.h                                              */
/*     メニュー処理用ヘッダファイル                             */
/*                                                              */
/*                 97.1.28 - 99.5.24  naoki iimura              */
/* ------------------------------------------------------------ */

/* definitions */
#define		mApple		128
enum {
	iAbout=1,
};

#define		mFile		129
enum {
	iNew=1,
	iOpen,
	iClose,
	iSave,
	iSaveAs,
	iSaveSelection,
	iRevert,
	iOpenClipboard=9,
	iPreferences=11,
	iQuit=13,
};

#define		mEdit		130
enum {
	iUndo=1,
	iCut=3,
	iCopy,
	iPaste,
	iClear,
	iSelectAll,
	iDuplicate=9,
	iIconSize=11,
	iTypeString,
	iBackground=14,
	iUnionBackground,
	iDeleteForeground,
	iDeleteBackground,
	iForeTransparency=19,
	iHideBackground
};

#define		mEffect		131
enum {
	iRepeat=1,
	iFill=3,
	iLighten,
	iDarken,
	iAntialias,
	iDotPicture,
	iWind,
	iEdge,
	iColorMode,
	iRotate=12,
	iFlipHorizontal,
	iFlipVertical,
	iOpaque=16,
	iTransparent,
	iBlend,
	iExchange=20,
};

#define		mTool		132
enum {
	iTransp=1,
	iPenColor,
	iPenSize,
	iEraserSize,
	iGrid,
	iRuler,
	iColorPalette,
	iBlendPalette,
	iSelection,
};

#define		mTransp		133
enum {
	i0percent=1,
	i25percent,
	i50percent,
	i75percent,
};

#define		mGrid		134
enum {
	iGNone=1,
	iGGray,
	iGDot,
	iGWhite,
	iGIcon32=6,
	iGIcon16,
};

#define		mPenColor	135
enum {
	iPenLighten=1,
	iPenDarken,
	iPrevColor=4,
	iPrevBlend=5,
	iFromDesktop=7,
};

#define		mColorMode	138
enum {
	i216Color=1,
	iAppleIconColor,
	i16Color,
	i16Gray,
	iMonochrome,
	i216ColorD=7,
	iAppleIconColorD,
	i16ColorD,
	i16GrayD,
	iMonoD,
};

#define		mIcon		139
enum {
	iNewIconFile=1,
	iNewIcon,
	iAddIcon=4,
	iIconInfo,
	iImportIcon=7,
	iExportIcon,
	iPreviewBackground=10,
	iOpenWithResEdit=12,
};

#define		mDisplay	140
enum {
	iDispTitle=1,
	iDispTool,
	iDispInfo,
	iDispColor1,
	iDispColor2,
	iDispBlend,
	iDispPattern,
	iDispFavorite,
	iShowAllPalettes = 10,
};

#define		mMag		141
enum {
	im100=1,
	im200,
	im400,
	im800,
};

#define		mTransp2	142

#define		mTitlePopup	143
#define		mMag2		144

#define		mPenSize	147
#define		mEraserSize	148
#define		mWindow		149
enum {
	iMag=1,
};

#define		mColorPalette	151
#define		mSelection	152
enum {
	iAutoSelect=1,
	iCancelSelect=2,
	iHideSelection=4,
};

#define		mListBackground	154
enum {
	iBackWhite=1,
	iBackGray,
	iBackBlack,
	iBackDesktop
};

#define		mBackground	160
enum {
	iBBackground=1,
	iBUnionBack,
	iBDeleteFore,
	iBDeleteBack,
	iBForeTransparency=6,
	iBHideBack=7
};

#define		mPopForeTrans	136
#define		mForeTransparency	137
enum {
	iForeTrans100=1,
	iForeTrans75,
	iForeTrans50,
	iForeTrans25,
	iForeTrans0
};

#define		mShapePop	161

#define		mDotLibPop	162
#define		mDotCmdPop	163
#define		mDotRecPop	164
enum {
	iInputCmd=1,
	iAddCmdToLib=3
};
#define		mDotInpPop	165
enum {
	iClearCmd=1,
	iRecordCmd
};

#define		mPNGTransparentPop	166
enum {
	iTransNone=1,
	iTransWhite=3,
	iTransBlack,
	iTransTrans,
	iTransBottomRight,
	iTransCurrent,
	iTransBack
};

#define		mStartupPop	167
enum {
	iStartupNone=1,
	iStartupNewPaintWindow,
	iStartupOpenDialog
};

#define		mPaintBackPop	168
enum {
	iPaintBackWhite=1,
	iPaintBackTransparent,
	iPaintBackBackColor
};

#define		mBlendPaletteSub	169
#define		mBlendPalettePop	170
enum {
	iBlendLoad=1,
	iBlendSave,
	iBlendLock=4
};

#define		mDotLibNamePop	171
enum {
	iEditLib=1,
};

/* prototypes */
void	MenuBarInit(void);
void	HandleMenuChoice(long menuChoice);

void	HandleAppleChoice(short item);
void	HandleFileChoice(short item);
void	HandleEditChoice(short item);
void	HandleEffectChoice(short item);
void	HandleToolChoice(short item);
void	HandleBlendChoice(short item);
void	HandleGridChoice(short item);
void	HandlePenColorChoice(short item);
void	HandleColorModeChoice(short item);
void	HandleIconChoice(short item);
void	HandleDispChoice(short item);
void	HandleListBackgroundChoice(short item);
void	HandleBlendPaletteChoice(short item);
void	HandleForeTransparencyChoice(short item);
void	HandlePaletteChoice(short item);

void	About(void);

void	UpdateNewMenu(void);
void	UpdateClipMenu(void);
void	UpdateSaveMenu(void);
void	UpdatePasteMenu(void);
void	UpdateToolMenu(void);
void	UpdateEffectMenu(void);
void	UpdateUndoMenu(void);
void	UpdateSelectionMenu(void);
void	UpdateBGMenu(void);
void	UpdateIconMenu(void);
void	UpdateTransparentMenu(void);

void	SetUndoMode(short mode);

void	UpdateMenus(void);

void	LoadBlendPalette(FSSpec *theFile);

/* ------------------------------------------------------------ */
/*  Preferences.h                                               */
/*     初期設定のヘッダファイル                                 */
/*                                                              */
/*                 99.2.16 - 99.5.24  naoki iimura              */
/* ------------------------------------------------------------ */

/* definitions */
#define		itemActive		0
#define		itemInactive	255

/* prototypes */
void	LoadPrefFile(void);
void	SavePrefFile(void);

OSErr	AddDataToPrefs(void *dataPtr,long dataSize,ResType type,short id,Str255 resName);
OSErr	LoadDataFromPrefs(void *dataPtr,long dataSize,ResType type,short id);

void	OpenPrefsDialog(void);

void	ConfigureGrid(void);

/* enumerations */
/* タブ */
enum {
	kFilePrefsTab=1,
	kPaintPrefsTab,
	kToolPrefsTab,
	kPNGPrefsTab,
	kTabletPrefsTab,
	kOtherPrefsTab
};

/* ファイル */
enum {
	diStartupPrefs=1,
	diSetPICTCreator,
	diSetGIFCreator,
	diSetPNGCreator,
	diSetIconCreator,
	diPICTCreatorName,
	diGIFCreatorName,
	diPNGCreatorName,
	diIconCreatorName,
	diUseNavServices,
	diPreviewIconFlag,
};

/* ペイント */
enum {
	diImageWidth=1,
	diImageHeight,
	diImageColorMode,
	diImageRatio,
	diAskSize,
	diBackground,
	diExternalEditor,
	diUseExternalEditor,
};

/* ツール */
enum {
	diEraseSameColor=1,
	diErase11,
	diSelectionMaskingOn,
	diKeyThreshControl,
	diChangeSpoitCursor,
	diChangePencilCursor,
	diUseColorCursor,
	diEraserByCmdKey,
};

/* PNG */
enum {
	diPNGInteraced=1,
	diPNGTransColor,
	diPNGCompLevel,
	diPNGUseTransSaving,
	diPNGUseTransLoading,
	diPNGOptimizeImage,
};

/* タブレット */
enum {
	diTabletUsePressure=1,
	diTabletOverrideTransparency,
	diTabletUseEraser,
	diTabletUseEraserPressure,
};

/* その他 */
enum {
	diCheckWhenColorChanged=1,
	diActivateDroppedWindow,
	diMaskAutoConvert,
	diContinuousID,
	diAddForeground,
	diCopyIconWithPicture,
	diUseHyperCardPalette,
	diImportAsFamilyIcon,
};


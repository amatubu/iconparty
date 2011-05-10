/* ------------------------------------------------------------ */
/*  WindowRoutines.h                                            */
/*     ウィンドウ処理ルーチンのプロトタイプ                     */
/*                                                              */
/*                 2001.1.21 - 2001.1.21  naoki iimura		    */
/* ------------------------------------------------------------ */

/* prototypes */
/* 初期化 */
void	ToolWindowInit(void);

/* アップデート、クリック */
void	DoUpdate(EventRecord *theEvent);
void	ContentClick(EventRecord *theEvent);
void	DrawBlend(void);
void	ChangeRatio(WindowPtr theWindow,Point popPt);
void	UpdatePaletteCheck(void);
void	RedrawInfo(void);
void	UpdatePaintWindow(WindowPtr theWindow);
void	UpdateBackInfo(WindowPtr theWindow);
void	HiliteSelectedColor(short palNo,Boolean hilite);

void	SetDotLibMain(FSSpec *theFile);

/* グリッド */
void	DrawGrid(WindowPtr theWindow);
void	DrawGridMain(short ratio,Rect *iconSize);

/* ruler */
void	ResetRuler(void);
void	DrawRuler(Point pt);

/* scroll */
void	DoScrollPaintWindowByKeys(WindowPtr theWindow,short theChar,short modifiers);

/* resize */
void	MyZoomWindow(WindowPtr theWindow,short thePart,const EventRecord *theEvent);
void	ZoomPaintWindow(WindowPtr theWindow,short thePart);
void	ResizePaintWindow(WindowPtr theWindow,Point globPt);
void	ResizePatternPalette(void);

/* dot command */
void	UpdateRecordedCommand(Str31 command);
void	UpdateInputCommand(Str31 command);

/* update mouse cursor */
void	UpdateCursorInBlendPalette(Point localPt);
void	UpdateCursorInFavoritePalette(Point localPt,Boolean cmdDown);

/* ペイントウィンドウのマスク表示 */
void	DispPaintMask(WindowPtr theWindow);

void	ShowHidePalette(WindowPtr theWindow,Boolean visible);
void	ShowHidePalette2(short item);
void	ShowAllPalettes(void);
void	SaveWindowPos(void);

/* フローティングウィンドウ関係 */
short	GetExtWindowKind(WindowPtr theWindow);
void	SetExtWindowKind(WindowPtr theWindow,short windowKind);

/* 描画 */
void	DarkenRect(const Rect *r);

void	ChangeRatioMain(WindowPtr theWindow,short ratio);
void	SetPaintWindowSize(WindowPtr theWindow,Rect *iconSize,short ratio);
void	SetScrollBarMax(WindowPtr theWindow);

/* ポップアップメニュー */
long	PopUpMenuSelectWFontSize(MenuHandle popMenu,short top,short left,
									short firstSelectedItem,short fontSize);

/* ウィンドウの位置 */
void	StackWindowPos(Point *startPt,Point offsetPt,Rect *validRect);
void	MoveDialogToParentWindow(DialogPtr theDialog);

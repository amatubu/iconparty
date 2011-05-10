/* ------------------------------------------------------------ */
/*  PaintRoutines.h                                             */
/*     ペイント処理のヘッダファイル                             */
/*                                                              */
/*                 2001.1.21 - 2001.1.21  naoki iimura        	*/
/* ------------------------------------------------------------ */

/* ツール */
void	DoPaint(WindowPtr theWindow,Point pt,Boolean shiftDown);
void	DoDotPaint(WindowPtr theWindow,Point pt);
void	DoDotPaintMain(WindowPtr theWindow,Point pt);
void	DoErase(WindowPtr theWindow,Point pt,Boolean shiftDown);
void	DoSelect(WindowPtr theWindow,Point pt,Boolean shiftDown,Boolean optDown,Boolean cmdDown);
void	DoAutoSelect(WindowPtr theWindow);
void	DoSpoit(WindowPtr theWindow,Point pt);
void	DoBucket(WindowPtr theWindow,Point pt,Boolean cmdDown);

void	DoReplaceColor(WindowPtr theWindow,RGBColor *srcColor,Boolean srcIsTransparent,
											RGBColor *dstColor,Boolean dstIsTransparent);

/* 選択範囲 */
void	DoSelectMain(WindowPtr theWindow,RgnHandle selectRgn);
void	MoveSelection(WindowPtr theWindow,short dx,short dy,Boolean redrawFlag);
void	FixSelection(WindowPtr theWindow);
void	ExchangeSelection(WindowPtr theWindow);
void	UpdatePSRgn(PaintWinRec *eWinRec);
void	DispSelection(WindowPtr theWindow);
void	DispSelectionMain(WindowPtr theWindow,Boolean dispPaint,Boolean dispPreview);

void	DoSelectColorArea(WindowPtr theWindow,RGBColor *color,Boolean isTransparent);

/* 画像サイズ */
void	ChangeIconSize(WindowPtr theWindow);

/* バックグラウンド */
void	GoBackgroundMode(WindowPtr theWindow);
void	UnionBackground(WindowPtr theWindow);
void	DeleteBackground(WindowPtr theWindow);
void	DeleteForeground(WindowPtr theWindow);
void	ShowHideBackground(WindowPtr theWindow);
short	SetForeBlend(PaintWinRec *eWinRec);

/* アイコン補助 */
Handle	GetSelectionIconHandle(WindowPtr theWindow,short size,short depth,Boolean rectMask);

/* マスク作成 */
OSErr	CreateMask(GWorldPtr gWorld,Rect *srcRect,Rect *dstRect,MyBitMapRec *bmpRec);
OSErr	CreateMask2(GWorldPtr gWorld,GWorldPtr *maskGWorld);

/* 他 */
OSErr	GoOffPort(WindowPtr theWindow);
void	DispOffPort(WindowPtr theWindow);

void	ToRealPos(Point *pt,short ratio);
void	ToGridPos(Point *pt,short ratio);

Boolean	RealPtInMask(Point pt,GWorldPtr maskGWorld);
void	ChangeMask(WindowPtr theWindow,short mode);

OSErr	DisposeBitMap(MyBitMapRec *bmpRec);

#define	GetPaintWinRec(w)	(*(PaintWinRec **)GetExtWRefCon(w))

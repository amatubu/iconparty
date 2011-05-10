/* ------------------------------------------------------------ */
/*  EffectRoutines.h                                            */
/*     効果処理などのヘッダファイル                             */
/*                                                              */
/*                 2001.11.17 - 2001.11.17  naoki iimura       	*/
/* ------------------------------------------------------------ */

/* 設定関係 */
void	LoadPaintPrefs(void);
void	SavePaintPrefs(void);

/* 効果 */
void	DoDarken(WindowPtr theWindow);
void	DoLighten(WindowPtr theWindow);
void	DoFill(WindowPtr theWindow);
void	DoAntialias(WindowPtr theWindow);
void	DoDotPicture(WindowPtr theWindow);
void	DoWind(WindowPtr theWindow);
void	DoRotate(WindowPtr theWindow);
void	DoEdging(WindowPtr theWindow);

void	FlipVertical(WindowPtr theWindow);
void	FlipHorizontal(WindowPtr theWindow);

void	ChangeColorMode(WindowPtr theWindow,short mode,Boolean dither);

/* 文字入力 */
void	TypeString(WindowPtr theWindow);

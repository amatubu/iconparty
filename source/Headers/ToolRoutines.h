/* ------------------------------------------------------------ */
/*  ToolRoutines.h                                              */
/*     ツール処理のヘッダ                                       */
/*                                                              */
/*                 2001.1.27 - 2001.1.27  naoki iimura        	*/
/* ------------------------------------------------------------ */


/* prototypes */
void	ToolSelect(short tool);
void	DoubleClickTool(short tool);
void	ChangePenSize(void);
void	ChangeEraserSize(void);
void	ChangeMode(short item);
void	ChangeTransparency(void);
void	ChangeGrid(short item);
void	ChangeColor(RGBColor *newColor,Boolean isTransparent);
void	ChangeBackColor(RGBColor *newColor,Boolean isTransparent);
void	GetDesktopColor(void);
void	FixColor(RGBColor *c);

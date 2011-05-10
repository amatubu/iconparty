/* ------------------------------------------------------------ */
/*  EditRoutines.h                                              */
/*     編集メニュー（クリップボード）の処理のヘッダ             */
/*                                                              */
/*                 2001.1.27 - 2001.1.27  naoki iimura         	*/
/* ------------------------------------------------------------ */

/* prototypes */
void	DoCut(void);
void	DoCopy(void);
void	DoPaste(void);
void	DoDelete(Boolean optDown);
void	DoDuplicate(void);
void	DoSelectAll(void);
void	DoUndo(void);

PicHandle	GetSelectionPic(WindowPtr theWindow,Boolean getBackground);
OSErr	PutPictureIntoEditWin(PicHandle picture,WindowPtr theWindow);

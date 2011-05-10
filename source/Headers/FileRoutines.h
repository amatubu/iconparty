/* ------------------------------------------------------------ */
/*  FileRoutines.h                                              */
/*     ファイル処理ルーチンのプロトタイプ                       */
/*                                                              */
/*                 97.9.21 - 99.5.24  naoki iimura              */
/* ------------------------------------------------------------ */

#include	"Globals.h"

void	NewPaintWindow(void);
void	NewIconFile(void);
OSErr	MakeIconFile(IconListWinRec *iWinRec);
WindowPtr	MakePaintWindow(Rect *iconRect,short ratio,short colorMode);
void	OpenIcon(void);
short	ClosePaintWindow(WindowPtr theWindow,Boolean quitFlag);
short	SavePaintWindow(Boolean selectionFlag,Boolean saveAsFlag);
void	DoRevertPaintWindow(WindowPtr theWindow);
short	RevertYN(Str255 docKind,Str255 docName);
void	Quit(void);
short	SaveYN(Str255 docuKind,Str255 docuName,short dialogFlag);
void	SetEditWindowTitle(WindowPtr theWindow);

short	HandleOpenApp(void);
short	HandleOpenDoc(FSSpec *spec);
PicHandle	LoadFromPict(FSSpec *picFileSpec);
OSErr	SaveAsPictMain(PicHandle picture,const FSSpec *theFile);

void	OpenClipboard(void);


/* テンポラリファイル */
OSErr	MakeTempFile(FSSpec *theTempFile,Boolean resourceFork);

/* その他 */
Boolean EqualFile(FSSpec *spec1,FSSpec *spec2);
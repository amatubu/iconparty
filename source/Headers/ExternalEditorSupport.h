/* ------------------------------------------------------------ */
/*  ExternalEditorSupport.h                                     */
/*      外部エディタのサポートルーチンのヘッダ                  */
/*                                                              */
/*                 2001.1.27 - 2000.1.27  naoki iimura        	*/
/* ------------------------------------------------------------ */

/* includes */

/* definitions */
enum {
	kXIconOpenMode=1,
	kXIconCloseMode,
};

/* prototypes */
void	UpdateXIconList(void);
OSErr	OpenByExternalEditor(WindowPtr fWindow);
Boolean	IsOpenedByExternalEditor(WindowPtr fWindow,short iconKind,short mode);
OSErr	CloseXIcon(WindowPtr fWindow);
OSErr	DropFromExternalEditor(WindowPtr fWindow,short selectedIcon);


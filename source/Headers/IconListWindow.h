void	LoadFromIconFile(FSSpec *spec);
OSErr	SaveIconFile(WindowPtr theWindow);
OSErr	CloseIconFile(WindowPtr theWindow,Boolean quitFlag);
void	DoRevertIconFile(WindowPtr theWindow);
void	OpenSelectedIcon(WindowPtr theWindow);
WindowPtr	MakeIconWindow(short refNum);
void	ResizeIconWindow(WindowPtr theWindow,Point mousePt);
void	UpdateIconWindow(WindowPtr theWindow);
void	ClickIconWindow(WindowPtr iconWindow,Point mousePt,EventRecord *theEvent);
void	MoveSelectedIcon(WindowPtr iconWindow,short dh,short dv);

void	AddNewIcon(WindowPtr iconWindow);
void	ImportIconFromFile(WindowPtr iconWindow);
void	ExportIconToFolder(WindowPtr iconWindow);
void	CopySelectedIcon(IconListWinRec *iWinRec);
void	DeleteSelectedIcon(IconListWinRec *iWinRec);
void	SelectedIconInfo(IconListWinRec *iWinRec);
void	DuplicateSelectedIcon(WindowPtr iconWindow);
void	PasteIcon(WindowPtr iconWindow);
void	SelectAllIcons(WindowPtr iconWindow);

Boolean	IsIconSelected(IconListWinRec *iWinRec);
short	IsMultiIconSelected(IconListWinRec *iWinRec);

void	OpenWithResEdit(WindowPtr iWindow);

void	DoUndoIconList(WindowPtr iWindow);
Boolean	IDToCell(short id,Cell *theCell,ListHandle iconList);

OSErr	GetSelectedIconData(IconListWinRec *iWinRec,IconListDataRec **data);

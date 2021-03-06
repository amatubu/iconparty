void	NewIconFamilyWindow(void);

WindowPtr	MakeFamilyWindow(Point windPos,Boolean is32Exist,FSSpec *theFile,
	OSType iconType,const IPIconRec *ipIcon,const StringPtr iconName);

void	OpenFolderIcon(FSSpec *theFolderSpec,Boolean isFolder);
void	OpenXIconFile(FSSpec *theFile);
void	OpenWinIconFile(FSSpec *theFile);
OSErr	CloseFamilyWindow(WindowPtr familyWindow,Boolean saveFlag,Boolean deleteFlag);
void	UpdateFamilyWindow(WindowPtr familyWindow);
void	ClickFamilyWindow(WindowPtr familyWindow,Point mousePt,EventRecord *theEvent);
void	SelectNextIcon(IconFamilyWinRec *fWinRec,Boolean shiftDown);
void	SelectNextIcon2(IconFamilyWinRec *fWinRec,short dx,short dy);
void	SaveFamilyIcon(WindowPtr fWindow,Boolean saveToFile,Boolean saveAsFlag);
void	DoRevertIconFamilyWindow(WindowPtr theWindow);

void	UpdateCursorInFamilyWindow(WindowPtr theWindow,Point pt);

void	CopySelectedIconPicture(IconFamilyWinRec *fWinRec);
PicHandle	IconToPicture(IconSuiteRef theIconSuite,short kind);
void	DeleteSelectedIconPicture(WindowPtr fWindow);
void	PasteToSelectedIcon(WindowPtr fWindow);
void	SavePictureToIconData(PicHandle picture,IconFamilyWinRec *fWinRec,
	Boolean convertMaskFlag);

void	CreateEmptyIconData(WindowPtr fWindow);

/* ファミリ<->ペイント */
void	EditFamilyIcon(WindowPtr fWindow,short forceFlag);
void	SaveToFamily(WindowPtr eWindow);

void	DoUndoIconFamily(WindowPtr fWindow);

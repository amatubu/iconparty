/* ------------------------------------------------------------ */
/*  IconParty.h                                                 */
/*     Icon Party.cのプロトタイプ                               */
/*                                                              */
/*                 97.9.21 - 99.5.24  naoki iimura              */
/* ------------------------------------------------------------ */

/* prototypes */
void	EraseOffPort(PaintWinRec *eWinRec);
void	DoEvent(EventRecord *theEvent);
void	UpdateEditWinData(void);

void	ChangeSysSettings(void);

OSErr   GetFile(FSSpec *theSpec,long index,Boolean *type);

void	ResetDot(WindowPtr theWindow);
Boolean	HandleDotKey(short eventKind,char theChar,WindowPtr theWindow);
void	ExecuteCommand(WindowPtr theWindow,Str31 command);

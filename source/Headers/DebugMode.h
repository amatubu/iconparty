/* debugMode.h */

//#define	DEBUG_MODE

#ifdef DEBUG_MODE

void	CreateDebugFile(void);
void	WriteStrToDebugFile(Str255 str);
void	WriteNumToDebugFile(short num);
void	CloseDebugFile(void);

#else

#define	CreateDebugFile()	
#define WriteStrToDebugFile(str)	
#define WriteNumToDebugFile(num)	
#define CloseDebugFile()	

#endif

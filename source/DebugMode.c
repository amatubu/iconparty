/* ------------------------------------------------------------ */
/*  DebugMode.c                                                 */
/*     バグの原因を探るためにログを書き出す                     */
/*                                                              */
/*                 1999.11.29 - 1999.11.29  naoki iimura       	*/
/* ------------------------------------------------------------ */

#include	"debugMode.h"

#include	<MacTypes.h>
#include	<Files.h>
#include	<Processes.h>
#include	<Script.h>
#include	<TextUtils.h>

#ifdef DEBUG_MODE

/* prototypes */
void	CreateDebugFile(void);
void	WriteStrToDebugFile(Str255 str);
void	WriteNumToDebugFile(short num);
void	CloseDebugFile(void);


// #define	BACKUP_LOG

#ifndef		kDebugFileName
#define		kDebugFileName		"\pdebug.txt"
#ifdef	BACKUP_LOG
	#define		kBDebugFileName		"\p~debug.txt"
#endif
#endif

#define		kDebugFileType		'TEXT'
#define		kDebugFileCreator	'ttxt'

#define	PStrCpy(s, d)	{ BlockMove((s), (d), *(s) +1); }
#define	CatChar(c, d)	{ ((d)[*(d) +1]) = (c); *(d) += 1; }


static short	debugFileRefNum=0;

/* デバッグファイルを作成し、開く */
void CreateDebugFile(void)
{
	OSErr	err;
	FSSpec	spec;
	ProcessSerialNumber	psn;
	ProcessInfoRec		processInfo;
	
	/* アプリケーションの位置を記録 */
	err=GetCurrentProcess(&psn);
	processInfo.processInfoLength=sizeof(ProcessInfoRec);
	processInfo.processName=nil;
	processInfo.processAppSpec=&spec;
	err=GetProcessInformation(&psn,&processInfo);
	
	if (err!=noErr) return;
	
	PStrCpy(kDebugFileName,spec.name);
	
	#ifdef BACKUP_LOG
	{
		/* バックアップする */
		FSSpec	bSpec=spec;
		PStrCpy(kBDebugFileName,bSpec.name);
		
		err=FSpCreate(&bSpec,kDebugFileCreator,kDebugFileType,smSystemScript);
		
		err=FSpExchangeFiles(&spec,&bSpec);
	}
	#endif
	
	/* まず消す */
	err=FSpDelete(&spec);
	
	err=FSpCreate(&spec,kDebugFileCreator,kDebugFileType,smSystemScript);
	if (err!=noErr) return;
	
	err=FSpOpenDF(&spec,fsWrPerm,&debugFileRefNum);
	if (err!=noErr) return;
}

/* デバッグファイルに文字列を出力 */
void WriteStrToDebugFile(Str255 str)
{
	OSErr	err;
	long	count;
	
	if (debugFileRefNum<=0) return;
	
	count=*str;
	err=FSWrite(debugFileRefNum,&count,str+1);
}

/* デバッグファイルに数字を出力 */
void WriteNumToDebugFile(short num)
{
	Str255	str;
	
	NumToString(num,str);
	WriteStrToDebugFile(str);
}

/* デバッグファイルを閉じる */
void CloseDebugFile(void)
{
	OSErr	err;
	
	if (debugFileRefNum<=0) return;
	
	err=FSClose(debugFileRefNum);
}

#endif /* #ifdef DEBUG_MODE */
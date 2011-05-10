/* Winアイコン関連のファイルヘッダ */
typedef struct WinIconHeaderRec {
	short	reserved1;		/* 00 00 */
	short	reserved2;		/* 01 00 */
	short	iconDataNum;	
} WinIconHeaderRec;

typedef struct WinIconDataTypeRec {
	char	iconWidth;		/* 20 or 10 */
	char	iconHeight;		/* 20 or 10 */
	short	colorFlag;		/* 10 00:16色 or 00 00:256色 */
	long	reserved1;		/* 00 00 00 00 */
	long	dataSize;	
	long	dataOffset;	
} WinIconDataTypeRec;

typedef struct WinIconDataHeaderRec {
	long	headerLength;	/* 28 00 00 00 */
	long	iconSize;		/* 20 00 00 00 or 10 00 00 00 */
	long	iconSize2;		/* 40 00 00 00 or 20 00 00 00 */
	short	reserved1;		/* 01 00 */
	short	iconDepth;		/* 04:16色 or 08:256色 */
	long	reserved2;		/* 00 00 00 00 */
	long	dataSize;		/* カラーパレットを除いたデータサイズ（データ＋マスク） */
	long	reserved3;		/* 00 00 00 00 */
	long	reserved4;		/* 00 00 00 00 */
	long	colorNum;		/* 10 00 00 00:16色 or 00 01 00 00:256色 */
	long	reserved5;		/* 00 00 00 00 */
} WinIconDataHeaderRec;

typedef struct WinIconColorRec {
	char	b;
	char	g;
	char	r;
	char	reserved;
} WinIconColorRec;

typedef struct BMPHeaderRec {
	char	bmpCode[2];		/* 'BM' */
	long	fileSize;
	short	reserved1;		/* 00 00 */
	short	reserved2;		/* 00 00 */
	long	dataOffset;
	long	headerLength;	/* 28 00 00 00 */
	long	imageWidth;		/* 20 00 00 00:32 */
	long	imageHeight;	/* 20 00 00 00:32 */
	short	reserved3;		/* 01 00 */
	short	imageDepth;		/* 08 00:256色 */
	long	compression;	/* 00 00 00 00:圧縮なし */
	long	dataSize;		/* 00 04 00 00 */
	long	hRes;			/* 水平解像度（pixels/meter） */
	long	vRes;			/* 垂直解像度（pixels/meter） */
	long	reserved4;		/* 00 00 00 00 */
	long	reserved5;		/* 00 00 00 00 */
} BMPHeaderRec;


/* プロトタイプ */
OSErr	WinIconToIconSuite(FSSpec *theIconFile,IconSuiteRef *theIconSuite);
OSErr	MakeWinIconFromSuite(FSSpec *theWinIcon,IconSuiteRef iconSuite);


/* ------------------------------------------------------------ */
/*  TabletUtils.h                                               */
/*     タブレット処理のヘッダファイル                           */
/*                                                              */
/*                 97.12.25 - 99.5.24  naoki iimura             */
/* ------------------------------------------------------------ */

#if !TARGET_API_MAC_CARBON
#ifndef	__WACOM__
#include	"Wacom.h"
#endif
#endif

#ifdef __APPLE_CC__
#include	<Carbon/Carbon.h>
#else
#include	<DeskBus.h>
#endif

#if TARGET_API_MAC_CARBON
#define	GetTablet()		(false)
#define	IsEraser()		(false)
#define	GetPressure()	0xFFFFU;
#define	IsTabletInUse()	(false)
#else

Boolean GetTablet(void);
Boolean	IsEraser(void);
unsigned short	GetPressure(void);
Boolean	IsTabletInUse(void);

#define		bEraser		3

#endif
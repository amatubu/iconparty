/* ------------------------------------------------------------ */
/*  TabletUtils.c                                               */
/*     タブレット処理                                           */
/*                                                              */
/*                 1998.5.31 - 2000.7.2  naoki iimura    	    */
/* ------------------------------------------------------------ */

/* includes */
#include	<MacTypes.h>
#include	<Traps.h>
#include	<Patches.h>
#include	<Events.h>

#include	"TabletUtils.h"

#if !TARGET_API_MAC_CARBON

static AppleRecordPtr	gTheTabletData;

/*************** GetTablet *********************/
/*  This routine finds a tablet data structure */
/***********************************************/

Boolean GetTablet(void) 
{
	ADBDataBlock	ADBInfo;		/* info for this ADB table entry */
	ADBAddress		myADBAddrs;		/* ADB table entry we are looking at */
    short			numADBs;		/* total number of devices on ADB bus */
    short			ADBIndex;		/* index into valid ADB table entries */

	/* see if there is an ADB bus on this machine */
	if ((GetToolTrapAddress(_InitGraf) != GetToolTrapAddress(0xAA6E))) {
		if (GetToolTrapAddress(_Unimplemented) != GetOSTrapAddress(_GetADBInfo)) {
	
			/* see if there are any ADB devices */
			numADBs = CountADBs();
			if (numADBs != 0) {
	
				/* search thru ADB table entries for a keyboard */
				for (ADBIndex = 1; ADBIndex <= numADBs; ADBIndex++) {
					myADBAddrs = GetIndADB(&ADBInfo, ADBIndex);
					
					/* tablets start at address 4 */
					if ((myADBAddrs > 0) && (ADBInfo.origADBAddr == 4)) {
	
						/* check for 'TBLT' identifier */
						if (((AppleRecordPtr)ADBInfo.dbDataAreaAddr)->tabletID == 'TBLT') {
							gTheTabletData = (AppleRecordPtr)ADBInfo.dbDataAreaAddr;
							return(true);
						}
					}
				}
			}
		}
	}
	return(false);
}

/* 消しゴムかどうか */
Boolean IsEraser(void)
{
	UInt32	time;
	
	time=TickCount();
	if (time<=gTheTabletData->trans[0].timeStamp+10)
		return ((gTheTabletData->trans[0].DOFTrans & TRANSMASK) == ERASER);
	else
		return false;
}

unsigned short GetPressure(void)
{
	if (gTheTabletData->trans[0].pressure == 0) return 0xFFFFU;
	return 0xFFFFUL * gTheTabletData->trans[0].pressure / gTheTabletData->trans[0].pressLevels;
}

Boolean IsTabletInUse(void)
{
	return (gTheTabletData->trans[0].buttons > 0);
}

#endif /* TARGET_API_MAC_CARBON */
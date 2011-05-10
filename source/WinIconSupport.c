/* ------------------------------------------------------------ */
/*  WinIconSupport.c                                            */
/*     Windows�A�C�R���̃T�|�[�g                                */
/*                                                              */
/*                 2001.9.25 - 2001.9.25  naoki iimura        	*/
/* ------------------------------------------------------------ */


/* includes */
#ifdef __APPLE_CC__
#include	<Carbon/Carbon.h>
#else
#endif

#include	"Globals.h"
#include	"PreCarbonSupport.h"
#include	"WinIconSupport.h"

/* Win�A�C�R���֘A�̃��[�`�� */
static OSErr	WinIconToIconData(Ptr winIcon,WinIconDataTypeRec *dataType,Handle *iconData,Handle *maskData);
static OSErr	BMPToIconData(Ptr winIcon,BMPHeaderRec *bmpHeader,Handle *iconData);
static OSErr	ConvertIconData(Ptr winIcon,Handle *iconData,short iconDepth,short macIconDepth,Rect *iconSize,CTabHandle ctab);
static OSErr	GetWinIconCTable(Ptr cTablePtr,short depth,CTabHandle *ctab);

/* �f�[�^�i�[�̏��Ԃ��t���ɂ��� */
#define	WinToMacShort(data)	((data<<8)+(0xff &(data>>8)))
#define	WinToMacLong(data)	((data<<24)+((data & 0xff00)<<8)+((data & 0xff0000)>>8)+(0xffL &(data>>24)))

/* �Ƃ肠�����u�`.ico�v���邢�̓t�@�C���^�C�v'ICO '�̃t�@�C�����J�����炱�����Ă� */
OSErr WinIconToIconSuite(FSSpec *theIconFile,IconSuiteRef *theIconSuite)
{
	IconSuiteRef	iconSuite;
	OSErr			err=noErr;
	Ptr				winIcon;
	short			refNum;
	long			eof;
	WinIconHeaderRec	iconHeader;
	short			i;
	WinIconDataTypeRec	dataType;
	Handle			iconData,maskData,temp;
	ResType			iconType=0,maskType=0;
	BMPHeaderRec	*bmpHeader;
	
	/* �܂��t�@�C�����J�� */
	err=FSpOpenDF(theIconFile,fsRdPerm,&refNum);
	if (err!=noErr) return err;
	
	err=GetEOF(refNum,&eof);
	if (err!=noErr)
	{
		FSClose(refNum);
		return err;
	}
	
	winIcon=NewPtr(eof);
	if (winIcon==nil)
	{
		err=memFullErr;
		FSClose(refNum);
		return err;
	}
	err=FSRead(refNum,&eof,winIcon);
	if (err!=noErr)
	{
		FSClose(refNum);
		return err;
	}
	
	FSClose(refNum);
	
	err=NewIconSuite(&iconSuite);
	
	/* �w�b�_��ǂݍ��݁A�A�C�R���̐��𐔂��� */
	iconHeader=*(WinIconHeaderRec *)winIcon;
	
	/* BMP�`�������m��Ȃ��̂ł�����`�F�b�N���� */
	if (iconHeader.reserved1 == 'BM')
	{
		/* BMP */
		long	fileSize;
		
		bmpHeader=(BMPHeaderRec *)winIcon;
		fileSize=WinToMacLong(bmpHeader->fileSize);
		if (fileSize!=0x0836)
		{
			DisposeIconSuite(iconSuite,false);
			DisposePtr(winIcon);
			return -1;
		}
		
		err=BMPToIconData(winIcon+sizeof(BMPHeaderRec),bmpHeader,&iconData);
		if (err==noErr)
		{
			err=AddIconToSuite(iconData,iconSuite,kLarge8BitData);
			UseResFile(gApplRefNum);
			iconData=Get1Resource(kLarge1BitMask,2000);
			err=AddIconToSuite(iconData,iconSuite,kLarge1BitMask);
		}
		else
		{
			DisposeIconSuite(iconSuite,false);
			DisposePtr(winIcon);
			return err;
		}
	}
	else	
	{
		iconHeader.iconDataNum=WinToMacShort(iconHeader.iconDataNum);
		
		for (i=0; i<iconHeader.iconDataNum; i++)
		{
			dataType=*(WinIconDataTypeRec *)(winIcon+sizeof(WinIconHeaderRec)+i*sizeof(WinIconDataTypeRec));
			dataType.dataSize=WinToMacLong(dataType.dataSize);
			dataType.dataOffset=WinToMacLong(dataType.dataOffset);
			
			if (dataType.iconWidth != dataType.iconHeight || (dataType.iconWidth!=16 && dataType.iconWidth!=32))
				continue;
			
			maskData=nil;
			err=WinIconToIconData(winIcon+dataType.dataOffset,&dataType,&iconData,&maskData);
			if (err==noErr)
			{
				switch (dataType.iconWidth)
				{
					case 0x20:
						maskType=kLarge1BitMask;
						switch (dataType.colorFlag)
						{
							case 0x1000:
								iconType=kLarge4BitData;
								break;
							
							case 0x0000:
							case 0x0001:
								iconType=kLarge8BitData;
								break;
							
							case 0x0200:
								iconType=kLarge1BitMask;
								break;
						}
						break;
					
					case 0x10:
						maskType=kSmall1BitMask;
						switch (dataType.colorFlag)
						{
							case 0x1000:
								iconType=kSmall4BitData;
								break;
							
							case 0x0000:
							case 0x0001:
								iconType=kSmall8BitData;
								break;
							
							case 0x0200:
								iconType=kSmall1BitMask;
								break;
						}
						break;
				}
				if (dataType.colorFlag==0x0200) /* �����̎��̓}�X�N���ǉ� */
				{
					HLock(maskData);
					HandAndHand(maskData,iconData);
					HUnlock(maskData);
					err=GetIconFromSuite(&temp,iconSuite,iconType);
					if (temp==nil)
						err=AddIconToSuite(iconData,iconSuite,iconType);
					else
					{
						HLock(iconData);
						BlockMoveData(*iconData,*temp,GetHandleSize(temp));
						HUnlock(iconData);
						DisposeHandle(iconData);
					}
				}
				else
				{
					err=GetIconFromSuite(&temp,iconSuite,iconType);
					if (temp==nil)
						err=AddIconToSuite(iconData,iconSuite,iconType);
					else
					{
						HLock(iconData);
						BlockMoveData(*iconData,*temp,GetHandleSize(temp));
						HUnlock(iconData);
						DisposeHandle(iconData);
					}
					
					/* �}�X�N���Ȃ���Έꉞ�ǉ� */
					temp=nil;
					err=GetIconFromSuite(&temp,iconSuite,maskType);
					if (temp==nil)
					{
						temp=NewHandleClear(GetHandleSize(maskData));
						HLock(maskData);
						HandAndHand(maskData,temp);
						HUnlock(maskData);
						err=AddIconToSuite(temp,iconSuite,maskType);
					}
					
					if (dataType.colorFlag==0x1000) /* 16�F�̃A�C�R���̏ꍇ��256�F������Ă��� */
					{
						switch (dataType.iconWidth)
						{
							case 0x20:
								iconType=kLarge8BitData;
								break;
							
							case 0x10:
								iconType=kSmall8BitData;
								break;
						}
						
						err=GetIconFromSuite(&temp,iconSuite,iconType);
						if (temp==nil)
						{
							err=WinIconToIconData(winIcon+dataType.dataOffset,&dataType,&iconData,nil);
							err=AddIconToSuite(iconData,iconSuite,iconType);
						}
					}
				}
				DisposeHandle(maskData);
			}
			else
			{
				DisposeIconSuite(iconSuite,true);
				DisposePtr(winIcon);
				return err;
			}
		}
	}
	DisposePtr(winIcon);
	
	*theIconSuite=iconSuite;
	return noErr;
}

/* �A�C�R���f�[�^�̎��ۂ̓ǂݍ��� */
OSErr WinIconToIconData(Ptr winIcon,WinIconDataTypeRec *dataType,Handle *iconData,Handle *maskData)
{
	WinIconDataHeaderRec	iconDataHeader;
	OSErr					err=noErr;
	CTabHandle				ctab;
	Rect					iconSize;
	short					i,j;
	long					iconDataOffset,iconMaskOffset;
	short					iconDepth,macIconDepth;
	short					iconRowBytes;
	short					maskRowBytes;
	
	iconDataHeader=*(WinIconDataHeaderRec *)winIcon;
	
	/* �J���[�e�[�u���𓾂� */
	iconDepth=WinToMacShort(iconDataHeader.iconDepth);
	
	err=GetWinIconCTable(winIcon+sizeof(iconDataHeader),iconDepth,&ctab);
	if (err!=noErr) return err;
	
	/* �A�C�R���̑傫�� */
	SetRect(&iconSize,0,0,dataType->iconWidth,dataType->iconHeight);
	
	iconRowBytes=iconSize.right*iconDepth/8;
	iconDataOffset=sizeof(iconDataHeader)+(4<<iconDepth);
	iconMaskOffset=iconDataOffset+iconRowBytes*iconSize.bottom;
	
	if (maskData==nil)
		macIconDepth=8;
	else
		macIconDepth=iconDepth;
	
	err=ConvertIconData(winIcon+iconDataOffset,iconData,iconDepth,macIconDepth,&iconSize,ctab);
	if (err!=noErr)
	{
		DisposeHandle((Handle)ctab);
		return err;
	}
	
	if (maskData==nil) return noErr;
	
	/* �}�X�N�f�[�^ */
	maskRowBytes=iconSize.right/8;
	*maskData=NewHandle(iconSize.bottom*maskRowBytes);
	HLock(*maskData);
	for (i=0; i<iconSize.bottom; i++)
	{
		for (j=0; j<maskRowBytes; j++)
			*(**maskData+i*maskRowBytes+j)=~*(winIcon+iconMaskOffset+(iconSize.bottom-i-1)*maskRowBytes+j);
	}
	HUnlock(*maskData);
	
	return noErr;
}

/* BMP����A�C�R���ɂ��� */
OSErr BMPToIconData(Ptr winIcon,BMPHeaderRec *bmpHeader,Handle *iconData)
{
	short		iconDepth,macIconDepth=8;
	Rect		iconSize;
	CTabHandle	ctab;
	short		iconRowBytes;
	OSErr		err;
	long		iconDataOffset;
	
	SetRect(&iconSize,0,0,WinToMacLong(bmpHeader->imageWidth),WinToMacLong(bmpHeader->imageHeight));
	iconDepth=WinToMacShort(bmpHeader->imageDepth);
	
	err=GetWinIconCTable(winIcon,iconDepth,&ctab);
	
	iconRowBytes=iconSize.right*iconDepth/8;
	iconDataOffset=(4<<iconDepth);
	
	err=ConvertIconData(winIcon+iconDataOffset,iconData,iconDepth,macIconDepth,&iconSize,ctab);
	
	return err;
}

/* �A�C�R���f�[�^�̕ϊ� */
OSErr ConvertIconData(Ptr winIcon,Handle *iconData,short iconDepth,short macIconDepth,Rect *iconSize,CTabHandle ctab)
{
	OSErr			err=noErr;
	GWorldPtr		iconGWorld,winIconGWorld;
	PixMapHandle	iconPix,winIconPix;
	short			iconRowBytes;
	short			rowBytes;
	short			i;
	Ptr				src,dst;
	GWorldPtr		cPort;
	GDHandle		cDevice;
	
	iconRowBytes=iconSize->right*iconDepth/8;
	
	GetGWorld(&cPort,&cDevice);
	
	#if TARGET_API_MAC_CARBON
	if (iconDepth==8)
		iconDepth=8;
	else if (iconDepth==4)
		iconDepth=4;
	
	if (macIconDepth==8)
		macIconDepth=8;
	else if (macIconDepth==4)
		macIconDepth=4;
	#endif
	
	/* ����ꂽ�J���[�e�[�u�������ɃI�t�|�[�g���쐬���A�f�[�^������ */
	err=NewGWorld(&winIconGWorld,iconDepth,iconSize,ctab,nil,useTempMem);
	if (err!=noErr)
		return err;
	
	winIconPix=GetGWorldPixMap(winIconGWorld);
	SetGWorld(winIconGWorld,0);
	LockPixels(winIconPix);
	EraseRect(iconSize);
	
	rowBytes=MyGetPixRowBytes(winIconPix) & 0x3fff;
	src=winIcon;
	dst=MyGetPixBaseAddr(winIconPix);
	for (i=0; i<iconSize->bottom; i++)
	{
		BlockMoveData(src,dst,iconRowBytes);
		src+=iconRowBytes;
		dst+=rowBytes;
	}
	
	/* Mac�p�̃A�C�R���ɕϊ�����GWorld���쐬 */
	err=NewGWorld(&iconGWorld,macIconDepth,iconSize,nil,nil,useTempMem);//8
	if (err!=noErr)
	{
		SetGWorld(cPort,cDevice);
		DisposeGWorld(winIconGWorld);
		return err;
	}
	iconPix=GetGWorldPixMap(iconGWorld);
	LockPixels(iconPix);
	
	SetGWorld(iconGWorld,0);
	EraseRect(iconSize);
	CopyBits(GetPortBitMapForCopyBits(winIconGWorld),
		GetPortBitMapForCopyBits(iconGWorld),iconSize,iconSize,srcCopy+ditherCopy,nil);
	SetGWorld(cPort,cDevice);
	
	UnlockPixels(winIconPix);
	DisposeGWorld(winIconGWorld);
	DisposeHandle((Handle)ctab);
	
	/* �A�C�R���̃f�[�^�Ƃ��Ă����߂� */
	iconRowBytes=iconSize->right*macIconDepth/8;
	
	*iconData=NewHandle(iconRowBytes*iconSize->bottom);
	if (*iconData == nil)
	{
		DisposeGWorld(winIconGWorld);
		return memFullErr;
	}
	HLock(*iconData);
	rowBytes=MyGetPixRowBytes(iconPix) & 0x3fff;
	src=MyGetPixBaseAddr(iconPix);
	dst=**iconData+iconRowBytes*(iconSize->bottom-1);
	
	for (i=0; i<iconSize->bottom; i++)
	{
		BlockMoveData(src,dst,iconRowBytes);
		src+=rowBytes;
		dst-=iconRowBytes;
	}
	
	UnlockPixels(iconPix);
	HUnlock(*iconData);
	
	DisposeGWorld(iconGWorld);
	
	return noErr;
}

/* �J���[�e�[�u���f�[�^�𓾂� */
OSErr GetWinIconCTable(Ptr cTablePtr,short depth,CTabHandle *ctab)
{
	ColorSpecPtr	cSpec;
	WinIconColorRec	*color;
	short			i;
	short			colorNum=1<<depth;
	long			size=sizeof(ColorTable)+sizeof(ColorSpec)*(colorNum-1);
	
	*ctab=(CTabHandle)NewHandle(size);
	HLock((Handle)*ctab);
	(***ctab).ctSize=colorNum-1;
	(***ctab).ctSeed=UniqueID('clut');
	(***ctab).ctFlags=0;
	cSpec=&(***ctab).ctTable[0];
	color=(WinIconColorRec *)cTablePtr;
	for (i=0; i<colorNum; i++)
	{
		cSpec[i].value=i;
		cSpec[i].rgb.red=color->r<<8;
		cSpec[i].rgb.blue=color->b<<8;
		cSpec[i].rgb.green=color->g<<8;
		color++;
	}
	HUnlock((Handle)*ctab);
	
	return noErr;
}

/* �A�C�R����Windows�A�C�R���Ƃ��ď����o�� */
OSErr MakeWinIconFromSuite(FSSpec *theWinIcon,IconSuiteRef iconSuite)
{
	OSErr	err;
	short	refNum;
	WinIconHeaderRec		iconHeader;
	WinIconDataTypeRec		iconDataType;
	WinIconDataHeaderRec	iconDataHeader;
	long	offset,headerOffset,dataLength;
	short	iconNum=0;
	Handle	h;
	long	count;
	Handle	data,mask;
	short	i,j,k;
	long	temp;
	short	tempS;
	
	OSType	iconTypeList[4]={kLarge8BitData,kLarge4BitData,kSmall8BitData,kSmall4BitData};
	short	iconSizeList[4]={32,32,16,16};
	short	iconDepthList[4]={8,4,8,4};
	Boolean	dataAvailable[4]={false,false,false,false};
	
	short	iconRowBytes;
	short	maskOffset,maskRowBytes;
	Ptr		maskPtr,dataPtr;
	
	CTabHandle	ctable;
	Ptr		cpalette,cptr;
	long	filePos;
	
	/* �ۑ�����A�C�R���̐��𒲂ׂ� */
	for (i=0; i<4; i++)
	{
		err=GetIconFromSuite(&h,iconSuite,iconTypeList[i]);
		if (err==noErr && h!=nil)
		{
			dataAvailable[i]=true;
			iconNum++;
		}
		else
			dataAvailable[i]=false;
	}
	
	if (iconNum == 0) /* �ۑ�����A�C�R�����Ȃ� */
	{
		SysBeep(0);
		return -1;
	}
	
	/* �t�@�C�����쐬 */
	err=FSpCreate(theWinIcon,kIconPartyCreator,kWinIconFileType,smSystemScript);
	err=FSpOpenDF(theWinIcon,fsWrPerm,&refNum);
	
	/* �w�b�_�Ƀf�[�^�����Ă��� */
	iconHeader.reserved1=0x0000;
	iconHeader.reserved2=0x0100;
	iconHeader.iconDataNum=WinToMacShort(iconNum);
	
	/* �w�b�_�������o�� */
	count=sizeof(iconHeader);
	err=FSWrite(refNum,&count,&iconHeader);
	
	/* �_�~�[�̃f�[�^�^�C�v�������o�� */
	for (k=0; k<iconNum; k++)
	{
		count=sizeof(WinIconDataTypeRec);
		err=FSWrite(refNum,&count,&iconDataType);
	}
	
	headerOffset=sizeof(WinIconHeaderRec);
	offset=sizeof(WinIconHeaderRec)+iconNum*sizeof(WinIconDataTypeRec);
	
	/* �A�C�R�������ɏ����o�� */
	for (k=0; k<4; k++)
	{
		if (!dataAvailable[k]) continue;
		
		iconRowBytes=iconSizeList[k]*iconDepthList[k]/8;
		
		/* �f�[�^�^�C�v */
		iconDataType.iconWidth=(char)iconSizeList[k];
		iconDataType.iconHeight=(char)iconSizeList[k];
		iconDataType.colorFlag=(iconDepthList[k]==8 ? 0x0000 : 0x1000);
		iconDataType.reserved1=0;
		dataLength=0x28L+(4L<<iconDepthList[k])+(long)iconSizeList[k]*iconRowBytes+
					(long)iconSizeList[k]*iconSizeList[k]/8; /* Header+Color palette+Data+Mask */
		iconDataType.dataSize=WinToMacLong(dataLength);
		iconDataType.dataOffset=WinToMacLong(offset);
		
		err=SetFPos(refNum,fsFromStart,headerOffset);
		count=sizeof(WinIconDataTypeRec);
		err=FSWrite(refNum,&count,&iconDataType);
		
		/* �f�[�^�w�b�_ */
		iconDataHeader.headerLength=0x28000000;
		iconDataHeader.iconSize=WinToMacLong((long)iconSizeList[k]);
		iconDataHeader.iconSize2=WinToMacLong((long)iconSizeList[k]*2);
		iconDataHeader.reserved1=0x0100;
		iconDataHeader.iconDepth=WinToMacShort(iconDepthList[k]);
		iconDataHeader.reserved2=0x00000000;
		iconDataHeader.dataSize=WinToMacLong((long)iconSizeList[k]*iconRowBytes+
					(long)iconSizeList[k]*iconSizeList[k]/8);
		iconDataHeader.reserved3=0;
		iconDataHeader.reserved4=0;
		iconDataHeader.colorNum=WinToMacLong(1L<<iconDepthList[k]);
		iconDataHeader.reserved5=0;
		
		err=SetFPos(refNum,fsFromStart,offset);
		count=sizeof(iconDataHeader);
		err=FSWrite(refNum,&count,&iconDataHeader);
		
		/* �J���[�p���b�g */
		UseResFile(0);
		ctable=GetCTable(iconDepthList[k]);
		count=4*((*ctable)->ctSize+1);
		cpalette=NewPtr(count);
		cptr=cpalette;
		for (i=0; i<=(*ctable)->ctSize; i++)
		{
			*cptr++=((*ctable)->ctTable[i].rgb.blue)>>8;
			*cptr++=((*ctable)->ctTable[i].rgb.green)>>8;
			*cptr++=((*ctable)->ctTable[i].rgb.red)>>8;
			*cptr++=0;
		}
		err=FSWrite(refNum,&count,cpalette);
		DisposePtr(cpalette);
		UseResFile(gApplRefNum);
		
		/* �f�[�^ */
		err=GetIconFromSuite(&data,iconSuite,iconTypeList[k]);
		HandToHand(&data);
		HLock(data);
		err=GetIconFromSuite(&mask,iconSuite,(iconSizeList[k]==32 ? kLarge1BitMask : 'ics#'));
		HLock(mask);
		
		dataPtr=*data+(iconSizeList[k]-1)*iconRowBytes;
		maskRowBytes=iconSizeList[k]/8;
		maskOffset=iconSizeList[k]*maskRowBytes;
		maskPtr=*mask+maskOffset+(iconSizeList[k]-1)*maskRowBytes;
		
		switch (iconDepthList[k])
		{
			case 8:
				for (i=iconSizeList[k]-1; i>=0; i--)
				{
					for (j=0; j<iconSizeList[k]; j++)
					{
						if (!BitTst(maskPtr,j))
							*(dataPtr+j)=(char)0xff;
					}
					
					count=iconRowBytes;
					err=FSWrite(refNum,&count,dataPtr);
					maskPtr-=maskRowBytes;
					dataPtr-=iconRowBytes;
				}
				break;
			
			case 4:
				for (i=iconSizeList[k]-1; i>=0; i--)
				{
					for (j=0; j<iconSizeList[k]/2; j++)
					{
						if (!BitTst(maskPtr,j*2))
							*(dataPtr+j)|=0xf0;
						if (!BitTst(maskPtr,j*2+1))
							*(dataPtr+j)|=0x0f;
					}
					
					count=iconRowBytes;
					err=FSWrite(refNum,&count,dataPtr);
					maskPtr-=maskRowBytes;
					dataPtr-=iconRowBytes;
				}
				break;
		}
		HUnlock(data);
		DisposeHandle(data);
		
		maskPtr=*mask+maskOffset+(iconSizeList[k]-1)*maskRowBytes;
		
		switch (iconSizeList[k])
		{
			case 32:
				for (i=32-1; i>=0; i--)
				{
					count=4;
					temp=~*(long *)(maskPtr);
					err=FSWrite(refNum,&count,&temp);
					maskPtr-=maskRowBytes;
				}
				break;
			
			case 16:
				for (i=16-1; i>=0; i--)
				{
					count=2;
					tempS=~*(short *)(maskPtr);
					err=FSWrite(refNum,&count,&tempS);
					maskPtr-=maskRowBytes;
				}
				break;
		}
		
		HUnlock(mask);
		
		offset+=dataLength;
		headerOffset+=sizeof(WinIconDataTypeRec);
	}
	err=GetFPos(refNum,&filePos);
	err=SetEOF(refNum,filePos);
	err=FSClose(refNum);
	
	return err;
}


/* ------------------------------------------------------------ */
/*  IPIconSupport.h                                             */
/*     IconPartyアイコン内部形式のサポート用ヘッダ              */
/*                                                              */
/*                 2001.7.4 - 2001.7.4  naoki iimura        	*/
/* ------------------------------------------------------------ */


#pragma once

typedef struct {
	IconSelectorValue		selector;
	Boolean					it32;
	Boolean					t8mk;
} IPIconSelector;


/* prototypes */
OSErr		IconFamilyToIPIcon(IconFamilyHandle theIconFamily,IPIconRec *ipIcon);
OSErr		IPIconToIconFamily(const IPIconRec *ipIcon,IconFamilyHandle *iconFamily);
OSErr		IconFamilyToIPIconWithSelector(IconFamilyHandle theIconFamily,
				const IPIconSelector *ipSelector,IPIconRec *ipIcon);

OSErr		XIconToIPIcon(const FSSpec *theFile,IPIconRec *ipIcon);
OSErr		GetFileIPIcon(const FSSpec *theFile,IPIconRec *ipIcon,
							Boolean isFolder,Boolean *is32Exist,short *alertMode);

OSErr		MakeFolderWithIPIcon(const FSSpec *theFolder,const IPIconRec *ipIcon);
OSErr		MakeFileWithIPIcon(const FSSpec *theFile,const IPIconRec *ipIcon);
OSErr		MakeXIconWithIPIcon(const FSSpec *theFile,const IPIconRec *ipIcon);

OSErr		GetDataFromIPIcon(Handle *dataHandle,const IPIconRec *ipIcon,short iconKind);
OSErr		SetDataToIPIcon(Handle dataHandle,IPIconRec *ipIcon,short iconKind);
PicHandle	IPIconToPicture(const IPIconRec *ipIcon,short iconKind);

OSErr		IPIconHas32Icons(const IPIconRec *ipIcon,Boolean *is32Exist);
OSErr		IPIconHasThumbnailIcon(const IPIconRec *ipIcon,Boolean *isThumbnailExist);
Boolean		IsIPIconHasThumbnailIcon(const IPIconRec *ipIcon);

OSErr		DisposeIPIcon(const IPIconRec *ipIcon);

void		DrawIPIconPreview(const IPIconRec *ipIcon);

OSErr		IPIconToClip(const IPIconRec *ipIcon);

OSErr		Get1IPIcon(IPIconRec *ipIcon,short resID,const IPIconSelector *selector);


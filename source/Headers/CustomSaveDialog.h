/* ------------------------------------------------------------ */
/*  CustomSaveDialog.h                                          */
/*     保存ダイアログの処理                                     */
/*                                                              */
/*                 1997.1.28 - 2001.1.27  naoki iimura        	*/
/* ------------------------------------------------------------ */


#if !TARGET_API_MAC_CARBON
Boolean	StandardSaveAs(FSSpec *spec,OSType *fType,long splitNum);
void	ExportIconDialog(Str31 iconName,StandardFileReply *reply,OSType *fType);
#endif
/*
 *  UpdataCursor.h
 *  IconParty
 *
 *  Created by naoki iimura on Sun Jul 14 2002.
 *  Copyright (c) 1997-2002 naoki iimura. All rights reserved.
 *
 */

#if __APPLE_CC__
#include <Carbon/Carbon.h>
#endif

void	MakeArrowCursorRgn(void);
void	UpdateMouseCursor(Point pt);

void	SetPenCursor(short tool);
void	MySetCursor(short id);

//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 Nuke.YKT

This file is part of NBlood.

NBlood is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
//-------------------------------------------------------------------------
#pragma once
#include "common_game.h"
#include "db.h"
class CViewMap {
public:
    char bActive;
    int x, y, nZoom;
    short angle;
    char bFollowMode;
    int forward, strafe;
    int xoffset;
    fix16_t turn;
    CViewMap();
    void Init(int, int, int, short, char);
    void Draw(void);
    void Process(int nX, int nY, short nAng);
    void SetPos(int *, int*);
    void FollowMode(char);
};

extern CViewMap gViewMap;

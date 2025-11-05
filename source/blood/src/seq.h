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
#include "resource.h"

struct SEQFRAME {
    unsigned int tile           : 12;
    unsigned int transparent    : 1; // mostly translucent
    unsigned int transparent2   : 1; // less translucent
    unsigned int blockable      : 1; // blocking
    unsigned int hittable       : 1; // hitscan sensitive
    unsigned int xrepeat        : 8; //
    unsigned int yrepeat        : 8; //
    signed   int shade          : 8; //
    unsigned int pal            : 5; //
    unsigned int trigger        : 1; // callback trigger
    unsigned int smoke          : 1; // smoke view sprite
    unsigned int autoaim        : 1; // weapon auto-aim sensitive
    unsigned int pushable       : 1; // can be pushed down via action key
    unsigned int playSound      : 1; // play global sound + random(soundRange)
    unsigned int invisible      : 1; //
    unsigned int xflip          : 1; // 
    unsigned int yflip          : 1; //
    unsigned int tile2          : 4; // extends max tile number to 32767
#ifdef NOONE_EXTENSIONS
    unsigned int soundRange     : 4; // random sound range relative to global SEQ sound
    unsigned int surfaceSound   : 1; // trigger surface sound when moving / touching
    unsigned int pal2           : 2; // extends max palookup number to 128
#else
    unsigned int reserved       : 7;
#endif
};

struct Seq {
    char signature[4];
    short version;
    short nFrames; // at6
    short ticksPerFrame;
    short nSoundID;
    int flags;
    SEQFRAME frames[];
    void Preload(void);
    void Precache(void);
};

struct ACTIVE
{
    unsigned char type;
    unsigned short xindex;
};

struct SEQINST
{
    DICTNODE *hSeq;
    Seq *pSequence;
    int nSeq;
    int nCallbackID;
    short timeCount;
    unsigned char frameIndex;
    char isPlaying;
    void Update(ACTIVE *pActive);
};

inline int seqGetTile(SEQFRAME* pFrame)
{
    return pFrame->tile+(pFrame->tile2<<12);
}

inline int seqGetPal(SEQFRAME* pFrame)
{
#ifdef NOONE_EXTENSIONS
    return pFrame->pal+(pFrame->pal2<<5);
#else
    return pFrame->pal;
#endif
}

int seqRegisterClient(void(*pClient)(int, int));
void seqPrecacheId(int id);
SEQINST* GetInstance(int nType, int nXIndex);
void UnlockInstance(SEQINST *pInst);
void seqSpawn(int nSeq, int nType, int nXIndex, int nCallbackID = -1);
void seqKill(int nType, int nXIndex);
void seqKillAll(void);
int seqGetStatus(int nType, int nXIndex);
int seqGetID(int nType, int nXIndex);
void seqProcess(int nTicks);
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
#include <string.h>
#include "build.h"
#include "common_game.h"

#include "blood.h"
#include "db.h"
#include "eventq.h"
#include "globals.h"
#include "levels.h"
#include "loadsave.h"
#include "sfx.h"
#include "sound.h"
#include "seq.h"
#include "gameutil.h"
#include "actor.h"
#include "tile.h"
#include "view.h"

#define kMaxClients 256
#define kMaxSequences 1024
#define kSurfSoundBase  800

static ACTIVE activeList[kMaxSequences];
static int activeCount = 0;
static int nClients = 0;
static void(*clientCallback[kMaxClients])(int, int);

int seqRegisterClient(void(*pClient)(int, int))
{
    dassert(nClients < kMaxClients);
    clientCallback[nClients] = pClient;
    return nClients++;
}

void Seq::Preload(void)
{
    if (memcmp(signature, "SEQ\x1a", 4) != 0)
        ThrowError("Invalid sequence");
    if ((version & 0xff00) != 0x300)
        ThrowError("Obsolete sequence version");
    for (int i = 0; i < nFrames; i++)
        tilePreloadTile(seqGetTile(&frames[i]));
}

void Seq::Precache(void)
{
    if (memcmp(signature, "SEQ\x1a", 4) != 0)
        ThrowError("Invalid sequence");
    if ((version & 0xff00) != 0x300)
        ThrowError("Obsolete sequence version");
    for (int i = 0; i < nFrames; i++)
        tilePrecacheTile(seqGetTile(&frames[i]));
}

void seqPrecacheId(int id)
{
    DICTNODE *hSeq = gSysRes.Lookup(id, "SEQ");
    if (!hSeq)
        return;
    Seq *pSeq = (Seq*)gSysRes.Lock(hSeq);
    pSeq->Precache();
    gSysRes.Unlock(hSeq);
}

SEQINST siWall[kMaxXWalls];
SEQINST siCeiling[kMaxXSectors];
SEQINST siFloor[kMaxXSectors];
SEQINST siSprite[kMaxXSprites];
SEQINST siMasked[kMaxXWalls];

void UpdateSprite(int nXSprite, SEQFRAME *pFrame)
{
    dassert(nXSprite > 0 && nXSprite < kMaxXSprites);
    int nSprite = xsprite[nXSprite].reference;
    dassert(nSprite >= 0 && nSprite < kMaxSprites);
    spritetype *pSprite = &sprite[nSprite];
    dassert(pSprite->extra == nXSprite);
    
    int nScale = xsprite[nXSprite].scale; // SEQ size scaling

    if (pSprite->flags & kPhysGravity)
    {
        if (tilesiz[pSprite->picnum].y != tilesiz[seqGetTile(pFrame)].y || picanm[pSprite->picnum].yofs != picanm[seqGetTile(pFrame)].yofs
            || (pFrame->yrepeat && pFrame->yrepeat != pSprite->yrepeat))
            pSprite->flags |= kPhysFalling;
    }

    pSprite->picnum = seqGetTile(pFrame);

    if (pFrame->pal)
        pSprite->pal = seqGetPal(pFrame);

    pSprite->shade = pFrame->shade;

    if (pFrame->xrepeat)
    {
        if (nScale) pSprite->xrepeat = ClipRange(mulscale8(pFrame->xrepeat, nScale), 0, 255);
        else pSprite->xrepeat = pFrame->xrepeat;
    }

    if (pFrame->yrepeat)
    {
        if (nScale) pSprite->yrepeat = ClipRange(mulscale8(pFrame->yrepeat, nScale), 0, 255);
        else pSprite->yrepeat = pFrame->yrepeat;
    }

    if (pFrame->transparent)
        pSprite->cstat |= CSTAT_SPRITE_TRANSLUCENT;
    else
        pSprite->cstat &= ~CSTAT_SPRITE_TRANSLUCENT;

    if (pFrame->transparent2)
        pSprite->cstat |= CSTAT_SPRITE_TRANSLUCENT_INVERT;
    else
        pSprite->cstat &= ~CSTAT_SPRITE_TRANSLUCENT_INVERT;

    if (pFrame->blockable)
        pSprite->cstat |= CSTAT_SPRITE_BLOCK;
    else
        pSprite->cstat &= ~CSTAT_SPRITE_BLOCK;

    if (pFrame->hittable)
        pSprite->cstat |= CSTAT_SPRITE_BLOCK_HITSCAN;
    else
        pSprite->cstat &= ~CSTAT_SPRITE_BLOCK_HITSCAN;

    if (pFrame->invisible)
        pSprite->cstat |= CSTAT_SPRITE_INVISIBLE;
    else
        pSprite->cstat &= (unsigned short)~CSTAT_SPRITE_INVISIBLE;

    if (pFrame->pushable)
        pSprite->cstat |= CSTAT_SPRITE_RESERVED3;
    else
        pSprite->cstat &= ~CSTAT_SPRITE_RESERVED3;

    if (pFrame->smoke)
        pSprite->flags |= kHitagSmoke;
    else
        pSprite->flags &= ~kHitagSmoke;

    if (pFrame->autoaim)
        pSprite->flags |= kHitagAutoAim;
    else
        pSprite->flags &= ~kHitagAutoAim;

    if (pFrame->xflip)
        pSprite->flags |= kHitagFlipX;
    else
        pSprite->flags &= ~kHitagFlipX;

    if (pFrame->yflip)
        pSprite->flags |= kHitagFlipY;
    else
        pSprite->flags &= ~kHitagFlipY;

}

void UpdateWall(int nXWall, SEQFRAME *pFrame)
{
    dassert(nXWall > 0 && nXWall < kMaxXWalls);
    int nWall = xwall[nXWall].reference;
    dassert(nWall >= 0 && nWall < kMaxWalls);
    walltype *pWall = &wall[nWall];
    dassert(pWall->extra == nXWall);

    pWall->picnum = seqGetTile(pFrame);

    if (pFrame->pal)
        pWall->pal = seqGetPal(pFrame);

    if (pFrame->transparent)
        pWall->cstat |= CSTAT_WALL_TRANSLUCENT;
    else
        pWall->cstat &= ~CSTAT_WALL_TRANSLUCENT;

    if (pFrame->transparent2)
        pWall->cstat |= CSTAT_WALL_TRANS_FLIP;
    else
        pWall->cstat &= ~CSTAT_WALL_TRANS_FLIP;

    if (pFrame->blockable)
        pWall->cstat |= CSTAT_WALL_BLOCK;
    else
        pWall->cstat &= ~CSTAT_WALL_BLOCK;

    if (pFrame->hittable)
        pWall->cstat |= CSTAT_WALL_BLOCK_HITSCAN;
    else
        pWall->cstat &= ~CSTAT_WALL_BLOCK_HITSCAN;
}

void UpdateMasked(int nXWall, SEQFRAME *pFrame)
{
    dassert(nXWall > 0 && nXWall < kMaxXWalls);
    int nWall = xwall[nXWall].reference;
    dassert(nWall >= 0 && nWall < kMaxWalls);
    walltype *pWall = &wall[nWall];
    dassert(pWall->extra == nXWall);
    dassert(pWall->nextwall >= 0);
    walltype *pWallNext = &wall[pWall->nextwall];

    pWall->overpicnum = pWallNext->overpicnum = seqGetTile(pFrame);

    if (pFrame->pal)
        pWall->pal = pWallNext->pal = seqGetPal(pFrame);

    if (pFrame->transparent)
    {
        pWall->cstat |= CSTAT_WALL_TRANSLUCENT;
        pWallNext->cstat |= CSTAT_WALL_TRANSLUCENT;
    }
    else
    {
        pWall->cstat &= ~CSTAT_WALL_TRANSLUCENT;
        pWallNext->cstat &= ~CSTAT_WALL_TRANSLUCENT;
    }

    if (pFrame->transparent2)
    {
        pWall->cstat |= CSTAT_WALL_TRANS_FLIP;
        pWallNext->cstat |= CSTAT_WALL_TRANS_FLIP;
    }
    else
    {
        pWall->cstat &= ~CSTAT_WALL_TRANS_FLIP;
        pWallNext->cstat &= ~CSTAT_WALL_TRANS_FLIP;
    }

    if (pFrame->blockable)
    {
        pWall->cstat |= CSTAT_WALL_BLOCK;
        pWallNext->cstat |= CSTAT_WALL_BLOCK;
    }
    else
    {
        pWall->cstat &= ~CSTAT_WALL_BLOCK;
        pWallNext->cstat &= ~CSTAT_WALL_BLOCK;
    }

    if (pFrame->hittable)
    {
        pWall->cstat |= CSTAT_WALL_BLOCK_HITSCAN;
        pWallNext->cstat |= CSTAT_WALL_BLOCK_HITSCAN;
    }
    else
    {
        pWall->cstat &= ~CSTAT_WALL_BLOCK_HITSCAN;
        pWallNext->cstat &= ~CSTAT_WALL_BLOCK_HITSCAN;
    }
}

void UpdateFloor(int nXSector, SEQFRAME *pFrame)
{
    dassert(nXSector > 0 && nXSector < kMaxXSectors);
    int nSector = xsector[nXSector].reference;
    dassert(nSector >= 0 && nSector < kMaxSectors);
    sectortype *pSector = &sector[nSector];
    dassert(pSector->extra == nXSector);

    pSector->floorpicnum = seqGetTile(pFrame);

    pSector->floorshade = pFrame->shade;

    if (pFrame->pal)
        pSector->floorpal = seqGetPal(pFrame);
}

void UpdateCeiling(int nXSector, SEQFRAME *pFrame)
{
    dassert(nXSector > 0 && nXSector < kMaxXSectors);
    int nSector = xsector[nXSector].reference;
    dassert(nSector >= 0 && nSector < kMaxSectors);
    sectortype *pSector = &sector[nSector];
    dassert(pSector->extra == nXSector);

    pSector->ceilingpicnum = seqGetTile(pFrame);

    pSector->ceilingshade = pFrame->shade;

    if (pFrame->pal)
        pSector->ceilingpal = seqGetPal(pFrame);
}

void SEQINST::Update(ACTIVE *pActive)
{
    dassert(frameIndex < pSequence->nFrames);
    SEQFRAME* pFrame = &pSequence->frames[frameIndex];
    
    switch (pActive->type)
    {
    case 0:
        UpdateWall(pActive->xindex, pFrame);
        break;
    case 1:
        UpdateCeiling(pActive->xindex , pFrame);
        break;
    case 2:
        UpdateFloor(pActive->xindex, pFrame);
        break;
    case 3: 
    {
        UpdateSprite(pActive->xindex, pFrame);
        spritetype* pSpr = &sprite[xsprite[pActive->xindex].reference];
        int nSnd, nSurf;

#ifdef NOONE_EXTENSIONS
        if (pFrame->playSound)
        {
            nSnd = pSequence->nSoundID;
            
            // add random sound range feature
            if (!VanillaMode() && pFrame->soundRange > 0)
                nSnd += Random((pFrame->soundRange == 1) ? 2 : pFrame->soundRange);
            
            sfxPlay3DSound(pSpr, nSnd, -1, 0);
        }
        
        // add surfaceSound trigger feature
        if (!VanillaMode() && pFrame->surfaceSound && zvel[pSpr->index] == 0 && approxDist(xvel[pSpr->index], yvel[pSpr->index]))
        {
            if (pSpr->sectnum >= 0 && gUpperLink[pSpr->sectnum] < 0 && (nSurf = tileGetSurfType(pSpr->sectnum, 0x4000)) > 0)
            {
                DICTNODE* hRes;
                
                nSnd = kSurfSoundBase + ((nSurf << 1) + Random(2));
                if ((hRes = gSoundRes.Lookup(nSnd, "SFX")) != NULL)
                {
                    SFX* pSFX = (SFX*)gSoundRes.Load(hRes);
                    sfxPlay3DSoundCP(pSpr, nSnd, -1, 0, 0, (pSFX->relVol != 80) ? pSFX->relVol : 25);
                }
            }
        }
#else
        if (pFrame->playSound)
            sfxPlay3DSound(pSpr, pSequence->nSoundID, -1, 0);
#endif
        break;
    }
    case 4:
        UpdateMasked(pActive->xindex, pFrame);
        break;
    }
    if (pFrame->trigger && nCallbackID != -1)
        clientCallback[nCallbackID](pActive->type, pActive->xindex);
}

SEQINST * GetInstance(int nType, int nXIndex)
{
    switch (nType)
    {
    case 0:
        if (nXIndex > 0 && nXIndex < kMaxXWalls) return &siWall[nXIndex];
        break;
    case 1:
        if (nXIndex > 0 && nXIndex < kMaxXSectors) return &siCeiling[nXIndex];
        break;
    case 2:
        if (nXIndex > 0 && nXIndex < kMaxXSectors) return &siFloor[nXIndex];
        break;
    case 3:
        if (nXIndex > 0 && nXIndex < kMaxXSprites) return &siSprite[nXIndex];
        break;
    case 4:
        if (nXIndex > 0 && nXIndex < kMaxWalls) return &siMasked[nXIndex];
        break;
    }
    return NULL;
}

void UnlockInstance(SEQINST *pInst)
{
    dassert(pInst != NULL);
    dassert(pInst->hSeq != NULL);
    dassert(pInst->pSequence != NULL);
    gSysRes.Unlock(pInst->hSeq);
    pInst->hSeq = NULL;
    pInst->pSequence = NULL;
    pInst->isPlaying = 0;
}

void seqSpawn(int nSeq, int nType, int nXIndex, int nCallbackID)
{
    SEQINST *pInst = GetInstance(nType, nXIndex);
    if (!pInst) return;
    
    DICTNODE *hSeq = gSysRes.Lookup(nSeq, "SEQ");
    if (!hSeq)
        ThrowError("Missing sequence #%d", nSeq);

    int i = activeCount;
    if (pInst->isPlaying)
    {
        if (hSeq == pInst->hSeq)
            return;
        UnlockInstance(pInst);
        for (i = 0; i < activeCount; i++)
        {
            if (activeList[i].type == nType && activeList[i].xindex == nXIndex)
                break;
        }
        dassert(i < activeCount);
    }
    Seq *pSeq = (Seq*)gSysRes.Lock(hSeq);
    if (memcmp(pSeq->signature, "SEQ\x1a", 4) != 0)
        ThrowError("Invalid sequence %d", nSeq);
    if ((pSeq->version & 0xff00) != 0x300)
        ThrowError("Sequence %d is obsolete version", nSeq);
    if ((pSeq->version & 0xff) == 0x00)
    {
        for (int i = 0; i < pSeq->nFrames; i++)
        {
            pSeq->frames[i].tile2 = 0;
#ifdef NOONE_EXTENSIONS
            pSeq->frames[i].pal2 = 0;
#endif
        }
    }
    pInst->isPlaying = 1;
    pInst->hSeq = hSeq;
    pInst->pSequence = pSeq;
    pInst->nSeq = nSeq;
    pInst->nCallbackID = nCallbackID;
    pInst->timeCount = pSeq->ticksPerFrame;
    pInst->frameIndex = 0;
    if (i == activeCount)
    {
        dassert(activeCount < kMaxSequences);
        activeList[activeCount].type = nType;
        activeList[activeCount].xindex = nXIndex;
        activeCount++;
    }
    pInst->Update(&activeList[i]);
}

void seqKill(int nType, int nXIndex)
{
    SEQINST *pInst = GetInstance(nType, nXIndex);
    if (!pInst || !pInst->isPlaying)
        return;
    int i;
    for (i = 0; i < activeCount; i++)
    {
        if (activeList[i].type == nType && activeList[i].xindex == nXIndex)
            break;
    }
    dassert(i < activeCount);
    activeCount--;
    activeList[i] = activeList[activeCount];
    pInst->isPlaying = 0;
    UnlockInstance(pInst);
}

void seqKillAll(void)
{
    for (int i = 0; i < kMaxXWalls; i++)
    {
        if (siWall[i].isPlaying)
            UnlockInstance(&siWall[i]);
        if (siMasked[i].isPlaying)
            UnlockInstance(&siMasked[i]);
    }
    for (int i = 0; i < kMaxXSectors; i++)
    {
        if (siCeiling[i].isPlaying)
            UnlockInstance(&siCeiling[i]);
        if (siFloor[i].isPlaying)
            UnlockInstance(&siFloor[i]);
    }
    for (int i = 0; i < kMaxXSprites; i++)
    {
        if (siSprite[i].isPlaying)
            UnlockInstance(&siSprite[i]);
    }
    activeCount = 0;
}

int seqGetStatus(int nType, int nXIndex)
{
    SEQINST *pInst = GetInstance(nType, nXIndex);
    if (pInst && pInst->isPlaying)
        return pInst->frameIndex;
    return -1;
}

int seqGetID(int nType, int nXIndex)
{
    SEQINST *pInst = GetInstance(nType, nXIndex);
    if (pInst)
        return pInst->nSeq;
    return -1;
}

void seqProcess(int nTicks)
{
    for (int i = 0; i < activeCount; i++)
    {
        SEQINST *pInst = GetInstance(activeList[i].type, activeList[i].xindex);
        Seq *pSeq = pInst->pSequence;
        dassert(pInst->frameIndex < pSeq->nFrames);
        pInst->timeCount -= nTicks;
        while (pInst->timeCount < 0)
        {
            pInst->timeCount += pSeq->ticksPerFrame;
            pInst->frameIndex++;
            if (pInst->frameIndex == pSeq->nFrames)
            {
                if (pSeq->flags & 1)
                    pInst->frameIndex = 0;
                else
                {
                    UnlockInstance(pInst);
                    if (pSeq->flags & 2)
                    {
                        switch (activeList[i].type)
                        {
                        case 3:
                        {
                            int nXSprite = activeList[i].xindex;
                            int nSprite = xsprite[nXSprite].reference;
                            dassert(nSprite >= 0 && nSprite < kMaxSprites);
                            evKill(nSprite, 3);
                            if ((sprite[nSprite].flags & kHitagRespawn) && sprite[nSprite].inittype >= kDudeBase && sprite[nSprite].inittype < kDudeMax)
                                evPost(nSprite, 3, gGameOptions.nMonsterRespawnTime, kCallbackRespawn);
                            else if (VanillaMode() || sprite[nSprite].statnum < kMaxStatus)
                                DeleteSprite(nSprite);
                            break;
                        }
                        case 4:
                        {
                            int nXWall = activeList[i].xindex;
                            int nWall = xwall[nXWall].reference;
                            dassert(nWall >= 0 && nWall < kMaxWalls);
                            wall[nWall].cstat &= ~(8 + 16 + 32);
                            if (wall[nWall].nextwall != -1)
                                wall[wall[nWall].nextwall].cstat &= ~(8 + 16 + 32);
                            break;
                        }
                        }
                    }
                    activeList[i--] = activeList[--activeCount];
                    break;
                }
            }
            pInst->Update(&activeList[i]);
        }
    }
}

class SeqLoadSave : public LoadSave {
    virtual void Load(void);
    virtual void Save(void);
};

void SeqLoadSave::Load(void)
{
    Read(&siWall, sizeof(siWall));
    Read(&siMasked, sizeof(siMasked));
    Read(&siCeiling, sizeof(siCeiling));
    Read(&siFloor, sizeof(siFloor));
    Read(&siSprite, sizeof(siSprite));
    Read(&activeList, sizeof(activeList));
    Read(&activeCount, sizeof(activeCount));
    for (int i = 0; i < kMaxXWalls; i++)
    {
        siWall[i].hSeq = NULL;
        siMasked[i].hSeq = NULL;
        siWall[i].pSequence = NULL;
        siMasked[i].pSequence = NULL;
    }
    for (int i = 0; i < kMaxXSectors; i++)
    {
        siCeiling[i].hSeq = NULL;
        siFloor[i].hSeq = NULL;
        siCeiling[i].pSequence = NULL;
        siFloor[i].pSequence = NULL;
    }
    for (int i = 0; i < kMaxXSprites; i++)
    {
        siSprite[i].hSeq = NULL;
        siSprite[i].pSequence = NULL;
    }
    for (int i = 0; i < activeCount; i++)
    {
        SEQINST *pInst = GetInstance(activeList[i].type, activeList[i].xindex);
        if (pInst->isPlaying)
        {
            int nSeq = pInst->nSeq;
            DICTNODE *hSeq = gSysRes.Lookup(nSeq, "SEQ");
            if (!hSeq) {
                ThrowError("Missing sequence #%d", nSeq);
                continue;
            }
            Seq *pSeq = (Seq*)gSysRes.Lock(hSeq);
            if (memcmp(pSeq->signature, "SEQ\x1a", 4) != 0)
                ThrowError("Invalid sequence %d", nSeq);
            if ((pSeq->version & 0xff00) != 0x300)
                ThrowError("Sequence %d is obsolete version", nSeq);

            // Edited SEQ and loading old savegame...
            if (pInst->frameIndex >= pSeq->nFrames)
                pInst->frameIndex = pSeq->nFrames - 1;
               
            pInst->hSeq = hSeq;
            pInst->pSequence = pSeq;
        }
    }
}

void SeqLoadSave::Save(void)
{
    Write(&siWall, sizeof(siWall));
    Write(&siMasked, sizeof(siMasked));
    Write(&siCeiling, sizeof(siCeiling));
    Write(&siFloor, sizeof(siFloor));
    Write(&siSprite, sizeof(siSprite));
    Write(&activeList, sizeof(activeList));
    Write(&activeCount, sizeof(activeCount));
}

static SeqLoadSave *myLoadSave;

void SeqLoadSaveConstruct(void)
{
    myLoadSave = new SeqLoadSave();
}

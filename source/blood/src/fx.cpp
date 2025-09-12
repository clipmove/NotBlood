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

#include "build.h"
#include "common_game.h"

#include "actor.h"
#include "blood.h"
#include "callback.h"
#include "config.h"
#include "db.h"
#include "eventq.h"
#include "fx.h"
#include "gameutil.h"
#include "levels.h"
#include "seq.h"
#include "sfx.h"
#include "tile.h"
#include "trig.h"
#include "view.h"
#include "warp.h"

CFX gFX;

struct FXDATA {
    CALLBACK_ID funcID;
    char detail;
    short seq;
    short flags;
    int gravity;
    int airdrag;
    int duration;
    short picnum;
    unsigned char xrepeat;
    unsigned char yrepeat;
    short cstat;
    signed char shade;
    char pal;
};

FXDATA gFXData[] = {
    { kCallbackNone, 0, 49, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { kCallbackNone, 0, 50, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { kCallbackNone, 0, 51, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { kCallbackNone, 0, 52, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { kCallbackNone, 0, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { kCallbackNone, 0, 44, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { kCallbackNone, 0, 45, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { kCallbackNone, 0, 46, 1, -128, 8192, 0, 0, 0, 0, 0, 0, 0 },
    { kCallbackNone, 2, 6, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { kCallbackNone, 2, 42, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { kCallbackNone, 2, 43, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { kCallbackNone, 1, 48, 3, -256, 8192, 0, 0, 0, 0, 0, 0, 0 },
    { kCallbackNone, 1, 60, 3, -256, 8192, 0, 0, 0, 0, 0, 0, 0 },
    { kCallbackFXBloodBits, 2, 0, 1, 46603, 2048, 480, 2154, 40, 40, 0, -12, 0 },
    { kCallbackNone, 2, 0, 3, 46603, 5120, 480, 2269, 24, 24, 0, -128, 0 },
    { kCallbackNone, 2, 0, 3, 46603, 5120, 480, 1720, 24, 24, 0, -128, 0 },
    { kCallbackNone, 1, 0, 1, 58254, 3072, 480, 2280, 48, 48, 0, -128, 0 },
    { kCallbackNone, 1, 0, 1, 58254, 3072, 480, 3135, 48, 48, 0, -128, 0 },
    { kCallbackNone, 0, 0, 3, 58254, 1024, 480, 3261, 32, 32, 0, 0, 0 },
    { kCallbackNone, 1, 0, 3, 58254, 1024, 480, 3265, 32, 32, 0, 0, 0 },
    { kCallbackNone, 1, 0, 3, 58254, 1024, 480, 3269, 32, 32, 0, 0, 0 },
    { kCallbackNone, 1, 0, 3, 58254, 1024, 480, 3273, 32, 32, 0, 0, 0 },
    { kCallbackNone, 1, 0, 3, 58254, 1024, 480, 3277, 32, 32, 0, 0, 0 },
    { kCallbackNone, 2, 0, 1, -27962, 8192, 600, 1128, 16, 16, 514, -16, 0 }, // bubble 1
    { kCallbackNone, 2, 0, 1, -18641, 8192, 600, 1128, 12, 12, 514, -16, 0 }, // bubble 2
    { kCallbackNone, 2, 0, 1, -9320, 8192, 600, 1128, 8, 8, 514, -16, 0 }, // bubble 3
    { kCallbackNone, 2, 0, 1, -18641, 8192, 600, 1131, 32, 32, 514, -16, 0 },
    { kCallbackFXBloodBits, 2, 0, 3, 27962, 4096, 480, 733, 32, 32, 0, -16, 0 },
    { kCallbackNone, 1, 0, 3, 18641, 4096, 120, 2261, 12, 12, 0, -128, 0 },
    { kCallbackNone, 0, 47, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { kCallbackNone, 1, 0, 3, 58254, 3328, 480, 2185, 48, 48, 0, 0, 0 },
    { kCallbackNone, 0, 0, 3, 58254, 1024, 480, 2620, 48, 48, 0, 0, 0 },
    { kCallbackNone, 1, 55, 1, -13981, 5120, 0, 0, 0, 0, 0, 0, 0 },
    { kCallbackNone, 1, 56, 1, -13981, 5120, 0, 0, 0, 0, 0, 0, 0 },
    { kCallbackNone, 1, 57, 1, 0, 2048, 0, 0, 0, 0, 0, 0, 0 },
    { kCallbackNone, 1, 58, 1, 0, 2048, 0, 0, 0, 0, 0, 0, 0 },
    { kCallbackNone, 2, 0, 0, 0, 0, 960, 956, 32, 32, 610, 0, 0 },
    { kCallbackFXBouncingSleeve, 2, 62, 0, 46603, 1024, 0, 0, 0, 0, 0, 0, 0 },
    { kCallbackFXBouncingSleeve, 2, 63, 0, 46603, 1024, 0, 0, 0, 0, 0, 0, 0 },
    { kCallbackFXBouncingSleeve, 2, 64, 0, 46603, 1024, 0, 0, 0, 0, 0, 0, 0 },
    { kCallbackFXBouncingSleeve, 2, 65, 0, 46603, 1024, 0, 0, 0, 0, 0, 0, 0 },
    { kCallbackFXBouncingSleeve, 2, 66, 0, 46603, 1024, 0, 0, 0, 0, 0, 0, 0 },
    { kCallbackFXBouncingSleeve, 2, 67, 0, 46603, 1024, 0, 0, 0, 0, 0, 0, 0 },
    { kCallbackNone, 1, 0, 3, 0, 0, 0, 838, 16, 16, 80, -8, 0 },
    { kCallbackNone, 0, 0, 3, 34952, 8192, 0, 2078, 64, 64, 0, -8, 0 },
    { kCallbackNone, 0, 0, 3, 34952, 8192, 0, 1106, 64, 64, 0, -8, 0 },
    { kCallbackNone, 0, 0, 3, 58254, 3328, 480, 2406, 48, 48, 0, 0, 0 },
    { kCallbackNone, 1, 0, 3, 46603, 4096, 480, 3511, 64, 64, 0, -128, 0 },
    { kCallbackNone, 0, 8, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { kCallbackNone, 2, 11, 3, -256, 8192, 0, 0, 0, 0, 0, 0, 0 },
    { kCallbackNone, 2, 11, 3, 0, 8192, 0, 0, 0, 0, 0, 0, 0 },
    { kCallbackNone, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { kCallbackNone, 1, 30, 3, 0, 0, 0, 0, 40, 40, 80, -8, 0 },
    { kCallbackFXPodBloodSplat, 2, 0, 3, 27962, 4096, 480, 4023, 32, 32, 0, -16, 0 },
    { kCallbackFXPodBloodSplat, 2, 0, 3, 27962, 4096, 480, 4028, 32, 32, 0, -16, 0 },
    { kCallbackNone, 2, 0, 0, 0, 0, 480, 926, 32, 32, 610, -12, 0 },
    { kCallbackNone, 1, 70, 1, -13981, 5120, 0, 0, 0, 0, 0, 0, 0 }
};

void CFX::fxKill(int nSprite)
{
    if (nSprite < 0 || nSprite >= kMaxSprites)
        return;
    evKill(nSprite, 3);
    if (sprite[nSprite].extra > 0)
        seqKill(3, sprite[nSprite].extra);
    DeleteSprite(nSprite);
}

void CFX::fxFree(int nSprite)
{
    if (nSprite < 0 || nSprite >= kMaxSprites)
        return;
    spritetype *pSprite = &sprite[nSprite];
    if (pSprite->extra > 0)
        seqKill(3, pSprite->extra);
    if (pSprite->statnum != kStatFree)
        actPostSprite(nSprite, kStatFree);
}

spritetype * CFX::fxSpawn(FX_ID nFx, int nSector, int x, int y, int z, unsigned int duration)
{
    if (nSector < 0 || nSector >= numsectors)
        return NULL;
    int nSector2 = nSector;
    if (!FindSector(x, y, z, &nSector2))
        return NULL;
#if 0
    if (gbAdultContent && gGameOptions.nGameType == kGameTypeSinglePlayer)
    {
        switch (nFx)
        {
        case FX_0:
        case FX_1:
        case FX_2:
        case FX_3:
        case FX_13:
        case FX_34:
        case FX_35:
        case FX_36:
            return NULL;
        default:
            break;
        }
    }
#endif
    if (nFx < 0 || nFx >= kFXMax)
        return NULL;
    FXDATA *pFX = &gFXData[nFx];

    int kFxMax = 512;
    if (gGameOptions.bGoreBehavior && !VanillaMode())
    {
        kFxMax = 4096;
        if (!duration) // no override duration given, load from global fx data struct
            duration = pFX->duration;
        switch (nFx)
        {
        case FX_0:
        case FX_1:
        case FX_2:
        case FX_3:
        case FX_13: // blood chunk
        case FX_34:
            duration *= 20;
            break;
        case FX_35:
            duration *= 10;
            break;
        case FX_36: // blood splat
            duration *= 200;
            break;
        case FX_39: // bullet casing
            duration *= 2;
            break;
        case FX_40: // shell casing
            duration *= 5;
            break;
        default:
            break;
        }
    }
    if (gStatCount[kStatFX] == kFxMax) // hit upper limit of available sprites, don't spawn any more
    {
        int nSprite = headspritestat[kStatFX];
        while ((sprite[nSprite].flags & 32) && nSprite != -1) // scan through sprites for free slot
            nSprite = nextspritestat[nSprite];
        if (nSprite == -1)
            return NULL;
        fxKill(nSprite);
    }
    spritetype *pSprite = actSpawnSprite(nSector, x, y, z, 1, 0);
    pSprite->type = nFx;
    pSprite->picnum = pFX->picnum;
    pSprite->cstat |= pFX->cstat;
    pSprite->shade = pFX->shade;
    pSprite->pal = pFX->pal;
    qsprite_filler[pSprite->index] = pFX->detail;
    if (pFX->xrepeat > 0)
        pSprite->xrepeat = pFX->xrepeat;
    if (pFX->yrepeat > 0)
        pSprite->yrepeat = pFX->yrepeat;
    if ((pFX->flags & 1) && Chance(0x8000))
        pSprite->cstat ^= 4;
    if ((pFX->flags & 2) && Chance(0x8000))
        pSprite->cstat ^= 8;
    if (pFX->seq)
    {
        int nXSprite = dbInsertXSprite(pSprite->index);
        seqSpawn(pFX->seq, 3, nXSprite, -1);
    }
    if (duration == 0) // no override duration set, load from global fx data struct
        duration = pFX->duration;
    if (duration)
        evPost((int)pSprite->index, 3, duration+Random2(duration>>1), kCallbackRemove);
    return pSprite;
}

inline void fxCallback(int nSprite, FXDATA *pFXData)
{
    if (!pFXData)
        return;
    if (pFXData->funcID < 0 || pFXData->funcID >= kCallbackMax)
    {
        gFX.fxFree(nSprite);
        return;
    }
    dassert(gCallback[pFXData->funcID] != NULL);
    gCallback[pFXData->funcID](nSprite);
}

void CFX::fxProcess(void)
{
    for (int nSprite = headspritestat[kStatFX]; nSprite >= 0; nSprite = nextspritestat[nSprite])
    {
        spritetype *pSprite = &sprite[nSprite];
        if (pSprite->statnum == kStatFree) // skip free'd fx sprite
            continue;
        viewBackupSpriteLoc(nSprite, pSprite);
        short nSector = pSprite->sectnum;
        dassert(nSector >= 0 && nSector < kMaxSectors);
        dassert(pSprite->type < kFXMax);
        FXDATA *pFXData = &gFXData[pSprite->type];
        vec3_t oldPos = pSprite->xyz;
        int nGravity = pFXData->gravity;
        int nAirDrag = pFXData->airdrag;
        if ((pSprite->type == FX_27) && !IsUnderwaterSector(nSector) && gGameOptions.bGoreBehavior && !VanillaMode())
        {
            nGravity = 80000; // make blood heavier
            nAirDrag >>= 1; // make blood drag 
        }
        actAirDrag(pSprite, nAirDrag);
        if (xvel[nSprite])
            pSprite->x += xvel[nSprite]>>12;
        if (yvel[nSprite])
            pSprite->y += yvel[nSprite]>>12;
        if (zvel[nSprite])
            pSprite->z += zvel[nSprite]>>8;
        const bool bCasingType = (pSprite->type >= FX_37) && (pSprite->type <= FX_42);
        if (bCasingType && IsUnderwaterSector(nSector) && gGameOptions.bSectorBehavior && !VanillaMode()) // lower gravity by 75% underwater (only for bullet casings)
            nGravity >>= 2;
#ifdef NOONE_EXTENSIONS
        if (bCasingType && gGameOptions.bSectorBehavior && !gModernMap && !VanillaMode()) // check if new xy position is within a wall
#else
        if (bCasingType && gGameOptions.bSectorBehavior && !VanillaMode()) // check if new xy position is within a wall
#endif
        {
            if (!cansee(oldPos.x, oldPos.y, oldPos.z, nSector, pSprite->x, pSprite->y, oldPos.z, nSector)) // if new position has clipped into wall, invert velocity and continue
            {
                xvel[nSprite] = -((xvel[nSprite]>>1)+(xvel[nSprite]>>2)); // lower velocity by 75% and invert
                yvel[nSprite] = -((yvel[nSprite]>>1)+(yvel[nSprite]>>2));
                pSprite->x = oldPos.x + (xvel[nSprite]>>12);
                pSprite->y = oldPos.y + (yvel[nSprite]>>12);
            }
        }
        else if ((pSprite->type == FX_27 || pSprite->type == FX_13) && gGameOptions.bGoreBehavior && !VanillaMode()) // check if new xy position is within a wall
        {
            if ((xvel[nSprite] || yvel[nSprite]) && !cansee(oldPos.x, oldPos.y, oldPos.z, nSector, pSprite->x, pSprite->y, oldPos.z, nSector)) // if new position has clipped into wall, freeze xy position
            {
                pSprite->x = oldPos.x;
                pSprite->y = oldPos.y;
            }
        }
        // Weird...
        if (xvel[nSprite] || (yvel[nSprite] && pSprite->z >= sector[pSprite->sectnum].floorz))
        {
            updatesector(pSprite->x, pSprite->y, &nSector);
            if (nSector == -1)
            {
                fxFree(nSprite);
                continue;
            }
            if (getflorzofslope(pSprite->sectnum, pSprite->x, pSprite->y) <= pSprite->z)
            {
                if (gGameOptions.bSectorBehavior && !VanillaMode())
                {
                    int nSectnum = nSector;
                    oldPos = pSprite->xyz;
                    const bool bOpenAirROR = (sector[pSprite->sectnum].floorpicnum >= 4080) && (sector[pSprite->sectnum].floorpicnum <= 4095);
                    if (bCasingType || bOpenAirROR) // if casing type/current sector is open air, find next floor
                    {
                        if (CheckLink(&pSprite->x, &pSprite->y, &pSprite->z, &nSectnum)) // if found ror floor underneath
                        {
                            ChangeSpriteSect(nSprite, nSectnum);
                            viewCorrectSpriteInterpolateOffsets(nSprite, pSprite, &oldPos); // if sprite is set to be interpolated, update previous position
                            sfxUpdateSpritePos(pSprite, (vec3_t const*)&oldPos); // update any assigned sfx to new sprite position
                            continue;
                        }
                        else if (bOpenAirROR) // something went terribly wrong, free sprite
                        {
                            fxFree(nSprite);
                            continue;
                        }
                    }
                }
                fxCallback(nSprite, pFXData);
                continue;
            }
            if (nSector != pSprite->sectnum)
            {
                dassert(nSector >= 0 && nSector < kMaxSectors);
                ChangeSpriteSect(nSprite, nSector);
            }
        }
        if (xvel[nSprite] || yvel[nSprite] || zvel[nSprite])
        {
            int32_t floorZ, ceilZ;
            getzsofslope(nSector, pSprite->x, pSprite->y, &ceilZ, &floorZ);
            if (((pSprite->type == FX_27 && !(pSprite->index&7)) || (pSprite->type == FX_13 && zvel[nSprite] > -1250000)) && (ceilZ > pSprite->z) && !(sector[nSector].ceilingstat&1) && !IsUnderwaterSector(nSector) && gGameOptions.bGoreBehavior && !VanillaMode()) // make blood gibs stick to ceiling
            {
                pSprite->z = ceilZ;
                if (pSprite->type == FX_13)
                {
                    if (zvel[nSprite] < 0)
                        zvel[nSprite] = -(zvel[nSprite]>>1);
                }
                else
                {
                    nGravity >>= zvel[nSprite] < -1500000 ? 1 : 2;
                    xvel[nSprite] >>= 2;
                    yvel[nSprite] >>= 2;
                }
            }
            if (ceilZ > pSprite->z && !(sector[nSector].ceilingstat&1))
            {
                fxFree(nSprite);
                continue;
            }
            if (floorZ < pSprite->z)
            {
                fxCallback(nSprite, pFXData);
                continue;
            }
        }
        zvel[nSprite] += nGravity;
    }
}

void fxSpawnBlood(spritetype *pSprite, int a2)
{
    UNREFERENCED_PARAMETER(a2);
    if (pSprite->sectnum < 0 || pSprite->sectnum >= numsectors)
        return;
    int nSector = pSprite->sectnum;
    if (!FindSector(pSprite->x, pSprite->y, pSprite->z, &nSector))
        return;
#if 0
    if (gbAdultContent && gGameOptions.nGameType == kGameTypeSinglePlayer)
        return;
#endif
    spritetype *pBlood = gFX.fxSpawn(FX_27, pSprite->sectnum, pSprite->x, pSprite->y, pSprite->z);
    if (pBlood)
    {
        pBlood->ang = 1024;
        if (IsUnderwaterSector(pSprite->sectnum) && gGameOptions.bGoreBehavior && !VanillaMode())
        {
            // make bloodspray more dense underwater
            // scale velocities (bigger nr is bigger spread)                
            const int kScale = 80;
            xvel[pBlood->index] = mulscale8(Random2(0x6aaaa), kScale);
            yvel[pBlood->index] = mulscale8(Random2(0x6aaaa), kScale);
            zvel[pBlood->index] = mulscale8(-((int)Random(0x10aaaa)) - 100, kScale);
        }
        else
        {
            xvel[pBlood->index] = Random2(0x6aaaa);
            yvel[pBlood->index] = Random2(0x6aaaa);
            zvel[pBlood->index] = -((int)Random(0x10aaaa)) - 100;
        }
        evPost(pBlood->index, 3, 8, kCallbackFXBloodSpurt);
    }
}

void fxSpawnPodBlood(spritetype *pSprite, int a2)
{
    UNREFERENCED_PARAMETER(a2);
    if (pSprite->sectnum < 0 || pSprite->sectnum >= numsectors)
        return;
    int nSector = pSprite->sectnum;
    if (!FindSector(pSprite->x, pSprite->y, pSprite->z, &nSector))
        return;
#if 0
    if (gbAdultContent && gGameOptions.nGameType == kGameTypeSinglePlayer)
        return;
#endif
    spritetype *pSpawn;
    if (pSprite->type == kDudePodGreen)
        pSpawn = gFX.fxSpawn(FX_53, pSprite->sectnum, pSprite->x, pSprite->y, pSprite->z);
    else
        pSpawn = gFX.fxSpawn(FX_54, pSprite->sectnum, pSprite->x, pSprite->y, pSprite->z);
    if (pSpawn)
    {
        pSpawn->ang = 1024;
        xvel[pSpawn->index] = Random2(0x6aaaa);
        yvel[pSpawn->index] = Random2(0x6aaaa);
        zvel[pSpawn->index] = -((int)Random(0x10aaaa))-100;
        evPost(pSpawn->index, 3, 8, kCallbackFXPodBloodSpray);
    }
}

void fxSpawnEjectingBrass(spritetype *pSprite, int z, int a3, int a4)
{
    int x = pSprite->x+mulscale28(pSprite->clipdist-4, Cos(pSprite->ang));
    int y = pSprite->y+mulscale28(pSprite->clipdist-4, Sin(pSprite->ang));
    int nSector = pSprite->sectnum;
    x += mulscale30(a3, Cos(pSprite->ang+512));
    y += mulscale30(a3, Sin(pSprite->ang+512));
    if (gGameOptions.bSectorBehavior && !VanillaMode()) // fix weird edge case when spawning casings over ror
    {
        int cX = x, cY = y, cZ = z, nSector2 = nSector;
        if (CheckLink(&cX, &cY, &cZ, &nSector2)) // if casing spawn position is overlapping into ror sector, move origin to ror sector
            x = cX, y = cY, z = cZ, nSector = nSector2;
    }
    spritetype *pBrass = gFX.fxSpawn((FX_ID)(FX_37+Random(3)), nSector, x, y, z);
    if (pBrass)
    {
        if (!VanillaMode())
        {
            pBrass->ang = Random(2047);
            if (EnemiesNotBlood() && !IsPlayerSprite(pSprite)) // used to flag if shells should be removed when touching grass (like you 'ought to, nerd)
                pBrass->cstat |= kPhysFalling;
        }
        int nDist = (a4<<18)/120+Random2(((a4/4)<<18)/120);
        int nAngle = pSprite->ang+Random2(56)+512;
        xvel[pBrass->index] = mulscale30(nDist, Cos(nAngle));
        yvel[pBrass->index] = mulscale30(nDist, Sin(nAngle));
        zvel[pBrass->index] = zvel[pSprite->index]-(0x20000+(Random2(40)<<18)/120);
        if (gGameOptions.bSectorBehavior && !VanillaMode()) // set casing velocity to inherit spawner's velocity
        {
            xvel[pBrass->index] += xvel[pSprite->index]>>1;
            yvel[pBrass->index] += yvel[pSprite->index]>>1;
        }
    }
}

void fxSpawnEjectingShell(spritetype *pSprite, int z, int a3, int a4)
{
    int x = pSprite->x+mulscale28(pSprite->clipdist-4, Cos(pSprite->ang));
    int y = pSprite->y+mulscale28(pSprite->clipdist-4, Sin(pSprite->ang));
    int nSector = pSprite->sectnum;
    x += mulscale30(a3, Cos(pSprite->ang+512));
    y += mulscale30(a3, Sin(pSprite->ang+512));
    if (gGameOptions.bSectorBehavior && !VanillaMode()) // fix weird edge case when spawning casings over ror
    {
        int cX = x, cY = y, cZ = z, nSector2 = nSector;
        if (CheckLink(&cX, &cY, &cZ, &nSector2)) // if casing spawn position is overlapping into ror sector, move origin to ror sector
            x = cX, y = cY, z = cZ, nSector = nSector2;
    }
    spritetype *pShell = gFX.fxSpawn((FX_ID)(FX_40+Random(3)), nSector, x, y, z);
    if (pShell)
    {
        if (!VanillaMode())
        {
            pShell->ang = Random(2047);
            if (EnemiesNotBlood() && !IsPlayerSprite(pSprite)) // used to flag if shells should be removed when touching grass (like you 'ought to, nerd)
                pShell->cstat |= kPhysFalling;
        }
        int nDist = (a4<<18)/120+Random2(((a4/4)<<18)/120);
        int nAngle = pSprite->ang+Random2(56)+512;
        xvel[pShell->index] = mulscale30(nDist, Cos(nAngle));
        yvel[pShell->index] = mulscale30(nDist, Sin(nAngle));
        zvel[pShell->index] = zvel[pSprite->index]-(0x20000+(Random2(20)<<18)/120);
        if (gGameOptions.bSectorBehavior && !VanillaMode()) // set casing velocity to inherit spawner's velocity
        {
            xvel[pShell->index] += xvel[pSprite->index]>>1;
            yvel[pShell->index] += yvel[pSprite->index]>>1;
        }
    }
}

void fxPrecache(void)
{
    for (int i = 0; i < kFXMax; i++)
    {
        tilePrecacheTile(gFXData[i].picnum, 0);
        if (gFXData[i].seq)
            seqPrecacheId(gFXData[i].seq);
    }
}

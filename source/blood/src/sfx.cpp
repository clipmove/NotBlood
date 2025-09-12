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
#include "compat.h"
#include "common_game.h"
#include "fx_man.h"

#include "config.h"
#include "gameutil.h"
#include "player.h"
#include "resource.h"
#include "sfx.h"
#include "sound.h"
#include "trig.h"

#define kEarDist (int)((32<<4) * 0.17) // distance between ears (17cm)

static POINT2D earL, earR, earL0, earR0; // Ear position
static VECTOR2D earVL, earVR; // Ear velocity
static int lPhase, rPhase, lVol, rVol, lPitch, rPitch;

int gSoundSpeed = 343; // speed of sound is 343m/s
static int oldSoundSpeed = gSoundSpeed;
static int nSoundSpeed = 5853;

int gSoundEarAng = 15; // angle for ear focus
static int oldEarAng = gSoundEarAng;
static int nEarAng = kAng15;

int gSoundOcclusion = 0; // adjust 3D sound sources volume if they don't have clear line of sight to player

BONKLE Bonkle[256];
BONKLE *BonkleCache[256];

int nBonkles;

DMGFEEDBACK gSoundDingSprite[4];
const DMGFEEDBACK kSoundDingSpriteInit = {-1, 0, 0};

int gSoundDing = 0;
int gSoundDingVol = 75;
int gSoundDingMinPitch = 22050;
int gSoundDingMaxPitch = 22050;
int gSoundDingKill = 0;
int gSoundDingKillVol = 255;
int gSoundDingKillPitch = 22050;

void sfxInit(void)
{
    for (int i = 0; i < 256; i++)
        BonkleCache[i] = &Bonkle[i];
    nBonkles = 0;

    for (int i = 0; i < 4; i++)
        gSoundDingSprite[i] = kSoundDingSpriteInit;
}

void sfxTerm()
{
}

int Vol3d(int angle, int dist)
{
    return dist - mulscale16(dist, 0x2000 - mulscale30(0x2000, Cos(angle)));
}

inline int CalcYOffset(spritetype *pSprite)
{
    int z = pSprite->z;
    const int nPicnum = pSprite->picnum;
    const int nSizY = tilesiz[nPicnum].y;
    if (nSizY == 0)
        return z;

    int height = (nSizY*pSprite->yrepeat)<<2;
    if (pSprite->cstat&CSTAT_SPRITE_YCENTER)
        z += height / 2;
    const int nOffset = picanm[nPicnum].yofs;
    if (nOffset)
        z -= (nOffset*pSprite->yrepeat)<<2;
    return pSprite->z - (z-(height>>1));
}

char Calc3DSectOffset(spritetype *pLink, int *srcx, int *srcy, int *srcz, int *pCanSeeSect, const int dstsect)
{
    const int nLink = pLink->owner;
    if (!spriRangeIsFine(nLink)) // if invalid link
        return 0;
    const spritetype *pOtherLink = &sprite[nLink];

    const int linksect = pLink->sectnum;
    if (!sectRangeIsFine(linksect) || !sectRangeIsFine(pOtherLink->sectnum)) // if invalid sector
        return 0;
    if (IsUnderwaterSector(linksect)) // if other link is underwater
        return 0;
    if (!AreSectorsNeighbors(pOtherLink->sectnum, dstsect, 4, true)) // if linked sector is not parallel to destination sector, abort
        return 0;

    *srcx += pOtherLink->x-pLink->x;
    *srcy += pOtherLink->y-pLink->y;
    *srcz += pOtherLink->z-pLink->z;
    *pCanSeeSect = pOtherLink->sectnum;
    return 1;
}

inline void Calc3DSects(int *srcx, int *srcy, int *srcz, const int srcsect, const int dstsect, int *pCanSeeSect)
{
    if (srcsect == dstsect) // if source and listener are in same sector
        return;
    if (!sectRangeIsFine(srcsect) || !sectRangeIsFine(dstsect))
        return;
    if (IsUnderwaterSector(srcsect) || IsUnderwaterSector(dstsect)) // if either sectors are underwater, don't calculate ror offset
        return;

    const int nUpper = gUpperLink[srcsect], nLower = gLowerLink[srcsect];
    if ((nUpper >= 0) && (sector[sprite[nUpper].sectnum].floorpicnum >= 4080) && (sector[sprite[nUpper].sectnum].floorpicnum <= 4095)) // sector has a ror upper link
    {
        if (Calc3DSectOffset(&sprite[nUpper], srcx, srcy, srcz, pCanSeeSect, dstsect))
            return;
    }
    if ((nLower >= 0) && (sector[sprite[nLower].sectnum].ceilingpicnum >= 4080) && (sector[sprite[nLower].sectnum].ceilingpicnum <= 4095)) // sector has a ror lower link
    {
        if (Calc3DSectOffset(&sprite[nLower], srcx, srcy, srcz, pCanSeeSect, dstsect))
            return;
    }
    return;
}

inline void Calc3DOcclude(const BONKLE *pBonkle, int *nDist, const int posX, const int posY, const int posZ, int nCanSeeSect)
{
    if ((pBonkle->nType >= kGenSound) && (pBonkle->nType <= kSoundPlayer)) // don't occlude these types
        return;
    if (!sectRangeIsFine(nCanSeeSect))
        nCanSeeSect = pBonkle->sectnum;

    const char bIsExplosion = (pBonkle->sfxId >= 303) && (pBonkle->sfxId <= 307);
    int bCanSeeZ = posZ-pBonkle->zOff;
    const int fz = getflorzofslope(nCanSeeSect, posX, posY);
    if (fz <= bCanSeeZ)
    {
        bCanSeeZ = fz-pBonkle->zOff;
    }
    else
    {
        const int cz = getceilzofslope(nCanSeeSect, posX, posY);
        if (cz >= bCanSeeZ)
            bCanSeeZ = cz-(-pBonkle->zOff);
    }

    if (!cansee(gMe->pSprite->x, gMe->pSprite->y, gMe->zView, gMe->pSprite->sectnum, posX, posY, bCanSeeZ, nCanSeeSect))
        *nDist = bIsExplosion ? *nDist+(*nDist>>1)-(*nDist>>2) : *nDist<<1;
}

void Calc3DValues(BONKLE *pBonkle)
{
    int nCanSeeSect = -1;
    int posX = pBonkle->curPos.x;
    int posY = pBonkle->curPos.y;
    int posZ = pBonkle->curPos.z;
    if (!VanillaMode()) // check if sound source is occurring in a linked sector (room over room)
        Calc3DSects(&posX, &posY, &posZ, pBonkle->sectnum, gMe->pSprite->sectnum, &nCanSeeSect);
    const int dx = posX - gMe->pSprite->x;
    const int dy = posY - gMe->pSprite->y;
    const int dz = posZ - gMe->pSprite->z;
    const int angle = getangle(dx, dy);

    const int distanceL = approxDist(pBonkle->curPos.x - earL.x, pBonkle->curPos.y - earL.y);
    const int distanceR = approxDist(pBonkle->curPos.x - earR.x, pBonkle->curPos.y - earR.y);
    const int phaseLeft = mulscale16r(distanceL, pBonkle->format == 1 ? 4114 : 8228);
    const int phaseRight = mulscale16r(distanceR, pBonkle->format == 1 ? 4114 : 8228);
    const int phaseMin = ClipHigh(phaseLeft, phaseRight);
    lPhase = phaseRight - phaseMin;
    rPhase = phaseLeft - phaseMin;

    int distance3D = approxDist3D(dx, dy, dz);
    if (gSoundOcclusion)
        Calc3DOcclude(pBonkle, &distance3D, posX, posY, posZ, nCanSeeSect);
    distance3D = ClipLow((distance3D >> 2) + (distance3D >> 3), 64);
    const int nVol = scale(pBonkle->vol, 80, distance3D);
    const int nEarAngle = gStereo ? nEarAng : kAng15;
    lVol = Vol3d(angle - (gMe->pSprite->ang - nEarAngle), nVol);
    rVol = Vol3d(angle - (gMe->pSprite->ang + nEarAngle), nVol);

    if (MIRRORMODE & 1)
    {
        int nTemp;
        nTemp = lPhase;
        lPhase = rPhase;
        rPhase = nTemp;

        nTemp = lVol;
        lVol = rVol;
        rVol = nTemp;
    }

    if (!DopplerToggle)
    {
        lPitch = rPitch = ClipRange(pBonkle->pitch, 1000, 50000);
        return;
    }
    const int sinVal = Sin(angle);
    const int cosVal = Cos(angle);
    const int nPitch = dmulscale30r(cosVal, pBonkle->curPos.x - pBonkle->oldPos.x, sinVal, pBonkle->curPos.y - pBonkle->oldPos.y) + nSoundSpeed;
    if (nPitch == 0) // don't allow div by zero
    {
        lPitch = rPitch = ClipRange(pBonkle->pitch, 1000, 50000);
        return;
    }

    lPitch = scale(pBonkle->pitch, dmulscale30r(cosVal, earVL.dx, sinVal, earVL.dy) + nSoundSpeed, nPitch);
    rPitch = scale(pBonkle->pitch, dmulscale30r(cosVal, earVR.dx, sinVal, earVR.dy) + nSoundSpeed, nPitch);
    lPitch = ClipRange(lPitch, 1000, 50000);
    rPitch = ClipRange(rPitch, 1000, 50000);
    if (MIRRORMODE & 1)
    {
        int nTemp;
        nTemp = lPitch;
        lPitch = rPitch;
        rPitch = nTemp;
    }
}

void sfxPlay3DSound(int x, int y, int z, int soundId, int nSector)
{
    if (!SoundToggle || soundId < 0) return;

    if ((soundId == 3017) && gGameOptions.bQuadDamagePowerup && !VanillaMode()) // if quad damage is active, do not play quote about having two guns
        soundId = 3016;
    DICTNODE *hRes = gSoundRes.Lookup(soundId, "SFX");
    if (!hRes)return;

    SFX *pEffect = (SFX*)gSoundRes.Load(hRes);
    hRes = gSoundRes.Lookup(pEffect->rawName, "RAW");
    if (!hRes) return;

    int nPitch = mulscale16(pEffect->pitch, sndGetRate(pEffect->format));
    if (nBonkles >= 256)
        return;
    BONKLE *pBonkle = BonkleCache[nBonkles++];
    pBonkle->pSndSpr = NULL;
    pBonkle->curPos.x = x;
    pBonkle->curPos.y = y;
    pBonkle->curPos.z = z;
    pBonkle->sectnum = nSector;
    FindSector(x, y, z, &pBonkle->sectnum);
    pBonkle->oldPos = pBonkle->curPos;
    pBonkle->zOff = 0;
    pBonkle->nType = 0;
    pBonkle->sfxId = soundId;
    pBonkle->hSnd = hRes;
    pBonkle->vol = pEffect->relVol;
    pBonkle->pitch = nPitch;
    pBonkle->format = pEffect->format;
    int size = hRes->size;
    char *pData = (char*)gSoundRes.Lock(hRes);
    Calc3DValues(pBonkle);
    int priority = 1;
    if (priority < lVol)
        priority = lVol;
    if (priority < rVol)
        priority = rVol;
    if (gStereo)
    {
        // MV_Lock();
        pBonkle->lChan = FX_PlayRaw(pData + lPhase, size - lPhase, lPitch, 0, lVol, lVol, 0, priority, fix16_one, (intptr_t)&pBonkle->lChan);
        pBonkle->rChan = FX_PlayRaw(pData + rPhase, size - rPhase, rPitch, 0, rVol, 0, rVol, priority, fix16_one, (intptr_t)&pBonkle->rChan);
        // MV_Unlock();
    }
    else
    {
        pBonkle->lChan = FX_PlayRaw(pData + lPhase, size - lPhase, nPitch, 0, lVol, lVol, rVol, priority, fix16_one, (intptr_t)&pBonkle->lChan);
        pBonkle->rChan = 0;
    }
}

void sfxPlay3DSound(spritetype *pSprite, int soundId, int chanId, int nFlags)
{
    if (!SoundToggle)
        return;
    if (!pSprite)
        return;
    if (soundId < 0)
        return;
    if ((soundId == 3017) && gGameOptions.bQuadDamagePowerup && !VanillaMode()) // if quad damage is active, do not play quote about having two guns
        soundId = 3016;
    DICTNODE *hRes = gSoundRes.Lookup(soundId, "SFX");
    if (!hRes)
        return;

    SFX *pEffect = (SFX*)gSoundRes.Load(hRes);
    hRes = gSoundRes.Lookup(pEffect->rawName, "RAW");
    if (!hRes)
        return;
    int size = hRes->size;
    if (size <= 0)
        return;
    int nPitch = mulscale16(pEffect->pitch, sndGetRate(pEffect->format));
    BONKLE *pBonkle = NULL;
    if (chanId >= 0)
    {
        int i;
        for (i = 0; i < nBonkles; i++)
        {
            pBonkle = BonkleCache[i];
            if (pBonkle->chanId == chanId && (pBonkle->pSndSpr == pSprite || (nFlags & 1) != 0))
            {
                if ((nFlags & 4) != 0 && pBonkle->chanId == chanId)
                    return;
                if ((nFlags & 2) != 0 && pBonkle->sfxId == soundId)
                    return;
                if (pBonkle->lChan > 0)
                    FX_StopSound(pBonkle->lChan);
                if (pBonkle->rChan > 0)
                    FX_StopSound(pBonkle->rChan);
                if (pBonkle->hSnd)
                {
                    gSoundRes.Unlock(pBonkle->hSnd);
                    pBonkle->hSnd = NULL;
                }
                break;
            }
        }
        if (i == nBonkles)
        {
            if (nBonkles >= 256)
                return;
            pBonkle = BonkleCache[nBonkles++];
        }
        pBonkle->pSndSpr = !(nFlags & 8) ? pSprite : NULL;
        pBonkle->chanId = chanId;
    }
    else
    {
        if (nBonkles >= 256)
            return;
        pBonkle = BonkleCache[nBonkles++];
        pBonkle->pSndSpr = NULL;
    }
    pBonkle->curPos.x = pSprite->x;
    pBonkle->curPos.y = pSprite->y;
    pBonkle->curPos.z = pSprite->z;
    pBonkle->sectnum = pSprite->sectnum;
    pBonkle->oldPos = pBonkle->curPos;
    pBonkle->zOff = CalcYOffset(pSprite);
    pBonkle->nType = pSprite->type;
    pBonkle->sfxId = soundId;
    pBonkle->hSnd = hRes;
    pBonkle->vol = pEffect->relVol;
    pBonkle->pitch = nPitch;
    Calc3DValues(pBonkle);
    int priority = 1;
    if (priority < lVol)
        priority = lVol;
    if (priority < rVol)
        priority = rVol;
    int loopStart = pEffect->loopStart;
    int loopEnd = ClipLow(size - 1, 0);
    if (chanId < 0)
        loopStart = -1;
    // MV_Lock();
    char *pData = (char*)gSoundRes.Lock(hRes);
    if (loopStart >= 0)
    {
        if (gStereo)
        {
            pBonkle->lChan = FX_PlayLoopedRaw(pData + lPhase, size - lPhase, pData + loopStart, pData + loopEnd, lPitch, 0, lVol, lVol, 0, priority, fix16_one, (intptr_t)&pBonkle->lChan);
            pBonkle->rChan = FX_PlayLoopedRaw(pData + rPhase, size - rPhase, pData + loopStart, pData + loopEnd, rPitch, 0, rVol, 0, rVol, priority, fix16_one, (intptr_t)&pBonkle->rChan);
        }
        else
        {
            pBonkle->lChan = FX_PlayLoopedRaw(pData + lPhase, size - lPhase, pData + loopStart, pData + loopEnd, nPitch, 0, lVol, lVol, rVol, priority, fix16_one, (intptr_t)&pBonkle->lChan);
            pBonkle->rChan = 0;
        }
    }
    else
    {
        pData = (char*)gSoundRes.Lock(pBonkle->hSnd);
        if (gStereo)
        {
            pBonkle->lChan = FX_PlayRaw(pData + lPhase, size - lPhase, lPitch, 0, lVol, lVol, 0, priority, fix16_one, (intptr_t)&pBonkle->lChan);
            pBonkle->rChan = FX_PlayRaw(pData + rPhase, size - rPhase, rPitch, 0, rVol, 0, rVol, priority, fix16_one, (intptr_t)&pBonkle->rChan);
        }
        else
        {
            pBonkle->lChan = FX_PlayRaw(pData + lPhase, size - lPhase, nPitch, 0, lVol, lVol, rVol, priority, fix16_one, (intptr_t)&pBonkle->lChan);
            pBonkle->rChan = 0;
        }
    }
    // MV_Unlock();
}

// by NoOne: same as previous, but allows to set custom pitch for sound AND volume.
void sfxPlay3DSoundCP(spritetype* pSprite, int soundId, int chanId, int nFlags, int pitch, int volume, const char *pzSound)
{
    if (!SoundToggle || !pSprite || soundId < 0) return;
    if ((soundId == 3017) && gGameOptions.bQuadDamagePowerup && !VanillaMode()) // if quad damage is active, do not play quote about having two guns
        soundId = 3016;
    DICTNODE* hRes = gSoundRes.Lookup(soundId, "SFX");
    if (!hRes) return;

    SFX* pEffect = (SFX*)gSoundRes.Load(hRes);
    if (!pzSound) // if raw name override not provided, use slot's raw name
        pzSound = pEffect->rawName;
    hRes = gSoundRes.Lookup(pzSound, "RAW");
    if (!hRes) return;
    int size = hRes->size;
    if (size <= 0) return;
    
    if (pitch <= 0) pitch = pEffect->pitch;
    else pitch -= QRandom(pEffect->pitchRange);

    int nPitch = mulscale16(pitch, sndGetRate(pEffect->format));
    
    BONKLE * pBonkle = NULL;
    if (chanId >= 0)
    {
        int i;
        for (i = 0; i < nBonkles; i++)
        {
            pBonkle = BonkleCache[i];
            if (pBonkle->chanId == chanId && (pBonkle->pSndSpr == pSprite || (nFlags & 1) != 0))
            {
                if ((nFlags & 4) != 0 && pBonkle->chanId == chanId)
                    return;
                if ((nFlags & 2) != 0 && pBonkle->sfxId == soundId)
                    return;
                if (pBonkle->lChan > 0)
                    FX_StopSound(pBonkle->lChan);
                if (pBonkle->rChan > 0)
                    FX_StopSound(pBonkle->rChan);
                if (pBonkle->hSnd)
                {
                    gSoundRes.Unlock(pBonkle->hSnd);
                    pBonkle->hSnd = NULL;
                }
                break;
            }
        }
        if (i == nBonkles)
        {
            if (nBonkles >= 256)
                return;
            pBonkle = BonkleCache[nBonkles++];
        }
        pBonkle->pSndSpr = !(nFlags & 8) ? pSprite : NULL;
        pBonkle->chanId = chanId;
    }
    else
    {
        if (nBonkles >= 256)
            return;
        pBonkle = BonkleCache[nBonkles++];
        pBonkle->pSndSpr = NULL;
    }
    pBonkle->curPos.x = pSprite->x;
    pBonkle->curPos.y = pSprite->y;
    pBonkle->curPos.z = pSprite->z;
    pBonkle->sectnum = pSprite->sectnum;
    pBonkle->oldPos = pBonkle->curPos;
    pBonkle->zOff = CalcYOffset(pSprite);
    pBonkle->nType = pSprite->type;
    pBonkle->sfxId = soundId;
    pBonkle->hSnd = hRes;
    switch (volume)
    {
        case 0:
            pBonkle->vol = pEffect->relVol;
            break;
        case -1:
            pBonkle->vol = 0;
            break;
        default:
            pBonkle->vol = volume;
            break;
    }

    pBonkle->pitch = nPitch;
    Calc3DValues(pBonkle);
    int priority = 1;
    if (priority < lVol)
        priority = lVol;
    if (priority < rVol)
        priority = rVol;
    int loopStart = pEffect->loopStart;
    int loopEnd = ClipLow(size - 1, 0);
    if (chanId < 0)
        loopStart = -1;
    // MV_Lock();
    char* pData = (char*)gSoundRes.Lock(hRes);
    if (loopStart >= 0)
    {
        if (gStereo)
        {
            pBonkle->lChan = FX_PlayLoopedRaw(pData + lPhase, size - lPhase, pData + loopStart, pData + loopEnd, lPitch, 0, lVol, lVol, 0, priority, fix16_one, (intptr_t)& pBonkle->lChan);
            pBonkle->rChan = FX_PlayLoopedRaw(pData + rPhase, size - rPhase, pData + loopStart, pData + loopEnd, rPitch, 0, rVol, 0, rVol, priority, fix16_one, (intptr_t)& pBonkle->rChan);
        }
        else
        {
            pBonkle->lChan = FX_PlayLoopedRaw(pData + lPhase, size - lPhase, pData + loopStart, pData + loopEnd, nPitch, 0, lVol, lVol, rVol, priority, fix16_one, (intptr_t)& pBonkle->lChan);
            pBonkle->rChan = 0;
        }
    }
    else
    {
        pData = (char*)gSoundRes.Lock(pBonkle->hSnd);
        if (gStereo)
        {
            pBonkle->lChan = FX_PlayRaw(pData + lPhase, size - lPhase, lPitch, 0, lVol, lVol, 0, priority, fix16_one, (intptr_t)& pBonkle->lChan);
            pBonkle->rChan = FX_PlayRaw(pData + rPhase, size - rPhase, rPitch, 0, rVol, 0, rVol, priority, fix16_one, (intptr_t)& pBonkle->rChan);
        }
        else
        {
            pBonkle->lChan = FX_PlayRaw(pData + lPhase, size - lPhase, nPitch, 0, lVol, lVol, rVol, priority, fix16_one, (intptr_t)& pBonkle->lChan);
            pBonkle->rChan = 0;
        }
    }
    // MV_Unlock();
}

EXTERN_INLINE void sfxKillSoundInternal(int i)
{
    BONKLE *pBonkle = BonkleCache[i];
    if (pBonkle->lChan > 0)
    {
        FX_EndLooping(pBonkle->lChan);
        FX_StopSound(pBonkle->lChan);
    }
    if (pBonkle->rChan > 0)
    {
        FX_EndLooping(pBonkle->rChan);
        FX_StopSound(pBonkle->rChan);
    }
    if (pBonkle->hSnd)
    {
        gSoundRes.Unlock(pBonkle->hSnd);
        pBonkle->hSnd = NULL;
    }
    BonkleCache[i] = BonkleCache[--nBonkles];
    BonkleCache[nBonkles] = pBonkle;
}

void sfxKill3DSound(spritetype *pSprite, int chanId, int soundId)
{
    if (!pSprite)
        return;
    for (int i = nBonkles - 1; i >= 0; i--)
    {
        BONKLE *pBonkle = BonkleCache[i];
        if (pBonkle->pSndSpr == pSprite && (chanId < 0 || chanId == pBonkle->chanId) && (soundId < 0 || soundId == pBonkle->sfxId))
        {
            sfxKillSoundInternal(i);
            break;
        }
    }
}

void sfxKillAllSounds(void)
{
    for (int i = nBonkles - 1; i >= 0; i--)
    {
        sfxKillSoundInternal(i);
    }

    for (int i = 0; i < 4; i++)
        gSoundDingSprite[i] = kSoundDingSpriteInit;
}

void sfxKillSpriteSounds(spritetype *pSprite)
{
    if (!pSprite)
        return;
    for (int i = nBonkles - 1; i >= 0; i--)
    {
        BONKLE *pBonkle = BonkleCache[i];
        if (pBonkle->pSndSpr == pSprite)
        {
            sfxKillSoundInternal(i);
        }
    }
}

void sfxUpdateSpritePos(spritetype *pSprite, vec3_t const *pOldPos)
{
    dassert(pSprite != NULL);
    for (int i = nBonkles - 1; i >= 0; i--) // update all attached sprite sfx to new position
    {
        BONKLE *pBonkle = BonkleCache[i];
        if (pBonkle->pSndSpr == pSprite)
        {
            pBonkle->curPos.x = pSprite->x;
            pBonkle->curPos.y = pSprite->y;
            pBonkle->curPos.z = pSprite->z;
            if (pOldPos)
            {
                pBonkle->oldPos.x = pSprite->x+(pBonkle->oldPos.x-pOldPos->x);
                pBonkle->oldPos.y = pSprite->y+(pBonkle->oldPos.y-pOldPos->y);
                pBonkle->oldPos.z = pSprite->z+(pBonkle->oldPos.z-pOldPos->z);
            }
            else // no old position given, reset current velocity
            {
                pBonkle->oldPos = pBonkle->curPos;
            }
            if (gSoundOcclusion)
            {
                pBonkle->zOff = CalcYOffset(pSprite);
                pBonkle->nType = pSprite->type;
            }
        }
    }
}

inline void sfxUpdateListenerPos(void)
{
    const int dx = mulscale30(Cos(gMe->pSprite->ang + kAng90), kEarDist>>1);
    const int dy = mulscale30(Sin(gMe->pSprite->ang + kAng90), kEarDist>>1);
    earL0 = earL;
    earR0 = earR;
    earL = {gMe->pSprite->x - dx, gMe->pSprite->y - dy};
    earR = {gMe->pSprite->x + dx, gMe->pSprite->y + dy};
}

inline void sfxUpdateListenerVel(void)
{
    earVL = {earL.x - earL0.x, earL.y - earL0.y};
    earVR = {earR.x - earR0.x, earR.y - earR0.y};
}

void sfxCorrectListenerPos(void)
{
    sfxUpdateListenerPos();
    earL0 = earL;
    earR0 = earR;
    earL0.x += -earVL.dx;
    earL0.y += -earVL.dy;
    earR0.x += -earVR.dx;
    earR0.y += -earVR.dy;
}

void sfxResetListener(void)
{
    sfxUpdateListenerPos(); // update ear position
    earVL = earVR = {0, 0}; // reset ear velocity
}

void sfxUpdateSpeedOfSound(void)
{
    if (gSoundSpeed != oldSoundSpeed) // if speed of sound setting has been changed, convert real world meters to build engine units
    {
        oldSoundSpeed = gSoundSpeed = ClipRange(gSoundSpeed, 10, 1000);
        nSoundSpeed = (int)(((32<<4) * gSoundSpeed) / kTicsPerSec);
    }
}

void sfxUpdateEarAng(void)
{
    if (gSoundEarAng != oldEarAng) // if ear angle setting has been changed, convert degrees to build engine degrees
    {
        oldEarAng = gSoundEarAng = ClipRange(gSoundEarAng, 15, 90);
        nEarAng = (int)(gSoundEarAng * (kAng360 / 360.f));
    }
}

int ClampScale(int nVal, int nInMin, int nInMax, int nOutMin, int nOutMax)
{
	if (nInMin == nInMax)
		return (nVal - nInMax) >= 0 ? nOutMax : nOutMin;
	float cVal = float(nVal - nInMin) / float(nInMax - nInMin);
	cVal = ClipRangeF(cVal, 0.f, 1.f);

	return nOutMin + int(float(nOutMax - nOutMin) * cVal);
}

void sfxPlayerDamageFeedback(void)
{
    const int kMinDam = 50, kMaxDam = 1500, kDelayTicks = 7;
    for (int i = 0; i < 4; i++)
    {
        DMGFEEDBACK *pSoundDmgSprite = &gSoundDingSprite[i];
        if (pSoundDmgSprite->nSprite == -1) // reached end of attacked sprite list, stop
            break;
        const int nTickDiff = klabs(gLevelTime - pSoundDmgSprite->nTick);
        if (!nTickDiff) // this sfx will trigger in the same tick, skip
            continue;
        if (nTickDiff < kDelayTicks) // this sfx will trigger too soon, skip
            continue;

        const int nRate = ClampScale(pSoundDmgSprite->nDamage, kMinDam, kMaxDam, gSoundDingMinPitch, gSoundDingMaxPitch);
        sndStartSample("NOTHIT", gSoundDingVol, -1, nRate);
        pSoundDmgSprite->nSprite = -1;
        pSoundDmgSprite->nDamage = 0;
        pSoundDmgSprite->nTick = gLevelTime;
    }
}

void sfxUpdate3DSounds(void)
{
#ifdef NORENDER
    return;
#endif
    sfxUpdateListenerPos();
    sfxUpdateListenerVel();
    sfxUpdateSpeedOfSound();
    sfxUpdateEarAng();
    if (gSoundDing)
        sfxPlayerDamageFeedback();
    for (int i = nBonkles - 1; i >= 0; i--)
    {
        BONKLE *pBonkle = BonkleCache[i];
        if (pBonkle->lChan > 0 || pBonkle->rChan > 0)
        {
            if (!pBonkle->hSnd)
                continue;
            if (pBonkle->pSndSpr)
            {
                pBonkle->oldPos = pBonkle->curPos;
                pBonkle->curPos = {pBonkle->pSndSpr->x, pBonkle->pSndSpr->y, pBonkle->pSndSpr->z};
                pBonkle->sectnum = pBonkle->pSndSpr->sectnum;
            }
            Calc3DValues(pBonkle);
            // MV_Lock();
            if (pBonkle->lChan > 0)
            {
                if (pBonkle->rChan > 0)
                {
                    FX_SetPan(pBonkle->lChan, lVol, lVol, 0);
                    FX_SetFrequency(pBonkle->lChan, lPitch);
                }
                else
                    FX_SetPan(pBonkle->lChan, lVol, lVol, rVol);
            }
            if (pBonkle->rChan > 0)
            {
                FX_SetPan(pBonkle->rChan, rVol, 0, rVol);
                FX_SetFrequency(pBonkle->rChan, rPitch);
            }
            // MV_Unlock();
        }
        else
        {
            gSoundRes.Unlock(pBonkle->hSnd);
            pBonkle->hSnd = NULL;
            BonkleCache[i] = BonkleCache[--nBonkles];
            BonkleCache[nBonkles] = pBonkle;
        }
    }
}

void sfxSetReverb(bool toggle)
{
    if (toggle)
    {
        FX_SetReverb(128);
        FX_SetReverbDelay(10);
    }
    else
        FX_SetReverb(0);
}

void sfxSetReverb2(bool toggle)
{
    if (toggle)
    {
        FX_SetReverb(128);
        FX_SetReverbDelay(20);
    }
    else
        FX_SetReverb(0);
}

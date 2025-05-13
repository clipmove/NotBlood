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
#include "editor.h"
#ifdef POLYMER
#include "polymer.h"
#endif
#include "compat.h"
#include "common_game.h"
#include "crc32.h"
#include "md4.h"

#include "actor.h"
#include "config.h"
#include "globals.h"
#include "blood.h"
#include "db.h"
#include "iob.h"
#include "eventq.h"
#include "levels.h"
#ifdef NOONE_EXTENSIONS
#include "nnexts.h"
#include "view.h"
#endif

#define kMapHeaderNew 0x7474614D // 'ttaM' signature
#define kMapHeaderOld 0x4D617474 // 'Matt' signature

#ifdef NOONE_EXTENSIONS
uint8_t gModernMap = false;
#endif // !NOONE_EXTENSIONS


unsigned short gStatCount[kMaxStatus + 1];

XSPRITE xsprite[kMaxXSprites];
XSECTOR xsector[kMaxXSectors];
XWALL xwall[kMaxXWalls];

SPRITEHIT gSpriteHit[kMaxXSprites];

int xvel[kMaxSprites], yvel[kMaxSprites], zvel[kMaxSprites];

#ifdef POLYMER
PolymerLight_t gPolymerLight[kMaxSprites];
#endif

char qsprite_filler[kMaxSprites], qsector_filler[kMaxSectors];

int gVisibility;
const char *gItemText[] = {
    "Skull Key",
    "Eye Key",
    "Fire Key",
    "Dagger Key",
    "Spider Key",
    "Moon Key",
    "Key 7",
    "Doctor's Bag",
    "Medicine Pouch",
    "Life Essence",
    "Life Seed",
    "Red Potion",
    "Feather Fall",
    "Limited Invisibility",
    "INVULNERABILITY",
    "Boots of Jumping",
    "Raven Flight",
    "Guns Akimbo",
    "Diving Suit",
    "Gas mask",
    "Clone",
    "Crystal Ball",
    "Decoy",
    "Doppleganger",
    "Reflective shots",
    "Beast Vision",
    "ShadowCloak",
    "Rage shroom",
    "Delirium Shroom",
    "Grow shroom",
    "Shrink shroom",
    "Death mask",
    "Wine Goblet",
    "Wine Bottle",
    "Skull Grail",
    "Silver Grail",
    "Tome",
    "Black Chest",
    "Wooden Chest",
    "Asbestos Armor",
    "Basic Armor",
    "Body Armor",
    "Fire Armor",
    "Spirit Armor",
    "Super Armor",
    "Blue Team Base",
    "Red Team Base",
    "Blue Flag",
    "Red Flag",
    "DUMMY",
    "Level map",
};

const char *gAmmoText[] = {
    "Spray can",
    "Bundle of TNT*",
    "Bundle of TNT",
    "Case of TNT",
    "Proximity Detonator",
    "Remote Detonator",
    "Trapped Soul",
    "4 shotgun shells",
    "Box of shotgun shells",
    "A few bullets",
    "Voodoo Doll",
    "OBSOLETE",
    "Full drum of bullets",
    "Tesla Charge",
    "OBSOLETE",
    "OBSOLETE",
    "Flares",
    "OBSOLETE",
    "OBSOLETE",
    "Gasoline Can",
    NULL,
};

const char *gWeaponText[] = {
    "RANDOM",
    "Sawed-off",
    "Tommy Gun",
    "Flare Pistol",
    "Voodoo Doll",
    "Tesla Cannon",
    "Napalm Launcher",
    "Pitchfork",
    "Spray Can",
    "Dynamite",
    "Life Leech",
};



uint32_t XSPRITE::CalcChecksum(void)
{
    // This was originally written to calculate the checksum
    // the way OUWB v1.21 does. Therefore, certain bits may
    // be skipped or calculated in a different order.
    uint32_t sum = 0;
    sum += (reference&16383) | ((state&1)<<14) | ((busy&131071)<<15);
    sum += (txID&1023) | ((rxID&1023)<<10) | ((command&255)<<20) |
           ((triggerOn&1)<<28) | ((triggerOff&1)<<29) | ((wave&3)<<30);
    sum += (busyTime&4095) | ((waitTime&4095)<<12) |
           ((restState&1)<<24) | ((Interrutable&1)<<25) |
           ((unused1&3)<<26) | ((respawnPending&3)<<28) |
           ((unused2&1)<<30) | ((lT&1)<<31);
    sum += (dropMsg&255) | ((Decoupled&1)<<8) |
           ((triggerOnce&1)<<9) | ((isTriggered&1)<<10) |
           ((key&7)<<11) | ((Push&1)<<14) | ((Vector&1)<<15) |
           ((Impact&1)<<16) | ((Pickup&1)<<17) | ((Touch&1)<<18) |
           ((Sight&1)<<19) | ((Proximity&1)<<20) |
           ((unused3&3)<<21) | ((lSkill&31)<<23) |
           ((lS&1)<<28) | ((lB&1)<<29) | ((lC&1)<<30) | ((DudeLockout&1)<<31);
    sum += (data1&65535) | ((data2&65535)<<16);
    sum += (data3&65535) | ((goalAng&2047)<<16) | ((dodgeDir&3)<<27) |
           ((locked&1)<<29) | ((medium&3)<<30);
    sum += (respawn&3) | ((data4&65535)<<2) |
           ((unused4&63)<<18) | ((lockMsg&255)<<24);
    sum += (health&4095) | ((dudeDeaf&1)<<12) | ((dudeAmbush&1)<<13) |
           ((dudeGuard&1)<<14) | ((dudeFlag4&1)<<15) | ((target&65535)<<16);
    sum += targetX&0xFFFFFFFF;
    sum += targetY&0xFFFFFFFF;
    sum += targetZ&0xFFFFFFFF;
    sum += (burnTime&65535) | ((burnSource&65535)<<16);
    sum += (height&65535) | ((stateTimer&65535)<<16);
    // aiState is a state pointer. Exact pointer values depend on exe layout.
    // For player sprites, it is apparently always set to 0.
    sum += aiState ? (((uintptr_t)aiState)&0xFFFFFFFF) : 0;
    return sum;
}

void dbCrypt(char *pPtr, int nLength, int nKey)
{
    for (int i = 0; i < nLength; i++)
    {
        pPtr[i] = pPtr[i] ^ nKey;
        nKey++;
    }
}

#ifdef POLYMER

void DeleteLight(int32_t s)
{
    if (gPolymerLight[s].lightId >= 0)
        polymer_deletelight(gPolymerLight[s].lightId);
    gPolymerLight[s].lightId = -1;
    gPolymerLight[s].lightptr = NULL;
}

#endif


void InsertSpriteSect(int nSprite, int nSector)
{
    dassert(nSprite >= 0 && nSprite < kMaxSprites);
    dassert(nSector >= 0 && nSector < kMaxSectors);
    int nOther = headspritesect[nSector];
    if (nOther >= 0)
    {
        prevspritesect[nSprite] = prevspritesect[nOther];
        nextspritesect[nSprite] = -1;
        nextspritesect[prevspritesect[nOther]] = nSprite;
        prevspritesect[nOther] = nSprite;
    }
    else
    {
        prevspritesect[nSprite] = nSprite;
        nextspritesect[nSprite] = -1;
        headspritesect[nSector] = nSprite;
    }
    sprite[nSprite].sectnum = nSector;
}

void RemoveSpriteSect(int nSprite)
{
    dassert(nSprite >= 0 && nSprite < kMaxSprites);
    int nSector = sprite[nSprite].sectnum;
    dassert(nSector >= 0 && nSector < kMaxSectors);
    int nOther = nextspritesect[nSprite];
    if (nOther < 0)
    {
        nOther = headspritesect[nSector];
    }
    prevspritesect[nOther] = prevspritesect[nSprite];
    if (headspritesect[nSector] != nSprite)
    {
        nextspritesect[prevspritesect[nSprite]] = nextspritesect[nSprite];
    }
    else
    {
        headspritesect[nSector] = nextspritesect[nSprite];
    }
    sprite[nSprite].sectnum = -1;
}

void InsertSpriteStat(int nSprite, int nStat)
{
    dassert(nSprite >= 0 && nSprite < kMaxSprites);
    dassert(nStat >= 0 && nStat <= kMaxStatus);
    int nOther = headspritestat[nStat];
    if (nOther >= 0)
    {
        prevspritestat[nSprite] = prevspritestat[nOther];
        nextspritestat[nSprite] = -1;
        nextspritestat[prevspritestat[nOther]] = nSprite;
        prevspritestat[nOther] = nSprite;
    }
    else
    {
        prevspritestat[nSprite] = nSprite;
        nextspritestat[nSprite] = -1;
        headspritestat[nStat] = nSprite;
    }
    sprite[nSprite].statnum = nStat;
    gStatCount[nStat]++;
}

void RemoveSpriteStat(int nSprite)
{
    dassert(nSprite >= 0 && nSprite < kMaxSprites);
    int nStat = sprite[nSprite].statnum;
    dassert(nStat >= 0 && nStat <= kMaxStatus);
    int nOther = nextspritestat[nSprite];
    if (nOther < 0)
    {
        nOther = headspritestat[nStat];
    }
    prevspritestat[nOther] = prevspritestat[nSprite];
    if (headspritestat[nStat] != nSprite)
    {
        nextspritestat[prevspritestat[nSprite]] = nextspritestat[nSprite];
    }
    else
    {
        headspritestat[nStat] = nextspritestat[nSprite];
    }
    sprite[nSprite].statnum = kStatNothing;
    gStatCount[nStat]--;
}

void qinitspritelists(void) // Replace
{
    for (short i = 0; i <= kMaxSectors; i++)
    {
        headspritesect[i] = -1;
    }
    for (short i = 0; i <= kMaxStatus; i++)
    {
        headspritestat[i] = -1;
    }
    int const nMaxSprites = VanillaMode() ? 4096 : kMaxSprites;
    for (short i = 0; i < nMaxSprites; i++)
    {
        sprite[i].sectnum = -1;
        sprite[i].index = -1;
        InsertSpriteStat(i, kMaxStatus);
    }
    memset(gStatCount, 0, sizeof(gStatCount));
    Numsprites = 0;
}

int InsertSprite(int nSector, int nStat)
{
    int nSprite = headspritestat[kMaxStatus];
    dassert(nSprite < kMaxSprites);
    if (nSprite < 0)
    {
        return nSprite;
    }
    RemoveSpriteStat(nSprite);
    spritetype *pSprite = &sprite[nSprite];
    memset(&sprite[nSprite], 0, sizeof(spritetype));
    InsertSpriteStat(nSprite, nStat);
    InsertSpriteSect(nSprite, nSector);
    pSprite->cstat = 128;
    pSprite->clipdist = 32;
    pSprite->xrepeat = pSprite->yrepeat = 64;
    pSprite->owner = -1;
    pSprite->extra = -1;
    pSprite->index = nSprite;
    xvel[nSprite] = yvel[nSprite] = zvel[nSprite] = 0;
    qsprite_filler[nSprite] = 0;

#ifdef POLYMER
    gPolymerLight[nSprite].lightId = -1;
#endif

    Numsprites++;

    return nSprite;
}

int qinsertsprite(short nSector, short nStat) // Replace
{
    return InsertSprite(nSector, nStat);
}

int DeleteSprite(int nSprite)
{
#ifdef POLYMER
    if (gPolymerLight[nSprite].lightptr != NULL && videoGetRenderMode() == REND_POLYMER)
        DeleteLight(nSprite);
#endif
    if (sprite[nSprite].extra > 0)
    {
        dbDeleteXSprite(sprite[nSprite].extra);
    }
    dassert(sprite[nSprite].statnum >= 0 && sprite[nSprite].statnum < kMaxStatus);
    RemoveSpriteStat(nSprite);
    dassert(sprite[nSprite].sectnum >= 0 && sprite[nSprite].sectnum < kMaxSectors);
    RemoveSpriteSect(nSprite);
    InsertSpriteStat(nSprite, kMaxStatus);

    Numsprites--;

    return nSprite;
}

int qdeletesprite(short nSprite) // Replace
{
    return DeleteSprite(nSprite);
}

int ChangeSpriteSect(int nSprite, int nSector)
{
    dassert(nSprite >= 0 && nSprite < kMaxSprites);
    dassert(nSector >= 0 && nSector < kMaxSectors);
    dassert(sprite[nSprite].sectnum >= 0 && sprite[nSprite].sectnum < kMaxSectors);
    RemoveSpriteSect(nSprite);
    InsertSpriteSect(nSprite, nSector);
    return 0;
}

int qchangespritesect(short nSprite, short nSector)
{
    return ChangeSpriteSect(nSprite, nSector);
}

int ChangeSpriteStat(int nSprite, int nStatus)
{
    dassert(nSprite >= 0 && nSprite < kMaxSprites);
    dassert(nStatus >= 0 && nStatus < kMaxStatus);
    dassert(sprite[nSprite].statnum >= 0 && sprite[nSprite].statnum < kMaxStatus);
    dassert(sprite[nSprite].sectnum >= 0 && sprite[nSprite].sectnum < kMaxSectors);
    RemoveSpriteStat(nSprite);
    InsertSpriteStat(nSprite, nStatus);
    return 0;
}

int qchangespritestat(short nSprite, short nStatus)
{
    return ChangeSpriteStat(nSprite, nStatus);
}

unsigned short nextXSprite[kMaxXSprites];
unsigned short nextXWall[kMaxXWalls];
unsigned short nextXSector[kMaxXSectors];

void InitFreeList(unsigned short *pList, int nCount)
{
    for (int i = 1; i < nCount; i++)
    {
        pList[i] = i-1;
    }
    pList[0] = nCount - 1;
}

void InsertFree(unsigned short *pList, int nIndex)
{
    pList[nIndex] = pList[0];
    pList[0] = nIndex;
}

unsigned short dbInsertXSprite(int nSprite)
{
    int nXSprite = nextXSprite[0];
    nextXSprite[0] = nextXSprite[nXSprite];
    if (nXSprite == 0)
    {
        ThrowError("Out of free XSprites");
    }
    memset(&xsprite[nXSprite], 0, sizeof(XSPRITE));
#if 0 // this intentionally causes demos to desync like dos (eg: playing BLOOD002.DEM after finishing BLOOD001.DEM) - for notblood we'll disable it
    if (!VanillaMode())
#endif
        memset(&gSpriteHit[nXSprite], 0, sizeof(SPRITEHIT));
    xsprite[nXSprite].reference = nSprite;
    sprite[nSprite].extra = nXSprite;
    return nXSprite;
}

void dbDeleteXSprite(int nXSprite)
{
    dassert(xsprite[nXSprite].reference >= 0);
    dassert(sprite[xsprite[nXSprite].reference].extra == nXSprite);
    InsertFree(nextXSprite, nXSprite);
    sprite[xsprite[nXSprite].reference].extra = -1;
    xsprite[nXSprite].reference = -1;
}

unsigned short dbInsertXWall(int nWall)
{
    int nXWall = nextXWall[0];
    nextXWall[0] = nextXWall[nXWall];
    if (nXWall == 0)
    {
        ThrowError("Out of free XWalls");
    }
    memset(&xwall[nXWall], 0, sizeof(XWALL));
    xwall[nXWall].reference = nWall;
    wall[nWall].extra = nXWall;
    return nXWall;
}

void dbDeleteXWall(int nXWall)
{
    dassert(xwall[nXWall].reference >= 0);
    InsertFree(nextXWall, nXWall);
    wall[xwall[nXWall].reference].extra = -1;
    xwall[nXWall].reference = -1;
}

unsigned short dbInsertXSector(int nSector)
{
    int nXSector = nextXSector[0];
    nextXSector[0] = nextXSector[nXSector];
    if (nXSector == 0)
    {
        ThrowError("Out of free XSectors");
    }
    memset(&xsector[nXSector], 0, sizeof(XSECTOR));
    xsector[nXSector].reference = nSector;
    sector[nSector].extra = nXSector;
    return nXSector;
}

void dbDeleteXSector(int nXSector)
{
    dassert(xsector[nXSector].reference >= 0);
    InsertFree(nextXSector, nXSector);
    sector[xsector[nXSector].reference].extra = -1;
    xsector[nXSector].reference = -1;
}

void dbXSpriteClean(void)
{
    for (int i = 0; i < kMaxSprites; i++)
    {
        int nXSprite = sprite[i].extra;
        if (nXSprite == 0)
        {
            sprite[i].extra = -1;
        }
        if (sprite[i].statnum < kMaxStatus && nXSprite > 0)
        {
            dassert(nXSprite < kMaxXSprites);
            if (xsprite[nXSprite].reference != i)
            {
                int nXSprite2 = dbInsertXSprite(i);
                memcpy(&xsprite[nXSprite2], &xsprite[nXSprite], sizeof(XSPRITE));
                xsprite[nXSprite2].reference = i;
            }
        }
    }
    for (int i = 1; i < kMaxXSprites; i++)
    {
        int nSprite = xsprite[i].reference;
        if (nSprite >= 0)
        {
            dassert(nSprite < kMaxSprites);
            if (sprite[nSprite].statnum >= kMaxStatus || sprite[nSprite].extra != i)
            {
                InsertFree(nextXSprite, i);
                xsprite[i].reference = -1;
            }
        }
    }
}

void dbXWallClean(void)
{
    for (int i = 0; i < numwalls; i++)
    {
        int nXWall = wall[i].extra;
        if (nXWall == 0)
        {
            wall[i].extra = -1;
        }
        if (nXWall > 0)
        {
            dassert(nXWall < kMaxXWalls);
            if (xwall[nXWall].reference == -1)
            {
                wall[i].extra = -1;
            }
            else
            {
                xwall[nXWall].reference = i;
            }
        }
    }
    for (int i = 0; i < numwalls; i++)
    {
        int nXWall = wall[i].extra;
        if (nXWall > 0)
        {
            dassert(nXWall < kMaxXWalls);
            if (xwall[nXWall].reference != i)
            {
                int nXWall2 = dbInsertXWall(i);
                memcpy(&xwall[nXWall2], &xwall[nXWall], sizeof(XWALL));
                xwall[nXWall2].reference = i;
            }
        }
    }
    for (int i = 1; i < kMaxXWalls; i++)
    {
        int nWall = xwall[i].reference;
        if (nWall >= 0)
        {
            dassert(nWall < kMaxWalls);
            if (nWall >= numwalls || wall[nWall].extra != i)
            {
                InsertFree(nextXWall, i);
                xwall[i].reference = -1;
            }
        }
    }
}

void dbXSectorClean(void)
{


    for (int i = 0; i < numsectors; i++)
    {
        int nXSector = sector[i].extra;
        if (nXSector == 0)
        {
            sector[i].extra = -1;
        }
        if (nXSector > 0)
        {
            dassert(nXSector < kMaxXSectors);
            if (xsector[nXSector].reference == -1)
            {
                sector[i].extra = -1;
            }
            else
            {
                xsector[nXSector].reference = i;
            }
        }
    }
    for (int i = 0; i < numsectors; i++)
    {
        int nXSector = sector[i].extra;
        if (nXSector > 0)
        {
            dassert(nXSector < kMaxXSectors);
            if (xsector[nXSector].reference != i)
            {
                int nXSector2 = dbInsertXSector(i);
                memcpy(&xsector[nXSector2], &xsector[nXSector], sizeof(XSECTOR));
                xsector[nXSector2].reference = i;
            }
        }
    }
    for (int i = 1; i < kMaxXSectors; i++)
    {
        int nSector = xsector[i].reference;
        if (nSector >= 0)
        {
            dassert(nSector < kMaxSectors);
            if (nSector >= numsectors || sector[nSector].extra != i)
            {
                InsertFree(nextXSector, i);
                xsector[i].reference = -1;
            }
        }
    }
}

void dbInit(void)
{
    InitFreeList(nextXSprite, kMaxXSprites);
    for (int i = 1; i < kMaxXSprites; i++)
    {
        xsprite[i].reference = -1;
    }
    InitFreeList(nextXWall, kMaxXWalls);
    for (int i = 1; i < kMaxXWalls; i++)
    {
        xwall[i].reference = -1;
    }
    InitFreeList(nextXSector, kMaxXSectors);
    for (int i = 1; i < kMaxXSectors; i++)
    {
        xsector[i].reference = -1;
    }
    initspritelists();
    for (int i = 0; i < kMaxSprites; i++)
    {
        sprite[i].cstat = 128;
    }
}

void PropagateMarkerReferences(void)
{
    int nSprite, nNextSprite;
    for (nSprite = headspritestat[kStatMarker]; nSprite != -1; nSprite = nNextSprite) {
        
        nNextSprite = nextspritestat[nSprite];
        
        switch (sprite[nSprite].type)  {
            case kMarkerOff:
            case kMarkerAxis:
            case kMarkerWarpDest: {
                int nOwner = sprite[nSprite].owner;
                if (nOwner >= 0 && nOwner < numsectors) {
                    int nXSector = sector[nOwner].extra;
                    if (nXSector > 0 && nXSector < kMaxXSectors) {
                        xsector[nXSector].marker0 = nSprite;
                        continue;
                    }
                }
            }
            break;
            case kMarkerOn: {
                int nOwner = sprite[nSprite].owner;
                if (nOwner >= 0 && nOwner < numsectors) {
                    int nXSector = sector[nOwner].extra;
                    if (nXSector > 0 && nXSector < kMaxXSectors) {
                        xsector[nXSector].marker1 = nSprite;
                        continue;
                    }
                }
            }
            break;
        }
        
        DeleteSprite(nSprite);
    }
}

char dbIsBannedSpriteType(int nType)
{
    const unsigned int nBannedType = gGameOptions.uSpriteBannedFlags;
    if (nBannedType == BANNED_NONE) // no monsters banned, return
        return 0;
    char bBanned = 0;

    // monsters
    if (!bBanned && (nBannedType&BANNED_BATS))
        bBanned = nType == kDudeBat;
    if (!bBanned && (nBannedType&BANNED_RATS))
        bBanned = nType == kDudeRat;
    if (!bBanned && (nBannedType&BANNED_FISH))
        bBanned = (nType == kDudeGillBeast) || (nType == kDudeBoneEel);
    if (!bBanned && (nBannedType&BANNED_HANDS))
        bBanned = nType == kDudeHand;
    if (!bBanned && (nBannedType&BANNED_GHOSTS))
        bBanned = nType == kDudePhantasm;
    if (!bBanned && (nBannedType&BANNED_SPIDERS))
        bBanned = (nType == kDudeSpiderBrown) || (nType == kDudeSpiderRed) || (nType == kDudeSpiderBlack);
    if (!bBanned && (nBannedType&BANNED_TCALEBS))
        bBanned = nType == kDudeTinyCaleb;
    if (!bBanned && (nBannedType&BANNED_HHOUNDS))
        bBanned = nType == kDudeHellHound;

    // weapons
    if (!bBanned && (nBannedType&BANNED_FLARE))
        bBanned = (nType == kItemWeaponFlarePistol) || (nType == kItemAmmoFlares);
    if (!bBanned && (nBannedType&BANNED_SHOTGUN))
        bBanned = (nType == kItemWeaponSawedoff) || (nType == kItemAmmoSawedoffFew) || (nType == kItemAmmoSawedoffBox);
    if (!bBanned && (nBannedType&BANNED_TOMMYGUN))
        bBanned = (nType == kItemWeaponTommygun) || (nType == kItemAmmoTommygunFew) || (nType == kItemAmmoTommygunDrum);
    if (!bBanned && (nBannedType&BANNED_NAPALM))
        bBanned = (nType == kItemWeaponNapalmLauncher) || (nType == kItemAmmoGasolineCan);
    if (!bBanned && (nBannedType&BANNED_TNT))
        bBanned = (nType == kItemWeaponTNT) || (nType == kItemAmmoTNTBundle) || (nType == kItemAmmoTNTBox);
    if (!bBanned && (nBannedType&BANNED_SPRAYCAN))
        bBanned = (nType == kItemWeaponSprayCan) || (nType == kItemAmmoSprayCan);
    if (!bBanned && (nBannedType&BANNED_TESLA))
        bBanned = (nType == kItemWeaponTeslaCannon) || (nType == kItemAmmoTeslaCharge);
    if (!bBanned && (nBannedType&BANNED_LIFELEECH))
        bBanned = (nType == kItemWeaponLifeLeech) || (nType == kItemAmmoTrappedSoul);
    if (!bBanned && (nBannedType&BANNED_VOODOO))
        bBanned = (nType == kItemWeaponVoodooDoll) || (nType == kItemAmmoVoodooDoll);
    if (!bBanned && (nBannedType&BANNED_PROXY))
        bBanned = nType == kItemAmmoProxBombBundle;
    if (!bBanned && (nBannedType&BANNED_REMOTE))
        bBanned = nType == kItemAmmoRemoteBombBundle;

    // items
    if (!bBanned && (nBannedType&BANNED_MEDKIT))
        bBanned = nType == kItemHealthDoctorBag;
    if (!bBanned && (nBannedType&BANNED_LIFEESSENCE)) // life essence, and other misc healing items
        bBanned = (nType == kItemHealthMedPouch) || (nType == kItemHealthLifeEssense) || (nType == kItemHealthRedPotion);
    if (!bBanned && (nBannedType&BANNED_LIFESEED))
        bBanned = nType == kItemHealthLifeSeed;
    if (!bBanned && (nBannedType&BANNED_SUPERARMOR))
        bBanned = nType == kItemArmorSuper;
    if (!bBanned && (nBannedType&BANNED_CRYSTALBALL))
        bBanned = nType == kItemCrystalBall;

    // powerups
    if (!bBanned && (nBannedType&BANNED_JUMPBOOTS))
        bBanned = nType == kItemJumpBoots;
    if (!bBanned && (nBannedType&BANNED_CLOAK))
        bBanned = nType == kItemShadowCloak;
    if (!bBanned && (nBannedType&BANNED_DEATHMASK))
        bBanned = nType == kItemDeathMask;
    if (!bBanned && (nBannedType&BANNED_AKIMBO))
        bBanned = nType == kItemTwoGuns;
    if (!bBanned && (nBannedType&BANNED_REFLECT))
        bBanned = nType == kItemReflectShots;

    return bBanned;
}

char dbIsBannedSprite(spritetype *pSprite, XSPRITE* pXSprite)
{
    if (gGameOptions.uSpriteBannedFlags == BANNED_NONE) // no sprites banned, return
        return 0;
    const char bRemove = dbIsBannedSpriteType(pSprite->type);
    if (bRemove && IsDudeSprite(pSprite) && pXSprite)
    {
        if (pXSprite->key > 0) // drop key
            actDropObject(pSprite, kItemKeyBase + (pXSprite->key - 1));
        if (pXSprite->dropMsg > 0) // drop item
            actDropObject(pSprite, pXSprite->dropMsg);
    }
    return bRemove;
}

static uint32_t curRandomizerSeed = 0;
static uint32_t curRandomizerSeedDudes = 0;
static uint32_t curRandomizerSeedThings = 0;

inline int dbRandomizerRNG(const int range)
{
    curRandomizerSeed = (214013 * curRandomizerSeed + 2531011);
    return mulscale15(((curRandomizerSeed >> 16) & 0x7FFF), range);
}

inline int dbRandomizerRNGDudes(const int range)
{
    curRandomizerSeedDudes = (214013 * curRandomizerSeedDudes + 2531011);
    return mulscale15(((curRandomizerSeedDudes >> 16) & 0x7FFF), range);
}

inline int dbRandomizerRNGThings(const int range)
{
    curRandomizerSeedThings = (214013 * curRandomizerSeedThings + 2531011);
    return mulscale15(((curRandomizerSeedThings >> 16) & 0x7FFF), range);
}

void dbRandomizerModeInit(void)
{
    // if adding a new seed cheat, ensure that you do the following:
    // - add the new entry to the switch statement in dbRandomizerMode()
    // - updating range cheat range for dbRandomizerModeScale()
    const char randomizerCheats[][sizeof(gzRandomizerSeed)] =
    {
        "AAAAAAAA", // phantoms only
        "BUTCHERS", // butchers only
        "SOULSEEK", // hands only
        "EPISODE6", // cultists only
        "GARGOYLE", // gargoyles only
        "FLAMEDOG", // hell hounds only
        "CAPYBARA", // rats only
        "HURTSCAN", // shotgun/tommy gun cultists only
        "HUGEFISH", // gill beasts only
        "SHOCKING", // tesla cultists only
        "CRUONITA", // boss types only
        "BILLYRAY", // shotgun cultists only
        "WEED420!", // cultists only but they're green (and make you dizzy on damage)
        "BRAAAINS", // zombies only
        "OKBOOMER", // tnt cultists only
        "OKZOOMER", // tnt/tesla cultists only
        "SNEAKYFU", // prone shotgun/tommy gun cultists only
    };

    const uint32_t defaultSeed = 0xCA1EB666;
    gGameOptions.nRandomizerCheat = -1;
    if (gGameOptions.szRandomizerSeed[0] == '\0') // if seed is empty, generate a new one
    {
        if (gGameOptions.nGameType != kGameTypeSinglePlayer) // if in multiplayer, use a failsafe seed
            curRandomizerSeed = defaultSeed;
        else // in single-player
            curRandomizerSeed = QRandom2(defaultSeed);
        curRandomizerSeedThings = curRandomizerSeedDudes = curRandomizerSeed;
        return;
    }

    for (int curCheat = 0; curCheat < (int)ARRAY_SSIZE(randomizerCheats); curCheat++) // compare seed against any cheat seed entries
    {
        for (int i = 0; i < (int)sizeof(gzRandomizerSeed); i++)
        {
            const char charSeed = toupper(gGameOptions.szRandomizerSeed[i]);
            const char charSeedCheat = toupper(randomizerCheats[curCheat][i]);
            if (charSeed != charSeedCheat) // if encountered a non-matching character, stop and check next entry
            {
                gGameOptions.nRandomizerCheat = -1;
                break;
            }
            else if ((charSeedCheat == '\0') || ((i+1) == (int)sizeof(gzRandomizerSeed))) // if matched cheat seed completely and reached end of string
            {
                gGameOptions.nRandomizerCheat = curCheat; // set to seed cheat index
                break;
            }
        }
        if (gGameOptions.nRandomizerCheat != -1) // if found a match, stop searching
            break;
    }

    if ((gGameOptions.nRandomizerCheat != -1) && (gGameOptions.nGameType == kGameTypeSinglePlayer)) // if seed cheat is active and in single-player
        curRandomizerSeed = qrand(); // always re-roll random seed when cheat seed mode is active
    else
        curRandomizerSeed = defaultSeed; // reset seed
    for (int i = 0; i < (int)sizeof(gGameOptions.szRandomizerSeed); i++) // shitty seed system, but if it works for the N64's CIC then who am I to judge
    {
        if (gGameOptions.szRandomizerSeed[i] == '\0')
            break;
        curRandomizerSeed += ~(toupper(gGameOptions.szRandomizerSeed[i]));
    }
    curRandomizerSeedThings = curRandomizerSeedDudes = curRandomizerSeed;
}

void dbRandomizerMode(spritetype *pSprite)
{
    if (pSprite == NULL) // invalid sprite, don't bother processing
        return;
    if (!spriRangeIsFine(pSprite->index))
        return;

    if ((gGameOptions.nRandomizerMode >= 2) && (pSprite->type == kItemBeastVision)) // always replace beast vision if pickups or enemies+pickups mode
    {
        const int pickupsrngtype[] = {kItemArmorBasic, kItemArmorBody, kItemArmorFire, kItemWeaponFlarePistol, kItemWeaponSawedoff, kItemWeaponTommygun, kItemAmmoTNTBox, kItemAmmoSawedoffBox, kItemAmmoTommygunDrum};
        const int pickupsrngpicnum[] = {2628, 2586, 2578, 524, 559, 558, 809, 812, 817};
        const int rng = dbRandomizerRNGThings(ARRAY_SSIZE(pickupsrngtype));
        pSprite->type = pickupsrngtype[rng];
        pSprite->picnum = pickupsrngpicnum[rng];
        if ((pSprite->type < kThingBase) || (pSprite->type >= kThingMax)) // update sprite type
            changespritestat(pSprite->index, kStatItem);
    }

    if ((gGameOptions.nRandomizerMode & 1) && (gGameOptions.nRandomizerCheat != -1)) // if enemies or enemies+weapons mode, and seed cheat is active, replace enemy
    {
        const int type = pSprite->type;
#ifdef NOONE_EXTENSIONS
        if (type == kDudeModernCustom) // ignore custom dude types
            return;
#endif
        if ((type >= kDudeCultistTommy) && (type <= kDudeBurningBeast) && !(type >= kDudePlayer1 && type <= kDudePlayer8) && (type != kDudeCultistReserved) && (type != kDudeBeast) && (type != kDudeCultistBeast) && (type != kDudeGargoyleStone) && (type != kDudeTchernobog) && (type != kDudeCerberusTwoHead) && (type != kDudeCerberusOneHead) && (type != kDudeSpiderMother)) // filter problematic enemy types
        {
            pSprite->flags |= kPhysGravity;
            switch (gGameOptions.nRandomizerCheat) // replace enemy according to cheat type
            {
            case  0: // "AAAAAAAA" - phantoms only
                pSprite->type = kDudePhantasm;
                break;
            case  1: // "BUTCHERS" - butchers only
                pSprite->type = kDudeZombieButcher;
                break;
            case  2: // "SOULSEEK" - hands only
                pSprite->type = kDudeHand;
                break;
            case  3: // "EPISODE6" - cultists only
            {
                const int enemiesrng[] = {kDudeCultistTommy, kDudeCultistShotgun, kDudeCultistTommyProne, kDudeCultistShotgunProne, kDudeCultistTesla, kDudeCultistTNT};
                pSprite->type = enemiesrng[dbRandomizerRNGDudes(ARRAY_SSIZE(enemiesrng))];
                break;
            }
            case  4: // "GARGOYLE" - gargoyles only
                pSprite->type = dbRandomizerRNGDudes(20) ? kDudeGargoyleFlesh : kDudeGargoyleStone;
                pSprite->flags &= ~kPhysGravity;
                break;
            case  5: // "FLAMEDOG" - hell hounds only
                pSprite->type = dbRandomizerRNGDudes(20) ? kDudeHellHound : kDudeCerberusOneHead;
                break;
            case  6: // "CAPYBARA" - rats only
                pSprite->type = kDudeRat;
                break;
            case  7: // "HURTSCAN" - shotgun/tommy gun cultists only
            {
                const int enemiesrng[] = {kDudeCultistTommy, kDudeCultistShotgun, kDudeCultistTommyProne, kDudeCultistShotgunProne};
                pSprite->type = enemiesrng[dbRandomizerRNGDudes(ARRAY_SSIZE(enemiesrng))];
                break;
            }
            case  8: // "HUGEFISH" - gill beasts only
                pSprite->type = kDudeGillBeast;
                break;
            case  9: // "SHOCKING" - tesla cultists only
                pSprite->type = kDudeCultistTesla;
                break;
            case 10: // "CRUONITA" - boss types only
            {
                const int enemiesrng[] = {kDudeCultistBeast, kDudeTchernobog, kDudeCerberusTwoHead, kDudeSpiderMother, kDudeGargoyleStone};
                pSprite->type = enemiesrng[dbRandomizerRNGDudes(ARRAY_SSIZE(enemiesrng))];
                break;
            }
            case 11: // "BILLYRAY" - shotgun cultists only
            {
                const int enemiesrng[] = {kDudeCultistShotgun, kDudeCultistShotgunProne};
                pSprite->type = enemiesrng[dbRandomizerRNGDudes(ARRAY_SSIZE(enemiesrng))];
                break;
            }
            case 12: // "WEED420!" - cultists only but they're green (and make you dizzy on damage)
            {
                const int enemiesrng[] = {kDudeCultistShotgun, kDudeCultistShotgunProne, kDudeCultistTNT};
                pSprite->type = enemiesrng[dbRandomizerRNGDudes(ARRAY_SSIZE(enemiesrng))];
                pSprite->pal = 4; // DUDE WEED XDDD 420 bro
                break;
            }
            case 13: // "BRAAAINS" - zombies only
                pSprite->type = kDudeZombieAxeNormal;
                break;
            case 14: // "OKBOOMER" - tnt cultists only
                pSprite->type = kDudeCultistTNT;
                break;
            case 15: // "OKZOOMER" - tnt/tesla cultists only
            {
                const int enemiesrng[] = {kDudeCultistTesla, kDudeCultistTNT};
                pSprite->type = enemiesrng[dbRandomizerRNGDudes(ARRAY_SSIZE(enemiesrng))];
                break;
            }
            case 16: // "SNEAKYFU" - prone shotgun/tommy gun cultists only
            {
                const int enemiesrng[] = {kDudeCultistShotgunProne, kDudeCultistTommyProne};
                pSprite->type = enemiesrng[dbRandomizerRNGDudes(ARRAY_SSIZE(enemiesrng))];
                break;
            }
            default: // unknown cheat id, don't do anything
            {
                static char bShownError = 0;
                if (!bShownError) // only show once per session
                    LOG_F(WARNING, "Error invalid cheat seed %s (%d)\nAdd seed cheat effect to func dbRandomizerMode()", gGameOptions.szRandomizerSeed, gGameOptions.nRandomizerCheat);
                bShownError = 1;
                break;
            }
            }
            if (gGameOptions.nRandomizerCheat != 12)
                pSprite->pal = 0;
            pSprite->cstat &= ~CSTAT_SPRITE_YFLIP;
            return;
        }
    }

    if (!dbRandomizerRNG(5)) return;

    if (gGameOptions.nRandomizerMode & 1) // if enemies or enemies+weapons mode, randomize enemy
    {
        switch (pSprite->type)
        {
        case kDudeInnocent:
        {
            const int enemiesrng[] = {kDudeBat, kDudeRat, kDudeZombieAxeNormal, kDudeGillBeast, kDudeHand, kDudeCultistShotgunProne};
            pSprite->type = enemiesrng[dbRandomizerRNGDudes(ARRAY_SSIZE(enemiesrng))];
            break;
        }
        case kDudeBat:
        {
            const int enemiesrng[] = {kDudeRat, kDudeZombieAxeLaying, kDudeHand, kDudeInnocent, kDudeCultistTNT, kDudePhantasm};
            pSprite->type = enemiesrng[dbRandomizerRNGDudes(ARRAY_SSIZE(enemiesrng))];
            pSprite->cstat &= ~CSTAT_SPRITE_YFLIP;
            break;
        }
        case kDudeRat:
        {
            const int enemiesrng[] = {kDudeBat, kDudeZombieAxeBuried, kDudeHand, kDudeTinyCaleb, kDudeSpiderBrown, kDudeSpiderRed};
            pSprite->type = enemiesrng[dbRandomizerRNGDudes(ARRAY_SSIZE(enemiesrng))];
            break;
        }
        case kDudeZombieAxeBuried:
        {
            const int enemiesrng[] = {kDudeSpiderRed, kDudeZombieAxeLaying, kDudeGillBeast, kDudeZombieButcher, kDudeCultistShotgun, kDudeCultistTommy, kDudeCultistTNT};
            pSprite->type = enemiesrng[dbRandomizerRNGDudes(ARRAY_SSIZE(enemiesrng))];
            break;
        }
        case kDudeZombieAxeLaying:
        {
            const int enemiesrng[] = {kDudeSpiderRed, kDudeZombieAxeBuried, kDudeGillBeast, kDudeZombieButcher, kDudeCultistShotgunProne, kDudeCultistTNT};
            pSprite->type = enemiesrng[dbRandomizerRNGDudes(ARRAY_SSIZE(enemiesrng))];
            break;
        }
        case kDudeZombieAxeNormal:
        {
            const int enemiesrng[] = {kDudeZombieAxeBuried, kDudeZombieAxeLaying, kDudeGillBeast, kDudeZombieButcher, kDudeCultistShotgun, kDudeCultistTommy, kDudeTinyCaleb, kDudeCultistTNT};
            pSprite->type = enemiesrng[dbRandomizerRNGDudes(ARRAY_SSIZE(enemiesrng))];
            break;
        }
        case kDudeBoneEel:
        {
            const int enemiesrng[] = {kDudeZombieAxeNormal, kDudeGillBeast, kDudeHand, kDudeCultistShotgunProne, kDudeInnocent, kDudeCultistTNT};
            pSprite->type = enemiesrng[dbRandomizerRNGDudes(ARRAY_SSIZE(enemiesrng))];
            break;
        }
        case kDudeGillBeast:
        {
            const int enemiesrng[] = {kDudeZombieAxeNormal, kDudeZombieAxeLaying, kDudeZombieButcher, kDudeCultistShotgun, kDudeCultistTommy, kDudeCultistTNT, kDudeGargoyleFlesh, kDudeCultistShotgunProne, kDudeCultistTesla};
            pSprite->type = enemiesrng[dbRandomizerRNGDudes(ARRAY_SSIZE(enemiesrng))];
            break;
        }
        case kDudeZombieButcher:
        {
            const int enemiesrng[] = {kDudeZombieAxeNormal, kDudeZombieAxeLaying, kDudeGillBeast, kDudeCultistShotgun, kDudeCultistTommy, kDudeCultistTNT, kDudeGargoyleFlesh, kDudePhantasm, kDudeCultistShotgunProne, kDudeHellHound, kDudeCultistTommyProne, kDudeCultistTesla};
            pSprite->type = enemiesrng[dbRandomizerRNGDudes(ARRAY_SSIZE(enemiesrng))];
            break;
        }
        case kDudeSpiderBrown:
        case kDudeSpiderRed:
        {
            const int enemiesrng[] = {kDudeHand, kDudeTinyCaleb, kDudeCultistTNT, kDudeHellHound, kDudeCultistTommyProne, kDudeCultistShotgunProne};
            pSprite->type = enemiesrng[dbRandomizerRNGDudes(ARRAY_SSIZE(enemiesrng))];
            break;
        }
        case kDudeSpiderBlack:
        {
            const int enemiesrng[] = {kDudeCultistTNT, kDudeGargoyleFlesh, kDudeCultistShotgunProne, kDudeHellHound, kDudeCultistTommyProne, kDudeCultistTesla, kDudeSpiderMother};
            pSprite->type = enemiesrng[dbRandomizerRNGDudes(ARRAY_SSIZE(enemiesrng))];
            break;
        }
        case kDudePhantasm:
        {
            const int enemiesrng[] = {kDudeBat, kDudeHellHound, kDudeZombieButcher, kDudeCultistTNT, kDudeCultistTommyProne, kDudeGillBeast};
            pSprite->type = enemiesrng[dbRandomizerRNGDudes(ARRAY_SSIZE(enemiesrng))];
            break;
        }
        case kDudeHand:
        case kDudeTinyCaleb:
        {
            const int enemiesrng[] = {kDudeSpiderBrown, kDudeSpiderRed, kDudeCultistTNT, kDudePhantasm, kDudeHellHound, kDudeCultistTommyProne, kDudeCultistShotgunProne, kDudeGillBeast, kDudeZombieAxeLaying};
            pSprite->type = enemiesrng[dbRandomizerRNGDudes(ARRAY_SSIZE(enemiesrng))];
            break;
        }
        case kDudeTentacleGreen:
        case kDudeTentacleFire:
        {
            const int enemiesrng[] = {kDudePodFire, kDudePodGreen, kDudeCultistShotgunProne, kDudeCultistTommyProne, kDudeZombieAxeBuried};
            pSprite->type = enemiesrng[dbRandomizerRNGDudes(ARRAY_SSIZE(enemiesrng))];
            break;
        }
        case kDudeCultistShotgun:
        case kDudeCultistTommy:
        {
            const int enemiesrng[] = {kDudeInnocent, kDudeZombieAxeBuried, kDudeZombieAxeNormal, kDudeGillBeast, kDudeCultistTesla, kDudeCultistTNT, kDudeCultistShotgun, kDudeCultistShotgunProne, kDudeCultistTommyProne};
            pSprite->type = enemiesrng[dbRandomizerRNGDudes(ARRAY_SSIZE(enemiesrng))];
            break;
        }
        case kDudeCultistShotgunProne:
        case kDudeCultistTommyProne:
        {
            const int enemiesrng[] = {kDudeGillBeast, kDudeZombieAxeLaying, kDudeSpiderBlack, kDudeCultistShotgun, kDudeCultistTommy, kDudeCultistTNT, kDudeHellHound, kDudeCultistTesla};
            pSprite->type = enemiesrng[dbRandomizerRNGDudes(ARRAY_SSIZE(enemiesrng))];
            break;
        }
        case kDudeCultistTNT:
        {
            const int enemiesrng[] = {kDudeZombieAxeNormal, kDudeGillBeast, kDudeZombieAxeLaying, kDudeCultistTommy, kDudeCultistShotgunProne, kDudeHellHound, kDudeCultistTommyProne, kDudeCultistTesla};
            pSprite->type = enemiesrng[dbRandomizerRNGDudes(ARRAY_SSIZE(enemiesrng))];
            break;
        }
        case kDudeGargoyleFlesh:
        {
            const int enemiesrng[] = {kDudeBat, kDudeCultistTNT, kDudeHellHound, kDudeCultistTommyProne, kDudeCultistTesla};
            pSprite->type = enemiesrng[dbRandomizerRNGDudes(ARRAY_SSIZE(enemiesrng))];
            break;
        }
        case kDudeHellHound:
        {
            const int enemiesrng[] = {kDudeCultistTNT, kDudeCultistTommyProne, kDudeCultistTesla, kDudeGargoyleFlesh};
            pSprite->type = enemiesrng[dbRandomizerRNGDudes(ARRAY_SSIZE(enemiesrng))];
            break;
        }
        case kDudeCultistTesla:
        {
            const int enemiesrng[] = {kDudeCultistTesla, kDudeCultistTesla, kDudeCultistTesla, kDudeGargoyleStone, kDudeCerberusOneHead, kDudeCultistTommyProne, kDudeCultistTNT};
            pSprite->type = enemiesrng[dbRandomizerRNGDudes(ARRAY_SSIZE(enemiesrng))];
            break;
        }
        case kDudeCerberusOneHead:
        case kDudeCerberusTwoHead:
        {
            const int enemiesrng[] = {kDudeCerberusTwoHead, kDudeGargoyleStone, kDudeTchernobog, kDudeHellHound, kDudeBeast};
            pSprite->type = enemiesrng[dbRandomizerRNGDudes(ARRAY_SSIZE(enemiesrng))];
            break;
        }
        default:
            break;
        }
        if (pSprite->type == kDudeGargoyleFlesh || pSprite->type == kDudeGargoyleStone || pSprite->type == kDudeBat) // set the correct flags needed for flying enemies
            pSprite->flags &= ~kPhysGravity;
        else
            pSprite->flags |= kPhysGravity;
        pSprite->pal = 0;
    }
    if (gGameOptions.nRandomizerMode >= 2) // if pickups or enemies+pickups mode, randomize pickup
    {
        const int nPicnumOrig = pSprite->picnum;
        int nTopOrig, nBottomOrig;
        GetSpriteExtents(pSprite, &nTopOrig, &nBottomOrig);

        switch (pSprite->type)
        {
        case kItemWeaponFlarePistol:
        {
            const int pickupsrngtype[] = {kItemAmmoSprayCan, kItemWeaponTommygun, kItemWeaponTNT, kItemHealthLifeEssense};
            const int pickupsrngpicnum[] = {618, 558, 589, 2169};
            const int rng = dbRandomizerRNGThings(ARRAY_SSIZE(pickupsrngtype));
            pSprite->type = pickupsrngtype[rng];
            pSprite->picnum = pickupsrngpicnum[rng];
            break;
        }
        case kItemWeaponSawedoff:
        {
            const int pickupsrngtype[] = {kItemWeaponTommygun, kItemAmmoSprayCan, kItemAmmoTNTBox};
            const int pickupsrngpicnum[] = {558, 618, 809};
            const int rng = dbRandomizerRNGThings(ARRAY_SSIZE(pickupsrngtype));
            pSprite->type = pickupsrngtype[rng];
            pSprite->picnum = pickupsrngpicnum[rng];
            break;
        }
        case kItemWeaponTommygun:
        {
            const int pickupsrngtype[] = {kItemWeaponSawedoff, kItemWeaponFlarePistol, kItemAmmoTNTBox};
            const int pickupsrngpicnum[] = {559, 524, 809};
            const int rng = dbRandomizerRNGThings(ARRAY_SSIZE(pickupsrngtype));
            pSprite->type = pickupsrngtype[rng];
            pSprite->picnum = pickupsrngpicnum[rng];
            break;
        }
        case kItemAmmoTNTBox:
        {
            const int pickupsrngtype[] = {kItemAmmoSprayCan, kItemAmmoRemoteBombBundle, kItemAmmoTNTBox};
            const int pickupsrngpicnum[] = {618, 810, 809};
            const int rng = dbRandomizerRNGThings(ARRAY_SSIZE(pickupsrngtype));
            pSprite->type = pickupsrngtype[rng];
            pSprite->picnum = pickupsrngpicnum[rng];
            break;
        }
        case kItemWeaponTNT:
        case kItemAmmoTNTBundle:
        case kItemAmmoProxBombBundle:
        case kItemAmmoRemoteBombBundle:
        {
            const int pickupsrngtype[] = {kItemAmmoSprayCan, kItemAmmoSawedoffFew, kItemAmmoTommygunFew};
            const int pickupsrngpicnum[] = {618, 619, 813};
            const int rng = dbRandomizerRNGThings(ARRAY_SSIZE(pickupsrngtype));
            pSprite->type = pickupsrngtype[rng];
            pSprite->picnum = pickupsrngpicnum[rng];
            break;
        }
        case kItemWeaponSprayCan:
        {
            const int pickupsrngtype[] = {kItemWeaponSawedoff, kItemAmmoTommygunFew};
            const int pickupsrngpicnum[] = {559, 813};
            const int rng = dbRandomizerRNGThings(ARRAY_SSIZE(pickupsrngtype));
            pSprite->type = pickupsrngtype[rng];
            pSprite->picnum = pickupsrngpicnum[rng];
            break;
        }
        case kItemWeaponVoodooDoll:
        case kItemAmmoTrappedSoul:
        {
            const int pickupsrngtype[] = {kItemWeaponLifeLeech, kItemWeaponNapalmLauncher, kItemWeaponSawedoff, kItemHealthLifeSeed, kItemTwoGuns, kItemReflectShots, kItemWeaponTeslaCannon};
            const int pickupsrngpicnum[] = {800, 526, 559, 2433, 829, 2428, 539};
            const int rng = dbRandomizerRNGThings(ARRAY_SSIZE(pickupsrngtype));
            pSprite->type = pickupsrngtype[rng];
            pSprite->picnum = pickupsrngpicnum[rng];
            break;
        }
        case kItemWeaponTeslaCannon:
        {
            const int pickupsrngtype[] = {kItemWeaponTommygun, kItemHealthLifeSeed, kItemShadowCloak, kItemReflectShots, kItemWeaponVoodooDoll};
            const int pickupsrngpicnum[] = {558, 2433, 896, 2428, 525};
            const int rng = dbRandomizerRNGThings(ARRAY_SSIZE(pickupsrngtype));
            pSprite->type = pickupsrngtype[rng];
            pSprite->picnum = pickupsrngpicnum[rng];
            break;
        }
        case kItemWeaponNapalmLauncher:
        case kItemWeaponLifeLeech:
        case kItemArmorSuper:
        {
            const int pickupsrngtype[] = {kItemTwoGuns, kItemReflectShots, kItemWeaponVoodooDoll};
            const int pickupsrngpicnum[] = {829, 2428, 525};
            const int rng = dbRandomizerRNGThings(ARRAY_SSIZE(pickupsrngtype));
            pSprite->type = pickupsrngtype[rng];
            pSprite->picnum = pickupsrngpicnum[rng];
            break;
        }
        case kItemHealthDoctorBag:
        {
            const int pickupsrngtype[] = {kItemHealthLifeEssense, kItemHealthLifeSeed, kItemAmmoSawedoffBox, kItemWeaponTeslaCannon};
            const int pickupsrngpicnum[] = {2169, 2433, 812, 539};
            const int rng = dbRandomizerRNGThings(ARRAY_SSIZE(pickupsrngtype));
            pSprite->type = pickupsrngtype[rng];
            pSprite->picnum = pickupsrngpicnum[rng];
            break;
        }
        case kItemAmmoFlares:
        {
            const int pickupsrngtype[] = {kItemAmmoSprayCan, kItemAmmoRemoteBombBundle, kItemAmmoTNTBundle, kItemAmmoSprayCan, kItemAmmoTNTBox, kItemAmmoGasolineCan};
            const int pickupsrngpicnum[] = {618, 810, 589, 618, 809, 801};
            const int rng = dbRandomizerRNGThings(ARRAY_SSIZE(pickupsrngtype));
            pSprite->type = pickupsrngtype[rng];
            pSprite->picnum = pickupsrngpicnum[rng];
            break;
        }
        case kItemAmmoSawedoffBox:
        {
            const int pickupsrngtype[] = {kItemAmmoTeslaCharge, kItemWeaponSawedoff, kItemAmmoSawedoffBox};
            const int pickupsrngpicnum[] = {548, 559, 812};
            const int rng = dbRandomizerRNGThings(ARRAY_SSIZE(pickupsrngtype));
            pSprite->type = pickupsrngtype[rng];
            pSprite->picnum = pickupsrngpicnum[rng];
            break;
        }
        case kItemAmmoSawedoffFew:
        {
            const int pickupsrngtype[] = {kItemAmmoRemoteBombBundle, kItemAmmoTeslaCharge, kItemAmmoTommygunDrum};
            const int pickupsrngpicnum[] = {810, 548, 817};
            const int rng = dbRandomizerRNGThings(ARRAY_SSIZE(pickupsrngtype));
            pSprite->type = pickupsrngtype[rng];
            pSprite->picnum = pickupsrngpicnum[rng];
            break;
        }
        case kItemAmmoTommygunFew:
        {
            const int pickupsrngtype[] = {kItemAmmoRemoteBombBundle, kItemAmmoTNTBundle, kItemAmmoSawedoffFew};
            const int pickupsrngpicnum[] = {810, 589, 619};
            const int rng = dbRandomizerRNGThings(ARRAY_SSIZE(pickupsrngtype));
            pSprite->type = pickupsrngtype[rng];
            pSprite->picnum = pickupsrngpicnum[rng];
            break;
        }
        case kItemArmorBasic:
        case kItemArmorBody:
        case kItemArmorFire:
        case kItemArmorSpirit:
        {
            const int pickupsrngtype[] = {kItemArmorBasic, kItemArmorBody, kItemArmorFire, kItemArmorSpirit, kItemArmorSuper, kItemHealthLifeEssense, kItemHealthLifeSeed, kItemTwoGuns, kItemReflectShots, kItemShadowCloak, kItemHealthDoctorBag};
            const int pickupsrngpicnum[] = {2628, 2586, 2578, 2602, 2594, 2169, 2433, 829, 2428, 896, 519};
            const int rng = dbRandomizerRNGThings(ARRAY_SSIZE(pickupsrngtype));
            pSprite->type = pickupsrngtype[rng];
            pSprite->picnum = pickupsrngpicnum[rng];
            break;
        }
        case kItemTwoGuns:
        case kItemReflectShots:
        {
            const int pickupsrngtype[] = {kItemTwoGuns, kItemReflectShots};
            const int pickupsrngpicnum[] = {829, 2428};
            const int rng = dbRandomizerRNGThings(ARRAY_SSIZE(pickupsrngtype));
            pSprite->type = pickupsrngtype[rng];
            pSprite->picnum = pickupsrngpicnum[rng];
            break;
        }
        case kThingObjectGib: // generic breakable objects
        {
            if (pSprite->statnum != kStatThing) // unexpected type, don't replace
                break;
            if (xspriRangeIsFine(pSprite->extra))
            {
                XSPRITE *pXSprite = &xsprite[pSprite->extra];
                if (pXSprite->key > 0) // sprite has key attached, don't replace
                    break;
                if (pXSprite->dropMsg > 0) // item has item attached, don't replace
                    break;
            }
            switch (pSprite->picnum)
            {
                case 520: // skull cup
                case 521: // wine glass (half-empty)
                case 574: // wine glass (empty)
                case 759: // wine bottle
                {
                    const int pickupsrngtype[] = {kItemAmmoSawedoffFew, kItemAmmoTommygunFew, kItemAmmoTNTBundle};
                    const int pickupsrngpicnum[] = {619, 813, 589};
                    const int rng = dbRandomizerRNGThings(ARRAY_SSIZE(pickupsrngtype));
                    pSprite->type = pickupsrngtype[rng];
                    pSprite->picnum = pickupsrngpicnum[rng];
                    changespritestat(pSprite->index, kStatItem);
                    break;
                }
                case 605: // glass bottle
                case 606: // glass bottle
                case 616: // glass jug
                {
                    const int pickupsrngtype[] = {kItemAmmoSawedoffFew, kItemAmmoTommygunFew, kItemAmmoTommygunDrum, kItemAmmoSawedoffBox, kItemAmmoTNTBox};
                    const int pickupsrngpicnum[] = {619, 813, 817, 812, 809};
                    const int rng = dbRandomizerRNGThings(ARRAY_SSIZE(pickupsrngtype));
                    pSprite->type = pickupsrngtype[rng];
                    pSprite->picnum = pickupsrngpicnum[rng];
                    changespritestat(pSprite->index, kStatItem);
                    break;
                }
                case 834: // montana jar
                case 75: // brain in jar
                case 76: // brain in jar
                {
                    if (dbRandomizerRNGThings(2)) // lower chance of replacing object
                        break;
                    const int pickupsrngtype[] = {kItemHealthLifeEssense, kItemAmmoTommygunDrum, kItemAmmoSawedoffBox, kItemAmmoTNTBox};
                    const int pickupsrngpicnum[] = {2169, 817, 812, 809};
                    const int rng = dbRandomizerRNGThings(ARRAY_SSIZE(pickupsrngtype));
                    pSprite->type = pickupsrngtype[rng];
                    pSprite->picnum = pickupsrngpicnum[rng];
                    changespritestat(pSprite->index, kStatItem);
                    break;
                }
                case 563: // zombie barrel
                case 201: // toxic waste
                {
                    if (dbRandomizerRNGThings(5)) // lower chance of replacing object
                        break;
                    pSprite->type = kThingTNTBarrel; // replace with tnt barrel
                    pSprite->picnum = 907;
                    changespritestat(pSprite->index, kStatThing);
                    break;
                }
                default:
                    break;
            }
        }
        default:
            break;
        }

        if (pSprite->picnum != nPicnumOrig) // if changed sprite type and picnum, readjust position
        {
            int top, bottom;
            GetSpriteExtents(pSprite, &top, &bottom);
            if (bottom >= nBottomOrig)
                pSprite->z -= bottom - nBottomOrig;
        }
    }
}

void dbRandomizerModeScale(spritetype *pSprite, XSPRITE* pXSprite)
{
    const bool randomCheatActive = (gGameOptions.nRandomizerCheat >= 0) && (gGameOptions.nRandomizerCheat <= 15); // only randomize enemy sizes if seed cheats 0-15 are active
    if (!randomCheatActive)
        return;
    if (!pXSprite->scale && !dbRandomizerRNGDudes(3)) { // randomly change enemy scale (only if enemy does not have scale set already)
        switch (pSprite->type) { // make enemies randomly huge
            case kDudeRat:
                pXSprite->scale = ClipRange(dbRandomizerRNGDudes(2048-128)+128, 128, 2048);
                break;
            case kDudeHand:
                pXSprite->scale = ClipRange(dbRandomizerRNGDudes(1024-128)+128, 128, 1024);
                break;
            case kDudeHellHound:
                pXSprite->scale = ClipRange(dbRandomizerRNGDudes(512-128)+128, 128, 512);
                break;
            case kDudeCultistBeast: // boss types
            case kDudeTchernobog:
            case kDudeCerberusTwoHead:
            case kDudeCerberusOneHead:
            case kDudeSpiderMother:
            case kDudeGargoyleStone:
                pXSprite->scale = ClipRange(dbRandomizerRNGDudes(512-32)+32, 32, 512);
                break;
            case kDudeGargoyleFlesh:
                pXSprite->scale = ClipRange(dbRandomizerRNGDudes(512-32)+32, 32, 512);
                break;
            case kDudeCultistShotgun: // regular enemies
            case kDudeCultistTommy:
            case kDudeCultistTommyProne:
            case kDudeCultistShotgunProne:
                pXSprite->scale = ClipRange(dbRandomizerRNGDudes(384-128)+128, 128, 384);
                break;
            case kDudeCultistTNT:
                pXSprite->scale = ClipRange(dbRandomizerRNGDudes(384-96)+96, 96, 256);
                break;
            case kDudeZombieButcher:
            case kDudeGillBeast:
                pXSprite->scale = ClipRange(dbRandomizerRNGDudes(384-64)+64, 64, 384);
                break;
            default:
                break;
        }
    }
}

inline int dbShuffleEnemyList(spritetype **pSpriteList = NULL)
{
    int nSprites = 0;
    for (int i = headspritestat[kStatDude]; i >= 0; i = nextspritestat[i])
    {
        if (!IsDudeSprite(&sprite[i]) || IsPlayerSprite(&sprite[i])) // not an enemy sprite, skip
            continue;
        const int type = sprite[i].type;
        if (!((type > kDudeBase) && (type < kDudeMax)))
            continue;
        switch (type) // filter problematic enemy types
        {
        case kDudeBurningBeast:
        case kDudeBurningInnocent:
        case kDudeBurningCultist:
        case kDudeBurningZombieAxe:
        case kDudeBurningZombieButcher:
        case kDudeBurningTinyCaleb:
        case kDudeCultistReserved:
        case kDudeBeast:
        case kDudeCultistBeast:
        case kDudeGargoyleStone:
        case kDudeTchernobog:
        case kDudeCerberusTwoHead:
        case kDudeCerberusOneHead:
        case kDudeSpiderMother:
        case kDudeBoneEel:
        case kDudePlayer1:
        case kDudePlayer2:
        case kDudePlayer3:
        case kDudePlayer4:
        case kDudePlayer5:
        case kDudePlayer6:
        case kDudePlayer7:
        case kDudePlayer8:
            continue;
        default:
            break;
        }
        if (pSpriteList)
            pSpriteList[nSprites] = &sprite[i];
        nSprites++;
    }
    return nSprites;
}

void dbShuffleEnemy(void)
{
    int nSprites = dbShuffleEnemyList(); // get total enemies to shuffle
    if (nSprites < 2) // only two enemies in the level, abort
        return;

    nSprites = ClipRange(nSprites, 0, kMaxSprites-1);
    spritetype **pSprite = (spritetype **)Bmalloc((nSprites+1) * sizeof(spritetype));
    if (!pSprite)
        return;
    dbShuffleEnemyList(pSprite); // assign sprites to pointer array

    for (int i = 0; i < nSprites; i++) // shuffle enemies
    {
        int j = qrand() % nSprites;
        if (i == j)
        {
            j = qrand() % nSprites; // re-roll
            if (i == j) // bah! just our luck...
                continue;
        }

        const int16_t tempType = pSprite[j]->type;
        pSprite[j]->type = pSprite[i]->type;
        pSprite[i]->type = tempType;

        const int16_t tempPicnum = pSprite[j]->picnum;
        pSprite[j]->picnum = pSprite[i]->picnum;
        pSprite[i]->picnum = tempPicnum;

        const uint8_t tempPal = pSprite[j]->pal;
        pSprite[j]->pal = pSprite[i]->pal;
        pSprite[i]->pal = tempPal;

        const uint16_t tempCstat = pSprite[j]->cstat;
        pSprite[j]->cstat = pSprite[i]->cstat;
        pSprite[i]->cstat = tempCstat;

        const int16_t tempAng = pSprite[j]->ang;
        pSprite[j]->ang = pSprite[i]->ang;
        pSprite[i]->ang = tempAng;

        const int16_t tempOwner = pSprite[j]->owner;
        pSprite[j]->owner = pSprite[i]->owner;
        pSprite[i]->owner = tempOwner;

        const int16_t tempInittype = pSprite[j]->inittype;
        pSprite[j]->inittype = pSprite[i]->inittype;
        pSprite[i]->inittype = tempInittype;
    }
    Bfree(pSprite);
}

bool byte_1A76C6, byte_1A76C7, byte_1A76C8;

MAPHEADER2 byte_19AE44;

unsigned int dbReadMapCRC(const char *pPath)
{
    char name2[BMAX_PATH];
    byte_1A76C7 = 0;
    byte_1A76C8 = 0;

    int const bakpathsearchmode = pathsearchmode;
    pathsearchmode = 1;

    Bstrncpy(name2, pPath, BMAX_PATH);
    Bstrupr(name2);
    DICTNODE* pNode = *gSysRes.Probe(name2, "MAP");
    if (pNode && pNode->flags & DICT_EXTERNAL)
    {
        gSysRes.RemoveNode(pNode);
    }
    pNode = gSysRes.Lookup(pPath, "MAP");
    if (!pNode)
    {
        char name2[BMAX_PATH];
        Bstrncpy(name2, pPath, BMAX_PATH);
        ChangeExtension(name2, "");
        pNode = gSysRes.Lookup(name2, "MAP");
    }

    if (!pNode)
    {
        LOG_F(ERROR, "Error opening map file %s", pPath);
        pathsearchmode = bakpathsearchmode;
        return -1;
    }
    char *pData = (char*)gSysRes.Lock(pNode);
    pathsearchmode = bakpathsearchmode;

    int nSize = pNode->size;
    MAPSIGNATURE header;
    IOBuffer(nSize, pData).Read(&header, 6);
#if B_BIG_ENDIAN == 1
    header.version = B_LITTLE16(header.version);
#endif
    if (memcmp(header.signature, "BLM\x1a", 4))
    {
        ThrowError("Map file corrupted");
    }
    if ((header.version & 0xff00) == 0x600)
    {
    }
    else if ((header.version & 0xff00) == 0x700)
    {
        byte_1A76C8 = 1;
    }
    else
    {
        ThrowError("Map file is wrong version");
    }
    unsigned int nCRC = *(unsigned int*)(pData+nSize-4);
    gSysRes.Unlock(pNode);
    return nCRC;
}

int gMapRev, gSongId, gSkyCount;
//char byte_19AE44[128];
const int nXSectorSize = 60;
const int nXSpriteSize = 56;
const int nXWallSize = 24;

int dbLoadMap(const char *pPath, int *pX, int *pY, int *pZ, short *pAngle, short *pSector, unsigned int *pCRC) {
    char name2[BMAX_PATH]; int16_t tpskyoff[256];

    memset(show2dsector,0,sizeof(show2dsector));
    memset(show2dwall,0,sizeof(show2dwall));
    memset(show2dsprite,0,sizeof(show2dsprite));
    memset(spriteext,0,kMaxSprites*sizeof(spriteext_t));

    memset(xvel,0,sizeof(xvel));
    memset(yvel,0,sizeof(yvel));
    memset(zvel,0,sizeof(zvel));
    memset(xsprite,0,kMaxXSprites*sizeof(XSPRITE));
    memset(sprite,0,kMaxSprites*sizeof(spritetype));

    memset(qsprite_filler,0,sizeof(qsprite_filler));
    memset(qsector_filler,0,sizeof(qsector_filler));

    #ifdef NOONE_EXTENSIONS
    gModernMap = false;
    #endif

#ifdef USE_OPENGL
    Polymost_prepare_loadboard();
#endif

    int const bakpathsearchmode = pathsearchmode;
    pathsearchmode = 1;

    Bstrncpy(name2, pPath, BMAX_PATH);
    Bstrupr(name2);
    DICTNODE* pNode = *gSysRes.Probe(name2, "MAP");
    if (pNode && pNode->flags & DICT_EXTERNAL)
    {
        gSysRes.RemoveNode(pNode);
    }

    pNode = gSysRes.Lookup(pPath, "MAP");
    if (!pNode)
    {
        char name2[BMAX_PATH];
        Bstrncpy(name2, pPath, BMAX_PATH);
        ChangeExtension(name2, "");
        pNode = gSysRes.Lookup(name2, "MAP");
    }

    if (!pNode)
    {
        LOG_F(ERROR, "Error opening map file %s", pPath);
        pathsearchmode = bakpathsearchmode;
        return -1;
    }
    char *pData = (char*)gSysRes.Lock(pNode);
    pathsearchmode = bakpathsearchmode;
    int nSize = pNode->size;
    MAPSIGNATURE header;
    IOBuffer IOBuffer1 = IOBuffer(nSize, pData);
    IOBuffer1.Read(&header, 6);
#if B_BIG_ENDIAN == 1
    header.version = B_LITTLE16(header.version);
#endif
    if (memcmp(header.signature, "BLM\x1a", 4))
    {
        LOG_F(ERROR, "Map file corrupted");
        gSysRes.Unlock(pNode);
        return -1;
    }
    byte_1A76C8 = 0;
    if ((header.version & 0xff00) == 0x700) {
        byte_1A76C8 = 1;
        
        #ifdef NOONE_EXTENSIONS
        // indicate if the map requires modern features to work properly
        // for maps wich created in PMAPEDIT BETA13 or higher versions. Since only minor version changed,
        // the map is still can be loaded with vanilla BLOOD / MAPEDIT and should work in other ports too.
        int tmp = (header.version & 0x00ff);
        
        // get the modern features revision
        switch (tmp) {
            case 0x001:
                gModernMap = 1;
                break;
            case 0x002:
                gModernMap = 2;
                break;
        }
        #endif

    } else {
        LOG_F(ERROR, "Map file is wrong version");
        gSysRes.Unlock(pNode);
        return -1;
    }

    MAPHEADER mapHeader;
    IOBuffer1.Read(&mapHeader,37/* sizeof(mapHeader)*/);
    if (mapHeader.at16 != 0 && mapHeader.at16 != kMapHeaderNew && mapHeader.at16 != kMapHeaderOld) {
        dbCrypt((char*)&mapHeader, sizeof(mapHeader), kMapHeaderNew);
        byte_1A76C7 = 1;
    }

#if B_BIG_ENDIAN == 1
    mapHeader.at0 = B_LITTLE32(mapHeader.at0);
    mapHeader.at4 = B_LITTLE32(mapHeader.at4);
    mapHeader.at8 = B_LITTLE32(mapHeader.at8);
    mapHeader.atc = B_LITTLE16(mapHeader.atc);
    mapHeader.ate = B_LITTLE16(mapHeader.ate);
    mapHeader.at10 = B_LITTLE16(mapHeader.at10);
    mapHeader.at12 = B_LITTLE32(mapHeader.at12);
    mapHeader.at16 = B_LITTLE32(mapHeader.at16);
    mapHeader.at1b = B_LITTLE32(mapHeader.at1b);
    mapHeader.at1f = B_LITTLE16(mapHeader.at1f);
    mapHeader.at21 = B_LITTLE16(mapHeader.at21);
    mapHeader.at23 = B_LITTLE16(mapHeader.at23);
#endif

    psky_t *pSky = tileSetupSky(0);
    pSky->horizfrac = 65536;

    *pX = mapHeader.at0;
    *pY = mapHeader.at4;
    *pZ = mapHeader.at8;
    *pAngle = mapHeader.atc;
    *pSector = mapHeader.ate;
    pSky->lognumtiles = mapHeader.at10;
    gVisibility = g_visibility = mapHeader.at12;
    gSongId = mapHeader.at16;
    if (byte_1A76C8)
    {
        if (mapHeader.at16 == kMapHeaderNew || mapHeader.at16 == kMapHeaderOld)
        {
            byte_1A76C6 = 1;
        }
        else if (!mapHeader.at16)
        {
            byte_1A76C6 = 0;
        }
        else
        {
            LOG_F(ERROR, "Corrupted Map file");
            gSysRes.Unlock(pNode);
            return -1;
        }
    }
    else if (mapHeader.at16)
    {
        LOG_F(ERROR, "Corrupted Map file");
        gSysRes.Unlock(pNode);
        return -1;
    }
    parallaxtype = mapHeader.at1a;
    gMapRev = mapHeader.at1b;
    numsectors = mapHeader.at1f;
    numwalls = mapHeader.at21;
    dbInit();
    if (gGameOptions.nRandomizerMode && !VanillaMode())
    {
        dbRandomizerModeInit(); // seed enemy/pickup randomizer
    }
    if (byte_1A76C8)
    {
        IOBuffer1.Read(&byte_19AE44, 128);
        dbCrypt((char*)&byte_19AE44, 128, numwalls);
#if B_BIG_ENDIAN == 1
        byte_19AE44.at40 = B_LITTLE32(byte_19AE44.at40);
        byte_19AE44.at44 = B_LITTLE32(byte_19AE44.at44);
        byte_19AE44.at48 = B_LITTLE32(byte_19AE44.at48);
#endif
    }
    else
    {
        memset(&byte_19AE44, 0, 128);
    }
    gSkyCount = 1<<pSky->lognumtiles;
    IOBuffer1.Read(tpskyoff, gSkyCount*sizeof(tpskyoff[0]));
    if (byte_1A76C8)
    {
        dbCrypt((char*)tpskyoff, gSkyCount*sizeof(tpskyoff[0]), gSkyCount*2);
    }
    for (int i = 0; i < ClipHigh(gSkyCount, MAXPSKYTILES); i++)
    {
        pSky->tileofs[i] = B_LITTLE16(tpskyoff[i]);
    }
    for (int i = 0; i < numsectors; i++)
    {
        sectortype *pSector = &sector[i];
        IOBuffer1.Read(pSector, sizeof(sectortype));
        if (byte_1A76C8)
        {
            dbCrypt((char*)pSector, sizeof(sectortype), gMapRev*sizeof(sectortype));
        }
#if B_BIG_ENDIAN == 1
        pSector->wallptr = B_LITTLE16(pSector->wallptr);
        pSector->wallnum = B_LITTLE16(pSector->wallnum);
        pSector->ceilingz = B_LITTLE32(pSector->ceilingz);
        pSector->floorz = B_LITTLE32(pSector->floorz);
        pSector->ceilingstat = B_LITTLE16(pSector->ceilingstat);
        pSector->floorstat = B_LITTLE16(pSector->floorstat);
        pSector->ceilingpicnum = B_LITTLE16(pSector->ceilingpicnum);
        pSector->ceilingheinum = B_LITTLE16(pSector->ceilingheinum);
        pSector->floorpicnum = B_LITTLE16(pSector->floorpicnum);
        pSector->floorheinum = B_LITTLE16(pSector->floorheinum);
        pSector->type = B_LITTLE16(pSector->type);
        pSector->hitag = B_LITTLE16(pSector->hitag);
        pSector->extra = B_LITTLE16(pSector->extra);
#endif
        qsector_filler[i] = pSector->fogpal;
        pSector->fogpal = 0;
        if (sector[i].extra > 0)
        {
            char pBuffer[nXSectorSize];
            int nXSector = dbInsertXSector(i);
            XSECTOR *pXSector = &xsector[nXSector];
            memset(pXSector, 0, sizeof(XSECTOR));
            int nCount;
            if (!byte_1A76C8)
            {
                nCount = nXSectorSize;
            }
            else
            {
                nCount = byte_19AE44.at48;
            }
            dassert(nCount <= nXSectorSize);
            IOBuffer1.Read(pBuffer, nCount);
            BitReader bitReader(pBuffer, nCount);
            pXSector->reference = bitReader.readSigned(14);
            pXSector->state = bitReader.readUnsigned(1);
            pXSector->busy = bitReader.readUnsigned(17);
            pXSector->data = bitReader.readUnsigned(16);
            pXSector->txID = bitReader.readUnsigned(10);
            pXSector->busyWaveA = bitReader.readUnsigned(3);
            pXSector->busyWaveB = bitReader.readUnsigned(3);
            pXSector->rxID = bitReader.readUnsigned(10);
            pXSector->command = bitReader.readUnsigned(8);
            pXSector->triggerOn = bitReader.readUnsigned(1);
            pXSector->triggerOff = bitReader.readUnsigned(1);
            pXSector->busyTimeA = bitReader.readUnsigned(12);
            pXSector->waitTimeA = bitReader.readUnsigned(12);
            pXSector->restState = bitReader.readUnsigned(1);
            pXSector->interruptable = bitReader.readUnsigned(1);
            pXSector->amplitude = bitReader.readSigned(8);
            pXSector->freq = bitReader.readUnsigned(8);
            pXSector->reTriggerA = bitReader.readUnsigned(1);
            pXSector->reTriggerB = bitReader.readUnsigned(1);
            pXSector->phase = bitReader.readUnsigned(8);
            pXSector->wave = bitReader.readUnsigned(4);
            pXSector->shadeAlways = bitReader.readUnsigned(1);
            pXSector->shadeFloor = bitReader.readUnsigned(1);
            pXSector->shadeCeiling = bitReader.readUnsigned(1);
            pXSector->shadeWalls = bitReader.readUnsigned(1);
            pXSector->shade = bitReader.readSigned(8);
            pXSector->panAlways = bitReader.readUnsigned(1);
            pXSector->panFloor = bitReader.readUnsigned(1);
            pXSector->panCeiling = bitReader.readUnsigned(1);
            pXSector->Drag = bitReader.readUnsigned(1);
            pXSector->Underwater = bitReader.readUnsigned(1);
            pXSector->Depth = bitReader.readUnsigned(3);
            pXSector->panVel = bitReader.readUnsigned(8);
            pXSector->panAngle = bitReader.readUnsigned(11);
            pXSector->unused1 = bitReader.readUnsigned(1);
            pXSector->decoupled = bitReader.readUnsigned(1);
            pXSector->triggerOnce = bitReader.readUnsigned(1);
            pXSector->isTriggered = bitReader.readUnsigned(1);
            pXSector->Key = bitReader.readUnsigned(3);
            pXSector->Push = bitReader.readUnsigned(1);
            pXSector->Vector = bitReader.readUnsigned(1);
            pXSector->Reserved = bitReader.readUnsigned(1);
            pXSector->Enter = bitReader.readUnsigned(1);
            pXSector->Exit = bitReader.readUnsigned(1);
            pXSector->Wallpush = bitReader.readUnsigned(1);
            pXSector->color = bitReader.readUnsigned(1);
            pXSector->unused2 = bitReader.readUnsigned(1);
            pXSector->busyTimeB = bitReader.readUnsigned(12);
            pXSector->waitTimeB = bitReader.readUnsigned(12);
            pXSector->stopOn = bitReader.readUnsigned(1);
            pXSector->stopOff = bitReader.readUnsigned(1);
            pXSector->ceilpal = bitReader.readUnsigned(4);
            pXSector->offCeilZ = bitReader.readSigned(32);
            pXSector->onCeilZ = bitReader.readSigned(32);
            pXSector->offFloorZ = bitReader.readSigned(32);
            pXSector->onFloorZ = bitReader.readSigned(32);
            pXSector->marker0 = bitReader.readUnsigned(16);
            pXSector->marker1 = bitReader.readUnsigned(16);
            pXSector->Crush = bitReader.readUnsigned(1);
            pXSector->ceilXPanFrac = bitReader.readUnsigned(8);
            pXSector->ceilYPanFrac = bitReader.readUnsigned(8);
            pXSector->floorXPanFrac = bitReader.readUnsigned(8);
            pXSector->damageType = bitReader.readUnsigned(3);
            pXSector->floorpal = bitReader.readUnsigned(4);
            pXSector->floorYPanFrac = bitReader.readUnsigned(8);
            pXSector->locked = bitReader.readUnsigned(1);
            pXSector->windVel = bitReader.readUnsigned(10);
            pXSector->windAng = bitReader.readUnsigned(11);
            pXSector->windAlways = bitReader.readUnsigned(1);
            pXSector->dudeLockout = bitReader.readUnsigned(1);
            pXSector->bobTheta = bitReader.readUnsigned(11);
            pXSector->bobZRange = bitReader.readUnsigned(5);
            pXSector->bobSpeed = bitReader.readSigned(12);
            pXSector->bobAlways = bitReader.readUnsigned(1);
            pXSector->bobFloor = bitReader.readUnsigned(1);
            pXSector->bobCeiling = bitReader.readUnsigned(1);
            pXSector->bobRotate = bitReader.readUnsigned(1);
            xsector[sector[i].extra].reference = i;
            xsector[sector[i].extra].busy = xsector[sector[i].extra].state<<16;

        }
    }
    for (int i = 0; i < numwalls; i++)
    {
        walltype *pWall = &wall[i];
        IOBuffer1.Read(pWall, sizeof(walltype));
        if (byte_1A76C8)
        {
            dbCrypt((char*)pWall, sizeof(walltype), (gMapRev*sizeof(sectortype)) | kMapHeaderNew);
        }
#if B_BIG_ENDIAN == 1
        pWall->x = B_LITTLE32(pWall->x);
        pWall->y = B_LITTLE32(pWall->y);
        pWall->point2 = B_LITTLE16(pWall->point2);
        pWall->nextwall = B_LITTLE16(pWall->nextwall);
        pWall->nextsector = B_LITTLE16(pWall->nextsector);
        pWall->cstat = B_LITTLE16(pWall->cstat);
        pWall->picnum = B_LITTLE16(pWall->picnum);
        pWall->overpicnum = B_LITTLE16(pWall->overpicnum);
        pWall->type = B_LITTLE16(pWall->type);
        pWall->hitag = B_LITTLE16(pWall->hitag);
        pWall->extra = B_LITTLE16(pWall->extra);
#endif
        if (wall[i].extra > 0)
        {
            char pBuffer[nXWallSize];
            int nXWall = dbInsertXWall(i);
            XWALL *pXWall = &xwall[nXWall];
            memset(pXWall, 0, sizeof(XWALL));
            int nCount;
            if (!byte_1A76C8)
            {
                nCount = nXWallSize;
            }
            else
            {
                nCount = byte_19AE44.at44;
            }
            dassert(nCount <= nXWallSize);
            IOBuffer1.Read(pBuffer, nCount);
            BitReader bitReader(pBuffer, nCount);
            pXWall->reference = bitReader.readSigned(14);
            pXWall->state = bitReader.readUnsigned(1);
            pXWall->busy = bitReader.readUnsigned(17);
            pXWall->data = bitReader.readSigned(16);
            pXWall->txID = bitReader.readUnsigned(10);
            pXWall->unused1 = bitReader.readUnsigned(6);
            pXWall->rxID = bitReader.readUnsigned(10);
            pXWall->command = bitReader.readUnsigned(8);
            pXWall->triggerOn = bitReader.readUnsigned(1);
            pXWall->triggerOff = bitReader.readUnsigned(1);
            pXWall->busyTime = bitReader.readUnsigned(12);
            pXWall->waitTime = bitReader.readUnsigned(12);
            pXWall->restState = bitReader.readUnsigned(1);
            pXWall->interruptable = bitReader.readUnsigned(1);
            pXWall->panAlways = bitReader.readUnsigned(1);
            pXWall->panXVel = bitReader.readSigned(8);
            pXWall->panYVel = bitReader.readSigned(8);
            pXWall->decoupled = bitReader.readUnsigned(1);
            pXWall->triggerOnce = bitReader.readUnsigned(1);
            pXWall->isTriggered = bitReader.readUnsigned(1);
            pXWall->key = bitReader.readUnsigned(3);
            pXWall->triggerPush = bitReader.readUnsigned(1);
            pXWall->triggerVector = bitReader.readUnsigned(1);
            pXWall->triggerTouch = bitReader.readUnsigned(1);
            pXWall->unused2 = bitReader.readUnsigned(2);
            pXWall->xpanFrac = bitReader.readUnsigned(8);
            pXWall->ypanFrac = bitReader.readUnsigned(8);
            pXWall->locked = bitReader.readUnsigned(1);
            pXWall->dudeLockout = bitReader.readUnsigned(1);
            pXWall->unused3 = bitReader.readUnsigned(4);
            pXWall->unused4 = bitReader.readUnsigned(32);
            xwall[wall[i].extra].reference = i;
            xwall[wall[i].extra].busy = xwall[wall[i].extra].state << 16;

        }
    }
    initspritelists();
    for (int i = 0; i < mapHeader.at23; i++)
    {
        RemoveSpriteStat(i);
        spritetype *pSprite = &sprite[i];
        IOBuffer1.Read(pSprite, sizeof(spritetype));
        if (byte_1A76C8)
        {
            dbCrypt((char*)pSprite, sizeof(spritetype), (gMapRev*sizeof(spritetype)) | kMapHeaderNew);
        }
#if B_BIG_ENDIAN == 1
        pSprite->x = B_LITTLE32(pSprite->x);
        pSprite->y = B_LITTLE32(pSprite->y);
        pSprite->z = B_LITTLE32(pSprite->z);
        pSprite->cstat = B_LITTLE16(pSprite->cstat);
        pSprite->picnum = B_LITTLE16(pSprite->picnum);
        pSprite->sectnum = B_LITTLE16(pSprite->sectnum);
        pSprite->statnum = B_LITTLE16(pSprite->statnum);
        pSprite->ang = B_LITTLE16(pSprite->ang);
        pSprite->owner = B_LITTLE16(pSprite->owner);
        pSprite->index = B_LITTLE16(pSprite->index);
        pSprite->yvel = B_LITTLE16(pSprite->yvel);
        pSprite->inittype = B_LITTLE16(pSprite->inittype);
        pSprite->type = B_LITTLE16(pSprite->type);
        pSprite->flags = B_LITTLE16(pSprite->hitag);
        pSprite->extra = B_LITTLE16(pSprite->extra);
#endif
        InsertSpriteSect(i, sprite[i].sectnum);
        InsertSpriteStat(i, sprite[i].statnum);
        Numsprites++;
        sprite[i].index = i;
        qsprite_filler[i] = pSprite->blend;
        pSprite->blend = 0;
        if (sprite[i].extra > 0)
        {
            char pBuffer[nXSpriteSize];
            int nXSprite = dbInsertXSprite(i);
            XSPRITE *pXSprite = &xsprite[nXSprite];
            memset(pXSprite, 0, sizeof(XSPRITE));
            int nCount;
            if (!byte_1A76C8)
            {
                nCount = nXSpriteSize;
            }
            else
            {
                nCount = byte_19AE44.at40;
            }
            dassert(nCount <= nXSpriteSize);
            IOBuffer1.Read(pBuffer, nCount);
            BitReader bitReader(pBuffer, nCount);
            pXSprite->reference = bitReader.readSigned(14);
            pXSprite->state = bitReader.readUnsigned(1);
            pXSprite->busy = bitReader.readUnsigned(17);
            pXSprite->txID = bitReader.readUnsigned(10);
            pXSprite->rxID = bitReader.readUnsigned(10);
            pXSprite->command = bitReader.readUnsigned(8);
            pXSprite->triggerOn = bitReader.readUnsigned(1);
            pXSprite->triggerOff = bitReader.readUnsigned(1);
            pXSprite->wave = bitReader.readUnsigned(2);
            pXSprite->busyTime = bitReader.readUnsigned(12);
            pXSprite->waitTime = bitReader.readUnsigned(12);
            pXSprite->restState = bitReader.readUnsigned(1);
            pXSprite->Interrutable = bitReader.readUnsigned(1);
            pXSprite->unused1 = bitReader.readUnsigned(2);
            pXSprite->respawnPending = bitReader.readUnsigned(2);
            pXSprite->unused2 = bitReader.readUnsigned(1);
            pXSprite->lT = bitReader.readUnsigned(1);
            pXSprite->dropMsg = bitReader.readUnsigned(8);
            pXSprite->Decoupled = bitReader.readUnsigned(1);
            pXSprite->triggerOnce = bitReader.readUnsigned(1);
            pXSprite->isTriggered = bitReader.readUnsigned(1);
            pXSprite->key = bitReader.readUnsigned(3);
            pXSprite->Push = bitReader.readUnsigned(1);
            pXSprite->Vector = bitReader.readUnsigned(1);
            pXSprite->Impact = bitReader.readUnsigned(1);
            pXSprite->Pickup = bitReader.readUnsigned(1);
            pXSprite->Touch = bitReader.readUnsigned(1);
            pXSprite->Sight = bitReader.readUnsigned(1);
            pXSprite->Proximity = bitReader.readUnsigned(1);
            pXSprite->unused3 = bitReader.readUnsigned(2);
            pXSprite->lSkill = bitReader.readUnsigned(5);
            pXSprite->lS = bitReader.readUnsigned(1);
            pXSprite->lB = bitReader.readUnsigned(1);
            pXSprite->lC = bitReader.readUnsigned(1);
            pXSprite->DudeLockout = bitReader.readUnsigned(1);
            pXSprite->data1 = bitReader.readSigned(16);
            pXSprite->data2 = bitReader.readSigned(16);
            pXSprite->data3 = bitReader.readSigned(16);
            pXSprite->goalAng = bitReader.readUnsigned(11);
            pXSprite->dodgeDir = bitReader.readSigned(2);
            pXSprite->locked = bitReader.readUnsigned(1);
            pXSprite->medium = bitReader.readUnsigned(2);
            pXSprite->respawn = bitReader.readUnsigned(2);
            pXSprite->data4 = bitReader.readUnsigned(16);
            pXSprite->unused4 = bitReader.readUnsigned(6);
            pXSprite->lockMsg = bitReader.readUnsigned(8);
            pXSprite->health = bitReader.readUnsigned(12);
            pXSprite->dudeDeaf = bitReader.readUnsigned(1);
            pXSprite->dudeAmbush = bitReader.readUnsigned(1);
            pXSprite->dudeGuard = bitReader.readUnsigned(1);
            pXSprite->dudeFlag4 = bitReader.readUnsigned(1);
            pXSprite->target = bitReader.readSigned(16);
            pXSprite->targetX = bitReader.readSigned(32);
            pXSprite->targetY = bitReader.readSigned(32);
            pXSprite->targetZ = bitReader.readSigned(32);
            pXSprite->burnTime = bitReader.readUnsigned(16);
            pXSprite->burnSource = bitReader.readSigned(16);
            pXSprite->height = bitReader.readUnsigned(16);
            pXSprite->stateTimer = bitReader.readUnsigned(16);
            pXSprite->aiState = NULL;
            bitReader.skipBits(32);
            xsprite[sprite[i].extra].reference = i;
            xsprite[sprite[i].extra].busy = xsprite[sprite[i].extra].state << 16;
            if (!byte_1A76C8) {
                xsprite[sprite[i].extra].lT = xsprite[sprite[i].extra].lB;
            }

            #ifdef NOONE_EXTENSIONS
            // indicate if the map requires modern features to work properly
            // for maps wich created in different editors (include vanilla MAPEDIT) or in PMAPEDIT version below than BETA13
            if (!gModernMap && pXSprite->rxID == pXSprite->txID && pXSprite->command == kCmdModernFeaturesEnable)
            {
                // get the modern features revision
                switch (pXSprite->txID) {
                case kChannelMapModernRev1:
                    gModernMap = 1;
                    break;
                case kChannelMapModernRev2:
                    gModernMap = 2;
                    break;
                }
            }
            #endif
        }
#if 0
        if ((sprite[i].cstat & 0x30) == 0x30)
        {
            sprite[i].cstat &= ~0x30;
        }
#endif
    }
    unsigned int nCRC;
    IOBuffer1.Read(&nCRC, 4);
#if B_BIG_ENDIAN == 1
    nCRC = B_LITTLE32(nCRC);
#endif
    md4once((unsigned char*)pData, nSize, g_loadedMapHack.md4);
    if (Bcrc32(pData, nSize-4, 0) != nCRC)
    {
        LOG_F(ERROR, "Map File does not match CRC");
        gSysRes.Unlock(pNode);
        return -1;
    }
    if (pCRC)
        *pCRC = nCRC;
    gSysRes.Unlock(pNode);
    PropagateMarkerReferences();
    if (byte_1A76C8)
    {
        if (gSongId == kMapHeaderNew || gSongId == kMapHeaderOld)
        {
            byte_1A76C6 = 1;
        }
        else if (!gSongId)
        {
            byte_1A76C6 = 0;
        }
        else
        {
            LOG_F(ERROR, "Corrupted Map file");
            return -1;
        }
    }
    else if (gSongId != 0)
    {
        LOG_F(ERROR, "Corrupted Map file");
        return -1;
    }

#ifdef NOONE_EXTENSIONS
    if (VanillaMode() && gModernMap)
    {
        viewSetMessage("Warning: Modern levels are not compatible with vanilla mode");
        viewSetMessage("Please disable vanilla mode and restart level");
    }
#endif

#ifdef POLYMER
    if (videoGetRenderMode() == REND_POLYMER)
        polymer_loadboard();
#endif

    if ((header.version & 0xff00) == 0x600)
    {
        switch (header.version&0xff)
        {
        case 0:
            for (int i = 0; i < numsectors; i++)
            {
                sectortype *pSector = &sector[i];
                if (pSector->extra > 0)
                {
                    XSECTOR *pXSector = &xsector[pSector->extra];
                    pXSector->busyTimeB = pXSector->busyTimeA;
                    if (pXSector->busyTimeA > 0)
                    {
                        if (!pXSector->restState)
                        {
                            pXSector->reTriggerA = 1;
                        }
                        else
                        {
                            pXSector->waitTimeB = pXSector->busyTimeA;
                            pXSector->waitTimeA = 0;
                            pXSector->reTriggerB = 1;
                        }
                    }
                }
            }
            fallthrough__;
        case 1:
            for (int i = 0; i < numsectors; i++)
            {
                sectortype *pSector = &sector[i];
                if (pSector->extra > 0)
                {
                    XSECTOR *pXSector = &xsector[pSector->extra];
                    pXSector->freq >>= 1;
                }
            }
            fallthrough__;
        case 2:
            for (int i = 0; i < kMaxSprites; i++)
            {
            }
            break;
            
        }
    }

#ifdef YAX_ENABLE
    yax_update((header.version & 0xff00) > 0x700 ? 0 : 1);
    if (editstatus)
        yax_updategrays(*pZ);
#endif

    calc_sector_reachability();

    g_loadedMapVersion = 7;

    return 0;
}

int dbSaveMap(const char *pPath, int nX, int nY, int nZ, short nAngle, short nSector)
{
    char sMapExt[BMAX_PATH];
    //char sBakExt[BMAX_PATH];
    int16_t tpskyoff[256];
    int nSpriteNum;
    psky_t *pSky = tileSetupSky(0);
    gSkyCount = 1<<pSky->lognumtiles;
    gMapRev++;
    nSpriteNum = 0;
    strcpy(sMapExt, pPath);
    ChangeExtension(sMapExt, ".MAP");
    int nSize = sizeof(MAPSIGNATURE)+sizeof(MAPHEADER);
    if (byte_1A76C8)
    {
        nSize += sizeof(MAPHEADER2);
    }
    for (int i = 0; i < gSkyCount; i++)
        tpskyoff[i] = pSky->tileofs[i];
    nSize += gSkyCount*sizeof(tpskyoff[0]);
    nSize += sizeof(sectortype)*numsectors;
    for (int i = 0; i < numsectors; i++)
    {
        if (sector[i].extra > 0)
        {
            nSize += nXSectorSize;
        }
    }
    nSize += sizeof(walltype)*numwalls;
    for (int i = 0; i < numwalls; i++)
    {
        if (wall[i].extra > 0)
        {
            nSize += nXWallSize;
        }
    }
    for (int i = 0; i < kMaxSprites; i++)
    {
        if (sprite[i].statnum < kMaxStatus)
        {
            nSpriteNum++;
            if (sprite[i].extra > 0)
            {
                nSize += nXSpriteSize;
            }
        }
    }
    nSize += sizeof(spritetype)*nSpriteNum;
    nSize += 4;
    char *pData = (char*)Xmalloc(nSize);
    IOBuffer IOBuffer1 = IOBuffer(nSize, pData);
    MAPSIGNATURE header;
    memcpy(&header, "BLM\x1a", 4);
    if (byte_1A76C8)
    {
        header.version = 0x700;
        byte_1A76C7 = 1;
    }
    else
    {
        header.version = 0x603;
        byte_1A76C7 = 0;
    }
    IOBuffer1.Write(&header, sizeof(header));
    MAPHEADER mapheader;
    mapheader.at0 = B_LITTLE32(nX);
    mapheader.at4 = B_LITTLE32(nY);
    mapheader.at8 = B_LITTLE32(nZ);
    mapheader.atc = B_LITTLE16(nAngle);
    mapheader.ate = B_LITTLE16(nSector);
    mapheader.at10 = B_LITTLE16(pSky->lognumtiles);
    mapheader.at12 = B_LITTLE32(gVisibility);
    if (byte_1A76C6)
    {
        gSongId = kMapHeaderNew;
    }
    else
    {
        gSongId = 0;
    }
    mapheader.at16 = B_LITTLE32(gSongId);
    mapheader.at1a = parallaxtype;
    mapheader.at1b = gMapRev;
    mapheader.at1f = B_LITTLE16(numsectors);
    mapheader.at21 = B_LITTLE16(numwalls);
    mapheader.at23 = B_LITTLE16(nSpriteNum);
    if (byte_1A76C7)
    {
        dbCrypt((char*)&mapheader, sizeof(MAPHEADER), kMapHeaderNew);
    }
    IOBuffer1.Write(&mapheader, sizeof(MAPHEADER));
    if (byte_1A76C8)
    {
        Bstrcpy(byte_19AE44.at0, AppProperName);
        byte_19AE44.at48 = nXSectorSize;
        byte_19AE44.at44 = nXWallSize;
        byte_19AE44.at40 = nXSpriteSize;
        dbCrypt((char*)&byte_19AE44, sizeof(MAPHEADER2), numwalls);
        IOBuffer1.Write(&byte_19AE44, sizeof(MAPHEADER2));
        dbCrypt((char*)&byte_19AE44, sizeof(MAPHEADER2), numwalls);
    }
    if (byte_1A76C8)
    {
        dbCrypt((char*)tpskyoff, gSkyCount*sizeof(tpskyoff[0]), gSkyCount*sizeof(tpskyoff[0]));
    }
    IOBuffer1.Write(tpskyoff, gSkyCount*sizeof(tpskyoff[0]));
    if (byte_1A76C8)
    {
        dbCrypt((char*)tpskyoff, gSkyCount*sizeof(tpskyoff[0]), gSkyCount*sizeof(tpskyoff[0]));
    }
    for (int i = 0; i < numsectors; i++)
    {
        if (byte_1A76C8)
        {
            dbCrypt((char*)&sector[i], sizeof(sectortype), gMapRev*sizeof(sectortype));
        }
        IOBuffer1.Write(&sector[i], sizeof(sectortype));
        if (byte_1A76C8)
        {
            dbCrypt((char*)&sector[i], sizeof(sectortype), gMapRev*sizeof(sectortype));
        }
        if (sector[i].extra > 0)
        {
            char pBuffer[nXSectorSize];
            BitWriter bitWriter(pBuffer, nXSectorSize);
            XSECTOR* pXSector = &xsector[sector[i].extra];
            bitWriter.write(pXSector->reference, 14);
            bitWriter.write(pXSector->state, 1);
            bitWriter.write(pXSector->busy, 17);
            bitWriter.write(pXSector->data, 16);
            bitWriter.write(pXSector->txID, 10);
            bitWriter.write(pXSector->busyWaveA, 3);
            bitWriter.write(pXSector->busyWaveB, 3);
            bitWriter.write(pXSector->rxID, 10);
            bitWriter.write(pXSector->command, 8);
            bitWriter.write(pXSector->triggerOn, 1);
            bitWriter.write(pXSector->triggerOff, 1);
            bitWriter.write(pXSector->busyTimeA, 12);
            bitWriter.write(pXSector->waitTimeA, 12);
            bitWriter.write(pXSector->restState, 1);
            bitWriter.write(pXSector->interruptable, 1);
            bitWriter.write(pXSector->amplitude, 8);
            bitWriter.write(pXSector->freq, 8);
            bitWriter.write(pXSector->reTriggerA, 1);
            bitWriter.write(pXSector->reTriggerB, 1);
            bitWriter.write(pXSector->phase, 8);
            bitWriter.write(pXSector->wave, 4);
            bitWriter.write(pXSector->shadeAlways, 1);
            bitWriter.write(pXSector->shadeFloor, 1);
            bitWriter.write(pXSector->shadeCeiling, 1);
            bitWriter.write(pXSector->shadeWalls, 1);
            bitWriter.write(pXSector->shade, 8);
            bitWriter.write(pXSector->panAlways, 1);
            bitWriter.write(pXSector->panFloor, 1);
            bitWriter.write(pXSector->panCeiling, 1);
            bitWriter.write(pXSector->Drag, 1);
            bitWriter.write(pXSector->Underwater, 1);
            bitWriter.write(pXSector->Depth, 3);
            bitWriter.write(pXSector->panVel, 8);
            bitWriter.write(pXSector->panAngle, 11);
            bitWriter.write(pXSector->unused1, 1);
            bitWriter.write(pXSector->decoupled, 1);
            bitWriter.write(pXSector->triggerOnce, 1);
            bitWriter.write(pXSector->isTriggered, 1);
            bitWriter.write(pXSector->Key, 3);
            bitWriter.write(pXSector->Push, 1);
            bitWriter.write(pXSector->Vector, 1);
            bitWriter.write(pXSector->Reserved, 1);
            bitWriter.write(pXSector->Enter, 1);
            bitWriter.write(pXSector->Exit, 1);
            bitWriter.write(pXSector->Wallpush, 1);
            bitWriter.write(pXSector->color, 1);
            bitWriter.write(pXSector->unused2, 1);
            bitWriter.write(pXSector->busyTimeB, 12);
            bitWriter.write(pXSector->waitTimeB, 12);
            bitWriter.write(pXSector->stopOn, 1);
            bitWriter.write(pXSector->stopOff, 1);
            bitWriter.write(pXSector->ceilpal, 4);
            bitWriter.write(pXSector->offCeilZ, 32);
            bitWriter.write(pXSector->onCeilZ, 32);
            bitWriter.write(pXSector->offFloorZ, 32);
            bitWriter.write(pXSector->onFloorZ, 32);
            bitWriter.write(pXSector->marker0, 16);
            bitWriter.write(pXSector->marker1, 16);
            bitWriter.write(pXSector->Crush, 1);
            bitWriter.write(pXSector->ceilXPanFrac, 8);
            bitWriter.write(pXSector->ceilYPanFrac, 8);
            bitWriter.write(pXSector->floorXPanFrac, 8);
            bitWriter.write(pXSector->damageType, 3);
            bitWriter.write(pXSector->floorpal, 4);
            bitWriter.write(pXSector->floorYPanFrac, 8);
            bitWriter.write(pXSector->locked, 1);
            bitWriter.write(pXSector->windVel, 10);
            bitWriter.write(pXSector->windAng, 11);
            bitWriter.write(pXSector->windAlways, 1);
            bitWriter.write(pXSector->dudeLockout, 1);
            bitWriter.write(pXSector->bobTheta, 11);
            bitWriter.write(pXSector->bobZRange, 5);
            bitWriter.write(pXSector->bobSpeed, 12);
            bitWriter.write(pXSector->bobAlways, 1);
            bitWriter.write(pXSector->bobFloor, 1);
            bitWriter.write(pXSector->bobCeiling, 1);
            bitWriter.write(pXSector->bobRotate, 1);
            IOBuffer1.Write(pBuffer, nXSectorSize);
        }
    }
    for (int i = 0; i < numwalls; i++)
    {
        if (byte_1A76C8)
        {
            dbCrypt((char*)&wall[i], sizeof(walltype), gMapRev*sizeof(sectortype) | kMapHeaderNew);
        }
        IOBuffer1.Write(&wall[i], sizeof(walltype));
        if (byte_1A76C8)
        {
            dbCrypt((char*)&wall[i], sizeof(walltype), gMapRev*sizeof(sectortype) | kMapHeaderNew);
        }
        if (wall[i].extra > 0)
        {
            char pBuffer[nXWallSize];
            BitWriter bitWriter(pBuffer, nXWallSize);
            XWALL* pXWall = &xwall[wall[i].extra];
            bitWriter.write(pXWall->reference, 14);
            bitWriter.write(pXWall->state, 1);
            bitWriter.write(pXWall->busy, 17);
            bitWriter.write(pXWall->data, 16);
            bitWriter.write(pXWall->txID, 10);
            bitWriter.write(pXWall->unused1, 6);
            bitWriter.write(pXWall->rxID, 10);
            bitWriter.write(pXWall->command, 8);
            bitWriter.write(pXWall->triggerOn, 1);
            bitWriter.write(pXWall->triggerOff, 1);
            bitWriter.write(pXWall->busyTime, 12);
            bitWriter.write(pXWall->waitTime, 12);
            bitWriter.write(pXWall->restState, 1);
            bitWriter.write(pXWall->interruptable, 1);
            bitWriter.write(pXWall->panAlways, 1);
            bitWriter.write(pXWall->panXVel, 8);
            bitWriter.write(pXWall->panYVel, 8);
            bitWriter.write(pXWall->decoupled, 1);
            bitWriter.write(pXWall->triggerOnce, 1);
            bitWriter.write(pXWall->isTriggered, 1);
            bitWriter.write(pXWall->key, 3);
            bitWriter.write(pXWall->triggerPush, 1);
            bitWriter.write(pXWall->triggerVector, 1);
            bitWriter.write(pXWall->triggerTouch, 1);
            bitWriter.write(pXWall->unused2, 2);
            bitWriter.write(pXWall->xpanFrac, 8);
            bitWriter.write(pXWall->ypanFrac, 8);
            bitWriter.write(pXWall->locked, 1);
            bitWriter.write(pXWall->dudeLockout, 1);
            bitWriter.write(pXWall->unused3, 4);
            bitWriter.write(pXWall->unused4, 32);
            IOBuffer1.Write(pBuffer, nXWallSize);
        }
    }
    for (int i = 0; i < kMaxSprites; i++)
    {
        if (sprite[i].statnum < kMaxStatus)
        {
            if (byte_1A76C8)
            {
                dbCrypt((char*)&sprite[i], sizeof(spritetype), gMapRev*sizeof(spritetype) | kMapHeaderNew);
            }
            IOBuffer1.Write(&sprite[i], sizeof(spritetype));
            if (byte_1A76C8)
            {
                dbCrypt((char*)&sprite[i], sizeof(spritetype), gMapRev*sizeof(spritetype) | kMapHeaderNew);
            }
            if (sprite[i].extra > 0)
            {
                char pBuffer[nXSpriteSize];
                BitWriter bitWriter(pBuffer, nXSpriteSize);
                XSPRITE* pXSprite = &xsprite[sprite[i].extra];
                bitWriter.write(pXSprite->reference, 14);
                bitWriter.write(pXSprite->state, 1);
                bitWriter.write(pXSprite->busy, 17);
                bitWriter.write(pXSprite->txID, 10);
                bitWriter.write(pXSprite->rxID, 10);
                bitWriter.write(pXSprite->command, 8);
                bitWriter.write(pXSprite->triggerOn, 1);
                bitWriter.write(pXSprite->triggerOff, 1);
                bitWriter.write(pXSprite->wave, 2);
                bitWriter.write(pXSprite->busyTime, 12);
                bitWriter.write(pXSprite->waitTime, 12);
                bitWriter.write(pXSprite->restState, 1);
                bitWriter.write(pXSprite->Interrutable, 1);
                bitWriter.write(pXSprite->unused1, 2);
                bitWriter.write(pXSprite->respawnPending, 2);
                bitWriter.write(pXSprite->unused2, 1);
                bitWriter.write(pXSprite->lT, 1);
                bitWriter.write(pXSprite->dropMsg, 8);
                bitWriter.write(pXSprite->Decoupled, 1);
                bitWriter.write(pXSprite->triggerOnce, 1);
                bitWriter.write(pXSprite->isTriggered, 1);
                bitWriter.write(pXSprite->key, 3);
                bitWriter.write(pXSprite->Push, 1);
                bitWriter.write(pXSprite->Vector, 1);
                bitWriter.write(pXSprite->Impact, 1);
                bitWriter.write(pXSprite->Pickup, 1);
                bitWriter.write(pXSprite->Touch, 1);
                bitWriter.write(pXSprite->Sight, 1);
                bitWriter.write(pXSprite->Proximity, 1);
                bitWriter.write(pXSprite->unused3, 2);
                bitWriter.write(pXSprite->lSkill, 5);
                bitWriter.write(pXSprite->lS, 1);
                bitWriter.write(pXSprite->lB, 1);
                bitWriter.write(pXSprite->lC, 1);
                bitWriter.write(pXSprite->DudeLockout, 1);
                bitWriter.write(pXSprite->data1, 16);
                bitWriter.write(pXSprite->data2, 16);
                bitWriter.write(pXSprite->data3, 16);
                bitWriter.write(pXSprite->goalAng, 11);
                bitWriter.write(pXSprite->dodgeDir, 2);
                bitWriter.write(pXSprite->locked, 1);
                bitWriter.write(pXSprite->medium, 2);
                bitWriter.write(pXSprite->respawn, 2);
                bitWriter.write(pXSprite->data4, 16);
                bitWriter.write(pXSprite->unused4, 6);
                bitWriter.write(pXSprite->lockMsg, 8);
                bitWriter.write(pXSprite->health, 12);
                bitWriter.write(pXSprite->dudeDeaf, 1);
                bitWriter.write(pXSprite->dudeAmbush, 1);
                bitWriter.write(pXSprite->dudeGuard, 1);
                bitWriter.write(pXSprite->dudeFlag4, 1);
                bitWriter.write(pXSprite->target, 16);
                bitWriter.write(pXSprite->targetX, 32);
                bitWriter.write(pXSprite->targetY, 32);
                bitWriter.write(pXSprite->targetZ, 32);
                bitWriter.write(pXSprite->burnTime, 16);
                bitWriter.write(pXSprite->burnSource, 16);
                bitWriter.write(pXSprite->height, 16);
                bitWriter.write(pXSprite->stateTimer, 16);
                IOBuffer1.Write(pBuffer, nXSpriteSize);
            }
        }
    }
    unsigned int nCRC = Bcrc32(pData, nSize-4, 0);
    IOBuffer1.Write(&nCRC, 4);
    int nHandle = Bopen(sMapExt, BO_BINARY|BO_TRUNC|BO_CREAT|BO_WRONLY, BS_IREAD|BS_IWRITE);
    if (nHandle == -1)
    {
        LOG_F(ERROR, "Couldn't open \"%s\" for writing: %s", sMapExt, strerror(errno));
        Xfree(pData);
        return -1;
    }
    if (Bwrite(nHandle, pData, nSize) != nSize)
    {
        LOG_F(ERROR, "Couldn't write to \"%s\": %s", sMapExt, strerror(errno));
        Bclose(nHandle);
        Xfree(pData);
        return -1;
    }
    Bclose(nHandle);
    Xfree(pData);
    return 0;
#if 0
    char *pExt = strchr(sMapExt, '.');
    if (pExt)
    {
        *pExt = 0;
    }
    gSysRes.AddExternalResource(sMapExt, "MAP", nSize);
    DICTNODE *hMap = gSysRes.Lookup(sMapExt, "MAP");
    dassert(hMap != NULL);
#endif
}

int32_t qloadboard(const char* filename, char flags, vec3_t* dapos, int16_t* daang, int16_t* dacursectnum)
{
    // NUKE-TODO: implement flags, see mapedit.cpp
    UNREFERENCED_PARAMETER(flags);
    return dbLoadMap(filename, &dapos->x, &dapos->y, &dapos->z, (short*)daang, (short*)dacursectnum, NULL);
}

int32_t qsaveboard(const char* filename, const vec3_t* dapos, int16_t daang, int16_t dacursectnum)
{
    // NUKE-TODO: see mapedit.cpp
    byte_1A76C6 = byte_1A76C8 = byte_1A76C7 = 1;
    return dbSaveMap(filename, dapos->x, dapos->y, dapos->z, daang, dacursectnum);
}

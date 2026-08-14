//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 Nuke.YKT
Copyright (C) 2026 NoOne

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
#ifdef POLYMER
#include "polymer.h"
#endif
#include "common_game.h"
#include "db.h"
#include "loadsave.h"
#include "player.h"
#include "sectorfx.h"
#include "trig.h"
#include "view.h"
#include "tile.h"
#include "mirrors.h"

#define kMirrorTile                     504
#define kMirrorPicStartVanilla          4080
#define kMirrorPicEndVanilla            kMirrorPicStartVanilla + 16

#ifdef USE_OPENGL
#define MASKED_ROR_ISSUE                1
// NoOne: There is a problem in polymost. Despite that 'r_rortexture'
// and 'r_rortexturerange' variables are correct, it makes
// non-empty tiles to not shown for the masked RORs.
// For example, it just shows the black color
// when using tile 502.
#endif

struct MIRROR
{
    uint8_t type, flags;
    uint16_t id, thisID;
    uint16_t picStart, picEnd, picOld;
    uint8_t hNeighID, vNeighID;
    POINT3D ofs;
};

struct RORCAM
{
    int32_t x, y, z, d;
    uint8_t index;
};

char gMirrorDrawing;
uint16_t mirrorcnt = 0;
uint16_t mirrorPicStart = 0;
uint16_t mirrorPicEnd   = 0;

static int16_t mirrorsector, mirrorwall[4];
static MIRROR mirror[kMaxMirrors];
static RORCAM vcam[kMaxMirrors];
static RORCAM hcam[kMaxMirrors];
static uint8_t list[kMaxMirrors];

FORCE_INLINE char IsMirrorTile(int nTile)            { return irngok(nTile, mirrorPicStart, mirrorPicEnd); }
FORCE_INLINE void tileDeleteRange(int s, int e)      { while (--e >= s) tileDelete(e); }
FORCE_INLINE int ROR_GetOther(int nRor)              { return (mirror[nRor].type == OBJ_FLOOR) ? ++nRor : --nRor; }

void ROR_ClearGotPic(int n)
{
    int i = mirror[n].picEnd;
    while (--i >= mirror[n].picStart)
        ClearBitString(gotpic, i);
}

void ROR_ClearGotPic(void)
{
    int i = mirrorPicEnd;
    while (--i >= mirrorPicStart)
        ClearBitString(gotpic, i);
}

char ROR_TestGotPic(int n)
{
    int i = mirror[n].picEnd;
    while (--i >= mirror[n].picStart)
    {
        if (TestBitString(gotpic, i))
            return 1;
    }

    return 0;
}

void ROR_SetGotPic(int n)
{
    int i = mirror[n].picEnd;
    while (--i >= mirror[n].picStart)
        SetBitString(gotpic, i);
}

int ROR_FindBySector(int nSect, int nType)
{
    int i = mirrorcnt;
    while (--i >= 0 && (mirror[i].type != nType || mirror[i].id != nSect));
    return i;
}

void ROR_CollectNeighborsH(int nStart, uint8_t *list, int* num)
{
    MIRROR* pRor = &mirror[nStart];
    int s, e, n, i;
    
    list[*num] = nStart; *num = *num + 1;
    e = s = sector[pRor->id].wallptr; e += sector[pRor->id].wallnum;
    
    while(s < e) // collect all the matching sectors while not separated
    {
        if ((n = wall[s].nextsector) >= 0)
        {
            if ((n = ROR_FindBySector(n, pRor->type)) >= 0)
            {
                i = *num;
                while (--i >= 0 && list[i] != n);
                if (i < 0) ROR_CollectNeighborsH(n, list, num);
            }
        }
        
        s++;
    }
}

void ROR_CollectNeighborsV(int nStart, uint8_t* list, int* num)
{
    MIRROR *pRor = &mirror[nStart];
    short* linkArr = (pRor->type == OBJ_FLOOR) ? gUpperLink : gLowerLink;
    int nType, nSpr, s, n, t;
    
    nType = pRor->type, s = n = pRor->id;

    while( 1 )
    {
        t = nStart;
        do
        {
            pRor = &mirror[t];
            if (pRor->type == nType && pRor->id == n)
            {
                list[*num] = t;
                *num = *num + 1;
                break;
            }
            
            t = IncRotate(t, mirrorcnt);
        }
        while(t != nStart);
        
        if ((nSpr = linkArr[n]) < 0)
            break;
        
        n = sprite[nSpr].owner;
        n = sprite[n].sectnum;
        
        if (n < 0 || n == s)
            break;
    }
}


// this is from engine.cpp
static void getclosestpointonline(vec2_t const p, vec2_t w, vec2_t w2, vec2_t *const closest)
{
    vec2_t const d = w2 - w;

    int64_t i = d.x * ((int64_t)p.x - w.x) + d.y * ((int64_t)p.y - w.y);

    if (i <= 0)
    {
        *closest = w;
        return;
    }

    int64_t const j = (int64_t)d.x * d.x + (int64_t)d.y * d.y;

    if (i >= j)
    {
        *closest = w2;
        return;
    }

    i = tabledivide64((i << 15), j) << 15;

    *closest = { (int32_t)(w.x + ((d.x * i) >> 30)), (int32_t)(w.y + ((d.y * i) >> 30)) };
}

static int getDistToSect(int32_t nSect, int32_t x, int32_t y)
{
    int32_t s, e, d, nDist = INT32_MAX;
    vec2_t p = {x, y}, w;
    
    e = s = sector[nSect].wallptr; e += sector[nSect].wallnum;

    while(s < e)
    {
        getclosestpointonline(p, wall[s].xy, wall[wall[s].point2].xy, &w);
        if ((d = approxDist(p.x - w.x, p.y - w.y)) < nDist)
            nDist = d;
        
        s++;
    }
    
    return nDist;
}


#ifdef POLYMER
void PolymerRORCallback(int16_t sectnum, int16_t wallnum, int8_t rorstat, int16_t* msectnum, int32_t* gx, int32_t* gy, int32_t* gz)
{
    UNREFERENCED_PARAMETER(wallnum);
    int nMirror;

    if ((nMirror = ROR_FindBySector(sectnum, rorstat)) >= 0)
    {
        *msectnum = mirror[nMirror].id;
        *gx += mirror[nMirror].ofs.x;
        *gy += mirror[nMirror].ofs.y;
        *gz += mirror[nMirror].ofs.z;
    }
}
#endif

char IsRorSector(int nSect, int stat)
{
    if (stat == OBJ_FLOOR)
    {
        if (rngok(sector[nSect].floorpicnum, mirrorPicStart, mirrorPicEnd))     return (sector[nSect].floorstat & 0x180) ? 2 : 1;
        else if (sector[nSect].floorpicnum == kMirrorTile)                      return 1;
        else if ((sector[nSect].floorstat & 0x180) != 0)                        return 2;
        else                                                                    return 0;
    }
    else if (rngok(sector[nSect].ceilingpicnum, mirrorPicStart, mirrorPicEnd))  return (sector[nSect].ceilingstat & 0x180) ? 2 : 1;
    else if (sector[nSect].ceilingpicnum == kMirrorTile)                        return 1;
    else if ((sector[nSect].ceilingstat & 0x180) != 0)                          return 2;
    else                                                                        return 0;
}

static int CreateMirrorPic(MIRROR* pFor, int nStart, int16_t* objTile)
{
    int32_t nObjTile = *objTile;
    picanm_t* pnm = &picanm[nObjTile];
    int32_t wh, hg, o;

    pFor->picEnd = pFor->picStart = pFor->picOld = nObjTile;
    pFor->picEnd += pnm->num + 1;

    if (nStart < 0)
        return 0;

    o = 0;
    if (pnm->num)
        o = ((pnm->sf & PICANM_ANIMTYPE_MASK) == PICANM_ANIMTYPE_BACK) ? -pnm->num : pnm->num;

    *objTile = nStart;
    pFor->picEnd = pFor->picStart = nStart;
    pFor->picEnd += pnm->num + 1;

    if (o < 0) // backwards animation...
    {
        o = klabs(o);
        nObjTile -= o;
        *objTile += o;
    }

    while (o-- >= 0)
    {
        if ((wh = tilesiz[nObjTile].x) > 0 && (hg = tilesiz[nObjTile].y) > 0)
        {
            if (tileCreate(nStart, wh, hg) && tileLoad(nObjTile))
            {
                tileSetSize(nStart, wh, hg); rottile[nStart].owner = -1;
                Bmemmove((void*)waloff[nStart], (void*)waloff[nObjTile], wh * hg);
                Bmemmove(&picanm[nStart], &picanm[nObjTile], sizeof(picanm[0]));
                Bmemmove(&surfType[nStart], &surfType[nObjTile], sizeof(surfType[0]));
            }
        }

        nObjTile++, nStart++;
    }

    return pFor->picEnd - pFor->picStart;
}

static void InitMirrorSector(void)
{
    // Create a room to translate the mirror
    mirrorsector = numsectors;
    for (int i = 0; i < 4; i++)
    {
        mirrorwall[i]                   = numwalls + i;
        wall[mirrorwall[i]].picnum      = kMirrorTile;
        wall[mirrorwall[i]].overpicnum  = kMirrorTile;
        wall[mirrorwall[i]].cstat       = 0;
        wall[mirrorwall[i]].nextsector  = -1;
        wall[mirrorwall[i]].nextwall    = -1;
        wall[mirrorwall[i]].point2      = numwalls + i + 1;
    }

    wall[mirrorwall[3]].point2          = mirrorwall[0];
    sector[mirrorsector].ceilingpicnum  = kMirrorTile;
    sector[mirrorsector].floorpicnum    = kMirrorTile;
    sector[mirrorsector].wallptr        = mirrorwall[0];
    sector[mirrorsector].wallnum        = 4;
}

static int MirrorPicsInit(int nRange)
{
    MIRROR* pRor;
    int nStart, i;
    int r;

#ifdef USE_OPENGL
    r_rortexturerange = kMirrorPicEndVanilla - kMirrorPicStartVanilla;
    r_rortexture = kMirrorPicStartVanilla;
    
    #ifdef  POLYMER
        polymer_setrorcallback(PolymerRORCallback);
    #endif
#endif

    tileDelete(kMirrorTile);
    tileDeleteRange(kMirrorPicStartVanilla, kMirrorPicEndVanilla);                      // compatibility
    if (mirrorPicEnd > mirrorPicStart) tileDeleteRange(mirrorPicStart, mirrorPicEnd);   // previous session?

    mirrorPicStart = mirrorPicEnd = 0;
    if (nRange <= 0)
        return -1;

    if ((nStart = tileSearchFreeRange(nRange, MAXUSERTILES)) >= 0)
    {
        mirrorPicEnd = mirrorPicStart = nStart;
        mirrorPicEnd += nRange;
#ifdef USE_OPENGL
        r_rortexturerange = nRange;
        r_rortexture = nStart;
#endif
    }
    else
    {
        viewSetSystemMessage("Not enough range of free tiles for mirrors. Required range = %d.", nRange);
    }

    r = nStart;
    i = mirrorcnt;
    while (--i >= 0)
    {
        pRor = &mirror[i];

        switch (pRor->type)
        {
            case OBJ_WALL:
                if (wall[pRor->id].type == kWallStack)
                    nStart += CreateMirrorPic(pRor, nStart, &wall[pRor->thisID].overpicnum);
                else
                    nStart += CreateMirrorPic(pRor, nStart, &wall[pRor->thisID].picnum);
                break;
            case OBJ_FLOOR:
                nStart += CreateMirrorPic(pRor, nStart, &sector[pRor->thisID].floorpicnum);
                break;
            case OBJ_CEILING:
                nStart += CreateMirrorPic(pRor, nStart, &sector[pRor->thisID].ceilingpicnum);
                break;
        }
    }

#ifdef USE_OPENGL
    if (r != nStart
        && videoGetRenderMode() != REND_CLASSIC)
            gltexinvalidatetype(INVALIDATE_ART); // need update after tile copying
#endif
    return r;
}

static void MirrorPicsUninit(void)
{
    MIRROR* pRor;
    int i;
    
    i = mirrorcnt;
    while (--i >= 0)
    {
        pRor = &mirror[i];
        switch (pRor->type)
        {
            case OBJ_WALL:
                if (wall[pRor->thisID].type == kWallStack && IsMirrorTile(wall[pRor->thisID].overpicnum))
                    wall[pRor->thisID].overpicnum = pRor->picOld;
                else if (IsMirrorTile(wall[pRor->thisID].picnum))
                    wall[pRor->thisID].picnum = pRor->picOld;
                break;
            case OBJ_FLOOR:
                if (IsMirrorTile(sector[pRor->thisID].floorpicnum))
                    sector[pRor->thisID].floorpicnum = pRor->picOld;
                break;
            case OBJ_CEILING:
                if (IsMirrorTile(sector[pRor->thisID].ceilingpicnum))
                    sector[pRor->thisID].ceilingpicnum = pRor->picOld;
                break;
        }
    }

    if (mirrorPicEnd > mirrorPicStart)
        tileDeleteRange(mirrorPicStart, mirrorPicEnd);
}

void InitMirrors(void)
{
    walltype* pWall;
    int i, j, k, nLinkA, nLinkB;
    char rorTypeA, rorTypeB;
    int nRange = 0;

    memset(mirror, 0, sizeof(mirror));
    gMirrorDrawing = false;
    mirrorsector = -1;
    mirrorcnt = 0;

    i = numwalls; // Prepare wall mirrors and stacks
    while(--i >= 0 && mirrorcnt < kMaxMirrors)
    {
        pWall = &wall[i];
        if (pWall->overpicnum == kMirrorTile && pWall->extra > 0 && GetWallType(i) == kWallStack)
        {
            j = numwalls;
            while (--j >= 0)
            {
                if (j == i || wall[j].extra <= 0) continue;
                else if (GetWallType(j) != pWall->type) continue;
                else if (xwall[wall[j].extra].data != xwall[pWall->extra].data) continue;

                pWall->cstat               |= CSTAT_WALL_1WAY;
                pWall->hitag                = j;
                wall[j].hitag               = i;

                mirror[mirrorcnt].type      = OBJ_WALL;
                mirror[mirrorcnt].thisID    = i;
                mirror[mirrorcnt].id        = j;
                mirrorcnt++;
                nRange++;
                break;
            }
            
            if (j < 0)
                viewSetSystemMessage("Wall #%d has no matching wall link! (data = %d)", i, xwall[pWall->extra].data);
        }
        else if (pWall->picnum == kMirrorTile)
        {
            pWall->cstat               |= CSTAT_WALL_1WAY;
            pWall->overpicnum           = kMirrorTile;
            
            mirror[mirrorcnt].type      = OBJ_WALL;
            mirror[mirrorcnt].thisID    = i;
            mirror[mirrorcnt].id        = i;
            mirrorcnt++;
            nRange++;
        }
    }

    if (mirrorcnt > 0)
    {
        if (numsectors + 1 >= kMaxSectors || numwalls + 4 >= kMaxWalls)
        {
            viewSetSystemMessage("Must have at least %d sectors with %d walls free for mirrors!", 1, 4);
            mirrorcnt = 0; // cancel the wall mirrors
        }

        if (mirrorcnt > 0)
            InitMirrorSector();
    }

    i = numsectors; // Prepare sector stacks
    while(--i >= 0 && mirrorcnt < kMaxMirrors - 1)
    {
        if ((rorTypeA = IsRorSector(i, OBJ_FLOOR)) <= 0)
            continue;

        if ((nLinkA = gUpperLink[i]) < 0
            || (nLinkB = sprite[nLinkA].owner) < 0)
                continue;

        j = sprite[nLinkB].sectnum;
        if ((rorTypeB = IsRorSector(j, OBJ_CEILING)) <= 0)
            sector[j].ceilingpicnum = kMirrorTile; // force lower sector to be ROR

#ifdef MASKED_ROR_ISSUE
        if (rorTypeA == 2 || rorTypeB == 2)
        {
            viewSetSystemMessage("Sorry, masked sectors are not supported!");
            continue;
        }
#endif

        mirror[mirrorcnt].type      = OBJ_FLOOR;
        mirror[mirrorcnt].thisID    = i;
        mirror[mirrorcnt].id        = j;
            
        mirror[mirrorcnt].ofs.x     = sprite[nLinkB].x - sprite[nLinkA].x;
        mirror[mirrorcnt].ofs.y     = sprite[nLinkB].y - sprite[nLinkA].y;
        mirror[mirrorcnt].ofs.z     = sprite[nLinkB].z - sprite[nLinkA].z;

        nRange += picanm[sector[i].floorpicnum].num+1;
        mirrorcnt++;
                    
        mirror[mirrorcnt].type      = OBJ_CEILING;
        mirror[mirrorcnt].thisID    = j;
        mirror[mirrorcnt].id        = i;

        mirror[mirrorcnt].ofs.x     = sprite[nLinkA].x - sprite[nLinkB].x;
        mirror[mirrorcnt].ofs.y     = sprite[nLinkA].y - sprite[nLinkB].y;
        mirror[mirrorcnt].ofs.z     = sprite[nLinkA].z - sprite[nLinkB].z;

        nRange += picanm[sector[j].ceilingpicnum].num+1;
        mirrorcnt++;
    }

    if (MirrorPicsInit(nRange) < 0)
        mirrorcnt = 0; // cancel everything

    i = mirrorcnt;
    while (--i >= 0 && mirror[i].type != OBJ_WALL)
    {
        // Grouping splitted ROR sectors
        // for better and faster
        // drawing.

        if (mirror[i].hNeighID == 0)
        {
            j = 0;
            ROR_CollectNeighborsH(i, list, &j);
            for (k = 0; k < j - 1; k++)
                mirror[list[k]].hNeighID = list[k + 1];

            mirror[list[k]].hNeighID = list[0];
        }

        // Grouping ceilings and floors
        // for faster access.

        if (mirror[i].vNeighID == 0)
        {
            j = 0;
            ROR_CollectNeighborsV(i, list, &j);
            for (k = 0; k < j - 1; k++)
                mirror[list[k]].vNeighID = list[k + 1];

            mirror[list[k]].vNeighID = list[0];
        }
    }
}

static void TranslateMirrorColors(int nShade, int nPalette)
{
    if (videoGetRenderMode() != REND_CLASSIC)
        return;
    videoBeginDrawing();
    nShade = ClipRange(nShade, 0, 63);
    char *pMap = palookup[nPalette] + (nShade<<8);
    extern intptr_t frameplace;
    char *pFrame = (char*)frameplace;
    unsigned int nPixels = xdim*ydim;
    for (unsigned int i = 0; i < nPixels; i++, pFrame++)
    {
        *pFrame = pMap[*pFrame];
    }
    videoEndDrawing();
}

static int DoWallMirrors(int x, int y, int z, fix16_t a, fix16_t horiz, int smooth)
{
    walltype* pWall; MIRROR* pRor;
    int32_t nSect, nNextW, nNextS;
    int32_t didmirror, ca, dx, dy;
    int32_t i;

    for (i = 0; i < mirrorcnt && mirror[i].type == OBJ_WALL; i++)
    {
        if (!ROR_TestGotPic(i))
            continue;

        ROR_ClearGotPic(i);

        pRor = &mirror[i];
        gMirrorDrawing = true;

        pWall = &wall[pRor->id];
        nSect = sectorofwall(pRor->id);

        nNextW = pWall->nextwall;
        nNextS = pWall->nextsector;

        pWall->nextwall = mirrorwall[0];
        pWall->nextsector = mirrorsector;

        wall[mirrorwall[0]].nextwall        = pRor->id;
        wall[mirrorwall[0]].nextsector      = nSect;
        wall[mirrorwall[0]].x               = wall[pWall->point2].x;
        wall[mirrorwall[0]].y               = wall[pWall->point2].y;
        wall[mirrorwall[1]].x               = pWall->x;
        wall[mirrorwall[1]].y               = pWall->y;
        wall[mirrorwall[2]].x               = wall[mirrorwall[1]].x+(wall[mirrorwall[1]].x-wall[mirrorwall[0]].x)*16;
        wall[mirrorwall[2]].y               = wall[mirrorwall[1]].y+(wall[mirrorwall[1]].y-wall[mirrorwall[0]].y)*16;
        wall[mirrorwall[3]].x               = wall[mirrorwall[0]].x+(wall[mirrorwall[0]].x-wall[mirrorwall[1]].x)*16;
        wall[mirrorwall[3]].y               = wall[mirrorwall[0]].y+(wall[mirrorwall[0]].y-wall[mirrorwall[1]].y)*16;
            
        sector[mirrorsector].floorz         = sector[nSect].floorz;
        sector[mirrorsector].ceilingz       = sector[nSect].ceilingz;

        if (GetWallType(pRor->id) == kWallStack)
        {
            dx = x - (wall[pWall->hitag].x-wall[pWall->point2].x);
            dy = y - (wall[pWall->hitag].y-wall[pWall->point2].y);
            ca = a;
        }
        else
        {
            renderPrepareMirror(x, y, z, a, horiz, pRor->id, &dx, &dy, &ca);
        }

#ifdef POLYMER
        if (videoGetRenderMode() == REND_POLYMER)
            polymer_setanimatesprites(viewProcessSprites, dx, dy, z, fix16_to_int(ca), smooth);
#endif

        yax_preparedrawrooms();  
        didmirror = renderDrawRoomsQ16(dx, dy, z, ca, horiz, mirrorsector|kMaxSectors);    
        UpdateGotSectorSectorFX();
        yax_drawrooms(viewProcessSprites, mirrorsector, didmirror, smooth);
        viewProcessSprites(dx, dy, z, fix16_to_int(ca), smooth);
        renderDrawMasks();
                
        if (GetWallType(pRor->id) != kWallStack)
            renderCompleteMirror();
                
        if (pWall->pal || pWall->shade)
            TranslateMirrorColors(pWall->shade, pWall->pal);
                
        pWall->nextwall = nNextW;
        pWall->nextsector = nNextS;
        gMirrorDrawing = false;
        return 1;
    }

    return 0;
}

static int qsSortByDist(RORCAM *a, RORCAM *b) { return a->d - b->d; }
static int DoRoomOverRoom(int x, int y, int z, fix16_t a, fix16_t horiz, int smooth, int viewPlayer)
{
    // Current limitations:
    // 1. Can't see the wall mirrors/stacks and ROR at the same time.
    // 2. Can't see the RORs of other sectors through RORs.

    static int32_t hdrawcnt, vdrawcnt, i;
    
    MIRROR *pRor, *pOth; RORCAM* pCam; spritetype* pSpr; uint16_t* pSecStat;
    int32_t nIndex, t, n, oSprStat, oSectStat, dx, dy, dz;
    int32_t r = 0;

    hdrawcnt = 0;

    i = mirrorcnt;
    while(--i >= 0 && mirror[i].type != OBJ_WALL)
    {
        // First collect all the floors or ceilings
        // we are currently see
        // horizontally.
        
        if (!ROR_TestGotPic(i))
            continue;

        pCam = &hcam[hdrawcnt];
        pCam->d = INT32_MAX;

        n = i;
        do
        {
            pRor = &mirror[n];

            if (pCam->d > 0)
            {
                pOth = &mirror[ROR_GetOther(n)];
                
                if (inside(x, y, pOth->id))
                {
                    pCam->index = n;
                    pCam->d = 0; // Priority
                }
                else if (ROR_TestGotPic(n))
                {
                    if ((t = getDistToSect(pOth->id, x, y)) < pCam->d)
                    {
                        pCam->index = n;
                        pCam->d = t;
                    }
                }
            }

            ROR_ClearGotPic(n); // Must keep clearing for single drawing
            n = pRor->hNeighID;
        }
        while(n != i);
        
        hdrawcnt++;
    }
    
    // Sort the collected RORs by distance
    // so that closest to the camera
    // becomes first.
        
    if (hdrawcnt > 1)
        qsort((void*)hcam, hdrawcnt, sizeof(hcam[0]), (int(*)(const void*,const void*))qsSortByDist);
        
    while(--hdrawcnt >= 0)
    {
        // Processing from the most far to
        // the closest for better
        // covering.
        
        vdrawcnt = 0;
        pCam = &hcam[hdrawcnt];
        nIndex = pCam->index; dx = x, dy = y, dz = z;

        n = nIndex;
        do
        {
            // Keep adding rooms until we reach the most far vertically.
            // For ceilings search to the top and for
            // floors to the bottom. 
                    
            pRor = &mirror[n];
            ROR_ClearGotPic(n);
                        
            pCam = &vcam[vdrawcnt];
            pCam->index = n;
                        
            dx += pRor->ofs.x;
            dy += pRor->ofs.y;
            dz += pRor->ofs.z;
                        
            pCam->x = dx;
            pCam->y = dy;
            pCam->z = dz;
                     
            vdrawcnt++;

            n = pRor->vNeighID;
        }
        while(n != nIndex);
            
        r += vdrawcnt;
        
        while(--vdrawcnt >= 0)
        {
            // Drawing from the most far room to
            // the current for better
            // covering.
                
            pCam = &vcam[vdrawcnt];
            pRor = &mirror[pCam->index];

#ifdef USE_OPENGL
            r_rorphase = 1;
#endif
            if (viewPlayer >= 0)
            {
                pSpr = gPlayer[viewPlayer].pSprite;
                oSprStat = pSpr->cstat;

                pSpr->cstat |= (gViewPos == VIEWPOS_0)
                    ? (CSTAT_SPRITE_INVISIBLE) : (CSTAT_SPRITE_TRANSLUCENT_INVERT | CSTAT_SPRITE_TRANSLUCENT);
            }

#ifdef POLYMER
            if (videoGetRenderMode() == REND_POLYMER)
                polymer_setanimatesprites(viewProcessSprites, pCam->x, pCam->y, pCam->z, fix16_to_int(a), smooth);
#endif
            yax_preparedrawrooms();
            renderDrawRoomsQ16(pCam->x, pCam->y, pCam->z, a, horiz, pRor->id | kMaxSectors);
            UpdateGotSectorSectorFX();
            yax_drawrooms(viewProcessSprites, pRor->id, 0, smooth);
            
            viewProcessSprites(pCam->x, pCam->y, pCam->z, fix16_to_int(a), smooth);
            
            pSecStat = (pRor->type == OBJ_CEILING)
                ? &sector[pRor->id].floorstat : &sector[pRor->id].ceilingstat;

            oSectStat = *pSecStat, *pSecStat |= 0x01;
            
            renderDrawMasks();
            
            *pSecStat = oSectStat;
            
            if (viewPlayer >= 0)
                pSpr->cstat = oSprStat;

#ifdef USE_OPENGL
            r_rorphase = 0;
#endif
        }
    }
    
    return r;
}

void DrawMirrors(int x, int y, int z, fix16_t a, fix16_t horiz, int smooth, int viewPlayer)
{
    if (videoGetRenderMode() == REND_POLYMER)
        return;

    if (mirrorsector >= 0 && DoWallMirrors(x, y, z, a, horiz, smooth))
    {
        ROR_ClearGotPic();
        return;
    }
    
    if (DoRoomOverRoom(x, y, z, a, horiz, smooth, viewPlayer))
    {
        ROR_ClearGotPic();
        return;
    }
}

void sub_5571C(char mode)
{
    static MIRROR* pRor;
    static int i;
  
    i = mirrorcnt;
    while (--i >= 0 && mirror[i].type != OBJ_WALL)
    {
        if (!ROR_TestGotPic(i))
            continue;

        pRor = &mirror[i];
        switch (pRor->type)
        {
            case OBJ_CEILING:
                if (mode)
                    sector[pRor->thisID].ceilingstat |= 1;
                else
                    sector[pRor->thisID].ceilingstat &= ~1;
                break;
            case OBJ_FLOOR:
                if (mode)
                    sector[pRor->thisID].floorstat |= 1;
                else
                    sector[pRor->thisID].floorstat &= ~1;
                break;
        }
    }
}

void sub_557C4(int x, int y, int interpolation)
{
    static MIRROR* pRor;
    static int i;
    
    if (spritesortcnt == 0) return;
    int nViewSprites = spritesortcnt;
    for (int nTSprite = nViewSprites-1; nTSprite >= 0; nTSprite--)
    {
        tspritetype *pTSprite = &tsprite[nTSprite];
        pTSprite->xrepeat = pTSprite->yrepeat = 0;
    }

    i = mirrorcnt;
    while (--i >= 0 && mirror[i].type != OBJ_WALL)
    {
        if (!ROR_TestGotPic(i))
            continue;

        pRor = &mirror[i];
        int nSector = pRor->id;
        int nSector2 = pRor->thisID;
        for (int nSprite = headspritesect[nSector]; nSprite >= 0; nSprite = nextspritesect[nSprite])
        {
            spritetype *pSprite = &sprite[nSprite];
            if (pSprite == gView->pSprite)
                continue;
            int top, bottom;
            GetSpriteExtents(pSprite, &top, &bottom);
            int zCeil, zFloor;
            getzsofslope(nSector, pSprite->x, pSprite->y, &zCeil, &zFloor);
            if (pSprite->statnum == kStatDude && (top < zCeil || bottom > zFloor))
            {
                int j = ROR_GetOther(i);
                int dx = mirror[j].ofs.x;
                int dy = mirror[j].ofs.y;
                int dz = mirror[j].ofs.z;
                if (spritesortcnt < maxspritesonscreen)
                {
                    tspritetype *pTSprite = &tsprite[spritesortcnt];
                    memset(pTSprite, 0, sizeof(tspritetype));
                    pTSprite->type = pSprite->type;
                    pTSprite->index = pSprite->index;
                    pTSprite->sectnum = nSector2;
                    pTSprite->x = pSprite->x+dx;
                    pTSprite->y = pSprite->y+dy;
                    pTSprite->z = pSprite->z+dz;
                    pTSprite->ang = pSprite->ang;
                    pTSprite->picnum = pSprite->picnum;
                    pTSprite->shade = pSprite->shade;
                    pTSprite->pal = pSprite->pal;
                    pTSprite->xrepeat = pSprite->xrepeat;
                    pTSprite->yrepeat = pSprite->yrepeat;
                    pTSprite->xoffset = pSprite->xoffset;
                    pTSprite->yoffset = pSprite->yoffset;
                    pTSprite->cstat = pSprite->cstat;
                    pTSprite->statnum = kStatDecoration;
                    pTSprite->owner = pSprite->index;
                    pTSprite->extra = pSprite->extra;
                    pTSprite->flags = pSprite->hitag|0x200;
                    if (gViewInterpolate)
                    {
                        LOCATION *pLocation = &gPrevSpriteLoc[pSprite->index];
                        pTSprite->x = dx+interpolate(pLocation->x, pSprite->x, interpolation);
                        pTSprite->y = dy+interpolate(pLocation->y, pSprite->y, interpolation);
                        pTSprite->z = dz+interpolate(pLocation->z, pSprite->z, interpolation);
                        pTSprite->ang = pLocation->ang+mulscale16(((pSprite->ang-pLocation->ang+1024)&2047)-1024,interpolation);
                    }
                    if (!VanillaMode())
                    {
                        pTSprite->statnum = pSprite->statnum;
                        viewReplacePlayerAsCultist(pTSprite, pSprite->index, pSprite->extra);
                    }
                    spritesortcnt++;
                }
            }
        }
    }
    for (int nTSprite = spritesortcnt-1; nTSprite >= nViewSprites; nTSprite--)
    {
        tspritetype *pTSprite = &tsprite[nTSprite];
        int nAnim = 0;
        switch (picanm[pTSprite->picnum].extra&7)
        {
        case 1:
        {
            int dX = x - pTSprite->x;
            int dY = y - pTSprite->y;
            RotateVector(&dX, &dY, 128 - pTSprite->ang);
            nAnim = GetOctant(dX, dY);
            if (nAnim <= 4)
            {
                pTSprite->cstat &= ~4;
            }
            else
            {
                nAnim = 8 - nAnim;
                pTSprite->cstat |= 4;
            }
            break;
        }
        case 2:
        {
            int dX = x - pTSprite->x;
            int dY = y - pTSprite->y;
            RotateVector(&dX, &dY, 128 - pTSprite->ang);
            nAnim = GetOctant(dX, dY);
            break;
        }
        }
        while (nAnim > 0)
        {
            pTSprite->picnum += picanm[pTSprite->picnum].num+1;
            nAnim--;
        }
    }
}

class MirrorLoadSave : public LoadSave {
public:
    void Load(void);
    void Save(void);
};

static MirrorLoadSave *myLoadSave;

void MirrorLoadSave::Load(void)
{
    // from map with RORs to map without
    if (mirrorPicEnd > mirrorPicStart)
        tileDeleteRange(mirrorPicStart, mirrorPicEnd);

    Read(&mirrorcnt,            sizeof(mirrorcnt));
    Read(&mirrorPicStart,       sizeof(mirrorPicStart));
    Read(&mirrorPicEnd,         sizeof(mirrorPicEnd));
    Read(&mirrorsector,         sizeof(mirrorsector));
    Read(mirror,                sizeof(mirror[0]) * mirrorcnt);

    MirrorPicsUninit();
    
    if (mirrorcnt > 0)
    {
        MirrorPicsInit(mirrorPicEnd - mirrorPicStart);
        if (mirrorsector >= 0) // if there was any wall mirrors
            InitMirrorSector();
    }
}

void MirrorLoadSave::Save(void)
{
    Write(&mirrorcnt,           sizeof(mirrorcnt));
    Write(&mirrorPicStart,      sizeof(mirrorPicStart));
    Write(&mirrorPicEnd,        sizeof(mirrorPicEnd));
    Write(&mirrorsector,        sizeof(mirrorsector));
    Write(mirror,               sizeof(mirror[0]) * mirrorcnt);
}

void MirrorLoadSaveConstruct(void)
{
    myLoadSave = new MirrorLoadSave();
}

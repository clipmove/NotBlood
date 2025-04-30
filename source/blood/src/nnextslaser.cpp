//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 Nuke.YKT
Copyright (C) NoOne

A class that builds ray of tsprites similar to laser with support of sector
intersection. This file provides functionality for kModernLaserGen types
which is part of nnexts.cpp. More info at http://cruo.bloodgame.ru/xxsystem

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

#ifdef NOONE_EXTENSIONS
#include "nnextslaser.h"
#include "replace.h"
#include "tile.h"
#include "view.h"

#define kFlagLaserTrOnceHit         0x001
#define kFlagLaserTrOnceLast        0x002
#define kFlagLaserNoDudes           0x004
#define kFlagLaserNoPlayer          0x008
#define kFlagLaserNoThings          0x010
#define kFlagLaserNoMasks           0x040
#define kFlagLaserNoDead            0x080
#define kFlagLaserStrictCmd         0x200
#define kFlagLaserUseHitscan        0x400
#define kFlagLaserDynamicSets       0x800
#define DIST2PIX(a) (a << 8)

/**------------------------------------------
// Comment to save on tiles and use tsprite
// shade fading instead of
// tile animation.
**/
//#define LASER_SHD_USE_TILEANIM


/**-----------------------------------------
// Comment to not compensate view shade
// for tsprite (when put laser view
// processing after
// tsprites).
**/
//#ifndef LASER_SHD_USE_TILEANIM
//#define LASER_SHD_COMPENSATE
//#endif

unsigned char LASER::colors[kLaserMaxColors] =
{
    152,    // 0, LightRed
    191,    // 1, LightGreen
    123,    // 2, LightBlue
    159,    // 3, Yellow
    21,     // 4, LightGrey
    31,     // 5, White
    149,    // 6, Red
    189,    // 7, Green
    179,    // 8, Brown
    10,     // 9, DarkGray
};

LASER::CAMERA LASER::cam;
unsigned int LASER::tile[2][kLaserMaxColors];
unsigned int LASER::size[2][3][256];

LASER::LASER(spritetype* pLaser)
{
    pieces = NULL;
    Init(pLaser);
}

LASER::~LASER()
{
    RayDelete();
    if (pieces)
        free(pieces);
}

void LASER::Init(spritetype* pLaser)
{
    color               = 0;
    numpieces           = 0;
    piecescapacity      = 0;
    xrepeat             = 6;
    yrepeat             = 6;
    thickness           = 6;
    maxdist             = 0;
    raydone             = 0;

    oldscan.hitsprite   = -1;

    pOwn = pLaser;
    pXOwn = &xsprite[pOwn->extra];
    UpdateProperties();
    RayDelete();
}

void LASER::GenerateTiles(char nID)
{
    #define kTileW              48  // must be even for perfect positioning
    #define kTileH              16  // must be even for perfect positioning
    #define kGlobShdStep        8

    #ifdef LASER_SHD_USE_TILEANIM
    #define kFrames             32 / kGlobShdStep
    #else
    #define kFrames             1
    #endif

    char *pTileH, *pTileV, *pPlu;
    int l = kTileW*kTileH, i, f, j, k, g;
    int nColor = (unsigned char)colors[nID];
    int nLineShade, nStep;
    int nTileShade = 0;
    int nTile;

    for (i = 0; i < LENGTH(tile); i++)
    {
        if (tile[i][nID] > 0)
            return;
    }

    if ((nTile = tileSearchFreeRange(kFrames<<1)) >= 0)
    {
        for (i = nTile; i < nTile + kFrames; i++)
        {
            tileAllocTile(i,  kTileH, kTileW, 0, 0);
            pTileV = tileLoadTile(i);


            tileAllocTile(i + kFrames,  kTileW, kTileH, 0, 0);
            pTileH = tileLoadTile(i + kFrames);

            nLineShade  = 48;
            nStep       = -8;

            // first make gradient color for each line
            for (j = 0, f = 0; j < kTileH; j++, f+=kTileW)
            {
                pPlu = (char*)(palookup[0] + (qgetpalookup(0, nLineShade) << 8));
                for (k = f; k < f + kTileW; k++)
                    pTileV[k] = pPlu[nColor];

                if (j == (kTileH>>1) - 1)
                {
                    nStep = -nStep;
                }
                else
                {
                    nLineShade += nStep;
                }
            }

            // make the whole tile  darker
            pPlu = (char*)(palookup[0] + (qgetpalookup(0, nTileShade) << 8));
            for (j = 0; j < l; j++)
                pTileV[j] = pPlu[pTileV[j]];

            // rotate and copy vertical tile
            j = k = f = g = 0;
            while (j < l && f < l)
            {
                pTileH[j] = pTileV[k];

                f = j, g = k;
                while ((g += kTileW) < l && ++f < l)
                    pTileH[f] = pTileV[g];

                j+=kTileH;
                k++;
            }

            nTileShade += kGlobShdStep;
        }

        tile[1][nID]        = nTile;
        tile[0][nID]        = nTile + kFrames;

        #if (kFrames > 1)
            picanm[nTile].num = kFrames - 1;
            picanm[nTile].sf |= PICANM_ANIMTYPE_FWD;
            picanm[nTile].sf |= 1;

            picanm[nTile + kFrames] = picanm[nTile];
        #endif
    }
}

void LASER::GenerateTables()
{
    int x0, y0, x1, y1, zb, zt;
    int i, j, k;

    spritetype* pModel;
    int nSpr = InsertSprite(0, 0);
    dassert(nSpr >= 0);

    pModel = &sprite[nSpr];

    pModel->cstat |= (CSTAT_SPRITE_ALIGNMENT_WALL | CSTAT_SPRITE_YCENTER);
    pModel->x = pModel->y = pModel->z = 0;
    pModel->ang = 1536;

    for (i = 0; i < 2; i++)
    {
        pModel->picnum = 0;
        for (j = 0; j < LENGTH(tile[0]) && !pModel->picnum; j++)
            pModel->picnum = tile[i][j];

        for (k = 1; k < 256; k++)
        {
            pModel->xrepeat = k;
            pModel->yrepeat = k;

            GetSpriteExtents(pModel, &x0, &y0, &x1, &y1, &zt, &zb, 0x07);

            size[i][0][k] = klabs(x1-x0);
            size[i][2][k] = klabs(zb-zt);
            size[i][1][k] = size[i][2][k] >> 4;
        }
    }

    DeleteSprite(nSpr);
}

#ifdef LASER_SHD_COMPENSATE
int helperViewSpriteShade(int nShade, int nTile, int nSect)
{
    sectortype *pSect = &sector[nSect];
    if ((pSect->ceilingstat & kSecCParallax) && !(pSect->floorstat & kSecCFloorShade))
        nShade += pSect->ceilingshade + tileShade[pSect->ceilingpicnum];
    else
        nShade += pSect->floorshade + tileShade[pSect->floorpicnum];

    nShade += tileShade[nTile];
    return nShade;
}
#endif

tspritetype* LASER::Put(RAYPIECE* p)
{
    tspritetype* pSpr;

    #define kSprTranslucR (CSTAT_SPRITE_TRANSLUCENT|CSTAT_SPRITE_TRANSLUCENT_INVERT)

    if ((pSpr = viewInsertTSprite(p->s, 32767, (tspritetype*)pOwn)) != NULL)
    {
        pSpr->xrepeat   = p->w;
        pSpr->yrepeat   = p->h;
        pSpr->picnum    = GetTile();
        pSpr->ang       = (pOwn->ang + kAng90) & kAngMask;
        pSpr->pal       = pOwn->pal;
        pSpr->owner     = pOwn->index;

        if ((pOwn->cstat & kSprTranslucR) == CSTAT_SPRITE_TRANSLUCENT)
            pSpr->cstat |= CSTAT_SPRITE_TRANSLUCENT;
        else
            pSpr->cstat |= kSprTranslucR;

        #ifndef LASER_SHD_USE_TILEANIM
            pSpr->shade = Fade(0, -16, 8);
            #ifdef LASER_SHD_COMPENSATE
                pSpr->shade -= helperViewSpriteShade(0, pSpr->picnum, pSpr->sectnum);
            #endif
        #else
            pSpr->shade = -128;
        #endif

        pSpr->x     = p->x;
        pSpr->y     = p->y;
        pSpr->z     = p->z;
    }

    return pSpr;
}

char LASER::PiecePutH(RAYPIECE* p, int nDAng1, int nDAng2)
{
    UNREFERENCED_PARAMETER(nDAng2);
    int hys, hzs; tspritetype* pSpr;

    if (spritesortcnt + 2 >= maxspritesonscreen)
        return 0;

    hys = size[0][1][p->h] >> 1;
    hzs = size[0][2][p->h] >> 1;

    if (nDAng1 < 0)
    {
        // left
        pSpr = Put(p);
        pSpr->cstat |= (CSTAT_SPRITE_ALIGNMENT_WALL | CSTAT_SPRITE_ONE_SIDED);
        nnExtOffsetPos(-(hys-1), 0, 0, pOwn->ang, &pSpr->x, &pSpr->y, NULL);
    }
    else
    {
        // right
        pSpr = Put(p);
        pSpr->cstat |= (CSTAT_SPRITE_ALIGNMENT_WALL | CSTAT_SPRITE_ONE_SIDED);
        pSpr->ang = (pSpr->ang + kAng180) & kAngMask;
        nnExtOffsetPos(+(hys-1), 0, 0, pOwn->ang, &pSpr->x, &pSpr->y, NULL);
    }

    if (cam.z >= p->z + hzs)
    {
        // bot
        pSpr = Put(p);
        pSpr->cstat |= (CSTAT_SPRITE_ALIGNMENT_FLOOR | CSTAT_SPRITE_ONE_SIDED | CSTAT_SPRITE_YFLIP);
        nnExtOffsetPos(0, 0, (hzs-4), pOwn->ang, NULL, NULL, &pSpr->z);
    }
    else
    {
        // top
        pSpr = Put(p);
        pSpr->cstat |= (CSTAT_SPRITE_ALIGNMENT_FLOOR | CSTAT_SPRITE_ONE_SIDED);
        nnExtOffsetPos(0, 0, -(hzs-4), pOwn->ang, NULL, NULL, &pSpr->z);
    }

    return 1;
}

char LASER::PiecePutV(RAYPIECE* p, int nDAng1, int nDAng2)
{
    int hxs; tspritetype* pSpr;

    if (spritesortcnt + 2 >= maxspritesonscreen)
        return 0;

    hxs = size[1][0][p->w] >> 1;

    if (nDAng1 < 0)
    {
        // left
        pSpr = Put(p);
        pSpr->cstat |= (CSTAT_SPRITE_ALIGNMENT_WALL | CSTAT_SPRITE_ONE_SIDED), pSpr->cstat &= ~CSTAT_SPRITE_YCENTER;
        nnExtOffsetPos(-hxs, 0, 0, pOwn->ang, &pSpr->x, &pSpr->y, NULL);
    }
    else
    {
        // right
        pSpr = Put(p);
        pSpr->ang = ((pOwn->ang + kAng90) + kAng180) & kAngMask;
        pSpr->cstat |= (CSTAT_SPRITE_ALIGNMENT_WALL | CSTAT_SPRITE_ONE_SIDED), pSpr->cstat &= ~CSTAT_SPRITE_YCENTER;
        nnExtOffsetPos(hxs, 0, 0, pOwn->ang, &pSpr->x, &pSpr->y, NULL);

    }

    if (nDAng2 < 0)
    {
        // forward
        pSpr = Put(p);
        pSpr->ang = (pOwn->ang) & kAngMask;
        pSpr->cstat |= (CSTAT_SPRITE_ALIGNMENT_WALL | CSTAT_SPRITE_ONE_SIDED), pSpr->cstat &= ~CSTAT_SPRITE_YCENTER;
        nnExtOffsetPos(0, hxs, 0, pOwn->ang, &pSpr->x, &pSpr->y, NULL);
    }
    else
    {
        // backward
        pSpr = Put(p);
        pSpr->ang = ((pOwn->ang) + kAng180) & kAngMask;
        pSpr->cstat |= (CSTAT_SPRITE_ALIGNMENT_WALL | CSTAT_SPRITE_ONE_SIDED), pSpr->cstat &= ~CSTAT_SPRITE_YCENTER;
        nnExtOffsetPos(0, -hxs, 0, pOwn->ang, &pSpr->x, &pSpr->y, NULL);
    }

    return 1;
}

void LASER::PieceListAdd(RAYPIECE* pPiece)
{
    if (numpieces >= piecescapacity)
        PieceListGrow();

    SetBitString(piecesectmap, pPiece->s);
    pieces[numpieces++] = *pPiece;
}

void LASER::PieceListGrow(int nBy)
{
    piecescapacity = numpieces + nBy;
    pieces = (RAYPIECE*)realloc(pieces, sizeof(RAYPIECE) * piecescapacity);
    dassert(pieces != NULL);
}

void LASER::RayCollectPiecesH(int nSect, int nFullDist, POSOFFS* pos)
{
    int nDist = 0, sz, t;
    SCANWALL src, dst;

    src.pos = *pos;
    src.s = nSect;
    src.w = -1;

    RAYPIECE piece;
    piece.z = pos->z;
    piece.h = yrepeat;

    while(src.s >= 0 && nFullDist > 8)
    {
        src.pos.Forward(4);
        if (!scanWallOfSector(&src, &dst))
            return; // there is no space to shoot the ray

        nDist = Distance(pos->x - dst.pos.x, pos->y - dst.pos.y);
        nDist = ClipHigh(nDist, nFullDist);

        do
        {
            xrepeat = LookupRepeat(&size[0][0][0], nDist);
            sz = size[0][0][xrepeat];
            nFullDist -= sz;
            nDist -= sz;
            t = sz>>1;

            pos->Forward(t);

            piece.w = xrepeat;
            piece.x = pos->x;
            piece.y = pos->y;
            piece.s = src.s;

            PieceListAdd(&piece);

            pos->Forward(t);
        }
        while(nDist > 32);

        pos->Backward(1); // for less visable spaces

        src.pos = dst.pos, src.pos.a = pos->a;
        src.s   = wall[dst.w].nextsector;
        src.w   = wall[dst.w].nextwall;
    }
}

void LASER::RayCollectPiecesV(int nSect, int nDist, POSOFFS* pos)
{
    int sz;
    RAYPIECE piece;

    piece.x = pos->x;
    piece.y = pos->y;
    piece.w = xrepeat;
    piece.s = nSect;

    while(nDist > 8)
    {
        yrepeat = LookupRepeat(&size[1][2][0], nDist);
        sz = size[1][2][yrepeat];
        piece.h = yrepeat;
        nDist -= sz;

        if (direction)
        {
            piece.z = pos->z;
            pos->Up(sz);
        }
        else
        {
            pos->Down(sz);
            piece.z = pos->z;
        }

        PieceListAdd(&piece);
    }
}


void LASER::SetState(char nState)
{
    LASERSCAN* o = &oldscan;
    int nCmd;

    if (nState)
    {
        pXOwn->state = 1;
        o->hitsprite = -1;
        raydone = 0;
        Process();
    }
    else
    {
        if (pXOwn->triggerOff && o->hitsprite >= 0)
        {
            nCmd = (strictcmd)
                    ? kCmdOff : (COMMAND_ID)pXOwn->command;

            if (pXOwn->txID)
            {
                 evSend(pOwn->index, OBJ_SPRITE, pXOwn->txID, (COMMAND_ID)nCmd, o->hitsprite);
            }
            else
            {
                nnExtTriggerObject(OBJ_SPRITE, o->hitsprite, nCmd, o->hitsprite);
            }
        }

        pXOwn->state = raydone = 0;
        RayDelete();
    }
}

char LASER::UpdateProperties()
{
    char r = 0;
    char t;

    troncehit   = CheckFlags(kFlagLaserTrOnceHit);
    troncelast  = CheckFlags(kFlagLaserTrOnceLast);
    strictcmd   = CheckFlags(kFlagLaserStrictCmd);

    if (pXOwn->data1 != color && rngok(pXOwn->data1, 0, kLaserMaxColors))
    {
        LASER::GenerateTiles(pXOwn->data1);
        color = pXOwn->data1;
    }

    /** The following changes will require ray rebuilding **/
    /*******************************************************/

    if ((t = CheckFlags(kFlagLaserNoDudes)) != nodudes)         nodudes     = t,    r = 1;
    if ((t = CheckFlags(kFlagLaserNoPlayer)) != noplayers)      noplayers   = t,    r = 1;
    if ((t = CheckFlags(kFlagLaserNoThings)) != nothings)       nothings    = t,    r = 1;
    if ((t = CheckFlags(kFlagLaserNoMasks)) != nomasks)         nomasks     = t,    r = 1;
    if ((t = CheckFlags(kFlagLaserNoDead)) != nodead)           nodead      = t,    r = 1;
    if ((t = CheckFlags(kFlagLaserUseHitscan)) != usehitscan)   usehitscan  = t,    r = 1;
    hasignoreflags = (nodudes | noplayers | nothings | nomasks | nodead);

    t = ((pOwn->cstat & CSTAT_SPRITE_ALIGNMENT) == CSTAT_SPRITE_ALIGNMENT_FLOOR);
    if (t != vertical)
        vertical = t, r = 1;

    if (vertical)
    {
        t = ((pOwn->cstat & CSTAT_SPRITE_ONE_SIDED) != 0 && (pOwn->cstat & CSTAT_SPRITE_YFLIP) == 0);
        if (t != direction)
            direction = t, r = 1;
    }

    if (pXOwn->data2 != thickness && irngok(pXOwn->data2, 1, 255))
    {
        (vertical) ? xrepeat = pXOwn->data2 : yrepeat = pXOwn->data2;
        thickness = pXOwn->data2;
        r = 1;
    }

    if (pXOwn->data3 != maxdist)
        maxdist = pXOwn->data3, r = 1;

    return r;
}

void LASER::ScanV(LASERSCAN* l)
{
    int nDist = DIST2PIX(maxdist)<<4, nDist2 = 0;
    vec3_t pos = { l->srcx, l->srcy, l->srcz };
    int t, cz, fz, ch, fh;

    HITINFO* h = &gHitInfo;

    // Use getzrange because hitscan stuff
    // doesn't really hit sprites when
    // slope argument passed as
    // vertical

    while( 1 )
    {
        getzrange(&pos, l->srcs, &cz, &ch, &fz, &fh, 4, CLIPMASK1);
        l->hitz = (direction) ? cz : fz;
        h->hitsprite = -1;

        if (maxdist)
        {
            nDist2 += klabs(l->srcz-l->hitz);

            if (nDist2 > nDist)
            {
                l->hitz = l->srcz + ((direction) ? -nDist : nDist);
                break;
            }
        }

        if (direction)
        {
            if (ch >= 0 && (ch & 0xC000) == 0xC000)
            {
                t = ch & 0x3FFF;
                GetSpriteExtents(&sprite[t], &cz, &fz);
                if (l->hitz >= cz)
                    h->hitsprite = t;
            }
        }
        else if (fh >= 0 && (fh & 0xC000) == 0xC000)
        {
            t = fh & 0x3FFF;
            GetSpriteExtents(&sprite[t], &cz, &fz);
            if (l->hitz <= fz)
                h->hitsprite = t;
        }

        if (h->hitsprite >= 0
            && hasignoreflags && !SpriteAllowed(&sprite[h->hitsprite]))
            {
                t = klabs(fz-cz);
                if (direction)
                    t = -t;

                pos.z += t;
                continue;
            }

        break;
    }

    l->hitsprite    = h->hitsprite;
    l->hitx         = l->srcx;
    l->hity         = l->srcy;
}

void LASER::ScanH(LASERSCAN* l)
{
    HITINFO* h = &gHitInfo;
    int s, bs, a, ba, x, bx, y, by;
    int nDist, nCode;
    spritetype* pSpr;

    if (hasignoreflags)
    {
        nDist = DIST2PIX(maxdist)>>4;
        pSpr = pOwn;

        s = pSpr->sectnum;
        a = pOwn->ang;
        x = l->srcx;
        y = l->srcy;

        while(s >= 0)
        {
            bs = pSpr->sectnum, pSpr->sectnum   = s;
            ba = pSpr->ang,     pSpr->ang       = a;
            bx = pSpr->x,       pSpr->x         = x;
            by = pSpr->y,       pSpr->y         = y;

            nnExtOffsetPos(0, 4, 0, l->srca, &pSpr->x, &pSpr->y, NULL);

            if (usehitscan)
            {
                nCode = HitScan(pSpr, pSpr->z, Cos(l->srca) >> 16, Sin(l->srca) >> 16, 0, CLIPMASK1, 0);
            }
            else
            {
                nCode = VectorScan(pSpr, 0, 0, Cos(l->srca) >> 16, Sin(l->srca) >> 16, 0, 0, 0x01);
            }

            pSpr->sectnum   = bs;
            pSpr->ang       = ba;
            pSpr->x         = bx;
            pSpr->y         = by;

            if (maxdist
                && (nDist -= (Distance(h->hitx - x, h->hity - y)>>4)))
                {
                    // Somehow, i think there is something wrong
                    // with masked walls and max distance
                    // argument, so we clamp it
                    // manually

                    h->hitx = l->srcx; h->hity = l->srcy;
                    nnExtOffsetPos(0, DIST2PIX(maxdist), 0, l->srca, &h->hitx, &h->hity, NULL);
                    h->hitsprite = -1;
                    break;
                }

            x = h->hitx;
            y = h->hity;

            if (nCode == 4 && nomasks)
            {
                s = wall[h->hitwall].nextsector;
                nnExtOffsetPos(0, 2, 0, l->srca, &x, &y, NULL);
                continue;
            }

            if (h->hitsprite >= 0)
            {
                pSpr = &sprite[h->hitsprite];
                if (SpriteAllowed(pSpr))
                    break;

                h->hitsprite = -1;
                s = pSpr->sectnum;
                nnExtOffsetPos(0, 2, 0, l->srca, &x, &y, NULL);
                continue;
            }

            break;
        }
    }
    else
    {
        nnExtOffsetPos(0, 4, 0, l->srca, &pOwn->x, &pOwn->y, NULL);

        if (usehitscan)
        {
            HitScan(pOwn, pOwn->z, Cos(l->srca) >> 16, Sin(l->srca) >> 16, 0, CLIPMASK1, DIST2PIX(maxdist) >> 4);
        }
        else
        {
            VectorScan(pOwn, 0, 0, Cos(l->srca) >> 16, Sin(l->srca) >> 16, 0, DIST2PIX(maxdist) >> 4, 0x01);
        }

        pOwn->x = l->srcx; pOwn->y = l->srcy;
    }

    l->hitsprite    = h->hitsprite;
    l->hitx         = h->hitx;
    l->hity         = h->hity;
    l->hitz         = h->hitz;
}

void LASER::Process()
{
    static POSOFFS pos;
    LASERSCAN *o = &oldscan, *n = &newscan;
    int nOSpr = o->hitsprite;
    int nHSpr, nCmd;

    n->srcs = pOwn->sectnum;
    n->srca = pOwn->ang;
    n->srcx = pOwn->x;
    n->srcy = pOwn->y;
    n->srcz = pOwn->z;
    n->srch = 0;

    if (CheckFlags(kFlagLaserDynamicSets) && UpdateProperties())
        raydone = 0;

    if (vertical)
    {
        ScanV(n);
    }
    else
    {
        ScanH(n);
    }

    nHSpr = n->hitsprite;

    if (!pXOwn->locked && !pXOwn->isTriggered)
    {
        if (pXOwn->triggerOn)
        {
            // trigger at hit
            if (nHSpr >= 0 && (!troncehit || nHSpr != nOSpr))
            {
                nCmd = (strictcmd)
                        ? kCmdOn : (COMMAND_ID)pXOwn->command;

                if (pXOwn->txID)
                {
                    evSend(pOwn->index, OBJ_SPRITE, pXOwn->txID, (COMMAND_ID)nCmd, nHSpr);
                }
                else
                {
                    nnExtTriggerObject(OBJ_SPRITE, nHSpr, nCmd, nHSpr);
                }
            }
        }

        // trigger at miss
        if (troncelast && nOSpr >= 0 && nHSpr != nOSpr)
        {
            nCmd = (strictcmd)
                        ? kCmdOff : (COMMAND_ID)pXOwn->command;

            if (pXOwn->txID)
            {
                evSend(pOwn->index, OBJ_SPRITE, pXOwn->txID, (COMMAND_ID)nCmd, nOSpr);
            }
            else
            {
                nnExtTriggerObject(OBJ_SPRITE, nOSpr, nCmd, nOSpr);
            }
        }
    }

    o->hitsprite = nHSpr;

    if (!raydone || AreScansDifferent())
    {
        pos.New(n->srca, n->srcx, n->srcy, n->srcz);
        memcpy(o, n, sizeof(LASERSCAN));
        RayDelete();
        raydone = 1;

        if (vertical)
        {
            RayCollectPiecesV(n->srcs, klabs(n->hitz - n->srcz), &pos);
        }
        else
        {
            RayCollectPiecesH(n->srcs, Distance(n->srcx - n->hitx, n->srcy - n->hity), &pos);
        }
    }
}

char LASER::RayShow(int nSect)
{
    enum
    {
        kRetnSTOP   = 0,
        kRetnSKIP   = 1,
        kRetnOKAY   = 2,
    };

    if (!TestBitString(piecesectmap, nSect))
        return kRetnSKIP;

    int nAng, xAng, yAng;
    int i = numpieces;
    RAYPIECE* pPiece;

    while(--i >= 0)
    {
        pPiece = &pieces[i];
        if (pPiece->s == nSect)
        {
            nAng = getangle(pPiece->x-cam.x, pPiece->y-cam.y);
            yAng = DANGLE(nAng, pOwn->ang + kAng90);
            xAng = DANGLE(nAng, pOwn->ang);

            if (vertical)
            {
                if (!PiecePutV(pPiece, xAng, yAng))
                    return kRetnSTOP;
            }
            else
            {
                if (!PiecePutH(pPiece, xAng, yAng))
                    return kRetnSTOP;
            }

        }
    }

    return kRetnOKAY;
}

char LASER::SpriteAllowed(spritetype* pSpr)
{
    const int n = pSpr->inittype;

    if (nodead)
    {
        if ((pSpr->extra <= 0 || !xsprite[pSpr->extra].health)
            || (rngok(n, kDudeBase, kDudeMax) && pSpr->statnum == kStatThing))
                return 0; // could be dude turned in gib
    }

    if (nodudes && rngok(n, kDudeBase, kDudeMax) && !irngok(n, kDudePlayer1, kDudePlayer8))     return 0;
    else if (noplayers && irngok(n, kDudePlayer1, kDudePlayer8))                                return 0;
    else if (nothings && pSpr->statnum == kStatThing)                                           return 0;
    else                                                                                        return 1;
}

int LASER::LookupRepeat(unsigned int* pSiz, int nDist, unsigned int nRep)
{
    while(nRep > 1 && pSiz[nRep--] > (unsigned int)nDist);
    return nRep;
}

#if 0
char LASER::PieceBehindCam(RAYPIECE* pPiece)
{
    #define kPeriphery kAng90

    int sz = size[0][0][pPiece->w]>>1;
    int x, y;

    x = pPiece->x; y = pPiece->y;
    nnExtOffsetPos(0, +sz, 0, pOwn->ang, &x, &y, NULL);
    if (klabs(DANGLE(getangle(x-cam.x, y-cam.y), cam.a)) < kPeriphery)
        return 0;

    x = pPiece->x; y = pPiece->y;
    nnExtOffsetPos(0, -sz, 0, pOwn->ang, &x, &y, NULL);
    if (klabs(DANGLE(getangle(x-cam.x, y-cam.y), cam.a)) <= kPeriphery)
        return 0;

    return 1;
}
#endif
#endif
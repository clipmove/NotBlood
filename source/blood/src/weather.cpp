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
#include "gameutil.h"
#include "levels.h"
#include "trig.h"
#include "weather.h"

CWeather gWeather;

CWeather::CWeather()
{
    nDraw.b = 0;
    nWidth = 0;
    nHeight = 0;
    nOffsetX = 0;
    nOffsetY = 0;
    nScreenPitch = 0;
    nScaleFactor = 0;
    nCount = 0;
    nLimit = kMaxVectors;
    nGravity = 0;
    nWindX = 0;
    nWindY = 0;
    nWindXOffset = 0;
    nWindYOffset = 0;
    memset(nPos, 0, sizeof(nPos));
    nPalColor = 1;
    nPalShift = 0;
    nFadeIn = 16;
    nFadeOut = 56;
    memset(nColorTable, 0, sizeof(nColorTable));
    nScaleTableWidth = 0;
    nScaleTableFov = 0;
    nFovV = 0;
    nAspectRatioModifier = 0;
    nFovPitchModifier = 0;
    memset(nScaleTable, 0, sizeof(nScaleTable));
    nLastFrameClock = 0;
    nWeatherCheat = WEATHERTYPE_NONE;
    nWeatherCur = WEATHERTYPE_NONE;
    nWeatherForecast = WEATHERTYPE_NONE;
    nWeatherOverride = 0;
    nWeatherOverrideType = WEATHERTYPE_NONE;
    nWeatherOverrideTypeInside = WEATHERTYPE_NONE;
    nWeatherOverrideWindX = 0;
    nWeatherOverrideWindY = 0;
    nWeatherOverrideGravity = 0;
    nSectorChecked = -1;
    nSectorCheckedFound = -1;
    nSectorCheckedFoundForward = -1;
    nSectorCheckedTime = 0;
    nSectorCheckedAng = 0;
}

CWeather::~CWeather()
{
    nDraw.b = 0;
    nWidth = 0;
    nHeight = 0;
    nOffsetX = 0;
    nOffsetY = 0;
    nScreenPitch = 0;
    nScaleFactor = 0;
    nCount = 0;
    nLimit = kMaxVectors;
    nGravity = 0;
    nWindX = 0;
    nWindY = 0;
    nWindXOffset = 0;
    nWindYOffset = 0;
    memset(nPos, 0, sizeof(nPos));
    nPalColor = 1;
    nPalShift = 0;
    nFadeIn = 16;
    nFadeOut = 56;
    memset(nColorTable, 0, sizeof(nColorTable));
    nScaleTableWidth = 0;
    nScaleTableFov = 0;
    nFovV = 0;
    nAspectRatioModifier = 0;
    nFovPitchModifier = 0;
    memset(nScaleTable, 0, sizeof(nScaleTable));
    nLastFrameClock = 0;
    nWeatherCheat = WEATHERTYPE_NONE;
    nWeatherCur = WEATHERTYPE_NONE;
    nWeatherForecast = WEATHERTYPE_NONE;
    nWeatherOverride = 0;
    nWeatherOverrideType = WEATHERTYPE_NONE;
    nWeatherOverrideTypeInside = WEATHERTYPE_NONE;
    nWeatherOverrideWindX = 0;
    nWeatherOverrideWindY = 0;
    nWeatherOverrideGravity = 0;
    nSectorChecked = -1;
    nSectorCheckedFound = -1;
    nSectorCheckedFoundForward = -1;
    nSectorCheckedTime = 0;
    nSectorCheckedAng = 0;
}

void CWeather::RandomizeVectors(void)
{
    for (int i = 0; i < kMaxVectors; i++)
    {
        nPos[i][0] = krand()&0x3fff;
        nPos[i][1] = krand()&0x3fff;
        nPos[i][2] = krand()&0x3fff;
    }
    nWindXOffset = krand()&0x3fff;
    nWindYOffset = krand()&0x3fff;
}

void CWeather::SetViewport(int nX, int nY, int nXOffset0, int nXOffset1, int nYOffset0, int nYOffset1, int nFov, int nAspect)
{
    nWidth = (nXOffset1-nXOffset0)+1; // based off calculations from setview()
    nHeight = (nYOffset1-nYOffset0)+1;
    nOffsetX = nXOffset0;
    nOffsetY = nYOffset0;
    if (nX < 320 || nY < 200 || nOffsetX < 0 || nOffsetY < 0 || nWidth + nOffsetX > nX || nHeight + nOffsetY > nY) // something went very wrong, disable weather effects
    {
        SetWeatherType(WEATHERTYPE_NONE, 0);
        nDraw.bActive = 0;
        return;
    }
    nDraw.bActive = 1;
    nScaleFactor = mulscale16(nWidth<<16, nAspect); // based off calculations from setaspect()
    nScaleFactor = divscale16(nScaleFactor, 320<<16);
    nScreenPitch = nX; // store framebuffer offset for Y offset calculation
    if (nScaleTableFov != nFov)
    {
        nFovV = nFov;
    }
    if (nScaleTableFov != nFov || nScaleTableWidth != nWidth)
    {
        nScaleTable[0] = 0; // index 0 is unused (depth 0 is invalid/culled earlier) - set to 0 to be safe
        const int nNumerator = nWidth<<15;
        for (int i = 1; i < kScaleTableSize; i++)
        {
            const int nScale = divscale16(nNumerator, i<<16);
            nScaleTable[i] = divscale16(nScale, nFovV);
        }
    }
    nScaleTableWidth = nWidth;
    nScaleTableFov = nFov;
    nAspectRatioModifier = r_usenewaspect ? divscale16((nWidth<<16)*3, (nHeight<<16)*4) : fix16_one; // calculate correct screen ratio
    nFovPitchModifier = divscale16(nScaleFactor, nFovV);
}

void CWeather::SetParticles(short nCount, short nLimit)
{
    dassert(nCount >= 0 && nCount <= kMaxVectors);
    if (nLimit < 0 || nLimit > kMaxVectors)
        nLimit = kMaxVectors;
    if (!nCount) // if set particles to 0, set count to max and switch weather to none so weather skips fade in
    {
        this->nCount = kMaxVectors;
        SetWeatherType(WEATHERTYPE_NONE, 0);
        nWeatherCur = nWeatherForecast = WEATHERTYPE_NONE;
        return;
    }
    this->nCount = nCount;
    this->nLimit = nLimit;
}

void CWeather::SetGravity(short nGravity, char bVariance)
{
    this->nGravity = nGravity;
    this->nDraw.bGravityVariance = bVariance & 1;
}

void CWeather::SetWind(short nX, short nY)
{
    nWindX = nX;
    nWindY = nY;
}

void CWeather::RandomWind(char bHeavyWind, unsigned int uMapCRC)
{
    nWindX = !bHeavyWind ? (uMapCRC&0x3f) - 0x20 : (uMapCRC&0x7f) - 0x40;
    uMapCRC >>= 16;
    nWindY = !bHeavyWind ? (uMapCRC&0x3f) - 0x20 : (uMapCRC&0x7f) - 0x40;
}

void CWeather::SetTranslucency(int a1)
{
    nDraw.nTransparent = ClipRange(a1, 0, 2);
}

void CWeather::SetColor(unsigned char a1)
{
    nPalColor = ClipRange(a1, 1, 255); // color picked is always minus 1, so don't allow anything less than 1
    UpdateColorTable();
}

void CWeather::SetColorShift(char a1)
{
    nPalShift = ClipRange(a1, 0, 2);
    UpdateColorTable();
}

void CWeather::SetFade(char nIn, char nOut)
{
    nFadeIn = ClipRange(nIn, 1, 255);
    nFadeOut = ClipRange(nOut, 1, 255);
}

void CWeather::UpdateColorTable(void)
{
    const int kDepthStep = 8192 / kColorTableSize; // potential range for nDepth is 5-8191, or 0-31 after shifting
    for (int i = 0, nDepth = 0; i < kColorTableSize; i++, nDepth += kDepthStep)
    {
        const int nDepthColorShift = ClipLow(nDepth >> (nPalShift + 8), 1); // max range is 1-31
        const uint8_t nColor = ClipRange(nPalColor - nDepthColorShift, 0, 255); // clamp color
        nColorTable[i] = nColor;
    }
}

void CWeather::SetShape(char a1)
{
    nDraw.bShape = a1&3;
}

void CWeather::SetStaticView(char a1)
{
    nDraw.bStaticView = a1;
}

void CWeather::Initialize(void)
{
    nDraw.bActive = 1;
    nScaleTableWidth = 0;
    nScaleTableFov = 0;
    nFovV = 0;
    Restart();
}

void CWeather::Restart(void)
{
    nLastFrameClock = 0;
    nWeatherCur = WEATHERTYPE_NONE;
    nWeatherForecast = WEATHERTYPE_NONE;
    UnloadPreset();
    nSectorChecked = -1;
    nSectorCheckedFound = -1;
    nSectorCheckedFoundForward = -1;
    nSectorCheckedTime = 0;
    nSectorCheckedAng = 0;
    RandomizeVectors();
    SetParticles(0);
}

void CWeather::Draw(char *pBuffer, int nWidth, int nHeight, int nOffsetX, int nOffsetY, int nX, int nY, int nZ, int nAng, int nHoriz, int nCount, int nDelta)
{
    dassert(pBuffer != NULL);
    dassert(nCount > 0 && nCount <= kMaxVectors);

    // move to first pixel within framebuffer
    pBuffer += nScreenPitch * nOffsetY + nOffsetX;

    // adjust to starfield relative scale
    if (!nDraw.bStaticView)
    {
        nX <<= 1;
        nY <<= 1;
        nZ >>= 3;
    }
    else
    {
        nX = 0;
        nY = 0;
        nZ = 0;
    }

    // calculate wind offsets
    if (nWindX || nWindY)
    {
        const int nWindDeltaX = mulscale16(nWindX, nDelta);
        const int nWindDeltaY = mulscale16(nWindY, nDelta);
        nWindXOffset += nWindDeltaX;
        nWindYOffset += nWindDeltaY;
        const short nAntiWindX = nWindDeltaX>>2;
        const short nAntiWindY = nWindDeltaY>>2;
        for (int i = 0; i < nCount; i += 40) // add random variance by slowing the wind down for a few particles
        {
            nPos[i][1] += nAntiWindX;
            nPos[i][0] += nAntiWindY;
        }
    }
    nX += nWindXOffset;
    nY += nWindYOffset;

    // use drawrooms() calculation to find horizon and center of viewport
    nHoriz = (mulscale16(nHoriz-(100<<16), nFovPitchModifier)>>16)+(nHeight>>1);

    const int nCos = Cos(nAng)>>16;
    const int nSin = Sin(nAng)>>16;
    const int bShape = nDraw.bShape;
    const int bTransparent = nDraw.nTransparent;
    const int nMaxPixelSize = (nScaleFactor>>16)+1; // use screen res as factor for pixel size
    const int nGrav = mulscale16(nGravity, nDelta);
    const int nGravityFast = nDraw.bGravityVariance && (nGrav != 0) ? nGrav - (nGrav >> 2) : nGrav;

    for (int i = 0; i < nCount; i++)
    {
        // calculate and wrap X/Y
        const int relX = ((nPos[i][1] - nX)&0x3fff) - 0x2000;
        const int relY = ((nPos[i][0] - nY)&0x3fff) - 0x2000;

        // depth in rotated/view space
        const int nDepth = (relX * nCos + relY * nSin)>>14;
        if (nDepth <= 4)
            continue;

        // cull by radius in rotated space
        const int nLatOffset = (relY * nCos - relX * nSin)>>14;
        if (nDepth * nDepth + nLatOffset * nLatOffset >= 0x4000000) // < 8192*8192
            continue;

        // perspective scale with fov adjustment (uses precomputed table instead of divscale16)
        const int nScale = nScaleTable[nDepth]; // potential range for nDepth is 5-8191
        const unsigned int screenX = (divscale16(nLatOffset * nScale, nAspectRatioModifier)>>16) + (nWidth>>1);
        nPos[i][2] += i&4 ? nGravityFast : nGrav;
        if (screenX < (unsigned)nWidth) // if within screen bounds
        {
            // wrapping/centering logic for Z
            const int relZ = ((nPos[i][2] - nZ)&0x3fff) - 0x2000;
            unsigned int screenY = nHoriz + ((nScale * relZ)>>16);
            if (screenY < (unsigned)nHeight) // if within screen bounds
            {
                // size/palette color calculation
                const int nSize = ClipHigh(nScale>>12, nMaxPixelSize); // why did I pick 12? because it looked the best
                const uint8_t nColor = nColorTable[nDepth>>8]; // potential range for nDepth is 5-8191, or 0-31 after shift

                if (nSize <= 1) // if size is a pixel, don't bother calculating box fill
                {
                    for (int j = 1<<bShape; j > 0 && screenY > 0; j--, screenY--)
                    {
                        char *pDest = pBuffer + (nScreenPitch * screenY) + screenX;
                        switch (bTransparent)
                        {
                        case 0:
                            *pDest = nColor;
                            break;
                        case 1:
                            *pDest = blendtable[0][(nColor << 8) + *pDest];
                            break;
                        case 2:
                            *pDest = blendtable[0][(*pDest << 8) + nColor];
                            break;
                        }
                    }
                    continue;
                }
                // do block fill
                const int cx = (int)screenX;
                const int cy = (int)screenY;
                const int nHalfX = nSize >> 1;
                const int nHalfY = !bShape ? nSize>>1 : nSize * bShape;
                switch (bTransparent)
                {
                case 0: // opaque path: fill size x size
                {
                    int yStart = cy - nHalfY;
                    int yEnd   = cy + nHalfY;
                    if (yStart < 0) yStart = 0;
                    if (yEnd >= nHeight) yEnd = nHeight - 1;

                    int xStart = cx - nHalfX;
                    int xEnd   = cx + nHalfX;
                    if (xStart < 0) xStart = 0;
                    if (xEnd >= nWidth) xEnd = nWidth - 1;

                    if (yStart <= yEnd && xStart <= xEnd)
                    {
                        size_t len = (size_t)(xEnd - xStart + 1);
                        for (int yy = yStart; yy <= yEnd; yy++)
                        {
                            char *pDest = pBuffer + (nScreenPitch * yy) + xStart;
                            memset(pDest, (int)nColor, len);
                        }
                    }
                    break;
                }
                case 1: // translucency/blend path: blend per-pixel using transluc table
                {
                    for (int yy = cy - nHalfY; yy <= cy + nHalfY; yy++)
                    {
                        if ((unsigned)yy >= (unsigned)nHeight) continue;
                        char *pDest = pBuffer + (nScreenPitch * yy);
                        for (int xx = cx - nHalfX; xx <= cx + nHalfX; xx++)
                        {
                            if ((unsigned)xx >= (unsigned)nWidth) continue;
                            uint8_t dst = (uint8_t)pDest[xx];
                            pDest[xx] = blendtable[0][(nColor << 8) + dst];
                        }
                    }
                    break;
                }
                case 2: // translucency/blend path: blend per-pixel using transluc table
                {
                    for (int yy = cy - nHalfY; yy <= cy + nHalfY; yy++)
                    {
                        if ((unsigned)yy >= (unsigned)nHeight) continue;
                        char *pDest = pBuffer + (nScreenPitch * yy);
                        for (int xx = cx - nHalfX; xx <= cx + nHalfX; xx++)
                        {
                            if ((unsigned)xx >= (unsigned)nWidth) continue;
                            uint8_t dst = (uint8_t)pDest[xx];
                            pDest[xx] = blendtable[0][(dst << 8) + nColor];
                        }
                    }
                    break;
                }
                }
            }
        }
    }
}

void CWeather::Draw(int nX, int nY, int nZ, int nAng, int nHoriz, int nClock, int nInterpolate, unsigned int uMapCRC)
{
    if (!IsActive())
        return;
    nClock += mulscale16(1, nInterpolate<<2);
    int nDelta = (nClock - nLastFrameClock)<<16;
    nLastFrameClock = nClock;
    int nCountLimited = GetCount(); // get count with limit applied
    if (nCountLimited > 0)
    {
        videoBeginDrawing();
        char *framebuffer = (char*)frameplace;
        if (framebuffer != NULL)
            Draw(framebuffer, nWidth, nHeight, nOffsetX, nOffsetY, nX, nY, nZ, nAng, nHoriz, nCountLimited, nDelta);
        videoEndDrawing();
    }
    nDelta >>= 16;
    if (nWeatherForecast == nWeatherCur) // increase until reached weather limit
    {
        nCountLimited += nDelta * (int)nFadeIn;
        SetCount(nCountLimited);
    }
    else if (nCountLimited && (nWeatherForecast != nWeatherCur)) // changed weather type, fade out then switch to new weather type
    {
        nCountLimited -= nDelta * (int)nFadeOut;
        SetCount(nCountLimited);
    }
    else if (!nCountLimited && (nWeatherForecast != WEATHERTYPE_NONE)) // fade out complete, now switch to new weather
    {
        nWindXOffset = krand()&0x3fff;
        nWindYOffset = krand()&0x3fff;
        SetWeatherType(nWeatherForecast, uMapCRC);
    }
}

void CWeather::LoadPreset(unsigned int uMapCRC)
{
    nWeatherCheat = WEATHERTYPE_NONE;
    switch (uMapCRC)
    {
    case 0xBBF1A5D5: // e1m3
    case 0xF524ACA4: // e5m2
    case 0xFE99F0E7: // e5m6
        SetWeatherOverride(WEATHERTYPE_SNOWHARD, WEATHERTYPE_DUST, 0, -96, 32);
        break;
    case 0xAEC06508: // e1m5
    case 0xFA1A3218: // e4m1
    case 0x2D6A6F3D: // e4m3
    case 0x98FDBE0E: // e6m4
    case 0xEEC4C591: // dwe2m5 (2.0.1)
    case 0xD36FCE2C: // dwe2m5 (2.0)
    case 0xF53BE871: // dwe2m7 (2.0.1)
    case 0xEAD60AD8: // dwe2m7 (2.0)
        SetWeatherOverride(WEATHERTYPE_RAINHARD, WEATHERTYPE_DUST, 32, 0, 96);
        break;
    case 0xD2230111: // dwe2m1 (2.0.1)
    case 0xDD0134A5: // dwe2m1 (2.0)
    case 0x2C3E5A1D: // dwe2m6
    case 0x81BC32DB: // dwe4m3 (2.0.1)
    case 0x0304987F: // dwe4m3 (2.0)
        SetWeatherOverride(WEATHERTYPE_RAINHARD, WEATHERTYPE_DUST, 0, -96, 96);
        break;
    case 0x76937196: // e1m1
    case 0xBA5DB227: // e1m2
    case 0xCA80EAA3: // e2m1
    case 0x29D27D07: // e2m2
    case 0xE6B88CA6: // e2m3
    case 0x6AF2A719: // e2m4
    case 0xA0639DE5: // e4m6
    case 0xAD4AB780: // dwe1m3
    case 0x08E13935: // dwe2m8
    case 0x46FE750F: // dwe3m10 (2.0.1)
    case 0x2DDDF1C7: // dwe3m10 (2.0)
    case 0x3C75227E: // dwe3m12
    case 0xB221E1D3: // dwe4m10 (2.0.1)
    case 0x0FCD5D63: // dwe4m10 (2.0)
    case 0x816F43E4: // dwe4m12
        SetWeatherOverride(WEATHERTYPE_SNOW, WEATHERTYPE_DUST, 0, 4, 24);
        break;
    case 0xD64D2666: // e2m5
    case 0x09E3434D: // e2m6
    case 0x0FFF85AC: // e2m7
    case 0x602296E1: // e2m8
    case 0xF369A447: // e2m9
        SetWeatherOverride(WEATHERTYPE_SNOWHARD, WEATHERTYPE_DUST, 0, -32, 32);
        break;
    case 0xE898B54C: // e4m4
        SetWeatherOverride(WEATHERTYPE_DUST, WEATHERTYPE_DUST, 0, 1, 1);
        break;
    case 0xCB7A97D6: // e6m2
    case 0xED3F7DDD: // dwe1m1
    case 0xE6300128: // dwe1m2
    case 0x46CD9019: // dwe2m2
    case 0x2C5C62ED: // dwe2m4
    case 0xB1CECBA0: // dwe2m12 (2.0.1)
    case 0xA1CCA0BB: // dwe2m12 (2.0)
    case 0x4906E33F: // dwe3m2
    case 0x576BD280: // dwe3m3 (2.0.1)
    case 0xB62F2696: // dwe3m3 (2.0)
    case 0x553C726B: // dwe3m4 (2.0.1)
    case 0xF87D299D: // dwe3m4 (2.0)
    case 0x13E99900: // dwe3m5
    case 0x7DC075E8: // dwe3m6 (2.0.1)
    case 0x39B69AE0: // dwe3m6 (2.0)
    case 0x53A7E95F: // dwe3m8 (2.0.1)
    case 0xF108B090: // dwe3m8 (2.0)
    case 0x5E335E92: // dwe4m2
    case 0x163E6891: // dwe4m6 (2.0.1)
    case 0x7A914A32: // dwe4m6 (2.0)
    case 0x06DC56EC: // dwe4m8
        SetWeatherOverride(WEATHERTYPE_RAIN, WEATHERTYPE_DUST, 0, -16, 96);
        break;
    case 0xC3B72664: // e3m7
    case 0xFA3CEC6B: // e4m5
    case 0x9C722FD2: // dwe2m9
        SetWeatherOverride(WEATHERTYPE_LAVA, WEATHERTYPE_LAVA, 0, 0, 6);
        break;
    case 0x9AF82E43: // dwe3m1 (2.0.1)
    case 0x68339F0E: // dwe3m1 (2.0)
    case 0x23942FBB: // dwe3m7 (2.0.1)
    case 0x59A0D871: // dwe3m7 (2.0)
        SetWeatherOverride(WEATHERTYPE_NONE, WEATHERTYPE_DUST, 0, -16, 96);
        break;
    case 0x86E14FD4: // dwe3m9 (2.0.1)
    case 0xC1F2EA93: // dwe3m9 (2.0)
        SetWeatherOverride(WEATHERTYPE_NONE, WEATHERTYPE_NONE, 0, -16, 96);
        break;
    case 0xC583C1D3: // dwe4m1 (2.0.1)
    case 0x20B49994: // dwe4m1 (2.0)
    case 0xA3757FD5: // dwe4m4 (2.0.1)
    case 0x93B50A9F: // dwe4m4 (2.0)
    case 0x606DC94F: // dwe4m7 (2.0.1)
    case 0xFA05C913: // dwe4m7 (2.0)
    case 0xBAD00C54: // dwe4m9 (2.0.1)
    case 0x58778C35: // dwe4m9 (2.0)
        SetWeatherOverride(WEATHERTYPE_RAIN, WEATHERTYPE_NONE, 0, -16, 96);
        break;
    default:
        if (nWeatherOverride)
            UnloadPreset();
        break;
    }
}

void CWeather::UnloadPreset(void)
{
    nWeatherOverride = 0;
    nWeatherOverrideType = WEATHERTYPE_NONE;
    nWeatherOverrideTypeInside = WEATHERTYPE_NONE;
    nWeatherOverrideWindX = 0;
    nWeatherOverrideWindY = 0;
    nWeatherOverrideGravity = 0;
}

void CWeather::SetWeatherOverride(WEATHERTYPE nOverride, WEATHERTYPE nOverrideInside, short nX, short nY, short nZ)
{
    if ((nOverride < WEATHERTYPE_NONE) || (nOverride >= WEATHERTYPE_MAX)) // invalid, unload
    {
        UnloadPreset();
        return;
    }
    nWeatherOverride = 1;
    nWeatherOverrideType = nOverride;
    nWeatherOverrideTypeInside = nOverrideInside;
    nWeatherOverrideWindX = nX;
    nWeatherOverrideWindY = nY;
    nWeatherOverrideGravity = nZ;
}

inline WEATHERTYPE RandomWeather(unsigned int nRNG)
{
    if (nRNG&0x10)
        return WEATHERTYPE_SNOWHARD;
    else if (nRNG&8)
        return WEATHERTYPE_SNOW;
    else if (nRNG&4)
        return WEATHERTYPE_RAIN;
    else if (nRNG&2)
        return WEATHERTYPE_RAINHARD;
    return WEATHERTYPE_DUST;
}

const uint8_t bAngDiff[8][8] = {
    {0, 0, 1, 1, 1, 1, 1, 0},
    {0, 0, 0, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 1, 1, 1, 1},
    {1, 1, 0, 0, 0, 1, 1, 1},
    {1, 1, 1, 0, 0, 0, 1, 1},
    {1, 1, 1, 1, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 0, 0},
};

void CWeather::Process(int nX, int nY, int nZ, int nAng, short nSector, int nTime, int nClipDist, unsigned int uMapCRC)
{
    if (nWeatherCheat > WEATHERTYPE_NONE)
    {
        if (nWeatherCur != nWeatherCheat)
            nWeatherCur = WEATHERTYPE_NONE;
        nWeatherForecast = nWeatherCheat;
        SetWeatherType(nWeatherCheat, uMapCRC);
        return;
    }
    if (IsUnderwaterSector(nSector))
    {
        nWeatherForecast = WEATHERTYPE_UNDERWATER; // skip transition if player is underwater
        SetWeatherType(WEATHERTYPE_UNDERWATER, uMapCRC);
        return;
    }
    nTime = (nTime>>3)&3;
    const char bEnoughTimePassed = (nSectorChecked != nSector && nTime != nSectorCheckedTime) || (((nTime - nSectorCheckedTime)>>1) != 0);
    if (bEnoughTimePassed || (bAngDiff[nSectorCheckedAng][(nAng&2047)>>8] != 0)) // if moved to new sector, waited half a second or turned more than 22.5 degrees, hitscan above
    {
        nSectorChecked = nSector;
        nSectorCheckedTime = nTime;
        nSectorCheckedAng = (nAng&2047)>>8; // round down to 22.5 degrees
        int tmpSect = nSectorCheckedFoundForward >= 0 ? nSectorCheckedFoundForward : nSector;
        const long nStepX = nX+mulscale30(1024, Cos(nAng)); // move forward a bit
        const long nStepY = nY+mulscale30(1024, Sin(nAng));
        if (FindSector(nStepX, nStepY, nZ, &tmpSect)) // check sector in front of player
        {
            if (tmpSect == nSector)
            {
                nX = nStepX;
                nY = nStepY;
            }
            else if (cansee(nStepX, nStepY, nZ, tmpSect, nX, nY, nZ, nSector)) // if we can see the newly found sector, set as position to test ceiling
            {
                nX = nStepX;
                nY = nStepY;
                nSector = tmpSect & 0x3ff;
                nSectorCheckedFoundForward = nSector; // save for next FindSector() call
            }
        }
        int ve8, vec, vf0, vf4;
        GetZRangeAtXYZ(nX, nY, nZ, nSector, &vf4, &vf0, &vec, &ve8, nClipDist, 0);
        if ((vf0 & 0xc000) == 0x4000) // we hit ceiling
            nSector = vf0 & 0x3ff;
        nSectorCheckedFound = nSector;
    }
    else if (nSectorCheckedFound >= 0)
    {
        nSector = nSectorCheckedFound;
    }
    if (sector[nSector].ceilingstat&1) // outside
        nWeatherForecast = nWeatherOverride ? nWeatherOverrideType : RandomWeather(uMapCRC);
    else // inside
        nWeatherForecast = nWeatherOverride ? nWeatherOverrideTypeInside : WEATHERTYPE_DUST;
    if ((nWeatherCur == WEATHERTYPE_UNDERWATER) && (nWeatherCur != nWeatherForecast)) // if player has just left underwater, skip transition
    {
        nWindXOffset = krand()&0x3fff;
        nWindYOffset = krand()&0x3fff;
        SetWeatherType(nWeatherForecast, uMapCRC);
    }
}

void CWeather::SetWeatherType(WEATHERTYPE nWeather, unsigned int uMapCRC)
{
    if (nWeather != nWeatherCur || nWeather == WEATHERTYPE_NONE)
    {
        nWeatherCur = nWeather;
        switch (nWeather)
        {
        case WEATHERTYPE_RAIN:
            SetTranslucency(2);
            SetGravity(96, 1);
            RandomWind(0, uMapCRC);
            SetColor(128);
            SetColorShift(0);
            SetFade(16, 56);
            SetShape(2);
            SetStaticView(0);
            nLimit = kMaxVectors>>1;
            break;
        case WEATHERTYPE_SNOW:
            SetTranslucency(0);
            SetGravity(32, 1);
            RandomWind(0, uMapCRC);
            SetColor(31);
            SetColorShift(0);
            SetFade(16, 56);
            SetShape(0);
            SetStaticView(0);
            nLimit = kMaxVectors>>1;
            break;
        case WEATHERTYPE_BLOOD:
            SetTranslucency(0);
            SetGravity(56, 1);
            SetWind(0, 0);
            SetColor(152);
            SetColorShift(2);
            SetFade(16, 56);
            SetShape(1);
            SetStaticView(0);
            nLimit = kMaxVectors;
            break;
        case WEATHERTYPE_UNDERWATER:
            SetTranslucency(2);
            SetGravity(1, 1);
            SetWind(0, 0);
            SetColor(170);
            SetColorShift(2);
            SetFade(16, 56);
            SetShape(0);
            SetStaticView(0);
            nLimit = kMaxVectors>>1;
            break;
        case WEATHERTYPE_DUST:
            SetTranslucency(2);
            SetGravity(4, 1);
            SetWind(0, 0);
            SetColor(170);
            SetColorShift(2);
            SetFade(16, 56);
            SetShape(0);
            SetStaticView(0);
            nLimit = kMaxVectors>>2;
            break;
        case WEATHERTYPE_LAVA:
            SetTranslucency(1);
            SetGravity(6, 1);
            SetWind(0, 0);
            SetColor(160);
            SetColorShift(1);
            SetFade(16, 56);
            SetShape(0);
            SetStaticView(0);
            nLimit = kMaxVectors>>3;
            break;
        case WEATHERTYPE_STARS:
            SetTranslucency(1);
            SetGravity(0, 0);
            SetWind(0, 0);
            SetColor(128);
            SetColorShift(0);
            SetFade(32, 56);
            SetShape(0);
            SetStaticView(1);
            nLimit = kMaxVectors;
            break;
        case WEATHERTYPE_RAINHARD:
            SetTranslucency(2);
            SetGravity(128, 1);
            RandomWind(1, uMapCRC);
            SetColor(128);
            SetColorShift(0);
            SetFade(32, 64);
            SetShape(2);
            SetStaticView(0);
            nLimit = kMaxVectors;
            break;
        case WEATHERTYPE_SNOWHARD:
            SetTranslucency(0);
            SetGravity(128, 1);
            RandomWind(1, uMapCRC);
            SetColor(31);
            SetColorShift(0);
            SetFade(32, 64);
            SetShape(1);
            SetStaticView(0);
            nLimit = kMaxVectors>>1;
            break;
        default:
            SetFade(255, 255);
            nLimit = 0;
            break;
        }
    }
    if (nWeatherOverride && (nWeather == nWeatherOverrideType) && (nWeatherCheat == WEATHERTYPE_NONE)) // apply exterior weather override
    {
        SetWind(nWeatherOverrideWindX, nWeatherOverrideWindY);
        SetGravity(nWeatherOverrideGravity, 1);
    }
}

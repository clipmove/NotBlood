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

#define kMaxVectors 4096
#define kScaleTableSize 8192 // this mirrors the maximum nDepth value (8191)
#define kColorTableSize 32 // this mirrors the maximum nDepth value after shifting (31)
#define kWeatherTileY 1600 // max res supported

enum WEATHERTYPE : int32_t {
    WEATHERTYPE_NONE,
    WEATHERTYPE_RAIN,
    WEATHERTYPE_SNOW,
    WEATHERTYPE_BLOOD,
    WEATHERTYPE_UNDERWATER,
    WEATHERTYPE_DUST,
    WEATHERTYPE_LAVA,
    WEATHERTYPE_STARS,
    WEATHERTYPE_RAINHARD,
    WEATHERTYPE_SNOWHARD,
    WEATHERTYPE_MAX,
};

class CWeather {
public:
    CWeather();
    ~CWeather();
    void RandomizeVectors(void);
    void SetViewport(int nX, int nY, int nXOffset0, int nXOffset1, int nYOffset0, int nYOffset1, int nFov, int nAspect);
    void SetParticles(short nCount, short nLimit = -1);
    void SetGravity(short nGravity, char bVariance);
    void SetWind(short nX, short nY);
    void RandomWind(char bHeavyWind, unsigned int nRNG);
    void SetTranslucency(int);
    void SetColor(unsigned char a1);
    void SetColorShift(char);
    void SetFade(char nIn, char nOut);
    void SetShape(char);
    void SetStaticView(char);
    void Initialize(void);
    void Restart(void);
    void Draw(int nX, int nY, int nZ, int nAng, int nHoriz, int nClock, int nInterpolate, unsigned int uMapCRC);
    void LoadPreset(unsigned int uMapCRC);
    void UnloadPreset(void);
    void SetWeatherOverride(WEATHERTYPE nOverride, WEATHERTYPE nOverrideInside, short nX, short nY, short nZ);
    void Process(int nX, int nY, int nZ, int nAng, int nSector, int nTime, int nClipDist, unsigned int uMapCRC);
    void SetWeatherType(WEATHERTYPE nWeather, unsigned int nRNG);

    short GetCount(void) {
        return ClipHigh(nCount, nLimit);
    }

    void SetCount(int t) {
        nCount = ClipRange(t, 0, kMaxVectors);
    }

    BOOL Status(void) {
        return nDraw.bActive ? 1 : 0;
    }

    WEATHERTYPE GetWeather(void) {
        return nWeatherCur;
    }

    WEATHERTYPE GetWeatherForecast(void) {
        return nWeatherForecast;
    }

    WEATHERTYPE nWeatherCheat : 8;
private:
    void Draw(char *pBuffer, int nWidth, int nHeight, int nOffsetX, int nOffsetY, int nX, int nY, int nZ, int nAng, int nHoriz, int nCount, int nDelta);
    void UpdateColorTable(void);
    union {
        uint8_t b;
        struct {
            unsigned int bActive : 1;
            unsigned int bStaticView : 1;
            unsigned int bShape : 2;
            unsigned int nTransparent : 2;
            unsigned int bGravityVariance : 1;
        };
    } nDraw;
    int nWidth;
    int nHeight;
    int nOffsetX;
    int nOffsetY;
    int nScreenPitch;
    unsigned int nScaleFactor;
    short nCount;
    short nLimit;
    short nGravity;
    short nWindX;
    short nWindY;
    short nWindXOffset;
    short nWindYOffset;
    short nPos[kMaxVectors][3];
    unsigned char nPalColor;
    char nPalShift;
    uint8_t nFadeIn;
    uint8_t nFadeOut;
    uint8_t nColorTable[kColorTableSize];
    int nScaleTableWidth;
    int nScaleTableFov;
    int nFovV;
    unsigned int nAspectRatioModifier;
    unsigned int nFovPitchModifier;
    int nScaleTable[kScaleTableSize];
    int nLastFrameClock;
    WEATHERTYPE nWeatherCur : 8;
    WEATHERTYPE nWeatherForecast : 8;
    char nWeatherOverride;
    WEATHERTYPE nWeatherOverrideType : 8;
    WEATHERTYPE nWeatherOverrideTypeInside : 8;
    short nWeatherOverrideWindX;
    short nWeatherOverrideWindY;
    short nWeatherOverrideGravity;
    short nSectorChecked;
    short nSectorCheckedFound;
    short nSectorCheckedFoundForward;
    char nSectorCheckedTime;
    char nSectorCheckedAng;
};

extern CWeather gWeather;

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
#include "a.h"
#include "build.h"
#include "colmatch.h"
#include "common_game.h"

#include "globals.h"
#include "config.h"
#include "gfx.h"
#include "resource.h"
#include "screen.h"

RGB StdPal[32] = {
    { 0, 0, 0 },
    { 0, 0, 170 },
    { 0, 170, 170 },
    { 0, 170, 170 },
    { 170, 0, 0 },
    { 170, 0, 170 },
    { 170, 85, 0 },
    { 170, 170, 170 },
    { 85, 85, 85 },
    { 85, 85, 255 },
    { 85, 255, 85 },
    { 85, 255, 255 },
    { 255, 85, 85 },
    { 255, 85, 255 },
    { 255, 255, 85 },
    { 255, 255, 255 },
    { 241, 241, 241 },
    { 226, 226, 226 },
    { 211, 211, 211 },
    { 196, 196, 196 },
    { 181, 181, 181 },
    { 166, 166, 166 },
    { 151, 151, 151 },
    { 136, 136, 136 },
    { 120, 120, 120 },
    { 105, 105, 105 },
    { 90, 90, 90 },
    { 75, 75, 75 },
    { 60, 60, 60 },
    { 45, 45, 45 },
    { 30, 30, 30 },
    { 15, 15, 15 }
};

LOADITEM PLU[15] = {
    { 0, "NORMAL" },
    { 1, "SATURATE" },
    { 2, "BEAST" },
    { 3, "TOMMY" },
    { 4, "SPIDER3" },
    { 5, "GRAY" },
    { 6, "GRAYISH" },
    { 7, "SPIDER1" },
    { 8, "SPIDER2" },
    { 9, "FLAME" },
    { 10, "COLD" },
    { 11, "P1" },
    { 12, "P2" },
    { 13, "P3" },
    { 14, "P4" }
};

LOADITEM PAL[5] = {
    { 0, "BLOOD" },
    { 1, "WATER" },
    { 2, "BEAST" },
    { 3, "SEWER" },
    { 4, "INVULN1" }
};


bool DacInvalid = true;
static char(*gammaTable)[256];
RGB curDAC[256];
RGB baseDAC[256];
static RGB fromDAC[256];
static RGB toRGB;
static RGB *palTable[kMaxPalettes];
static int curPalette;
static int curGamma;
int gGammaLevels;
bool gFogMode = false;
char gStdColor[32];
int32_t gBrightness;
int32_t gCustomPalette;
int32_t gCustomPaletteCIEDE2000;
int32_t gCustomPaletteGrayscale;
int32_t gCustomPaletteInvert;

char scrFindClosestColor(int red, int green, int blue)
{
    int dist = 0x7fffffff;
    int best;
    for (int i = 0; i < 256; i++)
    {
        int sum = (palette[i*3+1]-green)*(palette[i*3+1]-green);
        if (sum >= dist) continue;
        sum += (palette[i*3+0]-red)*(palette[i*3+0]-red);
        if (sum >= dist) continue;
        sum += (palette[i*3+2]-blue)*(palette[i*3+2]-blue);
        if (sum >= dist) continue;
        best = i;
        dist = sum;
        if (sum == 0)
            break;
    }
    return best;
}

void scrCreateStdColors(void)
{
    for (int i = 0; i < 32; i++)
        gStdColor[i] = scrFindClosestColor(StdPal[i].red, StdPal[i].green, StdPal[i].blue);
}

void scrResetPalette(void)
{
    if (palTable[0] == nullptr)
        return;

    paletteSetColorTable(0, (uint8_t*)palTable[0]);
}

void gSetDacRange(int start, int end, RGB *pPal)
{
    UNREFERENCED_PARAMETER(start);
    UNREFERENCED_PARAMETER(end);
    if (videoGetRenderMode() == REND_CLASSIC)
    {
        memcpy(palette, pPal, sizeof(palette));
        videoSetPalette(gBrightness>>2, 0, 0);
    }
}

void scrLoadPLUs(void)
{
    if (gFogMode)
    {
        DICTNODE *pFog = gSysRes.Lookup("FOG", "FLU");
        if (!pFog)
            ThrowError("FOG.FLU not found");
        palookup[0] = (char*)gSysRes.Lock(pFog);
        for (int i = 0; i < 15; i++)
            palookup[PLU[i].id] = palookup[0];
        parallaxvisibility = 3072;
        return;
    }
    
    // load default palookups
    for (int i = 0; i < 15; i++) {
        DICTNODE *pPlu = gSysRes.Lookup(PLU[i].name, "PLU");
        if (!pPlu)
            ThrowError("%s.PLU not found", PLU[i].name);
        if (pPlu->size / 256 != 64)
            ThrowError("Incorrect PLU size");
        palookup[PLU[i].id] = (char*)gSysRes.Lock(pPlu);
    }

    // by NoOne: load user palookups
    for (int i = kUserPLUStart; i < MAXPALOOKUPS; i++) {
        DICTNODE* pPlu = gSysRes.Lookup(i, "PLU");
        if (!pPlu) continue;
        else if (pPlu->size / 256 != 64) { consoleSysMsg("Incorrect filesize of PLU#%d", i); }
        else palookup[i] = (char*)gSysRes.Lock(pPlu);
    }

#ifdef USE_OPENGL
    palookupfog[1].r = 255;
    palookupfog[1].g = 255;
    palookupfog[1].b = 255;
#endif
}

#ifdef USE_OPENGL
glblend_t const bloodglblend =
{
    {
        { 1.f/3.f, BLENDFACTOR_SRC_ALPHA, BLENDFACTOR_ONE_MINUS_SRC_ALPHA, 0 },
        { 2.f/3.f, BLENDFACTOR_SRC_ALPHA, BLENDFACTOR_ONE_MINUS_SRC_ALPHA, 0 },
    },
};
#endif

void scrLoadPalette(void)
{
    paletteInitClosestColorScale(30, 59, 11);
    paletteInitClosestColorGrid();
    paletteloaded = 0;
    LOG_F(INFO, "Loading palettes");
    int i;
    int nPal;
    
    DICTNODE* pPal;
    for (i = 0; i < kMaxPalettes; i++)
    {
        nPal = i;
        if ((pPal = gSysRes.Lookup(i, "PAL")) == NULL)
        {
            if (i < LENGTH(PAL))
            {
                // some blood versions have palettes added without ID
                if ((pPal = gSysRes.Lookup(PAL[i].name, "PAL")) == NULL)
                    ThrowError("%s.PAL not found (RFF files may be wrong version)", PAL[i].name);
                
                nPal = PAL[i].id;
            }

            if (i == 0)
                ThrowError("Palette ID #%d not found!", i);
        }
        
        palTable[nPal] = (pPal) ? (RGB*)gSysRes.Lock(pPal) : palTable[0];
        paletteSetColorTable(nPal, (uint8_t*)palTable[nPal]);
    }

    memcpy(palette, palTable[0], sizeof(palette));
    numshades = 64;
    paletteloaded |= PALETTE_MAIN;
    scrLoadPLUs();
    paletteloaded |= PALETTE_SHADE;
    LOG_F(INFO, "Loading translucency table");
    DICTNODE *pTrans = gSysRes.Lookup("TRANS", "TLU");
    if (!pTrans)
        ThrowError("TRANS.TLU not found");
    blendtable[0] = (char*)gSysRes.Lock(pTrans);
    paletteloaded |= PALETTE_TRANSLUC;

#ifdef USE_OPENGL
    for (auto & x : glblend)
        x = bloodglblend;

    for (int i = 0; i < MAXPALOOKUPS; i++)
        palookupfogfactor[i] = 1.f;
#endif

    paletteInitClosestColorMap((uint8_t*)palTable[0]);
    palettePostLoadTables();
    // Make color index 255 of palette black.
    for (int i = 0; i < kMaxPalettes; i++)
    {
        if (basepaltable[i] != NULL)
            Bmemset(&basepaltable[i][255 * 3], 0, 3);
    }
    palettePostLoadLookups();
}

void scrSetPalette(int palId)
{
    curPalette = palId;
    scrSetGamma(0/*curGamma*/);
}

void scrSetGamma(int nGamma)
{
    dassert(nGamma < gGammaLevels);
    curGamma = nGamma;
    for (int i = 0; i < 256; i++)
    {
        baseDAC[i].red = gammaTable[curGamma][palTable[curPalette][i].red];
        baseDAC[i].green = gammaTable[curGamma][palTable[curPalette][i].green];
        baseDAC[i].blue = gammaTable[curGamma][palTable[curPalette][i].blue];
    }
    DacInvalid = 1;
}

void scrSetupFade(char red, char green, char blue)
{
    memcpy(fromDAC, curDAC, sizeof(fromDAC));
    toRGB.red = red;
    toRGB.green = green;
    toRGB.blue = blue;
}

void scrSetupUnfade(void)
{
    memcpy(fromDAC, baseDAC, sizeof(fromDAC));
}

void scrFadeAmount(int amount)
{
    for (int i = 0; i < 256; i++)
    {
        curDAC[i].red = interpolate(fromDAC[i].red, toRGB.red, amount, 1);
        curDAC[i].green = interpolate(fromDAC[i].green, toRGB.green, amount, 1);
        curDAC[i].blue = interpolate(fromDAC[i].blue, toRGB.blue, amount, 1);
    }
    gSetDacRange(0, 256, curDAC);
}

void scrSetDac(void)
{
    if (DacInvalid)
        gSetDacRange(0, 256, baseDAC);
    DacInvalid = 0;
}

void scrInit(void)
{
    LOG_F(INFO, "Initializing engine");
#ifdef USE_OPENGL
    glrendmode = REND_POLYMOST;
#endif
    engineInit();
    curPalette = 0;
    curGamma = 0;
    LOG_F(INFO, "Loading gamma correction table");
    DICTNODE *pGamma = gSysRes.Lookup("gamma", "DAT");
    if (!pGamma)
        ThrowError("Gamma table not found");
    gGammaLevels = pGamma->size / 256;
    gammaTable = (char(*)[256])gSysRes.Lock(pGamma);
}

void scrUnInit(void)
{
    memset(palookup, 0, sizeof(palookup));
    memset(blendtable, 0, sizeof(blendtable));
    engineUnInit();
}


void scrSetGameMode(int vidMode, int XRes, int YRes, int nBits)
{
    videoResetMode();
    //videoSetGameMode(vidMode, XRes, YRes, nBits, 0);
    if (videoSetGameMode(vidMode, XRes, YRes, nBits, gUpscaleFactor) < 0)
    {
        LOG_F(ERROR, "Failure setting video mode %dx%dx%d %s! Trying next mode...", XRes, YRes,
                    nBits, vidMode ? "fullscreen" : "windowed");

        int resIdx = 0;

        for (int i=0; i < validmodecnt; i++)
        {
            if (validmode[i].xdim == XRes && validmode[i].ydim == YRes)
            {
                resIdx = i;
                break;
            }
        }

        int const savedIdx = resIdx;
        int bpp = nBits;

        while (videoSetGameMode(0, validmode[resIdx].xdim, validmode[resIdx].ydim, bpp, gUpscaleFactor) < 0)
        {
            LOG_F(ERROR, "Failure setting video mode %dx%dx%d windowed! Trying next mode...",
                        validmode[resIdx].xdim, validmode[resIdx].ydim, bpp);

            if (++resIdx == validmodecnt)
            {
                if (bpp == 8)
                    ThrowError("Fatal error: unable to set any video mode!");

                resIdx = savedIdx;
                bpp = 8;
            }
        }

        gSetup.xdim = validmode[resIdx].xdim;
        gSetup.ydim = validmode[resIdx].ydim;
        gSetup.bpp  = bpp;
    }
    videoClearViewableArea(0);
    scrNextPage();
    scrSetPalette(curPalette);
    gfxSetClip(0, 0, xdim, ydim);
}

void scrNextPage(void)
{
    videoNextPage();
}

#include "screenpals.h" // where the custom palettes are stored

// Computes the CIEDE2000 color-difference between two Lab colors
// Based on the article:
// The CIEDE2000 Color-Difference Formula: Implementation Notes,
// Supplementary Test Data, and Mathematical Observations,", G. Sharma,
// W. Wu, E. N. Dalal, submitted to Color Research and Application,
// January 2004.
// Available at http://www.ece.rochester.edu/~/gsharma/ciede2000/
// Based on the C++ implementation by Ofir Pele, The Hebrew University of Jerusalem 2010.
// Written in C by Adam Borowski (kilobyte)
// Shamelessly taken from https://github.com/kilobyte/colormatch (no license)

#define pi 3.141592653589793238462643383279

static double srcDeltaE2000(double *lab1, double *lab2)
{
    double Lstd = *lab1;
    double astd = *(lab1+1);
    double bstd = *(lab1+2);

    double Lsample = *lab2;
    double asample = *(lab2+1);
    double bsample = *(lab2+2);

    double Cabstd= sqrt(astd*astd+bstd*bstd);
    double Cabsample= sqrt(asample*asample+bsample*bsample);

    double Cabarithmean= (Cabstd + Cabsample)/2.0;

    double G= 0.5*( 1.0 - sqrt( pow(Cabarithmean,7.0)/(pow(Cabarithmean,7.0) + pow(25.0,7.0))));

    double apstd= (1.0+G)*astd; // aprime in paper
    double apsample= (1.0+G)*asample; // aprime in paper
    double Cpsample= sqrt(apsample*apsample+bsample*bsample);

    double Cpstd= sqrt(apstd*apstd+bstd*bstd);
    // Compute product of chromas
    double Cpprod= (Cpsample*Cpstd);


    // Ensure hue is between 0 and 2pi
    double hpstd= atan2(bstd,apstd);
    if (hpstd<0) hpstd+= 2.0*pi;  // rollover ones that come -ve

    double hpsample= atan2(bsample,apsample);
    if (hpsample<0) hpsample+= 2.0*pi;
    if ( (fabs(apsample)+fabs(bsample))==0.0)  hpsample= 0.0;

    double dL= (Lsample-Lstd);
    double dC= (Cpsample-Cpstd);

    // Computation of hue difference
    double dhp= (hpsample-hpstd);
    if (dhp>pi)  dhp-= 2.0*pi;
    if (dhp<-pi) dhp+= 2.0*pi;
    // set chroma difference to zero if the product of chromas is zero
    if (Cpprod == 0.0) dhp= 0.0;

    // Note that the defining equations actually need
    // signed Hue and chroma differences which is different
    // from prior color difference formulae

    double dH= 2.0*sqrt(Cpprod)*sin(dhp/2.0);
    //%dH2 = 4*Cpprod.*(sin(dhp/2)).^2;

    // weighting functions
    double Lp= (Lsample+Lstd)/2.0;
    double Cp= (Cpstd+Cpsample)/2.0;

    // Average Hue Computation
    // This is equivalent to that in the paper but simpler programmatically.
    // Note average hue is computed in radians and converted to degrees only
    // where needed
    double hp= (hpstd+hpsample)/2.0;
    // Identify positions for which abs hue diff exceeds 180 degrees
    if ( fabs(hpstd-hpsample)  > pi ) hp-= pi;
    // rollover ones that come -ve
    if (hp<0) hp+= 2.0*pi;

    // Check if one of the chroma values is zero, in which case set
    // mean hue to the sum which is equivalent to other value
    if (Cpprod==0.0) hp= hpsample+hpstd;

    double Lpm502= (Lp-50.0)*(Lp-50.0);
    double Sl= 1.0+0.015*Lpm502/sqrt(20.0+Lpm502);
    double Sc= 1.0+0.045*Cp;
    double T= 1.0 - 0.17*cos(hp - pi/6.0) + 0.24*cos(2.0*hp) + 0.32*cos(3.0*hp+pi/30.0) - 0.20*cos(4.0*hp-63.0*pi/180.0);
    double Sh= 1.0 + 0.015*Cp*T;
    double delthetarad= (30.0*pi/180.0)*exp(- pow(( (180.0/pi*hp-275.0)/25.0),2.0));
    double Rc=  2.0*sqrt(pow(Cp,7.0)/(pow(Cp,7.0) + pow(25.0,7.0)));
    double RT= -sin(2.0*delthetarad)*Rc;

    // The CIE 00 color difference
    return sqrt( pow((dL/Sl),2.0) + pow((dC/Sc),2.0) + pow((dH/Sh),2.0) + RT*(dC/Sc)*(dH/Sh) );
}

static void srcRGB2LAB(int rgb, double lab[3])
{
    double r = ((rgb>>16)&0xff)/255.0;
    double g = ((rgb>> 8)&0xff)/255.0;
    double b = ((rgb    )&0xff)/255.0;
    double x,y,z;

    r = (r > 0.04045) ? pow((r + 0.055) / 1.055, 2.4) : r / 12.92;
    g = (g > 0.04045) ? pow((g + 0.055) / 1.055, 2.4) : g / 12.92;
    b = (b > 0.04045) ? pow((b + 0.055) / 1.055, 2.4) : b / 12.92;

    // at D65
    x = (r * 0.4124 + g * 0.3576 + b * 0.1805) / 0.95047;
    y = (r * 0.2126 + g * 0.7152 + b * 0.0722) / 1.00000;
    z = (r * 0.0193 + g * 0.1192 + b * 0.9505) / 1.08883;

    x = (x > 0.008856) ? pow(x, 1.0/3) : (7.787 * x) + 16.0/116;
    y = (y > 0.008856) ? pow(y, 1.0/3) : (7.787 * y) + 16.0/116;
    z = (z > 0.008856) ? pow(z, 1.0/3) : (7.787 * z) + 16.0/116;

    lab[0] = 116 * y - 16;
    lab[1] = 500 * (x - y);
    lab[2] = 200 * (y - z);
}

static double srcColorDiff(int rgb1, int rgb2)
{
    double lab1[3], lab2[3];
    srcRGB2LAB(rgb1, lab1);
    srcRGB2LAB(rgb2, lab2);
    return srcDeltaE2000(lab1, lab2);
}

static void scrTweakPalette(uint8_t *curPal, int replacePal, char bUseCIEDE2000, char bGrayscale, char bInvertPal, int palSize)
{
    if (!replacePal && !bGrayscale && !bInvertPal) // do nothing
        return;
    for (int j = 0; j < palSize; j += 3) // count through all palette colors
    {
        if (bInvertPal)
        {
            curPal[j+0] = 255-curPal[j+0];
            curPal[j+1] = 255-curPal[j+1];
            curPal[j+2] = 255-curPal[j+2];
        }
        if (replacePal == 1) // sepia tone
        {
            char color = clamp((curPal[j]+curPal[j+1]+curPal[j+2])/3, 0, 255);
            curPal[j+0] = clamp(color+(92/1.5f)-20, 0, 255);
            curPal[j+1] = clamp(color+(46/1.5f)-20, 0, 255);
            curPal[j+2] = clamp(color-20, 0, 255);
            continue;
        }
        else if (bGrayscale)
        {
            char color = clamp((curPal[j]+curPal[j+1]+curPal[j+2])/3, 0, 255);
            curPal[j+0] = color;
            curPal[j+1] = color;
            curPal[j+2] = color;
        }
        if (!replacePal) // only allow invert/grayscale for default palette
            continue;
        int palIndex = 0;
        double nearestColor = 255*3;
        const uint8_t (*newPal)[3] = (const uint8_t (*)[3])srcCustomPaletteList[replacePal];
        if (!newPal) // null entry, skip
            continue;
        for (int i = 0; i < srcCustomPaletteColors[replacePal]; i++)
        {
            double testedDistance;
            if (bUseCIEDE2000)
                testedDistance = srcColorDiff((curPal[j+0]<<16)|(curPal[j+1]<<8)|(curPal[j+2]), (newPal[i][0]<<16)|(newPal[i][1]<<8)|(newPal[i][2]));
            else
                testedDistance = sqrt(pow(abs(curPal[j+0]-newPal[i][0]),2.0)+pow(abs(curPal[j+1]-newPal[i][1]),2.0)+pow(abs(curPal[j+2]-newPal[i][2]),2.0));
            if (testedDistance < nearestColor)
            {
                palIndex = i;
                nearestColor = testedDistance;
            }
        }
        curPal[j+0] = newPal[palIndex][0];
        curPal[j+1] = newPal[palIndex][1];
        curPal[j+2] = newPal[palIndex][2];
        if (bInvertPal == 2)
        {
            curPal[j+0] = 255-curPal[j+0];
            curPal[j+1] = 255-curPal[j+1];
            curPal[j+2] = 255-curPal[j+2];
        }
    }
}

void scrCustomizePalette(int replacePal, char bUseCIEDE2000, char bGrayscale, char bInvertPal)
{
    static RGB bakPalTable[kMaxPalettes][256];
    static char bHasBakPalTable = 0;

    for (int i = 0; i < kMaxPalettes; i++)
    {
        if (!basepaltable[i]) // not initialized, should never happen
            continue;
        if (!bHasBakPalTable)
            memcpy(bakPalTable[i], palTable[i], sizeof(bakPalTable[0])); // create backup
        else
            memcpy(palTable[i], bakPalTable[i], sizeof(bakPalTable[0])); // restore from backup
        scrTweakPalette((uint8_t *)palTable[i], replacePal, bUseCIEDE2000, bGrayscale, bInvertPal, sizeof(bakPalTable[0]));
        paletteSetColorTable(i, (uint8_t *)palTable[i]);
    }
    memcpy(palette, palTable[0], sizeof(palette));
    bHasBakPalTable = 1;

    // Make color index 255 of palette black (or closest)
    for (int i = 0; i < kMaxPalettes; i++)
    {
        if (!basepaltable[i])
            continue;
        Bmemset(&basepaltable[i][255 * 3], 0, 3);
        scrTweakPalette(&basepaltable[i][255 * 3], replacePal, bUseCIEDE2000, bGrayscale, bInvertPal, 3); // find palette for black
    }
    scrSetPalette(0);
}
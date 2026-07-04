//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 Nuke.YKT
Copyright (C) NoOne

*****************************************************************
NoOne: AI code for Custom Dude system.
*****************************************************************

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
#include "nnexts.h"
#include "aicdud.h"
#include "globals.h"
#include "player.h"
#include "trig.h"
#include "endgame.h"
#include "mmulti.h"
#include "view.h"

#define SEQOFFSC(x) (kCdudeDefaultSeq + x)
#define SEQOFFSG(x) (kCdudeDefaultSeqF + x)

#define ATTACK_FAIL       -1
#define ATTACK_CONTINUE   -2
#define ATTACK_BREAK_ALL  -3

#pragma pack(push, 1)
struct TARGET_INFO
{
    spritetype* pSpr;
    XSPRITE* pXSpr;
    unsigned int nDist  : 32;
    unsigned int nAng   : 12;
    unsigned int nDang  : 12;
    int nCode;
};

struct ARG_WEAPONSHOT
{
    POINT3D* offs;
    int dx, dy, dz;
    char check;
};
#pragma pack(pop)

static void resetTarget(spritetype*, XSPRITE* pXSpr)            { pXSpr->target = -1; }
static void moveForwardStop(spritetype* pSpr, XSPRITE*)         { xvel[pSpr->index] = yvel[pSpr->index] = 0; }
static char posObstructed(int x, int y, int z, int nRadius);
static char clipFlying(spritetype* pSpr, XSPRITE* pXSpr);
void helperPounce(spritetype* pSpr, spritetype* pHSpr, int nDmgType, int nDmg, int kickPow);

static void thinkSearch(spritetype*, XSPRITE*);
static void thinkChase(spritetype*, XSPRITE*);
static void thinkFlee(spritetype*, XSPRITE*);
static void thinkKnock(spritetype* pSpr, XSPRITE*);
static void thinkTarget(spritetype* pSpr, XSPRITE*);
static void thinkMorph(spritetype* pSpr, XSPRITE* pXSpr);
static void thinkDying(spritetype* pSpr, XSPRITE* pXSpr);
static void enterIdle(spritetype* pSpr, XSPRITE* pXSpr);
static void enterSearch(spritetype* pSpr, XSPRITE* pXSpr);
static void enterChase(spritetype* pSpr, XSPRITE*);
static void enterFlee(spritetype* pSpr, XSPRITE* pXSpr);
static void enterSearchBurn(spritetype* pSpr, XSPRITE* pXSpr);
static void enterMorph(spritetype* pSpr, XSPRITE* pXSpr);
static void enterDying(spritetype* pSpr, XSPRITE* pXSpr);
static void enterDeath(spritetype* pSpr, XSPRITE* pXSpr);
static void enterSleep(spritetype* pSpr, XSPRITE* pXSpr);
static void enterWake(spritetype* pSpr, XSPRITE*);
static void enterAttack(spritetype* pSpr, XSPRITE* pXSpr);
static void enterStartKnock(spritetype* pSpr, XSPRITE* pXSpr);
static void enterKnock(spritetype* pSpr, XSPRITE* pXSpr);
static void exitKnock(spritetype* pSpr, XSPRITE* pXSpr);
static void enterDodgeFly(spritetype* pSpr, XSPRITE*);
static int getTargetAng(spritetype* pSpr, XSPRITE* pXSpr);
static void turnToTarget(spritetype* pSpr, XSPRITE* pXSpr);
static void moveTurn(spritetype* pSpr, XSPRITE* pXSpr);
static void moveDodge(spritetype* pSpr, XSPRITE* pXSpr);
static void moveNormal(spritetype* pSpr, XSPRITE* pXSpr);
static void moveKnockout(spritetype* pSpr, XSPRITE* pXSpr);

static void weaponShotCallback(int, int);
static int nWeaponShot  = seqRegisterClient(weaponShotCallback);

static int weaponShotNone(CUSTOMDUDE*, CUSTOMDUDE_WEAPON*, ARG_WEAPONSHOT*) { return kMaxSprites; }
static int weaponShotHitscan(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, ARG_WEAPONSHOT* a);
static int weaponShotMissile(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, ARG_WEAPONSHOT* a);
static int weaponShotThing(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, ARG_WEAPONSHOT* a);
static int weaponShotSummon(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, ARG_WEAPONSHOT* a);
static int weaponShotKamikaze(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, ARG_WEAPONSHOT* a);
static int weaponShotSpecial(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, ARG_WEAPONSHOT* a);
static int weaponShotMorph(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, ARG_WEAPONSHOT* a);
static int weaponShotDmgOrHp(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, ARG_WEAPONSHOT* a);

static int (*gWeaponShotFunc[])(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, ARG_WEAPONSHOT* a) =
{
    weaponShotNone,
    weaponShotHitscan,
    weaponShotMissile,
    weaponShotThing,
    weaponShotSummon,       // vanilla dude
    weaponShotSummon,       // custom  dude
    weaponShotKamikaze,
    weaponShotSpecial,
    weaponShotMorph,        // vanilla dude
    weaponShotMorph,        // custom  dude
    weaponShotDmgOrHp,      // damage
    weaponShotDmgOrHp,      // heal
};

static AISTATE gCdudeStateDeath = { kAiStateOther, -1, -1, 0, enterDeath, NULL, NULL, NULL }; // just turns dude to a gib





// Land, Crouch, Swim, Fly (proper order matters!)
AISTATE gCdudeStateTemplate[kCdudeStateNormalMax][kCdudePostureMax] =
{
    // idle (don't change pos or patrol gets broken!)
    {
        { kAiStateIdle,     SEQOFFSC(0),    -1, 0, enterIdle, NULL, thinkTarget, NULL },
        { kAiStateIdle,     SEQOFFSC(17),   -1, 0, enterIdle, NULL, thinkTarget, NULL },
        { kAiStateIdle,     SEQOFFSC(13),   -1, 0, enterIdle, NULL, thinkTarget, NULL },
        { kAiStateIdle,     SEQOFFSG(0),    -1, 0, enterIdle, NULL, thinkTarget, NULL },
    },

    // search (don't change pos or patrol gets broken!)
    {
        { kAiStateSearch,   SEQOFFSC(9),    -1, 800, enterSearch,  moveNormal, thinkSearch, &gCdudeStateTemplate[kCdudeStateIdle][kCdudePostureL] },
        { kAiStateSearch,   SEQOFFSC(14),   -1, 800, enterSearch,  moveNormal, thinkSearch, &gCdudeStateTemplate[kCdudeStateIdle][kCdudePostureC] },
        { kAiStateSearch,   SEQOFFSC(13),   -1, 800, enterSearch,  moveNormal, thinkSearch, &gCdudeStateTemplate[kCdudeStateIdle][kCdudePostureW] },
        { kAiStateSearch,   SEQOFFSG(0),    -1, 800, enterSearch,  moveNormal, thinkSearch, &gCdudeStateTemplate[kCdudeStateIdle][kCdudePostureF] },
    },

    // dodge
    {
        { kAiStateMove,     SEQOFFSC(9),    -1, 90, NULL,           moveDodge,	NULL, &gCdudeStateTemplate[kCdudeStateChase][kCdudePostureL] },
        { kAiStateMove,     SEQOFFSC(14),   -1, 90, NULL,           moveDodge,	NULL, &gCdudeStateTemplate[kCdudeStateChase][kCdudePostureC] },
        { kAiStateMove,     SEQOFFSC(13),   -1, 90, NULL,           moveDodge,	NULL, &gCdudeStateTemplate[kCdudeStateChase][kCdudePostureW] },
        { kAiStateMove,     SEQOFFSG(0),    -1, 90, enterDodgeFly,  moveDodge,	NULL, &gCdudeStateTemplate[kCdudeStateChase][kCdudePostureF] },
    },

    // chase
    {
        { kAiStateChase,    SEQOFFSC(9),    -1, 30, enterChase,	moveNormal, thinkChase, NULL },
        { kAiStateChase,    SEQOFFSC(14),   -1, 30, enterChase,	moveNormal, thinkChase, NULL },
        { kAiStateChase,    SEQOFFSC(13),   -1, 30, enterChase,	moveNormal, thinkChase, NULL },
        { kAiStateChase,    SEQOFFSG(0),    -1, 30, enterChase,	moveNormal, thinkChase, NULL },
    },

    // flee
    {
        { kAiStateMove,    SEQOFFSC(9),     -1, 256, enterFlee,	moveNormal, thinkFlee, &gCdudeStateTemplate[kCdudeStateSearch][kCdudePostureL] },
        { kAiStateMove,    SEQOFFSC(14),    -1, 256, enterFlee,	moveNormal, thinkFlee, &gCdudeStateTemplate[kCdudeStateSearch][kCdudePostureC] },
        { kAiStateMove,    SEQOFFSC(13),    -1, 256, enterFlee,	moveNormal, thinkFlee, &gCdudeStateTemplate[kCdudeStateSearch][kCdudePostureW] },
        { kAiStateMove,    SEQOFFSG(0),     -1, 256, enterFlee,	moveNormal, thinkFlee, &gCdudeStateTemplate[kCdudeStateSearch][kCdudePostureF] },
    },

    // recoil normal
    {
        { kAiStateRecoil,   SEQOFFSC(5),    -1, 0, NULL, NULL, NULL, &gCdudeStateTemplate[kCdudeStateChase][kCdudePostureL] },
        { kAiStateRecoil,   SEQOFFSC(5),    -1, 0, NULL, NULL, NULL, &gCdudeStateTemplate[kCdudeStateChase][kCdudePostureC] },
        { kAiStateRecoil,   SEQOFFSC(5),    -1, 0, NULL, NULL, NULL, &gCdudeStateTemplate[kCdudeStateChase][kCdudePostureW] },
        { kAiStateRecoil,   SEQOFFSG(5),    -1, 0, NULL, NULL, NULL, &gCdudeStateTemplate[kCdudeStateChase][kCdudePostureF] },
    },

    // recoil tesla
    {
        { kAiStateRecoil,   SEQOFFSC(4),    -1, 0, NULL, NULL, NULL, &gCdudeStateTemplate[kCdudeStateChase][kCdudePostureL] },
        { kAiStateRecoil,   SEQOFFSC(4),    -1, 0, NULL, NULL, NULL, &gCdudeStateTemplate[kCdudeStateChase][kCdudePostureC] },
        { kAiStateRecoil,   SEQOFFSC(4),    -1, 0, NULL, NULL, NULL, &gCdudeStateTemplate[kCdudeStateChase][kCdudePostureW] },
        { kAiStateRecoil,   SEQOFFSG(4),    -1, 0, NULL, NULL, NULL, &gCdudeStateTemplate[kCdudeStateChase][kCdudePostureF] },
    },

    // burn search
    {
        { kAiStateSearch,   SEQOFFSC(3), -1, 3600, enterSearchBurn, moveNormal, NULL, &gCdudeStateTemplate[kCdudeBurnStateSearch][kCdudePostureL] },
        { kAiStateSearch,   SEQOFFSC(3), -1, 3600, enterSearchBurn, moveNormal, NULL, &gCdudeStateTemplate[kCdudeBurnStateSearch][kCdudePostureC] },
        { kAiStateSearch,   SEQOFFSC(3), -1, 3600, enterSearchBurn, moveNormal, NULL, &gCdudeStateTemplate[kCdudeBurnStateSearch][kCdudePostureW] },
        { kAiStateSearch,   SEQOFFSG(3), -1, 3600, enterSearchBurn, moveNormal, NULL, &gCdudeStateTemplate[kCdudeBurnStateSearch][kCdudePostureF] },
    },

    // morph (put thinkFunc in moveFunc because it supposed to work fast)
    {
        { kAiStateOther,   SEQOFFSC(18), -1, 0, enterMorph, thinkMorph, NULL, NULL },
        { kAiStateOther,   SEQOFFSC(18), -1, 0, enterMorph, thinkMorph, NULL, NULL },
        { kAiStateOther,   SEQOFFSC(18), -1, 0, enterMorph, thinkMorph, NULL, NULL },
        { kAiStateOther,   SEQOFFSG(0),  -1, 0, enterMorph, thinkMorph, NULL, NULL },
    },

    // knock / last stand enter
    {
        { kAiStateKnockout,   SEQOFFSC(0), -1, 0, enterStartKnock, NULL,         NULL, &gCdudeStateTemplate[kCdudeStateKnock][kCdudePostureL] },
        { kAiStateKnockout,   SEQOFFSC(0), -1, 0, enterStartKnock, NULL,         NULL, &gCdudeStateTemplate[kCdudeStateKnock][kCdudePostureC] },
        { kAiStateKnockout,   SEQOFFSC(0), -1, 0, enterStartKnock, moveKnockout, NULL, &gCdudeStateTemplate[kCdudeStateKnock][kCdudePostureW] },
        { kAiStateKnockout,   SEQOFFSG(0), -1, 0, enterStartKnock, NULL,         NULL, &gCdudeStateTemplate[kCdudeStateKnock][kCdudePostureF] },
    },

    // knock / last stand
    {
        { kAiStateKnockout,   SEQOFFSC(0), -1, 0, enterKnock, NULL,         thinkKnock, &gCdudeStateTemplate[kCdudeStateKnockExit][kCdudePostureL] },
        { kAiStateKnockout,   SEQOFFSC(0), -1, 0, enterKnock, NULL,         thinkKnock, &gCdudeStateTemplate[kCdudeStateKnockExit][kCdudePostureC] },
        { kAiStateKnockout,   SEQOFFSC(0), -1, 0, enterKnock, moveKnockout, thinkKnock, &gCdudeStateTemplate[kCdudeStateKnockExit][kCdudePostureW] },
        { kAiStateKnockout,   SEQOFFSG(0), -1, 0, enterKnock, NULL,         thinkKnock, &gCdudeStateTemplate[kCdudeStateKnockExit][kCdudePostureF] },
    },

    // knock / last stand exit
    {
        { kAiStateKnockout,   SEQOFFSC(0), -1, 0, exitKnock, turnToTarget, NULL, &gCdudeStateTemplate[kCdudeStateSearch][kCdudePostureL] },
        { kAiStateKnockout,   SEQOFFSC(0), -1, 0, exitKnock, turnToTarget, NULL, &gCdudeStateTemplate[kCdudeStateSearch][kCdudePostureC] },
        { kAiStateKnockout,   SEQOFFSC(0), -1, 0, exitKnock, turnToTarget, NULL, &gCdudeStateTemplate[kCdudeStateSearch][kCdudePostureW] },
        { kAiStateKnockout,   SEQOFFSG(0), -1, 0, exitKnock, turnToTarget, NULL, &gCdudeStateTemplate[kCdudeStateSearch][kCdudePostureF] },
    },

    // sleep
    {
        { kAiStateIdle,     SEQOFFSC(0),  -1, 0, enterSleep, NULL, thinkTarget, NULL },
        { kAiStateIdle,     SEQOFFSC(0),  -1, 0, enterSleep, NULL, thinkTarget, NULL },
        { kAiStateIdle,     SEQOFFSC(0),  -1, 0, enterSleep, NULL, thinkTarget, NULL },
        { kAiStateIdle,     SEQOFFSG(0),  -1, 0, enterSleep, NULL, thinkTarget, NULL },
    },

    // wake
    {
        { kAiStateIdle,     SEQOFFSC(0),  -1, 0, enterWake, turnToTarget, NULL, &gCdudeStateTemplate[kCdudeStateSearch][kCdudePostureL] },
        { kAiStateIdle,     SEQOFFSC(0),  -1, 0, enterWake, turnToTarget, NULL, &gCdudeStateTemplate[kCdudeStateSearch][kCdudePostureC] },
        { kAiStateIdle,     SEQOFFSC(0),  -1, 0, enterWake, turnToTarget, NULL, &gCdudeStateTemplate[kCdudeStateSearch][kCdudePostureW] },
        { kAiStateIdle,     SEQOFFSG(0),  -1, 0, enterWake, turnToTarget, NULL, &gCdudeStateTemplate[kCdudeStateSearch][kCdudePostureF] },
    },

    // generic idle (ai fight compat.)
    {
        { kAiStateGenIdle,     SEQOFFSC(0),     -1, 0, resetTarget, NULL, NULL, NULL },
        { kAiStateGenIdle,     SEQOFFSC(17),    -1, 0, resetTarget, NULL, NULL, NULL },
        { kAiStateGenIdle,     SEQOFFSC(13),    -1, 0, resetTarget, NULL, NULL, NULL },
        { kAiStateGenIdle,     SEQOFFSG(0),     -1, 0, resetTarget, NULL, NULL, NULL },
    },
};

// Land, Crouch, Swim, Fly
AISTATE gCdudeStateAttackTemplate[kCdudePostureMax] =
{
    // attack (put thinkFunc in moveFunc because it supposed to work fast)
    { kAiStateAttack,   SEQOFFSC(6), nWeaponShot, 0, enterAttack, thinkChase, NULL, &gCdudeStateAttackTemplate[kCdudePostureL] },
    { kAiStateAttack,   SEQOFFSC(8), nWeaponShot, 0, enterAttack, thinkChase, NULL, &gCdudeStateAttackTemplate[kCdudePostureC] },
    { kAiStateAttack,   SEQOFFSC(8), nWeaponShot, 0, enterAttack, thinkChase, NULL, &gCdudeStateAttackTemplate[kCdudePostureW] },
    { kAiStateAttack,   SEQOFFSG(6), nWeaponShot, 0, enterAttack, thinkChase, NULL, &gCdudeStateAttackTemplate[kCdudePostureF] },
};

// Land, Crouch, Swim, Fly or random pick
AISTATE gCdudeStateDyingTemplate[kCdudePostureMax] =
{
    // dying
    { kAiStateOther,   SEQOFFSC(1), -1, 0, enterDying, NULL, thinkDying, &gCdudeStateDeath },
    { kAiStateOther,   SEQOFFSC(1), -1, 0, enterDying, NULL, thinkDying, &gCdudeStateDeath },
    { kAiStateOther,   SEQOFFSC(1), -1, 0, enterDying, NULL, thinkDying, &gCdudeStateDeath },
    { kAiStateOther,   SEQOFFSG(1), -1, 0, enterDying, NULL, thinkDying, &gCdudeStateDeath },
};

// for kModernThingThrowableRock
static short gCdudeDebrisPics[6] =
{
    2406, 2280, 2185, 2155, 2620, 3135
};

static int weaponShotHitscan(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, ARG_WEAPONSHOT* a)
{
    if (a->check)
        return kMaxSprites;

    VECTORDATA* pVect = &gVectorData[pWeap->id];
    spritetype* pSpr = pDude->pSpr;
    int t;

    // ugly hack to make it fire at required distance
    t = pVect->maxDist, pVect->maxDist = pWeap->GetDistance();
    actFireVector(pSpr, a->offs->x, a->offs->z, a->dx, a->dy, a->dz, (VECTOR_TYPE)pWeap->id);
    pVect->maxDist = t;

    return kMaxSprites;
}

static char CanSee(spritetype* pSrc, spritetype* pDst)
{
    int zA[3], zB[3];

    zA[1] = pSrc->z; zB[1] = pDst->z;
    GetSpriteExtents(pSrc, &zA[0], &zA[2]);
    GetSpriteExtents(pSrc, &zB[0], &zB[2]);

    for (int i = 0; i < 3; i++)
    {
        if (cansee(pSrc->x, pSrc->y, zA[i], pSrc->sectnum, pDst->x, pDst->y, zB[i], pDst->sectnum))
            return 1;
    }

    return 0;
}

static int weaponShotDmgOrHp(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, ARG_WEAPONSHOT* a)
{
    spritetype* pSpr = pDude->pSpr, *pTarg = NULL, *pOrig;
    XSPRITE* pXSpr = pDude->pXSpr, *pXOrig;
    PLAYER* pPlay;
    
    int nBVal, nVal, nBTime, nTime = 0, nChance;
    int nSlope, nMax, nDist = 0;
    int effectID, t;

    char heal     = (pWeap->type == kCdudeWeaponHealth);
    char targOrig = (((pWeap->data5 & 0x001) == 0) ^ heal);

    if (pXSpr->target >= 0 && xsprIsFine(&sprite[pXSpr->target]))
    {
        pTarg = &sprite[pXSpr->target];
    }
    else if (targOrig)
    {
        return ATTACK_FAIL;
    }

    char dst2val  = ((pWeap->data5 & 0x002) == 0);
    char dst2eff  = ((pWeap->data5 & 0x004) != 0);
    char clipVal  = ((pWeap->data5 & 0x008) == 0);
    char clipEff  = ((pWeap->data5 & 0x010) == 0);
    char relEff   = ((pWeap->data5 & 0x020) != 0);

    char mustSee  = ((pWeap->data5 & 0x040) == 0);
    char mustDist = ((pWeap->data5 & 0x080) != 0);
    char mustScan = ((pWeap->data5 & 0x100) != 0);
    char mustVuln = ((pWeap->data5 & 0x200) != 0 && !heal);
    
    pOrig = (targOrig) ? pTarg : pSpr;
    nBVal = nVal = pWeap->data1 << 4;
    
    if (targOrig)
    {
        nDist = approxDist(pSpr->x - (pXSpr->targetX + a->dx), pSpr->y - (pXSpr->targetY + a->dy));

        if ((mustSee && !CanSee(pSpr, pTarg))
            || (mustDist && nDist > pWeap->GetDistance()))
                return ATTACK_FAIL;

        if (mustScan)
        {
            nSlope = pDude->AdjustSlope(pXSpr->target, 0);
            HitScan(pSpr, pSpr->z, Cos(pSpr->ang) >> 16, Sin(pSpr->ang) >> 16, nSlope, CLIPMASK1, 0);
            if (gHitInfo.hitsprite != pXSpr->target)
                return ATTACK_FAIL;
        }

        if (dst2val)
            nVal = divscale12(nVal, nDist);
    }

    pPlay = getPlayerById(pOrig->type);
    if (mustVuln && (nnExtIsImmune(pOrig, pWeap->id)
        || (pPlay && ((powerupCheck(pPlay, kPwUpDeathMask) > 0) || pPlay->godMode))))
            return ATTACK_FAIL;

    if (a->check)
        return kMaxSprites;
    
    if (clipVal)
        nVal = ClipRange(nVal, 0, nBVal);

    if (pWeap->data3 > 0 && irngok(pWeap->data2, kPlayerEffectPickup+1, kPlayerEffectMax))
    {
        nChance = perc2val(pWeap->data4, kChanceMax);
        if (Chance(nChance) && pTarg && (pPlay = getPlayerById(pTarg->type)) != NULL)
        {
            effectID = pWeap->data2 - 1;
            nBTime = nTime = pWeap->data3;
            t = playerEffectGet(pPlay, effectID);

            if (dst2eff)    nTime = ClipRange(divscale12(nTime, nDist), -nBTime, nBTime);
            if (relEff)     nTime = (nTime / nnExtRandom(1, 6)) + t;

            if (t < nTime)
            {
                t = (clipEff) ? pWeap->data3 : 1280;
                nTime = ClipRange(nTime, -t, t);
                playerEffectSet(pPlay, effectID, nTime);
            }
        }
    }

    if (nVal > 0)
    {
        if (heal)
        {
            pXOrig = &xsprite[pOrig->extra];
            nBVal = nnExtGetStartHealth(pOrig);
            nMax = perc2val((targOrig) ? pWeap->targHpRange[1] : pWeap->dudeHpRange[1], nBVal) + 16;

            if (pXOrig->health < nMax)
                pXOrig->health = ClipHigh(pXOrig->health + nVal, nMax);
        }
        else
        {
            actDamageSprite(pSpr->index, pOrig, (DAMAGE_TYPE)pWeap->id, nVal);
        }
    }

    pWeap->shot.appearance.Set(pOrig, pSpr);
    
    if (pWeap->HaveVelocity())
        nnExtScaleVelocity(pOrig, pWeap->shot.velocity, a->dx, a->dy, (pWeap->HaveSlope()) ? a->dz : 0);

    return kMaxSprites;
}

static int weaponShotMissile(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, ARG_WEAPONSHOT* a)
{
    if (a->check)
        return kMaxSprites;

    spritetype* pSpr = pDude->pSpr, *pShot;
    XSPRITE* pXSpr = pDude->pXSpr, *pXShot;

    pShot = nnExtFireMissile(pSpr, a->offs->x, a->offs->z, a->dx, a->dy, a->dz, pWeap->id);
    if (pShot)
    {
        pXShot = &xsprite[pShot->extra];
        nnExtOffsetSprite(pShot, 0, a->offs->y, 0);

        if (pWeap->shot.clipdist)
            pShot->clipdist = pWeap->shot.clipdist;

        if (pWeap->HaveVelocity())
        {
            pXShot->target = -1; // have to erase, so vanilla won't set velocity back
            nnExtScaleVelocity(pShot, pWeap->shot.velocity, a->dx, a->dy, a->dz);
        }

        pWeap->shot.appearance.Set(pShot, pSpr);

        if (pWeap->shot.targetFollow)
        {
            pXShot->goalAng = pWeap->shot.targetFollow;
            gFlwSpritesList.Add(pShot->index);
            pXShot->sysData1 = pXSpr->target;
            pXShot->target = -1; // have own target follow code
        }


        return pShot->index;
    }

    return ATTACK_FAIL;
}

static int weaponShotThing(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, ARG_WEAPONSHOT* a)
{
    if (a->check)
        return kMaxSprites;

    spritetype* pSpr = pDude->pSpr, *pShot;
    XSPRITE* pXSpr = pDude->pXSpr, * pXShot;

    int nTime = 120 * Random(2) + 120;
    int nDist = approxDist(a->dx, a->dy), nSlope = 12000, nDiv = 540;
    int nVel = divscale23(nDist / nDiv, 120);
    int tx = pSpr->x + a->dx, ty = pSpr->y + a->dy;
    int ta = pSpr->ang;

    nSlope = (pWeap->HaveSlope()) ? pWeap->shot.slope : ((a->dz / 128) - nSlope);
    
    pSpr->ang = getangle(pSpr->x - tx, pSpr->y - ty);
    pShot = actFireThing(pSpr, -a->offs->x, a->offs->z, nSlope, pWeap->id, nVel);
    pSpr->ang = ta;
    
    if (pShot)
    {
        nnExtOffsetSprite(pShot, 0, a->offs->y, 0);

        THINGINFO* pInfo        = &thingInfo[pWeap->id - kThingBase];
        pShot->picnum           = ClipLow(pInfo->picnum, 0);
        pXShot                  = &xsprite[pShot->extra];
        pShot->owner            = pSpr->index;

        switch (pWeap->id)
        {
            case kThingArmedProxBomb:
            case kModernThingTNTProx:
                pXShot->state = 0;
                fallthrough__;
            case kThingArmedRemoteBomb:
            case kThingArmedTNTBundle:
            case kThingArmedTNTStick:
            case kThingTNTBarrel:
                if (pWeap->data1 > 0)
                {
                    nTime = pWeap->data1;
                    if (pWeap->data2 > pWeap->data1)
                        nTime += Random(pWeap->data2 - pWeap->data1);
                }
                break;
            case kModernThingThrowableRock:
                pShot->picnum   = gCdudeDebrisPics[Random(LENGTH(gCdudeDebrisPics))];
                pShot->xrepeat  = pShot->yrepeat = 24 + Random(42);
                pShot->cstat    |= CSTAT_SPRITE_BLOCK;
                pShot->pal      = 5;

                if (Chance(0x5000)) pShot->cstat |= CSTAT_SPRITE_XFLIP;
                if (Chance(0x5000)) pShot->cstat |= CSTAT_SPRITE_YFLIP;

                if (pWeap->data1 > 0)
                {
                    pXShot->data1 = pWeap->data1;
                    if (pWeap->data2 > pWeap->data1)
                        pXShot->data1 += Random(pWeap->data2 - pWeap->data1);
                }
                else if (pShot->xrepeat > 60)  pXShot->data1 = 43;
                else if (pShot->xrepeat > 40)  pXShot->data1 = 33;
                else if (pShot->xrepeat > 30)  pXShot->data1 = 23;
                else                           pXShot->data1 = 12;
                break;
            case kThingBloodBits:
            case kThingBloodChunks:
                DudeToGibCallback1(pShot->index, pShot->extra);
                break;
            case kThingNapalmBall:
                pShot->xrepeat = pShot->yrepeat = 24;
                if (pWeap->data1 > 0)
                {
                    pXShot->data4 = pWeap->data1;
                    if (pWeap->data2 > pWeap->data1)
                        pXShot->data4 += Random(pWeap->data2 - pWeap->data1);
                }
                else pXShot->data4 = 3 + Random2(2);
                break;
            case kModernThingEnemyLifeLeech:
            case kThingDroppedLifeLeech:
                pXShot->health = ((pInfo->startHealth << 4) * ClipLow(gGameOptions.nDifficulty, 1)) >> 1;
                pShot->cstat        &= ~CSTAT_SPRITE_BLOCK;
                pShot->pal          = 6;
                pShot->clipdist     = 0;
                pXShot->data3       = 512 / (gGameOptions.nDifficulty + 1);
                pXShot->target      = pXSpr->target;
                pXShot->Proximity   = true;
                pXShot->stateTimer  = 1;

                evPost(pShot->index, 3, 80, kCallbackLeechStateTimer);
                pDude->pXLeech = &xsprite[pShot->extra];
                break;
        }

        if (pWeap->shot.clipdist)               pShot->clipdist = pWeap->shot.clipdist;
        if (pWeap->HaveVelocity())              nnExtScaleVelocity(pShot, -(pWeap->shot.velocity<<3), a->dx, a->dy, a->dz, 0x01);
        
        pWeap->shot.appearance.Set(pShot, pSpr);

        if (pWeap->shot.targetFollow)
        {
            pXShot->goalAng = pWeap->shot.targetFollow;
            pXShot->sysData1 = pXSpr->target;
            gFlwSpritesList.Add(pShot->index);
        }

        pXShot->Impact = pWeap->shot.impact;

        if (!pXShot->Impact)
            evPost(pShot->index, OBJ_SPRITE, nTime, kCmdOn, pSpr->index);

        return pShot->index;
    }

    return ATTACK_FAIL;
}


static int weaponShotSummon(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, ARG_WEAPONSHOT* a)
{
    spritetype* pShot, *pSpr = pDude->pSpr, *pDst = pSpr;
    XSPRITE *pXShot, *pXSpr = pDude->pXSpr;

    char blockCheck   = (pWeap->data2 & 0x001) == 0;
    char addKill      = (pWeap->data2 & 0x002) == 0;
    char targetOrigin = (pWeap->data2 & 0x004) != 0;
    char resetVel     = (pWeap->data2 & 0x008) != 0;

    if (targetOrigin && pXSpr->target >= 0)
        pDst = &sprite[pXSpr->target]; // target is origin

    int x = pDst->x+a->dx, y = pDst->y+a->dy, z = pDst->z+a->dz, nAng = 0;
    int nRadius = (blockCheck) ? 32 : 0;
    int nDude = pWeap->id;
    if (pWeap->type == kCdudeWeaponSummonCdude)
        nDude = kDudeModernCustom;

    if (nRadius && pWeap->data3 > 0)
        nRadius = pWeap->data3;

    nnExtOffsetPos(a->offs->x, 800 + a->offs->y, a->offs->z, pDst->ang, &x, &y, &z);

    while (nAng < kAng180)
    {
        if (!posObstructed(x, y, z, nRadius))
        {
            if (a->check)
                return kMaxSprites;
            
            if ((pShot = nnExtSpawnDude(pDst, nDude, x, y, z)) != NULL)
            {
                if (resetVel)
                {
                    xvel[pShot->index] = 0;
                    yvel[pShot->index] = 0;
                    zvel[pShot->index] = 0;
                }

                pXShot = &xsprite[pShot->extra];
                if (nDude == kDudeModernCustom)
                    pXShot->data1 = pWeap->id;

                if (pWeap->shot.clipdist)
                    pShot->clipdist = pWeap->shot.clipdist;

                if (pWeap->HaveVelocity())
                    nnExtScaleVelocity(pShot, pWeap->shot.velocity, a->dx, a->dy, a->dz);

                if (pWeap->data1)
                {
                    int nHealth = ClipHigh(pWeap->data1, 65535);
                    pXShot->health = ClipHigh(nHealth << 4, 65535);
                    pXShot->data4 = pXShot->sysData2 = nHealth;
                }

                aiInitSprite(pShot);

                pWeap->shot.appearance.Set(pShot, pSpr);

                pXShot->targetX = pXSpr->targetX;
                pXShot->targetY = pXSpr->targetY;
                pXShot->targetZ = pXSpr->targetZ;
                pXShot->target  = pXSpr->target;
                pShot->ang      = pSpr->ang;
                pShot->owner    = pSpr->index;

                aiActivateDude(pShot, pXShot);

                pDude->slaves.list->Add(pShot->index);
                
                if (addKill)
                    gKillMgr.AddCount(pShot);

                return pShot->index;
            }
        }
        else
        {
            RotatePoint(&x, &y, nAng, pDst->x, pDst->y);
            nAng += kAng15;
            continue;
        }

        break;
    }

    return ATTACK_FAIL;
}

static int weaponShotKamikaze(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, ARG_WEAPONSHOT* a)
{
    if (a->check)
        return kMaxSprites;
    
    spritetype* pSpr = pDude->pSpr;
    spritetype* pShot = actSpawnSprite(pSpr->sectnum, pSpr->x, pSpr->y, pSpr->z, kStatExplosion, true);
    XSPRITE* pXSpr = pDude->pXSpr;
    int nShot = -1;

    if (pShot)
    {
        int nType = pWeap->id - kTrapExploder;
        XSPRITE* pXShot = &xsprite[pShot->extra];
        EXPLOSION* pExpl = &explodeInfo[nType];
        EXPLOSION_EXTRA* pExtra = &gExplodeExtra[nType];

        pShot->type = nType;
        pShot->cstat |= CSTAT_SPRITE_INVISIBLE;
        pShot->owner = pSpr->index;
        pShot->shade = -127;
        pShot->yrepeat = pShot->xrepeat = pExpl->repeat;
        pShot->ang = pSpr->ang;
        
        pXShot->data1 = pExpl->ticks;
        pXShot->data2 = pExpl->quakeEffect;
        pXShot->data3 = pExpl->flashEffect;
        pXShot->data4 = ClipLow(pWeap->GetDistance() >> 4, pExpl->radius);

        seqSpawn(pExtra->seq, OBJ_SPRITE, pShot->extra, -1);

        if (pExtra->ground)
           pShot->z = getflorzofslope(pShot->sectnum, pShot->x, pShot->y);

        pWeap->shot.appearance.Set(pShot, pSpr);

        clampSprite(pShot);
        nnExtOffsetSprite(pShot, a->offs->x, a->offs->y, a->offs->z); // offset after default sprite placement
        nShot = pShot->index;
    }

    if (pXSpr->health)
    {
        pXSpr->health = 0; // it supposed to attack once
        pDude->Kill(pSpr->index, kDamageExplode, 0x10000);
    }
    
    return nShot;
}

static int weaponShotSpecialBeastStomp(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeapon, ARG_WEAPONSHOT* a)
{
    static short stat[] = { kStatDude, kStatThing };
    spritetype* pSpr = pDude->pSpr;
    
    int i, j;
    int vc = 400 << 4;
    int v1c = 7 * gGameOptions.nDifficulty;
    int v10 = 55 * gGameOptions.nDifficulty;

    for (i = 0; i < LENGTH(stat); i++)
    {
        for (j = headspritestat[stat[i]]; j >= 0; j = nextspritestat[j])
        {
            spritetype* pSpr2 = &sprite[j];
            if (pSpr2->index == pSpr->index || !xsprIsFine(pSpr2) || pSpr2->owner == pSpr->index)
                continue;

            if (CheckProximity(pSpr2, pSpr->x, pSpr->y, pSpr->z, pSpr->sectnum, pWeapon->GetDistance() << 4))
            {
                int dx = klabs(pSpr->x - pSpr2->x);
                int dy = klabs(pSpr->y - pSpr2->y);
                int nDist2 = ksqrt(dx * dx + dy * dy);
                if (nDist2 <= vc)
                {
                    int nDamage;
                    if (!nDist2)
                        nDamage = v1c + v10;
                    else
                        nDamage = v1c + ((vc - nDist2) * v10) / vc;
                    
                    if (a->check)
                    {
                        if ((nDamage << 4) > 0 && pSpr2->index == pDude->pXSpr->target)
                            return kMaxSprites;

                        continue;
                    }
                    
                    if (IsPlayerSprite(pSpr2))
                    {
                        PLAYER* pPlayer = &gPlayer[pSpr2->type - kDudePlayer1];
                        pPlayer->quakeEffect = ClipHigh(pPlayer->quakeEffect + (nDamage << 2), 1280);
                    }

                    actDamageSprite(pSpr->index, pSpr2, kDamageFall, nDamage << 4);
                }
            }
        }
    }

    return (a->check) ? ATTACK_FAIL : kMaxSprites;
}

static int weaponShotSpecialRam(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, ARG_WEAPONSHOT* a)
{
    if (a->check)
        return kMaxSprites;
    
    spritetype *pSpr = pDude->pSpr,   *pHSpr;
    XSPRITE *pXSpr   = pDude->pXSpr,  *pXHSpr;
    SPRITEHIT *pTouch = &gSpriteHit[pSpr->extra];

    int cz, fz, cf, zt, zb, zv, x, y, z, s, d, k, m;
    int tx = pXSpr->targetX - pSpr->x;
    int ty = pXSpr->targetY - pSpr->y;
    int nDist, nAng, nDAng, nTouch;

    char targetOnly   = (pWeap->data1 & 0x001) == 0;
    char touchOnce    = (pWeap->data1 & 0x002) != 0;
    char absoluteVel  = (pWeap->data1 & 0x004) != 0;
    char scaleDmg     = (pWeap->data1 & 0x008) == 0;
    char checkCF      = (pWeap->data1 & 0x010) == 0;
    char keepDrag     = (pWeap->data1 & 0x020) != 0;
    char stopOnAng    = (pWeap->data1 & 0x040) != 0;
    char scaleKick    = (pWeap->data1 & 0x080) != 0;
    char stopOnDist   = (pWeap->data1 & 0x100) == 0;

    nTouch = pXSpr->target;
    nAng = getangle(tx, ty);

    while ( 1 )
    {
        if ((pTouch->hit & 0xc000) == 0x8000)
            break;

        if ((pTouch->hit & 0xc000) == 0xc000)
        {
            pHSpr = &sprite[pTouch->hit & 0x3fff];
            if (pHSpr->extra <= 0)
                break;

            if (targetOnly && pXSpr->target != pHSpr->index)
                break;

            pXHSpr = &xsprite[pHSpr->extra];
            if ((pXHSpr->physAttr & kPhysDebrisTouch) == 0)
            {
                if (pHSpr->statnum != kStatDude && pHSpr->statnum != kStatThing)
                    break;
                
                if ((pHSpr->flags & (kPhysMove | kPhysGravity)) == 0)
                    break;
            }

            k = pWeap->data4 << 8;
            if (scaleKick && k > 0)
            {
                if (IsCustomDude(pHSpr))        m = cdudeGet(pHSpr->index)->mass;
                else if (IsDudeSprite(pHSpr))   m = getDudeInfo(pHSpr->type)->mass;
                else                            m = 0;
                
                if (m)
                    k = divscale4(k, m);
            }

            d = (scaleDmg) ? mulscale18(pWeap->data3, approxDist(xvel[pSpr->index], yvel[pSpr->index])) : pWeap->data3;
            helperPounce(pSpr, pHSpr, pWeap->data2, d, k);
            if (touchOnce)
                break;

            s = pHSpr->sectnum, x = pHSpr->x, y = pHSpr->y, z = pHSpr->z;
            ClipMove(&x, &y, &z, &s, xvel[pHSpr->index] >> 12, yvel[pHSpr->index] >> 12, pHSpr->clipdist << 2, 0, 0, CLIPMASK0);
            if (approxDist(x - pHSpr->x, y - pHSpr->y) <= 0)
                break;
            
            pDude->goalZ = pXSpr->targetZ;
            checkCF = (keepDrag == 0);
            nTouch  = pHSpr->index;
        }

        nDist = pWeap->GetDistance();
        nDAng = klabs(DANGLE(nAng, pXSpr->goalAng));
        if (nDAng > kAng90 && (stopOnAng || (stopOnDist && approxDist(tx, ty) >= nDist)))   break;
        else if (!CanMove(pSpr, nTouch, pSpr->ang, pSpr->clipdist << 2))                    break;
        else if (checkCF && pDude->IsFlying())
        {
            cf = pDude->flight.cfDist, zv = zvel[pSpr->index];
            getzsofslope(pSpr->sectnum, pSpr->x, pSpr->y, &cz, &fz); GetSpriteExtents(pSpr, &zt, &zb);
            if ((zv > 0 && zb >= fz - cf) || (zv < 0 && zt <= cz + cf))
                break;
        }
        
        if (pWeap->HaveVelocity())
        {
            if (!pDude->IsFlying() && !pWeap->HaveSlope())
                a->dz = 0;

            if (!absoluteVel && !pDude->IsFlying())
                nnExtScaleVelocityRel(pDude->pSpr, pWeap->shot.velocity, a->dx, a->dy, a->dz);
            else
                nnExtScaleVelocity(pDude->pSpr, pWeap->shot.velocity, a->dx, a->dy, a->dz);
        }

        // continue to move
        return ATTACK_CONTINUE;
    }

    pXSpr->goalAng = (nAng + Random2(kAng45)) & kAngMask;
    pDude->NewState(kCdudeStateSearch);
    return ATTACK_FAIL;
}

static int weaponShotSpecialTeleport(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, ARG_WEAPONSHOT* a)
{
    if (a->check)
        return kMaxSprites;
    
    spritetype* pSpr = pDude->pSpr; XSPRITE* pXSpr = pDude->pXSpr;
    int ox, rx, x, oy, ry, y, oz, z, s;
    int c = 32;

    x = ox = a->dx, y = oy = a->dy, z = oz = pXSpr->targetZ - a->dz;
    s = pSpr->sectnum;

    nnExtOffsetPos(a->offs, pSpr->ang, &x, &y, &z);
    rx = ox - x, ry = oy - y;

    while (--c >= 0)
    {
        if (!FindSector(x, y, z, &s))
        {
            if (c > 0)
            {
                x = ox + Random(rx);
                y = oy + Random(ry);
                z = oz;
            }
            else
            {
                x = pXSpr->targetX;
                y = pXSpr->targetY;
                z = pXSpr->targetZ;
            }

            continue;
        }
        
        pSpr->x = x;
        pSpr->y = y;
        pSpr->z = z;

        if (s != pSpr->sectnum)
            ChangeSpriteSect(pSpr->index, s);

        if (pWeap->turnToTarget)
            pXSpr->goalAng = getTargetAng(pSpr, pXSpr), pSpr->ang = pXSpr->goalAng;

        if (pWeap->HaveVelocity())
            nnExtScaleVelocity(pDude->pSpr, pWeap->shot.velocity, a->dx, a->dy, a->dz);

        clampSprite(pSpr);

        if (!pDude->IsFlying() && pSpr->flags & kPhysGravity)
            pSpr->flags |= kPhysFalling;
        
        break;
    }

    return kMaxSprites;
}

static int weaponShotMorph(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, ARG_WEAPONSHOT* a)
{
    if (a->check)
        return kMaxSprites;
    
    spritetype* pSpr = pDude->pSpr; XSPRITE* pXSpr = pDude->pXSpr;
    CUSTOMDUDE_TIMER delay; CUSTOMDUDE_WEAPON* w;
    int nTarget = pXSpr->target, i, n;
    int nHealth = pXSpr->health;
    int nID = pWeap->id;

    char chgHealth  = (pWeap->data2 & 0x001) != 0;
    char chgAmmo    = (pWeap->data2 & 0x002) != 0;
    
    if (pWeap->type == kCdudeWeaponMorph)
    {
        pSpr->clipdist  = getDudeInfo(pSpr->type)->clipdist;
        pSpr->type      = pWeap->id;
        pSpr->inittype  = pSpr->type;
        pXSpr->data1    = 0;

        if (chgHealth)
            pXSpr->health = nnExtDudeStartHealth(pSpr, 0);
    }
    else if (pWeap->type == kCdudeWeaponMorphCdude)
    {
        pXSpr->data1 = pWeap->id;
    }
    else
    {
        return ATTACK_FAIL;
    }
    
    pDude->initialized  = 0;
    pXSpr->data2        = 0;
    pXSpr->data3        = 0;

    if (chgHealth)
        pXSpr->sysData2 = pXSpr->data4 = 0;

    memcpy(&delay, &pWeap->cooldown.delay, sizeof(delay));

    if (pWeap->HaveVelocity())
        nnExtScaleVelocity(pSpr, pWeap->shot.velocity, a->dx, a->dy, a->dz);

    pWeap->shot.appearance.Set(pSpr, NULL);
    pWeap->shotSound.Play(pSpr);

    if (pWeap->data1)
    {
        int nHealth = ClipHigh(pWeap->data1, 65535);
        pXSpr->health = ClipHigh(nHealth << 4, 65535);
        pXSpr->data4 = pXSpr->sysData2 = nHealth;
    }

    aiInitSprite(pSpr);             // re-init sprite with all new settings
    
    if (!chgHealth)
        pXSpr->health = nHealth;
    
    if (spriRangeIsFine(nTarget)) aiSetTarget(pXSpr, nTarget); // try to restore target
    else aiSetTarget(pXSpr, pSpr->x, pSpr->y, pSpr->z);
    aiActivateDude(pSpr, pXSpr);

    if (pSpr->type == kDudeModernCustom)
    {
        pDude = cdudeGet(pSpr);

        i = pDude->numWeapons;
        while (--i >= 0)
        {
            w = &pDude->weapons[i];
            if (w->type != kCdudeWeaponMorph && w->type != kCdudeWeaponMorphCdude)
                continue;
                
            if ((n = delay.rng[0]) > 0)
            {
                // manually set delay from previous
                // dude form to a new
                // one

                if (delay.rng[1])
                    n += Random(delay.rng[1] - delay.rng[0]);

                if (delay.mul)
                    n *= delay.mul;

                w->cooldown.delay.Set(n);
            }
            
            if (w->id == nID && chgAmmo)
                w->ammo.Dec();
        }
    }

    return ATTACK_BREAK_ALL;
}

static int weaponShotSpecial(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, ARG_WEAPONSHOT* a)
{
    switch (pWeap->id)
    {
        case kCdudeWeaponIdSpecialBeastStomp:   return weaponShotSpecialBeastStomp(pDude, pWeap, a);
        case kCdudeWeaponIdSpecialRam:          return weaponShotSpecialRam(pDude, pWeap, a);
        case kCdudeWeaponIdSpecialTeleport:     return weaponShotSpecialTeleport(pDude, pWeap, a);
        default:                                return ATTACK_FAIL;
    }
}

static void weaponShotCallback(int, int nXIndex)
{
    if (!xspriRangeIsFine(nXIndex))
        return;

    XSPRITE* pXSpr = &xsprite[nXIndex];
    CUSTOMDUDE* pDude = cdudeGet(pXSpr->reference);

    cdudeWeaponShot(pDude, pDude->pWeapon, 0);
}

int cdudeWeaponShot(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pMainWeap, char check)
{
    XSPRITE* pXSpr = pDude->pXSpr; CUSTOMDUDE_WEAPON *pWeap;
    spritetype* pSpr = pDude->pSpr, *pShot, *pTarg;
    CUSTOMDUDE_WEAPON::PREDICTION* pPredict;
    POINT3D shotOffs, *pStyleOffs;
    
    ARG_WEAPONSHOT argShot;

    int nAng, nDang, nShots, nTime, nCode, nDist;
    int dx1, dy1, dz1, tx, ty, tx2, ty2;
    int dx2, dy2, dz2=0, dx3=0, dy3=0, dz3;
    int sx, sy, i, j, t;

    int txof; char hxof=0;
    int sang=0; int  hsht;
    int tang=0; char styled;

    if (!pMainWeap)
        return ATTACK_BREAK_ALL;

    memset(&argShot, 0, sizeof(argShot));
    argShot.check = check;

    for (i = 0; i < pDude->numWeapons && (check || pDude->IsAttacking()); i++)
    {
        if (!check)
        {
            pWeap = &pDude->weapons[i];
            if (!pWeap->available)
                continue;

            if (pMainWeap != pWeap)
            {
                // check if this weapon could be used in conjunction with current
                if (!pMainWeap->sharedId || pMainWeap->sharedId != pWeap->sharedId)
                    continue;
            }

            if (pDude->numAvailWeapons >= 2 // check if weapon must shot on this seq frame index
                && pWeap->pFrames && !pWeap->pFrames->Exists(seqGetStatus(OBJ_SPRITE, pSpr->extra)))
                continue;
        }
        else
        {
            pWeap = pMainWeap;
        }

        nShots = pWeap->GetNumshots();
        styled = (nShots > 1 && pWeap->style.available);
        shotOffs = pWeap->shot.offset;
        pPredict = &pWeap->prediction;

        nAng = pSpr->ang;
        sx = pSpr->x, sy = pSpr->y;
        tx = pXSpr->targetX, ty = pXSpr->targetY;
        nDang = DANGLE(pSpr->ang, getangle(tx - sx, ty - sy));

        if (!check)
            pWeap->ammo.Dec(nShots);

        if (klabs(nDang) < pWeap->angle)
        {
            nnExtOffsetPos(&shotOffs, nAng, &sx, &sy, NULL);
            nAng = getangle(tx - sx, ty - sy);
        }

        if (pPredict->distance && spriRangeIsFine(pXSpr->target))
        {
            // this is similar to TARGETTRACK
            pTarg = &sprite[pXSpr->target], tx = pTarg->x, ty = pTarg->y;
            nDist = approxDist(tx-sx, ty-sy);

            if ((xvel[pTarg->index] || yvel[pTarg->index]) && nDist < (int)pPredict->distance)
            {
                j = divscale12(nDist, pPredict->accuracy);
                tx2 = tx + ((xvel[pTarg->index]*j) >> 12);
                ty2 = ty + ((yvel[pTarg->index]*j) >> 12);
                j = getangle(tx2 - sx, ty2 - sy);
                        
                nDang = DANGLE(j, nAng);
                if (klabs(nDang) < pPredict->angle)
                    nAng = j, tx = tx2, ty = ty2;
            }
        }

        switch (pWeap->type)
        {
            case kCdudeWeaponSummon:
            case kCdudeWeaponSummonCdude:
                dx1 = 0;
                dy1 = 0;
                break;
            case kCdudeWeaponThrow:
                dx1 = sx - tx;
                dy1 = sy - ty;
                break;
            default:
                switch (pWeap->id)
                {
                    case kCdudeWeaponIdSpecialTeleport:
                        dx1 = tx;
                        dy1 = ty;
                        break;
                    default:
                        dx1 = Cos(nAng) >> 16;
                        dy1 = Sin(nAng) >> 16;
                        break;
                }
                break;
        }

        if (styled)
        {
            t = nShots;
            pStyleOffs = &pWeap->style.offset; hsht = t >> 1;
            if (t % 2)
                t++;
            
            sang = pWeap->style.angle / t;
            hxof = 0;
            tang = 0;
        }

        dz1 = (pWeap->shot.slope == INT32_MAX) ?
                pDude->AdjustSlope(pXSpr->target, pWeap->shot.offset.z) : pWeap->shot.slope;

        nCode = -1;
        for (j = nShots; j > 0 && (check || pDude->IsAttacking()); j--)
        {
            if (!styled || j == nShots)
            {
                dx3 = Random3(pWeap->dispersion[0]);
                dy3 = Random3(pWeap->dispersion[0]);
                dz3 = Random3(pWeap->dispersion[1]);

                dx2 = dx1 + dx3;
                dy2 = dy1 + dy3;
                dz2 = dz1 + dz3;
            }

            argShot.offs = &shotOffs;
            argShot.dx = dx2, argShot.dy = dy2, argShot.dz = dz2;

            if (pWeap->spawnsSprite && !pDude->CheckUnits(pWeap))
            {
                nCode = ATTACK_FAIL;
                j = 0;
            }
            else
            {
                nCode = gWeaponShotFunc[pWeap->type](pDude, pWeap, &argShot);
            }

            if (argShot.check)
                return nCode;

            if (nCode == ATTACK_BREAK_ALL)
                return ATTACK_BREAK_ALL;

            if (nCode >= 0)
            {
                pShot = (nCode < kMaxSprites) ? &sprite[nCode] : NULL;
                if (pShot)
                {
                    pShot->ang = nAng;
                    if ((pShot->cstat & CSTAT_SPRITE_ALIGNMENT_MASK) == CSTAT_SPRITE_ALIGNMENT_WALL)
                        pShot->ang = (pShot->ang + kAng90) & kAngMask;

                    // override removal timer
                    if ((nTime = pWeap->shot.remTime) >= 0)
                    {
                        evKill(pShot->index, OBJ_SPRITE, kCallbackRemove);
                        if (nTime)
                            evPost(pShot->index, OBJ_SPRITE, nTime, kCallbackRemove);
                    }
                }

                // setup style
                if (styled)
                {
                    if (pStyleOffs->x)
                    {
                        txof = pStyleOffs->x;
                        if (j <= hsht)
                        {
                            if (!hxof)
                            {
                                shotOffs.x = pWeap->shot.offset.x;
                                hxof = 1;
                            }

                            txof = -txof;
                        }

                        shotOffs.x += txof;
                    }

                    shotOffs.y += pStyleOffs->y;
                    shotOffs.z += pStyleOffs->z;

                    if (pWeap->style.angle)
                    {
                        // for sprites
                        if (pShot)
                        {
                            if (j <= hsht && sang > 0)
                            {
                                sang = -sang;
                                tang = sang;
                            }
                            
                            RotatePoint(&xvel[pShot->index], &yvel[pShot->index], tang, pSpr->x, pSpr->y);
                            
                            pShot->ang = getVelocityAngle(pShot);
                            if ((pShot->cstat & CSTAT_SPRITE_ALIGNMENT_MASK) == CSTAT_SPRITE_ALIGNMENT_WALL)
                                pShot->ang = (pShot->ang + kAng90) & kAngMask;

                            tang += sang;
                            
                        }
                        // for hitscan
                        else
                        {
                            if (j <= hsht && sang > 0)
                            {
                                nnExtCoSin(pSpr->ang, &dx2, &dy2);
                                dx2 += dx3; dy2 += dy3;
                                sang = -sang;
                            }
                                    
                            RotatePoint(&dx2, &dy2, sang, pSpr->x, pSpr->y);
                        }
                    }
                }
            }
        }

        pWeap->shotSound.Play(pSpr);
        if (nCode != ATTACK_CONTINUE && pWeap->cooldown.Check())
            pWeap->available = 0;
    }

    return 0; // !!!
}

static int checkTarget(CUSTOMDUDE* pDude, spritetype* pTarget, TARGET_INFO* pOut)
{
    spritetype*pSpr = pDude->pSpr;
    if (!xspriRangeIsFine(pTarget->extra))
        return -1;

    XSPRITE* pXTarget = &xsprite[pTarget->extra];
    if (pSpr->owner == pTarget->index || pXTarget->health <= 0)
        return -2;

    if (IsPlayerSprite(pTarget))
    {
        PLAYER* pPlayer = &gPlayer[pTarget->type - kDudePlayer1];
        if (powerupCheck(pPlayer, kPwUpShadowCloak) > 0)
            return -3;
    }

    int x = pTarget->x;
    int y = pTarget->y;
    int z = pTarget->z;
    int nSector = pTarget->sectnum;
    int dx = x - pSpr->x;
    int dy = y - pSpr->y;
    int nDist = approxDist(dx, dy);
    int nHeigh = pDude->eyeHeight;
    char s = (nDist < pDude->seeDist);
    char h = (nDist < pDude->hearDist);
    
    if (!s && !h)
        return -4;

    if (pDude->IsFlipped()) nHeigh = -nHeigh;
    if (!cansee(x, y, z, nSector, pSpr->x, pSpr->y, pSpr->z - nHeigh, pSpr->sectnum))
        return -5;

    int nAng = getangle(dx, dy);
    if (s)
    {
        int nDang = klabs(((nAng + kAng180 - pSpr->ang) & kAngMask) - kAng180);
        if (nDang <= pDude->periphery)
        {
            pOut->pSpr  = pTarget;
            pOut->pXSpr = pXTarget;
            pOut->nDist = nDist;
            pOut->nDang = nDang;
            pOut->nAng  = nAng;
            pOut->nCode = 1;
            return 1;
        }
    }
    
    if (h)
    {
        pOut->pSpr  = pTarget;
        pOut->pXSpr = pXTarget;
        pOut->nDist = nDist;
        pOut->nDang = 0;
        pOut->nAng  = nAng;
        pOut->nCode = 2;
        return 2;
    }

    return -255;
}

static void thinkTarget(spritetype* pSpr, XSPRITE* pXSpr)
{
    static TARGET_INFO target, tmp;

    CUSTOMDUDE* pDude = cdudeGet(pSpr->index), *tDude;
    DUDEINFO* pInfo = pDude->pInfo;
    spritetype* pTarg;
    int i;

    target.nDist = INT32_MAX;
    target.nCode = -255;

    if (Chance(pInfo->alertChance))
    {
        for (i = connecthead; i >= 0; i = connectpoint2[i])
        {
            PLAYER* pPlay = &gPlayer[i];
            if (checkTarget(pDude, pPlay->pSprite, &tmp) > 0 && tmp.nDist < target.nDist)
                Bmemcpy(&target, &tmp, sizeof(tmp));
        }

        if (target.nCode < 0)
        {
            if (pXSpr->state && pDude->StatusTest(kCdudeStatusCruel))
            {
                // also search for innocents
                for (i = headspritestat[kStatDude]; i >= 0; i = nextspritestat[i])
                {
                    pTarg = &sprite[i];

                    if (pTarg->index == pSpr->index)
                        continue;

                    switch (pTarg->type)
                    {
                        case kDudeInnocent:
                            break;
                        case kDudeModernCustom:
                            tDude = cdudeGet(pTarg->index);
                            if (tDude->StatusTest(KCdudeStatusInnocent)
                                || (tDude->numWeapons == 1 && tDude->pWeapon && tDude->pWeapon->type == kCdudeWeaponNone))
                                    break;
                            
                            fallthrough__;
                        default:
                            continue;
                    }

                    if (checkTarget(pDude, pTarg, &tmp) > 0 && tmp.nDist < target.nDist)
                        Bmemcpy(&target, &tmp, sizeof(tmp));
                }
            }
        }

        if (target.nCode > 0)
        {
            pTarg = target.pSpr;

            if (!pDude->IsBurning()
                && (pXSpr->target != pTarg->index || Chance(0x0400)))
                    pDude->PlaySound(kCdudeSndTargetSpot);

            pXSpr->goalAng = target.nAng & kAngMask;
            if (target.nCode == 1) aiSetTarget(pXSpr, pTarg->index);
            else aiSetTarget(pXSpr, pTarg->x, pTarg->y, pTarg->z);
            aiActivateDude(pSpr, pXSpr);
        }
    }
}

static void enterFlee(spritetype* pSpr, XSPRITE* pXSpr)
{
    CUSTOMDUDE* pDude = cdudeGet(pSpr);
    CUSTOMDUDE_WEAPON* pWeap = pDude->pWeapon;
    AISTATE* pState;

    pDude->velocity.SetModID(kCdudeVelModFlee);

    if (pWeap && pWeap->type == kCdudeWeaponNone)
    {
        if (pWeap->available)
        {
            pWeap->ammo.Dec();

            if (pWeap->cooldown.Check())
                pWeap->available = 0;

            pState = pDude->states[pWeap->stateID];
            if (!pState->stateTicks)
                pState = gCdudeStateTemplate[kCdudeStateFlee];

            // try to override state time
            pXSpr->aiState->stateTicks = pState->stateTicks;
            pXSpr->stateTimer          = pState->stateTicks;
        }
    }

    thinkFlee(pSpr, pXSpr);
}

static void thinkFlee(spritetype* pSpr, XSPRITE* pXSpr)
{
    int nAng = getangle(pSpr->x - pXSpr->targetX, pSpr->y - pXSpr->targetY);
    int nDang = klabs(((nAng + kAng180 - pSpr->ang) & kAngMask) - kAng180);
    if (nDang > kAng45)
        pXSpr->goalAng = (nAng + (kAng15 * Random2(2))) & kAngMask;

    aiChooseDirection(pSpr, pXSpr, pXSpr->goalAng);

}

static void thinkKnock(spritetype* pSpr, XSPRITE*)
{
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);

    if (pDude->StatusTest(kCdudeStatusLStand))
    {
        if (Chance(0x4000))
            pDude->PlaySound(kCdudeSndLStand);
    }
}

static void enterIdle(spritetype* pSpr, XSPRITE* pXSpr)
{
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);
    pDude->velocity.SetModID(kCdudeVelModNone);
    pDude->pExtra->stats.thinkTime = 0;
    pDude->pExtra->stats.active = 0;
    resetTarget(pSpr, pXSpr);
}

static void enterSearch(spritetype* pSpr, XSPRITE* pXSpr)
{
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);
    spritetype* pTarg;
    int dz;

    pDude->velocity.SetModID(kCdudeVelModSearch);

    if (!pDude->CanFly() || pDude->IsCrouching() || pDude->IsSwimming())
        return;

    if (pDude->IsFlying())
    {
        pXSpr->goalAng = (pXSpr->goalAng + Random2(kAng45)) & kAngMask;
        return;
    }

    dz = pXSpr->targetZ - pSpr->z;
    if (dz >= 0 || klabs(dz) < perc2val(150, pDude->height))
    {
        pTarg = spriRangeIsFine(pXSpr->target) ? &sprite[pXSpr->target] : NULL;
        if (!pTarg || cansee(pSpr->x, pSpr->y, pSpr->z, pSpr->sectnum, pTarg->x, pTarg->y, pTarg->z, pTarg->sectnum))
            return;
    }

    zvel[pSpr->index] = pDude->GetStartFlyVel();
    pDude->ChangePosture(kCdudePostureF);
    pDude->timer.fLaunch.Set();
}

static void enterChase(spritetype* pSpr, XSPRITE*)
{
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);
    pDude->velocity.SetModID(kCdudeVelModChase);
}


static void thinkSearch(spritetype* pSpr, XSPRITE* pXSpr)
{
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);
    
    if (pDude->timer.moveDir.Pass())
        aiChooseDirection(pSpr, pXSpr, pXSpr->goalAng);

    thinkTarget(pSpr, pXSpr);
}

static void thinkChase(spritetype* pSpr, XSPRITE* pXSpr)
{
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index); HITINFO* pHit = &gHitInfo;
    CUSTOMDUDE_WEAPON* pWeapon = pDude->pWeapon;
    CUSTOMDUDE_FLIGHT::TYPE* pFly;

    int nDist, nHeigh, dx, dy, dz, nDAng, nSlope = 0;
    char thinkTime = pDude->IsThinkTime();
    char turn2target = 0, interrupt = 0;
    char inAttack = pDude->IsAttacking();
    char changePos = 0;

    if (!spriRangeIsFine(pXSpr->target))
    {
        pDude->NewState(kCdudeStateSearch);
        return;
    }

    spritetype* pTarget = &sprite[pXSpr->target];
    if (pTarget->owner == pSpr->index || !IsDudeSprite(pTarget) || !xsprIsFine(pTarget)) // target lost
    {
        pDude->NewState(kCdudeStateSearch);
        return;
    }

    XSPRITE* pXTarget = &xsprite[pTarget->extra];
    if (pXTarget->health <= 0) // target is dead
    {
        PLAYER* pPlayer = NULL;
        if ((!IsPlayerSprite(pTarget)) || ((pPlayer = getPlayerById(pTarget->type)) != NULL && pPlayer->fraggerId == pSpr->index))
            pDude->PlaySound(kCdudeSndTargetDead);
        
        if (inAttack) pDude->NextState(kCdudeStateSearch);
        else pDude->NewState(kCdudeStateSearch);
        return;
    }

    if (IsPlayerSprite(pTarget))
    {
        PLAYER* pPlayer = &gPlayer[pTarget->type - kDudePlayer1];
        if (powerupCheck(pPlayer, kPwUpShadowCloak) > 0)
        {
            pDude->NewState(kCdudeStateSearch);
            return;
        }
    }

    // check target
    dx = pTarget->x - pSpr->x;
    dy = pTarget->y - pSpr->y;
    dz = pTarget->z - pSpr->z;

    if (dx == 0) dx = 1;
    if (dy == 0) dy = 1;

    nDist  = approxDist(dx, dy);
    nDAng  = klabs(((getangle(dx, dy) + kAng180 - pSpr->ang) & kAngMask) - kAng180);
    nHeigh = pDude->eyeHeight;

    // is the target visible?
    if (pDude->IsFlipped()) nHeigh = -nHeigh;
    if (nDist > pDude->seeDist || !cansee(pTarget->x, pTarget->y, pTarget->z, pTarget->sectnum, pSpr->x, pSpr->y, pSpr->z - nHeigh, pSpr->sectnum))
    {
        if (inAttack) pDude->NextState(kCdudeStateSearch);
        else pDude->NewState(kCdudeStateSearch);
        return;
    }
    else if (nDAng > pDude->periphery)
    {
        if (inAttack) pDude->NextState(kCdudeStateSearch);
        else pDude->NewState(kCdudeStateSearch);
        return;
    }

    if (thinkTime)
    {
        aiSetTarget(pXSpr, pTarget->index);

        if (!inAttack)
        {
           if (pDude->timer.moveDir.Pass() || !CanMove(pSpr, pXSpr->target, pSpr->ang, pSpr->clipdist<<2))
           {
               aiChooseDirection(pSpr, pXSpr, getangle(dx, dy));
               pDude->timer.moveDir.Set();
           }

           if (pDude->IsFlying())
           {
               pFly = &pDude->flight.type[kCdudeFlyLand];
               if (pDude->timer.fLand.Pass() && Chance(pFly->chance) && irngok(nDist, pFly->distance[0], pFly->distance[1]))
               {
                   if (!pFly->distance[2] || rngok(dz, -pFly->distance[2], pFly->distance[2]))
                   {
                       pDude->timer.fLand.Set();
                       if (Chance(0x8000))
                           return;
                   }
               }
           }
           else if (pDude->CanFly())
           {
               pFly = &pDude->flight.type[kCdudeFlyStart];
               if (pDude->timer.fLaunch.Pass() && Chance(pFly->chance) && irngok(nDist, pFly->distance[0], pFly->distance[1]))
               {
                   if (!pFly->distance[2] || dz <= -(int)pFly->distance[2])
                   {
                       zvel[pSpr->index] = pDude->GetStartFlyVel();
                       pDude->ChangePosture(kCdudePostureF);
                       pDude->timer.fLaunch.Set();
                       if (Chance(0x8000))
                           return;
                   }
               }
           }
        }

        if (Chance(0x2000))
            pDude->PlaySound(kCdudeSndTargetChase);
    }

    if (pWeapon)
    {
        nSlope      = pDude->AdjustSlope(pXSpr->target, pWeapon->shot.offset.z);
        turn2target = pWeapon->turnToTarget;
        interrupt   = pWeapon->interruptable;
    }

    ARG_PICK_WEAPON weapData(pSpr, pXSpr, pTarget, pXTarget, nDist, nDAng, nSlope);

    // in attack
    if (inAttack)
    {
        if (turn2target && thinkTime)
        {
            pXSpr->goalAng = getTargetAng(pSpr, pXSpr);
            moveTurn(pSpr, pXSpr);
        }

        if (pXSpr->aiState->stateTicks) // attack timer set
        {
            if (!pXSpr->stateTimer)
            {
                pWeapon = pDude->PickWeapon(&weapData);
                if (pWeapon && pWeapon == pDude->pWeapon)
                {
                    pDude->pWeapon = pWeapon;
                    pDude->NewState(pWeapon->stateID);
                }
                else
                    pDude->NewState(kCdudeStateChase);
            }
            else if (interrupt)
            {
                pDude->PickWeapon(&weapData);
                if (!pWeapon->available)
                    pDude->NewState(kCdudeStateChase);
            }

            return;
        }

        if (!pDude->SeqPlaying()) // final frame
        {
            pWeapon = pDude->PickWeapon(&weapData);
            if (!pWeapon)
            {
                pDude->NewState(kCdudeStateChase);
                return;
            }
            else
            {
                pDude->pWeapon = pWeapon;
            }
        }
        else // playing previous animation
        {
            if (!interrupt)
            {
                if (!pWeapon)
                {
                    pDude->NextState(kCdudeStateChase);
                }

                return;
            }
            else
            {
                pDude->PickWeapon(&weapData);
                if (!pWeapon->available)
                {
                    pDude->NewState(kCdudeStateChase);
                    return;
                }
            }
        }
    }
    else
    {
        // enter attack
        pWeapon = pDude->PickWeapon(&weapData);
        if (pWeapon)
            pDude->pWeapon = pWeapon;
    }

    if (pWeapon)
    {
        switch (pWeapon->type)
        {
            case kCdudeWeaponNone:
                if (inAttack) pDude->NextState(pDude->CanMove() ? kCdudeStateFlee : kCdudeStateSearch);
                else pDude->NewState(pDude->CanMove()  ? kCdudeStateFlee : kCdudeStateSearch);
                return;
            case kCdudeWeaponHitscan:
            case kCdudeWeaponMissile:
            case kCdudeWeaponThrow:
            case kCdudeWeaponSpecial:
                HitScan(pSpr, pSpr->z, dx, dy, nSlope, pWeapon->clipMask, nDist);
                if (pHit->hitsprite != pXSpr->target && !pDude->AdjustSlope(nDist, &nSlope))
                {
                    changePos = 1;
                    if (spriRangeIsFine(pHit->hitsprite))
                    {
                        spritetype* pHitSpr = &sprite[pHit->hitsprite];
                        XSPRITE* pXHitSpr = NULL;
                        if (xsprIsFine(pHitSpr))
                            pXHitSpr = &xsprite[pHitSpr->extra];

                        if (IsDudeSprite(pHitSpr))
                        {
                            if (pXHitSpr)
                            {
                                if (pXHitSpr->target == pSpr->index)
                                    return;

                                if (pXHitSpr->dodgeDir > 0)
                                    pXSpr->dodgeDir = -pXHitSpr->dodgeDir;
                            }
                        }
                        else if (pHitSpr->owner == pSpr->index) // projectiles, things, fx etc...
                        {
                            if (!pXHitSpr || !pXHitSpr->health)
                                changePos = 0;
                        }
                            
                        if (changePos && pDude->CanMove())
                        {
                            // prefer dodge
                            if (pDude->dodge.onAimMiss.Allow())
                            {
                                pDude->NewState(kCdudeStateDodge, 30 * (Random(2) + 1));
                                return;
                            }
                        }
                    }

                    if (changePos)
                    {
                        // prefer chase
                        pDude->NewState(kCdudeStateChase);
                        return;
                    }
                }
                fallthrough__;
            default:
                pDude->NewState(pWeapon->stateID);
                if (pDude->IsAttacking()) pDude->NextState(pWeapon->nextStateID);
                return;
        }
    }

    if (!pDude->CanMove())
        pDude->NextState(kCdudeStateSearch);
}

static int getTargetAng(spritetype* pSpr, XSPRITE* pXSpr)
{
    int x, y;
    if (spriRangeIsFine(pXSpr->target))
    {
        spritetype* pTarg = &sprite[pXSpr->target];
        x = pTarg->x;
        y = pTarg->y;
    }
    else
    {
        x = pXSpr->targetX;
        y = pXSpr->targetY;
    }

    return getangle(x - pSpr->x, y - pSpr->y);
}

static void turnToTarget(spritetype* pSpr, XSPRITE* pXSpr)
{
    pSpr->ang = getTargetAng(pSpr, pXSpr);
    pXSpr->goalAng = pSpr->ang;
}

static void moveTurn(spritetype* pSpr, XSPRITE* pXSpr)
{
    CUSTOMDUDE* pDude = cdudeGet(pSpr);
    int nVel = pDude->velocity.GetBase(pDude->posture, kParVelocityTurn);
    int nAng = ((kAng180 + pXSpr->goalAng - pSpr->ang) & kAngMask) - kAng180;
    pSpr->ang = ((pSpr->ang + ClipRange(nAng, -nVel, nVel)) & kAngMask);
}

static void moveDodge(spritetype* pSpr, XSPRITE* pXSpr)
{
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);
    moveTurn(pSpr, pXSpr);

    if (pXSpr->dodgeDir && pDude->CanMove())
    {
        int nVelDodge = pDude->GetVelocity(kParVelocityDodge);
        int nCos = Cos(pSpr->ang);                  int nSin = Sin(pSpr->ang);
        int dX = xvel[pSpr->index];                 int dY = yvel[pSpr->index];
        int t1 = dmulscale30(dX, nCos, dY, nSin);   int t2 = dmulscale30(dX, nSin, -dY, nCos);

        if (pXSpr->dodgeDir > 0)
        {
            t2 += nVelDodge;
        }
        else
        {
            t2 -= nVelDodge;
        }

        xvel[pSpr->index] = dmulscale30(t1, nCos, t2, nSin);
        yvel[pSpr->index] = dmulscale30(t1, nSin, -t2, nCos);
        
        if (pDude->IsFlying() && pDude->dodge.zDir)
        {
            int cz, fz, zt, zb, r = 4 + Random(2);
            getzsofslope(pSpr->sectnum, pSpr->x, pSpr->y, &cz, &fz);
            GetSpriteExtents(pSpr, &zt, &zb);

            if (Chance(0x8000) && klabs(fz - zb) > pDude->flight.cfDist)    zvel[pSpr->index] = +(nVelDodge * r);
            else if (klabs(zt - cz) > pDude->flight.cfDist)                 zvel[pSpr->index] = -(nVelDodge * r);
            else                                                            zvel[pSpr->index] = 0;
        }
    }
}

static void moveKnockout(spritetype* pSpr, XSPRITE*)
{
    int zv = zvel[pSpr->index];
    zvel[pSpr->index] = ClipRange(zv + mulscale16(zv, 0x3000), 0x1000, 0x40000);
}

static void moveNormalF(spritetype* pSpr, XSPRITE* pXSpr)
{
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);
    CUSTOMDUDE_FLIGHT* pFly = &pDude->flight;

    int fwdVel = pDude->GetVelocity(kParVelocityForward);
    int flyHg = pDude->GetMaxFlyHeigh(pFly->clipDist);
    int cz, fz, z, cf = pFly->cfDist;
    char stat;

    stat = clipFlying(pSpr, pXSpr);
    if (pDude->IsThinkTime())
    {
        if (!pDude->timer.fLand.Pass())
        {
            getzsofslope(pSpr->sectnum, pSpr->x, pSpr->y, &cz, &fz);
            pDude->goalZ = ClipRange(pXSpr->targetZ, cz + cf, fz - cf);
        }
        else if (pDude->IsSearching() && pDude->timer.goalZ.Pass())
        {
            getzsofslope(pSpr->sectnum, pSpr->x, pSpr->y, &cz, &fz);
            z = (pXSpr->targetZ > fz) ? fz : pXSpr->targetZ;
            pDude->goalZ = z - (flyHg >> 1) - ((pFly->absGoalZ) ? Random(flyHg>>1) : perc2val(Random(50), flyHg));
            pDude->goalZ = ClipRange(pDude->goalZ, cz + cf, fz - cf);
            pDude->timer.goalZ.Set();
        }
        else if (pDude->timer.goalZ.Pass() && (stat == 0 || !pFly->mustReach || klabs(pXSpr->targetZ - pDude->goalZ) > flyHg))
        {
            getzsofslope(pSpr->sectnum, pSpr->x, pSpr->y, &cz, &fz);
            pDude->goalZ = pXSpr->targetZ - ((pFly->absGoalZ) ? Random(flyHg) : perc2val(Random(100), flyHg));
            pDude->goalZ = ClipRange(pDude->goalZ, cz + cf, fz - cf);
            pDude->timer.goalZ.Set();
        }
    }

    if (pFly->friction)
        nnExtFixDudeDrag(pSpr, pFly->friction);

    xvel[pSpr->index] += mulscale30(Cos(pSpr->ang), fwdVel);
    yvel[pSpr->index] += mulscale30(Sin(pSpr->ang), fwdVel);
}

static void moveNormalW(spritetype* pSpr, XSPRITE* pXSpr)
{
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);
    int fwdVel, dz = 0;

    if (spriRangeIsFine(pXSpr->target))
    {
        spritetype* pTarg = &sprite[pXSpr->target];
        if (spriteIsUnderwater(pTarg, true))
            dz = (pTarg->z - pSpr->z) + (10 << Random(12));
    }
    else
    {
        dz = (pXSpr->targetZ - pSpr->z);
    }

    if (Chance(0x0500))
        dz <<= 1;

    fwdVel = pDude->GetVelocity(kParVelocityForward);
    xvel[pSpr->index] += mulscale30(Cos(pSpr->ang), fwdVel);
    yvel[pSpr->index] += mulscale30(Sin(pSpr->ang), fwdVel);
    zvel[pSpr->index] += dz;
}

static void moveNormalL(spritetype* pSpr, XSPRITE* pXSpr)
{
    UNREFERENCED_PARAMETER(pXSpr);
    
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);
    int fwdVel;

    fwdVel = pDude->GetVelocity(kParVelocityForward);
    xvel[pSpr->index] += mulscale30(Cos(pSpr->ang), fwdVel);
    yvel[pSpr->index] += mulscale30(Sin(pSpr->ang), fwdVel);
}

static void moveNormal(spritetype* pSpr, XSPRITE* pXSpr)
{
    UNREFERENCED_PARAMETER(pXSpr);
    
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);
    int nAng = ((kAng180 + pXSpr->goalAng - pSpr->ang) & kAngMask) - kAng180;

    moveTurn(pSpr, pXSpr);

    // don't move forward if trying to turn around
    if (klabs(nAng) > pDude->turnAng)
    {
        if (pDude->stopMoveOnTurn)
            moveForwardStop(pSpr, pXSpr);

        return;
    }

    if (pDude->CanMove())
    {
        if (pDude->IsUnderwater())  moveNormalW(pSpr, pXSpr);
        else if (pDude->IsFlying()) moveNormalF(pSpr, pXSpr);
        else                        moveNormalL(pSpr, pXSpr);
    }
}

static void enterDodgeFly(spritetype* pSpr, XSPRITE*)
{
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);
    pDude->dodge.zDir = Chance(0x8000) ? 1 : 0;
}

static void enterAttack(spritetype* pSpr, XSPRITE* pXSpr)
{
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);
    CUSTOMDUDE_WEAPON* pWeap = pDude->pWeapon;
    SEQINST* pInst;

    int tDist, bDist, flyHg, nVel, zt, zb, fz;
    int nTime;

    if (pXSpr->target >= 0)
        aiSetTarget(pXSpr, pXSpr->target);

    if (pWeap)
    {
        if (pWeap->locked && !pXSpr->locked)
        {
            if ((nTime = pXSpr->stateTimer) <= 0)
            {
                if ((pInst = GetInstance(OBJ_SPRITE, pSpr->extra)) != NULL && pInst->pSequence)
                    nTime = pInst->pSequence->ticksPerFrame * pInst->pSequence->nFrames;
            }

            if (nTime > 0) // a cheap way to get limited invulnerability time
                pXSpr->locked = 1, evPost(pSpr->index, OBJ_SPRITE, nTime, (COMMAND_ID)kCmdUnlock, kCauserGame);
        }
        
        pWeap->attackSound.Play(pSpr);
        
        if (pWeap->inertia == 0)
        {
            moveForwardStop(pSpr, pXSpr);
            if (pDude->IsFlying())
                zvel[pSpr->index] = 0;
        }
        else if (pDude->IsFlying())
        {
            GetSpriteExtents(pSpr, &zt, &zb);
            flyHg = pDude->GetMaxFlyHeigh(pDude->flight.clipDist);
            nVel = perc2val(10, pDude->GetVelocity(kParVelocityZ));
            fz = getflorzofslope(pSpr->sectnum, pSpr->x, pSpr->y);
            tDist = klabs(zt - fz - flyHg);
            bDist = klabs(fz - zb);

            if (Chance(0x8000) && bDist > pDude->flight.cfDist)  zvel[pSpr->index] = nVel;
            else if (tDist > pDude->flight.cfDist)               zvel[pSpr->index] = -nVel;
            else                                                 zvel[pSpr->index] = 0;
        }
    }
}

static void enterStartKnock(spritetype* pSpr, XSPRITE* pXSpr)
{
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);
    CUSTOMDUDE_LASTSTAND* pStand;
    int nSeq;

    pDude->StatusSet(kCdudeStatusKnocked);
    
    if (pDude->IsLastStand())
    {
        pXSpr->locked = 1;
        pStand = &pDude->lStand;
        if ((nSeq = pStand->animid[0][pDude->posture]) > 0)
            seqSpawn(nSeq, OBJ_SPRITE, pSpr->extra, pXSpr->aiState->funcId); // override
    }

    if (pDude->IsFlying())
    {
        pSpr->flags |= (kPhysGravity | kPhysFalling);
    }
}

static void enterKnock(spritetype* pSpr, XSPRITE* pXSpr)
{
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);
    CUSTOMDUDE_LASTSTAND* pStand;
    int nSeq;

    if (pDude->IsLastStand())
    {
        pXSpr->locked = 0;
        pStand = &pDude->lStand;
        if ((nSeq = pStand->animid[1][pDude->posture]) > 0)
            seqSpawn(nSeq, OBJ_SPRITE, pSpr->extra, pXSpr->aiState->funcId);  // override
    }
}

static void exitKnock(spritetype* pSpr, XSPRITE* pXSpr)
{
    CUSTOMDUDE_LASTSTAND* pStand; CUSTOMDUDE_LASTSTAND::WAKE* pWake;
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);
    int nSeq;

    pDude->StatusRem(kCdudeStatusKnocked);

    if (pDude->IsFlying())
    {
        pSpr->flags &= ~(kPhysGravity | kPhysFalling);
        if (zvel[pSpr->index] == 0)
            zvel[pSpr->index] = -0x100;
    }

    if (pDude->IsLastStand())
    {
        pStand = &pDude->lStand; pWake = &pStand->wake;

        if (Chance(pWake->chance))
        {
            if (pWake->health)
                pXSpr->health = ClipHigh(perc2val(pWake->health, nnExtGetStartHealth(pSpr)), 65535);
            
            if ((nSeq = pStand->animid[2][pDude->posture]) > 0)
                seqSpawn(nSeq, OBJ_SPRITE, pSpr->extra, pXSpr->aiState->funcId);  // override

            pDude->PlaySound(kCdudeSndLStandWake);
        }
        else
        {
            pDude->Kill(pSpr->index, (pDude->IsUnderwater()) ? kDamageDrown : kDamageFall, pXSpr->health << 4);
        }

        pDude->StatusRem(kCdudeStatusLStand);
    }
}

static void enterSleep(spritetype* pSpr, XSPRITE* pXSpr)
{
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);
    pDude->StatusSet(kCdudeStatusSleep);
    moveForwardStop(pSpr, pXSpr);
    resetTarget(pSpr, pXSpr);

    // reduce distances while sleeping
    pDude->seeDist      = pDude->sleepDist;
    pDude->hearDist     = pDude->sleepDist>>1;
    pDude->periphery    = kAng360;
}

static void enterWake(spritetype* pSpr, XSPRITE*)
{
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);
    CUSTOMDUDE* pModel = pDude->pTemplate;

    if (pDude->StatusTest(kCdudeStatusSleep))
    {
        pDude->StatusRem(kCdudeStatusSleep);
        
        // restore distances when awaked
        if (pModel)
        {
            pDude->seeDist      = pModel->seeDist;
            pDude->hearDist     = pModel->hearDist;
            pDude->periphery    = pModel->periphery;
        }
        else
        {
            pDude->seeDist      = pDude->pInfo->seeDist;
            pDude->hearDist     = pDude->pInfo->hearDist;
            pDude->periphery    = pDude->pInfo->periphery;
        }
    }

    pDude->PlaySound(kCdudeSndWake);
}


static void enterDying(spritetype* pSpr, XSPRITE*)
{
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);
    if (pDude->mass > 48)
        pDude->mass = ClipLow(pDude->mass >> 2, 48);
}

static void thinkDying(spritetype* pSpr, XSPRITE*)
{
    SPRITEHIT* pHit = &gSpriteHit[pSpr->extra];
    if (!pHit->florhit && spriteIsUnderwater(pSpr, true))
        zvel[pSpr->index] = ClipLow(zvel[pSpr->index], 0x40000);
}

static void enterDeath(spritetype* pSpr, XSPRITE*)
{
    // don't let the data fields gets overwritten!
    if (!(pSpr->flags & kHitagRespawn))
        DudeToGibCallback1(pSpr->index, pSpr->extra);

    pSpr->type = kThingBloodChunks;
    actPostSprite(pSpr->index, kStatThing);
}

static void enterMorph(spritetype* pSpr, XSPRITE* pXSpr)
{
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);
    if (!pDude->IsMorphing())
    {
        pDude->PlaySound(kCdudeSndTransforming);
        pDude->StatusSet(kCdudeStatusMorph); // set morph status
        pXSpr->locked = 1; // lock it while morphing

        pSpr->flags &= ~kPhysMove;
        moveForwardStop(pSpr, pXSpr);
        if (pXSpr->aiState->seqId <= 0)
            seqKill(OBJ_SPRITE, pSpr->extra);
    }
}

static void thinkMorph(spritetype* pSpr, XSPRITE* pXSpr)
{
    int nTarget; char triggerOn, triggerOff;
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);
    spritetype* pNext = NULL; XSPRITE* pXNext = NULL;
    int nextDude = pDude->nextDude;

    if (pDude->SeqPlaying())
    {
        moveForwardStop(pSpr, pXSpr);
        return;
    }
    
    pDude->ClearEffectCallbacks();
    pDude->StatusRem(kCdudeStatusMorph);    // clear morph status
    pXSpr->burnSource = -1;
    pXSpr->burnTime = 0;
    pXSpr->locked = 0;
    pXSpr->scale = 0;

    if (rngok(nextDude, 0, kMaxSprites))
    {
        // classic morphing to already inserted sprite by TX ID
        pNext  = &sprite[nextDude]; pXNext = &xsprite[pNext->extra];

        pXSpr->key = pXSpr->dropMsg = 0;

        // save incarnation's going on and off options
        triggerOn = pXNext->triggerOn, triggerOff = pXNext->triggerOff;

        // then remove it from incarnation so it won't send the commands
        pXNext->triggerOn = pXNext->triggerOff = 0;

        // trigger dude death before morphing
        trTriggerSprite(pSpr->index, pXSpr, kCmdOff, pSpr->index);

        pSpr->type = pSpr->inittype = pNext->type;
        pSpr->flags = pNext->flags;
        pSpr->pal = pNext->pal;
        pSpr->shade = pNext->shade;
        pSpr->clipdist = pNext->clipdist;
        pSpr->xrepeat = pNext->xrepeat;
        pSpr->yrepeat = pNext->yrepeat;

        pXSpr->txID = pXNext->txID;
        pXSpr->command = pXNext->command;
        pXSpr->triggerOn = triggerOn;
        pXSpr->triggerOff = triggerOff;
        pXSpr->busyTime = pXNext->busyTime;
        pXSpr->waitTime = pXNext->waitTime;

        // inherit respawn properties
        pXSpr->respawn = pXNext->respawn;
        pXSpr->respawnPending = pXNext->respawnPending;

        pXSpr->data1    = pXNext->data1;                        // for v1 this is weapon id, v2 - descriptor id
        pXSpr->data2    = pXNext->data2;                        // for v1 this is seqBase id
        pXSpr->data3    = pXSpr->sysData1 = pXNext->sysData1;   // for v1 this is soundBase id
        pXSpr->data4    = pXSpr->sysData2 = pXNext->sysData2;   // start hp

        // inherit dude flags
        pXSpr->dudeGuard = pXNext->dudeGuard;
        pXSpr->dudeDeaf = pXNext->dudeDeaf;
        pXSpr->dudeAmbush = pXNext->dudeAmbush;
        pXSpr->dudeFlag4 = pXNext->dudeFlag4;
        pXSpr->unused1 = pXNext->unused1;

        pXSpr->dropMsg = pXNext->dropMsg;
        pXSpr->key = pXNext->key;

        pXSpr->Decoupled = pXNext->Decoupled;
        pXSpr->locked = pXNext->locked;

        // set health
        pXSpr->health = nnExtDudeStartHealth(pSpr, pXSpr->data4);

        // restore values for incarnation
        pXNext->triggerOn = triggerOn;
        pXNext->triggerOff = triggerOff;
    }
    else
    {
        // trigger dude death before morphing
        if (pDude->morph.trDeath)
            trTriggerSprite(pSpr->index, pXSpr, kCmdOff, pSpr->index);

        // v2 morphing
        if (nextDude >= kMaxSprites)
        {
            // morph to another custom dude
            pXSpr->data1 = nextDude - kMaxSprites;
        }
        else if (nextDude < -1)
        {
            // morph to some vanilla dude
            pSpr->type      = klabs(nextDude) - 1;
            pSpr->clipdist  = getDudeInfo(pSpr->type)->clipdist;
            pXSpr->data1    = 0;
        }

        pSpr->inittype  = pSpr->type;
        pXSpr->health   = nnExtDudeStartHealth(pSpr, 0);
        pXSpr->data4    = pXSpr->sysData2 = 0;
        pXSpr->data2    = 0;
        pXSpr->data3    = 0;
    }

    // clear init status
    pDude->initialized = 0;

    nTarget = pXSpr->target;        // save target
    aiInitSprite(pSpr);             // re-init sprite with all new settings

    switch (pSpr->type)
    {
        case kDudePodMother:        // fake dude
        case kDudeTentacleMother:   // fake dude
            break;
        default:
            if (pXSpr->dudeFlag4) break;
            else if (spriRangeIsFine(nTarget)) aiSetTarget(pXSpr, nTarget); // try to restore target
            else aiSetTarget(pXSpr, pSpr->x, pSpr->y, pSpr->z);
            aiActivateDude(pSpr, pXSpr); // finally activate it
            break;
    }
}

static void enterSearchBurn(spritetype* pSpr, XSPRITE* pXSpr)
{
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);
    pDude->velocity.SetModID(kCdudeVelModBurn);
    pXSpr->aiState->thinkFunc = (Chance(0x8000)) ? thinkSearch : NULL;
}

void cdudeDoExplosion(CUSTOMDUDE* pDude)
{
    CUSTOMDUDE_WEAPON* pWeap = pDude->pWeapon;
    if (pWeap && pWeap->type == kCdudeWeaponKamikaze)
    {
        ARG_WEAPONSHOT argShot; int nShot;
        memset(&argShot, 0, sizeof(argShot));
        argShot.offs = &pWeap->shot.offset;

        if ((nShot = weaponShotKamikaze(pDude, pWeap, &argShot)) >= 0)
            pWeap->shotSound.Play(&sprite[nShot]);
    }
}

static char posObstructed(int x, int y, int z, int nRadius)
{
    UNREFERENCED_PARAMETER(z);
    
    int i = numsectors;
    while (--i >= 0 && !inside(x, y, i));
    if (i < 0)
        return true;

    if (nRadius == 0)
        return false;

    for (i = 0; i < kMaxSprites; i++)
    {
        spritetype* pSpr = &sprite[i];
        if ((pSpr->flags & kHitagFree) || (pSpr->flags & kHitagRespawn)) continue;
        if ((pSpr->cstat & CSTAT_SPRITE_ALIGNMENT_MASK) != CSTAT_SPRITE_ALIGNMENT_FACING)
            continue;

        if (!(pSpr->cstat & CSTAT_SPRITE_BLOCK))
        {
            if (!IsDudeSprite(pSpr) || !dudeIsAlive(pSpr))
                continue;
        }
        else
        {
            int w = tilesiz[pSpr->picnum].x;
            int h = tilesiz[pSpr->picnum].y;

            if (w <= 0 || h <= 0)
                continue;
        }

        if (approxDist(pSpr->x - x, pSpr->y - y) <= nRadius)
            return true;
    }

    return false;
}

static char clipFlying(spritetype* pSpr, XSPRITE* pXSpr)
{
    #define kCdudeMinHG 0x1000
    
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);
    int flyVel = pDude->GetVelocity(kParVelocityZ), flyHg = pDude->GetMaxFlyHeigh(0);
    int curz = pSpr->z, goalz = pDude->goalZ, &zv = zvel[pSpr->index];
    int t, zt, zb, cz, fz, zs = klabs(zv) >> 8;
    int cf = pDude->flight.cfDist;

    getzsofslope(pSpr->sectnum, pSpr->x, pSpr->y, &cz, &fz);
    GetSpriteExtents(pSpr, &zt, &zb);

    if (pDude->flight.backOnTrackAccel)
    {
        if ((curz < pXSpr->targetZ && klabs(zb - pXSpr->targetZ) > flyHg) || (curz > pXSpr->targetZ && klabs(zt - pXSpr->targetZ) > flyHg))
        {
            flyVel += perc2val(pDude->flight.backOnTrackAccel, flyVel);
        }
    }

    if (pDude->timer.fLand.Pass())
    {
        if ((zv > 0 && zb + zs >= fz - cf) || (zv < 0 && zt - zs <= cz + cf))
        {
            zv >>= 2;
            return 0;
        }
    }

    if (curz < goalz)
    {
        curz += zs; // Above max height
        if (curz < goalz)
        {
            t = mulscale20(flyVel, ClipLow(klabs(goalz-curz), kCdudeMinHG));
            zv = ClipHigh(zv + t, flyVel);
            return 1;
        }
        
        zv >>= 1;
    }
    else if (curz > goalz)
    {
        curz -= zs; // Below max height
        if (curz > goalz)
        {
            t = mulscale20(flyVel, ClipLow(klabs(goalz-curz), kCdudeMinHG));
            zv = ClipLow(zv - t, -flyVel);
            return 2;
        }
        
        zv >>= 1;
    }

    return 0;
}

void helperPounce(spritetype* pSpr, spritetype* pHSpr, int nDmgType, int nDmg, int kickPow)
{
    if (kickPow)
    {
        int nVel = ClipHigh(mulscale16(approxDist(xvel[pSpr->index], yvel[pSpr->index]), kickPow), kickPow);
        xvel[pHSpr->index] += mulscale30(nVel, Cos(pSpr->ang + Random2(kAng15)));
        yvel[pHSpr->index] += mulscale30(nVel, Sin(pSpr->ang + Random2(kAng15)));
        pHSpr->flags |= kPhysFalling;
    }

    if (nDmg)
        actDamageSprite(pSpr->index, pHSpr, (DAMAGE_TYPE)nDmgType, nDmg);
}
#endif

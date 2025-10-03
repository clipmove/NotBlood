//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 Nuke.YKT
Copyright (C) NoOne

*********************************************************************
NoOne: Custom Dude system. Includes compatibility with older versions
For full documentation visit: http://cruo.bloodgame.ru/xxsystem/cdud/v2/
*********************************************************************

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
#pragma once
#include "nnexts.h"
#include "tile.h"
#include "sfx.h"
#include "view.h"
#include "globals.h"
#include "trig.h"
#include "sound.h"

#define kCdudeFileNamePrefix        "CDUD"
#define kCdudeFileNamePrefixWild    "CDUD*"
#define kCdudeFileExt               "CDU"
#define kCdudeVer1                  1
#define kCdudeVer2                  2

#define kCdudeMaxVelocity           2345648
#define kCdudeMaxSounds             3
#define kCdudeMaxWeapons            8
#define kCdudeMaxDmgScale           32767
#define kCdudeDmgCheckDelay         1
#define kCdudeMaxDispersion         3500
#define kCdudeDefaultSeq            11520
#define kCdudeDefaultSeqF           4864
#define kCdudeV1MaxAttackDist       20000
#define kCdudeMaxDropItems          5
#define kCdudeDefaultAnimScale      256
#define kCdudeMaxEffectGroups       16
#define kCdudeMaxEffects            3
#define kCdudeFXEffectBase          0
#define kCudeFXEffectCallbackBase   512
#define kCdudeGIBEffectBase         1024
#define kCdudeStateTypeDeathPosture kAiStateOther - 1
#define kCdudeMinCFDist             0x2000
#define kCdudeLandDist              0xB00
#define kCdudeLaunchDist            0x1000
#define kCdudeCrouchDist            0x2000

#define kCdudeMinSeeDist            3000
#define kCdudeMinHearDist           (kCdudeMinSeeDist >> 1)
#define kCdudeBurningHealth         (25 << 4)
#define kCdudeFlyStartZvel          -0x22222

#define kChanceMax                  0x10000

class CUSTOMDUDE;
extern int nCdudeAppearanceCallback;
extern char gCdudeCustomCallback[];

enum enum_VALUE_TYPE {
kValNone                        = 0,
kValAny,
kValFix,
kValUfix,
kValPerc,
kValArrC,
kValArrA,
kValBool,
kValIdKeywordBase,
kValCdud                        = kValIdKeywordBase,
kValWeapon,
kValFX,
kValGIB,
kValVector,
kValProjectile,
kValThrow,
kValVdud,
kValKamikaze,
kValSpecial,
kValIdKeywordMax,
kValMax                         = kValIdKeywordMax,
};

enum enum_ERROR {
kErrInvalidValType              = 0,
kErrInvalidParam,
kErrInvalidParamPos,
kErrInvalidRange,
kErrInvalidResultC,
kErrInvalidVersion,
kErrReqParamNotFound,
kErrInvalidArrayLen1,
kErrInvalidArrayLen2,
kErrReqGroupNotFound,
kErrInvaliValuePos,
kErrInvalidValType2,
kErrMax,
};

enum enum_PAR_GROUP {
kParGroupGeneral                = 0,
kParGroupVelocity,
kParGroupAnimation,
kParGroupSound,
kParGroupTweaks,
kParGroupWeapon,
kParGroupDodge,
kParGroupRecoil,
kParGroupCrouch,
kParGroupKnockout,
kParGroupDamage,
kParGroupFXEffect,
kParGroupMovePat,
kParGroupDropItem,
kParGroupParser,
kParGroupFlyPat,
kParGroupMorph,
kParGroupSleep,
kParGroupSlaves,
kParGroupRandomness,
};

enum enum_PAR_GENERAL {
kParGeneralVersion              = 0,
kParGeneralMass,
kParGeneralMedium,
kParGeneralHealth,
kParGeneralClipdist,
kParGeneralMorphTo,
kParGeneralActiveTime,
kParGeneralSeedist,
kParGeneralPeriphery,
kParGeneralHeardist,
};


enum enum_PAR_PARSER {
kParParserWarnings              = 0,
};

enum enum_PAR_EVENT {
kParEventOnDmg                  = 0,
kParEventOnDmgNamed,
kParEventOnAimTargetWrong,
kParEvnDeath,
kParEvnDeathNamed,
};

enum enum_PAR_DAMAGE {
kParDmgSource                   = kDmgMax,
kParDmgBotTouch,
};

enum enum_PAR_DAMAGE_SOURCE {
kDmgSourceDude                  = 0x01,
kDmgSourcePlayer                = 0x02,
kDmgSourceSelf                  = 0x04,
kDmgSourceFriend                = 0x08,
kDmgSourceEnemy                 = 0x10,
kDmgSourceSlave                 = 0x20,
kDmgSourceKin                   = 0x40,
};

enum enum_PAR_VELOCITY {
kParVelocityForward             = 0,
kParVelocityTurn,
kParVelocityDodge,
kParVelocityZ,
kParVelocityMax,
};

enum enum_PAR_APPEARANCE {
kAppearSeq                      = 0,
kAppearClb,
kAppearSnd,
kAppearScale,
kAppearPic,
kAppearPal,
kAppearShade,
kAppearSize,
kAppearOffs1,
kAppearCstat,
};


enum enum_PAR_MEDIUM {
kParMediumAny                   = 0,
kParMediumLand,
kParMediumWater,
};

enum enum_PAR_WEAPON {
kParWeaponId                    = 0,
kParWeaponAkimbo,
kParWeaponDist,
kParWeaponPosture,
kParWeaponDisp,
kParWeaponAttackAng,
kParWeaponMedium,
kParWeaponAmmo,
kParWeaponPickChance,
kParWeaponAttackSetup,
kParWeaponShotSetup,
kParWeaponShotSnd,
kParWeaponShotAppearance,
kParWeaponAttackAnim,
kParWeaponDudeHealth,
kParWeaponTargetHealth,
kParWeaponSkill,
kParWeaponCooldown,
kParWeaponStyle,
kParWeaponSlope,
kParWeaponAkimboFrame,
kParWeaponHeigh,
kParWeaponData,
kParWeaponAttackSnd,
kParWeaponIsDefault,
};

enum enum_PAR_ATTACK {
kParAttackTime                  = 0,
kParAttackInterrupt,
kParAttackTurn2Target,
kParAttackNumShots,
kParAttackInertia,
kParAttackPredict,
};

enum enum_PAR_SHOT {
kParWeaponShotOffs              = 0,
kParWeaponShotVel,
kParWeaponShotSlope,
kParWeaponShotFollow,
kParWeaponShotClipdist,
kParWeaponShotImpact,
kParWeaponShotRemTimer,
};

enum enum_PAR_WEAP_STYLE {
kParWeaponStyleOffset           = 0,
kParWeaponStyleAngle,
};

enum enum_PAR_WEAP_COOLDOWN {
kParWeaponCooldownTime          = 0,
kParWeaponCooldownCount,
};

enum enum_PAR_EFFECT {
kParEffectId                    = 0,
kParEffectTimer,
kParEffectOffset,
kParEffectAppearance,
kParEffectAiState,
kParEffectAnimID,
kParEffectAnimFrame,
kParEffectAngle,
kParEffectVelocity,
kParEffectSlope,
kParEffectPosture,
kParEffectMedium,
kParEffectRemTime,
kParEffectChance,
kParEffectAllUnique,
kParEffectSrcVel,
kParEffectFx2Gib,
kParEffectHpRange,
};

enum enum_PAR_MOVE {
kParMoveFallHeight              = 0,
kParMoveTurnAng,
kParMoveStopOnTurn,
kParMoveDirTimer,
};

enum enum_PAR_DROP_ITEM {
kParDropItem                    = 0,
kParDropItemType,
kParDropItemChance,
kParDropItemSkill,
kParDropItemSprChance,
};

enum enum_PAR_GIB {
kParGibType,
kParGibSoundID,
kParGibForce,
kParGibFlags,
kParGibPhysics,
};

enum enum_PAR_TRIG_FLAGS {
kParTigNone                     = 0x00,
kParTrigVector                  = 0x01,
kParTrigTouch                   = 0x02,
kParTrigImpact                  = 0x04,
kParTrigLocked                  = 0x08,
};

enum enum_PAR_EVENT_DMG {
kParEvDmgAmount                 = 0,
kParEvDmgChance,
kParEvDmgHealth,
kParEvDmgCooldown,
kParEvDmgHitCount,
kParEvDmgCumulative,
kParEvDmgTime,
};

enum enum_PAR_RANDOMNESS {
kParRandStateTime               = 0,
kParRandVelocity,
kParRandThinkTime,
kParRandAnimScale,
kParRandWeapChance,
};

enum enum_PAR_TWEAKS {
kParTweaksThinkClock            = 0,
kParTweaksWaponSort,
};

enum enum_PAR_MORPH {
kParMorphDude                   = 0,
kParMorphSkill,
};

enum enum_CDUD_FLYTYPE {
kCdudeFlyStart                  = 0,
kCdudeFlyLand,
kCdudeFlightMax,
};

enum enum_PAR_FLIGHT {
kParFlyHeigh                    = kCdudeFlightMax,
kParFlyGoalzTime,
kParFlyFriction,
kParFlyClipHeighDist,
kParFlyBackOnTrack,
kParFlyCFDist,
kParFlyMustReachGoalz,
kParFlyRelGoalz,
};

enum enum_PAR_FLYTYPE {
kParFlyTypeDist                 = 0,
kParFlyTypeTime,
kParFlyTypeChance,
};

enum enum_PAR_SLEEP {
kParSleepSpotRadius             = 0,
};

enum enum_PAR_SLAVES {
kParSlaveNoForce                = 0,
kParSlaveOnDeathDie,
};

enum enum_PAR_KEYWORD {
kParKeywordInherit              = 0,
kParKeywordAbove,
kParKeywordBelow,
};

enum enum_CDUD_POSTURE {
kCdudePosture                   = 0,
kCdudePostureL                  = kCdudePosture,
kCdudePostureC,
kCdudePostureW,
kCdudePostureF,
kCdudePostureMax,
};

enum enum_CDUD_SOUND {
kCdudeSnd                       = 0,
kCdudeSndTargetSpot             = kCdudeSnd,
kCdudeSndGotHit,
kCdudeSndDeathNormal,
kCdudeSndBurning,
kCdudeSndDeathExplode,
kCdudeSndTargetDead,
kCdudeSndTargetChase,
kCdudeSndTransforming,
kCdudeSndWake,
kCdudeSndMax,
kCdudeSndCompatAttack1,
kCdudeSndCompatAttack2,
kCdudeSndCompatAttack3,
kParSndMultiSrc,
};

enum enum_CDUD_WEAPON_TYPE {
kCdudeWeaponNone                = 0,
kCdudeWeaponHitscan,
kCdudeWeaponMissile,
kCdudeWeaponThrow,
kCdudeWeaponSummon,
kCdudeWeaponSummonCdude,
kCdudeWeaponKamikaze,
kCdudeWeaponSpecial,
kCdudeWeaponMax,
};

enum enum_CDUD_WEAPON_TYPE_SPECIAL {
kCdudeWeaponIdSpecialBase       = 900,
kCdudeWeaponIdSpecialBeastStomp = kCdudeWeaponIdSpecialBase,
kCdudeWeaponIdSpecialRam,
kCdudeWeaponIdSpecialTeleport,
kCdudeWeaponIdSpecialMax,
};

enum enum_CDUD_AISTATE {
kCdudeStateBase                 = 0,
kCdudeStateIdle                 = kCdudeStateBase,
kCdudeStateMoveBase,
kCdudeStateSearch               = kCdudeStateMoveBase,
kCdudeStateDodge,
kCdudeStateChase,
kCdudeStateFlee,
kCdudeStateMoveMax,
kCdudeStateRecoil               = kCdudeStateMoveMax,
kCdudeStateRecoilT,
kCdudeBurnStateSearch,
kCdudeStateMorph,
kCdudeStateKnockEnter,
kCdudeStateKnock,
kCdudeStateKnockExit,
kCdudeStateSleep,
kCdudeStateWake,
kCdudeStateGenIdle,
kCdudeStateNormalMax,
kCdudeStateDeathBase            = kCdudeStateNormalMax,
kCdudeStateDeathBurn,
kCdudeStateDeathVector,
kCdudeStateDeathExplode,
kCdudeStateDeathChoke,
kCdudeStateDeathSpirit,
kCdudeStateDeathElectric,
kCdudeStateDeathMax,
kCdudeStateAttackBase           = kCdudeStateDeathMax,
kCdudeStateAttackMax            = kCdudeStateAttackBase + kCdudeMaxWeapons,
kCdudeStateMax                  = kCdudeStateAttackMax,
kCdudeStateMove,
kCdudeStateDeath,
kCdudeStateAttack,
kCdudeAnimScale,
};

enum enum_CDUD_STATUS {
kCdudeStatusNormal              = 0x00,
kCdudeStatusAwaked              = 0x01,
kCdudeStatusForceCrouch         = 0x02,
kCdudeStatusSleep               = 0x04,
kCdudeStatusMorph               = 0x08,
kCdudeStatusBurning             = 0x10,
kCdudeStatusDying               = 0x20,
kCdudeStatusRespawn             = 0x40,
kCdudeStatusKnocked             = 0x80,
kCdudeStatusFlipped             = 0x100,
};

struct PARAM
{
    unsigned int id             : 8;
    const char* text;
};

class ARG_PICK_WEAPON
{
    public:
        unsigned int angle          : 12;
        unsigned int distance       : 32;
        signed   int height         : 32;
        signed   int slope          : 32;
        unsigned int dudeHealth     : 8;
        unsigned int targHealth     : 8;
        ARG_PICK_WEAPON(spritetype* pSpr, XSPRITE* pXSpr, spritetype* pTarg, XSPRITE* pXTarg, int nDist, int nAng, int nSlope)
        {
            int zt, zb;

            distance = nDist;
            angle = nAng;
            dudeHealth = CountHealthPerc(pSpr, pXSpr);
            targHealth = CountHealthPerc(pTarg, pXTarg);

            height = 0, slope = 0;
            GetSpriteExtents(pSpr, &zt, &zb);
            if (!irngok(pTarg->z, zt, zb))
            {
                slope = nSlope;
                if (pTarg->z < zt)          height =  klabs(pTarg->z - zt);
                else if (pTarg->z > zb)     height = -klabs(pTarg->z - zb);
            }
        }

        char CountHealthPerc(spritetype* pSpr, XSPRITE* pXSpr)
        {
            int nHealth = ClipLow(nnExtGetStartHealth(pSpr), 1);
            return ClipHigh((kPercFull * pXSpr->health) / nHealth, 255);
        }
};

class CUSTOMDUDE_TIMER
{
    public:
        uint32_t time;
        uint16_t rng[2];
        uint8_t mul;

        inline void Clear(void)   { time = 0; }
        inline char Exists(void)  { return (time != 0); }
        inline int  Diff(void)    { return (int)(time - (uint32_t)gFrameClock); }
        inline char Pass(void)    { return (Diff() < 0); }
        inline void Set(int n)    { time = (uint32_t)gFrameClock + n; }
        void Set(void)
        {
            int n = rng[0];
            if (rng[1])
                n += Random(rng[1] - rng[0]);

            if (mul)
                n *= mul;

            Set(n);
        }
};

class CUSTOMDUDE_SOUND
{
    public:
        unsigned int id[kCdudeMaxSounds];
        unsigned int medium         : 3;
        unsigned int ai             : 1;
        unsigned int interruptable  : 1;
        unsigned int once           : 2;
        signed   int volume         : 11;
        int  Pick()                                         { return id[Random(kCdudeMaxSounds)]; }
        void Play(spritetype* pSpr)                         { Play(pSpr, Pick()); }
        void Play(spritetype* pSpr, int nID)
        {
            if (nID > 0)
            {
                int i, j;
                int nClock = (int)gFrameClock;
                char uwater = spriteIsUnderwater(pSpr, true);
                int nRand = Random2(80);

                if (!medium || ((medium & 0x01) && !uwater) || ((medium & 0x02) && uwater))
                {
                    DUDEEXTRA* pExtra = &gDudeExtra[pSpr->extra];
                    if (once)
                    {
                        for (i = 0; i < nBonkles; i++)
                        {
                            BONKLE* pBonk = BonkleCache[i];
                            for (j = 0; j < kCdudeMaxSounds; j++)
                            {
                                if ((pBonk->sfxId == (int)id[j])
                                    && (once != 2 || pBonk->pSndSpr == pSpr))
                                        return;
                            }
                        }
                    }

                    if (interruptable)
                    {
                        pExtra->clock = nClock;
                        Kill(pSpr);
                    }

                    if (ai)
                    {
                        if (pExtra->clock <= nClock)
                        {
                            sfxKill3DSound(pSpr, AI_SFX_PRIORITY_2, -1);
                            sfxPlay3DSoundCP(pSpr, nID, AI_SFX_PRIORITY_2, 0, 0x0, volume);
                            pExtra->sfx_priority = (AI_SFX_PRIORITY)AI_SFX_PRIORITY_2;
                            pExtra->clock = nClock + 384 + nRand;
                        }
                    }
                    else
                    {
                        sfxPlay3DSoundCP(pSpr, nID, -1, 0, 0x0, volume);
                    }
                }
            }
        }

        void Kill(spritetype* pSpr)
        {
            int i = kCdudeMaxSounds;
            while (--i >= 0)
                sfxKill3DSound(pSpr, -1, id[i]);
        }

        void KillOutside(spritetype* pSpr)
        {
            int i = nBonkles, j;
            while (--i >= 0)
            {
                BONKLE* pBonkle = BonkleCache[i];
                if (pBonkle->pSndSpr == pSpr)
                {
                    j = kCdudeMaxSounds;
                    while (--j >= 0)
                    {
                        if (pBonkle->sfxId == (signed int)id[j])
                            return;
                    }
                    
                    sfxKill3DSound(pSpr, -1, pBonkle->sfxId);
                }
            }
        }
};

class APPEARANCE
{
    public:
        CUSTOMDUDE_SOUND sound;
        unsigned short scl[2];
        unsigned int available      : 1;
        unsigned int soundAvailable : 1;
        unsigned int seq            : 20;
        unsigned int clb            : 12;
        unsigned int pic            : 16;
        unsigned int csta           : 20;
        unsigned int cstr           : 20;
        signed   int xrp            : 10;
        signed   int yrp            : 10;
        signed   int xof            : 8;
        signed   int yof            : 8;
        signed   int pal            : 10;
        signed   int shd            : 10;

        void Clear(void)
        {
            Bmemset(this, 0, sizeof(APPEARANCE));
            pal = -1;   shd = 128;
        }

        void SetScale(spritetype* pSpr, XSPRITE* pXSpr, XSPRITE* pXSrc)
        {
            if (available)
            {
                int nScale = 0;
                if (pXSrc == NULL)
                {
                    nScale = scl[0];
                    if (scl[1] > 0)
                        nScale += Random(scl[1]-scl[0]);
                }
                else
                {
                    nScale = pXSrc->scale;
                }

                nnExtSprScaleSet(pSpr, nScale);

                if (pXSpr)
                    pXSpr->scale = nScale;
            }
        }

        void Set(spritetype* pSpr, spritetype* pSrc)
        {
            if (!available)
                return;
            
            if (!xspriRangeIsFine(pSpr->extra))
                dbInsertXSprite(pSpr->index);
            
            bool xs = true, ys = true, plu = true;
            XSPRITE* pXSpr = &xsprite[pSpr->extra];
            SEQINST* pInst; int nCallback = -1;
            int nSeq = seq;

            if (pSrc == NULL)
                pSrc = pSpr;

            if (soundAvailable)
            {
                sound.KillOutside(pSpr);
                sound.Play(pSpr);
            }

            if (pic)
            {
                seqKill(OBJ_SPRITE, pSpr->extra);
                pSpr->picnum = pic;
                nSeq = -1;
            }

            if (nSeq >= 0)
            {
                pInst = GetInstance(OBJ_SPRITE, pSpr->extra);
                if (pInst && pInst->isPlaying)
                {
                    seqCanOverride(pInst->pSequence, 0, &xs, &ys, &plu);
                    nCallback = pInst->nCallbackID;
                }

                if (nSeq > 0)
                {
                    seqSpawn(nSeq, OBJ_SPRITE, pSpr->extra, clb ? -1 : nCallback);
                    pInst = GetInstance(OBJ_SPRITE, pSpr->extra);
                    if (pInst && pInst->isPlaying)
                        seqCanOverride(pInst->pSequence, 0, &xs, &ys, &plu);
                }

                if (clb && pInst)
                {
                    pXSpr->sysData4 = clb - 1;
                    pInst->nCallbackID  = nCdudeAppearanceCallback;
                }
            }

            if (shd != 128)
            {
                seqKill(OBJ_SPRITE, pSpr->extra);
                pSpr->shade = (shd == -129) ? pSrc->shade : shd;
            }

            if (pal == -129 || pal >= 0)
            {
                if (!plu)
                    seqKill(OBJ_SPRITE, pSpr->extra);

                pSpr->pal = (pal == -129) ? pSrc->pal : pal;
            }

            if (xrp == -129 || xrp > 0)
            {
                if (!xs)
                    seqKill(OBJ_SPRITE, pSpr->extra);

                pSpr->xrepeat = (xrp == -129) ? pSrc->xrepeat : xrp;
            }

            if (yrp == -129 || yrp > 0)
            {
                if (!ys)
                    seqKill(OBJ_SPRITE, pSpr->extra);

                pSpr->yrepeat = (yrp == -129) ? pSrc->yrepeat : yrp;
            }

            if (cstr)
                pSpr->cstat &= ~((uint16_t)cstr);
            
            if (csta)
                pSpr->cstat |=  ((uint16_t)csta);


            if (xof) pSpr->xoffset = xof;
            if (yof) pSpr->yoffset = yof;

            if (scl[0])
                SetScale(pSpr, pXSpr, (scl[0] == 1024 && scl[1] == scl[0] && pSrc->extra > 0) ? &xsprite[pSrc->extra] : NULL);
        }
};

class CUSTOMDUDE_WEAPON
{
    public:
        unsigned int  type                  : 4;
        unsigned int  numshots              : 6;
        unsigned int  id                    : 16;
        unsigned int  sharedId              : 4;
        unsigned int  angle                 : 12;
        unsigned int  medium                : 3;
        unsigned int  pickChance            : 20;
        unsigned int  available             : 1;
        unsigned int  posture               : 8;
        unsigned int  interruptable         : 1;
        unsigned int  turnToTarget          : 1;
        unsigned int  stateID               : 8;
        unsigned int  nextStateID           : 8;
        unsigned int  clipMask              : 32;
        unsigned int  group                 : 4;
        unsigned int  inertia               : 1;
        unsigned int  isDefault             : 1;
        signed   int  data1                 : 32;
        signed   int  data2                 : 32;
        signed   int  data3                 : 32;
        signed   int  data4                 : 32;
        unsigned int  dispersion[2];
        unsigned int  distRange[2];
        signed   int  heighRange[4];
        signed   int  slopeRange[2];
        unsigned char targHpRange[2];
        unsigned char dudeHpRange[2];
        CUSTOMDUDE_SOUND shotSound;
        CUSTOMDUDE_SOUND attackSound;
        IDLIST* pFrames;
        struct PREDICTION
        {
            unsigned int distance           : 32;
            unsigned int angle              : 12;
            unsigned int accuracy           : 32;
        }
        prediction;
        struct SHOT
        {
            signed   int velocity           : 32;
            signed   int slope              : 32;
            unsigned int targetFollow       : 12;
            unsigned int clipdist           : 8;
            unsigned int impact             : 1;
            signed   int remTime            : 14;
            APPEARANCE appearance;
            POINT3D offset;
        }
        shot;
        struct AMMO
        {
            unsigned int cur, total         : 16;
            void SetTotal(int nVal) { total = nVal; }
            void SetFull()          { Set(total); }
            void Set(int nVal)      { cur = ClipRange(nVal, 0, total); }
            void Inc(int nBy = 1)   { Set(cur + nBy); }
            void Dec(int nBy = 1)   { Set(cur - nBy); }
        }
        ammo;
        struct COOLDOWN
        {
            CUSTOMDUDE_TIMER delay;
            unsigned short useCount;
            unsigned short totalUseCount;
            unsigned short totalUseCountRng[2];
            char Check(void)
            {
                if (!delay.Pass())
                    return 2;
                
                if (totalUseCount && ++useCount < totalUseCount)
                    return 0;

                int a = totalUseCountRng[0];
                int b = totalUseCountRng[1];

                totalUseCount = a;
                if (b)
                    totalUseCount += Random(b - a);

                delay.Set();
                if (delay.Diff() > 0)
                    return 1;

                return 0;
            }
        }
        cooldown;
        struct SHOT_STYLE
        {
            unsigned int available  : 1;
            unsigned int angle      : 12;
            POINT3D offset;
        }
        style;
        void Clear()
        {
            if (pFrames)
                delete(pFrames);
            
            Bmemset(this, 0, sizeof(CUSTOMDUDE_WEAPON));
            
            angle           = kAng45;
            numshots        = 1;
            pickChance      = kChanceMax;
            stateID         = kCdudeStateAttackBase;
            turnToTarget    = true;

            distRange[1]    = 20000;
            dudeHpRange[1]  = 255;
            targHpRange[1]  = 255;
            
            heighRange[0]   = INT32_MIN;
            heighRange[3]   = INT32_MAX;

            slopeRange[0]   = INT32_MIN; 
            slopeRange[1]   = INT32_MAX;

            shot.remTime    = -1;
            shot.velocity   = INT32_MAX;
            shot.slope      = INT32_MAX;

            cooldown.delay.mul = kTicsPerFrame;
        }
        char HaveAmmmo(void)        { return (!ammo.total || ammo.cur); }
        int  GetDistance(void)      { return ClipLow(distRange[1] - distRange[0], 0); }
        int  GetNumshots(void)      { return (ammo.total) ? ClipHigh(ammo.cur, numshots) : numshots; }
        char HaveSlope(void)        { return (shot.slope != INT32_MAX); }
        char HaveVelocity(void)     { return (shot.velocity != INT32_MAX); }

};

class CUSTOMDUDE_GIB
{
    public:
        unsigned int available      : 1;
        unsigned int force          : 1;
        unsigned int trFlags        : 8;
        unsigned int physics        : 4;
        unsigned int thingType      : 8;
        unsigned int data1          : 20;
        unsigned int data2          : 20;
        unsigned int data3          : 20;
        unsigned int data4          : 20;
        void Clear()
        {
            Bmemset(this, 0, sizeof(CUSTOMDUDE_GIB));
            physics     = (kPhysMove | kPhysGravity | kPhysFalling);
            thingType   = kThingObjectExplode - kThingBase;
            force       = true;
        }

        void Setup(spritetype* pSpr)
        {
            int nStat = pSpr->statnum;
            if (pSpr->statnum != kStatThing)
            {
                if (pSpr->statnum >= kMaxStatus)
                    return;
                
                if (!force)
                    return;

                evKill(pSpr->index, OBJ_SPRITE, kCallbackRemove);
                ChangeSpriteStat(pSpr->index, kStatThing);
            }

            if (pSpr->extra <= 0)
                dbInsertXSprite(pSpr->index);

            XSPRITE* pXSpr = &xsprite[pSpr->extra];
            pSpr->type  = kThingBase + thingType;
            pSpr->flags &= ~(kPhysMove | kPhysGravity | kPhysFalling);
            pSpr->flags |= physics;
            
            if (!(pSpr->flags & kPhysMove))
                xvel[pSpr->index] = yvel[pSpr->index] = 0;

            if (nStat == kStatFX)
            {
                // scale velocity a bit
                if (zvel[pSpr->index])
                    zvel[pSpr->index] += perc2val(35, zvel[pSpr->index]);
            }

            pXSpr->health       = thingInfo[pSpr->type - kThingBase].startHealth << 4;
            pXSpr->data1        = data1;
            pXSpr->data2        = data2;
            pXSpr->data3        = data3;
            pXSpr->data4        = data4;

            pXSpr->isTriggered  = false;
            pXSpr->triggerOnce  = true;
            pXSpr->state        = 1;

            if (trFlags & kParTrigVector)
            {
                pSpr->cstat |= CSTAT_SPRITE_BLOCK_HITSCAN;
                pXSpr->Vector = true;
            }

            if (trFlags & kParTrigTouch)
            {
                pSpr->cstat |= CSTAT_SPRITE_BLOCK;
                pXSpr->Touch = true;
            }
            
            if (trFlags & kParTrigImpact)
                pXSpr->Impact = true;

            pXSpr->locked = true;
            if (!(trFlags & kParTrigLocked))
                evPost(pSpr->index, OBJ_SPRITE, 16, (COMMAND_ID)kCmdUnlock, kCauserGame);
        }
};

class CUSTOMDUDE_EFFECT
{
    public:
        unsigned short id[kCdudeMaxEffects];
        signed   int liveTime       : 32;
        signed   int velocity       : 32;
        signed   int velocitySlope  : 32;
        signed   int angle          : 16;
        unsigned int posture        : 8;
        unsigned int medium         : 3;
        unsigned int allUnique      : 1;
        unsigned int srcVelocity    : 1;
        unsigned int chance         : 20;
        unsigned char hpRange[2];
        CUSTOMDUDE_TIMER delay;
        CUSTOMDUDE_GIB spr2gib;
        APPEARANCE appearance;
        IDLIST* pAnims;
        IDLIST* pFrames;
        IDLIST* pStates;
        POINT3D offset;
        void Clear()
        {
            if (pAnims)     delete(pAnims);
            if (pFrames)    delete(pFrames);
            if (pStates)    delete(pStates);

            Bmemset(this, 0, sizeof(CUSTOMDUDE_EFFECT));
            angle       = kAng360;
            chance      = kChanceMax;
            velocity    = -1;
            srcVelocity = 1;

            hpRange[0] = 0;
            hpRange[1] = 255;

            delay.rng[0] = 1;
            delay.rng[1] = 0;
            delay.mul    = kTicsPerFrame;

            pAnims  = new IDLIST(true);
            pFrames = new IDLIST(true);
            pStates = new IDLIST(true);
        }

        char CanSpawn(spritetype* pSpr)
        {
            int nFrame = 1;
            int nACount = pAnims->Length();
            int nFCount = pFrames->Length();

            if (hpRange[1] != 255 && pSpr->extra > 0)
            {
                XSPRITE* pXSpr = &xsprite[pSpr->extra];
                int nHealth = ClipLow(nnExtGetStartHealth(pSpr), 1);
                int tHealth = ClipHigh((kPercFull * pXSpr->health) / nHealth, 255);
                if (!irngok(tHealth, hpRange[0], hpRange[1]))
                    return false;
            }

            if (nACount || nFCount)
            {
                SEQINST* pInst = GetInstance(OBJ_SPRITE, pSpr->extra);
                if (pInst)
                {
                    if (pInst->isPlaying)
                    {
                        if (nACount && !pAnims->Exists(pInst->nSeq))
                            return false;

                        nFrame = pInst->frameIndex + 1;
                    }
                    else
                        return false;
                }
                else if (nACount)
                {
                    return false;
                }

                if (nFCount)
                    return pFrames->Exists(nFrame);
            }

            return true;
        }

        void Setup(spritetype* pSrc, spritetype* pEff, char relVel)
        {
            int dx = 0, dy = 0, dz = velocitySlope;
            int nAng = ((angle != kAng360) ? (pSrc->ang + angle) : Random2(kAng360)) & kAngMask;
            int nVel = velocity;
            int rp = Random(15);

            pEff->owner = pSrc->index;

            dz   += Random2(perc2val(rp, dz));
            nAng += Random2(perc2val(rp>>1, nAng)) & kAngMask;
            pEff->ang = nAng;

            appearance.Set(pEff, pSrc);

            if (nVel >= 0)
            {
                nVel += Random2(perc2val(rp, nVel));
                dx = (Cos(nAng) >> 16);
                dy = (Sin(nAng) >> 16);
                
                if (nVel == 0)
                    relVel = false;
            }
            else
            {
                nVel = klabs(dz);
            }

            if (relVel)
            {
                nnExtScaleVelocityRel(pEff, nVel, dx, dy, dz);
            }
            else
            {
                nnExtScaleVelocity(pEff, nVel, dx, dy, dz);
            }

            if (srcVelocity)
            {
                xvel[pEff->index] += xvel[pSrc->index];
                yvel[pEff->index] += yvel[pSrc->index];
                zvel[pEff->index] += zvel[pSrc->index];
            }

            if (spr2gib.available)
                spr2gib.Setup(pEff);

            if (liveTime)
            {
                evKill(pEff->index, OBJ_SPRITE, kCallbackRemove);
                if (liveTime > 0)
                    evPost(pEff->index, OBJ_SPRITE, liveTime, kCallbackRemove);
            }
        }

        void Spawn(int nID, spritetype* pSpr)
        {
            spritetype* pFX;
            int x = pSpr->x, y = pSpr->y, z = pSpr->z;
            nnExtOffsetPos(&offset, pSpr->ang, &x, &y, &z);

            if (nID >= kCdudeGIBEffectBase)
            {
                nID -= kCdudeGIBEffectBase;
                
                IDLIST fxList(true); CGibPosition fxPos = { x, y, z };
                if (nnExtGibSprite(pSpr, &fxList, (GIBTYPE)nID, &fxPos, NULL))
                {
                    int32_t* pDb = fxList.First();
                    while (*pDb != kListEndDefault)
                    {
                        pFX = &sprite[*pDb++];
                        Setup(pSpr, pFX, true);
                    }
                }
                
                fxList.Free();
            }
            else if (nID >= kCudeFXEffectCallbackBase)
            {
                nID = gCdudeCustomCallback[nID - kCudeFXEffectCallbackBase];
                evKill(pSpr->index, OBJ_SPRITE, (CALLBACK_ID)nID);
                evPost(pSpr->index, OBJ_SPRITE, 0, (CALLBACK_ID)nID);
            }
            else
            {
                nID -= kCdudeFXEffectBase;
                if ((pFX = gFX.fxSpawn((FX_ID)nID, pSpr->sectnum, x, y, z)) != NULL)
                    Setup(pSpr, pFX, false);
            }
        }
        
        void Spawn(spritetype* pSpr)
        {
            if (Chance(chance))
            {
                if (allUnique)
                {
                    for (int i = 0; i < kCdudeMaxEffects; i++)
                        Spawn(id[i], pSpr);
                }
                else
                {
                    Spawn(Pick(), pSpr);
                }
            }
        }

        int Pick() { return id[Random(kCdudeMaxEffects)]; }
};


class  CUSTOMDUDE_DAMAGE
{
    public:
        unsigned short id[kDmgMax];
        unsigned int ignoreSources  : 8;
        unsigned int stompDamage    : 32;
        void Set(int nVal, int nFor) { id[nFor] = ClipRange(nVal, 0, kCdudeMaxDmgScale); }
        void Inc(int nVal, int nFor) { Set(id[nFor] + abs(nVal), nFor); }
        void Dec(int nVal, int nFor) { Set(id[nFor] - abs(nVal), nFor); }
};

struct CUSTOMDUDE_EVENT_DAMAGE
{
    unsigned int amount         : 20;
    unsigned int health         : 8;
    unsigned int chance         : 20;
    unsigned int hitcount       : 10;
    unsigned int cumulative     : 1;
    unsigned short statetime[2];
    CUSTOMDUDE_TIMER cooldown;
    
    char Allow(XSPRITE* pXSpr, int nAmount)
    {
        int nHealth, nChance = chance;

        if (nAmount > amount)
        {
            if (health)
            {
                nHealth = ClipLow(nnExtGetStartHealth(&sprite[pXSpr->reference]), 1);
                nHealth = ClipHigh((100 * pXSpr->health) / nHealth, 255);
                if (nHealth > health)
                    return 0;
            }
            
            if (hitcount)
            {
                if (!cooldown.Pass())
                {
                    hitcount += (5 - gGameOptions.nDifficulty);
                    nChance = ClipHigh(perc2val(hitcount, nChance), nChance);
                    return Chance(nChance);
                }

                hitcount = 1;
                cooldown.Set();
                return Chance(nChance);
            }

            if (cooldown.Pass())
            {
                cooldown.Set();
                return Chance(nChance);
            }
        }

        return 0;
    }

    int PickTime()
    {
        int n = statetime[0];
        if (statetime[1])
            n += Random(statetime[1] - n);
        
        return n;
    }
};


class  CUSTOMDUDE_RECOIL
{
    public:
        CUSTOMDUDE_EVENT_DAMAGE onDamage[kDmgMax];
};

class  CUSTOMDUDE_KNOCKOUT : public CUSTOMDUDE_RECOIL { };
class  CUSTOMDUDE_CROUCH   : public CUSTOMDUDE_RECOIL { };
class  CUSTOMDUDE_DODGE    : public CUSTOMDUDE_RECOIL
{
    public:
        signed int zDir : 2;
        struct
        {
            unsigned int chance : 20;
            char Allow(void) { return Chance(chance); }
        }
        onAimMiss;
};

class CUSTOMDUDE_MORPH
{
    public:
        signed short id[kDmgMax];
};

class  CUSTOMDUDE_FLIGHT
{
    public:
        struct TYPE
        {
            unsigned int distance[3];
            unsigned int chance         : 20;
        }
        type[kCdudeFlightMax];
        unsigned int absGoalZ           : 1;
        unsigned int mustReach          : 1;
        unsigned int maxHeight          : 32;
        signed   int cfDist             : 20;
        unsigned int friction           : 10;
        signed   int clipDist           : 32;
        signed   int backOnTrackAccel   : 12;
        void Clear()
        {
            Bmemset(this, 0, sizeof(CUSTOMDUDE_FLIGHT));
            cfDist          = kCdudeMinCFDist;
            maxHeight       = INT32_MAX;
            mustReach       = 1;
            clipDist        = 1024;
        }
};

class CUSTOMDUDE_VELOCITY
{
    public:
        unsigned int id[kParVelocityMax];
        //unsigned short mod[kCdudeStateMoveMax - kCdudeStateMoveBase];
        void Set(int nVal, int nFor) { id[nFor] = ClipRange(nVal, 0, kCdudeMaxVelocity); }
        void Inc(int nVal, int nFor) { Set(id[nFor] + abs(nVal), nFor); }
        void Dec(int nVal, int nFor) { Set(id[nFor] - abs(nVal), nFor); }
};

class CUSTOMDUDE_DROPITEM
{
    public:
        unsigned char items[kCdudeMaxDropItems][2];
        unsigned int  sprDropItemChance : 20;
        void Clear()
        {
            Bmemset(this, 0, sizeof(CUSTOMDUDE_DROPITEM));
            sprDropItemChance = kChanceMax;
        }

        int Pick(XSPRITE* pXSpr, IDLIST* pOut)
        {
            unsigned char nItem;
            unsigned char nPerc;
            int i;

            // add key
            if (pXSpr->key)
                pOut->AddIfNotExists(kItemKeyBase + (pXSpr->key - 1));

            // add item
            if (pXSpr->dropMsg && Chance(sprDropItemChance))
                pOut->AddIfNotExists(pXSpr->dropMsg);

            // add all items with 100% chance
            for (i = 0; i < kCdudeMaxDropItems; i++)
            {
                nItem = items[i][0];
                nPerc = items[i][1];
                if (nItem && nPerc >= 100)
                    pOut->AddIfNotExists(nItem);
            }

            // add item with < 100% chance
            while(--i >= 0)
            {
                nItem = items[i][0];
                nPerc = items[i][1];
                if (nItem)
                {
                    if (nPerc < 100 && Chance(perc2val(nPerc, kChanceMax)))
                    {
                        pOut->AddIfNotExists(nItem);
                        break;
                    }
                }
            }

            return pOut->Length();
        }
};

class CUSTOMDUDE_SUMMON
{
    public:
        IDLIST* list;
        unsigned int noSetTarget    : 1;
        unsigned int killOnDeath    : 1;
};

class CUSTOMDUDE
{
    public:
        unsigned int version                            : 2;
        unsigned int initialized                        : 1;
        unsigned int numEffects                         : 5;
        unsigned int numWeapons                         : 5;
        unsigned int numAvailWeapons                    : 5;
        DUDEEXTRA* pExtra; DUDEINFO* pInfo;
        spritetype* pSpr; XSPRITE*pXSpr, *pXLeech;
        CUSTOMDUDE* pTemplate;
        CUSTOMDUDE_WEAPON    weapons[kCdudeMaxWeapons];                             // the weapons it may have
        CUSTOMDUDE_WEAPON*   pWeapon;                                               // pointer to current weapon
        CUSTOMDUDE_DAMAGE    damage;                                                // damage control
        CUSTOMDUDE_VELOCITY  velocity[kCdudePostureMax];                            // velocity control
        CUSTOMDUDE_SOUND     sound[kCdudeSndMax];                                   // ai state sounds
        CUSTOMDUDE_DODGE     dodge;                                                 // dodge control
        CUSTOMDUDE_RECOIL    recoil;                                                // recoil control
        CUSTOMDUDE_KNOCKOUT  knockout;                                              // knock control
        CUSTOMDUDE_CROUCH    crouch;                                                // crouch control
        CUSTOMDUDE_MORPH     morph;                                                 // morph control
        CUSTOMDUDE_FLIGHT    flight;                                                // flight control
        CUSTOMDUDE_DROPITEM  dropItem;                                              // drop item control
        CUSTOMDUDE_EFFECT    effects[kCdudeMaxEffectGroups];                        // fx, gib effect stuff
        CUSTOMDUDE_SUMMON    slaves;                                                // summoned dudes under control of this dude
        AISTATE states[kCdudeStateMax][kCdudePostureMax];                           // includes states for deaths and weapons                                                           
        struct
        {
            CUSTOMDUDE_TIMER     fLaunch;
            CUSTOMDUDE_TIMER     fLand;
            CUSTOMDUDE_TIMER     goalZ;
            CUSTOMDUDE_TIMER     crouch;
            CUSTOMDUDE_TIMER     moveDir;
            CUSTOMDUDE_TIMER     floor;
        }
        timer;
        struct
        {
            // Yup, need to create templates
            // eventually to store
            // stuff like
            // this...
            
            unsigned int statetime                      : 8;
            unsigned int velocity                       : 8;
            unsigned int thinktime                      : 8;
            unsigned int animscale                      : 8;
            unsigned int weapchance                     : 8;
        }
        randomness;
        unsigned int medium                             : 3;                        // medium in which it can live
        unsigned int posture                            : 8;                        // current posture
        unsigned int mass                               : 20;                       // mass in KG
        unsigned int health                             : 20;                       // default health
        signed   int seeDist                            : 32;                       // current see distance
        signed   int hearDist                           : 32;                       // current hear distance
        unsigned int periphery                          : 12;                       // current periphery
        signed   int sleepDist                          : 32;                       // see / hear distance while sleeping
        signed   int height                             : 32;                       // in z relative to largest pic
        signed   int eyeHeight                          : 32;                       // in z relative to height
        signed   int fallHeight                         : 32;                       // in z
        signed   int goalZ                              : 32;                       // to where it must fly
        unsigned int turnAng                            : 12;                       // max turn delta ang
        unsigned int stopMoveOnTurn                     : 1;                        // clear velocity or not
        unsigned int thinkClock                         : 8;                        // a period with the think functs gets called
        signed   int nextDude                           : 16;                       // -1: none, <-1: vdude, >=0: ins, >=kMaxSprites: cdude
        //----------------------------------------------------------------------------------------------------
        FORCE_INLINE void PlaySound(int nState)                     { return (sound[nState].Play(pSpr)); }
        FORCE_INLINE int  GetDistToFloor(void)                      { return pXSpr->height << 8; };
        FORCE_INLINE int  GetStateSeq(int nState, int nPosture)     { return states[nState][nPosture].seqId; }
        FORCE_INLINE int  GetVelocity(int nPosture, int nVelType)   { return velocity[nPosture].id[nVelType]; }
        FORCE_INLINE int  GetVelocity(int nVelType)                 { return GetVelocity(posture, nVelType); }
        //----------------------------------------------------------------------------------------------------
        FORCE_INLINE char IsUnderwater(void)                        { return (pXSpr->medium != kMediumNormal); }
        FORCE_INLINE char IsStanding(void)                          { return (posture == kCdudePostureL); }
        FORCE_INLINE char IsCrouching(void)                         { return (posture == kCdudePostureC); }
        FORCE_INLINE char IsSwimming(void)                          { return (posture == kCdudePostureW); }
        FORCE_INLINE char IsFlying(void)                            { return (posture == kCdudePostureF); }
        FORCE_INLINE char SeqPlaying(void)                          { return (seqGetStatus(OBJ_SPRITE, pSpr->extra) >= 0); }
        FORCE_INLINE char IsAttacking(void)                         { return (pXSpr->aiState->stateType == kAiStateAttack); }
        FORCE_INLINE char IsRecoil(void)                            { return (pXSpr->aiState->stateType == kAiStateRecoil); }
        FORCE_INLINE char IsChasing(void)                           { return (pXSpr->aiState->stateType == kAiStateChase); }
        FORCE_INLINE char IsSearching(void)                         { return (pXSpr->aiState->stateType == kAiStateSearch); }
        FORCE_INLINE char IsKnockout(void)                          { return StatusTest(kCdudeStatusKnocked); }
        FORCE_INLINE char IsBurning(void)                           { return StatusTest(kCdudeStatusBurning); }
        FORCE_INLINE char IsMorphing(void)                          { return StatusTest(kCdudeStatusMorph); }
        FORCE_INLINE char IsDying(void)                             { return StatusTest(kCdudeStatusDying); }
        FORCE_INLINE char IsSleeping(void)                          { return StatusTest(kCdudeStatusSleep); }
        FORCE_INLINE char IsLeechBroken(void)                       { return (pXLeech && pXLeech->locked); }
        FORCE_INLINE char IsFlipped(void)                           { return ((pSpr->cstat & CSTAT_SPRITE_YFLIP) != 0 || (pSpr->flags & 2048) != 0); }
        FORCE_INLINE char IsThinkTime(void)                         { return ((gFrame & thinkClock) == (pSpr->index & thinkClock)); }
        // ---------------------------------------------------------------------------------------------------
        FORCE_INLINE void StatusSet(int nStatus)                    { pXSpr->sysData3 |= nStatus; }
        FORCE_INLINE void StatusRem(int nStatus)                    { pXSpr->sysData3 &= ~nStatus; }
        FORCE_INLINE char StatusTest(int nStatus)                   { return ((pXSpr->sysData3 & nStatus) != 0); }
        //----------------------------------------------------------------------------------------------------
        FORCE_INLINE char CanRecoil(void)                           { return (GetStateSeq(kCdudeStateRecoil, posture) > 0); }
        FORCE_INLINE char CanElectrocute(void)                      { return (GetStateSeq(kCdudeStateRecoilT, posture) > 0); }
        FORCE_INLINE char CanKnockout(void)                         { return (GetStateSeq(kCdudeStateKnock, posture)); }
        FORCE_INLINE char CanBurn(void)                             { return (GetStateSeq(kCdudeBurnStateSearch, posture) > 0); }
        FORCE_INLINE char CanStand(void)                            { return (GetStateSeq(kCdudeStateSearch, kCdudePostureL) > 0); }
        FORCE_INLINE char CanCrouch(void)                           { return (GetStateSeq(kCdudeStateSearch, kCdudePostureC) > 0); }
        FORCE_INLINE char CanSwim(void)                             { return (GetStateSeq(kCdudeStateSearch, kCdudePostureW) > 0); }
        FORCE_INLINE char CanFly(void)                              { return (GetStateSeq(kCdudeStateSearch, kCdudePostureF) > 0); }
        FORCE_INLINE char CanSleep(void)                            { return (!StatusTest(kCdudeStatusAwaked) && GetStateSeq(kCdudeStateSleep, posture) > 0); }
        FORCE_INLINE char CanMove(void)                             { return (GetStateSeq(kCdudeStateSearch, posture) > 0); }
        //----------------------------------------------------------------------------------------------------
        int  GetDamage(int nSource, int nDmgType);
        int  GetMaxFlyHeigh(char targClip);
        int  GetStartFlyVel(void);
        void ChangePosture(int nNewPosture);
        char IsPostureMatch(int nPosture);
        char IsMediumMatch(int nMedium);
        char IsTooTight(void);
        //----------------------------------------------------------------------------------------------------
        CUSTOMDUDE_WEAPON* PickWeapon(ARG_PICK_WEAPON* pArg);
        int  AdjustSlope(int nTarget, int zOffs);
        char AdjustSlope(int nDist, int* nSlope);
        //----------------------------------------------------------------------------------------------------
        void InitSprite(void);
        void Activate(void);
        void Process(void);
        void ProcessPosture(void);
        void ProcessEffects(void);
        void Recoil(int nStateTime = 0);
        int  Damage(int nFrom, int nDmgType, int nDmg);
        void Kill(int nFrom, int nDmgType, int nDmg);
        //----------------------------------------------------------------------------------------------------
        char CanMove(XSECTOR* pXSect, char Crusher, char Water, char Uwater, char Depth, int bottom, int floorZ);
        char FindState(AISTATE* pState, int* nStateType, int* nPosture);
        void NewState(int nStateType, int nTimeOverride = -1);
        char NewState(AISTATE* pState);
        void NextState(int nStateType, int nTimeOverride = 0);
        void NextState(AISTATE* pState, int nTimeOverride = 0);
        AISTATE* PickDeath(int nDmgType);
        void SyncState(void);
        //----------------------------------------------------------------------------------------------------
        void LeechPickup(void);
        void LeechKill(char delSpr);
        void SlavesUpdate(void);
        void SlavesKill(void);
        void DropItems(void);
        void ClearEffectCallbacks(void);
        void Clear(void);
        //----------------------------------------------------------------------------------------------------
};

class CUSTOMDUDE_SETUP
{
    protected:
        static const char* pValue;  static DICTNODE* hIni;
        static CUSTOMDUDE* pDude;   static IniFile* pIni;
        static PARAM* pGroup;       static PARAM* pParam;
        static char key[256];       static char val[256];
        static int nWarnings;       static char showWarnings;
        static int nDefaultPosture; static char sortWeapons;
        /*------------------------------------------------------------*/
        static int FindParam(const char* str, PARAM* pDb);
        static PARAM* FindParam(int nParam, PARAM* pDb);
        static int ParamLength(PARAM* pDb);
        /*-------------------------------------------------*/
        static char DescriptLoad(int nID);
        static void DescriptClose(void);
        static char DescriptGroupExist(const char* pGroupName);
        static char DescriptParamExist(const char* pGroupName, const char* pParamName);
        static int  DescriptCheck(void);
        static const char* DescriptGetValue(const char* pGroupName, const char* pParamName);
        /*------------------------------------------------------------*/
        static void Warning(const char* pFormat, ...);
        static const char* GetValType(int nType);
        static const char* GetError(int nErr);
        /*------------------------------------------------------------*/
        static void DamageSetDefault(void);
        static void DamageScaleToSkill(int nSkill);
        /*------------------------------------------------------------*/
        static void VelocitySetDefault(int nMaxVel);
        /*------------------------------------------------------------*/
        static void WeaponSoundSetDefault(CUSTOMDUDE_WEAPON* pWeapon);
        static void WeaponDispersionSetDefault(CUSTOMDUDE_WEAPON* pWeapon);
        static void WeaponRangeSet(CUSTOMDUDE_WEAPON* pWeapon, int nMin, int nMax);
        /*------------------------------------------------------------*/
        static void AnimationConvert(int baseID);
        static void AnimationFill(AISTATE* pState, int nAnim);
        static void AnimationFill(void);
        /*------------------------------------------------------------*/
        static void SoundConvert(int baseID);
        static void SoundFill(CUSTOMDUDE_SOUND* pSound, int nSnd);
        static void SoundFill(void);
        /*------------------------------------------------------------*/
        static void CountHeight(void);
        static void RandomizeDudeSettings(void);
        static void SetupSlaves(void);
        static void SetupLeech(void);
        /*------------------------------------------------------------*/
        static CUSTOMDUDE* SameDudeExist(CUSTOMDUDE* pCmp);
        static CUSTOMDUDE* DudeTemplateFind(int nVer);
        static spritetype* DudeTemplateFindEmpty(void);
        static CUSTOMDUDE* DudeTemplateCreate();
    public:
        static char FindAiState(AISTATE stateArr[][kCdudePostureMax], int arrLen, AISTATE* pNeedle, int* nType, int* nPosture);
        static inline void CopyListContents(IDLIST* pDst, IDLIST* pSrc) { for (int32_t* p=pSrc->First(); *p!=kListEndDefault; p++) pDst->Add(*p); }
        static void Setup(spritetype* pSpr, XSPRITE* pXSpr);
        static void Setup(CUSTOMDUDE* pOver = NULL);
        static void SetupFromDude(CUSTOMDUDE* pSrc);
};

class CUSTOMDUDEV1_SETUP : CUSTOMDUDE_SETUP
{
    private:
        static void DamageScaleToSurface(int nSurface);
        static void DamageScaleToWeapon(CUSTOMDUDE_WEAPON* pWeapon);
        static void WeaponMeleeSet(CUSTOMDUDE_WEAPON* pWeap);
        static void WeaponConvert(int nWeaponID);
        static void SetupIncarnation(void);
        static void SetupBasics(void);
        static void SetupDamage(void);
    public:
        static void Setup(void);
};

class CUSTOMDUDEV2_SETUP : CUSTOMDUDE_SETUP
{
    private:
        static char ParseVelocity(const char* str, CUSTOMDUDE_VELOCITY* pVelocity);
        static char ParseAppearance(const char* str, APPEARANCE* pAppear);
        static char ParseSound(const char* str, CUSTOMDUDE_SOUND* pSound);
        static char ParseAnimation(const char* str, AISTATE* pState, char asPosture);
        static char ParseRange(const char* str, int nValType, int out[2], int nBaseVal = 0);
        static int  ParseMedium(const char* str);
        static char ParseOffsets(const char* str, POINT3D* pOut);
        static char ParseShotSetup(const char* str, CUSTOMDUDE_WEAPON* pWeap);
        static char ParseAttackSetup(const char* str, CUSTOMDUDE_WEAPON* pWeap);
        static char ParseWeaponStyle(const char* str, CUSTOMDUDE_WEAPON* pWeap);
        static char ParseWeaponBasicInfo(const char* str, CUSTOMDUDE_WEAPON* pWeap);
        static char ParseWeaponPrediction(const char* str, CUSTOMDUDE_WEAPON* pWeap);
        static char ParseWeaponHeight(const char* str, CUSTOMDUDE_WEAPON* pWeap);
        static char ParseWeaponCooldown(const char* str, CUSTOMDUDE_WEAPON* pWeap);
        static char ParsePosture(const char* str);
        static char ParseOnEventDmg(const char* str, int* pOut, int nLen);
        static char ParseOnEventDmgEx(const char* str, CUSTOMDUDE_EVENT_DAMAGE* pOut);
        static char ParseDropItem(const char* str, unsigned char out[2]);
        static char ParseSkill(const char* str);
        static int  ParseKeywords(const char* str, PARAM* pDb);
        static int  ParseIDs(const char* str, int nValType, IDLIST* pOut, int nMax = 0);
        static int  ParseIDs(const char* str, int nValType, int* pOut, int nMax);
        static int  ParseEffectIDs(const char* str, const char* paramName, unsigned short* pOut, int nLen = 0);
        static int  ParseStatesToList(const char* str, IDLIST* pOut);
        static char ParseGibSetup(const char* str, CUSTOMDUDE_GIB* pOut);
        static char ParseFlyType(const char* str, CUSTOMDUDE_FLIGHT::TYPE* pOut, CUSTOMDUDE_TIMER* pTimer);
        static int  ParseDudeType(const char* str);
        static int  ParseMorphData(const char* str, int* pOut);
        static int  ParseTimer(const char* str, CUSTOMDUDE_TIMER* pTimer);

        /*-------------------------------------------------*/
        static int  CheckArray(const char* str, int nMin = 0, int nMax = 0, int nDefault = 1);
        static int  CheckValue(const char* str, int nValType, int nDefault);
        static int  CheckValue(const char* str, int nValType, int nMin, int nMax);
        static int  CheckValue(const char* str, int nValType, int nMin, int nMax, int nDefault);
        static int  CheckRange(const char* str, int nVal, int nMin, int nMax);
        static int  CheckParam(const char* str, PARAM* pDb);
        /*-------------------------------------------------*/
        static void SetupGeneral(void);
        static void SetupVelocity(void);
        static void SetupAnimation(AISTATE* pState, char asPosture);
        static void SetupAnimation(void);
        static void SetupSound(CUSTOMDUDE_SOUND* pSound);
        static void SetupEventDamage(CUSTOMDUDE_EVENT_DAMAGE* pEvn);
        static void SetupMovePattern(void);
        static void SetupFlyPattern(void);
        static void SetupSound(void);
        static void SetupDamage(void);
        static void SetupRecoil(void);
        static void SetupDodge(void);
        static void SetupKnockout(void);
        static void SetupCrouch(void);
        static void SetupWeapons(void);
        static void SetupEffect(void);
        static void SetupDropItem(void);
        static void SetupMorphing(void);
        static void SetupSleeping(void);
        static void SetupSlaves(void);
        static void SetupRandomness(void);
        static void SetupTweaks(void);
    public:
        static void Setup(void);
};

void cdudeFree();
CUSTOMDUDE* cdudeAlloc();
CUSTOMDUDE* cdudeGet(int nIndex);
FORCE_INLINE char IsCustomDude(spritetype* pSpr)        { return (pSpr->type == kDudeModernCustom); }
FORCE_INLINE CUSTOMDUDE* cdudeGet(spritetype* pSpr) { return cdudeGet(pSpr->index); };
spritetype* cdudeSpawn(XSPRITE* pXSource, spritetype* pSprite, int nDist);
void cdudeLeechOperate(spritetype* pSprite, XSPRITE* pXSprite);
void cdudeSave(LoadSave* pSave);
void cdudeLoad(LoadSave* pLoad);
#endif
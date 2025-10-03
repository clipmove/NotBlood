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
#include "aicdud.h"
#include "nnextcdud.h"
#include "globals.h"
#include "gib.h"
#include "trig.h"
#include "mmulti.h"
#include "endgame.h"
#include "sound.h"
#include "view.h"

#define IVAL2PERC(val, full) 	((val * 100) / full)
#define kStatTemplate (kMaxStatus-1)
#define kParamMax 255
#pragma pack(push, 1)
struct SEQCOMPAT
{
    unsigned char offset[3];
    signed   int nAiStateType       : 8;
};

struct SNDCOMPAT
{
    unsigned int offset             : 5;
    unsigned int range              : 3;
    unsigned int nSoundType         : 4;
};

struct WEAPINFO
{
    unsigned int type               : 4;
    const char* keyword;
    unsigned short range[2];
    unsigned int clipMask;
};
#pragma pack(pop)

static CUSTOMDUDE* gCustomDude = NULL;

static CUSTOMDUDE_SOUND gSoundTemplate[] =
{
    { { 1003, 1004, 1003 },  0x03, true,   true,  true,  0 },  // spot
    { { 1013, 1014, 1013 },  0x03, true,   true,  true,  0 },  // pain
    { { 1018, 1019, 1018 },  0x03, true,   true,  true,  0 },  // death
    { { 1031, 1032, 1031 },  0x03, true,   true,  true,  0 },  // burning state
    { { 1018, 1019, 1018 },  0x03, true,   true,  true,  0 },  // explosive death or end of burning state
    { { 4021, 4022, 4021 },  0x03, true,   true,  true,  0 },  // target of dude is dead
    { { 1005, 1006, 1005 },  0x03, true,   true,  true,  0 },  // target chase
    { { 9008, 9008, 9008 },  0x03, true,   true,  true,  0 },  // morph in another dude
    { { 745,  745,  745 },   0x03, true,   true,  true,  0 },  // wake after sleeping
};

// v1 dude animation compatibility
static SEQCOMPAT gSeqCompat[] =
{
    { { 0,  17, 13 },  kCdudeStateIdle,             },
    { { 0,  17, 13 },  kCdudeStateGenIdle,          },
    { { 9,  14, 13 },  kCdudeStateSearch,           },
    { { 9,  14, 13 },  kCdudeStateDodge,            },
    { { 9,  14, 13 },  kCdudeStateChase,            },
    { { 9,  14, 13 },  kCdudeStateFlee,             },
    { { 5,   5,  5 },  kCdudeStateRecoil,           },
    { { 4,   4,  4 },  kCdudeStateRecoilT,          },
    { { 6,   8,  8 },  kCdudeStateAttackBase,       },
    { { 7,   7,  7 },  kCdudeStateAttackBase + 1,   },
    { { 10, 10, 10 },  kCdudeStateAttackBase + 2,   },
    { { 3,   3,  3 },  kCdudeBurnStateSearch,       },
    { { 18, 18, 18 },  kCdudeStateMorph,            },
    { { 1,   1,  1 },  kCdudeStateDeathBase,        },
    { { 2,   2,  2 },  kCdudeStateDeathExplode,     },
    { { 15, 15, 15 },  kCdudeStateDeathBurn,        },
};

// v1 dude sound compatibility
static SNDCOMPAT gSndCompat[] =
{
    {0,     2,     kCdudeSndTargetSpot},      {2,     2,     kCdudeSndGotHit},
    {4,     2,     kCdudeSndDeathNormal},     {6,     2,     kCdudeSndBurning},
    {8,     2,     kCdudeSndDeathExplode},    {10,    2,     kCdudeSndTargetDead},
    {12,    2,     kCdudeSndTargetChase},     {14,    1,     kCdudeSndCompatAttack1},
    {15,    1,     kCdudeSndCompatAttack2},   {16,    1,     kCdudeSndCompatAttack3},
    {17,    1,     kCdudeSndTransforming},
};


/*************************************************************************************************/
static const char* gErrors[kErrMax] =
{
    "The value \"%s\" is not correct %s value",
    "Invalid parameter \"%s\"",
    "Invalid position of parameter \"%s\"",
    "Correct range of \"%s\" is %d - %d",
    "\"%s\" is not valid value of \"%s\"",
    "Invalid descriptor version",
    "Required parameter \"%s\" not found",
    "Invalid array length! Expected: %d. Given: %d",
    "Invalid array length! Expected: %d - %d. Given: %d",
    "Required group \"%s\" not found",
    "Invalid position of value \"%s\"",
    "The value \"%s\" is not correct value type",
};

static const char* gValTypes[kValMax] =
{
    "NONE",
    "ANY",
    "FIXED",
    "UNSIGNED FIXED",
    "PERCENT",
    "COMMON ARRAY",
    "ASSOCIATIVE ARRAY",
    "BOOLEAN",
    "CDUD",
    "WEAPON",
    "FX",
    "GIB",
    "VECTOR",
    "PROJECTILE",
    "THROW",
    "VDUD",
    "KAMIKAZE",
    "SPECIAL",
};

static WEAPINFO gCdudeWeaponInfo[] =
{
    { kCdudeWeaponNone,              gValTypes[kValNone],        { 0, 0 },                                                   0 },
    { kCdudeWeaponHitscan,           gValTypes[kValVector],      { 0, kVectorMax },                                         CLIPMASK1 },
    { kCdudeWeaponMissile,           gValTypes[kValProjectile],  { kMissileBase, kMissileMax },                             CLIPMASK0 },
    { kCdudeWeaponThrow,             gValTypes[kValThrow],       { kThingBase, kThingMax},                                  CLIPMASK0 },
    { kCdudeWeaponSummon,            gValTypes[kValVdud],        { kDudeCultistTommy, kDudeVanillaMax },                    CLIPMASK0 },
    { kCdudeWeaponSummonCdude,       gValTypes[kValCdud],        { 1, 10000 },                                              CLIPMASK0 },
    { kCdudeWeaponKamikaze,          gValTypes[kValKamikaze],    { kTrapExploder, kTrapExploder + kExplosionMax },          0 },
    { kCdudeWeaponSpecial,           gValTypes[kValSpecial],     { kCdudeWeaponIdSpecialBase, kCdudeWeaponIdSpecialMax },   CLIPMASK0 },
};

/*************************************************************************************************/
static PARAM gParGroup[] =
{
    {kParGroupGeneral,              "General"   },              {kParGroupVelocity,     "Velocity"  },
    {kParGroupAnimation,            "Animation" },              {kParGroupSound,        "Sound"     },
    {kParGroupTweaks,               "Tweaks"    },              {kParGroupWeapon,       "Weapon%d"  },
    {kParGroupDodge,                "Dodge"     },              {kParGroupRecoil,       "Recoil"    },
    {kParGroupDamage,               "Damage"    },              {kParGroupCrouch,       "Crouch"    },
    {kParGroupKnockout,             "Knockout"  },              {kParGroupFXEffect,     "FXEffect%d"},
    {kParGroupDropItem,             "DropItem"  },              {kParGroupMovePat,      "MoveSetup" },
    {kParGroupParser,               "Parser"    },              {kParGroupFlyPat,       "Flight"    },
    {kParGroupMorph,                "Morphing"  },              {kParGroupSleep,        "Sleeping"  },
    {kParGroupSlaves,               "Slaves"    },              {kParGroupRandomness,   "Randomness"},
    {kParamMax, NULL},
};

static PARAM gParamGeneral[] =
{
    {kParGeneralVersion,            "Version"    },              {kParGeneralMass,           "Mass"            },
    {kParGeneralMedium,             "Medium"     },              {kParGeneralClipdist,       "Clipdist"        },
    {kParGeneralHealth,             "Health"     },              {kParGeneralMorphTo,        "MorphTo"         }, // deprecated
    {kParGeneralActiveTime,         "SearchTime" },              {kParGeneralSeedist,        "Seedist"         },
    {kParGeneralPeriphery,          "Periphery"  },              {kParGeneralHeardist,       "Heardist"        },
    {kParamMax, NULL},
};

static PARAM gParamParser[] =
{
    {kParParserWarnings,       "ShowWarnings"},
    {kParamMax, NULL},
};

static PARAM gParamAnim[] =
{
    {kCdudeStateIdle,           "Idle"          },      {kCdudeStateSearch,         "Search"        },
    {kCdudeStateDodge,          "Dodge"         },      {kCdudeStateChase,          "Chase"         },
    {kCdudeStateFlee,           "Flee"          },      {kCdudeStateRecoil,         "RecoilDefault" },
    {kCdudeStateRecoilT,        "RecoilElectric"},      {kCdudeBurnStateSearch,     "Burning"       },
    {kCdudeStateMorph,          "Morphing"      },      {kCdudeStateKnockEnter,     "KnockoutEnter" },
    {kCdudeStateKnock,          "Knockout"      },      {kCdudeStateKnockExit,      "KnockoutExit"  },
    {kCdudeStateSleep,          "IdleSleep"     },      {kCdudeStateWake,           "IdleWake"      },
    {kCdudeStateDeathBase,      "DeathDefault"  },      {kCdudeStateDeathBurn,      "DeathBurning"  },
    {kCdudeStateDeathVector,    "DeathBullet"   },      {kCdudeStateDeathExplode,   "DeathExplode"  },
    {kCdudeStateDeathChoke,     "DeathChoke"    },      {kCdudeStateDeathSpirit,    "DeathSpirit"   },
    {kCdudeStateDeathElectric,  "DeathElectric" },      {kCdudeAnimScale,           "Scale"         },
    {kCdudeStateDeath,          "Death"         },      {kCdudeStateMove,           "Move"          },
    {kCdudeStateAttack,         "Attack"        },
    {kParamMax, NULL},
};

static PARAM gParamSounds[] =
{
    {kCdudeSndTargetSpot,           "TargetSpot"  },          {kCdudeSndTargetChase,      "TargetChase"       },
    {kCdudeSndTargetDead,           "TargetDead"  },          {kCdudeSndGotHit,           "GotHit"            },
    {kCdudeSndBurning,              "Burning"     },          {kCdudeSndDeathNormal,      "DeathDefault"      },
    {kCdudeSndDeathExplode,         "DeathBurning"},          {kCdudeSndTransforming,     "Morphing"          },
    {kCdudeSndWake,                 "IdleWake"    },          {kParSndMultiSrc,           "MultipleSources"   },
    {kParamMax, NULL},
};

static PARAM gParamWeapon[] =
{
    {kParWeaponId,                  "Type"             },          {kParWeaponDist,                "Distance"        },
    {kParWeaponDisp,                "Dispersion"       },          {kParWeaponAttackAng,           "AimAngle"        },
    {kParWeaponMedium,              "Medium"           },          {kParWeaponAmmo,                "Ammo"            },
    {kParWeaponPickChance,          "PickChance"       },          {kParWeaponShotAppearance,      "ShotAppearance"  },
    {kParWeaponShotSnd,             "ShotSound"        },          {kParWeaponAttackAnim,          "AttackAnimation" },
    {kParWeaponShotSetup,           "ShotSetup"        },          {kParWeaponAttackSetup,         "AttackSetup"     },
    {kParWeaponPosture,             "Posture"          },          {kParWeaponTargetHealth,        "TargetHealth"    },
    {kParWeaponDudeHealth,          "DudeHealth"       },          {kParWeaponAkimbo,              "Akimbo"          },
    {kParWeaponSkill,               "Skill"            },          {kParWeaponCooldown,            "Cooldown"        },
    {kParWeaponStyle,               "Style"            },          {kParWeaponSlope,               "AimSlope"        },
    {kParWeaponAkimboFrame,         "AkimboAnimFrame"  },          {kParWeaponHeigh,               "Height"          },
    {kParWeaponData,                "Data"             },          {kParWeaponAttackSnd,           "AttackSound"     },
    {kParWeaponIsDefault,           "Default"          },
    {kParamMax, NULL},
};

static PARAM gParamWeaponStyle[] =
{
    {kParWeaponStyleOffset,         "Offset"        },
    {kParWeaponStyleAngle,          "Angle"         },
    {kParamMax, NULL},
};

static PARAM gParamWeaponCooldown[] =
{
    {kParWeaponCooldownTime,        "Timer"        },
    {kParWeaponCooldownCount,       "UseCount"     },
    {kParamMax, NULL},
};

static PARAM gParamAttack[] =
{
    {kParAttackTime,                "Timer"         },
    {kParAttackInterrupt,           "Interruptable" },
    {kParAttackTurn2Target,         "TurnToTarget"  },
    {kParAttackNumShots,            "NumShots"      },
    {kParAttackInertia,             "Inertia"       },
    {kParAttackPredict,             "Prediction"    },
    {kParamMax, NULL},
};

static PARAM gWeaponShotSetup[] =
{
    {kParWeaponShotOffs,            "Offset"        },          {kParWeaponShotVel,         "Velocity"          },
    {kParWeaponShotSlope,           "Slope"         },          {kParWeaponShotFollow,      "FollowAngle"       },
    {kParWeaponShotClipdist,        "Clipdist"      },          {kParWeaponShotImpact,      "Impact"            },
    {kParWeaponShotRemTimer,        "LiveTime"      },
    {kParamMax, NULL},
};

static PARAM gParamOnEvent[] =
{
    {kParEventOnDmg,                "OnDamage"      },
    {kParEventOnDmgNamed,           "OnDamage%s"    },
    {kParEventOnAimTargetWrong,     "OnAimMiss"     },
    {kParEvnDeath,                  "OnDeath"       },
    {kParEvnDeathNamed,             "OnDeath%s"     },
    {kParamMax, NULL},
};
static PARAM* gParamDodge = gParamOnEvent;
static PARAM* gParamMorph = gParamOnEvent;

static PARAM gParamVelocity[] =
{
    {kParVelocityForward,   "Forward"    },
    {kParVelocityTurn,      "Turn"       },
    {kParVelocityDodge,     "Dodge"      },
    {kParVelocityZ,         "Vertical"   },
    {kParamMax, NULL},
};

static PARAM gParamPosture[] =
{
    {kCdudePostureL,        "Stand"  },
    {kCdudePostureC,        "Crouch" },
    {kCdudePostureW,        "Swim"   },
    {kCdudePostureF,        "Fly"   },
    {kParamMax, NULL},
};

static PARAM gParamMedium[]
{
    {kParMediumAny,         "Any"   },
    {kParMediumLand,        "Land"  },
    {kParMediumWater,       "Water" },
    {kParamMax, NULL},
};

static PARAM gParamDamage[] =
{
    {kDmgFall,              "Falling"          },              {kDmgBurn,              "Burning"          },
    {kDmgBullet,            "Bullet"           },              {kDmgExplode,           "Explode"          },
    {kDmgChoke,             "Choke"            },              {kDmgSpirit,            "Spirit"           },
    {kDmgElectric,          "Electric"         },              {kParDmgSource,         "IgnoreDamageFrom" },
    {kParDmgBotTouch,       "StompDamage"      },
    {kParamMax, NULL},
};

static PARAM gParamDamageSource[] =
{
    {kDmgSourcePlayer,      "Player" },                      {kDmgSourceDude,        "Dude" },
    {kDmgSourceSelf,        "Self"   },                      {kDmgSourceFriend,      "Friend" },
    {kDmgSourceEnemy,       "Enemy"  },                      {kDmgSourceSlave,       "Slave" },
    {kDmgSourceKin,         "Kin"    },
    {kParamMax, NULL},
};

static PARAM gParamAppearance[] =
{
    {kAppearSeq,            "Seq"   },                       {kAppearClb,        "Callback" },
    {kAppearSnd,            "Sound" },                       {kAppearScale,      "Scale"    },
    {kAppearPic,            "Tile"  },                       {kAppearShade,      "Shade"    },
    {kAppearPal,            "Pal"   },                       {kAppearSize,       "Size"     },
    {kAppearOffs1,          "Offset"},                       {kAppearCstat,      "Cstat"    },
    {kParamMax, NULL},
};

static PARAM gParamEffect[] =
{
    {kParEffectId,          "Type"          },     {kParEffectTimer,        "Delay"             },
    {kParEffectOffset,      "Offset"        },     {kParEffectAppearance,   "Appearance"        },
    {kParEffectAiState,     "OnState"       },     {kParEffectAnimFrame,    "AnimFrame"         },
    {kParEffectVelocity,    "Velocity"      },     {kParEffectAngle,        "Angle"             },
    {kParEffectSlope,       "VelocitySlope" },     {kParEffectPosture,      "Posture"           },
    {kParEffectMedium,      "Medium"        },     {kParEffectRemTime,      "LiveTime"          },
    {kParEffectChance,      "SpawnChance"   },     {kParEffectAllUnique,    "SpawnAll"          },
    {kParEffectAnimID,      "OnAnim"        },     {kParEffectSrcVel,       "AddSourceVelocity" },
    {kParEffectFx2Gib,      "GibSetup"      },     {kParEffectHpRange,      "SourceHealth"      },
    {kParamMax, NULL},
};

static PARAM gParamMorphTo[] =
{
    {kValVdud, gValTypes[kValVdud]},
    {kValCdud, gValTypes[kValCdud]},
    {kParamMax, NULL},
};

static PARAM gParamMovePat[] =
{
    {kParMoveFallHeight,    "FallHeight"},
    {kParMoveTurnAng,       "TurnAngle"},
    {kParMoveStopOnTurn,    "StopMoveOnTurn"},
    {kParMoveDirTimer,      "MoveDirTimer"},
    {kParamMax, NULL},
};

static PARAM gParamDropItem[] =
{
    {kParDropItem,              "Item%d"},                  {kParDropItemType,          "Type"},
    {kParDropItemChance,        "Chance"},                  {kParDropItemSkill,         "Skill"},
    {kParDropItemSprChance,     "SpriteDropItemChance"},
    {kParamMax, NULL},
};

static PARAM gParamGib[] =
{
    {kParGibType,               "Type"},        {kParGibForce,              "Force"},
    {kParGibSoundID,            "SoundID"},     {kParGibFlags,              "Flags"},
    {kParGibPhysics,            "Physics"},
    {kParamMax, NULL},
};

static PARAM gParamTriggerFlags[] =
{
    {kParTigNone,               "None"  },         {kParTrigVector,            "Vector"},
    {kParTrigImpact,            "Impact"},         {kParTrigTouch,             "Touch" },
    {kParTrigLocked,            "Locked"},
    {kParamMax, NULL},
};

static PARAM gParamPhysics[] =
{
    {0,                             "None"},
    {kPhysGravity | kPhysFalling,   "Gravity"},
    {kPhysMove,                     "Motion"},
    {kParamMax, NULL},
};

static PARAM gParamRandomness[] =
{
    {kParRandStateTime,       "StateTime"       },
    {kParRandVelocity,        "Velocity"        },
    {kParRandThinkTime,       "ThinkClock"      },
    {kParRandAnimScale,       "AnimationScale"  },
    {kParRandWeapChance,      "WeaponPickChance"  },
    {kParamMax, NULL},
};

static PARAM gParamEventDmg[] =
{
    {kParEvDmgAmount,               "Amount"       },
    {kParEvDmgChance,               "Chance"       },
    {kParEvDmgHealth,               "Health"       },
    {kParEvDmgCooldown,             "Cooldown"     },
    {kParEvDmgHitCount,             "CountHits"    },
    {kParEvDmgCumulative,           "Cumulative"   },
    {kParEvDmgTime,                 "StateTime"    },
    {kParamMax, NULL},
};

static PARAM gParamFlyPat[] =
{
    {kCdudeFlyStart,            "LaunchSetup"        },
    {kCdudeFlyLand,             "LandSetup"          },
    {kParFlyHeigh,              "MaxHeight"          },
    {kParFlyGoalzTime,          "SetHeightTime"      },
    {kParFlyFriction,           "DragCompensation"   },
    {kParFlyClipHeighDist,      "ClipHeightDistance" },
    {kParFlyBackOnTrack,        "BackOnTrackAccel"   },
    {kParFlyCFDist,             "MinCFHeight"        },
    {kParFlyRelGoalz,           "SetHeightInZ"       },
    {kParFlyMustReachGoalz,     "MustReachHeight"    },
    {kParamMax, NULL},
};

static PARAM gParamFlyType[] =
{
    {kParFlyTypeDist,           "Distance"},
    {kParFlyTypeTime,           "Time"},
    {kParFlyTypeChance,         "Chance"},
    {kParamMax, NULL},
};

static PARAM gParamTweaks[] =
{
    {kParTweaksThinkClock,      "ThinkClock"},
    {kParTweaksWaponSort,       "SortWeapons"},
    {kParamMax, NULL},
};

static PARAM gParamSleeping[] =
{
    {kParSleepSpotRadius,       "SpotRadius"},
    {kParamMax, NULL},
};

static PARAM gParamSlaves[] =
{
    {kParSlaveNoForce,          "NoTargetTrack"},
    {kParSlaveOnDeathDie,       "DeathOnMasterDeath"},
    {kParamMax, NULL},
};

static PARAM gParamMorphData[]
{
    {kParMorphDude,             "Dude"},
    {kParMorphSkill,            "Skill"},
    {kParamMax, NULL},
};

static PARAM gParamKeyword[] =
{
    {kParKeywordInherit,        "Inherit"},
    {kParKeywordAbove,          "Above"},
    {kParKeywordBelow,          "Below"},
    {kParamMax, NULL},
};

char gCdudeCustomCallback[] =
{
    kCallbackFXFlameLick,           // 1
    kCallbackFXFlareSpark,          // 2
    kCallbackFXFlareSparkLite,      // 3
    kCallbackFXBloodSpurt,          // 4 replace kCallbackFXZombieSpurt
    kCallbackFXBloodSpurt,          // 5
    kCallbackFXArcSpark,            // 6
    kCallbackFXDynPuff,             // 7
    kCallbackFXTeslaAlt,            // 8
    kCallbackFxPodGreenBloodSpray,  // 9
    kCallbackFXPodBloodSpray,       // 10
};

static char CheckProximity(spritetype* pSpr1, spritetype* pSpr2, int nDist)
{
    return CheckProximityPoint(pSpr1->x, pSpr1->y, pSpr1->z, pSpr2->x, pSpr2->y, pSpr2->z, nDist);
}

static char CanSee(spritetype* pSpr1, spritetype* pSpr2)
{
    return cansee(pSpr1->x, pSpr1->y, pSpr1->z, pSpr1->sectnum, pSpr2->x, pSpr2->y, pSpr2->z, pSpr2->sectnum);
}

static char isIdKeyword(const char* fullStr, const char* prefix, int* nID = NULL);
static char isNone(const char* str) { return (Bstrcasecmp(str, gValTypes[kValNone]) == 0); }
static char getArrayType(const char* str, int* nLen = NULL);
static int qsSortWeapons(CUSTOMDUDE_WEAPON* ref1, CUSTOMDUDE_WEAPON* ref2) { return ref1->pickChance - ref2->pickChance; }
static DICTNODE* helperSeqExists(int nSeq);
static DICTNODE* helperSndExists(int nSnd);
static int helperGetFirstPic(CUSTOMDUDE* pDude);
static char helperSeqTriggerExists(int nSeq);
static Seq* helperSeqLoad(int nSeq);
static Seq* helperSeqLock(int nSeq);
static void callbackSeqCustom(int, int xIndex);
int nCdudeAppearanceCallback = seqRegisterClient(callbackSeqCustom);


CUSTOMDUDE* CUSTOMDUDE_SETUP::pDude;
IniFile* CUSTOMDUDE_SETUP::pIni;
DICTNODE* CUSTOMDUDE_SETUP::hIni;

char CUSTOMDUDE_SETUP::key[256];
char CUSTOMDUDE_SETUP::val[256];
char CUSTOMDUDE_SETUP::showWarnings;
char CUSTOMDUDE_SETUP::sortWeapons;
PARAM* CUSTOMDUDE_SETUP::pGroup;
PARAM* CUSTOMDUDE_SETUP::pParam;
const char* CUSTOMDUDE_SETUP::pValue;
int CUSTOMDUDE_SETUP::nWarnings;
int CUSTOMDUDE_SETUP::nDefaultPosture;


/*************************************************************************************************/


char CUSTOMDUDE::IsMediumMatch(int nMedium)
{
    if (!nMedium)
        return true;
    
    char uwater = IsUnderwater();
    return (((nMedium & 0x01) && !uwater) || ((nMedium & 0x02) && uwater));
}

char CUSTOMDUDE::IsPostureMatch(int nPosture)
{
    if (!nPosture) return true;
    if ((nPosture & 0x01) && posture == kCdudePostureL)  return true;
    if ((nPosture & 0x02) && posture == kCdudePostureC)  return true;
    if ((nPosture & 0x04) && posture == kCdudePostureW)  return true;
    if ((nPosture & 0x08) && posture == kCdudePostureF)  return true;
    return false;
}

int CUSTOMDUDE::AdjustSlope(int nTarget, int zOffs)
{
    if (spriRangeIsFine(nTarget))
    {
        spritetype* pTarget = &sprite[nTarget];
        int dx = pTarget->x - pSpr->x; int dy = pTarget->y - pSpr->y;
        return divscale10((pTarget->z - pSpr->z) - zOffs, approxDist(dx, dy));
    }

    return 0;
}

char CUSTOMDUDE::AdjustSlope(int nDist, int* nSlope)
{
    if (spriRangeIsFine(pXSpr->target) && pWeapon)
    {
        int nStep, zTop, zBot;
        spritetype* pTarget = &sprite[pXSpr->target];
        GetSpriteExtents(pTarget, &zTop, &zBot);

        nStep = ClipLow(klabs(zTop - zBot) >> 4, 10);
        while (nDist > 0)
        {
            *nSlope = divscale10(pTarget->z - pSpr->z, nDist);
            HitScan(pSpr, pSpr->z, Cos(pSpr->ang) >> 16, Sin(pSpr->ang) >> 16, *nSlope, pWeapon->clipMask, nDist);
            if (pXSpr->target == gHitInfo.hitsprite)
                return true;

            nDist -= nStep;
        }
    }

    return false;
}

CUSTOMDUDE_WEAPON* CUSTOMDUDE::PickWeapon(ARG_PICK_WEAPON* pArg)
{
    CUSTOMDUDE_WEAPON* pDefl = NULL;
    CUSTOMDUDE_WEAPON* pRetn = NULL;
    CUSTOMDUDE_WEAPON* pWeap;
    int i;
    
    // clear available weapons counter
    numAvailWeapons = 0;

    for (i = 0; i < numWeapons; i++)
    {
        pWeap = &weapons[i];
        pWeap->available = 0;

        if (!pWeap->cooldown.delay.Pass())
            continue;

        if (pWeap->cooldown.totalUseCount)
        {
            if (pWeap->cooldown.useCount >= pWeap->cooldown.totalUseCount)
                pWeap->cooldown.useCount = 0;
        }

        if (pArg->angle >= pWeap->angle)                                                    continue;
        if (!rngok(pArg->distance, pWeap->distRange[0], pWeap->distRange[1]))               continue;
        if (!irngok(pArg->height, pWeap->heighRange[2], pWeap->heighRange[3]))
        {
            if (!irngok(pArg->height, pWeap->heighRange[0], pWeap->heighRange[1]))
                continue;
        }
        if (!pWeap->HaveAmmmo())                                                            continue;
        if (!IsMediumMatch(pWeap->medium))                                                  continue;
        if (!IsPostureMatch(pWeap->posture))                                                continue;
        if (!irngok(pArg->dudeHealth, pWeap->dudeHpRange[0], pWeap->dudeHpRange[1]))        continue;
        if (!irngok(pArg->targHealth, pWeap->targHpRange[0], pWeap->targHpRange[1]))        continue;
        if (!irngok(pArg->slope, pWeap->slopeRange[0], pWeap->slopeRange[1]))               continue;

        pWeap->available = 1; // at least available for akimbo mode
        numAvailWeapons++;

        if (!pRetn)
        {
            // can be selected as the main weapon
            if (Chance(pWeap->pickChance)) pRetn = pWeap;
            else if (!pDefl && pWeap->isDefault)
                pDefl = pWeap;
        }
    }

    return (pRetn) ? pRetn : pDefl;
}

char CUSTOMDUDE::IsTooTight(void)
{
    if (pSpr->sectnum >= 0)
    {
        int fZ, cZ, fH, cH;
        vec3_t pos = { pSpr->x, pSpr->y, pSpr->z };
        getzrange(&pos, pSpr->sectnum, &cZ, &cH, &fZ, &fH, pSpr->clipdist<<2, CLIPMASK0);
        return (klabs(fZ - cZ) - 256 < height);
    }

    return false;
}

int CUSTOMDUDE::GetDamage(int nSource, int nDmgType)
{
    int nDamage = damage.id[nDmgType];
    int nIgnore  = damage.ignoreSources;

    if (IsSleeping())
        nDamage <<= 4;

    if (nSource < 0 || !nIgnore)
        return nDamage;

    spritetype* pSrc    = &sprite[nSource];
    XSPRITE* pXSrc      = (xspriRangeIsFine(pSrc->extra)) ? &xsprite[pSrc->extra] : NULL;
    char play           = IsPlayerSprite(pSrc);
    char dude           = IsDudeSprite(pSrc);
    char self           = (pSrc->index == pSpr->index);

    if (dude && !play && !self)
    {
        if (nIgnore & kDmgSourceKin)
        {
            if (pXSrc && pSrc->type == pSpr->type && pXSrc->data1 == pXSpr->data1)
                return 0;
        }

        if (nIgnore & kDmgSourceSlave)
        {
            if (slaves.list->Exists(nSource))
                return 0;
        }

        if (nIgnore & kDmgSourceEnemy)
        {
            if (pXSpr->target == pSrc->index || (pXSrc && pXSrc->target == pSpr->index))
                return 0;
        }

        if (nIgnore & kDmgSourceFriend)
        {
            if (pXSrc && pXSrc->target == pXSpr->target)
                return 0;
        }
    }

    if ((nIgnore & kDmgSourcePlayer) && play)                          return 0;
    if ((nIgnore & kDmgSourceDude) && dude && !play && !self)          return 0;
    if ((nIgnore & kDmgSourceSelf) && self)                            return 0;
    return nDamage;
}

int CUSTOMDUDE::GetMaxFlyHeigh(char targClip)
{
    spritetype* pTarg;
    int nSHeigh, cz, fz, cz2, fz2, t;
    int percHg;

    getzsofslope(pSpr->sectnum, pSpr->x, pSpr->y, &cz, &fz);
    nSHeigh = klabs(fz - cz) - flight.cfDist;
    if (nSHeigh <= 0)
        return 0;

    percHg  = 100 - IVAL2PERC(height, nSHeigh);
    if (percHg <= 0)
        return 0;

    cz = fz - perc2val(percHg, nSHeigh);
    nSHeigh = klabs(fz - cz);

    if (targClip && spriRangeIsFine(pXSpr->target) && approxDist(pXSpr->targetX - pSpr->x, pXSpr->targetY - pSpr->y) <= flight.clipDist)
    {
        pTarg = &sprite[pXSpr->target];
        if (pTarg->sectnum != pSpr->sectnum)
        {
            getzsofslope(pTarg->sectnum, pTarg->x, pTarg->y, &cz2, &fz2);
            cz = ClipLow(cz, cz2), fz = ClipHigh(fz, fz2);

            if (cz == cz2 || fz == fz2)
            {
                t = klabs(fz - cz) - flight.cfDist;
                if (t > 0)
                {
                    percHg = 100 - IVAL2PERC(height, t);

                    if (percHg > 0)
                    {
                        cz = fz - perc2val(percHg, t);
                        nSHeigh = klabs(fz - cz);
                    }
                }
            }
        }
    }

    return ClipHigh(nSHeigh, flight.maxHeight);
}

int CUSTOMDUDE::GetStartFlyVel(void)
{
    int nHeigh = GetMaxFlyHeigh(flight.clipDist);
    int nVel = ClipLow(mulscale16(nHeigh, kCdudeFlyStartZvel), kCdudeFlyStartZvel);
    return nVel;
}

void CUSTOMDUDE::ChangePosture(int nNewPosture)
{
    if (posture == nNewPosture)
        return;

    switch (nNewPosture)
    {
        case kCdudePostureL:
        case kCdudePostureC:
            posture = nNewPosture;
            if (IsFlipped())
            {
                pSpr->flags &= ~(kPhysGravity | kPhysFalling);
                pSpr->flags |= kPhysMove;
            }
            else
            {
                pSpr->flags |= (kPhysMove | kPhysGravity | kPhysFalling);
            }
            break;
        case kCdudePostureW:
            posture = nNewPosture;
            pSpr->flags |= (kPhysMove | kPhysGravity);
            break;
        case kCdudePostureF:
            posture = nNewPosture;
            pSpr->flags &= ~(kPhysGravity | kPhysFalling);
            pSpr->flags |= kPhysMove;
            break;
        default:
            return;
    }
    
    SyncState(); // have to sync posture and animations...
}

void CUSTOMDUDE::Process(void)
{
    if (pXSpr->aiState->moveFunc)
        pXSpr->aiState->moveFunc(pSpr, pXSpr);

    if (pXSpr->aiState->thinkFunc && IsThinkTime())
        pXSpr->aiState->thinkFunc(pSpr, pXSpr);

    if (!IsMorphing() && !IsDying())
    {
        if (!IsMediumMatch(medium))
        {
            Kill(pSpr->index, (IsUnderWater(pSpr)) ? kDamageDrown : kDamageFall, pXSpr->health << 4);
            return;
        }

        if (IsFlipped())
        {
            int nVel = GetVelocity(kParVelocityZ);
            int zt, zb, cz;
            
            nnExtFixDudeDrag(pSpr, 16);
            GetSpriteExtents(pSpr, &zt, &zb);
            cz = getceilzofslope(pSpr->sectnum, pSpr->x, pSpr->y);
            if (zvel[pSpr->index] <= 0 && klabs(cz - zt) > 0)
                zvel[pSpr->index] -= nVel;
            else
                zvel[pSpr->index] = 0;
        }

        SlavesUpdate();
        if (pExtra->stats.active)
            ProcessPosture();
    }

    if (damage.stompDamage && (IsStanding() || IsCrouching()))
    {
        SPRITEHIT* pTouch = &gSpriteHit[pSpr->extra];
        
        if ((pTouch->florhit & 0xc000) == 0xc000)
        {
            spritetype* pHSpr = &sprite[pTouch->florhit & 0x3fff];
            if (dudeIsAlive(pHSpr))
                actDamageSprite(pSpr->index, pHSpr, (Chance(0x8000)) ? kDamageExplode : kDamageFall, damage.stompDamage);
        }
    }

    if (pXSpr->aiState->nextState && !pXSpr->stateTimer)
    {
        if (pXSpr->aiState->stateTicks > 0 || !SeqPlaying())
            NewState(pXSpr->aiState->nextState);
    }

    if (numEffects)
        ProcessEffects();
}

void CUSTOMDUDE::ProcessPosture(void)
{
    char tight = 0;
    
    if (IsUnderwater())
    {
        if (!IsSwimming())
        {
            ChangePosture(kCdudePostureW);

            if (pXSpr->burnTime)
            {
                evKill(pSpr->index, OBJ_SPRITE, (CALLBACK_ID)kCallbackFXFlameLick);
                pXSpr->burnTime = 0;
            }
        }
    }
    else if (IsCrouching())
    {
        if (timer.crouch.Pass())
        {
            if (!IsTooTight())
            {
                NewState(kCdudeStateSearch);
                ChangePosture(kCdudePostureL);
            }

            timer.crouch.Clear();
        }
    }
    else if (CanCrouch() && (((tight = IsTooTight()) > 0) || timer.crouch.Exists()))
    {
        if (tight)
        {
            ChangePosture(kCdudePostureC);
            if (!timer.crouch.Exists())
                timer.crouch.Set();
        }
        else
        {
            // How long should be close to the floor
            // before it changes to
            // crouch
                
            if (zvel[pSpr->index] >= -0x1000 && GetDistToFloor() <= kCdudeCrouchDist)
                ChangePosture(kCdudePostureC);
            else
                timer.crouch.Clear();
        }
    }
    else if (IsFlying())
    {
        if (CanStand() && !IsAttacking() && !IsRecoil() && timer.fLaunch.Pass())
        {
            // How long should fly close to the floor
            // before it changes to
            // land
                
            if (GetDistToFloor() <= kCdudeLandDist)
            {
                if (zvel[pSpr->index] >= 0)
                {
                    if (!timer.floor.Exists())
                        timer.floor.Set(8 + Random(8));
                }

                if (timer.floor.Exists() && timer.floor.Pass())
                {
                    ChangePosture(kCdudePostureL);
                    timer.floor.Clear();
                }
            }
        }
    }
    else if (CanFly())
    {
        if (!IsAttacking() && !IsRecoil())
        {
            // How long should fall and far from floor
            // before it changes to
            // fly
                
            if (zvel[pSpr->index] > 0x2000 && GetDistToFloor() > kCdudeLaunchDist)
            {
                if (!timer.floor.Exists())
                {
                    timer.floor.Set(12 + Random(8));
                }
                else if (timer.floor.Pass())
                {
                    zvel[pSpr->index] = GetStartFlyVel();
                    ChangePosture(kCdudePostureF);
                    timer.fLaunch.Set();
                    timer.floor.Clear();
                }
            }
            else
            {
                ChangePosture(kCdudePostureL);
            }
        }
    }
    else if (!IsStanding())
    {
        ChangePosture(kCdudePostureL);
    }
}

void CUSTOMDUDE::ProcessEffects(void)
{
    CUSTOMDUDE_EFFECT* pEff;
    spritetype* pPlay;
    IDLIST* pList;
    int nS, nP;
    int i, j;
    
    for (i = connecthead; i >= 0; i = connectpoint2[i])
    {
        pPlay = gPlayer[i].pSprite;
        if (!CheckProximity(pPlay, pSpr, 2048) && !CanSee(pSpr, pPlay))
            continue;

        for (j = 0; j < numEffects; j++)
        {
            pEff = &effects[j];
            if (!pEff->delay.Pass() || !IsPostureMatch(pEff->posture) || !IsMediumMatch(pEff->medium))
                continue;

            pList = pEff->pStates;
            if (pList->Length())
            {
                if (aiInPatrolState(pXSpr->aiState))
                {
                    if ((aiPatrolMoving(pXSpr->aiState) || aiPatrolTurning(pXSpr->aiState)) && pList->Exists(kCdudeStateMove));
                    else if (aiPatrolWaiting(pXSpr->aiState) && pList->Exists(kCdudeStateIdle));
                    else continue;
                }
                else if (!FindState(pXSpr->aiState, &nS, &nP) || !pList->Exists(nS))
                {
                    continue;
                }
            }

            if (pEff->CanSpawn(pSpr))
            {
                pEff->Spawn(pSpr);
                pEff->delay.Set();
            }
        }

        break;
    }
}

char CUSTOMDUDE::NewState(AISTATE* pState)
{
    if (!IsMorphing())
    {
        pXSpr->stateTimer = pState->stateTicks;
        pXSpr->aiState = pState;

        if (pState->seqId > 0)
        {
            seqSpawn(pState->seqId, 3, pSpr->extra, pState->funcId);
        }
        else if (pState->seqId == 0)
        {
            seqKill(OBJ_SPRITE, pSpr->extra);
        }

        if (pState->enterFunc)
            pState->enterFunc(pSpr, pXSpr);

        return true;
    }

    return false;
}

void CUSTOMDUDE::NewState(int nStateType, int nTimeOverride)
{
    AISTATE* pTmp   = &states[nStateType][posture];
    AISTATE* pState = &states[nStateType][kCdudePostureL];
    if (pTmp->seqId > 0)
        pState = pTmp;

    if (NewState(pState))
    {
        if (nTimeOverride >= 0)
            pXSpr->stateTimer = nTimeOverride;
    }
}

void CUSTOMDUDE::NextState(AISTATE* pState, int nTimeOverride)
{
    pXSpr->aiState->nextState = pState;
    if (pXSpr->aiState->nextState && nTimeOverride > 0)
        pXSpr->aiState->nextState->stateTicks = nTimeOverride;
}

void CUSTOMDUDE::NextState(int nStateType, int nTimeOverride)
{
    NextState((nStateType >= 0) ? &states[nStateType][posture] : NULL, nTimeOverride);
}

void CUSTOMDUDE::SyncState(void)
{
    int t1, t2;
    if (pXSpr->aiState && FindState(pXSpr->aiState, &t1, &t2) && t2 != posture)
    {
        t2 = pXSpr->stateTimer; // save time
        AISTATE* pState = &states[t1][posture];
            
        NewState(pState);
        if (pState->stateTicks)
            pXSpr->stateTimer = t2; // restore time
    }
}

char CUSTOMDUDE::FindState(AISTATE* pState, int* nStateType, int* nPosture)
{
    return CUSTOMDUDE_SETUP::FindAiState(states, LENGTH(states), pState, nStateType, nPosture);
}

void CUSTOMDUDE::InitSprite(void)
{
    dassert(initialized != 0);
    int zt, zb, cz, fz;
    int nSect;

    if (StatusTest(kCdudeStatusRespawn))
    {
        nSect = pSpr->sectnum; // it does not change sector on respawn
        if (FindSector(pSpr->x, pSpr->y, pSpr->z, &nSect) && nSect != pSpr->sectnum)
            ChangeSpriteSect(pSpr->index, nSect);

        StatusRem(kCdudeStatusAwaked);
        StatusRem(kCdudeStatusRespawn);
        StatusRem(kCdudeStatusBurning);
        StatusRem(kCdudeStatusDying);

        if (StatusTest(kCdudeStatusFlipped))
        {
            pSpr->cstat |= CSTAT_SPRITE_YFLIP;
            StatusRem(kCdudeStatusFlipped);
        }
    }

    pSpr->cstat |= 4096 + CSTAT_SPRITE_BLOCK_HITSCAN + CSTAT_SPRITE_BLOCK;
    pSpr->flags |= (kPhysMove | kPhysGravity | kPhysFalling);
    posture = kCdudePostureL;
    goalZ = pSpr->z;
    

    if (pXSpr->scale)
        nnExtSprScaleSet(pSpr, pXSpr->scale);

    if (IsBurning())
    {
        NewState(kCdudeBurnStateSearch);
        pXSpr->burnTime = 1200;
    }
    else
    {
        pExtra->stats.active    = 0;
        pExtra->stats.thinkTime = 0;
        if (spriteIsUnderwater(pSpr))
        {
            pXSpr->medium   = kMediumWater;
            ChangePosture(kCdudePostureW);
        }
        else
        {
            pXSpr->medium   = kMediumNormal;
            
            if (CanFly())
            {
                if (CanStand())
                {
                    GetSpriteExtents(pSpr, &zt, &zb);
                    getzsofslope(pSpr->sectnum, pSpr->x, pSpr->y, &cz, &fz);
                    if (fz - 0x1000 > zb)
                        ChangePosture(kCdudePostureF);
                }
                else
                {
                    ChangePosture(kCdudePostureF);
                }
            }
        }
        
        if (IsKnockout())       NewState(kCdudeStateKnock);
        else if (IsMorphing())  NewState(kCdudeStateMorph);
        else if (CanSleep())    NewState(kCdudeStateSleep);
        else
        {
            if (IsStanding() && (StatusTest(kCdudeStatusForceCrouch) || IsTooTight()))
            {
                if (GetStateSeq(kCdudeStateIdle, kCdudePostureC))
                {
                    ChangePosture(kCdudePostureC);
                }
                else
                {
                    StatusRem(kCdudeStatusForceCrouch);
                }
            }

            NewState(kCdudeStateIdle);
        }
    }

    if (IsFlipped())        pSpr->flags &= ~(kPhysGravity | kPhysFalling);
    else if (!IsFlying())   pSpr->flags |=  (kPhysGravity | kPhysFalling);
    clampSprite(pSpr);
}

void CUSTOMDUDE::Activate(void)
{
    if (IsDying())
        return;
    
    if (!IsBurning())
    {
        if (!IsMorphing())
        {
            if (!IsSleeping())
            {
                if (!pExtra->stats.active && StatusTest(kCdudeStatusForceCrouch) && IsCrouching())
                    ChangePosture(kCdudePostureL);

                pExtra->stats.active = 1;
                NewState(kCdudeStateChase);
            }
            else
            {
                NewState(kCdudeStateWake);
                StatusSet(kCdudeStatusAwaked); // once awaked cannot enter in sleep anymore (unless respawn)
            }
        }
    }
    else
    {
        NewState(kCdudeBurnStateSearch);
    }
}

int CUSTOMDUDE::Damage(int nFrom, int nDmgType, int nDmg)
{
    #define CUMUL_OR_CUR (pEvn->cumulative) ? nCDmg : nDmg
    
    UNREFERENCED_PARAMETER(nFrom);
    CUSTOMDUDE_EVENT_DAMAGE* pEvn;
    int nCDmg = cumulDamage[pSpr->extra];
    int nTime;

    if (IsBurning())
    {
        if (IsUnderwater())
        {
            pSpr->type = kDudeModernCustom;
            pInfo = getDudeInfo(pSpr->type);
            pXSpr->burnSource = -1;
            pXSpr->burnTime = 0;
            pXSpr->health = 1;

            NewState(kCdudeStateSearch);
        }
        else if (pXSpr->burnTime <= 0)
        {
            PlaySound(kCdudeSndBurning);
            pXSpr->burnTime = 1200;
        }
    }
    else if (nDmgType == kDamageBurn)
    {
        if (pXSpr->health <= pInfo->fleeHealth)
        {
            pXSpr->health = 0; // let the actDamageSprite to call actKillDude
            return nDmg;
        }
    }
    else if (pXSpr->health)
    {
        if (IsKnockout())
        {
            pExtra->teslaHit = 0;
            return nDmg;
        }
        else if (CanKnockout())
        {
            pEvn = &knockout.onDamage[nDmgType];
            if (pEvn->Allow(pXSpr, CUMUL_OR_CUR))
            {
                NewState(kCdudeStateKnockEnter);
                NextState(kCdudeStateKnock, pEvn->PickTime());

                PlaySound(kCdudeSndGotHit);
                pExtra->teslaHit = 0;
                return nDmg;
            }
        }

        if (nDmgType == kDmgElectric)
            pExtra->teslaHit = 1;


        if (CanRecoil() || (pExtra->teslaHit && CanElectrocute()))
        {
            pEvn = &recoil.onDamage[nDmgType];
            if (pEvn->Allow(pXSpr, CUMUL_OR_CUR))
                Recoil(pEvn->PickTime());
        }

        if ((IsStanding() && CanCrouch()) || IsCrouching())
        {
            pEvn = &crouch.onDamage[nDmgType];
            if (pEvn->Allow(pXSpr, CUMUL_OR_CUR))
                timer.crouch.Set(pEvn->PickTime()); // handled in ProcessPosture()
        }

        pEvn = &dodge.onDamage[nDmgType];
        if (CanMove() && pEvn->Allow(pXSpr, CUMUL_OR_CUR))
        {
            if ((nTime = pEvn->PickTime()) == 0 && (nTime = (30 * Random(2))) == 0)
                nTime = 90;
            
            if (IsRecoil())
            {
                NextState(kCdudeStateDodge, nTime);
            }
            else
            {
                if (Chance(0x0500))
                    PlaySound(kCdudeSndGotHit);

                NewState(kCdudeStateDodge, nTime);
            }
        }
        else if (IsRecoil())
        {
            NextState(kCdudeStateChase);
        }
    }

    pExtra->teslaHit = 0;
    return nDmg;
}

void CUSTOMDUDE::Recoil(int nStateTime)
{
    int nState = -1;
    if (!IsKnockout())
    {
        if (CanRecoil())
            nState = kCdudeStateRecoil;

        if (pExtra->teslaHit && CanElectrocute() && !IsUnderwater())
        {
            if (CanElectrocute() && !IsUnderwater())
            {
                nState = kCdudeStateRecoilT;
            }
        }

        PlaySound(kCdudeSndGotHit);
        
        if (nState >= 0)
            NewState(nState, nStateTime);
    }

    pExtra->teslaHit = 0;
}

AISTATE* CUSTOMDUDE::PickDeath(int nDmgType)
{
    AISTATE* pDeath = &states[kCdudeStateDeathBase + nDmgType][posture];
    int i, nRand;
    
    // try posture mode first
    if (pDeath->stateType == kCdudeStateTypeDeathPosture)
    {
        if (pDeath->seqId > 0)
            return pDeath;

        // use default death
        return &states[kCdudeStateDeathBase][posture];
    }

    nRand = Random(kCdudePostureMax);
    pDeath = &states[kCdudeStateDeathBase + nDmgType][nRand];
    if (pDeath->seqId > 0)
        return pDeath;

    // find first existing animation
    pDeath = states[kCdudeStateDeathBase + nDmgType];
    for (i = 0; i < kCdudePostureMax; i++)
    {
        if (pDeath[i].seqId > 0)
            return &pDeath[i];
    }

    // use default death
    pDeath = &states[kCdudeStateDeathBase][nRand];
    if (pDeath->stateType == kCdudeStateTypeDeathPosture)
        return &states[kCdudeStateDeathBase][posture]; // posture mode

    return pDeath;
}

void CUSTOMDUDE::Kill(int nFrom, int nDmgType, int nDmg)
{
    spritetype* pFrom = &sprite[nFrom];
    GAMEOPTIONS* pOpt = &gGameOptions;
    AISTATE* pDeath;
    int i;

    if (IsDying())
        return;

    for (i = 0; i < kCdudeSndMax; i++) sound[i].Kill(pSpr);

    if (nextDude != -1 || (nextDude = morph.id[nDmgType]) != -1)
    {
        // clamp hp so is not count as dead
        pXSpr->health = ClipLow(pXSpr->health, 16);
        if (!IsMorphing())
            NewState(kCdudeStateMorph);

        return;
    }

    if (nDmgType == kDamageDrown && !IsSwimming() && IsUnderWater(pSpr))
        ChangePosture(kCdudePostureW); // for DeathChoke animations access

    if (pWeapon && pWeapon->type == kCdudeWeaponKamikaze)
    {
        if (!IsAttacking())
        {
            switch (nDmgType)
            {
                case kDamageSpirit:
                    break;
                default:
                    if (!Chance(0x2000)) break;
                    fallthrough__;
                case kDamageBurn:
                case kDamageExplode:
                case kDamageBullet:
                    pXSpr->health = 0; // so it won't repeat killing
                    cdudeDoExplosion(this);
                    break;
            }
        }
    }
    else if (nDmgType == kDamageBurn)
    {
        if (!IsUnderwater())
        {
            if (CanBurn() && !IsBurning())
            {
                evKill(pSpr->index, OBJ_SPRITE, kCallbackFXFlameLick);
                sfxPlay3DSound(pSpr, 361, -1, -1);
                StatusSet(kCdudeStatusBurning);
                PlaySound(kCdudeSndBurning);

                pXSpr->health = kCdudeBurningHealth;
                NewState(kCdudeBurnStateSearch);
                damage.Set(256, kDmgBurn);
                return;
            }
        }
        else
        {
            pXSpr->burnTime = 0;
            pXSpr->burnSource = -1;
            nDmgType = kDamageDrown;
        }
    }
    else if (IsBurning())
    {
        nDmgType = kDamageBurn;
    }

    ClearEffectCallbacks();
    LeechKill(true);
    DropItems();

    if (slaves.killOnDeath)
        SlavesKill();

    if (pOpt->nGameType != kGameTypeSinglePlayer)
    {
        PLAYER* pPlayer;
        for (i = connecthead; i >= 0; i = connectpoint2[i])
        {
            pPlayer = &gPlayer[i];
            if (pPlayer->deathTime > 0 && pPlayer->fraggerId == pSpr->index)
                pPlayer->fraggerId = -1;

            if (pOpt->nGameType == kGameTypeCoop && pFrom->type == pPlayer->pSprite->type)
                pPlayer->fragCount++;
        }
    }

    trTriggerSprite(pSpr->index, pXSpr, kCmdOff, nFrom);
    pSpr->flags |= (kPhysGravity | kPhysFalling | kPhysMove);

    switch (nDmgType)
    {
        case kDamageExplode:
            PlaySound(kCdudeSndDeathExplode);
            break;
        case kDamageBurn:
            sfxPlay3DSound(pSpr, 351, -1, 0);
            break;
    }

    if (IsBurning())
    {
        PlaySound(kCdudeSndDeathExplode);
        if (Chance(0x4000))
        {
            int top, bottom;
            GetSpriteExtents(pSpr, &top, &bottom);
            CGibPosition gibPos(pSpr->x, pSpr->y, top);
            CGibVelocity gibVel(xvel[pSpr->index] >> 1, yvel[pSpr->index] >> 1, -0xccccc);
            GibSprite(pSpr, GIBTYPE_7, &gibPos, &gibVel);

            i = ClipLow(Random(3), 1);
            while (--i >= 0)
                fxSpawnBlood(pSpr, nDmg);
        }
    }
    else
    {
        PlaySound(kCdudeSndDeathNormal);
    }

    if (IsUnderwater())
        evPost(pSpr->index, OBJ_SPRITE, 0, (CALLBACK_ID)kCallbackEnemeyBubble);

    if (nDmgType == kDamageExplode)
    {
        for (i = 0; i < LENGTH(pInfo->nGibType); i++)
        {
            if (pInfo->nGibType[i] >= 0)
                GibSprite(pSpr, (GIBTYPE)pInfo->nGibType[i], NULL, NULL);
        }

        while (--i)
            fxSpawnBlood(pSpr, nDmg);
    }

    pSpr->type = kDudeModernCustom;
    gKillMgr.AddKill(pSpr);
    pXSpr->health   = 0;

    if (actCheckRespawn(pSpr))
    {
        // keep the dude statnum
        actPostSprite(pSpr->index, kStatDude);
        StatusSet(kCdudeStatusRespawn);
    }
    else
    {
        // fix double item drop
        pXSpr->dropMsg = 0;
    }

    if (pSpr->cstat & CSTAT_SPRITE_YFLIP)
        StatusSet(kCdudeStatusFlipped);

    pSpr->cstat &= ~(CSTAT_SPRITE_BLOCK | CSTAT_SPRITE_BLOCK_HITSCAN | CSTAT_SPRITE_YFLIP);
    pSpr->cstat |= CSTAT_SPRITE_INVISIBLE;
    seqKill(OBJ_SPRITE, pSpr->extra);
    pDeath = PickDeath(nDmgType);
    StatusSet(kCdudeStatusDying);
    NewState(pDeath);
}

void CUSTOMDUDE::DropItems(void)
{
    IDLIST itemList(true);
    spritetype* pDrop;
    
    if (dropItem.Pick(pXSpr, &itemList))
    {
        int32_t* pDb = itemList.First();
        while (*pDb != kListEndDefault)
        {
            pDrop = actDropObject(pSpr, *pDb++);
            if (pDrop && IsUnderwater()) // add a physics for drop items
            {
                pDrop->z = pSpr->z;
                if (!xspriRangeIsFine(pDrop->extra))
                    dbInsertXSprite(pDrop->index);

                XSPRITE* pXDrop = &xsprite[pDrop->extra];
                pXDrop->physAttr |= (kPhysGravity | kPhysFalling); // we only want it to drown/fall
                gPhysSpritesList.Add(pDrop->index);
                getSpriteMassBySize(pDrop);
            }
        }
    }
}

void CUSTOMDUDE::ClearEffectCallbacks(void)
{
    int i = LENGTH(gCdudeCustomCallback);
    while (--i >= 0)
        evKill(pSpr->index, OBJ_SPRITE, (CALLBACK_ID)gCdudeCustomCallback[i]);
}

void CUSTOMDUDE::LeechPickup(void)
{
    if (pXLeech)
    {
        spritetype* pLeech = &sprite[pXLeech->reference];
        spritetype* pFX = gFX.fxSpawn((FX_ID)52, pLeech->sectnum, pLeech->x, pLeech->y, pLeech->z, pLeech->ang);
        if (pFX)
        {
            pFX->cstat = CSTAT_SPRITE_ALIGNMENT_FACING;
            pFX->xrepeat = pFX->yrepeat = 64 + Random(50);
            pFX->pal = 6;
        }

        sfxPlay3DSoundCP(pLeech, 490, -1, 0, 60000);
        actPostSprite(pLeech->index, kStatFree);
        pLeech = NULL;
    }
}

void CUSTOMDUDE::LeechKill(char delSpr)
{
    if (pXLeech)
    {
        spritetype* pLeech = &sprite[pXLeech->reference];
        pLeech->cstat &= ~(CSTAT_SPRITE_BLOCK | CSTAT_SPRITE_BLOCK_HITSCAN);
        pLeech->cstat |= CSTAT_SPRITE_INVISIBLE;
        pLeech->flags = 0;

        // lock it, so we can detect it as killed after loading a save
        pXLeech->health = 1;
        pXLeech->locked = 1;
        pXLeech->Proximity = 0;

        if (delSpr)
            actPostSprite(pLeech->index, kStatFree);
    }

    CUSTOMDUDE_WEAPON* pWeap; // set a zero ammo, so dude cannot use it
    for (int i = 0; i < numWeapons; i++)
    {
        pWeap = &weapons[i];
        if (pWeap->id == kModernThingEnemyLifeLeech)
        {
            pWeap->ammo.SetTotal(1);
            pWeap->ammo.Set(0);
        }
    }
}

void CUSTOMDUDE::SlavesUpdate()
{
    int l, t = 0;
    IDLIST* pSlaves = slaves.list;
    int32_t* pDb;

    if ((l = pSlaves->Length()) <= 0)
        return;

    pDb = pSlaves->Last();
    while (--l >= 0 && *pDb != kListEndDefault)
    {
        spritetype* pSlave = &sprite[*pDb];
        if (IsDudeSprite(pSlave) && xsprIsFine(pSlave) && pSlave->owner == pSpr->index)
        {
            XSPRITE* pXSlave = &xsprite[pSlave->extra];
            if (pXSlave->health > 0)
            {
                if (!slaves.noSetTarget)
                {
                    if (!spriRangeIsFine(pXSpr->target))
                    {
                        if (!spriRangeIsFine(pXSlave->target))
                        {
                            // try return to master
                            aiSetTarget(pXSlave, pSpr->x, pSpr->y, pSpr->z);
                        }
                        else
                        {
                            // call master!
                            spritetype* pTarget = &sprite[pXSlave->target];
                            switch (pXSpr->aiState->stateType)
                            {
                                case kAiStateIdle:
                                    aiSetTarget(pXSpr, pTarget->x, pTarget->y, pTarget->z);
                                    Activate();
                                    break;
                            }
                        }
                    }
                    else if (pXSpr->target != pXSlave->target)
                    {
                        // set same target
                        aiSetTarget(pXSlave, pXSpr->target);
                    }
                }

                pDb--;
                continue;
            }
        }

        t++;
        pDb = pSlaves->Remove(*pDb);
    }

    if (version == kCdudeVer1 && t)
    {
        CUSTOMDUDE_WEAPON* pWeap;
        // add ammo for summon weapons
        for (l = 0; l < numWeapons; l++)
        {
            pWeap = &weapons[l];
            if (pWeap->type == kCdudeWeaponSummon)
                pWeap->ammo.Inc(t);
        }
    }
}

void CUSTOMDUDE::SlavesKill(void)
{
    IDLIST* pSlaves = slaves.list;
    for (int32_t* pDb = pSlaves->First(); *pDb != kListEndDefault; pDb++)
    {
        spritetype* pSlave = &sprite[*pDb];
        if (IsDudeSprite(pSlave) && xsprIsFine(pSlave) && pSlave->owner == pSpr->index)
        {
            XSPRITE* pXSlave = &xsprite[pSlave->extra];
            if (pXSlave->health > 0)
                actKillDude(pSpr->index, pSlave, kDamageFall, pXSlave->health << 4);
        }
    }

    delete(slaves.list);
    slaves.list = new IDLIST(true);
}

char CUSTOMDUDE::CanMove(XSECTOR* pXSect, char Crusher, char Water, char Uwater, char Depth, int bottom, int floorZ)
{
    UNREFERENCED_PARAMETER(Depth);
    sectortype* pSect;
    int zt, zb, cz;

    if (pXSpr->health)
    {
        if (IsFlipped())
        {
            pSect = &sector[pSpr->sectnum];
            if (pSect->ceilingstat & kSecCParallax)
                return false;

            GetSpriteExtents(pSpr, &zt, &zb);
            cz = getceilzofslope(pSpr->sectnum, pSpr->x, pSpr->y);
            if (klabs(zt - cz) > (int)fallHeight)
                return false;
        }
        else
        {
            if (klabs(floorZ - bottom) > (int)fallHeight)
            {
                if (!Uwater)
                    return false;
            }

            if (Water || Uwater)
            {
                if (!CanSwim() && !IsFlying())
                    return false;
            }
            else if (!IsFlying() && Crusher && pXSect && !nnExtIsImmune(pSpr, pXSect->damageType))
                return false;
        }
    }

    return true;
}

void CUSTOMDUDE::Clear()
{
    int i;

    for (i = 0; i < numEffects; i++)
    {
        if (effects[i].pAnims)  delete(effects[i].pAnims);
        if (effects[i].pStates) delete(effects[i].pStates);
        if (effects[i].pFrames) delete(effects[i].pFrames);
    }

    for (i = 0; i < numWeapons; i++)
    {
        if (weapons[i].pFrames) delete(weapons[i].pFrames);
    }

    if (slaves.list)
        delete(slaves.list);

    Bmemset(this, 0, sizeof(CUSTOMDUDE));
}

/*************************************************************************************************/
PARAM* CUSTOMDUDE_SETUP::FindParam(int nParam, PARAM* pDb)
{
    PARAM* ptr = pDb;
    while (ptr->id != kParamMax)
    {
        if (ptr->id == nParam)
            return ptr;

        ptr++;
    }

    return NULL;
}

int CUSTOMDUDE_SETUP::FindParam(const char* str, PARAM* pDb)
{
    PARAM* ptr = pDb;
    while (ptr->id != kParamMax)
    {
        if (Bstrcasecmp(str, ptr->text) == 0)
            return ptr->id;

        ptr++;
    }

    return -1;
}

int CUSTOMDUDE_SETUP::ParamLength(PARAM* pDb)
{
    int i = 0;  PARAM* pParam = pDb;
    while (pParam->id != kParamMax)
        pParam++, i++;

    return i;
}


const char* CUSTOMDUDE_SETUP::DescriptGetValue(const char* pGroupName, const char* pParamName)
{
    const char* pRetn = pIni->GetKeyString(pGroupName, pParamName, NULL);
    if (isempty(pRetn)) // zero length strings leads to NULL
        pRetn = NULL;

    return pRetn;
}

char CUSTOMDUDE_SETUP::DescriptGroupExist(const char* pGroupName)
{
    return pIni->SectionExists(pGroupName);
}

char CUSTOMDUDE_SETUP::DescriptParamExist(const char* pGroupName, const char* pParamName)
{
    return pIni->KeyExists(pGroupName, pParamName);
}


char CUSTOMDUDE_SETUP::DescriptLoad(int nID)
{   
    static char tmp[BMAX_PATH]; unsigned char* pRawIni = NULL;
    const char* fname = kCdudeFileNamePrefix;
    const char* fext = kCdudeFileExt;
    static IDLIST ignore(true);

    if (rngok(nID, 0, 10000) && !ignore.Exists(nID))
    {
        Bsprintf(tmp, "%s%d", fname, nID);
#ifdef NNEXTS_USE_RES_SYS
        if ((hIni = nnExtResFileSearch(&gSysRes, tmp, fext)) == NULL) // name not found
            hIni = gSysRes.Lookup(nID, fext); // try by ID

        if (hIni && (pRawIni = (unsigned char*)gSysRes.Load(hIni)) != NULL)
        {
            pIni = new IniFile((unsigned char*)pRawIni, gSysRes.Size(hIni));
            return true;
        }
#else
        BDIR* pDir; Bdirent* pEntry; static DICTNODE node;
        static char path[BMAX_PATH], dir[BMAX_PATH], type[16], *p;

        Bmemset(&node, 0, sizeof(node)); 
        Bmemset(path, 0, sizeof(path));
        Bmemset(dir, 0, sizeof(dir));

        if (*g_modDir != '/' && *g_modDir != '\\')
            Bsprintf(dir, "%s/", g_modDir);

        Bsprintf(path, "%s%s.%s", dir, tmp, fext);
        Bstrcpy(type, fext);

        node.type = (char*)type;
        node.name = (char*)tmp;
        
        // Search "CDUDNNNN.CDU"
        if (access(path, F_OK) >= 0)
        {
            node.flags |= DICT_EXTERNAL;
            node.path = path;
            hIni = &node;

            pIni = new IniFile(path);
            return true;
        }

        // Search "CDU" file type with ID NNNN or with name "CDUDNNNN"
        if ((hIni = gSysRes.Lookup(nID, fext)) != NULL || (hIni = gSysRes.Lookup(tmp, fext)) != NULL)
        {
            if ((pRawIni = (unsigned char*)gSysRes.Load(hIni)) != NULL)
            {
                pIni = new IniFile(pRawIni, gSysRes.Size(hIni));
                return true;
            }
        }

        // Search "CDUDNNNN_SOMETEXT.CDU"
        if ((pDir = Bopendir(dir)) != NULL)
        {
            while ((pEntry = Breaddir(pDir)) != NULL)
            {
                if ((p = Bstrrchr(pEntry->name, '.')) == NULL)                       continue;
                else if (Bstrcasecmp(fext, p+1) != 0)                                continue;
                else if ((p = Bstrchr(pEntry->name, '_')) == NULL)                   continue;
                else if (Bstrncasecmp(pEntry->name, tmp, p-pEntry->name) != 0)       continue;


                Bsprintf(path, "%s%s", dir, pEntry->name);

                node.flags |= DICT_EXTERNAL;
                node.path = path;
                hIni = &node;

                pIni = new IniFile(path);
                Bclosedir(pDir);
                return true;
            }

            Bclosedir(pDir);
        }
#endif

        ignore.Add(nID);
    }

    return false;
}

void CUSTOMDUDE_SETUP::DescriptClose(void)
{
    if (pIni)
        delete(pIni);
}

int CUSTOMDUDE_SETUP::DescriptCheck(void)
{
    int nRetn = 0; char major[16];
    pGroup = FindParam(kParGroupGeneral, gParGroup);
    pParam = FindParam(kParGeneralVersion, gParamGeneral);
    pValue = NULL;

    if (pIni)
    {
        if (!DescriptGroupExist(pGroup->text))
        {
            Warning(GetError(kErrReqGroupNotFound), pGroup->text);
            return nRetn;
        }
        
        pValue = DescriptGetValue(pGroup->text, pParam->text);
        if (pValue && rngok(Bstrlen(pValue), 0, 5))
        {
            major[0] = pValue[0]; major[1] = '\0';
            if (isdigit(major[0]))
                nRetn = atoi(major);
        }
        
        pGroup = FindParam(kParGroupParser, gParGroup);
        pParam = FindParam(kParParserWarnings, gParamParser);
        if (pGroup && pParam)
        {
            pValue = DescriptGetValue(pGroup->text, pParam->text);
            if (isbool(pValue))
                showWarnings = btoi(pValue);
        }
    }

    return nRetn;
}

void CUSTOMDUDE_SETUP::SetupSlaves()
{
    spritetype* pSpr = pDude->pSpr;
    int i;

    if (pDude->slaves.list)
        delete(pDude->slaves.list);

    pDude->slaves.list = new IDLIST(true);

    for (i = headspritestat[kStatDude]; i >= 0; i = nextspritestat[i])
    {
        spritetype* pSpr2 = &sprite[i];
        if (pSpr2->owner == pSpr->index && IsDudeSprite(pSpr2) && xsprIsFine(pSpr2))
        {
            pDude->slaves.list->Add(pSpr2->index);
        }
    }
}

void CUSTOMDUDE_SETUP::SetupLeech()
{
    spritetype* pSpr = pDude->pSpr;
    int i;

    pDude->pXLeech = NULL;
    for (i = headspritestat[kStatThing]; i >= 0; i = nextspritestat[i])
    {
        spritetype* pSpr2 = &sprite[i];
        if (pSpr2->owner == pSpr->index && xspriRangeIsFine(pSpr2->extra))
        {
            if (pSpr2->type == kModernThingEnemyLifeLeech)
            {
                XSPRITE *pXSpr2 = &xsprite[pSpr2->extra];
                if (pXSpr2->locked)
                    pDude->LeechKill(false);  // repeat fake killing to set 0 ammo

                // found!
                pDude->pXLeech = pXSpr2;
                break;
            }
        }
    }
}

CUSTOMDUDE* CUSTOMDUDE_SETUP::SameDudeExist(CUSTOMDUDE* pCmp)
{
    int i;
    CUSTOMDUDE* pOther;
    for (i = headspritestat[kStatDude]; i >= 0; i = nextspritestat[i])
    {
        pOther = &gCustomDude[i];
        if (pOther != pCmp && pOther->initialized && pOther->version == pCmp->version)
        {
            if (pOther->pXSpr->data1 == pCmp->pXSpr->data1)
                return pOther;
        }
    }

    return NULL;
}

void CUSTOMDUDE_SETUP::RandomizeDudeSettings()
{
    // randomize some dude's properties, so they look more or less different
    const int states[] = { kCdudeStateSearch, kCdudeStateKnock, kCdudeStateFlee };
    int nVal, i, j; AISTATE* pState;

    if (pDude->randomness.statetime)
    {
        for (i = 0; i < LENGTH(states); i++)
        {
            pState = pDude->states[states[i]];
            for (j = 0; j < kCdudePostureMax; j++)
            {
                nVal = pState->stateTicks;
                pState->stateTicks -= perc2val(pDude->randomness.statetime, nVal);
                pState->stateTicks += perc2val(nnExtRandom(0, pDude->randomness.statetime), nVal);
                pState++;
            }
        }
    }

    if (pDude->randomness.velocity)
    {
        for (i = 0; i < kCdudePostureMax; i++)
        {
            for (j = 0; j < kParVelocityMax; j++)
            {
                if (j == kParVelocityTurn)
                    continue; // too small values to deal with?
                
                nVal = pDude->velocity[i].id[j];
                pDude->velocity[i].id[j] -= perc2val(pDude->randomness.velocity, nVal);
                pDude->velocity[i].id[j] += perc2val(nnExtRandom(0, pDude->randomness.velocity), nVal);
            }
        }
    }

    if (pDude->randomness.thinktime)
    {
        nVal = perc2val(pDude->randomness.thinktime, pDude->thinkClock);
        pDude->thinkClock -= nVal, pDude->thinkClock += nnExtRandom(0, nVal);
    }

    if (pDude->randomness.animscale)
    {
        if (!pDude->pXSpr->scale)
            pDude->pXSpr->scale = kCdudeDefaultAnimScale;
        
        nVal = perc2val(pDude->randomness.animscale, pDude->pXSpr->scale);
        pDude->pXSpr->scale -= nVal, pDude->pXSpr->scale += nnExtRandom(0, nVal);
        CountHeight();
    }

    if (pDude->randomness.weapchance)
    {
        for (i = 0; i < pDude->numWeapons; i++)
        {
            nVal = pDude->weapons[i].pickChance;
            if (nVal >= kChanceMax)
                continue;

            pDude->weapons[i].pickChance -= perc2val(pDude->randomness.weapchance, nVal);
            pDude->weapons[i].pickChance += perc2val(nnExtRandom(0, pDude->randomness.weapchance), nVal);
        }
    }
}

char CUSTOMDUDE_SETUP::FindAiState(AISTATE stateArr[][kCdudePostureMax], int arrLen, AISTATE* pNeedle, int* nType, int* nPosture)
{
    int i, j;
    for (i = 0; i < arrLen; i++)
    {
        for (j = 0; j < kCdudePostureMax; j++)
        {
            if (pNeedle == &stateArr[i][j])
            {
                *nType = i; *nPosture = j;
                return true;
            }
        }
    }

    return false;
}

void CUSTOMDUDE_SETUP::Setup(spritetype* pSpr, XSPRITE* pXSpr)
{
    AISTATE* pModel, * pState;
    int nStateType, nPosture;
    int i, j;

    nDefaultPosture = kCdudePostureL;
    pIni = NULL, hIni = NULL;

    pDude = cdudeGet(pSpr->index);
    pDude->Clear();
    
    pDude->pInfo = getDudeInfo(pSpr->type);
    pDude->pSpr = pSpr; pDude->pXSpr = pXSpr;
    pDude->pExtra = &gDudeExtra[pSpr->extra];
    pDude->pTemplate = NULL;
    pDude->pXLeech = NULL;

    pDude->pWeapon  = &pDude->weapons[0];
    pDude->posture  = kCdudePostureL;
    pDude->nextDude = -1;

    pDude->slaves.list = new IDLIST(true);

    // default stuff
    pDude->randomness.statetime = 20;
    pDude->seeDist      = pDude->pInfo->seeDist;
    pDude->hearDist     = pDude->pInfo->hearDist;
    pDude->periphery    = pDude->pInfo->periphery;
    pDude->sleepDist    = kCdudeMinSeeDist;
    pDude->fallHeight   = INT32_MAX;
    pDude->thinkClock   = 3;

    for (i = 0; i < kDmgMax; i++)
        pDude->morph.id[i] = -1;

    // copy general states
    for (i = 0; i < kCdudeStateNormalMax; i++)
    {
        for (j = 0; j < kCdudePostureMax; j++)
        {
            pModel = &gCdudeStateTemplate[i][j];
            pState = &pDude->states[i][j];

            Bmemcpy(pState, pModel, sizeof(AISTATE));

            if (pModel->nextState)
            {
                // fix next state pointers after copying
                if (FindAiState(gCdudeStateTemplate, LENGTH(gCdudeStateTemplate), pModel->nextState, &nStateType, &nPosture))
                    pState->nextState = &pDude->states[nStateType][nPosture];
                else
                    pState->nextState = NULL;
            }
        }
    }

    // copy dying states
    pModel = gCdudeStateDyingTemplate;
    for (i = kCdudeStateDeathBase; i < kCdudeStateDeathMax; i++)
    {
        for (j = 0; j < kCdudePostureMax; j++)
        {
            pState = &pDude->states[i][j];
            Bmemcpy(pState, pModel, sizeof(AISTATE));
        }
    }

    // copy weapon attack states
    pModel = gCdudeStateAttackTemplate;
    for (i = kCdudeStateAttackBase; i < kCdudeStateAttackMax; i++)
    {
        for (j = 0; j < kCdudePostureMax; j++)
        {
            pState = &pDude->states[i][j];
            Bmemcpy(pState, pModel, sizeof(AISTATE));
            pState->nextState = pState;
        }
    }

    Setup();

    pDude->initialized = 1;
    if (pDude->version == kCdudeVer2)
    {
        if (pDude->pTemplate == NULL)
            pDude->pTemplate = DudeTemplateCreate();

        if (pXSpr->data2 & kCdudeStatusAwaked)
        {
            AnimationFill(pDude->states[kCdudeStateSleep], 0);
            pDude->StatusSet(kCdudeStatusAwaked);
        }

        if (pXSpr->data2 & kCdudeStatusForceCrouch)
            pDude->StatusSet(kCdudeStatusForceCrouch);
    }

    SetupLeech();
    SetupSlaves();

    if (pDude->pTemplate)
        RandomizeDudeSettings();
}

void CUSTOMDUDE_SETUP::Setup(CUSTOMDUDE* pOver)
{
    if (pOver)
        pDude = pOver;

    dassert(pDude != NULL);
    dassert(pDude->pXSpr != NULL);
    XSPRITE* pXSpr = pDude->pXSpr;
    int nPrevVer = pDude->version;
    int nVer;

    nWarnings    = 0;
    showWarnings = true;
    pDude->pTemplate = DudeTemplateFind(kCdudeVer2);

    if (pDude->pTemplate)
    {
        SetupFromDude(pDude->pTemplate);
        return;
    }

    pDude->version = kCdudeVer1;
    if (DescriptLoad(pXSpr->data1))
    {
        nVer = DescriptCheck();
        if (nVer == kCdudeVer2)
        {
            pDude->version = kCdudeVer2;
        }
        else
        {
            Warning(GetError(kErrInvalidVersion));
            DescriptClose();

            pXSpr->data1 = 0;
            pXSpr->data2 = 0;
            pXSpr->data3 = 0;
        }
    }

    if (nPrevVer)
    {
        pDude->ClearEffectCallbacks();
        if (nPrevVer != pDude->version)
        {
            if (pDude->version == kCdudeVer2)
                DescriptClose();

            // do a full re-init
            Setup(pDude->pSpr, pXSpr);
            return;
        }
    }

    pDude->dropItem.Clear();

    switch (pDude->version)
    {
        case kCdudeVer2:
            CUSTOMDUDEV2_SETUP::Setup();
            DescriptClose();
            break;
        default:
            CUSTOMDUDEV1_SETUP::Setup();
            break;
    }

    AnimationFill();
    CountHeight();
    SoundFill();
}

void CUSTOMDUDE_SETUP::SetupFromDude(CUSTOMDUDE* pSrc)
{
    spritetype* pSpr = pDude->pSpr; XSPRITE* pXSpr = pDude->pXSpr;
    CUSTOMDUDE_EFFECT *pEffA, *pEffB; CUSTOMDUDE_WEAPON *pWA;
    AISTATE *pModel, *pState;
    int i, j;

    // copy general stuff
    pDude->version          = pSrc->version;
    pDude->mass             = pSrc->mass;
    pDude->seeDist          = pSrc->seeDist;
    pDude->hearDist         = pSrc->hearDist;
    pDude->periphery        = pSrc->periphery;
    pDude->sleepDist        = pSrc->sleepDist;
    pDude->medium           = pSrc->medium;
    pDude->posture          = pSrc->posture;
    pDude->fallHeight       = pSrc->fallHeight;
    pDude->eyeHeight        = pSrc->eyeHeight;
    pDude->nextDude         = pSrc->nextDude;
    pDude->height           = pSrc->height;
    pDude->turnAng          = pSrc->turnAng;
    pDude->stopMoveOnTurn   = pSrc->stopMoveOnTurn;
    pDude->thinkClock       = pSrc->thinkClock;
    pDude->health           = pSrc->health;

    pSpr->clipdist          = pSrc->pSpr->clipdist;
    pXSpr->scale            = pSrc->pXSpr->scale;

    if (!pDude->initialized)
    {
        if (!pXSpr->sysData2)
            pXSpr->data4 = pXSpr->sysData2 = pDude->health;

        pXSpr->health = nnExtDudeStartHealth(pSpr, pXSpr->sysData2);
    }

    // copy some ai state stuff
    for (i = 0; i < kCdudeStateMax; i++)
    {
        for (j = 0; j < kCdudePostureMax; j++)
        {
            pModel = &pSrc->states[i][j];
            pState = &pDude->states[i][j];
            
            pState->stateType   = pModel->stateType;
            pState->stateTicks  = pModel->stateTicks;
            pState->seqId       = pModel->seqId;
        }
    }
    
    // clear out old data before copying
    for (pWA = pDude->weapons, i = 0; i < pDude->numWeapons; i++, pWA++)
    {
        if (pWA->pFrames)
            delete(pWA->pFrames);
    }
    
    // copy weapons
    Bmemcpy(pDude->weapons, pSrc->weapons, sizeof(pDude->weapons));
    pDude->numAvailWeapons = pSrc->numAvailWeapons;
    pDude->numWeapons = pSrc->numWeapons;
    pDude->pWeapon = &pDude->weapons[0];

    for (pWA = pDude->weapons, i = 0; i < pDude->numWeapons; i++, pWA++)
    {
        if (pWA->pFrames)
        {
            pWA->pFrames = new IDLIST(true);
            CopyListContents(pWA->pFrames,  pSrc->weapons[i].pFrames);
        }
    }


    // clear out old data before copying
    for (i = 0; i < pDude->numEffects; i++)
    {
        pEffA = &pDude->effects[i];
        if (pEffA->pAnims)  delete(pEffA->pAnims);
        if (pEffA->pStates) delete(pEffA->pStates);
        if (pEffA->pFrames) delete(pEffA->pFrames);
    }

    // copy and handle new effects
    Bmemcpy(&pDude->effects, &pSrc->effects, sizeof(pDude->effects));
    pDude->numEffects = pSrc->numEffects;
    for (i = 0; i < pDude->numEffects; i++)
    {
        pEffA = &pDude->effects[i];
        pEffB = &pSrc->effects[i];

        pEffA->pAnims  = new IDLIST(true);
        pEffA->pStates = new IDLIST(true);
        pEffA->pFrames = new IDLIST(true);

        if (pEffB->pAnims && pEffB->pAnims->Length())   CopyListContents(pEffA->pAnims,  pEffB->pAnims);
        if (pEffB->pStates && pEffB->pStates->Length()) CopyListContents(pEffA->pStates, pEffB->pStates);
        if (pEffB->pFrames && pEffB->pFrames->Length()) CopyListContents(pEffA->pFrames, pEffB->pFrames);

    }
    
    // copy everything else
    pDude->slaves.killOnDeath = pSrc->slaves.killOnDeath;
    pDude->slaves.noSetTarget = pSrc->slaves.noSetTarget;
    Bmemcpy(&pDude->damage,     &pSrc->damage,      sizeof(pDude->damage));
    Bmemcpy(&pDude->velocity,   &pSrc->velocity,    sizeof(pDude->velocity));
    Bmemcpy(&pDude->sound,      &pSrc->sound,       sizeof(pDude->sound));
    Bmemcpy(&pDude->dodge,      &pSrc->dodge,       sizeof(pDude->dodge));
    Bmemcpy(&pDude->recoil,     &pSrc->recoil,      sizeof(pDude->recoil));
    Bmemcpy(&pDude->crouch,     &pSrc->crouch,      sizeof(pDude->crouch));
    Bmemcpy(&pDude->knockout,   &pSrc->knockout,    sizeof(pDude->knockout));
    Bmemcpy(&pDude->morph,      &pSrc->morph,       sizeof(pDude->morph));
    Bmemcpy(&pDude->flight,     &pSrc->flight,      sizeof(pDude->flight));
    Bmemcpy(&pDude->dropItem,   &pSrc->dropItem,    sizeof(pDude->dropItem));
    Bmemcpy(&pDude->timer,      &pSrc->timer,       sizeof(pDude->timer));
    Bmemcpy(&pDude->randomness, &pSrc->randomness,  sizeof(pDude->randomness));

}

CUSTOMDUDE* CUSTOMDUDE_SETUP::DudeTemplateFind(int nVer)
{
    spritetype* pSpr; XSPRITE* pXSpr;
    CUSTOMDUDE* pModel;
    int i;

    for (i = headspritestat[kStatTemplate]; i >= 0; i = nextspritestat[i])
    {
        pSpr = &sprite[i];
        if (pSpr->type != kDudeModernCustom)        continue;
        if (!xspriRangeIsFine(pSpr->extra))         continue;
        if ((pModel = cdudeGet(pSpr)) == NULL)      continue;
        if (!pModel->initialized)                   continue;
        if (nVer != pModel->version)                continue;

        pXSpr = &xsprite[pSpr->extra];

        switch (nVer)
        {
            case kCdudeVer2:
                if (pXSpr->data1 != pDude->pXSpr->data1) break;
                return pModel;
            case kCdudeVer1:
                if (pXSpr->data1 != pDude->pXSpr->data1) break;
                if (pXSpr->data2 != pDude->pXSpr->data2) break;
                if (pXSpr->data3 != pDude->pXSpr->data3) break;
                return pModel;
        }
    }

    return NULL;
}

spritetype* CUSTOMDUDE_SETUP::DudeTemplateFindEmpty(void)
{
    spritetype* pSpr;
    CUSTOMDUDE* pModel;
    int i;

    for (i = headspritestat[kStatTemplate]; i >= 0; i = nextspritestat[i])
    {
        pSpr = &sprite[i];
        if (pSpr->type != kDudeModernCustom)        continue;
        if (!xspriRangeIsFine(pSpr->extra))         continue;
        if ((pModel = cdudeGet(pSpr)) == NULL)      continue;
        if (pModel->initialized)                    continue;
        return pSpr;
    }

    return NULL;
}

CUSTOMDUDE* CUSTOMDUDE_SETUP::DudeTemplateCreate()
{
    spritetype* pSpr; XSPRITE* pXSpr;
    CUSTOMDUDE* pModel; IDLIST* pNew;
    int nSpr, nXSpr, i;

    if ((pSpr = DudeTemplateFindEmpty()) == NULL)
    {
        if ((pSpr = actSpawnSprite(pDude->pSpr, kStatTemplate)) == NULL)
            return NULL;
    }

    nSpr = pSpr->index, nXSpr = pSpr->extra;
    Bmemcpy(pSpr, pDude->pSpr, sizeof(*pSpr));
    pSpr->statnum = kStatTemplate, pSpr->index = nSpr;

    pXSpr = &xsprite[nXSpr];
    Bmemcpy(pXSpr, pDude->pXSpr, sizeof(*pXSpr));
    pSpr->extra = nXSpr, pXSpr->reference = nSpr;
    pXSpr->rxID = pXSpr->txID = 0;
    pXSpr->locked = 1;

    pSpr->cstat &= ~(CSTAT_SPRITE_BLOCK | CSTAT_SPRITE_BLOCK_HITSCAN);
    pSpr->cstat |= CSTAT_SPRITE_INVISIBLE;

    if ((pModel = cdudeGet(pSpr)) == NULL)
    {
        actPostSprite(pSpr->index, kStatFree);
        return NULL;
    }

    Bmemcpy(pModel, pDude, sizeof(*pDude));
    pModel->pXSpr = pXSpr; pModel->pSpr = pSpr;
    pModel->pExtra = &gDudeExtra[pSpr->extra];
    pModel->pInfo = getDudeInfo(pSpr->type);
    pModel->slaves.list = NULL;
    pModel->pTemplate = NULL;
    pModel->pXLeech = NULL;

    // It's better to create new list
    // pointers for safe freeing
    // during data change

    for (i = 0; i < pModel->numWeapons; i++)
    {
        CUSTOMDUDE_WEAPON* pWeap = &pModel->weapons[i];
        
        if (pWeap->pFrames)
        {
            pNew = new IDLIST(true);
            CopyListContents(pNew, pWeap->pFrames);
            pWeap->pFrames = pNew;
        }
    }

    for (i = 0; i < pModel->numEffects; i++)
    {
        CUSTOMDUDE_EFFECT* pEff = &pModel->effects[i];
        
        if (pEff->pStates)
        {
            pNew = new IDLIST(true);
            CopyListContents(pNew, pEff->pStates);
            pEff->pStates = pNew;
        }

        if (pEff->pAnims)
        {
            pNew = new IDLIST(true);
            CopyListContents(pNew, pEff->pAnims);
            pEff->pAnims = pNew;
        }

        if (pEff->pFrames)
        {
            pNew = new IDLIST(true);
            CopyListContents(pNew, pEff->pFrames);
            pEff->pFrames = pNew;
        }
    }
    
    return pModel;
}

void CUSTOMDUDE_SETUP::Warning(const char* pFormat, ...)
{
    int i = kErrMax;
    char buffer[512], *pBuf = buffer;

    if (!SameDudeExist(pDude) && showWarnings)
    {
        pBuf += Bsprintf(pBuf, "\n");
        while (--i >= 0)
        {
            if (gErrors[i] == pFormat)
            {
                pBuf += Bsprintf(pBuf, "CDUD_WARN#%d", ++nWarnings);
                break;
            }
        }

        pBuf += Bsprintf(pBuf, " ");

        if (hIni)
        {
            pBuf += Bsprintf(pBuf, "in file ");
            if (hIni->flags & DICT_EXTERNAL)
            {
                pBuf += Bsprintf(pBuf, "%s:", hIni->path);
            }
            else
            {
                pBuf += Bsprintf(pBuf, "%s.%s:", hIni->name, hIni->type);
            }
        }

        pBuf += Bsprintf(pBuf, " ");

        va_list args;
        va_start(args, pFormat);
        pBuf += vsprintf(pBuf, pFormat, args);
        va_end(args);

        pBuf += Bsprintf(pBuf, ".\n");

        if (pGroup)
        {
            pBuf += Bsprintf(pBuf, "Group: %s", pGroup->text);
            if (pIni)
            {
                pBuf += Bsprintf(pBuf, " ");
                pBuf += Bsprintf(pBuf, "(%s)", DescriptGroupExist(pGroup->text) ? "exist" : "not exist");
            }

            pBuf += Bsprintf(pBuf, ",");
            pBuf += Bsprintf(pBuf, " ");
            pBuf += Bsprintf(pBuf, "Parameter: %s", pParam->text);
            if (pIni)
            {
                pBuf += Bsprintf(pBuf, " ");
                pBuf += Bsprintf(pBuf, "(%s)", DescriptParamExist(pGroup->text, pParam->text) ? "exist" : "not exist");
            }

            pBuf += Bsprintf(pBuf, "\n");
            pBuf += Bsprintf(pBuf, "Value: \"%s\"", (pValue) ? pValue : "<empty>");
            pBuf += Bsprintf(pBuf, "\n");
            pBuf += Bsprintf(pBuf, "\n");
        }

        OSD_Printf("%s", buffer);
        if (nWarnings == 1)
            sndStartSample(778, 255);
    }
}

const char* CUSTOMDUDE_SETUP::GetValType(int nType)
{
    nType = ClipRange(nType, 0, LENGTH(gValTypes) - 1);
    return gValTypes[nType];
}

const char* CUSTOMDUDE_SETUP::GetError(int nErr)
{
    nErr = ClipRange(nErr, 0, LENGTH(gErrors) - 1);
    return gErrors[nErr];
}

void CUSTOMDUDE_SETUP::VelocitySetDefault(int nMaxVel)
{
    DUDEINFO* pInfo = pDude->pInfo; XSPRITE* pXSpr = pDude->pXSpr;
    int nFrontSpeed = pInfo->frontSpeed;
    int nSideSpeed = pInfo->sideSpeed;
    int nAngSpeed = pInfo->angSpeed;
    int i;

    if (pXSpr->busyTime)
        nFrontSpeed = ClipHigh((nFrontSpeed / 3) + (2500 * pXSpr->busyTime), nMaxVel);

    for (i = 0; i < kCdudePostureMax; i++)
    {
        pDude->velocity[i].Set(nFrontSpeed, kParVelocityForward);
        pDude->velocity[i].Set(nSideSpeed, kParVelocityDodge);
        pDude->velocity[i].Set(nAngSpeed, kParVelocityTurn);
        pDude->velocity[i].Set(nFrontSpeed, kParVelocityZ);
    }
}

void CUSTOMDUDE_SETUP::DamageSetDefault(void)
{
    for (int i = 0; i < kDmgMax; i++)
        pDude->damage.id[i] = getDudeInfo(kDudeModernCustom)->startDamage[i];
}

void CUSTOMDUDE_SETUP::DamageScaleToSkill(int nSkill)
{
    CUSTOMDUDE_DAMAGE* pDmg = &pDude->damage;
    for (int i = 0; i < kDmgMax; i++)
        pDmg->Set(mulscale8(DudeDifficulty[nSkill], pDmg->id[i]), i);
}

void CUSTOMDUDE_SETUP::WeaponDispersionSetDefault(CUSTOMDUDE_WEAPON* pWeapon)
{
    int nDisp = ClipLow((3000 / (gGameOptions.nDifficulty + 1)) + Random2(500), 500);
    pWeapon->dispersion[0] = nDisp;
    pWeapon->dispersion[1] = nDisp >> 1;
}

void CUSTOMDUDE_SETUP::WeaponRangeSet(CUSTOMDUDE_WEAPON* pWeapon, int nMin, int nMax)
{
    pWeapon->distRange[0] = nMin, pWeapon->distRange[1] = nMax;
    if (nMin > nMax)
        swap(&pWeapon->distRange[0], &pWeapon->distRange[1]);
}

void CUSTOMDUDE_SETUP::WeaponSoundSetDefault(CUSTOMDUDE_WEAPON* pWeapon)
{
    int i, j = 0;
    CUSTOMDUDE_SOUND* pSound = &pWeapon->shotSound;
    for (i = 0; i < kCdudeMaxSounds; i++)
    {
        switch (pWeapon->type)
        {
            case kCdudeWeaponHitscan:
            {
                VECTORINFO_EXTRA* pExtra = &gVectorInfoExtra[pWeapon->id];
                pSound->id[i] = pExtra->fireSound[j];
                j = IncRotate(j, LENGTH(pExtra->fireSound));
                break;
            }
            case kCdudeWeaponMissile:
            {
                MISSILEINFO_EXTRA* pExtra = &gMissileInfoExtra[pWeapon->id - kMissileBase];
                pSound->id[i] = pExtra->fireSound[j];
                j = IncRotate(j, LENGTH(pExtra->fireSound));
                break;
            }
            case kCdudeWeaponKamikaze:
            {
                EXPLOSION_EXTRA* pExtra = &gExplodeExtra[pWeapon->id - kTrapExploder];
                pSound->id[i] = pExtra->snd;
                break;
            }
            case kCdudeWeaponThrow:
                pSound->id[i] = 455;
                break;
            case kCdudeWeaponSummon:
                pSound->id[i] = 379;
                break;
        }
    }
}

void CUSTOMDUDE_SETUP::AnimationConvert(int baseID)
{
    SEQCOMPAT* pEntry;
    AISTATE* pState;
    int i, j, k;

    for (i = 0; i < kCdudeStateMax; i++)
    {
        pState = pDude->states[i];
        AnimationFill(pState, 0);
    }

    for (i = 0; i < LENGTH(gSeqCompat); i++)
    {
        pEntry = &gSeqCompat[i];
        if (pEntry->nAiStateType < 0)
            continue;

        pState = pDude->states[pEntry->nAiStateType];
        if (rngok(pEntry->nAiStateType, kCdudeStateDeathBase, kCdudeStateDeathMax))
        {
            // deaths must be filled for random pick
            for (j = 0, k = 0; j < kCdudePostureMax; j++)
            {
                pState[j].seqId = baseID + pEntry->offset[k];
                k = IncRotate(k, LENGTH(pEntry->offset));
            }
        }
        else
        {
            // other states depends on posture
            for (j = 0; j < LENGTH(pEntry->offset); j++)
            {
                pState[j].seqId = baseID + pEntry->offset[j];
            }
        }
    }
}

void CUSTOMDUDE_SETUP::AnimationFill(AISTATE* pState, int nAnim)
{
    for (int i = 0; i < kCdudePostureMax; i++) pState[i].seqId = nAnim;
}

void CUSTOMDUDE_SETUP::AnimationFill(void)
{
    AISTATE* pState;
    int i, j;

    for (i = 0; i < kCdudeStateMax; i++)
    {
        for (j = 0; j < kCdudePostureMax; j++)
        {
            pState = &pDude->states[i][j];
            if (!helperSeqExists(pState->seqId))
                pState->seqId = 0;
        }
    }
}

void CUSTOMDUDE_SETUP::SoundConvert(int baseID)
{
    CUSTOMDUDE_WEAPON* pWeap;
    CUSTOMDUDE_SOUND* pSound;
    unsigned int i, j, k;
    SNDCOMPAT* pEntry;

    // fill with default sounds first
    Bmemcpy(pDude->sound, gSoundTemplate, sizeof(gSoundTemplate));
    for (i = 0; i < pDude->numWeapons; i++)
    {
        pWeap = &pDude->weapons[i];
        WeaponSoundSetDefault(pWeap);
    }

    if (baseID > 0)
    {
        for (i = 0; i < LENGTH(gSndCompat); i++)
        {
            pEntry = &gSndCompat[i];
            if (irngok(pEntry->nSoundType, kCdudeSndCompatAttack1, kCdudeSndCompatAttack3))
            {
                switch (pEntry->nSoundType)
                {
                    case kCdudeSndCompatAttack3:                        // melee attack
                        pWeap = &pDude->weapons[1];                     // it's a second weapon now
                        break;
                    case kCdudeSndCompatAttack2:                        // throw weapon
                        pWeap = &pDude->weapons[0];
                        if (pWeap->type == kCdudeWeaponThrow) break;
                        else continue;                                  // no way to convert it?
                    default:                                            // normal attack
                        pWeap = &pDude->weapons[0];
                        break;
                }

                pSound = &pWeap->shotSound;

                // setup sound properties
                pSound->medium          = 0x03;
                pSound->interruptable   = 0x01;
                pSound->ai              = 0x00;
                pSound->once            = 0x00;

                // the weapon uses default sounds
                if (!helperSndExists(baseID + pEntry->offset))
                    continue;
            }
            else
            {
                pSound = &pDude->sound[pEntry->nSoundType];
            }

            j = 0, k = 0; // now, fill
            while (j < kCdudeMaxSounds)
            {
                int nSnd = baseID + k + pEntry->offset;
                k = IncRotate(k, pEntry->range);
                pSound->id[j++] = nSnd;
            }
        }
    }
}

void CUSTOMDUDE_SETUP::SoundFill(CUSTOMDUDE_SOUND* pSound, int nSnd)
{
    for (int i = 0; i < kCdudeMaxSounds; i++) pSound->id[i] = nSnd;
}

void CUSTOMDUDE_SETUP::SoundFill(void)
{
    CUSTOMDUDE_SOUND* pSnd; int i, j, nSnd;

    // fill dude sounds
    for (i = 0; i < kCdudeSndMax; i++)
    {
        pSnd = pDude->sound;
        for (j = 0, nSnd = -1; j < kCdudeMaxSounds; j++)
        {
            if (nSnd < 0)
            {
                if (helperSndExists(pSnd->id[j]))
                {
                    nSnd = pSnd->id[j];
                    j = 0;
                }
            }
            else if (!helperSndExists(pSnd->id[j]))
            {
                pSnd->id[j] = nSnd;
            }
        }
    }

    // fill weapon sounds
    for (i = 0; i < kCdudeMaxWeapons; i++)
    {
        pSnd = &pDude->weapons[i].shotSound;
        for (j = 0, nSnd = -1; j < kCdudeMaxSounds; j++)
        {
            if (nSnd < 0)
            {
                if (helperSndExists(pSnd->id[j]))
                {
                    nSnd = pSnd->id[j];
                    j = 0;
                }
            }
            else if (!helperSndExists(pSnd->id[j]))
            {
                pSnd->id[j] = nSnd;
            }
        }
    }
}

void CUSTOMDUDE_SETUP::CountHeight(void)
{
    spritetype* pSpr = pDude->pSpr;
    XSPRITE* pXSpr = pDude->pXSpr;
    AISTATE* pState; Seq* pSeq;
    SEQFRAME* pFrame;
    
    int hg = 0, i, j, zt, zb;
    int opic, oxr, oyr;

    opic = pSpr->picnum;
    pDude->height = 128;

    for (i = kCdudeStateIdle; i < kCdudeStateMoveMax; i++)
    {
        pState = &pDude->states[i][kCdudePostureL];
        if ((pSeq = helperSeqLoad(pState->seqId)) == NULL)
            continue;

        for (j = 0; j < pSeq->nFrames; j++)
        {
            pFrame = &pSeq->frames[j];
            
            oxr = pSpr->xrepeat;
            oyr = pSpr->yrepeat;
            
            if (pFrame->yrepeat > 0)
                pSpr->yrepeat = pFrame->yrepeat;
            
            pSpr->picnum = seqGetTile(pFrame);

            if (pXSpr->scale)
                nnExtSprScaleSet(pSpr, pXSpr->scale);

            GetSpriteExtents(pSpr, &zt, &zb);
            if ((hg = klabs(zb - zt)) > pDude->height)
                pDude->height = hg;

            pSpr->xrepeat = oxr;
            pSpr->yrepeat = oyr;
        }
    }
    
    pSpr->picnum = opic;
    pDude->eyeHeight = perc2val(85, pDude->height>>1);
}


/*************************************************************************************************/


int CUSTOMDUDEV2_SETUP::CheckRange(const char* str, int nVal, int nMin, int nMax)
{
    if (!irngok(nVal, nMin, nMax))
    {
        Warning(GetError(kErrInvalidRange), str, nMin, nMax);
        return nMin;
    }

    return nVal;
}

int CUSTOMDUDEV2_SETUP::CheckValue(const char* str, int nValType, int nDefault)
{
    int t = 0;
    char ok = false;
    
    if (rngok(nValType, kValIdKeywordBase, kValIdKeywordMax))
    {
        if (isIdKeyword(str, GetValType(nValType), &t))
            return t;
    }
    else
    {
        switch (nValType)
        {
            case kValFix:       ok = isfix(str);                                    break;
            case kValUfix:      ok = isufix(str);                                   break;
            case kValPerc:      ok = isperc(str);                                   break;
            case kValBool:      ok = isbool(str);                                   break;
            case kValArrC:      ok = isarray(str, &t);                              break;
            case kValArrA:      ok = isarray(str, &t);                              break;
        }
    }

    if (!ok)
    {
        Warning(GetError(kErrInvalidValType), str, GetValType(nValType));
        return nDefault;
    }

    switch (nValType)
    {
        case kValArrC:
        case kValArrA:
            return t;
        case kValBool:
            return btoi(str);
        default:
            return atoi(str);
    }
}

int CUSTOMDUDEV2_SETUP::CheckValue(const char* str, int nValType, int nMin, int nMax, int nDefault)
{
    int nRetn = CheckValue(str, nValType, INT32_MAX);
    if (nRetn == INT32_MAX)
        return nDefault;

    if (!irngok(nRetn, nMin, nMax))
    {
        Warning(GetError(kErrInvalidRange), str, nMin, nMax);
        return nDefault;
    }

    return nRetn;
}

int CUSTOMDUDEV2_SETUP::CheckValue(const char* str, int nValType, int nMin, int nMax)
{
    int nRetn = CheckValue(str, nValType, INT32_MAX);
    if (nRetn != INT32_MAX)
        return CheckRange(str, nRetn, nMin, nMax);

    return nMin;
}

int CUSTOMDUDEV2_SETUP::CheckParam(const char* str, PARAM* pDb)
{
    int nRetn = FindParam(str, pDb);
    if (nRetn < 0)
        Warning(GetError(kErrInvalidParam), str);

    return nRetn;
}

int CUSTOMDUDEV2_SETUP::ParseKeywords(const char* str, PARAM* pDb)
{
    char tmp[256];
    int nRetn = 0, nPar, i;
    int dLen = ParamLength(pDb);

    i = 0;
    while (i < dLen && enumStr(i++, str, tmp))
    {
        if ((nPar = FindParam(tmp, pDb)) >= 0)
        {
            nRetn |= nPar;
        }
        else
        {
            Warning(GetError(kErrInvalidParam), tmp);
        }
    }

    return nRetn;
}

void CUSTOMDUDEV2_SETUP::SetupGeneral(void)
{
    int nVal = 0, i;
    spritetype* pSpr = pDude->pSpr;
    XSPRITE* pXSpr = pDude->pXSpr;

    /* ----------------------------------*/
    /* DEFAULT VALUES                    */
    /* ----------------------------------*/
    pDude->mass         = 75;
    pDude->medium       = kParMediumAny;
    pDude->nextDude     = -1;

    pParam = gParamGeneral;
    while (pParam->id != kParamMax)
    {
        pValue = DescriptGetValue(pGroup->text, pParam->text);
        if (pValue)
        {
            switch (pParam->id)
            {
                case kParGeneralActiveTime:
                    nVal = CheckValue(pValue, kValUfix, 0, 32767, 800);
                    for (i = 0; i < kCdudePostureMax; i++) pDude->states[kCdudeStateSearch][i].stateTicks = nVal;
                    break;
                case kParGeneralMass:
                    pDude->mass = CheckValue(pValue, kValUfix, 1, 65535, 75);
                    break;
                case kParGeneralMedium:
                    if ((nVal = ParseMedium(pValue)) >= 0)
                    {
                        pDude->medium = nVal;
                        break;
                    }
                    Warning(GetError(kErrInvalidResultC), pValue, pParam->text);
                    break;
                case kParGeneralHealth:
                    if (!pDude->initialized)
                    {
                        pDude->health = CheckValue(pValue, kValUfix, 1, 65535, 60);
                        if (pXSpr->sysData2 == 0)
                            pXSpr->data4 = pXSpr->sysData2 = pDude->health;

                        pXSpr->health = nnExtDudeStartHealth(pSpr, pXSpr->sysData2);
                    }
                    break;
                case kParGeneralClipdist:
                    pSpr->clipdist = CheckValue(pValue, kValUfix, 0, 255, 48);
                    break;
                case kParGeneralSeedist:
                    pDude->seeDist = CheckValue(pValue, kValUfix, 0, 65535, pDude->pInfo->seeDist) << 3;
                    break;
                case kParGeneralHeardist:
                    pDude->hearDist = CheckValue(pValue, kValUfix, 0, 65535, pDude->pInfo->hearDist) << 3;
                    break;
                case kParGeneralPeriphery:
                    pDude->periphery = CheckValue(pValue, kValUfix, 0, kAng360, pDude->pInfo->periphery);
                    break;
            }
        }

        pParam++;
    }
}


void CUSTOMDUDEV2_SETUP::SetupVelocity(void)
{
    CUSTOMDUDE_VELOCITY* pVel;
    VelocitySetDefault(kCdudeMaxVelocity);
    pParam = gParamPosture;

    int nPosture = -1;
    int c = 0;

    if (DescriptGroupExist(pGroup->text))
    {
        while (pParam->id != kParamMax)
        {
            if (rngok(pParam->id, kCdudePosture, kCdudePostureMax))
            {
                pVel = &pDude->velocity[pParam->id - kCdudePosture];
                pValue = DescriptGetValue(pGroup->text, pParam->text);
                if (ParseVelocity(pValue, pVel))
                {
                    if (nPosture < 0)
                        nPosture = pParam->id;

                    c++;
                }
            }

            pParam++;
        }
    }

    if (nPosture >= 0 && c < 2)
        nDefaultPosture = nPosture;
}

void CUSTOMDUDEV2_SETUP::SetupAnimation(AISTATE* pState, char asPosture)
{
    AnimationFill(pState, 0); // clear seqID first
    ParseAnimation(pValue, pState, asPosture);
}

void CUSTOMDUDEV2_SETUP::SetupAnimation(void)
{
    AISTATE* pState;
    int range[2];
    int i;

    /* ----------------------------------*/
    /* DEFAULT VALUES                    */
    /* ----------------------------------*/
    pDude->pXSpr->scale = kCdudeDefaultAnimScale;

    if (!DescriptGroupExist(pGroup->text))
    {
        // whole group is not found, so just use v1 animation set
        AnimationConvert(kCdudeDefaultSeq);
        return;
    }

    pParam = gParamAnim;
    while (pParam->id != kParamMax)
    {
        pValue = DescriptGetValue(pGroup->text, pParam->text);

        switch (pParam->id)
        {
            case kCdudeAnimScale:
                range[0] = 0; range[1] = 1024;
                switch (ParseRange(pValue, kValPerc, range, kCdudeDefaultAnimScale))
                {
                    case 1: pDude->pXSpr->scale = range[0];                                 break;
                    case 2: pDude->pXSpr->scale = range[0] + Random(range[1] - range[0]);   break;
                }
                break;
            case kCdudeStateMove:
                for (int i = kCdudeStateMoveBase; i < kCdudeStateMoveMax; i++)
                {
                    PARAM* pMove = FindParam(i, gParamAnim);
                    if (!pMove || !DescriptGetValue(pGroup->text, pMove->text))
                        SetupAnimation(pDude->states[i], true);
                }
                break;
            default:
                if (rngok(pParam->id, kCdudeStateBase, kCdudeStateNormalMax))
                {
                    SetupAnimation(pDude->states[pParam->id], true);
                    switch (pParam->id)
                    {
                        case kCdudeStateIdle:
                            SetupAnimation(pDude->states[kCdudeStateGenIdle], true); // ai fight compat
                            break;
                    }
                }
                else if (rngok(pParam->id, kCdudeStateDeathBase, kCdudeStateDeathMax))
                {
                    pState = pDude->states[pParam->id];
                    if (getArrayType(pValue) == 2) // assoc array
                    {
                        // the dude must set death animation
                        // according to it's current posture

                        SetupAnimation(pState, true);
                        for (i = 0; i < kCdudePostureMax; i++)
                            pState[i].stateType = kCdudeStateTypeDeathPosture; // mark state
                    }
                    else
                    {
                        // the dude can select death animation randomly
                        SetupAnimation(pState, false);
                    }
                }
                break;
        }

        pParam++;
    }
}

void CUSTOMDUDEV2_SETUP::SetupSound(CUSTOMDUDE_SOUND* pSound)
{
    SoundFill(pSound, 0); // clear rawIDs first
    ParseSound(pValue, pSound);
}

void CUSTOMDUDEV2_SETUP::SetupSound(void)
{
    int nVal, allowMultiSrc = -1;
    CUSTOMDUDE_SOUND* pSound;

    if (!DescriptGroupExist(pGroup->text))
    {
        // whole group is not found, so just use v1 sound set
        SoundConvert(0);
        return;
    }

    // first check global sound params
    if ((pParam = FindParam(kParSndMultiSrc, gParamSounds)) != NULL)
    {
        if ((pValue = DescriptGetValue(pGroup->text, pParam->text)) != NULL)
        {
            if ((nVal = CheckValue(pValue, kValBool, -1)) != -1)
                allowMultiSrc = nVal;
        }
    }

    for (pParam = gParamSounds; pParam->id != kParamMax; pParam++)
    {
        if (rngok(pParam->id, kCdudeSnd, kCdudeSndMax))
        {
            pSound = &pDude->sound[pParam->id];
            pValue = DescriptGetValue(pGroup->text, pParam->text);
            Bmemcpy(pSound, &gSoundTemplate[pParam->id], sizeof(CUSTOMDUDE_SOUND));
            SetupSound(pSound);

            if (pSound->once == 1 && allowMultiSrc > 0)
                pSound->once = 2;
        }
    }
}

void CUSTOMDUDEV2_SETUP::SetupDamage(void)
{
    CUSTOMDUDE_DAMAGE* pDamage = &pDude->damage;
    int nVal, t;
    
    /* ----------------------------------*/
    /* DEFAULT VALUES                    */
    /* ----------------------------------*/
    DamageSetDefault();

    if (DescriptGroupExist(pGroup->text))
    {
        pParam = gParamDamage;
        while (pParam->id != kParamMax)
        {
            pValue = DescriptGetValue(pGroup->text, pParam->text);
            if (pValue)
            {
                switch (pParam->id)
                {
                    case kParDmgSource:
                        pDamage->ignoreSources = ParseKeywords(pValue, gParamDamageSource);
                        break;
                    case kParDmgBotTouch:
                        pDamage->stompDamage = CheckValue(pValue, kValUfix, 0, 65535, 0) << 2;
                        break;
                    default:
                        if (rngok(pParam->id, kDmgFall, kDmgMax))
                        {
                            t = pDamage->id[pParam->id];
                            nVal = CheckValue(pValue, kValPerc, 0, kCdudeMaxDmgScale, 100);
                            pDamage->Set(perc2val(nVal, t), pParam->id);
                        }
                        break;
                }
            }
            
            pParam++;
        }
    }

    DamageScaleToSkill(gGameOptions.nDifficulty);

}

void CUSTOMDUDEV2_SETUP::SetupEventDamage(CUSTOMDUDE_EVENT_DAMAGE* pEvn)
{
    const char* oParText;
    char tmp[64];
    int i;

    if ((pParam = FindParam(kParEventOnDmg, gParamOnEvent)) != NULL)
    {
        // first try to find global damage
        if ((pValue = DescriptGetValue(pGroup->text, pParam->text)) != NULL)
        {
            if (ParseOnEventDmgEx(pValue, pEvn))
            {
                i = kDmgMax;
                while (--i > 0) // fill other damage types
                    Bmemcpy(&pEvn[i], pEvn, sizeof(*pEvn));
            }
        }
    }

    // now try to find damage event for specific damage type
    if ((pParam = FindParam(kParEventOnDmgNamed, gParamOnEvent)) != NULL)
    {
        for (PARAM* pDmgParam = gParamDamage; pDmgParam->id != kParamMax; pDmgParam++)
        {
            if (rngok(pDmgParam->id, kDmgFall, kDmgMax))
            {
                oParText = pParam->text;
                Bsprintf(tmp, pParam->text, pDmgParam->text);
                pParam->text = tmp;

                if ((pValue = DescriptGetValue(pGroup->text, tmp)) != NULL)
                    ParseOnEventDmgEx(pValue, &pEvn[pDmgParam->id]);

                pParam->text = oParText;
            }
        }
    }
}

void CUSTOMDUDEV2_SETUP::SetupRecoil(void)
{
    CUSTOMDUDE_RECOIL* pRecoil = &pDude->recoil;
    CUSTOMDUDE_EVENT_DAMAGE* pEvent = pRecoil->onDamage;
    int i;

    /* ----------------------------------*/
    /* DEFAULT VALUES                    */
    /* ----------------------------------*/
    Bmemset(pRecoil, 0, sizeof(*pRecoil));
    pEvent->cooldown.rng[0] = kCdudeDmgCheckDelay;
    pEvent->cumulative        = 1;
    pEvent->hitcount          = 1;

    i = kDmgMax;
    while(--i > 0)
        Bmemcpy(&pEvent[i], pEvent, sizeof(*pEvent));

    if (DescriptGroupExist(pGroup->text))
        SetupEventDamage(pEvent);
}

void CUSTOMDUDEV2_SETUP::SetupDodge(void)
{
    CUSTOMDUDE_DODGE* pDodge = &pDude->dodge;
    CUSTOMDUDE_EVENT_DAMAGE* pEvnDmg = pDodge->onDamage;
    int nVal, i;

    /* ----------------------------------*/
    /* DEFAULT VALUES                    */
    /* ----------------------------------*/
    Bmemset(pDodge, 0, sizeof(*pDodge));
    pEvnDmg->cooldown.rng[0] = kCdudeDmgCheckDelay;
    pEvnDmg->cumulative        = 1;
    pEvnDmg->hitcount          = 1;
    pDodge->onAimMiss.chance   = kChanceMax;

    i = kDmgMax;
    while(--i > 0)
        Bmemcpy(&pEvnDmg[i], pEvnDmg, sizeof(*pEvnDmg));

    if (!DescriptGroupExist(pGroup->text))
        return;

    pParam = gParamDodge;
    while (pParam->id != kParamMax)
    {
        switch (pParam->id)
        {
            case kParEventOnDmg:
            case kParEventOnDmgNamed:
                SetupEventDamage(pEvnDmg);
                break;
            case kParEventOnAimTargetWrong:
                if ((pValue = DescriptGetValue(pGroup->text, pParam->text)) != NULL)
                {
                    nVal = CheckValue(pValue, kValPerc, 0, 100);
                    pDodge->onAimMiss.chance = perc2val(nVal, kChanceMax);
                }
                break;
        }

        pParam++;
    }
}

void CUSTOMDUDEV2_SETUP::SetupKnockout(void)
{
    CUSTOMDUDE_KNOCKOUT* pKnock = &pDude->knockout;
    CUSTOMDUDE_EVENT_DAMAGE* pEvent = pKnock->onDamage;
    int i;

    /* ----------------------------------*/
    /* DEFAULT VALUES                    */
    /* ----------------------------------*/
    Bmemset(pKnock, 0, sizeof(*pKnock));
    pEvent->cooldown.rng[0] = kCdudeDmgCheckDelay;
    pEvent->cumulative        = 1;
    pEvent->hitcount          = 1;

    i = kDmgMax;
    while(--i > 0)
        Bmemcpy(&pEvent[i], pEvent, sizeof(*pEvent));

    if (DescriptGroupExist(pGroup->text))
        SetupEventDamage(pEvent);
}


void CUSTOMDUDEV2_SETUP::SetupCrouch(void)
{
    CUSTOMDUDE_CROUCH* pCrouch = &pDude->crouch;
    CUSTOMDUDE_TIMER* pTimer = &pDude->timer.crouch;
    CUSTOMDUDE_EVENT_DAMAGE* pEvent = pCrouch->onDamage;
    int i;

    /* ----------------------------------*/
    /* DEFAULT VALUES                    */
    /* ----------------------------------*/
    Bmemset(pCrouch, 0, sizeof(*pCrouch));
    pEvent->cooldown.rng[0] = kCdudeDmgCheckDelay;
    pEvent->cumulative        = 1;
    pEvent->hitcount          = 1;
    pTimer->rng[0]            = 32;
    pTimer->rng[1]            = 0;

    i = kDmgMax;
    while(--i > 0)
        Bmemcpy(&pEvent[i], pEvent, sizeof(*pEvent));

    if (DescriptGroupExist(pGroup->text))
        SetupEventDamage(pEvent);
}

int CUSTOMDUDEV2_SETUP::ParseIDs(const char* str, int nValType, IDLIST* pOut, int nMax)
{
    char tmp[256];
    int i, nVal, nLen;

    if (isarray(str, &nLen))
    {
        if (nMax > 0 && nMax < nLen)
        {
            Warning(GetError(kErrInvalidArrayLen2), 1, nMax, nLen);
            return 0;
        }

        nMax = nLen;
    }

    i = 0;
    while ((nMax <= 0 || i < nMax) && enumStr(i, str, tmp))
    {
        if ((nVal = CheckValue(tmp, nValType, INT32_MAX)) != INT32_MAX)
            pOut->Add(nVal);

        i++;
    }

    return pOut->Length();
}

int CUSTOMDUDEV2_SETUP::ParseIDs(const char* str, int nValType, int* pOut, int nMax)
{
    int i = 0;
    IDLIST ids(true, INT32_MAX);
    if (ParseIDs(str, nValType, &ids, nMax))
    {
        int32_t* pDb = ids.First();
        while (*pDb != INT32_MAX)
        {
            pOut[i++] = *pDb++;
        }
    }

    return i;
}

int CUSTOMDUDEV2_SETUP::ParseEffectIDs(const char* str, const char* paramName, unsigned short* pOut, int nMax)
{
    char tmp[256];
    int i, j, nVal, nLen;

    if (isarray(str, &nLen))
    {
        if (nMax && nMax < nLen)
        {
            Warning(GetError(kErrInvalidArrayLen2), 1, nMax, nLen);
            return 0;
        }
    }

    i = 0;
    while (i < nMax && enumStr(i, str, tmp))
    {
        if (isIdKeyword(tmp, GetValType(kValFX), &nVal))
        {
            if (nVal >= kCudeFXEffectCallbackBase)
            {
                pOut[i] = CheckRange(tmp, nVal, kCudeFXEffectCallbackBase, kCudeFXEffectCallbackBase + LENGTH(gCdudeCustomCallback) - 1);
            }
            else
            {
                pOut[i] = CheckRange(tmp, nVal, 1, 56) + kCdudeFXEffectBase; // it's ok to not do - 1
            }
        }
        else if (isIdKeyword(tmp, GetValType(kValGIB), &nVal))
        {
            pOut[i] = CheckRange(tmp, nVal, 1, 31) + kCdudeGIBEffectBase - 1;
        }
        else
        {
            Warning(GetError(kErrInvalidResultC), tmp, paramName); i++;
            continue;
        }

        // fill others
        for (j = i; j < nMax; j++) { pOut[j] = pOut[i]; }
        i++;
    }

    return i;
}

int CUSTOMDUDEV2_SETUP::ParseStatesToList(const char* str, IDLIST* pOut)
{
    CUSTOMDUDE_WEAPON* pWeap;
    int i = 0, j, nVal, nPar;
    char tmp[256];
    PARAM* pPar;

    while (enumStr(i++, str, tmp))
    {
        nPar = FindParam(tmp, gParamAnim);
        switch (nPar)
        {
            case kCdudeStateMove:
                for (j = kCdudeStateMoveBase; j < kCdudeStateMoveMax; j++)
                {
                    if ((pPar = FindParam(j, gParamAnim)) != NULL)
                        pOut->AddIfNotExists(pPar->id);
                }
                pOut->AddIfNotExists(nPar);
                continue;
            case kCdudeStateDeath:
                for (j = kCdudeStateDeathBase; j < kCdudeStateDeathMax; j++)
                {
                    if ((pPar = FindParam(j, gParamAnim)) != NULL)
                        pOut->AddIfNotExists(pPar->id);
                }
                pOut->AddIfNotExists(nPar);
                continue;
            case kCdudeStateAttack:
                for (j = 0; j < pDude->numWeapons; j++)
                {
                    pWeap = &pDude->weapons[j];
                    if (pWeap->group)
                    {
                        pOut->AddIfNotExists(pWeap->stateID);
                        break;
                    }
                }
                pOut->AddIfNotExists(nPar);
                continue;
            case kCdudeStateIdle:
                pOut->AddIfNotExists(kCdudeStateGenIdle);
                pOut->AddIfNotExists(nPar);
                continue;
            default:
                if (!rngok(nPar, kCdudeStateBase, kCdudeStateMax)) break;
                pOut->AddIfNotExists(nPar);
                continue;
        }

        if (isIdKeyword(tmp, GetValType(kValWeapon), &nVal))
        {
            if (irngok(nVal, 1, kCdudeMaxWeapons))
            {
                j = pDude->numWeapons;
                while(--j >= 0)
                {
                    pWeap = &pDude->weapons[j];
                    if (pWeap->group == nVal)
                    {
                        pOut->AddIfNotExists(pWeap->stateID);
                        break;
                    }
                }
                
                // weapon NA probably because of skill level
                // fill it with dummy value, otherwise it will
                // spawn effects anyway

                if (j < 0 && !pOut->Length())
                    pOut->AddIfNotExists(kCdudeStateMax);
            }
            else Warning(GetError(kErrInvalidRange), tmp, 1, kCdudeMaxWeapons);
            continue;
        }

        Warning(GetError(kErrInvalidResultC), tmp, str);
    }

    return pOut->Length();
}

char CUSTOMDUDEV2_SETUP::ParseGibSetup(const char* str, CUSTOMDUDE_GIB* pGib)
{
    int i = 0, nPar, data[3];
    Bmemset(data, 0, sizeof(data));
    pGib->Clear();

    if (!isempty(str))
    {
        while ((i = enumStr(i, str, key, val)) != 0)
        {
            nPar = FindParam(key, gParamGib);
            switch (nPar)
            {
                case kParGibType:
                    if (ParseIDs(val, kValGIB, data, LENGTH(data)))
                    {
                        pGib->data1 = data[0];
                        pGib->data2 = data[1];
                        pGib->data3 = data[2];
                    }
                    break;
                case kParGibSoundID:
                    pGib->data4 = CheckValue(val, kValUfix, 0, 65535);
                    break;
                case kParGibFlags:
                    pGib->trFlags = ParseKeywords(val, gParamTriggerFlags);
                    break;
                case kParGibPhysics:
                    pGib->physics = ParseKeywords(val, gParamPhysics);
                    break;
                case kParGibForce:
                    pGib->force = CheckValue(val, kValBool, false);
                    break;
            }
        }

        pGib->available = true;
        return true;
    }

    pGib->available = false;
    return false;
}

void CUSTOMDUDEV2_SETUP::SetupEffect()
{
    CUSTOMDUDE_EFFECT* pEff;
    const char* pGroupText = pGroup->text;
    int i, range[2], nVal;
    char tmp[64];

    /* ----------------------------------*/
    /* DEFAULT VALUES                    */
    /* ----------------------------------*/
    pDude->numEffects = 0;

    for (i = 0; i < kCdudeMaxEffectGroups; i++)
    {
        pEff = &pDude->effects[pDude->numEffects];
        pEff->Clear();

        Bsprintf(tmp, pGroupText, i + 1);
        if (!DescriptGroupExist(tmp))
            continue;
        
        pGroup->text = tmp;

        for (pParam = gParamEffect; pParam->text; pParam++)
        {
            pValue = DescriptGetValue(pGroup->text, pParam->text);
            
            if (pParam->id == kParEffectId)
            {
                if (!pValue || ParseEffectIDs(pValue, pParam->text, pEff->id, kCdudeMaxEffects) <= 0)
                {
                    Warning(GetError(kErrReqParamNotFound), pParam->text);
                    break;
                }

                continue;
            }
            else if (!pValue)
                continue;

            switch (pParam->id)
            {
                case kParEffectTimer:
                    ParseTimer(pValue, &pEff->delay);
                    break;
                case kParEffectAngle:
                    nVal = kAng360 - 1;
                    pEff->angle = CheckValue(pValue, kValFix, -nVal, nVal, 0);
                    break;
                case kParEffectMedium:
                    if ((nVal = ParseMedium(pValue)) >= 0)
                    {
                        pEff->medium = nVal;
                        break;
                    }
                    Warning(GetError(kErrInvalidResultC), pValue, pParam->text);
                    break;
                case kParEffectHpRange:
                    range[0] = 0; range[1] = 200;
                    switch (ParseRange(pValue, kValPerc, range))
                    {
                        case 2:
                            pEff->hpRange[0] = range[0];
                            pEff->hpRange[1] = range[1];
                            break;
                        case 1:
                            pEff->hpRange[0] = 0;
                            pEff->hpRange[1] = range[0];
                            break;
                        default:
                            Warning(GetError(kErrInvalidValType), pValue, GetValType(kValArrC));
                            break;
                    }
                    break;
                case kParEffectAiState:     ParseStatesToList(pValue, pEff->pStates);                       break;
                case kParEffectAnimFrame:   ParseIDs(pValue, kValUfix, pEff->pFrames);                      break;
                case kParEffectAnimID:      ParseIDs(pValue, kValUfix, pEff->pAnims);                       break;
                case kParEffectOffset:      ParseOffsets(pValue, &pEff->offset);                            break;
                case kParEffectFx2Gib:      ParseGibSetup(pValue, &pEff->spr2gib);                          break;
                case kParEffectAppearance:  ParseAppearance(pValue, &pEff->appearance);                     break;
                case kParEffectPosture:     pEff->posture       = ParsePosture(pValue);                     break;
                case kParEffectVelocity:    pEff->velocity      = CheckValue(pValue, kValUfix, 0) << 10;    break;
                case kParEffectSlope:       pEff->velocitySlope = CheckValue(pValue, kValFix, 0) << 4;      break;
                case kParEffectRemTime:     pEff->liveTime      = CheckValue(pValue, kValFix, -1, 65535);   break;
                case kParEffectAllUnique:   pEff->allUnique     = CheckValue(pValue, kValBool, 0);          break;
                case kParEffectSrcVel:      pEff->srcVelocity   = CheckValue(pValue, kValBool, 1);          break;
                case kParEffectChance:
                    nVal = CheckValue(pValue, kValPerc, 0, 100, 100);
                    pEff->chance = perc2val(nVal, kChanceMax);
                    break;
            }
        }

        pDude->numEffects++;

    }

    pGroup->text = pGroupText;
}

void CUSTOMDUDEV2_SETUP::SetupMovePattern(void)
{
    int nVal;

    /* ----------------------------------*/
    /* DEFAULT VALUES                    */
    /* ----------------------------------*/
    pDude->fallHeight       = INT32_MAX;
    pDude->turnAng          = kAng60;
    pDude->stopMoveOnTurn   = 0;

    pParam = gParamMovePat;
    while (pParam->id != kParamMax)
    {
        pValue = DescriptGetValue(pGroup->text, pParam->text);
        if (pValue)
        {
            switch (pParam->id)
            {
                case kParMoveFallHeight:
                    nVal = CheckValue(pValue, kValUfix, INT32_MAX);
                    if (nVal != INT32_MAX) pDude->fallHeight = nVal << 4;
                    break;
                case kParMoveTurnAng:
                    pDude->turnAng = CheckValue(pValue, kValUfix, 0, kAng360, kAng60);
                    break;
                case kParMoveStopOnTurn:
                    pDude->stopMoveOnTurn = CheckValue(pValue, kValBool, 0);
                    break;
                case kParMoveDirTimer:
                    ParseTimer(pValue, &pDude->timer.moveDir);
                    break;
            }
        }

        pParam++;
    }

}

int CUSTOMDUDEV2_SETUP::ParseTimer(const char* str, CUSTOMDUDE_TIMER* pTimer)
{
    int range[2] = {0, 32767};
    switch (ParseRange(str, kValUfix, range))
    {
        case 2:
            pTimer->rng[0] = (uint16_t)range[0];
            pTimer->rng[1] = (uint16_t)range[1];
            return 2;
        case 1:
            pTimer->rng[0] = (uint16_t)range[0];
            pTimer->rng[1] = 0;
            return 1;
    }

    return 0;
}

char CUSTOMDUDEV2_SETUP::ParseFlyType(const char* str, CUSTOMDUDE_FLIGHT::TYPE* pOut, CUSTOMDUDE_TIMER* pTimer)
{
    int i = 0, nVal;
    Bmemset(pOut, 0, sizeof(*pOut));

    if (!isempty(str))
    {
        while ((i = enumStr(i, str, key, val)) != 0)
        {
            switch (FindParam(key, gParamFlyType))
            {
                case kParFlyTypeDist:
                    switch (ParseIDs(val, kValUfix, (int*)pOut->distance, 3))
                    {
                        case 1:
                            pOut->distance[1] = pOut->distance[0] << 3;
                            pOut->distance[0] = 0;
                            pOut->distance[2] = INT32_MAX;
                            break;
                        case 2:
                            pOut->distance[0] <<= 3;
                            pOut->distance[1] <<= 3;
                            pOut->distance[2] = INT32_MAX;
                            break;
                        case 3:
                            pOut->distance[0] <<= 3;
                            pOut->distance[1] <<= 3;
                            pOut->distance[2] <<= 4;
                            break;
                    }
                    break;
                case kParFlyTypeTime:
                    if (pTimer) ParseTimer(val, pTimer);
                    break;
                case kParFlyTypeChance:
                    nVal = CheckValue(val, kValPerc, 0, 100, 100);
                    pOut->chance = perc2val(nVal, kChanceMax);
                    break;

            }
        }

        return 1;
    }

    return 0;
}


void CUSTOMDUDEV2_SETUP::SetupFlyPattern(void)
{
    CUSTOMDUDE_FLIGHT* pFlight = &pDude->flight;
    int nVal;

    /* ----------------------------------*/
    /* DEFAULT VALUES                    */
    /* ----------------------------------*/
    pFlight->Clear();
    pDude->timer.goalZ.rng[0] = 32;

    for (pParam = gParamFlyPat; pParam->id != kParamMax; pParam++)
    {
        if ((pValue = DescriptGetValue(pGroup->text, pParam->text)) == NULL)
            continue;

        switch (pParam->id)
        {
            case kCdudeFlyStart:        ParseFlyType(pValue, &pFlight->type[kCdudeFlyStart], &pDude->timer.fLaunch);            break;
            case kCdudeFlyLand:         ParseFlyType(pValue, &pFlight->type[kCdudeFlyLand], &pDude->timer.fLand);               break;
            case kParFlyFriction:       pFlight->friction = CheckValue(pValue, kValUfix, 0, 0x100);                             break;
            case kParFlyClipHeighDist:  pFlight->clipDist = CheckValue(pValue, kValUfix, 0, 65535, 1024) << 3;                  break;
            case kParFlyCFDist:         pFlight->cfDist = CheckValue(pValue, kValUfix, 0, 65535, kCdudeMinCFDist >> 4) << 4;    break;
            case kParFlyBackOnTrack:    pFlight->backOnTrackAccel = CheckValue(pValue, kValPerc, -2000, 2000, 0);               break;
            case kParFlyGoalzTime:      ParseTimer(pValue, &pDude->timer.goalZ);                                                break;
            case kParFlyRelGoalz:       pFlight->absGoalZ = CheckValue(pValue, kValBool, 0);                                    break;
            case kParFlyMustReachGoalz: pFlight->mustReach = CheckValue(pValue, kValBool, 1);                                   break;
            case kParFlyHeigh:
                nVal = CheckValue(pValue, kValUfix, INT32_MAX);
                if (nVal != INT32_MAX) pFlight->maxHeight = nVal << 4;
                break;
        }
    }
}

char CUSTOMDUDEV2_SETUP::ParseWeaponBasicInfo(const char* str, CUSTOMDUDE_WEAPON* pWeap)
{
    int i, nMin, nMax, nID;
    char isNum = isufix(str);
    WEAPINFO* pInfo;

    if (isempty(str))
    {
        isNum = true;
        nID = 0;
    }
    else if (isNum)
    {
        nID = CheckValue(str, kValUfix, 0);
    }

    for (i = 0; i < LENGTH(gCdudeWeaponInfo); i++)
    {
        pInfo = &gCdudeWeaponInfo[i];
        if (isNum)
        {
            // classic style
            if (nID != 0)
            {
                if (!rngok(nID, pInfo->range[0], pInfo->range[1]))
                    continue;
            }
            
            pWeap->id = nID;
        }
        else if (isIdKeyword(str, pInfo->keyword, &nID))
        {
            nMin        = 1;
            nMax        = pInfo->range[1] - pInfo->range[0];
            nID         = CheckRange(str, nID, nMin, nMax) - 1;
            pWeap->id   = pInfo->range[0] + nID;
        }
        else if (isNone(str))
        {
            pWeap->id = 0;
        }
        else
        {
            continue;
        }

        pWeap->clipMask = pInfo->clipMask;
        pWeap->type     = pInfo->type;
        return true;
    }

    return false;
}

char CUSTOMDUDEV2_SETUP::ParseSkill(const char* str)
{
    int range[2] = {1, 5};
    int nSkill = gGameOptions.nDifficulty + 1;
    switch (ParseRange(str, kValUfix, range))
    {
        case 1:
            if (nSkill != range[0]) return false;
            break;
        case 2:
            if (!irngok(nSkill, range[0], range[1])) return false;
            break;
        default:
            return false;
    }

    return true;
}

char CUSTOMDUDEV2_SETUP::ParseDropItem(const char* str, unsigned char out[2])
{
    int nPar, i = 0;
    unsigned char nItem = 0;
    unsigned char nPerc = 100;

    out[0] = out[1] = 0;

    if (isarray(str))
    {
        while ((i = enumStr(i, str, key, val)) != 0)
        {
            nPar = FindParam(key, gParamDropItem);
            switch (nPar)
            {
                case kParDropItemSkill:
                    if (!ParseSkill(val))
                        return false;
                    break;
                case kParDropItemChance:
                    nPerc = CheckValue(val, kValPerc, 0, 100, 100);
                    break;
                case kParDropItemType:
                    nItem = CheckValue(val, kValUfix, kItemWeaponBase, kDudeBase - 1, 0);
                    if (!IsUserItem(nItem) && !rngok(nItem, kItemWeaponBase, kItemMax))
                        nItem = 0;
                    break;
            }
        }

        if (nItem)
        {
            out[0] = nItem;
            out[1] = nPerc;
            return true;
        }
    }
    else if (!isempty(str))
    {
        nItem = CheckValue(str, kValUfix, kItemWeaponBase, kItemMax - 1, 0);
        if (!IsUserItem(nItem) && !rngok(nItem, kItemWeaponBase, kItemMax))
            nItem = 0;

        if (nItem)
        {
            out[0] = nItem;
            out[1] = 100;
            return true;
        }
    }

    return false;
}

void CUSTOMDUDEV2_SETUP::SetupDropItem(void)
{
    int nVal;
    int i, c; char tmp[64];
    CUSTOMDUDE_DROPITEM* pDrop = &pDude->dropItem;
    PARAM* pItem = FindParam(kParDropItem, gParamDropItem);
    
    // global params first
    pParam = gParamDropItem;
    while (pParam->id != kParamMax)
    {
        pValue = DescriptGetValue(pGroup->text, pParam->text);
        if (pValue)
        {
            switch (pParam->id)
            {
                case kParDropItemSprChance:
                    nVal = CheckValue(pValue, kValPerc, 0, 100, 100);
                    pDrop->sprDropItemChance = perc2val(nVal, kChanceMax);
                    break;
            }
        }

        pParam++;
    }
        
    // ItemN params then
    for (i = 0, c = 0; i < kCdudeMaxDropItems; i++)
    {
        Bsprintf(tmp, pItem->text, i + 1);
        pParam->text = tmp;

        pValue = DescriptGetValue(pGroup->text, tmp);
        if (pValue)
        {
            if (ParseDropItem(pValue, pDrop->items[c]))
                c++;
        }
    }
}

int CUSTOMDUDEV2_SETUP::ParseDudeType(const char* str)
{
    int nVal, range[2];
    PARAM* pType;

    for (pType = gParamMorphTo; pType->text; pType++)
    {
        if (isIdKeyword(str, pType->text, &nVal))
        {
            switch (pType->id)
            {
                case kValCdud:
                    return CheckRange(pType->text, nVal, 0, 9999) + kMaxSprites;
                case kValVdud:
                    range[0] = kDudeCultistTommy - kDudeBase;
                    range[1] = kDudeVanillaMax - kDudeBase;
                    return -(kDudeBase + CheckRange(pType->text, nVal, range[0], range[1])) - 1;
                default:
                    continue;
            }

            break;
        }
    }

    return -1;
}

int CUSTOMDUDEV2_SETUP::ParseMorphData(const char* str, int* pOut)
{
    int nPar, i;

    if (isarray(str))
    {
        // search for skill settings first
        // ----------------------------------------------

        i = 0;
        while ((i = enumStr(i, str, key, val)) != 0)
        {
            nPar = FindParam(key, gParamMorphData);
            if (nPar == kParMorphSkill && !ParseSkill(val))
            {
                 // unavailable on current skill level
                *pOut = -1;
                return 1;
            }
        }

        // now other params
        // ----------------------------------------------

        i = 0;
        while ((i = enumStr(i, str, key, val)) != 0)
        {
            nPar = FindParam(key, gParamMorphData);
            if (nPar == kParMorphDude)
            {
               *pOut = ParseDudeType(val);
               return 1;
            }
        }
    }
    else
    {
        *pOut = ParseDudeType(str);
        return 1;
    }

    return 0;
}

void CUSTOMDUDEV2_SETUP::SetupMorphing(void)
{
    CUSTOMDUDE_MORPH* pMorph = &pDude->morph; PARAM *pOGroup = pGroup;
    const char* oParText; char tmp[64];
    int nDude = -1, nVal, i;

    // clear it out
    pDude->nextDude = -1;

    // first try to read deprecated param from "General" group
    pGroup = FindParam(kParGroupGeneral, gParGroup);
    pParam = FindParam(kParGeneralMorphTo, gParamGeneral);
    pValue = DescriptGetValue(pGroup->text, pParam->text);
    if (!isempty(pValue) && ParseMorphData(pValue, &nVal))
        nDude = nVal;
        
    pGroup = pOGroup;
    
    // fill all other damage types
    for (i = 0; i < kDmgMax; i++) pMorph->id[i] = nDude;
    
    // new "Morphing" group is not found
    if (!DescriptGroupExist(pGroup->text))
        return;

    // try to find global param in new morphing group
    if ((pParam = FindParam(kParEvnDeath, gParamMorph)) != NULL)
    {
        pValue = DescriptGetValue(pGroup->text, pParam->text);
        if (!isempty(pValue) && ParseMorphData(pValue, &nVal))
            for (i = 0; i < kDmgMax; i++) pMorph->id[i] = nVal; // fill all other damage types
    }

    // now try to find death event for specific death type
    if ((pParam = FindParam(kParEvnDeathNamed, gParamMorph)) != NULL)
    {
        for (PARAM* pDmgParam = gParamDamage; pDmgParam->id != kParamMax; pDmgParam++)
        {
            if (rngok(pDmgParam->id, kDmgFall, kDmgMax))
            {
                oParText = pParam->text;
                sprintf(tmp, pParam->text, pDmgParam->text);
                pParam->text = tmp;

                pValue = DescriptGetValue(pGroup->text, tmp);
                if (!isempty(pValue) && ParseMorphData(pValue, &nVal))
                    pMorph->id[pDmgParam->id] = nVal;
                
                pParam->text = oParText;
            }
        }
    }
}

void CUSTOMDUDEV2_SETUP::SetupSleeping(void)
{
    int nVal;
    
    /* ----------------------------------*/
    /* DEFAULT VALUES                    */
    /* ----------------------------------*/
    pDude->sleepDist = kCdudeMinSeeDist;

    pParam = gParamSleeping;
    while (pParam->id != kParamMax)
    {
        pValue = DescriptGetValue(pGroup->text, pParam->text);
        if (pValue)
        {
            switch (pParam->id)
            {
                case kParSleepSpotRadius:
                    nVal = CheckValue(pValue, kValUfix, 0, 65535, pDude->sleepDist) << 3;
                    pDude->sleepDist = nVal;
                    break;
            }
        }

        pParam++;
    }
}

void CUSTOMDUDEV2_SETUP::SetupSlaves(void)
{
    /* ----------------------------------*/
    /* DEFAULT VALUES                    */
    /* ----------------------------------*/
    pDude->slaves.killOnDeath = 0;
    pDude->slaves.noSetTarget = 0;

    for (pParam = gParamSlaves; pParam->id != kParamMax; pParam++)
    {
        if ((pValue = DescriptGetValue(pGroup->text, pParam->text)) == NULL)
            continue;
       
        switch (pParam->id)
        {
            case kParSlaveNoForce:      pDude->slaves.noSetTarget = CheckValue(pValue, kValBool, 0, 1, 0);  break;
            case kParSlaveOnDeathDie:   pDude->slaves.killOnDeath = CheckValue(pValue, kValBool, 0, 1, 0);  break;
        }
    }
}

void CUSTOMDUDEV2_SETUP::SetupRandomness(void)
{
    /* ----------------------------------*/
    /* DEFAULT VALUES                    */
    /* ----------------------------------*/
    pDude->randomness.statetime = 20;
    
    for (pParam = gParamRandomness; pParam->id != kParamMax; pParam++)
    {
        if ((pValue = DescriptGetValue(pGroup->text, pParam->text)) == NULL)
            continue;
       
        switch (pParam->id)
        {
            case kParRandStateTime: pDude->randomness.statetime = CheckValue(pValue, kValPerc, 0, 100, 20); break;
            case kParRandVelocity:  pDude->randomness.velocity  = CheckValue(pValue, kValPerc, 0, 100, 0);  break;
            case kParRandThinkTime: pDude->randomness.thinktime = CheckValue(pValue, kValPerc, 0, 100, 0);  break;
            case kParRandAnimScale: pDude->randomness.animscale = CheckValue(pValue, kValPerc, 0, 100, 0);  break;
            case kParRandWeapChance: pDude->randomness.weapchance = CheckValue(pValue, kValPerc, 0, 100, 0);  break;
        }
    }
}

void CUSTOMDUDEV2_SETUP::SetupTweaks(void)
{
    /* ----------------------------------*/
    /* DEFAULT VALUES                    */
    /* ----------------------------------*/
    pDude->thinkClock   = 3;
    sortWeapons         = 1;
    
    for (pParam = gParamTweaks; pParam->id != kParamMax; pParam++)
    {
        if ((pValue = DescriptGetValue(pGroup->text, pParam->text)) == NULL)
            continue;
       
        switch (pParam->id)
        {
            case kParTweaksThinkClock:
                pDude->thinkClock = CheckValue(pValue, kValUfix, 0, 255, 3);
                break;
            case kParTweaksWaponSort:
                sortWeapons = CheckValue(pValue, kValBool, 0, 1, 1);
                break;
        }
    }
}

void CUSTOMDUDEV2_SETUP::SetupWeapons(void)
{
    int nVal, i, t = -1;
    const char* pGroupText = pGroup->text;
    char tmp[64]; int data[32];
    CUSTOMDUDE_WEAPON* pWeap;
    AISTATE* pState;

    /* ----------------------------------*/
    /* DEFAULT VALUES                    */
    /* ----------------------------------*/
    pDude->numWeapons = 0;

    for (i = 0; i < kCdudeMaxWeapons; i++)
    {
        pWeap = &pDude->weapons[pDude->numWeapons];
        pWeap->Clear();

        Bsprintf(tmp, pGroupText, i + 1);
        if (!DescriptGroupExist(tmp))
            continue;

        pGroup->text = tmp;

        // search for skill settings first
        // ----------------------------------------------
        pParam = FindParam(kParWeaponSkill, gParamWeapon);
        if (pParam && (pValue = DescriptGetValue(tmp, pParam->text)) != NULL)
        {
            if (!ParseSkill(pValue))
                continue;
        }

        pWeap->group = i + 1;
        pDude->numWeapons++;

        for (pParam = gParamWeapon; pParam->id != kParamMax; pParam++)
        {
            pValue = DescriptGetValue(tmp, pParam->text);
            switch (pParam->id)
            {
                case kParWeaponAttackAng:
                    pWeap->angle = CheckValue(pValue, kValUfix, 0, kAng360, kAng15);
                    break;
                case kParWeaponDist:
                    data[0] = 0; data[1] = 0x10000;
                    switch (ParseRange(pValue, kValUfix, data))
                    {
                        case 2:
                            pWeap->distRange[0] = data[0] << 3;
                            pWeap->distRange[1] = data[1] << 3;
                            break;
                        default:
                            Warning(GetError(kErrInvalidValType), pValue, GetValType(kValArrC));
                            pWeap->distRange[0] = 0;
                            pWeap->distRange[1] = 32767;
                            break;
                    }
                    break;
                case kParWeaponHeigh:
                    ParseWeaponHeight(pValue, pWeap);
                    break;
                case kParWeaponSlope:
                    data[0] = INT32_MIN; data[1] = INT32_MAX;
                    switch (ParseRange(pValue, kValFix, data))
                    {
                        case 1:
                            pWeap->slopeRange[0] = -(data[0] << 3);
                            pWeap->slopeRange[1] =  klabs(data[0] << 3);
                            break;
                        case 2:
                            pWeap->slopeRange[0] = (data[0] << 3);
                            pWeap->slopeRange[1] = (data[1] << 3);
                            break;
                        default:
                            pWeap->slopeRange[0] = INT32_MIN;
                            pWeap->slopeRange[1] = INT32_MAX;
                            break;
                    }
                    break;
                case kParWeaponAkimboFrame:
                    if (pValue)
                    {
                        pWeap->pFrames = new IDLIST(true);
                        ParseIDs(pValue, kValUfix, pWeap->pFrames);
                        for (int32_t* p = pWeap->pFrames->First(); *p != kListEndDefault; *p = *p + 1, p++);
                    }
                    break;
                case kParWeaponPickChance:
                    nVal = CheckValue(pValue, kValPerc, 0, 100, 100);
                    pWeap->pickChance = perc2val(nVal, kChanceMax);
                    break;
                case kParWeaponId:
                    if (ParseWeaponBasicInfo(pValue, pWeap)) break;
                    Warning(GetError(kErrInvalidResultC), pValue, pParam->text);
                    break;
                case kParWeaponCooldown:
                    ParseWeaponCooldown(pValue, pWeap);
                    break;
                case kParWeaponMedium:
                    if (pValue)
                    {
                        if ((nVal = ParseMedium(pValue)) >= 0)
                        {
                            pWeap->medium = nVal;
                            break;
                        }
                        
                        Warning(GetError(kErrInvalidResultC), pValue, pParam->text);
                    }
                    break;
                case kParWeaponDudeHealth:
                case kParWeaponTargetHealth:
                    if (pValue)
                    {
                        data[0] = 0; data[1] = 200;
                        if (ParseRange(pValue, kValPerc, data) == 2)
                        {
                            switch (pParam->id)
                            {
                                case kParWeaponDudeHealth:
                                    pWeap->dudeHpRange[0] = data[0];
                                    pWeap->dudeHpRange[1] = data[1];
                                    break;
                                default:
                                    pWeap->targHpRange[0] = data[0];
                                    pWeap->targHpRange[1] = data[1];
                                    break;
                            }

                            break;
                        }

                        Warning(GetError(kErrInvalidValType), pValue, GetValType(kValArrC));
                    }
                    break;
                case kParWeaponPosture:
                    pWeap->posture = ParsePosture(pValue);
                    break;
                case kParWeaponStyle:
                case kParWeaponShotSetup:
                case kParWeaponAttackSetup:
                    if (pValue)
                    {
                        if (isarray(pValue))
                        {
                            switch (pParam->id)
                            {
                                case kParWeaponStyle:           ParseWeaponStyle(pValue, pWeap);    break;
                                case kParWeaponShotSetup:       ParseShotSetup(pValue, pWeap);      break;
                                case kParWeaponAttackSetup:     ParseAttackSetup(pValue, pWeap);    break;
                            }

                            break;
                        }

                        Warning(GetError(kErrInvalidValType), pValue, GetValType(kValArrA));
                    }
                    break;
                case kParWeaponAttackAnim:
                    pWeap->stateID = kCdudeStateAttackBase + pDude->numWeapons - 1;
                    pWeap->nextStateID = pWeap->stateID;

                    pState = pDude->states[pWeap->stateID];
                    SetupAnimation(pState, true);
                    for (t = 0; t < kCdudePostureMax; t++)
                    {
                        if (!helperSeqTriggerExists(pState->seqId))
                        {
                            Seq* pSeq = helperSeqLock(pState->seqId);
                            if (pSeq)
                                pSeq->frames[pSeq->nFrames - 1].at5_5 = 1;
                        }

                        pState++;
                    }
                    break;
                case kParWeaponAmmo:
                case kParWeaponDisp:
                case kParWeaponAkimbo:
                    if (pValue)
                    {
                        switch (pParam->id)
                        {
                            case kParWeaponAmmo:
                                nVal = CheckValue(pValue, kValUfix, 0, 32767);
                                pWeap->ammo.SetTotal(nVal); pWeap->ammo.SetFull();
                                break;
                            case kParWeaponDisp:
                                data[0] = 0; data[1] = 1024;
                                switch (ParseRange(pValue, kValPerc, data, kCdudeMaxDispersion))
                                {
                                    case 2:
                                        pWeap->dispersion[0] = data[0];
                                        pWeap->dispersion[1] = data[1];
                                        break;
                                    case 1:
                                        pWeap->dispersion[0] = data[0];
                                        pWeap->dispersion[1] = data[0] >> 1;
                                        break;
                                    default:
                                        Warning(GetError(kErrInvalidResultC), pValue, pParam->text);
                                        break;
                                }
                                break;
                            case kParWeaponAkimbo:
                                pWeap->sharedId = CheckValue(pValue, kValUfix, 0, kCdudeMaxWeapons);
                                break;
                        }
                    }
                    break;
                case kParWeaponShotSnd:
                    if (ParseSound(pValue, &pWeap->shotSound)) break;
                    else WeaponSoundSetDefault(pWeap);
                    break;
                case kParWeaponAttackSnd:
                    if (ParseSound(pValue, &pWeap->attackSound))
                    {
                        pWeap->attackSound.ai            = 0;
                        pWeap->attackSound.once          = 1;
                        pWeap->attackSound.interruptable = 1;
                    }
                    break;
                case kParWeaponShotAppearance:
                    ParseAppearance(pValue, &pWeap->shot.appearance);
                    break;
                case kParWeaponData:
                    Bmemset(data, 0, sizeof(data));
                    if (ParseIDs(pValue, kValFix, data, LENGTH(data)))
                    {
                        pWeap->data1 = data[0];
                        pWeap->data2 = data[1];
                        pWeap->data3 = data[2];
                        pWeap->data4 = data[3];
                    }
                    break;
                case kParWeaponIsDefault:
                    if (pValue)
                        pWeap->isDefault = CheckValue(pValue, kValBool, 0, 1, 0);
                    break;
            }
        }
    }

    pGroup->text = pGroupText;
    if (sortWeapons && pDude->numWeapons > 1) // weapon with lowest pickChance in top
        qsort(pDude->weapons, pDude->numWeapons, sizeof(pDude->weapons[0]), (int(*)(const void*, const void*))qsSortWeapons);
}

char CUSTOMDUDEV2_SETUP::ParseVelocity(const char* str, CUSTOMDUDE_VELOCITY* pVelocity)
{
    int nMod = pDude->pXSpr->busyTime;
    int i, nPar, nVal;
        
    if (isarray(str))
    {
        i = 0, nVal = -1;
        while (enumStr(i++, str, key, val))
        {
            switch (nPar = FindParam(key, gParamVelocity))
            {
                case kParVelocityDodge:
                case kParVelocityForward:
                    if ((nVal = CheckValue(val, kValUfix, 0, kCdudeMaxVelocity, -1)) != -1)
                    {
                        nVal <<= 2;
                        if (nMod && nPar == kParVelocityForward) // allow to mod velocity by busyTime
                             nVal = (nVal / 3) + (2500 * nMod);

                        pVelocity->id[nPar] = nVal;
                    }
                    break;
                case kParVelocityTurn:
                    if ((nVal = CheckValue(val, kValUfix, 0, 100, -1)) == -1) break;
                    pVelocity->id[nPar] = nVal << 2;
                    break;
                case kParVelocityZ:
                    if ((nVal = CheckValue(val, kValUfix, 0, kCdudeMaxVelocity, -1)) != -1)
                    {
                        nVal <<= 5;
                        if (nMod) // allow to mod velocity by busyTime
                            nVal = (nVal / 3) + (2500 * nMod);

                        pVelocity->id[nPar] = nVal;
                    }
                    break;
            }
        }

        return true;
    }
    else if (!isempty(str))
    {
        if ((nVal = CheckValue(str, kValUfix, 0, kCdudeMaxVelocity, -1)) != -1)
        {
            nVal <<= 2;
            if (nMod)
                nVal = (nVal / 3) + (2500 * nMod); // allow to mod forward velocity by busyTime

            pVelocity->id[kParVelocityForward] = nVal;
            return true;
        }
    }

    return false;
}

char CUSTOMDUDEV2_SETUP::ParseAppearance(const char* str, APPEARANCE* pAppear)
{
    int i = 0, nPar, nVal, range[2];
    char inherit;

    pAppear->Clear();

    if (!isempty(str))
    {
        while ((i = enumStr(i, str, key, val)) != 0)
        {
            nPar = FindParam(key, gParamAppearance);
            switch (nPar)
            {
                case kAppearClb:
                    if (isIdKeyword(val, gValTypes[kValFX], &nVal))         pAppear->clb = kCdudeFXEffectBase + nVal;
                    else if (isIdKeyword(val, gValTypes[kValGIB], &nVal))   pAppear->clb = kCdudeGIBEffectBase + nVal;
                    else if (isufix(val))                                   pAppear->clb = CheckValue(val, kValUfix, 0, LENGTH(gCdudeCustomCallback));
                    else                                                    Warning(GetError(kErrInvalidValType), val);
                    break;
                case kAppearSeq:
                    pAppear->seq = CheckValue(val, kValUfix, 0, 65535);
                    break;
                case kAppearSnd:
                    pAppear->soundAvailable = ParseSound(val, &pAppear->sound);
                    pAppear->sound.once     = true;
                    break;
                case kAppearPic:
                    pAppear->pic = CheckValue(val, kValUfix, 1, kMaxTiles-1, 0);
                    break;
                case kAppearShade:
                case kAppearPal:
                case kAppearSize:
                case kAppearScale:
                    inherit = (FindParam(val, gParamKeyword) == kParKeywordInherit);
                    switch (nPar)
                    {
                        case kAppearShade:
                            pAppear->shd = (inherit) ? -129 : CheckValue(val, kValFix, -128, 64, 128);
                            break;
                        case kAppearPal:
                            pAppear->pal = (inherit) ? -129 : CheckValue(val, kValUfix, 0, 255);
                            break;
                        case kAppearSize:
                            if (!inherit)
                            {
                                switch (ParseIDs(val, kValUfix, range, 2))
                                {
                                    case 1:
                                        pAppear->xrp = pAppear->yrp = CheckRange(val, range[0], 0, 255);
                                        break;
                                    case 2:
                                        pAppear->xrp = CheckRange(val, range[0], 0, 255);
                                        pAppear->yrp = CheckRange(val, range[1], 0, 255);
                                        break;
                                }

                                break;
                            }
                            pAppear->xrp = -129;
                            pAppear->yrp = -129;
                            break;
                        case kAppearScale:
                            if (!inherit)
                            {
                                range[0] = 0; range[1] = 1024;
                                switch (ParseRange(val, kValPerc, range, kCdudeDefaultAnimScale))
                                {
                                    case 2:
                                        pAppear->scl[0] = range[0];
                                        pAppear->scl[1] = range[1];
                                        break;
                                    case 1:
                                        pAppear->scl[0] = range[0];
                                        pAppear->scl[1] = 0;
                                        break;
                                }

                                break;
                            }
                            pAppear->scl[0] = 1024;
                            pAppear->scl[1] = 1024;
                            break;
                    }
                    break;
                case kAppearOffs1:
                    switch (ParseIDs(val, kValFix, range, 2))
                    {
                        case 1:
                            pAppear->xof = pAppear->yof = CheckRange(val, range[0], -128, 127);
                            break;
                        case 2:
                            pAppear->xof = CheckRange(val, range[0], -128, 127);
                            pAppear->yof = CheckRange(val, range[1], -128, 127);
                            break;
                    }
                    break;
                case kAppearCstat:
                    switch (ParseIDs(val, kValUfix, range, 2))
                    {
                        case 2:
                            pAppear->csta = range[0];
                            pAppear->cstr = range[1];
                            break;
                        case 1:
                            pAppear->csta = range[0];
                            pAppear->cstr = 0;
                            break;
                        default:
                            Warning(GetError(kErrInvalidValType), val, GetValType(kValArrC));
                            break;
                    }
                    break;
            }
        }

        pAppear->available = true;
        return true;
    }

    pAppear->available = false;
    return false;
}

char CUSTOMDUDEV2_SETUP::ParseSound(const char* str, CUSTOMDUDE_SOUND* pSound)
{
    int i = 0, j, nMedium = pSound->medium, nVolume = pSound->volume;
    int t = kCdudeMaxSounds, nVal, nLen;
    char tmp[256];

    if ((nLen = CheckArray(str, 0, t + 2, 0)) > 0)
    {
        // first 1 - 3 elements supposed to be ufix
        while (i < t && enumStr(i, str, tmp) && isufix(tmp))
        {
            nVal = CheckValue(tmp, kValUfix, 0, 65535);
            for (j = i; j < t; j++) pSound->id[j] = nVal;
            i++;
        }

        // at least 1 sound id
        if (!i)
        {
            Warning(GetError(kErrInvalidParam), tmp);
            t = 0;
        }
        else if (t < nLen) // continue parsing
            t += 2;

        // next is the sound volume % and/or medium
        while (i < t && enumStr(i, str, tmp))
        {
            if (isperc(tmp))
            {
                if (nLen - i <= 2)
                {
                    nVolume = CheckValue(tmp, kValPerc, 0, 1023, 100);
                }
                else
                {
                    Warning(GetError(kErrInvaliValuePos), tmp);
                    break;
                }
            }
            else if ((nMedium = ParseMedium(tmp)) >= 0)
            {
                if (nLen - i != 1)
                {
                    Warning(GetError(kErrInvaliValuePos), tmp);
                    break;
                }
            }
            else
            {
                Warning(GetError(kErrInvalidParam), tmp);
                break;
            }

            i++;
        }
        
        if (t)
        {
            pSound->volume = perc2val(nVolume, 255);
            pSound->medium = nMedium;
            return true;
        }
    }
    else if (!isempty(str))
    {
        nVal = CheckValue(str, kValUfix, 0, 65535);
        pSound->medium = kParMediumAny;
        SoundFill(pSound, nVal);
        return true;
    }

    return false;
}

char CUSTOMDUDEV2_SETUP::ParseAnimation(const char* str, AISTATE* pState, char asPosture)
{
    int i, j, nPar, nLen;
    int nVal;

    if ((nLen = CheckArray(str, 0, kCdudePostureMax, 0)) > 0)
    {
        for (i = 0; i < nLen; i++)
        {
            if (asPosture)
            {
                if (enumStr(i, str, key, val))
                {
                    nPar = FindParam(key, gParamPosture);
                    if (rngok(nPar, kCdudePosture, kCdudePostureMax))
                        pState[nPar].seqId = CheckValue(val, kValUfix, 0, 65535);

                    continue;
                }
            }
            else if (enumStr(i, str, val))
            {
                nVal = CheckValue(val, kValUfix, 0, 65535);
                
                pState[i].seqId = nVal;
                for (j = i; j < kCdudePostureMax; j++)
                    pState[j].seqId = nVal;

                continue;
            }

            break;
        }
        
        return (i > 0);
    }
    else if (!isempty(str))
    {
        i = (asPosture) ? nDefaultPosture : 0;
        pState[i].seqId = CheckValue(str, kValUfix, 0, 65535);
        if (!asPosture)
            AnimationFill(pState, pState[i].seqId);

        return true;
    }

    return false;
}

char CUSTOMDUDEV2_SETUP::ParseRange(const char* str, int nValType, int out[2], int nBaseVal)
{
    int nLen, nVal;
    int nMin = out[0];
    int nMax = out[1];
    char chkrng = (nMin && nMax);
    char tmp[256];
    int i = 0;

    if (!isempty(str))
    {
        nLen = CheckArray(str, 0, 2);
        while (i < nLen && enumStr(i, str, tmp))
        {
            if (chkrng)
            {
                nVal = CheckValue(tmp, nValType, nMin, nMax);
            }
            else
            {
                nVal = CheckValue(tmp, nValType, 0);
            }

            switch (nValType)
            {
                case kValPerc:
                    if (nBaseVal) nVal = perc2val(nVal, nBaseVal);
                    break;
            }

            out[i++] = nVal;
        }

        return i;
    }

    return 0;
}

int CUSTOMDUDEV2_SETUP::ParseMedium(const char* str)
{
    switch (FindParam(str, gParamMedium))
    {
        case kParMediumLand:   return 0x01;
        case kParMediumWater:  return 0x02;
        case kParMediumAny:    return 0x00;
        default:               return   -1;
    }
}

int CUSTOMDUDEV2_SETUP::CheckArray(const char* str, int nMin, int nMax, int nDefault)
{
    int nLen;
    if (isarray(str, &nLen))
    {
        if (nMax == 0 && nMin > 0 && nLen < nMin)
        {
            Warning(GetError(kErrInvalidArrayLen1), nMin, nLen);
            return nDefault;
        }

        if ((nMin > 0 && nLen < nMin) || (nMax > 0 && nLen > nMax))
        {
            Warning(GetError(kErrInvalidArrayLen2), nMin, nMax, nLen);
            return nDefault;
        }

        return nLen;
    }

    return nDefault;
}

char CUSTOMDUDEV2_SETUP::ParseOffsets(const char* str, POINT3D* pOut)
{
    int i, nVal, nLen; char tmp[256];
    Bmemset(pOut, 0, sizeof(POINT3D));
    nLen = CheckArray(str, 3, 0, 0);

    i = 0;
    while (i < nLen && enumStr(i, str, tmp))
    {
        nVal = CheckValue(tmp, kValFix, -16383, 16383, 0);
        switch (i)
        {
        case 0: pOut->x = nVal;       break;
        case 1: pOut->y = nVal;       break;
        case 2: pOut->z = nVal << 4;  break;
        }

        i++;
    }

    return i;
}

char CUSTOMDUDEV2_SETUP::ParseShotSetup(const char* str, CUSTOMDUDE_WEAPON* pWeap)
{
    int i, nPar, nVal;
    if (isarray(str))
    {
        i = 0;
        while ((i = enumStr(i, str, key, val)) != 0)
        {
            nPar = FindParam(key, gWeaponShotSetup);
            switch (nPar)
            {
                case kParWeaponShotOffs:
                    ParseOffsets(val, &pWeap->shot.offset);
                    break;
                case kParWeaponShotVel:
                    pWeap->shot.velocity = CheckValue(val, kValFix, INT32_MAX);
                    if (pWeap->shot.velocity != INT32_MAX)
                        pWeap->shot.velocity <<= 10;
                    break;
                case kParWeaponShotSlope:
                    pWeap->shot.slope = CheckValue(val, kValFix, INT32_MAX);
                    if (pWeap->shot.slope != INT32_MAX)
                        pWeap->shot.slope <<= 4;
                    break;
                case kParWeaponShotFollow:
                    nVal = CheckValue(val, kValUfix, 0, kAng360);
                    pWeap->shot.targetFollow = ClipHigh(nVal, kAng360 - 1);
                    break;
                case kParWeaponShotClipdist:
                    pWeap->shot.clipdist = CheckValue(val, kValUfix, 0, 255, 32);
                    break;
                case kParWeaponShotImpact:
                    pWeap->shot.impact = CheckValue(val, kValBool, false);
                    break;
                case kParWeaponShotRemTimer:
                    pWeap->shot.remTime = CheckValue(val, kValUfix, 0, 4095);
                    break;
            }
        }

        return true;
    }

    return false;
}

char CUSTOMDUDEV2_SETUP::ParseWeaponStyle(const char* str, CUSTOMDUDE_WEAPON* pWeap)
{
    Bmemset(&pWeap->style, 0, sizeof(pWeap->style));
    int nPar, i = 0;

    i = 0;
    while ((i = enumStr(i, str, key, val)) != 0)
    {
        nPar = FindParam(key, gParamWeaponStyle);
        switch (nPar)
        {
            case kParWeaponStyleOffset:
                ParseOffsets(val, &pWeap->style.offset);
                break;
            case kParWeaponStyleAngle:
                pWeap->style.angle = CheckValue(val, kValUfix, 0, kAng360);
                break;
        }
    }

    pWeap->style.available = true;
    return true;
}

char CUSTOMDUDEV2_SETUP::ParseWeaponPrediction(const char* str, CUSTOMDUDE_WEAPON* pWeap)
{
    CUSTOMDUDE_WEAPON::PREDICTION* pPredict = &pWeap->prediction;
    char buf[sizeof(val)];
    int nVal, i;

    pPredict->accuracy  = 0x1AAAAA;
    pPredict->distance  = 0x0;
    pPredict->angle     = kAng15;

    i = 0;
    while (enumStr(i, str, buf))
    {
        switch (i)
        {
            case 0:
                pPredict->distance = CheckValue(buf, kValUfix, 0, 65535) << 3;
                break;
            case 1:
                pPredict->angle = CheckValue(buf, kValUfix, 0, kAng360);
                if (!pPredict->angle) pPredict->angle = kAng360;
                break;
            case 2:
                nVal = CheckValue(buf, kValUfix, 1, 1000, 100);
                pPredict->accuracy = perc2val(nVal, 0x1AAAAA);
                break;
        }

        i++;
    }

    return 1;
}

char CUSTOMDUDEV2_SETUP::ParseWeaponHeight(const char* str, CUSTOMDUDE_WEAPON* pWeap)
{
    int i = 0, nVal, range[2] = {INT32_MIN, INT32_MAX};
    char found = 0;

    if (isempty(str))
        return false;

    if ((nVal = getArrayType(str)) == 2)
    {
        while ((i = enumStr(i, str, key, val)) != 0)
        {
            range[0] = INT32_MIN, range[1] = INT32_MAX;
            switch (FindParam(key, gParamKeyword))
            {
                case kParKeywordAbove:
                    switch (ParseRange(val, kValUfix, range))
                    {
                        case 2:
                            pWeap->heighRange[0] = -range[1];
                            pWeap->heighRange[1] = -range[0];
                            break;
                        case 1:
                            pWeap->heighRange[0] = -range[0];
                            pWeap->heighRange[1] = 0;
                            break;
                    }
                    found |= 0x01;
                    break;
                case kParKeywordBelow:
                    switch (ParseRange(val, kValUfix, range))
                    {
                        case 2:
                            pWeap->heighRange[2] = range[0];
                            pWeap->heighRange[3] = range[1];
                            break;
                        case 1:
                            pWeap->heighRange[2] = 0;
                            pWeap->heighRange[3] = range[0];
                            break;
                    }
                    found |= 0x02;
                    break;
            }
        }

        if ((found & 0x01) == 0) pWeap->heighRange[0] = pWeap->heighRange[1] = INT32_MIN;
        if ((found & 0x02) == 0) pWeap->heighRange[2] = pWeap->heighRange[3] = INT32_MAX;
    }
    else
    {
        switch (ParseRange(str, kValUfix, range))
        {
            case 2:
                pWeap->heighRange[0] = -range[1];
                pWeap->heighRange[1] = -range[0];
                pWeap->heighRange[2] =  range[0];
                pWeap->heighRange[3] =  range[1];
                break;
            case 1:
                pWeap->heighRange[0] = -range[0];
                pWeap->heighRange[1] =  0;
                pWeap->heighRange[2] =  0;
                pWeap->heighRange[3] =  range[0];
                break;
            default:
                return false;
        }
    }

    for (i = 0; i < 4; i++)
    {
        if (rngok(pWeap->heighRange[i], INT32_MIN + 1, INT32_MAX)) pWeap->heighRange[i] <<= 4;
    }

    return true;
}

char CUSTOMDUDEV2_SETUP::ParseWeaponCooldown(const char* str, CUSTOMDUDE_WEAPON* pWeap)
{
    CUSTOMDUDE_WEAPON::COOLDOWN* pCool = &pWeap->cooldown;
    int i = 0, nVal, data[2];

    if (isempty(str))
        return false;

    if ((nVal = getArrayType(str)) == 2)
    {
        while ((i = enumStr(i, str, key, val)) != 0)
        {
            switch (FindParam(key, gParamWeaponCooldown))
            {
                case kParWeaponCooldownTime:
                    ParseTimer(val, &pCool->delay);
                    break;
                case kParWeaponCooldownCount:
                    data[0] = 1, data[1] = 16384;
                    switch (ParseRange(val, kValUfix, data))
                    {
                        case 2:
                            pCool->totalUseCountRng[0] = (unsigned short)data[0];
                            pCool->totalUseCountRng[1] = (unsigned short)data[1];
                            break;
                        case 1:
                            pCool->totalUseCountRng[0] = (unsigned short)data[0];
                            pCool->totalUseCountRng[1] = 0;
                            break;
                    }
                    pCool->totalUseCount = pCool->totalUseCountRng[0];
                    break;
            }
        }

        return true;
    }

    switch (ParseIDs(pValue, kValUfix, data, 2))
    {
        case 2:
            nVal = CheckRange(pParam->text, data[0], 1, 32767);
            pCool->delay.rng[0] = nVal;
            pCool->delay.rng[1] = 0;
                                
            nVal = CheckRange(pParam->text, data[1], 1, 16384);
            pCool->totalUseCountRng[0] = pCool->totalUseCount = nVal;
            pCool->totalUseCountRng[1] = 0;
            break;
        case 1:
            pCool->delay.rng[0] = CheckRange(pParam->text, data[0], 1, 32767);
            pCool->delay.rng[1] = 0;
            break;
        default:
            return false;
    }

    return true;
}

char CUSTOMDUDEV2_SETUP::ParseAttackSetup(const char* str, CUSTOMDUDE_WEAPON* pWeap)
{
    int i = 0, j, nPar, nVal;
    while ((i = enumStr(i, str, key, val)) != 0)
    {
        nPar = FindParam(key, gParamAttack);
        switch (nPar)
        {
            case kParAttackTime:
                nVal = CheckValue(val, kValUfix, 0, 32767);
                for (j = 0; j < kCdudePostureMax; j++)
                    pDude->states[pWeap->stateID][j].stateTicks = nVal;
                break;
            case kParAttackInterrupt:
            case kParAttackTurn2Target:
                nVal = CheckValue(val, kValBool, false);
                switch (nPar)
                {
                    case kParAttackInterrupt:     pWeap->interruptable = nVal;    break;
                    case kParAttackTurn2Target:   pWeap->turnToTarget  = nVal;    break;
                }
                break;
            case kParAttackNumShots:
                pWeap->numshots = CheckValue(val, kValUfix, 1, 63);
                break;
            case kParAttackInertia:
                pWeap->inertia = CheckValue(val, kValBool, false);
                break;
            case kParAttackPredict:
                ParseWeaponPrediction(val, pWeap);
                break;
        }
    }

    return true;
}


char CUSTOMDUDEV2_SETUP::ParsePosture(const char* str)
{
    char nRetn = 0;
    int i, nVal;

    i = 0;
    while (i < kCdudePostureMax && enumStr(i++, str, key))
    {
        nVal = FindParam(key, gParamPosture);
        switch (nVal)
        {
            case kCdudePostureL:    nRetn |= 0x01;   break;
            case kCdudePostureC:    nRetn |= 0x02;   break;
            case kCdudePostureW:    nRetn |= 0x04;   break;
            case kCdudePostureF:    nRetn |= 0x08;   break;
        }
    }

    return nRetn;
}

char CUSTOMDUDEV2_SETUP::ParseOnEventDmg(const char* str, int* pOut, int nLen)
{
    int i;
    if (isarray(str))
    {
        i = 0;
        while (i < nLen && enumStr(i, str, key))
        {
            switch (i)
            {
                case 0: pOut[i] = CheckValue(key, kValUfix, 0, 16384) << 4;                  break;
                case 1: pOut[i] = perc2val(CheckValue(key, kValPerc,  0, 100), kChanceMax);  break;
                case 2: pOut[i] = CheckValue(key, kValUfix, 0, 2048);                        break;
            }

            i++;
        }

        return true;
    }
    
    Warning(GetError(kErrInvalidValType), str, GetValType(kValArrC));
    return false;
}

char CUSTOMDUDEV2_SETUP::ParseOnEventDmgEx(const char* str, CUSTOMDUDE_EVENT_DAMAGE* pOut)
{
    int oldOnEventDmg[3], range[2];
    int nArrType;
    int i = 0;

    Bmemset(oldOnEventDmg, 0, sizeof(oldOnEventDmg));

    if ((nArrType = getArrayType(str)) == 1) // common array (deprecated)
    {
        if (ParseOnEventDmg(str, oldOnEventDmg, 3))
        {
            pOut->hitcount      = 1;
            pOut->amount        = oldOnEventDmg[0];
            pOut->chance        = oldOnEventDmg[1];
            pOut->statetime[0]  = oldOnEventDmg[2];
            pOut->statetime[1]  = 0;
        }

        return 1;
    }
    
    if (nArrType == 2) // assoc array
    {
        pOut->hitcount = 0;
        
        while ((i = enumStr(i, str, key, val)) != 0)
        {
            switch (FindParam(key, gParamEventDmg))
            {
                case kParEvDmgAmount:       pOut->amount = CheckValue(val, kValUfix, 0, 16384) << 4;                    break;
                case kParEvDmgChance:       pOut->chance = perc2val(CheckValue(val, kValPerc,  0, 100), kChanceMax);    break;
                case kParEvDmgHealth:       pOut->health = CheckValue(val, kValPerc,  0, 255);                          break;
                case kParEvDmgCumulative:   pOut->cumulative = CheckValue(val, kValBool, 0, 1, 1);                      break;
                case kParEvDmgHitCount:     pOut->hitcount = CheckValue(val, kValBool, 0, 1, 1);                        break;
                case kParEvDmgCooldown:     ParseTimer(val, &pOut->cooldown);                                           break;
                case kParEvDmgTime:
                    range[0] = 0, range[1] = 32767;
                    switch (ParseRange(val, kValUfix, range))
                    {
                        case 2:
                            pOut->statetime[0] = (unsigned short)range[0];
                            pOut->statetime[1] = (unsigned short)range[1];
                            break;
                        case 1:
                            pOut->statetime[0] = (unsigned short)range[0];
                            pOut->statetime[1] = 0;
                            break;
                    }
                    break;
            }
        }

        return 2;
    }
    
    Warning(GetError(kErrInvalidValType), str, GetValType(kValArrA));
    return 0;
}

void CUSTOMDUDEV2_SETUP::Setup(void)
{
    dassert(pDude != NULL);
    dassert(pIni != NULL);

    pGroup = gParGroup;
    while (pGroup->id != kParamMax)
    {
        switch (pGroup->id)
        {
            case kParGroupGeneral:       SetupGeneral();       break;
            case kParGroupVelocity:      SetupVelocity();      break;
            case kParGroupAnimation:     SetupAnimation();     break;
            case kParGroupSound:         SetupSound();         break;
            case kParGroupDamage:        SetupDamage();        break;
            case kParGroupRecoil:        SetupRecoil();        break;
            case kParGroupDodge:         SetupDodge();         break;
            case kParGroupKnockout:      SetupKnockout();      break;
            case kParGroupCrouch:        SetupCrouch();        break;
            case kParGroupWeapon:        SetupWeapons();       break;
            case kParGroupFXEffect:      SetupEffect();        break;
            case kParGroupMovePat:       SetupMovePattern();   break;
            case kParGroupDropItem:      SetupDropItem();      break;
            case kParGroupFlyPat:        SetupFlyPattern();    break;
            case kParGroupMorph:         SetupMorphing();      break;
            case kParGroupSleep:         SetupSleeping();      break;
            case kParGroupSlaves:        SetupSlaves();        break;
            case kParGroupRandomness:    SetupRandomness();    break;
            case kParGroupTweaks:        SetupTweaks();        break;
        }

        pGroup++;
    }
}


/*************************************************************************************************/


void CUSTOMDUDEV1_SETUP::DamageScaleToWeapon(CUSTOMDUDE_WEAPON* pWeapon)
{
    CUSTOMDUDE_DAMAGE* pDmg = &pDude->damage;
    
    switch (pWeapon->type)
    {
        case kCdudeWeaponKamikaze:
            pDmg->Set(1024, kDmgBurn);
            pDmg->Set(1024, kDmgExplode);
            pDmg->Set(1024, kDmgElectric);
            break;
        case kCdudeWeaponMissile:
        case kCdudeWeaponThrow:
            switch (pWeapon->id)
            {
                case kMissileButcherKnife:
                    pDmg->Set(100, kDmgBullet);
                    pDmg->Set(32, kDmgSpirit);
                    break;
                case kMissileLifeLeechAltNormal:
                case kMissileLifeLeechAltSmall:
                case kMissileArcGargoyle:
                    pDmg->Dec(32, kDmgSpirit);
                    pDmg->Set(52, kDmgElectric);
                    break;
                case kMissileFlareRegular:
                case kMissileFlareAlt:
                case kMissileFlameSpray:
                case kMissileFlameHound:
                case kThingArmedSpray:
                case kThingPodFireBall:
                case kThingNapalmBall:
                    pDmg->Set(32, kDmgBurn);
                    break;
                case kThingDroppedLifeLeech:
                case kModernThingEnemyLifeLeech:
                    pDmg->Set(32, kDmgSpirit);
                    break;
                case kMissileFireball:
                case kMissileFireballNapalm:
                case kMissileFireballCerberus:
                case kMissileFireballTchernobog:
                    pDmg->Set(50, kDmgBurn);
                    pDmg->Dec(32, kDmgExplode);
                    pDmg->Set(65, kDmgFall);
                    break;
                case kThingTNTBarrel:
                case kThingArmedProxBomb:
                case kThingArmedRemoteBomb:
                case kThingArmedTNTBundle:
                case kThingArmedTNTStick:
                case kModernThingTNTProx:
                    pDmg->Dec(32, kDmgBurn);
                    pDmg->Dec(32, kDmgExplode);
                    pDmg->Set(65, kDmgFall);
                    break;
                case kMissileTeslaAlt:
                case kMissileTeslaRegular:
                    pDmg->Set(32, kDmgElectric);
                    break;
            }
            break;
    }
}


void CUSTOMDUDEV1_SETUP::DamageScaleToSurface(int nSurface)
{
    CUSTOMDUDE_DAMAGE* pDmg = &pDude->damage;

    switch (nSurface)
    {
        case kSurfStone:
            pDmg->Set(0, kDmgFall);       pDmg->Dec(200, kDmgBullet);
            pDmg->Dec(100, kDmgBurn);     pDmg->Dec(80, kDmgExplode);
            pDmg->Inc(30, kDmgChoke);     pDmg->Inc(20, kDmgElectric);
            break;
        case kSurfMetal:
            pDmg->Dec(16, kDmgFall);      pDmg->Dec(128, kDmgBullet);
            pDmg->Dec(90, kDmgBurn);      pDmg->Dec(55, kDmgExplode);
            pDmg->Inc(20, kDmgChoke);     pDmg->Inc(30, kDmgElectric);
            break;
        case kSurfWood:
            pDmg->Dec(10, kDmgBullet);    pDmg->Inc(50, kDmgBurn);
            pDmg->Inc(40, kDmgExplode);   pDmg->Inc(10, kDmgChoke);
            pDmg->Dec(60, kDmgElectric);
            break;
        case kSurfWater:
        case kSurfDirt:
        case kSurfClay:
        case kSurfGoo:
            pDmg->Set(8, kDmgFall);       pDmg->Dec(20, kDmgBullet);
            pDmg->Dec(200, kDmgBurn);     pDmg->Dec(60, kDmgExplode);
            pDmg->Set(0, kDmgChoke);      pDmg->Inc(40, kDmgElectric);
            break;
        case kSurfSnow:
        case kSurfIce:
            pDmg->Set(8, kDmgFall);       pDmg->Dec(20, kDmgBullet);
            pDmg->Dec(100, kDmgBurn);     pDmg->Dec(50, kDmgExplode);
            pDmg->Set(0, kDmgChoke);      pDmg->Inc(40, kDmgElectric);
            break;
        case kSurfLeaves:
        case kSurfPlant:
        case kSurfCloth:
            pDmg->Set(0, kDmgFall);       pDmg->Dec(10, kDmgBullet);
            pDmg->Inc(70, kDmgBurn);      pDmg->Inc(50, kDmgExplode);
            break;
        case kSurfLava:
            pDmg->Set(0, kDmgBurn);
            pDmg->Set(0, kDmgExplode);
            pDmg->Inc(30, kDmgChoke);
            break;
    }
}

void CUSTOMDUDEV1_SETUP::WeaponMeleeSet(CUSTOMDUDE_WEAPON* pWeapon)
{
    char availStatus = pWeapon->available;
    pWeapon->Clear();
    
    pWeapon->type           = kCdudeWeaponHitscan;
    pWeapon->id             = kVectorGenDudePunch;
    pWeapon->stateID        = kCdudeStateAttackBase + 2;
    pWeapon->angle          = kAng90;
    pWeapon->clipMask       = CLIPMASK1;
    pWeapon->available      = availStatus;

    WeaponRangeSet(pWeapon, 0, 512);

    AISTATE* pState = pDude->states[pWeapon->stateID];
    for (int i = 0; i < kCdudePostureMax; i++)
    {
        if (!helperSeqTriggerExists(pState->seqId))
        {
            Seq* pSeq = helperSeqLock(pState->seqId);
            if (pSeq)
                pSeq->frames[pSeq->nFrames - 1].at5_5 = 1;
        }

        pState++;
    }
}

void CUSTOMDUDEV1_SETUP::WeaponConvert(int nWeaponID)
{
    CUSTOMDUDE_WEAPON* pW1 = &pDude->weapons[0];
    CUSTOMDUDE_WEAPON* pW2 = &pDude->weapons[1];
    char availStatus = pW1->available;

    pW1->Clear();
    if (rngok(nWeaponID, 1, kVectorMax))                                       pW1->type = kCdudeWeaponHitscan,     pW1->clipMask = CLIPMASK1;
    else if (rngok(nWeaponID, kDudeBase, kDudeMax))                            pW1->type = kCdudeWeaponSummon,      pW1->clipMask = CLIPMASK0;
    else if (rngok(nWeaponID, kMissileBase, kMissileMax))                      pW1->type = kCdudeWeaponMissile,     pW1->clipMask = CLIPMASK0;
    else if (rngok(nWeaponID, kThingBase, kThingMax))                          pW1->type = kCdudeWeaponThrow,       pW1->clipMask = CLIPMASK0;
    else if (rngok(nWeaponID, kTrapExploder, kTrapExploder + kExplodeMax))     pW1->type = kCdudeWeaponKamikaze,    pW1->clipMask = CLIPMASK0;
    else                                                                       pW1->type = kCdudeWeaponNone;

    pW1->id                         = nWeaponID;
    pW1->available                  = availStatus;
    pDude->numWeapons               = 1;

    pW1->ammo.SetTotal(0);  pW1->ammo.SetFull();
    WeaponDispersionSetDefault(pW1);

    if (pW1->type == kCdudeWeaponNone)
    {
        WeaponRangeSet(pW1, 0, kCdudeV1MaxAttackDist);
        pW1->angle = kAng360;
    }
    else if (pW1->type == kCdudeWeaponThrow)
    {
        pW1->angle      = kAng15;

        switch (pW1->id)
        {
            case kModernThingEnemyLifeLeech:
            case kThingDroppedLifeLeech:
                pW1->shot.slope = -5000;
                pW1->ammo.SetTotal(1); pW1->ammo.SetFull();
                fallthrough__;
            case kModernThingThrowableRock:
                pW1->stateID = kCdudeStateAttackBase + 1;
                WeaponRangeSet(pW1, 3000, 11071);
                break;
            default:
                pW1->stateID = kCdudeStateAttackBase + 1;
                WeaponRangeSet(pW1, 5000, 12264);
                break;
        }

        WeaponMeleeSet(pW2);
        pDude->numWeapons++;
    }
    else if (pW1->type == kCdudeWeaponKamikaze)
    {
        EXPLOSION* pExpl = &explodeInfo[pW1->id - kTrapExploder];
        WeaponRangeSet(pW1, 0, ClipLow(pExpl->radius, 768));
        pW1->angle      = kAng90;
    }
    else if (pW1->type == kCdudeWeaponSummon)
    {
        pW1->ammo.SetTotal(gGameOptions.nDifficulty + 1);
        pW1->ammo.SetFull();

        WeaponRangeSet(pW1, 2000, kCdudeV1MaxAttackDist);
        pW1->shot.offset.y  = pDude->pSpr->clipdist << 4;
        pW1->angle          = kAng90;

        WeaponMeleeSet(pW2);
        pDude->numWeapons++;
    }
    else if (pW1->type == kCdudeWeaponHitscan)
    {
        VECTORDATA* pVect = &gVectorData[pW1->id];
        WeaponRangeSet(pW1, 0, (pVect->maxDist > 0) ? pVect->maxDist : kCdudeV1MaxAttackDist);

        if (pVect->maxDist > 0 && pVect->maxDist <= 2048)
        {
            pW1->dispersion[0] = pW1->dispersion[1] = 0;
            pW1->angle = pDude->pInfo->periphery;
        }
        else
        {
            pW1->angle = 56;
        }
    }
    else if (pW1->type == kCdudeWeaponMissile)
    {
        pW1->angle      = 56;

        switch (pW1->id)
        {
            case kMissileFireball:
            case kMissileFireballNapalm:
            case kMissileFireballCerberus:
            case kMissileFireballTchernobog:
                WeaponRangeSet(pW1, 3000, kCdudeV1MaxAttackDist);
                WeaponMeleeSet(pW2); pDude->numWeapons++;
                break;
            case kMissileFlareAlt:
                WeaponRangeSet(pW1, 2500, kCdudeV1MaxAttackDist);
                WeaponMeleeSet(pW2); pDude->numWeapons++;
                break;
            case kMissileLifeLeechRegular:
                WeaponRangeSet(pW1, 1500, kCdudeV1MaxAttackDist);
                WeaponMeleeSet(pW2); pDude->numWeapons++;
                break;
            case kMissileFlameSpray:
            case kMissileFlameHound:
                WeaponRangeSet(pW1, 2000, 3500 + (gGameOptions.nDifficulty * 400));
                WeaponMeleeSet(pW2); pDude->numWeapons++;
                pW1->medium = kParMediumLand;
                break;
            default:
                WeaponRangeSet(pW1, 0, kCdudeV1MaxAttackDist);
                break;
        }
    }
}

void CUSTOMDUDEV1_SETUP::SetupBasics(void)
{
    spritetype* pSpr = pDude->pSpr;
    SPRITEMASS* pMass = &gSpriteMass[pSpr->index];

    if (!pDude->initialized)
    {
        // setup mass
        Bmemset(pMass, 0, sizeof(SPRITEMASS));                  // clear mass cache

        int nPic = pSpr->picnum;
        pSpr->picnum = helperGetFirstPic(pDude);                // we need a proper pic to get a proper mass
        pDude->mass = ClipLow(getSpriteMassBySize(pSpr), 40);   // count mass
        pSpr->picnum = nPic;
    }

    // auto clipdist
    pSpr->clipdist = ClipRange((pSpr->xrepeat + pSpr->yrepeat) >> 1, 10, 128); 

}

void CUSTOMDUDEV1_SETUP::SetupDamage(void)
{
    CUSTOMDUDE_EVENT_DAMAGE* pREvn = pDude->recoil.onDamage;
    CUSTOMDUDE_EVENT_DAMAGE* pDEvn = pDude->dodge.onDamage;
    CUSTOMDUDE_WEAPON* pWeap = &pDude->weapons[0];
    char isMelee = false;
    int i;

    Bmemset(&pDude->dodge,    0, sizeof(pDude->dodge));
    Bmemset(&pDude->recoil,   0, sizeof(pDude->recoil));
    Bmemset(&pDude->knockout, 0, sizeof(pDude->knockout));
    Bmemset(&pDude->crouch,   0, sizeof(pDude->crouch));

    pREvn->cooldown.rng[0] = kCdudeDmgCheckDelay;
    pREvn->health            = 0;
    pREvn->hitcount          = 1;
    pREvn->cumulative        = 1;

    // this is same for dodge
    Bmemcpy(pDEvn, pREvn, sizeof(*pDEvn));

    DamageSetDefault();
    DamageScaleToWeapon(pWeap);
    DamageScaleToSurface(surfType[helperGetFirstPic(pDude)]);
    DamageScaleToSkill(gGameOptions.nDifficulty);

    switch (pWeap->type)
    {
        case kCdudeWeaponHitscan:
            if (pWeap->GetDistance() > 2048) break;
            fallthrough__;
        case kCdudeWeaponKamikaze:
            isMelee = true;
            break;
    }

    if (isMelee)
    {
        // no dodge
        Bmemset(&pDude->dodge, 0, sizeof(pDude->dodge));

        // more dmg and lower chances (isMelee flag analogue)
        pREvn->amount = 25, pREvn->chance = 0x0400;
    }
    else
    {
        // average values so we don't have to keep the old code
        pDEvn->chance = ClipRange((0x6000 / pDude->mass) << 7, 0, kChanceMax);
        pDEvn->amount = 10;

        pREvn->chance = ClipRange((0x8000 / pDude->mass) << 7, 0, kChanceMax);
        pREvn->amount = 15;
    }

    i = kDmgMax;
    while (--i > 0) // fill other damage types
    {
        Bmemcpy(&pDude->recoil.onDamage[i], pREvn, sizeof(*pREvn));
        Bmemcpy(&pDude->dodge.onDamage[i],  pDEvn, sizeof(*pDEvn));
    }
}

void CUSTOMDUDEV1_SETUP::SetupIncarnation(void)
{
    int i;
    spritetype* pSpr = pDude->pSpr;
    XSPRITE* pXSpr = pDude->pXSpr;
    int nDude = -1;

    if (!pDude->initialized)
    {
        // first make dudes with matching RX to be inactive
        for (i = headspritestat[kStatDude]; i >= 0; i = nextspritestat[i])
        {
            spritetype* pSpr2 = &sprite[i];
            if (pSpr2->index != pSpr->index && xspriRangeIsFine(pSpr2->extra) && IsDudeSprite(pSpr2))
            {
                XSPRITE* pXSpr2 = &xsprite[pSpr2->extra];
                if (pXSpr2->rxID == pXSpr->txID)
                {
                    ChangeSpriteStat(pSpr2->index, kStatInactive);
                    seqKill(OBJ_SPRITE, pSpr2->extra);
                    i = headspritestat[kStatDude];
                }
            }
        }
    }

    // then search
    for (i = headspritestat[kStatInactive]; i >= 0; i = nextspritestat[i])
    {
        spritetype* pSpr2 = &sprite[i];
        if (pSpr2->index != pSpr->index && xspriRangeIsFine(pSpr2->extra) && IsDudeSprite(pSpr2))
        {
            XSPRITE* pXSpr2 = &xsprite[pSpr2->extra];
            if (pXSpr2->rxID == pXSpr->txID)
            {
                nDude = pSpr2->index;
                if (nnExtRandom(0, 6) == 3) // random stop
                    break;
            }
        }
    }
    
    // fill all other damage types
    for (i = 0; i < kDmgMax; i++) pDude->morph.id[i] = nDude;
}

void CUSTOMDUDEV1_SETUP::Setup(void)
{
    dassert(pDude != NULL);

    XSPRITE* pXSpr = pDude->pXSpr;
    int nBaseSeq = (pXSpr->data2 <= 0) ? kCdudeDefaultSeq : pXSpr->data2;

    /* ---->> PROPER CALL ORDER MATTERS <<---- */

    AnimationConvert(nBaseSeq);             // convert SEQ order from old to a new system
    WeaponConvert(pXSpr->data1);            // convert weapon and give a second one if required
    SoundConvert(pXSpr->sysData1);          // convert RAW order from old to a new system
    VelocitySetDefault(146603);             // default velocity
    SetupBasics();                          // clipdist, mass
    SetupDamage();                          // damage scale, dodge and recoil chances
    
    if (pXSpr->txID)
        SetupIncarnation();
}


static void callbackSeqCustom(int, int xIndex)
{
    if (xspriRangeIsFine(xIndex))
    {
        XSPRITE* pXSpr = &xsprite[xIndex];
        spritetype* pSpr = &sprite[pXSpr->reference];
        int nID = pXSpr->sysData4;

        if (rngok(nID, 0, LENGTH(gCdudeCustomCallback)))
        {
            int nFunc = gCdudeCustomCallback[nID];
            if (Chance(0x08000))
                evKill(pXSpr->reference, OBJ_SPRITE, (CALLBACK_ID)nFunc);

            evPost(pXSpr->reference, OBJ_SPRITE, 0, (CALLBACK_ID)nFunc);
        }
        else if (rngok(nID, kCdudeGIBEffectBase, kCdudeGIBEffectBase + kGibMax))
        {
            CGibPosition gibPos(pSpr->x, pSpr->y, pSpr->z);
            nID -= kCdudeGIBEffectBase;

            GibSprite(pSpr, (GIBTYPE)nID, &gibPos, NULL);
        }
        else if (rngok(nID, kCdudeFXEffectBase, kCdudeFXEffectBase + kFXMax))
        {
            nID -= kCdudeFXEffectBase;
            gFX.fxSpawn((FX_ID)nID, pSpr->sectnum, pSpr->x, pSpr->y, pSpr->z);
        }
    }
}


static DICTNODE* helperSeqExists(int nSeq) { return (nSeq > 0) ? gSysRes.Lookup(nSeq, "SEQ")   : NULL; }
static DICTNODE* helperSndExists(int nSnd) { return (nSnd > 0) ? gSoundRes.Lookup(nSnd, "SFX") : NULL; }
static int helperGetFirstPic(CUSTOMDUDE* pDude)
{
    spritetype* pSpr = pDude->pSpr;

    int nPic = pSpr->picnum;
    int nSeq = pDude->GetStateSeq(kCdudeStateIdle, kCdudePostureL);
    if (helperSeqExists(nSeq))
    {
        Seq* pSeq = helperSeqLoad(nSeq);
        if (pSeq)
        {
            SEQFRAME* pFrame = &pSeq->frames[0];
            nPic = pFrame->tile + (pFrame->tile2 << 12);
        }
    }

    return nPic;
}

static char helperSeqTriggerExists(int nSeq)
{
    int i;
    Seq* pSeq = helperSeqLoad(nSeq);
    if (pSeq)
    {
        i = pSeq->nFrames;
        while (--i >= 0)
        {
            if (pSeq->frames[i].at5_5)
                return true;
        }
    }

    return false;
}

static Seq* helperSeqLoad(int nSeq)
{
    DICTNODE* hSeq = helperSeqExists(nSeq);
    if (hSeq)
        return (Seq*)gSysRes.Load(hSeq);

    return NULL;
}

static Seq* helperSeqLock(int nSeq)
{
    DICTNODE* hSeq = helperSeqExists(nSeq);
    if (hSeq)
    {
        if (!hSeq->lockCount)
            return (Seq*)gSysRes.Lock(hSeq);

        return (Seq*)gSysRes.Load(hSeq);
    }

    return NULL;
}

static char isIdKeyword(const char* fullStr, const char* prefix, int* nID)
{
    if (!fullStr || !prefix)
        return false;
    
    int l1 = Bstrlen(fullStr);
    int l2 = Bstrlen(prefix);

    if (l2 < l1 && Bstrncasecmp(fullStr, prefix, l2) == 0)
    {
        while (fullStr[l2] == '_')
        {
            if (++l2 >= l1)
                return false;
        }
        
        if (isufix(&fullStr[l2]))
        {
            if (nID)
                *nID = atoi(&fullStr[l2]);

            return true;
        }
    }

    return false;
}

char getArrayType(const char* str, int* nLen)
{
    if (isarray(str, nLen))
        return Bstrchr(str, '=') ? 2 : 1;

    return 0;
}

CUSTOMDUDE* cdudeAlloc()
{
    if (!gCustomDude)
    {
        gCustomDude = (CUSTOMDUDE*)Bmalloc(sizeof(CUSTOMDUDE) * kMaxSprites);
        
        dassert(gCustomDude != NULL);
        Bmemset(gCustomDude, 0, sizeof(CUSTOMDUDE) * kMaxSprites);
    }

    return gCustomDude;
}

void cdudeFree()
{
    CUSTOMDUDE* pDude;
    int i;
    
    if (gCustomDude)
    {
        for (i = 0; i < kMaxSprites; i++)
        {
            pDude = &gCustomDude[i];
            if (pDude->initialized)
                pDude->Clear();
        }
        
        Bfree(gCustomDude);
    }

    gCustomDude = NULL;
}

CUSTOMDUDE* cdudeGet(int nIndex)
{
    dassert(spriRangeIsFine(nIndex));
    dassert(xspriRangeIsFine(sprite[nIndex].extra));
    return &gCustomDude[nIndex];
}


// for kModernCustomDudeSpawn markers
spritetype* cdudeSpawn(XSPRITE* pXSource, spritetype* pSprite, int nDist)
{
    POINT3D offs;
    Bmemset(&offs, 0, sizeof(offs));
    offs.y = nDist;

    spritetype* pSource = &sprite[pXSource->reference];
    spritetype* pDude = nnExtSpawnDude(pSprite, kDudeModernCustom, pSprite->x, pSprite->y, pSprite->z);

    if (pDude)
    {
        nnExtOffsetSprite(pDude, 0, offs.y, 0);
        XSPRITE* pXDude = &xsprite[pDude->extra];

        // inherit weapon, seq and sound settings.
        pXDude->data1 = pXSource->data1;
        pXDude->data2 = pXSource->data2;
        pXDude->data3 = pXDude->sysData1 = pXSource->data3; // move sndStartId from data3 to sysData1
        pXDude->data4 = pXDude->sysData2 = pXSource->data4; // health

        // inherit custom hp settings
        pXDude->health = nnExtDudeStartHealth(pDude, pXSource->data4);

        // inherit movement speed.
        pXDude->busyTime = pXSource->busyTime;

        // inherit clipdist?
        if (pSource->clipdist > 0)
            pDude->clipdist = pSource->clipdist;

        if (pSource->flags & kModernTypeFlag1)
        {
            switch (pSource->type) {
            case kModernCustomDudeSpawn:
                //inherit pal?
                if (pDude->pal <= 0) pDude->pal = pSource->pal;

                // inherit spawn sprite trigger settings, so designer can count monsters.
                pXDude->txID        = pXSource->txID;
                pXDude->command     = pXSource->command;
                pXDude->triggerOn   = pXSource->triggerOn;
                pXDude->triggerOff  = pXSource->triggerOff;

                // inherit drop items
                pXDude->dropMsg = pXSource->dropMsg;

                // inherit required key so it can be dropped
                pXDude->key = pXSource->key;

                // inherit dude flags
                pXDude->dudeDeaf    = pXSource->dudeDeaf;
                pXDude->dudeGuard   = pXSource->dudeGuard;
                pXDude->dudeAmbush  = pXSource->dudeAmbush;
                pXDude->dudeFlag4   = pXSource->dudeFlag4;
                pXDude->unused1     = pXSource->unused1;
                break;
            }
        }

        // inherit sprite size (useful for seqs with zero repeats)
        if (pSource->flags & kModernTypeFlag2)
        {
            pDude->xrepeat = pSource->xrepeat;
            pDude->yrepeat = pSource->yrepeat;
        }

        gKillMgr.AddCount(pDude);
        aiInitSprite(pDude);
    }

    return pDude;
}


// for kThingModernEnemyLifeLeech
void cdudeLeechOperate(spritetype* pSpr, XSPRITE* pXSpr)
{
    if (spriRangeIsFine(pSpr->owner))
    {
        spritetype* pOwn = &sprite[pSpr->owner];
        if (xsprIsFine(pOwn) && IsDudeSprite(pOwn))
        {
            XSPRITE* pXOwn = &xsprite[pOwn->extra];
            if (spriRangeIsFine(pXOwn->target))
            {
                pXSpr->target = pXOwn->target;
            }
            else if (spriRangeIsFine(pXSpr->target))
            {
                spritetype* pTarget = &sprite[pXSpr->target];
                aiSetTarget(pXOwn, pTarget->x, pTarget->y, pTarget->z);
                aiActivateDude(pOwn, pXOwn);
            }
        }
    }

    int nTarget = pXSpr->target;
    if (spriRangeIsFine(nTarget) && nTarget != pSpr->owner)
    {
        spritetype* pTarg = &sprite[nTarget];
        if (pTarg->statnum == kStatDude && xsprIsFine(pTarg) && !pXSpr->stateTimer)
        {
            if (IsPlayerSprite(pTarg))
            {
                PLAYER* pPlayer = &gPlayer[pTarg->type - kDudePlayer1];
                if (powerupCheck(pPlayer, kPwUpShadowCloak) > 0)
                    return;
            }

            int x = pTarg->x;
            int y = pTarg->y;
            int z = pTarg->z;

            int zTop, zBot;
            GetSpriteExtents(pSpr, &zTop, &zBot);
            int nDist = approxDist(x - pSpr->x, y - pSpr->y);
            if (nDist && cansee(pSpr->x, pSpr->y, zTop, pSpr->sectnum, x, y, z, pTarg->sectnum))
            {
                int t = divscale12(nDist, 0x1aaaaa);
                x += (xvel[nTarget] * t) >> 12;
                y += (yvel[nTarget] * t) >> 12;
                int nAng = getangle(x - pSpr->x, y - pSpr->y);
                int dx = Cos(nAng) >> 16;
                int dy = Sin(nAng) >> 16;
                int dz = divscale10(pTarg->z - zTop, nDist);
                
                int nMissileType = kMissileLifeLeechAltNormal + (pXSpr->data3 ? 1 : 0);
                int t2;

                if (!pXSpr->data3) t2 = 120 / 10.0;
                else t2 = (3 * 120) / 10.0;

                spritetype* pMissile = actFireMissile(pSpr, 0, zTop - pSpr->z - 384, dx, dy, dz, nMissileType);
                if (pMissile)
                {
                    pMissile->owner = pSpr->owner;
                    pXSpr->stateTimer = 1;
                    evPost(pSpr->index, 3, t2, kCallbackLeechStateTimer);
                    pXSpr->data3 = ClipLow(pXSpr->data3 - 1, 0);
                }
            }
        }
    }
}

void cdudeSave(LoadSave* pSave)
{
    int16_t na; uint8_t nw;
    CUSTOMDUDE* pDude;
    int i, j;

    for (i = headspritestat[kStatDude]; i >= 0; i = nextspritestat[i])
    {
        if (!IsCustomDude(&sprite[i]))
            continue;

        pDude = cdudeGet(i);

        pSave->Write(&i, sizeof(i));            // save sprite index
        nw = (uint8_t)pDude->numWeapons;
        pSave->Write(&nw, sizeof(nw));          // then number of weapons it have

        for (j = 0; j < nw; j++)
        {
            na = (int16_t)pDude->weapons[j].ammo.cur;
            pSave->Write(&na, sizeof(na));      // then cur ammo
        }
    }

    pSave->Write(&i, sizeof(i));                // then dude EOL

}

void cdudeLoad(LoadSave* pLoad)
{
    int16_t na; uint8_t nw;
    CUSTOMDUDE* pDude;
    int i, j;

    while ( 1 )
    {
        pLoad->Read(&i, sizeof(i));             // read sprite index
        if (i < 0)                              // check for EOL
            break;

        pDude = cdudeGet(i);
        pLoad->Read(&nw, sizeof(nw));           // then number of weapons it have    
        
        for (j = 0; j < nw; j++)
        {
            pLoad->Read(&na, sizeof(na));       // then cur ammo
            pDude->weapons[j].ammo.cur = na;
        }
    }
}

#endif
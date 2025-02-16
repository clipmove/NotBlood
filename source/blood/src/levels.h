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
#include "inifile.h"

#define kMaxMessages 32
#define kMaxEpisodes 7
#ifdef NOONE_EXTENSIONS
  // allow advanced mods to use more than 16 levels.
  // for example, Blood: What Lies Beneath has 43 (as of version 1.1)
  #define kMaxLevels 64
#else // original blood only allowed 16 levels per episode
  #define kMaxLevels 16
#endif

#pragma pack(push, 1)

struct GAMEOPTIONS {
    char nGameType;
    char nDifficulty;
    int nEpisode;
    int nLevel;
    char zLevelName[BMAX_PATH];
    char zLevelSong[BMAX_PATH];
    int nTrackNumber; //at12a;
    char szSaveGameName[BMAX_PATH];
    char szUserGameName[BMAX_PATH];
    short nSaveGameSlot;
    int picEntry;
    unsigned int uMapCRC;
    char nMonsterSettings;
    int uGameFlags;
    int uNetGameFlags;
    char nWeaponSettings;
    char nItemSettings;
    char nRespawnSettings;
    char nTeamSettings;
    int nMonsterRespawnTime;
    int nWeaponRespawnTime;
    int nItemRespawnTime;
    int nSpecialRespawnTime;
    bool bQuadDamagePowerup;
    int nDamageInvul;
    int nExplosionBehavior;
    int nProjectileBehavior;
    bool bNapalmFalloff;
    int nEnemyBehavior;
    bool bEnemyRandomTNT;
    int nWeaponsVer;
    bool bSectorBehavior;
    char nHitscanProjectiles;
    char nRandomizerMode;
    char szRandomizerSeed[9];
    int nRandomizerCheat;
    int nEnemyQuantity;
    int nEnemyHealth;
    int nEnemySpeed;
    bool bEnemyShuffle;
    bool bPitchforkOnly;
    bool bPermaDeath;
    bool bFriendlyFire;
    char nKeySettings;
    char bItemWeaponSettings;
    char bAutoTeams;
    char nSpawnProtection;
    char nSpawnWeapon;
    unsigned int uSpriteBannedFlags;
    char szUserMap[BMAX_PATH];
};

#pragma pack(pop)

enum {
    kLevelExitNormal = 0,
    kLevelExitSecret
};

enum {
    MUS_FIRST_SPECIAL = kMaxEpisodes*kMaxLevels,

    MUS_INTRO = MUS_FIRST_SPECIAL,
    MUS_LOADING = MUS_FIRST_SPECIAL + 1,
};

struct LEVELINFO
{
    char Filename[BMAX_PATH];
    char Title[32];
    char Author[32];
    char Song[BMAX_PATH];
    int SongId;
    int EndingA;
    int EndingB;
    char Messages[kMaxMessages][128];
    char Fog;
    char Weather;
}; // 0x8ee bytes

struct EPISODEINFO
{
    char title[32];
    int nLevels;
    unsigned int bloodbath : 1;
    unsigned int cutALevel : 4;
    LEVELINFO levelsInfo[kMaxLevels];
    char cutsceneASmkPath[BMAX_PATH];
    char cutsceneBSmkPath[BMAX_PATH];
    int cutsceneAWavRsrcID;
    int cutsceneBWavRsrcID;
    char cutsceneAWavPath[BMAX_PATH];
    char cutsceneBWavPath[BMAX_PATH];
};

// BANNED SPRITE TYPE FLAGS /////////////////////////////////////
enum {
    BANNED_NONE        =  0,

    // monsters
    BANNED_BATS        =  1 <<  0,
    BANNED_RATS        =  1 <<  1,
    BANNED_FISH        =  1 <<  2,
    BANNED_HANDS       =  1 <<  3,
    BANNED_GHOSTS      =  1 <<  4,
    BANNED_SPIDERS     =  1 <<  5,
    BANNED_TCALEBS     =  1 <<  6,
    BANNED_HHOUNDS     =  1 <<  7,

    // monster behavior tweaks
    BANNED_RATS_BITE   =  1 <<  8,
    BANNED_MSPIDERS_HP =  1 <<  9,
    BANNED_TCHERNOBOG_HP= 1 << 10,

    // weapons
    BANNED_FLARE       =  1 << 11,
    BANNED_SHOTGUN     =  1 << 12,
    BANNED_TOMMYGUN    =  1 << 13,
    BANNED_NAPALM      =  1 << 14,
    BANNED_TNT         =  1 << 15,
    BANNED_SPRAYCAN    =  1 << 16,
    BANNED_TESLA       =  1 << 17,
    BANNED_LIFELEECH   =  1 << 18,
    BANNED_VOODOO      =  1 << 19,
    BANNED_PROXY       =  1 << 20,
    BANNED_REMOTE      =  1 << 21,

    // items
    BANNED_MEDKIT      =  1 << 22,
    BANNED_LIFEESSENCE =  1 << 23,
    BANNED_LIFESEED    =  1 << 24,
    BANNED_SUPERARMOR  =  1 << 25,
    BANNED_CRYSTALBALL =  1 << 26,

    // powerups
    BANNED_JUMPBOOTS   =  1 << 27,
    BANNED_CLOAK       =  1 << 28,
    BANNED_DEATHMASK   =  1 << 29,
    BANNED_AKIMBO      =  1 << 30,
    BANNED_REFLECT     =  1 << 31, // max u32 flag possible
};

extern EPISODEINFO gEpisodeInfo[];
extern GAMEOPTIONS gSingleGameOptions;
extern GAMEOPTIONS gGameOptions;
extern int gSkill;
extern char BloodIniFile[];
extern char BloodIniPre[];
extern bool bINIOverride;
extern int gEpisodeCount;
extern int gNextLevel;
extern bool gGameStarted;
extern int gLevelTime;

void levelInitINI(const char *pzIni);
void levelOverrideINI(const char *pzIni);
void levelPlayIntroScene(int nEpisode);
void levelPlayEndScene(int nEpisode);
void levelClearSecrets(void);
void levelSetupSecret(int nCount);
void levelTriggerSecret(int nSecret);
void CheckSectionAbend(const char *pzSection);
void CheckKeyAbend(const char *pzSection, const char *pzKey);
LEVELINFO * levelGetInfoPtr(int nEpisode, int nLevel);
char * levelGetFilename(int nEpisode, int nLevel);
char * levelGetMessage(int nMessage);
char * levelGetTitle(void);
char * levelGetAuthor(void);
void levelSetupOptions(int nEpisode, int nLevel);
void levelLoadMapInfo(IniFile *pIni, LEVELINFO *pLevelInfo, const char *pzSection);
void levelLoadDefaults(void);
void levelAddUserMap(const char *pzMap);
// EndingA is normal ending, EndingB is secret level
void levelGetNextLevels(int nEpisode, int nLevel, int *pnEndingA, int *pnEndingB);
// arg: 0 is normal exit, 1 is secret level
void levelEndLevel(int nExitType);
void levelRestart(void);
int levelGetMusicIdx(const char *str);
bool levelTryPlayMusic(int nEpisode, int nlevel, bool bSetLevelSong = false);
void levelTryPlayMusicOrNothing(int nEpisode, int nLevel);

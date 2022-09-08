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
#include "compat.h"
#include "mmulti.h"
#include "common_game.h"
#include "fx_man.h"
#include "music.h"
#include "blood.h"
#include "demo.h"
#include "config.h"
#include "gamemenu.h"
#include "globals.h"
#include "loadsave.h"
#include "menu.h"
#include "messages.h"
#include "network.h"
#include "osdcmds.h"
#include "sfx.h"
#include "screen.h"
#include "sound.h"
#include "view.h"

void SaveGame(CGameMenuItemZEditBitmap *, CGameMenuEvent *);

void SaveGameProcess(CGameMenuItemChain *);
void ShowDifficulties();
void SetDifficultyAndStart(CGameMenuItemChain *);
void SetCustomDifficultyAndStart(CGameMenuItemChain *);
void SetDetail(CGameMenuItemSlider *);
void SetGamma(CGameMenuItemSlider *);
void SetMusicVol(CGameMenuItemSlider *);
void SetSoundVol(CGameMenuItemSlider *);
void SetCDVol(CGameMenuItemSlider *);
void SetMonoStereo(CGameMenuItemZBool *);
void SetCrosshair(CGameMenuItemZCycle *);
void SetCenterHoriz(CGameMenuItemZBool *);
void SetShowPlayerNames(CGameMenuItemZBool *);
void SetShowWeapons(CGameMenuItemZCycle *);

void SetMonsters(CGameMenuItemZCycle*);
void SetQuadDamagePowerup(CGameMenuItemZBool*);
void SetDamageInvul(CGameMenuItemZBool*);
void SetExplosionBehavior(CGameMenuItemZCycle*);
void SetProjectileBehavior(CGameMenuItemZCycle*);
void SetEnemyBehavior(CGameMenuItemZBool*);
void SetEnemyRandomTNT(CGameMenuItemZBool*);
void SetWeaponsVer(CGameMenuItemZCycle*);
void SetSectorBehavior(CGameMenuItemZBool*);
void SetHitscanProjectiles(CGameMenuItemZBool*);
void SetRandomizerMode(CGameMenuItemZCycle*);
void SetRandomizerSeed(CGameMenuItemZEdit *pItem, CGameMenuEvent *pEvent);

void SetSlopeTilting(CGameMenuItemZBool *);
void SetViewBobbing(CGameMenuItemZBool *);
void SetViewSwaying(CGameMenuItemZBool *);
void SetWeaponSwaying(CGameMenuItemZCycle *);
void SetWeaponInterpolate(CGameMenuItemZCycle *);
void SetMouseSensitivity(CGameMenuItemSliderFloat *);
void SetMouseAimFlipped(CGameMenuItemZBool *);
void SetTurnSpeed(CGameMenuItemSlider *);
void SetCrouchToggle(CGameMenuItemZBool *);
void SetAutoRun(CGameMenuItemZBool *);
void ResetKeys(CGameMenuItemChain *);
void ResetKeysClassic(CGameMenuItemChain *);
void SetMessages(CGameMenuItemZBool *);
void LoadGame(CGameMenuItemZEditBitmap *, CGameMenuEvent *);
void SetupNetLevels(CGameMenuItemZCycle *);
void NetClearUserMap(CGameMenuItemZCycle *);
void StartNetGame(CGameMenuItemChain *);
void SetParentalLock(CGameMenuItemZBool *);
void TenProcess(CGameMenuItem7EA1C *);
void SetupLevelMenuItem(int);
void SetupVideoModeMenu(CGameMenuItemChain *);
void SetVideoMode(CGameMenuItemChain *);
void SetWidescreen(CGameMenuItemZBool *);
void SetHudRatio(CGameMenuItemZCycle *);
void SetMirrorMode(CGameMenuItemZCycle *);
void SetWeaponSelectMode(CGameMenuItemZCycle *);
void SetSlowRoomFlicker(CGameMenuItemZBool *);
void SetFOV(CGameMenuItemSlider *);
void UpdateVideoModeMenuFrameLimit(CGameMenuItemZCycle *pItem);
//void UpdateVideoModeMenuFPSOffset(CGameMenuItemSlider *pItem);
void UpdateVideoColorMenu(CGameMenuItemSliderFloat *);
void UpdateVideoPaletteCycleMenu(CGameMenuItemZCycle *);
void UpdateVideoPaletteBoolMenu(CGameMenuItemZBool *);
void ResetVideoColor(CGameMenuItemChain *);
#ifdef USE_OPENGL
void SetupVideoPolymostMenu(CGameMenuItemChain *);
#endif

char strRestoreGameStrings[kMaxLoadSaveSlot][16] = 
{
    "<Empty>",
    "<Empty>",
    "<Empty>",
    "<Empty>",
    "<Empty>",
    "<Empty>",
    "<Empty>",
    "<Empty>",
    "<Empty>",
    "<Empty>",

    "<Empty>",
    "<Empty>",
};

char restoreGameDifficulty[kMaxLoadSaveSlot] = 
{
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,

    2,
    2,
};

const char *zNetGameTypes[] =
{
    "Cooperative",
    "Bloodbath",
    "Teams",
};

const char *zMonsterStrings[] =
{
    "None",
    "Bring 'em on",
    "Respawn (15 Secs)",
    "Respawn (30 Secs)",
    "Respawn (60 Secs)",
    "Respawn (90 Secs)",
    "Respawn (120 Secs)",
};

const char *zWeaponStrings[] =
{
    "Do not Respawn",
    "Are Permanent",
    "Respawn",
    "Respawn with Markers",
};

const char *zItemStrings[] =
{
    "Do not Respawn",
    "Respawn",
    "Respawn with Markers",
};

const char *zKeyStrings[] =
{
    "LOST ON DEATH",
    "KEPT ON RESPAWN",
    "SHARED",
};

const char *zSpawnProtectStrings[] =
{
    "OFF",
    "ON (1 SEC)",
    "ON (2 SEC)",
    "ON (3 SEC)",
};

const char *zSpawnWeaponStrings[] =
{
    "Pitchfork",
    "Flare Pistol",
    "Sawed-off",
    "Tommy Gun",
    "Napalm Launcher",
    "Dynamite",
    "Spray Can",
    "Tesla Cannon",
    "Life Leech",
    "Voodoo Doll",
    "Proximity TNT",
    "Remote TNT",
    "Random Weapon",
    "All Weapons"
};

const char *zRespawnStrings[] =
{
    "At Random Locations",
    "Close to Weapons",
    "Away from Enemies",
};

const char *zDiffStrings[] =
{
    "STILL KICKING",
    "PINK ON THE INSIDE",
    "LIGHTLY BROILED",
    "WELL DONE",
    "EXTRA CRISPY",
};

const char *pzCrosshairStrings[] = {
    "OFF",
    "ON",
    "ON (AUTO AIM)"
};

const char *pzShowWeaponStrings[] = {
    "OFF",
    "SPRITE",
    "VOXEL"
};

const char *pzMonsterStrings[] =
{
    "None",
    "Default",
    "Respawn (15 Secs)",
    "Respawn (30 Secs)",
    "Respawn (60 Secs)",
    "Respawn (90 Secs)",
    "Respawn (120 Secs)",
};

const char *pzExplosionBehaviorStrings[] = {
    "Original",
    "NotBlood",
    "Raze",
};

const char *pzProjectileBehaviorStrings[] = {
    "Original",
    "NotBlood",
    "Raze",
};

const char *pzWeaponsVersionStrings[] = {
    "Original",
    "NotBlood",
    "V1.0x",
};

const char *pzRandomizerModeStrings[] = {
    "Off",
    "Enemies",
    "Pickups",
    "Enemies+Pickups",
};

char zUserMapName[BMAX_PATH];
const char *zEpisodeNames[6];
const char *zLevelNames[6][16];
static char szRandomizerSeedMenu[9];

static char MenuGameFuncs[NUMGAMEFUNCTIONS][MAXGAMEFUNCLEN];
static char const *MenuGameFuncNone = "  -None-";
static char const *pzGamefuncsStrings[NUMGAMEFUNCTIONS + 1];
static int nGamefuncsValues[NUMGAMEFUNCTIONS + 1];
static int nGamefuncsNum;

CGameMenu menuMain;
CGameMenu menuMainWithSave;
CGameMenu menuNetMain;
CGameMenu menuNetStart;
CGameMenu menuEpisode;
CGameMenu menuDifficulty;
CGameMenu menuCustomDifficulty;
CGameMenu menuOptionsOld;
CGameMenu menuControls;
CGameMenu menuMessages;
CGameMenu menuKeys;
CGameMenu menuSaveGame;
CGameMenu menuLoadGame;
CGameMenu menuLoading;
CGameMenu menuSounds;
CGameMenu menuQuit;
CGameMenu menuRestart;
CGameMenu menuCredits;
CGameMenu menuOrder;
CGameMenu menuPlayOnline;
CGameMenu menuParentalLock;
CGameMenu menuSorry;
CGameMenu menuSorry2;
CGameMenu menuNetwork;
CGameMenu menuNetworkHost;
CGameMenu menuNetworkJoin;
CGameMenu menuNetworkGameEnhancements;

CGameMenuItemQAV itemBloodQAV("", 3, 160, 100, "BDRIP", true);
CGameMenuItemQAV itemCreditsQAV("", 3, 160, 100, "CREDITS", false, true);
CGameMenuItemQAV itemHelp3QAV("", 3, 160, 100, "HELP3", false, false);
CGameMenuItemQAV itemHelp3BQAV("", 3, 160, 100, "HELP3B", false, false);
CGameMenuItemQAV itemHelp4QAV("", 3, 160, 100, "HELP4", false, true);
CGameMenuItemQAV itemHelp5QAV("", 3, 160, 100, "HELP5", false, true);

CGameMenuItemTitle itemMainTitle("BLOOD", 1, 160, 20, 2038);
CGameMenuItemChain itemMain1("NEW GAME", 1, 0, 45, 320, 1, &menuEpisode, -1, NULL, 0);
//CGameMenuItemChain itemMain2("PLAY ONLINE", 1, 0, 65, 320, 1, &menuPlayOnline, -1, NULL, 0);
CGameMenuItemChain itemMain2("MULTIPLAYER", 1, 0, 65, 320, 1, &menuNetwork, -1, NULL, 0);
CGameMenuItemChain itemMain3("OPTIONS", 1, 0, 85, 320, 1, &menuOptions, -1, NULL, 0);
CGameMenuItemChain itemMain4("LOAD GAME", 1, 0, 105, 320, 1, &menuLoadGame, -1, NULL, 0);
CGameMenuItemChain itemMain5("HELP", 1, 0, 125, 320, 1, &menuOrder, -1, NULL, 0);
CGameMenuItemChain itemMain6("CREDITS", 1, 0, 145, 320, 1, &menuCredits, -1, NULL, 0);
CGameMenuItemChain itemMain7("QUIT", 1, 0, 165, 320, 1, &menuQuit, -1, NULL, 0);

CGameMenuItemTitle itemMainSaveTitle("BLOOD", 1, 160, 20, 2038);
CGameMenuItemChain itemMainSave1("NEW GAME", 1, 0, 45, 320, 1, &menuEpisode, -1, NULL, 0);
//CGameMenuItemChain itemMainSave2("PLAY ONLINE", 1, 0, 60, 320, 1, &menuPlayOnline, -1, NULL, 0);
CGameMenuItemChain itemMainSave2("OPTIONS", 1, 0, 60, 320, 1, &menuOptions, -1, NULL, 0);
CGameMenuItemChain itemMainSave3("SAVE GAME", 1, 0, 75, 320, 1, &menuSaveGame, -1, SaveGameProcess, 0);
CGameMenuItemChain itemMainSave4("LOAD GAME", 1, 0, 90, 320, 1, &menuLoadGame, -1, NULL, 0);
CGameMenuItemChain itemMainSave5("HELP", 1, 0, 105, 320, 1, &menuOrder, -1, NULL, 0);
CGameMenuItemChain itemMainSave6("CREDITS", 1, 0, 120, 320, 1, &menuCredits, -1, NULL, 0);
CGameMenuItemChain itemMainSave7("END GAME", 1, 0, 135, 320, 1, &menuRestart, -1, NULL, 0);
CGameMenuItemChain itemMainSave8("QUIT", 1, 0, 150, 320, 1, &menuQuit, -1, NULL, 0);

CGameMenuItemTitle itemEpisodesTitle("EPISODES", 1, 160, 20, 2038);
CGameMenuItemChain7F2F0 itemEpisodes[kMaxEpisodes-1];

CGameMenu menuUserMap;
CGameMenuItemChain itemUserMap("< USER MAP >", 1, 0, 60, 320, 1, &menuUserMap, 0, NULL, 0, 8);
CGameMenuItemTitle itemUserMapTitle("USER MAP", 1, 160, 20, 2038);
CGameMenuFileSelect itemUserMapList("", 3, 0, 0, 0, "./", "*.map", gGameOptions.szUserMap, ShowDifficulties, 0);

CGameMenuItemTitle itemDifficultyTitle("DIFFICULTY", 1, 160, 20, 2038);
CGameMenuItemChain itemDifficulty1("STILL KICKING", 1, 0, 55, 320, 1, NULL, -1, SetDifficultyAndStart, 0);
CGameMenuItemChain itemDifficulty2("PINK ON THE INSIDE", 1, 0, 75, 320, 1, NULL, -1, SetDifficultyAndStart, 1);
CGameMenuItemChain itemDifficulty3("LIGHTLY BROILED", 1, 0, 95, 320, 1, NULL, -1, SetDifficultyAndStart, 2);
CGameMenuItemChain itemDifficulty4("WELL DONE", 1, 0, 115, 320, 1, NULL, -1, SetDifficultyAndStart, 3);
CGameMenuItemChain itemDifficulty5("EXTRA CRISPY", 1, 0, 135, 320, 1, 0, -1, SetDifficultyAndStart, 4);
CGameMenuItemChain itemDifficultyCustom("< CUSTOM >", 1, 0, 155, 320, 1, &menuCustomDifficulty, -1, NULL, 0, 8);

CGameMenuItemTitle itemCustomDifficultyTitle("CUSTOM", 1, 160, 20, 2038);
CGameMenuItemSlider itemCustomDifficultyEnemyQuantity("ENEMIES QUANTITY:", 3, 66, 40, 180, 2, 0, 4, 1, NULL, -1, -1);
CGameMenuItemSlider itemCustomDifficultyEnemyHealth("ENEMIES HEALTH:", 3, 66, 50, 180, 2, 0, 4, 1, NULL, -1, -1);
CGameMenuItemSlider itemCustomDifficultyEnemyDifficulty("ENEMIES DIFFICULTY:", 3, 66, 60, 180, 2, 0, 4, 1, NULL, -1, -1);
CGameMenuItemSlider itemCustomDifficultyPlayerDamage("PLAYER DAMAGE TAKEN:", 3, 66, 70, 180, 2, 0, 4, 1, NULL, -1, -1);
CGameMenuItemZBool itemCustomDifficultyPitchfork("PITCHFORK START:", 3, 66, 80, 180, false, NULL, NULL, NULL);
CGameMenuItemZBool itemCustomDifficultyMonsterBanBats("BATS:", 3, 75, 93, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemCustomDifficultyMonsterBanRats("RATS:", 3, 75, 101, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemCustomDifficultyMonsterBanFish("FISH:", 3, 75, 109, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemCustomDifficultyMonsterBanHands("HANDS:", 3, 75, 117, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemCustomDifficultyMonsterBanGhosts("GHOSTS:", 3, 75, 125, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemCustomDifficultyMonsterBanSpiders("SPIDERS:", 3, 75, 133, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemCustomDifficultyMonsterBanTinyCaleb("TINY CALEBS:", 3, 75, 141, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemCustomDifficultyMonsterBanHellHounds("HELL HOUNDS:", 3, 75, 149, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemChain itemCustomDifficultyStart("START GAME", 1, 0, 161, 320, 1, NULL, -1, SetCustomDifficultyAndStart, 0);

CGameMenuItemTitle itemOptionsOldTitle("OPTIONS", 1, 160, 20, 2038);
CGameMenuItemChain itemOption1("CONTROLS...", 3, 0, 40, 320, 1, &menuControls, -1, NULL, 0);
CGameMenuItemSlider sliderDetail("DETAIL:", 3, 66, 50, 180, gDetail, 0, 4, 1, SetDetail, -1, -1);
CGameMenuItemSlider sliderGamma("GAMMA:", 3, 66, 60, 180, gGamma, 0, 15, 2, SetGamma, -1, -1);
CGameMenuItemSlider sliderMusic("MUSIC:", 3, 66, 70, 180, MusicVolume, 0, 255, 17, SetMusicVol, -1, -1);
CGameMenuItemSlider sliderSound("SOUND:", 3, 66, 80, 180, FXVolume, 0, 255, 17, SetSoundVol, -1, -1);
CGameMenuItemSlider sliderCDAudio("CD AUDIO:", 3, 66, 90, 180, CDVolume, 0, 255, 17, SetCDVol, -1, -1);
CGameMenuItemZBool bool3DAudio("3D AUDIO:", 3, 66, 100, 180, gStereo, SetMonoStereo, NULL, NULL);
CGameMenuItemZCycle itemCycleCrosshair("CROSSHAIR:", 3, 66, 110, 180, 0, SetCrosshair, pzCrosshairStrings, ARRAY_SSIZE(pzCrosshairStrings), 0);
CGameMenuItemZCycle itemCycleShowWeapons("SHOW WEAPONS:", 3, 66, 120, 180, 0, SetShowWeapons, pzShowWeaponStrings, ARRAY_SSIZE(pzShowWeaponStrings), 0);
CGameMenuItemZBool boolSlopeTilting("SLOPE TILTING:", 3, 66, 130, 180, gSlopeTilting, SetSlopeTilting, NULL, NULL);
CGameMenuItemZBool boolViewBobbing("VIEW BOBBING:", 3, 66, 140, 180, gViewVBobbing, SetViewBobbing, NULL, NULL);
CGameMenuItemZBool boolViewSwaying("VIEW SWAYING:", 3, 66, 150, 180, gViewHBobbing, SetViewSwaying, NULL, NULL);
CGameMenuItem7EE34 itemOption2("VIDEO MODE...", 3, 0, 160, 320, 1);
CGameMenuItemChain itemChainParentalLock("PARENTAL LOCK", 3, 0, 170, 320, 1, &menuParentalLock, -1, NULL, 0);

CGameMenuItemTitle itemControlsTitle("CONTROLS", 1, 160, 20, 2038);
CGameMenuItemSliderFloat sliderMouseSpeed("Mouse Sensitivity:", 1, 10, 70, 300, CONTROL_MouseSensitivity, 1.f, 100.f, 1.f, SetMouseSensitivity, -1,-1);
CGameMenuItemZBool boolMouseFlipped("Invert Mouse Aim:", 1, 10, 90, 300, gMouseAimingFlipped, SetMouseAimFlipped, NULL, NULL);
CGameMenuItemSlider sliderTurnSpeed("Key Turn Speed:", 1, 10, 110, 300, gTurnSpeed, 64, 128, 4, SetTurnSpeed, -1, -1);
CGameMenuItemChain itemChainKeyList("Configure Keys...", 1, 0, 130, 320, 1, &menuKeys, -1, NULL, 0);
CGameMenuItemChain itemChainKeyReset("Reset Keys (default)...", 1, 0, 150, 320, 1, &menuKeys, -1, ResetKeys, 0);
CGameMenuItemChain itemChainKeyResetClassic("Reset Keys (classic)...", 1, 0, 170, 320, 1, &menuKeys, -1, ResetKeysClassic, 0);

CGameMenuItemTitle itemMessagesTitle("MESSAGES", 1, 160, 20, 2038);
CGameMenuItemZBool boolMessages("MESSAGES:", 3, 66, 70, 180, 0, SetMessages, NULL, NULL);
CGameMenuItemSlider sliderMsgCount("MESSAGE COUNT:", 3, 66, 80, 180, gMessageCount, 1, 16, 1, NULL, -1, -1);
CGameMenuItemSlider sliderMsgTime("MESSAGE TIME:", 3, 66, 90, 180, gMessageTime, 1, 8, 1, NULL, -1, -1);
CGameMenuItemZBool boolMsgFont("LARGE FONT:", 3, 66, 100, 180, 0, 0, NULL, NULL);
CGameMenuItemZBool boolMsgIncoming("INCOMING:", 3, 66, 110, 180, 0, 0, NULL, NULL);
CGameMenuItemZBool boolMsgSelf("SELF PICKUP:", 3, 66, 120, 180, 0, 0, NULL, NULL);
CGameMenuItemZBool boolMsgOther("OTHER PICKUP:", 3, 66, 130, 180, 0, 0, NULL, NULL);
CGameMenuItemZBool boolMsgRespawn("RESPAWN:", 3, 66, 140, 180, 0, 0, NULL, NULL);

CGameMenuItemTitle itemKeysTitle("KEY SETUP", 1, 160, 20, 2038);
CGameMenuItemKeyList itemKeyList("", 3, 56, 40, 200, 16, NUMGAMEFUNCTIONS, 0);

CGameMenuItemTitle itemSaveTitle("Save Game", 1, 160, 20, 2038);
CGameMenuItemZEditBitmap itemSaveGame0(NULL, 3, 20, 60, 320, strRestoreGameStrings[kLoadSaveSlot0], 16, 1, SaveGame, kLoadSaveSlot0);
CGameMenuItemZEditBitmap itemSaveGame1(NULL, 3, 20, 70, 320, strRestoreGameStrings[kLoadSaveSlot1], 16, 1, SaveGame, kLoadSaveSlot1);
CGameMenuItemZEditBitmap itemSaveGame2(NULL, 3, 20, 80, 320, strRestoreGameStrings[kLoadSaveSlot2], 16, 1, SaveGame, kLoadSaveSlot2);
CGameMenuItemZEditBitmap itemSaveGame3(NULL, 3, 20, 90, 320, strRestoreGameStrings[kLoadSaveSlot3], 16, 1, SaveGame, kLoadSaveSlot3);
CGameMenuItemZEditBitmap itemSaveGame4(NULL, 3, 20, 100, 320, strRestoreGameStrings[kLoadSaveSlot4], 16, 1, SaveGame, kLoadSaveSlot4);
CGameMenuItemZEditBitmap itemSaveGame5(NULL, 3, 20, 110, 320, strRestoreGameStrings[kLoadSaveSlot5], 16, 1, SaveGame, kLoadSaveSlot5);
CGameMenuItemZEditBitmap itemSaveGame6(NULL, 3, 20, 120, 320, strRestoreGameStrings[kLoadSaveSlot6], 16, 1, SaveGame, kLoadSaveSlot6);
CGameMenuItemZEditBitmap itemSaveGame7(NULL, 3, 20, 130, 320, strRestoreGameStrings[kLoadSaveSlot7], 16, 1, SaveGame, kLoadSaveSlot7);
CGameMenuItemZEditBitmap itemSaveGame8(NULL, 3, 20, 140, 320, strRestoreGameStrings[kLoadSaveSlot8], 16, 1, SaveGame, kLoadSaveSlot8);
CGameMenuItemZEditBitmap itemSaveGame9(NULL, 3, 20, 150, 320, strRestoreGameStrings[kLoadSaveSlot9], 16, 1, SaveGame, kLoadSaveSlot9);
CGameMenuItemBitmapLS itemSaveGamePic(NULL, 3, 0, 0, 2050);

CGameMenuItemTitle itemLoadTitle("Load Game", 1, 160, 20, 2038);
CGameMenuItemZEditBitmap itemLoadGame0(NULL, 3, 20, 60, 320, strRestoreGameStrings[kLoadSaveSlot0], 16, 1, LoadGame, kLoadSaveSlot0);
CGameMenuItemZEditBitmap itemLoadGame1(NULL, 3, 20, 70, 320, strRestoreGameStrings[kLoadSaveSlot1], 16, 1, LoadGame, kLoadSaveSlot1);
CGameMenuItemZEditBitmap itemLoadGame2(NULL, 3, 20, 80, 320, strRestoreGameStrings[kLoadSaveSlot2], 16, 1, LoadGame, kLoadSaveSlot2);
CGameMenuItemZEditBitmap itemLoadGame3(NULL, 3, 20, 90, 320, strRestoreGameStrings[kLoadSaveSlot3], 16, 1, LoadGame, kLoadSaveSlot3);
CGameMenuItemZEditBitmap itemLoadGame4(NULL, 3, 20, 100, 320, strRestoreGameStrings[kLoadSaveSlot4], 16, 1, LoadGame, kLoadSaveSlot4);
CGameMenuItemZEditBitmap itemLoadGame5(NULL, 3, 20, 110, 320, strRestoreGameStrings[kLoadSaveSlot5], 16, 1, LoadGame, kLoadSaveSlot5);
CGameMenuItemZEditBitmap itemLoadGame6(NULL, 3, 20, 120, 320, strRestoreGameStrings[kLoadSaveSlot6], 16, 1, LoadGame, kLoadSaveSlot6);
CGameMenuItemZEditBitmap itemLoadGame7(NULL, 3, 20, 130, 320, strRestoreGameStrings[kLoadSaveSlot7], 16, 1, LoadGame, kLoadSaveSlot7);
CGameMenuItemZEditBitmap itemLoadGame8(NULL, 3, 20, 140, 320, strRestoreGameStrings[kLoadSaveSlot8], 16, 1, LoadGame, kLoadSaveSlot8);
CGameMenuItemZEditBitmap itemLoadGame9(NULL, 3, 20, 150, 320, strRestoreGameStrings[kLoadSaveSlot9], 16, 1, LoadGame, kLoadSaveSlot9);
CGameMenuItemZEditBitmap itemLoadGameAutosaveStart(NULL, 3, 20, 170, 320, strRestoreGameStrings[kLoadSaveSlotSpawn], 16, 1, LoadGame, kLoadSaveSlotSpawn);
CGameMenuItemZEditBitmap itemLoadGameAutosaveKey(NULL, 3, 20, 180, 320, strRestoreGameStrings[kLoadSaveSlotKey], 16, 1, LoadGame, kLoadSaveSlotKey);
CGameMenuItemBitmapLS itemLoadGamePic(NULL, 3, 0, 0, 2518);

CGameMenu menuMultiUserMaps;

CGameMenuItemTitle itemNetStartUserMapTitle("USER MAP", 1, 160, 20, 2038);
CGameMenuFileSelect menuMultiUserMap("", 3, 0, 0, 0, "./", "*.map", zUserMapName);

CGameMenuItemTitle itemNetStartTitle("MULTIPLAYER", 1, 160, 20, 2038);
CGameMenuItemZCycle itemNetStart1("GAME:", 3, 66, 35, 180, 0, 0, zNetGameTypes, 3, 0);
CGameMenuItemZCycle itemNetStart2("EPISODE:", 3, 66, 45, 180, 0, SetupNetLevels, NULL, 0, 0);
CGameMenuItemZCycle itemNetStart3("LEVEL:", 3, 66, 55, 180, 0, NetClearUserMap, NULL, 0, 0);
CGameMenuItemZCycle itemNetStart4("DIFFICULTY:", 3, 66, 65, 180, 0, 0, zDiffStrings, ARRAY_SSIZE(zDiffStrings), 0);
CGameMenuItemZCycle itemNetStart5("MONSTERS:", 3, 66, 75, 180, 0, 0, zMonsterStrings, ARRAY_SSIZE(zMonsterStrings), 0);
CGameMenuItemZCycle itemNetStart6("WEAPONS:", 3, 66, 85, 180, 0, 0, zWeaponStrings, 4, 0);
CGameMenuItemZCycle itemNetStart7("ITEMS:", 3, 66, 95, 180, 0, 0, zItemStrings, 3, 0);
CGameMenuItemZBool itemNetStart8("FRIENDLY FIRE:", 3, 66, 105, 180, true, 0, NULL, NULL);
CGameMenuItemZCycle itemNetStart9("KEYS SETTING:", 3, 66, 115, 180, 0, 0, zKeyStrings, ARRAY_SSIZE(zKeyStrings), 0);
CGameMenuItemZCycle itemNetStart10("SPAWN PROTECTION:", 3, 66, 125, 180, 0, 0, zSpawnProtectStrings, ARRAY_SSIZE(zSpawnProtectStrings), 0);
CGameMenuItemZCycle itemNetStart11("SPAWN WITH WEAPON:", 3, 66, 135, 180, 0, 0, zSpawnWeaponStrings, ARRAY_SSIZE(zSpawnWeaponStrings), 0);
CGameMenuItemChain itemNetStart12("USER MAP", 3, 66, 150, 320, 0, &menuMultiUserMaps, 0, NULL, 0);
CGameMenuItemChain itemNetStart13("ENHANCEMENTS", 3, 66, 160, 320, 0, &menuNetworkGameEnhancements, -1, NULL, 0);
CGameMenuItemChain itemNetStart14("START GAME", 1, 0, 175, 320, 1, 0, -1, StartNetGame, 0);

///////////////
CGameMenuItemZBool itemNetEnhancementBoolQuadDamagePowerup("REPLACE AKIMBO WITH 4X DAMAGE:", 3, 66, 45, 180, false, NULL, NULL, NULL);
CGameMenuItemZBool itemNetEnhancementBoolDamageInvul("HITSCAN DAMAGE INVULNERABILITY:", 3, 66, 55, 180, false, NULL, NULL, NULL);
CGameMenuItemZCycle itemNetEnhancementExplosionBehavior("EXPLOSIONS BEHAVIOR:", 3, 66, 65, 180, 0, NULL, pzExplosionBehaviorStrings, ARRAY_SSIZE(pzExplosionBehaviorStrings), 0);
CGameMenuItemZCycle itemNetEnhancementProjectileBehavior("PROJECTILES BEHAVIOR:", 3, 66, 75, 180, 0, NULL, pzProjectileBehaviorStrings, ARRAY_SSIZE(pzProjectileBehaviorStrings), 0);
CGameMenuItemZBool itemNetEnhancementEnemyBehavior("ENEMY BEHAVIOR:", 3, 66, 85, 180, false, NULL, "NOTBLOOD", "ORIGINAL");
CGameMenuItemZBool itemNetEnhancementBoolEnemyRandomTNT("RANDOM CULTIST TNT:", 3, 66, 95, 180, false, NULL, NULL, NULL);
CGameMenuItemZCycle itemNetEnhancementWeaponsVer("WEAPON BEHAVIOR:", 3, 66, 105, 180, 0, NULL, pzWeaponsVersionStrings, ARRAY_SSIZE(pzWeaponsVersionStrings), 0);
CGameMenuItemZBool itemNetEnhancementSectorBehavior("SECTOR BEHAVIOR:", 3, 66, 115, 180, false, NULL, "NOTBLOOD", "ORIGINAL");
CGameMenuItemZBool itemNetEnhancementBoolHitscanProjectiles("HITSCAN PROJECTILES:", 3, 66, 125, 180, false, NULL, NULL, NULL);
CGameMenuItemZCycle itemNetEnhancementRandomizerMode("RANDOMIZER MODE:", 3, 66, 135, 180, 0, NULL, pzRandomizerModeStrings, ARRAY_SSIZE(pzRandomizerModeStrings), 0);
CGameMenuItemZEdit itemNetEnhancementRandomizerSeed("RANDOMIZER SEED:", 3, 66, 145, 180, szRandomizerSeedMenu, sizeof(szRandomizerSeedMenu), 0, SetRandomizerSeed, 0);
///////////////////

CGameMenuItemText itemLoadingText("LOADING...", 1, 160, 100, 1);

CGameMenuItemTitle itemSoundsTitle("SOUNDS", 1, 160, 20, 2038);
CGameMenuItemSlider itemSoundsMusic("MUSIC:", 3, 40, 60, 180, MusicVolume, 0, 255, 17, SetMusicVol, -1, -1);
CGameMenuItemSlider itemSoundsSound("SOUND:", 3, 40, 70, 180, FXVolume, 0, 255, 17, SetSoundVol, -1, -1);
CGameMenuItemSlider itemSoundsCDAudio("CD AUDIO:", 3, 40, 80, 180, CDVolume, 0, 255, 17, SetCDVol, -1, -1);
CGameMenuItemZBool itemSounds3DAudio("3D SOUND:", 3, 40, 90, 180, gStereo, SetMonoStereo, NULL, NULL);

CGameMenuItemTitle itemQuitTitle("QUIT", 1, 160, 20, 2038);
CGameMenuItemText itemQuitText1("Do you really want to quit?", 0, 160, 100, 1);
CGameMenuItemYesNoQuit itemQuitYesNo("[Y/N]", 0, 20, 110, 280, 1, 0);

CGameMenuItemTitle itemRestartTitle("RESTART GAME", 1, 160, 20, 2038);
CGameMenuItemText itemRestartText1("Do you really want to restart game?", 0, 160, 100, 1);
CGameMenuItemYesNoQuit itemRestartYesNo("[Y/N]", 0, 20, 110, 280, 1, 1);

CGameMenuItemPicCycle itemCreditsPicCycle(0, 0, NULL, NULL, 0, 0);
CGameMenuItemPicCycle itemOrderPicCycle(0, 0, NULL, NULL, 0, 0);

CGameMenuItemTitle itemParentalLockTitle("PARENTAL LOCK", 1, 160, 20, 2038);
CGameMenuItemZBool itemParentalLockToggle("LOCK:", 3, 66, 70, 180, 0, SetParentalLock, NULL, NULL);
CGameMenuItemPassword itemParentalLockPassword("SET PASSWORD:", 3, 160, 80);

CGameMenuItemPicCycle itemSorryPicCycle(0, 0, NULL, NULL, 0, 0);
CGameMenuItemText itemSorryText1("Loading and saving games", 0, 160, 90, 1);
CGameMenuItemText itemSorryText2("not supported", 0, 160, 100, 1);
CGameMenuItemText itemSorryText3("in this demo version of Blood.", 0, 160, 110, 1);

CGameMenuItemText itemSorry2Text1("Buy the complete version of", 0, 160, 90, 1);
CGameMenuItemText itemSorry2Text2("Blood for three new episodes", 0, 160, 100, 1);
CGameMenuItemText itemSorry2Text3("plus eight BloodBath-only levels!", 0, 160, 110, 1);

CGameMenuItemTitle unk_26E06C(" ONLINE ", 1, 160, 20, 2038);
CGameMenuItem7EA1C unk_26E090("DWANGO", 1, 0, 45, 320, "matt", "DWANGO", 1, -1, NULL, 0);
CGameMenuItem7EA1C unk_26E0E8("RTIME", 1, 0, 65, 320, "matt", "RTIME", 1, -1, NULL, 0);
CGameMenuItem7EA1C unk_26E140("HEAT", 1, 0, 85, 320, "matt", "HEAT", 1, -1, NULL, 0);
CGameMenuItem7EA1C unk_26E198("KALI", 1, 0, 105, 320, "matt", "KALI", 1, -1, NULL, 0);
CGameMenuItem7EA1C unk_26E1F0("MPATH", 1, 0, 125, 320, "matt", "MPATH", 1, -1, NULL, 0);
CGameMenuItem7EA1C unk_26E248("TEN", 1, 0, 145, 320, "matt", "TEN", 1, -1, TenProcess, 0);


// static int32_t newresolution, newrendermode, newfullscreen, newvsync;

enum resflags_t {
    RES_FS = 0x1,
    RES_WIN = 0x2,
};

#define MAXRESOLUTIONSTRINGLENGTH 19

struct resolution_t {
    int32_t xdim, ydim;
    int32_t flags;
    int32_t bppmax;
    char name[MAXRESOLUTIONSTRINGLENGTH];
};

resolution_t gResolution[MAXVALIDMODES];
int gResolutionNum;
const char *gResolutionName[MAXVALIDMODES];

CGameMenu menuOptions;
CGameMenu menuOptionsGame;
CGameMenu menuOptionsGameEnhancements;
CGameMenu menuOptionsDisplay;
CGameMenu menuOptionsDisplayColor;
CGameMenu menuOptionsDisplayMode;
#ifdef USE_OPENGL
CGameMenu menuOptionsDisplayPolymost;
#endif
CGameMenu menuOptionsSound;
CGameMenu menuOptionsPlayer;
CGameMenu menuOptionsControl;

void SetupOptionsSound(CGameMenuItemChain *pItem);

CGameMenuItemTitle itemOptionsTitle("OPTIONS", 1, 160, 20, 2038);
CGameMenuItemChain itemOptionsChainGame("GAME SETUP", 1, 0, 50, 320, 1, &menuOptionsGame, -1, NULL, 0);
CGameMenuItemChain itemOptionsChainDisplay("DISPLAY SETUP", 1, 0, 70, 320, 1, &menuOptionsDisplay, -1, NULL, 0);
CGameMenuItemChain itemOptionsChainSound("SOUND SETUP", 1, 0, 90, 320, 1, &menuOptionsSound, -1, SetupOptionsSound, 0);
CGameMenuItemChain itemOptionsChainPlayer("PLAYER SETUP", 1, 0, 110, 320, 1, &menuOptionsPlayer, -1, NULL, 0);
CGameMenuItemChain itemOptionsChainControl("CONTROL SETUP", 1, 0, 130, 320, 1, &menuOptionsControl, -1, NULL, 0);
CGameMenuItemChain itemOptionsChainEnhancements("ENHANCEMENTS", 1, 0, 150, 320, 1, &menuOptionsGameEnhancements, -1, NULL, 0);
//CGameMenuItemChain itemOptionsChainOld("OLD MENU", 1, 0, 170, 320, 1, &menuOptionsOld, -1, NULL, 0);

const char *pzAutoAimStrings[] = {
    "NEVER",
    "ALWAYS",
    "HITSCAN ONLY"
};

const char *pzWeaponHBobbingStrings[] = {
    "OFF",
    "ORIGINAL",
    "V1.0X",
};

const char *pzWeaponInterpolateStrings[] = {
    "OFF",
    "ONLY SWAYING",
    "ALL ANIMATION"
};

const char *pzStatsPowerupRatioStrings[] = {
    "OFF",
    "ON",
    "ON (4:3)",
    "ON (16:10)",
    "ON (16:9)",
    "ON (21:9)",
};

const char *pzHudRatioStrings[] = {
    "NATIVE",
    "4:3",
    "16:10",
    "16:9",
    "21:9",
};

const char *pzMirrorModeStrings[] = {
    "OFF",
    "HORIZONTAL",
    "VERTICAL",
    "HORIZONTAL+VERTICAL"
};

const char *pzWeaponSelectStrings[] = {
    "OFF",
    "BOTTOM",
    "TOP"
};

const char *pzWeaponSwitchStrings[] = {
    "NEVER",
    "IF NEW",
    "BY RATING"
};

const char *pzAutosaveModeStrings[] = {
    "OFF",
    "NEW LEVEL",
    "KEYS+NEW LEVEL",
};

const char *pzVanillaModeStrings[] = {
    "OFF",
    "ON",
    "DOS MOUSE+ON",
};

void SetAutoAim(CGameMenuItemZCycle *pItem);
void SetLevelStats(CGameMenuItemZCycle *pItem);
void SetPowerupDuration(CGameMenuItemZCycle *pItem);
void SetShowMapTitle(CGameMenuItemZBool *pItem);
void SetWeaponSwitch(CGameMenuItemZCycle *pItem);
void SetWeaponFastSwitch(CGameMenuItemZBool *pItem);
void SetAutosaveMode(CGameMenuItemZCycle *pItem);
void SetLockSaving(CGameMenuItemZBool *pItem);
void SetVanillaMode(CGameMenuItemZCycle *pItem);

CGameMenuItemTitle itemOptionsGameTitle("GAME SETUP", 1, 160, 20, 2038);
CGameMenuItemTitle itemGameEnhancementsTitle("ENHANCEMENTS", 1, 160, 20, 2038);

///////////////
CGameMenuItemZCycle itemEnhancementMonsterSettings("MONSTERS:", 3, 66, 45, 180, 0, SetMonsters, pzMonsterStrings, ARRAY_SSIZE(pzMonsterStrings), 0);
CGameMenuItemZBool itemEnhancementBoolQuadDamagePowerup("REPLACE AKIMBO WITH 4X DAMAGE:", 3, 66, 55, 180, false, SetQuadDamagePowerup, NULL, NULL);
CGameMenuItemZBool itemEnhancementBoolDamageInvul("HITSCAN DAMAGE INVULNERABILITY:", 3, 66, 65, 180, false, SetDamageInvul, NULL, NULL);
CGameMenuItemZCycle itemEnhancementExplosionBehavior("EXPLOSIONS BEHAVIOR:", 3, 66, 75, 180, 0, SetExplosionBehavior, pzExplosionBehaviorStrings, ARRAY_SSIZE(pzExplosionBehaviorStrings), 0);
CGameMenuItemZCycle itemEnhancementProjectileBehavior("PROJECTILES BEHAVIOR:", 3, 66, 85, 180, 0, SetProjectileBehavior, pzProjectileBehaviorStrings, ARRAY_SSIZE(pzProjectileBehaviorStrings), 0);
CGameMenuItemZBool itemEnhancementEnemyBehavior("ENEMY BEHAVIOR:", 3, 66, 95, 180, false, SetEnemyBehavior, "NOTBLOOD", "ORIGINAL");
CGameMenuItemZBool itemEnhancementBoolEnemyRandomTNT("RANDOM CULTIST TNT:", 3, 66, 105, 180, false, SetEnemyRandomTNT, NULL, NULL);
CGameMenuItemZCycle itemEnhancementWeaponsVer("WEAPON BEHAVIOR:", 3, 66, 115, 180, 0, SetWeaponsVer, pzWeaponsVersionStrings, ARRAY_SSIZE(pzWeaponsVersionStrings), 0);
CGameMenuItemZBool itemEnhancementSectorBehavior("SECTOR BEHAVIOR:", 3, 66, 125, 180, false, SetSectorBehavior, "NOTBLOOD", "ORIGINAL");
CGameMenuItemZBool itemEnhancementBoolHitscanProjectiles("HITSCAN PROJECTILES:", 3, 66, 135, 180, false, SetHitscanProjectiles, NULL, NULL);
CGameMenuItemZCycle itemEnhancementRandomizerMode("RANDOMIZER MODE:", 3, 66, 145, 180, 0, SetRandomizerMode, pzRandomizerModeStrings, ARRAY_SSIZE(pzRandomizerModeStrings), 0);
CGameMenuItemZEdit itemEnhancementRandomizerSeed("RANDOMIZER SEED:", 3, 66, 155, 180, szRandomizerSeedMenu, sizeof(szRandomizerSeedMenu), 0, SetRandomizerSeed, 0);
///////////////////

CGameMenuItemZBool itemOptionsGameBoolShowPlayerNames("SHOW PLAYER NAMES:", 3, 66, 37, 180, gShowPlayerNames, SetShowPlayerNames, NULL, NULL);
CGameMenuItemZCycle itemOptionsGameShowWeapons("SHOW WEAPONS:", 3, 66, 47, 180, 0, SetShowWeapons, pzShowWeaponStrings, ARRAY_SSIZE(pzShowWeaponStrings), 0);
CGameMenuItemZBool itemOptionsGameBoolSlopeTilting("SLOPE TILTING:", 3, 66, 57, 180, gSlopeTilting, SetSlopeTilting, NULL, NULL);
CGameMenuItemZBool itemOptionsGameBoolViewBobbing("VIEW BOBBING:", 3, 66, 67, 180, gViewVBobbing, SetViewBobbing, NULL, NULL);
CGameMenuItemZBool itemOptionsGameBoolViewSwaying("VIEW SWAYING:", 3, 66, 77, 180, gViewHBobbing, SetViewSwaying, NULL, NULL);
CGameMenuItemZCycle itemOptionsGameBoolWeaponSwaying("WEAPON SWAYING:", 3, 66, 87, 180, 0, SetWeaponSwaying, pzWeaponHBobbingStrings, ARRAY_SSIZE(pzWeaponHBobbingStrings), 0);
CGameMenuItemZCycle itemOptionsGameBoolWeaponInterpolation("WEAPON SMOOTHING:", 3, 66, 97, 180, 0, SetWeaponInterpolate, pzWeaponInterpolateStrings, ARRAY_SSIZE(pzWeaponInterpolateStrings), 0);
CGameMenuItemZCycle itemOptionsGameBoolAutoAim("AUTO AIM:", 3, 66, 107, 180, 0, SetAutoAim, pzAutoAimStrings, ARRAY_SSIZE(pzAutoAimStrings), 0);
CGameMenuItemZCycle itemOptionsGameWeaponSwitch("EQUIP PICKUPS:", 3, 66, 117, 180, 0, SetWeaponSwitch, pzWeaponSwitchStrings, ARRAY_SSIZE(pzWeaponSwitchStrings), 0);
CGameMenuItemZBool itemOptionsGameWeaponFastSwitch("FAST WEAPON SWITCH:", 3, 66, 127, 180, 0, SetWeaponFastSwitch, NULL, NULL);
CGameMenuItemZCycle itemOptionsGameAutosaveMode("AUTOSAVE:", 3, 66, 137, 180, 0, SetAutosaveMode, pzAutosaveModeStrings, ARRAY_SSIZE(pzAutosaveModeStrings), 0);
CGameMenuItemZBool itemOptionsGameLockSaving("LOCK MANUAL SAVING:", 3, 66, 147, 180, 0, SetLockSaving, "AUTOSAVES ONLY", "NEVER");
CGameMenuItemZCycle itemOptionsGameBoolVanillaMode("VANILLA MODE:", 3, 66, 157, 180, 0, SetVanillaMode, pzVanillaModeStrings, ARRAY_SSIZE(pzVanillaModeStrings), 0);
//CGameMenuItemChain itemOptionsGameChainParentalLock("PARENTAL LOCK", 3, 0, 167, 320, 1, &menuParentalLock, -1, NULL, 0);

CGameMenuItemTitle itemOptionsDisplayTitle("DISPLAY SETUP", 1, 160, 20, 2038);
CGameMenuItemChain itemOptionsDisplayColor("COLOR CORRECTION", 3, 66, 40, 180, 0, &menuOptionsDisplayColor, -1, NULL, 0);
CGameMenuItemChain itemOptionsDisplayMode("VIDEO MODE", 3, 66, 50, 180, 0, &menuOptionsDisplayMode, -1, SetupVideoModeMenu, 0);
CGameMenuItemZCycle itemOptionsDisplayCrosshair("CROSSHAIR:", 3, 66, 60, 180, 0, SetCrosshair, pzCrosshairStrings, ARRAY_SSIZE(pzCrosshairStrings), 0);
CGameMenuItemZBool itemOptionsDisplayBoolCenterHoriz("CENTER HORIZON LINE:", 3, 66, 70, 180, gCenterHoriz, SetCenterHoriz, NULL, NULL);
CGameMenuItemZCycle itemOptionsDisplayLevelStats("LEVEL STATS:", 3, 66, 80, 180, 0, SetLevelStats, pzStatsPowerupRatioStrings, ARRAY_SSIZE(pzStatsPowerupRatioStrings), 0);
CGameMenuItemZCycle itemOptionsDisplayPowerupDuration("POWERUP DURATION:", 3, 66, 90, 180, 0, SetPowerupDuration, pzStatsPowerupRatioStrings, ARRAY_SSIZE(pzStatsPowerupRatioStrings), 0);
CGameMenuItemZBool itemOptionsDisplayBoolShowMapTitle("MAP TITLE:", 3, 66, 100, 180, gShowMapTitle, SetShowMapTitle, NULL, NULL);
CGameMenuItemZBool itemOptionsDisplayBoolMessages("MESSAGES:", 3, 66, 110, 180, gMessageState, SetMessages, NULL, NULL);
CGameMenuItemZBool itemOptionsDisplayBoolWidescreen("WIDESCREEN:", 3, 66, 120, 180, r_usenewaspect, SetWidescreen, NULL, NULL);
CGameMenuItemZCycle itemOptionsDisplayHudRatio("HUD ALIGNMENT:", 3, 66, 130, 180, 0, SetHudRatio, pzHudRatioStrings, ARRAY_SSIZE(pzHudRatioStrings), 0);
CGameMenuItemZCycle itemOptionsDisplayMirrorMode("MIRROR MODE:", 3, 66, 140, 180, 0, SetMirrorMode, pzMirrorModeStrings, ARRAY_SSIZE(pzMirrorModeStrings), 0);
CGameMenuItemZCycle itemOptionsDisplayWeaponSelect("SHOW WEAPON SELECT:", 3, 66, 150, 180, 0, SetWeaponSelectMode, pzWeaponSelectStrings, ARRAY_SSIZE(pzWeaponSelectStrings), 0);
CGameMenuItemZBool itemOptionsDisplayBoolSlowRoomFlicker("SLOW DOWN FLICKERING LIGHTS:", 3, 66, 160, 180, gSlowRoomFlicker, SetSlowRoomFlicker, NULL, NULL);
CGameMenuItemSlider itemOptionsDisplayFOV("FOV:", 3, 66, 170, 180, &gFov, 75, 140, 1, SetFOV, -1, -1, kMenuSliderValue);
#ifdef USE_OPENGL
CGameMenuItemChain itemOptionsDisplayPolymost("POLYMOST SETUP", 3, 66, 180, 180, 0, &menuOptionsDisplayPolymost, -1, SetupVideoPolymostMenu, 0);
#endif

const char *pzRendererStrings[] = {
    "CLASSIC",
    "POLYMOST"
};

const int nRendererValues[] = {
    REND_CLASSIC,
    REND_POLYMOST
};

const char *pzVSyncStrings[] = {
    "ADAPTIVE",
    "OFF",
    "ON"
};

const int nVSyncValues[] = {
    -1,
    0,
    1
};

const char *pzFrameLimitStrings[] = {
    "30 FPS",
    "60 FPS",
    "75 FPS",
    "100 FPS",
    "120 FPS",
    "144 FPS",
    "165 FPS",
    "240 FPS"
};

const int nFrameLimitValues[] = {
    30,
    60,
    75,
    100,
    120,
    144,
    165,
    240
};


void PreDrawVideoModeMenu(CGameMenuItem *);

CGameMenuItemTitle itemOptionsDisplayModeTitle("VIDEO MODE", 1, 160, 20, 2038);
CGameMenuItemZCycle itemOptionsDisplayModeResolution("RESOLUTION:", 3, 66, 60, 180, 0, NULL, NULL, 0, 0, true);
CGameMenuItemZCycle itemOptionsDisplayModeRenderer("RENDERER:", 3, 66, 70, 180, 0, NULL, pzRendererStrings, 2, 0);
CGameMenuItemZBool itemOptionsDisplayModeFullscreen("FULLSCREEN:", 3, 66, 80, 180, 0, NULL, NULL, NULL);
CGameMenuItemZCycle itemOptionsDisplayModeVSync("VSYNC:", 3, 66, 90, 180, 0, NULL, pzVSyncStrings, 3, 0);
CGameMenuItemZCycle itemOptionsDisplayModeFrameLimit("FRAMERATE LIMIT:", 3, 66, 100, 180, 0, UpdateVideoModeMenuFrameLimit, pzFrameLimitStrings, 8, 0);
// CGameMenuItemSlider itemOptionsDisplayModeFPSOffset("FPS OFFSET:", 3, 66, 110, 180, 0, -10, 10, 1, UpdateVideoModeMenuFPSOffset, -1, -1, kMenuSliderValue);
CGameMenuItemChain itemOptionsDisplayModeApply("APPLY CHANGES", 3, 66, 115, 180, 0, NULL, 0, SetVideoMode, 0);

void PreDrawDisplayColor(CGameMenuItem *);

CGameMenuItemTitle itemOptionsDisplayColorTitle("COLOR CORRECTION", 1, 160, 20, -1);
CGameMenuItemZCycle itemOptionsDisplayColorPaletteCustom("PALETTE:", 3, 66, 100, 180, 0, UpdateVideoPaletteCycleMenu, srcCustomPaletteStr, ARRAY_SSIZE(srcCustomPaletteStr), 0);
CGameMenuItemZBool itemOptionsDisplayColorPaletteCIEDE2000("CIEDE2000 COMPARE:", 3, 66, 110, 180, 0, UpdateVideoPaletteBoolMenu, NULL, NULL);
CGameMenuItemZBool itemOptionsDisplayColorPaletteGrayscale("GRAYSCALE PALETTE:", 3, 66, 120, 180, 0, UpdateVideoPaletteBoolMenu, NULL, NULL);
CGameMenuItemZBool itemOptionsDisplayColorPaletteInvert("INVERT PALETTE:", 3, 66, 130, 180, 0, UpdateVideoPaletteBoolMenu, NULL, NULL);
CGameMenuItemSliderFloat itemOptionsDisplayColorGamma("GAMMA:", 3, 66, 140, 180, &g_videoGamma, 0.3f, 4.f, 0.1f, UpdateVideoColorMenu, -1, -1, kMenuSliderValue);
CGameMenuItemSliderFloat itemOptionsDisplayColorContrast("CONTRAST:", 3, 66, 150, 180, &g_videoContrast, 0.1f, 2.7f, 0.05f, UpdateVideoColorMenu, -1, -1, kMenuSliderValue);
CGameMenuItemSliderFloat itemOptionsDisplayColorBrightness("BRIGHTNESS:", 3, 66, 160, 180, &g_videoBrightness, -0.8f, 0.8f, 0.05f, UpdateVideoColorMenu, -1, -1, kMenuSliderValue);
CGameMenuItemSliderFloat itemOptionsDisplayColorVisibility("VISIBILITY:", 3, 66, 170, 180, &r_ambientlight, 0.125f, 4.f, 0.125f, UpdateVideoColorMenu, -1, -1, kMenuSliderValue);
CGameMenuItemChain itemOptionsDisplayColorReset("RESET TO DEFAULTS", 3, 66, 180, 180, 0, NULL, 0, ResetVideoColor, 0);

const char *pzTextureModeStrings[] = {
    "CLASSIC",
    "FILTERED"
};

#ifdef USE_OPENGL
int nTextureModeValues[] = {
    TEXFILTER_OFF,
    TEXFILTER_ON
};
#endif

const char *pzAnisotropyStrings[] = {
    "MAX",
    "NONE",
    "2X",
    "4X",
    "8X",
    "16X"
};

int nAnisotropyValues[] = {
    0,
    1,
    2,
    4,
    8,
    16
};

const char *pzTexQualityStrings[] = {
    "FULL",
    "HALF",
    "BARF"
};

const char *pzTexCacheStrings[] = {
    "OFF",
    "ON",
    "COMPRESSED"
};

void UpdateTextureMode(CGameMenuItemZCycle *pItem);
void UpdateAnisotropy(CGameMenuItemZCycle *pItem);
void UpdateTrueColorTextures(CGameMenuItemZBool *pItem);
void UpdateTexQuality(CGameMenuItemZCycle *pItem);
void UpdatePreloadCache(CGameMenuItemZBool *pItem);
void UpdateTexCache(CGameMenuItemZCycle *pItem);
void UpdateDetailTex(CGameMenuItemZBool *pItem);
void UpdateGlowTex(CGameMenuItemZBool *pItem);
void Update3DModels(CGameMenuItemZBool *pItem);
void UpdateDeliriumBlur(CGameMenuItemZBool *pItem);
void UpdateTexColorIndex(CGameMenuItemZBool *pItem);
#ifdef USE_OPENGL
void PreDrawDisplayPolymost(CGameMenuItem *pItem);
CGameMenuItemTitle itemOptionsDisplayPolymostTitle("POLYMOST SETUP", 1, 160, 20, 2038);
CGameMenuItemZCycle itemOptionsDisplayPolymostTextureMode("TEXTURE MODE:", 3, 66, 60, 180, 0, UpdateTextureMode, pzTextureModeStrings, 2, 0);
CGameMenuItemZCycle itemOptionsDisplayPolymostAnisotropy("ANISOTROPY:", 3, 66, 70, 180, 0, UpdateAnisotropy, pzAnisotropyStrings, 6, 0);
CGameMenuItemZBool itemOptionsDisplayPolymostTrueColorTextures("TRUE COLOR TEXTURES:", 3, 66, 80, 180, 0, UpdateTrueColorTextures, NULL, NULL);
CGameMenuItemZCycle itemOptionsDisplayPolymostTexQuality("GL TEXTURE QUALITY:", 3, 66, 90, 180, 0, UpdateTexQuality, pzTexQualityStrings, 3, 0);
CGameMenuItemZBool itemOptionsDisplayPolymostPreloadCache("PRE-LOAD MAP TEXTURES:", 3, 66, 100, 180, 0, UpdatePreloadCache, NULL, NULL);
CGameMenuItemZCycle itemOptionsDisplayPolymostTexCache("ON-DISK TEXTURE CACHE:", 3, 66, 110, 180, 0, UpdateTexCache, pzTexCacheStrings, 3, 0);
CGameMenuItemZBool itemOptionsDisplayPolymostDetailTex("DETAIL TEXTURES:", 3, 66, 120, 180, 0, UpdateDetailTex, NULL, NULL);
CGameMenuItemZBool itemOptionsDisplayPolymostGlowTex("GLOW TEXTURES:", 3, 66, 130, 180, 0, UpdateGlowTex, NULL, NULL);
CGameMenuItemZBool itemOptionsDisplayPolymost3DModels("3D MODELS:", 3, 66, 140, 180, 0, Update3DModels, NULL, NULL);
CGameMenuItemZBool itemOptionsDisplayPolymostDeliriumBlur("DELIRIUM EFFECT BLUR:", 3, 66, 150, 180, 0, UpdateDeliriumBlur, NULL, NULL);
CGameMenuItemZBool itemOptionsDisplayPolymostUseColorIndexedTex("RENDER WITH COLOR INDEXING:", 3, 66, 160, 180, 0, UpdateTexColorIndex, NULL, NULL);
#endif

void UpdateSoundToggle(CGameMenuItemZBool *pItem);
void UpdateMusicToggle(CGameMenuItemZBool *pItem);
void UpdateCDToggle(CGameMenuItemZBool *pItem);
void SetDoppler(CGameMenuItemZBool *pItem);
void UpdateSoundVolume(CGameMenuItemSlider *pItem);
void UpdateMusicVolume(CGameMenuItemSlider *pItem);
void UpdateSoundRate(CGameMenuItemZCycle *pItem);
void UpdateNumVoices(CGameMenuItemSlider *pItem);
void UpdateSpeakerAngle(CGameMenuItemSlider *pItem);
void UpdateCalebTalk(CGameMenuItemZCycle *pItem);
void UpdateMusicDevice(CGameMenuItemZCycle *pItem);
void SetSound(CGameMenuItemChain *pItem);
void PreDrawSound(CGameMenuItem *pItem);
const char *pzSoundRateStrings[] = {
    "22050HZ",
    "44100HZ",
    "48000HZ"
};

int nSoundRateValues[] = {
    22050,
    44100,
    48000
};

const char *pzCalebTalkStrings[] = {
    "ON",
    "NO IDLE TALK",
    "NO GIB TALK",
    "OFF",
};

int nMusicDeviceValues[] = {
    ASS_OPL3,
#ifdef _WIN32
    ASS_WinMM,
#endif
    ASS_SF2,
};

const char *pzMusicDeviceStrings[] = {
    "OPL3(SB/ADLIB)",
#ifdef _WIN32
    "SYSTEM MIDI",
#endif
    ".SF2 SYNTH",
};
static char sf2bankfile[BMAX_PATH];

CGameMenu menuOptionsSoundSF2;

CGameMenuItemTitle itemOptionsSoundSF2Title("SELECT SF2 BANK", 1, 160, 20, 2038);
CGameMenuFileSelect itemOptionsSoundSF2FS("", 3, 0, 0, 0, "./", "*.sf2", sf2bankfile);

CGameMenuItemTitle itemOptionsSoundTitle("SOUND SETUP", 1, 160, 20, 2038);
CGameMenuItemZBool itemOptionsSoundSoundToggle("SOUND:", 3, 66, 40, 180, false, UpdateSoundToggle, NULL, NULL);
CGameMenuItemZBool itemOptionsSoundMusicToggle("MUSIC:", 3, 66, 50, 180, false, UpdateMusicToggle, NULL, NULL);
CGameMenuItemZBool itemOptionsSoundMonoStereo("STEREO AUDIO:", 3, 66, 60, 180, false, SetMonoStereo, NULL, NULL);
CGameMenuItemZBool itemOptionsSoundDoppler("DOPPLER EFFECT:", 3, 66, 70, 180, false, SetDoppler, NULL, NULL);
CGameMenuItemSlider itemOptionsSoundSoundVolume("SOUND VOLUME:", 3, 66, 80, 180, &FXVolume, 0, 255, 17, UpdateSoundVolume, -1, -1, kMenuSliderPercent);
CGameMenuItemSlider itemOptionsSoundMusicVolume("MUSIC VOLUME:", 3, 66, 90, 180, &MusicVolume, 0, 255, 17, UpdateMusicVolume, -1, -1, kMenuSliderPercent);
CGameMenuItemZCycle itemOptionsSoundSampleRate("SAMPLE RATE:", 3, 66, 100, 180, 0, UpdateSoundRate, pzSoundRateStrings, ARRAY_SIZE(pzSoundRateStrings), 0);
CGameMenuItemSlider itemOptionsSoundNumVoices("VOICES:", 3, 66, 110, 180, NumVoices, 16, 255, 16, UpdateNumVoices, -1, -1, kMenuSliderValue);
CGameMenuItemSlider itemOptionsSoundSpeakerAngle("SPEAKER ANGLE:", 3, 66, 120, 180, &gSoundEarAng, 15, 90, 5, UpdateSpeakerAngle, -1, -1, kMenuSliderValue);
CGameMenuItemZCycle itemOptionsSoundCalebTalk("CALEB TALK:", 3, 66, 130, 180, 0, UpdateCalebTalk, pzCalebTalkStrings, ARRAY_SIZE(pzCalebTalkStrings), 0);
CGameMenuItemZBool itemOptionsSoundCDToggle("REDBOOK AUDIO:", 3, 66, 140, 180, false, UpdateCDToggle, NULL, NULL);
CGameMenuItemZCycle itemOptionsSoundMusicDevice("MIDI DRIVER:", 3, 66, 150, 180, 0, UpdateMusicDevice, pzMusicDeviceStrings, ARRAY_SIZE(pzMusicDeviceStrings), 0);
CGameMenuItemChain itemOptionsSoundSF2Bank("SF2 BANK", 3, 66, 160, 180, 0, &menuOptionsSoundSF2, 0, NULL, 0);
CGameMenuItemChain itemOptionsSoundApplyChanges("APPLY CHANGES", 3, 66, 170, 180, 0, NULL, 0, SetSound, 0);


void UpdatePlayerName(CGameMenuItemZEdit *pItem, CGameMenuEvent *pEvent);
void UpdatePlayerSkill(CGameMenuItemZCycle *pItem);
void UpdatePlayerChatMessageSound(CGameMenuItemZBool *pItem);
void UpdatePlayerKillMessage(CGameMenuItemZBool *pItem);
void UpdatePlayerMultiKill(CGameMenuItemZCycle *pItem);

const char *pzPlayerMultiKillStrings[] = {
    "OFF",
    "ON",
    "SFX+ON",
};

const char *pzPlayerSkillStrings[] = {
    "-2",
    "-1",
    "DEFAULT",
    "+1",
    "+2",
};

CGameMenuItemTitle itemOptionsPlayerTitle("PLAYER SETUP", 1, 160, 20, 2038);
CGameMenuItemZEdit itemOptionsPlayerName("PLAYER NAME:", 3, 66, 65, 180, szPlayerName, MAXPLAYERNAME, 0, UpdatePlayerName, 0);
CGameMenuItemZCycle itemOptionsPlayerSkill("HEALTH HANDICAP:", 3, 66, 75, 180, 0, UpdatePlayerSkill, pzPlayerSkillStrings, ARRAY_SIZE(pzPlayerSkillStrings), 0);
CGameMenuItemZBool itemOptionsPlayerChatSound("CHAT BEEP:", 3, 66, 90, 180, true, UpdatePlayerChatMessageSound, NULL, NULL);
CGameMenuItemZBool itemOptionsPlayerKillMsg("KILL MESSAGES:", 3, 66, 100, 180, true, UpdatePlayerKillMessage, NULL, NULL);
CGameMenuItemZCycle itemOptionsPlayerMultiKill("MULTI KILL MESSAGES:", 3, 66, 110, 180, 0, UpdatePlayerMultiKill, pzPlayerMultiKillStrings, ARRAY_SIZE(pzPlayerMultiKillStrings), 0);

CGameMenu menuOptionsControlKeyboard;
CGameMenu menuOptionsControlMouse;
CGameMenu menuOptionsControlMouseButtonAssignment;

void SetupMouseMenu(CGameMenuItemChain *pItem);

CGameMenuItemTitle itemOptionsControlTitle("CONTROL SETUP", 1, 160, 20, 2038);
CGameMenuItemChain itemOptionsControlKeyboard("KEYBOARD SETUP", 1, 0, 60, 320, 1, &menuOptionsControlKeyboard, -1, NULL, 0);
CGameMenuItemChain itemOptionsControlMouse("MOUSE SETUP", 1, 0, 80, 320, 1, &menuOptionsControlMouse, -1, SetupMouseMenu, 0);

CGameMenuItemTitle itemOptionsControlKeyboardTitle("KEYBOARD SETUP", 1, 160, 20, 2038);
CGameMenuItemSlider itemOptionsControlKeyboardSliderTurnSpeed("Key Turn Speed:", 1, 18, 50, 280, &gTurnSpeed, 64, 128, 4, SetTurnSpeed, -1, -1);
CGameMenuItemZBool itemOptionsControlKeyboardBoolCrouchToggle("Crouch Toggle:", 1, 18, 70, 280, gCrouchToggle, SetCrouchToggle, NULL, NULL);
CGameMenuItemZBool itemOptionsControlKeyboardBoolAutoRun("Always Run:", 1, 18, 90, 280, gAutoRun, SetAutoRun, NULL, NULL);
CGameMenuItemChain itemOptionsControlKeyboardList("Configure Keys...", 1, 0, 125, 320, 1, &menuKeys, -1, NULL, 0);
CGameMenuItemChain itemOptionsControlKeyboardReset("Reset Keys (default)...", 1, 0, 145, 320, 1, &menuKeys, -1, ResetKeys, 0);
CGameMenuItemChain itemOptionsControlKeyboardResetClassic("Reset Keys (classic)...", 1, 0, 165, 320, 1, &menuKeys, -1, ResetKeysClassic, 0);

void SetMouseAimMode(CGameMenuItemZBool *pItem);
void SetMouseVerticalAim(CGameMenuItemZBool *pItem);
void SetMouseXSensitivity(CGameMenuItemSliderFloat *pItem);
void SetMouseYSensitivity(CGameMenuItemSliderFloat*pItem);

void PreDrawControlMouse(CGameMenuItem *pItem);

void SetupMouseButtonMenu(CGameMenuItemChain *pItem);

CGameMenuItemTitle itemOptionsControlMouseTitle("MOUSE SETUP", 1, 160, 20, 2038);
CGameMenuItemChain itemOptionsControlMouseButton("BUTTON ASSIGNMENT", 3, 66, 60, 180, 0, &menuOptionsControlMouseButtonAssignment, 0, SetupMouseButtonMenu, 0);
CGameMenuItemSliderFloat itemOptionsControlMouseSensitivity("SENSITIVITY:", 3, 66, 70, 180, &CONTROL_MouseSensitivity, 1.f, 100.f, 1.f, SetMouseSensitivity, -1, -1, kMenuSliderValue);
CGameMenuItemZBool itemOptionsControlMouseAimFlipped("INVERT AIMING:", 3, 66, 80, 180, false, SetMouseAimFlipped, NULL, NULL);
CGameMenuItemZBool itemOptionsControlMouseAimMode("AIMING TYPE:", 3, 66, 90, 180, false, SetMouseAimMode, "HOLD", "TOGGLE");
CGameMenuItemZBool itemOptionsControlMouseVerticalAim("VERTICAL AIMING:", 3, 66, 100, 180, false, SetMouseVerticalAim, NULL, NULL);
CGameMenuItemSliderFloat itemOptionsControlMouseXSensitivity("HORIZ SENS:", 3, 66, 110, 180, &CONTROL_MouseAxesSensitivity[0], 1.f, 100.f, 1.f, SetMouseXSensitivity, -1, -1, kMenuSliderValue);
CGameMenuItemSliderFloat itemOptionsControlMouseYSensitivity("VERT SENS:", 3, 66, 120, 180, &CONTROL_MouseAxesSensitivity[1], 1.f, 100.f, 1.f, SetMouseYSensitivity, -1, -1, kMenuSliderValue);

void SetupNetworkMenu(void);
void SetupNetworkHostMenu(CGameMenuItemChain *pItem);
void SetupNetworkJoinMenu(CGameMenuItemChain *pItem);
void NetworkHostGame(CGameMenuItemChain *pItem);
void NetworkJoinGame(CGameMenuItemChain *pItem);

char zNetAddressBuffer[16] = "localhost";
char zNetPortBuffer[6] = "23513";

CGameMenuItemTitle itemNetworkTitle("MULTIPLAYER", 1, 160, 20, 2038);
CGameMenuItemChain itemNetworkHost("HOST A GAME", 1, 0, 80, 320, 1, &menuNetworkHost, -1, SetupNetworkHostMenu, 0);
CGameMenuItemChain itemNetworkJoin("JOIN A GAME", 1, 0, 100, 320, 1, &menuNetworkJoin, -1, SetupNetworkJoinMenu, 0);

CGameMenuItemTitle itemNetworkHostTitle("HOST A GAME", 1, 160, 20, 2038);
CGameMenuItemSlider itemNetworkHostPlayerNum("PLAYER NUMBER:", 3, 66, 70, 180, 1, 2, kMaxPlayers, 1, NULL, -1, -1, kMenuSliderValue);
CGameMenuItemZEdit itemNetworkHostPort("NETWORK PORT:", 3, 66, 80, 180, zNetPortBuffer, 6, 0, NULL, 0);
CGameMenuItemChain itemNetworkHostHost("HOST A GAME", 3, 66, 100, 180, 1, NULL, -1, NetworkHostGame, 0);

CGameMenuItemTitle itemNetworkJoinTitle("JOIN A GAME", 1, 160, 20, 2038);
CGameMenuItemZEdit itemNetworkJoinAddress("NETWORK ADDRESS:", 3, 66, 70, 180, zNetAddressBuffer, 16, 0, NULL, 0);
CGameMenuItemZEdit itemNetworkJoinPort("NETWORK PORT:", 3, 66, 80, 180, zNetPortBuffer, 6, 0, NULL, 0);
CGameMenuItemChain itemNetworkJoinJoin("JOIN A GAME", 3, 66, 100, 180, 1, NULL, -1, NetworkJoinGame, 0);

// There is no better way to do this than manually.

#define MENUMOUSEFUNCTIONS 12

static char const *MenuMouseNames[MENUMOUSEFUNCTIONS] = {
    "Button 1",
    "Double Button 1",
    "Button 2",
    "Double Button 2",
    "Button 3",
    "Double Button 3",

    "Wheel Up",
    "Wheel Down",

    "Button 4",
    "Double Button 4",
    "Button 5",
    "Double Button 5",
};

static int32_t MenuMouseDataIndex[MENUMOUSEFUNCTIONS][2] = {
    { 0, 0, },
    { 0, 1, },
    { 1, 0, },
    { 1, 1, },
    { 2, 0, },
    { 2, 1, },

    // note the mouse wheel
    { 4, 0, },
    { 5, 0, },

    { 3, 0, },
    { 3, 1, },
    { 6, 0, },
    { 6, 1, },
};

void SetMouseButton(CGameMenuItemZCycle *pItem);

CGameMenuItemZCycle *pItemOptionsControlMouseButton[MENUMOUSEFUNCTIONS];

void SetupLoadingScreen(void)
{
    menuLoading.Add(&itemLoadingText, true);
}

void SetupKeyListMenu(void)
{
    menuKeys.Add(&itemKeysTitle, false);
    menuKeys.Add(&itemKeyList, true);
    menuKeys.Add(&itemBloodQAV, false);
}

void SetupMessagesMenu(void)
{
    menuMessages.Add(&itemMessagesTitle, false);
    menuMessages.Add(&boolMessages, true);
    menuMessages.Add(&sliderMsgCount, false);
    menuMessages.Add(&sliderMsgTime, false);
    menuMessages.Add(&boolMsgFont, false);
    menuMessages.Add(&boolMsgIncoming, false);
    menuMessages.Add(&boolMsgSelf, false);
    menuMessages.Add(&boolMsgOther, false);
    menuMessages.Add(&boolMsgRespawn, false);
    menuMessages.Add(&itemBloodQAV, false);
}

void SetupControlsMenu(void)
{
    sliderMouseSpeed.fValue = ClipRangeF(CONTROL_MouseSensitivity, sliderMouseSpeed.fRangeLow, sliderMouseSpeed.fRangeHigh);
    sliderTurnSpeed.nValue = ClipRange(gTurnSpeed, sliderTurnSpeed.nRangeLow, sliderTurnSpeed.nRangeHigh);
    boolMouseFlipped.at20 = gMouseAimingFlipped;
    menuControls.Add(&itemControlsTitle, false);
    menuControls.Add(&sliderMouseSpeed, true);
    menuControls.Add(&boolMouseFlipped, false);
    menuControls.Add(&sliderTurnSpeed, false);
    menuControls.Add(&itemChainKeyList, false);
    menuControls.Add(&itemChainKeyReset, false);
    menuControls.Add(&itemChainKeyResetClassic, false);
    menuControls.Add(&itemBloodQAV, false);
}

void SetupOptionsOldMenu(void)
{
    sliderDetail.nValue = ClipRange(gDetail, sliderDetail.nRangeLow, sliderDetail.nRangeHigh);
    sliderGamma.nValue = ClipRange(gGamma, sliderGamma.nRangeLow, sliderGamma.nRangeHigh);
    sliderMusic.nValue = ClipRange(MusicVolume, sliderMusic.nRangeLow, sliderMusic.nRangeHigh);
    sliderSound.nValue = ClipRange(FXVolume, sliderSound.nRangeLow, sliderSound.nRangeHigh);
    bool3DAudio.at20 = gStereo;
    itemCycleCrosshair.m_nFocus = gAimReticle % ARRAY_SSIZE(pzCrosshairStrings);
    itemCycleShowWeapons.m_nFocus = gShowWeapon;
    boolSlopeTilting.at20 = gSlopeTilting;
    boolViewBobbing.at20 = gViewVBobbing;
    boolViewSwaying.at20 = gViewHBobbing;
    boolMessages.at20 = gGameMessageMgr.state;
    menuOptionsOld.Add(&itemOptionsTitle, false);
    menuOptionsOld.Add(&itemOption1, true);
    menuOptionsOld.Add(&sliderDetail, false);
    menuOptionsOld.Add(&sliderGamma, false);
    menuOptionsOld.Add(&sliderMusic, false);
    menuOptionsOld.Add(&sliderSound, false);
    menuOptionsOld.Add(&sliderCDAudio, false);
    menuOptionsOld.Add(&bool3DAudio, false);
    menuOptionsOld.Add(&itemCycleCrosshair, false);
    menuOptionsOld.Add(&itemCycleShowWeapons, false);
    menuOptionsOld.Add(&boolSlopeTilting, false);
    menuOptionsOld.Add(&boolViewBobbing, false);
    menuOptionsOld.Add(&boolViewSwaying, false);
    menuOptionsOld.Add(&itemOption2, false);
    menuOptionsOld.Add(&itemChainParentalLock, false);
    menuOptionsOld.Add(&itemBloodQAV, false);
}

void SetupDifficultyMenu(void)
{
    menuDifficulty.Add(&itemDifficultyTitle, false);
    menuDifficulty.Add(&itemDifficulty1, false);
    menuDifficulty.Add(&itemDifficulty2, false);
    menuDifficulty.Add(&itemDifficulty3, true);
    menuDifficulty.Add(&itemDifficulty4, false);
    menuDifficulty.Add(&itemDifficulty5, false);
    menuDifficulty.Add(&itemDifficultyCustom, false);
    menuDifficulty.Add(&itemBloodQAV, false);
    itemDifficultyCustom.bDisableForNet = 1;

    menuCustomDifficulty.Add(&itemCustomDifficultyTitle, false);
    menuCustomDifficulty.Add(&itemCustomDifficultyEnemyQuantity, true);
    menuCustomDifficulty.Add(&itemCustomDifficultyEnemyHealth, false);
    menuCustomDifficulty.Add(&itemCustomDifficultyEnemyDifficulty, false);
    menuCustomDifficulty.Add(&itemCustomDifficultyPlayerDamage, false);
    menuCustomDifficulty.Add(&itemCustomDifficultyPitchfork, false);
    menuCustomDifficulty.Add(&itemCustomDifficultyMonsterBanBats, false);
    menuCustomDifficulty.Add(&itemCustomDifficultyMonsterBanRats, false);
    menuCustomDifficulty.Add(&itemCustomDifficultyMonsterBanFish, false);
    menuCustomDifficulty.Add(&itemCustomDifficultyMonsterBanHands, false);
    menuCustomDifficulty.Add(&itemCustomDifficultyMonsterBanGhosts, false);
    menuCustomDifficulty.Add(&itemCustomDifficultyMonsterBanSpiders, false);
    menuCustomDifficulty.Add(&itemCustomDifficultyMonsterBanTinyCaleb, false);
    menuCustomDifficulty.Add(&itemCustomDifficultyMonsterBanHellHounds, false);
    menuCustomDifficulty.Add(&itemCustomDifficultyStart, false);
    menuCustomDifficulty.Add(&itemBloodQAV, false);
    itemCustomDifficultyEnemyQuantity.tooltip_pzTextUpper = "";
    itemCustomDifficultyEnemyQuantity.tooltip_pzTextLower = "Set how many enemies will spawn in the level";
    itemCustomDifficultyEnemyHealth.tooltip_pzTextUpper = "";
    itemCustomDifficultyEnemyHealth.tooltip_pzTextLower = "Set enemy's starting health";
    itemCustomDifficultyEnemyDifficulty.tooltip_pzTextUpper = "";
    itemCustomDifficultyEnemyDifficulty.tooltip_pzTextLower = "Set enemy's behavior difficulty";
    itemCustomDifficultyPlayerDamage.tooltip_pzTextUpper = "";
    itemCustomDifficultyPlayerDamage.tooltip_pzTextLower = "Set player's damage taken scale";
    itemCustomDifficultyPitchfork.tooltip_pzTextUpper = "";
    itemCustomDifficultyPitchfork.tooltip_pzTextLower = "Player will lose all items on new level";
}

void SetupEpisodeMenu(void)
{
    menuEpisode.Add(&itemEpisodesTitle, false);
    int height;
    gMenuTextMgr.GetFontInfo(1, NULL, NULL, &height);
    int nOffset = 100;
    for (int i = 0; i < gEpisodeCount; i++)
    {
        EPISODEINFO *pEpisode = &gEpisodeInfo[i];
        if (!pEpisode->bloodbath || gGameOptions.nGameType != 0)
            nOffset -= 10;
    }
    nOffset = max(min(nOffset, 55), 35);
    int j = 0;
    for (int i = 0; i < gEpisodeCount; i++)
    {
        EPISODEINFO *pEpisode = &gEpisodeInfo[i];
        if (!pEpisode->bloodbath || gGameOptions.nGameType != 0)
        {
            if (j >= ARRAY_SSIZE(itemEpisodes))
                ThrowError("Too many ini episodes to display (max %d).\n", ARRAY_SSIZE(itemEpisodes));
            CGameMenuItemChain7F2F0 *pEpisodeItem = &itemEpisodes[j];
            pEpisodeItem->m_nFont = 1;
            pEpisodeItem->m_nX = 0;
            pEpisodeItem->m_nWidth = 320;
            pEpisodeItem->at20 = 1;
            pEpisodeItem->m_pzText = pEpisode->title;
            pEpisodeItem->m_nY = nOffset+(height+8)*j;
            pEpisodeItem->at34 = i;
            pEpisodeItem = &itemEpisodes[j];
            pEpisodeItem->at24 = &menuDifficulty;
            pEpisodeItem->at28 = 3;
            pEpisodeItem = &itemEpisodes[j];
            pEpisodeItem->bCanSelect = 1;
            pEpisodeItem->bEnable = 1;
            bool first = j == 0;
            menuEpisode.Add(&itemEpisodes[j], first);
            if (first)
                SetupLevelMenuItem(j);
            j++;
        }
    }

    if (j < 5) // if menu slots are not all filled, add space for user maps item
        itemUserMap.m_nY = 50+(height+8)*5;
    else
        itemUserMap.m_nY = nOffset+(height+8)*j;
    menuEpisode.Add(&itemUserMap, false);
    menuEpisode.Add(&itemBloodQAV, false);

    menuUserMap.Add(&itemUserMapTitle, true);
    menuUserMap.Add(&itemUserMapList, true);
}

void SetupMainMenu(void)
{
    menuMain.Add(&itemMainTitle, false);
    menuMain.Add(&itemMain1, true);
    if (gGameOptions.nGameType > 0)
    {
        itemMain1.at24 = &menuNetStart;
        itemMain1.at28 = 2;
    }
    else
    {
        itemMain1.at24 = &menuEpisode;
        itemMain1.at28 = -1;
    }
    menuMain.Add(&itemMain2, false);
    menuMain.Add(&itemMain3, false);
    menuMain.Add(&itemMain4, false);
    menuMain.Add(&itemMain5, false);
    menuMain.Add(&itemMain6, false);
    menuMain.Add(&itemMain7, false);
    menuMain.Add(&itemBloodQAV, false);
}

void SetupMainMenuWithSave(void)
{
    menuMainWithSave.Add(&itemMainSaveTitle, false);
    menuMainWithSave.Add(&itemMainSave1, true);
    if (gGameOptions.nGameType > 0)
    {
        itemMainSave1.at24 = &menuNetStart;
        itemMainSave1.at28 = 2;
    }
    else
    {
        itemMainSave1.at24 = &menuEpisode;
        itemMainSave1.at28 = -1;
    }
    menuMainWithSave.Add(&itemMainSave2, false);
    menuMainWithSave.Add(&itemMainSave3, false);
    menuMainWithSave.Add(&itemMainSave4, false);
    menuMainWithSave.Add(&itemMainSave5, false);
    menuMainWithSave.Add(&itemMainSave6, false);
    menuMainWithSave.Add(&itemMainSave7, false);
    menuMainWithSave.Add(&itemMainSave8, false);
    menuMainWithSave.Add(&itemBloodQAV, false);

    itemMainSave3.bEnable = !gLockManualSaving; // disable save option in main menu if lock saving mode is set
    itemMainSave3.bDisableForNet = 1;
    itemMainSave4.bDisableForNet = 1;
    itemMainSave5.bDisableForNet = 1;
    itemMainSave6.bDisableForNet = 1;
}

void SetupNetStartMenu(void)
{
    bool oneEpisode = false;
    menuNetStart.Add(&itemNetStartTitle, false);
    menuNetStart.Add(&itemNetStart1, false);
    for (int i = 0; i < (oneEpisode ? 1 : 6); i++)
    {
        EPISODEINFO *pEpisode = &gEpisodeInfo[i];
        if (i < gEpisodeCount)
            itemNetStart2.Add(pEpisode->title, i == 0);
    }
    menuNetStart.Add(&itemNetStart2, false);
    menuNetStart.Add(&itemNetStart3, false);
    menuNetStart.Add(&itemNetStart4, false);
    menuNetStart.Add(&itemNetStart5, false);
    menuNetStart.Add(&itemNetStart6, false);
    menuNetStart.Add(&itemNetStart7, false);
    menuNetStart.Add(&itemNetStart8, false);
    menuNetStart.Add(&itemNetStart9, false);
    menuNetStart.Add(&itemNetStart10, false);
    menuNetStart.Add(&itemNetStart11, false);
    menuNetStart.Add(&itemNetStart12, false);
    menuNetStart.Add(&itemNetStart13, false);
    menuNetStart.Add(&itemNetStart14, false);
    menuMultiUserMaps.Add(&itemNetStartUserMapTitle, true);
    menuMultiUserMaps.Add(&menuMultiUserMap, true);

    //////////////////////
    menuNetworkGameEnhancements.Add(&itemGameEnhancementsTitle, false);
    menuNetworkGameEnhancements.Add(&itemNetEnhancementBoolQuadDamagePowerup, true);
    menuNetworkGameEnhancements.Add(&itemNetEnhancementBoolDamageInvul, false);
    menuNetworkGameEnhancements.Add(&itemNetEnhancementExplosionBehavior, false);
    menuNetworkGameEnhancements.Add(&itemNetEnhancementProjectileBehavior, false);
    menuNetworkGameEnhancements.Add(&itemNetEnhancementEnemyBehavior, false);
    menuNetworkGameEnhancements.Add(&itemNetEnhancementBoolEnemyRandomTNT, false);
    menuNetworkGameEnhancements.Add(&itemNetEnhancementWeaponsVer, false);
    menuNetworkGameEnhancements.Add(&itemNetEnhancementSectorBehavior, false);
    menuNetworkGameEnhancements.Add(&itemNetEnhancementBoolHitscanProjectiles, false);
    menuNetworkGameEnhancements.Add(&itemNetEnhancementRandomizerMode, false);
    menuNetworkGameEnhancements.Add(&itemNetEnhancementRandomizerSeed, false);
    menuNetworkGameEnhancements.Add(&itemBloodQAV, false);
    itemNetEnhancementBoolQuadDamagePowerup.tooltip_pzTextUpper = "Replaces guns akimbo powerup";
    itemNetEnhancementBoolQuadDamagePowerup.tooltip_pzTextLower = "with Quake's quad damage";
    itemNetEnhancementBoolDamageInvul.tooltip_pzTextUpper = "Apply a short invulnerability state";
    itemNetEnhancementBoolDamageInvul.tooltip_pzTextLower = "on bullet/spirit/tesla damage";
    itemNetEnhancementExplosionBehavior.tooltip_pzTextUpper = "Uses enhanced explosion calculation";
    itemNetEnhancementProjectileBehavior.tooltip_pzTextUpper = "Use smaller hitboxes and improve collision";
    itemNetEnhancementProjectileBehavior.tooltip_pzTextLower = "accuracy for player projectiles";
    itemNetEnhancementEnemyBehavior.tooltip_pzTextUpper = "Fix various original bugs with enemies";
    itemNetEnhancementBoolEnemyRandomTNT.tooltip_pzTextUpper = "Set cultist to have a chance of";
    itemNetEnhancementBoolEnemyRandomTNT.tooltip_pzTextLower = "throwing random projectiles";
    itemNetEnhancementWeaponsVer.tooltip_pzTextUpper = "Check readme.txt for full";
    itemNetEnhancementWeaponsVer.tooltip_pzTextLower = "list of weapon changes";
    itemNetEnhancementSectorBehavior.tooltip_pzTextUpper = "Improve room over room sector handling";
    itemNetEnhancementSectorBehavior.tooltip_pzTextLower = "for hitscans (e.g: firing above water)";
    itemNetEnhancementBoolHitscanProjectiles.tooltip_pzTextUpper = "Set hitscan enemies to";
    itemNetEnhancementBoolHitscanProjectiles.tooltip_pzTextLower = "spawn bullet projectiles";
    itemNetEnhancementRandomizerMode.tooltip_pzTextUpper = "Set the randomizer's mode";
    itemNetEnhancementRandomizerSeed.tooltip_pzTextUpper = "Set the randomizer's seed";
    itemNetEnhancementRandomizerSeed.tooltip_pzTextLower = "No seed = always use a random seed";
    //////////////////////

    itemNetStart1.SetTextIndex(1);
    itemNetStart4.SetTextIndex(2);
    itemNetStart5.SetTextIndex(0);
    itemNetStart6.SetTextIndex(1);
    itemNetStart7.SetTextIndex(1);
    itemNetStart10.SetTextIndex(1);

    ///////
    itemNetEnhancementBoolQuadDamagePowerup.at20 = !!gQuadDamagePowerup;
    itemNetEnhancementBoolDamageInvul.at20 = !!gDamageInvul;
    itemNetEnhancementExplosionBehavior.m_nFocus = gExplosionBehavior % ARRAY_SSIZE(pzExplosionBehaviorStrings);
    itemNetEnhancementProjectileBehavior.m_nFocus = gProjectileBehavior % ARRAY_SSIZE(pzProjectileBehaviorStrings);
    itemNetEnhancementEnemyBehavior.at20 = !!gEnemyBehavior;
    itemNetEnhancementBoolEnemyRandomTNT.at20 = !!gEnemyRandomTNT;
    itemNetEnhancementWeaponsVer.m_nFocus = gWeaponsVer % ARRAY_SSIZE(pzWeaponsVersionStrings);
    itemNetEnhancementSectorBehavior.at20 = !!gSectorBehavior;
    itemNetEnhancementBoolHitscanProjectiles.at20 = !!gHitscanProjectiles;
    itemNetEnhancementRandomizerMode.m_nFocus = gRandomizerMode % ARRAY_SSIZE(pzRandomizerModeStrings);
    Bstrncpy(szRandomizerSeedMenu, gzRandomizerSeed, sizeof(gPacketStartGame.szRandomizerSeed));
    ///////

    menuNetStart.Add(&itemBloodQAV, false);
}

void SetupSaveGameMenu(void)
{
    menuSaveGame.Add(&itemSaveTitle, false);
    menuSaveGame.Add(&itemSaveGame0, true);
    menuSaveGame.Add(&itemSaveGame1, false);
    menuSaveGame.Add(&itemSaveGame2, false);
    menuSaveGame.Add(&itemSaveGame3, false);
    menuSaveGame.Add(&itemSaveGame4, false);
    menuSaveGame.Add(&itemSaveGame5, false);
    menuSaveGame.Add(&itemSaveGame6, false);
    menuSaveGame.Add(&itemSaveGame7, false);
    menuSaveGame.Add(&itemSaveGame8, false);
    menuSaveGame.Add(&itemSaveGame9, false);
    menuSaveGame.Add(&itemSaveGamePic, false);
    menuSaveGame.Add(&itemBloodQAV, false);

    itemSaveGame0.at2c = &itemSaveGamePic;
    if (!strcmp(strRestoreGameStrings[kLoadSaveSlot0], "<Empty>"))
        itemSaveGame0.at37 = 1;

    itemSaveGame1.at2c = &itemSaveGamePic;
    if (!strcmp(strRestoreGameStrings[kLoadSaveSlot1], "<Empty>"))
        itemSaveGame1.at37 = 1;

    itemSaveGame2.at2c = &itemSaveGamePic;
    if (!strcmp(strRestoreGameStrings[kLoadSaveSlot2], "<Empty>"))
        itemSaveGame2.at37 = 1;

    itemSaveGame3.at2c = &itemSaveGamePic;
    if (!strcmp(strRestoreGameStrings[kLoadSaveSlot3], "<Empty>"))
        itemSaveGame3.at37 = 1;

    itemSaveGame4.at2c = &itemSaveGamePic;
    if (!strcmp(strRestoreGameStrings[kLoadSaveSlot4], "<Empty>"))
        itemSaveGame4.at37 = 1;

    itemSaveGame5.at2c = &itemSaveGamePic;
    if (!strcmp(strRestoreGameStrings[kLoadSaveSlot5], "<Empty>"))
        itemSaveGame5.at37 = 1;

    itemSaveGame6.at2c = &itemSaveGamePic;
    if (!strcmp(strRestoreGameStrings[kLoadSaveSlot6], "<Empty>"))
        itemSaveGame6.at37 = 1;

    itemSaveGame7.at2c = &itemSaveGamePic;
    if (!strcmp(strRestoreGameStrings[kLoadSaveSlot7], "<Empty>"))
        itemSaveGame7.at37 = 1;

    itemSaveGame8.at2c = &itemSaveGamePic;
    if (!strcmp(strRestoreGameStrings[kLoadSaveSlot8], "<Empty>"))
        itemSaveGame8.at37 = 1;

    itemSaveGame9.at2c = &itemSaveGamePic;
    if (!strcmp(strRestoreGameStrings[kLoadSaveSlot9], "<Empty>"))
        itemSaveGame9.at37 = 1;
}

void SetupLoadGameMenu(void)
{
    menuLoadGame.Add(&itemLoadTitle, false);
    menuLoadGame.Add(&itemLoadGame0, true);
    menuLoadGame.Add(&itemLoadGame1, false);
    menuLoadGame.Add(&itemLoadGame2, false);
    menuLoadGame.Add(&itemLoadGame3, false);
    menuLoadGame.Add(&itemLoadGame4, false);
    menuLoadGame.Add(&itemLoadGame5, false);
    menuLoadGame.Add(&itemLoadGame6, false);
    menuLoadGame.Add(&itemLoadGame7, false);
    menuLoadGame.Add(&itemLoadGame8, false);
    menuLoadGame.Add(&itemLoadGame9, false);
    menuLoadGame.Add(&itemLoadGameAutosaveStart, false);
    menuLoadGame.Add(&itemLoadGameAutosaveKey, false);
    menuLoadGame.Add(&itemLoadGamePic, false);
    itemLoadGamePic.at28 = gMenuPicnum;
    itemLoadGame0.at35 = 0;
    itemLoadGame1.at35 = 0;
    itemLoadGame2.at35 = 0;
    itemLoadGame3.at35 = 0;
    itemLoadGame4.at35 = 0;
    itemLoadGame5.at35 = 0;
    itemLoadGame6.at35 = 0;
    itemLoadGame7.at35 = 0;
    itemLoadGame8.at35 = 0;
    itemLoadGame9.at35 = 0;
    itemLoadGameAutosaveStart.at35 = 0;
    itemLoadGameAutosaveKey.at35 = 0;
    itemLoadGame0.at2c = &itemLoadGamePic;
    itemLoadGame1.at2c = &itemLoadGamePic;
    itemLoadGame2.at2c = &itemLoadGamePic;
    itemLoadGame3.at2c = &itemLoadGamePic;
    itemLoadGame4.at2c = &itemLoadGamePic;
    itemLoadGame5.at2c = &itemLoadGamePic;
    itemLoadGame6.at2c = &itemLoadGamePic;
    itemLoadGame7.at2c = &itemLoadGamePic;
    itemLoadGame8.at2c = &itemLoadGamePic;
    itemLoadGame9.at2c = &itemLoadGamePic;
    itemLoadGameAutosaveStart.at2c = &itemLoadGamePic;
    itemLoadGameAutosaveKey.at2c = &itemLoadGamePic;
    menuLoadGame.Add(&itemBloodQAV, false);
}

void SetupSoundsMenu(void)
{
    itemSoundsMusic.nValue = ClipRange(MusicVolume, itemSoundsMusic.nRangeLow, itemSoundsMusic.nRangeHigh);
    itemSoundsSound.nValue = ClipRange(FXVolume, itemSoundsSound.nRangeLow, itemSoundsSound.nRangeHigh);
    menuSounds.Add(&itemSoundsTitle, false);
    menuSounds.Add(&itemSoundsMusic, true);
    menuSounds.Add(&itemSoundsSound, false);
    menuSounds.Add(&itemSoundsCDAudio, false);
    menuSounds.Add(&itemSounds3DAudio, false);
    menuSounds.Add(&itemBloodQAV, false);
}

void SetupQuitMenu(void)
{
    menuQuit.Add(&itemQuitTitle, false);
    menuQuit.Add(&itemQuitText1, false);
    menuQuit.Add(&itemQuitYesNo, true);
    menuQuit.Add(&itemBloodQAV, false);

    menuRestart.Add(&itemRestartTitle, false);
    menuRestart.Add(&itemRestartText1, false);
    menuRestart.Add(&itemRestartYesNo, true);
    menuRestart.Add(&itemBloodQAV, false);
}

void SetupHelpOrderMenu(void)
{
    menuOrder.Add(&itemHelp4QAV, true);
    menuOrder.Add(&itemHelp5QAV, false);
    menuOrder.Add(&itemHelp3QAV, false);
    menuOrder.Add(&itemHelp3BQAV, false);
    itemHelp4QAV.bEnable = 1;
    itemHelp4QAV.bNoDraw = 1;
    itemHelp5QAV.bEnable = 1;
    itemHelp5QAV.bNoDraw = 1;
    itemHelp3QAV.bEnable = 1;
    itemHelp3QAV.bNoDraw = 1;
    itemHelp3BQAV.bEnable = 1;
    itemHelp3BQAV.bNoDraw = 1;
}

void SetupCreditsMenu(void)
{
    menuCredits.Add(&itemCreditsQAV, true);
    itemCreditsQAV.bEnable = 1;
    itemCreditsQAV.bNoDraw = 1;
}

void SetupParentalLockMenu(void)
{
    itemParentalLockToggle.at20 = gbAdultContent;
    strcpy(itemParentalLockPassword.at20, gzAdultPassword);
    menuParentalLock.Add(&itemParentalLockTitle, false);
    menuParentalLock.Add(&itemParentalLockToggle, true);
    menuParentalLock.Add(&itemParentalLockPassword, false);
    menuParentalLock.Add(&itemBloodQAV, false);
}

void SetupSorry3Menu(void)
{
    menuPlayOnline.Add(&unk_26E06C, false);
    menuPlayOnline.Add(&unk_26E090, true);
    menuPlayOnline.Add(&unk_26E0E8, false);
    menuPlayOnline.Add(&unk_26E140, false);
    menuPlayOnline.Add(&unk_26E198, false);
    menuPlayOnline.Add(&unk_26E1F0, false);
    menuPlayOnline.Add(&unk_26E248, false);
    menuPlayOnline.Add(&itemBloodQAV, false);
}

void SetupSorryMenu(void)
{
    menuSorry.Add(&itemSorryPicCycle, true);
    menuSorry.Add(&itemSorryText1, false);
    menuSorry.Add(&itemSorryText3, false);
    menuSorry.Add(&itemBloodQAV, false);
}

void SetupSorry2Menu(void)
{
    menuSorry2.Add(&itemSorryPicCycle, true);
    menuSorry2.Add(&itemSorry2Text1, false);
    menuSorry2.Add(&itemSorry2Text2, false);
    menuSorry2.Add(&itemSorry2Text3, false);
    menuSorry2.Add(&itemBloodQAV, false);
}

void SetupOptionsMenu(void)
{
    menuOptions.Add(&itemOptionsTitle, false);
    menuOptions.Add(&itemOptionsChainGame, true);
    menuOptions.Add(&itemOptionsChainDisplay, false);
    menuOptions.Add(&itemOptionsChainSound, false);
    menuOptions.Add(&itemOptionsChainPlayer, false);
    menuOptions.Add(&itemOptionsChainControl, false);
    menuOptions.Add(&itemOptionsChainEnhancements, false);
    //menuOptions.Add(&itemOptionsChainOld, false);
    menuOptions.Add(&itemBloodQAV, false);

    menuOptionsGame.Add(&itemOptionsGameTitle, false);
    menuOptionsGame.Add(&itemOptionsGameBoolShowPlayerNames, true);
    menuOptionsGame.Add(&itemOptionsGameShowWeapons, false);
    menuOptionsGame.Add(&itemOptionsGameBoolSlopeTilting, false);
    menuOptionsGame.Add(&itemOptionsGameBoolViewBobbing, false);
    menuOptionsGame.Add(&itemOptionsGameBoolViewSwaying, false);
    menuOptionsGame.Add(&itemOptionsGameBoolWeaponSwaying, false);
    menuOptionsGame.Add(&itemOptionsGameBoolWeaponInterpolation, false);
    menuOptionsGame.Add(&itemOptionsGameBoolAutoAim, false);
    menuOptionsGame.Add(&itemOptionsGameWeaponSwitch, false);
    menuOptionsGame.Add(&itemOptionsGameWeaponFastSwitch, false);
    menuOptionsGame.Add(&itemOptionsGameAutosaveMode, false);
    menuOptionsGame.Add(&itemOptionsGameLockSaving, false);
    menuOptionsGame.Add(&itemOptionsGameBoolVanillaMode, false);
    itemOptionsGameAutosaveMode.bDisableForNet = 1;
    itemOptionsGameLockSaving.bDisableForNet = 1;
    itemOptionsGameBoolVanillaMode.bDisableForNet = 1;
    itemOptionsGameBoolShowPlayerNames.tooltip_pzTextUpper = "Display player's names";
    itemOptionsGameBoolShowPlayerNames.tooltip_pzTextLower = "over crosshair";
    itemOptionsGameShowWeapons.tooltip_pzTextUpper = "Display player's weapon";
    itemOptionsGameShowWeapons.tooltip_pzTextLower = "over their head";
    itemOptionsGameBoolSlopeTilting.tooltip_pzTextUpper = "Tilt view when looking";
    itemOptionsGameBoolSlopeTilting.tooltip_pzTextLower = "towards a slope";
    itemOptionsGameBoolWeaponInterpolation.tooltip_pzTextUpper = "Set QAV interpolation for";
    itemOptionsGameBoolWeaponInterpolation.tooltip_pzTextLower = "weapon sprites (experimental)";
    itemOptionsGameWeaponFastSwitch.tooltip_pzTextUpper = "Allow weapon switching while";
    itemOptionsGameWeaponFastSwitch.tooltip_pzTextLower = "weapon is being lowered/raised";
    itemOptionsGameAutosaveMode.tooltip_pzTextUpper = "Set when autosave will trigger";
    itemOptionsGameLockSaving.tooltip_pzTextUpper = "Disable manual saving/save scumming";
    itemOptionsGameBoolVanillaMode.tooltip_pzTextUpper = "Disable all non-vanilla features/enhancements";
    itemOptionsGameBoolVanillaMode.tooltip_pzTextLower = "(v1.21 compatibility mode)";

    //////////////////////
    menuOptionsGameEnhancements.Add(&itemGameEnhancementsTitle, false);
    menuOptionsGameEnhancements.Add(&itemEnhancementMonsterSettings, true);
    menuOptionsGameEnhancements.Add(&itemEnhancementBoolQuadDamagePowerup, false);
    menuOptionsGameEnhancements.Add(&itemEnhancementBoolDamageInvul, false);
    menuOptionsGameEnhancements.Add(&itemEnhancementExplosionBehavior, false);
    menuOptionsGameEnhancements.Add(&itemEnhancementProjectileBehavior, false);
    menuOptionsGameEnhancements.Add(&itemEnhancementEnemyBehavior, false);
    menuOptionsGameEnhancements.Add(&itemEnhancementBoolEnemyRandomTNT, false);
    menuOptionsGameEnhancements.Add(&itemEnhancementWeaponsVer, false);
    menuOptionsGameEnhancements.Add(&itemEnhancementSectorBehavior, false);
    menuOptionsGameEnhancements.Add(&itemEnhancementBoolHitscanProjectiles, false);
    menuOptionsGameEnhancements.Add(&itemEnhancementRandomizerMode, false);
    menuOptionsGameEnhancements.Add(&itemEnhancementRandomizerSeed, false);
    menuOptionsGameEnhancements.Add(&itemBloodQAV, false);
    itemOptionsChainEnhancements.bDisableForNet = 1;
    itemEnhancementMonsterSettings.tooltip_pzTextUpper = "Set the monster settings";
    itemEnhancementMonsterSettings.tooltip_pzTextLower = "for single-player mode";
    itemEnhancementBoolQuadDamagePowerup.tooltip_pzTextUpper = "Replaces guns akimbo powerup";
    itemEnhancementBoolQuadDamagePowerup.tooltip_pzTextLower = "with Quake's quad damage";
    itemEnhancementBoolDamageInvul.tooltip_pzTextUpper = "Apply a short invulnerability state";
    itemEnhancementBoolDamageInvul.tooltip_pzTextLower = "on bullet/spirit/tesla damage";
    itemEnhancementExplosionBehavior.tooltip_pzTextUpper = "Uses enhanced explosion calculation";
    itemEnhancementProjectileBehavior.tooltip_pzTextUpper = "Use smaller hitboxes and improve collision";
    itemEnhancementProjectileBehavior.tooltip_pzTextLower = "accuracy for player projectiles";
    itemEnhancementEnemyBehavior.tooltip_pzTextUpper = "Fix various original bugs with enemies";
    itemEnhancementBoolEnemyRandomTNT.tooltip_pzTextUpper = "Set cultist to have a chance of";
    itemEnhancementBoolEnemyRandomTNT.tooltip_pzTextLower = "throwing random projectiles";
    itemEnhancementWeaponsVer.tooltip_pzTextUpper = "Check readme.txt for full";
    itemEnhancementWeaponsVer.tooltip_pzTextLower = "list of weapon changes";
    itemEnhancementSectorBehavior.tooltip_pzTextUpper = "Improve room over room sector handling";
    itemEnhancementSectorBehavior.tooltip_pzTextLower = "for hitscans (e.g: firing above water)";
    itemEnhancementBoolHitscanProjectiles.tooltip_pzTextUpper = "Set hitscan enemies to";
    itemEnhancementBoolHitscanProjectiles.tooltip_pzTextLower = "spawn bullet projectiles";
    itemEnhancementRandomizerMode.tooltip_pzTextUpper = "Set the randomizer's mode";
    itemEnhancementRandomizerSeed.tooltip_pzTextUpper = "Set the randomizer's seed";
    itemEnhancementRandomizerSeed.tooltip_pzTextLower = "No seed = always use a random seed";
    /////////////////////

    //menuOptionsGame.Add(&itemOptionsGameChainParentalLock, false);
    menuOptionsGame.Add(&itemBloodQAV, false);
    itemOptionsGameBoolShowPlayerNames.at20 = gShowPlayerNames;
    itemOptionsGameShowWeapons.m_nFocus = gShowWeapon;
    itemOptionsGameBoolSlopeTilting.at20 = gSlopeTilting;
    itemOptionsGameBoolViewBobbing.at20 = gViewVBobbing;
    itemOptionsGameBoolViewSwaying.at20 = gViewHBobbing;
    itemOptionsGameBoolWeaponSwaying.m_nFocus = gWeaponHBobbing % ARRAY_SSIZE(pzWeaponHBobbingStrings);
    itemOptionsGameBoolWeaponInterpolation.m_nFocus = gWeaponInterpolate % ARRAY_SSIZE(pzWeaponInterpolateStrings);
    itemOptionsGameBoolAutoAim.m_nFocus = gAutoAim;
    itemOptionsGameWeaponSwitch.m_nFocus = (gWeaponSwitch&1) ? ((gWeaponSwitch&2) ? 1 : 2) : 0;
    itemOptionsGameWeaponFastSwitch.at20 = !!gWeaponFastSwitch;
    itemOptionsGameAutosaveMode.m_nFocus = gAutosave % ARRAY_SSIZE(pzAutosaveModeStrings);
    itemOptionsGameLockSaving.at20 = !!gLockManualSaving;
    itemMainSave3.bEnable = !gLockManualSaving; // disable save option in main menu if lock saving mode is set
    itemOptionsGameBoolVanillaMode.m_nFocus = gVanilla % ARRAY_SSIZE(pzVanillaModeStrings);
    SetGameVanillaMode(gVanilla); // enable/disable menu items depending on current vanilla mode state

    ///////
    itemEnhancementMonsterSettings.m_nFocus = gMonsterSettings % ARRAY_SSIZE(pzMonsterStrings);
    itemEnhancementBoolQuadDamagePowerup.at20 = !!gQuadDamagePowerup;
    itemEnhancementBoolDamageInvul.at20 = !!gDamageInvul;
    itemEnhancementExplosionBehavior.m_nFocus = gExplosionBehavior % ARRAY_SSIZE(pzExplosionBehaviorStrings);
    itemEnhancementProjectileBehavior.m_nFocus = gProjectileBehavior % ARRAY_SSIZE(pzProjectileBehaviorStrings);
    itemEnhancementWeaponsVer.m_nFocus = gWeaponsVer % ARRAY_SSIZE(pzWeaponsVersionStrings);
    itemEnhancementSectorBehavior.at20 = !!gSectorBehavior;
    itemEnhancementEnemyBehavior.at20 = !!gEnemyBehavior;
    itemEnhancementBoolEnemyRandomTNT.at20 = !!gEnemyRandomTNT;
    itemEnhancementBoolHitscanProjectiles.at20 = !!gHitscanProjectiles;
    itemEnhancementRandomizerMode.m_nFocus = gRandomizerMode % ARRAY_SSIZE(pzRandomizerModeStrings);
    Bstrncpy(szRandomizerSeedMenu, gzRandomizerSeed, sizeof(szRandomizerSeedMenu));
    ///////

    menuOptionsDisplay.Add(&itemOptionsDisplayTitle, false);
    menuOptionsDisplay.Add(&itemOptionsDisplayColor, true);
    menuOptionsDisplay.Add(&itemOptionsDisplayMode, false);
    menuOptionsDisplay.Add(&itemOptionsDisplayCrosshair, false);
    menuOptionsDisplay.Add(&itemOptionsDisplayBoolCenterHoriz, false);
    menuOptionsDisplay.Add(&itemOptionsDisplayLevelStats, false);
    menuOptionsDisplay.Add(&itemOptionsDisplayPowerupDuration, false);
    menuOptionsDisplay.Add(&itemOptionsDisplayBoolShowMapTitle, false);
    menuOptionsDisplay.Add(&itemOptionsDisplayBoolMessages, false);
    menuOptionsDisplay.Add(&itemOptionsDisplayBoolWidescreen, false);
    menuOptionsDisplay.Add(&itemOptionsDisplayHudRatio, false);
    menuOptionsDisplay.Add(&itemOptionsDisplayMirrorMode, false);
    menuOptionsDisplay.Add(&itemOptionsDisplayWeaponSelect, false);
    menuOptionsDisplay.Add(&itemOptionsDisplayBoolSlowRoomFlicker, false);
    menuOptionsDisplay.Add(&itemOptionsDisplayFOV, false);
#ifdef USE_OPENGL
    menuOptionsDisplay.Add(&itemOptionsDisplayPolymost, false);
#endif
    menuOptionsDisplay.Add(&itemBloodQAV, false);
    itemOptionsDisplayCrosshair.m_nFocus = gAimReticle % ARRAY_SSIZE(pzCrosshairStrings);
    itemOptionsDisplayBoolCenterHoriz.at20 = gCenterHoriz;
    itemOptionsDisplayLevelStats.m_nFocus = gLevelStats % ARRAY_SSIZE(pzStatsPowerupRatioStrings);
    itemOptionsDisplayPowerupDuration.m_nFocus = gPowerupDuration % ARRAY_SSIZE(pzStatsPowerupRatioStrings);
    itemOptionsDisplayBoolShowMapTitle.at20 = gShowMapTitle;
    itemOptionsDisplayBoolMessages.at20 = gMessageState;
    itemOptionsDisplayBoolWidescreen.at20 = r_usenewaspect;
    itemOptionsDisplayHudRatio.m_nFocus = gHudRatio % ARRAY_SSIZE(pzHudRatioStrings);
    itemOptionsDisplayMirrorMode.m_nFocus = r_mirrormode % ARRAY_SSIZE(pzMirrorModeStrings);
    itemOptionsDisplayWeaponSelect.m_nFocus = gShowWeaponSelect % ARRAY_SSIZE(pzWeaponSelectStrings);
    itemOptionsDisplayBoolSlowRoomFlicker.at20 = gSlowRoomFlicker;

    menuOptionsDisplayMode.Add(&itemOptionsDisplayModeTitle, false);
    menuOptionsDisplayMode.Add(&itemOptionsDisplayModeResolution, true);
    // prepare video setup
    for (int i = 0; i < validmodecnt; ++i)
    {
        int j;
        for (j = 0; j < gResolutionNum; ++j)
        {
            if (validmode[i].xdim == gResolution[j].xdim && validmode[i].ydim == gResolution[j].ydim)
            {
                gResolution[j].flags |= validmode[i].fs ? RES_FS : RES_WIN;
                Bsnprintf(gResolution[j].name, MAXRESOLUTIONSTRINGLENGTH, "%d x %d%s", gResolution[j].xdim, gResolution[j].ydim, (gResolution[j].flags & RES_FS) ? "" : "Win");
                gResolutionName[j] = gResolution[j].name;
                if (validmode[i].bpp > gResolution[j].bppmax)
                    gResolution[j].bppmax = validmode[i].bpp;
                break;
            }
        }

        if (j == gResolutionNum) // no match found
        {
            gResolution[j].xdim = validmode[i].xdim;
            gResolution[j].ydim = validmode[i].ydim;
            gResolution[j].bppmax = validmode[i].bpp;
            gResolution[j].flags = validmode[i].fs ? RES_FS : RES_WIN;
            Bsnprintf(gResolution[j].name, MAXRESOLUTIONSTRINGLENGTH, "%d x %d%s", gResolution[j].xdim, gResolution[j].ydim, (gResolution[j].flags & RES_FS) ? "" : "Win");
            gResolutionName[j] = gResolution[j].name;
            ++gResolutionNum;
        }
    }
    SetupVideoModeMenu(NULL);
    itemOptionsDisplayModeResolution.SetTextArray(gResolutionName, gResolutionNum, 0);
#ifdef USE_OPENGL
    menuOptionsDisplayMode.Add(&itemOptionsDisplayModeRenderer, false);
#endif
    menuOptionsDisplayMode.Add(&itemOptionsDisplayModeFullscreen, false);
#ifdef USE_OPENGL
    menuOptionsDisplayMode.Add(&itemOptionsDisplayModeVSync, false);
#endif
    menuOptionsDisplayMode.Add(&itemOptionsDisplayModeFrameLimit, false);
    //menuOptionsDisplayMode.Add(&itemOptionsDisplayModeFPSOffset, false);
    menuOptionsDisplayMode.Add(&itemOptionsDisplayModeApply, false);
    menuOptionsDisplayMode.Add(&itemBloodQAV, false);

#ifdef USE_OPENGL
    itemOptionsDisplayModeRenderer.pPreDrawCallback = PreDrawVideoModeMenu;
#endif
    itemOptionsDisplayModeFullscreen.pPreDrawCallback = PreDrawVideoModeMenu;
    //itemOptionsDisplayModeFPSOffset.pPreDrawCallback = PreDrawVideoModeMenu;

    menuOptionsDisplayColor.Add(&itemOptionsDisplayColorTitle, false);
    menuOptionsDisplayColor.Add(&itemOptionsDisplayColorPaletteCustom, true);
    menuOptionsDisplayColor.Add(&itemOptionsDisplayColorPaletteCIEDE2000, false);
    menuOptionsDisplayColor.Add(&itemOptionsDisplayColorPaletteGrayscale, false);
    menuOptionsDisplayColor.Add(&itemOptionsDisplayColorPaletteInvert, false);
    menuOptionsDisplayColor.Add(&itemOptionsDisplayColorGamma, false);
    menuOptionsDisplayColor.Add(&itemOptionsDisplayColorContrast, false);
    menuOptionsDisplayColor.Add(&itemOptionsDisplayColorBrightness, false);
    menuOptionsDisplayColor.Add(&itemOptionsDisplayColorVisibility, false);
    menuOptionsDisplayColor.Add(&itemOptionsDisplayColorReset, false);
    menuOptionsDisplayColor.Add(&itemBloodQAV, false);

    itemOptionsDisplayColorPaletteCustom.m_nFocus = gCustomPalette % ARRAY_SSIZE(srcCustomPaletteStr);
    itemOptionsDisplayColorPaletteCIEDE2000.at20 = gCustomPaletteCIEDE2000;
    itemOptionsDisplayColorPaletteGrayscale.at20 = gCustomPaletteGrayscale;
    itemOptionsDisplayColorPaletteInvert.at20 = gCustomPaletteInvert;
    itemOptionsDisplayColorContrast.pPreDrawCallback = PreDrawDisplayColor;
    itemOptionsDisplayColorBrightness.pPreDrawCallback = PreDrawDisplayColor;

#ifdef USE_OPENGL
    menuOptionsDisplayPolymost.Add(&itemOptionsDisplayPolymostTitle, false);
    //menuOptionsDisplayPolymost.Add(&itemOptionsDisplayPolymostTextureMode, true);
    //menuOptionsDisplayPolymost.Add(&itemOptionsDisplayPolymostAnisotropy, false);
    menuOptionsDisplayPolymost.Add(&itemOptionsDisplayPolymostTrueColorTextures, true);
    menuOptionsDisplayPolymost.Add(&itemOptionsDisplayPolymostTexQuality, false);
    menuOptionsDisplayPolymost.Add(&itemOptionsDisplayPolymostPreloadCache, false);
    menuOptionsDisplayPolymost.Add(&itemOptionsDisplayPolymostTexCache, false);
    menuOptionsDisplayPolymost.Add(&itemOptionsDisplayPolymostDetailTex, false);
    menuOptionsDisplayPolymost.Add(&itemOptionsDisplayPolymostGlowTex, false);
    menuOptionsDisplayPolymost.Add(&itemOptionsDisplayPolymost3DModels, false);
    menuOptionsDisplayPolymost.Add(&itemOptionsDisplayPolymostDeliriumBlur, false);
    menuOptionsDisplayPolymost.Add(&itemOptionsDisplayPolymostUseColorIndexedTex, false);
    menuOptionsDisplayPolymost.Add(&itemBloodQAV, false);

    itemOptionsDisplayPolymostTexQuality.pPreDrawCallback = PreDrawDisplayPolymost;
    itemOptionsDisplayPolymostPreloadCache.pPreDrawCallback = PreDrawDisplayPolymost;
    itemOptionsDisplayPolymostTexCache.pPreDrawCallback = PreDrawDisplayPolymost;
    itemOptionsDisplayPolymostDetailTex.pPreDrawCallback = PreDrawDisplayPolymost;
    itemOptionsDisplayPolymostGlowTex.pPreDrawCallback = PreDrawDisplayPolymost;
#endif

    menuOptionsSound.Add(&itemOptionsSoundTitle, false);
    menuOptionsSound.Add(&itemOptionsSoundSoundToggle, true);
    menuOptionsSound.Add(&itemOptionsSoundMusicToggle, false);
    menuOptionsSound.Add(&itemOptionsSoundMonoStereo, false);
    menuOptionsSound.Add(&itemOptionsSoundDoppler, false);
    menuOptionsSound.Add(&itemOptionsSoundSoundVolume, false);
    menuOptionsSound.Add(&itemOptionsSoundMusicVolume, false);
    menuOptionsSound.Add(&itemOptionsSoundSampleRate, false);
    menuOptionsSound.Add(&itemOptionsSoundNumVoices, false);
    menuOptionsSound.Add(&itemOptionsSoundSpeakerAngle, false);
    menuOptionsSound.Add(&itemOptionsSoundCalebTalk, false);
    menuOptionsSound.Add(&itemOptionsSoundCDToggle, false);
    menuOptionsSound.Add(&itemOptionsSoundMusicDevice, false);
    menuOptionsSound.Add(&itemOptionsSoundSF2Bank, false);
    itemOptionsSoundSpeakerAngle.tooltip_pzTextUpper = "";
    itemOptionsSoundSpeakerAngle.tooltip_pzTextLower = "Set left/right speaker offset angle (in degrees)";
    itemOptionsSoundCDToggle.tooltip_pzTextUpper = "";
    itemOptionsSoundCDToggle.tooltip_pzTextLower = "Use blood##.ogg/flac files in base folder as redbook audio";

    menuOptionsSound.Add(&itemOptionsSoundApplyChanges, false);
    menuOptionsSound.Add(&itemBloodQAV, false);

    menuOptionsSoundSF2.Add(&itemOptionsSoundSF2Title, true);
    menuOptionsSoundSF2.Add(&itemOptionsSoundSF2FS, true);

    menuOptionsPlayer.Add(&itemOptionsPlayerTitle, false);
    menuOptionsPlayer.Add(&itemOptionsPlayerName, true);
    menuOptionsPlayer.Add(&itemOptionsPlayerSkill, false);
    menuOptionsPlayer.Add(&itemOptionsPlayerChatSound, false);
    menuOptionsPlayer.Add(&itemOptionsPlayerKillMsg, false);
    menuOptionsPlayer.Add(&itemOptionsPlayerMultiKill, false);
    menuOptionsPlayer.Add(&itemBloodQAV, false);
    itemOptionsPlayerSkill.tooltip_pzTextUpper = "Set player's damage taken handicap";
    itemOptionsPlayerSkill.tooltip_pzTextLower = "(only for multiplayer)";
    itemOptionsPlayerKillMsg.tooltip_pzTextUpper = "Show player killed on screen";
    itemOptionsPlayerKillMsg.tooltip_pzTextLower = "(for bloodbath/teams multiplayer)";
    itemOptionsPlayerMultiKill.tooltip_pzTextUpper = "Show multi kill alerts on screen";
    itemOptionsPlayerMultiKill.tooltip_pzTextLower = "(for bloodbath/teams multiplayer)";

    itemOptionsPlayerSkill.m_nFocus = 4 - (gSkill % ARRAY_SSIZE(pzPlayerSkillStrings)); // invert because string order is reversed (lower skill == easier)
    itemOptionsPlayerChatSound.at20 = gChatSnd;
    itemOptionsPlayerKillMsg.at20 = gKillMsg;
    itemOptionsPlayerMultiKill.m_nFocus = gMultiKill % ARRAY_SSIZE(pzPlayerMultiKillStrings);

    menuOptionsControl.Add(&itemOptionsControlTitle, false);
    menuOptionsControl.Add(&itemOptionsControlKeyboard, true);
    menuOptionsControl.Add(&itemOptionsControlMouse, false);
    menuOptionsControl.Add(&itemBloodQAV, false);

    menuOptionsControlKeyboard.Add(&itemOptionsControlKeyboardTitle, false);
    menuOptionsControlKeyboard.Add(&itemOptionsControlKeyboardSliderTurnSpeed, true);
    menuOptionsControlKeyboard.Add(&itemOptionsControlKeyboardBoolCrouchToggle, false);
    menuOptionsControlKeyboard.Add(&itemOptionsControlKeyboardBoolAutoRun, false);
    menuOptionsControlKeyboard.Add(&itemOptionsControlKeyboardList, false);
    menuOptionsControlKeyboard.Add(&itemOptionsControlKeyboardReset, false);
    menuOptionsControlKeyboard.Add(&itemOptionsControlKeyboardResetClassic, false);
    menuOptionsControlKeyboard.Add(&itemBloodQAV, false);

    itemOptionsControlKeyboardSliderTurnSpeed.nValue = gTurnSpeed;
    itemOptionsControlKeyboardBoolCrouchToggle.at20 = gCrouchToggle;
    itemOptionsControlKeyboardBoolAutoRun.at20 = gAutoRun;

    menuOptionsControlMouse.Add(&itemOptionsControlMouseTitle, false);
    menuOptionsControlMouse.Add(&itemOptionsControlMouseButton, true);
    menuOptionsControlMouse.Add(&itemOptionsControlMouseSensitivity, false);
    menuOptionsControlMouse.Add(&itemOptionsControlMouseAimFlipped, false);
    menuOptionsControlMouse.Add(&itemOptionsControlMouseAimMode, false);
    menuOptionsControlMouse.Add(&itemOptionsControlMouseVerticalAim, false);
    menuOptionsControlMouse.Add(&itemOptionsControlMouseXSensitivity, false);
    menuOptionsControlMouse.Add(&itemOptionsControlMouseYSensitivity, false);
    menuOptionsControlMouse.Add(&itemBloodQAV, false);

    itemOptionsControlMouseVerticalAim.pPreDrawCallback = PreDrawControlMouse;

    menuOptionsControlMouseButtonAssignment.Add(&itemOptionsControlMouseTitle, false);
    int y = 60;
    for (int i = 0; i < MENUMOUSEFUNCTIONS; i++)
    {
        pItemOptionsControlMouseButton[i] = new CGameMenuItemZCycle(MenuMouseNames[i], 3, 66, y, 180, 0, SetMouseButton, pzGamefuncsStrings, NUMGAMEFUNCTIONS+1, 0, true);
        dassert(pItemOptionsControlMouseButton[i] != NULL);
        menuOptionsControlMouseButtonAssignment.Add(pItemOptionsControlMouseButton[i], i == 0);
        y += 10;
    }
    menuOptionsControlMouseButtonAssignment.Add(&itemBloodQAV, false);
}

void SetupMenus(void)
{
    // prepare gamefuncs and keys
    pzGamefuncsStrings[0] = MenuGameFuncNone;
    nGamefuncsValues[0] = -1;
    int k = 1;
    for (int i = 0; i < NUMGAMEFUNCTIONS; ++i)
    {
        Bstrcpy(MenuGameFuncs[i], gamefunctions[i]);

        for (int j = 0; j < MAXGAMEFUNCLEN; ++j)
            if (MenuGameFuncs[i][j] == '_')
                MenuGameFuncs[i][j] = ' ';

        if (gamefunctions[i][0] != '\0')
        {
            pzGamefuncsStrings[k] = MenuGameFuncs[i];
            nGamefuncsValues[k] = i;
            ++k;
        }
    }

    nGamefuncsNum = k;

    SetupLoadingScreen();
    SetupKeyListMenu();
    SetupMessagesMenu();
    SetupControlsMenu();
    SetupSaveGameMenu();
    SetupLoadGameMenu();
    SetupOptionsOldMenu();
    SetupCreditsMenu();
    SetupHelpOrderMenu();
    SetupSoundsMenu();
    SetupDifficultyMenu();
    SetupEpisodeMenu();
    SetupMainMenu();
    SetupMainMenuWithSave();
    SetupNetStartMenu();
    SetupQuitMenu();
    SetupParentalLockMenu();
    SetupSorryMenu();
    SetupSorry2Menu();
    SetupSorry3Menu();

    SetupOptionsMenu();
    SetupNetworkMenu();
}

void UpdateNetworkMenus(void)
{
    if (gGameOptions.nGameType > 0)
    {
        itemMain1.at24 = &menuNetStart;
        itemMain1.at28 = 2;
    }
    else
    {
        itemMain1.at24 = &menuEpisode;
        itemMain1.at28 = -1;
    }
    if (gGameOptions.nGameType > 0)
    {
        itemMainSave1.at24 = &menuNetStart;
        itemMainSave1.at28 = 2;
    }
    else
    {
        itemMainSave1.at24 = &menuEpisode;
        itemMainSave1.at28 = -1;
    }
}

void SetMonoStereo(CGameMenuItemZBool *pItem)
{
    gStereo = pItem->at20;
    itemOptionsSoundDoppler.bEnable = gStereo;
}

void SetCrosshair(CGameMenuItemZCycle *pItem)
{
    gAimReticle = pItem->m_nFocus % ARRAY_SSIZE(pzCrosshairStrings);
}

void SetCenterHoriz(CGameMenuItemZBool *pItem)
{
    gCenterHoriz = pItem->at20;
}

void ResetKeys(CGameMenuItemChain *)
{
    CONFIG_SetDefaultKeys(keydefaults);
}

void ResetKeysClassic(CGameMenuItemChain *)
{
    CONFIG_SetDefaultKeys(oldkeydefaults);
}

////
void SetMonsters(CGameMenuItemZCycle *pItem)
{
    if ((gGameOptions.nGameType == 0) && (numplayers == 1)) {
        gMonsterSettings = pItem->m_nFocus % ARRAY_SSIZE(pzMonsterStrings);
        if (!VanillaMode())
        {
            gGameOptions.nMonsterSettings = ClipRange(gMonsterSettings, 0, 2);
            if (gMonsterSettings <= 1)
                gGameOptions.nMonsterRespawnTime = 3600; // default (30 secs)
            else if (gMonsterSettings == 2)
                gGameOptions.nMonsterRespawnTime = 15 * kTicRate; // 15 secs
            else
                gGameOptions.nMonsterRespawnTime = (gMonsterSettings - 2) * 30 * kTicRate;
        }
    } else {
        pItem->m_nFocus = gMonsterSettings % ARRAY_SSIZE(pzMonsterStrings);
    }
}

void SetQuadDamagePowerup(CGameMenuItemZBool* pItem)
{
    if ((gGameOptions.nGameType == 0) && (numplayers == 1)) {
        gQuadDamagePowerup = pItem->at20;
        gGameOptions.bQuadDamagePowerup = gQuadDamagePowerup;
    } else {
        pItem->at20 = gQuadDamagePowerup;
    }
}

void SetDamageInvul(CGameMenuItemZBool* pItem)
{
    if ((gGameOptions.nGameType == 0) && (numplayers == 1)) {
        gDamageInvul = pItem->at20;
        gGameOptions.bDamageInvul = gDamageInvul;
    } else {
        pItem->at20 = !!gDamageInvul;
    }
}

void SetExplosionBehavior(CGameMenuItemZCycle *pItem)
{
    if ((gGameOptions.nGameType == 0) && (numplayers == 1)) {
        gExplosionBehavior = pItem->m_nFocus % ARRAY_SSIZE(pzExplosionBehaviorStrings);
        gGameOptions.nExplosionBehavior = gExplosionBehavior;
    } else {
        pItem->m_nFocus = gExplosionBehavior % ARRAY_SSIZE(pzExplosionBehaviorStrings);
    }
}

void SetProjectileBehavior(CGameMenuItemZCycle *pItem)
{
    if ((gGameOptions.nGameType == 0) && (numplayers == 1)) {
        gProjectileBehavior = pItem->m_nFocus % ARRAY_SSIZE(pzProjectileBehaviorStrings);
        gGameOptions.nProjectileBehavior = gProjectileBehavior;
    } else {
        pItem->m_nFocus = gProjectileBehavior % ARRAY_SSIZE(pzProjectileBehaviorStrings);
    }
}

void SetEnemyBehavior(CGameMenuItemZBool *pItem)
{
    if ((gGameOptions.nGameType == 0) && (numplayers == 1)) {
        gEnemyBehavior = pItem->at20;
        gGameOptions.bEnemyBehavior = gEnemyBehavior;
    } else {
        pItem->at20 = !!gEnemyBehavior;
    }
}

void SetEnemyRandomTNT(CGameMenuItemZBool* pItem)
{
    if ((gGameOptions.nGameType == 0) && (numplayers == 1)) {
        gEnemyRandomTNT = pItem->at20;
        gGameOptions.bEnemyRandomTNT = gEnemyRandomTNT;
    } else {
        pItem->at20 = !!gEnemyRandomTNT;
    }
}

void SetWeaponsVer(CGameMenuItemZCycle* pItem)
{
    if ((gGameOptions.nGameType == 0) && (numplayers == 1)) {
        gWeaponsVer = pItem->m_nFocus % ARRAY_SSIZE(pzWeaponsVersionStrings);
        gGameOptions.nWeaponsVer = gWeaponsVer;
    } else {
        pItem->m_nFocus = gWeaponsVer % ARRAY_SSIZE(pzWeaponsVersionStrings);
    }
}

void SetSectorBehavior(CGameMenuItemZBool *pItem)
{
    if ((gGameOptions.nGameType == 0) && (numplayers == 1)) {
        gSectorBehavior = pItem->at20;
        gGameOptions.bSectorBehavior = gSectorBehavior;
    } else {
        pItem->at20 = !!gSectorBehavior;
    }
}

void SetHitscanProjectiles(CGameMenuItemZBool *pItem)
{
    if ((gGameOptions.nGameType == 0) && (numplayers == 1)) {
        gHitscanProjectiles = pItem->at20;
        gGameOptions.bHitscanProjectiles = gHitscanProjectiles;
    } else {
        pItem->at20 = !!gHitscanProjectiles;
    }
}

void SetRandomizerMode(CGameMenuItemZCycle *pItem)
{
    if ((gGameOptions.nGameType == 0) && (numplayers == 1)) {
        gRandomizerMode = pItem->m_nFocus % ARRAY_SSIZE(pzRandomizerModeStrings);
        gGameOptions.nRandomizerMode = gRandomizerMode;
    } else {
        pItem->m_nFocus = gRandomizerMode % ARRAY_SSIZE(pzRandomizerModeStrings);
    }
}

void SetRandomizerSeed(CGameMenuItemZEdit *pItem, CGameMenuEvent *pEvent)
{
    UNREFERENCED_PARAMETER(pItem);
    UNREFERENCED_PARAMETER(pEvent);
    Bstrncpy(gzRandomizerSeed, szRandomizerSeedMenu, sizeof(gzRandomizerSeed));
    if ((gGameOptions.nGameType == 0) && (numplayers == 1))
        Bstrncpy(gGameOptions.szRandomizerSeed, gzRandomizerSeed, sizeof(gGameOptions.szRandomizerSeed));
}
////

void SetShowPlayerNames(CGameMenuItemZBool *pItem)
{
    gShowPlayerNames = pItem->at20;
}

void SetShowWeapons(CGameMenuItemZCycle *pItem)
{
    gShowWeapon = pItem->m_nFocus;
}

void SetSlopeTilting(CGameMenuItemZBool *pItem)
{
    gSlopeTilting = pItem->at20;
}

void SetViewBobbing(CGameMenuItemZBool *pItem)
{
    gViewVBobbing = pItem->at20;
}

void SetViewSwaying(CGameMenuItemZBool *pItem)
{
    gViewHBobbing = pItem->at20;
}

void SetWeaponSwaying(CGameMenuItemZCycle *pItem)
{
    gWeaponHBobbing = pItem->m_nFocus % ARRAY_SSIZE(pzWeaponHBobbingStrings);
    if (!gDemo.bRecording && !gDemo.bPlaying)
    {
        if ((numplayers > 1) || (gGameOptions.nGameType > 0)) // if multiplayer session is active
        {
            netBroadcastPlayerInfoUpdate(myconnectindex);
            return;
        }
        gProfile[myconnectindex].nWeaponHBobbing = gWeaponHBobbing;
        netBroadcastPlayerInfo(myconnectindex);
    }
}

void SetWeaponInterpolate(CGameMenuItemZCycle *pItem)
{
    gWeaponInterpolate = pItem->m_nFocus % ARRAY_SSIZE(pzWeaponInterpolateStrings);
}

void SetDetail(CGameMenuItemSlider *pItem)
{
    gDetail = pItem->nValue;
}

void SetGamma(CGameMenuItemSlider *pItem)
{
    gGamma = pItem->nValue;
    scrSetGamma(gGamma);
}

void SetMusicVol(CGameMenuItemSlider *pItem)
{
    sndSetMusicVolume(pItem->nValue);
}

void SetSoundVol(CGameMenuItemSlider *pItem)
{
    sndSetFXVolume(pItem->nValue);
}

void SetCDVol(CGameMenuItemSlider *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
    // NUKE-TODO:
}

void SetMessages(CGameMenuItemZBool *pItem)
{
    gMessageState = pItem->at20;
    gGameMessageMgr.SetState(gMessageState);
}

void SetMouseSensitivity(CGameMenuItemSliderFloat *pItem)
{
	CONTROL_MouseSensitivity = pItem->fValue;
}

void SetMouseAimFlipped(CGameMenuItemZBool *pItem)
{
    gMouseAimingFlipped = pItem->at20;
}

void SetTurnSpeed(CGameMenuItemSlider *pItem)
{
    gTurnSpeed = pItem->nValue;
}

void SetCrouchToggle(CGameMenuItemZBool *pItem)
{
    gCrouchToggle = pItem->at20;
}

void SetAutoRun(CGameMenuItemZBool *pItem)
{
    gAutoRun = pItem->at20;
}

void SetAutoAim(CGameMenuItemZCycle *pItem)
{
    gAutoAim = pItem->m_nFocus;
    if (!gDemo.bRecording && !gDemo.bPlaying)
    {
        if ((numplayers > 1) || (gGameOptions.nGameType > 0)) // if multiplayer session is active
        {
            netBroadcastPlayerInfoUpdate(myconnectindex);
            return;
        }
        gProfile[myconnectindex].nAutoAim = gAutoAim;
        netBroadcastPlayerInfo(myconnectindex);
    }
}

void SetLevelStats(CGameMenuItemZCycle *pItem)
{
    gLevelStats = pItem->m_nFocus % ARRAY_SSIZE(pzStatsPowerupRatioStrings);
    viewResizeView(gViewSize);
}

void SetPowerupDuration(CGameMenuItemZCycle* pItem)
{
    gPowerupDuration = pItem->m_nFocus % ARRAY_SSIZE(pzStatsPowerupRatioStrings);
    viewResizeView(gViewSize);
}

void SetShowMapTitle(CGameMenuItemZBool* pItem)
{
    gShowMapTitle = pItem->at20;
}

void SetWeaponSwitch(CGameMenuItemZCycle *pItem)
{
    gWeaponSwitch &= ~(1|2);
    switch (pItem->m_nFocus)
    {
    case 0:
        break;
    case 1:
        gWeaponSwitch |= 2;
        fallthrough__;
    case 2:
    default:
        gWeaponSwitch |= 1;
        break;
    }
    if (!gDemo.bRecording && !gDemo.bPlaying)
    {
        if ((numplayers > 1) || (gGameOptions.nGameType > 0)) // if multiplayer session is active
        {
            netBroadcastPlayerInfoUpdate(myconnectindex);
            return;
        }
        gProfile[myconnectindex].nWeaponSwitch = gWeaponSwitch;
        netBroadcastPlayerInfo(myconnectindex);
    }
}

void SetWeaponFastSwitch(CGameMenuItemZBool *pItem)
{
    gWeaponFastSwitch = pItem->at20;
    if (!gDemo.bRecording && !gDemo.bPlaying)
    {
        if ((numplayers > 1) || (gGameOptions.nGameType > 0)) // if multiplayer session is active
        {
            netBroadcastPlayerInfoUpdate(myconnectindex);
            return;
        }
        gProfile[myconnectindex].bWeaponFastSwitch = gWeaponFastSwitch;
        netBroadcastPlayerInfo(myconnectindex);
    }
}

void SetAutosaveMode(CGameMenuItemZCycle *pItem)
{
    gAutosave = pItem->m_nFocus % ARRAY_SSIZE(pzAutosaveModeStrings);
}

void SetLockSaving(CGameMenuItemZBool *pItem)
{
    gLockManualSaving = pItem->at20;

    itemMainSave3.bEnable = !gLockManualSaving; // hide save option in main menu if lock saving mode is set
}

void SetGameVanillaMode(char nState)
{
    gVanilla = nState % ARRAY_SSIZE(pzVanillaModeStrings);
    itemOptionsDisplayWeaponSelect.bEnable = !gVanilla;
    itemOptionsGameWeaponFastSwitch.bEnable = !gVanilla;
    itemOptionsChainEnhancements.bEnable = !gVanilla;
}

void SetVanillaMode(CGameMenuItemZCycle *pItem)
{
    if ((gGameOptions.nGameType == 0) && (numplayers == 1))
    {
        SetGameVanillaMode((char)pItem->m_nFocus);
        viewClearInterpolations();
    }
    else
        pItem->m_nFocus = gVanilla % ARRAY_SSIZE(pzVanillaModeStrings);
    viewResizeView(gViewSize);
}

short gQuickLoadSlot = kLoadSaveNull;
short gQuickSaveSlot = kLoadSaveNull;

void ShowDifficulties()
{
    gGameMenuMgr.Push(&menuDifficulty, 3);
}

void SetDifficultyAndStart(CGameMenuItemChain *pItem)
{
    gGameOptions.nDifficulty = pItem->at30;
    gGameOptions.nEnemyQuantity = gGameOptions.nDifficulty;
    gGameOptions.nEnemyHealth = gGameOptions.nDifficulty;
    playerSetSkill(gGameOptions.nDifficulty); // set skill to same value as current difficulty
    gGameOptions.bPitchforkOnly = false;
    gGameOptions.uMonsterBannedType = BANNED_NONE;
    gGameOptions.nLevel = 0;
    gGameOptions.uGameFlags = 0;
    if (gDemo.bPlaying)
        gDemo.StopPlayback();
    else if (gDemo.bRecording)
        gDemo.Close();
    gStartNewGame = true;
    gAutosaveInCurLevel = false;
    gQuickLoadSlot = gQuickSaveSlot = kLoadSaveNull;
    gCheatMgr.ResetCheats();
    if (Bstrlen(gGameOptions.szUserMap))
    {
        levelAddUserMap(gGameOptions.szUserMap);
        levelSetupOptions(gGameOptions.nEpisode, gGameOptions.nLevel);
        StartLevel(&gGameOptions);
        viewResizeView(gViewSize);
    }
    gGameMenuMgr.Deactivate();
}

void SetCustomDifficultyAndStart(CGameMenuItemChain *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
    gGameOptions.nDifficulty = ClipRange(itemCustomDifficultyEnemyDifficulty.nValue, 0, 4);
    gGameOptions.nEnemyQuantity = ClipRange(itemCustomDifficultyEnemyQuantity.nValue, 0, 4);
    gGameOptions.nEnemyHealth = ClipRange(itemCustomDifficultyEnemyHealth.nValue, 0, 4);
    playerSetSkill(itemCustomDifficultyPlayerDamage.nValue);
    gGameOptions.bPitchforkOnly = !!itemCustomDifficultyPitchfork.at20;
    gGameOptions.uMonsterBannedType = BANNED_NONE;
    if (itemCustomDifficultyMonsterBanBats.at20)
        gGameOptions.uMonsterBannedType |= BANNED_BATS;
    if (itemCustomDifficultyMonsterBanRats.at20)
        gGameOptions.uMonsterBannedType |= BANNED_RATS;
    if (itemCustomDifficultyMonsterBanFish.at20)
        gGameOptions.uMonsterBannedType |= BANNED_FISH;
    if (itemCustomDifficultyMonsterBanHands.at20)
        gGameOptions.uMonsterBannedType |= BANNED_HANDS;
    if (itemCustomDifficultyMonsterBanGhosts.at20)
        gGameOptions.uMonsterBannedType |= BANNED_GHOSTS;
    if (itemCustomDifficultyMonsterBanSpiders.at20)
        gGameOptions.uMonsterBannedType |= BANNED_SPIDERS;
    if (itemCustomDifficultyMonsterBanTinyCaleb.at20)
        gGameOptions.uMonsterBannedType |= BANNED_TCALEBS;
    if (itemCustomDifficultyMonsterBanHellHounds.at20)
        gGameOptions.uMonsterBannedType |= BANNED_HHOUNDS;
    gGameOptions.nLevel = 0;
    gGameOptions.uGameFlags = 0;
    if (gDemo.bPlaying)
        gDemo.StopPlayback();
    else if (gDemo.bRecording)
        gDemo.Close();
    gStartNewGame = true;
    gAutosaveInCurLevel = false;
    gQuickLoadSlot = gQuickSaveSlot = kLoadSaveNull;
    gCheatMgr.ResetCheats();
    if (Bstrlen(gGameOptions.szUserMap))
    {
        levelAddUserMap(gGameOptions.szUserMap);
        levelSetupOptions(gGameOptions.nEpisode, gGameOptions.nLevel);
        StartLevel(&gGameOptions);
        viewResizeView(gViewSize);
    }
    gGameMenuMgr.Deactivate();
}

void SetVideoModeOld(CGameMenuItemChain *pItem)
{
    if (pItem->at30 == validmodecnt)
    {
        gSetup.fullscreen = 0;
        gSetup.xdim = 640;
        gSetup.ydim = 480;
    }
    else
    {
        gSetup.fullscreen = 0;
        gSetup.xdim = validmode[pItem->at30].xdim;
        gSetup.ydim = validmode[pItem->at30].ydim;
    }
    scrSetGameMode(gSetup.fullscreen, gSetup.xdim, gSetup.ydim, gSetup.bpp);
    scrSetDac();
    viewResizeView(gViewSize);
}

void SetVideoMode(CGameMenuItemChain *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
    resolution_t p = { xres, yres, fullscreen, bpp, 0 };
    int32_t prend = videoGetRenderMode();
    int32_t pvsync = vsync;

    int32_t nResolution = itemOptionsDisplayModeResolution.m_nFocus;
    resolution_t n = { gResolution[nResolution].xdim, gResolution[nResolution].ydim,
                       (gResolution[nResolution].flags & RES_FS) ? itemOptionsDisplayModeFullscreen.at20 : 0,
                       (nRendererValues[itemOptionsDisplayModeRenderer.m_nFocus] == REND_CLASSIC) ? 8 : gResolution[nResolution].bppmax, 0 };
    int32_t UNUSED(nrend) = nRendererValues[itemOptionsDisplayModeRenderer.m_nFocus];
    int32_t nvsync = nVSyncValues[itemOptionsDisplayModeVSync.m_nFocus];

    if (videoSetGameMode(n.flags, n.xdim, n.ydim, n.bppmax, upscalefactor) < 0)
    {
        if (videoSetGameMode(p.flags, p.xdim, p.ydim, p.bppmax, upscalefactor) < 0)
        {
            videoSetRenderMode(prend);
            ThrowError("Failed restoring old video mode.");
        }
        else
        {
            onvideomodechange(p.bppmax > 8);
            vsync = videoSetVsync(pvsync);
        }
    }
    else onvideomodechange(n.bppmax > 8);

    viewResizeView(gViewSize);
    vsync = videoSetVsync(nvsync);
    gSetup.fullscreen = fullscreen;
    gSetup.xdim = xres;
    gSetup.ydim = yres;
    gSetup.bpp = bpp;
}

void SetWidescreen(CGameMenuItemZBool *pItem)
{
    r_usenewaspect = pItem->at20;
}

void SetHudRatio(CGameMenuItemZCycle *pItem)
{
    gHudRatio = pItem->m_nFocus % ARRAY_SSIZE(pzHudRatioStrings);
    viewResizeView(gViewSize);
}

void SetMirrorMode(CGameMenuItemZCycle *pItem)
{
    r_mirrormode = pItem->m_nFocus % ARRAY_SSIZE(pzMirrorModeStrings);
}

void SetWeaponSelectMode(CGameMenuItemZCycle *pItem)
{
    gShowWeaponSelect = pItem->m_nFocus % ARRAY_SSIZE(pzRandomizerModeStrings);
}

void SetSlowRoomFlicker(CGameMenuItemZBool *pItem)
{
    gSlowRoomFlicker = pItem->at20;
}

void SetFOV(CGameMenuItemSlider *pItem)
{
    gFov = pItem->nValue;
}

void SetupVideoModeMenu(CGameMenuItemChain *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
    for (int i = 0; i < gResolutionNum; i++)
    {
        if (gSetup.xdim == gResolution[i].xdim && gSetup.ydim == gResolution[i].ydim)
        {
            itemOptionsDisplayModeResolution.m_nFocus = i;
            break;
        }
    }
    itemOptionsDisplayModeFullscreen.at20 = gSetup.fullscreen;
#ifdef USE_OPENGL
    for (int i = 0; i < 2; i++)
    {
        if (videoGetRenderMode() == nRendererValues[i])
        {
            itemOptionsDisplayModeRenderer.m_nFocus = i;
            break;
        }
    }
#endif
    for (int i = 0; i < 3; i++)
    {
        if (vsync == nVSyncValues[i])
        {
            itemOptionsDisplayModeVSync.m_nFocus = i;
            break;
        }
    }
    for (int i = 0; i < 8; i++)
    {
        if (r_maxfps == nFrameLimitValues[i])
        {
            itemOptionsDisplayModeFrameLimit.m_nFocus = i;
            break;
        }
    }
    // itemOptionsDisplayModeFPSOffset.nValue = r_maxfpsoffset;
}

void PreDrawVideoModeMenu(CGameMenuItem *pItem)
{
    if (pItem == &itemOptionsDisplayModeFullscreen)
        pItem->bEnable = !!(gResolution[itemOptionsDisplayModeResolution.m_nFocus].flags & RES_FS);
#ifdef USE_OPENGL
    else if (pItem == &itemOptionsDisplayModeRenderer)
        pItem->bEnable = gResolution[itemOptionsDisplayModeResolution.m_nFocus].bppmax > 8;
#endif
}

void UpdateVideoModeMenuFrameLimit(CGameMenuItemZCycle *pItem)
{
    r_maxfps = nFrameLimitValues[pItem->m_nFocus];
    g_frameDelay = calcFrameDelay(r_maxfps);
}

//void UpdateVideoModeMenuFPSOffset(CGameMenuItemSlider *pItem)
//{
//    r_maxfpsoffset = pItem->nValue;
//    g_frameDelay = calcFrameDelay(r_maxfps);
//}

void UpdateVideoColorMenu(CGameMenuItemSliderFloat *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
    g_videoGamma = itemOptionsDisplayColorGamma.fValue;
    g_videoContrast = itemOptionsDisplayColorContrast.fValue;
    g_videoBrightness = itemOptionsDisplayColorBrightness.fValue;
    r_ambientlight = itemOptionsDisplayColorVisibility.fValue;
    r_ambientlightrecip = 1.f/r_ambientlight;
    gBrightness = GAMMA_CALC<<2;
    videoSetPalette(gBrightness>>2, gLastPal, 0);
}

void UpdateVideoPalette(void)
{
    scrCustomizePalette(gCustomPalette % ARRAY_SSIZE(srcCustomPaletteStr), gCustomPaletteCIEDE2000, gCustomPaletteGrayscale, gCustomPaletteInvert);
    videoSetPalette(gBrightness>>2, gLastPal, 0);
#ifdef USE_OPENGL
    if (!r_useindexedcolortextures && (videoGetRenderMode() != REND_CLASSIC))
    {
        videoResetMode();
        if (videoSetGameMode(fullscreen, xres, yres, bpp, upscalefactor))
            ThrowError("restartvid: Reset failed...\n");
        onvideomodechange(gSetup.bpp>8);
    }
#endif
}

void UpdateVideoPaletteCycleMenu(CGameMenuItemZCycle *pItem)
{
    gCustomPalette = pItem->m_nFocus % ARRAY_SSIZE(srcCustomPaletteStr);
    UpdateVideoPalette();
}

void UpdateVideoPaletteBoolMenu(CGameMenuItemZBool *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
    gCustomPaletteCIEDE2000 = itemOptionsDisplayColorPaletteCIEDE2000.at20;
    gCustomPaletteGrayscale = itemOptionsDisplayColorPaletteGrayscale.at20;
    gCustomPaletteInvert = itemOptionsDisplayColorPaletteInvert.at20;
    UpdateVideoPalette();
}

void PreDrawDisplayColor(CGameMenuItem *pItem)
{
    if (pItem == &itemOptionsDisplayColorContrast)
        pItem->bEnable = gammabrightness;
    else if (pItem == &itemOptionsDisplayColorBrightness)
        pItem->bEnable = gammabrightness;
}

void ResetVideoColor(CGameMenuItemChain *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
    g_videoGamma = DEFAULT_GAMMA;
    g_videoContrast = DEFAULT_CONTRAST;
    g_videoBrightness = DEFAULT_BRIGHTNESS;
    gBrightness = 0;
    r_ambientlight = r_ambientlightrecip = 1.f;
    gCustomPalette = itemOptionsDisplayColorPaletteCustom.m_nFocus = 0;
    gCustomPaletteCIEDE2000 = itemOptionsDisplayColorPaletteCIEDE2000.at20 = 0;
    gCustomPaletteGrayscale = itemOptionsDisplayColorPaletteGrayscale.at20 = 0;
    gCustomPaletteInvert = itemOptionsDisplayColorPaletteInvert.at20 = 0;
    scrCustomizePalette(gCustomPalette % ARRAY_SSIZE(srcCustomPaletteStr), gCustomPaletteCIEDE2000, gCustomPaletteGrayscale, gCustomPaletteInvert);
    videoSetPalette(gBrightness>>2, gLastPal, 0);
#ifdef USE_OPENGL
    if (!r_useindexedcolortextures && (videoGetRenderMode() != REND_CLASSIC))
    {
        videoResetMode();
        if (videoSetGameMode(fullscreen, xres, yres, bpp, upscalefactor))
            ThrowError("restartvid: Reset failed...\n");
        onvideomodechange(gSetup.bpp>8);
    }
#endif
}

#ifdef USE_OPENGL
void SetupVideoPolymostMenu(CGameMenuItemChain *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
    itemOptionsDisplayPolymostTextureMode.m_nFocus = 0;
    for (int i = 0; i < 2; i++)
    {
        if (nTextureModeValues[i] == gltexfiltermode)
        {
            itemOptionsDisplayPolymostTextureMode.m_nFocus = i;
            break;
        }
    }
    itemOptionsDisplayPolymostAnisotropy.m_nFocus = 0;
    for (int i = 0; i < 6; i++)
    {
        if (nAnisotropyValues[i] == glanisotropy)
        {
            itemOptionsDisplayPolymostAnisotropy.m_nFocus = i;
            break;
        }
    }
    itemOptionsDisplayPolymostTrueColorTextures.at20 = usehightile;
    itemOptionsDisplayPolymostTexQuality.m_nFocus = r_downsize;
    itemOptionsDisplayPolymostPreloadCache.at20 = useprecache;
    itemOptionsDisplayPolymostTexCache.m_nFocus = glusetexcache;
    itemOptionsDisplayPolymostDetailTex.at20 = r_detailmapping;
    itemOptionsDisplayPolymostGlowTex.at20 = r_glowmapping;
    itemOptionsDisplayPolymost3DModels.at20 = usemodels;
    itemOptionsDisplayPolymostDeliriumBlur.at20 = gDeliriumBlur;
    itemOptionsDisplayPolymostUseColorIndexedTex.at20 = r_useindexedcolortextures;
}

void UpdateTextureMode(CGameMenuItemZCycle *pItem)
{
    gltexfiltermode = nTextureModeValues[pItem->m_nFocus];
    gltexapplyprops();
}

void UpdateAnisotropy(CGameMenuItemZCycle *pItem)
{
    glanisotropy = nAnisotropyValues[pItem->m_nFocus];
    gltexapplyprops();
}

void UpdateTrueColorTextures(CGameMenuItemZBool *pItem)
{
    usehightile = pItem->at20;
}
#endif

void DoModeChange(void)
{
    videoResetMode();
    if (videoSetGameMode(fullscreen, xres, yres, bpp, upscalefactor))
        OSD_Printf("restartvid: Reset failed...\n");
    onvideomodechange(gSetup.bpp > 8);
}

#ifdef USE_OPENGL
void UpdateTexQuality(CGameMenuItemZCycle *pItem)
{
    r_downsize = pItem->m_nFocus;
    texcache_invalidate();
    r_downsizevar = r_downsize;
    DoModeChange();
}

void UpdatePreloadCache(CGameMenuItemZBool *pItem)
{
    useprecache = pItem->at20;
}

void UpdateTexCache(CGameMenuItemZCycle *pItem)
{
    glusetexcache = pItem->m_nFocus;
}

void UpdateDetailTex(CGameMenuItemZBool *pItem)
{
    r_detailmapping = pItem->at20;
}

void UpdateGlowTex(CGameMenuItemZBool *pItem)
{
    r_glowmapping = pItem->at20;
}

void Update3DModels(CGameMenuItemZBool *pItem)
{
    usemodels = pItem->at20;
}

void UpdateDeliriumBlur(CGameMenuItemZBool *pItem)
{
    gDeliriumBlur = pItem->at20;
}

void UpdateTexColorIndex(CGameMenuItemZBool *pItem)
{
    r_useindexedcolortextures = pItem->at20;
}

void PreDrawDisplayPolymost(CGameMenuItem *pItem)
{
    if (pItem == &itemOptionsDisplayPolymostTexQuality)
        pItem->bEnable = usehightile;
    else if (pItem == &itemOptionsDisplayPolymostPreloadCache)
        pItem->bEnable = usehightile;
    else if (pItem == &itemOptionsDisplayPolymostTexCache)
        pItem->bEnable = glusetexcompr && usehightile;
    else if (pItem == &itemOptionsDisplayPolymostDetailTex)
        pItem->bEnable = usehightile;
    else if (pItem == &itemOptionsDisplayPolymostGlowTex)
        pItem->bEnable = usehightile;
}
#endif

void UpdateSoundToggle(CGameMenuItemZBool *pItem)
{
    SoundToggle = pItem->at20;
    if (!SoundToggle)
        FX_StopAllSounds();
}

void UpdateMusicToggle(CGameMenuItemZBool *pItem)
{
    MusicToggle = pItem->at20;
    if (!MusicToggle)
        sndStopSong();
    else
    {
        if (gGameStarted || gDemo.bPlaying)
            sndPlaySong(gGameOptions.zLevelSong, true);
    }
}

void UpdateCDToggle(CGameMenuItemZBool *pItem)
{
    CDAudioToggle = pItem->at20;
    if (gGameStarted || gDemo.bPlaying)
        levelTryPlayMusicOrNothing(gGameOptions.nEpisode, gGameOptions.nLevel);
}

void SetDoppler(CGameMenuItemZBool *pItem)
{
    DopplerToggle = pItem->at20;
}

void UpdateSoundVolume(CGameMenuItemSlider *pItem)
{
    sndSetFXVolume(pItem->nValue);
}

void UpdateMusicVolume(CGameMenuItemSlider *pItem)
{
    sndSetMusicVolume(pItem->nValue);
}

void UpdateSoundRate(CGameMenuItemZCycle *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
}

void UpdateNumVoices(CGameMenuItemSlider *pItem)
{
    NumVoices = ClipRange(pItem->nValue, 16, 255);
}

void UpdateSpeakerAngle(CGameMenuItemSlider *pItem)
{
    gSoundEarAng = ClipRange(pItem->nValue, 15, 90);
}

void UpdateCalebTalk(CGameMenuItemZCycle *pItem)
{
    gCalebTalk = pItem->m_nFocus % ARRAY_SIZE(pzCalebTalkStrings);
}

void UpdateMusicDevice(CGameMenuItemZCycle *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
    itemOptionsSoundSF2Bank.bEnable = 0;
    itemOptionsSoundSF2Bank.bNoDraw = 1;
    switch (nMusicDeviceValues[itemOptionsSoundMusicDevice.m_nFocus])
    {
    case ASS_SF2:
        itemOptionsSoundSF2Bank.bEnable = 1;
        itemOptionsSoundSF2Bank.bNoDraw = 0;
        break;
    }
}

void SetSound(CGameMenuItemChain *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
    MixRate = nSoundRateValues[itemOptionsSoundSampleRate.m_nFocus];
    NumVoices = itemOptionsSoundNumVoices.nValue;
    MusicDevice = nMusicDeviceValues[itemOptionsSoundMusicDevice.m_nFocus];
    Bstrcpy(SF2_BankFile, sf2bankfile);
    sfxTerm();
    sndTerm();

    sndInit();
    sfxInit();

    if (MusicToggle && (gGameStarted || gDemo.bPlaying))
        sndPlaySong(gGameOptions.zLevelSong, true);
}

void PreDrawSound(CGameMenuItem *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
}

void SetupOptionsSound(CGameMenuItemChain *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
    itemOptionsSoundSoundToggle.at20 = SoundToggle;
    itemOptionsSoundMusicToggle.at20 = MusicToggle;
    itemOptionsSoundMonoStereo.at20 = gStereo;
    itemOptionsSoundCDToggle.at20 = CDAudioToggle;
    itemOptionsSoundDoppler.at20 = DopplerToggle;
    itemOptionsSoundDoppler.bEnable = gStereo;
    itemOptionsSoundSampleRate.m_nFocus = 0;
    for (int i = 0; i < 3; i++)
    {
        if (nSoundRateValues[i] == MixRate)
        {
            itemOptionsSoundSampleRate.m_nFocus = i;
            break;
        }
    }
    itemOptionsSoundNumVoices.nValue = NumVoices;
    itemOptionsSoundSpeakerAngle.nValue = gSoundEarAng;
    itemOptionsSoundCalebTalk.m_nFocus = gCalebTalk % ARRAY_SIZE(pzCalebTalkStrings);
    itemOptionsSoundMusicDevice.m_nFocus = 0;
    for (int i = 0; i < (int)ARRAY_SIZE(nMusicDeviceValues); i++)
    {
        if (nMusicDeviceValues[i] == MusicDevice)
        {
            itemOptionsSoundMusicDevice.m_nFocus = i;
            break;
        }
    }

    UpdateMusicDevice(NULL);
}

void UpdatePlayerName(CGameMenuItemZEdit *pItem, CGameMenuEvent *pEvent)
{
    UNREFERENCED_PARAMETER(pItem);
    if (pEvent->at0 == kMenuEventEnter)
        netBroadcastPlayerInfo(myconnectindex);
}

void UpdatePlayerSkill(CGameMenuItemZCycle *pItem)
{
    gSkill = 4 - (pItem->m_nFocus % ARRAY_SIZE(pzPlayerSkillStrings)); // invert because string order is reversed (lower skill == easier)
    if ((numplayers > 1) || (gGameOptions.nGameType > 0)) // if multiplayer session is active
        netBroadcastPlayerInfoUpdate(myconnectindex);
}

void UpdatePlayerChatMessageSound(CGameMenuItemZBool *pItem)
{
    gChatSnd = pItem->at20;
}

void UpdatePlayerKillMessage(CGameMenuItemZBool *pItem)
{
    gKillMsg = pItem->at20;
}

void UpdatePlayerMultiKill(CGameMenuItemZCycle *pItem)
{
    gMultiKill = pItem->m_nFocus % ARRAY_SIZE(pzPlayerMultiKillStrings);
}

void SetMouseAimMode(CGameMenuItemZBool *pItem)
{
    gMouseAiming = pItem->at20;
}

void SetMouseVerticalAim(CGameMenuItemZBool *pItem)
{
    gMouseAim = pItem->at20;
}

void SetMouseXSensitivity(CGameMenuItemSliderFloat *pItem)
{
    CONTROL_MouseAxesSensitivity[0] = pItem->fValue;
}

void SetMouseYSensitivity(CGameMenuItemSliderFloat*pItem)
{
    CONTROL_MouseAxesSensitivity[1] = pItem->fValue;
}

void SetupMouseMenu(CGameMenuItemChain *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
    itemOptionsControlMouseAimFlipped.at20 = gMouseAimingFlipped;
    itemOptionsControlMouseAimMode.at20 = gMouseAiming;
    itemOptionsControlMouseVerticalAim.at20 = gMouseAim;
    // itemOptionsControlMouseXScale.nValue = CONTROL_MouseAxesScale[0];
    // itemOptionsControlMouseYScale.nValue = CONTROL_MouseAxesScale[1];
}

void PreDrawControlMouse(CGameMenuItem *pItem)
{
    if (pItem == &itemOptionsControlMouseVerticalAim)
        pItem->bEnable = !gMouseAiming;
}

void SetMouseButton(CGameMenuItemZCycle *pItem)
{
    for (int i = 0; i < MENUMOUSEFUNCTIONS; i++)
    {
        if (pItem == pItemOptionsControlMouseButton[i])
        {
            int nFunc = nGamefuncsValues[pItem->m_nFocus];
            MouseFunctions[MenuMouseDataIndex[i][0]][MenuMouseDataIndex[i][1]] = nFunc;
            CONTROL_MapButton(nFunc, MenuMouseDataIndex[i][0], MenuMouseDataIndex[i][1], controldevice_mouse);
            CONTROL_FreeMouseBind(MenuMouseDataIndex[i][0]);
            break;
        }
    }
}

void SetupMouseButtonMenu(CGameMenuItemChain *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
    for (int i = 0; i < MENUMOUSEFUNCTIONS; i++)
    {
        auto pItem = pItemOptionsControlMouseButton[i];
        pItem->m_nFocus = 0;
        for (int j = 0; j < NUMGAMEFUNCTIONS+1; j++)
        {
            if (MouseFunctions[MenuMouseDataIndex[i][0]][MenuMouseDataIndex[i][1]] == nGamefuncsValues[j])
            {
                pItem->m_nFocus = j;
                break;
            }
        }
    }
}

void SetupNetworkMenu(void)
{
    if (strlen(gNetAddress) > 0)
        strncpy(zNetAddressBuffer, gNetAddress, sizeof(zNetAddressBuffer)-1);

    menuNetwork.Add(&itemNetworkTitle, false);
    menuNetwork.Add(&itemNetworkHost, true);
    menuNetwork.Add(&itemNetworkJoin, false);
    menuNetwork.Add(&itemBloodQAV, false);

    menuNetworkHost.Add(&itemNetworkHostTitle, false);
    menuNetworkHost.Add(&itemNetworkHostPlayerNum, true);
    menuNetworkHost.Add(&itemNetworkHostPort, false);
    menuNetworkHost.Add(&itemNetworkHostHost, false);
    menuNetworkHost.Add(&itemBloodQAV, false);

    menuNetworkJoin.Add(&itemNetworkJoinTitle, false);
    menuNetworkJoin.Add(&itemNetworkJoinAddress, true);
    menuNetworkJoin.Add(&itemNetworkJoinPort, false);
    menuNetworkJoin.Add(&itemNetworkJoinJoin, false);
    menuNetworkJoin.Add(&itemBloodQAV, false);
}

void SetupNetworkHostMenu(CGameMenuItemChain *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
}

void SetupNetworkJoinMenu(CGameMenuItemChain *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
}

void NetworkHostGame(CGameMenuItemChain *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
    sndStopSong();
    FX_StopAllSounds();
    UpdateDacs(0, true);
    gNetPlayers = itemNetworkHostPlayerNum.nValue;
    gNetPort = strtoul(zNetPortBuffer, NULL, 10);
    if (!gNetPort)
        gNetPort = kNetDefaultPort;
    gNetMode = NETWORK_SERVER;
    netInitialize(false);
    gGameMenuMgr.Deactivate();
    gQuitGame = gRestartGame = true;
}

void NetworkJoinGame(CGameMenuItemChain *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
    sndStopSong();
    FX_StopAllSounds();
    UpdateDacs(0, true);
    strcpy(gNetAddress, zNetAddressBuffer);
    gNetPort = strtoul(zNetPortBuffer, NULL, 10);
    if (!gNetPort)
        gNetPort = kNetDefaultPort;
    gNetMode = NETWORK_CLIENT;
    netInitialize(false);
    gGameMenuMgr.Deactivate();
    gQuitGame = gRestartGame = true;
}

void SaveGameProcess(CGameMenuItemChain *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
}

void TenProcess(CGameMenuItem7EA1C *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
}

inline void UpdateSaveGameItemText(int nSlot)
{
    switch (nSlot) // set save slot text flag
    {
    case kLoadSaveSlot0:
        itemSaveGame0.at37 = 0;
        break;
    case kLoadSaveSlot1:
        itemSaveGame1.at37 = 0;
        break;
    case kLoadSaveSlot2:
        itemSaveGame2.at37 = 0;
        break;
    case kLoadSaveSlot3:
        itemSaveGame3.at37 = 0;
        break;
    case kLoadSaveSlot4:
        itemSaveGame4.at37 = 0;
        break;
    case kLoadSaveSlot5:
        itemSaveGame5.at37 = 0;
        break;
    case kLoadSaveSlot6:
        itemSaveGame6.at37 = 0;
        break;
    case kLoadSaveSlot7:
        itemSaveGame7.at37 = 0;
        break;
    case kLoadSaveSlot8:
        itemSaveGame8.at37 = 0;
        break;
    case kLoadSaveSlot9:
        itemSaveGame9.at37 = 0;
        break;
    default:
        break;
    }
}

void SaveGame(CGameMenuItemZEditBitmap *pItem, CGameMenuEvent *event)
{
    char strSaveGameName[BMAX_PATH];
    int nSlot = pItem->at28;
    if (gGameOptions.nGameType > 0 || !gGameStarted)
        return;
    if (event->at0 != 6/* || strSaveGameName[0]*/)
    {
        gGameMenuMgr.Deactivate();
        return;
    }
    G_ModDirSnprintf(strSaveGameName, BMAX_PATH, "game00%02d.sav", nSlot);
    memset(gGameOptions.szUserGameName, 0, sizeof(gGameOptions.szSaveGameName));
    strcpy(gGameOptions.szUserGameName, strRestoreGameStrings[nSlot]);
    memset(gGameOptions.szSaveGameName, 0, sizeof(gGameOptions.szSaveGameName));
    sprintf(gGameOptions.szSaveGameName, "%s", strSaveGameName);
    gGameOptions.nSaveGameSlot = nSlot;
    viewLoadingScreen(gMenuPicnum, "Saving", "Saving Your Game", strRestoreGameStrings[nSlot]);
    videoNextPage();
    gSaveGameNum = nSlot;
    LoadSave::SaveGame(strSaveGameName);
    gGameOptions.picEntry = gSavedOffset;
    gSaveGameOptions[nSlot] = gGameOptions;
    LoadUpdateSaveGame(nSlot, gProfile[myconnectindex].skill);
    UpdateSaveGameItemText(nSlot);
    gQuickSaveSlot = gQuickLoadSlot = nSlot;
    gGameMenuMgr.Deactivate();
    viewSetMessage("Game saved");
}

void QuickSaveGame(void)
{
    char strSaveGameName[BMAX_PATH];
    if (gGameOptions.nGameType > 0 || !gGameStarted)
        return;
    /*if (strSaveGameName[0])
    {
        gGameMenuMgr.Deactivate();
        return;
    }*/
    G_ModDirSnprintf(strSaveGameName, BMAX_PATH, "game00%02d.sav", gQuickSaveSlot);
    memset(gGameOptions.szUserGameName, 0, sizeof(gGameOptions.szSaveGameName));
    strcpy(gGameOptions.szUserGameName, strRestoreGameStrings[gQuickSaveSlot]);
    memset(gGameOptions.szSaveGameName, 0, sizeof(gGameOptions.szSaveGameName));
    sprintf(gGameOptions.szSaveGameName, "%s", strSaveGameName);
    gGameOptions.nSaveGameSlot = gQuickSaveSlot;
    viewLoadingScreen(gMenuPicnum, "Saving", "Saving Your Game", strRestoreGameStrings[gQuickSaveSlot]);
    videoNextPage();
    LoadSave::SaveGame(strSaveGameName);
    gGameOptions.picEntry = gSavedOffset;
    gSaveGameOptions[gQuickSaveSlot] = gGameOptions;
    LoadUpdateSaveGame(gQuickSaveSlot, gProfile[myconnectindex].skill);
    UpdateSaveGameItemText(gQuickSaveSlot);
    gQuickLoadSlot = gQuickSaveSlot;
    gGameMenuMgr.Deactivate();
    viewSetMessage("Game saved");
}

void AutosaveGame(bool bLevelStartSave)
{
    char strSaveGameName[BMAX_PATH];
    int nSlot = bLevelStartSave ? kLoadSaveSlotSpawn : kLoadSaveSlotKey;
    if (gGameOptions.nGameType > 0 || !gGameStarted)
        return;
    G_ModDirSnprintf(strSaveGameName, BMAX_PATH, "gameautosave%1d.sav", nSlot - kLoadSaveSlotSpawn);
    snprintf(gGameOptions.szUserGameName, sizeof(gGameOptions.szUserGameName), "%s %s", gGameOptions.zLevelName, nSlot == kLoadSaveSlotSpawn ? "start": "key");
    snprintf(gGameOptions.szSaveGameName, sizeof(gGameOptions.szSaveGameName), "%s", strSaveGameName);
    gGameOptions.nSaveGameSlot = nSlot;
    const PLAYER playerTemp = *gMe; // temp player struct while we make autosaving a little more easier (blood is stressful enough already)
    if (!bLevelStartSave) // if key save, reset problematic weapon states
    {
        playerResetWeaponState(gMe, true);
        gMe->invulTime = (int)gFrameClock; // in case they get hitscanned right after loading
    }
    LoadSave::SaveGame(strSaveGameName);
    *gMe = playerTemp; // restore current player struct
    if (!bLevelStartSave) // don't print message on level start autosaves
        viewSetMessage("Autosaved...");
    gGameOptions.picEntry = gSavedOffset;
    gSaveGameOptions[nSlot] = gGameOptions;
    LoadUpdateSaveGame(nSlot, gProfile[myconnectindex].skill);
    gQuickLoadSlot = nSlot;
    gAutosaveInCurLevel = true;
}

void LoadGame(CGameMenuItemZEditBitmap *pItem, CGameMenuEvent *event)
{
    UNREFERENCED_PARAMETER(event);
    char strLoadGameName[BMAX_PATH];
    int nSlot = pItem->at28;
    if (gGameOptions.nGameType > 0)
        return;
    if (nSlot <= kLoadSaveSlot9)
        G_ModDirSnprintf(strLoadGameName, BMAX_PATH, "game00%02d.sav", nSlot);
    else
        G_ModDirSnprintf(strLoadGameName, BMAX_PATH, "gameautosave%1d.sav", nSlot == kLoadSaveSlotSpawn ? 0 : 1);
    if (!testkopen(strLoadGameName, 0))
        return;
    if (!gGameStarted || LoadSavedInCurrentSession(nSlot)) // if save slot is from a different session, set autosave state to false
        gAutosaveInCurLevel = false;
    viewLoadingScreen(gMenuPicnum, "Loading", "Loading Saved Game", strRestoreGameStrings[nSlot]);
    videoNextPage();
    LoadSave::LoadGame(strLoadGameName);
    gGameMenuMgr.Deactivate();
    gQuickLoadSlot = nSlot;
}

void QuickLoadGame(void)
{
    char strLoadGameName[BMAX_PATH];
    if (gGameOptions.nGameType > 0)
        return;
    if (gQuickLoadSlot < kLoadSaveSlotAutosave)
        G_ModDirSnprintf(strLoadGameName, BMAX_PATH, "game00%02d.sav", gQuickLoadSlot);
    else
        G_ModDirSnprintf(strLoadGameName, BMAX_PATH, "gameautosave%1d.sav", gQuickLoadSlot == kLoadSaveSlotSpawn ? 0 : 1);
    if (!testkopen(strLoadGameName, 0))
        return;
    if (!LoadSavedInCurrentSession(gQuickLoadSlot)) // if save slot is from a different session, set autosave state to false
        gAutosaveInCurLevel = false;
    viewLoadingScreen(gMenuPicnum, "Loading", "Loading Saved Game", strRestoreGameStrings[gQuickLoadSlot]);
    videoNextPage();
    LoadSave::LoadGame(strLoadGameName);
    gGameMenuMgr.Deactivate();
}

void SetupLevelMenuItem(int nEpisode)
{
    dassert(nEpisode >= 0 && nEpisode < gEpisodeCount);
    itemNetStart3.SetTextArray(zLevelNames[nEpisode], gEpisodeInfo[nEpisode].nLevels, 0);
}

void SetupNetLevels(CGameMenuItemZCycle *pItem)
{
    SetupLevelMenuItem(pItem->m_nFocus);
    NetClearUserMap(pItem);
}

void NetClearUserMap(CGameMenuItemZCycle *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
    memset(zUserMapName, 0, sizeof(zUserMapName));
}

void StartNetGame(CGameMenuItemChain *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
    gPacketStartGame.gameType = itemNetStart1.m_nFocus+1;
    if (gPacketStartGame.gameType == 0)
        gPacketStartGame.gameType = 2;
    gPacketStartGame.episodeId = itemNetStart2.m_nFocus;
    gPacketStartGame.levelId = itemNetStart3.m_nFocus;
    gPacketStartGame.difficulty = itemNetStart4.m_nFocus;
    gPacketStartGame.monsterSettings = itemNetStart5.m_nFocus;
    gPacketStartGame.weaponSettings = itemNetStart6.m_nFocus;
    gPacketStartGame.itemSettings = itemNetStart7.m_nFocus;
    gPacketStartGame.respawnSettings = 0;
    gPacketStartGame.bFriendlyFire = itemNetStart8.at20;
    gPacketStartGame.keySettings = itemNetStart9.m_nFocus;
    gPacketStartGame.nSpawnProtection = itemNetStart10.m_nFocus;
    gPacketStartGame.nSpawnWeapon = itemNetStart11.m_nFocus;
    ////
    SetGameVanillaMode(0); // turn off vanilla mode for multiplayer so menus don't get bugged
    gPacketStartGame.bQuadDamagePowerup = itemNetEnhancementBoolQuadDamagePowerup.at20;
    gPacketStartGame.bDamageInvul = itemNetEnhancementBoolDamageInvul.at20;
    gPacketStartGame.nExplosionBehavior = itemNetEnhancementExplosionBehavior.m_nFocus % ARRAY_SSIZE(pzExplosionBehaviorStrings);
    gPacketStartGame.nProjectileBehavior = itemNetEnhancementProjectileBehavior.m_nFocus % ARRAY_SSIZE(pzProjectileBehaviorStrings);
    gPacketStartGame.bEnemyBehavior = itemNetEnhancementEnemyBehavior.at20;
    gPacketStartGame.bEnemyRandomTNT = itemNetEnhancementBoolEnemyRandomTNT.at20;
    gPacketStartGame.nWeaponsVer = itemNetEnhancementWeaponsVer.m_nFocus % ARRAY_SSIZE(pzWeaponsVersionStrings);
    gPacketStartGame.bSectorBehavior = itemNetEnhancementSectorBehavior.at20;
    gPacketStartGame.bHitscanProjectiles = itemNetEnhancementBoolHitscanProjectiles.at20;
    gPacketStartGame.randomizerMode = itemNetEnhancementRandomizerMode.m_nFocus % ARRAY_SSIZE(pzRandomizerModeStrings);
    Bstrncpy(gPacketStartGame.szRandomizerSeed, szRandomizerSeedMenu, sizeof(gPacketStartGame.szRandomizerSeed));
    if (gPacketStartGame.szRandomizerSeed[0] == '\0') // if no seed entered, generate new one before sending packet
        sprintf(gPacketStartGame.szRandomizerSeed, "%08X", QRandom2(gGameMenuMgr.m_mousepos.x*gGameMenuMgr.m_mousepos.y));
    ////
    gPacketStartGame.unk = 0;
    Bstrncpy(gPacketStartGame.userMapName, zUserMapName, sizeof(zUserMapName));
    gPacketStartGame.userMap = gPacketStartGame.userMapName[0] != 0;

    netBroadcastNewGame();
    gStartNewGame = 1;
    gGameMenuMgr.Deactivate();
}

void Restart(CGameMenuItemChain *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
    if (gGameOptions.nGameType == 0 || numplayers == 1)
    {
        gQuitGame = true;
        gRestartGame = true;
    }
    else
        gQuitRequest = 2;
    gGameMenuMgr.Deactivate();
}

void Quit(CGameMenuItemChain *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
    if (gGameOptions.nGameType == 0 || numplayers == 1)
        gQuitGame = true;
    else
        gQuitRequest = 1;
    gGameMenuMgr.Deactivate();
}

void SetParentalLock(CGameMenuItemZBool *pItem)
{
    if (!pItem->at20)
    {
        pItem->at20 = true;
        pItem->Draw();
        if (strcmp(itemParentalLockPassword.at20, ""))
        {
            itemParentalLockPassword.pMenu->FocusNextItem();
            itemParentalLockPassword.at32 = 0;
            itemParentalLockPassword.at37 = 1;
            itemParentalLockPassword.at5f = pItem;
            itemParentalLockPassword.at29[0] = 0;
            return;
        }
        else
        {
            itemParentalLockPassword.at20[0] = 0;
            pItem->Draw();
            gbAdultContent = false;
        }
    }
    else
        gbAdultContent = true;
    // NUKE-TODO: CONFIG_WriteAdultMode();
}

void MenuSetupEpisodeInfo(void)
{
    memset(zEpisodeNames, 0, sizeof(zEpisodeNames));
    memset(zLevelNames, 0, sizeof(zLevelNames));
    for (int i = 0; i < 6; i++)
    {
        if (i < gEpisodeCount)
        {
            EPISODEINFO *pEpisode = &gEpisodeInfo[i];
            zEpisodeNames[i] = pEpisode->title;
            for (int j = 0; j < 16; j++)
            {
                if (j < pEpisode->nLevels)
                {
                    zLevelNames[i][j] = pEpisode->levelsInfo[j].Title;
                }
            }
        }
    }
}

void drawLoadingScreen(void)
{
    char buffer[80];
    if (gGameOptions.nGameType == 0)
    {
        if (gDemo.bPlaying)
            sprintf(buffer, "Loading Demo");
        else
            sprintf(buffer, "Loading Level");
    }
    else
        sprintf(buffer, "%s", zNetGameTypes[gGameOptions.nGameType-1]);
    viewLoadingScreen(kLoadScreen, buffer, levelGetTitle(), NULL);
}

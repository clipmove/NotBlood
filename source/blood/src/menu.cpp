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
#ifdef _WIN32 // required for MME device selection
#include "driver_winmm.h"
#include "midi.h"
#endif

void SaveGame(CGameMenuItemZEditBitmap *, CGameMenuEvent *);

void SaveGameProcess(CGameMenuItemChain *);
void ShowDifficulties();
void SetDifficultyAndStart(CGameMenuItemChain *);
void SetMonsters(CGameMenuItemZCycle *);
void SetCustomDifficultyAndStart(CGameMenuItemChain *);
void SetFirstLaunchOptions(CGameMenuItemChain *);
void SetMusicVol(CGameMenuItemSlider *);
void SetSoundVol(CGameMenuItemSlider *);
void SetCDVol(CGameMenuItemSlider *);
void SetMonoStereo(CGameMenuItemZBool *);
void SetCrosshair(CGameMenuItemZCycle *);

void SetQuadDamagePowerup(CGameMenuItemZBool*);
void SetDamageInvul(CGameMenuItemZCycle*);
void SetExplosionBehavior(CGameMenuItemZCycle*);
void SetProjectileBehavior(CGameMenuItemZCycle*);
void SetNapalmFalloff(CGameMenuItemZBool*);
void SetEnemyBehavior(CGameMenuItemZCycle*);
void SetEnemyRandomTNT(CGameMenuItemZBool*);
void SetWeaponsVer(CGameMenuItemZCycle*);
void SetSectorBehavior(CGameMenuItemZBool*);
void SetHitscanProjectiles(CGameMenuItemZCycle*);
void SetRandomizerMode(CGameMenuItemZCycle*);
void SetRandomizerSeed(CGameMenuItemZEdit *pItem, CGameMenuEvent *pEvent);

void SetHudSize(CGameMenuItemSlider *);
void SetHudRatio(CGameMenuItemZCycle *);
void SetCenterHoriz(CGameMenuItemZBool *);
void SetSlopeTilting(CGameMenuItemZBool *);
void SetViewBobbing(CGameMenuItemZBool *);
void SetViewSwaying(CGameMenuItemZBool *);
void SetWeaponSwaying(CGameMenuItemZCycle *);
void SetWeaponInterpolate(CGameMenuItemZCycle *);
void SetViewInterpolate(CGameMenuItemZBool *);
void SetLevelCompleteTime(CGameMenuItemZBool *);
void SetMirrorMode(CGameMenuItemZCycle *);
void SetSlowRoomFlicker(CGameMenuItemZBool *);
void SetMouseSensitivity(CGameMenuItemSliderFloat *);
void SetMouseAimFlipped(CGameMenuItemZBool *);
void SetTurnSpeed(CGameMenuItemSlider *);
void SetTurnAcceleration(CGameMenuItemZCycle *);
void SetCenterView(CGameMenuItemZBool *);
void SetJoystickTargetAimAssist(CGameMenuItemZBool *);
void SetJoystickRumble(CGameMenuItemZBool *);
void SetCrouchToggle(CGameMenuItemZBool *);
void ResetKeys(CGameMenuItemChain *);
void ResetKeysClassic(CGameMenuItemChain *);
void SetMessages(CGameMenuItemZBool *);
void LoadGame(CGameMenuItemZEditBitmap *, CGameMenuEvent *);
void SetupNetLevels(CGameMenuItemZCycle *);
void NetClearUserMap(CGameMenuItemZCycle *);
void StartNetGame(CGameMenuItemChain *);
void SetupLevelMenuItem(int);
void SetupVideoModeMenu(CGameMenuItemChain *);
void SetupViewMenu(CGameMenuItemChain *);
void SetVideoMode(CGameMenuItemChain *);
void SetWidescreen(CGameMenuItemZBool *);
void SetWeaponSelectMode(CGameMenuItemZCycle *);
void SetDetail(CGameMenuItemSlider *);
void SetVoxels(CGameMenuItemZBool *);
void SetFOV(CGameMenuItemSlider *);
void UpdateVideoModeMenuFrameLimit(CGameMenuItemZCycle *pItem);
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

const char *zKeepItemWeaponStrings[] =
{
    "LOST ON DEATH",
    "KEEP ON DEATH"
};

const char *zSpawnProtectStrings[] =
{
    "OFF",
    "1 SEC",
    "2 SECS",
    "3 SECS",
};

const char *zShowWeapon[] =
{
    "ALWAYS SHOW",
    "HIDE WHEN CLOAKED",
    "ALWAYS HIDE"
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
    "Random",
    "Smart Random",
    "Farthest From Enemies",
};

const char *zLengthStrings[] =
{
    "Unlimited",
    "Minutes",
    "Frags",
};

const char *zLengthStringsCoop[] =
{
    "Unlimited",
    "Lives",
};

const char *pzMirrorModeStrings[] = {
    "OFF",
    "HORIZONTAL",
    "VERTICAL",
    "HORIZONTAL+VERTICAL"
};

const char *zDiffStrings[] =
{
    "STILL KICKING",
    "PINK ON THE INSIDE",
    "LIGHTLY BROILED",
    "WELL DONE",
    "EXTRA CRISPY",
};

const char *pzDamageInvulBehaviorStrings[] = {
    "Off",
    "Shortest",
    "Short",
    "Medium",
    "Long",
    "Longest",
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

const char *pzEnemyBehaviorStrings[] = {
    "Original",
    "NBlood",
    "NotBlood",
};

const char *pzWeaponsVersionStrings[] = {
    "Original",
    "NotBlood",
    "V1.0x",
};

const char *pzHitscanProjectilesStrings[] = {
    "Off",
    "Slow",
    "Medium",
    "Fast",
};

const char *pzRandomizerModeStrings[] = {
    "Off",
    "Enemies",
    "Pickups",
    "Enemies+Pickups",
};

const char *pzEnemySpeeds[] = {
    "Default",
    "1.5x",
    "2.0x",
    "2.5x",
    "3.0x",
};

char zUserMapName[BMAX_PATH] = "";
const char *zEpisodeNames[6];
const char *zLevelNames[6][kMaxLevels];
static char szRandomizerSeedMenu[9];

static char MenuGameFuncs[NUMGAMEFUNCTIONS][MAXGAMEFUNCLEN];
static char const *MenuGameFuncNone = "  -None-";
static char const *pzGamefuncsStrings[NUMGAMEFUNCTIONS + 1];
static int nGamefuncsValues[NUMGAMEFUNCTIONS + 1];
static int nGamefuncsNum;

CGameMenu menuMain;
CGameMenu menuMainWithSave;
CGameMenu menuFirstLaunch;
CGameMenu menuNetMain;
CGameMenu menuNetStart;
CGameMenu menuEpisode;
CGameMenu menuDifficulty;
CGameMenu menuCustomDifficulty;
CGameMenu menuBannedMonsters;
CGameMenu menuBannedItems;
CGameMenu menuControls;
CGameMenu menuMessages;
CGameMenu menuSaveGame;
CGameMenu menuLoadGame;
CGameMenu menuLoading;
CGameMenu menuQuit;
CGameMenu menuRestart;
CGameMenu menuCredits;
CGameMenu menuHelp;
CGameMenu menuNetwork;
CGameMenu menuNetworkHost;
CGameMenu menuNetworkJoin;
CGameMenu menuNetworkGameMode;
CGameMenu menuNetworkGameMonsters;
CGameMenu menuNetworkGameMutators;

CGameMenuItemQAV itemBloodQAV("", 3, 160, 100, "BDRIP", true);
CGameMenuItemQAV itemCreditsQAV("", 3, 160, 100, "CREDITS", false, true);
CGameMenuItemQAV itemHelp3QAV("", 3, 160, 100, "HELP3", false, false);
CGameMenuItemQAV itemHelp3BQAV("", 3, 160, 100, "HELP3B", false, false);
CGameMenuItemQAV itemHelp4QAV("", 3, 160, 100, "HELP4", false, true);
CGameMenuItemQAV itemHelp5QAV("", 3, 160, 100, "HELP5", false, true);

CGameMenuItemTitle itemMainTitle("BLOOD", 1, 160, 20, 2038);
CGameMenuItemChain itemMain1("NEW GAME", 1, 0, 45, 320, 1, &menuEpisode, -1, NULL, 0);
CGameMenuItemChain itemMain2("MULTIPLAYER", 1, 0, 65, 320, 1, &menuNetwork, -1, NULL, 0);
CGameMenuItemChain itemMain3("OPTIONS", 1, 0, 85, 320, 1, &menuOptions, -1, NULL, 0);
CGameMenuItemChain itemMain4("LOAD GAME", 1, 0, 105, 320, 1, &menuLoadGame, -1, NULL, 0);
CGameMenuItemChain itemMain5("HELP", 1, 0, 125, 320, 1, &menuHelp, -1, NULL, 0);
CGameMenuItemChain itemMain6("CREDITS", 1, 0, 145, 320, 1, &menuCredits, -1, NULL, 0);
CGameMenuItemChain itemMain7("QUIT", 1, 0, 165, 320, 1, &menuQuit, -1, NULL, 0);

CGameMenuItemTitle itemMainSaveTitle("BLOOD", 1, 160, 20, 2038);
CGameMenuItemChain itemMainSave1("NEW GAME", 1, 0, 45, 320, 1, &menuEpisode, -1, NULL, 0);
CGameMenuItemChain itemMainSave2("OPTIONS", 1, 0, 60, 320, 1, &menuOptions, -1, NULL, 0);
CGameMenuItemChain itemMainSave3("SAVE GAME", 1, 0, 75, 320, 1, &menuSaveGame, -1, SaveGameProcess, 0);
CGameMenuItemChain itemMainSave4("LOAD GAME", 1, 0, 90, 320, 1, &menuLoadGame, -1, NULL, 0);
CGameMenuItemChain itemMainSave5("HELP", 1, 0, 105, 320, 1, &menuHelp, -1, NULL, 0);
CGameMenuItemChain itemMainSave6("CREDITS", 1, 0, 120, 320, 1, &menuCredits, -1, NULL, 0);
CGameMenuItemChain itemMainSave7("END GAME", 1, 0, 135, 320, 1, &menuRestart, -1, NULL, 0);
CGameMenuItemChain itemMainSave8("QUIT", 1, 0, 150, 320, 1, &menuQuit, -1, NULL, 0);

CGameMenuItemTitle itemFirstLaunchInfo1("THIS IS YOUR FIRST TIME LAUNCHING NOTBLOOD", 1, 160, 45, -1);
CGameMenuItemTitle itemFirstLaunchInfo2("PLEASE SELECT YOUR PREFERRED EXPERIENCE", 1, 160, 65, -1);
CGameMenuItemChain itemFirstLaunchVanilla("DOS BLOOD", 1, 0, 90, 320, 1, NULL, -1, SetFirstLaunchOptions, 0);
CGameMenuItemChain itemFirstLaunchNBlood("NBLOOD", 1, 0, 110, 320, 1, NULL, -1, SetFirstLaunchOptions, 0);
CGameMenuItemChain itemFirstLaunchNotBlood("NOTBLOOD", 1, 0, 130, 320, 1, NULL, -1, SetFirstLaunchOptions, 0);

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
CGameMenuItemChain itemDifficulty6("< CUSTOM >", 1, 0, 155, 320, 1, &menuCustomDifficulty, -1, NULL, 0, 8);

CGameMenuItemTitle itemCustomDifficultyTitle("CUSTOM", 1, 160, 20, 2038);
CGameMenuItemZCycle itemCustomDifficultyMonsterSettings("MONSTERS:", 3, 66, 40, 180, 0, SetMonsters, zMonsterStrings, ARRAY_SSIZE(zMonsterStrings), 1);
CGameMenuItemSlider itemCustomDifficultyEnemyQuantity("ENEMIES QUANTITY:", 3, 66, 50, 180, 2, 0, 4, 1, NULL, -1, -1);
CGameMenuItemSlider itemCustomDifficultyEnemyHealth("ENEMIES HEALTH:", 3, 66, 60, 180, 2, 0, 4, 1, NULL, -1, -1);
CGameMenuItemSlider itemCustomDifficultyEnemyDifficulty("ENEMIES DIFFICULTY:", 3, 66, 70, 180, 2, 0, 4, 1, NULL, -1, -1);
CGameMenuItemSlider itemCustomDifficultyPlayerDamage("PLAYER DAMAGE TAKEN:", 3, 66, 80, 180, 2, 0, 4, 1, NULL, -1, -1);
CGameMenuItemZCycle itemCustomDifficultyEnemySpeed("ENEMIES SPEED:", 3, 66, 90, 180, 0, 0, pzEnemySpeeds, ARRAY_SSIZE(pzEnemySpeeds), 0);
CGameMenuItemZBool itemCustomDifficultyEnemyShuffle("RANDOMIZE ENEMY POSITIONS:", 3, 66, 100, 180, false, NULL, NULL, NULL);
CGameMenuItemZBool itemCustomDifficultyPitchfork("PITCHFORK START:", 3, 66, 110, 180, false, NULL, NULL, NULL);
CGameMenuItemZBool itemCustomDifficultyPermaDeath("PERMANENT DEATH:", 3, 66, 120, 180, false, NULL, NULL, NULL);
CGameMenuItemChain itemCustomDifficultyBannedMonsters("SET MONSTERS", 3, 66, 132, 180, 1, &menuBannedMonsters, -1, NULL, 0);
CGameMenuItemChain itemCustomDifficultyBannedItems("SET ITEMS", 3, 66, 143, 180, 1, &menuBannedItems, -1, NULL, 0);
CGameMenuItemChain itemCustomDifficultyStart("START GAME", 1, 0, 156, 320, 1, NULL, -1, SetCustomDifficultyAndStart, 0);

CGameMenuItemTitle itemBannedMonstersTitle("SET MONSTERS", 1, 160, 20, 2038);
CGameMenuItemZBool itemBannedMonstersBats("BATS:", 3, 75, 40, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemBannedMonstersRats("RATS:", 3, 75, 48, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemBannedMonstersFish("FISH:", 3, 75, 56, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemBannedMonstersHands("HANDS:", 3, 75, 64, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemBannedMonstersGhosts("GHOSTS:", 3, 75, 72, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemBannedMonstersSpiders("SPIDERS:", 3, 75, 80, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemBannedMonstersTinyCaleb("TINY CALEBS:", 3, 75, 88, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemBannedMonstersHellHounds("HELL HOUNDS:", 3, 75, 96, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemBannedMonstersRatsAttack("RAT ATTACK DISTANCE:", 3, 75, 112, 161, false, NULL, "SHORTER", "DEFAULT");
CGameMenuItemZBool itemBannedMonstersMotherSpiderHealth("MOTHER SPIDER HEALTH:", 3, 75, 120, 161, false, NULL, "HALF", "DEFAULT");
CGameMenuItemZBool itemBannedMonstersTchernobogHealth("TCHERNOBOG HEALTH:", 3, 75, 128, 161, false, NULL, "FULL", "DEFAULT");

CGameMenuItemTitle itemBannedItemsTitle("SET ITEMS", 1, 160, 20, 2038);
CGameMenuItemZBool itemBannedItemsFlare("FLARE PISTOL:", 3, 75, 32, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemBannedItemsShotgun("SAWED-OFF:", 3, 75, 40, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemBannedItemsTommyGun("TOMMY GUN:", 3, 75, 48, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemBannedItemsNapalm("NAPALM LAUNCHER:", 3, 75, 56, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemBannedItemsTNT("DYNAMITE:", 3, 75, 64, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemBannedItemsSpray("SPRAY CAN:", 3, 75, 72, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemBannedItemsTesla("TESLA CANNON:", 3, 75, 80, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemBannedItemsLifeLeech("LIFE LEECH:", 3, 75, 88, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemBannedItemsVoodoo("VOODOO DOLL:", 3, 75, 96, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemBannedItemsProxy("PROXIMITY TNT:", 3, 75, 104, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemBannedItemsRemote("REMOTE TNT:", 3, 75, 112, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemBannedItemsMedKit("DOCTOR'S BAG:", 3, 75, 120, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemBannedItemsLifeEssence("LIFE ESSENCE/MED POUCH:", 3, 75, 128, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemBannedItemsLifeSeed("LIFE SEED:", 3, 75, 136, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemBannedItemsSuperArmor("SUPER ARMOR:", 3, 75, 144, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemBannedItemsCrystalBall("CRYSTAL BALL:", 3, 75, 152, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemBannedItemsJumpBoots("JUMP BOOTS:", 3, 75, 160, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemBannedItemsCloak("INVISIBILITY CLOAK:", 3, 75, 168, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemBannedItemsDeathMask("INVULNERABILITY MASK:", 3, 75, 176, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemBannedItemsAkimbo("GUNS AKIMBO:", 3, 75, 184, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemBannedItemsReflect("REFLECTIVE SHOTS:", 3, 75, 192, 161, false, NULL, "REMOVE", "KEEP");

CGameMenuItemTitle itemMessagesTitle("MESSAGES", 1, 160, 20, 2038);
CGameMenuItemZBool boolMessages("MESSAGES:", 3, 66, 70, 180, 0, SetMessages, NULL, NULL);
CGameMenuItemSlider sliderMsgCount("MESSAGE COUNT:", 3, 66, 80, 180, gMessageCount, 1, 16, 1, NULL, -1, -1);
CGameMenuItemSlider sliderMsgTime("MESSAGE TIME:", 3, 66, 90, 180, gMessageTime, 1, 8, 1, NULL, -1, -1);
CGameMenuItemZBool boolMsgFont("LARGE FONT:", 3, 66, 100, 180, 0, 0, NULL, NULL);
CGameMenuItemZBool boolMsgIncoming("INCOMING:", 3, 66, 110, 180, 0, 0, NULL, NULL);
CGameMenuItemZBool boolMsgSelf("SELF PICKUP:", 3, 66, 120, 180, 0, 0, NULL, NULL);
CGameMenuItemZBool boolMsgOther("OTHER PICKUP:", 3, 66, 130, 180, 0, 0, NULL, NULL);
CGameMenuItemZBool boolMsgRespawn("RESPAWN:", 3, 66, 140, 180, 0, 0, NULL, NULL);

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
CGameMenuItemZEditBitmap itemLoadGameQuick(NULL, 3, 20, 163, 320, strRestoreGameStrings[kLoadSaveSlotQuick], 16, 1, LoadGame, kLoadSaveSlotQuick);
CGameMenuItemZEditBitmap itemLoadGameAutosaveStart(NULL, 3, 20, 176, 320, strRestoreGameStrings[kLoadSaveSlotSpawn], 16, 1, LoadGame, kLoadSaveSlotSpawn);
CGameMenuItemZEditBitmap itemLoadGameAutosaveKey(NULL, 3, 20, 186, 320, strRestoreGameStrings[kLoadSaveSlotKey], 16, 1, LoadGame, kLoadSaveSlotKey);
CGameMenuItemBitmapLS itemLoadGamePic(NULL, 3, 0, 0, 2518);

CGameMenu menuMultiUserMaps;

CGameMenuItemTitle itemNetStartUserMapTitle("USER MAP", 1, 160, 20, 2038);
CGameMenuFileSelect menuMultiUserMap("", 3, 0, 0, 0, "./", "*.map", zUserMapName);

void SetNetGameMode(CGameMenuItemZCycle *pItem);
void SetNetMonsterMenu(CGameMenuItemZCycle *pItem);

CGameMenuItemTitle itemNetStartTitle("MULTIPLAYER", 1, 160, 20, 2038);
CGameMenuItemChain itemNetStart1("GAME:", 3, 66, 35, 180, 0, &menuNetworkGameMode, -1, NULL, 0);
CGameMenuItemZCycle itemNetStart2("EPISODE:", 3, 66, 45, 180, 0, SetupNetLevels, NULL, 0, 0);
CGameMenuItemZCycle itemNetStart3("LEVEL:", 3, 66, 55, 180, 0, NetClearUserMap, NULL, 0, 0);
CGameMenuItemZCycle itemNetStart4("DIFFICULTY:", 3, 66, 65, 180, 0, SetNetMonsterMenu, zDiffStrings, ARRAY_SSIZE(zDiffStrings), 0);
CGameMenuItemChain itemNetStart5("MONSTER SETTING:", 3, 66, 75, 180, 0, &menuNetworkGameMonsters, -1, NULL, 0);
CGameMenuItemZCycle itemNetStart6("WEAPONS:", 3, 66, 85, 180, 0, 0, zWeaponStrings, 4, 0);
CGameMenuItemZCycle itemNetStart7("ITEMS:", 3, 66, 95, 180, 0, 0, zItemStrings, 3, 0);
CGameMenuItemChain itemNetStart8("SET ITEMS", 3, 0, 132, 320, 1, &menuBannedItems, -1, NULL, 0);
CGameMenuItemChain itemNetStart9("SET MUTATORS", 3, 0, 145, 320, 1, &menuNetworkGameMutators, -1, NULL, 0);
CGameMenuItemChain itemNetStart10("USER MAP", 3, 0, 158, 320, 1, &menuMultiUserMaps, 0, NULL, 0);
CGameMenuItemChain itemNetStart11("START GAME", 1, 0, 175, 320, 1, 0, -1, StartNetGame, 0);

CGameMenuItemTitle itemNetGameTitle("GAME SETTINGS", 1, 160, 20, 2038);
CGameMenuItemZCycle itemNetGameMode("GAME:", 3, 66, 35, 180, 0, SetNetGameMode, zNetGameTypes, ARRAY_SSIZE(zNetGameTypes), 0);
CGameMenuItemZCycle itemNetGameCycleLength("LENGTH:", 3, 66, 43, 180, 0, SetNetGameMode, zLengthStrings, ARRAY_SSIZE(zLengthStrings), 0);
CGameMenuItemZCycle itemNetGameCycleLengthCoop("LENGTH:", 3, 66, 43, 180, 0, SetNetGameMode, zLengthStringsCoop, ARRAY_SSIZE(zLengthStringsCoop), 0);
CGameMenuItemSlider itemNetGameSliderLimit("LIMIT:", 3, 66, 51, 180, 1, 1, 255, 1, NULL, -1, -1, kMenuSliderValue);
CGameMenuItemZBool itemNetGameBoolExit("LEVEL EXIT:", 3, 66, 65, 180, true, NULL, NULL, NULL);
CGameMenuItemZBool itemNetGameBoolTeleFrag("TELEFRAGS:", 3, 66, 73, 180, true, NULL, NULL, NULL);
CGameMenuItemZBool itemNetGameBoolSkillOverride("PLAYER HANDICAP:", 3, 66, 81, 180, true, NULL, "ALLOWED", "OFF");
CGameMenuItemZBool itemNetGameBoolModelOverride("PLAYER MODEL:", 3, 66, 89, 180, true, NULL, "CULTISTS/CALEB", "CALEB ONLY");
CGameMenuItemZBool itemNetGameBoolFriendlyFire("FRIENDLY FIRE:", 3, 66, 97, 180, true, 0, NULL, NULL);
CGameMenuItemZCycle itemNetGameCycleKey("KEYS SETTING:", 3, 66, 105, 180, 0, 0, zKeyStrings, ARRAY_SSIZE(zKeyStrings), 0);
CGameMenuItemZCycle itemNetGameCycleItemWeapon("ITEM/WEAPON SETTING:", 3, 66, 113, 180, 0, SetNetGameMode, zKeepItemWeaponStrings, ARRAY_SSIZE(zKeepItemWeaponStrings), 0);
CGameMenuItemZBool itemNetGameBoolAutoTeams("AUTO TEAMS:", 3, 66, 105, 180, true, 0, NULL, NULL);
CGameMenuItemZBool itemNetGameBoolTeamColors("TEAM COLORS:", 3, 66, 113, 180, true, 0, NULL, NULL);
CGameMenuItemZBool itemNetGameBoolTeamFlags("TEAM FLAGS:", 3, 66, 121, 180, true, 0, NULL, NULL);
CGameMenuItemZCycle itemNetGameCycleSpawnLocation("SPAWN AREA:", 3, 66, 129, 180, 0, 0, zRespawnStrings, ARRAY_SSIZE(zRespawnStrings), 0);
CGameMenuItemZCycle itemNetGameCycleShowWeaponsOverride("ENEMY WEAPONS:", 3, 66, 137, 180, 0, 0, zShowWeapon, ARRAY_SSIZE(zShowWeapon), 0);
CGameMenuItemZCycle itemNetGameCycleSpawnProtection("SPAWN PROTECTION:", 3, 66, 145, 180, 0, 0, zSpawnProtectStrings, ARRAY_SSIZE(zSpawnProtectStrings), 0);
CGameMenuItemZCycle itemNetGameCycleSpawnWeapon("SPAWN WITH WEAPON:", 3, 66, 153, 180, 0, SetNetGameMode, zSpawnWeaponStrings, ARRAY_SSIZE(zSpawnWeaponStrings), 0);
CGameMenuItemZCycle itemNetGameCycleMirrorModeOverride("MIRROR MODE:", 3, 66, 161, 180, 0, NULL, pzMirrorModeStrings, ARRAY_SSIZE(pzMirrorModeStrings), 0);

CGameMenuItemTitle itemNetMonsterTitle("MONSTERS", 1, 160, 20, 2038);
CGameMenuItemZCycle itemNetMonsterSettings("MONSTERS:", 3, 66, 40, 180, 0, SetNetMonsterMenu, zMonsterStrings, ARRAY_SSIZE(zMonsterStrings), 0);
CGameMenuItemSlider itemNetMonsterQuantity("MONSTER QUANTITY:", 3, 66, 50, 180, 2, 0, 4, 1, NULL, -1, -1);
CGameMenuItemSlider itemNetMonsterHealth("MONSTER HEALTH:", 3, 66, 60, 180, 2, 0, 4, 1, NULL, -1, -1);
CGameMenuItemZCycle itemNetMonsterSpeed("MONSTER SPEED:", 3, 66, 70, 180, 0, 0, pzEnemySpeeds, ARRAY_SSIZE(pzEnemySpeeds), 0);
CGameMenuItemZBool itemNetMonsterBats("BATS:", 3, 75, 90, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemNetMonsterRats("RATS:", 3, 75, 98, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemNetMonsterFish("FISH:", 3, 75, 106, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemNetMonsterHands("HANDS:", 3, 75, 114, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemNetMonsterGhosts("GHOSTS:", 3, 75, 122, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemNetMonsterSpiders("SPIDERS:", 3, 75, 130, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemNetMonsterTinyCaleb("TINY CALEBS:", 3, 75, 138, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemNetMonsterHellHounds("HELL HOUNDS:", 3, 75, 146, 161, false, NULL, "REMOVE", "KEEP");
CGameMenuItemZBool itemNetMonsterRatsAttack("RAT ATTACK DISTANCE:", 3, 75, 162, 161, false, NULL, "SHORTER", "DEFAULT");
CGameMenuItemZBool itemNetMonsterMotherSpiderHealth("MOTHER SPIDER HEALTH:", 3, 75, 170, 161, false, NULL, "HALF", "DEFAULT");
CGameMenuItemZBool itemNetMonsterTchernobogHealth("TCHERNOBOG HEALTH:", 3, 75, 178, 161, false, NULL, "FULL", "DEFAULT");

///////////////
CGameMenuItemZBool itemNetMutatorBoolQuadDamagePowerup("REPLACE AKIMBO WITH 4X DAMAGE:", 3, 66, 45, 180, false, NULL, NULL, NULL);
CGameMenuItemZCycle itemNetMutatorDamageInvul("INVULNERABILITY DURATION:", 3, 66, 55, 180, 0, NULL, pzDamageInvulBehaviorStrings, ARRAY_SSIZE(pzDamageInvulBehaviorStrings), 0);
CGameMenuItemZCycle itemNetMutatorExplosionBehavior("EXPLOSIONS BEHAVIOR:", 3, 66, 65, 180, 0, NULL, pzExplosionBehaviorStrings, ARRAY_SSIZE(pzExplosionBehaviorStrings), 0);
CGameMenuItemZCycle itemNetMutatorProjectileBehavior("PROJECTILES BEHAVIOR:", 3, 66, 75, 180, 0, NULL, pzProjectileBehaviorStrings, ARRAY_SSIZE(pzProjectileBehaviorStrings), 0);
CGameMenuItemZBool itemNetMutatorNapalmFalloff("NAPALM GRAVITY FALLOFF:", 3, 66, 85, 180, false, NULL, NULL, NULL);
CGameMenuItemZCycle itemNetMutatorEnemyBehavior("ENEMY BEHAVIOR:", 3, 66, 95, 180, false, NULL, pzEnemyBehaviorStrings, ARRAY_SSIZE(pzEnemyBehaviorStrings), 0);
CGameMenuItemZBool itemNetMutatorBoolEnemyRandomTNT("RANDOM CULTIST TNT:", 3, 66, 105, 180, false, NULL, NULL, NULL);
CGameMenuItemZCycle itemNetMutatorWeaponsVer("WEAPON BEHAVIOR:", 3, 66, 115, 180, 0, NULL, pzWeaponsVersionStrings, ARRAY_SSIZE(pzWeaponsVersionStrings), 0);
CGameMenuItemZBool itemNetMutatorSectorBehavior("SECTOR BEHAVIOR:", 3, 66, 125, 180, false, NULL, "NOTBLOOD", "ORIGINAL");
CGameMenuItemZCycle itemNetMutatorHitscanProjectiles("HITSCAN PROJECTILES:", 3, 66, 135, 180, 0, NULL, pzHitscanProjectilesStrings, ARRAY_SSIZE(pzHitscanProjectilesStrings), 0);
CGameMenuItemZCycle itemNetMutatorRandomizerMode("RANDOMIZER MODE:", 3, 66, 145, 180, 0, NULL, pzRandomizerModeStrings, ARRAY_SSIZE(pzRandomizerModeStrings), 0);
CGameMenuItemZEdit itemNetMutatorRandomizerSeed("RANDOMIZER SEED:", 3, 66, 155, 180, szRandomizerSeedMenu, sizeof(szRandomizerSeedMenu), 0, SetRandomizerSeed, 0);
///////////////////

CGameMenuItemText itemLoadingText("LOADING...", 1, 160, 100, 1);

CGameMenuItemTitle itemQuitTitle("QUIT", 1, 160, 20, 2038);
CGameMenuItemText itemQuitText1("Do you really want to quit?", 0, 160, 100, 1);
CGameMenuItemYesNoQuit itemQuitYesNo("[Y/N]", 0, 20, 110, 280, 1, 0);

CGameMenuItemTitle itemRestartTitle("RESTART GAME", 1, 160, 20, 2038);
CGameMenuItemText itemRestartText1("Do you really want to restart game?", 0, 160, 100, 1);
CGameMenuItemYesNoQuit itemRestartYesNo("[Y/N]", 0, 20, 110, 280, 1, 1);

CGameMenuItemPicCycle itemCreditsPicCycle(0, 0, NULL, NULL, 0, 0);
CGameMenuItemPicCycle itemOrderPicCycle(0, 0, NULL, NULL, 0, 0);

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
CGameMenu menuOptionsGameMutators;
CGameMenu menuOptionsDisplay;
CGameMenu menuOptionsDisplayView;
CGameMenu menuOptionsDisplayColor;
CGameMenu menuOptionsDisplayMode;
#ifdef USE_OPENGL
CGameMenu menuOptionsDisplayPolymost;
#endif
CGameMenu menuOptionsSound;
CGameMenu menuOptionsPlayer;
CGameMenu menuOptionsControl;

void SetupOptionsSound(CGameMenuItemChain *pItem);
void SetupPollJoystick(CGameMenuItemChain *pItem);

CGameMenuItemTitle itemOptionsTitle("OPTIONS", 1, 160, 20, 2038);
CGameMenuItemChain itemOptionsChainGame("GAME SETUP", 1, 0, 50, 320, 1, &menuOptionsGame, -1, NULL, 0);
CGameMenuItemChain itemOptionsChainDisplay("DISPLAY SETUP", 1, 0, 70, 320, 1, &menuOptionsDisplay, -1, NULL, 0);
CGameMenuItemChain itemOptionsChainSound("SOUND SETUP", 1, 0, 90, 320, 1, &menuOptionsSound, -1, SetupOptionsSound, 0);
CGameMenuItemChain itemOptionsChainPlayer("PLAYER SETUP", 1, 0, 110, 320, 1, &menuOptionsPlayer, -1, NULL, 0);
CGameMenuItemChain itemOptionsChainControl("CONTROL SETUP", 1, 0, 130, 320, 1, &menuOptionsControl, -1, SetupPollJoystick, 0);
CGameMenuItemChain itemOptionsChainMutators("MUTATORS", 1, 0, 150, 320, 1, &menuOptionsGameMutators, -1, NULL, 0);

const char *pzAutoAimStrings[] = {
    "NEVER",
    "ALWAYS",
    "HITSCAN ONLY",
    "HITSCAN RATS/EELS ONLY",
    "PITCHFORK ONLY"
};

const char *pzWeaponSwitchStrings[] = {
    "NEVER",
    "BY RATING",
    "IF NEW",
    "IF HOLDING PITCHFORK"
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

const char *pzCrosshairStrings[] = {
    "OFF",
    "ON",
    "ON (AUTO AIM)"
};

const char *pzStatsPowerupRatioStrings[] = {
    "OFF",
    "ON",
    "ON (4:3)",
    "ON (16:10)",
    "ON (16:9)",
    "ON (21:9)",
};

const char *pzWeaponSelectStrings[] = {
    "OFF",
    "BOTTOM",
    "TOP"
};

const char *pzHudRatioStrings[] = {
    "NATIVE",
    "4:3",
    "16:10",
    "16:9",
    "21:9",
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

void SetAutoAim(CGameMenuItemZCycle *pItem);
void SetAutoRun(CGameMenuItemZBool *pItem);
void SetLevelStats(CGameMenuItemZCycle *pItem);
void SetLevelStatsOnAutomap(CGameMenuItemZBool *pItem);
void SetPowerupDuration(CGameMenuItemZCycle *pItem);
void SetPowerupStyle(CGameMenuItemZBool *pItem);
void SetShowMapTitle(CGameMenuItemZBool *pItem);
void SetWeaponSwitch(CGameMenuItemZCycle *pItem);
void SetWeaponFastSwitch(CGameMenuItemZBool *pItem);
void SetAutosaveMode(CGameMenuItemZCycle *pItem);
void SetLockSaving(CGameMenuItemZBool *pItem);
void SetRestoreLastSave(CGameMenuItemZBool *pItem);
void SetVanillaMode(CGameMenuItemZCycle *pItem);

///////////////
CGameMenuItemTitle itemGameMutatorsTitle("MUTATORS", 1, 160, 20, 2038);
CGameMenuItemZBool itemMutatorBoolQuadDamagePowerup("REPLACE AKIMBO WITH 4X DAMAGE:", 3, 66, 45, 180, false, SetQuadDamagePowerup, NULL, NULL);
CGameMenuItemZCycle itemMutatorDamageInvul("INVULNERABILITY DURATION:", 3, 66, 55, 180, 0, SetDamageInvul, pzDamageInvulBehaviorStrings, ARRAY_SSIZE(pzDamageInvulBehaviorStrings), 0);
CGameMenuItemZCycle itemMutatorExplosionBehavior("EXPLOSIONS BEHAVIOR:", 3, 66, 65, 180, 0, SetExplosionBehavior, pzExplosionBehaviorStrings, ARRAY_SSIZE(pzExplosionBehaviorStrings), 0);
CGameMenuItemZCycle itemMutatorProjectileBehavior("PROJECTILES BEHAVIOR:", 3, 66, 75, 180, 0, SetProjectileBehavior, pzProjectileBehaviorStrings, ARRAY_SSIZE(pzProjectileBehaviorStrings), 0);
CGameMenuItemZBool itemMutatorNapalmFalloff("NAPALM GRAVITY FALLOFF:", 3, 66, 85, 180, false, SetNapalmFalloff, NULL, NULL);
CGameMenuItemZCycle itemMutatorEnemyBehavior("ENEMY BEHAVIOR:", 3, 66, 95, 180, 0, SetEnemyBehavior, pzEnemyBehaviorStrings, ARRAY_SSIZE(pzEnemyBehaviorStrings), 0);
CGameMenuItemZBool itemMutatorBoolEnemyRandomTNT("RANDOM CULTIST TNT:", 3, 66, 105, 180, false, SetEnemyRandomTNT, NULL, NULL);
CGameMenuItemZCycle itemMutatorWeaponsVer("WEAPON BEHAVIOR:", 3, 66, 115, 180, 0, SetWeaponsVer, pzWeaponsVersionStrings, ARRAY_SSIZE(pzWeaponsVersionStrings), 0);
CGameMenuItemZBool itemMutatorSectorBehavior("SECTOR BEHAVIOR:", 3, 66, 125, 180, false, SetSectorBehavior, "NOTBLOOD", "ORIGINAL");
CGameMenuItemZCycle itemMutatorHitscanProjectiles("HITSCAN PROJECTILES:", 3, 66, 135, 180, 0, SetHitscanProjectiles, pzHitscanProjectilesStrings, ARRAY_SSIZE(pzHitscanProjectilesStrings), 0);
CGameMenuItemZCycle itemMutatorRandomizerMode("RANDOMIZER MODE:", 3, 66, 145, 180, 0, SetRandomizerMode, pzRandomizerModeStrings, ARRAY_SSIZE(pzRandomizerModeStrings), 0);
CGameMenuItemZEdit itemMutatorRandomizerSeed("RANDOMIZER SEED:", 3, 66, 155, 180, szRandomizerSeedMenu, sizeof(szRandomizerSeedMenu), 0, SetRandomizerSeed, 0);
///////////////////

CGameMenuItemTitle itemOptionsGameTitle("GAME SETUP", 1, 160, 20, 2038);
CGameMenuItemZCycle itemOptionsGameCycleAutoAim("AUTO AIM:", 3, 66, 60, 180, 0, SetAutoAim, pzAutoAimStrings, ARRAY_SSIZE(pzAutoAimStrings), 0);
CGameMenuItemZBool itemOptionsGameBoolAutoRun("AUTO RUN:", 3, 66, 70, 180, 0, SetAutoRun, NULL, NULL);
CGameMenuItemZCycle itemOptionsGameWeaponSwitch("EQUIP PICKUPS:", 3, 66, 80, 180, 0, SetWeaponSwitch, pzWeaponSwitchStrings, ARRAY_SSIZE(pzWeaponSwitchStrings), 0);
CGameMenuItemZBool itemOptionsGameWeaponFastSwitch("FAST WEAPON SWITCH:", 3, 66, 90, 180, 0, SetWeaponFastSwitch, NULL, NULL);
CGameMenuItemZCycle itemOptionsGameAutosaveMode("AUTOSAVE TRIGGER:", 3, 66, 110, 180, 0, SetAutosaveMode, pzAutosaveModeStrings, ARRAY_SSIZE(pzAutosaveModeStrings), 0);
CGameMenuItemZBool itemOptionsGameLockSaving("LOCK MANUAL SAVING:", 3, 66, 120, 180, 0, SetLockSaving, "AUTOSAVES ONLY", "NEVER");
CGameMenuItemZBool itemOptionsGameRestoreLastSave("ASK TO RESTORE LAST SAVE:", 3, 66, 130, 180, 0, SetRestoreLastSave, NULL, NULL);
CGameMenuItemZCycle itemOptionsGameBoolVanillaMode("VANILLA MODE:", 3, 66, 140, 180, 0, SetVanillaMode, pzVanillaModeStrings, ARRAY_SSIZE(pzVanillaModeStrings), 0);

CGameMenuItemTitle itemOptionsDisplayTitle("DISPLAY SETUP", 1, 160, 20, 2038);
CGameMenuItemChain itemOptionsDisplayColor("COLOR CORRECTION", 3, 66, 40, 180, 0, &menuOptionsDisplayColor, -1, NULL, 0);
CGameMenuItemChain itemOptionsDisplayMode("VIDEO MODE", 3, 66, 50, 180, 0, &menuOptionsDisplayMode, -1, SetupVideoModeMenu, 0);
CGameMenuItemChain itemOptionsDisplayView("VIEW SETUP", 3, 66, 60, 180, 0, &menuOptionsDisplayView, -1, NULL, 0);
CGameMenuItemSlider itemOptionsDisplaySliderDetail("DETAIL:", 3, 66, 80, 180, &gDetail, 0, 4, 1, SetDetail, -1, -1);
CGameMenuItemZBool itemOptionsDisplayBoolVoxels("VOXELS:", 3, 66, 90, 180, 0, SetVoxels, NULL, NULL);
CGameMenuItemZCycle itemOptionsDisplayCrosshair("CROSSHAIR:", 3, 66, 100, 180, 0, SetCrosshair, pzCrosshairStrings, ARRAY_SSIZE(pzCrosshairStrings), 0);
CGameMenuItemZCycle itemOptionsDisplayLevelStats("LEVEL STATS:", 3, 66, 110, 180, 0, SetLevelStats, pzStatsPowerupRatioStrings, ARRAY_SSIZE(pzStatsPowerupRatioStrings), 0);
CGameMenuItemZBool itemOptionsDisplayLevelStatsOnAutomap("LEVEL STATS ON AUTOMAP ONLY:", 3, 66, 120, 180, gLevelStatsOnlyOnMap, SetLevelStatsOnAutomap, NULL, NULL);
CGameMenuItemZCycle itemOptionsDisplayPowerupDuration("POWERUP DURATION:", 3, 66, 130, 180, 0, SetPowerupDuration, pzStatsPowerupRatioStrings, ARRAY_SSIZE(pzStatsPowerupRatioStrings), 0);
CGameMenuItemZBool itemOptionsDisplayBoolShowMapTitle("MAP TITLE:", 3, 66, 140, 180, gShowMapTitle, SetShowMapTitle, NULL, NULL);
CGameMenuItemZBool itemOptionsDisplayBoolMessages("MESSAGES:", 3, 66, 150, 180, gMessageState, SetMessages, NULL, NULL);
CGameMenuItemZBool itemOptionsDisplayBoolWidescreen("WIDESCREEN:", 3, 66, 160, 180, r_usenewaspect, SetWidescreen, NULL, NULL);
CGameMenuItemZCycle itemOptionsDisplayWeaponSelect("SHOW WEAPON SELECT:", 3, 66, 170, 180, 0, SetWeaponSelectMode, pzWeaponSelectStrings, ARRAY_SSIZE(pzWeaponSelectStrings), 0);
CGameMenuItemSlider itemOptionsDisplayFOV("FOV:", 3, 66, 180, 180, &gFov, 75, 140, 1, SetFOV, -1, -1, kMenuSliderValue);

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
    "ON",
#if defined USE_OPENGL && defined _WIN32 && defined RENDERTYPESDL
    "KMT"
#endif
};

const int nVSyncValues[] = {
    -1,
    0,
    1,
#if defined USE_OPENGL && defined _WIN32 && defined RENDERTYPESDL
    2
#endif
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

const char *pzInvertPaletteStrings[] = {
    "OFF",
    "ON",
    "INVERT TWICE",
};


void PreDrawVideoModeMenu(CGameMenuItem *);

CGameMenuItemTitle itemOptionsDisplayModeTitle("VIDEO MODE", 1, 160, 20, 2038);
CGameMenuItemZCycle itemOptionsDisplayModeResolution("RESOLUTION:", 3, 66, 60, 180, 0, NULL, NULL, 0, 0, true);
CGameMenuItemZCycle itemOptionsDisplayModeRenderer("RENDERER:", 3, 66, 70, 180, 0, NULL, pzRendererStrings, 2, 0);
#ifdef USE_OPENGL
CGameMenuItemChain itemOptionsDisplayModePolymost("POLYMOST SETUP", 3, 66, 80, 180, 0, &menuOptionsDisplayPolymost, -1, SetupVideoPolymostMenu, 0);
#endif
CGameMenuItemZBool itemOptionsDisplayModeFullscreen("FULLSCREEN:", 3, 66, 90, 180, 0, NULL, NULL, NULL);
CGameMenuItemZCycle itemOptionsDisplayModeVSync("VSYNC:", 3, 66, 100, 180, 0, NULL, pzVSyncStrings, ARRAY_SSIZE(pzVSyncStrings), 0);
CGameMenuItemZCycle itemOptionsDisplayModeFrameLimit("FRAMERATE LIMIT:", 3, 66, 110, 180, 0, UpdateVideoModeMenuFrameLimit, pzFrameLimitStrings, 8, 0);
CGameMenuItemChain itemOptionsDisplayModeApply("APPLY CHANGES", 3, 66, 125, 180, 0, NULL, 0, SetVideoMode, 0);

void PreDrawDisplayColor(CGameMenuItem *);

CGameMenuItemTitle itemOptionsDisplayColorTitle("COLOR CORRECTION", 1, 160, 20, -1);
CGameMenuItemZCycle itemOptionsDisplayColorPaletteCustom("PALETTE:", 3, 66, 60, 180, 0, UpdateVideoPaletteCycleMenu, srcCustomPaletteStr, ARRAY_SSIZE(srcCustomPaletteStr), 0);
CGameMenuItemZBool itemOptionsDisplayColorPaletteCIEDE2000("CIEDE2000 COMPARE:", 3, 66, 70, 180, 0, UpdateVideoPaletteBoolMenu, NULL, NULL);
CGameMenuItemZBool itemOptionsDisplayColorPaletteGrayscale("GRAYSCALE PALETTE:", 3, 66, 80, 180, 0, UpdateVideoPaletteBoolMenu, NULL, NULL);
CGameMenuItemZCycle itemOptionsDisplayColorPaletteInvert("INVERT PALETTE:", 3, 66, 90, 180, 0, UpdateVideoPaletteCycleMenu, pzInvertPaletteStrings, ARRAY_SSIZE(pzInvertPaletteStrings), 0);
CGameMenuItemSliderFloat itemOptionsDisplayColorGamma("GAMMA:", 3, 66, 100, 180, &g_videoGamma, MIN_GAMMA, MAX_GAMMA, 0.1f, UpdateVideoColorMenu, -1, -1, kMenuSliderValue);
CGameMenuItemSliderFloat itemOptionsDisplayColorContrast("CONTRAST:", 3, 66, 110, 180, &g_videoContrast, MIN_CONTRAST, MAX_CONTRAST, 0.05f, UpdateVideoColorMenu, -1, -1, kMenuSliderValue);
CGameMenuItemSliderFloat itemOptionsDisplayColorSaturation("SATURATION:", 3, 66, 120, 180, &g_videoSaturation, MIN_SATURATION, MAX_SATURATION, 0.05f, UpdateVideoColorMenu, -1, -1, kMenuSliderValue);
CGameMenuItemSliderFloat itemOptionsDisplayColorVisibility("VISIBILITY:", 3, 66, 130, 180, &r_ambientlight, 0.125f, 4.f, 0.125f, UpdateVideoColorMenu, -1, -1, kMenuSliderValue);
CGameMenuItemChain itemOptionsDisplayColorReset("RESET TO DEFAULTS", 3, 66, 150, 180, 0, NULL, 0, ResetVideoColor, 0);

CGameMenuItemTitle itemOptionsDisplayViewTitle("VIEW SETUP", 1, 160, 20, 2038);
CGameMenuItemSlider itemOptionsDisplayViewHudSize("HUD SIZE:", 3, 66, 50, 180, &gViewSize, 0, 9, 1, SetHudSize, -1, -1, kMenuSliderValue);
CGameMenuItemZCycle itemOptionsDisplayViewHudRatio("HUD ALIGNMENT:", 3, 66, 60, 180, 0, SetHudRatio, pzHudRatioStrings, ARRAY_SSIZE(pzHudRatioStrings), 0);
CGameMenuItemZBool itemOptionsDisplayViewBoolCenterHoriz("CENTER HORIZON LINE:", 3, 66, 70, 180, gCenterHoriz, SetCenterHoriz, NULL, NULL);
CGameMenuItemZBool itemOptionsDisplayViewBoolSlopeTilting("SLOPE TILTING:", 3, 66, 80, 180, gSlopeTilting, SetSlopeTilting, NULL, NULL);
CGameMenuItemZBool itemOptionsDisplayViewBoolViewBobbing("VIEW BOBBING:", 3, 66, 90, 180, gViewVBobbing, SetViewBobbing, NULL, NULL);
CGameMenuItemZBool itemOptionsDisplayViewBoolViewSwaying("VIEW SWAYING:", 3, 66, 100, 180, gViewHBobbing, SetViewSwaying, NULL, NULL);
CGameMenuItemZCycle itemOptionsDisplayViewWeaponSwaying("WEAPON SWAYING:", 3, 66, 110, 180, 0, SetWeaponSwaying, pzWeaponHBobbingStrings, ARRAY_SSIZE(pzWeaponHBobbingStrings), 0);
CGameMenuItemZCycle itemOptionsDisplayViewWeaponInterpolation("WEAPON SMOOTHING:", 3, 66, 120, 180, 0, SetWeaponInterpolate, pzWeaponInterpolateStrings, ARRAY_SSIZE(pzWeaponInterpolateStrings), 0);
CGameMenuItemZBool itemOptionsDisplayViewBoolInterpolation("VIEW INTERPOLATE:", 3, 66, 130, 180, gViewInterpolateMethod, SetViewInterpolate, "MODERN", "ORIGINAL");
CGameMenuItemZBool itemOptionsDisplayViewBoolLevelCompleteTime("LEVEL TIME AT INTERMISSION:", 3, 66, 140, 180, gShowCompleteTime, SetLevelCompleteTime, "SHOW", "HIDE");
CGameMenuItemZBool itemOptionsDisplayViewBoolPowerupStyle("POWERUP STYLE:", 3, 66, 150, 180, gPowerupStyle, SetPowerupStyle, "NOTBLOOD", "NBLOOD");
CGameMenuItemZCycle itemOptionsDisplayViewMirrorMode("MIRROR MODE:", 3, 66, 160, 180, 0, SetMirrorMode, pzMirrorModeStrings, ARRAY_SSIZE(pzMirrorModeStrings), 0);
CGameMenuItemZBool itemOptionsDisplayViewBoolSlowRoomFlicker("SLOW FLICKERING LIGHTS:", 3, 66, 170, 180, gSlowRoomFlicker, SetSlowRoomFlicker, NULL, NULL);

#ifdef USE_OPENGL
const char *pzTextureModeStrings[] = {
    "CLASSIC",
    "FILTERED"
};

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
void UpdateShadeInterpolation(CGameMenuItemZBool *pItem);
void UpdateYShrearing(CGameMenuItemZBool *pItem);
void UpdateRollAngle(CGameMenuItemSlider *pItem);
#ifdef USE_OPENGL
void PreDrawDisplayPolymost(CGameMenuItem *pItem);
CGameMenuItemTitle itemOptionsDisplayPolymostTitle("POLYMOST SETUP", 1, 160, 20, 2038);
CGameMenuItemZCycle itemOptionsDisplayPolymostTextureMode("TEXTURE MODE:", 3, 66, 40, 180, 0, UpdateTextureMode, pzTextureModeStrings, 2, 0);
CGameMenuItemZCycle itemOptionsDisplayPolymostAnisotropy("ANISOTROPY:", 3, 66, 50, 180, 0, UpdateAnisotropy, pzAnisotropyStrings, 6, 0);
CGameMenuItemZBool itemOptionsDisplayPolymostTrueColorTextures("TRUE COLOR TEXTURES:", 3, 66, 60, 180, 0, UpdateTrueColorTextures, NULL, NULL);
CGameMenuItemZCycle itemOptionsDisplayPolymostTexQuality("GL TEXTURE QUALITY:", 3, 66, 70, 180, 0, UpdateTexQuality, pzTexQualityStrings, 3, 0);
CGameMenuItemZBool itemOptionsDisplayPolymostPreloadCache("PRE-LOAD MAP TEXTURES:", 3, 66, 80, 180, 0, UpdatePreloadCache, NULL, NULL);
CGameMenuItemZCycle itemOptionsDisplayPolymostTexCache("ON-DISK TEXTURE CACHE:", 3, 66, 90, 180, 0, UpdateTexCache, pzTexCacheStrings, 3, 0);
CGameMenuItemZBool itemOptionsDisplayPolymostDetailTex("DETAIL TEXTURES:", 3, 66, 100, 180, 0, UpdateDetailTex, NULL, NULL);
CGameMenuItemZBool itemOptionsDisplayPolymostGlowTex("GLOW TEXTURES:", 3, 66, 110, 180, 0, UpdateGlowTex, NULL, NULL);
CGameMenuItemZBool itemOptionsDisplayPolymost3DModels("3D MODELS:", 3, 66, 120, 180, 0, Update3DModels, NULL, NULL);
CGameMenuItemZBool itemOptionsDisplayPolymostDeliriumBlur("DELIRIUM EFFECT BLUR:", 3, 66, 130, 180, 0, UpdateDeliriumBlur, NULL, NULL);
CGameMenuItemZBool itemOptionsDisplayPolymostUseColorIndexedTex("PALETTE EMULATON:", 3, 66, 140, 180, 0, UpdateTexColorIndex, NULL, NULL);
CGameMenuItemZBool itemOptionsDisplayPolymostShadeInterpolation("PALETTE INTERPOLATION:", 3, 66, 150, 180, 0, UpdateShadeInterpolation, NULL, NULL);
CGameMenuItemZBool itemOptionsDisplayPolymostYShearing("Y-SHEARING:", 3, 66, 160, 180, 0, UpdateYShrearing, NULL, NULL);
CGameMenuItemSlider itemOptionsDisplayPolymostRollAngle("VIEW ROLLING:", 3, 66, 170, 180, &gRollAngle, 0, 5, 1, UpdateRollAngle, -1, -1, kMenuSliderValue);
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
#ifdef _WIN32
void UpdateMidiDevice(CGameMenuItemSlider *pItem);
#endif
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
#ifdef _WIN32
CGameMenuItemSlider itemOptionsSoundMIDIDevice("MIDI DEVICE:", 3, 66, 160, 180, 0, 0, 255, 1, UpdateMidiDevice, -1, -1, kMenuSliderValue);
#endif
CGameMenuItemChain itemOptionsSoundApplyChanges("APPLY CHANGES", 3, 66, 170, 180, 0, NULL, 0, SetSound, 0);


void UpdatePlayerName(CGameMenuItemZEdit *pItem, CGameMenuEvent *pEvent);
void UpdatePlayerSkill(CGameMenuItemZCycle *pItem);
void UpdatePlayerTeamPreference(CGameMenuItemZCycle *pItem);
void UpdatePlayerColorPreference(CGameMenuItemZCycle *pItem);
void UpdatePlayerModel(CGameMenuItemZBool *pItem);
void SetShowPlayerNames(CGameMenuItemZBool *);
void SetShowWeapons(CGameMenuItemZCycle *);
void UpdatePlayerChatMessageSound(CGameMenuItemZBool *pItem);
void UpdatePlayerColorMessages(CGameMenuItemZBool *pItem);
void UpdatePlayerKillObituaryMessages(CGameMenuItemZBool *pItem);
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

const char *pzPlayerTeamPreferenceStrings[] = {
    "NONE",
    "BLUE",
    "RED"
};


const char *pzPlayerColorPreferenceStrings[] = {
    "NONE",
    "BLUE",
    "RED",
    "TEAL",
    "GRAY",
};

const char *pzShowWeaponStrings[] = {
    "OFF",
    "SPRITE",
    "VOXEL"
};

CGameMenuItemTitle itemOptionsPlayerTitle("PLAYER SETUP", 1, 160, 20, 2038);
CGameMenuItemZEdit itemOptionsPlayerName("PLAYER NAME:", 3, 66, 40, 180, szPlayerName, MAXPLAYERNAME, 0, UpdatePlayerName, 0);
CGameMenuItemZCycle itemOptionsPlayerSkill("HEALTH HANDICAP:", 3, 66, 50, 180, 0, UpdatePlayerSkill, pzPlayerSkillStrings, ARRAY_SIZE(pzPlayerSkillStrings), 0);
CGameMenuItemZCycle itemOptionsPlayerTeamPreference("TEAM PREFERENCE:", 3, 66, 60, 180, 0, UpdatePlayerTeamPreference, pzPlayerTeamPreferenceStrings, ARRAY_SIZE(pzPlayerTeamPreferenceStrings), 0);
CGameMenuItemZCycle itemOptionsPlayerColorPreference("COLOR PREFERENCE:", 3, 66, 70, 180, 0, UpdatePlayerColorPreference, pzPlayerColorPreferenceStrings, ARRAY_SIZE(pzPlayerColorPreferenceStrings), 0);
CGameMenuItemZBool itemOptionsPlayerModel("PLAYER MODEL:", 3, 66, 80, 180, false, UpdatePlayerModel, "CULTIST", "CALEB");
CGameMenuItemZBool itemOptionsPlayerBoolShowPlayerNames("SHOW PLAYER NAMES:", 3, 66, 100, 180, gShowPlayerNames, SetShowPlayerNames, NULL, NULL);
CGameMenuItemZCycle itemOptionsPlayerShowWeapons("SHOW WEAPONS:", 3, 66, 110, 180, 0, SetShowWeapons, pzShowWeaponStrings, ARRAY_SSIZE(pzShowWeaponStrings), 0);
CGameMenuItemZBool itemOptionsPlayerChatSound("MESSAGE BEEP:", 3, 66, 120, 180, true, UpdatePlayerChatMessageSound, NULL, NULL);
CGameMenuItemZBool itemOptionsPlayerColorMsg("COLORED MESSAGES:", 3, 66, 130, 180, true, UpdatePlayerColorMessages, NULL, NULL);
CGameMenuItemZBool itemOptionsPlayerObituaryMsg("USE OBITUARY MESSAGES:", 3, 66, 140, 180, true, UpdatePlayerKillObituaryMessages, NULL, NULL);
CGameMenuItemZBool itemOptionsPlayerKillMsg("SHOW KILLS ON HUD:", 3, 66, 150, 180, true, UpdatePlayerKillMessage, NULL, NULL);
CGameMenuItemZCycle itemOptionsPlayerMultiKill("MULTI KILL MESSAGES:", 3, 66, 160, 180, 0, UpdatePlayerMultiKill, pzPlayerMultiKillStrings, ARRAY_SIZE(pzPlayerMultiKillStrings), 0);

#define JOYSTICKITEMSPERPAGE 16 // this must be an even value, as double tap inputs rely on odd index position
#define MAXJOYSTICKBUTTONPAGES (max(1, (MAXJOYBUTTONSANDHATS*2 / JOYSTICKITEMSPERPAGE))) // we double all buttons/hats so each input can be bind for double tap

CGameMenu menuOptionsControlKeyboard;
CGameMenu menuKeys;
CGameMenu menuOptionsControlMouse;
CGameMenu menuOptionsControlMouseButtonAssignment;
CGameMenu menuOptionsControlJoystickButtonAssignment[MAXJOYSTICKBUTTONPAGES];
CGameMenu menuOptionsControlJoystickListAxes; // contains list of editable joystick axes
CGameMenu menuOptionsControlJoystickAxis[MAXJOYAXES]; // options menu for each joystick axis
CGameMenu menuOptionsControlJoystickMisc;

void SetupMouseMenu(CGameMenuItemChain *pItem);
void SetupJoystickButtonsMenu(CGameMenuItemChain *pItem);
void SetupJoystickAxesMenu(CGameMenuItemChain *pItem);
void SetJoystickScale(CGameMenuItemSlider* pItem);
void SetJoystickAnalogue(CGameMenuItemZCycle* pItem);
void SetJoystickAnalogueInvert(CGameMenuItemZBool* pItem);
void SetJoystickDigitalPos(CGameMenuItemZCycle* pItem);
void SetJoystickDigitalNeg(CGameMenuItemZCycle* pItem);
void SetJoystickDeadzone(CGameMenuItemSlider* pItem);
void SetJoystickSaturate(CGameMenuItemSlider* pItem);
void SetJoystickSoloDeadzone(CGameMenuItemZBool* pItem);

const char *pzTurnAccelerationStrings[] = {
    "OFF",
    "ON RUNNING",
    "ALWAYS ON",
};

CGameMenuItemTitle itemOptionsControlTitle("CONTROL SETUP", 1, 160, 20, 2038);
CGameMenuItemChain itemOptionsControlKeyboard("KEYBOARD SETUP", 1, 0, 60, 320, 1, &menuOptionsControlKeyboard, -1, NULL, 0);
CGameMenuItemChain itemOptionsControlMouse("MOUSE SETUP", 1, 0, 80, 320, 1, &menuOptionsControlMouse, -1, SetupMouseMenu, 0);
CGameMenuItemChain itemOptionsControlJoystickButtons("JOYSTICK BUTTONS SETUP", 1, 0, 120, 320, 1, &menuOptionsControlJoystickButtonAssignment[0], -1, SetupJoystickButtonsMenu, 0);
CGameMenuItemChain itemOptionsControlJoystickAxes("JOYSTICK AXES SETUP", 1, 0, 140, 320, 1, &menuOptionsControlJoystickListAxes, -1, SetupJoystickAxesMenu, 0);
CGameMenuItemChain itemOptionsControlJoystickMisc("JOYSTICK MISC SETUP", 1, 0, 160, 320, 1, &menuOptionsControlJoystickMisc, -1, NULL, 0);

CGameMenuItemTitle itemOptionsControlKeyboardTitle("KEYBOARD SETUP", 1, 160, 20, 2038);
CGameMenuItemSlider itemOptionsControlKeyboardSliderTurnSpeed("Key Turn Speed:", 1, 18, 50, 280, &gTurnSpeed, 64, 128, 4, SetTurnSpeed, -1, -1);
CGameMenuItemZCycle itemOptionsControlKeyboardCycleTurnAcceleration("Key Turn Acceleration:", 1, 18, 70, 280, 0, SetTurnAcceleration, pzTurnAccelerationStrings, ARRAY_SIZE(pzTurnAccelerationStrings), 0);
CGameMenuItemZBool itemOptionsControlKeyboardBoolCrouchToggle("Crouch Toggle:", 1, 18, 90, 280, gCrouchToggle, SetCrouchToggle, NULL, NULL);
CGameMenuItemChain itemOptionsControlKeyboardList("Configure Keys...", 1, 0, 110, 320, 1, &menuKeys, -1, NULL, 0);
CGameMenuItemChain itemOptionsControlKeyboardReset("Reset Keys (default)...", 1, 0, 135, 320, 1, &menuKeys, -1, ResetKeys, 0);
CGameMenuItemChain itemOptionsControlKeyboardResetClassic("Reset Keys (classic)...", 1, 0, 155, 320, 1, &menuKeys, -1, ResetKeysClassic, 0);

CGameMenuItemTitle itemKeysTitle("KEY SETUP", 1, 160, 20, 2038);
CGameMenuItemKeyList itemKeyList("", 3, 56, 40, 200, 16, NUMGAMEFUNCTIONS, 0);

void SetMouseAimMode(CGameMenuItemZBool *pItem);
void SetMouseVerticalAim(CGameMenuItemZBool *pItem);
void SetMouseXSensitivity(CGameMenuItemSliderFloat *pItem);
void SetMouseYSensitivity(CGameMenuItemSliderFloat*pItem);

void PreDrawControlMouse(CGameMenuItem *pItem);
void SetMouseButton(CGameMenuItemZCycle *pItem);
void SetJoyButton(CGameMenuItemZCycle *pItem);

void SetupMouseButtonMenu(CGameMenuItemChain *pItem);

CGameMenuItemTitle itemOptionsControlMouseTitle("MOUSE SETUP", 1, 160, 20, 2038);
CGameMenuItemChain itemOptionsControlMouseButton("BUTTON ASSIGNMENT", 3, 66, 60, 180, 0, &menuOptionsControlMouseButtonAssignment, 0, SetupMouseButtonMenu, 0);
CGameMenuItemSliderFloat itemOptionsControlMouseSensitivity("SENSITIVITY:", 3, 66, 70, 180, &CONTROL_MouseSensitivity, 0.f, 100.f, 1.f, SetMouseSensitivity, -1, -1, kMenuSliderValue);
CGameMenuItemZBool itemOptionsControlMouseAimFlipped("INVERT AIMING:", 3, 66, 80, 180, false, SetMouseAimFlipped, NULL, NULL);
CGameMenuItemZBool itemOptionsControlMouseAimMode("AIMING TYPE:", 3, 66, 90, 180, false, SetMouseAimMode, "HOLD", "TOGGLE");
CGameMenuItemZBool itemOptionsControlMouseVerticalAim("VERTICAL AIMING:", 3, 66, 100, 180, false, SetMouseVerticalAim, NULL, NULL);
CGameMenuItemSliderFloat itemOptionsControlMouseXSensitivity("HORIZ SENS:", 3, 66, 110, 180, &CONTROL_MouseAxesSensitivity[0], 0.f, 100.f, 1.f, SetMouseXSensitivity, -1, -1, kMenuSliderValue);
CGameMenuItemSliderFloat itemOptionsControlMouseYSensitivity("VERT SENS:", 3, 66, 120, 180, &CONTROL_MouseAxesSensitivity[1], 0.f, 100.f, 1.f, SetMouseYSensitivity, -1, -1, kMenuSliderValue);

void SetupNetworkMenu(void);
void SetupNetworkHostMenu(CGameMenuItemChain *pItem);
void SetupNetworkJoinMenu(CGameMenuItemChain *pItem);
void NetworkHostGame(CGameMenuItemChain *pItem);
void NetworkJoinGame(CGameMenuItemChain *pItem);

char zNetAddressBuffer[16] = "localhost";
char zNetPortBuffer[6] = "23513";

CGameMenuItemTitle itemNetworkTitle("MULTIPLAYER", 1, 160, 20, 2038);
CGameMenuItemChain itemNetworkHost("HOST A GAME", 1, 0, 70, 320, 1, &menuNetworkHost, -1, SetupNetworkHostMenu, 0);
CGameMenuItemChain itemNetworkJoin("JOIN A GAME", 1, 0, 90, 320, 1, &menuNetworkJoin, -1, SetupNetworkJoinMenu, 0);
CGameMenuItemChain itemNetworkPlayer("PLAYER SETUP", 1, 0, 110, 320, 1, &menuOptionsPlayer, -1, NULL, 0);

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

CGameMenuItemZCycle *pItemOptionsControlMouseButton[MENUMOUSEFUNCTIONS];

char MenuJoyButtonNames[MAXJOYBUTTONSANDHATS*2][64] = {""};

const char *zJoystickAnalogue[] =
{
    "-None-",
    "Turning",
    "Strafing",
    "Moving",
    "Look Up/Down",
};

CGameMenuItemTitle itemJoyButtonsTitle("JOYSTICK SETUP", 1, 160, 20, 2038);
CGameMenuItemZCycle *pItemOptionsControlJoyButton[MAXJOYSTICKBUTTONPAGES][JOYSTICKITEMSPERPAGE];
CGameMenuItemChain *pItemOptionsControlJoyButtonNextPage[MAXJOYSTICKBUTTONPAGES];

char MenuJoyAxisNames[MAXJOYAXES][64] = {""};

CGameMenuItemTitle itemJoyAxesTitle("JOYSTICK AXES", 1, 160, 20, 2038);
CGameMenuItemChain *pItemOptionsControlJoystickAxis[MAXJOYAXES]; // dynamic list for each axis

CGameMenuItemTitle *pItemOptionsControlJoystickAxisName[MAXJOYAXES];
CGameMenuItemSlider *pItemOptionsControlJoystickAxisScale[MAXJOYAXES];
CGameMenuItemZCycle *pItemOptionsControlJoystickAxisAnalogue[MAXJOYAXES];
CGameMenuItemZBool *pItemOptionsControlJoystickAxisAnalogueInvert[MAXJOYAXES];
CGameMenuItemZCycle *pItemOptionsControlJoystickAxisDigitalPos[MAXJOYAXES];
CGameMenuItemZCycle *pItemOptionsControlJoystickAxisDigitalNeg[MAXJOYAXES];
CGameMenuItemSlider *pItemOptionsControlJoystickAxisDeadzone[MAXJOYAXES];
CGameMenuItemSlider *pItemOptionsControlJoystickAxisSaturate[MAXJOYAXES];
CGameMenuItemZBool *pItemOptionsControlJoystickAxisSoloDeadzone[MAXJOYAXES];

CGameMenuItemTitle itemOptionsControlJoystickMiscTitle("JOYSTICK MISC", 1, 160, 20, 2038);
CGameMenuItemZBool itemOptionsControlJoystickMiscCrouchToggle("CROUCH TOGGLE:", 1, 18, 70, 280, gCrouchToggle, SetCrouchToggle, NULL, NULL);
CGameMenuItemZBool itemOptionsControlJoystickMiscCenterView("CENTER VIEW ON DROP:", 1, 18, 90, 280, gCenterViewOnDrop, SetCenterView, NULL, NULL);
CGameMenuItemZBool itemOptionsControlJoystickMiscTargetAimAssist("TARGET AIM ASSIST:", 1, 18, 110, 280, 0, SetJoystickTargetAimAssist, NULL, NULL);
CGameMenuItemZBool itemOptionsControlJoystickMiscRumble("RUMBLE CONTROLLER:", 1, 18, 130, 280, 0, SetJoystickRumble, NULL, NULL);

void SetupLoadingScreen(void)
{
    menuLoading.Add(&itemLoadingText, true);
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

void SetupDifficultyMenu(void)
{
    menuDifficulty.Add(&itemDifficultyTitle, false);
    menuDifficulty.Add(&itemDifficulty1, false);
    menuDifficulty.Add(&itemDifficulty2, false);
    menuDifficulty.Add(&itemDifficulty3, true);
    menuDifficulty.Add(&itemDifficulty4, false);
    menuDifficulty.Add(&itemDifficulty5, false);
    menuDifficulty.Add(&itemDifficulty6, false);
    menuDifficulty.Add(&itemBloodQAV, false);

    menuCustomDifficulty.Add(&itemCustomDifficultyTitle, false);
    menuCustomDifficulty.Add(&itemCustomDifficultyMonsterSettings, true);
    menuCustomDifficulty.Add(&itemCustomDifficultyEnemyQuantity, false);
    menuCustomDifficulty.Add(&itemCustomDifficultyEnemyHealth, false);
    menuCustomDifficulty.Add(&itemCustomDifficultyEnemyDifficulty, false);
    menuCustomDifficulty.Add(&itemCustomDifficultyPlayerDamage, false);
    menuCustomDifficulty.Add(&itemCustomDifficultyEnemySpeed, false);
    menuCustomDifficulty.Add(&itemCustomDifficultyEnemyShuffle, false);
    menuCustomDifficulty.Add(&itemCustomDifficultyPitchfork, false);
    menuCustomDifficulty.Add(&itemCustomDifficultyPermaDeath, false);
    menuCustomDifficulty.Add(&itemCustomDifficultyBannedMonsters, false);
    menuCustomDifficulty.Add(&itemCustomDifficultyBannedItems, false);
    menuCustomDifficulty.Add(&itemCustomDifficultyStart, false);
    menuCustomDifficulty.Add(&itemBloodQAV, false);
    itemCustomDifficultyEnemyQuantity.tooltip_pzTextUpper = "Set how many enemies will spawn in the level";
    itemCustomDifficultyEnemyHealth.tooltip_pzTextUpper = "Set enemy's starting health";
    itemCustomDifficultyEnemyDifficulty.tooltip_pzTextUpper = "Set enemy's behavior difficulty";
    itemCustomDifficultyPlayerDamage.tooltip_pzTextUpper = "Set player's damage taken scale";
    itemCustomDifficultyEnemySpeed.tooltip_pzTextUpper = "Set enemy's movement speed modifier";
    itemCustomDifficultyEnemyShuffle.tooltip_pzTextUpper = "Shuffle enemy's spawn position";
    itemCustomDifficultyPitchfork.tooltip_pzTextUpper = "Player will lose all items on new level";
    itemCustomDifficultyPermaDeath.tooltip_pzTextUpper = "No saving, and you only live once";
    itemCustomDifficultyBannedMonsters.tooltip_pzTextUpper = "Set which monsters to spawn";
    itemCustomDifficultyBannedItems.tooltip_pzTextUpper = "Set which items to spawn";

    menuBannedMonsters.Add(&itemBannedMonstersTitle, false);
    menuBannedMonsters.Add(&itemBannedMonstersBats, true);
    menuBannedMonsters.Add(&itemBannedMonstersRats, false);
    menuBannedMonsters.Add(&itemBannedMonstersFish, false);
    menuBannedMonsters.Add(&itemBannedMonstersHands, false);
    menuBannedMonsters.Add(&itemBannedMonstersGhosts, false);
    menuBannedMonsters.Add(&itemBannedMonstersSpiders, false);
    menuBannedMonsters.Add(&itemBannedMonstersTinyCaleb, false);
    menuBannedMonsters.Add(&itemBannedMonstersHellHounds, false);
    menuBannedMonsters.Add(&itemBannedMonstersRatsAttack, false);
    menuBannedMonsters.Add(&itemBannedMonstersMotherSpiderHealth, false);
    menuBannedMonsters.Add(&itemBannedMonstersTchernobogHealth, false);
    menuBannedMonsters.Add(&itemBloodQAV, false);
    itemBannedMonstersSpiders.tooltip_pzTextUpper = "";
    itemBannedMonstersSpiders.tooltip_pzTextLower = "Mother spiders are not removed to avoid boss softlocks";
    itemBannedMonstersTchernobogHealth.tooltip_pzTextUpper = "";
    itemBannedMonstersTchernobogHealth.tooltip_pzTextLower = "Fix overflow health bug for Tchernobog";

    menuBannedItems.Add(&itemBannedItemsTitle, false);
    menuBannedItems.Add(&itemBannedItemsFlare, true);
    menuBannedItems.Add(&itemBannedItemsShotgun, false);
    menuBannedItems.Add(&itemBannedItemsTommyGun, false);
    menuBannedItems.Add(&itemBannedItemsNapalm, false);
    menuBannedItems.Add(&itemBannedItemsTNT, false);
    menuBannedItems.Add(&itemBannedItemsSpray, false);
    menuBannedItems.Add(&itemBannedItemsTesla, false);
    menuBannedItems.Add(&itemBannedItemsLifeLeech, false);
    menuBannedItems.Add(&itemBannedItemsVoodoo, false);
    menuBannedItems.Add(&itemBannedItemsProxy, false);
    menuBannedItems.Add(&itemBannedItemsRemote, false);
    menuBannedItems.Add(&itemBannedItemsMedKit, false);
    menuBannedItems.Add(&itemBannedItemsLifeEssence, false);
    menuBannedItems.Add(&itemBannedItemsLifeSeed, false);
    menuBannedItems.Add(&itemBannedItemsSuperArmor, false);
    menuBannedItems.Add(&itemBannedItemsCrystalBall, false);
    menuBannedItems.Add(&itemBannedItemsJumpBoots, false);
    menuBannedItems.Add(&itemBannedItemsCloak, false);
    menuBannedItems.Add(&itemBannedItemsDeathMask, false);
    menuBannedItems.Add(&itemBannedItemsAkimbo, false);
    menuBannedItems.Add(&itemBannedItemsReflect, false);
    menuBannedItems.Add(&itemBloodQAV, false);
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
        if (!pEpisode->bloodbath || gGameOptions.nGameType != kGameTypeSinglePlayer)
            nOffset -= 10;
    }
    nOffset = max(min(nOffset, 55), 35);
    int j = 0;
    for (int i = 0; i < gEpisodeCount; i++)
    {
        EPISODEINFO *pEpisode = &gEpisodeInfo[i];
        if (!pEpisode->bloodbath || gGameOptions.nGameType != kGameTypeSinglePlayer)
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
    if (gGameOptions.nGameType != kGameTypeSinglePlayer)
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

#ifdef NETCODE_DISABLE
    itemMain2.bEnable = 0; // disable multiplayer menu item for non-netcode build
#endif
}

void SetupMainMenuWithSave(void)
{
    menuMainWithSave.Add(&itemMainSaveTitle, false);
    menuMainWithSave.Add(&itemMainSave1, true);
    if (gGameOptions.nGameType != kGameTypeSinglePlayer)
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

    itemMainSave3.bDisableForNet = itemMainSave3.bDisableForPermaDeath = 1;
    itemMainSave4.bDisableForNet = 1;
    itemMainSave5.bDisableForNet = 1;
    itemMainSave6.bDisableForNet = 1;
    itemMainSave3.bEnable = !gLockManualSaving; // disable save option in main menu if lock saving mode is set
}

void SetupFirstLaunchMenu(void)
{
    menuFirstLaunch.Add(&itemFirstLaunchInfo1, false);
    menuFirstLaunch.Add(&itemFirstLaunchInfo2, false);
    menuFirstLaunch.Add(&itemFirstLaunchVanilla, true);
    menuFirstLaunch.Add(&itemFirstLaunchNBlood, false);
    menuFirstLaunch.Add(&itemFirstLaunchNotBlood, false);
    itemFirstLaunchVanilla.tooltip_pzTextUpper = "Set options to match v1.21 DOS Blood and use vanilla mode";
    itemFirstLaunchNBlood.tooltip_pzTextUpper = "Set options to match NBlood";
    itemFirstLaunchNotBlood.tooltip_pzTextUpper = "Keep default NotBlood options";
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
    menuMultiUserMaps.Add(&itemNetStartUserMapTitle, true);
    menuMultiUserMaps.Add(&menuMultiUserMap, true);

    menuNetworkGameMode.Add(&itemNetGameTitle, false);
    menuNetworkGameMode.Add(&itemNetGameMode, true);
    menuNetworkGameMode.Add(&itemNetGameCycleLength, false);
    menuNetworkGameMode.Add(&itemNetGameCycleLengthCoop, false);
    menuNetworkGameMode.Add(&itemNetGameSliderLimit, false);
    menuNetworkGameMode.Add(&itemNetGameBoolExit, false);
    menuNetworkGameMode.Add(&itemNetGameBoolTeleFrag, false);
    menuNetworkGameMode.Add(&itemNetGameBoolSkillOverride, false);
    menuNetworkGameMode.Add(&itemNetGameBoolModelOverride, false);
    menuNetworkGameMode.Add(&itemNetGameBoolFriendlyFire, false);
    menuNetworkGameMode.Add(&itemNetGameCycleKey, false);
    menuNetworkGameMode.Add(&itemNetGameCycleItemWeapon, false);
    menuNetworkGameMode.Add(&itemNetGameBoolAutoTeams, false);
    menuNetworkGameMode.Add(&itemNetGameBoolTeamColors, false);
    menuNetworkGameMode.Add(&itemNetGameBoolTeamFlags, false);
    menuNetworkGameMode.Add(&itemNetGameCycleSpawnLocation, false);
    menuNetworkGameMode.Add(&itemNetGameCycleShowWeaponsOverride, false);
    menuNetworkGameMode.Add(&itemNetGameCycleSpawnProtection, false);
    menuNetworkGameMode.Add(&itemNetGameCycleSpawnWeapon, false);
    menuNetworkGameMode.Add(&itemNetGameCycleMirrorModeOverride, false);
    menuNetworkGameMode.Add(&itemBloodQAV, false);
    itemNetGameBoolExit.tooltip_pzTextUpper = "Toggle level exit switch functionality";
    itemNetGameBoolTeleFrag.tooltip_pzTextUpper = "Toggle telefrags kills";
    itemNetGameBoolSkillOverride.tooltip_pzTextUpper = "Toggle player health handicap";
    itemNetGameBoolSkillOverride.tooltip_pzTextLower = "(When off, use difficulty setting)";
    itemNetGameBoolModelOverride.tooltip_pzTextUpper = "Set global setting for player models";
    itemNetGameBoolModelOverride.tooltip_pzTextLower = "(This is applied to all players in round)";
    itemNetGameBoolAutoTeams.tooltip_pzTextUpper = "Automatically sort players into teams";
    itemNetGameBoolTeamColors.tooltip_pzTextUpper = "Highlight players with team colors";
    itemNetGameBoolTeamFlags.tooltip_pzTextUpper = "Toggle team flags for teams mode";
    itemNetGameCycleSpawnLocation.tooltip_pzTextUpper = "Set spawn location behavior";
    itemNetGameCycleShowWeaponsOverride.tooltip_pzTextUpper = "Set global setting for show weapons option";
    itemNetGameCycleShowWeaponsOverride.tooltip_pzTextLower = "(This is applied to all players in round)";
    itemNetGameCycleMirrorModeOverride.tooltip_pzTextUpper = "Set global setting for mirror mode";
    itemNetGameCycleMirrorModeOverride.tooltip_pzTextLower = "(This is applied to all players in round)";

    menuNetworkGameMonsters.Add(&itemNetMonsterTitle, false);
    menuNetworkGameMonsters.Add(&itemNetMonsterSettings, true);
    menuNetworkGameMonsters.Add(&itemNetMonsterQuantity, false);
    menuNetworkGameMonsters.Add(&itemNetMonsterHealth, false);
    menuNetworkGameMonsters.Add(&itemNetMonsterSpeed, false);
    menuNetworkGameMonsters.Add(&itemNetMonsterBats, false);
    menuNetworkGameMonsters.Add(&itemNetMonsterRats, false);
    menuNetworkGameMonsters.Add(&itemNetMonsterFish, false);
    menuNetworkGameMonsters.Add(&itemNetMonsterHands, false);
    menuNetworkGameMonsters.Add(&itemNetMonsterGhosts, false);
    menuNetworkGameMonsters.Add(&itemNetMonsterSpiders, false);
    menuNetworkGameMonsters.Add(&itemNetMonsterTinyCaleb, false);
    menuNetworkGameMonsters.Add(&itemNetMonsterHellHounds, false);
    menuNetworkGameMonsters.Add(&itemNetMonsterRatsAttack, false);
    menuNetworkGameMonsters.Add(&itemNetMonsterMotherSpiderHealth, false);
    menuNetworkGameMonsters.Add(&itemNetMonsterTchernobogHealth, false);
    menuNetworkGameMonsters.Add(&itemBloodQAV, false);

    //////////////////////
    menuNetworkGameMutators.Add(&itemGameMutatorsTitle, false);
    menuNetworkGameMutators.Add(&itemNetMutatorBoolQuadDamagePowerup, true);
    menuNetworkGameMutators.Add(&itemNetMutatorDamageInvul, false);
    menuNetworkGameMutators.Add(&itemNetMutatorExplosionBehavior, false);
    menuNetworkGameMutators.Add(&itemNetMutatorProjectileBehavior, false);
    menuNetworkGameMutators.Add(&itemNetMutatorNapalmFalloff, false);
    menuNetworkGameMutators.Add(&itemNetMutatorEnemyBehavior, false);
    menuNetworkGameMutators.Add(&itemNetMutatorBoolEnemyRandomTNT, false);
    menuNetworkGameMutators.Add(&itemNetMutatorWeaponsVer, false);
    menuNetworkGameMutators.Add(&itemNetMutatorSectorBehavior, false);
    menuNetworkGameMutators.Add(&itemNetMutatorHitscanProjectiles, false);
    menuNetworkGameMutators.Add(&itemNetMutatorRandomizerMode, false);
    menuNetworkGameMutators.Add(&itemNetMutatorRandomizerSeed, false);
    menuNetworkGameMutators.Add(&itemBloodQAV, false);
    itemNetMutatorBoolQuadDamagePowerup.tooltip_pzTextUpper = "Replaces guns akimbo powerup";
    itemNetMutatorBoolQuadDamagePowerup.tooltip_pzTextLower = "with Quake's quad damage";
    itemNetMutatorDamageInvul.tooltip_pzTextUpper = "Apply a short invulnerability state";
    itemNetMutatorDamageInvul.tooltip_pzTextLower = "on bullet/spirit/tesla damage";
    itemNetMutatorExplosionBehavior.tooltip_pzTextUpper = "Uses enhanced explosion calculation";
    itemNetMutatorProjectileBehavior.tooltip_pzTextUpper = "Use smaller hitboxes and improve collision";
    itemNetMutatorProjectileBehavior.tooltip_pzTextLower = "accuracy for player projectiles";
    itemNetMutatorNapalmFalloff.tooltip_pzTextUpper = "Set napalm projectiles to";
    itemNetMutatorNapalmFalloff.tooltip_pzTextLower = "be affected by gravity";
    itemNetMutatorEnemyBehavior.tooltip_pzTextUpper = "Fix various original bugs with enemies";
    itemNetMutatorBoolEnemyRandomTNT.tooltip_pzTextUpper = "Set cultist to have a chance of";
    itemNetMutatorBoolEnemyRandomTNT.tooltip_pzTextLower = "throwing random projectiles";
    itemNetMutatorWeaponsVer.tooltip_pzTextUpper = "Check readme.txt for full";
    itemNetMutatorWeaponsVer.tooltip_pzTextLower = "list of weapon changes";
    itemNetMutatorSectorBehavior.tooltip_pzTextUpper = "Improve room over room sector handling";
    itemNetMutatorSectorBehavior.tooltip_pzTextLower = "for hitscans (e.g: firing above water)";
    itemNetMutatorHitscanProjectiles.tooltip_pzTextUpper = "Set hitscan enemies to";
    itemNetMutatorHitscanProjectiles.tooltip_pzTextLower = "spawn bullet projectiles";
    itemNetMutatorRandomizerMode.tooltip_pzTextUpper = "Set the randomizer's mode";
    itemNetMutatorRandomizerSeed.tooltip_pzTextUpper = "Set the randomizer's seed";
    itemNetMutatorRandomizerSeed.tooltip_pzTextLower = "No seed = always use a random seed";
    //////////////////////

    itemNetStart2.SetTextIndex(gMultiEpisodeInit != -1 ? gMultiEpisodeInit : 0);
    SetupLevelMenuItem(gMultiEpisodeInit != -1 ? gMultiEpisodeInit : 0);
    itemNetStart3.SetTextIndex(gMultiLevelInit != -1 ? gMultiLevelInit : 0);
    itemNetStart4.SetTextIndex(gMultiDiffInit != -1 ? gMultiDiffInit : 2);
    itemNetStart6.SetTextIndex(gMultiWeapons != -1 ? gMultiWeapons : 1);
    itemNetStart7.SetTextIndex(gMultiItems != -1 ? gMultiItems : 1);

    itemNetGameMode.SetTextIndex(gMultiModeInit != -1 ? gMultiModeInit : 1);
    itemNetGameCycleLength.SetTextIndex(gMultiLength != -1 ? gMultiLength : 0);
    itemNetGameSliderLimit.nValue = gMultiLimit != -1 ? gMultiLimit : itemNetGameSliderLimit.nValue;
    if (gMultiModeNoExit)
        itemNetGameBoolExit.at20 = 0;
    itemNetGameBoolAutoTeams.at20 = !gPlayerTeamPreference;
    if (gMultiModeNoFlag)
        itemNetGameBoolTeamFlags.at20 = 0;
    itemNetGameCycleSpawnLocation.SetTextIndex(gMultiSpawnLocation != -1 ? gMultiSpawnLocation : 1);
    itemNetGameCycleShowWeaponsOverride.SetTextIndex(1);
    itemNetGameCycleSpawnProtection.SetTextIndex(gMultiSpawnProtection != -1 ? gMultiSpawnProtection : 1);
    SetNetGameMode(&itemNetGameMode); // hide friendly fire/keys menu items depending on game mode

    itemNetMonsterSettings.SetTextIndex(gMultiMonsters != -1 ? gMultiMonsters : 0);
    SetNetMonsterMenu(NULL);

    ///////
    itemNetMutatorBoolQuadDamagePowerup.at20 = !!gQuadDamagePowerup;
    itemNetMutatorDamageInvul.m_nFocus = gDamageInvul % ARRAY_SSIZE(pzDamageInvulBehaviorStrings);
    itemNetMutatorExplosionBehavior.m_nFocus = gExplosionBehavior % ARRAY_SSIZE(pzExplosionBehaviorStrings);
    itemNetMutatorProjectileBehavior.m_nFocus = gProjectileBehavior % ARRAY_SSIZE(pzProjectileBehaviorStrings);
    itemNetMutatorNapalmFalloff.at20 = !!gNapalmFalloff;
    itemNetMutatorEnemyBehavior.m_nFocus = gEnemyBehavior % ARRAY_SSIZE(pzEnemyBehaviorStrings);
    itemNetMutatorBoolEnemyRandomTNT.at20 = !!gEnemyRandomTNT;
    itemNetMutatorWeaponsVer.m_nFocus = gWeaponsVer % ARRAY_SSIZE(pzWeaponsVersionStrings);
    itemNetMutatorSectorBehavior.at20 = !!gSectorBehavior;
    itemNetMutatorHitscanProjectiles.m_nFocus = gHitscanProjectiles % ARRAY_SSIZE(pzHitscanProjectilesStrings);
    itemNetMutatorRandomizerMode.m_nFocus = gRandomizerMode % ARRAY_SSIZE(pzRandomizerModeStrings);
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
    menuLoadGame.Add(&itemLoadGameQuick, false);
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
    itemLoadGameQuick.at35 = 0;
    itemLoadGameAutosaveStart.at35 = 0;
    itemLoadGameAutosaveKey.at35 = 0;
    itemLoadGameAutosaveStart.bEnable = gAutosave != 0; // remove autosave slots if autosaves are disabled
    itemLoadGameAutosaveStart.bNoDraw = gAutosave == 0;
    itemLoadGameAutosaveKey.bEnable = gAutosave == 2;
    itemLoadGameAutosaveKey.bNoDraw = gAutosave != 2;
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
    itemLoadGameQuick.at2c = &itemLoadGamePic;
    itemLoadGameAutosaveStart.at2c = &itemLoadGamePic;
    itemLoadGameAutosaveKey.at2c = &itemLoadGamePic;
    menuLoadGame.Add(&itemBloodQAV, false);
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

void SetupHelpMenu(void)
{
    menuHelp.Add(&itemHelp4QAV, true);
    menuHelp.Add(&itemHelp5QAV, false);
    menuHelp.Add(&itemHelp3QAV, false);
    menuHelp.Add(&itemHelp3BQAV, false);
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

void SetupOptionsMenu(void)
{
    menuOptions.Add(&itemOptionsTitle, false);
    menuOptions.Add(&itemOptionsChainGame, true);
    menuOptions.Add(&itemOptionsChainDisplay, false);
    menuOptions.Add(&itemOptionsChainSound, false);
    menuOptions.Add(&itemOptionsChainPlayer, false);
    menuOptions.Add(&itemOptionsChainControl, false);
    menuOptions.Add(&itemOptionsChainMutators, false);
    menuOptions.Add(&itemBloodQAV, false);

    menuOptionsGame.Add(&itemOptionsGameTitle, false);
    menuOptionsGame.Add(&itemOptionsGameCycleAutoAim, true);
    menuOptionsGame.Add(&itemOptionsGameBoolAutoRun, false);
    menuOptionsGame.Add(&itemOptionsGameWeaponSwitch, false);
    menuOptionsGame.Add(&itemOptionsGameWeaponFastSwitch, false);
    menuOptionsGame.Add(&itemOptionsGameAutosaveMode, false);
    menuOptionsGame.Add(&itemOptionsGameLockSaving, false);
    menuOptionsGame.Add(&itemOptionsGameRestoreLastSave, false);
    menuOptionsGame.Add(&itemOptionsGameBoolVanillaMode, false);
    itemOptionsGameLockSaving.bDisableForNet = 1;
    itemOptionsGameBoolVanillaMode.bDisableForNet = 1;
    itemOptionsGameWeaponFastSwitch.tooltip_pzTextUpper = "Allow weapon switching while";
    itemOptionsGameWeaponFastSwitch.tooltip_pzTextLower = "weapon is being lowered/raised";
    itemOptionsGameAutosaveMode.tooltip_pzTextUpper = "Set when autosave will trigger";
    itemOptionsGameLockSaving.tooltip_pzTextUpper = "Disable manual saving/save scumming";
    itemOptionsGameRestoreLastSave.tooltip_pzTextUpper = "Prompt to restore last save game on death";
    itemOptionsGameBoolVanillaMode.tooltip_pzTextUpper = "Disable all non-vanilla features/mutators";
    itemOptionsGameBoolVanillaMode.tooltip_pzTextLower = "(v1.21 DOS compatibility mode)";

    //////////////////////
    menuOptionsGameMutators.Add(&itemGameMutatorsTitle, false);
    menuOptionsGameMutators.Add(&itemMutatorBoolQuadDamagePowerup, true);
    menuOptionsGameMutators.Add(&itemMutatorDamageInvul, false);
    menuOptionsGameMutators.Add(&itemMutatorExplosionBehavior, false);
    menuOptionsGameMutators.Add(&itemMutatorProjectileBehavior, false);
    menuOptionsGameMutators.Add(&itemMutatorNapalmFalloff, false);
    menuOptionsGameMutators.Add(&itemMutatorEnemyBehavior, false);
    menuOptionsGameMutators.Add(&itemMutatorBoolEnemyRandomTNT, false);
    menuOptionsGameMutators.Add(&itemMutatorWeaponsVer, false);
    menuOptionsGameMutators.Add(&itemMutatorSectorBehavior, false);
    menuOptionsGameMutators.Add(&itemMutatorHitscanProjectiles, false);
    menuOptionsGameMutators.Add(&itemMutatorRandomizerMode, false);
    menuOptionsGameMutators.Add(&itemMutatorRandomizerSeed, false);
    menuOptionsGameMutators.Add(&itemBloodQAV, false);
    itemOptionsChainMutators.bDisableForNet = 1;
    itemMutatorBoolQuadDamagePowerup.tooltip_pzTextUpper = "Replaces guns akimbo powerup";
    itemMutatorBoolQuadDamagePowerup.tooltip_pzTextLower = "with Quake's quad damage";
    itemMutatorDamageInvul.tooltip_pzTextUpper = "Apply a short invulnerability state";
    itemMutatorDamageInvul.tooltip_pzTextLower = "on bullet/spirit/tesla damage";
    itemMutatorExplosionBehavior.tooltip_pzTextUpper = "Uses enhanced explosion calculation";
    itemMutatorProjectileBehavior.tooltip_pzTextUpper = "Use smaller hitboxes and improve collision";
    itemMutatorProjectileBehavior.tooltip_pzTextLower = "accuracy for player projectiles";
    itemMutatorNapalmFalloff.tooltip_pzTextUpper = "Set napalm projectiles to";
    itemMutatorNapalmFalloff.tooltip_pzTextLower = "be affected by gravity";
    itemMutatorEnemyBehavior.tooltip_pzTextUpper = "Fix various original bugs with enemies";
    itemMutatorBoolEnemyRandomTNT.tooltip_pzTextUpper = "Set cultist to have a chance of";
    itemMutatorBoolEnemyRandomTNT.tooltip_pzTextLower = "throwing random projectiles";
    itemMutatorWeaponsVer.tooltip_pzTextUpper = "Check readme.txt for full";
    itemMutatorWeaponsVer.tooltip_pzTextLower = "list of weapon changes";
    itemMutatorSectorBehavior.tooltip_pzTextUpper = "Improve room over room sector handling";
    itemMutatorSectorBehavior.tooltip_pzTextLower = "for hitscans (e.g: firing above water)";
    itemMutatorHitscanProjectiles.tooltip_pzTextUpper = "Set hitscan enemies to";
    itemMutatorHitscanProjectiles.tooltip_pzTextLower = "spawn bullet projectiles";
    itemMutatorRandomizerMode.tooltip_pzTextUpper = "Set the randomizer's mode";
    itemMutatorRandomizerSeed.tooltip_pzTextUpper = "Set the randomizer's seed";
    itemMutatorRandomizerSeed.tooltip_pzTextLower = "No seed = always use a random seed";
    /////////////////////

    menuOptionsGame.Add(&itemBloodQAV, false);
    itemOptionsGameCycleAutoAim.m_nFocus = gAutoAim;
    itemOptionsGameBoolAutoRun.at20 = !!gAutoRun;
    itemOptionsGameWeaponSwitch.m_nFocus = gWeaponSwitch % ARRAY_SSIZE(pzWeaponSwitchStrings);
    itemOptionsGameWeaponFastSwitch.at20 = !!gWeaponFastSwitch;
    itemOptionsGameAutosaveMode.m_nFocus = gAutosave % ARRAY_SSIZE(pzAutosaveModeStrings);
    itemOptionsGameLockSaving.at20 = !!gLockManualSaving;
    itemOptionsGameRestoreLastSave.at20 = !!gRestoreLastSave;
    itemOptionsGameBoolVanillaMode.m_nFocus = gVanilla % ARRAY_SSIZE(pzVanillaModeStrings);
    SetGameVanillaMode(gVanilla); // enable/disable menu items depending on current vanilla mode state

    ///////
    itemMutatorBoolQuadDamagePowerup.at20 = !!gQuadDamagePowerup;
    itemMutatorDamageInvul.m_nFocus = gDamageInvul % ARRAY_SSIZE(pzDamageInvulBehaviorStrings);
    itemMutatorExplosionBehavior.m_nFocus = gExplosionBehavior % ARRAY_SSIZE(pzExplosionBehaviorStrings);
    itemMutatorProjectileBehavior.m_nFocus = gProjectileBehavior % ARRAY_SSIZE(pzProjectileBehaviorStrings);
    itemMutatorNapalmFalloff.at20 = !!gNapalmFalloff;
    itemMutatorEnemyBehavior.m_nFocus = gEnemyBehavior % ARRAY_SSIZE(pzEnemyBehaviorStrings);
    itemMutatorBoolEnemyRandomTNT.at20 = !!gEnemyRandomTNT;
    itemMutatorWeaponsVer.m_nFocus = gWeaponsVer % ARRAY_SSIZE(pzWeaponsVersionStrings);
    itemMutatorSectorBehavior.at20 = !!gSectorBehavior;
    itemMutatorHitscanProjectiles.m_nFocus = gHitscanProjectiles % ARRAY_SSIZE(pzHitscanProjectilesStrings);
    itemMutatorRandomizerMode.m_nFocus = gRandomizerMode % ARRAY_SSIZE(pzRandomizerModeStrings);
    Bstrncpy(szRandomizerSeedMenu, gzRandomizerSeed, sizeof(szRandomizerSeedMenu));
    ///////

    menuOptionsDisplay.Add(&itemOptionsDisplayTitle, false);
    menuOptionsDisplay.Add(&itemOptionsDisplayColor, true);
    menuOptionsDisplay.Add(&itemOptionsDisplayMode, false);
    menuOptionsDisplay.Add(&itemOptionsDisplayView, false);
    menuOptionsDisplay.Add(&itemOptionsDisplaySliderDetail, false);
    menuOptionsDisplay.Add(&itemOptionsDisplayBoolVoxels, false);
    menuOptionsDisplay.Add(&itemOptionsDisplayCrosshair, false);
    menuOptionsDisplay.Add(&itemOptionsDisplayLevelStats, false);
    menuOptionsDisplay.Add(&itemOptionsDisplayLevelStatsOnAutomap, false);
    menuOptionsDisplay.Add(&itemOptionsDisplayPowerupDuration, false);
    menuOptionsDisplay.Add(&itemOptionsDisplayBoolShowMapTitle, false);
    menuOptionsDisplay.Add(&itemOptionsDisplayBoolMessages, false);
    menuOptionsDisplay.Add(&itemOptionsDisplayBoolWidescreen, false);
    menuOptionsDisplay.Add(&itemOptionsDisplayWeaponSelect, false);
    menuOptionsDisplay.Add(&itemOptionsDisplayFOV, false);
    menuOptionsDisplay.Add(&itemBloodQAV, false);
    itemOptionsDisplayBoolVoxels.at20 = usevoxels;
    itemOptionsDisplayCrosshair.m_nFocus = gAimReticle % ARRAY_SSIZE(pzCrosshairStrings);
    itemOptionsDisplayLevelStats.m_nFocus = gLevelStats % ARRAY_SSIZE(pzStatsPowerupRatioStrings);
    itemOptionsDisplayLevelStatsOnAutomap.at20 = gLevelStatsOnlyOnMap;
    itemOptionsDisplayLevelStatsOnAutomap.bEnable = !!gLevelStats;
    itemOptionsDisplayPowerupDuration.m_nFocus = gPowerupDuration % ARRAY_SSIZE(pzStatsPowerupRatioStrings);
    itemOptionsDisplayBoolShowMapTitle.at20 = gShowMapTitle;
    itemOptionsDisplayBoolMessages.at20 = gMessageState;
    itemOptionsDisplayBoolWidescreen.at20 = r_usenewaspect;
    itemOptionsDisplayWeaponSelect.m_nFocus = gShowWeaponSelect % ARRAY_SSIZE(pzWeaponSelectStrings);

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
    menuOptionsDisplayMode.Add(&itemOptionsDisplayModePolymost, false);
#endif
    menuOptionsDisplayMode.Add(&itemOptionsDisplayModeFullscreen, false);
#ifdef USE_OPENGL
    menuOptionsDisplayMode.Add(&itemOptionsDisplayModeVSync, false);
#endif
    menuOptionsDisplayMode.Add(&itemOptionsDisplayModeFrameLimit, false);
    menuOptionsDisplayMode.Add(&itemOptionsDisplayModeApply, false);
    menuOptionsDisplayMode.Add(&itemBloodQAV, false);

#ifdef USE_OPENGL
    itemOptionsDisplayModeRenderer.pPreDrawCallback = PreDrawVideoModeMenu;
    itemOptionsDisplayModePolymost.bEnable = videoGetRenderMode() == REND_POLYMOST;
#endif
    itemOptionsDisplayModeFullscreen.pPreDrawCallback = PreDrawVideoModeMenu;

    menuOptionsDisplayColor.Add(&itemOptionsDisplayColorTitle, false);
    menuOptionsDisplayColor.Add(&itemOptionsDisplayColorPaletteCustom, true);
    menuOptionsDisplayColor.Add(&itemOptionsDisplayColorPaletteCIEDE2000, false);
    menuOptionsDisplayColor.Add(&itemOptionsDisplayColorPaletteGrayscale, false);
    menuOptionsDisplayColor.Add(&itemOptionsDisplayColorPaletteInvert, false);
    menuOptionsDisplayColor.Add(&itemOptionsDisplayColorGamma, false);
    menuOptionsDisplayColor.Add(&itemOptionsDisplayColorContrast, false);
    menuOptionsDisplayColor.Add(&itemOptionsDisplayColorSaturation, false);
    menuOptionsDisplayColor.Add(&itemOptionsDisplayColorVisibility, false);
    menuOptionsDisplayColor.Add(&itemOptionsDisplayColorReset, false);
    menuOptionsDisplayColor.Add(&itemBloodQAV, false);

    itemOptionsDisplayColorPaletteCustom.m_nFocus = gCustomPalette % ARRAY_SSIZE(srcCustomPaletteStr);
    itemOptionsDisplayColorPaletteCIEDE2000.at20 = gCustomPaletteCIEDE2000;
    itemOptionsDisplayColorPaletteGrayscale.at20 = gCustomPaletteGrayscale;
    itemOptionsDisplayColorPaletteInvert.m_nFocus = gCustomPaletteInvert % ARRAY_SSIZE(pzInvertPaletteStrings);
    itemOptionsDisplayColorContrast.pPreDrawCallback = PreDrawDisplayColor;
    itemOptionsDisplayColorSaturation.pPreDrawCallback = PreDrawDisplayColor;

    menuOptionsDisplayView.Add(&itemOptionsDisplayViewTitle, false);
    menuOptionsDisplayView.Add(&itemOptionsDisplayViewHudSize, true);
    menuOptionsDisplayView.Add(&itemOptionsDisplayViewHudRatio, false);
    menuOptionsDisplayView.Add(&itemOptionsDisplayViewBoolCenterHoriz, false);
    menuOptionsDisplayView.Add(&itemOptionsDisplayViewBoolSlopeTilting, false);
    menuOptionsDisplayView.Add(&itemOptionsDisplayViewBoolViewBobbing, false);
    menuOptionsDisplayView.Add(&itemOptionsDisplayViewBoolViewSwaying, false);
    menuOptionsDisplayView.Add(&itemOptionsDisplayViewWeaponSwaying, false);
    menuOptionsDisplayView.Add(&itemOptionsDisplayViewWeaponInterpolation, false);
    menuOptionsDisplayView.Add(&itemOptionsDisplayViewBoolInterpolation, false);
    menuOptionsDisplayView.Add(&itemOptionsDisplayViewBoolLevelCompleteTime, false);
    menuOptionsDisplayView.Add(&itemOptionsDisplayViewBoolPowerupStyle, false);
    menuOptionsDisplayView.Add(&itemOptionsDisplayViewMirrorMode, false);
    menuOptionsDisplayView.Add(&itemOptionsDisplayViewBoolSlowRoomFlicker, false);
    menuOptionsDisplayView.Add(&itemBloodQAV, false);
    itemOptionsDisplayViewMirrorMode.bDisableForNet = 1;
    itemOptionsDisplayViewBoolSlopeTilting.tooltip_pzTextUpper = "";
    itemOptionsDisplayViewBoolSlopeTilting.tooltip_pzTextLower = "Tilt view when looking towards slope";
    itemOptionsDisplayViewWeaponInterpolation.tooltip_pzTextUpper = "";
    itemOptionsDisplayViewWeaponInterpolation.tooltip_pzTextLower = "Interpolate QAV weapon animations (experimental)";
    itemOptionsDisplayViewWeaponSwaying.tooltip_pzTextUpper = "";
    itemOptionsDisplayViewWeaponSwaying.tooltip_pzTextLower = "Set weapon sway (v1.0x alters tommy cultist behavior)";
    itemOptionsDisplayViewBoolInterpolation.tooltip_pzTextUpper = "";
    itemOptionsDisplayViewBoolInterpolation.tooltip_pzTextLower = "Set interpolation method (original/integer or modern/float)";
    itemOptionsDisplayViewBoolPowerupStyle.tooltip_pzTextUpper = "";
    itemOptionsDisplayViewBoolPowerupStyle.tooltip_pzTextLower = "Set the display style for power-ups";
    itemOptionsDisplayViewBoolSlowRoomFlicker.tooltip_pzTextUpper = "";
    itemOptionsDisplayViewBoolSlowRoomFlicker.tooltip_pzTextLower = "Reduce the speed of flickering sectors (e.g: E1M4)";

    itemOptionsDisplayViewHudRatio.m_nFocus = gHudRatio % ARRAY_SSIZE(pzHudRatioStrings);
    itemOptionsDisplayViewBoolCenterHoriz.at20 = gCenterHoriz;
    itemOptionsDisplayViewBoolSlopeTilting.at20 = gSlopeTilting;
    itemOptionsDisplayViewBoolViewBobbing.at20 = gViewVBobbing;
    itemOptionsDisplayViewBoolViewSwaying.at20 = gViewHBobbing;
    itemOptionsDisplayViewWeaponSwaying.m_nFocus = gWeaponHBobbing % ARRAY_SSIZE(pzWeaponHBobbingStrings);
    itemOptionsDisplayViewWeaponInterpolation.m_nFocus = gWeaponInterpolate % ARRAY_SSIZE(pzWeaponInterpolateStrings);
    itemOptionsDisplayViewBoolInterpolation.at20 = gViewInterpolateMethod;
    itemOptionsDisplayViewBoolLevelCompleteTime.at20 = gShowCompleteTime;
    itemOptionsDisplayViewBoolPowerupStyle.at20 = gPowerupStyle;
    itemOptionsDisplayViewMirrorMode.m_nFocus = r_mirrormode % ARRAY_SSIZE(pzMirrorModeStrings);
    itemOptionsDisplayViewBoolSlowRoomFlicker.at20 = gSlowRoomFlicker;

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
    menuOptionsDisplayPolymost.Add(&itemOptionsDisplayPolymostShadeInterpolation, false);
    menuOptionsDisplayPolymost.Add(&itemOptionsDisplayPolymostYShearing, false);
    menuOptionsDisplayPolymost.Add(&itemOptionsDisplayPolymostRollAngle, false);
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
#ifdef _WIN32
    if (WinMMDrv_MIDI_GetNumDevices() > 0) // do not add this item if only 1 midi device is detected
        menuOptionsSound.Add(&itemOptionsSoundMIDIDevice, false);
#endif
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
    menuOptionsPlayer.Add(&itemOptionsPlayerTeamPreference, false);
    menuOptionsPlayer.Add(&itemOptionsPlayerColorPreference, false);
    menuOptionsPlayer.Add(&itemOptionsPlayerModel, false);
    menuOptionsPlayer.Add(&itemOptionsPlayerBoolShowPlayerNames, false);
    menuOptionsPlayer.Add(&itemOptionsPlayerShowWeapons, false);
    menuOptionsPlayer.Add(&itemOptionsPlayerChatSound, false);
    menuOptionsPlayer.Add(&itemOptionsPlayerColorMsg, false);
    menuOptionsPlayer.Add(&itemOptionsPlayerObituaryMsg, false);
    menuOptionsPlayer.Add(&itemOptionsPlayerKillMsg, false);
    menuOptionsPlayer.Add(&itemOptionsPlayerMultiKill, false);
    menuOptionsPlayer.Add(&itemBloodQAV, false);
    itemOptionsPlayerSkill.tooltip_pzTextUpper = "Set player's damage scale handicap";
    itemOptionsPlayerSkill.tooltip_pzTextLower = "(higher = more damage)";
    itemOptionsPlayerTeamPreference.tooltip_pzTextUpper = "Set player's preferred team";
    itemOptionsPlayerColorPreference.tooltip_pzTextUpper = "Set player's preferred color";
    itemOptionsPlayerColorPreference.tooltip_pzTextLower = "(for bloodbath/co-op mode)";
    itemOptionsPlayerModel.tooltip_pzTextUpper = "Set player's sprite model";
    itemOptionsPlayerBoolShowPlayerNames.tooltip_pzTextUpper = "Display player's name";
    itemOptionsPlayerBoolShowPlayerNames.tooltip_pzTextLower = "over crosshair";
    itemOptionsPlayerShowWeapons.tooltip_pzTextUpper = "Display player's weapon";
    itemOptionsPlayerShowWeapons.tooltip_pzTextLower = "over their head";
    itemOptionsPlayerChatSound.tooltip_pzTextUpper = "Play beep sound for chat messages";
    itemOptionsPlayerColorMsg.tooltip_pzTextUpper = "Color player names for chat messages";
    itemOptionsPlayerObituaryMsg.tooltip_pzTextUpper = "Use random obituary kill messages";
    itemOptionsPlayerKillMsg.tooltip_pzTextUpper = "Show player killed on screen";
    itemOptionsPlayerKillMsg.tooltip_pzTextLower = "(for bloodbath/teams mode)";
    itemOptionsPlayerMultiKill.tooltip_pzTextUpper = "Show multi kill alerts on screen";
    itemOptionsPlayerMultiKill.tooltip_pzTextLower = "(for bloodbath/teams mode)";

    itemOptionsPlayerSkill.m_nFocus = gSkill % ARRAY_SSIZE(pzPlayerSkillStrings);
    itemOptionsPlayerTeamPreference.m_nFocus = gPlayerTeamPreference % ARRAY_SSIZE(pzPlayerTeamPreferenceStrings);
    itemOptionsPlayerColorPreference.m_nFocus = gPlayerColorPreference % ARRAY_SSIZE(pzPlayerColorPreferenceStrings);
    itemOptionsPlayerModel.at20 = gPlayerModel;
    itemOptionsPlayerBoolShowPlayerNames.at20 = gShowPlayerNames;
    itemOptionsPlayerShowWeapons.m_nFocus = gShowWeapon;
    itemOptionsPlayerChatSound.at20 = gChatSnd;
    itemOptionsPlayerColorMsg.at20 = gColorMsg;
    itemOptionsPlayerObituaryMsg.at20 = gKillObituary;
    itemOptionsPlayerKillMsg.at20 = gKillMsg;
    itemOptionsPlayerMultiKill.m_nFocus = gMultiKill % ARRAY_SSIZE(pzPlayerMultiKillStrings);
}

void SetupControlsMenu(void)
{
    menuOptionsControl.Add(&itemOptionsControlTitle, false);
    menuOptionsControl.Add(&itemOptionsControlKeyboard, true);
    menuOptionsControl.Add(&itemOptionsControlMouse, false);
    menuOptionsControl.Add(&itemOptionsControlJoystickButtons, false);
    menuOptionsControl.Add(&itemOptionsControlJoystickAxes, false);
    menuOptionsControl.Add(&itemOptionsControlJoystickMisc, false);
    menuOptionsControl.Add(&itemBloodQAV, false);

    menuOptionsControlKeyboard.Add(&itemOptionsControlKeyboardTitle, false);
    menuOptionsControlKeyboard.Add(&itemOptionsControlKeyboardSliderTurnSpeed, true);
    menuOptionsControlKeyboard.Add(&itemOptionsControlKeyboardCycleTurnAcceleration, false);
    menuOptionsControlKeyboard.Add(&itemOptionsControlKeyboardBoolCrouchToggle, false);
    menuOptionsControlKeyboard.Add(&itemOptionsControlKeyboardList, false);
    menuOptionsControlKeyboard.Add(&itemOptionsControlKeyboardReset, false);
    menuOptionsControlKeyboard.Add(&itemOptionsControlKeyboardResetClassic, false);
    menuOptionsControlKeyboard.Add(&itemBloodQAV, false);

    itemOptionsControlKeyboardSliderTurnSpeed.nValue = gTurnSpeed;
    itemOptionsControlKeyboardCycleTurnAcceleration.m_nFocus = gTurnAcceleration % ARRAY_SSIZE(pzTurnAccelerationStrings);
    itemOptionsControlKeyboardBoolCrouchToggle.at20 = gCrouchToggle;

    menuKeys.Add(&itemKeysTitle, false);
    menuKeys.Add(&itemKeyList, true);
    menuKeys.Add(&itemBloodQAV, false);

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
    for (int i = 0, y = 60; i < MENUMOUSEFUNCTIONS; i++)
    {
        pItemOptionsControlMouseButton[i] = new CGameMenuItemZCycle(MenuMouseNames[i], 3, 66, y, 180, 0, SetMouseButton, pzGamefuncsStrings, NUMGAMEFUNCTIONS+1, 0, true);
        dassert(pItemOptionsControlMouseButton[i] != NULL);
        menuOptionsControlMouseButtonAssignment.Add(pItemOptionsControlMouseButton[i], i == 0);
        y += 10;
    }
    menuOptionsControlMouseButtonAssignment.Add(&itemBloodQAV, false);
}

void SetupJoystickMenu(void)
{
    if (!CONTROL_JoystickEnabled) // joystick disabled, don't bother populating joystick menus
    {
        itemOptionsControlJoystickButtons.bEnable = 0;
        itemOptionsControlJoystickAxes.bEnable = 0;
        itemOptionsControlJoystickMisc.bEnable = 0;
        return;
    }

    menuOptionsControlJoystickMisc.Add(&itemOptionsControlJoystickMiscTitle, false);
    menuOptionsControlJoystickMisc.Add(&itemOptionsControlJoystickMiscCrouchToggle, true);
    menuOptionsControlJoystickMisc.Add(&itemOptionsControlJoystickMiscCenterView, false);
    menuOptionsControlJoystickMisc.Add(&itemOptionsControlJoystickMiscTargetAimAssist, false);
    menuOptionsControlJoystickMisc.Add(&itemOptionsControlJoystickMiscRumble, false);
    menuOptionsControlJoystickMisc.Add(&itemBloodQAV, false);
    itemOptionsControlJoystickMiscTargetAimAssist.tooltip_pzTextUpper = "Slows camera movement when";
    itemOptionsControlJoystickMiscTargetAimAssist.tooltip_pzTextLower = "crosshair is over an enemy";

    itemOptionsControlJoystickMiscCrouchToggle.at20 = gCrouchToggle;
    itemOptionsControlJoystickMiscCenterView.at20 = gCenterViewOnDrop;
    itemOptionsControlJoystickMiscTargetAimAssist.at20 = gTargetAimAssist;
    itemOptionsControlJoystickMiscRumble.at20 = gSetup.joystickrumble;

    int i = 0, y = 0;
    for (int nButton = 0; nButton < joystick.numButtons; nButton++) // store every joystick button/hat name for button list at launch
    {
        const char *pzButtonName = joyGetName(1, nButton);
        if (pzButtonName == NULL) // if we've ran out of button names, store joystick hat names
        {
            for (int nHats = 0; nHats < (joystick.numHats > 0) * 4; nHats++)
            {
                const char *pzHatName = joyGetName(2, nHats);
                if (pzHatName == NULL)
                    break;
                Bsnprintf(MenuJoyButtonNames[i++], 64, "%s", pzHatName);
                Bsnprintf(MenuJoyButtonNames[i++], 64, "Double %s", pzHatName);
            }
            break;
        }
        Bsnprintf(MenuJoyButtonNames[i++], 64, "%s", pzButtonName);
        Bsnprintf(MenuJoyButtonNames[i++], 64, "Double %s", pzButtonName);
    }
    const int nMaxJoyButtons = i;

    i = 0;
    for (int nAxis = 0; nAxis < joystick.numAxes; nAxis++) // store every joystick axes for axes list at launch
    {
        const char *pzAxisName = joyGetName(0, nAxis);
        if (pzAxisName == NULL) // if we've ran out of axes names, stop
            break;
        Bsnprintf(MenuJoyAxisNames[i++], 64, "%s", pzAxisName);
    }
    const int nMaxJoyAxes = i;

    if (nMaxJoyButtons <= 0) // joystick has no buttons, disable button menu
    {
        itemOptionsControlJoystickButtons.bEnable = 0;
    }
    else
    {
        i = 0;
        for (int nPage = 0; nPage < MAXJOYSTICKBUTTONPAGES; nPage++) // create lists of joystick button items
        {
            y = 35;
            menuOptionsControlJoystickButtonAssignment[nPage].Add(&itemJoyButtonsTitle, false);
            for (int nButton = 0; nButton < JOYSTICKITEMSPERPAGE; nButton++) // populate button list
            {
                pItemOptionsControlJoyButton[nPage][nButton] = new CGameMenuItemZCycle(MenuJoyButtonNames[i], 3, 66, y, 180, 0, SetJoyButton, pzGamefuncsStrings, NUMGAMEFUNCTIONS+1, 0, true);
                dassert(pItemOptionsControlJoyButton[nPage][nButton] != NULL);
                menuOptionsControlJoystickButtonAssignment[nPage].Add(pItemOptionsControlJoyButton[nPage][nButton], nButton == 0);
                y += 9;
                i++;
                if (i >= nMaxJoyButtons) // if we've reached the total number of buttons, stop populating list
                    break;
            }
            if (i < nMaxJoyButtons) // if we still have more buttons to list, add next page menu item at bottom of page
            {
                pItemOptionsControlJoyButtonNextPage[nPage] = new CGameMenuItemChain("NEXT PAGE", 3, 0, 182, 320, 1, &menuOptionsControlJoystickButtonAssignment[nPage+1], -1, NULL, 0);
                dassert(pItemOptionsControlJoyButtonNextPage[nPage] != NULL);
                menuOptionsControlJoystickButtonAssignment[nPage].Add(pItemOptionsControlJoyButtonNextPage[nPage], false);
            }
            menuOptionsControlJoystickButtonAssignment[nPage].Add(&itemBloodQAV, false);
        }
    }

    if (nMaxJoyAxes <= 0) // joystick has no axes, disable axis menu
    {
        itemOptionsControlJoystickAxes.bEnable = 0;
        return;
    }

    for (int nAxis = 0; nAxis < (int)ARRAY_SIZE(pItemOptionsControlJoystickAxis); nAxis++) // set all possible axis items to null (used for button setup)
        pItemOptionsControlJoystickAxis[nAxis] = NULL;

    y = 40;
    const char bUseBigFont = nMaxJoyAxes <= 6;
    menuOptionsControlJoystickListAxes.Add(&itemJoyAxesTitle, false);
    for (int nAxis = 0; nAxis < nMaxJoyAxes; nAxis++) // create list of axes for joystick axis menu
    {
        pItemOptionsControlJoystickAxis[nAxis] = new CGameMenuItemChain(MenuJoyAxisNames[nAxis], bUseBigFont ? 1 : 3, 66, y, 180, 0, &menuOptionsControlJoystickAxis[nAxis], -1, NULL, 0);
        dassert(pItemOptionsControlJoystickAxis[nAxis] != NULL);
        menuOptionsControlJoystickListAxes.Add(pItemOptionsControlJoystickAxis[nAxis], nAxis == 0);
        y += bUseBigFont ? 20 : 10;
    }
    menuOptionsControlJoystickListAxes.Add(&itemBloodQAV, false);

    for (int nAxis = 0; nAxis < nMaxJoyAxes; nAxis++) // create settings for each listed joystick axis
    {
        y = 40;
        menuOptionsControlJoystickAxis[nAxis].Add(&itemJoyAxesTitle, false);
        pItemOptionsControlJoystickAxisName[nAxis] = new CGameMenuItemTitle(MenuJoyAxisNames[nAxis], 3, 160, y, -1); // get axis name
        dassert(pItemOptionsControlJoystickAxisName[nAxis] != NULL);
        menuOptionsControlJoystickAxis[nAxis].Add(pItemOptionsControlJoystickAxisName[nAxis], false);
        y += 10;
        pItemOptionsControlJoystickAxisScale[nAxis] = new CGameMenuItemSlider("AXIS SCALE:", 1, 18, y, 280, &JoystickAnalogueScale[nAxis], fix16_from_int(0), fix16_from_float(2.f), fix16_from_float(0.025f), SetJoystickScale, -1, -1, kMenuSliderQ16); // get axis scale
        dassert(pItemOptionsControlJoystickAxisScale[nAxis] != NULL);
        menuOptionsControlJoystickAxis[nAxis].Add(pItemOptionsControlJoystickAxisScale[nAxis], true);
        y += 22;
        pItemOptionsControlJoystickAxisAnalogue[nAxis] = new CGameMenuItemZCycle("ANALOG:", 1, 18, y, 280, 0, SetJoystickAnalogue, zJoystickAnalogue, ARRAY_SSIZE(zJoystickAnalogue), 0); // get analog function
        dassert(pItemOptionsControlJoystickAxisAnalogue[nAxis] != NULL);
        menuOptionsControlJoystickAxis[nAxis].Add(pItemOptionsControlJoystickAxisAnalogue[nAxis], false);
        y += 17;
        pItemOptionsControlJoystickAxisAnalogueInvert[nAxis] = new CGameMenuItemZBool("ANALOG INVERT:", 1, 18, y, 280, false, SetJoystickAnalogueInvert, NULL, NULL); // get analog function
        dassert(pItemOptionsControlJoystickAxisAnalogueInvert[nAxis] != NULL);
        menuOptionsControlJoystickAxis[nAxis].Add(pItemOptionsControlJoystickAxisAnalogueInvert[nAxis], false);
        y += 17;
        pItemOptionsControlJoystickAxisDigitalPos[nAxis] = new CGameMenuItemZCycle("DIGITAL +:", 1, 18, y, 280, 0, SetJoystickDigitalPos, pzGamefuncsStrings, NUMGAMEFUNCTIONS+1, 0, true); // get digital function
        dassert(pItemOptionsControlJoystickAxisDigitalPos[nAxis] != NULL);
        menuOptionsControlJoystickAxis[nAxis].Add(pItemOptionsControlJoystickAxisDigitalPos[nAxis], false);
        y += 17;
        pItemOptionsControlJoystickAxisDigitalNeg[nAxis] = new CGameMenuItemZCycle("DIGITAL -:", 1, 18, y, 280, 0, SetJoystickDigitalNeg, pzGamefuncsStrings, NUMGAMEFUNCTIONS+1, 0, true); // get digital function
        dassert(pItemOptionsControlJoystickAxisDigitalNeg[nAxis] != NULL);
        menuOptionsControlJoystickAxis[nAxis].Add(pItemOptionsControlJoystickAxisDigitalNeg[nAxis], false);
        y += 22;
        pItemOptionsControlJoystickAxisDeadzone[nAxis] = new CGameMenuItemSlider("DEAD ZONE:", 1, 18, y, 280, &JoystickAnalogueDead[nAxis], fix16_from_int(0), fix16_from_float(0.5f), fix16_from_float(0.025f), SetJoystickDeadzone, -1, -1, kMenuSliderPercent); // get dead size
        dassert(pItemOptionsControlJoystickAxisDeadzone[nAxis] != NULL);
        menuOptionsControlJoystickAxis[nAxis].Add(pItemOptionsControlJoystickAxisDeadzone[nAxis], false);
        y += 17;
        pItemOptionsControlJoystickAxisSaturate[nAxis] = new CGameMenuItemSlider("SATURATE:", 1, 18, y, 280, &JoystickAnalogueSaturate[nAxis], fix16_from_int(0), fix16_from_float(0.5f), fix16_from_float(0.025f), SetJoystickSaturate, -1, -1, kMenuSliderPercent); // get saturate
        dassert(pItemOptionsControlJoystickAxisSaturate[nAxis] != NULL);
        menuOptionsControlJoystickAxis[nAxis].Add(pItemOptionsControlJoystickAxisSaturate[nAxis], false);
        y += 17;
        pItemOptionsControlJoystickAxisSoloDeadzone[nAxis] = new CGameMenuItemZBool("SINGLE AXIS DEAD ZONE:", 1, 18, y, 280, false, SetJoystickSoloDeadzone, NULL, NULL); // get isolated dead
        dassert(pItemOptionsControlJoystickAxisSoloDeadzone[nAxis] != NULL);
        menuOptionsControlJoystickAxis[nAxis].Add(pItemOptionsControlJoystickAxisSoloDeadzone[nAxis], false);
        menuOptionsControlJoystickAxis[nAxis].Add(&itemBloodQAV, false);
    }
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
    SetupMessagesMenu();
    SetupSaveGameMenu();
    SetupLoadGameMenu();
    SetupCreditsMenu();
    SetupHelpMenu();
    SetupDifficultyMenu();
    SetupEpisodeMenu();
    SetupMainMenu();
    SetupMainMenuWithSave();
    SetupFirstLaunchMenu();
    SetupNetStartMenu();
    SetupQuitMenu();

    SetupOptionsMenu();
    SetupControlsMenu();
    SetupJoystickMenu();
    SetupNetworkMenu();
}

void UpdateNetworkMenus(void)
{
    if (gGameOptions.nGameType != kGameTypeSinglePlayer)
    {
        itemMain1.at24 = &menuNetStart;
        itemMain1.at28 = 2;
    }
    else
    {
        itemMain1.at24 = &menuEpisode;
        itemMain1.at28 = -1;
    }
    if (gGameOptions.nGameType != kGameTypeSinglePlayer)
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

void SetVoxels(CGameMenuItemZBool *pItem)
{
    usevoxels = pItem->at20;
}

void SetCrosshair(CGameMenuItemZCycle *pItem)
{
    gAimReticle = pItem->m_nFocus % ARRAY_SSIZE(pzCrosshairStrings);
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
void SetQuadDamagePowerup(CGameMenuItemZBool* pItem)
{
    if ((gGameOptions.nGameType == kGameTypeSinglePlayer) && (numplayers == 1)) {
        gQuadDamagePowerup = pItem->at20;
        gGameOptions.bQuadDamagePowerup = gQuadDamagePowerup;
    } else {
        pItem->at20 = gQuadDamagePowerup;
    }
}

void SetDamageInvul(CGameMenuItemZCycle* pItem)
{
    if ((gGameOptions.nGameType == kGameTypeSinglePlayer) && (numplayers == 1)) {
        gDamageInvul = pItem->m_nFocus % ARRAY_SSIZE(pzDamageInvulBehaviorStrings);
        gGameOptions.nDamageInvul = gDamageInvul;
    } else {
        pItem->m_nFocus = gDamageInvul % ARRAY_SSIZE(pzDamageInvulBehaviorStrings);
    }
}

void SetExplosionBehavior(CGameMenuItemZCycle *pItem)
{
    if ((gGameOptions.nGameType == kGameTypeSinglePlayer) && (numplayers == 1)) {
        gExplosionBehavior = pItem->m_nFocus % ARRAY_SSIZE(pzExplosionBehaviorStrings);
        gGameOptions.nExplosionBehavior = gExplosionBehavior;
    } else {
        pItem->m_nFocus = gExplosionBehavior % ARRAY_SSIZE(pzExplosionBehaviorStrings);
    }
}

void SetProjectileBehavior(CGameMenuItemZCycle *pItem)
{
    if ((gGameOptions.nGameType == kGameTypeSinglePlayer) && (numplayers == 1)) {
        gProjectileBehavior = pItem->m_nFocus % ARRAY_SSIZE(pzProjectileBehaviorStrings);
        gGameOptions.nProjectileBehavior = gProjectileBehavior;
    } else {
        pItem->m_nFocus = gProjectileBehavior % ARRAY_SSIZE(pzProjectileBehaviorStrings);
    }
}

void SetNapalmFalloff(CGameMenuItemZBool *pItem)
{
    if ((gGameOptions.nGameType == kGameTypeSinglePlayer) && (numplayers == 1)) {
        gNapalmFalloff = pItem->at20;
        gGameOptions.bNapalmFalloff = gNapalmFalloff;
    } else {
        pItem->at20 = !!gNapalmFalloff;
    }
}

void SetEnemyBehavior(CGameMenuItemZCycle *pItem)
{
    if ((gGameOptions.nGameType == kGameTypeSinglePlayer) && (numplayers == 1)) {
        gEnemyBehavior = pItem->m_nFocus % ARRAY_SSIZE(pzEnemyBehaviorStrings);
        gGameOptions.nEnemyBehavior = gEnemyBehavior;
    } else {
        pItem->m_nFocus = gEnemyBehavior % ARRAY_SSIZE(pzEnemyBehaviorStrings);
    }
}

void SetEnemyRandomTNT(CGameMenuItemZBool* pItem)
{
    if ((gGameOptions.nGameType == kGameTypeSinglePlayer) && (numplayers == 1)) {
        gEnemyRandomTNT = pItem->at20;
        gGameOptions.bEnemyRandomTNT = gEnemyRandomTNT;
    } else {
        pItem->at20 = !!gEnemyRandomTNT;
    }
}

void SetWeaponsVer(CGameMenuItemZCycle* pItem)
{
    if ((gGameOptions.nGameType == kGameTypeSinglePlayer) && (numplayers == 1)) {
        gWeaponsVer = pItem->m_nFocus % ARRAY_SSIZE(pzWeaponsVersionStrings);
        gGameOptions.nWeaponsVer = gWeaponsVer;
    } else {
        pItem->m_nFocus = gWeaponsVer % ARRAY_SSIZE(pzWeaponsVersionStrings);
    }
}

void SetSectorBehavior(CGameMenuItemZBool *pItem)
{
    if ((gGameOptions.nGameType == kGameTypeSinglePlayer) && (numplayers == 1)) {
        gSectorBehavior = pItem->at20;
        gGameOptions.bSectorBehavior = gSectorBehavior;
    } else {
        pItem->at20 = !!gSectorBehavior;
    }
}

void SetHitscanProjectiles(CGameMenuItemZCycle *pItem)
{
    if ((gGameOptions.nGameType == kGameTypeSinglePlayer) && (numplayers == 1)) {
        gHitscanProjectiles = pItem->m_nFocus % ARRAY_SSIZE(pzHitscanProjectilesStrings);
        gGameOptions.nHitscanProjectiles = gHitscanProjectiles;
    } else {
        pItem->m_nFocus = gHitscanProjectiles % ARRAY_SSIZE(pzHitscanProjectilesStrings);
    }
}

void SetRandomizerMode(CGameMenuItemZCycle *pItem)
{
    if ((gGameOptions.nGameType == kGameTypeSinglePlayer) && (numplayers == 1)) {
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
    if ((gGameOptions.nGameType == kGameTypeSinglePlayer) && (numplayers == 1))
        Bstrncpy(gGameOptions.szRandomizerSeed, gzRandomizerSeed, sizeof(gGameOptions.szRandomizerSeed));
}
////

void SetHudSize(CGameMenuItemSlider *pItem)
{
    gViewSize = pItem->nValue;
    viewResizeView(gViewSize);
}

void SetHudRatio(CGameMenuItemZCycle *pItem)
{
    gHudRatio = pItem->m_nFocus % ARRAY_SSIZE(pzHudRatioStrings);
    viewResizeView(gViewSize);
}

void SetCenterHoriz(CGameMenuItemZBool *pItem)
{
    gCenterHoriz = pItem->at20;
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
        if ((numplayers > 1) || (gGameOptions.nGameType != kGameTypeSinglePlayer)) // if multiplayer session is active
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

void SetViewInterpolate(CGameMenuItemZBool *pItem)
{
    gViewInterpolateMethod = pItem->at20;
}

void SetLevelCompleteTime(CGameMenuItemZBool *pItem)
{
    gShowCompleteTime = pItem->at20;
}

void SetMirrorMode(CGameMenuItemZCycle *pItem)
{
    r_mirrormode = pItem->m_nFocus % ARRAY_SSIZE(pzMirrorModeStrings);
}

void SetSlowRoomFlicker(CGameMenuItemZBool *pItem)
{
    gSlowRoomFlicker = pItem->at20;
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

void SetTurnAcceleration(CGameMenuItemZCycle *pItem)
{
    gTurnAcceleration = pItem->m_nFocus % ARRAY_SSIZE(pzTurnAccelerationStrings);
}

void SetCenterView(CGameMenuItemZBool *pItem)
{
    gCenterViewOnDrop = pItem->at20;
}

void SetJoystickTargetAimAssist(CGameMenuItemZBool *pItem)
{
    gTargetAimAssist = pItem->at20;
}

void SetJoystickRumble(CGameMenuItemZBool *pItem)
{
    gSetup.joystickrumble = pItem->at20;
}

void SetCrouchToggle(CGameMenuItemZBool *pItem)
{
    gCrouchToggle = itemOptionsControlKeyboardBoolCrouchToggle.at20 = itemOptionsControlJoystickMiscCrouchToggle.at20 = pItem->at20;
}

void SetAutoAim(CGameMenuItemZCycle *pItem)
{
    gAutoAim = pItem->m_nFocus;
    if (!gDemo.bRecording && !gDemo.bPlaying)
    {
        if ((numplayers > 1) || (gGameOptions.nGameType != kGameTypeSinglePlayer)) // if multiplayer session is active
        {
            netBroadcastPlayerInfoUpdate(myconnectindex);
            return;
        }
        gProfile[myconnectindex].nAutoAim = gAutoAim;
        netBroadcastPlayerInfo(myconnectindex);
    }
}

void SetAutoRun(CGameMenuItemZBool *pItem)
{
    gAutoRun = pItem->at20;
}

void SetLevelStats(CGameMenuItemZCycle *pItem)
{
    gLevelStats = pItem->m_nFocus % ARRAY_SSIZE(pzStatsPowerupRatioStrings);
    itemOptionsDisplayLevelStatsOnAutomap.bEnable = !!gLevelStats;
    viewResizeView(gViewSize);
}

void SetLevelStatsOnAutomap(CGameMenuItemZBool *pItem)
{
    gLevelStatsOnlyOnMap = pItem->at20;
}

void SetPowerupDuration(CGameMenuItemZCycle* pItem)
{
    gPowerupDuration = pItem->m_nFocus % ARRAY_SSIZE(pzStatsPowerupRatioStrings);
    viewResizeView(gViewSize);
}

void SetPowerupStyle(CGameMenuItemZBool* pItem)
{
    gPowerupStyle = pItem->at20;
}

void SetShowMapTitle(CGameMenuItemZBool* pItem)
{
    gShowMapTitle = pItem->at20;
}

void SetWeaponSwitch(CGameMenuItemZCycle *pItem)
{
    gWeaponSwitch = pItem->m_nFocus % ARRAY_SSIZE(pzWeaponSwitchStrings);
    if (!gDemo.bRecording && !gDemo.bPlaying)
    {
        if ((numplayers > 1) || (gGameOptions.nGameType != kGameTypeSinglePlayer)) // if multiplayer session is active
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
        if ((numplayers > 1) || (gGameOptions.nGameType != kGameTypeSinglePlayer)) // if multiplayer session is active
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

    itemLoadGameAutosaveStart.bEnable = gAutosave != 0; // remove autosave slots if autosaves are disabled
    itemLoadGameAutosaveStart.bNoDraw = gAutosave == 0;
    itemLoadGameAutosaveKey.bEnable = gAutosave == 2;
    itemLoadGameAutosaveKey.bNoDraw = gAutosave != 2;
}

void SetLockSaving(CGameMenuItemZBool *pItem)
{
    gLockManualSaving = pItem->at20;

    itemMainSave3.bEnable = !gLockManualSaving; // hide save option in main menu if lock saving mode is set
}

void SetRestoreLastSave(CGameMenuItemZBool *pItem)
{
    gRestoreLastSave = pItem->at20;
}

void SetGameVanillaMode(char nState)
{
    gVanilla = nState % ARRAY_SSIZE(pzVanillaModeStrings);
    itemOptionsGameWeaponFastSwitch.bEnable = !gVanilla;
    itemOptionsGameBoolVanillaMode.m_nFocus = gVanilla % ARRAY_SSIZE(pzVanillaModeStrings);
    itemOptionsDisplayViewBoolInterpolation.bEnable = !gVanilla;
    itemOptionsDisplayWeaponSelect.bEnable = !gVanilla;
    itemOptionsChainMutators.bEnable = !gVanilla;
    VanillaModeUpdate();
}

void SetVanillaMode(CGameMenuItemZCycle *pItem)
{
    if ((gGameOptions.nGameType == kGameTypeSinglePlayer) && (numplayers == 1))
    {
        SetGameVanillaMode((char)pItem->m_nFocus);
        viewClearInterpolations();
        viewResizeView(gViewSize);
        gGameMessageMgr.Clear();
    }
    else
    {
        pItem->m_nFocus = gVanilla % ARRAY_SSIZE(pzVanillaModeStrings);
        viewSetMessage("Vanilla mode is disabled for multiplayer");
    }
}

inline unsigned int SetBannedSprites(char bSinglePlayer)
{
    unsigned int uSpriteBannedFlags = BANNED_NONE;

    if (bSinglePlayer)
    {
        // monsters
        if (itemBannedMonstersBats.at20)
            uSpriteBannedFlags |= BANNED_BATS;
        if (itemBannedMonstersRats.at20)
            uSpriteBannedFlags |= BANNED_RATS;
        if (itemBannedMonstersFish.at20)
            uSpriteBannedFlags |= BANNED_FISH;
        if (itemBannedMonstersHands.at20)
            uSpriteBannedFlags |= BANNED_HANDS;
        if (itemBannedMonstersGhosts.at20)
            uSpriteBannedFlags |= BANNED_GHOSTS;
        if (itemBannedMonstersSpiders.at20)
            uSpriteBannedFlags |= BANNED_SPIDERS;
        if (itemBannedMonstersTinyCaleb.at20)
            uSpriteBannedFlags |= BANNED_TCALEBS;
        if (itemBannedMonstersHellHounds.at20)
            uSpriteBannedFlags |= BANNED_HHOUNDS;

        // monster behavior tweaks
        if (itemBannedMonstersRatsAttack.at20)
            uSpriteBannedFlags |= BANNED_RATS_BITE;
        if (itemBannedMonstersMotherSpiderHealth.at20)
            uSpriteBannedFlags |= BANNED_MSPIDERS_HP;
        if (itemBannedMonstersTchernobogHealth.at20)
            uSpriteBannedFlags |= BANNED_TCHERNOBOG_HP;
    }
    else // load from network monster menu
    {
        // monsters
        if (itemNetMonsterBats.at20)
            uSpriteBannedFlags |= BANNED_BATS;
        if (itemNetMonsterRats.at20)
            uSpriteBannedFlags |= BANNED_RATS;
        if (itemNetMonsterFish.at20)
            uSpriteBannedFlags |= BANNED_FISH;
        if (itemNetMonsterHands.at20)
            uSpriteBannedFlags |= BANNED_HANDS;
        if (itemNetMonsterGhosts.at20)
            uSpriteBannedFlags |= BANNED_GHOSTS;
        if (itemNetMonsterSpiders.at20)
            uSpriteBannedFlags |= BANNED_SPIDERS;
        if (itemNetMonsterTinyCaleb.at20)
            uSpriteBannedFlags |= BANNED_TCALEBS;
        if (itemNetMonsterHellHounds.at20)
            uSpriteBannedFlags |= BANNED_HHOUNDS;

        // monster behavior tweaks
        if (itemNetMonsterRatsAttack.at20)
            uSpriteBannedFlags |= BANNED_RATS_BITE;
        if (itemNetMonsterMotherSpiderHealth.at20)
            uSpriteBannedFlags |= BANNED_MSPIDERS_HP;
        if (itemNetMonsterTchernobogHealth.at20)
            uSpriteBannedFlags |= BANNED_TCHERNOBOG_HP;
    }

    // weapons
    if (itemBannedItemsFlare.at20)
        uSpriteBannedFlags |= BANNED_FLARE;
    if (itemBannedItemsShotgun.at20)
        uSpriteBannedFlags |= BANNED_SHOTGUN;
    if (itemBannedItemsTommyGun.at20)
        uSpriteBannedFlags |= BANNED_TOMMYGUN;
    if (itemBannedItemsNapalm.at20)
        uSpriteBannedFlags |= BANNED_NAPALM;
    if (itemBannedItemsTNT.at20)
        uSpriteBannedFlags |= BANNED_TNT;
    if (itemBannedItemsSpray.at20)
        uSpriteBannedFlags |= BANNED_SPRAYCAN;
    if (itemBannedItemsTesla.at20)
        uSpriteBannedFlags |= BANNED_TESLA;
    if (itemBannedItemsLifeLeech.at20)
        uSpriteBannedFlags |= BANNED_LIFELEECH;
    if (itemBannedItemsVoodoo.at20)
        uSpriteBannedFlags |= BANNED_VOODOO;
    if (itemBannedItemsProxy.at20)
        uSpriteBannedFlags |= BANNED_PROXY;
    if (itemBannedItemsRemote.at20)
        uSpriteBannedFlags |= BANNED_REMOTE;

    // items
    if (itemBannedItemsMedKit.at20)
        uSpriteBannedFlags |= BANNED_MEDKIT;
    if (itemBannedItemsLifeEssence.at20)
        uSpriteBannedFlags |= BANNED_LIFEESSENCE;
    if (itemBannedItemsLifeSeed.at20)
        uSpriteBannedFlags |= BANNED_LIFESEED;
    if (itemBannedItemsSuperArmor.at20)
        uSpriteBannedFlags |= BANNED_SUPERARMOR;
    if (itemBannedItemsCrystalBall.at20)
        uSpriteBannedFlags |= BANNED_CRYSTALBALL;

    // powerups
    if (itemBannedItemsJumpBoots.at20)
        uSpriteBannedFlags |= BANNED_JUMPBOOTS;
    if (itemBannedItemsCloak.at20)
        uSpriteBannedFlags |= BANNED_CLOAK;
    if (itemBannedItemsDeathMask.at20)
        uSpriteBannedFlags |= BANNED_DEATHMASK;
    if (itemBannedItemsAkimbo.at20)
        uSpriteBannedFlags |= BANNED_AKIMBO;
    if (itemBannedItemsReflect.at20)
        uSpriteBannedFlags |= BANNED_REFLECT;

    return uSpriteBannedFlags;
}

short gQuickLoadSlot = kLoadSaveNull;

void ShowDifficulties()
{
    gGameMenuMgr.Push(&menuDifficulty, 3);
}

void SetMonsters(CGameMenuItemZCycle *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
    const char bEnable = itemCustomDifficultyMonsterSettings.m_nFocus > 0;
    itemCustomDifficultyEnemyDifficulty.bEnable = bEnable;
    itemCustomDifficultyEnemyQuantity.bEnable = bEnable;
    itemCustomDifficultyEnemyHealth.bEnable = bEnable;
    itemCustomDifficultyEnemySpeed.bEnable = bEnable;
    itemCustomDifficultyEnemyShuffle.bEnable = bEnable;
    itemCustomDifficultyBannedMonsters.bEnable = bEnable;
}

void SetDifficultyAndStart(CGameMenuItemChain *pItem)
{
    gGameOptions.nDifficulty = pItem->at30;
    gGameOptions.nMonsterSettings = 1;
    gGameOptions.nMonsterRespawnTime = 3600; // default (30 secs)
    gGameOptions.nEnemyQuantity = gGameOptions.nDifficulty;
    gGameOptions.nEnemyHealth = gGameOptions.nDifficulty;
    playerSetSkill(gGameOptions.nDifficulty); // set skill to same value as current difficulty
    gGameOptions.nEnemySpeed = 0;
    gGameOptions.bEnemyShuffle = false;
    gGameOptions.bPitchforkOnly = false;
    gGameOptions.bPermaDeath = false;
    gGameOptions.uSpriteBannedFlags = BANNED_NONE;
    gGameOptions.nLevel = 0;
    if (!gVanilla) // don't reset gameflags for vanilla mode
        gGameOptions.uGameFlags = kGameFlagNone;
    gGameOptions.uNetGameFlags = kNetGameFlagNone;
    if (gDemo.bPlaying)
        gDemo.StopPlayback();
    else if (gDemo.bRecording)
        gDemo.Close();
    gStartNewGame = true;
    gAutosaveInCurLevel = false;
    gQuickLoadSlot = kLoadSaveNull;
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
    gGameOptions.nMonsterSettings = ClipRange(itemCustomDifficultyMonsterSettings.m_nFocus, 0, ARRAY_SSIZE(zMonsterStrings)-1);
    if (gGameOptions.nMonsterSettings <= 1)
        gGameOptions.nMonsterRespawnTime = 3600; // default (30 secs)
    else if (gGameOptions.nMonsterSettings == 2)
        gGameOptions.nMonsterRespawnTime = 15 * kTicRate; // 15 secs
    else
        gGameOptions.nMonsterRespawnTime = (gGameOptions.nMonsterSettings - 2) * 30 * kTicRate, gGameOptions.nMonsterSettings = ClipRange(gGameOptions.nMonsterSettings, 0, 2);
    gGameOptions.nEnemyQuantity = ClipRange(itemCustomDifficultyEnemyQuantity.nValue, 0, 4);
    gGameOptions.nEnemyHealth = ClipRange(itemCustomDifficultyEnemyHealth.nValue, 0, 4);
    playerSetSkill(itemCustomDifficultyPlayerDamage.nValue);
    gGameOptions.nEnemySpeed = ClipRange(itemCustomDifficultyEnemySpeed.m_nFocus, 0, 4);
    gGameOptions.bEnemyShuffle = !!itemCustomDifficultyEnemyShuffle.at20;
    gGameOptions.bPitchforkOnly = !!itemCustomDifficultyPitchfork.at20;
    gGameOptions.bPermaDeath = !!itemCustomDifficultyPermaDeath.at20;
    gGameOptions.uSpriteBannedFlags = SetBannedSprites(1);
    gGameOptions.nLevel = 0;
    if (!gVanilla) // don't reset gameflags for vanilla mode
        gGameOptions.uGameFlags = kGameFlagNone;
    gGameOptions.uNetGameFlags = kNetGameFlagNone;
    if (gDemo.bPlaying)
        gDemo.StopPlayback();
    else if (gDemo.bRecording)
        gDemo.Close();
    gStartNewGame = true;
    gAutosaveInCurLevel = false;
    gQuickLoadSlot = kLoadSaveNull;
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

void SetFirstLaunchOptions(CGameMenuItemChain *pItem)
{
    gSetup.firstlaunch = 0;
    if (pItem == &itemFirstLaunchNotBlood)
    {
        gGameMenuMgr.Pop(); // close menu
        return;
    }
    if (pItem == &itemFirstLaunchVanilla)
    {
        gSlopeTilting = 1;
        gAimReticle = 0;
        gAutoAim = 1;
        gAutoRun = 0;
        gShowWeapon = 0;
        gWeaponSwitch = 1;
        SetGameVanillaMode(1);
        gAutosave = 0;
        gRestoreLastSave = 0;
        gPowerupDuration = 0;
        gPowerupStyle = 0;
        gShowCompleteTime = 0;
        gHudRatio = 1;
        gViewSize = 5;
        viewResizeView(gViewSize);
        gShowMapTitle = 0;
        gWeaponInterpolate = 0;
        gCenterHoriz = 0;
    }
    else if (pItem == &itemFirstLaunchNBlood)
    {
        gAimReticle = 0;
        gAutoAim = 1;
        gAutoRun = 0;
        gShowWeapon = 0;
        gWeaponSwitch = 1;
        gAutosave = 0;
        gRestoreLastSave = 0;
        gPowerupStyle = 0;
        gShowCompleteTime = 0;
        gHudRatio = 0;
        gViewSize = 2;
        viewResizeView(gViewSize);
        gCenterHoriz = 0;
    }
    itemOptionsDisplayViewBoolSlopeTilting.at20 = gSlopeTilting;
    itemOptionsDisplayCrosshair.m_nFocus = gAimReticle;
    itemOptionsGameCycleAutoAim.m_nFocus = gAutoAim;
    itemOptionsGameBoolAutoRun.at20 = !!gAutoRun;
    itemOptionsGameWeaponSwitch.m_nFocus = gWeaponSwitch;
    itemOptionsGameAutosaveMode.m_nFocus = gAutosave;
    itemOptionsGameRestoreLastSave.at20 = gRestoreLastSave;
    itemOptionsDisplayPowerupDuration.m_nFocus = gPowerupDuration;
    itemOptionsDisplayViewBoolPowerupStyle.at20 = gPowerupStyle;
    itemOptionsDisplayViewBoolLevelCompleteTime.at20 = gShowCompleteTime;
    itemOptionsDisplayViewHudRatio.m_nFocus = gHudRatio;
    itemOptionsDisplayViewHudSize.nValue = gViewSize;
    itemOptionsDisplayBoolShowMapTitle.at20 = gShowMapTitle;
    itemOptionsDisplayViewWeaponInterpolation.m_nFocus = gWeaponInterpolate;
    itemOptionsDisplayViewBoolCenterHoriz.at20 = gCenterHoriz;
    gGameMenuMgr.Pop(); // close menu
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

#ifdef USE_OPENGL
    itemOptionsDisplayModePolymost.bEnable = videoGetRenderMode() == REND_POLYMOST;
#endif
}

void SetWidescreen(CGameMenuItemZBool *pItem)
{
    r_usenewaspect = pItem->at20;
}

void SetWeaponSelectMode(CGameMenuItemZCycle *pItem)
{
    gShowWeaponSelect = pItem->m_nFocus % ARRAY_SSIZE(pzRandomizerModeStrings);
}

void SetDetail(CGameMenuItemSlider *pItem)
{
    gDetail = pItem->nValue;
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
    for (int i = 0; i < itemOptionsDisplayModeVSync.m_nItems; i++)
    {
        if (vsync == nVSyncValues[i])
        {
            itemOptionsDisplayModeVSync.m_nFocus = i;
            break;
        }
    }
    const int kMaxFps = r_maxfps == -1 ? refreshfreq : r_maxfps;
    for (int i = 0; i < 8; i++)
    {
        if (kMaxFps == nFrameLimitValues[i])
        {
            itemOptionsDisplayModeFrameLimit.m_nFocus = i;
            break;
        }
    }
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

void UpdateVideoColorMenu(CGameMenuItemSliderFloat *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
    g_videoGamma = itemOptionsDisplayColorGamma.fValue;
    g_videoContrast = itemOptionsDisplayColorContrast.fValue;
    g_videoSaturation = itemOptionsDisplayColorSaturation.fValue;
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
    UNREFERENCED_PARAMETER(pItem);
    gCustomPalette = itemOptionsDisplayColorPaletteCustom.m_nFocus % ARRAY_SSIZE(srcCustomPaletteStr);
    gCustomPaletteInvert = itemOptionsDisplayColorPaletteInvert.m_nFocus % ARRAY_SSIZE(pzInvertPaletteStrings);
    UpdateVideoPalette();
}

void UpdateVideoPaletteBoolMenu(CGameMenuItemZBool *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
    gCustomPaletteCIEDE2000 = itemOptionsDisplayColorPaletteCIEDE2000.at20;
    gCustomPaletteGrayscale = itemOptionsDisplayColorPaletteGrayscale.at20;
    UpdateVideoPalette();
}

void PreDrawDisplayColor(CGameMenuItem *pItem)
{
    if (pItem == &itemOptionsDisplayColorContrast)
        pItem->bEnable = gammabrightness;
    else if (pItem == &itemOptionsDisplayColorSaturation)
        pItem->bEnable = gammabrightness;
}

void ResetVideoColor(CGameMenuItemChain *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
    g_videoGamma = DEFAULT_GAMMA;
    g_videoContrast = DEFAULT_CONTRAST;
    g_videoSaturation = DEFAULT_SATURATION;
    gBrightness = 0;
    r_ambientlight = r_ambientlightrecip = 1.f;
    gCustomPalette = itemOptionsDisplayColorPaletteCustom.m_nFocus = 0;
    gCustomPaletteCIEDE2000 = itemOptionsDisplayColorPaletteCIEDE2000.at20 = 0;
    gCustomPaletteGrayscale = itemOptionsDisplayColorPaletteGrayscale.at20 = 0;
    gCustomPaletteInvert = itemOptionsDisplayColorPaletteInvert.m_nFocus = 0;
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
    itemOptionsDisplayPolymostShadeInterpolation.at20 = r_shadeinterpolate;
    itemOptionsDisplayPolymostShadeInterpolation.bEnable = !!r_useindexedcolortextures;
    itemOptionsDisplayPolymostYShearing.at20 = r_yshearing;
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
    itemOptionsDisplayPolymostShadeInterpolation.bEnable = pItem->at20;
}

void UpdateShadeInterpolation(CGameMenuItemZBool *pItem)
{
    r_shadeinterpolate = pItem->at20;
}

void UpdateYShrearing(CGameMenuItemZBool *pItem)
{
    r_yshearing = pItem->at20;
}

void UpdateRollAngle(CGameMenuItemSlider *pItem)
{
    gRollAngle = pItem->nValue;
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

void SetNetGameMode(CGameMenuItemZCycle *pItem)
{
    if ((itemNetGameMode.m_nFocus+1) == kGameTypeCoop) // hide multiplayer game length settings
    {
        itemNetGameCycleLength.bEnable = 0;
        itemNetGameCycleLength.bNoDraw = !itemNetGameCycleLength.bEnable;
        itemNetGameCycleLengthCoop.bEnable = 1;
        itemNetGameCycleLengthCoop.bNoDraw = !itemNetGameCycleLengthCoop.bEnable;
        itemNetGameSliderLimit.bEnable = itemNetGameCycleLengthCoop.m_nFocus != 0; // don't show limit option if set to unlimited
        itemNetGameSliderLimit.bNoDraw = !itemNetGameSliderLimit.bEnable;
    }
    else // hide co-op game length settings
    {
        itemNetGameCycleLength.bEnable = 1;
        itemNetGameCycleLength.bNoDraw = !itemNetGameCycleLength.bEnable;
        itemNetGameCycleLengthCoop.bEnable = 0;
        itemNetGameCycleLengthCoop.bNoDraw = !itemNetGameCycleLengthCoop.bEnable;
        itemNetGameSliderLimit.bEnable = itemNetGameCycleLength.m_nFocus != 0; // don't show limit option if set to unlimited
        itemNetGameSliderLimit.bNoDraw = !itemNetGameSliderLimit.bEnable;
    }

    if (pItem == &itemNetGameMode)
    {
        itemNetStart1.m_pzText2 = zNetGameTypes[pItem->m_nFocus];
        itemNetGameBoolExit.bEnable = ((itemNetGameMode.m_nFocus+1) != kGameTypeCoop);
        itemNetGameBoolExit.bNoDraw = !itemNetGameBoolExit.bEnable;
        itemNetGameBoolFriendlyFire.bEnable = (pItem->m_nFocus+1) != kGameTypeBloodBath;
        itemNetGameBoolFriendlyFire.bNoDraw = !itemNetGameBoolFriendlyFire.bEnable;
        itemNetGameCycleKey.bEnable = (pItem->m_nFocus+1) == kGameTypeCoop;
        itemNetGameCycleKey.bNoDraw = !itemNetGameCycleKey.bEnable;
        itemNetGameCycleItemWeapon.bEnable = (pItem->m_nFocus+1) == kGameTypeCoop;
        itemNetGameCycleItemWeapon.bNoDraw = !itemNetGameCycleItemWeapon.bEnable;
        itemNetGameBoolAutoTeams.bEnable = (pItem->m_nFocus+1) == kGameTypeTeams;
        itemNetGameBoolAutoTeams.bNoDraw = !itemNetGameBoolAutoTeams.bEnable;
        itemNetGameBoolTeamColors.bEnable = (pItem->m_nFocus+1) == kGameTypeTeams;
        itemNetGameBoolTeamColors.bNoDraw = !itemNetGameBoolTeamColors.bEnable;
        itemNetGameBoolTeamFlags.bEnable = (pItem->m_nFocus+1) == kGameTypeTeams;
        itemNetGameBoolTeamFlags.bNoDraw = !itemNetGameBoolTeamFlags.bEnable;
        itemNetGameCycleSpawnLocation.bEnable = (pItem->m_nFocus+1) != kGameTypeCoop;
        itemNetGameCycleSpawnLocation.bNoDraw = !itemNetGameCycleSpawnLocation.bEnable;
        return;
    }

    if ((pItem == &itemNetGameCycleItemWeapon) && (pItem->m_nFocus > 0)) // if adjusted keep weapon/item settings, set spawn weapon to pitchfork
        itemNetGameCycleSpawnWeapon.m_nFocus = 0;
    else if ((pItem == &itemNetGameCycleSpawnWeapon) && (pItem->m_nFocus > 0)) // if adjusted spawn with weapon, turn off keep weapon/item settings as it will overwrite any weapon picked up after spawning
        itemNetGameCycleItemWeapon.m_nFocus = 0;
}

void SetNetMonsterMenu(CGameMenuItemZCycle *pItem)
{
    itemNetStart5.m_pzText2 = zMonsterStrings[itemNetMonsterSettings.m_nFocus];

    if ((pItem == &itemNetStart4) || !pItem)
    {
        itemNetMonsterQuantity.nValue = itemNetStart4.m_nFocus;
        itemNetMonsterHealth.nValue = itemNetStart4.m_nFocus;
    }

    for (int i = 0; i < menuNetworkGameMonsters.m_nItems; i++)
    {
        if (menuNetworkGameMonsters.pItemList[i] == &itemNetMonsterTitle)
            continue;
        if (menuNetworkGameMonsters.pItemList[i] == &itemNetMonsterSettings)
            continue;
        if (menuNetworkGameMonsters.pItemList[i] == &itemBloodQAV)
            continue;
        menuNetworkGameMonsters.pItemList[i]->bEnable = itemNetMonsterSettings.m_nFocus != 0;
    }
}

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
#ifdef _WIN32
    itemOptionsSoundMIDIDevice.bEnable = 0;
    itemOptionsSoundMIDIDevice.bNoDraw = 1;
#endif
    switch (nMusicDeviceValues[itemOptionsSoundMusicDevice.m_nFocus])
    {
    case ASS_SF2:
        itemOptionsSoundSF2Bank.bEnable = 1;
        itemOptionsSoundSF2Bank.bNoDraw = 0;
        break;
#ifdef _WIN32
    case ASS_WinMM:
        itemOptionsSoundMIDIDevice.bEnable = 1;
        itemOptionsSoundMIDIDevice.bNoDraw = 0;
        break;
#endif
    }
}

#ifdef _WIN32
void UpdateMidiDevice(CGameMenuItemSlider *pItem)
{
    WinMM_DeviceID = (unsigned int)pItem->nValue-1;
    MIDI_Restart();
}
#endif

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
#ifdef _WIN32
    itemOptionsSoundMIDIDevice.nValue = int(WinMM_DeviceID+1);
    itemOptionsSoundMIDIDevice.nRangeHigh = WinMMDrv_MIDI_GetNumDevices();
#endif

    UpdateMusicDevice(NULL);
}

void SetupPollJoystick(CGameMenuItemChain *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
    if (CONTROL_JoyPresent && CONTROL_JoystickEnabled && !itemOptionsControlJoystickButtons.bEnable) // if joysticks menu was never initialized, and a controller is now detected, setup the menu
    {
        itemOptionsControlJoystickButtons.bEnable = 1;
        itemOptionsControlJoystickAxes.bEnable = 1;
        itemOptionsControlJoystickMisc.bEnable = 1;
        SetupJoystickMenu();
    }
}

void UpdatePlayerName(CGameMenuItemZEdit *pItem, CGameMenuEvent *pEvent)
{
    UNREFERENCED_PARAMETER(pItem);
    if (pEvent->at0 == kMenuEventEnter)
        netBroadcastPlayerInfo(myconnectindex);
}

void UpdatePlayerSkill(CGameMenuItemZCycle *pItem)
{
    gSkill = pItem->m_nFocus % ARRAY_SIZE(pzPlayerSkillStrings);
    if ((numplayers > 1) || (gGameOptions.nGameType != kGameTypeSinglePlayer)) // if multiplayer session is active
        netBroadcastPlayerInfoUpdate(myconnectindex);
}

void UpdatePlayerTeamPreference(CGameMenuItemZCycle *pItem)
{
    gPlayerTeamPreference = pItem->m_nFocus % ARRAY_SIZE(pzPlayerTeamPreferenceStrings);
    if ((numplayers > 1) || (gGameOptions.nGameType != kGameTypeSinglePlayer)) // if multiplayer session is active
    {
        netBroadcastPlayerInfoUpdate(myconnectindex);
        if (gGameOptions.bAutoTeams && (gGameOptions.nGameType == kGameTypeTeams))
        {
            viewSetMessage("Auto teams is on, cannot change team.", 8, MESSAGE_PRIORITY_INI); // 8: gold
            gNetNotifyProfileUpdate = false;
        }
    }
}

void UpdatePlayerColorPreference(CGameMenuItemZCycle *pItem)
{
    gPlayerColorPreference = pItem->m_nFocus % ARRAY_SIZE(pzPlayerColorPreferenceStrings);
    if ((numplayers > 1) || (gGameOptions.nGameType != kGameTypeSinglePlayer)) // if multiplayer session is active
        netBroadcastPlayerInfoUpdate(myconnectindex);
}

void UpdatePlayerModel(CGameMenuItemZBool *pItem)
{
    gPlayerModel = pItem->at20;
    if ((numplayers > 1) || (gGameOptions.nGameType != kGameTypeSinglePlayer)) // if multiplayer session is active
        netBroadcastPlayerInfoUpdate(myconnectindex);
}

void SetShowPlayerNames(CGameMenuItemZBool *pItem)
{
    gShowPlayerNames = pItem->at20;
}

void SetShowWeapons(CGameMenuItemZCycle *pItem)
{
    gShowWeapon = pItem->m_nFocus;
}

void UpdatePlayerChatMessageSound(CGameMenuItemZBool *pItem)
{
    gChatSnd = pItem->at20;
}

void UpdatePlayerColorMessages(CGameMenuItemZBool *pItem)
{
    gColorMsg = pItem->at20;
}

void UpdatePlayerKillObituaryMessages(CGameMenuItemZBool *pItem)
{
    gKillObituary = pItem->at20;
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

void SetupJoystickButtonsMenu(CGameMenuItemChain *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
    const int nMaxJoyButtons = (joystick.numButtons * 2) + ((joystick.numHats > 0) * 4);
    for (int nPage = 0; nPage < MAXJOYSTICKBUTTONPAGES; nPage++) // go through each axis and setup binds
    {
        for (int nButton = 0; nButton < JOYSTICKITEMSPERPAGE; nButton++)
        {
            if (nButton >= nMaxJoyButtons) // reached end of button list
                return;
            const char bDoubleTap = nButton & 1;
            const int nJoyButton = ((nPage * JOYSTICKITEMSPERPAGE)>>1) + (nButton>>1); // we halve the button index because button lists are listed in pairs of single tap/double tap inputs
            auto pButton = pItemOptionsControlJoyButton[nPage][nButton];
            if (!pButton)
                break;
            pButton->m_nFocus = 0;
            for (int j = 0; j < NUMGAMEFUNCTIONS+1; j++)
            {
                if (JoystickFunctions[nJoyButton][bDoubleTap] == nGamefuncsValues[j])
                {
                    pButton->m_nFocus = j;
                    break;
                }
            }
        }
    }
}

void SetupJoystickAxesMenu(CGameMenuItemChain *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
    for (int nAxis = 0; nAxis < MAXJOYAXES; nAxis++) // set settings for each listed joystick axis
    {
        if (pItemOptionsControlJoystickAxis[nAxis] == NULL) // reached end of list, stop
            return;
        pItemOptionsControlJoystickAxisScale[nAxis]->nValue = JoystickAnalogueScale[nAxis];
        switch (JoystickAnalogueAxes[nAxis])
        {
        case analog_lookingupanddown:
            pItemOptionsControlJoystickAxisAnalogue[nAxis]->m_nFocus = 4;
            break;
        case analog_moving:
            pItemOptionsControlJoystickAxisAnalogue[nAxis]->m_nFocus = 3;
            break;
        case analog_strafing:
            pItemOptionsControlJoystickAxisAnalogue[nAxis]->m_nFocus = 2;
            break;
        case analog_turning:
            pItemOptionsControlJoystickAxisAnalogue[nAxis]->m_nFocus = 1;
            break;
        default: // unsupported/none
            pItemOptionsControlJoystickAxisAnalogue[nAxis]->m_nFocus = 0;
            break;
        }
        pItemOptionsControlJoystickAxisAnalogueInvert[nAxis]->at20 = JoystickAnalogueInvert[nAxis];
        pItemOptionsControlJoystickAxisDigitalPos[nAxis]->m_nFocus = 0;
        for (int j = 0; j < NUMGAMEFUNCTIONS+1; j++)
        {
            if (JoystickDigitalFunctions[nAxis][0] == nGamefuncsValues[j])
            {
                pItemOptionsControlJoystickAxisDigitalPos[nAxis]->m_nFocus = j;
                break;
            }
        }
        pItemOptionsControlJoystickAxisDigitalNeg[nAxis]->m_nFocus = 0;
        for (int j = 0; j < NUMGAMEFUNCTIONS+1; j++)
        {
            if (JoystickDigitalFunctions[nAxis][1] == nGamefuncsValues[j])
            {
                pItemOptionsControlJoystickAxisDigitalNeg[nAxis]->m_nFocus = j;
                break;
            }
        }
        pItemOptionsControlJoystickAxisDeadzone[nAxis]->nValue = JoystickAnalogueDead[nAxis];
        pItemOptionsControlJoystickAxisSaturate[nAxis]->nValue = JoystickAnalogueSaturate[nAxis];
        pItemOptionsControlJoystickAxisSoloDeadzone[nAxis]->at20 = JoystickAnalogueAxisSoloDeadZone[nAxis];
    }
}

void SetJoystickScale(CGameMenuItemSlider* pItem)
{
    for (int nAxis = 0; nAxis < MAXJOYAXES; nAxis++)
    {
        if (pItem == pItemOptionsControlJoystickAxisScale[nAxis])
        {
            JoystickAnalogueScale[nAxis] = pItem->nValue;
            CONTROL_SetAnalogAxisScale(nAxis, JoystickAnalogueScale[nAxis], controldevice_joystick);
            break;
        }
    }
}

void SetJoystickAnalogue(CGameMenuItemZCycle* pItem)
{
    for (int nAxis = 0; nAxis < MAXJOYAXES; nAxis++)
    {
        if (pItem == pItemOptionsControlJoystickAxisAnalogue[nAxis])
        {
            switch (pItem->m_nFocus)
            {
            case 4: // looking up/down
                JoystickAnalogueAxes[nAxis] = analog_lookingupanddown;
                break;
            case 3: // moving
                JoystickAnalogueAxes[nAxis] = analog_moving;
                break;
            case 2: // strafing
                JoystickAnalogueAxes[nAxis] = analog_strafing;
                break;
            case 1: // turning
                JoystickAnalogueAxes[nAxis] = analog_turning;
                break;
            case 0: // none
            default:
                JoystickAnalogueAxes[nAxis] = -1;
                break;
            }
            CONTROL_MapAnalogAxis(nAxis, JoystickAnalogueAxes[nAxis]);
            break;
        }
    }
}

void SetJoystickAnalogueInvert(CGameMenuItemZBool* pItem)
{
    for (int nAxis = 0; nAxis < MAXJOYAXES; nAxis++)
    {
        if (pItem == pItemOptionsControlJoystickAxisAnalogueInvert[nAxis])
        {
            JoystickAnalogueInvert[nAxis] = pItem->at20;
            CONTROL_SetAnalogAxisInvert(nAxis, JoystickAnalogueInvert[nAxis]);
            break;
        }
    }
}

void SetJoystickDigitalPos(CGameMenuItemZCycle* pItem)
{
    for (int nAxis = 0; nAxis < MAXJOYAXES; nAxis++)
    {
        if (pItem == pItemOptionsControlJoystickAxisDigitalPos[nAxis])
        {
            for (int j = 0; j < NUMGAMEFUNCTIONS+1; j++)
            {
                if (JoystickDigitalFunctions[nAxis][0] == nGamefuncsValues[j])
                {
                    int nFunc = nGamefuncsValues[pItem->m_nFocus];
                    JoystickDigitalFunctions[nAxis][0] = nFunc;
                    CONTROL_MapDigitalAxis(nAxis, JoystickDigitalFunctions[nAxis][0], 0);
                    return;
                }
            }
        }
    }
}

void SetJoystickDigitalNeg(CGameMenuItemZCycle* pItem)
{
    for (int nAxis = 0; nAxis < MAXJOYAXES; nAxis++)
    {
        if (pItem == pItemOptionsControlJoystickAxisDigitalNeg[nAxis])
        {
            for (int j = 0; j < NUMGAMEFUNCTIONS+1; j++)
            {
                if (JoystickDigitalFunctions[nAxis][1] == nGamefuncsValues[j])
                {
                    int nFunc = nGamefuncsValues[pItem->m_nFocus];
                    JoystickDigitalFunctions[nAxis][1] = nFunc;
                    CONTROL_MapDigitalAxis(nAxis, JoystickDigitalFunctions[nAxis][1], 1);
                    return;
                }
            }
        }
    }
}

void SetJoystickDeadzone(CGameMenuItemSlider* pItem)
{
    for (int nAxis = 0; nAxis < MAXJOYAXES; nAxis++)
    {
        if (pItem == pItemOptionsControlJoystickAxisDeadzone[nAxis])
        {
            JoystickAnalogueDead[nAxis] = pItem->nValue;
            JOYSTICK_SetDeadZone(nAxis, JoystickAnalogueDead[nAxis], JoystickAnalogueSaturate[nAxis]);
            break;
        }
    }
}

void SetJoystickSaturate(CGameMenuItemSlider* pItem)
{
    for (int nAxis = 0; nAxis < MAXJOYAXES; nAxis++)
    {
        if (pItem == pItemOptionsControlJoystickAxisSaturate[nAxis])
        {
            JoystickAnalogueSaturate[nAxis] = pItem->nValue;
            JOYSTICK_SetDeadZone(nAxis, JoystickAnalogueDead[nAxis], JoystickAnalogueSaturate[nAxis]);
            break;
        }
    }
}

void SetJoystickSoloDeadzone(CGameMenuItemZBool* pItem)
{
    for (int nAxis = 0; nAxis < MAXJOYAXES; nAxis++)
    {
        if (pItem == pItemOptionsControlJoystickAxisSoloDeadzone[nAxis])
        {
            JoystickAnalogueAxisSoloDeadZone[nAxis] = pItem->at20;
            JOYSTICK_SetAxisSoloDeadZone(nAxis, JoystickAnalogueAxisSoloDeadZone[nAxis]);
            break;
        }
    }
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

void SetJoyButton(CGameMenuItemZCycle *pItem)
{
    for (int nPage = 0; nPage < MAXJOYSTICKBUTTONPAGES; nPage++) // find selected menu item used for this bind
    {
        for (int nButton = 0; nButton < JOYSTICKITEMSPERPAGE; nButton++)
        {
            if (pItem == pItemOptionsControlJoyButton[nPage][nButton]) // found menu item, now bind function to joystick button
            {
                const char bDoubleTap = nButton & 1;
                const int nJoyButton = ((nPage * JOYSTICKITEMSPERPAGE)>>1) + (nButton>>1); // we halve the button index because button lists are listed in pairs of single tap/double tap inputs
                int nFunc = nGamefuncsValues[pItem->m_nFocus];
                JoystickFunctions[nJoyButton][bDoubleTap] = nFunc;
                CONTROL_MapButton(nFunc, nJoyButton, bDoubleTap, controldevice_joystick);
                return;
            }
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
    menuNetwork.Add(&itemNetworkPlayer, false);
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
    if (gGameOptions.nGameType != kGameTypeSinglePlayer || !gGameStarted)
        return;
    if (event->at0 != 6/* || strSaveGameName[0]*/)
    {
        gGameMenuMgr.Deactivate();
        return;
    }
    if (G_ModDirSnprintf(strSaveGameName, BMAX_PATH, "game00%02d.sav", nSlot))
        return;
    memset(gGameOptions.szUserGameName, 0, sizeof(gGameOptions.szSaveGameName));
    strcpy(gGameOptions.szUserGameName, strRestoreGameStrings[nSlot]);
    memset(gGameOptions.szSaveGameName, 0, sizeof(gGameOptions.szSaveGameName));
    sprintf(gGameOptions.szSaveGameName, "%s", strSaveGameName);
    gGameOptions.nSaveGameSlot = nSlot;
    if (gShowLoadingSavingBackground)
    {
        viewLoadingScreen(gMenuPicnum, "Saving", "Saving Your Game", strRestoreGameStrings[nSlot]);
        videoNextPage();
    }
    gSaveGameNum = nSlot;
    LoadSave::SaveGame(strSaveGameName);
    gGameOptions.picEntry = gSavedOffset;
    gSaveGameOptions[nSlot] = gGameOptions;
    LoadUpdateSaveGame(nSlot, gProfile[myconnectindex].skill);
    UpdateSaveGameItemText(nSlot);
    gQuickLoadSlot = nSlot;
    gGameMenuMgr.Deactivate();
    viewSetMessage("Game saved");
}

void QuickSaveGame(void)
{
    char strSaveGameName[BMAX_PATH];
    if (gGameOptions.nGameType != kGameTypeSinglePlayer || !gGameStarted)
        return;
    /*if (strSaveGameName[0])
    {
        gGameMenuMgr.Deactivate();
        return;
    }*/
    if (G_ModDirSnprintf(strSaveGameName, BMAX_PATH, "game00%02d.sav", kLoadSaveSlotQuick))
        return;
    memset(gGameOptions.szUserGameName, 0, sizeof(gGameOptions.szSaveGameName));
    snprintf(gGameOptions.szUserGameName, sizeof(gGameOptions.szUserGameName), "%s quick", gGameOptions.zLevelName);
    memset(gGameOptions.szSaveGameName, 0, sizeof(gGameOptions.szSaveGameName));
    sprintf(gGameOptions.szSaveGameName, "%s", strSaveGameName);
    gGameOptions.nSaveGameSlot = kLoadSaveSlotQuick;
    if (gShowLoadingSavingBackground)
    {
        viewLoadingScreen(gMenuPicnum, "Saving", "Saving Your Game", strRestoreGameStrings[kLoadSaveSlotQuick]);
        videoNextPage();
    }
    LoadSave::SaveGame(strSaveGameName);
    gGameOptions.picEntry = gSavedOffset;
    gSaveGameOptions[kLoadSaveSlotQuick] = gGameOptions;
    LoadUpdateSaveGame(kLoadSaveSlotQuick, gProfile[myconnectindex].skill);
    UpdateSaveGameItemText(kLoadSaveSlotQuick);
    gQuickLoadSlot = kLoadSaveSlotQuick;
    gGameMenuMgr.Deactivate();
    viewSetMessage("Game saved");
}

void AutosaveGame(bool bLevelStartSave)
{
    char strSaveGameName[BMAX_PATH];
    int nSlot = bLevelStartSave ? kLoadSaveSlotSpawn : kLoadSaveSlotKey;
    if (gGameOptions.nGameType != kGameTypeSinglePlayer || !gGameStarted)
        return;
    if (G_ModDirSnprintf(strSaveGameName, BMAX_PATH, "gameautosave%1d.sav", nSlot - kLoadSaveSlotSpawn))
        return;
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
    if (gGameOptions.nGameType != kGameTypeSinglePlayer)
        return;
    if (nSlot <= kLoadSaveSlot10)
    {
        if (G_ModDirSnprintf(strLoadGameName, BMAX_PATH, "game00%02d.sav", nSlot))
            return;
    }
    else
    {
        if (G_ModDirSnprintf(strLoadGameName, BMAX_PATH, "gameautosave%1d.sav", nSlot == kLoadSaveSlotSpawn ? 0 : 1))
            return;
    }
    int const bakpathsearchmode = pathsearchmode;
    pathsearchmode = 1;
    if (!testkopen(strLoadGameName, 0))
    {
        pathsearchmode = bakpathsearchmode;
        return;
    }
    if (!gGameStarted || LoadSavedInCurrentSession(nSlot)) // if save slot is from a different session, set autosave state to false
        gAutosaveInCurLevel = false;
    if (gShowLoadingSavingBackground)
    {
        viewLoadingScreen(gMenuPicnum, "Loading", "Loading Saved Game", strRestoreGameStrings[nSlot]);
        videoNextPage();
    }
    LoadSave::LoadGame(strLoadGameName);
    gGameMenuMgr.Deactivate();
    gQuickLoadSlot = nSlot;
    pathsearchmode = bakpathsearchmode;
}

void QuickLoadGame(void)
{
    char strLoadGameName[BMAX_PATH];
    if (gGameOptions.nGameType != kGameTypeSinglePlayer)
        return;
    if (gQuickLoadSlot < kLoadSaveSlotAutosave)
    {
        if (G_ModDirSnprintf(strLoadGameName, BMAX_PATH, "game00%02d.sav", gQuickLoadSlot))
            return;
    }
    else
    {
        if (G_ModDirSnprintf(strLoadGameName, BMAX_PATH, "gameautosave%1d.sav", gQuickLoadSlot == kLoadSaveSlotSpawn ? 0 : 1))
            return;
    }
    int const bakpathsearchmode = pathsearchmode;
    pathsearchmode = 1;
    if (!testkopen(strLoadGameName, 0))
    {
        pathsearchmode = bakpathsearchmode;
        return;
    }
    if (!LoadSavedInCurrentSession(gQuickLoadSlot)) // if save slot is from a different session, set autosave state to false
        gAutosaveInCurLevel = false;
    if (gShowLoadingSavingBackground)
    {
        viewLoadingScreen(gMenuPicnum, "Loading", "Loading Saved Game", strRestoreGameStrings[gQuickLoadSlot]);
        videoNextPage();
    }
    LoadSave::LoadGame(strLoadGameName);
    gGameMenuMgr.Deactivate();
    pathsearchmode = bakpathsearchmode;
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
    if (gPlayerRoundEnding) // do not allow player to start a new match while round is ending
        return;
    gPacketStartGame.gameType = itemNetGameMode.m_nFocus+1;
    if (gPacketStartGame.gameType == kGameTypeSinglePlayer)
        gPacketStartGame.gameType = kGameTypeBloodBath;
    gPacketStartGame.uNetGameFlags = kNetGameFlagNone;
    if ((itemNetGameCycleLength.m_nFocus > 0) && (gPacketStartGame.gameType != kGameTypeCoop))
    {
        gPacketStartGame.uNetGameFlags |= itemNetGameCycleLength.m_nFocus == 1 ? kNetGameFlagLimitMinutes : kNetGameFlagLimitFrags;
        gPacketStartGame.uNetGameFlags |= (ClipRange(itemNetGameSliderLimit.nValue, 1, 255)<<kNetGameFlagLimitBase)&kNetGameFlagLimitMask;
    }
    else if ((itemNetGameCycleLengthCoop.m_nFocus > 0) && (gPacketStartGame.gameType == kGameTypeCoop))
    {
        gPacketStartGame.uNetGameFlags |= kNetGameFlagLimitFrags;
        gPacketStartGame.uNetGameFlags |= (ClipRange(itemNetGameSliderLimit.nValue, 1, 255)<<kNetGameFlagLimitBase)&kNetGameFlagLimitMask;
    }
    if (!itemNetGameBoolExit.at20 && (gPacketStartGame.gameType != kGameTypeCoop))
        gPacketStartGame.uNetGameFlags |= kNetGameFlagNoLevelExit;
    if (!itemNetGameBoolTeleFrag.at20)
        gPacketStartGame.uNetGameFlags |= kNetGameFlagNoTeleFrag;
    if (!itemNetGameBoolSkillOverride.at20)
        gPacketStartGame.uNetGameFlags |= kNetGameFlagSkillIssue;
    if (!itemNetGameBoolModelOverride.at20)
        gPacketStartGame.uNetGameFlags |= kNetGameFlagCalebOnly;
    if (!itemNetGameBoolTeamColors.at20)
        gPacketStartGame.uNetGameFlags |= kNetGameFlagNoTeamColors;
    if (!itemNetGameBoolTeamFlags.at20)
        gPacketStartGame.uNetGameFlags |= kNetGameFlagNoTeamFlags;
    if (itemNetGameCycleSpawnLocation.m_nFocus > 0)
        gPacketStartGame.uNetGameFlags |= itemNetGameCycleSpawnLocation.m_nFocus == 1 ? kNetGameFlagSpawnSmart : kNetGameFlagSpawnDist;
    if (itemNetGameCycleShowWeaponsOverride.m_nFocus > 0)
        gPacketStartGame.uNetGameFlags |= itemNetGameCycleShowWeaponsOverride.m_nFocus == 1 ? kNetGameFlagHideWeaponsCloak : kNetGameFlagHideWeaponsAlways;
    if (itemNetGameCycleMirrorModeOverride.m_nFocus & 1)
        gPacketStartGame.uNetGameFlags |= kNetGameFlagMirrorHoriz;
    if (itemNetGameCycleMirrorModeOverride.m_nFocus & 2)
        gPacketStartGame.uNetGameFlags |= kNetGameFlagMirrorVert;
    gPacketStartGame.episodeId = itemNetStart2.m_nFocus;
    gPacketStartGame.levelId = itemNetStart3.m_nFocus;
    gPacketStartGame.difficulty = itemNetStart4.m_nFocus;
    gPacketStartGame.weaponSettings = itemNetStart6.m_nFocus;
    gPacketStartGame.itemSettings = itemNetStart7.m_nFocus;
    gPacketStartGame.respawnSettings = 0;
    gPacketStartGame.bFriendlyFire = itemNetGameBoolFriendlyFire.at20;
    gPacketStartGame.keySettings = itemNetGameCycleKey.m_nFocus;
    gPacketStartGame.itemWeaponSettings = itemNetGameCycleItemWeapon.m_nFocus;
    gPacketStartGame.bAutoTeams = itemNetGameBoolAutoTeams.at20;
    gPacketStartGame.nSpawnProtection = itemNetGameCycleSpawnProtection.m_nFocus;
    gPacketStartGame.nSpawnWeapon = itemNetGameCycleSpawnWeapon.m_nFocus;

    gPacketStartGame.monsterSettings = itemNetMonsterSettings.m_nFocus;
    gPacketStartGame.monsterQuantity = ClipRange(itemNetMonsterQuantity.nValue, 0, 4);
    gPacketStartGame.monsterHealth = ClipRange(itemNetMonsterHealth.nValue, 0, 4);
    gPacketStartGame.monsterSpeed = ClipRange(itemNetMonsterSpeed.m_nFocus, 0, 4);

    ////
    SetGameVanillaMode(0); // turn off vanilla mode for multiplayer so menus don't get bugged
    gPacketStartGame.bQuadDamagePowerup = itemNetMutatorBoolQuadDamagePowerup.at20;
    gPacketStartGame.nDamageInvul = itemNetMutatorDamageInvul.m_nFocus % ARRAY_SSIZE(pzExplosionBehaviorStrings);
    gPacketStartGame.nExplosionBehavior = itemNetMutatorExplosionBehavior.m_nFocus % ARRAY_SSIZE(pzExplosionBehaviorStrings);
    gPacketStartGame.nProjectileBehavior = itemNetMutatorProjectileBehavior.m_nFocus % ARRAY_SSIZE(pzProjectileBehaviorStrings);
    gPacketStartGame.bNapalmFalloff = itemNetMutatorNapalmFalloff.at20;
    gPacketStartGame.nEnemyBehavior = itemNetMutatorEnemyBehavior.m_nFocus % ARRAY_SSIZE(pzEnemyBehaviorStrings);
    gPacketStartGame.bEnemyRandomTNT = itemNetMutatorBoolEnemyRandomTNT.at20;
    gPacketStartGame.nWeaponsVer = itemNetMutatorWeaponsVer.m_nFocus % ARRAY_SSIZE(pzWeaponsVersionStrings);
    gPacketStartGame.bSectorBehavior = itemNetMutatorSectorBehavior.at20;
    gPacketStartGame.nHitscanProjectiles = itemNetMutatorHitscanProjectiles.m_nFocus % ARRAY_SSIZE(pzHitscanProjectilesStrings);
    gPacketStartGame.randomizerMode = itemNetMutatorRandomizerMode.m_nFocus % ARRAY_SSIZE(pzRandomizerModeStrings);
    Bstrncpy(gPacketStartGame.szRandomizerSeed, szRandomizerSeedMenu, sizeof(gPacketStartGame.szRandomizerSeed));
    if (gPacketStartGame.szRandomizerSeed[0] == '\0') // if no seed entered, generate new one before sending packet
        sprintf(gPacketStartGame.szRandomizerSeed, "%08X", QRandom2(gGameMenuMgr.m_mousepos.x*gGameMenuMgr.m_mousepos.y));
    gPacketStartGame.uSpriteBannedFlags = SetBannedSprites(0);
    ////
    Bstrncpy(gPacketStartGame.userMapName, zUserMapName, sizeof(zUserMapName));
    gPacketStartGame.userMap = gPacketStartGame.userMapName[0] != '\0';

    netBroadcastNewGame();
    gStartNewGame = 1;
    gGameMenuMgr.Deactivate();
}

void Restart(CGameMenuItemChain *pItem)
{
    UNREFERENCED_PARAMETER(pItem);
    if (gGameOptions.nGameType == kGameTypeSinglePlayer || numplayers == 1)
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
    if (gGameOptions.nGameType == kGameTypeSinglePlayer || numplayers == 1)
        gQuitGame = true;
    else
        gQuitRequest = 1;
    gGameMenuMgr.Deactivate();
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
            for (int j = 0; j < kMaxLevels; j++)
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
    if (gGameOptions.nGameType == kGameTypeSinglePlayer)
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

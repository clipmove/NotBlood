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
#include "gamemenu.h"

extern CGameMenu menuMain;
extern CGameMenu menuMainWithSave;
extern CGameMenu menuFirstLaunch;
extern CGameMenu menuNetMain;
extern CGameMenu menuNetStart;
extern CGameMenu menuEpisode;
extern CGameMenu menuDifficulty;
extern CGameMenu menuCustomDifficulty;
extern CGameMenu menuControls;
extern CGameMenu menuMessages;
extern CGameMenu menuKeys;
extern CGameMenu menuSaveGame;
extern CGameMenu menuLoadGame;
extern CGameMenu menuLoading;
extern CGameMenu menuQuit;
extern CGameMenu menuRestart;
extern CGameMenu menuCredits;
extern CGameMenu menuHelp;

extern CGameMenu menuOptions;
extern CGameMenu menuOptionsGame;
extern CGameMenu menuOptionsGameMutators;
extern CGameMenu menuOptionsDisplay;
extern CGameMenu menuOptionsDisplayColor;
extern CGameMenu menuOptionsDisplayMode;
extern CGameMenu menuOptionsSound;
extern CGameMenu menuOptionsControl;
extern short gQuickLoadSlot;
extern char strRestoreGameStrings[][16];
extern int restoreGameDifficulty[];
extern const char *zDiffStrings[];
extern char zUserMapName[BMAX_PATH];
extern char zNetAddressBuffer[16];
extern char zNetPortBuffer[6];
void drawLoadingScreen(int nTile);
void SetupMenus(void);
void UpdateNetworkMenus(void);
void SetGameVanillaMode(char nState);
void QuickSaveGame(void);
void AutosaveGame(bool bLevelStartSave);
void QuickLoadGame(void);
void StartNetGame(CGameMenuItemChain *);
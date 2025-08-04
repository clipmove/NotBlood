<p align="center">
  <!-- logo -->
  <a href="https://github.com/clipmove/NotBlood" target="_blank"><img src="https://raw.githubusercontent.com/clipmove/NotBlood/master/.github/workflows/logo.png"></a>
  <br>
  <!-- primary badges -------------------------------------->
  <a href="https://github.com/clipmove/NotBlood/actions/workflows/build.yml" target"_blank"><img src="https://github.com/clipmove/NotBlood/actions/workflows/build.yml/badge.svg?style=flat-square" alt="Build Status"></a>
  <a href="https://github.com/clipmove/NotBlood/releases" target"_blank"><img src="https://raw.githubusercontent.com/clipmove/NotBlood/master/.github/workflows/download.svg?style=flat-square" alt="Github Download"></a>
</p><h1></h1>

### Overview
**NotBlood** is a fork of [NBlood](https://github.com/nukeykt/NBlood) with gameplay options, optional mutators and multiplayer features, while retaining NBlood mod support

### Downloads
Download for Windows/Linux/MacOS can be found on [https://github.com/clipmove/NotBlood/releases](https://github.com/clipmove/NotBlood/releases)

### Features
* Switch to last active weapon if TNT/spray can is active when entering water
* Basic room over room support for positional audio ([before](https://web.archive.org/web/20220319193715if_/https://files.catbox.moe/qca0k4.mp4)/[after](https://web.archive.org/web/20220514202908if_/https://files.catbox.moe/wq1so4.mp4))
* Autosaving support for collecting keys and start of level
* New last weapon key binding (default Q) ([demo](https://web.archive.org/web/20220319193716if_/https://files.catbox.moe/28cirg.mp4))
* BloodGDX style difficulty options for singleplayer
* Set item box selection to activated item
* Ability to record DOS compatible demos
* Interpolated level texture panning
* Drag and drop folder mod support ([demo](https://web.archive.org/web/20220514203004if_/https://files.catbox.moe/lb7nxb.mp4))
* Customizable palette adjustment
* New singleplayer cheats
* Weapon selection bar ([demo](https://web.archive.org/web/20220514203005if_/https://files.catbox.moe/0zh37q.mp4))
* Mirror mode
* Auto crouch

### Multiplayer Features
* Cloak powerup hides player weapon icon
* Improved spawning randomization logic
* Colored player names for messages
* Adjustable spawn weapon option
* Adjustable spawn protection
* UT99 style multi kill alerts
* Uneven teams support

### Mutators (Optional)
* Difficulty based invulnerability timer for player damage ([before](https://web.archive.org/web/20220319193718if_/https://files.catbox.moe/ucs7gp.mp4)/[after](https://web.archive.org/web/20220514201922if_/https://files.catbox.moe/8hyaqm.mp4))
* Raymarching collision testing for player projectiles ([before](https://web.archive.org/web/20220514202239if_/https://files.catbox.moe/qxtv05.mp4)/[after](https://web.archive.org/web/20220319193719if_/https://files.catbox.moe/vo03ck.mp4))
* Fix blood/bullet casings not being dragged with sectors ([before](https://web.archive.org/web/20220514202751if_/https://files.catbox.moe/4q9rc3.mp4)/[after](https://web.archive.org/web/20220514202840if_/https://files.catbox.moe/7n76gv.mp4))
* Fixed underwater issue with hitscan weapons ([before](https://web.archive.org/web/20220514203003if_/https://files.catbox.moe/k9dxjj.mp4)/[after](https://web.archive.org/web/20220514203004if_/https://files.catbox.moe/gfahdq.mp4))
* Fixed missiles colliding with water sector edges ([before](https://web.archive.org/web/20220514202908if_/https://files.catbox.moe/38t9t8.mp4)/[after](https://web.archive.org/web/20220514202910if_/https://files.catbox.moe/smvi92.mp4))
* Smaller hitboxes for player projectiles ([before](https://web.archive.org/web/20220319193738if_/https://files.catbox.moe/3peiru.mp4)/[after](https://web.archive.org/web/20220514201943if_/https://files.catbox.moe/zso8g4.mp4))
* NotBlood balance mod for weapons (see README.txt for details)
* Randomize mode for enemies and pickups (multiplayer supported)
* Quad damage replacement for guns akimbo powerup
* Respawning enemies option for singleplayer
* Bullet projectiles for hitscan enemies
* Fixed bullet casings clipping into walls
* Lower gravity of bullet casings and gibs underwater
* Allow particle sprites to traverse through room over room sectors
* Increased blood splatter duration and improved floor collision accuracy
* Make blood splatter/flare gun glow effect slope on sloped surfaces
</details>

### Installing
1. Extract NotBlood to a new directory
2. Copy the following files from Blood (v1.21) to NotBlood folder:
   * BLOOD.INI
   * BLOOD.RFF
   * BLOOD000.DEM, ..., BLOOD003.DEM (optional)
   * CP01.MAP, ..., CP09.MAP (optional, Cryptic Passage)
   * CPART07.AR_ (optional, Cryptic Passage)
   * CPART15.AR_ (optional, Cryptic Passage)
   * CPBB01.MAP, ..., CPBB04.MAP (optional, Cryptic Passage)
   * CPSL.MAP (optional, Cryptic Passage)
   * CRYPTIC.INI (optional, Cryptic Passage)
   * CRYPTIC.SMK (optional, Cryptic Passage)
   * CRYPTIC.WAV (optional, Cryptic Passage)
   * GUI.RFF
   * SOUNDS.RFF
   * SURFACE.DAT
   * TILES000.ART, ..., TILES017.ART
   * VOXEL.DAT

3. Optionally, if you want to use CD audio tracks instead of MIDI, provide FLAC/OGG recordings in following format: bloodXX.flac/ogg, where XX is track number. Make sure to enable Redbook audio option in sound menu.
4. Optionally, if you want cutscenes and you have the original CD, copy the `movie` folder into NotBlood's folder (the folder itself too).
If you have the GOG version of the game, do the following:
   * make a copy of `game.ins` (or `game.inst`) named `game.cue`
   * mount the `.cue` as a virtual CD (for example with `WinCDEmu`)
   * copy the `movie` folder from the mounted CD into NotBlood's folder
5. Launch NotBlood (on Linux, to play Cryptic Passage, launch with the `-ini CRYPTIC.INI` parameter)
</details>

### Building from source
See: https://wiki.eduke32.com/wiki/Main_Page

### Acknowledgments
  See AUTHORS.md
# NotBlood
A gameplay modification of Blood, based upon the EDuke32 driven Blood reverse-engineer port NBlood

New features include:
* Default midi uses a SC55 soundfont by trevor0402 (https://github.com/trevor0402/SC55Soundfont)
* Difficulty based invulnerability timer for player damage
* Smaller hitboxes for player projectiles
* Raymarching collision testing for player projectiles
* Replaced sector based damage logic for player explosions
* Vanilla mode with optional DOS mouse handling
* Quad damage replacement for guns akimbo powerup
* Randomize mode for enemies and pickups (multiplayer supported)
* Increase drawn sprites on screen limit from 2560 to 4096
* Autosaving on collecting keys and start of level
* Bullet projectiles for hitscan enemies
* Fixed bullet casings clipping into walls
* New enhancements menu under game options
* New last weapon key binding (default Q)
* Lower gravity of bullet casings and gibs underwater
* Fix blood/bullet casings not being dragged with sectors
* Improved spawning randomization logic for bloodbath mode
* Cloak powerup hides player weapon icon for bloodbath mode
* Allow particle sprites to traverse through room over room sectors
* Switch to last active weapon if TNT/spray can is active when entering water
* Increased blood splatter duration and improved floor collision accuracy
* Make blood splatter/flare gun glow effect slope on sloped surfaces
* Basic room over room support for positional audio
* Fixed missiles colliding with water sector edges
* Fixed underwater issue with hitscan weapons
* Respawning enemies option for singleplayer
* Custom difficult options for singleplayer
* Set item box selection to activated item
* Ability to record 1.21 compatible demos
* Colored names for multiplayer messages
* Interpolated level texture panning
* Drag and drop folder mod support
* Customizable palette adjustment
* Weapon selection bar
* Mirror mode

### Installing
1. Extract NotBlood to a new directory
2. Copy the following files from Blood 1.21 to NotBlood folder:

   BLOOD.INI
   BLOOD.RFF
   BLOOD000.DEM, ..., BLOOD003.DEM (optional)
   CP01.MAP, ..., CP09.MAP (optional, Cryptic Passage)
   CPART07.AR_ (optional, Cryptic Passage)
   CPART15.AR_ (optional, Cryptic Passage)
   CPBB01.MAP, ..., CPBB04.MAP (optional, Cryptic Passage)
   CPSL.MAP (optional, Cryptic Passage)
   CRYPTIC.INI (optional, Cryptic Passage)
   CRYPTIC.SMK (optional, Cryptic Passage)
   CRYPTIC.WAV (optional, Cryptic Passage)
   GUI.RFF
   SOUNDS.RFF
   SURFACE.DAT
   TILES000.ART, ..., TILES017.ART
   VOXEL.DAT

3. Optionally, if you want to use CD audio tracks instead of MIDI, provide FLAC/OGG recordings in following format: bloodXX.flac/ogg, where XX is track number. Make sure to enable Redbook audio option in sound menu.
4. Optionally, if you want cutscenes and you have the original CD, copy the `movie` folder into NotBlood's folder (the folder itself too).
If you have the GOG version of the game, do the following:
   - make a copy of `game.ins` (or `game.inst`) named `game.cue`
   - mount the `.cue` as a virtual CD (for example with `WinCDEmu`)
   - copy the `movie` folder from the mounted CD into NotBlood's folder
5. Launch NotBlood (on Linux, to play Cryptic Passage, launch with the `-ini CRYPTIC.INI` parameter)

### Vanilla Mode
This feature sets the game to match v1.21 DOS compatibility, and removes any bugs fixes that come with NBlood/NotBlood
The vanilla mode option can be found within game settings, and is only available for single-player
It should be noted that vanilla mode does not work with modern maps

### Enhancements Options
* Replace guns akimbo with quad damage
   - Replaces the guns akimbo powerup with Quake's quad damage (lasts 22 seconds)
* Player damage invulnerability
   - Apply a short invulnerability state for the player for bullet hitscans/spirit/tesla damage
   - Invulnerability duration changes depending on damage taken scale/current health (lower health = longer invulnerability state)
* Explosions behavior
   - Replace the default vanilla explosion calculation with an new improved sector scanning system
   - Fixes rare cases where enemies appear to ignore explosive damage due to sector/span order
   - Includes two methods to choose from: NotBlood (check linked sectors once) and Raze (always check linked sectors)
* Projectiles behavior (NotBlood)
   - For all missiles/projectiles, use the more accurate eduke32's clipmove() function
   - Player missiles/projectile hitboxes have been reduced so it's easier to throw/target around corners
   - Run raymarching checks on player projectiles to ensure they do not clip through enemies/geometry
   - When spawning player projectiles, it'll do a alpha pixel check for initial hitscan that return a non-dude sprite hit (e.g: throwing TNT up close to the tree sprites in CPSL.MAP)
* Projectiles behavior (Raze)
   - For all missiles/projectiles, use the more accurate eduke32's clipmove() function
   - Leave all hitbox sizes as original
* Enemy behavior
   - Fixes various original 1.21 bugs with enemies such as:
   - Tiny Caleb using the wrong burning sprite
   - Enemies sometimes burning indefinitely
   - Ignited cultists switching weapons when extinguished in water
   - Cerberus spinning uselessly on lava
   - Improved beast stomp attack sector scanning
   - Check if enemy is alive before setting target for AI
   - Fixes tesla cultists bugged prone attack sequence
   - Turn enemy around if stuck running into a corner for a few seconds
* Random cultist TNT
   - This will make cultists use a variety of random thrown sprites such as:
   - Napalm balls, proxy bundles, armed spray cans or pod projectiles
* Weapon behavior
   - Select between original weapon behavior, NBlood's V1.X behavior or NotBlood's tweaked weapon set which include:
   - Adjust pitch offset for spray/missile firing
   - Adds a charge up stab for pitchfork's alt fire
   - Makes lifeleech throwable and increases damage while in sentry mode
   - Do double melee damage if attacking enemies from 45 degrees behind
   - Allows tesla projectiles to be reflected back with reflective shots powerup
   - Adds a special alt fire missile attack for pitchfork's charge up while quad damage is active
   - Voodoo doll alt fire attacks all targets visible on screen (like V1.X behavior) and consumes all ammo
* Sector behavior
   - Fixes room over room sector handling with hitscan calculations
   - Allow hitscan weapons to be fired when floating above water
   - Prevent missiles from colliding with water surface sectors
   - Fixes blood splatter not using closest sector
   - Improves FX sprite handling regarding room over room transitioning
   - Check wall collision for spent bullet casings movement
   - Allow spent bullet casings to use water sector links
   - Support spent bullet casings and blood splatter for moving sectors
   - Lower gravity of bullet casings and gibs underwater
   - Add player's velocity to spawned bullet casings
   - Make blood splatter/flare gun glow effect slope on sloped surfaces
   - Prevent glow effects from rendering over room over room surfaces
   - Add room over room sector transversal for shadow rendering
   - Support wall sprites moving along with elevators
   - Fix bullet hole being placed across sky tile walls
* Hitscan projectiles
   - Makes enemies that use hitscan bullets spawn physical sprite based bullets with travel time
   - Projectile speed is adjusted depending on difficulty and if bullet is underwater
   - This enhancement does not support custom modern map enemies
* Randomizer mode
   - Set the enemy/pickups randomizer mode
   - The randomizer does not support custom modern map enemies
* Randomizer seed
   - Set the enemy/pickups randomizer's seed
   - An empty string will regenerate anew for every level start

### New cheats (press t in-game to type codes)
* BIG BERTHA - Toggle randomized lifeleech projectiles (works for alt fire mode)
* QSKFA - Toggle blood alpha alt fire missile for pitchfork
* ZOOMIES - Toggle fast enemies
* SONIC - Toggle fast player movement
* NO U - Activates reflect shots power-up

### Randomizer seed cheats
* AAAAAAAA - Phantoms only
* BUTCHERS - Butchers only
* SOULSEEK - Hands only
* EPISODE6 - Cultists only
* GARGOYLE - Gargoyles only
* FLAMEDOG - Hell hounds only
* CAPYBARA - Rats only
* HURTSCAN - Shotgun/tommy gun cultists only
* HUGEFISH - Gill beasts only
* SHOCKING - Tesla cultists only
* CRUONITA - Boss types only
* BILLYRAY - Shotgun cultists only
* WEED420! - Cultists only but they're green
* BRAAAINS - Zombies only
* OKBOOMER - TNT cultists only
* OKZOOMER - TNT/tesla cultists only
* SNEAKYFU - Proned shotgun/tommy gun cultists only

### New console variables for NotBlood
* cl_calebtalk
   - Enable/disable Caleb's dialog lines (0: on, 1: no idle, 2: no explosion/gib, 3: off)
* cl_chatsnd
   - Enable/disable multiplayer chat message beep
* cl_interpolatepanning
   - Enable/disable sector texture panning interpolation (cl_interpolate must be set on)
* cl_interpolateweapon
   - Enable/disable view interpolation for drawn weapon (0: disable, 1: position, 2: position/qav animation)
* cl_killmsg
   - Enable/disable kill messages
* cl_multikill
   - Enable/disable multi kill messages (0: disable, 1: enable, 2: enable + audio alert)
* cl_weaponhbob
   - Enable/disable view horizontal bobbing
* cl_slopecrosshair
   - Enable/disable adjusting crosshair position for slope tilting
* cl_slowroomflicker
   - Enable/disable slowed flickering speed for sectors (such as E1M4's snake pit room)
* cl_shadowsfake3d
   - Enable/disable 3D projection for fake sprite shadows
* cl_smoketrail3d
   - Enable/disable 3D smoke trail positioning for tnt/spray can (single-player only)
* cl_particlesduration
   - Enable/disable extended particle duration modification (single-player only - turned off for modern maps)
* cl_weaponfastswitch
   - Enable/disable fast weapon switching
* cl_packitemswitch
   - Enable/disable item slot switching to activated item (always enabled in multiplayer)
* cl_quickstart
   - Enable/disable quick start mode (start to menu on launch/disable demo playback)
* crosshair
   - Enable/disable crosshair (0: off, 1: on, 2: on [autoaim])
* hud_bgscale
   - Enable/disable hud background image scaling for resolution
* hud_stats
   - Set aspect ratio screen position for hud (0: native, 1: 4:3, 2: 16:10, 3: 16:9, 3: 21:9)
* hud_ratio
   - Enable/disable level statistics display (0: off, 1: on [default], 2: on [4:3], 3: on [16:10], 4: on [16:9], 5: on [21:9])
* hud_powerupdurationticks
   - set the tickrate divide value used for displaying the remaining time for power-ups (default: 100, realtime seconds: 120)
* hud_showweaponselect
   - Enable/disable weapon select bar display. (0: none, 1: bottom, 2: top)
* hud_showweaponselecttimestart
   - Length of time for selected weapon bar to appear
* hud_showweaponselecttimehold
   - Length of time to display selected weapon bar
* hud_showweaponselecttimeend
   - Length of time for selected weapon weapon bar to disappear
* hud_showweaponselectposition
   - Position offset for selected weapon weapon bar
* hud_showweaponselectscale
   - Sets scale for selected weapon weapon bar (default: 10, range: 5-20)
* in_centerviewondrop
   - Enable/disable recenter view when dropping down onto ground
* in_crouchmode
   - Toggles crouch button (0:hold, 1:toggle)
* in_turnsensitivity
   - Keyboard turning sensitivity multiplier (default: 92, range: 64-128)
* notarget
   - Toggles AI player detection
* r_drawinvisiblesprites
   - Forcefully draw invisible sprites
* r_mirrormode
   - Mirror output display: 0: off 1: mirror horizontal 2: mirror vertically 3: mirror horizontal/vertically
* r_rotatespriteinterp
   - Interpolate repeated rotatesprite calls
   - 0: disable
   - 1: only interpolate when explicitly requested with RS_LERP (half-step interpolation)
   - 2: only interpolate when explicitly requested with RS_LERP (full interpolation)
   - 3: interpolate if the picnum or size matches regardless of RS_LERP being set
   - 4: relax above picnum check to include the next tile, with potentially undesirable results
* r_rotatespriteinterpquantize
   - Enable/disable position quantizing for interpolate repeated rotatesprite calls
* r_shadowvoxels
   - Enable/disable wall/floor aligned transparent voxels
* skill
   - Changes the skill handicap for multiplayer (default: 2, range: 0-4)
* snd_earangle
   - Set the listening ear offset (15-90 degrees)
* snd_speed
   - Set the speed of sound m/s used for doppler calculation (default: 343, range: 10-1000)
* snd_underwaterpitch
   - Enable/disable lowering sound pitch by 7% while underwater
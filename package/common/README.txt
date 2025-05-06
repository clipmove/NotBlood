### NotBlood
NotBlood is a fork of NBlood with gameplay options, optional mutators and multiplayer features, while retaining NBlood mod support

### Features
* Switch to last active weapon if TNT/spray can is active when entering water
* Basic room over room support for positional audio
* Autosaving support for collecting keys and start of level
* New last weapon key binding (default Q)
* BloodGDX style difficulty options for singleplayer
* Set item box selection to activated item
* Ability to record DOS compatible demos
* Interpolated level texture panning
* Drag and drop folder mod support
* Customizable palette adjustment
* New singleplayer cheats
* Weapon selection bar
* Mirror mode

### Multiplayer Features
* Cloak powerup hides player weapon icon
* Improved spawning randomization logic
* Colored player names for messages
* Adjustable spawn weapon option
* Adjustable spawn protection
* UT99 style multi kill alerts

### Mutators (Optional)
* Replaced sector based damage logic for player explosions
* Difficulty based invulnerability timer for player damage
* Raymarching collision testing for player projectiles
* Fix blood/bullet casings not being dragged with sectors
* Fixed underwater issue with hitscan weapons
* Fixed missiles colliding with water sector edges
* Smaller hitboxes for player projectiles
* NotBlood balance mod for weapons (see below for details)
* Randomize mode for enemies and pickups (multiplayer supported)
* Quad damage replacement for guns akimbo powerup
* Respawning enemies option for singleplayer
* Bullet projectiles for hitscan enemies
* Fixed bullet casings clipping into walls
* Lower gravity of bullet casings and gibs underwater
* Allow particle sprites to traverse through room over room sectors
* Increased blood splatter duration and improved floor collision accuracy
* Make blood splatter/flare gun glow effect slope on sloped surfaces

### Installing
1. Extract NotBlood to a new directory
2. Copy the following files from Blood (v1.21) to NotBlood folder:

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

### Mutator Options
* Replace guns akimbo with quad damage
   - Replaces the guns akimbo powerup with Quake's quad damage (lasts 22 seconds)
* Player damage invulnerability
   - Apply a short invulnerability state for the player for bullet hitscans/spirit/tesla damage
   - Invulnerability duration changes depending on damage taken scale/current health (lower health = longer invulnerability state)
* Explosions behavior
   - Replace the default vanilla explosion calculation with an new improved sector scanning system
   - Fixes rare cases where enemies appear to ignore explosive damage due to sector/span order
   - Includes two methods to choose from: NotBlood (check linked sectors once) and Raze (always check linked sectors)
* Projectiles behavior (Raze)
   - For all missiles/projectiles, use the more accurate eduke32's clipmove() function
   - Use original hitbox sizes for projectiles
* Projectiles behavior (NotBlood)
   - For all missiles/projectiles, use the more accurate eduke32's clipmove() function
   - Player missiles/projectile hitboxes have been reduced so it's easier to throw/target around corners
   - Force TNT/spray cans to explode if directly landed on enemy's head
   - Run raymarching checks on player projectiles to ensure they do not clip through enemies/geometry
   - When spawning player projectiles, it'll do a alpha pixel check for initial hitscan that return a non-dude sprite hit (e.g: throwing TNT up close to the tree sprites in CPSL.MAP)
* Napalm gravity falloff
   - For player spawned napalm projectiles, make gravity affect their trajectory path
* Enemy behavior (NBlood)
   - Fixes various original 1.21 bugs with enemies such as:
   - Tiny Caleb using the wrong burning sprite
   - Enemies sometimes burning indefinitely
   - Ignited cultists switching weapons when extinguished in water
   - Cerberus spinning uselessly on lava
   - Check if enemy is alive before setting target for AI
   - Fixes tesla cultists bugged prone attack sequence
   - Fixes enemy death sound effects getting cut off by blood splatter sound effects
   - Fixes cultist's alert sound effct not getting cut on death
   - Fixes hands choking attack working in multiplayer
* Enemy behavior (NotBlood)
   - All of the above fixes and including:
   - Improved beast stomp attack sector scanning
   - Fix bloated butchers cleavers hitting prone players
   - Turn enemy around if stuck running into a corner for a few seconds
   - Limit impulse damage when shooting enemies downward at point-blank
   - Cheogh blasting/attacking can now hit prone players (only for well done and above difficulties)
   - Fix Beast state when leaving water sector
   - Restore unused fall animations for cultists
   - Shotgun/tommy gun cultists spawn spent shells (disabled in modern maps)
   - Beast stomp attack will not deal damage if target is not standing on floor
* Random cultist TNT
   - This will make cultists use a variety of random thrown sprites such as:
   - Napalm balls, proxy bundles, armed spray cans or pod projectiles
* Weapon behavior
   - Select between original weapon behavior, NBlood's V1.X behavior or NotBlood's tweaked weapon set which include:
   - Tommy gun alt fire uses autoaim
   - Autoaim is always enabled for pitchfork
   - Make beast vision see through nearby walls
   - Adjust pitch offset for spray/missile firing
   - Adds a charge up stab for pitchfork's alt fire
   - Lifeleech steals enemy's health (like V1.X behavior)
   - Makes lifeleech throwable and increases damage while in sentry mode
   - Lifeleech autoaim range increased by 30 degrees (only in single-player)
   - Do double melee damage if attacking enemies from 45 degrees behind
   - Allows tesla projectiles to be reflected back with reflective shots powerup
   - While quad damage is active, the pitchfork's alt fire max charge will fire a missile
   - Voodoo doll alt fire attacks all targets visible on screen (like V1.X behavior) and consumes all ammo
* Sector behavior
   - Fixes room over room sector handling with hitscan calculations
   - Allow hitscan weapons to be fired when floating above water
   - Prevent missiles from colliding with water surface sectors
   - Fix water/blood droplets transitioning through underwater (e.g: the cave secret in CP01.MAP)
   - Fixes blood splatter not using closest sector
   - Improves FX sprite handling regarding room over room transitioning
   - Check wall collision for spent bullet casings movement (disabled for modern maps)
   - Allow spent bullet casings to use water sector links
   - Support spent bullet casings and blood splatter for moving sectors
   - Lower gravity of bullet casings and gibs underwater
   - Add player's velocity to spawned bullet casings
   - Make blood splatter/flare gun glow effect slope on sloped surfaces
   - Support wall sprites moving along with elevators
   - Fix bullet hole being placed across sky tile walls
   - Use Raze phase calculation for smoother elevator rides
* Hitscan projectiles
   - Makes enemies that use hitscan bullets spawn physical sprite based bullets with travel time
   - Projectile speed is adjusted if bullet is underwater (50% speed penalty)
   - This mutator does not support custom modern map enemies
* Randomizer mode
   - Set the enemy/pickups randomizer mode
   - The randomizer does not support custom modern map enemies
* Randomizer seed
   - Set the enemy/pickups randomizer's seed
   - An empty string will regenerate anew for every level start

### New cheats (press t in-game to type codes)
* BIG BERTHA - Toggle randomized lifeleech projectiles (works for alt fire mode)
* QSKFA - Toggle blood alpha alt fire missile for pitchfork
* SONIC - Toggle fast player movement
* NO U - Activates reflect shots power-up
* JAMES HARDIE - Activates asbestos armor power-up
* VULOVIC - Activates feather fall power-up
* OPPPENHEIMER - Increases TNT explosion damage by 4 times against enemies
* THE ONE - Toggles infinite guns akimbo/quad damage mode, infinite ammo, and all weapons
* KRAVITZ - Toggles fly mode

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
* cl_autodivingsuit
   - Enable/disable automatic diving suit equipping when entering water (always enabled in multiplayer)
* cl_calebtalk
   - Enable/disable Caleb's dialog lines (0: on, 1: no idle, 2: no explosion/gib, 3: off)
* cl_chatsnd
   - Enable/disable multiplayer chat message beep
* cl_dim
   - Enable/disable dimming background when menu is active
* cl_interpolatemethod
   - Set view interpolation method (0: original [integer], 1: notblood [floating-point])"
* cl_interpolatepanning
   - Enable/disable sector texture panning interpolation (cl_interpolate must be set on)
* cl_interpolateweapon
   - Enable/disable view interpolation for drawn weapon (0: disable, 1: position, 2: position/qav animation)
* cl_colormsg
   - Enable/disable colored player names in messages
* cl_healthblink
   - Enable/disable health blinking when under 15 health points
* cl_killmsg
   - Enable/disable kill messages
* cl_killobituaries
   - Enable/disable random obituary kill messages
* cl_multikill
   - Enable/disable multi kill messages (0: disable, 1: enable, 2: enable + audio alert)
* cl_weaponhbob
   - Enable/disable view horizontal bobbing
* cl_rollangle
   - Sets how much your screen tilts when strafing (polymost only)
* cl_shotgunaltfirereload
   - Enable/disable alt fire as reload for shotgun when fired one shell (always off in multiplayer)
* cl_showloadsavebackdrop
   - Enable/disable the menu backdrop for loading/saving game
* cl_slopecrosshair
   - Enable/disable adjusting crosshair position for slope tilting
* cl_slowroomflicker
   - Enable/disable slowed flickering speed for sectors (such as E1M4's snake pit room)
* cl_shadowsfake3d
   - Enable/disable 3D projection for fake sprite shadows
* cl_smoketrail3d
   - Enable/disable 3D smoke trail positioning for tnt/spray can (single-player only)
* cl_packitemswitch
   - Enable/disable item slot switching to activated item (always enabled in multiplayer)
* cl_projectileoldsprite
   - Enable/disable old pink sprite for hitscan projectiles
* cl_weaponfastswitch
   - Enable/disable fast weapon switching
* color
   - Set preferred player color palette in multiplayer (0: none, 1: blue, 2: red, 3: teal, 4: gray)
* crosshair
   - Enable/disable crosshair (0: off, 1: on, 2: on [autoaim])
* crosshairoffsetx
   - Set X axis offset for crosshair (-32 to 32)
* crosshairoffsety
   - Set Y axis offset for crosshair (-32 to 32)
* detail
   - Change the detail graphics setting (0-4)
* fly
   - Toggles fly mode
* hud_bgnewborder
   - Enable/disable new hud bottom border background image (only for r_size 5)
* hud_bgscale
   - Enable/disable hud background image scaling for resolution
* hud_bgvanilla
   - Enable/disable hud vanilla background image override (0: default, 1: use new tile, 2: use original tile)
* hud_stats
   - Set aspect ratio screen position for hud (0: native, 1: 4:3, 2: 16:10, 3: 16:9, 3: 21:9)
* hud_statsautomaponly
   - Enable/disable showing level statistics display only on map view
* hud_ratio
   - Enable/disable level statistics display (0: off, 1: on [default], 2: on [4:3], 3: on [16:10], 4: on [16:9], 5: on [21:9])
* hud_powerupduration
   - Enable/disable displaying the remaining time for power-ups (0: off, 1: on [default], 2: on [4:3], 3: on [16:10], 4: on [16:9], 5: on [21:9])
* hud_powerupdurationstyle
   - Set the display style for the remaining time for power-ups (0: nblood, 1: notblood)
* hud_powerupdurationticks
   - Set the tickrate divide value used for displaying the remaining time for power-ups (default: 100, realtime seconds: 120)
* hud_showendtime
   - Enable/disable displaying the level completion time on end screen
* hud_showweaponselect
   - Enable/disable weapon select bar display (0: none, 1: bottom, 2: top)
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
* mus_redbookfallback
   - Enables/disables redbook audio if midi song is not specified for level (mus_redbook must already be enabled)
* in_centerviewondrop
   - Enable/disable recenter view when dropping down onto ground
* in_crouchmode
   - Toggles crouch button (0:hold, 1:toggle)
* in_targetaimassist
   - enable/disable slowing camera movement when aiming towards a target (joystick only)
* notarget
   - Toggles AI player detection
* r_drawinvisiblesprites
   - Forcefully draw invisible sprites
* r_mirrormode
   - Mirror output display: 0: off 1: mirror horizontal 2: mirror vertically 3: mirror horizontal/vertically
* r_renderscale
   - Adjust internal rendering resolution by scale while keeping hud elements native to full resolution (only for software renderer)
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
* snd_ding
   - Enable/disable hit noise when damaging an enemy. The sound can be changed by replacing the 'notblood.pk3/NOTHIT.RAW' file
* snd_dingvol
   - Set volume for hit sound (default: 75, range: 1-255)
* snd_dingminfreq
   - Set min damage frequency for hit sound (default: 22050, range: 11025-44100)
* snd_dingmaxfreq
   - Set max damage frequency for hit sound (default: 22050, range: 11025-44100)
* snd_speed
   - Set the speed of sound m/s used for doppler calculation (default: 343, range: 10-1000)
* snd_occlusion
   - Enable/disable lowering sound volume by 50% for occluded sound sources
* team
   - Set preferred team in multiplayer (0: none, 1: blue, 2: red)
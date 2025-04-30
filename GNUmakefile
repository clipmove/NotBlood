#
# EDuke32 Makefile for GNU Make
#

### Platform and Toolchain Configuration
include Common.mak

### File Extensions
asm := nasm
o := o

### Directories
source := source
obj := obj

### Functions
# Some of this still needs work--"getfiltered" takes a list of files and strips the paths off of them,
# while the other functions end up expanding the result later by adding the paths back.
# This is inefficient, but it was a better use of time than reworking all of this at the moment.
define parent
$(word 1,$(subst _, ,$1))
endef
define expandobjs
$$(addprefix $$($$(call parent,$1)_obj)/,$$(addsuffix .$$o,$$(basename $$($1_objs) $$($1_rsrc_objs) $$($1_gen_objs))))
endef
define expandsrcs
$(addprefix $($(call parent,$1)_src)/,$($1_objs)) $(addprefix $($(call parent,$1)_rsrc)/,$($1_rsrc_objs)) $(addprefix $($(call parent,$1)_obj)/,$($1_gen_objs))
endef
define expanddeps
$(strip $1 $(foreach j,$1,$(call $0,$($j_deps))))
endef
define getdeps
$(call expanddeps,$1_$2 $(common_$2_deps) engine)
endef
define getfiltered
$(filter-out $(strip $($1_excl)), $(subst $($1_src)/, ,$(wildcard $($1_src)/$2)))
endef

##### External Library Definitions

#### libxmp-lite

libxmplite := libxmp-lite

libxmplite_root := $(source)/$(libxmplite)
libxmplite_src := $(libxmplite_root)/src
libxmplite_inc := $(libxmplite_root)/include
libxmplite_obj := $(obj)/$(libxmplite)

libxmplite_excl :=
libxmplite_objs := $(call getfiltered,libxmplite,*.c)

libxmplite_cflags := -DHAVE_ROUND -DLIBXMP_CORE_PLAYER -DLIBXMP_NO_PROWIZARD -DLIBXMP_NO_DEPACKERS -DBUILDING_STATIC -I$(libxmplite_inc)/libxmp-lite -Wno-unused-parameter -Wno-unused-variable -Wno-sign-compare -Wno-cast-qual


#### PhysicsFS

physfs := physfs

physfs_root := $(source)/$(physfs)
physfs_src := $(physfs_root)/src
physfs_inc := $(physfs_root)/include
physfs_obj := $(obj)/$(physfs)

physfs_excl :=

ifeq ($(PLATFORM),WINDOWS)
    physfs_excl += physfs_platform_unix.c
else
    physfs_excl += physfs_platform_windows.c
endif

ifneq (0,$(USE_PHYSFS))
    physfs_objs := $(call getfiltered,physfs,*.c)
else
    physfs_objs :=
endif

ifeq ($(PLATFORM),APPLE)
    physfs_objs += physfs_platform_apple.m
endif

physfs_cflags :=


#### glad

glad := glad

glad_root := $(source)/$(glad)
glad_src := $(glad_root)/src
glad_inc := $(glad_root)/include
glad_obj := $(obj)/$(glad)

glad_excl :=

ifneq ($(RENDERTYPE),WIN)
    glad_excl += glad_wgl.c
endif

ifneq (0,$(USE_OPENGL))
    glad_objs := $(call getfiltered,glad,*.c)
else
    glad_objs :=
endif

glad_cflags :=


#### mimalloc

mimalloc := mimalloc

mimalloc_root := $(source)/$(mimalloc)
mimalloc_src := $(mimalloc_root)/src
mimalloc_inc := $(mimalloc_root)/include
mimalloc_obj := $(obj)/$(mimalloc)

mimalloc_excl := \
    alloc-override.c \
    arena-abandon.c \
    free.c \
    page-queue.c \
    static.c \

mimalloc_objs := $(call getfiltered,mimalloc,*.c)
mimalloc_objs += prim/prim.c

mimalloc_cflags := -D_WIN32_WINNT=0x0600 -DMI_USE_RTLGENRANDOM -DMI_SHOW_ERRORS -fexceptions -Wno-cast-qual -Wno-unknown-pragmas -Wno-array-bounds -Wno-null-dereference -Wno-missing-field-initializers

ifeq (,$(filter 1 2 3 4 5 6 7,$(GCC_MAJOR)))
    mimalloc_cflags += -Wno-class-memaccess
endif


#### imgui

imgui := imgui

imgui_root := $(source)/$(imgui)
imgui_src := $(imgui_root)/src
imgui_inc := $(imgui_root)/include
imgui_obj := $(obj)/$(imgui)

imgui_excl :=

ifneq ($(RENDERTYPE),SDL)
    imgui_excl += imgui_impl_sdl2.cpp
endif
ifneq ($(RENDERTYPE),WIN)
    imgui_excl += imgui_impl_win32.cpp
endif

imgui_objs := $(call getfiltered,imgui,*.cpp)

imgui_cflags := -I$(imgui_inc) -Wno-cast-qual -Wno-cast-function-type -Wno-null-dereference -Wno-stringop-overflow


#### libsmackerdec

libsmackerdec := libsmackerdec

libsmackerdec_objs := \
    BitReader.cpp \
    FileStream.cpp \
    HuffmanVLC.cpp \
    LogError.cpp \
    SmackerDecoder.cpp \

libsmackerdec_root := $(source)/$(libsmackerdec)
libsmackerdec_src := $(libsmackerdec_root)/src
libsmackerdec_inc := $(libsmackerdec_root)/include
libsmackerdec_obj := $(obj)/$(libsmackerdec)

libsmackerdec_cflags :=


#### hmpplay

hmpplay := hmpplay

hmpplay_objs := \
    hmpplay.cpp \
    fmmidi3.cpp \

hmpplay_root := $(source)/$(hmpplay)
hmpplay_src := $(hmpplay_root)/src
hmpplay_inc := $(hmpplay_root)/include
hmpplay_obj := $(obj)/$(hmpplay)

hmpplay_cflags :=


##### Component Definitions

#### EBacktrace

ifndef ebacktrace_dll
    ebacktrace_dll := ebacktrace1.dll
    ifeq ($(findstring x86_64,$(COMPILERTARGET)),x86_64)
        ebacktrace_dll := ebacktrace1-64.dll
    endif
endif


#### BUILD Engine

engine := build

engine_root := $(source)/$(engine)
engine_src := $(engine_root)/src
engine_inc := $(engine_root)/include
engine_obj := $(obj)/$(engine)

engine_cflags :=

engine_deps :=

ifneq (1,$(SDL_TARGET))
    engine_deps += imgui
endif

ifneq (0,$(USE_PHYSFS))
    engine_deps += physfs
endif

ifneq (0,$(USE_MIMALLOC))
    engine_deps += mimalloc
endif

engine_editor_objs := \
    build.cpp \
    config.cpp \

engine_tools_objs := \
    colmatch.cpp \
    compat.cpp \
    crc32.cpp \
    klzw.cpp \
    kplib.cpp \
    loguru.cpp \
    lz4.cpp \
    pragmas.cpp \
    smmalloc.cpp \
    smmalloc_generic.cpp \
    smmalloc_tls.cpp \
    vfs.cpp \

engine_excl := \
    a-c.cpp \
    sdlayer12.cpp \
    sdlkeytrans.cpp \
    startgtk.editor.cpp \
    startwin.editor.cpp \
    $(engine_editor_objs) \

ifeq (1,$(USE_OPENGL))
    engine_deps += glad
else
    engine_excl += \
        glbuild.cpp \
        glsurface.cpp \
        mdsprite.cpp \
        polymer.cpp \
        polymost.cpp \
        tilepacker.cpp \
        voxmodel.cpp \

endif

ifeq ($(PLATFORM),WINDOWS)
    ifeq ($(STARTUP_WINDOW),1)
        engine_editor_objs += startwin.editor.cpp
    endif
else
    engine_excl += winbits.cpp
endif

ifeq ($(PLATFORM),WII)
    LINKERFLAGS += -Wl,-wrap,c_default_exceptionhandler
else
    engine_excl += wiibits.cpp
endif

ifneq ($(RENDERTYPE),SDL)
    engine_excl += sdlayer.cpp
endif
ifneq ($(RENDERTYPE),WIN)
    engine_excl += winlayer.cpp rawinput.cpp
endif

ifeq (1,$(HAVE_GTK2))
    ifeq ($(STARTUP_WINDOW),1)
        engine_editor_objs += startgtk.editor.cpp
    endif
else
    engine_excl += gtkbits.cpp dynamicgtk.cpp
endif

ifeq ($(USE_LIBVPX),0)
    engine_excl += animvpx.cpp
endif

engine_objs := \
    $(call getfiltered,engine,*.c) \
    $(call getfiltered,engine,*.cpp) \
    polymost1Frag.glsl \
    polymost1Vert.glsl \

ifeq (0,$(NOASM))
  engine_objs += a.nasm
else
  engine_objs += a-c.cpp
endif

ifeq ($(PLATFORM),DARWIN)
    engine_objs += osxbits.mm
    engine_tools_objs += osxbits.mm
    ifeq ($(STARTUP_WINDOW),1)
        engine_editor_objs += startosx.editor.mm
    endif
    ifeq ($(SDL_TARGET),1)
        ifneq ($(SDL_FRAMEWORK),0)
            engine_objs += SDLMain.mm
        endif
    endif
endif


#### mact

mact := mact

mact_root := $(source)/$(mact)
mact_src := $(mact_root)/src
mact_inc := $(mact_root)/include
mact_obj := $(obj)/$(mact)

mact_excl :=
mact_objs := $(call getfiltered,mact,*.cpp)

mact_cflags :=

#### AudioLib

audiolib := audiolib

audiolib_root := $(source)/$(audiolib)
audiolib_src := $(audiolib_root)/src
audiolib_inc := $(audiolib_root)/include
audiolib_obj := $(obj)/$(audiolib)

audiolib_deps :=

audiolib_excl := \
    music_external.cpp \

ifneq ($(PLATFORM),WINDOWS)
    audiolib_excl += driver_directsound.cpp driver_winmm.cpp
endif
ifneq ($(SUBPLATFORM),LINUX)
    audiolib_excl += driver_alsa.cpp
endif
ifneq ($(RENDERTYPE),SDL)
    audiolib_excl += driver_sdl.cpp
endif

audiolib_objs := $(call getfiltered,audiolib,*.cpp)

audiolib_cflags :=

ifneq (0,$(HAVE_XMP))
    audiolib_cflags += -I$(libxmplite_inc)
    audiolib_deps += libxmplite
endif


#### Tools

tools := tools

tools_objs := \
    compat_tools.cpp \

tools_root := $(source)/$(tools)
tools_src := $(tools_root)/src
tools_obj := $(obj)/$(tools)

tools_cflags := $(engine_cflags) -I$(engine_src)

tools_deps := engine_tools

ifneq (0,$(USE_MIMALLOC))
    tools_deps += mimalloc
endif

tools_targets := \
    arttool \
    bsuite \
    cacheinfo \
    generateicon \
    givedepth \
    ivfrate \
    kextract \
    kgroup \
    kmd2tool \
    map2stl \
    md2tool \
    mkpalette \
    transpal \
    unpackssi \
    wad2art \
    wad2map \

ifeq ($(PLATFORM),WINDOWS)
    tools_targets += enumdisplay getdxdidf
endif
ifeq ($(RENDERTYPE),SDL)
    tools_targets += makesdlkeytrans
endif


#### Depends

ifeq ($(SUBPLATFORM),LINUX)
    LIBS += -lFLAC -lasound
endif

ifeq ($(PLATFORM),BSD)
    LIBS += -lFLAC -lexecinfo
endif

ifeq ($(PLATFORM),DARWIN)
    LIBS += -lFLAC \
            -Wl,-framework,Cocoa -Wl,-framework,Carbon \
            -Wl,-framework,CoreMIDI -Wl,-framework,AudioUnit \
            -Wl,-framework,AudioToolbox -Wl,-framework,IOKit
    ifneq (00,$(DARWIN9)$(DARWIN10))
        LIBS += -Wl,-framework,QuickTime
    endif
endif

ifeq ($(PLATFORM),WINDOWS)
    LIBS += -lFLAC -ldsound
endif


#### Blood

blood := blood

blood_game_ldflags :=

blood_game_stripflags :=

blood_root := $(source)/$(blood)
blood_src := $(blood_root)/src
blood_rsrc := $(blood_root)/rsrc
blood_obj := $(obj)/$(blood)

blood_cflags := -I$(blood_src)

blood_game_deps := audiolib mact libsmackerdec

ifneq (0,$(NETCODE))
    blood_game_deps += enet
endif

blood_game := notblood

ifneq (,$(APPBASENAME))
    blood_game := $(APPBASENAME)
endif

blood_game_proper := NotBlood

blood_game_objs := \
	blood.cpp \
	actor.cpp \
	ai.cpp \
	aibat.cpp \
	aibeast.cpp \
	aiboneel.cpp \
	aiburn.cpp \
	aicaleb.cpp \
	aicerber.cpp \
	aicult.cpp \
	aigarg.cpp \
	aighost.cpp \
	aigilbst.cpp \
	aihand.cpp \
	aihound.cpp \
	aiinnoc.cpp \
	aipod.cpp \
	airat.cpp \
	aispid.cpp \
	aitchern.cpp \
	aizomba.cpp \
	aizombf.cpp \
	asound.cpp \
	barf.cpp \
	callback.cpp \
	chatpipe.cpp \
	choke.cpp \
	common.cpp \
	config.cpp \
	controls.cpp \
	credits.cpp \
	db.cpp \
	demo.cpp \
	dude.cpp \
	endgame.cpp \
	eventq.cpp \
	fire.cpp \
	fx.cpp \
	gamemenu.cpp \
	gameutil.cpp \
	getopt.cpp \
	gfx.cpp \
	gib.cpp \
	globals.cpp \
	gui.cpp \
	inifile.cpp \
	iob.cpp \
	levels.cpp \
	loadsave.cpp \
	map2d.cpp \
	menu.cpp \
	messages.cpp \
	mirrors.cpp \
	misc.cpp \
	network.cpp \
	osdcmd.cpp \
	player.cpp \
	qav.cpp \
	qheap.cpp \
	replace.cpp \
	resource.cpp \
	screen.cpp \
	sectorfx.cpp \
	seq.cpp \
	sfx.cpp \
	sound.cpp \
	tile.cpp \
	trig.cpp \
	triggers.cpp \
	view.cpp \
	warp.cpp \
	weapon.cpp \

ifeq ($(NOONE_EXTENSIONS),1)
    blood_game_objs += nnextsif.cpp
    blood_game_objs += nnexts.cpp
    blood_game_objs += nnextstr.cpp
    blood_game_objs += nnextcitem.cpp
    blood_game_objs += nnextslaser.cpp
    blood_game_objs += nnextcdud.cpp
    blood_game_objs += aicdud.cpp
endif

blood_game_rsrc_objs :=
blood_game_gen_objs :=

blood_game_miscdeps :=
blood_game_orderonlydeps :=

ifeq ($(PLATFORM),DARWIN)
    ifeq ($(STARTUP_WINDOW),1)
        blood_game_objs += startosx.game.mm
    endif
endif

ifeq ($(PLATFORM),WINDOWS)
    blood_game_rsrc_objs += gameres.rc
    ifeq ($(STARTUP_WINDOW),1)
        blood_game_objs += startwin.game.cpp
    endif
endif

ifeq (11,$(HAVE_GTK2)$(STARTUP_WINDOW))
    blood_game_objs += startgtk.game.cpp
    blood_game_gen_objs += game_banner.c
endif
ifeq ($(RENDERTYPE),SDL)
    blood_game_rsrc_objs += game_icon.c
endif


#### Includes

COMPILERFLAGS += \
    -MP -MMD \
    -I$(engine_inc) \
    -I$(mact_inc) \
    -I$(audiolib_inc) \
    -I$(glad_inc) \
    -I$(imgui_inc) \
    -I$(libsmackerdec_inc) \
    -I$(hmpplay_inc) \

ifneq (0,$(USE_MIMALLOC))
    COMPILERFLAGS += -I$(mimalloc_inc)
endif

ifneq (0,$(USE_PHYSFS))
    COMPILERFLAGS += -I$(physfs_inc) -DUSE_PHYSFS
endif

ifneq (0,$(MICROPROFILE))
  COMPILERFLAGS += -DMICROPROFILE_ENABLED=1
endif

##### Recipes

games := \
    blood \

libraries := \
    audiolib \
    engine \
    glad \
    imgui \
    libxmplite \
    mact \
    libsmackerdec \
    hmpplay \

ifneq (0,$(USE_MIMALLOC))
    libraries += mimalloc
endif

ifneq (0,$(USE_PHYSFS))
    libraries += physfs
endif

components := \
    $(games) \
    $(libraries) \
    tools \

roles := \
    game \
#    editor \


ifeq ($(PRETTY_OUTPUT),1)
.SILENT:
endif
.PHONY: \
    $(addprefix clean,$(games) test utils tools) \
    $(engine_obj)/rev.$o \
    all \
    clang-tools \
    clean \
    printtools \
    printutils \
    rev \
    start \
    veryclean \

.SUFFIXES:
.SECONDEXPANSION:


#### Targets

all: \
    blood \

start:
	$(BUILD_STARTED)

tools: $(addsuffix $(EXESUFFIX),$(tools_targets)) | start
	@$(call LL,$^)

$(games): $$(foreach i,$(roles),$$($$@_$$i)$(EXESUFFIX)) | start
	@$(call LL,$^)

ebacktrace: $(ebacktrace_dll) | start
	@$(call LL,$^)

ifeq ($(PLATFORM),WII)
ifneq ($(ELF2DOL),)
%$(DOLSUFFIX): %$(EXESUFFIX)
endif
endif


define BUILDRULE

$$($1_$2)$$(EXESUFFIX): $$(foreach i,$(call getdeps,$1,$2),$$(call expandobjs,$$i)) $$($1_$2_miscdeps) | $$($1_$2_orderonlydeps)
	$$(LINK_STATUS)
	$$(call MKDIR,"$$(obj)/$$($1_$2)_dump")
	$$(RECIPE_IF) $$(LINKER) $$(call LF,$$(obj)/$$($1_$2)_dump) -o $$@ $$^ $$(GUI_LIBS) $$($1_$2_ldflags) $$(LIBDIRS) $$(LIBS) $$(RECIPE_RESULT_LINK)
ifeq ($$(PLATFORM),WII)
ifneq ($$(ELF2DOL),)
	$$(ELF2DOL) $$@ $$($1_$2)$$(DOLSUFFIX)
endif
endif
ifneq ($$(STRIP),)
	$$(STRIP) $$@ $$($1_$2_stripflags)
endif
ifeq ($$(PLATFORM),DARWIN)
	cp -RPf "platform/Apple/bundles/$$($1_$2_proper).app" "./"
	$(call MKDIR,"$$($1_$2_proper).app/Contents/MacOS")
	cp -f "$$($1_$2)$$(EXESUFFIX)" "$$($1_$2_proper).app/Contents/MacOS/"
endif
	$$(call RMDIR,"$$(obj)/$$($1_$2)_dump")

endef

$(foreach i,$(games),$(foreach j,$(roles),$(eval $(call BUILDRULE,$i,$j))))


#### Rules

$(ebacktrace_dll): platform/Windows/src/backtrace.c
	$(COMPILE_STATUS)
	$(RECIPE_IF) $(CC) $(CONLYFLAGS) -O2 -ggdb -shared -Wall -Wextra -static-libgcc -I$(engine_inc) -o $@ $^ -lbfd -liberty -limagehlp $(RECIPE_RESULT_COMPILE)

libklzw$(DLLSUFFIX): $(engine_src)/klzw.cpp
	$(COMPILE_STATUS)
	$(RECIPE_IF) $(COMPILER_C) -shared -fPIC $< -o $@ $(RECIPE_RESULT_COMPILE)

# to debug the tools link phase, make a copy of this rule explicitly replacing % with the name of a tool, such as kextract
%$(EXESUFFIX): $(tools_obj)/%.$o $(foreach i,tools $(tools_deps),$(call expandobjs,$i))
	$(LINK_STATUS)
	$(call MKDIR,"$(tools_obj)/$*")
	$(RECIPE_IF) $(LINKER) $(call LF,$(tools_obj)/$*) -o $@ $^ $(LIBDIRS) $(LIBS) $(RECIPE_RESULT_LINK)
ifneq ($(STRIP),)
	$(STRIP) $@
endif
	$(call RMDIR,"$(tools_obj)/$*")


### Main Rules

define OBJECTRULES

include $(wildcard $($1_obj)/*.d)

$$($1_obj)/%.$$o: $$($1_src)/%.nasm | $$($1_obj)
	$$(COMPILE_STATUS)
	$$(RECIPE_IF) $$(AS) $$(ASFLAGS) $$< -o $$@ $$(RECIPE_RESULT_COMPILE)

$$($1_obj)/%.$$o: $$($1_src)/%.yasm | $$($1_obj)
	$$(COMPILE_STATUS)
	$$(RECIPE_IF) $$(AS) $$(ASFLAGS) $$< -o $$@ $$(RECIPE_RESULT_COMPILE)

$$($1_obj)/%.$$o: $$($1_src)/%.c | $$($1_obj)
	$$(COMPILE_STATUS)
	$$(call MKDIR,$$(dir $$@))
	$$(RECIPE_IF) $$(COMPILER_C) $$($1_cflags) -c $$< -o $$@ $$(RECIPE_RESULT_COMPILE)

$$($1_obj)/%.$$o: $$($1_src)/%.cpp | $$($1_obj)
	$$(COMPILE_STATUS)
	$$(call MKDIR,$$(dir $$@))
	$$(RECIPE_IF) $$(COMPILER_CXX) $$($1_cflags) -c $$< -o $$@ $$(RECIPE_RESULT_COMPILE)

$$($1_obj)/%.$$o: $$($1_src)/%.m | $$($1_obj)
	$$(COMPILE_STATUS)
	$$(call MKDIR,$$(dir $$@))
	$$(RECIPE_IF) $$(COMPILER_OBJC) $$($1_cflags) -c $$< -o $$@ $$(RECIPE_RESULT_COMPILE)

$$($1_obj)/%.$$o: $$($1_src)/%.mm | $$($1_obj)
	$$(COMPILE_STATUS)
	$$(call MKDIR,$$(dir $$@))
	$$(RECIPE_IF) $$(COMPILER_OBJCXX) $$($1_cflags) -c $$< -o $$@ $$(RECIPE_RESULT_COMPILE)

$$($1_obj)/%.$$o: $$($1_obj)/%.c | $$($1_obj)
	$$(COMPILE_STATUS)
	$$(call MKDIR,$$(dir $$@))
	$$(RECIPE_IF) $$(COMPILER_C) $$($1_cflags) -c $$< -o $$@ $$(RECIPE_RESULT_COMPILE)

$$($1_obj)/%.$$o: $$($1_src)/%.glsl | $$($1_obj)
	@echo Creating $$($1_obj)/$$(<F).cpp from $$<
	@$$(call RAW_ECHO,extern char const *$$(basename $$(<F));) > $$($1_obj)/$$(<F).cpp
	@$$(call RAW_ECHO,char const *$$(basename $$(<F)) = R"shader$$(paren_open)) >> $$($1_obj)/$$(<F).cpp
	@$$(call CAT,$$<) >> $$($1_obj)/$$(<F).cpp
	@$$(call RAW_ECHO,$$(paren_close)shader";) >> $$($1_obj)/$$(<F).cpp
	$$(COMPILE_STATUS)
	$$(call MKDIR,$$(dir $$@))
	$$(RECIPE_IF) $$(COMPILER_CXX) $$($1_cflags) -c $$($1_obj)/$$(<F).cpp -o $$@ $$(RECIPE_RESULT_COMPILE)

## Cosmetic stuff

$$($1_obj)/%.$$o: $$($1_rsrc)/%.rc | $$($1_obj)
	$$(COMPILE_STATUS)
	$$(call MKDIR,$$(dir $$@))
	$$(RECIPE_IF) $$(RC) -i $$< -o $$@ --include-dir=$$(engine_inc) --include-dir=$$($1_src) --include-dir=$$($1_rsrc) -DPOLYMER=$$(POLYMER) $(REVFLAG) $$(RECIPE_RESULT_COMPILE)

$$($1_obj)/%.$$o: $$($1_rsrc)/%.c | $$($1_obj)
	$$(COMPILE_STATUS)
	$$(call MKDIR,$$(dir $$@))
	$$(RECIPE_IF) $$(COMPILER_C) $$($1_cflags) -c $$< -o $$@ $$(RECIPE_RESULT_COMPILE)

$$($1_obj)/%_banner.c: $$($1_rsrc)/%.bmp | $$($1_obj)
	echo "#include \"gtkpixdata_shim.h\"" > $$@
	gdk-pixbuf-csource --extern --struct --raw --name=startbanner_pixdata $$^ | sed 's/load_inc//' >> $$@

endef

$(foreach i,$(components),$(eval $(call OBJECTRULES,$i)))


### Other special cases

# Comment out the following rule to debug a-c.o
$(engine_obj)/a-c.$o: $(engine_src)/a-c.cpp | $(engine_obj)
	$(COMPILE_STATUS)
	$(RECIPE_IF) $(subst -O$(OPTLEVEL),-O2,$(subst $(ASAN_FLAGS),,$(COMPILER_CXX))) $(engine_cflags) -c $< -o $@ $(RECIPE_RESULT_COMPILE)

$(engine_obj)/rev.$o: $(engine_src)/rev.cpp | $(engine_obj)
	$(COMPILE_STATUS)
	$(RECIPE_IF) $(COMPILER_CXX) $(engine_cflags) $(REVFLAG) -c $< -o $@ $(RECIPE_RESULT_COMPILE)


### Directories

$(foreach i,$(components),$($i_obj)):
	-$(call MKDIR,$@)

### Phonies

clang-tools: $(filter %.c %.cpp,$(foreach i,$(call getdeps,duke3d,game),$(call expandsrcs,$i)))
	echo $^ -- -x c++ $(CXXONLYFLAGS) $(COMPILERFLAGS) $(CWARNS) $(foreach i,$(components),$($i_cflags))

$(addprefix clean,$(games)):
	-$(call RM,$(foreach i,$(roles),$($(subst clean,,$@)_$i)$(EXESUFFIX)))
	-$(call RMDIR,$($(subst clean,,$@)_obj))
ifeq ($(PLATFORM),DARWIN)
	-$(call RMDIR,$(foreach i,$(roles),"$($(subst clean,,$@)_$i_proper).app"))
endif

cleantools:
	-$(call RM,$(addsuffix $(EXESUFFIX),$($(subst clean,,$@)_targets)))
	-$(call RMDIR,$($(subst clean,,$@)_obj))

clean: cleanblood cleantools
	-$(call RMDIR,$(obj))
	-$(call RM,$(ebacktrace_dll))

printtools:
	echo "$(addsuffix $(EXESUFFIX),$(tools_targets))"

rev: $(engine_obj)/rev.$o


### Compatibility

cleanutils: cleantools
printutils: printtools
utils: tools
veryclean: clean

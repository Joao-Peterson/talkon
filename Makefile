# ---------------------------------------------------------------
# Commands:
# 	build 		: build lib objects and test file for testing 
# 	release 	: build lib objects, archive and organize the lib files for use in the 'dist/' folder
# 	dist 		: dist just organizes the lib files for use in the 'dist/' folder
# 	clear 		: clear compiled executables
# 	clearall 	: clear compiled objects and lib files in 'build/' and 'dist/' folders as well as executables
# ---------------------------------------------------------------

# ---------------------------------------------------------------
# User area
# ---------------------------------------------------------------

EXE := main

SOURCES = main.c 
SOURCES += src/curses_extra.c 
SOURCES += src/strfmt.c 
SOURCES += src/config.c 
SOURCES += src/db.c 
SOURCES += src/net_discovery.c 
SOURCES += src/net_messaging.c 
SOURCES += src/inet_extra.c 
SOURCES += src/log.c 
SOURCES += src/tui.c 

RESOURCES = res/config.json 
RESOURCES += res/init.sql 

LIBS = -ldoc 
LIBS += -lplibsysstatic 
LIBS += -lm 
LIBS += -pthread
LIBS += -ldl
LIBS += -lsqlite3
LIBS += -luuid

BUILD_DIR := build/
DIST_DIR := dist/
RESOURCES_DIR += res/

CC := gcc
BUILD_C_FLAGS = -g 
RELEASE_C_FLAGS = -O2 
C_FLAGS =
I_FLAGS = -Iinc -I/usr/local/include/plibsys
L_FLAGS = -L./

# ---------------------------------------------------------------
# Do not alter anything below!
# ---------------------------------------------------------------

OBJS = $(SOURCES:.c=.o)
OBJS += $(addsuffix .o, $(RESOURCES))
OBJS_BUILD = $(addprefix $(BUILD_DIR), $(OBJS))

# OS detection
OSFLAG :=
ifeq ($(OS),Windows_NT)
#		OSFLAG += -D WIN32 
		I_FLAGS += -Ipdcurses
		LIBS += -lpdcurses -lws2_32
	ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
#		OSFLAG += -D AMD64
	endif
	ifeq ($(PROCESSOR_ARCHITECTURE),x86)
#		OSFLAG += -D IA32
	endif
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		LIBS += -lncurses -lpanel
#		OSFLAG += -D LINUX
	endif
	ifeq ($(UNAME_S),Darwin)
#		OSFLAG += -D OSX
	endif
		UNAME_P := $(shell uname -p)
	ifeq ($(UNAME_P),x86_64)
#		OSFLAG += -D AMD64
	endif
		ifneq ($(filter %86,$(UNAME_P)),)
#		OSFLAG += -D IA32
		endif
	ifneq ($(filter arm%,$(UNAME_P)),)
#		OSFLAG += -D ARM
	endif
endif

# ---------------------------------------------------------------

.PHONY : build

build : C_FLAGS += $(BUILD_C_FLAGS)
build : $(OBJS_BUILD) $(EXE)

release : C_FLAGS += $(RELEASE_C_FLAGS)
release : clearall $(OBJS_BUILD) $(EXE) dist

$(BUILD_DIR)%.o : %.c
	@mkdir -p $(dir $@)
	$(CC) $(C_FLAGS) $(I_FLAGS) -c $< -o $@

$(EXE): $(OBJS_BUILD) 
	$(CC) $(L_FLAGS) $^ $(LIBS) -o $@

dist : $(OBJS_BUILD)
	@mkdir -p $(DIST_DIR)

$(BUILD_DIR)$(RESOURCES_DIR)%.o : $(RESOURCES_DIR)%
	@mkdir -p $(dir $@)
	ld -r -b binary -o $@ $<

clear : 
	rm -f $(EXE)
	rm -f -r $(BUILD_DIR)*

clearall : clear
	rm -f -r $(DIST_DIR)*

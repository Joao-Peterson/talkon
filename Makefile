# ---------------------------------------------------------------
# User area
# ---------------------------------------------------------------

EXE := main
SOURCES = main.c
LIBS := lib/libdoc.a

BUILD_DIR := build/
DIST_DIR := dist/

CC := gcc
BUILD_C_FLAGS = -g
RELEASE_C_FLAGS = -O2
C_FLAGS =
I_FLAGS = -Iinc
L_FLAGS = -Llib

# ---------------------------------------------------------------
# Do not alter anything below!
# ---------------------------------------------------------------

OBJS := $(SOURCES:.c=.o)
OBJS_BUILD := $(addprefix $(BUILD_DIR), $(OBJS))

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
	$(CC) $^ $(LIBS) -o $@

dist : $(OBJS_BUILD)
	@mkdir -p $(DIST_DIR)

clear : 
	rm -f $(EXE)
	rm -f -r $(BUILD_DIR)*

clearall : clear
	rm -f -r $(DIST_DIR)*

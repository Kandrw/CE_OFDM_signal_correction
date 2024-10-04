

# SRC_DIR = src
# SRC = $(SRC_DIR)/main.cpp
# MAKE_BUILD = build.mk
# CC = g++
# CFLAGS = #-lpthread -g -Wall -O0 -lm
# EXT := cpp
# .PHONY: main
# all: main
# main:
# 	@echo "\n\t\tBuild DHCP server"
# 	$(MAKE) -f $(MAKE_BUILD) EXT="cpp" CC="$(CC)" SRC="$(SRC)" PROGRAM="program" CFLAGS="$(CFLAGS)" EXT="$(EXT)"
THREAD_OBJ = -j16
CC := g++
EXT := cpp
PROJECT := Program
WIN := .exe
RM := rm
CFLAGS := -liio \
	-Bstatic -static-libgcc -static-libstdc++ -lm -g -Wall -pthread

#	-Wl, -Bstatic -static-libgcc -static-libstdc++ -lm -g -Wall
# SRC := src/main.cpp 
SRC := src/main.cpp \
	src/generate_packet/generate_packet.cpp \
	src/loaders/load_data.cpp \
	src/output/output.cpp

SRC += src/phy/phy.cpp \
	src/phy/modulation.cpp

SRC += src/trx/device_api.cpp

SRC += src/trx/ad9361/device_ad9361.cpp

SRC += src/model/modelling.cpp src/phy/types.cpp

BUILD_DIRECT := build

OBJ := $(patsubst %.$(EXT), %.o, $(SRC))

VAR_DEFINE = -DFILE_NAME_SAVE_MODULATION="data/1modulation.bin"
.PHONY: $(SRC)
all: clean main #$(SRC)

main: $(OBJ)

#	$(CC) -o $(PROJECT) $(addprefix $(BUILD_DIRECT)/, $(notdir $(OBJ))) $(CFLAGS)
	$(CC) -o $(PROJECT) $(addprefix $(BUILD_DIRECT)/, $(notdir $(OBJ))) $(CFLAGS)
	
	@echo End build: $(PROJECT)

%.o: %.cpp
#	$(CC) $< -c -o $(BUILD_DIRECT)/$(notdir $@)
	$(CC) $< -liio -c -o $(BUILD_DIRECT)/$(notdir $@)
	

clean:
#	@echo $(OBJ)
#	$(RM) $(PROJECT)$(WIN)
	-$(RM) $(PROJECT)

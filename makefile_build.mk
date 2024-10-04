
BIN := main
BUILD_DIRECT = build
ROOT_DIR := src
DIRS := $(sort $(dir $(wildcard $(ROOT_DIR)/*/*/)))
SRC := $(wildcard $(addsuffix *.cpp, $(DIRS))) $(ROOT_DIR)/main.cpp
OBJ := $(patsubst $(ROOT_DIR)%.cpp, $(ROOT_DIR)%.o, $(SRC))
CC := g++

#.PHONY: all 
all: $(BIN)

$(BIN): $(OBJ)
#	g++ -o Program $(BUILD_DIRECT)/*.o -lglfw3 -lglew32 -lopengl32 -lspng -lgdi32 -Wl,-Bstatic -lz -static-libgcc -static-libstdc++ -lm -g -Wall
	$(CC) -v -o Program $(BUILD_DIRECT)/*.o -lopengl32 -lglew32 -lfreeglut -lglu32 -lglfw3 -lglew32 -lopengl32 -lgdi32 -Wl,-Bstatic -static-libgcc -static-libstdc++ -lm -g -Wall
	
%.o: %.cpp
	$(CC) $< -c -o $(BUILD_DIRECT)/$(notdir $@)



info:
	@echo ROOT_DIR
	@echo $(ROOT_DIR)
	@echo DIRS
	@echo $(DIRS)
	@echo SRC
	@echo $(SRC)
	@echo OBJ
	@echo $(OBJ)


SRC := 
PROGRAM :=
EXT := c
OBJ := $(patsubst %.$(EXT), %.o, $(SRC))
CC :=
.PHONY: clear $(SRC) main clear_end
all: clear $(SRC) main clear_end

$(SRC):
	$(CC) -c $@

clear:
	-rm $(OBJ)
	-rm $(PROGRAM)
clear_end:
	-rm $(OBJ)

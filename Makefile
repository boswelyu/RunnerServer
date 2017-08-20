CC = g++
CFLAGS = -Wno-deprecated -I./src/utils/ -I./src/include/
EXEC_NAME = udpserver
OBJ_PATH = obj
BIN_PATH = bin
SRC_MAINPATH = src
SRC_PATHS = src src/utils

SOURCES = $(foreach x,$(SRC_PATHS),$(wildcard $(addprefix $(x)/*,.cpp .c)))

OBJECTS = $(addprefix $(OBJ_PATH)/,$(patsubst src/%.cpp,%.o,$(SOURCES)))

$(BIN_PATH)/udpserver : $(OBJECTS)
	$(CC) $(CFLAGS) -o ${BIN_PATH}/$(EXEC_NAME) $(OBJECTS)

#$(OBJ_PATH)/%.o : $(SRC_PATH)/%.cpp
#	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_PATH)/%.o : $(SRC_MAINPATH)/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<


.PHONY:clean
clean:
	rm -rf $(BIN_PATH)/* $(OBJ_PATH)/*

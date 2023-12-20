CC = gcc
ARGS = -g

PREF_SRC = ./src/
PREF_OBJ = ./obj/
PREF_BIN = ./bin/

SRC_CLIENT = $(wildcard $(PREF_SRC)src-client/*.c)
OBJ_CLIENT = $(patsubst $(PREF_SRC)src-client/%.c, $(PREF_OBJ)%.o, $(SRC_CLIENT))

SRC_SERVER = $(wildcard $(PREF_SRC)src-server/*.c)
OBJ_SERVER = $(patsubst $(PREF_SRC)src-server/%.c, $(PREF_OBJ)%.o, $(SRC_SERVER))

client: $(OBJ_CLIENT) $(PREF_OBJ)logger.o
	$(CC) $(ARGS) $^ -o $(PREF_BIN)$@ 

$(PREF_OBJ)%.o: $(PREF_SRC)src-client/%.c
	$(CC) $(ARGS) -c $^ -o $@

server: $(OBJ_SERVER) $(PREF_OBJ)logger.o
	$(CC) $(ARGS) $^ -o $(PREF_BIN)$@ 

$(OBJ_SERVER): $(SRC_SERVER)
	$(CC) $(ARGS) -c $^ -o $@

$(PREF_OBJ)logger.o: $(PREF_SRC)logger.c
	$(CC) $(ARGS) -c $^ -o $@

clean: 
	rm -vrI $(PREF_BIN)* $(PREF_OBJ)* 

clog: 
	rm -v src/logs/*.txt

all: 
	@echo $(SRC_CLIENT)
	@echo $(OBJ_CLIENT)
	@echo $(SRC_SERVER)
	@echo $(OBJ_SERVER)

APP := 2dplayground
SRC_DIR := ./src ./src/snake
SRC := $(foreach n, $(SRC_DIR), $(wildcard $(n)/*.c))
OBJ := $(SRC:.c=.o)

CFLAGS := $(foreach n, $(SRC_DIR), $(addprefix -I, $(n)))
CFLAGS += -Wall -Wno-unused-function -Wno-stringop-overread -g
LDFLAGS := -lgdi32 -lpthread -lWinmm -lws2_32

all: $(APP)

$(OBJ):$(SRC)

$(APP): $(OBJ)
	gcc -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(APP) $(OBJ)

PHONY = clean
.PHONY:$(PHONY)

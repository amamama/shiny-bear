CC		:= gcc

CFLAGS		:= -ansi -std=c99 -W -Wall -Werror -O2
LDFLAGS		:= -lcurl -loauth #-lcrypto

SRC_DIR		:= ./src
SRCS			:= $(notdir $(wildcard $(SRC_DIR)/*.c))

INC_DIR		:= ./include
INCS		:= $(addprefix $(INC_DIR)/, $(SRCS:.c=.h))

OBJ_DIR		:= ./obj
OBJS		:= $(addprefix $(OBJ_DIR)/, $(SRCS:.c=.o))

DEP_DIR		:= ./obj
DEPS		:= $(addprefix $(DEP_DIR)/, $(SRCS:.c=.d))

TARGET_DIR	:= ./bin
TARGET		:= $(TARGET_DIR)/tweet 



all : $(TARGET)

run : $(TARGET)
	$(TARGET)

#-include $(DEPS)


$(TARGET) : $(OBJS) main.c
	@[ -d `dirname $@` ] || mkdir -p `dirname $@`
	$(CC) main.c $(OBJS) $(LDFLAGS) -o $(TARGET)


$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c Makefile
	@[ -d `dirname $@` ] || mkdir -p `dirname $@`
	$(CC) -c -MMD -MP $< $(CFLAGS) -I$(INC_DIR) -o $@


clean :		
	rm -f -r $(TARGET_DIR) $(OBJ_DIR)


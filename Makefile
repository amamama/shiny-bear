CC		:= gcc

CFLAGS		:= -ansi -std=c99 -W -Wall -Werror -O2
LDFLAGS		:= -lcurl -loauth -lcrypto

SOURCES_DIR	:= .
SOURCES		:= $(wildcard $(SOURCES_DIR)/*.c)

OBJ_DIR		:= ./obj
OBJS		:= $(addprefix $(OBJ_DIR)/, $(subst ./,,$(patsubst %.c,%.o,$(SOURCES))))

DEP_DIR		:= ./obj
DEPENDS		:= $(addprefix $(DEP_DIR)/, $(subst ./,,$(patsubst %.c,%.d,$(SOURCES))))

TARGET_DIR	:= ./bin
TARGET		:= $(TARGET_DIR)/tweet 



all : $(TARGET)

run : $(TARGET)
	$(TARGET)

-include $(DEPENDS)


$(TARGET) : $(OBJS)
	@[ -d `dirname $@` ] || mkdir -p `dirname $@`
	$(CC) $(OBJS) $(CFLAGS) $(INCLUDE) $(LDFLAGS) -o $(TARGET)


$(OBJ_DIR)/%.o : $(SOURCES_DIR)/%.c Makefile
	@[ -d `dirname $@` ] || mkdir -p `dirname $@`
	$(CC) -c -MMD -MP $< $(CFLAGS) $(INCLUDE) $(LDFLAGS) -o $@ 


clean :		
	rm -f -r $(TARGET_DIR) $(OBJ_DIR)


THIS_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
EVSET_DIR := ../evsets
UTILS_DIR := ../utils

##################
# Build directory
##################
OBJDIR  = obj
CFLAGS += -O3 -I./$(OBJDIR) 
CFLAGS += -I$(EVSET_DIR)/$(OBJDIR)
# CFLAGS += -Wall 

####################
# Files and folders
####################
SRCS_BASIC  = main.c receiver.c transmitter.c 

OBJS_BASIC = $(addprefix $(OBJDIR)/,$(patsubst %.c,%.o,$(SRCS_BASIC)))

OBJS_EVS   = $(addprefix $(EVSET_DIR)/$(OBJDIR)/,$(notdir $(patsubst %.c,%.o,$(shell find $(EVSET_DIR)/*.c $(EVSET_DIR)/list/*.c))))
OBJS_UTL   = $(addprefix $(UTILS_DIR)/$(OBJDIR)/,$(notdir $(patsubst %.c,%.o,$(shell find $(UTILS_DIR)/*.c))))

##########
# Targets
##########
all: clean utils evsets channel

# Compilation targets

channel: $(OBJS_BASIC) $(OBJS_EVS) $(OBJS_UTL)
	$(CC) -o $@ $^ $(LDFLAGS) -lrt -lm

utils:
	$(MAKE) -C $(UTILS_DIR) 

evsets:
	$(MAKE) -C $(EVSET_DIR) CONFIG_FILE=$(THIS_DIR)/configuration.h

$(OBJDIR)/%.o: %.c | objdir
	$(CC) $(CFLAGS) -c $< -o $@

objdir:
	@mkdir -p $(OBJDIR) 

# Execution targets

dirs:
	mkdir -p log

run: dirs
	./channel
	@make stat

stat:
	python3 analyze.py

kill:
	killall channel

# Cleaning targets

clean:
	rm -rf channel 
	rm -rf $(OBJDIR) log/*
	$(MAKE) clean -C $(EVSET_DIR)/
	$(MAKE) clean -C $(UTILS_DIR)/

.PHONY: all clean

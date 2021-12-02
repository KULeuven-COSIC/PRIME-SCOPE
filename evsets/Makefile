THIS_DIR  := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
UTILS_DIR := ../utils

##################
# Build directory
##################
OBJDIR = obj
CFLAGS = -I./$(OBJDIR) -fPIC -std=gnu99

####################
# Files and folders
####################

SRCS_PRIME = 	  		   \
	ps_evset_noninc.c    \
	ps_evset_inc.c       \
	list/list_utils.c 	 \
	list/list_traverse.c 

OBJS_PRIME = $(addprefix $(OBJDIR)/,$(patsubst %.c,%.o,$(SRCS_PRIME)))

##########
# Targets
##########

all: checkconf $(OBJS_PRIME)

$(OBJDIR)/%.o: %.c | objdir
	$(CC) $(CFLAGS) -c $< -include $(CONFIG_FILE) -o $(OBJDIR)/$(notdir $@)

objdir:
	@mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR)

checkconf:
ifndef CONFIG_FILE
	$(error CONFIG_FILE is needed, e.g. 'make CONFIG_FILE=./configuration.h')
endif
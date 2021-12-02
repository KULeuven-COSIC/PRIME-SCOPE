THIS_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

##################
# Build directory
##################
OBJDIR = obj
CFLAGS = -I./$(OBJDIR) -fPIC

####################
# Files and folders
####################
SRCS = $(shell find ./*.c)
OBJS = $(addprefix $(OBJDIR)/,$(patsubst %.c,%.o,$(SRCS)))

##########
# Targets
##########
all: $(OBJS)

$(OBJDIR)/%.o: %.c | objdir
	$(CC) $(CFLAGS) -c $< -o $@

objdir:
	@mkdir -p $(OBJDIR) 

clean:
	rm -rf $(OBJDIR)
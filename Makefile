# Author: Don Stringham <donstringham@weber.edu>
.DEFAULT_GOAL=clean

# VARIABLES
CC=gcc
BUILD_TIME ?= $(shell date +%FT%T%z)
SDIR=./src
ODIR=$(SDIR)
BDIR=./bin

# TARGETS
bld: 
	[ -d $(BDIR) ] || mkdir -p $(BDIR) 
	$(CC) $(CFLAGS) $(SDIR)/myshell.c -o $(BDIR)/mysh -lreadline
run:
	$(BDIR)/mysh

release:
	git tag v$(V)
	@read -p "Press enter to confirm and push to origin ..." && git push origin v$(V)

.PHONY: clean
clean:
	-rm -r $(BDIR)
	-rm -f $(ODIR)/*.o

# this includes the framework configuration
-include .config

# decide whether we are building or dooing something other like cleaning or configuring
ifeq '' '$(filter $(MAKECMDGOALS), clean distclean config)'
  # check whether a .config file has been found
  ifeq '' '$(filter .config, $(MAKEFILE_LIST))'
    $(error "Cannot make the target '$(MAKECMDGOALS)' without configuring the application. Please run make config to do this.")
  endif
endif

# Compile options
PEDANTIC = # -pedantic

# Host-Compiler executables and flags
HOST_CC = gcc 
HOST_CFLAGS = $(HOST_FEATURES) -Wall $(PEDANTIC) -D OSC_HOST
HOST_LDFLAGS = -lm

# Cross-Compiler executables and flags
TARGET_CC = bfin-uclinux-gcc 
TARGET_CFLAGS = -Wall $(PEDANTIC) -O2 -D OSC_TARGET
TARGET_LDFLAGS = -Wl,-elf2flt="-s 1048576" -lbfdsp

# Source files of the application
SOURCES = main.c process.c config.c valves.c modbus.c

# Default target
all: leanxsugus leanxsugus_host cgi/www.tar.gz

# Compiles the executable
leanxsugus: $(SOURCES) oscar/staging/inc/* oscar/staging/lib/*
	$(TARGET_CC) $(SOURCES) oscar/staging/lib/libosc_target.a $(TARGET_CFLAGS) $(TARGET_LDFLAGS) -o $@

leanxsugus_host: $(SOURCES) oscar/staging/inc/* oscar/staging/lib/*
	$(HOST_CC) $(SOURCES) oscar/staging/lib/libosc_host.a $(HOST_CFLAGS) $(HOST_LDFLAGS) -o $@

cgi/www.tar.gz:
	make -C cgi www.tar.gz

.PHONY: doc
doc:
	rm -rf doc/{html,latex}
	doxygen doc/documentation.doxygen
	ln -sf html/index.html doc/index.html

# Target to explicitly start the configuration process
.PHONY: config
config:
	$(MAKE) -C oscar config
	@ ./configure

# This enables the automatic building of the oscar framework
oscar/staging/inc/* oscar/staging/lib/*:
	make oscar

# Set symlinks to the framework
.PHONY: oscar
oscar:
	make -C oscar

# deploying to the device
.PHONY: deploy
deploy: leanxsugus cgi/www.tar.gz
	- scp -p runapp.sh leanxsugus cgi/www.tar.gz $(CONFIG_TARGET_IP):/mnt/app/
# for some reason, scp exits with 1 after a success (as it does when something goes wrong).

.PHONY: run
run: | deploy
	ssh $(CONFIG_TARGET_IP) /mnt/app/runapp.sh

# Cleanup
.PHONY: clean
clean:	
	rm -f leanxsugus leanxsugus_host
	rm -rf doc/{html,latex,index.html}
	rm -f *.o *.gdb
	$(MAKE) -C cgi clean
	$(MAKE) -C oscar clean

# Cleans everything not intended for source distribution
.PHONY: distclean
distclean: clean
	rm -f .config
	$(MAKE) -C oscar distclean

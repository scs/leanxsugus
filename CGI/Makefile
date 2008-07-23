# The executable name is suffix depending on the target
OUT = leanxsugus.cgi
HOST_SUFFIX = _host
TARGET_SUFFIX = _target

# Host-Compiler executables and flags
HOST_CC = gcc 
HOST_CFLAGS = $(HOST_FEATURES) -Wall -pedantic -DOSC_HOST -g
HOST_LDFLAGS = -lm

# Cross-Compiler executables and flags
TARGET_CC = bfin-uclinux-gcc 
TARGET_CFLAGS = -Wall -pedantic -O2 -DOSC_TARGET
TARGETDBG_CFLAGS = -Wall -pedantic -ggdb3 -DOSC_TARGET
TARGET_LDFLAGS = -Wl,-elf2flt="-s 1048576"

# Source files of the application
SOURCES =

# Files served by the web server
WEB_FILES = www/*

# Default target
all: $(OUT)

$(OUT): target host

# Compiles the executable
target: $(SOURCES) ../inc/*.h ../lib/libosc_target.a $(WEB_FILES)
	@ tar -c -C www . | gzip > www.tar.gz
	
host: $(SOURCES) ../inc/*.h ../lib/libosc_host.a  $(WEB_FILES)
#	@echo "Compiling for host.."
#	$(HOST_CC) $(SOURCES) ../lib/libosc_host.a $(HOST_CFLAGS) \
#	$(HOST_LDFLAGS) -o $(OUT)$(HOST_SUFFIX)
#	@echo "Host executable done."
#	@cp $(OUT)$(HOST_SUFFIX) $(OUT)
	@cp $(OUT) www/cgi-bin
#	cp www/cgi-bin/* /var/www/cgi-bin/
#	@chmod a+x /var/www/cgi-bin/$(OUT)
#	cp  www/* /var/www -r
	
# Cleanup
clean:	
	rm -f $(OUT)$(HOST_SUFFIX) $(OUT)$(TARGET_SUFFIX) $(OUT)
	rm -f *.o *.cgi *.gdb
	rm -f www/cgi-bin/$(OUT)
	rm -f www.tar.gz
	@ echo "Directory cleaned"

	

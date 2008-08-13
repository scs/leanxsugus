# Default target
all: target host

# Compiles the executable
target: www/cgi-bin/config $(shell find www) $(WEB_FILES)
	tar -c -C www . | gzip > www.tar.gz

host: $(SOURCES) ../inc/*.h ../lib/libosc_host.a  $(WEB_FILES)
	cp $(OUT) www/cgi-bin
	cp -r www/* /var/www/

# Cleanup
clean:
	rm -f www.tar.gz
	@ echo "Directory cleaned"

# Default target
all: target host

# Compiles the executable
www.tar.gz: www/cgi-bin/config $(shell find www)
	tar -c -C www . | gzip > $@

www/cgi-bin/config:
	[ -p $@ ] || mkfifo $@

# Cleanup
clean:
	rm -f www.tar.gz
	@ echo "Directory cleaned"

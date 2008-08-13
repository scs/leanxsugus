# Default target
all: target host

# Compiles the executable
target: www/cgi-bin/config $(shell find www)
	tar -c -C www . | gzip > www.tar.gz

host: www/cgi-bin/config
	cp $(OUT) www/cgi-bin
	cp -r www/* /var/www/

www/cgi-bin/config:
	mkfifo $@

# Cleanup
clean:
	rm -f www.tar.gz
	@ echo "Directory cleaned"

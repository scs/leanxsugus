#! /bin/msh

# Kill all stray process instances.
killall leanxsugus 2> /dev/null

# Construct a fifo that will be used by the application to recieve configuration commands.
echo "Setting up the configuration fifo..."
if [ ! -p /tmp/leanxsugus-config ]; then
	rm -f /tmp/leanxsugus-config
	mkfifo /tmp/leanxsugus-config
fi

# Copy the web interface to the http server's root directory.
echo "Setting up the web interface files..."
rm -rf /home/httpd/*
gzip -d < /app/www.tar.gz | tar -x -C /home/httpd/

# Run the application
echo "Running the application..."
/app/leanxsugus
echo "The application ended with an exit status of $?."

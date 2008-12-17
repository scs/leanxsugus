#! /bin/msh

# Kill all stray process instances.
killall leanxsugus 2> /dev/null

# Copy the web interface to the http server's root directory.
echo "Setting up the web interface files..."
rm -rf /home/httpd/*
gzip -d < /mnt/app/www.tar.gz | tar -x -C /home/httpd/

# Run the application
echo "Running the application..."
/mnt/app/leanxsugus
echo "The application quit with an exit status of $?."

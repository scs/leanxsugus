#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <stdbool.h>

#define BAUDRATE B115200
#define MODEMDEVICE "/dev/ttyUSB0"

#define loop while (true)

main() {
//	FILE * fd;
	int fd;
	struct termios oldtio, newtio;
	char buf[255];
	
	fd = open(MODEMDEVICE, O_WRONLY | O_DSYNC | O_NOCTTY); 
	if (fd == -1) {
		perror(MODEMDEVICE);
		return -1;
	}
	
	tcgetattr(fd, &oldtio); /* save current port settings */
	
	bzero(&newtio, sizeof newtio);
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD | PARENB;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	
	/* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;
	 
	newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
	newtio.c_cc[VMIN] = 1; /* blocking read until 5 chars received */
	
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);
	
	loop { /* loop for input */
		unsigned char const inp = getchar();
		
		if (feof(stdin))
			break;
		
		write(fd, &inp, 1);
		printf("0x%x\n", inp);
	}
	
	tcsetattr(fd, TCSANOW,&oldtio);
	close(fd);
}
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>

int wdc_fd;

// -----------------------------------------------------------------------
void serial_setup(int fd)
{
	struct termios options;

	// 9600 baud
	cfsetispeed(&options, B19200);
	cfsetospeed(&options, B19200);

	// enable receiver, local mode
	options.c_cflag |= (CLOCAL | CREAD);

	// 8N1
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;

	// disable hardware flow control
	options.c_cflag &= ~CRTSCTS;

	// parity checking
	options.c_iflag &= ~(INPCK | ISTRIP);

	// disable software flow control
	options.c_iflag &= ~(IXON | IXOFF | IXANY);

	// raw mode
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	// don't map this shit
	options.c_iflag &= ~(INLCR | ICRNL | IGNCR | IUCLC);

	// no postprocess
	options.c_oflag &= ~(OPOST | ONLCR | OCRNL | OLCUC | ONLRET | OFILL );

	tcsetattr(fd, TCSANOW, &options);
}

// -----------------------------------------------------------------------
bool transmit(int fd, char *c)
{
	char inbuf[1024 + 1] = { 0 };

	printf(" WDC CMD: %s\n", c);
	write(fd, c, strlen(c));

	int res = 0;
	while (res < 5) {
		res += read(fd, inbuf+res, 1);
		usleep(1000);
	}

	printf(" WDC RESP: %s", inbuf);

	if (!strncmp(inbuf, "OK!", 3)) {
		return true;
	}
	return false;
}

// -----------------------------------------------------------------------
bool wdc_ready()
{
	return transmit(wdc_fd, "s\r");
}

// -----------------------------------------------------------------------
bool wdc_track0()
{
	return transmit(wdc_fd, "t\r");
}

// -----------------------------------------------------------------------
bool wdc_step_in()
{
	return transmit(wdc_fd, "i\r");
}

// -----------------------------------------------------------------------
bool wdc_step_out()
{
	return transmit(wdc_fd, "o\r");
}

// -----------------------------------------------------------------------
bool wdc_seek(unsigned int cyl)
{
	char buf[10];
	snprintf(buf, 10, "c%i\r", cyl);
	return transmit(wdc_fd, buf);
}

// -----------------------------------------------------------------------
bool wdc_set_drive(int d)
{
	char buf[10];
	snprintf(buf, 10, "d%i\r", d);
	return transmit(wdc_fd, buf);
}

// vim: tabstop=4

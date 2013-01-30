extern int wdc_fd;

extern int drive;
extern int cylinder;
extern int head;

void serial_setup(int fd);
bool transmit(int fd, char *c);
bool wdc_ready();
bool wdc_track0();
bool wdc_step_in();
bool wdc_step_out();
bool wdc_seek(unsigned int cyl);
bool wdc_set_drive(int d);
bool wdc_set_head(int h);

// vim: tabstop=4

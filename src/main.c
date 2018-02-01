#include <libserialport.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define BAUD_RATE   19200
#define BUFF_SIZE   512

unsigned char replay_array[] =
{0x54, 0x45, 0x53, 0x54, 0x0D};

struct sp_port *port;
char *serial_port_name;
char *message = "Hi there\r";

void
write_array()
{
    int ret = sp_nonblocking_write(port, message, strlen(message));
    printf("Number of bytes sent: %d\n", ret);
    if (ret < 0)
        fprintf(stderr, "Unable to write to serial port %s\n", serial_port_name);
}

void
print_buffer(unsigned char *byte_buff, int num_read) {
    for (int i = 0; i < num_read; i++)
        printf("%c" , byte_buff[i]);
}

void
print_usage(char *command)
{
    fprintf(stderr, "Usage: %s <serial device>\n", command);
}

void
print_banner()
{
    puts("       TJREVERB CubeSat 2018              ");
    puts(" ______   _____  _____   _________  ___   ");
    puts("/_  __/_ / / _ \\/ __/ | / / __/ _ \\/ _ )");
    puts(" / / / // / , _/ _/ | |/ / _// , _/ _  |  ");
    puts("/_/  \\___/_/|_/___/ |___/___/_/|_/____/  ");
    puts("");
    puts("Thomas Jefferson HS & George Mason Univ.  ");
    printf("Version 0.1 compiled %s %s\n", __DATE__, __TIME__);
}
int
main(int argc, char **argv)
{
    unsigned char byte_buff[BUFF_SIZE] = {0};

    int bytes_waiting = 0;
    int num_read = 0;

    print_banner();

    if (argc < 2) {
        print_usage(argv[0]);
        return -1;
    }
    if (strcmp(argv[2],"heartbeat")==0)
    {
      printf("HEARTBEAT MODE\n");
    }
    serial_port_name = argv[1]; // WARNING, No error checking here

    enum sp_return ret = sp_get_port_by_name(serial_port_name, &port);
    if (ret != SP_OK) {
        fprintf(stderr, "Unable to get serial port %s\n", serial_port_name);
        return -1;
    }

    ret = sp_open(port,SP_MODE_READ_WRITE);
    if (ret != SP_OK) {
        fprintf(stderr, "Unable to open serial port %s\n", serial_port_name);
        return -1;
      }
    /*ret = sp_open(port,SP_MODE_WRITE);
    if (ret != SP_OK) {
        fprintf(stderr, "Unable to open serial port %s\n", serial_port_name);
        return -1;
    }*/

    ret = sp_set_baudrate(port,BAUD_RATE);
    if (ret != SP_OK) {
        fprintf(stderr, "Unable to set the baud rate to: %d\n", BAUD_RATE);
        return -1;
    }

    while(1) {
        bytes_waiting = sp_input_waiting(port);
        if (bytes_waiting > 0) {
            num_read = sp_nonblocking_read(port,byte_buff, sizeof byte_buff);
            print_buffer(byte_buff,num_read);
        }
    }

    while (0) {
        printf("Sending message \n");
        write_array();
        sleep(2);
    }

    return 0;
}

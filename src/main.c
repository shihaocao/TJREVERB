#include <libserialport.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
//for patrick
#include <pthread.h>
#include <stdlib.h>

#define BAUD_RATE   19200
#define BUFF_SIZE   512

unsigned char replay_array[] =
{0x54, 0x45, 0x53, 0x54, 0x0D};

struct sp_port *port;
char *serial_port_name;
int bytes_waiting = 0;
int num_read = 0;

unsigned char byte_buff[BUFF_SIZE] = {0};

FILE *file;



void
write_array(char *message)
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


void *downlink(void *args){
    char string[99];
    
    while(1){
        scanf("%99s", string);
        printf("Downlink: %s\n", string);
        strcat(string,"\n");
        write_array(string);
    }
    
}

void *standard(void *args){
	unsigned char mybyte_buff[BUFF_SIZE] = {0};

    	int bytes_waiting = 0;
    	int num_read = 0;
	time_t rawtime;
      	time ( &rawtime );
      	time_t oldtime = rawtime; 
      	time ( &oldtime);
	while(1){
		time(&rawtime);
		//printf("listening\n");
		//sleep(1);
		bytes_waiting = sp_input_waiting(port);
		//update current time
		//time ( &rawtime );
		if (bytes_waiting > 0) {
	      		num_read = sp_blocking_read(port,mybyte_buff, sizeof mybyte_buff,500);
			//printf("read %d bytes\n",num_read);
	     		print_buffer(mybyte_buff,num_read);
			
			char* msg = "noop";
			char retmsg[100];
			
			if(strcmp("noop",msg)==0){
				snprintf(retmsg,sizeof retmsg,"IM ALIVE: %ld\n",rawtime);
			}
			else if(strcmp("gettime",msg)==0){
				snprintf(retmsg,sizeof retmsg,"THIS IS THE TIME: %ld\n",rawtime);
			}
			else{
				snprintf(retmsg,sizeof retmsg,"404 %ld\n",rawtime);
			}
			printf("Sending Response %s\n",retmsg);
			write_array((retmsg));

		}
		if(rawtime - oldtime > 20)
		{
			printf("SENDING SAT PERMA BEACON\n");
		  	write_array(("SAT PERMA BEACON \n"));
			time ( &oldtime);
          	}
    	}
    /*while(1) {
        bytes_waiting = sp_input_waiting(port);
        if (bytes_waiting > 0) {
            //Important change: Edit to blocking read with small timeout to stop double response bug
            num_read = sp_blocking_read(port,byte_buff, sizeof byte_buff,500);
            print_buffer(byte_buff,num_read);
            
            char* msg = "noop";
            char retmsg[100];
            
            if(strcmp("noop",msg)==0){
                snprintf(retmsg,sizeof retmsg,"IM ALIVE:");
            }
            else if(strcmp("gettime",msg)==0){
                snprintf(retmsg,sizeof retmsg,"THIS IS THE TIME:");
            }
            else{
                snprintf(retmsg,sizeof retmsg,"404");
            }
            printf("Sending Response %s\n",retmsg);
            write_array((retmsg));
            
        }
        //sp_flush(port, bytes_waiting);
    }*/
    
}

//For command threading
void *keyboard(void *args){
    	char string[99];

    	while(1){
   		scanf("%99s", string);
   		printf("Command: %s\n", string);
   		strcat(string,"\n");
   		write_array(string);
	}
}
void *listen(void *args){
	unsigned char mybyte_buff[BUFF_SIZE] = {0};

    	int bytes_waiting = 0;
    	int num_read = 0;
	while(1){
		//printf("listening\n");
		//sleep(1);
		bytes_waiting = sp_input_waiting(port);
		//update current time
		//time ( &rawtime );
		if (bytes_waiting > 0) {
	      		num_read = sp_blocking_read(port,mybyte_buff, sizeof mybyte_buff,500);
			//printf("read %d bytes\n",num_read);
	     		print_buffer(mybyte_buff,num_read);
	  	}
    	}
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
    //file = fopen("log.txt", "a");




    print_banner();

    if (argc < 2) {
        print_usage(argv[0]);
        return -1;
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

    ret = sp_set_baudrate(port,BAUD_RATE);
    if (ret != SP_OK) {
        fprintf(stderr, "Unable to set the baud rate to: %d\n", BAUD_RATE);
        return -1;
    }
    //MODE CHOOSING HERE
    if (strcmp(argv[2],"heartbeat")==0)
    {
	time_t rawtime;
      	time ( &rawtime );
      	time_t oldtime = rawtime; 
      	time ( &oldtime);
      printf("HEARTBEAT MODE\n");
	char retmsg[100];
      while(1) {
          time ( &rawtime );
          bytes_waiting = sp_input_waiting(port);
          if (bytes_waiting > 0) {
		snprintf(retmsg,sizeof retmsg,"IM ALIVE: %ld\n",rawtime);
              num_read = sp_blocking_read(port,byte_buff, sizeof byte_buff,500);
              print_buffer(byte_buff,num_read);
              printf("Sending Response %s\n",retmsg);
		write_array((retmsg));
              sleep(2);
          }
 	  //sp_flush(port, bytes_waiting);

      }
    }
if (strcmp(argv[2],"standard")==0)
    {
	printf("STANDARD MODE\n");
	//similiar to beacon
    pthread_t thread_id;
    pthread_t thread_id2;
    
    pthread_create(&thread_id, NULL, standard, NULL);
    pthread_create(&thread_id2,NULL,downlink,NULL);
    pthread_join(thread_id,NULL);
    pthread_join(thread_id2,NULL);
      	
    }
if (strcmp(argv[2],"beacon")==0)
	{
	//declare current time variable
	time_t rawtime;
	//assign the variable
	time ( &rawtime );
	//create a reference time (used to be long int)
	time_t oldtime = rawtime;

	printf("BEACON MODE\n");
	//operating loop      
	while(1) {
		char retmsg[100];
		time (&rawtime );
		time (&oldtime);
		printf("Current time: %ld",rawtime);
	      	printf("\n");
		printf("Sending beacon\n");
		snprintf(retmsg,sizeof retmsg,"BEACON PULSE: %ld\n",rawtime);
		write_array(retmsg);
          
		while(rawtime - oldtime < 8)
		{
			bytes_waiting = sp_input_waiting(port);
			//update current time
			time ( &rawtime );
			if (bytes_waiting > 0) {
		      		num_read = sp_blocking_read(port,byte_buff, sizeof byte_buff,500);
				//printf("read %d bytes\n",num_read);
		     		print_buffer(byte_buff,num_read);
		  	}
		}
      	}
    }
    if (strcmp(argv[2],"iridium")==0)
    {
	printf("IRIDIUM\n");
	time_t rawtime;
      	time ( &rawtime );
      	time_t oldtime = rawtime; 
      	time ( &oldtime);
	while(1){
	    	
	    	write_array("AT\r\n");
		bytes_waiting = sp_input_waiting(port);
		time (&rawtime );
		time (&oldtime);
		//update current time
		//time ( &rawtime );
		while(rawtime - oldtime < 8)
		{
			bytes_waiting = sp_input_waiting(port);
			//update current time
			time ( &rawtime );
			if (bytes_waiting > 0) {
		      		num_read = sp_blocking_read(port,byte_buff, sizeof byte_buff,500);
				//printf("read %d bytes\n",num_read);
		     		print_buffer(byte_buff,num_read);
		  	}
		}
		
	}
    }
    if (strcmp(argv[2],"command")==0)
    {
    	printf("COMMAND NEW MODE\n");
    	pthread_t thread_id;
        pthread_t thread_id2;
	
        pthread_create(&thread_id, NULL, listen, NULL);
        pthread_create(&thread_id2,NULL,keyboard,NULL);
        pthread_join(thread_id,NULL);
        pthread_join(thread_id2,NULL);
    }
    return 0;
}

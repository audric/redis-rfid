
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include "hiredis.h"

int main()
{
    /*
        Endlessly reads RFID card tokens (serially from /dev/ttyUSB0) and
        pushes the value into redis with name rfidcard.

        Tested on a Raspberry Pi with a Sparkfun RFID USB Reader SEN-09963

        Compile:
            first compile hiredis
            then compile this program and link it with libhiredis.a
            $ cc -o redis-rfid-serial redis-rfid-serial.c libhiredis.a
            $ strip redis-rfid-serial
        Test with:
            $ redis-cli get rfidcard

        Todo:
        .) publish to git
        .) makefile
        .) make redis connection settings and serial reader settings as parameter
        .) implement a debug flag to show on stdio what's happening
        .) wrap up a init.d script to daemonized the thing

        The complete project involves:
        .) running chromium in kiosk mode as a single .xinitrc program browsing localhost
        .) getting id and some values form the web page
        .) sending them via web-service to the ADempiere server

        Forgive me for my ugly/rusty c... github@audric.it
    */
    int  RFIDfd, n;
    char buffer[255];
    char buffer2[255];
    struct termios RFIDopt;
    unsigned int j;
    redisContext *c;
    redisReply *reply;

    struct timeval timeout = { 1, 400000 }; // 1,4 seconds
    c = redisConnectWithTimeout((char*)"127.0.0.1", 6379, timeout);
    if (c->err) {
        printf("ERROR: redis connection error: %s\n", c->errstr);
    } else {
        printf("OK: redis server responding.\n");
    }

    RFIDfd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
    if ( RFIDfd != -1) {
	printf("OK: opening /dev/ttyUSB0 returned an handle.\n");
	tcgetattr( RFIDfd, &RFIDopt );
	cfsetispeed( &RFIDopt, B9600);
	RFIDopt.c_cflag |= (CLOCAL | CREAD | CS8);
	tcsetattr( RFIDfd, TCSANOW, &RFIDopt);
	fcntl( RFIDfd, F_SETFL, 0);
	printf("OK: about to initely loop... ctrl-c to end.\n");
	while(1) {
	    n = read( RFIDfd, buffer, 255);
	    buffer[n-1]=0; /* makes the string printable and drop extra first and last char */
	    strncpy(buffer2,buffer+1,strlen(buffer+1));
//	    printf("OK: ttyUSB0 <= %s (%d)", buffer2, strlen(buffer2));
	    reply = redisCommand(c,"SET %s %s", "rfidcard", buffer2);
//	    printf(" : REDIS SET: %s\n", reply->str);
	    freeReplyObject(reply);
	    read(RFIDfd, buffer, 1); /* drop the extra char sent by the reader */
	}
    } else {
	printf("ERROR: opening /dev/ttyUSB0 did not return an handle!\n");
    }
}

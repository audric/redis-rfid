/*

  Endlessly reads RFID card tokens (serially from /dev/ttyUSB0) and
  pushes the value into redis with name rfidcard.

  Tested on a Raspberry Pi with a Sparkfun RFID USB Reader SEN-09963

  Compile:
    first compile hiredis
    then compile this program and link it with libhiredis.a
    cc -o redis-rfid-serial redis-rfid-serial.c libhiredis.a
  To run:         ./redis-rfid-serial
  Swipe an RFID on the reader.
  Test with:      redis-cli get rfidcard
  To test daemon: ps -ef|grep redis-rfid-serial
  To test log:    tail -f /tmp/redis-rfid-serial.log
  To test signal: kill -HUP `cat /tmp/redis-rfid-serial.lock`
  To terminate:   kill `cat /tmp/redis-rfid-serial.lock`

*/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>

#include "hiredis.h"

#define RUNNING_DIR	"/tmp"
#define LOCK_FILE	"redis-rfid-serial.lock"
#define LOG_FILE	"/tmp/redis-rfid-serial.log"

void log_message( filename, message )
char *filename;
char *message;
{
  FILE *logfile;

  logfile = fopen( filename, "a" );
  if(!logfile) return;
  fprintf( logfile, "%s\n", message );
  fclose( logfile );
}

void signal_handler(sig)
int sig;
{
  switch(sig) {
  case SIGHUP:
    log_message( LOG_FILE, "SIG: hangup signal catched." );
    break;
  case SIGTERM:
    log_message( LOG_FILE, "SIG: terminate signal catched. bye bye..." );
    exit(0);
    break;
  }
}

void daemonize()
{
  pid_t pid,sid;
  int  i,lfp;
  char str[10];

  if( getppid() == 1) return; /* already a daemon */

  pid = fork();
  if (pid < 0) exit( EXIT_FAILURE ); /* fork error */
  if (pid > 0) exit( 0 ); /* parent exits */
  /* child (daemon) continues */

  umask( 0 ); /* set newly created file permissions */

  for ( i = getdtablesize(); i >= 0; --i ) close(i); /* close all descriptors */
  i = open( LOG_FILE, O_RDWR ); dup(i); dup(i); /* handle standart I/O */

  sid = setsid(); /* obtain a new process group */
  if (sid < 0) exit( EXIT_FAILURE );

  if ( ( chdir(RUNNING_DIR) ) < 0 ) exit( EXIT_FAILURE );  /* change running directory */

  lfp = open( LOCK_FILE, O_RDWR|O_CREAT, 0640 );
  if ( lfp < 0) exit( EXIT_FAILURE ); /* can not open */
  if ( lockf( lfp, F_TLOCK, 0 ) < 0) exit( 0 ); /* can not lock */

  sprintf( str, "%d\n", getpid() );
  write( lfp, str, strlen(str) ); /* record pid to lockfile */

  close( STDIN_FILENO );
  close( STDOUT_FILENO );
  close( STDERR_FILENO );

  signal( SIGCHLD, SIG_IGN ); /* ignore child */
  signal( SIGTSTP, SIG_IGN ); /* ignore tty signals */
  signal( SIGTTOU, SIG_IGN );
  signal( SIGTTIN, SIG_IGN );
  signal( SIGHUP, signal_handler ); /* catch hangup signal */
  signal( SIGTERM, signal_handler ); /* catch kill signal */
}

int getrfidfd()
{
    int  RFIDfd;
    struct termios RFIDopt;

    RFIDfd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
    if ( RFIDfd != -1) {
	log_message( LOG_FILE, "OK: opening /dev/ttyUSB0 returned an handle.");
	tcgetattr( RFIDfd, &RFIDopt );
	cfsetispeed( &RFIDopt, B9600);
	RFIDopt.c_cflag |= (CLOCAL | CREAD | CS8);
	tcsetattr( RFIDfd, TCSANOW, &RFIDopt);
	fcntl( RFIDfd, F_SETFL, 0);
	return RFIDfd;
    } else {
	log_message( LOG_FILE, "ERROR: opening /dev/ttyUSB0 did not return an handle!\n");
	exit( EXIT_FAILURE );
    }
}

int main()
{
    int  RFIDfd, n;
    char buffer[255];
    char buffer2[255];
    unsigned int j;
    redisContext *c;
    redisReply *reply;

    daemonize();
    log_message( LOG_FILE, "OK: daemonized.");
    log_message( LOG_FILE, "CHK: checking redis server.");

    struct timeval timeout = { 1, 400000 }; // 1,4 seconds
    c = redisConnectWithTimeout((char*)"127.0.0.1", 6379, timeout);
    if (c->err) {
        log_message( LOG_FILE, "ERROR: redis connection error");
    } else {
        log_message( LOG_FILE, "OK: redis server responding.");
    }

    log_message( LOG_FILE, "CHK: checking RFID reader.");
    RFIDfd = getrfidfd();
    log_message( LOG_FILE, "OK: entering endless looooooop...");
    while(1) {
        n = read( RFIDfd, buffer, 255);
        buffer[n-1]=0; /* makes the string printable and drop extra first and last char */
	strncpy(buffer2,buffer+1,strlen(buffer+1));
//	printf("OK: ttyUSB0 <= %s (%d)", buffer2, strlen(buffer2));
	log_message( LOG_FILE, "   OK: read something inside the endless loop...");
        reply = redisCommand(c,"SET %s %s", "rfidcard", buffer2);
//	printf(" : REDIS SET: %s\n", reply->str);
        freeReplyObject(reply);
        read(RFIDfd, buffer, 1); /* drop the extra char sent by the reader */
        sleep(1);
    }

  log_message( LOG_FILE, "WTF: exiting from main.");
  exit( EXIT_SUCCESS );
}

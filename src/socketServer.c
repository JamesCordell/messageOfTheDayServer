#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 

#include <sys/ioctl.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>

#include "motd.h"

//#define LISTEN_QSIZE 50
//#define PORT	6666
//#define MOTDFILE "./motd.txt"
//#define MAXMOTDSIZE 9 /*must be bigger than 2. One for the command plus 1 for null*/


#define handelError(msg) \
	do { \
	syslog(LOG_ERR,"Error : %s \n",strerror(errno)); \
	perror(msg); exit(EXIT_FAILURE); \
	} while (0)



/* This function accepts a sockaddr_in and uses it to fill a string with the IP address and port */
void getIpAddr(struct sockaddr_in *addr , char *IpAddrPortStr) {

char ipStr[INET_ADDRSTRLEN];
int port;

	if ( addr->sin_family == AF_INET ) { // only IPv4
		port = ntohs( addr->sin_port );
	    	inet_ntop( AF_INET, &addr->sin_addr, ipStr, sizeof( ipStr ) );
	} else {
		handelError( "\n Error : IPv4 only \n" );
	}

	snprintf(IpAddrPortStr,INET_ADDRSTRLEN + 5,"%s:%d", ipStr,port);//Ports have a maximum of 5 digits 65535

}

/* This function accepts a null terminated string of len size and reads the motd file.*/
void openFileRead(char* msg , size_t len) {

	FILE *fp;
	if ( ! access( MOTDFILE, R_OK ) ) { //Do we need to make the file?
		fp = fopen( MOTDFILE,"r" );

		if ( fp ) {
		       	// Read the one motd line
		       	fgets( msg,len,fp );
			fclose( fp );
		} else {
			handelError( "\n Error : Could not open file \n" );
		}

	} else { //Create the file
		fp = fopen( MOTDFILE,"w+" );
		if ( fp < 0 ) 
			handelError( "\n Error : Could not create file \n" );

		fwrite( msg,sizeof(char),len,fp );//Allow the default message to be truncated if MAXMOTDSIZE is configured so
		fclose( fp );
	}
}

/* This function accepts a null terminated string and updates the motd file with said string */
void openFileWrite(char* msg ) {

        FILE *fp;
	syslog( LOG_NOTICE,"Updating new motd %s",msg );

        fp = fopen( MOTDFILE,"w+" );
        if ( fp < 0 )
        	handelError( "\n Error : Could not create file \n" );

	syslog( LOG_NOTICE,"Updating new motd to %s",msg );
        int written = fwrite( msg,sizeof( char ),strlen( msg ),fp );

	if ( written != strlen( msg ) ) {
		fclose( fp );
		handelError( "\n Error : Could not write data to file \n" );
	}
	if ( fflush( fp ) ) //flush buffers
		handelError( "\n Error : error fflush \n" );
	int fd = fileno( fp );
	if ( fsync( fd ) ) //Ensure data is written to disk
		handelError( "\n Error : error fsync \n" );
        fclose(fp);
}

int main(int argc , char *argv[] )
{
	setlogmask( LOG_UPTO ( LOG_NOTICE ) );
	openlog( "MOTD",LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1 );
	syslog( LOG_NOTICE,"Program started by User %d",getuid() );

	int listenFd = socket( AF_INET, SOCK_STREAM, 0 );

	int on=1;
	if ( setsockopt(listenFd, SOL_SOCKET,  SO_REUSEADDR,(char *)&on, sizeof(on)) )//This enables server to run if not shutdown properly or network issues
		handelError( "\n Error : Could not socketopt \n" );

	if ( listenFd < 0 )
		handelError( "\n Error : Could not create socket \n" );

	struct sockaddr_in servAddr; 
	memset( &servAddr, '0', sizeof( servAddr ) );

	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl( INADDR_ANY );
	servAddr.sin_port = htons( PORT ); 

	if( bind( listenFd, (struct sockaddr* )&servAddr, sizeof( servAddr ) ) < 0 )
		handelError("\n Error : Could not bind \n");

	if ( listen( listenFd, LISTEN_QSIZE ) < 0 )
		handelError(" \n Error : Could not listen \n" );
	
	char sendBuff[MAXMOTDSIZE];
	memset( &sendBuff,'\0', sizeof( sendBuff ) ); 
	char recvBuff[MAXMOTDSIZE];
	memset( &recvBuff,'\0', sizeof( recvBuff ) ); 

	while( 1 ) {//main server loop
	struct sockaddr_in remoteAddr;
	socklen_t len = sizeof( remoteAddr );
        int conn = accept( listenFd , (struct sockaddr *)&remoteAddr , &len ); 

	int status = read( conn,recvBuff, sizeof(recvBuff) );
	//int status = read( conn,recvBuff, sizeof(recvBuff)-1 );

	char commandChar = NULL;
	commandChar = recvBuff[0];
		if ( status > 0 ) {//Deal with data
			
			char ipAddrPort[20] = "";//space of ip address and port
			memset(ipAddrPort, '\0', sizeof(ipAddrPort));
			getIpAddr( &remoteAddr,ipAddrPort );


			syslog( LOG_NOTICE,"%i bytes received", status );
			if ( commandChar == 'r' ) {//read message
				openFileRead( sendBuff , sizeof( sendBuff ) );
	       			write( conn, sendBuff , sizeof( sendBuff ) ); 
				syslog( LOG_NOTICE,"IP address: %s performed read action message is: %s",ipAddrPort,sendBuff);
			} else if ( commandChar == 'w' ) {//write update message
				openFileWrite( recvBuff+1 );//skip first char command
				char confirmMsg[] = { "Message updated." };
	       			write( conn, confirmMsg, sizeof(confirmMsg) ); 
				syslog( LOG_NOTICE,"IP address: %s performed write action message is: %s",ipAddrPort,recvBuff+1);
			} else if ( commandChar == 'k' ) {//shutdown server
				syslog( LOG_NOTICE,"IP address: %s performed action: %c ",ipAddrPort,commandChar,recvBuff);
	       			close( conn );
				break;
			} else {
				syslog( LOG_NOTICE,"IP address: %s performed action: %c command not recognised",ipAddrPort,commandChar,recvBuff);
			}
		} else if ( status == -1 ) { // Error, check errno, take action... 
			close( conn );
			handelError( "\n Error : Could not read \n" );
		} else if ( status == 0 ) { // Peer closed the socket, finish the close 
       			close( conn );
		}
	}
	closelog();
return 0;
}

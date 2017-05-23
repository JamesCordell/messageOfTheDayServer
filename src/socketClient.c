#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#include "motd.h"

#define CONFIRMMSGSIZE 30 //Confirmation message.

int main(int argc, char *argv[]) {

        if ( argc <= 1 )
                handelError("Usage: %s <ip of server> \n");

        char sendBuff[MAXMOTDSIZE+1];//+1 to include the commandChar
        memset(sendBuff,'\0',sizeof(sendBuff));//clear send buffer
	
	printf("Enter r to read the message of the day. Or w to update the message of the day. Or k to kill the server. \n");
	char commandChar='\0';
	scanf("%c",&commandChar);
	sendBuff[0] = commandChar;
	if ( commandChar == 'w' ) {
	getchar(); 
	printf("\nPlease enter new message of the day\n");
        fgets(sendBuff+1,MAXMOTDSIZE,stdin);//read 
	} 
	int sockFd = socket(AF_INET, SOCK_STREAM, 0);
	
	        if ( sockFd == -1 )
	                handelError("Error : Could not create socket");
	
	        struct sockaddr_in servAddr;
	        memset(&servAddr, '0', sizeof(servAddr));
	        servAddr.sin_family = AF_INET;
	        servAddr.sin_port = htons(PORT);
	
	        if ( inet_pton(AF_INET, argv[1], &servAddr.sin_addr)<=0 )
	                handelError("Error : inet_pton address not converted");
	
	        if( connect(sockFd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0 )
	                handelError("Error : Connect Failed \n");
	
	        if ( write(sockFd, sendBuff ,strlen(sendBuff) )  < 0 )
	                handelError("Error : sending");
	
	        char recvBuff[CONFIRMMSGSIZE];
	        memset(recvBuff, '\0',sizeof(recvBuff));
	
	        int status = read(sockFd, recvBuff, sizeof(recvBuff)-1); 
		
		if ( status < 0 ) { 
	        	handelError("Error : Read");
		} else {
	
		if ( commandChar == 'r' )
			printf("Message of the day is:\n%s\n",recvBuff);

	        if ( commandChar == 'w' )
			printf("Message from server:%s",recvBuff);
	
		}
        
return 0;
}


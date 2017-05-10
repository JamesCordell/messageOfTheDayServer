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

#define handelError(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define CONFIRMMSGSIZE 30 //Confirmation message.

int main(int argc, char *argv[]) {

        if ( argc < 1 )
                handelError("Usage: %s <ip of server> \n");

        char sendBuff[MAXMOTDSIZE+1];
        memset(sendBuff,'\0',sizeof(sendBuff));
        fgets(sendBuff,MAXMOTDSIZE,stdin);

        int sockFd = socket(AF_INET, SOCK_STREAM, 0);

        if ( sockFd == -1 )
                handelError("\n Error : Could not create socket \n");

        struct sockaddr_in servAddr;
        memset(&servAddr, '0', sizeof(servAddr));
        servAddr.sin_family = AF_INET;
        servAddr.sin_port = htons(PORT);

        if ( inet_pton(AF_INET, argv[1], &servAddr.sin_addr)<=0 )
                handelError("\n Error : inet_pton address not converted \n");

        if( connect(sockFd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0 )
                handelError("\n Error : Connect Failed \n");

        if ( write(sockFd, sendBuff ,strlen(sendBuff) )  < 0 )
                handelError("Error sending");

        char recvBuff[CONFIRMMSGSIZE];
        memset(recvBuff, '\0',sizeof(recvBuff));

        int n=0;
        while ( (n = read(sockFd, recvBuff, sizeof(recvBuff)-1) ) > 0 ) {

                if ( sendBuff[0] == 'r' )
                        printf("Message of the day is:\n%s\n",recvBuff);

                if ( sendBuff[0] == 'w' )
                        printf("Message from server:%s",recvBuff);

        if ( n < 0 ) {
        printf("\n Read error \n");
        }

        return 0;
        }
return 0;
}


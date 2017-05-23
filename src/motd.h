#ifndef MOTD_H
#define MOTD_H

#include <syslog.h>

#define LISTEN_QSIZE 50
#define PORT    6666
#define MOTDFILE "./motd.txt"
#define MAXMOTDSIZE 1024 /*must be bigger than 2. One for the command plus 1 for null*/

#define handelError(msg) \
	do { \
	syslog(LOG_ERR,"Error : %s \n",strerror(errno)); \
	perror(msg); exit(EXIT_FAILURE); \
	} while (0)

#endif

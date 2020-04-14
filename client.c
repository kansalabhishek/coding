#include<sys/socket.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<stdio.h>
#include<fcntl.h>
#include<string.h>

#define max(a, b) (a > b ? a : b)
#define MAXLINE 1024
#define PORT 8080
#define SA struct sockaddr

void str_cli(FILE *fp, int sockfd)
{
	int maxfdp1, stdineof;
	fd_set rset;
	char buf[MAXLINE], user[MAXLINE], pass[MAXLINE];
	char *userMsg = "Enter user id: ";
	char *passMsg = "Enter password: ";
	int n, loginStatus = -1;
	stdineof = 0;
	FD_ZERO(&rset);
	for ( ; ; ) 
	{
		bzero(buf, sizeof(buf));
		bzero(user, sizeof(user));
		bzero(pass, sizeof(pass));
		
		if (stdineof == 0)
			FD_SET(fileno(fp), &rset);
		
		FD_SET(sockfd, &rset);
		maxfdp1 = max(fileno(fp), sockfd) + 1;
		
		if(loginStatus == -1)
		{
			write(1, userMsg, strlen(userMsg));
			read(1, user, MAXLINE);
			write(1, passMsg, strlen(passMsg));
			read(1, pass, MAXLINE);
			//strncpy(user, user, strlen(user) - 2);
			//strcat(user, " ");
			strcat(user, pass);
			//write(1, user, strlen(user));
			write(sockfd, user, strlen(user));
			//bzero(buf, sizeof(buf));
			//continue;
		}
		select(maxfdp1, &rset, NULL, NULL, NULL);
		
		if (FD_ISSET(sockfd, &rset)) { /* socket is readable */
			if ( (n = read(sockfd, buf, MAXLINE)) == 0) {
				if (stdineof == 1)
					return; /* normal termination */
				else{
					//write(1, buf, strlen(buf));
					// printf("**************");
					stdineof = 1;
					shutdown(sockfd, SHUT_WR); /* send FIN */
					FD_CLR(fileno(fp), &rset);
					continue;
					//printf("str_cli: server terminated prematurely");
					//exit(0);
				}
			}
			if(strcasecmp(buf, "Sorry your id and password is not correct\n") != 0)
				loginStatus = 0;
			write(fileno(stdout), buf, n);
		}
		if (FD_ISSET(fileno(fp), &rset)) { /* input is readable */
			if ( (n = read(fileno(fp), buf, MAXLINE)) == 0) {
				stdineof = 1;
				shutdown(sockfd, SHUT_WR); /* send FIN */
				FD_CLR(fileno(fp), &rset);
				continue;
			}

            char store[] = "STORE\n";
            //printf("here buffer is %s\n", buf);
            int res = strcasecmp(buf, store);
            //printf("%d", res);
            if(strcasecmp(buf, store) == 0) {
                //char *token = strtok(buf, "\n");
				char first[MAXLINE];
				read(fileno(fp), first, MAXLINE);
				char second[MAXLINE];
				read(fileno(fp), second, MAXLINE);
				strcat(buf, first);
				strcat(buf, second);
				//aree bindaas hoga.tu chinta mat krXD okmeine user passs mei bhi ni daala
				//chalo ho gya thanks : )ab server khol space mat daal server pr tokenise kse?
                /*char *first = strtok(NULL, "\n");
                char *second = strtok(NULL, "\n");
                printf("token: %s", token);*/
                //printf("first: %s", first);
                //printf("second: %s", second);
            }
			
			write(sockfd, buf, strlen(buf));
		}
	}
}

int main()
{
	int sockFd;
	struct sockaddr_in servaddr;
	char buff[1024];
	sockFd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockFd == -1)
	{
		printf("Socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created...\n");
	
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_port =htons(PORT);
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if(connect(sockFd, (SA*) &servaddr, sizeof(servaddr)) != 0)
	{
		printf("Connection with the server failed...\n");
		exit(0);
	}
	else
		printf("Connected to the SOUND server..\n");
		
	str_cli(stdin, sockFd);
	close(sockFd);
}
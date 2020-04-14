#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#define SA struct sockaddr
#define LISTENQ 128
#define MAXLINE 1024
#define SERV_PORT 8080

int findUser(char user[5][200], char pass[5][200], char buf[], int numOfUser)
{
    int result;
    int i = 0;
    char *token = strtok(buf, "\n");

    for (i = 0; i < numOfUser; i++)
    {
        result = strcmp(user[i], token);

        if (result == 0)
        {
            token = strtok(NULL, "\n");

            if (strcmp(pass[i], token) == 0)
            {
                return i;
            }
            return -1;
        }
    }
    if(numOfUser < 5)
    {
        strcpy(user[numOfUser], token);
        
        token = strtok(NULL, "\n");
        strcpy(pass[numOfUser], token);

        //numOfUser++;
        return -2;
    }
    return -1;
}

int findAnimal(char animals[5][200], char buf[], int count)
{
    int i, result;
    for (i = 0; i < count; i++)
    {
        result = strcasecmp(animals[i], buf);
        if (result == 0)
            return i;
    }
    return -1;
}

void printAnimal(int sockfd, char animals[15][200], int count)
{

    int i;
    for (i = 0; i < count; i++)
    {
        char *animal = animals[i];
        //write(1, animals[i], strlen(animals[i]));
        strcat(animal, "\n");

        write(sockfd, animal, strlen(animal));
        char *token = strtok(animals[i], "\n");
        strcpy(token, animals[i]);
        //write(1, animals[i], strlen(animals[i]));
    }
}

int main()
{

    char user[5][200];// = {"user1", "user2", "user3", "user4", "user5"};
    char pass[5][200];// = {"pass1", "pass2", "pass3", "pass4", "pass5"};
    int numOfUser = 0;

    int loginStatus[FD_SETSIZE] = {0};

    char animals[15][200] = {"DOG", "CAT", "SNAKE", "PIG", "HORSE"};
    char sounds[15][200] = {"WOOF", "MEOW", "HISS", "GRUNT", "NEIGH"};
    int count = 5;

    int result;

    int i, maxi, maxfd, listenfd, connfd, sockfd;
    int nready, client[FD_SETSIZE];
    ssize_t n;
    fd_set rset, allset;
    char buf[MAXLINE];
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    printf("Socket successfully created..\n");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    bind(listenfd, (SA *)&servaddr, sizeof(servaddr));
    printf("Socket successfully bind..\n");

    listen(listenfd, LISTENQ);
    printf("Server listening..\n");

    maxfd = listenfd;
    maxi = -1;

    for (i = 0; i < FD_SETSIZE; i++)
    {
        client[i] = -1;
    }
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    for (;;)
    {
        bzero(buf, sizeof(buf));
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(listenfd, &rset))
        {
            clilen = sizeof(cliaddr);
            connfd = accept(listenfd, (SA *)&cliaddr, &clilen);
            printf("Connected to client..\n");

            for (i = 0; i < FD_SETSIZE; i++)
            {
                if (client[i] < 0)
                {
                    client[i] = connfd;
                    break;
                }
            }

            if (i == FD_SETSIZE)
            {
                printf("Too many clients");
                return 0;
            }
            FD_SET(connfd, &allset);
            if (connfd > maxfd)
            {
                maxfd = connfd;
            }
            if (i > maxi)
            {
                maxi = i;
            }
            if (--nready <= 0)
                continue;
        }
        for (i = 0; i <= maxi; i++)
        {
            bzero(buf, sizeof(buf));
            if ((sockfd = client[i]) < 0)
                continue;
            if (FD_ISSET(sockfd, &rset))
            {
                if ((n = read(sockfd, buf, MAXLINE)) == 0)
                {
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                    loginStatus[i] = 0;
                    char *msg = "Connection closed with the client..\n";
                    write(1, msg, strlen(msg));
                }
                else
                {
                                   if (strcasecmp(buf, "SOUND") == 0)
                    {
                        char msg[] = "SOUND: OK\n";
                        write(sockfd, msg, strlen(msg));
                    }
                    else if (loginStatus[i] == 0)
                    {
                        int index = findUser(user, pass, buf, numOfUser);
                        if (index == -1)
                        {
                            char *temp = "Sorry your id and password is not correct\n";
                            write(sockfd, temp, strlen(temp));
                            loginStatus[i] = 0;
                        }
                        else if(index == -2)
                        {
                            numOfUser++;

                            char *msg = "Registered successfully..\n";
                            write(sockfd, msg, strlen(msg));
                            loginStatus[i] = 1;
                            char num[5];
                            write(1, msg, strlen(msg));
                            //write(sockfd, buf, strlen(buf));
                            //write(sockfd, itoa(numOfUser, num, 10), strlen(num));
                        }
                        else
                        {
                            char *msg = "User logged in..\n";
                            write(1, "User logged in..\n", 18);
                            loginStatus[i] = 1;
                            write(sockfd, msg, strlen(msg));
                        }
                    }
                    else
                    {
                        //write(1, buf, strlen(buf));
                        char *token = strtok(buf, "\n");
                        //write(1, token, strlen(token));

                        if (strcasecmp(token, "QUERY") == 0)
                        {
                            printAnimal(sockfd, animals, count);
                            char msg1[] = "SERVER:";
                            write(sockfd, msg1, strlen(msg1));
                            char *msg = "QUERY OK\n";
                            write(sockfd, msg, strlen(msg));
                            //FD_CLR(sockfd,&allset);
                        }
                        else if (strcasecmp(token, "STORE") == 0)
                        {
                            //printf("token: %s\n", token);
                            char *first = strtok(NULL, "\n");

                            //printf("first: %s\n", first);
                            char *second = strtok(NULL, "\n");
                            //printf("second: %s\n", second);

                            char *msg = "STORE: OK\n";

                            int storeIdx = findAnimal(animals, first, count);

                            //store new animal :p
                            if (storeIdx == -1)
                            {
                                // but check if storage is not full
                                if (count < 15)
                                {
                                    strcpy(sounds[count], second);
                                    //printf("second:** %s\n", second);
                                    //write(1, first, strlen(first));

                                    strcpy(animals[count], first);
                                    //write(1, second, strlen(second));

                                    //write(1, second, strlen(second));

                                    count++;
                                }
                            }
                            else // update animal's sound
                            {
                                //strcat(second, "\n");
                                //write(1, second, strlen(second));
                                strcpy(sounds[storeIdx], second);
                            }

                            write(sockfd, msg, strlen(msg));
                        }
                        else if (strcasecmp(buf, "BYE") == 0)
                        {
                            char *msg = "BYE: OK\n";
                            write(sockfd, msg, strlen(msg));
                            client[i] = -1;
                            loginStatus[i] = 0;
                            close(sockfd);
                            FD_CLR(sockfd, &allset);
                        }
                        else if (strcasecmp(buf, "END") == 0)
                        {
                            printf("END\n");
                            printf("%d\n", maxi);
                            char *msg1 = "SERVER: END: OK\n";
                            //printf(msg1);
                            for (int i = 0; i <= maxi; i++)
                            {
                                printf("END\n");
                                sockfd = client[i];
                                if (sockfd > -1)
                                {
                                    printf("%d\n", sockfd);
                                    write(1, msg1, strlen(msg1));
                                    write(sockfd, msg1, strlen(msg1));
                                    client[i] = -1;
                                    loginStatus[i] = 0;
                                    //shutdown(client[i], SHUT_RD);
                                    //shutdown(client[i], SHUT_WR);
                                    close(sockfd);
                                    FD_CLR(sockfd, &allset);
                                    
                                }
                            }
                            close(listenfd);
                            FD_CLR(listenfd, &allset);
                            exit(1);
                        }
                        else
                        {
                            //write(1, buf, strlen(buf));
                            int index = findAnimal(animals, token, count);

                            if (index == -1)
                            {
                                char temp[] = "I DON\'T KNOW ";
                                strcat(temp, buf);
                                strcat(temp, "\n");
                                write(sockfd, temp, strlen(temp));
                            }
                            else
                            {
                                char msg[] = "A ";
                                char *token = strtok(buf, "\n");
                                strcat(msg, token);
                                strcat(msg, " SAYS ");
                                strcat(msg, sounds[index]);
                                strcat(msg, "\n");

                                write(sockfd, msg, strlen(msg));
                            }
                        }
                        //write(sockfd, buf, n);
                    }
                }
                if (--nready <= 0)
                    break;
            }
        }
    }
}

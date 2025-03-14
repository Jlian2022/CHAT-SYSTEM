#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define SERV_TCP_PORT 23 /* server's port number */
#define MAX_SIZE 1200
#define MAX_CLIENTS 5
#define TIMESTAMP_SIZE 64

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int client_fds[MAX_CLIENTS];
int joined_clients = 0;//need this to keep track of the amount of client that has joined

//this send_client method is used to send message to all the client that are connected
void send_client(const char *message, int sender_fd) {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < joined_clients; i++) {
        if (client_fds[i] != sender_fd) { //literate through the loop to make sure the person that is sending this message is not the person we going to send the message back to meaning we going to send it to people who joined but didnt send the message
            write(client_fds[i], message, strlen(message)); //send the message to the clients that didnt send the message
        }
    }
    pthread_mutex_unlock(&mutex); //unlock the mutux because we are done sending the message to client
}

//this method is going to be reading from connected client
void *handle_client(void *arg) {
    int client_fd = *(int *)arg; // file descriptor is being pass in as a void but for read system call the file descriptor must be interger
    char string[MAX_SIZE];
    char timestamp[TIMESTAMP_SIZE];
    int len;

    for (;;) {      //infinity loop to read from the client
        len = read(client_fd, string, MAX_SIZE);    //read from client
        if (len <= 0) { //make sure read was successful
            if (len < 0) { //if something wrong happen during the reading we are going to print error
                perror("Error reading from client");
            }
            break;// Break the loop if the client disconnects or an error occurs
        }
        string[len] = 0;  /* make sure it's a proper string */

        // Get the current time
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        strftime(timestamp, TIMESTAMP_SIZE, "[%Y-%m-%d %H:%M:%S]", t);

        printf("%s: %s\n", timestamp, string); 
        send_client(string, client_fd); //send the message to other clients
    }

    close(client_fd);//close file descriptor
    //if anything error occur add a mutux here but most likely no need
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, clilen;
    struct sockaddr_in cli_addr, serv_addr;
    int port;
    pthread_t thread;
    /* command line: server [port_number] */
    if (argc == 2)
        sscanf(argv[1], "%d", &port);
    else
        port = SERV_TCP_PORT;
    /* open a TCP socket (an Internet stream socket) */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("can't open stream socket");
        exit(1);
    }
    /* bind the local address, so that the client can send to server */
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("can't bind local address");
        exit(1);
    }
    /* listen to the socket */
    listen(sockfd, MAX_CLIENTS);

    for (;;) {
        /* wait for a connection from a client */
        clilen = sizeof(cli_addr);
        if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) < 0) {
            perror("can't bind local address");
            continue;
        }

        pthread_mutex_lock(&mutex);
        if (joined_clients >= MAX_CLIENTS) {//check to see if it exceed 5 person
            printf("User attempted to join while server is at capacity\n");// if it did it will print this
            close(newsockfd);//close the socket
            pthread_mutex_unlock(&mutex);
            continue;//we dont want the program to end here other people who are connected might still want to chat
        }

        client_fds[joined_clients] = newsockfd;// adding the client that join in a array so we can use the if statement above to make sure it dont exceed 5 client which its max client.
        joined_clients +=1; //increment the number by 1
        pthread_mutex_unlock(&mutex);

        pthread_create(&thread, NULL, handle_client, (void *)&newsockfd); //make thread for client communication
        pthread_detach(thread); //detach thread when we done
    }

    close(sockfd);
    return 0;
}

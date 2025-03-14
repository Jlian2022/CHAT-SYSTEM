#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#define SERV_TCP_PORT 23 /* server's port */
#define MAX_MSG_SIZE 1200 // size for message
#define MAX_NAME_SIZE 200 // size for name
#define MAX_FULL_MSG_SIZE (MAX_NAME_SIZE + MAX_MSG_SIZE + 3) // full message require both in top plus the name and : but just in case I add one extra more

void *receive_messages(void *sockfd_ptr) {
    int sockfd = *(int *)sockfd_ptr; //turn void pointer to an interger
    char buffer[MAX_FULL_MSG_SIZE]; //store message coming in
    int len;

    while ((len = read(sockfd, buffer, MAX_FULL_MSG_SIZE - 1)) > 0) {//read from socket
        buffer[len] = 0;
        printf("%s\n", buffer);//print the recieve message
    }

    if (len < 0) {
        perror("Error reading from server");//if something goes wrong it print this
    }

    return NULL;//end the thread
}

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in serv_addr;
    char *serv_host = "localhost";
    struct hostent *host_ptr;
    int port;
    char message[MAX_MSG_SIZE]; // buffer for message
    char client_name[MAX_NAME_SIZE]; // buffer for client name
    char name_in_front[MAX_FULL_MSG_SIZE]; // buffer for the name in front
    pthread_t recv_thread;

    /* command line: client [host [port]] */
    if (argc >= 2)
        serv_host = argv[1];/* read the host if provided */
    if (argc == 3)
        sscanf(argv[2], "%d", &port);/* read the port if provided */
    else
        port = SERV_TCP_PORT;

    /* get the address of the host */
    if ((host_ptr = gethostbyname(serv_host)) == NULL) {
        perror("gethostbyname error");
        exit(1);
    }

    if (host_ptr->h_addrtype != AF_INET) {
        perror("unknown address type");
        exit(1);
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = ((struct in_addr *)host_ptr->h_addr_list[0])->s_addr;
    serv_addr.sin_port = htons(port);

    /* open a TCP socket */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("can't open stream socket");
        exit(1);
    }
    /* connect to the server */
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("can't connect to server");
        exit(1);
    }
    /* Add client Name */
    printf("Enter your real name: ");
    if (fgets(client_name, MAX_NAME_SIZE, stdin) == NULL) { //use the function char*fgets(char*str, int n, file*stream)
        perror("Can't read name");// if we can read the client name we print we can read it meaning if user enter nothing it will print this
        close(sockfd);//close the socket
        exit(1);
    }
    client_name[strcspn(client_name, "\n")] = '\0'; // Remove newline character meaning without this line the code will have (name then next line :#something you type) with this we move that error

    pthread_create(&recv_thread, NULL, receive_messages, (void *)&sockfd);//use thread to deal with the message that is send over from server
    pthread_detach(recv_thread);

    /* write messages to the server */
    while (1) {
        //printf("Enter message: ");
        if (fgets(message, MAX_MSG_SIZE, stdin) != NULL) {
            // Remove newline character if present meaning without line that make you go next we dont want that
            size_t len = strlen(message);
            if (len > 0 && message[len - 1] == '\n') {//check if last character is new line
                message[len - 1] = 0;// if it is new line we make sure it is proper string like you mention in video
            }
            // in front of the message with client name
            int n = snprintf(name_in_front, MAX_FULL_MSG_SIZE, "%s: %s", client_name, message);//basically we want the the server to be in format name, then : then the message
            if (n < 0) {
                fprintf(stderr, "error with format\n");
            } else if (n >= MAX_FULL_MSG_SIZE) {
                fprintf(stderr, "message is too long!!!\n");//message cant be to long
                name_in_front[MAX_FULL_MSG_SIZE - 1] = 0;
            }
            // Send the message to the server
            if (write(sockfd, name_in_front, strlen(name_in_front)) < 0) {
                perror("Error writing to socket");
                break;
            }
        } else {
            perror("Error reading input");
            break;
        }
    }

    close(sockfd);
    return 0;
}

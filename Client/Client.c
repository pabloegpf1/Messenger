#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdio.h>      /* for printf() and fprintf() */
#include <stdlib.h>     /* for atoi() */
#include <string.h>     /* for memset() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <unistd.h>     /* for close() */

#define RCVBUFSIZE 1000 /* Size of receive buffer */
#define MAXPENDING 5    /* Maximum outstanding connection requests */

int sock, clntSock, servSock; /* Socket descriptors */

char *echoString;              /* String to send to echo server */
char echoBuffer[RCVBUFSIZE];   /* Buffer for echo string */
char echoSend[RCVBUFSIZE];     /* Buffer for echo string */
unsigned int echoStringLen;    /* Length of string to echo */
int bytesRcvd, totalBytesRcvd; /* Bytes read in single recv() and total bytes read */
int recvMsgSize;               /* Size of received message */
int online = -1;               /* Boolean: -1 -> User disconnected, else User connected */
int userCount;

struct sockaddr_in echoServAddr;                         /* Local address */
struct sockaddr_in echoClntAddr;                         /* Client address */
unsigned short echoServPort;                             /* Server port */
unsigned int clntLen;                                    /* Length of client address data structure */
char ipAdress[10], port[10], username[30], password[30]; /* Parameters to connect to Server */

void DieWithError(char *errorMessage); /* Error handling function */
int determineOption(int option);       /* Determine user's option */

int main(int argc, char *argv[]) {
    /* Test for correct number of arguments */
    if (argc != 1) {
        fprintf(stderr, "Error: Client does not accept arguments\n");
        exit(1);
    }
    while (1) {
        int option = -1;

        /*Display menu*/
        while (option < 0 || option > 5) {
            printf("\n-------------------------\nCommand\n0. Connect to the server\n1. Get the user list\n2. Send a message\n3. Get my messages\n4. Initiate a chat with my friend\n5. Chat with my friend\nYour option<enter a number>:");
            fscanf(stdin, "%d", &option);
            if (option < 0 || option > 5) {
                printf("\nIncorrect Option!,try again:\n");
            } else {
                determineOption(option);
            }
        }
    }

    printf("\n"); /* Print a final linefeed */
    exit(0);
}

int connectToServer() {
    struct sockaddr_in echoServAddr; /* Echo server address */

    /* Create a reliable, stream socket using TCP */
    unsigned short echoServPort;
    char *end;

    /* Get arguments from user to connect to server */
    printf("Please enter the IP adress: ");
    fscanf(stdin, "%s", &ipAdress);
    printf("Please enter the Port Number: ");
    fscanf(stdin, "%s", port);
    echoServPort = strtol(port, &end, 0);
    printf("Connecting.....\n");

    if (online == -1) {
        if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
            DieWithError(" socket () failed");

        /* Construct the server address structure*/
        memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
        echoServAddr.sin_family = AF_INET;                  /* Internet address family */
        echoServAddr.sin_addr.s_addr = inet_addr(ipAdress); /* Server IP address */
        echoServAddr.sin_port = htons(echoServPort);        /* Server port */

        /* Port can be reused in case of failure*/
        int opt = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
            DieWithError(" connect () failed");

        /* Establish the connection to the echo server */
        if (connect(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
            DieWithError(" connect () failed");

        online = 1; /* User is now connected */
    }

    /* Send the string to the server */
    if (send(sock, "0", sizeof("0"), 0) != sizeof("0"))
        DieWithError("Error sending option");

    /* Log in */
    printf("Connected!\nWelcome,Please Log In:\n");
    printf("Username: ");
    fscanf(stdin, "%s", &username);
    printf("Password: ");
    fscanf(stdin, "%s", &password);

    echoStringLen = strlen(username);
    /* Send the string to the server */
    if (send(sock, username, echoStringLen, 0) != echoStringLen)
        DieWithError("Error sending username");

    online = 1;
    return 0;
}

int sendMessage() {
    /* Send the string to the server */
    if (send(sock, "2", sizeof("2"), 0) != sizeof("2"))
        DieWithError("Error sending option");

    memset(echoBuffer, '\0', sizeof(echoBuffer) * sizeof(char));

    /* Send the username to the server */
    printf("Please enter the user name: ");
    fscanf(stdin, "%s", &echoBuffer);
    if (send(sock, echoBuffer, strlen(echoBuffer), 0) != strlen(echoBuffer))
        DieWithError("User not found");

    memset(echoBuffer, '\0', sizeof(echoBuffer) * sizeof(char));
    memset(echoSend, '\0', sizeof(echoSend) * sizeof(char));

    printf("Please enter the message: ");
    scanf(" %[^\n]%*c", echoBuffer);
    strcat(echoSend, username);
    strcat(echoSend, ": ");
    strcat(echoSend, echoBuffer);

    if (send(sock, echoSend, strlen(echoSend), 0) != strlen(echoSend))
        DieWithError("Error sending message");

    return 0;
}

int receiveMessage() {
    /* Send option to the server */
    if (send(sock, "3", sizeof("3"), 0) != sizeof("3"))
        DieWithError("Error sending option");

    /* Receive all the user's messages at once */
    memset(echoBuffer, '\0', sizeof(echoBuffer) * sizeof(char));
    if ((recvMsgSize = recv(sock, echoBuffer, sizeof(echoBuffer), 0)) < 0)
        DieWithError("recv() failed");

    /* Print Messages */
    printf("\n-------Messages-------\n");
    printf("%s\n", echoBuffer);

    return 0;
}

int getUserList() {
    /* Send option to the server */
    if (send(sock, "1", sizeof("1"), 0) != sizeof("1"))
        DieWithError("Error sending option");

    memset(echoBuffer, '\0', sizeof(echoBuffer) * sizeof(char));
    if ((recvMsgSize = recv(sock, echoBuffer, sizeof(echoBuffer), 0)) < 0)
        DieWithError("recv() failed");

    userCount = atoi(echoBuffer);
    printf("\n-------UserList(%d)-------\n", userCount);

    /* Get the user list from server */
    memset(echoSend, '\0', sizeof(echoSend) * sizeof(char));
    if ((recvMsgSize = recv(sock, echoSend, sizeof(echoSend), 0)) < 0)
        DieWithError("recv() failed");

    /* Print user list */
    printf("%s", echoSend);

    return 0;
}

int startChat() {
    const char *last_three; /*Used to catch message "Bye"*/

    /*Get port to connect from user input*/
    printf("\n------------------Disconnected from server------------------\n");
    printf("Please enter the port number you want to listen on: ");
    scanf(" %s", &echoBuffer);

    /*Connect to server*/
    echoServPort = atoi(echoBuffer);
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket () failed");

    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof("127.0.0.1"));    /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(echoServPort);      /* Local port */

    int opt = 1;
    if (setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    if (bind(servSock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind () failed");

    if (listen(servSock, MAXPENDING) < 0)
        DieWithError("listen() failed");

    printf("I am listening on 127.0.0.1:%d\n", echoServPort);

    /* Set the size of the in-out parameter */
    clntLen = sizeof(echoClntAddr);

    /* Wait for a client to connect */
    if ((clntSock = accept(servSock, (struct sockaddr *)&echoClntAddr, &clntLen)) < 0)
        DieWithError("accept() failed");

    printf("<Type \"Bye\" to stop conversation>\n");

    while (1) {
        /*Get message from user*/
        if ((recvMsgSize = recv(clntSock, echoBuffer, RCVBUFSIZE, 0)) < 0)
            DieWithError("recv() failed");

        printf("%s\n", echoBuffer);

        /*Determine if message equals "Bye"*/
        last_three = &echoBuffer[strlen(echoBuffer) - 3];
        if (strcmp(last_three, "Bye") == 0) break;

        /*Build response to user*/
        memset(echoSend, '\0', sizeof(echoSend) * sizeof(char));
        printf("%s: ", username);
        scanf(" %[^\n]%*c", echoBuffer);
        strcat(echoSend, username);
        strcat(echoSend, ": ");
        strcat(echoSend, echoBuffer);

        /*Send Respose to user*/
        if (send(clntSock, echoSend, sizeof(echoSend), 0) != sizeof(echoSend))
            DieWithError("Error sending option");

        /*Check if respose was "Bye"*/
        last_three = &echoSend[strlen(echoSend) - 3];
        if (strcmp(last_three, "Bye") == 0) break;
    }
    return 0;
    close(clntSock);
}

int connectToChat() {
    const char *last_three;          /*Used to catch message "Bye"*/
    struct sockaddr_in echoServAddr; /* Echo server address */

    /* Create a reliable, stream socket using TCP */
    char ipAdress[10], port[10];
    unsigned short echoServPort;
    char *end;

    /*Get user 2 info from user input*/
    printf("Please enter the IP adress: ");
    fscanf(stdin, "%s", &ipAdress);
    printf("Please enter the Port Number: ");
    fscanf(stdin, "%s", port);
    echoServPort = strtol(port, &end, 0);
    printf("Connecting your friend.....\n");

    /* Create a reliable, stream socket using TCP*/
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError(" socket () failed");

    /* Construct the server address structure*/
    memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                  /* Internet address family */
    echoServAddr.sin_addr.s_addr = inet_addr(ipAdress); /* Server IP address */
    echoServAddr.sin_port = htons(echoServPort);        /* Server port */

    /* Establish the connection to the echo server */
    if (connect(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError(" connect () failed");
    printf("Connected!.....\n<Type \"Bye\" to stop conversation>\n");

    while (1) {
        /*Build message*/
        memset(echoSend, '\0', sizeof(echoSend) * sizeof(char));
        printf("%s: ", username);
        scanf(" %[^\n]%*c", echoBuffer);
        strcat(echoSend, username);
        strcat(echoSend, ": ");
        strcat(echoSend, echoBuffer);

        /*Send message*/
        if (send(sock, echoSend, sizeof(echoSend), 0) != sizeof(echoSend))
            DieWithError("Error sending message");

        /*Check if message equals "Bye"*/
        last_three = &echoSend[strlen(echoSend) - 3];
        if (strcmp(last_three, "Bye") == 0) break;

        /*Receive response from user 2*/
        memset(echoBuffer, '\0', sizeof(echoBuffer) * sizeof(char));
        if ((recvMsgSize = recv(sock, echoBuffer, RCVBUFSIZE, 0)) < 0)
            DieWithError("recv() failed");

        printf("%s\n", echoBuffer);

        /*Check if message equals "Bye"*/
        last_three = &echoBuffer[strlen(echoBuffer) - 3];
        if (strcmp(last_three, "Bye") == 0) break;
    }
    close(sock);
    return 0;
}

int determineOption(int option) {
    switch (option) {
        case 0:
            connectToServer();
            break;
        case 1:
            getUserList();
            break;
        case 2:
            sendMessage();
            break;
        case 3:
            receiveMessage();
            break;
        case 4:
            send(sock, "4", sizeof("4"), 0) != sizeof("4");
            startChat();
            break;
        case 5:
            send(sock, "5", sizeof("5"), 0) != sizeof("5");
            connectToChat();
            break;
    }
    return 0;
}

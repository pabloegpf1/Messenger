#include <stdio.h> /* for printf() and fprintf() */
#include <string.h>
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h>     /* for close() */

#define RCVBUFSIZE 32                  /* Size of receive buffer */
void DieWithError(char *errorMessage); /* Error handling function */
const char username;
const char password;
char buffer[30];
int found = -1;
int userCount = 0;
int clntSocket;
char echoBuffer[RCVBUFSIZE]; /* Buffer for echo string */
int recvMsgSize;             /* Size of received message */
char *endptr;
int online = 0;
int actualUserIndex;

int determineOption(int option);

struct User {
    char *username;
    char *password;
    const char *messages[10];
    int messageCount;
};

struct User user[5];

int disconnect() {
    online = -1;
}

int HandleTCPClient(int socket) {
    clntSocket = socket;

    while (1) {
        /* Receive message from client */
        if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
            DieWithError("recv() failed");

        if (determineOption(strtol(echoBuffer, &endptr, 10)) == -1)
            return 0;
    }

    return 0;
}

int login() {
    memset(echoBuffer, '\0', sizeof(echoBuffer) * sizeof(char));
    /* Receive message from client */
    if ((recvMsgSize = recv(clntSocket, echoBuffer, sizeof(echoBuffer), 0)) < 0)
        DieWithError("recv() failed");

    for (int i = 0; i < userCount; i++) {
        if (strcmp(user[i].username, echoBuffer) == 0) {
            printf("User %d connected (%s)\n", i, user[i].username);
            found = i;
            actualUserIndex = i;
        }
    }

    if (found == -1 || userCount == 0) {
        user[userCount].username = (char *)malloc(strlen(echoBuffer) + 1);
        strcpy(user[userCount].username, echoBuffer);
        printf("User %d created: %s\n", userCount, user[userCount].username);
        userCount++;
        actualUserIndex = userCount - 1;
    }

    found = -1;
    return 0;
}

int getUserList() {
    memset(echoBuffer, '\0', sizeof(echoBuffer) * sizeof(char));
    sprintf(echoBuffer, "%d", userCount);

    if (send(clntSocket, echoBuffer, sizeof(echoBuffer), 0) != sizeof(echoBuffer))
        DieWithError("Error sending userList");

    for (int i = 0; i < userCount; i++) {
        if (send(clntSocket, user[i].username, sizeof(user[i].username), 0) != sizeof(user[i].username))
            DieWithError("Error sending userList");
        printf("%s\n", user[i].username);
    }
    return 0;
}

int sendMessage() {
    memset(echoBuffer, '\0', sizeof(echoBuffer) * sizeof(char));
    /* Receive message from client */
    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
        DieWithError("recv() failed");

    for (int i = 0; i < userCount; i++) {
        if (strcmp(user[i].username, echoBuffer) == 0) {
            printf("Sending message to: %s\n", echoBuffer);
            found = i;
        }
    }

    if (found == -1) {
        user[userCount].username = (char *)malloc(strlen(echoBuffer) + 1);
        strcpy(user[userCount].username, echoBuffer);
        printf("Sending message to new user (%d) %s\n", userCount, user[userCount].username);
        found = userCount;
        userCount++;
    }

    memset(echoBuffer, '\0', sizeof(echoBuffer) * sizeof(char));
    /* Receive message from client */
    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
        DieWithError("recv() failed");

    user[found].messages[user[found].messageCount] = malloc(strlen(echoBuffer) + 1);
    strcpy(user[found].messages[user[found].messageCount], echoBuffer);

    user[found].messageCount++;
    found = -1;
    return 0;
}

int getMessages() {
    printf("Messages for %s:\n", user[actualUserIndex].username);
    for (int i = 0; i < user[actualUserIndex].messageCount; i++) {
        printf("%s\n", user[actualUserIndex].messages[i]);
    }
    return 0;
}

int determineOption(int option) {
    switch (option) {
        case 0:
            login();
            break;
        case 1:
            getUserList();
            break;
        case 2:
            sendMessage();
            break;
        case 3:
            getMessages();
            break;
        case 4:
            printf("User disconnected\n");
            return -1;
            break;
        case 5:
            printf("User disconnected\n");
            return -1;
            break;
    }
    return 0;
}

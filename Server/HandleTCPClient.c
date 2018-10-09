#include <stdio.h> /* for printf() and fprintf() */
#include <string.h>
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h>     /* for close() */

#define RCVBUFSIZE 1000                /* Size of receive buffer */
void DieWithError(char *errorMessage); /* Error handling function */
const char username;
const char password;
char buffer[30];
int found = -1;
int userCount = 0;
int clntSocket;
char echoBuffer[RCVBUFSIZE]; /* Buffer for echo string */
char echoSend[RCVBUFSIZE];
int recvMsgSize; /* Size of received message */
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

        if (determineOption(strtol(echoBuffer, &endptr, 10)) == -1) {
            break;
        }
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
        user[userCount].messageCount = 0;
    }

    found = -1;
    return 0;
}

int getUserList() {
    memset(echoBuffer, '\0', sizeof(echoBuffer) * sizeof(char));
    sprintf(echoBuffer, "%d", userCount);

    if (send(clntSocket, echoBuffer, sizeof(echoBuffer), 0) != sizeof(echoBuffer))
        DieWithError("Error sending userList");

    memset(echoSend, '\0', sizeof(echoSend) * sizeof(char));
    for (int i = 0; i < userCount; i++) {
        strcat(echoSend, user[i].username);
        strcat(echoSend, "\n");
    }
    if (send(clntSocket, echoSend, sizeof(echoSend), 0) != sizeof(echoSend))
        DieWithError("Error sending userList");
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
        user[userCount].messageCount = 0;
    }

    memset(echoBuffer, '\0', sizeof(echoBuffer) * sizeof(char));
    /* Receive message from client */
    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
        DieWithError("recv() failed");

    printf("-----Size of echoBuffer %d\n", strlen(echoBuffer));

    user[found].messages[user[found].messageCount] = malloc(strlen(echoBuffer) + 1);
    strcpy(user[found].messages[user[found].messageCount], echoBuffer);

    user[found].messageCount++;

    for (int i = 0; i < user[found].messageCount; i++) {
        printf(">(%i)%s\n", i, user[found].messages[i]);
    }

    found = -1;
    return 0;
}

int getMessages() {
    int pos = 0;

    printf("Sending messages to %s\n", user[actualUserIndex].username);
    memset(echoSend, '\0', sizeof(echoSend) * sizeof(char));

    for (int i = 0; i < user[actualUserIndex].messageCount; i++) {
        pos += sprintf(&echoSend[pos], "%s\n", user[actualUserIndex].messages[i]);
        printf("%d%s\n", pos, user[actualUserIndex].messages[i]);
    }
    printf("%s", echoSend);

    if (send(clntSocket, echoSend, sizeof(echoSend), 0) != sizeof(echoSend))
        DieWithError("Error sending userList");
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
            return -1;
            break;
        case 5:
            printf("User disconnected\n");
            return -1;
            break;
    }
    return 0;
}

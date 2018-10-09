#include <stdio.h> /* for printf() and fprintf() */
#include <string.h>
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h>     /* for close() */

#define RCVBUFSIZE 1000 /* Size of receive buffer */
#define MAX_USERS 5     /* Max number of users registered in server */

void DieWithError(char *errorMessage); /* Error handling function */
const char username, password;
int clntSocket;
char echoBuffer[RCVBUFSIZE]; /* Buffer for echo string */
char echoSend[RCVBUFSIZE];   /* Buffer for sending message */
int recvMsgSize;             /* Size of received message */
char *endptr;

/* Data of user connected to the server */
int online = 0;
int userCount = 0;
int actualUserIndex;

/* Data for each user */
struct User {
    char *username;
    char *password;
    const char *messages[10];
    int messageCount;
};

int determineOption(int option);
struct User user[MAX_USERS];

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
    int found = -1;

    /* Receive message from client */
    memset(echoBuffer, '\0', sizeof(echoBuffer) * sizeof(char));
    if ((recvMsgSize = recv(clntSocket, echoBuffer, sizeof(echoBuffer), 0)) < 0)
        DieWithError("recv() failed");

    /* Try to find user */
    for (int i = 0; i < userCount; i++) {
        if (strcmp(user[i].username, echoBuffer) == 0) {
            printf("User %d connected (%s)\n", i, user[i].username);
            found = i;
            actualUserIndex = i;
        }
    }

    /* User not found */
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
    /* Send number of users registered */
    memset(echoBuffer, '\0', sizeof(echoBuffer) * sizeof(char));
    sprintf(echoBuffer, "%d", userCount);
    if (send(clntSocket, echoBuffer, sizeof(echoBuffer), 0) != sizeof(echoBuffer))
        DieWithError("Error sending userList");

    /* Store all the users */
    memset(echoSend, '\0', sizeof(echoSend) * sizeof(char));
    for (int i = 0; i < userCount; i++) {
        strcat(echoSend, user[i].username);
        strcat(echoSend, "\n");
    }

    /* Send user list */
    if (send(clntSocket, echoSend, sizeof(echoSend), 0) != sizeof(echoSend))
        DieWithError("Error sending userList");
    return 0;
}

int sendMessage() {
    int found = -1;

    /* Receive username from client */
    memset(echoBuffer, '\0', sizeof(echoBuffer) * sizeof(char));
    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
        DieWithError("recv() failed");

    /* Check if user is already registered */
    for (int i = 0; i < userCount; i++) {
        if (strcmp(user[i].username, echoBuffer) == 0) {
            printf("Sending message to: %s -> ", echoBuffer);
            found = i;
        }
    }

    /* User not found */
    if (found == -1) {
        user[userCount].username = (char *)malloc(strlen(echoBuffer) + 1);
        strcpy(user[userCount].username, echoBuffer);
        printf("Sending message to new User%d -> ", userCount);
        found = userCount;
        userCount++;
        user[userCount].messageCount = 0;
    }

    /* Receive message from client */
    memset(echoBuffer, '\0', sizeof(echoBuffer) * sizeof(char));
    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
        DieWithError("recv() failed");

    printf("%s\n", echoBuffer);

    /* Store message in user message array  */
    user[found].messages[user[found].messageCount] = malloc(strlen(echoBuffer) + 1);
    strcpy(user[found].messages[user[found].messageCount], echoBuffer);
    user[found].messageCount++;

    return 0;
}

int getMessages() {
    int pos = 0;

    printf("Sending messages to %s\n", user[actualUserIndex].username);
    memset(echoSend, '\0', sizeof(echoSend) * sizeof(char));

    /* Get list of users */
    for (int i = 0; i < user[actualUserIndex].messageCount; i++) {
        pos += sprintf(&echoSend[pos], "%s\n", user[actualUserIndex].messages[i]);
    }

    /* Send list of users */
    if (send(clntSocket, echoSend, sizeof(echoSend), 0) != sizeof(echoSend))
        DieWithError("Error sending userList");
    return 0;
}

int disconnect() {
    online = -1;
    return 0;
}

int determineOption(int option) {
    switch (option) {
        case 0:
            login();
            break;
        case 1:
            printf("Return user list!\n");
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

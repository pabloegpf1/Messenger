#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h> /* for close() */
#include <string.h>

#define RCVBUFSIZE 32 /* Size of receive buffer */
void DieWithError(char *errorMessage); /* Error handling function */
const char username;
const char password;
char buffer[30];
int found = -1;
int userCount = 0;
int clntSocket;
char echoBuffer[RCVBUFSIZE]; /* Buffer for echo string */
int recvMsgSize; /* Size of received message */
char *endptr;
int online = 0;
int actualUserIndex;

int determineOption(int option);
int push(struct message** head_ref, void *new_data, size_t data_size);
int printList(struct message*node, void (*fptr)(void *));
int printMessage(void *n);

struct User{
   char* username;
   char* password;
   const char *messages[10];
   int messageCount;
};

struct User user[5];

void disconnect(){
  online = 0;
}

int HandleTCPClient(int socket){

  clntSocket = socket;

  while(1){

      /* Receive message from client */
      if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
          DieWithError("recv() failed") ;

      if(determineOption(strtol(echoBuffer, &endptr,10)) == -1)
        DieWithError("Incorrect option") ;
  }

  return 0;
}

int login(){

  printf("Logging in...\n");
  memset(echoBuffer, '\0', sizeof(echoBuffer)*sizeof(char));

  /* Receive message from client */
  if ((recvMsgSize = recv(clntSocket, echoBuffer, sizeof(echoBuffer), 0)) < 0)
      DieWithError("recv() failed") ;

  for(int i=0; i<userCount; i++){
    if(strcmp(user[i].username,echoBuffer) == 0){
      printf("User found\n",echoBuffer);
      found = i;
      actualUserIndex = i;
    }
  }

  if(found == -1 || userCount == 0){
    user[userCount].username = (char *)malloc( strlen(echoBuffer) + 1 );
    strcpy(user[userCount].username,echoBuffer);
    printf("User %d created: %s\n", userCount, user[userCount].username);
    userCount++;
    actualUserIndex = userCount-1;
  }

  found = -1;
  return 0;
}

int getUserList(){
  printf("-------UserList(%d)-------\n", userCount);
  for(int i=0; i<userCount;i++){
    printf("%s\n",user[i].username);
  }
  printf("--------------------------\n", userCount);
  return 0;
}

int sendMessage(){

  memset(echoBuffer, '\0', sizeof(echoBuffer)*sizeof(char));
  /* Receive message from client */
  if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
      DieWithError("recv() failed") ;

  for(int i=0; i<userCount; i++){
    if(strcmp(user[i].username,echoBuffer) == 0){
      printf("Sending message to: %s\n",echoBuffer);
      found = i;
    }
  }

  if(found == -1){
    user[userCount].username = (char *)malloc( strlen(echoBuffer) + 1 );
    strcpy(user[userCount].username,echoBuffer);
    printf("Sending message to new user (%d) %s\n", userCount, user[userCount].username);
    found = userCount;
    userCount++;
  }

  memset(echoBuffer, '\0', sizeof(echoBuffer)*sizeof(char));
  /* Receive message from client */
  if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
      DieWithError("recv() failed") ;

  printf("(Test) Sending to: %s message: %s\n",user[found].username,echoBuffer);

  user[found].messages[user[found].messageCount] = malloc(strlen(echoBuffer) + 1);
  strcpy(user[found].messages[user[found].messageCount],echoBuffer);

  user[found].messageCount++;
  found = -1;
  return 0;
}

int getMessages(){
  printf("Messages for %s:\n", user[actualUserIndex].username);
  for(int i = 0; i<user[actualUserIndex].messageCount; i++){
    printf("%s\n", user[actualUserIndex].messages[i]);
  }
  return 0;
}


int determineOption(int option){
  switch(option) {
    case 0:
      login();
      break;
    case 1  :
      getUserList();
      break;
    case 2  :
      sendMessage();
      break;
    case 3  :
      getMessages();
      break;
    case 4  :
      printf("Option 4\n");
      break;
    case 5  :
      printf("Option 5\n");
      DieWithError("Server closed") ;
      break;
  }return 0;
}

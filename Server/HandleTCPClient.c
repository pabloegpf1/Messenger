#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h> /* for close() */
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
int online;

int determineOption(int option);

struct User{
   char username[30];
   char password[30];
};

struct User user[5];

int HandleTCPClient(int socket){

  clntSocket = socket;
  online = -1;

  while(1){

    /* Receive message from client */
    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
        DieWithError("recv() failed") ;

    if(determineOption(strtol(echoBuffer, &endptr,10)) == -1){
      return 1;
    }
  }

  return 0;
}

int login(){

  printf("Logging in...\n");
  /* Receive message from client */
  if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
      DieWithError("recv() failed") ;

  for(int i=0; i<userCount;i++){
    if(strncmp(user[i].username,echoBuffer,RCVBUFSIZE) == 0){
      printf("%s\n",user[i].username);
      printf("\nUser found\n",echoBuffer);
      found = i;
    }
  }

  if(found != -1 || userCount == 0){
    if(found == -1)
      found = 0;
    strcpy(user[found].username,echoBuffer);
    printf("\nUser %d created: %s\n", found, user[found].username);
    userCount++;
  }

  online = 1;
  found = -1;
  return 0;
}

int getUserList(){
  printf("UserList: (%d)\n", userCount);
  for(int i=0; i<userCount;i++){
    printf("%s\n",user[i].username);
  }

  return 0;
}

int determineOption(int option){
  switch(option) {
    case 1  :
      getUserList();
      break;
    case 2  :
      printf("Option 2\n");
      break;
    case 3  :
      printf("Option 3\n");
      break;
    case 4  :
      printf("Option 4\n");
      break;
    case 5  :
      printf("Option 5\n");
      DieWithError("Server closed") ;
      break;
    default:
    printf("online %d\n", online);
      if(online == 1)
        return -1;
      login();
      online = 1;
      break;
  }return 0;
}

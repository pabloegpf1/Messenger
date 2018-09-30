#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h> /* for sockaddr_in and inet_addr() */
#include <stdlib.h> /* for atoi() */
#include <string.h> /* for memset() */
#include <unistd.h> /* for close() */

#define RCVBUFSIZE 32 /* Size of receive buffer */

int sock;                         /* Socket descriptor */
char *echoString;                 /* String to send to echo server */
char echoBuffer[RCVBUFSIZE];      /* Buffer for echo string */
unsigned int echoStringLen;       /* Length of string to echo */
int bytesRcvd, totalBytesRcvd;    /* Bytes read in single recv() and total bytes read */
int online = -1;

void DieWithError(char *errorMessage); /* Error handling function */
int determineOption(int option);

int main(int argc, char *argv[]){

  if (argc != 1){ /* Test for correct number of arguments */
    fprintf(stderr, "Error: Client does not accept arguments\n");
    exit(1);
  }
  while(1){
    /*Display menu*/
    int option =-1;
    while(option<0||option>5){
      printf("\n------------------\nCommand\n0. Connect to the server\n1. Get the user list\n2. Send a message\n3. Get my messages\n4. Initiate a chat with my friend\n5. Chat with my friend\nYour option<enter a number>:");
      fscanf(stdin,"%d",&option);
      if(option<0||option>5){
        printf("\nIncorrect Option!,try again:\n");
      }else{
        determineOption(option);
      }
    }
  }

  printf("\n"); /* Print a final linefeed */
  close(sock);
  exit(0);
}

int connectToServer(){
  struct sockaddr_in echoServAddr;  /* Echo server address */

  /* Create a reliable, stream socket using TCP */
  char ipAdress[10], port[10], username[30], password[30];
  unsigned short echoServPort;
  char *end;

  printf("Please enter the IP adress: ");
  fscanf(stdin,"%s",&ipAdress);
  printf("Please enter the Port Number: ");
  fscanf(stdin,"%s",port);
  echoServPort = strtol(port,&end,0);
  printf("Connecting.....\n");

  /* Create a reliable, stream socket using TCP*/
  if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    DieWithError(" socket () failed");

  /* Construct the server address structure*/
  memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
  echoServAddr.sin_family = AF_INET;                  /* Internet address family */
  echoServAddr.sin_addr.s_addr = inet_addr(ipAdress); /* Server IP address */
  echoServAddr.sin_port = htons(echoServPort);                /* Server port */

  /* Establish the connection to the echo server */
  if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
    DieWithError(" connect () failed");

  if(online == -1){
    /* Send the string to the server */
  if (send(sock, "0", sizeof("0"), 0) != sizeof("0"))
    DieWithError("Error sending option");
  }

  printf("Connected!\nWelcome,Please Log In:\n");
  printf("Username: ");
  fscanf(stdin,"%s",&username);
  printf("Password: ");
  fscanf(stdin,"%s",&password);

  echoStringLen = strlen(username);
    /* Send the string to the server */
  if (send(sock, username, echoStringLen, 0) != echoStringLen)
    DieWithError("Error sending username");

  online = 1;
    return 0;
  }

int determineOption(int option){
  switch(option) {
    case 0  :
      if(online == 1){
        if (send(sock, "0", sizeof("0"), 0) != sizeof("0"))
          DieWithError("Error sending option");
      }
      connectToServer();
      break;
    case 1  :
      if (send(sock, "1", sizeof("1"), 0) != sizeof("1"))
        DieWithError("Error sending option");
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
      if (send(sock, "5", sizeof("5"), 0) != sizeof("5"))
        DieWithError("Error sending option");
      break;
  }return 0;
}

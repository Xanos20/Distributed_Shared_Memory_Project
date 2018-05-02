#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

#define PORT 4450
#define SHARED_ARRAY_SIZE 10


int shared_array[SHARED_ARRAY_SIZE];

int establish_connection(char* input) {
  struct hostent* server_name = gethostbyname(input);
  if(server_name == NULL) {
    fprintf(stderr, "Unable to find host %s\n", input);
    exit(1);
  }
  int client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(client_socket == -1) {
    perror("socket failed");
    exit(2);
  }
  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_port = htons(PORT)
  };
  // fill in server address
  bcopy((char*)server_name->h_addr, (char*)&addr.sin_addr.s_addr, server_name->h_length);

  if(connect(client_socket, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) != 0) {
    perror("Connect to server failed: ");
    exit(2);
  }

  return client_socket;

}


int main (int argc, char* argv[]) {
  // server_name should be localhost for now
  int client_fd = establish_connection(argv[1]);
  int buffer[SHARED_ARRAY_SIZE];
  int bytes_read = read(client_fd, buffer, sizeof(int) * SHARED_ARRAY_SIZE);
  if(bytes_read < 0) {
    perror("read failed");
    exit(2);
  }
  for(int i = 0; i < SHARED_ARRAY_SIZE; i++){
    shared_array[i] = buffer[i];
  }

  for(int i = 0; i < SHARED_ARRAY_SIZE; i++){
    printf("Server sent: %d\n", shared_array[i]);
  }
  char buffer[SHARED_ARRAY_SIZE];
  while(true){
    fgets(buffer, SHARED_ARRAY_SIZE-1, stdin);
    if(strcmp(buffer, "quit") == 0){
      break;
    }
    
  }
  // write a test array and send it to other clients

  close(client_fd);
  return 0;
}

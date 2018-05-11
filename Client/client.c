#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <stdbool.h>

#define PORT 4444
#define SHARED_ARRAY_SIZE 10


char SHARED_ARRAY[SHARED_ARRAY_SIZE];
int SERVER_FD;

//TODO: use a lock to make sure the updates are only happening at one time

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



/*
 * pthread function to update array from server
 */
void* pull_array(void* arg){
  while(true){
    int bytes_read = read(SERVER_FD, SHARED_ARRAY, sizeof(char) * SHARED_ARRAY_SIZE);
    if(bytes_read < 0) {
      perror("read failed for pull_array");
      exit(2);
    }
    for(int i = 0; i < SHARED_ARRAY_SIZE; i++){
      printf("Server sent: %c\n", SHARED_ARRAY[i]);
    }
    printf("SHARED_ARRAY_SIZE is %d\n", SHARED_ARRAY_SIZE);
  }
}

/*
 * pthread function to push updates to server
 */
void* push_array(void* arg){
  printf("to change array type SHARED_ARRAY_SIZE letters\n");
  char buffer[SHARED_ARRAY_SIZE+1];
  while(true){
    fgets(buffer, SHARED_ARRAY_SIZE+1, stdin);
    // TODO: test if the array has been updated correctly
    printf("Array = ");
    if(buffer[0] != '\n'){
      for(int i = 0; i < SHARED_ARRAY_SIZE; i++) {
        SHARED_ARRAY[i] = buffer[i];
        printf("%c", SHARED_ARRAY[i]);
      }
      printf("\n");
      if(write(SERVER_FD, SHARED_ARRAY, sizeof(char) * SHARED_ARRAY_SIZE) < 0) {
        perror("Write failed for push_array");
        exit(2);
      }
    }
  }
}



int main (int argc, char* argv[]) {
  // server_name should be localhost for now
  SERVER_FD = establish_connection(argv[1]);

  // thread for querying updates from server_fd
  pthread_t pull_thread;

  if(pthread_create(&pull_thread, NULL, pull_array, NULL)) {
    perror("pthread_create failed for pull_thread");
    exit(2);
  }

  // thread for updating the client array
  pthread_t push_thread;

  if(pthread_create(&push_thread, NULL, push_array, NULL)) {
    perror("pthread_create failed for push_thread");
    exit(2);
  }

  if(pthread_join(pull_thread, NULL)) {
    perror("pthread_join failed");
    exit(2);
  }

  if(pthread_join(push_thread, NULL)) {
    perror("pthread_join failed");
    exit(2);
  }

  close(SERVER_FD);
  return 0;
}

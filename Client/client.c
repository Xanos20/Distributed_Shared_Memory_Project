#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


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
    .sin_port = htons(0)   // choose first available port
  };
  // fill in server address
  bcopy((char*)server_name->h_addr, (char*)&addr.sin_addr.s_addr, server_name->h_length);

  if(connect(client_socket, (struct sockaddr*)&addr, sizeof(struct sockaddr_in))) {
    perror("connect failed");
    exit(2);
  }

  return client_socket;

}


int main (int argc, char* argv[]) {
  // server_name should be localhost for now
  int client_fd = establish_connection(argv[1]);
  char buffer[256];
  int bytes_read = read(client_fd, buffer, 256);
  if(bytes_read < 0) {
    perror("read failed");
    exit(2);
  }

  printf("Server sent: %s\n", buffer);

  close(client_fd);
  return 0;
}

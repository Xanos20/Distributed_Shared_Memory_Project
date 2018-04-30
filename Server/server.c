#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


// The number of connecting clients to listen to
#define PENDING_CONNECTIONS 1
#define PORT 4448

// Struct stores the ip, port, and status of each client
typedef struct client_node {
  char ipstr[INET_ADDRSTRLEN];
  uint16_t port;
  int socket;
  bool status;
} client_node_t;

// Global Values
client_node_t client_array[PENDING_CONNECTIONS];

/*
  * return the server scoket descriptor
*/
int start_server() {
  // This line of code sets up an internet socket using TCP with no special protocol options.
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    perror("Establishment of the server socket failed");
    exit(2);
  }
  // Fill in a struct sockaddr_in, which is short for an internet socket address.
  //  This tells the socket where it where it should listen for incoming connections
  struct sockaddr_in addr;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT); // accept connections on port 4444

  //Tell the socket what address to listen from
  if(bind(server_fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) != 0) {
    perror("bind failed");
    exit(2);
  }

  //Make the socket listen for PENDING_CONNECTIONS number of connections
  if(listen(server_fd, PENDING_CONNECTIONS) != 0) {
    perror("listen failed");
    exit(2);
  }

  // return the socket fd
  return server_fd;
}

void initialize_client_array() {
  for(int i=0; i < PENDING_CONNECTIONS; i++){
    client_array[i].status = false;
    //client_array[i].ipstr;
    client_array[i].port = 0;
    client_array[i].socket = 0;
  }
}


int main(int argc ,char* argv[]) {
  int server_fd = start_server();
  initialize_client_array();
  // allow n clients to connect

  printf("I am before the for loop\n");
  int c = 0;
  for (; c < PENDING_CONNECTIONS; c++) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_length = sizeof(struct sockaddr_in);
    int client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_length);
    if(client_socket == -1) {
      printf("For Client %d ", c+1);
      perror("Client could not connect");
      exit(2);
    } else {
      printf("Server accepted new client\n");
      // fill in client info to client_array
      client_array[c].status = true;
      // get the client IP and fill it into the array
      inet_ntop(AF_INET, &client_addr.sin_addr, client_array[c].ipstr, INET_ADDRSTRLEN);
      // get the client port
      if (read(client_socket, &(client_array[c].port), sizeof(uint16_t)) < 0) {
        perror("Incorrect port from client in read");
        exit(2);
      }
      // set the socket
      client_array[c].socket = client_socket;
    }
    // TODO: Test message
    char* msg = "Hello client.\n";
    if (write(client_array[0].socket, msg, strlen(msg)) < 0) {
      perror("Write failed");
      exit(2);
    }

  }


  close(server_fd);
  return 0;
}

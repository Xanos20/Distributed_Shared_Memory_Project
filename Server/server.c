#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <poll.h>


// The number of connecting clients to listen to
#define PENDING_CONNECTIONS 3
#define PORT 4450
#define SHARED_ARRAY_SIZE 10

// Struct stores the ip, port, and status of each client
typedef struct client_node {
  char ipstr[INET_ADDRSTRLEN];
  int socket;
  bool status;
} client_node_t;

typedef struct shared_array{
  int array[SHARED_ARRAY_SIZE];
  pthread_mutex_t m;
} shared_array_t;

typedef struct request_queue {
  int sockets[PENDING_CONNECTIONS]; // the clients that requested to change shared_array
  int requests; // number of requests to update shared_array
  int head; // current position
} request_queue_t;

// Global Values
client_node_t client_array[PENDING_CONNECTIONS];
shared_array_t shared_array;
request_queue_t request_queue;

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
    client_array[i].socket = 0;
  }
}

void initialize_shared_array(){
  for(int i = 0; i < sizeof(shared_array.array); i++) {
    shared_array.array[i] = i;
  }
  pthread_mutex_init(&shared_array.m, NULL);
}

void initialize_request_queue() {
  for(int i = 0; i < PENDING_CONNECTIONS; i++) {
    request_queue.sockets[i] = 0;
    request_queue.requests = 0;
    request_queue.head = -1;
  }
}

/*
 * For all clients, send the updated array
*/
void distribute() {
  char user_input[20];
  int i = 0;

}


int main(int argc ,char* argv[]) {
  // create the shared memory region
  initialize_shared_array();

  int server_fd = start_server();
  initialize_client_array();



  // allow n clients to connect
  printf("SERVER UP AND RUNNING\n");
  for (int c = 0; c < PENDING_CONNECTIONS; c++) {
    printf("Server has %d clients\n", c);
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
      // set the socket
      client_array[c].socket = client_socket;
    }
    printf("send message\n");
    // TODO: Test message
    if (write(client_array[c].socket, shared_array.array, sizeof(int) * SHARED_ARRAY_SIZE) < 0) {
      perror("Write failed");
      exit(2);
    }
  }
  // TODO:Take requests
  // Complete requests and update client_array
  // Distribute the new array to all clients
  // Figure out a way to quit


  // make request_queue_
  initialize_request_queue();
  // distribute test array to all clients
  // loop over array and read from each client
  //get all file descriptors and put into new array
  struct pollfd fds[PENDING_CONNECTIONS];
  for(int i = 0; i < PENDING_CONNECTIONS; i++){
    fds[i].fd = client_array[i].socket;
  }
  while(true){
    if(poll(fds, PENDING_CONNECTIONS, 500) < 0) {
      perror("Poll failed");
      exit(2);
    }

  }





  close(server_fd);
  return 0;
}

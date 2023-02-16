#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <cstring>
#include <arpa/inet.h>

#include "potato.h"
#include "helper.hpp"

using namespace std;

int main(int argc, char * argv[]) {
  // Parse command line arguments
  if (argc != 4) {
    cerr << "Usage: " << argv[0] << " <port_num> <num_players> <num_hops>" << endl;
    exit(1);
  }
  const char * port = argv[1];
  int num_players = atoi(argv[2]);
  int num_hops = atoi(argv[3]);

  if (num_players <= 1) {
    cerr << "Error: num_hops should be greater than 1" << endl;
  }

  if (num_hops < 0 || num_hops > 512) {
    cerr << "Error: num_hops should be between 0 and 512." << endl;
    exit(1);
  }

  cout << "Potato Ringmaster" << endl;
  cout << "Players = " << num_players << endl;
  cout << "Hops = " << num_hops << endl; 

  //build a socket, blind, and listen it as ringmaster server
  //initialize getaddrinfo
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname = NULL;
  const char *port_num = port;

  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family   = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags    = AI_PASSIVE;

  status = getaddrinfo(hostname, port_num, &host_info, &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname << "," << port_num << ")" << endl;
    return -1;
  }

  //create socket for ringmaster server
  cout << "server socket created!!!!!!" << endl;
  socket_fd = socket(host_info_list->ai_family, 
	host_info_list->ai_socktype, 
	host_info_list->ai_protocol);
  if (socket_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port_num << ")" << endl;
    return -1;
  }

  //bind socket
  cout << "server socket binded" << endl;
  int yes = 1;
  status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot bind socket" << endl;
    cerr << "  (" << hostname << "," << port_num << ")" << endl;
    return -1;
  } 

  //listen mode
  cout << "server socket listened" << endl;
  status = listen(socket_fd, num_players);
  if (status == -1) {
      cerr << "Error: cannot listen on socket" << endl; 
      cerr << "  (" << hostname << "," << port_num << ")" << endl;
      return -1;
  }
  
  //pre game setting for each player 
  int player_socket_fd[num_players];
  int player_port[num_players];
  string player_ip[num_players];

  for(int i = 0; i < num_players; i++) {
    //accept connection for a player 
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    player_socket_fd[i] = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (player_socket_fd[i] == -1) {
      cerr << "Error: cannot accept connection on socket" << endl;
      return -1;
    }
    
    //receive initial message from player 
    char buffer[200];
    recv(player_socket_fd[i], buffer, 5, 0);
    cout << "player " << i << " is ready to play" << endl;

    //send info to the player (id, num player, neighbors)
    send(player_socket_fd[i], &i, sizeof(i), 0);
    send(player_socket_fd[i], &num_players, sizeof(num_players), 0);

    //Get the local IP address and port of player
    struct sockaddr_in * addr = (struct sockaddr_in *)&socket_addr;
    player_ip[i] = inet_ntoa(addr->sin_addr);
    //cout << "recv player ip: " << player_ip[i] << endl;
    int port;
    recv(player_socket_fd[i], &port, sizeof(port), 0);
    player_port[i] = port;
  
    //cout << "recv player port number: " << player_port[i] << endl;
  }

  //send neighbor info to each player
  for(int i = 0; i < num_players; i++) {
    int neighbor_id = (i + 1) % num_players;
    int ip_length = player_ip[neighbor_id].length() + 1;
    send(player_socket_fd[i], &ip_length, sizeof(int), 0);
    send(player_socket_fd[i], &player_port[neighbor_id], sizeof(int), 0);
    send(player_socket_fd[i], player_ip[neighbor_id].c_str(), player_ip[neighbor_id].length() + 1, 0);
    cout << "master send player port : " << player_port[i] << endl;
    cout << "master send player ip : " << player_ip[i] << endl;
  } 

  //create potato object
  Potato potato;
  potato.hops = num_hops;

  //select a random player to start the game 
  srand((unsigned int)time(NULL) + num_players);
  int random_num = rand() % num_players;
  cout << "Ready to start the game, sending potato to player " << 0 << endl;
  cout << "hops of potato: " << potato.hops <<endl;
  int test = 4 ;
  int bytes_sent = send(player_socket_fd[0], &test, sizeof(test), 0);
  if (bytes_sent != sizeof(test)) {
    cout << "error" <<endl;
    // error occurred
    // handle the error
  }
  cout << "here"<<endl;



  while(1){
    
  }
}
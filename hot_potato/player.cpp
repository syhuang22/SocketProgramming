#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <algorithm>
#include <random>
#include "potato.h"
#include <mutex>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

#include <cstdlib>
#include <iostream>
#include <string>
#include <cstring>

std::mutex output_mutex;

using namespace std;


int create_server(const char *port) {
    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;
    const char *hostname = NULL;
    // const char *port     = "4444";

    memset(&host_info, 0, sizeof(host_info));

    host_info.ai_family   = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags    = AI_PASSIVE;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
        cerr << "Error: cannot get address info for host" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    } //if

    socket_fd = socket(host_info_list->ai_family, 
		host_info_list->ai_socktype, 
		host_info_list->ai_protocol);
    if (socket_fd == -1) {
        cerr << "Error: cannot create socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    } //if

    int yes = 1;
    status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        cerr << "Error: cannot bind socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    } //if

    status = listen(socket_fd, 100);
    if (status == -1) {
        cerr << "Error: cannot listen on socket" << endl; 
        cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
    } 

    freeaddrinfo(host_info_list);
    return socket_fd;
}

int create_client(const char * hostname, const char * port) {
    struct addrinfo host_info;
    struct addrinfo * host_info_list;
    int socket_fd;
    int status;

    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
        cerr << "Error: cannot get address info for host" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        exit(EXIT_FAILURE);
    }

    socket_fd = socket(host_info_list->ai_family,
                        host_info_list->ai_socktype,
                        host_info_list->ai_protocol);
    if (socket_fd == -1) {
        cerr << "Error: cannot create socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        exit(EXIT_FAILURE);
    }

    status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        cerr << "Error: cannot connect to socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(host_info_list);
    return socket_fd;
}

void print_trace(Potato received_potato) {
    cout<< "Trace of potato:" <<endl;
    for(int i = 0; i < received_potato.index; i++) {
    cout << received_potato.trace[i];
    if (i != received_potato.index - 1) {
        cout << ",";
    } else {
        cout << endl;
    }
    }
}

int main(int argc, char * argv[]) {
    // Parse command line arguments
    if (argc != 3) {
        cout << "player <machine_name> <port_num>" << endl;
        return 1;
    }
    int id;
    int num_players;
    const char * machine_name  = argv[1];
    const char * port_num = argv[2];

    //create and connect to ringmaster as a client (MC for ringmaster client) (MC)
    int socket_fd_mc = create_client(machine_name, port_num);

    //send initial message to ringmaster server ********
    const char *message = "Ready";
    send(socket_fd_mc, message, strlen(message), 0);

    //receive id and number of players from ringmaster server
    recv(socket_fd_mc, &id, sizeof(id), 0);
    recv(socket_fd_mc, &num_players, sizeof(num_players), 0);
    cout << "Connected as player " << id << " out of " << num_players
        << " total players" << endl;
    
    //build a socket, blind, and listen it as player server (PS)
    int socket_fd_ps = create_server("0");

    //Get the local IP address and port of socket_fd_ps
    int* port_n = new int;
    struct sockaddr_in socketAddr;
    socklen_t socketAddrLen = sizeof(socketAddr);
    if (getsockname(socket_fd_ps, (struct sockaddr*)&socketAddr, &socketAddrLen) == -1) {
        cerr << "Error: cannot getsockname" << endl;
        return -1;
    }
    *port_n = ntohs(socketAddr.sin_port);
    send(socket_fd_mc, port_n, sizeof(*port_n), 0);

    //receive neighbor's ip from ringmaster server
    int neighbor_port;
    int ip_length;
    recv(socket_fd_mc, &ip_length, sizeof(int), 0);
    char neighbor_ip[ip_length];
    recv(socket_fd_mc, &neighbor_port, sizeof(int), 0);
    recv(socket_fd_mc, neighbor_ip, ip_length, MSG_WAITALL);
    neighbor_ip[sizeof(neighbor_ip)-1] = '\0';

    //cast machine name and port number 
    string neighbor_port_str = to_string(neighbor_port);
    const char * port_num_pc = neighbor_port_str.c_str();
    const char * hostname_pc = neighbor_ip;

    //cout << "neighbor's hostname: " << hostname_pc << endl;
    //cout << "neighbor's port number: " << port_num_pc << endl;

    //create and connect to right player as a client (PC)****************
    int socket_fd_pc = create_client(hostname_pc, port_num_pc);

    //accept connection as player server (PS)
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    int socket_fd_client_ps;
    socket_fd_client_ps = accept(socket_fd_ps, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (socket_fd_client_ps == -1) {
        cerr << "Error: cannot accept connection on socket" << endl;
        return -1;
    } 
    
    int passes;
    Potato received_potato;
    while (true) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(socket_fd_mc, &readfds);
        FD_SET(socket_fd_pc, &readfds);
        FD_SET(socket_fd_client_ps, &readfds);

        int max_fd = max({ socket_fd_mc, socket_fd_pc, socket_fd_client_ps }) + 1; // get the max fd value
        int result = select(max_fd, &readfds, NULL, NULL, NULL); 
        if (result == -1) {
            perror("select error");
            exit(1);
        }
        if (FD_ISSET(socket_fd_mc, &readfds)) {
            //std::lock_guard<std::mutex> guard(output_mutex);
            recv(socket_fd_mc, &received_potato, sizeof(received_potato), 0);
            if(!received_potato.isFirstPass || received_potato.hops == -1) {
                close(socket_fd_mc);
                close(socket_fd_pc);
                close(socket_fd_client_ps);
                break;
            }
            //cout << "from ringmaster" <<endl;
        } else if (FD_ISSET(socket_fd_pc, &readfds)) { 
            //std::lock_guard<std::mutex> guard(output_mutex);
            recv(socket_fd_pc, &received_potato, sizeof(received_potato), 0);
            if(received_potato.isFirstPass) {
                close(socket_fd_mc);
                close(socket_fd_pc);
                close(socket_fd_client_ps);
                break;
            }
        } else if (FD_ISSET(socket_fd_client_ps, &readfds)) {
            //std::lock_guard<std::mutex> guard(output_mutex);
            recv(socket_fd_client_ps, &received_potato, sizeof(received_potato), 0);
            if(received_potato.isFirstPass) {
                close(socket_fd_mc);
                close(socket_fd_pc);
                close(socket_fd_client_ps);
                break;
            }
        }
        //cout << "Player: " << id << " I've received the potato with hops: " << received_potato.hops << endl;
        assert(received_potato.hops != 0);
        received_potato.isFirstPass = 0;
        received_potato.hops--;
        received_potato.trace[received_potato.index] = id;
        received_potato.index++;

        //print trace
        //print_trace(received_potato);

        //cout << "hops left: " << received_potato.hops << endl;
        if (received_potato.hops > 0) { // pass to one of the neighbors 
            mt19937 rng(std::random_device{}());
            // randomly select a neighbor
            uniform_int_distribution<int> dist(0, 1);
            int random_neighbor = dist(rng);
            // srand((unsigned int)time(NULL) + 2);
            // int random_neighbor = rand() % 2; 
            if (random_neighbor == 0) {
                //std::lock_guard<std::mutex> guard(output_mutex);
                // Pass the potato to the left neighbor
                send(socket_fd_client_ps, &received_potato, sizeof(received_potato), 0);
                int lef_id = (id + num_players - 1) % num_players;
                cout << "Sending potato to "<< lef_id << endl;
            } else {
                //std::lock_guard<std::mutex> guard(output_mutex);
                // Pass the potato to the right neighbor
                send(socket_fd_pc, &received_potato, sizeof(received_potato), 0);
                int right_id = (id + 1) % num_players;
                cout << "Sending potato to "<< right_id << endl;
            }
        } else if (received_potato.hops == 0){ //pass potato back to ringmaster
            //std::lock_guard<std::mutex> guard(output_mutex);
            send(socket_fd_mc, &received_potato, sizeof(received_potato), 0);
            if(received_potato.trace[received_potato.index-2]!=id){
                cout << "I'm it" << endl;
            }
            break;
        }
    }
    //close sockets
    close(socket_fd_mc);
    close(socket_fd_pc);
    close(socket_fd_client_ps);
    
    return 1;
}
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <algorithm>
#include "potato.h"
#include "helper.hpp"

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

    //new socket_fd_mc (MC)
    int socket_fd_mc = create_client(machine_name, port_num);

    // //create and connect to ringmaster as a client (MC for ringmaster client)
    // int status_mc;
    // int socket_fd_mc;
    // struct addrinfo host_info_mc;
    // struct addrinfo *host_info_list_mc;

    // memset(&host_info_mc, 0, sizeof(host_info_mc));
    // host_info_mc.ai_family   = AF_UNSPEC;
    // host_info_mc.ai_socktype = SOCK_STREAM;

    // status_mc = getaddrinfo(machine_name, port_num, &host_info_mc, &host_info_list_mc);
    // if (status_mc != 0) {
    //     cerr << "Error: cannot get address info for host" << endl;
    //     cerr << "  (" << machine_name << "," << port_num << ")" << endl;
    //     return -1;
    // } 

    // socket_fd_mc = socket(host_info_list_mc->ai_family, 
	// 	host_info_list_mc->ai_socktype, 
	// 	host_info_list_mc->ai_protocol);
    // if (socket_fd_mc == -1) {
    //     cerr << "Error: cannot create socket" << endl;
    //     cerr << "  (" << machine_name << "," << port_num << ")" << endl;
    //     return -1;
    // } 

    // cout << "Connecting to " << machine_name << " on port " << port_num << "..." << endl;

    
    // //connect to the host server (MC)
    // status_mc = connect(socket_fd_mc, host_info_list_mc->ai_addr, host_info_list_mc->ai_addrlen);
    // if (status_mc == -1) {
    //     cerr << "Error: cannot connect to socket" << endl;
    //     cerr << "  (" << machine_name << "," << port_num << ")" << endl;
    //     return -1;
    // }


    //send initial message to ringmaster server ********
    const char *message = "Ready";
    send(socket_fd_mc, message, strlen(message), 0);

    //receive id and number of players from ringmaster server
    recv(socket_fd_mc, &id, sizeof(id), 0);
    recv(socket_fd_mc, &num_players, sizeof(num_players), 0);
    cout << "Connected as player " << id << " out of " << num_players
        << " total players" << endl;
    
    //new socket_fd_ps (PS)
    int socket_fd_ps = create_server("0");

    // //build a socket, blind, and listen it as player server (PS)
    // //initialize getaddrinfo
    // int status_ps;
    // int socket_fd_ps;
    // struct addrinfo host_info_ps;
    // struct addrinfo *host_info_list_ps;
    // const char * hostname = NULL;
    // const char * port_num_PS = "0";

    // memset(&host_info_ps, 0, sizeof(host_info_ps));

    // host_info_ps.ai_family   = AF_UNSPEC;
    // host_info_ps.ai_socktype = SOCK_STREAM;
    // host_info_ps.ai_flags    = AI_PASSIVE;

    // status_ps = getaddrinfo(hostname, port_num_PS, &host_info_ps, &host_info_list_ps);
    // if (status_ps != 0) {
    //     cerr << "Error: cannot get address info for host" << endl;
    //     cerr << "  (" << hostname << "," << port_num_PS << ")" << endl;
    //     return -1;
    // }

    // //create socket for player server (PS)
    // //cout << "player server socket created!!!!!!" << endl;
    // socket_fd_ps = socket(host_info_list_ps->ai_family, 
	//     host_info_list_ps->ai_socktype, 
	//     host_info_list_ps->ai_protocol);
    // if (socket_fd_ps == -1) {
    //     cerr << "Error: cannot create socket" << endl;
    //     cerr << "  (" << hostname << "," << port_num_PS << ")" << endl;
    //     return -1;
    // }

    // //bind socket (PS)
    // //cout << "player server socket binded" << endl;
    // int yes = 1;
    // status_ps = setsockopt(socket_fd_ps, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    // status_ps = bind(socket_fd_ps, host_info_list_ps->ai_addr, host_info_list_ps->ai_addrlen);
    // if (status_ps == -1) {
    //     cerr << "Error: cannot bind socket" << endl;
    //     cerr << "  (" << hostname << "," << port_num << ")" << endl;
    //     return -1;
    // } 

    // //listen mode (PS)
    // //cout << "player server socket listened" << endl;
    // status_ps = listen(socket_fd_ps, num_players);
    // if (status_ps == -1) {
    //     cerr << "Error: cannot listen on socket" << endl; 
    //     cerr << "  (" << hostname << "," << port_num << ")" << endl;
    //     return -1;
    // }

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

    //PC
    int socket_fd_pc = create_client(hostname_pc, port_num_pc);

    // //create and connect to right player as a client (PC)****************
    // int status_pc;
    // int socket_fd_pc;
    // struct addrinfo host_info_pc;
    // struct addrinfo *host_info_list_pc;
    // //const char * port_num_pc = "5555";

    // memset(&host_info_pc, 0, sizeof(host_info_pc));
    // host_info_pc.ai_family   = AF_UNSPEC;
    // host_info_pc.ai_socktype = SOCK_STREAM;

    // status_pc = getaddrinfo(hostname_pc, port_num_pc, &host_info_pc, &host_info_list_pc);
    // if (status_pc != 0) {
    //     cerr << "Error: cannot get address info for host" << endl;
    //     cerr << "  (" << hostname_pc << "," << port_num_pc << ")" << endl;
    //     return -1;
    // } 

    // // create socket (PC)
    // socket_fd_pc = socket(host_info_list_pc->ai_family, 
	//     host_info_list_pc->ai_socktype, 
	// 	host_info_list_pc->ai_protocol);
    // if (socket_fd_pc == -1) {
    //     cerr << "Error: cannot create socket" << endl;
    //     cerr << "  (" << hostname_pc << "," << port_num_pc << ")" << endl;
    //     return -1;
    // } 
  
    // cout << "Connecting to " << hostname_pc << " on port " << port_num_pc << "..." << endl;

    // // connect socket (PC)
    // status_pc = connect(socket_fd_pc, host_info_list_pc->ai_addr, host_info_list_pc->ai_addrlen);
    // if (status_pc == -1) {
    //     cerr << "Error: cannot connect to socket" << endl;
    //     cerr << "  (" << hostname_pc << "," << port_num_pc << ")" << endl;
    //     return -1;
    // } 

    //accept connection as player server (PS)
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    int socket_fd_client_ps;
    socket_fd_client_ps = accept(socket_fd_ps, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (socket_fd_client_ps == -1) {
        cerr << "Error: cannot accept connection on socket" << endl;
        return -1;
    } 

    //cout << "players connection success!"  << endl;

    //start playing the game of potato
    //initialize sockets descriptors set 

    // fd_set readfds;
    // FD_ZERO(&readfds);
    // FD_SET(socket_fd_mc, &readfds); //ringmaster 
    // FD_SET(socket_fd_pc, &readfds); //player as client (right neightbor)
    // FD_SET(socket_fd_client_ps, &readfds); //player as server (left neightbor)
    // Potato received_potato;

    // recv(socket_fd_mc, &received_potato, sizeof(received_potato), 0);
    // cout << "Player: " <<id<< "hop value: " << received_potato.hops<<endl;
    // process the game for potato passing 
    
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
            recv(socket_fd_mc, &received_potato, sizeof(received_potato), 0);
            cout << "potato from ringmaster" <<endl;
        } else if (FD_ISSET(socket_fd_pc, &readfds)) {
            cout << "potato from right neighbor" <<endl;
            recv(socket_fd_pc, &received_potato, sizeof(received_potato), 0);
        } else if (FD_ISSET(socket_fd_client_ps, &readfds)) {
            cout << "potato from left neighbor" <<endl;
            recv(socket_fd_client_ps, &received_potato, sizeof(received_potato), 0);
        }
        //cout << "Player: " << id << " I've received the potato with hops: " << received_potato.hops << endl;
        received_potato.hops--;
        received_potato.trace[received_potato.index] = id;
        received_potato.index++;
        cout << "hops left: " << received_potato.hops << endl;
        if (received_potato.hops > 0) { // pass to one of the neighbors 
            // Pass the potato to a neighbor
            srand((unsigned int)time(NULL) + 2);
            int random_neighbor = rand() % 2; // randomly select a neighbor
            cout <<endl;
            if (random_neighbor == 0) {
                // Pass the potato to the left neighbor
                send(socket_fd_client_ps, &received_potato, sizeof(received_potato), 0);
                int lef_id = (id + num_players - 1) % num_players;
                cout << "Player: " <<id<< "Sending potato to "<< lef_id << endl;
            } else {
                // Pass the potato to the right neighbor
                send(socket_fd_pc, &received_potato, sizeof(received_potato), 0);
                int right_id = (id + 1) % num_players;
                cout << "Player: " <<id<< " Sending potato to "<< right_id << endl;
            }
        } else if (received_potato.hops == 0){ //pass potato back to ringmaster
            send(socket_fd_mc, &received_potato, sizeof(received_potato), 0);
            cout << "I'm it" << endl;
            break;
        }
    }
    
    //close sockets
    close(socket_fd_mc);
    close(socket_fd_pc);
    close(socket_fd_client_ps);
    
    return 1;
}
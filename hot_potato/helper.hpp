#ifndef __HELPER_HPP__
#define __HELPER_HPP__


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

using namespace std;

int createServer(const char * port, int num_players) {
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

    //create socket
    cout << "server socket created" << endl;
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

    //freeaddrinfo(host_info_list);
    return socket_fd;
}

int createClient(const char * hostname, const char * port) {
    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;

    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family   = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
        cerr << "Error: cannot get address info for host" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    } 
    //create client socket 
    cout << "client socket created" << endl;
    socket_fd = socket(host_info_list->ai_family, 
		host_info_list->ai_socktype, 
		host_info_list->ai_protocol);
    if (socket_fd == -1) {
        cerr << "Error: cannot create socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    } 
    
    //connect to the host server
    cout << "client socket connected" << endl;
    status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        cerr << "Error: cannot connect to socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    }

    //freeaddrinfo(host_info_list);
    return socket_fd;
}

#endif
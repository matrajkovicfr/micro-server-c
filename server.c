#include <stdio.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_CONNECTIONS 1000
#define BUFFER_SIZE 65536
#define QUEUE_SIZE 100000


void startServer(const char *port);
void respond(int slot);


static int listenFileDescriptor;
int *clients;
static char *buffer;


void serve(const char *port) {
    // Shared memory for client slots
    // All are set to -1 to signify that a client is not connected
    clients = mmap(NULL, sizeof(*clients) * MAX_CONNECTIONS, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    for (int i=0; i<MAX_CONNECTIONS; ++i) {
        clients[i] = -1;
    }
    
    startServer(port);
    printf("Server started at address: %s:%s\n", "http://127.0.0.1", port);
    
    // Avoid zombie threads
    signal(SIGCHLD, SIG_IGN);
    
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength;
    int slot = 0;
    
    while (1) {
        clientAddressLength = sizeof(clientAddress);
        clients[slot] = accept(listenFileDescriptor, (struct sockaddr *) &clientAddress, &clientAddressLength);
        
        if (clients[slot] < 0) {
            printf("Error accepting client connection.");
            exit(-1);
        }
        else {
            if (fork() == 0) {
                // Close the listener (I am now the client)
                close(listenFileDescriptor);
                respond(slot);
                close(clients[slot]);
                clients[slot] = -1;
                exit(0);
            }
            else {
                // Close the accepted handle (I am still the server)
            }
        }
        
        while (clients[slot] != -1) {
            slot = (slot + 1) % MAX_CONNECTIONS;
        }
        
    }
        
    
}

void startServer(const char *port) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    // Params in which address info result (and its copy) will be saved
    struct addrinfo *res, *resTmp;
    if (getaddrinfo(NULL, port, &hints, &res) != 0) {
        printf("Error getting address information.");
        exit(-1);
    }
    
    for (resTmp = res; resTmp != NULL; resTmp = resTmp->ai_next) {
        int option = 1;
        listenFileDescriptor = socket(resTmp->ai_family, resTmp->ai_socktype, 0);
        setsockopt(listenFileDescriptor, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
        if (listenFileDescriptor == -1) {
            continue;
        }
        if (bind(listenFileDescriptor, resTmp->ai_addr, resTmp->ai_addrlen) == 0) {
            break;
        }
    }
    
    if (resTmp == NULL) {
        printf("Socket creation or binding failed.");
        exit(-1);
    }
    
    freeaddrinfo(res);
    
    // Listen for incoming connections
    if (listen(listenFileDescriptor, QUEUE_SIZE) != 0) {
        printf("Error listening for incoming connections.");
        exit(-1);
    }
    
}

void respond(int slot) {
    buffer = malloc(BUFFER_SIZE);
    
    // Receive message from socket
    long messageReceived;
    messageReceived = recv(clients[slot], buffer, BUFFER_SIZE, 0);
    if (messageReceived < 0) {
        printf("Message not received from client.");
    }
    else if (messageReceived == 0) {
        printf("Client disconnected unexpectedly.");
    }
    else {
        // Message received
        /*
         Printing the message (printf("%s", buffer);) returns something like this output:
         GET / HTTP/1.1
         Host: 127.0.0.1:8000
         Upgrade-Insecure-Requests: 1
         Accept: text/html,application/xhtml+xml,application/xml;q=0.9;q=0.8
         User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/14.1.2 Safari/605.1.15
         Accept-Language: en-us
         Accept-Encoding: gzip, deflate
         Connection: keep-alive
         */
        printf("%s", buffer);
    }
    
    free(buffer);
}

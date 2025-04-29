#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "db.h"

#define CHECK_ERROR(ret_val, func_name) do { \
    if (ret_val < 0) { \
        perror(func_name " failed"); \
        exit(EXIT_FAILURE); \
    } \
} while(0)

#define PORT 4500

int main() {
    CREATE_DB_STRUCT(my_db); 

    my_db.init(&my_db);
    my_db.connect(&my_db);   

    int server_fd, conn_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    // create
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // bind 
    CHECK_ERROR(bind(server_fd, (struct sockaddr *)&address, sizeof(address)), "bind");

    // listen
    CHECK_ERROR(listen(server_fd, 3), "listen");

    printf("Server listening on port %d...\n", PORT);

    // waiting client
    conn_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    CHECK_ERROR(conn_socket, "accept");
    printf("Client connected!\n");

    while (1) {
        memset(buffer, 0, sizeof(buffer));

        int bytes_read = read(conn_socket, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            printf("Received: %s\n", buffer);
            my_db.query(&my_db, buffer);   
            // reply
            //char *response = "Message received";
            //send(conn_socket, response, strlen(response), 0);
        } else if (bytes_read == 0) {
            printf("Client closed the connection.\n");
            break; // client close connection
        } else {
            perror("read error");
            break;
        }
    }

    close(conn_socket);
    close(server_fd);
    return 0;
}


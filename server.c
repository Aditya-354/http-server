#include <asm-generic/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>

#define BACKLOG 0
#define PORT "8080"
#define FILE_PATH "index.html"

int main(int argc, char **argv) {

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd == -1) {
        perror("socket");
        return 1;
    }

    int yes = 1;
    int sso_result = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
    if (sso_result == -1) {
        perror("setsockopt");
        return -1;
    }
    

    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int gai_result = getaddrinfo(NULL, PORT, &hints, &result);
    if (gai_result != 0) {
        perror("getaddrinfo");
        return 2;
    }

    int bind_result = bind(socket_fd, result->ai_addr, result->ai_addrlen);
    if (bind_result == -1) {
        perror("bind");
        return 3;
    }

    // Listen for incoming connections
    int listen_result = listen(socket_fd, BACKLOG);
    if (listen_result == -1) {
        perror("listen");
        return 4;
    }

    // accept
    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof their_addr;

    const char buf[] = "Hello http\n";

    int max_length = 1024;
    char http_request[max_length];
    memset(http_request, 0, max_length);

    while (1) {
        int new_socket_fd = accept(socket_fd, (struct sockaddr*) &their_addr, &addr_size);
        ssize_t bytes_received = recv(new_socket_fd, http_request, max_length, 0);
        if (bytes_received == -1) {
            perror("recv error");
            return 5;
        }

        if (strncmp(http_request, "GET", 3) == 0) {
            printf("bytes received = %lu\n", bytes_received);

            // respond to the HTTP request
            const char *response_header = "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n\r\n";

            FILE* file = fopen(FILE_PATH, "r");
            char c;
            char* response_body;
            size_t i = 0;
            int sent_bytes = send(new_socket_fd, response_header, strlen(response_header), 0);
            while ( (c = getc(file)) != EOF ) {
                response_body[i++] = c;
            }

            sent_bytes = send(new_socket_fd, response_body, strlen(response_body), 0);

            printf("bytes sent = %d bytes\n", sent_bytes);
            fclose(file);

        } else {
            printf("Received non-GET request!\n");
            close(socket_fd);
            close(new_socket_fd);
            return 7;
        }
        close(new_socket_fd);
    }


    return 0;
}

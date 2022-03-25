//
// Created by hrubyj on 25.03.2022.
//
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>

static bool send_request(int socket_fd) {
    const char *request = "M-SEARCH * HTTP/1.1\r\n"
                          "HOST: 239.255.255.250:1900\r\n"
                          "MAN: \"ssdp:discover\"\r\n"
                          "MX: 1\r\n"
                          "ST: ssdp:all\r\n"
                          "\r\n";

    const size_t request_length = strlen(request);
    const struct sockaddr_in dest = {
            .sin_family = AF_INET,
            .sin_addr.s_addr = htonl(0xEFFFFFFA),  // 239.255.255.250
            .sin_port = htons(1900)
    };

    ssize_t length = sendto(socket_fd, request, request_length, 0, (struct sockaddr*) &dest, sizeof dest);
    return length < 0 ? false : true;
}

static bool receive_response(int socket_fd) {
    char buffer[2048];
    const size_t max_length = (sizeof buffer)-1;
    ssize_t length = recvfrom(socket_fd, buffer, max_length, 0, NULL, NULL);
    if (length < 0) {
        return false;
    }

    buffer[length] = '\0';
    puts(buffer);
    fflush(stdout);
    return true;
}

int main() {
    int socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    int exit_status = 0;
    if (socket_fd < 0) {
        exit_status = 1;
    }

    if (!send_request(socket_fd)) {
        perror("Error while sending request");
        exit_status = 1;
    }

    struct pollfd pollfd = {
            .fd = socket_fd,
            .revents = 0,
            .events = POLLIN
    };

    int status = poll(&pollfd, 1, 2000);
    while (status > 0) {
        if (!receive_response(socket_fd)) {
            perror("Error while receiving response");
            exit_status = 1;
        }
        status = poll(&pollfd, 1, 1000);
    }

    if (status < 0) {
        exit_status = 1;
    }

    close(socket_fd);
    return exit_status;
}


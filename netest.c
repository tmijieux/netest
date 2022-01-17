#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>

int main(int argc, char* argv[])
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        fprintf(stderr, "Error while creating socket = %s\n", strerror(errno));
        exit(1);
    }
    fprintf(stderr, "socket opened on fd = %d\n", sock);

    struct sockaddr_in source_sockaddr = {
        .sin_family = AF_INET,
        .sin_port = 0,
        .sin_zero = { 0 }
    };
    struct sockaddr_in dest_sockaddr = {
        .sin_family = AF_INET,
        .sin_port = htons(80),
        .sin_zero = { 0 }
    };
    const char *addr = "0.0.0.0";
    int success = inet_aton(addr, &source_sockaddr.sin_addr);
    if (!success) {
        fprintf(stderr, "Invalid address %s\n", addr);
        exit(1);
    }
    int err = bind(sock, (struct sockaddr*)&source_sockaddr, sizeof(source_sockaddr));
    if (err == -1) {
        fprintf(stderr, "Error while binding socket = %s\n", strerror(errno));
        exit(1);
    }
    fprintf(stderr, "bind, err = %d\n", err);


    struct addrinfo *res = NULL;
    struct addrinfo hints = { 0 };
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */

    err = getaddrinfo("google.fr", "80", &hints, &res);
    if (err) {
        fprintf(stderr, "Error while resolving host = %s\n", gai_strerror(err));
        exit(1);
    }
    int address_found = 0;
    while (res != NULL) {
        fprintf(stderr, " family = %d\n", res->ai_family);
        fprintf(stderr, " sock_type = %d\n", res->ai_socktype);

        if (res->ai_family == AF_INET || res->ai_family == AF_INET6) {

            char buf[INET_ADDRSTRLEN+10] = {0};
            struct sockaddr_in *ptr= (struct sockaddr_in*) res->ai_addr;
            const char *errs = inet_ntop(res->ai_family, &ptr->sin_addr, buf, res->ai_addrlen);
            if (!errs ) {
                fprintf(stderr, "Error while converting addr = %s\n", strerror(errno));
                exit(1);
            }

            fprintf(stderr, "resolved address = %s\n", buf);
            if (res->ai_family == AF_INET) {
                address_found = 1;
                memcpy(&dest_sockaddr.sin_addr, &ptr->sin_addr, res->ai_addrlen);
            }
        }
        fprintf(stderr, "\n");
        res = res->ai_next;
    }
    freeaddrinfo(res);
    if (!address_found)
    {
        fprintf(stderr, "Could not resolve address\n");
        exit(1);
    }

    err = connect(sock, (const struct sockaddr*)&dest_sockaddr, sizeof(dest_sockaddr));
    if (err) {
        fprintf(stderr, "Error while connecting = %s\n", strerror(errno));
        exit(1);
    }
    char sendbuf[] = "GET / HTTP/1.1\r\nHost: google.fr\r\n\r\n";
    fprintf(stderr, "writing data\n");
    err = write(sock, sendbuf, sizeof(sendbuf)-1);
    if (err == -1) {
        fprintf(stderr, "Error while writing data = %s\n", strerror(errno));
        exit(1);
    }
    fprintf(stderr, "written = %d\n", err);
    char recvbuf[1024];
    memset(recvbuf, 0, sizeof recvbuf);
    fprintf(stderr, "receiving data\n");

    do {
        err = read(sock, recvbuf, sizeof recvbuf-1);
        if (err > 0) {
            recvbuf[err] = '\0';
            fprintf(stderr, "recv = %d\n", err);
            printf("%s", recvbuf);
        }
    } while (err >= sizeof recvbuf - 1);
    if (err < 0) {
        fprintf(stderr, "Error while reading data = %s\n", strerror(errno));
        exit(1);
    }
    close(sock);
    return 0;
}


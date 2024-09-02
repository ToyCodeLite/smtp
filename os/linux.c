#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../smtp.h"
#include "../lib/base64.h" 

int smtp_send(struct smtp* sm)
{
    struct hostent* host;
    struct sockaddr_in server;
    int sock_fd;

    // host = gethostbyname(domain);
    host = gethostbyname(sm->domain);
    if (!host) {
        printf("domain can not find！\n");
        return SMTP_ERROR_DOMAIN;
    }

    if (host->h_addrtype != AF_INET) {
        printf("address type is not support %d\n", host->h_addrtype);
        return SMTP_ERROR_DOMAIN;
    }

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        printf("can not create socket!\n");
        return SMTP_ERROR_SOCKET;
    }

    server.sin_family = AF_INET;
    // server.sin_port = htons(port);
    server.sin_port = htons(sm->port);
    server.sin_addr = *(struct in_addr*)host->h_addr_list[0];
    memset(&(server.sin_zero), 0, 8);

    if (connect(sock_fd, (struct sockaddr*)&server, sizeof(struct sockaddr)) == -1) {
        printf("can not connect socket！\n");
        close(sock_fd);
        return SMTP_ERROR_CONNECT;
    }

    printf("connect success ,ip address %s\n", inet_ntoa(server.sin_addr));
    unsigned char *encodedContent = base64_encode(sm->content);

    sm->status = SMTP_STATUS_EHLO;
    sm->socket = sock_fd;
    sm->content = encodedContent;

    if (smtp_read(sm) || strcmp(sm->cmd, "220")) {
        close(sock_fd);
        free(encodedContent);
        return SMTP_ERROR_READ;
    }

    while (sm->status != SMTP_STATUS_NULL) {
        int error = smtp_fun[sm->status](sm);
        if (error) {
            printf("error = %d\n", error);
            close(sock_fd);
            free(encodedContent);
            return error;
        }
    }

    close(sock_fd);
    free(encodedContent);
    return 0;
}

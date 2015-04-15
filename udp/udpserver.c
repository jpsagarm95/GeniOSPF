/* udpserver.c */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

/*
Port No has to be given as command line argument 
 */

int main(int argc, char *argv[]) {
    int sock;
    int addr_len, bytes_read;
    char recv_data[1024];
    struct sockaddr_in server_addr, client_addr;
    if (argc < 2) {
        printf("PortNo Missing");
        exit(0);
    }

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr.sin_zero), 8);

    if (bind(sock, (struct sockaddr *) &server_addr,
            sizeof (struct sockaddr)) == -1) {
        perror("Bind");
        exit(1);
    }

    addr_len = sizeof (struct sockaddr);

    printf("\nUDPServer Waiting for client on port %s\n", argv[1]);
    fflush(stdout);

    while (1) {
        bytes_read = recvfrom(sock, recv_data, 1024, 0,
                (struct sockaddr *) &client_addr, &addr_len);


        recv_data[bytes_read] = '\0';

        printf("\n(%s , %d) said : ", inet_ntoa(client_addr.sin_addr),
                ntohs(client_addr.sin_port));
        printf("%s", recv_data);
        fflush(stdout);
    }
    
    return 0;
}
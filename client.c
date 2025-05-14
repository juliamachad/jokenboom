#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "common.h"

#define BUFSZ 1024


void usage(int argc, char **argv) {
	printf("Uso: %s <IP> <porta>\n", argv[0]);
	printf("exemplo: %s 127.0.0.1 51511\n", argv[0]);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        usage(argc, argv);
    }

   struct sockaddr_storage storage;
	if (0 != addrparse(argv[1], argv[2], &storage)) {
		usage(argc, argv);
	}

	int s;
	s = socket(storage.ss_family, SOCK_STREAM, 0);
	if (s == -1) {
		logexit("socket");
	}
	struct sockaddr *addr = (struct sockaddr *)(&storage);
	if (0 != connect(s, addr, sizeof(storage))) {
		logexit("connect");
	}

	char addrstr[BUFSZ];
	addrtostr(addr, addrstr, BUFSZ);

	printf("connected to %s\n", addrstr);

	GameMessage msg;
    size_t count = recv(s, &msg, sizeof(msg), 0);
     if (count <= 0) {
        logexit("recv");
    }
    if (msg.type != MSG_REQUEST) {
        fprintf(stderr, "esperava MSG_REQUEST, recebeu tipo=%d\n", msg.type);
        exit(EXIT_FAILURE);
    }
    
    printf("%s\n", msg.message);
	memset(&msg, 0, sizeof(msg));
    msg.type = MSG_RESPONSE;
	printf("Escolha sua jogada:\n0 - Nuclear Attack\n1 - Intercept Attack\n...");
    scanf("%d", &msg.client_action);
	
    send(s, &msg, sizeof(msg), 0);
	

	memset(&msg, 0, sizeof(msg));
	count = recv(s, &msg, sizeof(msg), 0);
    if (count <= 0) {
        logexit("recv");
    }

	switch (msg.type) {
        case MSG_RESULT:
            // o servidor já preencheu msg.message com resultado/parcial
            printf("Resultado: %s\n", msg.message);
            break;
        case MSG_ERROR:
            fprintf(stderr, "Erro do servidor: %s\n", msg.message);
            break;
        default:
            fprintf(stderr, "Tipo inesperado: %d\n", msg.type);
    }
	close(s);

	

	exit(EXIT_SUCCESS);
}
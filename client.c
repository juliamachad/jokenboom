#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "common.h"

#define BUFSZ 1024 //CP

void usage(char **argv) { //CP
	printf("Uso: %s <IP> <porta>\n", argv[0]); //CP
	printf("exemplo: %s 127.0.0.1 51511\n", argv[0]); //CP
	exit(EXIT_FAILURE); //CP
}

int main(int argc, char *argv[]) { //CP
    if (argc != 3) { //CP
        usage(argv); //CP
    }

    struct sockaddr_storage storage; //CP
	if (0 != addrparse(argv[1], argv[2], &storage)) { //CP
		usage(argv); //CP
	}

	int s; //CP
	s = socket(storage.ss_family, SOCK_STREAM, 0);//CP
	if (s == -1) {//CP
		logexit("socket");//CP
	}

	struct sockaddr *addr = (struct sockaddr *)(&storage);//CP
	if (0 != connect(s, addr, sizeof(storage))) {//CP
		logexit("connect");//CP
	}

	char addrstr[BUFSZ];//CP
	addrtostr(addr, addrstr, BUFSZ);//CP
	printf("connected to %s\n", addrstr);//CP

    // Receber solicitação de ação do servidor
	GameMessage msg;
    size_t count = recv(s, &msg, sizeof(msg), 0);
     if (count <= 0) { // Servidor desconectado ou erro
        logexit("recv");
    }
    if (msg.type != MSG_REQUEST) { // Tipo de mensagem inesperado
        fprintf(stderr, "esperava MSG_REQUEST, recebeu tipo=%d\n", msg.type);
        exit(EXIT_FAILURE);
    }
    
    // Exibe a solicitação do servidor e obtém a escolha do usuário
    printf("%s", msg.message);//CP
	memset(&msg, 0, sizeof(msg));//CP
    msg.type = MSG_RESPONSE;
    scanf("%d", &msg.client_action); // Recebe a ação do usuário //CP
	
    // Manda resposta ao servidor
    if (send(s, &msg, sizeof(msg), 0) != sizeof(msg)) {
        logexit("send");//CP
    }
	
    // Recebe o resultado do servidor
	memset(&msg, 0, sizeof(msg));//CP
	count = recv(s, &msg, sizeof(msg), 0);//CP
    if (count <= 0) {
        logexit("recv");
    }

    // Lidar com a mensagem recebida
	switch (msg.type) {
        case MSG_RESULT:
            printf("Resultado: %s", msg.message);
            break;
        case MSG_ERROR:
            fprintf(stderr, "Erro do servidor: %s", msg.message);
            break;
        default:
            fprintf(stderr, "Tipo inesperado: %d", msg.type);
    }

    // Fecha conexão
	close(s);//CP
	exit(EXIT_SUCCESS);//CP
}
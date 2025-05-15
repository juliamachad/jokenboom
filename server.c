#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSZ 1024

void usage(char **argv) {//CP
    printf("Uso: %s <v4|v6> <porta>\n", argv[0]);//CP
    printf("exemplo: %s v4 51511\n", argv[0]);//CP
    exit(EXIT_FAILURE);//CP
}


int main(int argc, char *argv[]) {//CP
    if (argc != 3) {//CP
        usage(argv);//CP
    }
struct sockaddr_storage storage;//CP
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) {//CP
        usage(argv);//CP
    }

    int s;//CP
    s = socket(storage.ss_family, SOCK_STREAM, 0);//CP
    if (s == -1) {//CP
        logexit("socket");//CP
    }

    int enable = 1;//CP
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {//CP
        logexit("setsockopt");//CP
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);//CP
    if (0 != bind(s, addr, sizeof(storage))) {//CP
        logexit("bind");//CP
    }

    if (0 != listen(s, 10)) {//CP
        logexit("listen");//CP
    }

    char addrstr[BUFSZ];//CP
    addrtostr(addr, addrstr, BUFSZ);//CP
    printf("bound to %s, waiting connections\n", addrstr);//CP

    while (1) {//CP
        struct sockaddr_storage cstorage;//CP
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);//CP
        socklen_t caddrlen = sizeof(cstorage);//CP

        int csock = accept(s, caddr, &caddrlen);//CP
        if (csock == -1) {//CP
            logexit("accept");//CP
        }

        char caddrstr[BUFSZ];//CP
        addrtostr(caddr, caddrstr, BUFSZ);//CP
        printf("[log] connection from %s\n", caddrstr);//CP

        // Preparar mensagem para solicitar ação do cliente
        GameMessage msg;
        memset(&msg, 0, sizeof(msg));//CP
        msg.type = MSG_REQUEST;
        snprintf(msg.message, MSG_SIZE,
            "Escolha sua jogada:\n"
            "0 - Nuclear Attack\n"
            "1 - Intercept Attack\n"
            "2 - Cyber Attack\n"
            "3 - Drone Strike\n"
            "4 - Bio Attack\n"
        );
    printf("[debug] enviando MSG_REQUEST para %s\n", caddrstr);
    if (send(csock, &msg, sizeof(msg), 0) != sizeof(msg)) {
        logexit("send");
    }

        // Aguarda a resposta do cliente (MSG_RESPONSE)
        memset(&msg, 0, sizeof(msg));
        ssize_t count = recv(csock, &msg, sizeof(msg), 0);
        if (count <= 0) {
            close(csock);
            continue;
        }
        if (msg.type != MSG_RESPONSE) {
            fprintf(stderr, "esperava MSG_RESPONSE, recebeu tipo=%d\n", msg.type);
            close(csock);
            continue;
        }
        printf("[debug] recv MSG_RESPONSE, client_action=%d\n", msg.client_action);

        // Validar msg.client_action (0–4)  e devolver MSG_RESULT / MSG_ERROR conforme a lógica do jogo.
        //    Por enquanto só ecoando de volta
        GameMessage reply;
        memset(&reply, 0, sizeof(reply));
        reply.type = MSG_RESULT;
        // copiar de volta o que veio
        reply.client_action = msg.client_action;
        snprintf(reply.message, MSG_SIZE,
            "Você escolheu %d — Ainda sem lógica implementada.\n",
            msg.client_action
        );
         if (send(csock, &reply, sizeof(reply), 0) != sizeof(reply)) {
        logexit("send");
    }

        close(csock);
    }

    exit(EXIT_SUCCESS);
    
}
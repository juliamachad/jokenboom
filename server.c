#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define BUFSZ 1024

void usage(int argc, char **argv) {
    printf("Uso: %s <v4|v6> <porta>\n", argv[0]);
    printf("exemplo: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}


int main(int argc, char *argv[]) {
    if (argc != 3) {
         usage(argc, argv);
    }
struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) {
        usage(argc, argv);
    }

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) {
        logexit("socket");
    }

    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(s, addr, sizeof(storage))) {
        logexit("bind");
    }

    if (0 != listen(s, 10)) {
        logexit("listen");
    }

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("bound to %s, waiting connections\n", addrstr);

    while (1) {
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1) {
            logexit("accept");
        }

        char caddrstr[BUFSZ];
        addrtostr(caddr, caddrstr, BUFSZ);
        printf("[log] connection from %s\n", caddrstr);

        GameMessage msg;
        ssize_t count;
       memset(&msg, 0, sizeof(msg));
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
        count = recv(csock, &msg, sizeof(msg), 0);
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
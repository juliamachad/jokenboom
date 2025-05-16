#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSZ 1024

void usage(char **argv)
{                                                 // CP
    printf("Uso: %s <v4|v6> <porta>\n", argv[0]); // CP
    printf("exemplo: %s v4 51511\n", argv[0]);    // CP
    exit(EXIT_FAILURE);                           // CP
}

GameMessage verify_winner(GameMessage msg)
{
    static const int result[5][5] = {
        // 1 = cliente vence, 0 = servidor vence, -1 = empate
        {-1, 0, 1, 1, 0}, // Nuclear
        {1, -1, 0, 0, 1}, // Intercept
        {0, 1, -1, 1, 0}, // Cyber
        {0, 1, 0, -1, 1}, // Drone
        {1, 0, 1, 0, -1}  // Bio
    };
    msg.result = result[msg.client_action][msg.server_action];
    return msg;
}

int main(int argc, char *argv[])
{ // CP
    if (argc != 3)
    {                // CP
        usage(argv); // CP
    }
    struct sockaddr_storage storage; // CP
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage))
    {                // CP
        usage(argv); // CP
    }

    int s;                                         // CP
    s = socket(storage.ss_family, SOCK_STREAM, 0); // CP
    if (s == -1)
    {                      // CP
        logexit("socket"); // CP
    }

    int enable = 1; // CP
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)))
    {                          // CP
        logexit("setsockopt"); // CP
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage); // CP
    if (0 != bind(s, addr, sizeof(storage)))
    {                    // CP
        logexit("bind"); // CP
    }

    if (0 != listen(s, 10))
    {                      // CP
        logexit("listen"); // CP
    }

    char addrstr[BUFSZ];                                   // CP
    addrtostr(addr, addrstr, BUFSZ);                       // CP
    printf("bound to %s, waiting connections\n", addrstr); // CP

    while (1)
    {                                                            // CP
        struct sockaddr_storage cstorage;                        // CP
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage); // CP
        socklen_t caddrlen = sizeof(cstorage);                   // CP

        int csock = accept(s, caddr, &caddrlen); // CP
        if (csock == -1)
        {                      // CP
            logexit("accept"); // CP
        }

        char caddrstr[BUFSZ];                           // CP
        addrtostr(caddr, caddrstr, BUFSZ);              // CP
        printf("[log] connection from %s\n", caddrstr); // CP

        GameMessage msg;
        msg.client_wins = 0;
        msg.server_wins = 0;
        msg.result = -1;
        bool play_again = true;

        // Nomes das ações para mensagens
        const char *action_names[5] = {
            "Nuclear Attack", "Intercept Attack", "Cyber Attack", "Drone Strike", "Bio Attack"};

        while (play_again)
        {
            while (msg.result == -1) // partida até nao ser empate
            {
                // Preparar mensagem para solicitar ação do cliente

                memset(&msg, 0, sizeof(msg)); // CP
                msg.type = MSG_REQUEST;
                snprintf(msg.message, MSG_SIZE,
                         "Escolha sua jogada:\n"
                         "0 - Nuclear Attack\n"
                         "1 - Intercept Attack\n"
                         "2 - Cyber Attack\n"
                         "3 - Drone Strike\n"
                         "4 - Bio Attack\n");
                printf("[debug] enviando MSG_REQUEST para %s\n", caddrstr);
                if (send(csock, &msg, sizeof(msg), 0) != sizeof(msg))
                {
                    logexit("send");
                }

                ssize_t count = recv(csock, &msg, sizeof(msg), 0);
                if (count <= 0)
                {
                    close(csock);
                    continue;
                }
                if (msg.type != MSG_RESPONSE)
                {
                    fprintf(stderr, "esperava MSG_RESPONSE, recebeu tipo=%d\n", msg.type);
                    close(csock);
                    continue;
                }
                if (msg.client_action < 0 || msg.client_action > 4)
                {
                    memset(&msg, 0, sizeof(msg));
                    msg.type = MSG_ERROR;
                    snprintf(msg.message, MSG_SIZE, "Selecione um valor de 0 a 4.");
                    send(csock, &msg, sizeof(msg), 0);
                }

                msg.server_action = rand() % 5;
                msg = verify_winner(msg);
                const char *output_result;
                if (msg.result == 1)
                {
                    msg.client_wins++;
                    output_result = "Você venceu!";
                }
                else if (msg.result == 0)
                {
                    msg.server_wins++;
                    output_result = "Você perdeu :(";
                }
                else if (msg.result == -1)
                {
                    output_result = "Deu empate! Vamos reiniciar a rodada.";
                }

                msg.type = MSG_RESULT;
                snprintf(msg.message, MSG_SIZE,
                         "Você escolheu: %s\nServidor escolheu: %s\nResultado: %s\n",
                         action_names[msg.client_action], action_names[msg.server_action], output_result);
                send(csock, &msg, sizeof(msg), 0);
                // Aguarda a resposta do cliente (MSG_RESPONSE)
                // memset(&msg, 0, sizeof(msg));
            }

            GameMessage playagain_msg;
            memset(&playagain_msg, 0, sizeof(playagain_msg));
            playagain_msg.type = MSG_PLAY_AGAIN_REQUEST;
            snprintf(playagain_msg.message, MSG_SIZE, "Deseja jogar novamente?\n1 - Sim\n0 - Não\n");
            send(csock, &playagain_msg, sizeof(playagain_msg), 0);

            ssize_t count = recv(csock, &playagain_msg, sizeof(playagain_msg), 0);
            if (count <= 0)
            {
                close(csock);
            }
            if (playagain_msg.type != MSG_PLAY_AGAIN_RESPONSE)
                continue;
            if (playagain_msg.client_action == 0)
            {
                play_again = false;
                break;
            }
            else if (playagain_msg.client_action == 1)
            {
                play_again = true;
                break;
            }
            else
            {
                memset(&playagain_msg, 0, sizeof(playagain_msg));
                playagain_msg.type = MSG_ERROR;
                snprintf(playagain_msg.message, MSG_SIZE, "Digite 1 para sim ou 0 para não.");
                send(csock, &playagain_msg, sizeof(playagain_msg), 0);
            }
        }

        printf("[debug] recv MSG_RESPONSE, client_action=%d\n", msg.client_action);

        // Validar msg.client_action (0–4)  e devolver MSG_RESULT / MSG_ERROR conforme a lógica do jogo.
        GameMessage end_msg;
        memset(&end_msg, 0, sizeof(end_msg));
        end_msg.type = MSG_END;
        end_msg.client_wins = msg.client_wins;
        end_msg.server_wins = msg.server_wins;
        snprintf(end_msg.message, MSG_SIZE,
                 "Fim de jogo!\nPlacar final: Você %d x %d Servidor\nObrigado por jogar!",
                 msg.client_wins, msg.server_wins);
        send(csock, &end_msg, sizeof(end_msg), 0);

        close(csock);
    }

    exit(EXIT_SUCCESS);
}
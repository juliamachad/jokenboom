#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h> 

#define BUFSZ 1024

// Exibe instruções de uso e encerra o programa
void usage(char **argv)
{                                                 // CP
    printf("Uso: %s <v4|v6> <porta>\n", argv[0]); // CP
    printf("exemplo: %s v4 51511\n", argv[0]);    // CP
    exit(EXIT_FAILURE);                           // CP
}

// Determina o vencedor com base nas ações do cliente e servidor
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

    // Inicializa gerador de números aleatórios
    srand(time(NULL));

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
    printf("Servidor iniciado em modo %s na porta %s. Aguardando conexão..\n", addrstr); // CP

     // Nomes das ações para mensagens
        const char *action_names[5] = {
            "Nuclear Attack", "Intercept Attack", "Cyber Attack", "Drone Strike", "Bio Attack"};


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
        printf("Cliente conectado.\n", caddrstr); // CP

        int client_wins = 0;
        int server_wins = 0;
        bool play_again = true;


       
        while (play_again)
        {
            int result = -1;
            while (result == -1) // partida até nao ser empate
            {
                // Preparar mensagem para solicitar ação do cliente
                GameMessage msg;
                memset(&msg, 0, sizeof(msg)); // CP
                printf("Apresentando as opções para o cliente.\n");
                snprintf(msg.message, MSG_SIZE,
                         "Escolha sua jogada:\n"
                         "0 - Nuclear Attack\n"
                         "1 - Intercept Attack\n"
                         "2 - Cyber Attack\n"
                         "3 - Drone Strike\n"
                         "4 - Bio Attack\n");
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
                    
                    continue;
                }
                if (msg.client_action < 0 || msg.client_action > 4)
                {
                    memset(&msg, 0, sizeof(msg));
                    msg.type = MSG_ERROR;
                    printf("Erro: opção inválida de jogada.\n");
                    snprintf(msg.message, MSG_SIZE, "Por favor, selecione um valor de 0 a 4.\n");
                    send(csock, &msg, sizeof(msg), 0);
                    continue;
                }

                printf("Cliente escolheu %d.\n", msg.client_action);
                // Gera ação do servidor e determina resultado
                msg.server_action = rand() % 5;
                printf("Servidor escolheu aleatoriamente %d.\n", msg.server_action);
                msg = verify_winner(msg);
                result=msg.result;
                const char *output_result;
                if (result == 1)
                {
                    client_wins++;
                    output_result = "Vitória!";
                }
                else if (result == 0)
                {
                    server_wins++;
                    output_result = "Derrota!";
                }
                else if (result == -1)
                {
                    output_result = "Empate!";
                    printf("Jogo empatado.\nSolicitando ao cliente mais uma escolha.");
                }

                GameMessage result_msg;
                memset(&result_msg, 0, sizeof(result_msg));
                result_msg.type = MSG_RESULT;
                result_msg.client_action = msg.client_action;
                result_msg.server_action = msg.server_action;
                result_msg.result = result;
                snprintf(result_msg.message, MSG_SIZE,
                         "Você escolheu: %s\nServidor escolheu: %s\nResultado: %s\n",
                         action_names[msg.client_action], action_names[msg.server_action], output_result);
                if (send(csock, &result_msg, sizeof(result_msg), 0) != sizeof(result_msg)) {
                    close(csock);
                }
                if (result != -1) {
                printf("Placar atualizado: Cliente %d x %d Servidor\n", client_wins, server_wins);
            }
            }

             while (1) {
            GameMessage playagain_msg;
            memset(&playagain_msg, 0, sizeof(playagain_msg));
            playagain_msg.type = MSG_PLAY_AGAIN_REQUEST;
            snprintf(playagain_msg.message, MSG_SIZE, "Deseja jogar novamente?\n1 - Sim\n0 - Não\n");
            printf("Perguntando se o cliente deseja jogar novamente.\n");
            if (send(csock, &playagain_msg, sizeof(playagain_msg), 0) != sizeof(playagain_msg)) {
                     close(csock);
                }
            ssize_t count = recv(csock, &playagain_msg, sizeof(playagain_msg), 0);
            if (count <= 0)
            {
                close(csock);
            }
            if (playagain_msg.type != MSG_PLAY_AGAIN_RESPONSE){

                fprintf(stderr, "esperava MSG_PLAY_AGAIN_RESPONSE, recebeu tipo=%d\n", playagain_msg.type);
                continue;
            }
                
            if (playagain_msg.client_action == 0)
            {
                printf("Cliente não deseja jogar novamente.\n");
                play_again = false;
                break;
            }
            else if (playagain_msg.client_action == 1)
            {
                printf("Cliente deseja jogar novamente.\n");
                break;
            }
            else
            {
                memset(&playagain_msg, 0, sizeof(playagain_msg));
                playagain_msg.type = MSG_ERROR;
                printf("Erro: resposta inválida para jogar novamente.\n");
                snprintf(playagain_msg.message, MSG_SIZE, "Por favor, digite 1 para jogar novamente ou 0 para encerrar.");
                send(csock, &playagain_msg, sizeof(playagain_msg), 0);
            }
        }
    }
       
        // Validar msg.client_action (0–4)  e devolver MSG_RESULT / MSG_ERROR conforme a lógica do jogo.
        GameMessage end_msg;
        memset(&end_msg, 0, sizeof(end_msg));
        end_msg.type = MSG_END;
        end_msg.client_wins = client_wins;
        end_msg.server_wins = server_wins;
        snprintf(end_msg.message, MSG_SIZE,
                 "Fim de jogo!\nPlacar final: Você %d x %d Servidor\nObrigado por jogar!\n",
                 client_wins, server_wins);
        printf("Enviando placar final.\n");
        if (send(csock, &end_msg, sizeof(end_msg), 0) != sizeof(end_msg)) {
            fprintf(stderr, "Erro ao enviar MSG_END\n");
        }

        printf("Encerrando conexão.\n");
    close(csock);
    printf("Cliente desconectado.\n");
    }

    exit(EXIT_SUCCESS);
}
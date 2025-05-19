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

// Exibe instruções de uso
void usage(char **argv)
{
    printf("Uso: %s <v4|v6> <porta>\n", argv[0]);
    printf("exemplo: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    if (argc != 3) // Verifica se foram fornecidos exatamente 3 argumentos (nome do programa, modo, porta)
    {
        usage(argv);
    }

    // Inicializa gerador de números aleatórios
    srand(time(NULL));

    // Inicializa o endereço do servidor com base em IP e porta fornecidos
    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage))
    {
        usage(argv);
    }

    // Cria um socket TCP
    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1)
    {
        logexit("socket");
    }

    // Permite reuso do endereço
    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)))
    {
        logexit("setsockopt");
    }

    // Faz o bind do socket com o endereço configurado
    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(s, addr, sizeof(storage)))
    {
        logexit("bind");
    }

    // Coloca o socket em modo de escuta para até 10 conexões simultâneas
    if (0 != listen(s, 10))
    {
        logexit("listen");
    }

    // char addrstr[BUFSZ];
    // addrtostr(addr, addrstr, BUFSZ);

    // Exibe configurações do servidor inicializado
    printf("Servidor iniciado em modo IP%s na porta %s. Aguardando conexão..\n", argv[1], argv[2]);

    // Ações de jogadas disponíveis
    static const char *action_names[5] =
        {
            "Nuclear Attack", "Intercept Attack", "Cyber Attack", "Drone Strike", "Bio Attack"};

    // Resultados da partida possíveis (matriz: cliente x servidor)
    // 1 = cliente vence, 0 = servidor vence, -1 = empate
    static const int verify_winner[5][5] =
        {
            {-1, 0, 1, 1, 0},
            {1, -1, 0, 0, 1},
            {0, 1, -1, 1, 0},
            {0, 1, 0, -1, 1},
            {1, 0, 1, 0, -1}};

    while (1) // Loop principal do servidor
    {
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        // Aceita conexão do cliente
        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1)
        {
            logexit("accept");
        }
        // char caddrstr[BUFSZ];
        // addrtostr(caddr, caddrstr, BUFSZ);
        printf("Cliente conectado.\n");

        int client_wins = 0;
        int server_wins = 0;
        bool play_again = true;

        while (play_again) // Enquanto o cliente quiser jogar
        {
            int result = -1;
            while (result == -1) // Enquanto houver empate
            {
                GameMessage msg;
                memset(&msg, 0, sizeof(msg));

                // Envia opções de jogadas ao cliente
                printf("Apresentando as opções para o cliente.\n");
                snprintf(msg.message, MSG_SIZE,
                         "Escolha sua jogada:\n\n"
                         "0 - Nuclear Attack\n"
                         "1 - Intercept Attack\n"
                         "2 - Cyber Attack\n"
                         "3 - Drone Strike\n"
                         "4 - Bio Attack\n\n");
                if (send(csock, &msg, sizeof(msg), 0) != sizeof(msg))
                {
                    logexit("send");
                }

                // Aguarda jogada do cliente
                ssize_t count = recv(csock, &msg, sizeof(msg), 0);
                if (count <= 0)
                {
                    close(csock);
                    continue;
                }

                // Verifica tipo da mensagem recebida
                if (msg.type != MSG_RESPONSE)
                {
                    printf("Erro: opção inválida de jogada.\n");
                    continue;
                }

                // Verifica se a jogada do cliente é uma opção válida
                char *endptr;
                long client_action = strtol(msg.message, &endptr, 10);

                if (endptr == msg.message || *endptr != '\0')
                {
                    printf("Cliente nao escolheu um inteiro.\n");
                    printf("Erro: opção inválida de jogada.\n");
                    GameMessage error_msg;
                    memset(&error_msg, 0, sizeof(error_msg));
                    error_msg.type = MSG_ERROR;
                    snprintf(error_msg.message, MSG_SIZE, "Por favor, selecione um valor de 0 a 4.\n");
                    send(csock, &error_msg, sizeof(error_msg), 0);
                    continue;
                }
                else
                {
                    msg.client_action = (int)client_action;
                    printf("Cliente escolheu %d.\n", msg.client_action);

                    if (client_action < 0 || client_action > 4)
                    {
                        GameMessage error_msg;
                        memset(&error_msg, 0, sizeof(error_msg));
                        error_msg.type = MSG_ERROR;
                        printf("Erro: opção inválida de jogada.\n");
                        snprintf(error_msg.message, MSG_SIZE, "Por favor, selecione um valor de 0 a 4.\n");
                        send(csock, &error_msg, sizeof(error_msg), 0);
                        continue;
                    }

                   
                }

                // Gera jogada aleatória do servidor
                msg.server_action = rand() % 5;
                printf("Servidor escolheu aleatoriamente %d.\n", msg.server_action);

                // Verifica o resultado da partida na matriz
                msg.result = verify_winner[msg.client_action][msg.server_action];
                result = msg.result;

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
                    printf("Jogo empatado.\nSolicitando ao cliente mais uma escolha.\n");
                }

                // Envia mensagem de resultado para o cliente
                GameMessage result_msg;
                memset(&result_msg, 0, sizeof(result_msg));
                result_msg.type = MSG_RESULT;
                result_msg.client_action = msg.client_action;
                result_msg.server_action = msg.server_action;
                result_msg.result = result;
                snprintf(result_msg.message, MSG_SIZE,
                         "\nVocê escolheu: %s\nServidor escolheu: %s\nResultado: %s\n",
                         action_names[msg.client_action], action_names[msg.server_action], output_result);
                if (send(csock, &result_msg, sizeof(result_msg), 0) != sizeof(result_msg))
                {
                    close(csock);
                }

                // Exibe placar parcial, se não houve empate
                if (result != -1)
                {
                    printf("Placar atualizado: Cliente %d x %d Servidor\n", client_wins, server_wins);
                }
            }

            // Pergunta ao cliente se deseja jogar novamente
            while (1)
            {
                GameMessage playagain_msg;
                memset(&playagain_msg, 0, sizeof(playagain_msg));
                playagain_msg.type = MSG_PLAY_AGAIN_REQUEST;
                snprintf(playagain_msg.message, MSG_SIZE, "Deseja jogar novamente?\n1 - Sim\n0 - Não\n\n");
                printf("Perguntando se o cliente deseja jogar novamente.\n");
                if (send(csock, &playagain_msg, sizeof(playagain_msg), 0) != sizeof(playagain_msg))
                {
                    close(csock);
                }

                // Aguarda resposta do cliente
                ssize_t count = recv(csock, &playagain_msg, sizeof(playagain_msg), 0);
                if (count <= 0)
                {
                    close(csock);
                }

                if (playagain_msg.type != MSG_PLAY_AGAIN_RESPONSE)
                {

                    fprintf(stderr, "esperava MSG_PLAY_AGAIN_RESPONSE, recebeu tipo=%d\n", playagain_msg.type);
                    continue;
                }

                // Verifica se a jogada do cliente é uma opção válida
                char *endptr;
                long client_choice = strtol(playagain_msg.message, &endptr, 10);

                if (endptr == playagain_msg.message || *endptr != '\0')
                {
                    printf("Cliente nao escolheu um inteiro.\n");
                    printf("Erro: opção inválida de jogada.\n");
                    GameMessage error_msg;
                    memset(&error_msg, 0, sizeof(error_msg));
                    error_msg.type = MSG_ERROR;
                    snprintf(error_msg.message, MSG_SIZE, "Por favor, digite 1 para jogar novamente ou 0 para encerrar.\n");
                    send(csock, &error_msg, sizeof(error_msg), 0);
                    continue;
                }
                else
                {
                    playagain_msg.client_action = (int)client_choice;
                    printf("Cliente escolheu %d.\n", playagain_msg.client_action);

                    if (client_choice < 0 || client_choice > 4)
                    {
                        GameMessage error_msg;
                        memset(&error_msg, 0, sizeof(error_msg));
                        error_msg.type = MSG_ERROR;
                        printf("Erro: resposta inválida para jogar novamente.\n");
                        snprintf(error_msg.message, MSG_SIZE, "Por favor, digite 1 para jogar novamente ou 0 para encerrar.\n");
                        send(csock, &error_msg, sizeof(error_msg), 0);
                        continue;
                    }

                   
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

                

            }
        }

        // Envia mensagem final com placar ao cliente
        GameMessage end_msg;
        memset(&end_msg, 0, sizeof(end_msg));
        end_msg.type = MSG_END;
        end_msg.client_wins = client_wins;
        end_msg.server_wins = server_wins;
        snprintf(end_msg.message, MSG_SIZE,
                 "\nFim de jogo!\nPlacar final: Você %d x %d Servidor\nObrigado por jogar!\n",
                 client_wins, server_wins);
        printf("Enviando placar final.\n");
        if (send(csock, &end_msg, sizeof(end_msg), 0) != sizeof(end_msg))
        {
            fprintf(stderr, "Erro ao enviar MSG_END\n");
        }

        // Encerra conexão com cliente
        printf("Encerrando conexão.\n");
        close(csock);
        printf("Cliente desconectado.\n");
        exit(EXIT_SUCCESS);
    }

    return 0;
}
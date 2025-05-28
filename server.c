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

// Função para validar se a entrada é um inteiro e está dentro de um intervalo
// Retorna 0 se entrada válida, -1 se não é inteiro, -2 se inteiro mas fora do intervalo esperado
int validate_integer(const char *input, int min, int max, int *output)
{
    char *endptr;
    long value = strtol(input, &endptr, 10);

    if (endptr == input || *endptr != '\0')
    {
        return -1;
    }
    else if (value < min || value > max)
    {
        *output = (int)value;
        return -2;
    }
    else
    {
        *output = (int)value;
        return 0;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3) // Verifica se foram fornecidos exatamente 3 argumentos (nome do programa, modo, porta)
    {
        usage(argv);
    }

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

    // Coloca o socket em modo de escuta
    if (0 != listen(s, 10))
    {
        logexit("listen");
    }

    // Exibe configurações do servidor inicializado
    printf("Servidor iniciado em modo IP%s na porta %s. Aguardando conexão..\n", argv[1], argv[2]);

    // Inicializa gerador de números aleatórios
    srand(time(NULL));

    // Ações de jogadas disponíveis
    static const char *action_names[5] =
        {
            "Nuclear Attack", "Intercept Attack", "Cyber Attack", "Drone Strike", "Bio Attack"};

    // Resultados de partida possíveis (matriz: cliente x servidor)
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
        printf("Cliente conectado.\n");

        // Inicializa variáveis pra controlar o jogo
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
                msg.type = MSG_REQUEST;

                // Envia opções de jogadas ao cliente
                printf("Apresentando as opções para o cliente.\n");

                char options[MSG_SIZE] = "Escolha sua jogada:\n\n";
                for (int i = 0; i < 5; i++)
                {
                    char line[30];
                    snprintf(line, sizeof(line), "%d - %s\n", i, action_names[i]);
                    strcat(options, line);
                }
                strcat(options, "\n");

                snprintf(msg.message, MSG_SIZE, "%s", options);
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
                int client_action;
                int validation = validate_integer(msg.message, 0, 4, &client_action);

                if (validation != -1)
                {
                    printf("Cliente escolheu %d.\n", client_action);
                }
                if (validation != 0)
                {
                    GameMessage error_msg;
                    memset(&error_msg, 0, sizeof(error_msg));
                    error_msg.type = MSG_ERROR;

                    if (validation == -1)
                    {
                        printf("Cliente não escolheu um inteiro.\n");
                    }
                    else
                    {
                        printf("Erro: opção inválida de jogada.\n");
                    }

                    snprintf(error_msg.message, MSG_SIZE, "\nPor favor, selecione um valor de 0 a 4.\n");
                    send(csock, &error_msg, sizeof(error_msg), 0);
                    continue;
                }

                msg.client_action = client_action;

                // Gera jogada aleatória do servidor
                msg.server_action = rand() % 5;
                printf("Servidor escolheu aleatoriamente %d.\n", msg.server_action);

                // Verifica o resultado da partida na matriz
                msg.result = verify_winner[msg.client_action][msg.server_action];
                result = msg.result;
                const char *output_result;
                if (msg.result == 1)
                {
                    client_wins++;
                    output_result = "Vitória!";
                }
                else if (msg.result == 0)
                {
                    server_wins++;
                    output_result = "Derrota!";
                }
                else if (msg.result == -1)
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
                result_msg.result = msg.result;
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
                    printf("Conexão perdida com o cliente.\n");
                    close(csock);
                    play_again = false; // Força a saída do loop externo
                    break;           
                }

                if (playagain_msg.type != MSG_PLAY_AGAIN_RESPONSE)
                {

                    fprintf(stderr, "esperava MSG_PLAY_AGAIN_RESPONSE, recebeu tipo=%d\n", playagain_msg.type);
                    continue;
                }

                // Verifica se a jogada do cliente é uma opção válida
                int choice;
                int validation = validate_integer(playagain_msg.message, 0, 1, &choice);

                if (validation != 0)
                {
                    GameMessage error_msg;
                    memset(&error_msg, 0, sizeof(error_msg));
                    error_msg.type = MSG_ERROR;

                    if (validation == -1)
                    {
                        printf("Cliente não escolheu um inteiro.\n");
                    }
                    else
                    {
                        printf("Erro: resposta inválida para jogar novamente.\n");
                    }

                    snprintf(error_msg.message, MSG_SIZE, "\nPor favor, digite 1 para jogar novamente ou 0 para encerrar.\n");
                    send(csock, &error_msg, sizeof(error_msg), 0);
                    continue;
                }

                playagain_msg.client_action = choice;

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
    }

    return 0;
}
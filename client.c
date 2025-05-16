#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "common.h"

#define BUFSZ 1024 // CP

// Função para exibir instruções de uso
void usage(char **argv)
{                                                     // CP
    printf("Uso: %s <IP> <porta>\n", argv[0]);        // CP
    printf("exemplo: %s 127.0.0.1 51511\n", argv[0]); // CP
    exit(EXIT_FAILURE);                               // CP
}

int main(int argc, char *argv[])
{  // Verifica se foram fornecidos exatamente 3 argumentos (nome do programa, IP, porta)
    if (argc != 3)
    {                // CP
        usage(argv); // CP
    }

     // Configura a estrutura de endereço do servidor com base no IP e porta fornecidos
    struct sockaddr_storage storage; // CP
    if (0 != addrparse(argv[1], argv[2], &storage))
    {                // CP
        usage(argv); // CP
    }

    // Cria um socket TCP
    int s;                                         // CP
    s = socket(storage.ss_family, SOCK_STREAM, 0); // CP
    if (s == -1)
    {                      // CP
        logexit("socket"); // CP
    }

    // Conecta ao servidor
    struct sockaddr *addr = (struct sockaddr *)(&storage); // CP
    if (0 != connect(s, addr, sizeof(storage)))
    {                       // CP
        logexit("connect"); // CP
    }

    // Exibe o endereço ao qual o cliente se conectou
       // CP
    printf("Conectado ao servidor.\n"); // CP

    bool running = true;

    while (running)
    {
        // Receber mensagem do servidor
        GameMessage msg;
        size_t count = recv(s, &msg, sizeof(msg), 0);
        if (count <= 0)
        { // Servidor desconectado ou erro
            logexit("recv");
        }

        int valid_input = 0;
        // Processa a mensagem com base no tipo
        switch (msg.type)
        {
        case MSG_REQUEST: // Exibe a solicitação de jogada e lê a escolha do usuário
            printf("%s", msg.message);    // CP
            memset(&msg, 0, sizeof(msg)); // CP
            msg.type = MSG_RESPONSE;
            //scanf("%d", &msg.client_action); // Recebe a ação do usuário //CP

            valid_input = 0;
            while (!valid_input) {
                int input = scanf("%d", &msg.client_action);
                
                if (input == 1) {
                    valid_input = 1; // Entrada válida
                } else {
                    // Limpa o buffer de entrada para remover caracteres inválidos
                    int c;
                    while ((c = getchar()) != '\n' && c != EOF); // Descarta caracteres inválidos
                    printf("Por favor, selecione um valor de 0 a 4.\n ");
                }
            }

            // Manda resposta ao servidor
            if (send(s, &msg, sizeof(msg), 0) != sizeof(msg))
            {
                logexit("send"); // CP
            }
            break;
        case MSG_PLAY_AGAIN_REQUEST: // Exibe a solicitação de jogar novamente e lê a resposta do usuário
            printf("%s", msg.message);
            int play_again;
            valid_input = 0;
            while (!valid_input) {
                int input = scanf("%d", &play_again);
                
                if (input == 1) {
                    valid_input = 1; // Entrada válida
                } else {
                    // Limpa o buffer de entrada para remover caracteres inválidos
                    int c;
                    while ((c = getchar()) != '\n' && c != EOF); // Descarta caracteres inválidos
                    printf("Por favor, digite 1 para jogar novamente ou 0 para encerrar.\n$ ");
                }
            }
            GameMessage play_response;
            memset(&play_response, 0, sizeof(play_response));
            play_response.type = MSG_PLAY_AGAIN_RESPONSE;
            play_response.client_action = play_again;
            send(s, &play_response, sizeof(play_response), 0);
            break;

        case MSG_RESULT:  // Exibe o resultado da rodada
            printf("%s", msg.message);
            break;
        case MSG_ERROR: // Exibe mensagem de erro do servidor
           printf("%s\n", msg.message);
            break;
        case MSG_END: // Exibe mensagem final e encerra o programa
            printf("%s", msg.message);
            running= false;
        default:  // Tipo de mensagem inesperado
            fprintf(stderr, "Tipo inesperado: %d", msg.type);
        }
    }

    // if (msg.type != MSG_REQUEST) { // Tipo de mensagem inesperado
    //     fprintf(stderr, "esperava MSG_REQUEST, recebeu tipo=%d\n", msg.type);
    //     exit(EXIT_FAILURE);
    // }

    // Exibe a solicitação do servidor e obtém a escolha do usuário

    // Recebe o resultado do servidor
    // memset(&msg, 0, sizeof(msg));//CP
    // count = recv(s, &msg, sizeof(msg), 0);//CP
    // if (count <= 0) {
    //     logexit("recv");
    // }

    // Fecha conexão
    close(s);           // CP
    exit(EXIT_SUCCESS); // CP
}
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "common.h"

#define BUFSZ 1024 

// Exibe instruções de uso
void usage(char **argv)
{                                                     
    printf("Uso: %s <IP> <porta>\n", argv[0]);        
    printf("exemplo: %s 127.0.0.1 51511\n", argv[0]); 
    exit(EXIT_FAILURE);                               
}

// Função para ler entrada numérica (ignorar caracteres)
int get_valid_int(const char *mensagemErro)
{
    int valor, c;
    while (true)
    {
        if (scanf("%d", &valor) == 1)
        {
            break;
        }
        else
        {
            // Limpa o buffer de entrada
            while ((c = getchar()) != '\n' && c != EOF);
            printf("%s", mensagemErro);
        }
    }
    return valor;
}

int main(int argc, char *argv[])
{ 
    if (argc != 3)// Verifica se foram fornecidos exatamente 3 argumentos (nome do programa, IP, porta)
    {                
        usage(argv); 
    }

    // Configura a estrutura de endereço do servidor com base no IP e porta fornecidos
    struct sockaddr_storage storage; 
    if (0 != addrparse(argv[1], argv[2], &storage))
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

    // Conecta ao servidor
    struct sockaddr *addr = (struct sockaddr *)(&storage); 
    if (0 != connect(s, addr, sizeof(storage)))
    {                       
        logexit("connect"); 
    }    
    printf("Conectado ao servidor.\n"); 

    bool running = true; // Flag para controle do loop principal

    while (running) // Loop principal de comunicação com o servidor
    {
        // Recebe mensagem do servidor
        GameMessage msg;
        size_t count = recv(s, &msg, sizeof(msg), 0);
        if (count <= 0)
        { 
            logexit("recv");
        }

        int valid_input = 0; // Flag para validação de entrada (impede entrada de caracteres)
        
        // Processamento da mensagem de acordo com seu tipo
        switch (msg.type)
        {
        case MSG_REQUEST: // Exibe a solicitação de jogada e lê a escolha do usuário
            printf("%s", msg.message);    
            memset(&msg, 0, sizeof(msg)); 
            msg.type = MSG_RESPONSE;
           fgets(msg.message, MSG_SIZE, stdin);
            size_t len = strlen(msg.message);
            if (len > 0 && msg.message[len - 1] == '\n') {
                msg.message[len - 1] = '\0';
            }
            if (send(s, &msg, sizeof(msg), 0) != sizeof(msg)) {
                logexit("send");
            }
            break;

        case MSG_PLAY_AGAIN_REQUEST: // Exibe a solicitação de jogar novamente e lê a resposta do usuário
            printf("%s", msg.message);
            
            // Manda resposta ao servidor
            GameMessage play_response;
            memset(&play_response, 0, sizeof(play_response));
            play_response.type = MSG_PLAY_AGAIN_RESPONSE;
            fgets(play_response.message, MSG_SIZE, stdin);
            size_t len_again = strlen(play_response.message);
            if (len_again > 0 && play_response.message[len_again - 1] == '\n') {
                play_response.message[len_again - 1] = '\0';
            }
            if (send(s, &play_response, sizeof(play_response), 0) != sizeof(play_response)) {
                logexit("send");
            }
            
            break;

        case MSG_RESULT: // Exibe o resultado da rodada
            printf("%s\n", msg.message);
            break;
        case MSG_ERROR: // Exibe mensagem de erro do servidor
            printf("%s\n", msg.message);
            break;
        case MSG_END: // Exibe mensagem final e encerra o programa
            printf("%s\n", msg.message);
            running = false;
            close(s);
            exit(EXIT_SUCCESS);
        default: // Tipo de mensagem inesperado
            fprintf(stderr, "Tipo inesperado: %d\n", msg.type);
        }
    }

    // Fecha a conexão
    close(s);           
    exit(EXIT_SUCCESS); 
}
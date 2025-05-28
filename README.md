
# Jokenboom


## 📜 Descrição

**Jokenboom** é um jogo cliente-servidor desenvolvido em C, inspirado no clássico "pedra, papel, tesoura", mas adaptado para um cenário apocalíptico. No jogo, os jogadores escolhem entre cinco tipos de ataques:

- ☢️ Ataque Nuclear  
- 🚀 Interceptação de Mísseis  
- 💻 Ciberataque  
- ✈️ Bombardeio com Drones  
- 🧪 Armas Biológicas  

O cliente escolhe sua jogada, enquanto o servidor responde com uma jogada aleatória, calcula o resultado, atualiza o placar e permite múltiplas rodadas.

## ⚔️ Regras
Cada ataque vence de dois outros e perde para mais dois, conforme a matriz:

| Ataque          | Vence de                      | Perde para                        |
|-----------------|-------------------------------|------------------------------------|
| **Nuclear**     | Ciberataque, Drones           | Interceptação, Biológicas         |
| **Interceptação**| Nuclear, Biológicas          | Ciberataque, Drones               |
| **Ciberataque** | Interceptação, Biológicas     | Nuclear, Drones                   |
| **Drones**      | Interceptação, Biológicas     | Nuclear, Ciberataque              |
| **Biológicas**  | Nuclear, Ciberataque          | Interceptação, Drones             |


## 🎯 Funcionalidades

- Conexão TCP (IPv4 ou IPv6).
- Jogo interativo com feedback sobre jogadas e placar.
- Sistema de regras baseado em uma matriz de confrontos.
- Tratamento de erros robusto (entradas inválidas ou não numéricas).
- Suporte para repetir rodadas em caso de empate.
- Possibilidade de jogar várias partidas na mesma sessão.

## 🛠️ Tecnologias

- Linguagem C
- Sockets POSIX (Linux)
- Comunicação via TCP

## 🚀 Execução

### Compilar:

```bash
make
```

### Limpar arquivos de compilação:

```bash
make clean
```

### Executar o servidor:

```bash
./bin/server <v4|v6> <porta>
```

### Executar o cliente:

```bash
./bin/client <IP_servidor> <porta>
```

## 🏗️ Arquitetura

- **Servidor:** Recebe conexões (uma por vez), valida entradas, calcula resultados e gerencia o estado do jogo.
- **Cliente:** Envia jogadas, recebe respostas e interage via terminal.

## 📝 Observações

- O servidor suporta apenas uma conexão por vez.
- Empates não contam no placar e geram novas rodadas.

---
Desenvolvido por Júlia Machado para a disciplina de Redes de Computadores.

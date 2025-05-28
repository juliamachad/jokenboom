# Makefile para Jokenboom (server/client)
CC := gcc
CFLAGS := -std=c11 -Wall -Wextra
BUILDDIR := bin

# Targets finais
TARGETS := $(BUILDDIR)/server $(BUILDDIR)/client

# Fontes e objetos
SRCS_COMMON := common.c
SRCS_SERVER := server.c
SRCS_CLIENT := client.c
OBJS_COMMON := common.o
OBJS_SERVER := server.o
OBJS_CLIENT := client.o

.PHONY: all clean

all: $(TARGETS)

# Garante que o diretório bin/ existe antes de linkar
$(BUILDDIR):
	mkdir -p $@

# Link do server
$(BUILDDIR)/server: $(BUILDDIR) $(OBJS_COMMON) $(OBJS_SERVER)
	$(CC) $(CFLAGS) -o $@ $(OBJS_SERVER) $(OBJS_COMMON)

# Link do client
$(BUILDDIR)/client: $(BUILDDIR) $(OBJS_COMMON) $(OBJS_CLIENT)
	$(CC) $(CFLAGS) -o $@ $(OBJS_CLIENT) $(OBJS_COMMON)

# Compilação de .c para .o
%.o: %.c common.h
	$(CC) $(CFLAGS) -c $< -o $@

# Remove executáveis e objetos
clean:
	rm -f *.o
	rm -rf $(BUILDDIR)

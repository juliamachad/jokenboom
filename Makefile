# Makefile para Jokenboom (server/client)
CC := gcc
CFLAGS := -std=c11 -Wall -Wextra
BUILDDIR := bin
OBJDIR := obj

# Targets finais
TARGETS := $(BUILDDIR)/server $(BUILDDIR)/client

# Fontes e objetos
SRCS_COMMON := common.c
SRCS_SERVER := server.c
SRCS_CLIENT := client.c
OBJS_COMMON := $(OBJDIR)/common.o
OBJS_SERVER := $(OBJDIR)/server.o
OBJS_CLIENT := $(OBJDIR)/client.o

.PHONY: all clean

all: $(TARGETS)

# Garante que os diretórios existam
$(shell mkdir -p $(BUILDDIR) $(OBJDIR))

# Link do server 
$(BUILDDIR)/server: $(OBJS_COMMON) $(OBJS_SERVER) common.h
	$(CC) $(CFLAGS) -o $@ $(OBJS_SERVER) $(OBJS_COMMON)

# Link do client 
$(BUILDDIR)/client: $(OBJS_COMMON) $(OBJS_CLIENT) common.h
	$(CC) $(CFLAGS) -o $@ $(OBJS_CLIENT) $(OBJS_COMMON)

# Compilação de .c para .o 
$(OBJDIR)/%.o: %.c common.h
	$(CC) $(CFLAGS) -c $< -o $@

# Limpeza completa
clean:
	rm -f $(OBJDIR)/*.o
	rm -rf $(BUILDDIR)
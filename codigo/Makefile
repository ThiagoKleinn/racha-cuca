CC ?= cc
CFLAGS = -O2 -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L
LDLIBS = -lm

OBJ = tabuleiro.o imagem.o solver.o

all: racha_cuca teste

racha_cuca: main.o $(OBJ)
	$(CC) $(CFLAGS) -o $@ main.o $(OBJ) $(LDLIBS)

teste: teste.o $(OBJ)
	$(CC) $(CFLAGS) -o $@ teste.o $(OBJ) $(LDLIBS)

# o stb gera avisos que nao sao do nosso codigo
imagem.o: imagem.c imagem.h tabuleiro.h stb_image.h stb_image_write.h
	$(CC) $(CFLAGS) -Wno-unused-function -Wno-sign-compare -Wno-missing-field-initializers -c imagem.c

%.o: %.c
	$(CC) $(CFLAGS) -c $<

rodar-testes: teste
	./teste

clean:
	rm -f *.o racha_cuca teste

.PHONY: all clean rodar-testes

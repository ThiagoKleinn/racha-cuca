#ifndef TABULEIRO_H
#define TABULEIRO_H

#define N_MIN 2
#define N_MAX 6

typedef struct {
    int n;
    int pecas[N_MAX * N_MAX]; // 0 = vazio; a peca k pertence a posicao k-1
    int vazio;                // posicao do vazio
} Tabuleiro;

// direcao em que o VAZIO anda. O oposto de m e (m ^ 1)
typedef enum { CIMA, BAIXO, ESQUERDA, DIREITA } Movimento;

void tab_semente(unsigned int semente);
int  tab_iniciar(Tabuleiro *t, int n);          // estado final; 0 se n invalido
int  tab_vizinho(int n, int pos, Movimento m);  // posicao vizinha ou -1 se sair da borda
int  tab_pode_mover(const Tabuleiro *t, Movimento m);
int  tab_mover(Tabuleiro *t, Movimento m);      // 1 se moveu, 0 se invalido
void tab_embaralhar(Tabuleiro *t, int passos);  // so movimentos validos
void tab_aleatorio(Tabuleiro *t);               // permutacao aleatoria, sempre com solucao
int  tab_resolvido(const Tabuleiro *t);
int  tab_valido(const Tabuleiro *t);            // cada peca 1x, so um vazio
int  tab_solucionavel(const Tabuleiro *t);      // pela paridade das inversoes

#endif

//
// Created by Thiago Klein Costa Leite on 24/11/26.
//

#ifndef TABULEIRO_H
#define TABULEIRO_H

#define N_MIN 2
#define N_MAX 6

typedef struct {
    int n;
    int pecas[N_MAX * N_MAX]; // 0 = vazio
    int vazio;                // posicao do vazio
} Tabuleiro;

typedef enum { CIMA, BAIXO, ESQUERDA, DIREITA } Movimento;

int  tab_iniciar(Tabuleiro *t, int n);
int  tab_pode_mover(const Tabuleiro *t, Movimento m);
int  tab_mover(Tabuleiro *t, Movimento m);
void tab_embaralhar(Tabuleiro *t, int passos);
int  tab_resolvido(const Tabuleiro *t);

#endif //UNTITLED6_TABULEIRO_H

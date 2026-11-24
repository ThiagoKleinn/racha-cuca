//
// Created by Thiago Klein Costa Leite on 24/11/26.
//
#include <stdlib.h>
#include "tabuleiro.h"

int tab_iniciar(Tabuleiro *t, int n) {
    if (n < N_MIN || n > N_MAX) return 0;
    t->n = n;
    for (int i = 0; i < n * n - 1; i++)
        t->pecas[i] = i + 1;
    t->pecas[n * n - 1] = 0;
    t->vazio = n * n - 1;
    return 1;
}

int tab_pode_mover(const Tabuleiro *t, Movimento m) {
    int lin = t->vazio / t->n;
    int col = t->vazio % t->n;
    if (m == CIMA)     return lin > 0;
    if (m == BAIXO)    return lin < t->n - 1;
    if (m == ESQUERDA) return col > 0;
    return col < t->n - 1;
}

int tab_mover(Tabuleiro *t, Movimento m) {
    if (!tab_pode_mover(t, m)) return 0;
    int d = 1;
    if (m == CIMA) d = -t->n;
    else if (m == BAIXO) d = t->n;
    else if (m == ESQUERDA) d = -1;
    int destino = t->vazio + d;
    t->pecas[t->vazio] = t->pecas[destino];
    t->pecas[destino] = 0;
    t->vazio = destino;
    return 1;
}

// embaralha so com movimentos validos, entao sempre tem solucao
void tab_embaralhar(Tabuleiro *t, int passos) {
    int ultimo = -1;
    while (passos > 0) {
        int m = rand() % 4;
        if (ultimo != -1 && m == (ultimo ^ 1)) continue; // nao desfaz o anterior
        if (tab_mover(t, (Movimento)m)) {
            ultimo = m;
            passos--;
        }
    }
}

int tab_resolvido(const Tabuleiro *t) {
    for (int i = 0; i < t->n * t->n - 1; i++)
        if (t->pecas[i] != i + 1) return 0;
    return 1;
}
#include <stdlib.h>
#include "tabuleiro.h"

// gerador simples (xorshift), assim o resultado com a mesma semente
// e igual em qualquer sistema
static unsigned long long estado_rng = 88172645463325252ULL;

void tab_semente(unsigned int semente) {
    estado_rng = 88172645463325252ULL ^ ((unsigned long long)semente * 0x9E3779B97F4A7C15ULL);
    if (estado_rng == 0) estado_rng = 1;
}

static unsigned int sorteio(unsigned int limite) {
    estado_rng ^= estado_rng << 13;
    estado_rng ^= estado_rng >> 7;
    estado_rng ^= estado_rng << 17;
    return (unsigned int)((estado_rng >> 11) % limite);
}

int tab_iniciar(Tabuleiro *t, int n) {
    if (n < N_MIN || n > N_MAX) return 0;
    t->n = n;
    for (int i = 0; i < n * n - 1; i++)
        t->pecas[i] = i + 1;
    t->pecas[n * n - 1] = 0;
    t->vazio = n * n - 1;
    return 1;
}

int tab_vizinho(int n, int pos, Movimento m) {
    int lin = pos / n;
    int col = pos % n;
    switch (m) {
        case CIMA:     return lin > 0 ? pos - n : -1;
        case BAIXO:    return lin < n - 1 ? pos + n : -1;
        case ESQUERDA: return col > 0 ? pos - 1 : -1;
        case DIREITA:  return col < n - 1 ? pos + 1 : -1;
    }
    return -1;
}

int tab_pode_mover(const Tabuleiro *t, Movimento m) {
    return tab_vizinho(t->n, t->vazio, m) >= 0;
}

int tab_mover(Tabuleiro *t, Movimento m) {
    int destino = tab_vizinho(t->n, t->vazio, m);
    if (destino < 0) return 0;
    t->pecas[t->vazio] = t->pecas[destino];
    t->pecas[destino] = 0;
    t->vazio = destino;
    return 1;
}

// embaralha so com movimentos validos, entao sempre tem solucao
void tab_embaralhar(Tabuleiro *t, int passos) {
    int ultimo = -1;
    while (passos > 0) {
        int m = (int)sorteio(4);
        if (ultimo != -1 && m == (ultimo ^ 1)) continue; // nao desfaz o anterior
        if (tab_mover(t, (Movimento)m)) {
            ultimo = m;
            passos--;
        }
    }
}

int tab_resolvido(const Tabuleiro *t) {
    int total = t->n * t->n;
    for (int i = 0; i < total - 1; i++)
        if (t->pecas[i] != i + 1) return 0;
    return t->pecas[total - 1] == 0;
}

int tab_valido(const Tabuleiro *t) {
    if (t->n < N_MIN || t->n > N_MAX) return 0;
    int total = t->n * t->n;
    int visto[N_MAX * N_MAX] = {0};
    for (int i = 0; i < total; i++) {
        int v = t->pecas[i];
        if (v < 0 || v >= total || visto[v]) return 0;
        visto[v] = 1;
    }
    return t->vazio >= 0 && t->vazio < total && t->pecas[t->vazio] == 0;
}

int tab_solucionavel(const Tabuleiro *t) {
    int total = t->n * t->n;
    int inversoes = 0;
    for (int i = 0; i < total; i++) {
        if (t->pecas[i] == 0) continue;
        for (int j = i + 1; j < total; j++)
            if (t->pecas[j] != 0 && t->pecas[i] > t->pecas[j]) inversoes++;
    }
    if (t->n % 2 == 1)
        return inversoes % 2 == 0;
    int linha_de_baixo = t->n - t->vazio / t->n; // 1 = ultima linha
    return (inversoes + linha_de_baixo) % 2 == 1;
}

void tab_aleatorio(Tabuleiro *t) {
    int total = t->n * t->n;
    // Fisher-Yates
    for (int i = total - 1; i > 0; i--) {
        int j = (int)sorteio((unsigned int)(i + 1));
        int aux = t->pecas[i];
        t->pecas[i] = t->pecas[j];
        t->pecas[j] = aux;
    }
    for (int i = 0; i < total; i++)
        if (t->pecas[i] == 0) t->vazio = i;
    if (!tab_solucionavel(t)) {
        // trocar duas pecas (nao o vazio) inverte a paridade
        int a = -1, b = -1;
        for (int i = 0; i < total; i++) {
            if (t->pecas[i] == 0) continue;
            if (a < 0) a = i;
            else { b = i; break; }
        }
        int aux = t->pecas[a];
        t->pecas[a] = t->pecas[b];
        t->pecas[b] = aux;
    }
}

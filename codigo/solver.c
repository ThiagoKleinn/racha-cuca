// Estrategia: resolver por camadas (como uma pessoa faz).
//  1. Linha por linha, de cima para baixo, ate sobrarem 3 linhas.
//  2. Coluna por coluna, da esquerda para a direita, ate sobrar um bloco 3x3.
//  3. O bloco 3x3 final e resolvido de forma otima com IDA* (distancia de Manhattan).
// Cada peca (ou o par das duas ultimas de uma linha/coluna) e levada ao lugar
// com uma BFS pequena, sem mexer nas casas ja travadas.
// A solucao nao e a mais curta possivel, mas funciona rapido para qualquer N.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "solver.h"

typedef struct {
    Movimento *v;
    int tam, cap;
} Lista;

static void lista_add(Lista *l, Movimento m) {
    if (l->tam == l->cap) {
        l->cap = l->cap ? l->cap * 2 : 64;
        Movimento *novo = realloc(l->v, (size_t)l->cap * sizeof(Movimento));
        if (novo == NULL) {
            fprintf(stderr, "sem memoria\n");
            exit(1);
        }
        l->v = novo;
    }
    l->v[l->tam++] = m;
}

// BFS sobre o estado (vazio, peca 1, peca 2). Leva as k pecas (1 ou 2) para os
// alvos sem passar por casas travadas. Aplica os movimentos em t e na lista.
static int buscar_pecas(Tabuleiro *t, const int *pecas, const int *alvos, int k,
                        const unsigned char *trava, Lista *saida) {
    int n = t->n, M = n * n;
    int total = M * M * M;
    int *ant = malloc((size_t)total * sizeof(int));
    int *fila = malloc((size_t)total * sizeof(int));
    unsigned char *mov = malloc((size_t)total);
    if (!ant || !fila || !mov) {
        free(ant); free(fila); free(mov);
        return 0;
    }
    memset(ant, 0xFF, (size_t)total * sizeof(int)); // tudo -1

    int p0 = -1, p1 = 0;
    for (int pos = 0; pos < M; pos++) {
        if (t->pecas[pos] == pecas[0]) p0 = pos;
        if (k == 2 && t->pecas[pos] == pecas[1]) p1 = pos;
    }
    int inicio = (t->vazio * M + p0) * M + p1;
    int cabeca = 0, cauda = 0, fim = -1;
    ant[inicio] = inicio;
    fila[cauda++] = inicio;

    while (cabeca < cauda) {
        int s = fila[cabeca++];
        int q1 = s % M, q0 = (s / M) % M, e = s / (M * M);
        if (q0 == alvos[0] && (k == 1 || q1 == alvos[1])) {
            fim = s;
            break;
        }
        for (int d = 0; d < 4; d++) {
            int nb = tab_vizinho(n, e, (Movimento)d);
            if (nb < 0 || trava[nb]) continue;
            int r0 = q0, r1 = q1;
            if (nb == r0) r0 = e;                  // o vazio empurra a peca 1
            else if (k == 2 && nb == r1) r1 = e;   // ou a peca 2
            int ns = (nb * M + r0) * M + r1;
            if (ant[ns] != -1) continue;
            ant[ns] = s;
            mov[ns] = (unsigned char)d;
            fila[cauda++] = ns;
        }
    }

    int achou = (fim >= 0);
    if (achou) {
        int len = 0;
        for (int s = fim; s != inicio; s = ant[s]) len++;
        Movimento *caminho = malloc((size_t)(len > 0 ? len : 1) * sizeof(Movimento));
        if (caminho == NULL) {
            achou = 0;
        } else {
            int i = len;
            for (int s = fim; s != inicio; s = ant[s]) caminho[--i] = (Movimento)mov[s];
            for (i = 0; i < len; i++) {
                tab_mover(t, caminho[i]);
                lista_add(saida, caminho[i]);
            }
            free(caminho);
        }
    }
    free(ant); free(fila); free(mov);
    return achou;
}

// ---------- IDA* para o bloco final ----------

typedef struct {
    Tabuleiro *t;
    const unsigned char *trava;
    Movimento cam[128];
    int len;
} Busca;

static int dist(int n, int a, int b) {
    return abs(a / n - b / n) + abs(a % n - b % n);
}

static int h_regiao(const Tabuleiro *t, const unsigned char *trava) {
    int h = 0;
    for (int pos = 0; pos < t->n * t->n; pos++) {
        int v = t->pecas[pos];
        if (v != 0 && !trava[pos]) h += dist(t->n, pos, v - 1);
    }
    return h;
}

// devolve -1 se achou; senao o menor f que passou do limite
static int ida(Busca *b, int g, int h, int limite, int ultimo) {
    int f = g + h;
    if (f > limite) return f;
    if (h == 0) {
        b->len = g;
        return -1;
    }
    Tabuleiro *t = b->t;
    int menor = INT_MAX;
    for (int d = 0; d < 4; d++) {
        if (ultimo != -1 && d == (ultimo ^ 1)) continue; // nao volta
        int nb = tab_vizinho(t->n, t->vazio, (Movimento)d);
        if (nb < 0 || b->trava[nb]) continue;
        int peca = t->pecas[nb];
        // a peca vai de nb para a casa do vazio
        int delta = dist(t->n, t->vazio, peca - 1) - dist(t->n, nb, peca - 1);
        tab_mover(t, (Movimento)d);
        b->cam[g] = (Movimento)d;
        int r = ida(b, g + 1, h + delta, limite, d);
        if (r == -1) return -1;
        tab_mover(t, (Movimento)(d ^ 1)); // desfaz
        if (r < menor) menor = r;
    }
    return menor;
}

// ---------- programa principal do solver ----------

static int colocar_uma(Tabuleiro *t, int alvo, unsigned char *trava, Lista *l) {
    int pecas[1] = { alvo + 1 };
    int alvos[1] = { alvo };
    int ok = buscar_pecas(t, pecas, alvos, 1, trava, l);
    trava[alvo] = 1;
    return ok;
}

static int colocar_par(Tabuleiro *t, int alvo0, int alvo1, unsigned char *trava, Lista *l) {
    int pecas[2] = { alvo0 + 1, alvo1 + 1 };
    int alvos[2] = { alvo0, alvo1 };
    int ok = buscar_pecas(t, pecas, alvos, 2, trava, l);
    trava[alvo0] = 1;
    trava[alvo1] = 1;
    return ok;
}

int resolver(const Tabuleiro *inicial, Movimento **movs) {
    *movs = NULL;
    if (!tab_valido(inicial) || !tab_solucionavel(inicial)) return -1;

    Tabuleiro t = *inicial;
    int n = t.n;
    unsigned char trava[N_MAX * N_MAX];
    memset(trava, 0, sizeof(trava));
    Lista l = { NULL, 0, 0 };
    int ok = 1;

    // 1. linhas de cima, ate sobrarem 3 linhas
    for (int r = 0; r < n - 3 && ok; r++) {
        for (int c = 0; c < n - 2 && ok; c++)
            ok = colocar_uma(&t, r * n + c, trava, &l);
        if (ok) ok = colocar_par(&t, r * n + n - 2, r * n + n - 1, trava, &l);
    }

    // 2. colunas da esquerda, ate sobrar um bloco 3x3
    for (int c = 0; c < n - 3 && ok; c++) {
        for (int r = n - 3; r < n - 2 && ok; r++)
            ok = colocar_uma(&t, r * n + c, trava, &l);
        if (ok) ok = colocar_par(&t, (n - 2) * n + c, (n - 1) * n + c, trava, &l);
    }

    // 3. bloco final (ou o tabuleiro todo, se n = 2 ou 3)
    if (ok) {
        Busca b;
        b.t = &t;
        b.trava = trava;
        b.len = 0;
        int h = h_regiao(&t, trava);
        int limite = h;
        for (;;) {
            int r = ida(&b, 0, h, limite, -1);
            if (r == -1) break;
            if (r == INT_MAX) { ok = 0; break; }
            limite = r;
        }
        if (ok)
            for (int i = 0; i < b.len; i++) lista_add(&l, b.cam[i]);
    }

    // conferencia final: refaz os movimentos numa copia
    if (ok) {
        Tabuleiro copia = *inicial;
        for (int i = 0; i < l.tam && ok; i++)
            ok = tab_mover(&copia, l.v[i]);
        if (ok) ok = tab_resolvido(&copia);
    }

    if (!ok) {
        free(l.v);
        return -2;
    }
    *movs = l.v;
    return l.tam;
}

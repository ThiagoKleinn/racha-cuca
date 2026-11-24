#ifndef IMAGEM_H
#define IMAGEM_H

#include "tabuleiro.h"

#define IMG_OK            0
#define IMG_ERRO_ABRIR   -1  // arquivo nao existe ou nao e uma imagem
#define IMG_ERRO_PEQUENA -2  // imagem menor que N pixels em algum lado
#define IMG_ERRO_N       -3  // N fora de N_MIN..N_MAX

typedef struct {
    unsigned char *dados;    // RGB, 3 bytes por pixel
    int largura, altura;
    int n;
    int larg_peca, alt_peca; // tamanho de cada peca (divisao inteira)
    int desl_x, desl_y;      // onde comeca a area usada (sobras cortadas ao redor)
} Imagem;

int  img_carregar(Imagem *im, const char *caminho, int n);
void img_liberar(Imagem *im);

// monta a imagem do tabuleiro no estado t (vazio fica branco)
// quem chama deve dar free no resultado
unsigned char *img_renderizar(const Imagem *im, const Tabuleiro *t, int *larg, int *alt);
int  img_salvar_estado(const Imagem *im, const Tabuleiro *t, const char *caminho);

#endif

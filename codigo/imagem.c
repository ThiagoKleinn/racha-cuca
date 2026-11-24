#include <stdlib.h>
#include <string.h>
#include "imagem.h"

// bibliotecas de um arquivo so (stb), ver README
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int img_carregar(Imagem *im, const char *caminho, int n) {
    memset(im, 0, sizeof(*im));
    if (n < N_MIN || n > N_MAX) return IMG_ERRO_N;

    int canais;
    im->dados = stbi_load(caminho, &im->largura, &im->altura, &canais, 3);
    if (im->dados == NULL) return IMG_ERRO_ABRIR;

    if (im->largura < n || im->altura < n) {
        img_liberar(im);
        return IMG_ERRO_PEQUENA;
    }

    im->n = n;
    im->larg_peca = im->largura / n;
    im->alt_peca = im->altura / n;
    // se nao divide certinho, corta as sobras igualmente dos dois lados
    im->desl_x = (im->largura - im->larg_peca * n) / 2;
    im->desl_y = (im->altura - im->alt_peca * n) / 2;
    return IMG_OK;
}

void img_liberar(Imagem *im) {
    if (im->dados != NULL) stbi_image_free(im->dados);
    im->dados = NULL;
}

unsigned char *img_renderizar(const Imagem *im, const Tabuleiro *t, int *larg, int *alt) {
    int n = im->n;
    int lp = im->larg_peca, ap = im->alt_peca;
    int L = n * lp, A = n * ap;
    unsigned char *saida = malloc((size_t)L * A * 3);
    if (saida == NULL) return NULL;

    for (int pos = 0; pos < n * n; pos++) {
        int v = t->pecas[pos];
        int dest_lin = (pos / n) * ap, dest_col = (pos % n) * lp;
        for (int y = 0; y < ap; y++) {
            unsigned char *dst = saida + ((size_t)(dest_lin + y) * L + dest_col) * 3;
            if (v == 0) {
                memset(dst, 255, (size_t)lp * 3);
            } else {
                int origem = v - 1; // posicao de origem da peca
                int src_lin = im->desl_y + (origem / n) * ap + y;
                int src_col = im->desl_x + (origem % n) * lp;
                memcpy(dst, im->dados + ((size_t)src_lin * im->largura + src_col) * 3, (size_t)lp * 3);
            }
        }
    }
    *larg = L;
    *alt = A;
    return saida;
}

int img_salvar_estado(const Imagem *im, const Tabuleiro *t, const char *caminho) {
    int L, A;
    unsigned char *buf = img_renderizar(im, t, &L, &A);
    if (buf == NULL) return 0;
    int ok = stbi_write_png(caminho, L, A, 3, buf, L * 3);
    free(buf);
    return ok != 0;
}

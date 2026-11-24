#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include "tabuleiro.h"
#include "imagem.h"
#include "solver.h"

static void ajuda(const char *prog) {
    printf("Uso: %s <imagem> <N> [opcoes]\n", prog);
    printf("  <imagem>          arquivo PNG, JPG, BMP... \n");
    printf("  <N>               tamanho do tabuleiro, de %d a %d\n", N_MIN, N_MAX);
    printf("Opcoes:\n");
    printf("  --semente S       semente do embaralhamento (repete o mesmo tabuleiro)\n");
    printf("  --saida PASTA     pasta das imagens geradas (padrao: saida)\n");
    printf("  --quadros         salva uma imagem PNG para cada passo da solucao\n");
    printf("  --pausa           espera ENTER entre os passos\n");
}

static void imprimir_tabuleiro(const Tabuleiro *t) {
    for (int i = 0; i < t->n; i++) {
        printf("  ");
        for (int j = 0; j < t->n; j++) {
            int v = t->pecas[i * t->n + j];
            if (v == 0) printf(" . ");
            else printf("%2d ", v);
        }
        printf("\n");
    }
}

// o movimento guardado e o do vazio; a peca anda no sentido contrario
static const char *dir_peca(Movimento m) {
    switch (m) {
        case CIMA:     return "para baixo";
        case BAIXO:    return "para cima";
        case ESQUERDA: return "para a direita";
        case DIREITA:  return "para a esquerda";
    }
    return "?";
}

int main(int argc, char **argv) {
    if (argc < 3) {
        ajuda(argv[0]);
        return 2;
    }

    const char *caminho = argv[1];
    char *fim_num;
    errno = 0;
    long n_lido = strtol(argv[2], &fim_num, 10);
    if (errno != 0 || *argv[2] == '\0' || *fim_num != '\0' || n_lido < N_MIN || n_lido > N_MAX) {
        fprintf(stderr, "Erro: N invalido ('%s'). Use um inteiro de %d a %d.\n", argv[2], N_MIN, N_MAX);
        return 2;
    }
    int n = (int)n_lido;

    unsigned int semente = (unsigned int)time(NULL);
    const char *pasta = "saida";
    int quadros = 0, pausa = 0;

    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "--semente") == 0 && i + 1 < argc) {
            semente = (unsigned int)strtoul(argv[++i], NULL, 10);
        } else if (strcmp(argv[i], "--saida") == 0 && i + 1 < argc) {
            pasta = argv[++i];
        } else if (strcmp(argv[i], "--quadros") == 0) {
            quadros = 1;
        } else if (strcmp(argv[i], "--pausa") == 0) {
            pausa = 1;
        } else {
            fprintf(stderr, "Erro: opcao desconhecida '%s'\n\n", argv[i]);
            ajuda(argv[0]);
            return 2;
        }
    }

    Imagem im;
    int r = img_carregar(&im, caminho, n);
    if (r == IMG_ERRO_ABRIR) {
        fprintf(stderr, "Erro: nao foi possivel abrir a imagem '%s'.\n", caminho);
        return 3;
    }
    if (r == IMG_ERRO_PEQUENA) {
        fprintf(stderr, "Erro: a imagem e pequena demais para dividir em %dx%d.\n", n, n);
        return 3;
    }
    if (r != IMG_OK) {
        fprintf(stderr, "Erro ao carregar a imagem.\n");
        return 3;
    }

    if (mkdir(pasta, 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "Erro: nao foi possivel criar a pasta '%s'.\n", pasta);
        img_liberar(&im);
        return 3;
    }

    printf("Imagem: %dx%d pixels, tabuleiro %dx%d, pecas de %dx%d pixels\n",
           im.largura, im.altura, n, n, im.larg_peca, im.alt_peca);
    int sobra_x = im.largura - im.larg_peca * n, sobra_y = im.altura - im.alt_peca * n;
    if (sobra_x || sobra_y)
        printf("Sobras cortadas: %d px na largura e %d px na altura\n", sobra_x, sobra_y);

    tab_semente(semente);
    Tabuleiro t;
    tab_iniciar(&t, n);
    tab_aleatorio(&t);
    printf("Semente: %u\n\nTabuleiro embaralhado:\n", semente);
    imprimir_tabuleiro(&t);

    char arq[1024];
    snprintf(arq, sizeof(arq), "%s/embaralhado.png", pasta);
    if (!img_salvar_estado(&im, &t, arq)) fprintf(stderr, "Aviso: nao consegui salvar %s\n", arq);

    Movimento *movs = NULL;
    clock_t ini = clock();
    int total = resolver(&t, &movs);
    double seg = (double)(clock() - ini) / CLOCKS_PER_SEC;
    if (total < 0) {
        fprintf(stderr, "Erro: nao foi possivel resolver este tabuleiro (codigo %d).\n", total);
        img_liberar(&im);
        return 4;
    }
    printf("\nSolucao encontrada: %d movimentos (%.3f s)\n", total, seg);

    if (quadros) {
        snprintf(arq, sizeof(arq), "%s/quadro_%04d.png", pasta, 0);
        img_salvar_estado(&im, &t, arq);
    }

    for (int i = 0; i < total; i++) {
        int peca = t.pecas[tab_vizinho(t.n, t.vazio, movs[i])];
        if (!tab_mover(&t, movs[i])) {
            fprintf(stderr, "Erro interno: movimento invalido no passo %d\n", i + 1);
            free(movs);
            img_liberar(&im);
            return 5;
        }
        printf("\nPasso %d/%d: peca %d anda %s\n", i + 1, total, peca, dir_peca(movs[i]));
        imprimir_tabuleiro(&t);
        if (quadros) {
            snprintf(arq, sizeof(arq), "%s/quadro_%04d.png", pasta, i + 1);
            img_salvar_estado(&im, &t, arq);
        }
        if (pausa) {
            printf("(ENTER para continuar)");
            fflush(stdout);
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) { }
        }
    }

    if (!tab_resolvido(&t)) {
        fprintf(stderr, "Erro interno: o tabuleiro nao terminou resolvido\n");
        free(movs);
        img_liberar(&im);
        return 5;
    }
    snprintf(arq, sizeof(arq), "%s/resolvido.png", pasta);
    img_salvar_estado(&im, &t, arq);
    printf("\nPronto! Imagens em '%s/'.\n", pasta);

    free(movs);
    img_liberar(&im);
    return 0;
}

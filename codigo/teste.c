#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tabuleiro.h"
#include "imagem.h"
#include "solver.h"
#include "stb_image_write.h"

static int total = 0, falhas = 0;

#define CHECK(cond) do { \
    total++; \
    if (!(cond)) { falhas++; printf("  FALHOU (linha %d): %s\n", __LINE__, #cond); } \
} while (0)

static void secao(const char *nome) { printf("%s\n", nome); }

// tabuleiro valido qualquer, com o vazio na posicao pos
static void tab_com_vazio(Tabuleiro *t, int n, int pos) {
    tab_iniciar(t, n);
    int k = 1;
    for (int i = 0; i < n * n; i++)
        t->pecas[i] = (i == pos) ? 0 : k++;
    t->vazio = pos;
}

static void teste_limites_de_n(void) {
    secao("N valido e invalido");
    Tabuleiro t;
    CHECK(tab_iniciar(&t, 0) == 0);
    CHECK(tab_iniciar(&t, 1) == 0);
    CHECK(tab_iniciar(&t, -3) == 0);
    CHECK(tab_iniciar(&t, 7) == 0);
    for (int n = N_MIN; n <= N_MAX; n++) {
        CHECK(tab_iniciar(&t, n) == 1);
        CHECK(tab_valido(&t));
        CHECK(tab_resolvido(&t));
        CHECK(t.vazio == n * n - 1);
    }
}

static void teste_movimentos_em_todas_as_posicoes(void) {
    secao("Movimentos validos e invalidos (bordas e cantos)");
    for (int n = N_MIN; n <= N_MAX; n++) {
        for (int pos = 0; pos < n * n; pos++) {
            int lin = pos / n, col = pos % n;
            int esperado[4] = { lin > 0, lin < n - 1, col > 0, col < n - 1 };
            int delta[4] = { -n, n, -1, 1 };
            for (int m = 0; m < 4; m++) {
                Tabuleiro t, antes;
                tab_com_vazio(&t, n, pos);
                antes = t;
                CHECK(tab_pode_mover(&t, (Movimento)m) == esperado[m]);
                int moveu = tab_mover(&t, (Movimento)m);
                CHECK(moveu == esperado[m]);
                if (!esperado[m]) {
                    CHECK(memcmp(&t, &antes, sizeof(t)) == 0); // nada muda
                } else {
                    CHECK(t.vazio == pos + delta[m]);
                    CHECK(tab_valido(&t));
                    // so uma peca mudou de lugar: a que estava no destino
                    int diferentes = 0;
                    for (int i = 0; i < n * n; i++)
                        if (t.pecas[i] != antes.pecas[i]) diferentes++;
                    CHECK(diferentes == 2); // o vazio e a peca vizinha
                    CHECK(tab_mover(&t, (Movimento)(m ^ 1)) == 1); // volta
                    CHECK(memcmp(&t, &antes, sizeof(t)) == 0);
                }
            }
        }
    }
}

static void teste_embaralhamento(void) {
    secao("Embaralhamento: pecas unicas, um vazio, com solucao");
    for (int n = N_MIN; n <= N_MAX; n++) {
        for (unsigned int s = 1; s <= 30; s++) {
            Tabuleiro t;
            tab_semente(s);
            tab_iniciar(&t, n);
            tab_embaralhar(&t, 50 * n);
            CHECK(tab_valido(&t));
            CHECK(tab_solucionavel(&t));
            tab_iniciar(&t, n);
            tab_aleatorio(&t);
            CHECK(tab_valido(&t));
            CHECK(tab_solucionavel(&t));
            // conta o vazio e cada peca exatamente uma vez
            int vezes[N_MAX * N_MAX] = {0};
            for (int i = 0; i < n * n; i++) vezes[t.pecas[i]]++;
            int ok = 1;
            for (int v = 0; v < n * n; v++) if (vezes[v] != 1) ok = 0;
            CHECK(ok);
        }
    }
}

static void teste_paridade(void) {
    secao("Paridade (tabuleiro sem solucao)");
    for (int n = N_MIN; n <= N_MAX; n++) {
        Tabuleiro t;
        tab_iniciar(&t, n);
        CHECK(tab_solucionavel(&t));
        // trocar duas pecas torna o tabuleiro impossivel
        int aux = t.pecas[0]; t.pecas[0] = t.pecas[1]; t.pecas[1] = aux;
        CHECK(tab_valido(&t));
        CHECK(!tab_solucionavel(&t));
        Movimento *movs = NULL;
        CHECK(resolver(&t, &movs) == -1);
        CHECK(movs == NULL);
    }
    Tabuleiro t;
    tab_iniciar(&t, 3);
    t.pecas[0] = 5; // peca repetida: invalido
    CHECK(!tab_valido(&t));
    Movimento *movs = NULL;
    CHECK(resolver(&t, &movs) == -1);
}

// aplica a solucao passo a passo conferindo cada movimento
static int aplica_e_confere(const Tabuleiro *ini, const Movimento *movs, int qtd) {
    Tabuleiro t = *ini;
    for (int i = 0; i < qtd; i++) {
        if (!tab_pode_mover(&t, movs[i])) return 0;
        if (!tab_mover(&t, movs[i])) return 0;
        if (!tab_valido(&t)) return 0;
    }
    return tab_resolvido(&t);
}

static void teste_solver(void) {
    secao("Solver (permutacoes aleatorias e embaralhamento por movimentos)");
    int rodadas[N_MAX + 1] = { 0, 0, 300, 300, 150, 60, 40 };
    for (int n = N_MIN; n <= N_MAX; n++) {
        int todos_ok = 1;
        long soma = 0;
        int maior = 0;
        for (int s = 1; s <= rodadas[n]; s++) {
            Tabuleiro t;
            tab_semente((unsigned int)(1000 * n + s));
            tab_iniciar(&t, n);
            if (s % 2) tab_aleatorio(&t);
            else tab_embaralhar(&t, 200);
            Movimento *movs = NULL;
            int qtd = resolver(&t, &movs);
            if (qtd < 0 || !aplica_e_confere(&t, movs, qtd)) todos_ok = 0;
            if (qtd > maior) maior = qtd;
            if (qtd > 0) soma += qtd;
            free(movs);
        }
        CHECK(todos_ok);
        printf("  n=%d: %d tabuleiros resolvidos, media %.1f movimentos, maior %d\n",
               n, rodadas[n], (double)soma / rodadas[n], maior);
    }
    // ja resolvido: zero movimentos
    for (int n = N_MIN; n <= N_MAX; n++) {
        Tabuleiro t;
        tab_iniciar(&t, n);
        Movimento *movs = NULL;
        CHECK(resolver(&t, &movs) == 0);
        free(movs);
    }
    // casos extremos: vazio em cada canto, resto na ordem
    for (int n = N_MIN; n <= N_MAX; n++) {
        int cantos[4] = { 0, n - 1, n * (n - 1), n * n - 1 };
        for (int c = 0; c < 4; c++) {
            Tabuleiro t;
            tab_com_vazio(&t, n, cantos[c]);
            if (!tab_solucionavel(&t)) continue;
            Movimento *movs = NULL;
            int qtd = resolver(&t, &movs);
            CHECK(qtd >= 0 && aplica_e_confere(&t, movs, qtd));
            free(movs);
        }
    }
}

// ---------- imagem ----------

#define IMG_L 103   // nao e multiplo de 2, 3 nem 4
#define IMG_A 77
#define ARQ_TESTE "imagem_teste_tmp.png"

static void cria_imagem_teste(void) {
    unsigned char *px = malloc(IMG_L * IMG_A * 3);
    for (int y = 0; y < IMG_A; y++)
        for (int x = 0; x < IMG_L; x++) {
            unsigned char *p = px + (y * IMG_L + x) * 3;
            p[0] = (unsigned char)x;
            p[1] = (unsigned char)y;
            p[2] = (unsigned char)((x * 7 + y * 13) % 250); // nunca 255, para nao confundir com o vazio branco
        }
    stbi_write_png(ARQ_TESTE, IMG_L, IMG_A, 3, px, IMG_L * 3);
    free(px);
}

static void teste_imagem(void) {
    secao("Imagem (dimensoes nao multiplas de N)");
    cria_imagem_teste();
    for (int n = 2; n <= 4; n++) {
        Imagem im;
        CHECK(img_carregar(&im, ARQ_TESTE, n) == IMG_OK);
        CHECK(im.larg_peca == IMG_L / n && im.alt_peca == IMG_A / n);
        CHECK(im.desl_x >= 0 && im.desl_x * 2 <= IMG_L - im.larg_peca * n + 1);

        // estado final: igual ao recorte da imagem, com o canto inferior direito branco
        Tabuleiro t;
        tab_iniciar(&t, n);
        int L, A;
        unsigned char *final_ = img_renderizar(&im, &t, &L, &A);
        CHECK(final_ != NULL && L == n * im.larg_peca && A == n * im.alt_peca);
        int pixels_certos = 1;
        for (int y = 0; y < A; y++)
            for (int x = 0; x < L; x++) {
                unsigned char *p = final_ + (y * L + x) * 3;
                int no_canto = (y >= (n - 1) * im.alt_peca) && (x >= (n - 1) * im.larg_peca);
                if (no_canto) {
                    if (p[0] != 255 || p[1] != 255 || p[2] != 255) pixels_certos = 0;
                } else {
                    unsigned char *o = im.dados + ((im.desl_y + y) * im.largura + im.desl_x + x) * 3;
                    if (memcmp(p, o, 3) != 0) pixels_certos = 0;
                }
            }
        CHECK(pixels_certos);

        // embaralha, resolve e confere que a imagem volta igual
        for (unsigned int s = 1; s <= 10; s++) {
            Tabuleiro e;
            tab_semente(s);
            tab_iniciar(&e, n);
            tab_aleatorio(&e);
            int L2, A2;
            unsigned char *emb = img_renderizar(&im, &e, &L2, &A2);
            // embaralhado so e igual ao final se ja estava resolvido
            if (!tab_resolvido(&e)) CHECK(memcmp(emb, final_, (size_t)L * A * 3) != 0);
            Movimento *movs = NULL;
            int qtd = resolver(&e, &movs);
            CHECK(qtd >= 0);
            for (int i = 0; i < qtd; i++) tab_mover(&e, movs[i]);
            unsigned char *fim = img_renderizar(&im, &e, &L2, &A2);
            CHECK(memcmp(fim, final_, (size_t)L * A * 3) == 0);
            free(movs); free(emb); free(fim);
        }
        free(final_);
        img_liberar(&im);
    }

    // salvar em arquivo e ler de novo
    {
        Imagem im;
        Tabuleiro t;
        img_carregar(&im, ARQ_TESTE, 3);
        tab_iniciar(&t, 3);
        CHECK(img_salvar_estado(&im, &t, "estado_teste_tmp.png") == 1);
        Imagem lida;
        CHECK(img_carregar(&lida, "estado_teste_tmp.png", 3) == IMG_OK);
        CHECK(lida.largura == 3 * im.larg_peca && lida.altura == 3 * im.alt_peca);
        img_liberar(&lida);
        img_liberar(&im);
        remove("estado_teste_tmp.png");
    }
    remove(ARQ_TESTE);
}

static void teste_entradas_invalidas(void) {
    secao("Entradas invalidas");
    Imagem im;
    CHECK(img_carregar(&im, "nao_existe_mesmo.png", 3) == IMG_ERRO_ABRIR);
    CHECK(im.dados == NULL);

    // arquivo que existe mas nao e imagem
    FILE *f = fopen("nao_e_imagem_tmp.png", "w");
    fputs("isto nao e uma imagem\n", f);
    fclose(f);
    CHECK(img_carregar(&im, "nao_e_imagem_tmp.png", 3) == IMG_ERRO_ABRIR);
    remove("nao_e_imagem_tmp.png");

    cria_imagem_teste();
    CHECK(img_carregar(&im, ARQ_TESTE, 1) == IMG_ERRO_N);
    CHECK(img_carregar(&im, ARQ_TESTE, 7) == IMG_ERRO_N);
    CHECK(img_carregar(&im, ARQ_TESTE, 0) == IMG_ERRO_N);
    remove(ARQ_TESTE);

    // imagem menor que N pixels
    unsigned char px[3 * 3 * 3];
    memset(px, 100, sizeof(px));
    stbi_write_png("minuscula_tmp.png", 3, 3, 3, px, 9);
    CHECK(img_carregar(&im, "minuscula_tmp.png", 4) == IMG_ERRO_PEQUENA);
    CHECK(img_carregar(&im, "minuscula_tmp.png", 3) == IMG_OK);
    img_liberar(&im);
    remove("minuscula_tmp.png");
}

int main(void) {
    teste_limites_de_n();
    teste_movimentos_em_todas_as_posicoes();
    teste_embaralhamento();
    teste_paridade();
    teste_solver();
    teste_imagem();
    teste_entradas_invalidas();

    printf("\n%d verificacoes, %d falhas\n", total, falhas);
    return falhas == 0 ? 0 : 1;
}

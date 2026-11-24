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
    printf("  --quadros         salva um PNG por passo e gera visualizador.html\n");
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

// pagina HTML para passar os quadros (abre direto no navegador, sem servidor)
static void gerar_visualizador(const char *pasta, int total) {
    char arq[1024];
    snprintf(arq, sizeof(arq), "%s/visualizador.html", pasta);
    FILE *f = fopen(arq, "w");
    if (f == NULL) {
        fprintf(stderr, "Aviso: nao consegui criar %s\n", arq);
        return;
    }
    fputs("<!doctype html>\n<html lang=\"pt-br\">\n<head>\n<meta charset=\"utf-8\">\n"
          "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
          "<title>Racha Cuca</title>\n<style>\n"
          "body{font-family:system-ui,sans-serif;background:#1e1f22;color:#eee;margin:0;padding:16px;text-align:center}\n"
          "img{max-width:min(92vw,720px);height:auto;border:2px solid #444;border-radius:6px}\n"
          "button,select{font-size:18px;padding:6px 12px;margin:4px;border-radius:6px;border:1px solid #555;background:#2d2f33;color:#eee;cursor:pointer}\n"
          "input[type=range]{width:min(92vw,720px)}\n"
          "#info{margin:10px;font-size:18px}\n"
          "</style>\n</head>\n<body>\n"
          "<h2>Racha Cuca</h2>\n"
          "<div><img id=\"quadro\" alt=\"tabuleiro\"></div>\n"
          "<div id=\"info\"></div>\n"
          "<div><input id=\"barra\" type=\"range\" min=\"0\" value=\"0\"></div>\n"
          "<div>\n"
          "<button id=\"inicio\">&#9198;</button>"
          "<button id=\"volta\">&#9664;</button>"
          "<button id=\"toca\">&#9654; Tocar</button>"
          "<button id=\"avanca\">&#9654;</button>"
          "<button id=\"fim\">&#9197;</button>\n"
          "<select id=\"vel\"><option value=\"600\">lento</option>"
          "<option value=\"250\" selected>normal</option>"
          "<option value=\"80\">rapido</option></select>\n"
          "</div>\n"
          "<p>Teclas: setas esquerda e direita, espa&ccedil;o para tocar.</p>\n"
          "<script>\n", f);
    fprintf(f, "var TOTAL=%d;\n", total);
    fputs("var i=0,timer=null;\n"
          "var img=document.getElementById('quadro');\n"
          "var info=document.getElementById('info');\n"
          "var barra=document.getElementById('barra');\n"
          "var botao=document.getElementById('toca');\n"
          "barra.max=TOTAL;\n"
          "function nome(k){return 'quadro_'+String(k).padStart(4,'0')+'.png';}\n"
          "function mostra(k){\n"
          "  i=Math.max(0,Math.min(TOTAL,k));\n"
          "  img.src=nome(i);\n"
          "  barra.value=i;\n"
          "  if(i===0){info.textContent='Embaralhado';}\n"
          "  else if(i===TOTAL){info.textContent='Resolvido ('+i+' de '+TOTAL+')';}\n"
          "  else{info.textContent='Passo '+i+' de '+TOTAL;}\n"
          "}\n"
          "function para(){\n"
          "  if(timer){clearInterval(timer);timer=null;}\n"
          "  botao.innerHTML='&#9654; Tocar';\n"
          "}\n"
          "function toca(){\n"
          "  if(timer){para();return;}\n"
          "  if(i>=TOTAL){mostra(0);}\n"
          "  botao.innerHTML='&#10074;&#10074; Pausar';\n"
          "  timer=setInterval(function(){\n"
          "    if(i>=TOTAL){para();return;}\n"
          "    mostra(i+1);\n"
          "  },+document.getElementById('vel').value);\n"
          "}\n"
          "document.getElementById('inicio').onclick=function(){para();mostra(0);};\n"
          "document.getElementById('volta').onclick=function(){para();mostra(i-1);};\n"
          "document.getElementById('avanca').onclick=function(){para();mostra(i+1);};\n"
          "document.getElementById('fim').onclick=function(){para();mostra(TOTAL);};\n"
          "botao.onclick=toca;\n"
          "document.getElementById('vel').onchange=function(){if(timer){para();toca();}};\n"
          "barra.oninput=function(){para();mostra(+barra.value);};\n"
          "document.onkeydown=function(e){\n"
          "  if(e.key==='ArrowRight'){para();mostra(i+1);}\n"
          "  else if(e.key==='ArrowLeft'){para();mostra(i-1);}\n"
          "  else if(e.key===' '){e.preventDefault();toca();}\n"
          "};\n"
          "mostra(0);\n"
          "</script>\n</body>\n</html>\n", f);
    fclose(f);
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
    if (quadros) {
        gerar_visualizador(pasta, total);
        printf("Para ver passo a passo, abra '%s/visualizador.html' no navegador.\n", pasta);
    }

    free(movs);
    img_liberar(&im);
    return 0;
}
# Racha Cuca em C

Simulador do quebra-cabeça deslizante: recebe uma imagem e um tamanho N, divide a
imagem em N×N pedaços, tira o pedaço do canto inferior direito, embaralha e mostra
a solução passo a passo.

## Como compilar e rodar

Precisa só de um compilador C (no Mac: `xcode-select --install`). As bibliotecas de
imagem (`stb_image.h` e `stb_image_write.h`) já estão na pasta `codigo`, não precisa
instalar nada.

```
cd codigo
make
./racha_cuca ../imagens/exemplo.png 4
```

Opções:

| opção | o que faz |
|---|---|
| `--semente S` | repete o mesmo embaralhamento |
| `--saida PASTA` | pasta das imagens geradas (padrão `saida`) |
| `--quadros` | salva um PNG para cada passo (`quadro_0000.png`, `quadro_0001.png`...) |
| `--pausa` | espera ENTER entre os passos |

Sempre são gerados `embaralhado.png` e `resolvido.png` na pasta de saída. O passo a
passo também aparece no terminal (o `.` é o espaço vazio).

Testes: `make rodar-testes`

## Como funciona

- **Tabuleiro** (`tabuleiro.c`): vetor de N² inteiros. A peça `k` pertence à posição
  `k-1`, o `0` é o vazio e a posição do vazio fica guardada. Um movimento é o vazio
  andar para CIMA, BAIXO, ESQUERDA ou DIREITA. Fora da borda o movimento é recusado,
  então não existe diagonal, salto nem atravessar a borda.
- **Imagem** (`imagem.c`): cada peça tem `largura/N` × `altura/N` pixels. Se a imagem
  não divide certinho, as sobras (no máximo N-1 pixels) são cortadas igualmente dos
  quatro lados. A peça é só uma região da imagem original, nada é girado nem
  redimensionado. O vazio é desenhado em branco.
- **Embaralhamento**: permutação aleatória (Fisher-Yates). Metade das permutações não
  tem solução, então a paridade das inversões é checada (`tab_solucionavel`) e, se
  der impossível, duas peças são trocadas. O gerador é próprio (xorshift), assim a
  mesma semente dá o mesmo tabuleiro em qualquer computador.
- **Solução** (`solver.c`): por camadas.
  1. Linha por linha, de cima para baixo, até sobrarem 3 linhas.
  2. Coluna por coluna, da esquerda para a direita, até sobrar um bloco 3×3.
  3. O bloco 3×3 final é resolvido de forma ótima com IDA\* (distância de Manhattan).

  Cada peça é levada ao lugar com uma BFS pequena sobre (vazio, peça), sem mexer nas
  casas já travadas. As duas últimas peças de cada linha/coluna são levadas juntas
  (BFS sobre vazio + as duas peças), que é o caso que costuma travar. A solução não
  é a mais curta possível, mas é rápida para qualquer N.

## Por que essas escolhas

- IDA\* ou A\* sozinhos para o tabuleiro inteiro ficam inviáveis acima de 4×4 (o
  número de estados explode). Por camadas, cada busca tem no máximo N⁶ estados.
- O limite N ≤ 6 é a constante `N_MAX` em `tabuleiro.h`. Nos testes, N = 6 resolve em
  poucos milissegundos, então o limite é só o que foi pedido, não do algoritmo. Acima
  de 6 não foi testado.
- Imagem com menos de N pixels de largura ou altura é recusada, porque a peça teria
  tamanho zero.

## Resultados dos testes

`make rodar-testes` roda 3139 verificações (todas passam), incluindo:

- N inválido (0, 1, 7, negativo) e imagem inexistente, arquivo que não é imagem e
  imagem pequena demais;
- movimentos válidos e inválidos com o vazio em todas as posições (bordas e cantos)
  para N de 2 a 6, e que voltar o movimento restaura o tabuleiro;
- peças aparecem exatamente uma vez e só há um vazio, depois de embaralhar;
- tabuleiros sem solução (duas peças trocadas) são detectados;
- o solver resolve centenas de tabuleiros aleatórios para cada N de 2 a 6, e cada
  movimento da solução é conferido (válido, sem peça repetida);
- com imagem de 103×77 pixels (não múltipla de 2, 3 nem 4), a imagem refeita no fim
  da solução é igual, pixel a pixel, ao recorte original com o canto em branco.

Tamanho das soluções (nos testes):

| N | média de movimentos | maior |
|---|---|---|
| 2 | 3,5 | 6 |
| 3 | 21,8 | 28 |
| 4 | 82,4 | 113 |
| 5 | 181,5 | 236 |
| 6 | 323,1 | 455 |

## Arquivos

```
codigo/
  tabuleiro.c/.h   tabuleiro e movimentos
  imagem.c/.h      carregar, dividir e desenhar
  solver.c/.h      solução por camadas
  main.c           programa (terminal + PNGs)
  teste.c          testes
  Makefile
  stb_image.h, stb_image_write.h   bibliotecas de domínio público (nothings/stb)
imagens/exemplo.png   imagem de teste de 503×377
```

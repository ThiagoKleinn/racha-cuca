# Racha Cuca em C

Simulador do quebra-cabeça deslizante (o Racha Cuca). Você passa uma imagem e um tamanho N, o programa corta a imagem em N×N pedaços, tira o pedaço do canto de baixo à direita, embaralha tudo e mostra como resolver, passo a passo.

## Sobre as instruções

Eu uso Mac, então tudo aqui (comandos, caminhos, o jeito de compilar) foi feito e testado no macOS, no Terminal e no CLion. Em Linux deve funcionar igual, mas não testei. Em Windows também não testei e provavelmente precisa de ajuste, porque o programa usa o `mkdir` do sistema para criar a pasta das imagens.

## Como rodar no Mac

Precisa só de um compilador C. Se o Mac ainda não tiver, `xcode-select --install` resolve. As bibliotecas de imagem (`stb_image.h` e `stb_image_write.h`) já estão dentro da pasta `codigo`, não precisa instalar nada.

Pelo Terminal:

```
cd codigo
make
./racha_cuca ../imagens/exemplo.png 4
```

O primeiro argumento é a imagem e o segundo é o N (de 2 a 6). Tem também algumas opções no final:

- `--semente 7` repete o mesmo embaralhamento (muda o número para ter outro)
- `--saida pasta` escolhe onde salvar as imagens (o padrão é uma pasta `saida`)
- `--quadros` salva uma imagem para cada passo e cria um `visualizador.html`
- `--pausa` espera ENTER entre os passos no terminal

Sempre são criados o `embaralhado.png` e o `resolvido.png` na pasta de saída. Com `--quadros`, é só abrir o `visualizador.html` no navegador (duplo clique) e passar a solução com os botões, a barra ou as setas do teclado. A barra de espaço toca tudo sozinha.

Para rodar os testes: `make rodar-testes`.

## Como rodar no CLion

Eu uso o CLion, então tem um `CMakeLists.txt` na raiz com dois programas: `racha_cuca` e `teste`. Depois de abrir o projeto, é só recarregar o CMake e, em Run > Edit Configurations, colocar nos argumentos do `racha_cuca` algo como `../imagens/exemplo.png 4 --quadros`. O CLion roda de dentro da pasta `cmake-build-debug`, por isso o `../imagens` funciona e as imagens geradas ficam em `cmake-build-debug/saida`.

## Como funciona

O tabuleiro é só um vetor com N×N números. A peça `k` pertence à posição `k-1`, o `0` é o espaço vazio, e a posição do vazio fica guardada para não precisar procurar toda vez. Um movimento é o vazio andar para cima, baixo, esquerda ou direita. Se o movimento sair da borda, ele é recusado, então não existe diagonal, pulo nem atravessar a borda.

Para a imagem, cada peça tem `largura/N` por `altura/N` pixels. Quando a imagem não divide certinho, as sobras (no máximo N-1 pixels) são cortadas dos quatro lados. A peça é só um pedaço da imagem original, nada é girado nem redimensionado, e o vazio aparece em branco.

O embaralhamento é uma permutação aleatória. Só que metade das permutações não tem solução, então o programa confere a paridade das inversões e, se der impossível, troca duas peças para consertar. O gerador de números aleatórios é próprio, assim a mesma semente dá o mesmo tabuleiro em qualquer computador.

Para resolver, o programa vai por camadas, mais ou menos como uma pessoa faz: primeiro as linhas de cima, uma por uma, depois as colunas da esquerda, até sobrar um bloco 3×3. Esse bloco final é resolvido do jeito ótimo com IDA\*. Cada peça é levada ao lugar com uma busca pequena que não mexe no que já foi arrumado, e as duas últimas peças de cada linha ou coluna vão juntas, porque é aí que costuma travar. A solução não é a mais curta possível, mas é rápida para qualquer tamanho.

Eu fui por camadas porque resolver o tabuleiro inteiro com A\* ou IDA\* fica inviável de 5×5 para cima, o número de estados explode.

## Limites

O limite de N até 6 é uma constante (`N_MAX`, em `tabuleiro.h`). O 6×6 resolve em poucos milissegundos, então o limite é o que o enunciado pedia e não do algoritmo. Acima de 6 eu não testei. A imagem precisa ter pelo menos N pixels de largura e de altura, senão a peça teria tamanho zero e o programa recusa.

## Testes

O `teste.c` roda umas 3 mil verificações e todas passam. Ele confere N inválido, imagem que não existe, arquivo que não é imagem e imagem pequena demais. Confere também os movimentos com o vazio em todas as posições (bordas e cantos), se as peças aparecem exatamente uma vez depois de embaralhar, se tabuleiro sem solução é detectado e se o solver resolve centenas de tabuleiros aleatórios de 2×2 até 6×6, conferindo cada movimento. Por fim, testa com uma imagem de 103×77 (que não divide por 2, 3 nem 4) e vê se a imagem refeita no final é igual, pixel por pixel, ao recorte original.

Nos testes, o 3×3 resolveu em média com uns 22 movimentos e o 6×6 com uns 320, com o maior caso chegando a 455.

## Arquivos

Dentro de `codigo`: `tabuleiro.c` (tabuleiro e movimentos), `imagem.c` (carregar, dividir e desenhar), `solver.c` (a solução por camadas), `main.c` (o programa), `teste.c`, o `Makefile` e as duas bibliotecas stb. A imagem de teste está em `imagens/exemplo.png` (503×377).
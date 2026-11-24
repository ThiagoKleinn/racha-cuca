#ifndef SOLVER_H
#define SOLVER_H

#include "tabuleiro.h"

// Resolve o tabuleiro e devolve a sequencia de movimentos do VAZIO.
// Retorno: numero de movimentos (0 se ja esta resolvido),
//          -1 se o tabuleiro e invalido ou nao tem solucao,
//          -2 se deu erro interno.
// Quem chama deve dar free em *movs.
int resolver(const Tabuleiro *inicial, Movimento **movs);

#endif

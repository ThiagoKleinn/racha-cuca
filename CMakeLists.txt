cmake_minimum_required(VERSION 3.16)
project(racha_cuca C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_definitions(_POSIX_C_SOURCE=200809L)

add_executable(racha_cuca
    codigo/main.c codigo/tabuleiro.c codigo/imagem.c codigo/solver.c)
target_link_libraries(racha_cuca m)

add_executable(teste
    codigo/teste.c codigo/tabuleiro.c codigo/imagem.c codigo/solver.c)
target_link_libraries(teste m)

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include "estrutura.h"

#ifndef FUNCOES_H
#define FUNCOES_H

void renderizar();
bool processar_eventos();
void atualizar_temporizadores();
void atualizar_intervalos(int timestamp);
void carregar_assets();
void adicionar_bolha_aleatoria();
void renderizar_bolhas_aleatorias();
void remover_bolhas_aleatorias();
void verificar_fisica();
void protagonista_fisica(Personagem *personagem, const Uint8 *estado);
void personagem_andar(Personagem *personagem, bool esquerda);
void personagem_parado(Personagem *personagem);
void adicionar_tiro(float x, float y, float dx);
void renderizar_tiros();
void remover_tiros();

int obter_numero_aleatorio(int minimo, int maximo);

#endif
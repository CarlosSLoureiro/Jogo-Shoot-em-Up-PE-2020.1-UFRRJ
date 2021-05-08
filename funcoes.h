#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include "estrutura.h"

#ifndef FUNCOES_H
#define FUNCOES_H

void iniciar_SDL2();
void finalizar_SDL2();
void iniciar_jogo();
void carregar_assets();
void renderizar();
void criar_menu();
void renderizar_cursor_menu(int);
void criar_menu_item(char *);
void renderizar_menu();
void renderizar_logo_ufrrj();
void renderizar_lula_e_bob_dancando();
bool logica_do_menu(const Uint8 *);
void adicionar_cooldown(int);
bool processar_eventos();
void atualizar_temporizadores();
void atualizar_intervalos(int);
void adicionar_inimigo_aleatorio();
void renderizar_inimigos_aleatorio();
void remover_inimigos_aleatorio();
void adicionar_bolha_aleatoria();
void renderizar_bolhas_aleatorias();
void remover_bolhas_aleatorias();
void renderizar_interface();
void renderizar_protagonista();
void verificar_fisica();
void logica_do_jogo(Personagem *, const Uint8 *);
void personagem_andar(Personagem *, bool);
void personagem_parado(Personagem *);
void adicionar_tiro(float, float, float);
void renderizar_tiros();
void remover_tiros();

Texto * obter_texto(char *, int);
RGB obter_cor(int);
int obter_numero_aleatorio(int, int);

#endif
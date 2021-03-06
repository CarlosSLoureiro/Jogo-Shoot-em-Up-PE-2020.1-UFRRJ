#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#define LIMITE_DE_OBJETOS_NO_MAPA 100
#define LIMITE_DE_MENU_ITEMS 10

#ifndef ESTRUTURA_H
#define ESTRUTURA_H

typedef struct {
  char *texto;
} MenuItem;

typedef struct {
  bool aberto;
  int tamanho;
  int selecionado;
  MenuItem items[LIMITE_DE_MENU_ITEMS];
} Menu;

typedef struct {
  float x, y, dy;
  short vida;
  char *nome;
  int sprite, sprite_linha, altura, largura, sprite_bolhas, imunidade, energia;
  bool vivo, andando, pulando, viradoEsquerda, atirando, visivel;
} Personagem;

typedef struct {
  float x, y, dx;
} Tiro;

typedef struct {
  float x, y, base, velocidade, tamanho;
  bool inverter;
} Bolha;

typedef struct {
    float x, y, altura, largura;
    bool visivel, solido, acima;
} Objeto;

typedef struct {
    float x, y;
    Objeto *objetos[LIMITE_DE_OBJETOS_NO_MAPA];
} Mapa;

typedef struct {
  SDL_Texture *ufrrj;
  SDL_Texture *bob;
  SDL_Texture *vida;
  SDL_Texture *mapa;
  SDL_Texture *derrota;
  SDL_Texture *tiro;
  SDL_Texture *bolha;
  SDL_Texture *bolhas;
  SDL_Texture *arma;
  SDL_Texture *lula_dancando;
  SDL_Texture *bob_dancando;
  SDL_Texture *menu;
  SDL_Texture *mao;
  SDL_Texture *inimigo;
} Texturas;

typedef struct {
  Mix_Music *menu;
  Mix_Music *jogando;
  Mix_Music *derrota;
  Mix_Chunk *tiro;
  Mix_Chunk *choque;
  Mix_Chunk *selecionar;
  Mix_Chunk *vida;
} Sons;

typedef struct {
  double r;
  double g;
  double b;
} RGB;

typedef struct {
  SDL_Texture *textura;
  int altura;
  int largura;
} Texto;

#endif
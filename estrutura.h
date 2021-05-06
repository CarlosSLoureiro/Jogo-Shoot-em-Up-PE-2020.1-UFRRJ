#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#define LIMITE_DE_OBJETOS_NO_MAPA 100

#ifndef ESTRUTURA_H
#define ESTRUTURA_H

typedef struct {
  float x, y, dy;
  short vida;
  char *nome;
  int sprite, sprite_linha, altura, largura;
  bool vivo, andando, pulando, viradoEsquerda, atirando, visivel;
  
  SDL_Texture *textura;
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
    bool solido, acima;
} Objeto;

typedef struct {
    float x, y;
    Objeto *objetos[LIMITE_DE_OBJETOS_NO_MAPA];
} Mapa;

typedef struct {
  SDL_Texture *mapa;
  SDL_Texture *tiro;
  SDL_Texture *bolha;
  SDL_Texture *arma;
} Texturas;

typedef struct {
  Mix_Music *mapa;
  Mix_Chunk *tiro;
} Sons;

#endif
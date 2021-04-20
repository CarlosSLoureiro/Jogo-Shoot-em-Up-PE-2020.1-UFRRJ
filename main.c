#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdbool.h>
#include <stdio.h>

#define FPS 60
#define MS_POR_FRAME 1000 / FPS
#define MAX_TIROS 1000

/* Define as estruturas do jogo */

typedef struct {
  float x, y, dy;
  short vida;
  char *nome;
  int sprite;
  bool vivo, andando, viradoEsquerda, atirando, visivel;
  
  SDL_Texture *textura;
} Personagem;

typedef struct {
  float x, y, dx;
} Tiro;

typedef struct {
  SDL_Texture *mapa;
  SDL_Texture *tiro;
} Texturas;

typedef struct {
  Mix_Music *mapa;
  Mix_Chunk *tiro;
} Sons;


/* Define as variaveis do jogo */

Texturas texturas;
Sons sons;
Tiro *tiros[MAX_TIROS] = { NULL };
Personagem protagonista;
Personagem inimigo;

int tempoGlobal = 0;


/* Define as funções do jogo */

void adicionarTiro(float x, float y, float dx) {
  int indices = -1;

  for (int i = 0; i < MAX_TIROS; i++) {
    if (tiros[i] == NULL) {
      indices = i;
      break;
    }
  }
    
  if (indices >= 0) {
    int i = indices;
    tiros[i] = malloc(sizeof(Tiro));
    tiros[i]->x = x;
    tiros[i]->y = y;
    tiros[i]->dx = dx;
  }

  //Faz o som de tiro
  Mix_PlayChannel(-1, sons.tiro, 0);
}

void removerTiro(int i) {
  if (tiros[i]) {
    free(tiros[i]);
    tiros[i] = NULL;
  }
}

bool processar_eventos(SDL_Window *window, Personagem *protagonista) {
  SDL_Event event;
  
  bool finalizado = false;

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_WINDOWEVENT_CLOSE:
      {
        if (window) {
          SDL_DestroyWindow(window);
          window = NULL;
          finalizado = true;
        }
      }
      break;
      case SDL_KEYDOWN:
      {
        switch (event.key.keysym.sym) {
          case SDLK_ESCAPE:
            finalizado = true;
          break;
        }
      }
      break;
      case SDL_QUIT:
        finalizado = true;
      break;
    }
  }
  
  const Uint8 *estado = SDL_GetKeyboardState(NULL);

  if (!protagonista->atirando) {

    if (estado[SDL_SCANCODE_LEFT]) {

      protagonista->x -= 3;
      protagonista->andando = true;
      protagonista->viradoEsquerda = true;
    
      if (tempoGlobal % 6 == 0) {
        protagonista->sprite++;
        protagonista->sprite %= 4;  
      }

    } else if (estado[SDL_SCANCODE_RIGHT]) {
      protagonista->x += 3;
      protagonista->andando = true;
      protagonista->viradoEsquerda = false;
     
      if (tempoGlobal % 6 == 0) {
        protagonista->sprite++;
        protagonista->sprite %= 4;  
      }

    } else {
      protagonista->andando = false;
      protagonista->sprite = 4;
    }
  }

  if (!protagonista->andando) {
    if (estado[SDL_SCANCODE_SPACE]) {
      if (tempoGlobal % 6 == 0) {
        protagonista->sprite = ((protagonista->sprite == 4) ? 5 : 4);
        if (!protagonista->viradoEsquerda) {
          adicionarTiro(protagonista->x+35, protagonista->y+20, 3); 
        } else {
          adicionarTiro(protagonista->x+5, protagonista->y+20, -3);         
        }
      }  
  
      protagonista->atirando = true;
    } else {
      protagonista->sprite = 4;  
      protagonista->atirando = false;
    }
  }
  
  if (estado[SDL_SCANCODE_UP] && !protagonista->dy) {
    protagonista->dy = -8;
  }
  if (estado[SDL_SCANCODE_DOWN]) {
    //protagonista->y += 10;
  }
  
  return finalizado;
}

void renderizar(SDL_Renderer *renderer, Personagem *protagonista) {
  if (!Mix_PlayingMusic()) {
    Mix_PlayMusic(sons.mapa, 1);
  }

  //Define a cor do desenho para azul
  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
  
  //Deixa a tela com a cor do desenho (azul)
  SDL_RenderClear(renderer);
  
  //Define a cor do desenho para branco
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  
  //Copia o mapa para o renderer
  SDL_RenderCopy(renderer, texturas.mapa, NULL, NULL);

  //Copia o personagem protagonista para o renderer de acordo com o sprite atual
  if (protagonista->visivel) {
    SDL_Rect srcRect = { 40*protagonista->sprite, 0, 40, 50 };  
    SDL_Rect rect = { protagonista->x, protagonista->y, 40, 50 };  
    SDL_RenderCopyEx(renderer, protagonista->textura, &srcRect, &rect, 0, NULL, protagonista->viradoEsquerda);
  }

  //Copia o personagem inimigo para o renderer de acordo com o sprite atual
  if (inimigo.visivel) {
    SDL_Rect eSrcRect = { 40*inimigo.sprite, 0, 40, 50 };  
    SDL_Rect eRect = { inimigo.x, inimigo.y, 40, 50 };  
    SDL_RenderCopyEx(renderer, inimigo.textura, &eSrcRect, &eRect, 0, NULL, inimigo.viradoEsquerda);
  }

  for (int i = 0; i < MAX_TIROS; i++) if(tiros[i]) {
    SDL_Rect rect = { tiros[i]->x, tiros[i]->y, 8, 8 };  
    SDL_RenderCopy(renderer, texturas.tiro, NULL, &rect);
  }
  
  //Exibe na tela o que foi desenhado no renderer
  SDL_RenderPresent(renderer);
}

void atualizarLogica(Personagem *protagonista) {
  protagonista->y += protagonista->dy;
  protagonista->dy += 0.5;
  if (protagonista->y > 60) {
    protagonista->y = 60;
    protagonista->dy = 0;
  }
  
  //Verifica todos os tiros
  for (int i = 0; i < MAX_TIROS; i++) if(tiros[i]) {
    tiros[i]->x += tiros[i]->dx;
    
    //Verifica se o tiro acertou o inimigo
    if (tiros[i]->x > inimigo.x && tiros[i]->x < inimigo.x+40 && tiros[i]->y > inimigo.y && tiros[i]->y < inimigo.y+50) {
      inimigo.vivo = false;
    }
    
    if (tiros[i]->x < -1000 || tiros[i]->x > 1000) {
      removerTiro(i);
    }
  }
  
  if ((!inimigo.vivo) && tempoGlobal % 6 == 0) {
    if (inimigo.sprite < 6) {
      inimigo.sprite = 6;
    } else if (inimigo.sprite >= 6) {
      inimigo.sprite++;
      if (inimigo.sprite > 7) {
        inimigo.visivel = false;
        inimigo.sprite = 7;      
      }
    }
  }
  
  tempoGlobal++;
}

int main(int argc, char *argv[]) {
  SDL_Window *window;
  SDL_Renderer *renderer;
  
  //Inicia SDL2
  SDL_Init(SDL_INIT_VIDEO);

  //Inicia audio do SDL2
  SDL_Init(SDL_INIT_AUDIO);
  Mix_Init(MIX_INIT_OGG);
  Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 640);

  //Define valores iniciais
  protagonista.x = 50;
  protagonista.y = 0;
  protagonista.sprite = 4;  
  protagonista.vivo = true;
  protagonista.visivel = true;
  protagonista.viradoEsquerda = false;
  
  inimigo.x = 250;
  inimigo.y = 60;
  inimigo.sprite = 4;
  inimigo.viradoEsquerda = true;  
  inimigo.vivo = true;
  inimigo.visivel = true;
  
  //Define a window e o renderer
  window = SDL_CreateWindow("Jogo Shoot em Up", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  SDL_RenderSetLogicalSize(renderer, 320, 240);

  //Carrega imagem do protagonista
  SDL_Surface *imagem = IMG_Load("assets/imagens/protagonista-1.png");
  protagonista.textura = SDL_CreateTextureFromSurface(renderer, imagem);  
  SDL_FreeSurface(imagem);

  //Carrega imagem do inimigo
  imagem = IMG_Load("assets/imagens/inimigo-1.png");
  inimigo.textura = SDL_CreateTextureFromSurface(renderer, imagem);  
  SDL_FreeSurface(imagem);

  //Carrega o mapa  
  imagem = IMG_Load("assets/imagens/mapa-1.png");
  texturas.mapa = SDL_CreateTextureFromSurface(renderer, imagem);
  SDL_FreeSurface(imagem);

  //Carrega o tiro  
  imagem = IMG_Load("assets/imagens/tiro.png");
  texturas.tiro = SDL_CreateTextureFromSurface(renderer, imagem);
  SDL_FreeSurface(imagem);
  
  //Carrega as Musicas e Efeitos
  sons.mapa = Mix_LoadMUS("assets/sons/bob1.wav");
  sons.tiro = Mix_LoadWAV("assets/sons/tiro.wav");
  
  bool finalizado = false;
  
  //Loop de eventos
  do {
    //Pega tempo inicial
    unsigned inicio = SDL_GetTicks();

    //Verifica eventos
    finalizado = processar_eventos(window, &protagonista);
    
    //Atualiza a lógica
    atualizarLogica(&protagonista);
    
    //Atualiza o render
    renderizar(renderer, &protagonista);
    
    //Pega o tempo final
    unsigned fim = SDL_GetTicks();

    //Define a diferença entre os tempos 
    unsigned diferenca = fim - inicio;

    //Verifica se a diferença é menor que o ms por frame definido no inicio do codigo
    if (diferenca < MS_POR_FRAME) {
      SDL_Delay(MS_POR_FRAME - diferenca); //Define o delay de acordo com a diferença
    }
  } while (!finalizado);

  
  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);

  //Remove os sons
  Mix_FreeMusic(sons.mapa);
  Mix_FreeChunk(sons.tiro);

  //Remove as texturas
  SDL_DestroyTexture(protagonista.textura);
  SDL_DestroyTexture(texturas.mapa);
  SDL_DestroyTexture(texturas.tiro);
  SDL_DestroyTexture(inimigo.textura);
  
  //Remove todos as texturas de tiro
  for(int i = 0; i < MAX_TIROS; i++) {
    removerTiro(i);
  }
  
  SDL_Quit();
  return 0;
}
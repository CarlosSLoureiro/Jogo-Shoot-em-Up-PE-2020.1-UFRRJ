#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "estrutura.h"
#include "funcoes.h"

/* Define as constantes de configurações da janela do jogo */

#define FPS 60
#define MS_POR_FRAME 1000 / FPS
#define ALTURA 720
#define LARGURA 1280


/* Define as constantes de configurações de jogabilidade */

#define MAX_TIROS 100
#define MAX_BOLHAS 10
#define INTERVALO_BOLHAS 4


/* Define as variaveis do jogo */

SDL_Window *window;
SDL_Renderer *renderer;

long long tempoGlobal = 0;
long long tempoJogando = 0;
char data[64];

bool jogando = true;

Texturas texturas;
Sons sons;
Tiro *tiros[MAX_TIROS] = { NULL };
Bolha *bolhas[MAX_BOLHAS] = { NULL };
Personagem protagonista;
Personagem inimigo;
Mapa mapa;
Objeto chao;

int main(int argc, char *argv[]) {
  //Inicia SDL2
  SDL_Init(SDL_INIT_VIDEO);

  //Inicia audio do SDL2
  SDL_Init(SDL_INIT_AUDIO);
  Mix_Init(MIX_INIT_OGG);
  Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 640);

  chao.altura = 1;
  chao.largura = LARGURA;
  chao.x = 0;
  chao.y = ALTURA;
  chao.solido = true;

  mapa.objetos[0] = &chao;

  //Define valores iniciais
  protagonista.x = 50;
  protagonista.y = 0;
  protagonista.escala = 2;
  protagonista.sprite = 4;  
  protagonista.vivo = true;
  protagonista.visivel = true;
  protagonista.viradoEsquerda = false;
  
  inimigo.x = 250;
  inimigo.y = 100;
  inimigo.escala = 2;
  inimigo.sprite = 4;
  inimigo.viradoEsquerda = true;
  inimigo.vivo = true;
  inimigo.visivel = true;
  
  //Define a window e o renderer
  window = SDL_CreateWindow("Jogo Shoot em Up", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, LARGURA, ALTURA, 0);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  carregar_assets();
  
  bool finalizado = false;
  
  //Loop de eventos
  do {
    //Pega tempo inicial
    unsigned inicio = SDL_GetTicks();

    //Verifica eventos
    finalizado = processar_eventos();
    
    //Atualiza a fisica do mapa
    verificar_fisica();
    
    //Atualiza o render
    renderizar();
    
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
  SDL_DestroyTexture(texturas.bolha);
  SDL_DestroyTexture(inimigo.textura);
  
  //Remove todos as texturas de tiro
  for(int i = 0; i < MAX_TIROS; i++) {
    remover_tiros(i);
  }
  
  SDL_Quit();
  return 0;
}


/* Atualiza temporizadores */

void atualizar_temporizadores() {
  //Obtém a hora e data atual
  time_t temp = time(NULL);
  int timestamp = (int)temp;
  struct tm tm = *localtime(&temp);

  //Formtata a hora e data atual em yyyy-mm-dd hh:mm:ss
  char dt[64];
  sprintf(dt, "%02d-%02d-%d %02d:%02d:%02d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);

  //Verifica se a data anterior é diferente da data atual
  if (strcmp(dt, data)) {
    strcpy(data, dt); //Copia a string dt para data

    /* Teoricamente as funções abaixo rodarão a cada segundo. */
    //printf("%s\n", data); //Exibe a data atual

    int tempo = (tm.tm_hour * 3600) + (tm.tm_min * 60) + tm.tm_sec;

    atualizar_intervalos(tempo);
    
    //Conta o tempo de jogando
    if (jogando) {
      tempoJogando++;
    } 
  }

}

/* Atualiza os intervalos */

void atualizar_intervalos(int tempo) {
  //Sobe uma bolha na tela a cada x segundos
  if ((tempo % INTERVALO_BOLHAS) == 0) {
    adicionar_bolha_aleatoria();
  }
}


/* funções para: adicionar/renderizar/remover bolhas na tela */

void adicionar_bolha_aleatoria() {
  int indices = -1;

  for (int i = 0; i < MAX_BOLHAS; i++) {
    if (bolhas[i] == NULL) {
      indices = i;
      break;
    }
  }
    
  if (indices >= 0) {
    int i = indices;
    bolhas[i] = malloc(sizeof(Bolha));
    bolhas[i]->velocidade = obter_numero_aleatorio(1, 20) / 5;
    bolhas[i]->tamanho = obter_numero_aleatorio(50, 100);
    bolhas[i]->base = bolhas[i]->x = obter_numero_aleatorio(0, (LARGURA - (bolhas[i]->tamanho * 2)));
    bolhas[i]->inverter = false;
    bolhas[i]->y = ALTURA;
  } else {
    printf("sem bolhas!!\n");
  }
}

void renderizar_bolhas_aleatorias() {
  for (int i = 0; i < MAX_BOLHAS; i++) if(bolhas[i]) {
      if (!bolhas[i]->inverter) { 
        bolhas[i]->inverter = (bolhas[i]->x >= (bolhas[i]->base + bolhas[i]->tamanho));
      } else if (bolhas[i]->x == bolhas[i]->base) {
        bolhas[i]->inverter = false;
      }
      bolhas[i]->x += ((bolhas[i]->inverter) ? -.5 : .5);
      bolhas[i]->y -= bolhas[i]->velocidade;
      SDL_Rect rect = { bolhas[i]->x, bolhas[i]->y, bolhas[i]->tamanho, bolhas[i]->tamanho };
      SDL_RenderCopyEx(renderer, texturas.bolha, NULL, &rect, 0, NULL, 0);
  }
}

void remover_bolhas_aleatorias() {
  for (int i = 0; i < MAX_BOLHAS; i++) if (bolhas[i]) {
    if (bolhas[i]->y <= -(bolhas[i]->tamanho)) {
       free(bolhas[i]);
       bolhas[i] = NULL;
    }
  }
}


/* funções para: adicionar/renderizar/remover tiros na tela */

void adicionar_tiro(float x, float y, float dx) {
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

    //Faz o som de tiro
    Mix_PlayChannel(-1, sons.tiro, 0);
  }
}

void renderizar_tiros() {
  for (int i = 0; i < MAX_TIROS; i++) if(tiros[i]) {
    SDL_Rect rect = { tiros[i]->x, tiros[i]->y, 16, 16 };  
    SDL_RenderCopy(renderer, texturas.tiro, NULL, &rect);
  }
}

void remover_tiros(int i) {
  if (tiros[i]) {
    free(tiros[i]);
    tiros[i] = NULL;
  }
}


/* Função para processar os eventos do jogador */

bool processar_eventos() {
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

  if (!protagonista.atirando) {

    if (estado[SDL_SCANCODE_LEFT]) {

      protagonista.x -= 3;
      protagonista.andando = true;
      protagonista.viradoEsquerda = true;
    
      if (tempoGlobal % 6 == 0) {
        protagonista.sprite++;
        protagonista.sprite %= 4;  
      }

    } else if (estado[SDL_SCANCODE_RIGHT]) {
      protagonista.x += 3;
      protagonista.andando = true;
      protagonista.viradoEsquerda = false;
     
      if (tempoGlobal % 6 == 0) {
        protagonista.sprite++;
        protagonista.sprite %= 4;  
      }

    } else {
      protagonista.andando = false;
      protagonista.sprite = 4;
    }
  }

  if (!protagonista.andando) {
    if (estado[SDL_SCANCODE_SPACE]) {
      if (tempoGlobal % 6 == 0) {
        protagonista.sprite = ((protagonista.sprite == 4) ? 5 : 4);
        if (!protagonista.viradoEsquerda) {
          adicionar_tiro(protagonista.x+70, protagonista.y+40, 10); 
        } else {
          adicionar_tiro(protagonista.x, protagonista.y+40, -10);
        }
      }  
  
      protagonista.atirando = true;
    } else {
      protagonista.sprite = 4;  
      protagonista.atirando = false;
    }
  }
  
  if (estado[SDL_SCANCODE_UP] && !protagonista.dy) {
    protagonista.dy = -8;
  }
  if (estado[SDL_SCANCODE_DOWN]) {
    //protagonista.y += 10;
  }
  
  return finalizado;
}

void renderizar() {
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

  //Remove as bolhas aleatorias da tela
  remover_bolhas_aleatorias();

  //Copia o personagem protagonista para o renderer de acordo com o sprite atual
  if (protagonista.visivel) {
    SDL_Rect srcRect = { 40*protagonista.sprite, 0, 40, 50 };
    SDL_Rect rect = { protagonista.x, protagonista.y, 40*protagonista.escala, 50*protagonista.escala };
    SDL_RenderCopyEx(renderer, protagonista.textura, &srcRect, &rect, 0, NULL, protagonista.viradoEsquerda);
  }

  //Copia o personagem inimigo para o renderer de acordo com o sprite atual
  if (inimigo.visivel) {
    SDL_Rect eSrcRect = { 40*inimigo.sprite, 0, 40, 50 };
    SDL_Rect eRect = { inimigo.x, inimigo.y, 40*inimigo.escala, 50*inimigo.escala };
    SDL_RenderCopyEx(renderer, inimigo.textura, &eSrcRect, &eRect, 0, NULL, inimigo.viradoEsquerda);
  }

  //Atualiza temporizadores e renderiza eventos de invertvalo
  atualizar_temporizadores();
  renderizar_bolhas_aleatorias();
  renderizar_tiros();
  
  //Exibe na tela o que foi desenhado no renderer
  SDL_RenderPresent(renderer);
}

void verificar_fisica() {
  protagonista.y += protagonista.dy;
  protagonista.dy += 0.5;

  for (int i = 0; i < LIMITE_DE_OBJETOS_NO_MAPA; i++) if (mapa.objetos[i]) {

    int altura_do_personagem = 50 * protagonista.escala;
    if (mapa.objetos[i]->solido && (protagonista.y > (mapa.objetos[i]->y + mapa.objetos[i]->altura - altura_do_personagem))) {
      protagonista.y = (mapa.objetos[i]->y + mapa.objetos[i]->altura) - altura_do_personagem;
      protagonista.dy = 0;
      break;
    }
    
    //if (mapa.objetos[i] == NULL) {
      //break;
    //}
    
  }
  
  //Verifica todos os tiros
  for (int i = 0; i < MAX_TIROS; i++) if(tiros[i]) {
    tiros[i]->x += tiros[i]->dx;
    
    //Verifica se o tiro acertou o inimigo
    if (tiros[i]->x > inimigo.x && tiros[i]->x < inimigo.x+40 && tiros[i]->y > inimigo.y && tiros[i]->y < inimigo.y+50) {
      inimigo.vivo = false;
    }
    
    //Remove os tiros que estão fora da camera
    if (tiros[i]->x < 0 || tiros[i]->x > LARGURA) {
      remover_tiros(i);
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

void carregar_assets() {
  //Carrega imagem do protagonista
  SDL_Surface *imagem = IMG_Load("assets/imagens/protagonista-1.png");
  protagonista.textura = SDL_CreateTextureFromSurface(renderer, imagem);  
  SDL_FreeSurface(imagem);

  //Carrega imagem do inimigo
  imagem = IMG_Load("assets/imagens/inimigo-1.png");
  inimigo.textura = SDL_CreateTextureFromSurface(renderer, imagem);  
  SDL_FreeSurface(imagem);

  //Carrega o mapa  
  imagem = IMG_Load("assets/imagens/fundo-menu.jpeg");
  texturas.mapa = SDL_CreateTextureFromSurface(renderer, imagem);
  SDL_FreeSurface(imagem);

  //Carrega o tiro  
  imagem = IMG_Load("assets/imagens/tiro.png");
  texturas.tiro = SDL_CreateTextureFromSurface(renderer, imagem);
  SDL_FreeSurface(imagem);
  
  //Carrega a bolha
  imagem = IMG_Load("assets/imagens/bolha.png");
  texturas.bolha = SDL_CreateTextureFromSurface(renderer, imagem);
  SDL_FreeSurface(imagem);

  //Carrega as Musicas e Efeitos
  sons.mapa = Mix_LoadMUS("assets/sons/bob1.wav");
  sons.tiro = Mix_LoadWAV("assets/sons/tiro.wav");
}

int obter_numero_aleatorio(int minimo, int maximo) {
  return (int) rand()%(maximo-minimo + 1) + minimo;
}
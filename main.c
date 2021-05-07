#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "estrutura.h"
#include "funcoes.h"

#define NOME_DO_JOGO "Joguinho do Bob Esponja!"

/* Define as constantes de configurações da janela do jogo */

#define FPS 60
#define MS_POR_FRAME 1000 / FPS
#define ALTURA 720
#define LARGURA 1280


/* Define as constantes de configurações de jogabilidade */

#define MAX_TIROS 100
#define MAX_BOLHAS 10
#define MAX_INIMIGOS 10
#define INTERVALO_BOLHAS 3


/* Define as variaveis do jogo */

SDL_Window *window;
TTF_Font *font;
SDL_Renderer *renderer;

long long tempoGlobal = 0;
long long tempoJogando = 0;
int CoolDown = 0;
char data[64];

//Valores relativos que deve ser alterados de acordo com o tempo de jogo para dificultar a jogabilidade
int INIMIGOS_RESPAW = 1; //em segundos
int INIMIGOS_VELOCIDADE = 5; //em prixels

Menu menu;

Texturas texturas;
Sons sons;
Tiro *tiros[MAX_TIROS] = { NULL };
Bolha *bolhas[MAX_BOLHAS] = { NULL };
Personagem protagonista;
Personagem *inimigos[MAX_INIMIGOS] = { NULL };
Mapa mapa;
Objeto chao;

int main(int argc, char *argv[]) {
  iniciar_SDL2();
  carregar_assets();
  criar_menu();

  chao.altura = 1;
  chao.largura = LARGURA;
  chao.x = 0;
  chao.y = ALTURA;
  chao.solido = true;

  mapa.objetos[0] = &chao;

  //Define valores iniciais
  protagonista.x = 50;
  protagonista.y = 0;
  protagonista.altura = (40 * 3);
  protagonista.largura = (43 * 3);
  protagonista.sprite = 0;
  protagonista.sprite_linha = 0;
  protagonista.vivo = true;
  protagonista.visivel = true;
  protagonista.viradoEsquerda = false;
  
  bool finalizado = false;
  
  //Loop de eventos
  do {
    //Pega tempo inicial
    unsigned inicio = SDL_GetTicks();

    //Verifica eventos
    finalizado = processar_eventos();
    
    //Atualiza a fisica do mapa
    if (!menu.aberto) {
      verificar_fisica();
    } 
    
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

    if (CoolDown > 0) {
      CoolDown--;
    }
    tempoGlobal++;
  } while (!finalizado);

  finalizar_SDL2();
  return 0;
}

/* Iniciar o SDL2 */
void iniciar_SDL2() {
  //Inicia o video do SDL2
  SDL_Init(SDL_INIT_VIDEO);

  //Inicia audio do SDL2
  SDL_Init(SDL_INIT_AUDIO);
  Mix_Init(MIX_INIT_OGG);
  Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 640);

  //Inicia a biblioteca de texto do SDL2
  TTF_Init();

  //Define a window e o renderer
  window = SDL_CreateWindow(NOME_DO_JOGO, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, LARGURA, ALTURA, 0);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
}

/* Carrega o assets do jogo */
void carregar_assets() {
  //Carrega a font dos textos
  font = TTF_OpenFont("assets/fontes/KrabbyPatty.ttf", 80);

  //Carrega imagem do protagonista
  SDL_Surface *imagem = IMG_Load("assets/imagens/bob.png");
  protagonista.textura = SDL_CreateTextureFromSurface(renderer, imagem);  
  SDL_FreeSurface(imagem);

  //Carrega a imagem de fundo do menu  
  imagem = IMG_Load("assets/imagens/menu.png");
  texturas.menu = SDL_CreateTextureFromSurface(renderer, imagem);
  SDL_FreeSurface(imagem);

  //Carrega imagem da mao apontando para o item do menu
  imagem = IMG_Load("assets/imagens/mao-apontando.png");
  texturas.mao = SDL_CreateTextureFromSurface(renderer, imagem);
  SDL_FreeSurface(imagem);

  //Carrega imagem do lula molusco dancando (menu)
  imagem = IMG_Load("assets/imagens/lula-molusco-dancando.png");
  texturas.lula_dancando = SDL_CreateTextureFromSurface(renderer, imagem);
  SDL_FreeSurface(imagem);

  //Carrega imagem do lula molusco dancando (menu)
  imagem = IMG_Load("assets/imagens/bob-dancando.png");
  texturas.bob_dancando = SDL_CreateTextureFromSurface(renderer, imagem);
  SDL_FreeSurface(imagem);

  //Carrega imagem da arma do protagonista
  imagem = IMG_Load("assets/imagens/arma.png");
  texturas.arma = SDL_CreateTextureFromSurface(renderer, imagem);
  SDL_FreeSurface(imagem);

  //Carrega imagem do inimigo
  imagem = IMG_Load("assets/imagens/agua-viva.png");
  texturas.inimigo = SDL_CreateTextureFromSurface(renderer, imagem);  
  SDL_FreeSurface(imagem);

  //Carrega o mapa  
  imagem = IMG_Load("assets/imagens/mapa.png");
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

  //Carrega a bolha de morte
  imagem = IMG_Load("assets/imagens/bolhas.png");
  texturas.bolhas = SDL_CreateTextureFromSurface(renderer, imagem);
  SDL_FreeSurface(imagem);

  //Carrega as Musicas e Efeitos
  sons.menu = Mix_LoadMUS("assets/sons/menu.wav");
  sons.jogando = Mix_LoadMUS("assets/sons/jogando.wav");
  sons.tiro = Mix_LoadWAV("assets/sons/tiro.wav");
}

/* Finaliza o SDL2 */
void finalizar_SDL2() {
  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);

  //Remove a font
  TTF_CloseFont(font);
  TTF_Quit();

  //Remove os sons
  Mix_FreeMusic(sons.menu);
  Mix_FreeMusic(sons.jogando);
  Mix_FreeChunk(sons.tiro);

  //Remove as texturas
  SDL_DestroyTexture(protagonista.textura);
  SDL_DestroyTexture(texturas.lula_dancando);
  SDL_DestroyTexture(texturas.mao);
  SDL_DestroyTexture(texturas.menu);
  SDL_DestroyTexture(texturas.arma);
  SDL_DestroyTexture(texturas.mapa);
  SDL_DestroyTexture(texturas.tiro);
  SDL_DestroyTexture(texturas.bolha);
  SDL_DestroyTexture(texturas.bolhas);
  SDL_DestroyTexture(texturas.inimigo);
  
  //Remove todos as texturas de tiro
  for(int i = 0; i < MAX_TIROS; i++) {
    remover_tiros(i);
  }
  
  SDL_Quit();
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
    if (!menu.aberto) {
      tempoJogando++;
    } 
  }

}

/* Atualiza os intervalos */

void atualizar_intervalos(int tempo) {
  //Sobe uma bolha na tela a cada x segundos
  if (!(tempo % INTERVALO_BOLHAS)) {
    adicionar_bolha_aleatoria();
  }

  if ((!(tempo % INIMIGOS_RESPAW)) && !menu.aberto) {
    adicionar_inimigo_aleatorio();
  }
}


/* Funçoes do menu do jogo */

void criar_menu() {
  menu.aberto = true;
  menu.selecionado = 0;
  menu.tamanho = 0;
  
  criar_menu_item("Jogar!");
  criar_menu_item("Historia");
  criar_menu_item("Como Jogar");
  criar_menu_item("Ranking");
  criar_menu_item("Sair");
}

void criar_menu_item(char *str) {
  MenuItem item;
  item.texto = str;

  menu.items[menu.tamanho] = item;
  menu.tamanho++;
}

void renderizar_menu() {
  if (menu.aberto) {
    
    int distancia = 20;
    int somatorio = 0;
    int y_selecionado = 0;

    for (int i = 0; i < menu.tamanho; i++) {
      Texto * texto = obter_texto(menu.items[i].texto, 0xDDFF00);
      SDL_Rect rect;
      
      rect.x = 790;
      rect.y = 100;

      rect.w=texto->largura;
      rect.h=texto->altura;

      rect.y += somatorio + ((i > 0) ? (distancia * i) : 0);

      if (menu.selecionado == i) {
        y_selecionado = rect.y;
      }

      somatorio += rect.h;

      //SDL_RenderDrawRect(renderer, &rect); //Desenha bordas na textura
      SDL_RenderCopy(renderer, texto->textura, NULL, &rect);
      SDL_DestroyTexture(texto->textura);
    }

    renderizar_cursor_menu(y_selecionado + 5);
  }
}

void renderizar_cursor_menu(int y) {
  SDL_Rect srcRect = { 0, 0, 94, 82 };
  SDL_Rect rect = { 680, y, 94, 82 };
  SDL_RenderCopyEx(renderer, texturas.mao, &srcRect, &rect, 0, NULL, 0);
}

void renderizar_lula_e_bob_dancando() {
  //total de sprites: 52, delay: 4
  SDL_Rect srcRect = { 270 * round((tempoGlobal % (52 * 4)) / 4), 0, 270, 270 };
  SDL_Rect rect = { 100, 70, 270 * 2, 270 * 2 };
  SDL_RenderCopyEx(renderer, texturas.lula_dancando, &srcRect, &rect, 0, NULL, 0);

  //total de sprites: 17, delay: 4
  SDL_Rect srcRect2 = { 500 * round((tempoGlobal % (17 * 4)) / 4), 0, 500, 295 };
  SDL_Rect rect2 = { 820, 440, 500, 295 };
  SDL_RenderCopyEx(renderer, texturas.bob_dancando, &srcRect2, &rect2, 0, NULL, 0);
}

bool logica_do_menu(const Uint8 *estado) {
  bool finalizado = false;
  if (!CoolDown) {
    if (estado[SDL_SCANCODE_DOWN]) {
      menu.selecionado++;
      if (menu.selecionado >= menu.tamanho) {
        menu.selecionado = 0;
      }
      adicionar_cooldown(10);
    } else if (estado[SDL_SCANCODE_UP]) {
      menu.selecionado--;
      if (menu.selecionado < 0) {
        menu.selecionado = (menu.tamanho - 1);
      }
      adicionar_cooldown(10);
    } else if (estado[SDL_SCANCODE_RIGHT]) {
      adicionar_cooldown(10);
    } else if (estado[SDL_SCANCODE_LEFT]) {
      adicionar_cooldown(10);
    } else if (estado[SDL_SCANCODE_RETURN]) {
      switch (menu.selecionado) {
          case 0: //Jogar!
            menu.aberto = false;
            Mix_PlayMusic(sons.jogando, 1);
            break;
          case 4: //Sair
            finalizado = true;
            break;
      }
      adicionar_cooldown(10);
    }
  }

  return finalizado;
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
    bolhas[i]->velocidade = obter_numero_aleatorio(5, 20) / 5;
    bolhas[i]->tamanho = obter_numero_aleatorio(50, 100);
    bolhas[i]->base = bolhas[i]->x = obter_numero_aleatorio(0, (LARGURA - (bolhas[i]->tamanho * 2)));
    bolhas[i]->inverter = false;
    bolhas[i]->y = ALTURA;
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


/* funções para: adicionar/mover/renderizar/remover inimigos aleatórios no mapa */

void adicionar_inimigo_aleatorio() {
  int indices = -1;

  for (int i = 0; i < MAX_BOLHAS; i++) {
    if (inimigos[i] == NULL) {
      indices = i;
      break;
    }
  }
    
  if (indices >= 0) {
    int i = indices;
    inimigos[i] = malloc(sizeof(Personagem));
    inimigos[i]->altura = inimigos[i]->largura = obter_numero_aleatorio(50, 100);
    inimigos[i]->viradoEsquerda = ((obter_numero_aleatorio(1, 2) == 2) ? true : false);
    inimigos[i]->x = (inimigos[i]->viradoEsquerda ? (LARGURA - inimigos[i]->largura) : 0);
    inimigos[i]->y = obter_numero_aleatorio((ALTURA * 0.33), (ALTURA - (inimigos[i]->altura + 40)));
    inimigos[i]->sprite = inimigos[i]->sprite_bolhas = 0;
    inimigos[i]->vivo = true;
    inimigos[i]->visivel = true;
  }
}

void mover_inimigos_aleatorio() {
  if (!(tempoGlobal % 2)) {
    for (int i = 0; i < MAX_INIMIGOS; i++) if (inimigos[i]) {
      if ((inimigos[i]->vivo)) {
        int delta_mais = (inimigos[i]->x + INIMIGOS_VELOCIDADE);
        int delta_menos = (inimigos[i]->x - INIMIGOS_VELOCIDADE);

        inimigos[i]->x = ((delta_mais > protagonista.x && delta_menos < protagonista.x) ? protagonista.x : ((delta_mais > protagonista.x) ? delta_menos : delta_mais));

        delta_mais = (inimigos[i]->y + INIMIGOS_VELOCIDADE);
        delta_menos = (inimigos[i]->y - INIMIGOS_VELOCIDADE);

        inimigos[i]->y = ((delta_mais > protagonista.y && delta_menos < protagonista.y) ? protagonista.y : ((delta_mais > protagonista.y) ? delta_menos : delta_mais));
      }
    }
  }
}

void renderizar_inimigos_aleatorio() {
  for (int i = 0; i < MAX_INIMIGOS; i++) if (inimigos[i]) {

    if (tempoGlobal % 6 == 0) {
      inimigos[i]->sprite++;
      
      if (inimigos[i]->sprite > 6) {
        inimigos[i]->sprite = 0;
      }

      if (!inimigos[i]->vivo) {
        if (inimigos[i]->sprite_bolhas >= 1) {
          inimigos[i]->visivel = false;
        }

        inimigos[i]->sprite_bolhas++;
      }
    }

    if (inimigos[i]->visivel) {
      SDL_Rect eSrcRect = { 500*inimigos[i]->sprite, 0, 500, 500 };
      SDL_Rect eRect = { inimigos[i]->x, inimigos[i]->y, inimigos[i]->largura, inimigos[i]->altura };
      SDL_RenderCopyEx(renderer, texturas.inimigo, &eSrcRect, &eRect, 0, NULL, inimigos[i]->viradoEsquerda);
    }

    if (!inimigos[i]->vivo) {
      SDL_Rect eSrcRect2 = { 500*inimigos[i]->sprite_bolhas, 0, 500, 500 };
      SDL_Rect eRect2 = { inimigos[i]->x, inimigos[i]->y, inimigos[i]->largura, inimigos[i]->altura };
      SDL_RenderCopyEx(renderer, texturas.bolhas, &eSrcRect2, &eRect2, 0, NULL, inimigos[i]->viradoEsquerda);
    }
  }
}

void remover_inimigos_aleatorio() {
  for (int i = 0; i < MAX_INIMIGOS; i++) if (inimigos[i]) {
    if ((!inimigos[i]->vivo) && inimigos[i]->sprite_bolhas >= 7) {
       free(inimigos[i]);
       inimigos[i] = NULL;
    }
  }
}


/* funções de física e lógica do jogo */

void logica_do_jogo(Personagem *personagem, const Uint8 *estado) {
  if (estado[SDL_SCANCODE_LEFT]) {
    personagem_andar(personagem, true);
  } else if (estado[SDL_SCANCODE_RIGHT]) {
    personagem_andar(personagem, false);
  } else if (!personagem->pulando) {
    personagem_parado(personagem);
  }

  if (estado[SDL_SCANCODE_SPACE] && (!CoolDown)) {
    if (tempoGlobal % 6 == 0) {
      if (!personagem->viradoEsquerda) {
        adicionar_tiro(personagem->x + 120, personagem->y + 52, 10); 
      } else {
        adicionar_tiro(personagem->x, personagem->y + 52, -10);
      }
      adicionar_cooldown(10);
    }
    personagem->atirando = true;
  } else {
    personagem->atirando = false;
  }

  if (estado[SDL_SCANCODE_UP] && !personagem->pulando) {
    personagem->pulando = true;
    personagem->sprite = 0;
    personagem->dy = -20;
  }

  if (personagem->pulando) {
    personagem->sprite_linha = 40;
    if (tempoGlobal % 6 == 0 && personagem->sprite < 5) {
        personagem->sprite++;
    }
  } else {
    personagem->sprite_linha = 0;
  }
}

void personagem_andar(Personagem *personagem, bool esquerda) {
  personagem->x += (esquerda ? -4 : 4);
  personagem->andando = true;
  personagem->viradoEsquerda = esquerda;

  if (tempoGlobal % 6 == 0) {
    if (!personagem->pulando) {
      personagem->sprite++;
      personagem->sprite %= 8;
    } 
  }
}

void personagem_parado(Personagem *personagem) {
  personagem->sprite = 0;
  personagem->andando = false;
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
            if (!CoolDown) {
              if (!menu.aberto) {
                //Exibe o menu caso ele não esteja aberto
                menu.aberto = true;
                Mix_PlayMusic(sons.menu, 1);
              } else {
                //caso o meno ja esteja aberto, seleciona a opção "Sair" no menu
                menu.selecionado = 4;
              }
              adicionar_cooldown(10);
            }
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
  
  if (!menu.aberto) {
    logica_do_jogo(&protagonista, estado);
  } else {
    finalizado = logica_do_menu(estado);
  }
  
  return finalizado;
}

void renderizar() {
  //Define a cor do desenho para azul
  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
  
  //Deixa a tela com a cor do desenho (azul)
  SDL_RenderClear(renderer);
  
  //Define a cor do desenho para branco
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  
  if (menu.aberto) {
    if (!Mix_PlayingMusic()) {
      Mix_PlayMusic(sons.menu, 1);
    }

    SDL_RenderCopy(renderer, texturas.menu, NULL, NULL);

    //Renderizar menu
    renderizar_menu();
    renderizar_lula_e_bob_dancando();
    
  } else {
    if (!Mix_PlayingMusic()) {
      Mix_PlayMusic(sons.jogando, 1);
    }

    //Copia o mapa para o renderer
    SDL_RenderCopy(renderer, texturas.mapa, NULL, NULL);

    //Copia o personagem protagonista para o renderer de acordo com o sprite atual
    if (protagonista.visivel) {
      //adiciona a arma do protagonista
      SDL_Rect rect2 = { (protagonista.x + (protagonista.viradoEsquerda ? 0 : (protagonista.largura * 0.65))) - ((protagonista.atirando && (tempoGlobal % 2)) ? 2 : 0), protagonista.y + (protagonista.altura / 2.18) - ((protagonista.pulando) ? 12 : (((protagonista.sprite % 3) && protagonista.andando) ? 3 : 0)), 50, 25 };
      SDL_RenderCopyEx(renderer, texturas.arma, NULL, &rect2, 0, NULL, protagonista.viradoEsquerda);

      SDL_Rect srcRect = { 43*protagonista.sprite, protagonista.sprite_linha, 43, 40 };
      SDL_Rect rect = { protagonista.x, protagonista.y, protagonista.largura, protagonista.altura };
      SDL_RenderCopyEx(renderer, protagonista.textura, &srcRect, &rect, 0, NULL, protagonista.viradoEsquerda);
    }

    //Renderiza os tiros
    renderizar_tiros();

    //Move e renderiza os inimigos no mapa
    mover_inimigos_aleatorio();
    renderizar_inimigos_aleatorio();

    //Remove os inimigos mortos
    remover_inimigos_aleatorio();
  }

  //Atualiza temporizadores e renderiza eventos de invertvalo
  atualizar_temporizadores();
  renderizar_bolhas_aleatorias();

  //Remove as bolhas aleatorias da tela
  remover_bolhas_aleatorias();
  
  //Exibe na tela o que foi desenhado no renderer
  SDL_RenderPresent(renderer);
}

void verificar_fisica() {
  protagonista.y += protagonista.dy;
  protagonista.dy += 0.5;

  for (int i = 0; i < LIMITE_DE_OBJETOS_NO_MAPA; i++) if (mapa.objetos[i]) {

    int altura_do_personagem = protagonista.altura;
    if (mapa.objetos[i]->solido && (protagonista.y > (mapa.objetos[i]->y + mapa.objetos[i]->altura - altura_do_personagem))) {
      protagonista.y = (mapa.objetos[i]->y + mapa.objetos[i]->altura) - altura_do_personagem;
      protagonista.dy = 0;
      protagonista.pulando = false;
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
    for (int j = 0; j < MAX_INIMIGOS; j++) if (inimigos[j]) {
      if (inimigos[j]->vivo) {
        if (tiros[i]->x > inimigos[j]->x && tiros[i]->x < inimigos[j]->x+inimigos[j]->largura && tiros[i]->y > inimigos[j]->y && tiros[i]->y < inimigos[j]->y+inimigos[j]->altura) {
          inimigos[j]->vivo = false;
        }
      }
    }
    
    //Remove os tiros que estão fora da camera
    if (tiros[i]->x < 0 || tiros[i]->x > LARGURA) {
      remover_tiros(i);
    }
  }
}

/* Cria uma estrutura de texto com a textura inserida na estrutura */
Texto * obter_texto(char *msg, int hex) {
  Texto * texto = malloc(sizeof(Texto));

  RGB cor_rgb = obter_cor(hex);

  SDL_Color cor = {cor_rgb.r, cor_rgb.g, cor_rgb.b};
  SDL_Surface * texto_renderizado = TTF_RenderText_Solid( font, msg, cor);
  texto->textura = SDL_CreateTextureFromSurface(renderer, texto_renderizado);
  SDL_FreeSurface(texto_renderizado);
  SDL_QueryTexture(texto->textura, NULL, NULL, &(texto->largura), &(texto->altura));

  return texto;
}

/* Converte as cores em HEXDECIMAL para RGB */
RGB obter_cor(int hex) {
  RGB cor;

  cor.r = ((hex >> 16) & 0xFF);
  cor.g = ((hex >> 8) & 0xFF);
  cor.b = ((hex) & 0xFF);

  return cor; 
}

/* obtém um número aleatório entre um valor x e y */
int obter_numero_aleatorio(int minimo, int maximo) {
  return (int) rand()%(maximo-minimo + 1) + minimo;
}

/* Adiciona um valor ao CoolDown para obter um delay no devido evento a ser maipulado */
void adicionar_cooldown(int i) {
  CoolDown = i;
}
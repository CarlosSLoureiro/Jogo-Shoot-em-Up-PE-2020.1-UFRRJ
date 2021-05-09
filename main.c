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
#define MAX_BOLHAS 10
#define INTERVALO_BOLHAS 3

/* Define as constantes de configurações de jogabilidade */

#define MAX_VIDAS 6
#define VIDA_INICIAL 5
#define ENERGIA_INICIAL (MAX_VIDAS * 53)
#define MAX_TIROS 100
#define MAX_INIMIGOS 10
#define PROTAGONISTA_VELOCIDADE 6 //em pixels
#define INIMIGOS_RESPAW_INICIAL 2
#define INIMIGOS_VELOCIDADE_INICIAL 1
#define INIMIGOS_VELOCIDADE_INTERVALO 20
#define VIDA_DROPADA_POR_INTERVALO_DE_PONTOS 4
#define VIDA_DROPADA_CHANCE 33 //x chance em 100

/* Define as variaveis do jogo */

SDL_Window *window;
TTF_Font *font;
SDL_Renderer *renderer;

long long tempoGlobal = 0;
long long tempoJogando = 0;
long long pontos = 0;
int CoolDown = 0;
char data[64];

//Valores relativos que deve ser alterados de acordo com o tempo de jogo para dificultar a jogabilidade
int INIMIGOS_RESPAW = INIMIGOS_RESPAW_INICIAL; //em segundos
int INIMIGOS_VELOCIDADE = INIMIGOS_VELOCIDADE_INICIAL; //em prixels

Menu menu;

Texturas texturas;
Sons sons;
Tiro *tiros[MAX_TIROS] = { NULL };
Bolha *bolhas[MAX_BOLHAS] = { NULL };
Personagem protagonista;
Personagem *inimigos[MAX_INIMIGOS] = { NULL };
Mapa mapa;
Objeto chao;
Objeto vida_dropada;

int main(int argc, char *argv[]) {
  iniciar_SDL2();
  carregar_assets();
  criar_menu();
  
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

  //Carrega o logo da universidade
  SDL_Surface *imagem = IMG_Load("assets/imagens/UFRRJ.png");
  texturas.ufrrj = SDL_CreateTextureFromSurface(renderer, imagem);  
  SDL_FreeSurface(imagem);

  //Carrega imagem do protagonista  
  imagem = IMG_Load("assets/imagens/bob.png");
  texturas.bob = SDL_CreateTextureFromSurface(renderer, imagem);
  SDL_FreeSurface(imagem);

  //Carrega a imagem da vida do protagonista  
  imagem = IMG_Load("assets/imagens/vida.png");
  texturas.vida = SDL_CreateTextureFromSurface(renderer, imagem);
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

  //Carrega a imagem de game over  
  imagem = IMG_Load("assets/imagens/derrota.png");
  texturas.derrota = SDL_CreateTextureFromSurface(renderer, imagem);
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
  sons.derrota = Mix_LoadMUS("assets/sons/derrota.wav");
  sons.tiro = Mix_LoadWAV("assets/sons/tiro.wav");
  sons.choque = Mix_LoadWAV("assets/sons/choque.wav");
  sons.selecionar = Mix_LoadWAV("assets/sons/selecionar.wav");
  sons.vida = Mix_LoadWAV("assets/sons/vida.wav");
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
  Mix_FreeMusic(sons.derrota);
  Mix_FreeChunk(sons.tiro);
  Mix_FreeChunk(sons.choque);
  Mix_FreeChunk(sons.selecionar);
  Mix_FreeChunk(sons.vida);

  //Remove as texturas
  SDL_DestroyTexture(texturas.ufrrj);
  SDL_DestroyTexture(texturas.bob);
  SDL_DestroyTexture(texturas.lula_dancando);
  SDL_DestroyTexture(texturas.mao);
  SDL_DestroyTexture(texturas.menu);
  SDL_DestroyTexture(texturas.arma);
  SDL_DestroyTexture(texturas.mapa);
  SDL_DestroyTexture(texturas.derrota);
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

void iniciar_jogo() {
  tempoGlobal = 0;
  tempoJogando = 0;
  pontos = 0;
  CoolDown = 0;

  INIMIGOS_RESPAW = INIMIGOS_RESPAW_INICIAL; //em segundos
  INIMIGOS_VELOCIDADE = INIMIGOS_VELOCIDADE_INICIAL; //em prixels

  chao.altura = 1;
  chao.largura = LARGURA;
  chao.x = 0;
  chao.y = ALTURA;
  chao.solido = true;

  mapa.objetos[0] = &chao;

  //Define valores iniciais
  protagonista.altura = (40 * 3);
  protagonista.largura = (43 * 3);
  protagonista.x = (LARGURA / 2) - (protagonista.largura / 2);
  protagonista.y = 0;
  protagonista.imunidade = 0;
  protagonista.vida = VIDA_INICIAL;
  protagonista.energia = ENERGIA_INICIAL;
  protagonista.sprite = 0;
  protagonista.sprite_linha = 0;
  protagonista.vivo = true;
  protagonista.visivel = true;
  protagonista.viradoEsquerda = false;

  //Define o valor inicial da vida que dropa
  vida_dropada.visivel = false;

  //Remove todos os inimigos
  for (int i = 0; i < MAX_INIMIGOS; i++) if (inimigos[i]) {
    free(inimigos[i]);
    inimigos[i] = NULL;
  }

  //Remove todos os tiros
  for (int i = 0; i < MAX_TIROS; i++) if (tiros[i]) {
    free(tiros[i]);
    tiros[i] = NULL;
  }
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

  bool jogando = ((!menu.aberto) && protagonista.vivo);

  if (jogando) { //caso esteja jogando
    //Adiciona um inimigo aleatorio na tela caso esteja jogando
    if (!(tempo % INIMIGOS_RESPAW)) {
      adicionar_inimigo_aleatorio();
    }

    //Aumenta a velocidade dos inimigos a cada intervalo
    if (!(tempo % INIMIGOS_VELOCIDADE_INTERVALO)) {
      if (pontos > 20 || INIMIGOS_VELOCIDADE < 4) {
        INIMIGOS_VELOCIDADE++;
      }
    }

    //diminui o tempo de respaw do inimigo
    if (pontos > 50) {
      INIMIGOS_RESPAW = 1;
    } 

    //Diminui o tempo de imunidade do protagonista
    if (protagonista.imunidade > 0) {
      protagonista.imunidade--;
    }
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

void renderizar_logo_ufrrj() {
  SDL_Rect srcRect = { 0, 0, 700, 687 };
  SDL_Rect rect = { 10, 10, (700 / 5),  (687 / 5)};
  SDL_RenderCopyEx(renderer, texturas.ufrrj, &srcRect, &rect, 0, NULL, 0);
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
      Mix_PlayChannel(-1, sons.selecionar, 0);
      adicionar_cooldown(10);
    } else if (estado[SDL_SCANCODE_UP]) {
      menu.selecionado--;
      if (menu.selecionado < 0) {
        menu.selecionado = (menu.tamanho - 1);
      }
      Mix_PlayChannel(-1, sons.selecionar, 0);
      adicionar_cooldown(10);
    } else if (estado[SDL_SCANCODE_RIGHT]) {
      adicionar_cooldown(10);
    } else if (estado[SDL_SCANCODE_LEFT]) {
      adicionar_cooldown(10);
    } else if (estado[SDL_SCANCODE_RETURN]) {
      switch (menu.selecionado) {
          case 0: //Jogar!
            if (tempoJogando <= 0 || !protagonista.vivo) {
              iniciar_jogo();
            }
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
      SDL_RenderCopyEx(renderer, texturas.bolhas, &eSrcRect2, &eRect2, 0, NULL, inimigos[i]->x < protagonista.x);
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


/* função para: renderizar e checar se o protagonista pegou a vida */
void verificar_vida_dropada() {
  if (vida_dropada.visivel) {

    //Faz a vida cair no chão
    if ((vida_dropada.y + vida_dropada.altura) < ALTURA) {
      vida_dropada.y++;
    }

    //Renderiza a vida na tela
    SDL_Rect srcRect = { 0, 0, 254, 254 };
    SDL_Rect rect = { vida_dropada.x, vida_dropada.y, vida_dropada.largura, vida_dropada.altura };
    SDL_RenderCopyEx(renderer, texturas.vida, &srcRect, &rect, 0, NULL, 0);

    //Verifica se o protagonista pegou a vida
    if ((vida_dropada.x + vida_dropada.largura) > protagonista.x && vida_dropada.x < (protagonista.x + (protagonista.largura * 0.8)) && (vida_dropada.y + vida_dropada.altura) > protagonista.y && vida_dropada.y < (protagonista.y + protagonista.altura)) {
      protagonista.vida++;
      Mix_PlayChannel(-1, sons.vida, 0);
      vida_dropada.visivel = false;
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
    protagonista.energia = ENERGIA_INICIAL;
  } else {
    personagem->sprite_linha = 0;
    if (tempoJogando > 5) {
      protagonista.energia--;
    }
  }

  if (protagonista.energia <= 0) {
    remover_vida_protagonista(false);
    protagonista.energia = ENERGIA_INICIAL;
  }
}

void personagem_andar(Personagem *personagem, bool esquerda) {
  personagem->x += (esquerda ? (PROTAGONISTA_VELOCIDADE * -1) : PROTAGONISTA_VELOCIDADE);
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

void remover_vida_protagonista(bool deixar_imune) {
  protagonista.vida--;
  protagonista.vivo = (protagonista.vida > 0);
  if (!protagonista.vivo) {
    Mix_PlayMusic(sons.derrota, 1);
  } else if (deixar_imune) {
    protagonista.imunidade = 3; //em segundos
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
  
  if ((!protagonista.vivo) && (tempoJogando > 0)) {
    if (estado[SDL_SCANCODE_RETURN]) {
      menu.aberto = true;
      Mix_PlayMusic(sons.menu, 1);
      tempoJogando = 0;
      adicionar_cooldown(10);
    }
  } else if (!menu.aberto) {
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
  
  if (menu.aberto) { //caso o menu esteja aberto

    if (!Mix_PlayingMusic()) {
      Mix_PlayMusic(sons.menu, 1);
    }

    SDL_RenderCopy(renderer, texturas.menu, NULL, NULL);

    //Renderizar menu
    renderizar_menu();
    renderizar_lula_e_bob_dancando();
    renderizar_logo_ufrrj();
    
  } else if (!protagonista.vivo) { //caso o protagonista não tenha vida
    
    if (!Mix_PlayingMusic()) {
      Mix_PlayMusic(sons.derrota, 1);
    }

    renderizar_gameover();

  } else { //esta jogando...
    
    if (!Mix_PlayingMusic()) {
      Mix_PlayMusic(sons.jogando, 1);
    }

    //Copia o mapa para o renderer
    SDL_RenderCopy(renderer, texturas.mapa, NULL, NULL);

    //Renderiza e verfica se o protagonista pegou alguma vida
    verificar_vida_dropada();

    //Renderiza a interface (tempo de vida, tempo de jogo e quantidade de pontos)
    renderizar_interface();

    //Renderiza o protagonista
    renderizar_protagonista();

    //Renderiza os tiros
    renderizar_tiros();

    //Move e renderiza os inimigos no mapa
    mover_inimigos_aleatorio();
    renderizar_inimigos_aleatorio();

    //Remove os inimigos mortos
    remover_inimigos_aleatorio();

    //Renderiza o aviso na tela
    if (5 > tempoJogando) {
      renderizar_aviso();
    }
  }

  //Atualiza temporizadores e renderiza eventos de invertvalo
  atualizar_temporizadores();
  renderizar_bolhas_aleatorias();

  //Remove as bolhas aleatorias da tela
  remover_bolhas_aleatorias();
  
  //Exibe na tela o que foi desenhado no renderer
  SDL_RenderPresent(renderer);
}

void renderizar_gameover() {
  Texto * game_over = obter_texto("Game Over!", 0x1100FF);
  SDL_Rect rect;

  rect.w = game_over->largura;
  rect.h = game_over->altura;
  rect.x = (LARGURA / 2) - (rect.w / 2);
  rect.y = (ALTURA / 2) - (rect.h / 2);

  char str_pontos[100];
  sprintf(str_pontos, "Sua pontuacao foi: %lld!", pontos);
  Texto * pontuacao = obter_texto(str_pontos, 0x1100FF);
  SDL_Rect rect2;
  rect2.w = pontuacao->largura;
  rect2.h = pontuacao->altura;
  rect2.x = (LARGURA / 2) - (rect2.w / 2);
  rect2.y = (rect.y + rect.h + 10);

  SDL_RenderCopy(renderer, texturas.derrota, NULL, NULL);
  SDL_RenderCopy(renderer, game_over->textura, NULL, &rect);
  SDL_RenderCopy(renderer, pontuacao->textura, NULL, &rect2);

  SDL_DestroyTexture(game_over->textura);
  SDL_DestroyTexture(pontuacao->textura);
}

void renderizar_interface() {
  //Renderiza as vidas do protagonista
  for (int i = 0; i < protagonista.vida; i++) {
    SDL_Rect srcRect = { 0, 0, 254, 254 };
    SDL_Rect rect = { 5 + (55 * i), 5, 50, 50 };
    SDL_RenderCopyEx(renderer, texturas.vida, &srcRect, &rect, 0, NULL, 0);
  }
  
  //Rnderiza a energia do protagonista
  SDL_Rect rect_e = { 10, 70, protagonista.energia, 5};
  
  RGB cor = obter_cor(0xFF0000);
  SDL_SetRenderDrawColor(renderer, cor.r, cor.g, cor.b, 0);
  SDL_RenderFillRect(renderer, &rect_e);


  //Renderiza a pontuação
  char str_pontos[100];
  sprintf(str_pontos, "%lld", pontos);
  
  Texto * texto = obter_texto(str_pontos, 0xFF0000);
  SDL_Rect rect;

  rect.w=texto->largura;
  rect.h=texto->altura;
  rect.x = LARGURA - (rect.w + 10);
  rect.y = 0;

  SDL_RenderCopy(renderer, texto->textura, NULL, &rect);
  SDL_DestroyTexture(texto->textura);

}

void renderizar_aviso() {
  Texto * texto1 = obter_texto("Pule para filtrar a agua", 0xFFFF00);
  Texto * texto2 = obter_texto("e nao perder vidas!", 0xFFFF00);

  SDL_Rect rect1;
  rect1.w = texto1->largura;
  rect1.h = texto1->altura;
  rect1.x = (LARGURA / 2) - (rect1.w / 2);
  rect1.y = (ALTURA / 3) - (rect1.h / 2);

  SDL_Rect rect2;
  rect2.w = texto2->largura;
  rect2.h = texto2->altura;
  rect2.x = (LARGURA / 2) - (rect2.w / 2);
  rect2.y = (ALTURA / 2) - (rect2.h / 2);

  SDL_RenderCopy(renderer, texto1->textura, NULL, &rect1);
  SDL_RenderCopy(renderer, texto2->textura, NULL, &rect2);

  SDL_DestroyTexture(texto1->textura);
  SDL_DestroyTexture(texto2->textura);
}

void renderizar_protagonista() {
  //Copia o personagem protagonista para o renderer de acordo com o sprite atual
  if (protagonista.visivel) {
    //adiciona a arma do protagonista
    SDL_Rect rect2 = { (protagonista.x + (protagonista.viradoEsquerda ? 0 : (protagonista.largura * 0.65))) - ((protagonista.atirando && (tempoGlobal % 2)) ? 2 : 0), protagonista.y + (protagonista.altura / 2.18) - ((protagonista.pulando) ? 12 : (((protagonista.sprite % 3) && protagonista.andando) ? 3 : 0)), 50, 25 };
    SDL_RenderCopyEx(renderer, texturas.arma, NULL, &rect2, 0, NULL, protagonista.viradoEsquerda);

    SDL_Rect srcRect = { 43*protagonista.sprite, protagonista.sprite_linha, 43, 40 };
    SDL_Rect rect = { protagonista.x, protagonista.y, protagonista.largura, protagonista.altura };
    SDL_RenderCopyEx(renderer, texturas.bob, &srcRect, &rect, 0, NULL, protagonista.viradoEsquerda);
  }
}

void verificar_fisica() {
  protagonista.y += protagonista.dy;
  protagonista.dy += 0.5;

  //Verifica objetos físicos no mapa
  for (int i = 0; i < LIMITE_DE_OBJETOS_NO_MAPA; i++) if (mapa.objetos[i]) {

    if (mapa.objetos[i]->solido && (protagonista.y > (mapa.objetos[i]->y + mapa.objetos[i]->altura - protagonista.altura))) {
      protagonista.y = (mapa.objetos[i]->y + mapa.objetos[i]->altura) - protagonista.altura;
      protagonista.dy = 0;
      protagonista.pulando = false;
      break;
    }
    
  }
  
  //Verifica se algum inimigo encostou no protagonista
  if (!protagonista.imunidade && protagonista.vivo) {
    for (int i = 0; i < MAX_INIMIGOS; i++) if (inimigos[i]) {
      if (inimigos[i]->vivo) {
        if ((inimigos[i]->x + (inimigos[i]->largura * 0.5)) > protagonista.x && inimigos[i]->x < (protagonista.x + (protagonista.largura * 0.8)) && (inimigos[i]->y + inimigos[i]->altura) > protagonista.y && inimigos[i]->y < (protagonista.y + protagonista.altura)) {
          inimigos[i]->vivo = false;
          remover_vida_protagonista(true);
          Mix_PlayChannel(-1, sons.choque, 0);
          break;
        }
      }
    }
  }

  //Verifica todos os tiros
  for (int i = 0; i < MAX_TIROS; i++) if(tiros[i]) {
    tiros[i]->x += tiros[i]->dx;
    bool acertou = false;
    
    //Verifica se o tiro acertou o inimigo
    for (int j = 0; j < MAX_INIMIGOS; j++) if (inimigos[j]) {
      if (inimigos[j]->vivo) {
        if (tiros[i]->x > inimigos[j]->x && tiros[i]->x < inimigos[j]->x+inimigos[j]->largura && tiros[i]->y > inimigos[j]->y && tiros[i]->y < inimigos[j]->y+inimigos[j]->altura) {
          inimigos[j]->vivo = false;
          if (protagonista.vivo) {
            pontos++;

            if (protagonista.vida < MAX_VIDAS) {
              //Dropa uma vida do monstro
              if ((!(pontos % VIDA_DROPADA_POR_INTERVALO_DE_PONTOS)) && (!vida_dropada.visivel) && (VIDA_DROPADA_CHANCE >= obter_numero_aleatorio(1, 100))) {
                vida_dropada.altura = vida_dropada.largura = 40;
                vida_dropada.x = inimigos[j]->x + (inimigos[j]->largura / 2) - (vida_dropada.largura / 2);
                vida_dropada.y = inimigos[j]->y;
                vida_dropada.visivel = true;
              }
            }
          }
          acertou = true;
        }
      }
    }

    //Remove os tiros que estão fora do mapa ou que acertou um inimigo
    if (tiros[i]->x < 0 || tiros[i]->x > LARGURA || acertou) {
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
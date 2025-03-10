#include <stdio.h>
#include <stdlib.h>

#include "raylib/include/raylib.h"

// Definição dos elementos

typedef enum {
    TIPO_ELEMENTO_CONTAINER,
    TIPO_ELEMENTO_BOTAO,
} TIPO_ELEMENTO;

typedef enum {
    FLEX_DIR_ROW,
    FLEX_DIR_COL,
} FLEX_DIR;

typedef enum {
    TIPO_WIDTH_ABSOLUTO,
    TIPO_WIDTH_PORCENTAGEM,
} TIPO_WIDTH; 

typedef enum {
    TIPO_HEIGHT_ABSOLUTO,
    TIPO_HEIGHT_PORCENTAGEM,
} TIPO_HEIGHT;

typedef struct elem Elemento;

// Definição da lista de elementos

typedef struct {
    Elemento** items;
    int qtd;
    int cap;
} Lista_Elementos;

typedef struct {
    char* nome;
} Elemento_Container;

typedef struct {
    char* texto;
} Elemento_Botao;

struct elem {
    TIPO_ELEMENTO tipo;
    Elemento* pai;
    Lista_Elementos filhos;
    Color background_color;
    FLEX_DIR flex_dir;
    TIPO_WIDTH tipo_width;
    TIPO_HEIGHT tipo_height;
    float porcentagem_widht, porcentegem_height;
    float width, height;
    float posX, posY;
    union {
        Elemento_Container* container;
        Elemento_Botao* botao;
    } conteudo;
};

// Funções relacionadas aos elementos

#define CAPACIDADE_INICIAL_LISTA_ELEMENTOS 8

void adicionar_filho(Elemento* pai, Elemento* filho) {
    if (pai->filhos.cap == 0) {
        pai->filhos.items = malloc(sizeof(Elemento*) * CAPACIDADE_INICIAL_LISTA_ELEMENTOS);
        pai->filhos.cap = CAPACIDADE_INICIAL_LISTA_ELEMENTOS;
    } else if (pai->filhos.qtd + 1 >= pai->filhos.cap) {
        pai->filhos.items = realloc(pai->filhos.items, pai->filhos.cap * 2);
        pai->filhos.cap *= 2;
    }
    pai->filhos.items[pai->filhos.qtd] = filho;
    pai->filhos.qtd++;
    filho->pai = pai;
}

Elemento* novo_elemento() {
    Elemento* e = malloc(sizeof(Elemento));
    e->pai = NULL;
    e->filhos = (Lista_Elementos){.items = NULL, .qtd = 0, .cap = 0};
    e->background_color = WHITE;
    e->flex_dir = FLEX_DIR_ROW;
    e->tipo_width = TIPO_WIDTH_ABSOLUTO;
    e->tipo_height = TIPO_HEIGHT_ABSOLUTO;
    e->width = -1;
    e->height = -1;
    e->posX = 0;
    e->posY = 0;
    return e;
}

Elemento* novo_container(char* nome) {
    Elemento* e = novo_elemento();
    e->tipo = TIPO_ELEMENTO_CONTAINER;
    e->conteudo.container = malloc(sizeof(Elemento_Container));
    e->conteudo.container->nome = nome;
    return e;
}

Elemento* novo_botao(char* texto) {
    Elemento* e = novo_elemento();
    e->tipo = TIPO_ELEMENTO_BOTAO;
    e->conteudo.botao = malloc(sizeof(Elemento_Botao));
    e->conteudo.botao->texto = texto;
    e->width = MeasureText(texto, 20);
    e->height = 25;
    return e;
}

// calculos e renderizacao

typedef struct {
    int width, height;
} Tamanho_Elemento;

void calcular_tamanho_elemento(Elemento* el) {
    Tamanho_Elemento t = {0};
    if (el->flex_dir == FLEX_DIR_COL) {
        if (el->width <= 0) {
            int maior = 0;
            for (int i = 0; i < el->filhos.qtd; i++) {
                // Tamanho_Elemento atual = calcular_tamanho_elemento(el->filhos.items[i]);
                if (el->filhos.items[i]->width < maior) maior = el->filhos.items[i]->width;
            }
        } else {
            t.width = el->width;
        }
        if (el->height <= 0) {
            for (int i = 0; i < el->filhos.qtd; i++) {
                t.height += calcular_tamanho_elemento(el->filhos.items[i]).height;
            }
        } else {
            t.height = el->height;
        }
    } else {
        if (el->width <= 0) {
            for (int i = 0; i < el->filhos.qtd; i++) {
                t.height += calcular_tamanho_elemento(el->filhos.items[i]).width;
            }
        } else {
            t.width = el->width;
        }
        if (el->height <= 0) {
            int maior = 0;
            for (int i = 0; i < el->filhos.qtd; i++) {
                Tamanho_Elemento atual = calcular_tamanho_elemento(el->filhos.items[i]);
                if (atual.height < maior) maior = atual.height;
            }
        } else {
            t.height = el->height;
        }
    }
    el->width = t.width;
    el->height = t.height;
    // return t;
}

void calcular_posicoes(Elemento* el, int x, int y, int width, int height) {
    switch (el->tipo) {
    case TIPO_ELEMENTO_CONTAINER:
        
        break;
    case TIPO_ELEMENTO_BOTAO:
        break;
    default:
        printf("nao cobriu todos os casos em calcular_posicoes\n");
        exit(1);
    }
}

void atravessar_rec(Elemento* el, int x, int y, int width, int height) {
    switch (el->tipo) {
    case TIPO_ELEMENTO_CONTAINER:
        DrawRectangle(x, y, width, height, el->background_color);
        break;
    case TIPO_ELEMENTO_BOTAO:
        DrawRectangle(x, y, 100, 20, el->background_color);
        DrawText(el->conteudo.botao->texto, x, y, 20, BLACK);
        break;
    default:
        printf("nao cobriu todos os casos em atravessar_rec\n");
        exit(1);
    }
    int ultx = 0;
    int ulty = 0;
    for (int i = 0; i < el->filhos.qtd; i++) {
        atravessar_rec(el->filhos.items[i], ultx, ulty, width / el->filhos.qtd, height);
        ultx += width / el->filhos.qtd;
        ulty += 0;
    }
}

// debug

void printar_elementos(Elemento* raiz, int nivel) {
    for (int i = 0; i < nivel; i++) {
        printf(" ");
    }
    switch (raiz->tipo) {
    case TIPO_ELEMENTO_CONTAINER:
        printf("c: %s\n", raiz->conteudo.container->nome);
        break;
    case TIPO_ELEMENTO_BOTAO:
        printf("b: %s\n", raiz->conteudo.botao->texto);
        break;
    default:
        printf("nao cobriu todos os casos no printar elementos\n");
        exit(1);
    }
    for (int i = 0; i < raiz->filhos.qtd; i++) {
        printar_elementos(raiz->filhos.items[i], nivel + 1);
    }
}

int main() {
    Elemento* c1 = novo_container("c1");
    c1->tipo_width = TIPO_WIDTH_PORCENTAGEM;
    c1->porcentagem_widht = 100;
    c1->height = 720;
    Elemento* c2 = novo_container("c2");
    c2->tipo_width = TIPO_WIDTH_PORCENTAGEM;
    c2->porcentagem_widht = 50;
    c2->flex_dir = FLEX_DIR_COL;
    Elemento* c3 = novo_container("c3");
    c3->tipo_width = TIPO_WIDTH_PORCENTAGEM;
    c3->porcentagem_widht = 50;
    c3->flex_dir = FLEX_DIR_COL;
    adicionar_filho(c1, c2);
    adicionar_filho(c1, c3);
    Elemento* b1 = novo_botao("b1");
    Elemento* b2 = novo_botao("b2");
    adicionar_filho(c2, b1);
    adicionar_filho(c2, b2);
    Elemento* b3 = novo_botao("b3");
    Elemento* b4 = novo_botao("b4");
    adicionar_filho(c3, b3);
    adicionar_filho(c3, b4);
    printar_elementos(c1, 0);

    c2->background_color = GREEN;
    c3->background_color = RED;
    
    int w = 1280;
    int h = 720;
    calcular_posicoes(c1, 0, 0, w, -1);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(w, h, "tela elem");
    SetTargetFPS(144);

    while (!WindowShouldClose()) {

        if (IsWindowResized()) {
            w = GetScreenWidth();
            h = GetScreenHeight();
            calcular_posicoes(c1, 0, 0, w, -1);
        }

        BeginDrawing();

        ClearBackground(BLACK);
        // DrawCircle(100, 100, 50, RED);
        atravessar_rec(c1, 0, 0, w, h);

        DrawFPS(500, 500);

        EndDrawing();
    }

    CloseWindow();
}
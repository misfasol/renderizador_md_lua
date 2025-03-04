#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "raylib/include/raylib.h"

#include "./lua_src/lua.h"
#include "./lua_src/lualib.h"
#include "./lua_src/lauxlib.h"

#include "elem.c"

#define WIDTH 1280
#define HEIGHT 720

char* ler_arquivo(char* nome_arq) {
    FILE* f = fopen(nome_arq, "rb");
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buffer = malloc(length + 1);
    if (buffer) {
        fread(buffer, 1, length, f);
        buffer[length] = 0;
    }
    fclose(f);
    return buffer;
}

void separar_conteudo(char* conteudo, char** data, char** code) {
    if (strncmp(conteudo, "-- data", 7) != 0) {
        printf("ERROR: nao tem a secao data ou ela nao esta no comeco do arquivo\n");
        exit(1);
    }
    conteudo += 7;
    int tam = 0;
    while (strncmp(conteudo + tam, "-- code", 7) != 0) {
        tam += 1;
    }
    *(conteudo + tam) = 0;
    *data = conteudo;
    *code = conteudo + tam + 7;
}

int separar_linhas(char* conteudo, char*** linhas) {
    char* ponteiro = conteudo;
    char* final = ponteiro + strlen(ponteiro);
    int qtd = 0;
    while (ponteiro < final) {
        if (*ponteiro == '\r') {
            *ponteiro = 0;
        }
        if (*ponteiro == '\n') {
            *ponteiro = 0;
            qtd++;
        }
        ponteiro++;
    }
    *linhas = malloc(qtd * sizeof(char*));
    ponteiro = conteudo;
    bool achou_zero = true;
    int contador = 0;
    while (ponteiro < final) {
        if (achou_zero && *ponteiro != 0) {
            (*linhas)[contador] = ponteiro;
            contador++;
        }
        if (*ponteiro == 0) {
            achou_zero = true;
        } else {
            achou_zero = false;
        }
        ponteiro++;
    }
    *linhas = realloc(*linhas, contador * sizeof(char*));
    return contador;
}

typedef enum {
    LINHA_TEXTO,
    LINHA_VAR,
    LINHA_BOTAO,
} TIPO_LINHA;

typedef struct {
    char* texto;
} Linha_Texto;

typedef struct {
    char* nome_var;
    const char* valor_var;
} Linha_Var;

typedef struct {
    char* nome_func;
    char* valor_texto;
    float width;
    float roundness;
    int segments;
    Color cor;
} Linha_Botao;

typedef struct {
    TIPO_LINHA tipo;
    // Linhas_Union conteudo;
    union {
        Linha_Botao* l_botao;
        Linha_Var* l_var;
        Linha_Texto* l_texto;
    } conteudo;
} Linha;

Linha* renderizar_conteudo(lua_State* L, char** linhas_str, int qtd, Font fonte) {
    Linha* linhas = malloc(sizeof(Linha) * qtd);
    for (int i = 0; i < qtd; i++) {
        char* atual = linhas_str[i];
        if (atual[0] == '@') {
            Linha_Var* l = malloc(sizeof(Linha_Var));
            l->nome_var = atual + 1;
            lua_getglobal(L, l->nome_var);
            l->valor_var = lua_tostring(L, -1);
            linhas[i].tipo = LINHA_VAR;
            linhas[i].conteudo.l_var = l;
        } else if (atual[0] == '>') {
            char* nome_func = atual + 1;
            char* p = atual + 1;
            while (*p != ' ') {
                p++;
            }
            *p = 0;
            Linha_Botao* l = malloc(sizeof(Linha_Botao));
            l->nome_func = nome_func;
            l->cor = GREEN;
            l->roundness = 5;
            l->segments = 5;
            l->valor_texto = p + 1;
            l->width = MeasureTextEx(fonte, l->valor_texto, 35, 2).x;
            linhas[i].tipo = LINHA_BOTAO;
            linhas[i].conteudo.l_botao = l;
        } else {
            Linha_Texto* l = malloc(sizeof(Linha_Texto));
            l->texto = atual;
            linhas[i].tipo = LINHA_TEXTO;
            linhas[i].conteudo.l_texto = l;
        }
    }
    return linhas;
}

void rerenderizar(lua_State* L, Linha* linhas, int qtd) {
    Linha atual;
    for (int i = 0; i < qtd; i++) {
        atual = linhas[i];
        switch (atual.tipo) {
            case LINHA_VAR:
                lua_getglobal(L, atual.conteudo.l_var->nome_var);
                atual.conteudo.l_var->valor_var = lua_tostring(L, -1);
                break;
            default:
                break;
        }
    }
}

int main() {

    // Elemento c1 = novo_container("c1");
    // Elemento b1 = novo_botao("b1");
    // Elemento b2 = novo_botao("b2");
    // adicionar_filho(&c1, &b1);
    // adicionar_filho(&c1, &b2);

    // return 0;

    // separar conteudo
    char* str_conteudo = ler_arquivo("examples/ter.hl");
    char* data;
    char* code;
    separar_conteudo(str_conteudo, &data, &code);
    char** linhas_str = NULL;
    int qtd_linhas = separar_linhas(data, &linhas_str);

    // inicializar lua e executar parte "code"
    printf("%s\n", code);
    lua_State* L = luaL_newstate();
    int erro;
    erro = luaL_loadstring(L, code) || lua_pcall(L, 0, 0, 0);
    if (erro) {
        printf("%s", lua_tostring(L, -1));
        lua_pop(L, 1);
        return 1;
    }
    
    InitWindow(WIDTH, HEIGHT, "tela");
    SetTargetFPS(60);
    
    Font fonte = LoadFontEx("fontes/InclusiveSans-Regular.ttf", 100, 0, 0);
    Color cor_fundo = {10, 10, 10, 255};
    
    // renderiza
    Linha* linhas = renderizar_conteudo(L, linhas_str, qtd_linhas, fonte);

    while (!WindowShouldClose()) {

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            bool aconteceu = false;
            for (int i = 0; i < qtd_linhas; i++) {
                switch (linhas[i].tipo) {
                    case LINHA_BOTAO:
                        Linha_Botao l_b = *(linhas[i].conteudo.l_botao);
                        if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){.x = 10, .y = 10 + i * 35, .height = 40, .width = l_b.width + 20})) {
                            if (luaL_loadstring(L, l_b.nome_func) || lua_pcall(L, 0, 0, 0)) {
                                printf("%s", lua_tostring(L, -1));
                                lua_pop(L, 1);
                                return 1;
                            }
                            aconteceu = true;
                        }
                        break;
                    default:
                        break;
                }
            }
            if (aconteceu) rerenderizar(L ,linhas, qtd_linhas);
        }

        BeginDrawing();
        ClearBackground(cor_fundo);
        Linha atual;
        Linha_Texto l_texto;
        Linha_Var l_var;
        Linha_Botao l_botao;
        for (int i = 0; i < qtd_linhas; i++) {
            // printf("a");
            atual = linhas[i];
            switch (atual.tipo) {
                case LINHA_TEXTO:
                    l_texto = *atual.conteudo.l_texto;
                    DrawTextEx(fonte, l_texto.texto, (Vector2){10, 10 + i * 35}, 35, 2, WHITE);
                    break;
                case LINHA_VAR:
                    l_var = *atual.conteudo.l_var;
                    DrawTextEx(fonte, l_var.valor_var, (Vector2){10, 10 + i * 35}, 35, 2, WHITE);
                    break;
                case LINHA_BOTAO:
                    l_botao = *atual.conteudo.l_botao;
                    DrawRectangleRounded((Rectangle){.x = 10, .y = 10 + i * 35, .height = 40, .width = l_botao.width + 20}, l_botao.roundness, l_botao.segments, l_botao.cor);
                    DrawTextEx(fonte, l_botao.valor_texto, (Vector2){.x = 20, .y = 10 + i * 35}, 35, 2, WHITE);
                    break;
                default:
                    break;
            }
        }
        EndDrawing();
    }

    UnloadFont(fonte);
    CloseWindow();
}

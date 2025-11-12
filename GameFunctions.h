#ifndef GAMEFUNCTIONS_H
#define GAMEFUNCTIONS_H

#include "GameTypes.h"
#include "Utils.h"

// Inicializa SDL y crea la ventana/renderer
bool init(SDL_Window* &window, SDL_Renderer* &renderer, TTF_Font* &font);

// Función dedicada exclusivamente a posicionar los objetos (generar el mapa)
void generateLevel(Entity* player, Entity* enemy, Obstacle obstacles[], Entity* hole);

// Carga texturas e inicializa la posición de las entidades
void loadMedia(SDL_Renderer* renderer, Entity* player, Entity* enemy, Obstacle obstacles[], Entity* hole, SDL_Texture* &background, SDL_Texture* &obstacleTexture);

// Maneja toda la entrada del usuario (teclado, mouse)
void handleEvents(SDL_Event &event, GameState &state, Entity* player);

// Actualiza la lógica del juego (físicas, IA)
void update(SDL_Window* window, GameState &state, Entity* player, Entity* enemy, Obstacle obstacles[], Entity* hole);

// Dibuja todo en la pantalla
void render(SDL_Renderer* renderer, const GameState &state, const Entity* player, const Entity* enemy, const Obstacle obstacles[], const Entity* hole, SDL_Texture* background);

// Libera todos los recursos
void cleanup(SDL_Window* window, SDL_Renderer* renderer, Entity* player, Entity* enemy, Obstacle obstacles[], Entity* hole, SDL_Texture* background, SDL_Texture* &obstacleTexture);

#endif
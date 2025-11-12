#ifndef GAMEMAINMENU_H
#define GAMEMAINMENU_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "GameTypes.h"

// Solo maneja eventos y render, no inicializa SDL
void handleMainMenuEvents(SDL_Event &event, GameState &state);
void renderMainMenu(SDL_Renderer* renderer, TTF_Font* font);

#endif

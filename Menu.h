#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#include <string>

//Enumeración que representa las distintas opciones del menú
enum MenuOption {
    MENU_NONE,
    MENU_PLAY,
    MENU_OPTIONS,
    MENU_QUIT
};

// Inicializa el menú y devuelve la opción elegida
MenuOption runMenu(SDL_Renderer* renderer);

#endif

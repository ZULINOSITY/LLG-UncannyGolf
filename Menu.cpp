#include "Menu.h"
#include "Utils.h"
#include <iostream>
#include <SDL2/SDL_image.h>

//Inicia el menu
MenuOption runMenu(SDL_Renderer* renderer) {
    bool running = true;
    SDL_Event e;

    // Cargar imagen de fondo
    SDL_Texture* background = IMG_LoadTexture(renderer, "assets/menu_background.png");
    if (!background) {
        std::cerr << "No se pudo cargar el fondo del menú: " << IMG_GetError() << std::endl;
    }

    // Colores base
    SDL_Color bgColor = {30, 30, 30, 255};
    SDL_Color btnColor = {100, 100, 100, 255};
    SDL_Color hoverColor = {180, 180, 180, 255};

    // Posiciones y tamaños de botones
    SDL_Rect playButton = {250, 200, 300, 80};
    SDL_Rect exitButton = {250, 320, 300, 80};

    // Cargar texto como imágenes
    SDL_Texture* textPlay = loadTexture("assets/Start.png", renderer);
    SDL_Texture* textOptions = loadTexture("assets/Salir.png", renderer);

    // Rectángulos para el texto
    SDL_Rect textPlayRect = {playButton.x + 80, playButton.y + 20, 140, 40};
    SDL_Rect textExitRect = {exitButton.x + 60, exitButton.y + 20, 180, 40};

    int mouseX = 0, mouseY = 0;
    MenuOption selectedExit = MENU_NONE;

    //Evento de mouse
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
                selectedExit = MENU_QUIT;
            } else if (e.type == SDL_MOUSEMOTION) {
                SDL_GetMouseState(&mouseX, &mouseY);
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    SDL_Point mousePoint = {mouseX, mouseY};

                    if (SDL_PointInRect(&mousePoint, &playButton)) {
                        selectedExit = MENU_PLAY;
                        running = false;
                    } else if (SDL_PointInRect(&mousePoint, &exitButton)) {
                        selectedExit = MENU_OPTIONS;
                        running = false;
                    }
                }
            }
        }

        //Poder presionarlo con el mouse
        SDL_Point mousePoint = {mouseX, mouseY};

        // Botón JUGAR 
        if (SDL_PointInRect(&mousePoint, &playButton))
            SDL_SetRenderDrawColor(renderer, hoverColor.r, hoverColor.g, hoverColor.b, hoverColor.a);
        else
            SDL_SetRenderDrawColor(renderer, btnColor.r, btnColor.g, btnColor.b, btnColor.a);
        SDL_RenderFillRect(renderer, &playButton);

        if (textPlay)
            SDL_RenderCopy(renderer, textPlay, nullptr, &textPlayRect);

        // Botón de salida
        if (SDL_PointInRect(&mousePoint, &exitButton))
            SDL_SetRenderDrawColor(renderer, hoverColor.r, hoverColor.g, hoverColor.b, hoverColor.a);
        else
            SDL_SetRenderDrawColor(renderer, btnColor.r, btnColor.g, btnColor.b, btnColor.a);
        SDL_RenderFillRect(renderer, &exitButton);

        if (textOptions)
            SDL_RenderCopy(renderer, textOptions, nullptr, &textExitRect);

        // Mostrar todo
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    // Liberar recursos
    if (textPlay) SDL_DestroyTexture(textPlay);
    if (textOptions) SDL_DestroyTexture(textOptions);

    //Salir con el boton de salida (XD)
    return selectedExit;
}

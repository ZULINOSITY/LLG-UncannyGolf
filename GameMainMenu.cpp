#include "GameMainMenu.h"
#include <iostream>

using namespace std;

void handleMainMenuEvents(SDL_Event &event, GameState &state)
{
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            state.isRunning = false;
        }
        else if (event.type == SDL_KEYDOWN)
        {
            switch (event.key.keysym.sym)
            {
            case SDLK_RETURN: // Enter para empezar
                state.currentScene = Scene::GAMEPLAY;
                break;
            case SDLK_ESCAPE: // Escape para salir
                state.isRunning = false;
                break;
            }
        }
    }
}

void renderMainMenu(SDL_Renderer *renderer, TTF_Font *font)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Color white = {255, 255, 255, 255};

    // --- Título ---
    SDL_Surface* titleSurface = TTF_RenderText_Solid(font, "Uncanny Cat Golf", white);
    SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface);

    SDL_Rect titleRect = {300, 200, titleSurface->w, titleSurface->h};
    SDL_RenderCopy(renderer, titleTexture, nullptr, &titleRect);

    SDL_FreeSurface(titleSurface);
    SDL_DestroyTexture(titleTexture);

    // --- Texto inferior ---
    SDL_Surface* pressSurface = TTF_RenderText_Solid(font, "Presiona ENTER para iniciar", white);
    SDL_Texture* pressTexture = SDL_CreateTextureFromSurface(renderer, pressSurface);

    SDL_Rect pressRect = {250, 350, pressSurface->w, pressSurface->h};
    SDL_RenderCopy(renderer, pressTexture, nullptr, &pressRect);

    SDL_FreeSurface(pressSurface);
    SDL_DestroyTexture(pressTexture);

    SDL_RenderPresent(renderer);
}

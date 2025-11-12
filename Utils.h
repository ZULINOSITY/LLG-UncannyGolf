#ifndef UTILS_H
#define UTILS_H

#include <SDL2/SDL.h>
#include <string>

// Prototipos de funciones de ayuda

// Función de ayuda para comprobar colisiones AABB
bool checkCollision(float x1, float y1, int w1, int h1, float x2, float y2, int w2, int h2);

// SDL_IMAGE
// Esta función carga una imagen y la convierte en textura
SDL_Texture* loadTexture(std::string path, SDL_Renderer* renderer);

#endif
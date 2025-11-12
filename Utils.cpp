#include "Utils.h"
#include <SDL2/SDL_image.h>
#include <iostream>

using namespace std; // Pa no escribir std:: a cada rato

// Comprueba si dos rectángulos (cajas de colisión) se superponen.
bool checkCollision(float x1, float y1, int w1, int h1, float x2, float y2, int w2, int h2) {
    // Colisión en eje X
    bool collisionX = x1 + w1 > x2 && // El borde derecho de 1 supera el borde izquierdo de 2
                      x1 < x2 + w2;  // El borde izquierdo de 1 está antes del borde derecho de 2

    // Colisión en eje Y
    bool collisionY = y1 + h1 > y2 && // El borde inferior de 1 supera el borde superior de 2
                      y1 < y2 + h2;  // El borde superior de 1 está antes del borde inferior de 2

    // Hay colisión solo si se superponen en AMBOS ejes
    return collisionX && collisionY;
}

SDL_Texture* loadTexture(string path, SDL_Renderer* renderer) {
    SDL_Texture* newTexture = nullptr;

    // Carga la imagen desde un archivo a la memoria (CPU)
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (loadedSurface == nullptr) {
        cerr << "Error al cargar imagen " << path << ": " << IMG_GetError() << endl;
    } else {
        // Convierte la imagen (Surface) a una textura (Texture)
        // Esto la manda a la tarjeta gráfica (GPU), que es más rápido
        newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        if (newTexture == nullptr) {
            cerr << "Error al crear textura desde " << path << ": " << SDL_GetError() << endl;
        }

        // Ya no necesitamos la imagen en la CPU, la liberamos
        SDL_FreeSurface(loadedSurface);
    }

    return newTexture;
}
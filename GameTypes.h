#ifndef GAMETYPES_H
#define GAMETYPES_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// --- Constantes Globales ---

// Declaramos estas variables como extern para que todos sepan que existen,
// pero se definen realmente en GameFunctions.cpp
extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;

const float POWER_MULTIPLIER = 0.1f;   // Multiplicador de fuerza (qué tan fuerte es el golpe)
const float FRICTION = 0.985f;         // Fricción (ej. 0.985 = 1.5% de freno por frame)
const float ENEMY_SPEED = 0.5f;        // Velocidad de persecución del enemigo

const int NUM_OBSTACLES = 40;          // Cantidad de obstáculos

// --- Estructuras de Datos ---

// Enum para las escenas del juego
enum class Scene
{
    MAIN_MENU,
    GAMEPLAY
};


// Renombramos Player a Entity, ya que ahora tendremos dos
struct Entity {
    float x, y;           // Estas variables son para la posición actual papu
    float vx, vy;         // Estas variables equivalen a la velocidad
    int w, h;             // Estas variables son el ancho y alto (del objeto)
    SDL_Texture* texture; // Variable para guardar nuestra imagen
};

// Estructura para los obstáculos
// Ahora incluye la textura
struct Obstacle {
    float x, y;
    int w, h;
    SDL_Texture* texture; // Textura para el obstáculo
};

// Un struct para mantener el estado del juego organizado
struct GameState {
    bool isRunning = true;      // El game loop principal
    int levelCount = 1;         // Contador de niveles superados
    
    Scene currentScene = Scene::MAIN_MENU; // Escena actual del juego
    
    // Aqui empiezan las mecanicas del golf, y esta es una puta mierda asi que presta atención porque es dificil de explicar
    bool isAiming = false;      // Aca declaramos la variable que utilizaremos para saber si basicamente el jugador esta presionando el click para apuntar

    // En el momento que haces click entran a la mesa las siguientes variables
    int aimStartX = 0;          // Posición X de donde empezó el clic
    int aimStartY = 0;          // Posición Y de donde empezó el clic
    int aimCurrentX = 0;        // Posición X actual del mouse
    int aimCurrentY = 0;        // Posición Y actual del mouse
};

#endif
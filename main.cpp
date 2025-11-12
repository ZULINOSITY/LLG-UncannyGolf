#include <iostream>
#include "GameTypes.h"
#include "GameFunctions.h"
#include "GameMainMenu.h"
#include "AudioHandler.h"

using namespace std;

// --- Función Principal (main) ---
// Ahora main es solo el "director" que llama a las otras funciones.
int main(int argc, char *argv[])
{
    // Variables principales de SDL
    SDL_Window *g_window = nullptr;
    SDL_Renderer *g_renderer = nullptr;

    // Texturas y Entidades
    SDL_Texture *g_backgroundTexture = nullptr;
    SDL_Texture *g_obstacleBaseTexture = nullptr;

    // Fuentes de texto
    TTF_Font *g_font = nullptr;

    Entity *g_player = new Entity();
    Entity *g_enemy = new Entity();
    Obstacle *g_obstacles = new Obstacle[NUM_OBSTACLES];
    Entity *g_hole = new Entity();

    // Estado del juego
    GameState g_gameState;
    SDL_Event g_event;

    // Inicializar
    if (!init(g_window, g_renderer, g_font))
    {
        cerr << "Fallo al inicializar." << endl;
        return 1;
    }

    // Cargar Menú Principal

    // Cargar Media
    loadMedia(g_renderer, g_player, g_enemy, g_obstacles, g_hole, g_backgroundTexture, g_obstacleBaseTexture, g_font);

    cout << "Motor iniciado. Haz clic y arrastra para disparar." << endl;
    cout << "Llega al hoyo negro para ganar. Evita al Uncanny Cat." << endl;
    
    AudioHandler::getInstance().playSound("dep");

    // Game Loop
    while (g_gameState.isRunning)
    {

        switch (g_gameState.currentScene)
        {
        case Scene::MAIN_MENU:
            // Logica del Menu Principal
            handleMainMenuEvents(g_event, g_gameState);
            renderMainMenu(g_renderer, g_font);
            break;

        case Scene::GAMEPLAY:
            // Lógica del juego

            // Manejar Eventos
            handleEvents(g_event, g_gameState, g_player);
            // Actualizar Lógica
            update(g_window, g_gameState, g_player, g_enemy, g_obstacles, g_hole);
            render(g_renderer, g_gameState, g_player, g_enemy, g_obstacles, g_hole, g_backgroundTexture, g_font);
            break;

        default:
            break;
        }

    } // Fin del Game Loop

    // Limpiar y Salir
    cleanup(g_window, g_renderer, g_player, g_enemy, g_obstacles, g_hole, g_backgroundTexture, g_obstacleBaseTexture);

    return 0; // El programa termina
}
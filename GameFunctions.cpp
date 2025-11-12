#include "GameFunctions.h"
#include "AudioHandler.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <cmath>
#include <cstdlib> //Para rand() y srand()
#include <ctime>   //Para time() (semilla de rand)

using namespace std;

// Definimos las variables globales aquí
int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 600;

bool init(SDL_Window* &window, SDL_Renderer* &renderer, TTF_Font* &font) {
    //Inicializa el sdl2, la verdad ni puta idea, pero todos los videos lo ponian
    
    // Tenemos que inicializar SDL_VIDEO
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        cerr << "Error al inicializar SDL: " << SDL_GetError() << endl;
        return false;
    }

    // Inicializamos SDL_image (para PNGs, JPGs, etc.)
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG; // Añadimos JPG por si acaso
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        cerr << "Error al inicializar SDL_image: " << IMG_GetError() << endl;
        SDL_Quit();
        return false;
    }

    // inicializamos TTF
    if (TTF_Init() != 0) {
        cerr << "Error inicializando SDL_ttf: " << TTF_GetError() << endl;
        return false;
    }
    // Inicializa SDL_Audio (Para Wav´s)
    // AudioHandler& audio = AudioHandler::getInstance();
    // if (!audio.initialize()){
    //     cerr << "Error al Inicializa SDL_Audio. Averigua qué fué :)" << endl;
    // }


    //El evento que crea la ventana donde se va a cargar el jueguito
    window = SDL_CreateWindow(
        "Uncanny Cat Golf Nivel 1", 
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN 
    );

    //Si no se abre la ventana por cualquier cosa que se pare el programa
    if (!window) {
        cerr << "Error al crear la ventana: " << SDL_GetError() << endl;
        IMG_Quit();
        SDL_Quit();
        return false;
    }

    // Si la ventana NO está en fullscreen, usamos el tamaño fijo.
    Uint32 flags = SDL_GetWindowFlags(window);
    if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
        SDL_GetWindowSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
        cout << "Modo Fullscreen detectado. Tamaño real: " << SCREEN_WIDTH << "x" << SCREEN_HEIGHT << endl;
    }

    // Este es el "Renderizador". Es el pincel del SDL.
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        cerr << "Error al crear el renderizador: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return false;
    }

    
    AudioHandler::getInstance().setVolume(1.0f);
    cout << "Inicializando sistema de audio..." << endl;
    AudioHandler& audio = AudioHandler::getInstance();
    if (audio.initialize()) {
        cout << "AudioHandler Inicializado." << endl;
    } else {
        cerr << "AudioHandler NO inicializado" << endl;
    }
    
    return true; // Éxito
}

// Mueve todas las cosas de lugar aleatoriamente
void generateLevel(Entity* player, Entity* enemy, Obstacle obstacles[], Entity* hole) {
    // 1. Resetear Jugador al centro
    player->x = (SCREEN_WIDTH / 2.0f) - (player->w / 2.0f);
    player->y = (SCREEN_HEIGHT / 2.0f) - (player->h / 2.0f);
    player->vx = 0.0f; // Detener movimiento
    player->vy = 0.0f;

    // 2. Resetear Enemigo a la esquina
    enemy->x = 0.0f;
    enemy->y = 0.0f;

    // 3. Regenerar obstáculos
    cout << "Regenerando obstaculos y mapa..." << endl;
    
    for (int i = 0; i < NUM_OBSTACLES; ++i) {
        // Usamos el do-while para asegurarnos de que el bloque no caiga encima del jugador
        bool isColliding;
        do {
            isColliding = false; // Reseteamos la bandera
            
            // Generar Posición aleatoria dentro de la pantalla
            obstacles[i].x = static_cast<float>(rand() % (SCREEN_WIDTH - obstacles[i].w));
            obstacles[i].y = static_cast<float>(rand() % (SCREEN_HEIGHT - obstacles[i].h));

            // Verificar colisión con el JUGADOR
            if (checkCollision(obstacles[i].x, obstacles[i].y, obstacles[i].w, obstacles[i].h,
                               player->x, player->y, player->w, player->h)) {
                isColliding = true;
                continue;
            }

            // Verificar colisión con el ENEMIGO
            if (checkCollision(obstacles[i].x, obstacles[i].y, obstacles[i].w, obstacles[i].h,
                               enemy->x, enemy->y, enemy->w, enemy->h)) {
                isColliding = true;
                continue;
            }

            // Verificar con otros OBSTÁCULOS
            for (int j = 0; j < i; ++j) {
                if (checkCollision(obstacles[i].x, obstacles[i].y, obstacles[i].w, obstacles[i].h,
                                   obstacles[j].x, obstacles[j].y, obstacles[j].w, obstacles[j].h)) {
                    isColliding = true;
                    break; 
                }
            }

        } while (isColliding); 
    }

    // 4. Regenerar el hoyo
    bool isCollidingHole;
    do {
        isCollidingHole = false;
        
        // Generar una posición aleatoria
        hole->x = static_cast<float>(rand() % (SCREEN_WIDTH - hole->w));
        hole->y = static_cast<float>(rand() % (SCREEN_HEIGHT - hole->h));

        // Comprobar colisión con el jugador
        if (checkCollision(hole->x, hole->y, hole->w, hole->h, player->x, player->y, player->w, player->h)) {
            isCollidingHole = true;
            continue;
        }

        // Comprobar colisión con el enemigo
        if (checkCollision(hole->x, hole->y, hole->w, hole->h, enemy->x, enemy->y, enemy->w, enemy->h)) {
            isCollidingHole = true;
            continue;
        }

        // Comprobar colisión con todos los obstáculos
        for (int i = 0; i < NUM_OBSTACLES; ++i) {
            if (checkCollision(hole->x, hole->y, hole->w, hole->h, obstacles[i].x, obstacles[i].y, obstacles[i].w, obstacles[i].h)) {
                isCollidingHole = true;
                break;
            }
        }

    } while (isCollidingHole);

    cout << "Nuevo mapa generado." << endl;
}

void loadMedia(SDL_Renderer* renderer, Entity* player, Entity* enemy, Obstacle obstacles[], Entity* hole, SDL_Texture* &background, SDL_Texture* &obstacleTexture, TTF_Font* &font) {
    // Configurar dimensiones
    player->w = 50;
    player->h = 50;

    AudioHandler& audio = AudioHandler::getInstance();
    
    // Cargamos la textura del jugador
    player->texture = loadTexture("Assets/cat.png", renderer);
    if (player->texture == nullptr) {
        cerr << "Error: No se pudo cargar 'cat.png'. El jugador será un cuadrado blanco." << endl;
    }

    // Configurar dimensiones enemigo
    enemy->w = 50;
    enemy->h = 50;

    // Cargamos la textura del enemigo
    enemy->texture = loadTexture("Assets/uncanny_cat.png", renderer);
    if (enemy->texture == nullptr) {
        cerr << "Error: No se pudo cargar 'uncanny_cat.png'. El enemigo será un cuadrado rojo." << endl;
    }
    
    //cargar texturas de fondo
    background = loadTexture("Assets/fondo.png", renderer);
    if (background == nullptr) {
        cerr << "Error: No se pudo cargar 'fondo.png'. El fondo será gris." << endl;
    }

    //Cargar la textura base para los obstáculos
    obstacleTexture = loadTexture("Assets/block.png", renderer); 
    if (obstacleTexture == nullptr) {
        cerr << "Error: No se pudo cargar 'block.png'. Los obstáculos serán cuadrados grises." << endl;
    }

    
    audio.loadWAV("Assets/sfx/ahh_game.wav", "background");
    audio.loadWAV("Assets/sfx/die.wav", "dep");
    audio.loadWAV("Assets/sfx/GG.wav", "GG");
    audio.loadWAV("Assets/sfx/golf.wav", "shoot");
    audio.loadWAV("Assets/sfx/hit.wav", "bounce");

    //Configurar dimensiones obstáculos
    for (int i = 0; i < NUM_OBSTACLES; ++i) {
        obstacles[i].w = 40; // Dimensiones
        obstacles[i].h = 40;
        obstacles[i].texture = obstacleTexture; // Asignar textura
    }

    //Configurar dimensiones hoyo
    hole->w = 50; 
    hole->h = 50;
    hole->texture = loadTexture("Assets/hole.png", renderer); 
    if (hole->texture == nullptr) {
        cerr << "Error: No se pudo cargar 'hole.png'. El hoyo será un cuadrado negro." << endl;
    }

    // Cargar la fuente
    font = TTF_OpenFont("Assets/COMIC.TTF", 28);
    if (font == nullptr) {
        cerr << "Error al cargar la fuente: COMIC.TTF" << TTF_GetError() << endl;
        TTF_Quit();
    }

    //Inicializar la semilla aleatoria
    srand(time(NULL)); 

    // LLAMAR AL GENERADOR DE NIVEL PARA LA PRIMERA VEZ
    generateLevel(player, enemy, obstacles, hole);
}

void handleEvents(SDL_Event &event, GameState &state, Entity* player) {
    // Eventos tin ling
    while (SDL_PollEvent(&event)) {
        
        if (event.type == SDL_QUIT) {
            state.isRunning = false;
        }
        
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                state.isRunning = false;
            }
        }

        // Logica del tiro de golf
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            
            // Se añade una comprobación para verificar que el jugador esté quieto
            if (player->vx == 0.0f && player->vy == 0.0f) {
                state.isAiming = true;
                state.aimStartX = event.button.x;
                state.aimStartY = event.button.y;
                state.aimCurrentX = event.button.x;
                state.aimCurrentY = event.button.y;
            }
        }

        if (event.type == SDL_MOUSEMOTION) {
            if (state.isAiming) {
                state.aimCurrentX = event.motion.x;
                state.aimCurrentY = event.motion.y;
            }
        }

        if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
            if (state.isAiming) {
                state.isAiming = false;
                
                float launchDX = (state.aimStartX - state.aimCurrentX);
                float launchDY = (state.aimStartY - state.aimCurrentY);
                
                player->vx = launchDX * POWER_MULTIPLIER;
                player->vy = launchDY * POWER_MULTIPLIER;

                AudioHandler::getInstance().playSound("shoot");
            }
        }
    } 
}

void update(SDL_Window* window, GameState &state, Entity* player, Entity* enemy, Obstacle obstacles[], Entity* hole) {
    // Logica del enemigo
    { 
        float targetX = player->x + (player->w / 2.0f);
        float targetY = player->y + (player->h / 2.0f);

        float dx = targetX - (enemy->x + enemy->w / 2.0f);
        float dy = targetY - (enemy->y + enemy->h / 2.0f);

        float distance = sqrt(dx * dx + dy * dy);

        if (distance > 1.0f) { 
            float moveX = (dx / distance) * ENEMY_SPEED;
            float moveY = (dy / distance) * ENEMY_SPEED;

            enemy->x += moveX;
            enemy->y += moveY;
        }
    }
    
    // --- LÓGICA DEL JUGADOR REFACTORIZADA ---
    if (!state.isAiming) {
        
        // --- MOVIMIENTO Y COLISIÓN EJE X ---
        player->x += player->vx;

        // Comprobar colisión X con obstáculos
        for (int i = 0; i < NUM_OBSTACLES; ++i) {
            Obstacle* obs = &obstacles[i]; 
            
            if (checkCollision(player->x, player->y, player->w, player->h, obs->x, obs->y, obs->w, obs->h)) {
                if (player->vx > 0) { // Se movía a la derecha
                    player->x = obs->x - player->w; 
                } else if (player->vx < 0) { // Se movía a la izquierda
                    player->x = obs->x + obs->w; 
                }
                player->vx *= -1; // Rebotar
                AudioHandler::getInstance().playSound("bounce");
            }
        }

        // Colisión X con bordes
        if (player->x < 0) {
            player->x = 0;
            player->vx *= -1;
        } else if (player->x + player->w > SCREEN_WIDTH) {
            player->x = SCREEN_WIDTH - player->w;
            player->vx *= -1;
        }

        // --- MOVIMIENTO Y COLISIÓN EJE Y ---
        player->y += player->vy;
        
        // Comprobar colisión Y con obstáculos
        for (int i = 0; i < NUM_OBSTACLES; ++i) {
            Obstacle* obs = &obstacles[i];

            if (checkCollision(player->x, player->y, player->w, player->h, obs->x, obs->y, obs->w, obs->h)) {
                if (player->vy > 0) { // Se movía hacia abajo
                    player->y = obs->y - player->h; 
                } else if (player->vy < 0) { // Se movía hacia arriba
                    player->y = obs->y + obs->h; 
                }
                player->vy *= -1; // Rebotar
                AudioHandler::getInstance().playSound("bounce");
            }
        }

        // Colisión Y con bordes
        if (player->y < 0) {
            player->y = 0;
            player->vy *= -1;
        } else if (player->y + player->h > SCREEN_HEIGHT) {
            player->y = SCREEN_HEIGHT - player->h;
            player->vy *= -1;
        }

        // --- FRICCIÓN Y PARADA ---
        player->vx *= FRICTION;
        player->vy *= FRICTION;

        if (abs(player->vx) + abs(player->vy) < 2) {
            player->vx = 0.0f;
            player->vy = 0.0f;
        }
    } 

    // Comprobar colisión con el hoyo (VICTORIA)
    if (checkCollision(player->x, player->y, player->w, player->h, hole->x, hole->y, hole->w, hole->h))
    {
        AudioHandler::getInstance().playSound("GG");
        cout << "Has llegado al hoyo Nivel " << state.levelCount << " completado." << endl;
        
        state.levelCount++; // Aumentamos contador
        state.isAiming = false; // Dejamos de apuntar por si acaso

        // Actualizar título de la ventana
        string newTitle = "Uncanny Cat Golf Nivel " + to_string(state.levelCount);
        SDL_SetWindowTitle(window, newTitle.c_str());

        // Generar nuevo mapa
        generateLevel(player, enemy, obstacles, hole);
    }

    // Comprobamos si los rectángulos se superponen (enemy y player) (DERROTA)
    if (state.isRunning && 
        checkCollision(player->x, player->y, player->w, player->h, enemy->x, enemy->y, enemy->w, enemy->h))
    {
        AudioHandler::getInstance().playSound("gameover");
        state.isRunning = false; 
        cout << "¡El Uncanny Cat te ha atrapado! Fin del juego. Llegaste al nivel " << state.levelCount << "." << endl;
    }
}

void render(SDL_Renderer* renderer, const GameState &state, const Entity* player, const Entity* enemy, const Obstacle obstacles[], const Entity* hole, SDL_Texture* background, TTF_Font* &g_font) {
    
    // Dibuja el fondo
    if (background) {
        SDL_RenderCopy(renderer, background, NULL, NULL);
    } else {
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255); 
        SDL_RenderClear(renderer);
    }

    //Dibujar los obstáculos
    for (int i = 0; i < NUM_OBSTACLES; ++i) {
        SDL_Rect obsRect = {
            static_cast<int>(obstacles[i].x),
            static_cast<int>(obstacles[i].y),
            obstacles[i].w,
            obstacles[i].h
        };
        if (obstacles[i].texture) {
            SDL_RenderCopy(renderer, obstacles[i].texture, NULL, &obsRect);
        } else {
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255); 
            SDL_RenderFillRect(renderer, &obsRect);
        }
    }

    //Dibujar el hoyo
    SDL_Rect holeRenderRect = {
        static_cast<int>(hole->x),
        static_cast<int>(hole->y),
        hole->w,
        hole->h
    };
    if (hole->texture) {
        SDL_RenderCopy(renderer, hole->texture, NULL, &holeRenderRect);
    } else {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &holeRenderRect);
    }

    // Dibuja al enemigo
    SDL_Rect enemyRenderRect = {
        static_cast<int>(enemy->x),
        static_cast<int>(enemy->y),
        enemy->w,
        enemy->h
    };
    if (enemy->texture) {
        SDL_RenderCopy(renderer, enemy->texture, NULL, &enemyRenderRect);
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &enemyRenderRect);
    }

    // Dibujar al jugador
    SDL_Rect playerRenderRect = {
        static_cast<int>(player->x), 
        static_cast<int>(player->y), 
        player->w,                 
        player->h                   
    };
    
    if (player->texture) {
        SDL_RenderCopy(renderer, player->texture, NULL, &playerRenderRect);
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &playerRenderRect);
    }

    // Dibujar el numero de nivel en la esquina superior izquierda
    
    // Convertir el nivel a texto
    string levelText = "Nivel: " + to_string(state.levelCount);
    // Renderizar el texto
    SDL_Color textColor = { 255, 255, 255, 255 };
    SDL_Surface* textSurface = TTF_RenderText_Solid(g_font, levelText.c_str(), textColor);
    if (textSurface) {
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        if (textTexture) {
            SDL_Rect textRect = { 10, 10, textSurface->w, textSurface->h };
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
            SDL_DestroyTexture(textTexture);
        }
        SDL_FreeSurface(textSurface);
    }

    // Esta parte es la linea para apuntar
    if (state.isAiming) {
        int playerCenterX = static_cast<int>(player->x + player->w / 2);
        int playerCenterY = static_cast<int>(player->y + player->h / 2);

        int pullDX = state.aimCurrentX - state.aimStartX;
        int pullDY = state.aimCurrentY - state.aimStartY;

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // RGBA
        SDL_RenderDrawLine(renderer, 
            playerCenterX,          
            playerCenterY,          
            playerCenterX - pullDX, 
            playerCenterY - pullDY  
        );
    }

    //Esta función es para que se vea lo que ya se dibujo
    SDL_RenderPresent(renderer);
}

void cleanup(SDL_Window* window, SDL_Renderer* renderer, Entity* player, Entity* enemy, Obstacle obstacles[], Entity* hole, SDL_Texture* background, SDL_Texture* &obstacleTexture) {
    cout << "Cerrando el juego..." << endl;
    
    if (player->texture){
        SDL_DestroyTexture(player->texture);
    }
    delete player;
    player = nullptr; 
    
    if (enemy->texture){
        SDL_DestroyTexture(enemy->texture);
    }
    delete enemy;
    enemy = nullptr;

    if (hole->texture){
        SDL_DestroyTexture(hole->texture);
    }
    delete hole;
    hole = nullptr;

    if (obstacleTexture) {
        SDL_DestroyTexture(obstacleTexture);
        obstacleTexture = nullptr;
    }

    AudioHandler::getInstance().shutdown();

    delete[] obstacles;
    obstacles = nullptr;

    if (background) {
        SDL_DestroyTexture(background);
    }
    

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    IMG_Quit();
    SDL_Quit();
    TTF_Quit();
}
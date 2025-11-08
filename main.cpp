#include <SDL2/SDL.h> //EL moto de bajo nivel
#include <SDL2/SDL_image.h> //Sprites
#include <iostream> //No me acuerdo
#include <cmath> // Para las mates (sqrt, abs)
#include <string> // Para usar std::string
#include <cstdlib> //  Para rand() y srand()
#include <ctime>   //  Para time() (semilla de rand)

using namespace std; // Pa no escribir std:: a cada rato

// --- Constantes Globales ---
// Son las variables que definen el alto y ancho de la ventana
int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 600;

const float POWER_MULTIPLIER = 0.1f;  // Multiplicador de fuerza (qué tan fuerte es el golpe)
const float FRICTION = 0.985f;         // Fricción (ej. 0.985 = 1.5% de freno por frame)
const float ENEMY_SPEED = 0.5f;        // Velocidad de persecución del enemigo

const int NUM_OBSTACLES = 30; // Cantidad de obstáculos

// --- Estructuras de Datos ---

// Renombramos Player a Entity, ya que ahora tendremos dos
struct Entity {
    float x, y;           // Estas variables son para la posición actual papu
    float vx, vy;         // Estas variables equivalen a la velocidad
    int w, h;             // Estas variables son el ancho y alto (del objeto)
    SDL_Texture* texture; // Variable para guardar nuestra imagen
};

// Estructura para los obstáculos
// No necesita velocidad ni textura, solo posición y tamaño
struct Obstacle {
    float x, y;
    int w, h;
};

// Un struct para mantener el estado del juego organizado
struct GameState {
    bool isRunning = true;      // El game loop principal
    
    // Aqui empiezan las mecanicas del golf, y esta es una puta mierda asi que presta atención porque es dificil de explicar
    bool isAiming = false;      // Aca declaramos la variable que utilizaremos para saber si basicamente el jugador esta presionando el click para apuntar (la función para eso esta mas adelante)

    //En el momento que haces click entran a la mesa las siguientes variables
    int aimStartX = 0;          // Posición X de donde empezó el clic
    int aimStartY = 0;          // Posición Y de donde empezó el clic
    int aimCurrentX = 0;        // Posición X actual del mouse
    int aimCurrentY = 0;        // Posición Y actual del mouse
};


// --- Prototipos de Funciones ---
// (Le decimos al compilador qué funciones existen)

// Función de ayuda para comprobar colisiones AABB
bool checkCollision(float x1, float y1, int w1, int h1, float x2, float y2, int w2, int h2);

// SDL_IMAGE
// Esta función carga una imagen y la convierte en textura
SDL_Texture* loadTexture(string path, SDL_Renderer* renderer);

// Inicializa SDL y crea la ventana/renderer
bool init(SDL_Window* &window, SDL_Renderer* &renderer);

// Carga texturas e inicializa la posición de las entidades
// Añadido Obstacle obstacles[] y Entity* hole
void loadMedia(SDL_Renderer* renderer, Entity* player, Entity* enemy, Obstacle obstacles[], Entity* hole, SDL_Texture* &background);

// Maneja toda la entrada del usuario (teclado, mouse)
void handleEvents(SDL_Event &event, GameState &state, Entity* player);

// Actualiza la lógica del juego (físicas, IA)
// Añadido const Obstacle obstacles[] y const Entity* hole
void update(GameState &state, Entity* player, Entity* enemy, const Obstacle obstacles[], const Entity* hole);

// Dibuja todo en la pantalla
// Añadido const Obstacle obstacles[] y const Entity* hole
void render(SDL_Renderer* renderer, const GameState &state, const Entity* player, const Entity* enemy, const Obstacle obstacles[], const Entity* hole, SDL_Texture* background);

// Libera todos los recursos
// Añadido Obstacle obstacles[] y Entity* hole
void cleanup(SDL_Window* window, SDL_Renderer* renderer, Entity* player, Entity* enemy, Obstacle obstacles[], Entity* hole, SDL_Texture* background);


// --- Función Principal (main) ---
// Ahora main es solo el "director" que llama a las otras funciones.
int main(int argc, char* argv[]) {
    // Variables principales de SDL
    SDL_Window* g_window = nullptr;
    SDL_Renderer* g_renderer = nullptr;
    
    // Texturas y Entidades
    SDL_Texture* g_backgroundTexture = nullptr;
    Entity* g_player = new Entity();
    Entity* g_enemy = new Entity();
    // Array dinámico para guardar los obstáculos
    Obstacle* g_obstacles = new Obstacle[NUM_OBSTACLES];
    // Entidad para el hoyo (meta)
    Entity* g_hole = new Entity();

    // Estado del juego
    GameState g_gameState;
    // Esta es la "caja" donde SDL va a meter los eventos (teclas, mouse, etc)
    SDL_Event g_event;

    // Inicializar
    if (!init(g_window, g_renderer)) {
        cerr << "Fallo al inicializar." << endl;
        return 1;
    }

    // Cargar Media
    // Pasamos g_obstacles y g_hole
    loadMedia(g_renderer, g_player, g_enemy, g_obstacles, g_hole, g_backgroundTexture);

    //Es un cout wei, solo que se va a escribir en la consola
    cout << "Motor iniciado. Haz clic y arrastra para disparar." << endl;
    cout << "Llega al hoyo negro para ganar. Evita al Uncanny Cat." << endl; // 

    // Game Loop
    //El game loop, para que el juego continue el tiempo que requerimos, esto nos va a servir para poder finalizar el programa sin la necesidad de tener que presionar la x
    //Este apartado nos va a servir para finalizar el programa en ciertas ocasiones como cuando el uncanny cat nos toqué (no esta programada todavia)
    
    //Pues mientras que efectivamente el programa este funcionando proceda
    while (g_gameState.isRunning) {
        
        // Manejar Eventos
        handleEvents(g_event, g_gameState, g_player);
        
        // Actualizar Lógica
        // Pasamos g_obstacles y g_hole
        // Aquí es donde ocurre la física, esto se ejecuta en cada momento del programa, (osea cuando la pelota se esta moviendo, esto hace que se mueva y se siga moviendo).
        update(g_gameState, g_player, g_enemy, g_obstacles, g_hole);

        // Dibujar
        // Pasamos g_obstacles y g_hole
        // Este apartado es la parte grafica, lo que se ve
        render(g_renderer, g_gameState, g_player, g_enemy, g_obstacles, g_hole, g_backgroundTexture);

    } // Fin del Game Loop

    // Limpiar y Salir
    // Pasamos g_obstacles y g_hole
    cleanup(g_window, g_renderer, g_player, g_enemy, g_obstacles, g_hole, g_backgroundTexture);

    return 0; // El programa termina
}


// --- Implementación de Funciones ---

// 
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

bool init(SDL_Window* &window, SDL_Renderer* &renderer) {

    //Inicializa el sdl2, la verdad ni puta idea, pero todos los videos lo ponian, obviamente revisa si el sdl se inicio correctamente y si no que se detenga el programa,
    //pero la estructura esta algo complicada
    
    // Tenemos que inicializar SDL_VIDEO
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
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

    //El evento que crea la ventana donde se va a cargar el jueguito, y funciona asi: [Nombre, x , y , alto, ancho, y si se sobrepone o no]
    window = SDL_CreateWindow(
        "Uncanny Cat Golf", // Título actualizado
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN //Pantalla completa: SDL_WINDOW_FULLSCREEN_DESKTOP
    );

    //Si no se abre la ventana por cualquier cosa que se pare el programa, para ver los errores cawn
    if (!window) {
        cerr << "Error al crear la ventana: " << SDL_GetError() << endl;
        IMG_Quit();
        SDL_Quit();
        return false;
    }

    // 
    // Si la ventana NO está en fullscreen, usamos el tamaño fijo.
    // Si SÍ está en fullscreen, actualizamos las constantes.
    Uint32 flags = SDL_GetWindowFlags(window);
    if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
        SDL_GetWindowSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
        cout << "Modo Fullscreen detectado. Tamaño real: " << SCREEN_WIDTH << "x" << SCREEN_HEIGHT << endl;
    }


    // Este es el "Renderizador". Es el pincel del SDL.
    // Lo usas para dibujar cosas en la ventana .
    // SDL_RENDERER_ACCELERATED usa la tarjeta gráfica (modo rápido)
    // SDL_RENDERER_PRESENTVSYNC sincroniza el juego con el refresh del monitor (basicamente sincronización vertical)
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        // Si el pincel no se crea, tampoco podemos seguir
        cerr << "Error al crear el renderizador: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return false;
    }
    
    return true; // Éxito
}

// Añadido Obstacle obstacles[] y Entity* hole
void loadMedia(SDL_Renderer* renderer, Entity* player, Entity* enemy, Obstacle obstacles[], Entity* hole, SDL_Texture* &background) {
    // (Ahora es un 'Entity', pero la variable se sigue llamando g_player)
    player->w = 50; //Ancho
    player->h = 50; //Alto
    player->x = (SCREEN_WIDTH / 2.0f) - (player->w / 2.0f); //Posición x (centrado)
    player->y = (SCREEN_HEIGHT / 2.0f) - (player->h / 2.0f); // POsición y (centrado)
    player->vx = 0.0f; // Velocidad horizontal
    player->vy = 0.0f; // Velocidad vertical
    
    // Cargamos la textura del jugador
    player->texture = loadTexture("cat.png", renderer);
    if (player->texture == nullptr) {
        cerr << "Error: No se pudo cargar 'cat.png'. El jugador será un cuadrado blanco." << endl;
    }

    // Configurar enemigo
    enemy->w = 50; // Mismo tamaño
    enemy->h = 50;
    enemy->x = 0.0f; // Posición opuesta (esquina superior izquierda)
    enemy->y = 0.0f;
    enemy->vx = 0.0f;
    enemy->vy = 0.0f;

    // Cargamos la textura del enemigo
    enemy->texture = loadTexture("uncanny_cat.png", renderer);
    if (enemy->texture == nullptr) {
        cerr << "Error: No se pudo cargar 'uncanny_cat.png'. El enemigo será un cuadrado rojo." << endl;
    }
    
    //cargar texturas de fondo
    background = loadTexture("fondo.png", renderer);
    if (background == nullptr) {
        cerr << "Error: No se pudo cargar 'fondo.png'. El fondo será gris." << endl;
    }

    // Inicializar la semilla aleatoria
    srand(time(NULL)); 

    // Inicializar los obstáculos
    cout << "Generando " << NUM_OBSTACLES << " obstáculos..." << endl;
    for (int i = 0; i < NUM_OBSTACLES; ++i) {
        obstacles[i].w = 40; // Ancho de 50
        obstacles[i].h = 40; // Alto de 50
        
        // Posición aleatoria dentro de la pantalla
        obstacles[i].x = static_cast<float>(rand() % (SCREEN_WIDTH - obstacles[i].w));
        obstacles[i].y = static_cast<float>(rand() % (SCREEN_HEIGHT - obstacles[i].h));

        // NOTA: Podríamos comprobar si los obstáculos se superponen entre sí,
        // pero por ahora lo dejamos simple.
    }

    //  Configurar el hoyo
    hole->w = 50; // El hoyo tendrá 50x50
    hole->h = 50;
    hole->vx = 0;
    hole->vy = 0;
    hole->texture = loadTexture("hole.png", renderer); // Asegúrate de tener "hole.png"
    if (hole->texture == nullptr) {
        cerr << "Error: No se pudo cargar 'hole.png'. El hoyo será un cuadrado negro." << endl;
    }

    //  Lógica de generación para el hoyo (evitando colisiones)
    cout << "Buscando un lugar para el hoyo..." << endl;
    bool isColliding;
    do {
        isColliding = false; // Asumimos que no hay colisión al empezar
        
        // Generar una posición aleatoria
        hole->x = static_cast<float>(rand() % (SCREEN_WIDTH - hole->w));
        hole->y = static_cast<float>(rand() % (SCREEN_HEIGHT - hole->h));

        // Comprobar colisión con el jugador
        if (checkCollision(hole->x, hole->y, hole->w, hole->h, player->x, player->y, player->w, player->h)) {
            isColliding = true;
            continue; // Volver a empezar el bucle
        }

        // Comprobar colisión con el enemigo
        if (checkCollision(hole->x, hole->y, hole->w, hole->h, enemy->x, enemy->y, enemy->w, enemy->h)) {
            isColliding = true;
            continue; // Volver a empezar el bucle
        }

        // Comprobar colisión con todos los obstáculos
        for (int i = 0; i < NUM_OBSTACLES; ++i) {
            if (checkCollision(hole->x, hole->y, hole->w, hole->h, obstacles[i].x, obstacles[i].y, obstacles[i].w, obstacles[i].h)) {
                isColliding = true;
                break; // Salir del bucle 'for' y volver a empezar el 'do-while'
            }
        }

    } while (isColliding); // Repetir si 'isColliding' se marcó como 'true'

    cout << "Hoyo generado en (" << hole->x << ", " << hole->y << ")" << endl;
}


void handleEvents(SDL_Event &event, GameState &state, Entity* player) {
    // Eventos tin ling
    //Mientras haya eventos pendientes y verifica esos eventos pendientes con el event si te acuerdas lo declaramos arriba
    while (SDL_PollEvent(&event)) {
        
        //Si el evento evaluado es igual a "evento salir" que finalice el programa
        //Por cierto a mucho de esto me voy a referir como evento pero la mayoria mas que nada son funciones del mismo sdl2
        if (event.type == SDL_QUIT) {
            state.isRunning = false;
        }
        
        // Checa si se presionó una tecla
        if (event.type == SDL_KEYDOWN) {
            // Si la tecla fue el esc que se cierre el programa
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                state.isRunning = false;
            }
        }

        //-------------------------------------------------------------------------------------------------------------------------
        // Aqui va la Logica del tiro de golf
        
        // Esto seria lo que es el inicio del tiro, en el momento que hacen click
        //Si el evento evaluado es "evento-hacer click" y es el click izquierdo
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            
            // Se añade una comprobación para verificar que el jugador esté quieto
            // (velocidad en X e Y sean 0) antes de permitir apuntar.
            if (player->vx == 0.0f && player->vy == 0.0f) {
                
                //Ponemos el "esta apuntando en true" y actualizamos los valores de la posición actual (recuerda que las variables de posicionamiento y del tiro son diferentes ver "g_player")
                state.isAiming = true;
                state.aimStartX = event.button.x;
                state.aimStartY = event.button.y;
                state.aimCurrentX = event.button.x;
                state.aimCurrentY = event.button.y;
            }
            // Si el jugador no está quieto, el clic simplemente se ignora.
        }

        // Esto seria el apuntado
        // Si el mouse se mueve 
        if (event.type == SDL_MOUSEMOTION) {
            // Y estamos apuntando (o sea, el clic está presionado)
            if (state.isAiming) {
                // actualizamos la posición actual del mouse
                state.aimCurrentX = event.motion.x;
                state.aimCurrentY = event.motion.y;
            }
        }

        //Esta parte ya es para el momento que se suelta el click
        // Si el evento es "soltar botón" Y fue el izquierdo
        if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
            // y estábamos apuntando
            if (state.isAiming) {
                // se deja de apuntar, para evitar que se hagan tiros doubles o triples y ganen mas momentum (asi es el juego tiene momentum, se puede hacer bunny hop como el l4d2)
                state.isAiming = false;
                
                //Formulas brindadas por IA, no hay mas que decir
                float launchDX = (state.aimStartX - state.aimCurrentX);
                float launchDY = (state.aimStartY - state.aimCurrentY);
                
                // Aplicamos la fuerza (el vector por el multiplicador de poder)
                player->vx = launchDX * POWER_MULTIPLIER;
                player->vy = launchDY * POWER_MULTIPLIER;
            }
        }
    } // Fin del bucle de eventos (SDL_PollEvent)
}

// Añadido const Obstacle obstacles[] y const Entity* hole
void update(GameState &state, Entity* player, Entity* enemy, const Obstacle obstacles[], const Entity* hole) {
    // (Esta función contiene la lógica que estaba después de SDL_PollEvent)

    //-------------------------------------------------------------------------------------------------------------------------
    //Logica del enemigo
// ... (sin cambios en esta sección) ...
    // El enemigo se mueve hacia el jugador
    { // Se usa un bloque {} para aislar las variables de esta lógica
        // Encontrar el objetivo (el centro del jugador)
        // (Tu idea de usar aimCurrentX era buena, pero esto hace que persiga al jugador)
        float targetX = player->x + (player->w / 2.0f);
        float targetY = player->y + (player->h / 2.0f);

        // Calcular el vector hacia el objetivo (desde el centro del enemigo)
        float dx = targetX - (enemy->x + enemy->w / 2.0f);
        float dy = targetY - (enemy->y + enemy->h / 2.0f);

        //  Calcular la distancia (teorema de pitágoras)
        float distance = sqrt(dx * dx + dy * dy);

        // Mover al enemigo
        // Solo nos movemos si no estamos "encima" del jugador (para evitar vibraciones)
        if (distance > 1.0f) { // 1.0f es un pequeño margen de error
            // Normalizar el vector (convertirlo en dirección pura)
            float moveX = (dx / distance) * ENEMY_SPEED;
            float moveY = (dy / distance) * ENEMY_SPEED;

            // Aplicar el movimiento
            enemy->x += moveX;
            enemy->y += moveY;
        }
    }
    
    //-------------------------------------------------------------------------------------------------------------------------
    // --- LÓGICA DEL JUGADOR REFACTORIZADA ---
// ... (sin cambios en esta sección) ...
    // Esta era la parte que te decia que se bugueaba mucho, asi que de preferencia no le muevas
    if (!state.isAiming) {
        
        // Vamos a separar la física en X e Y.
        // Esto hace que las colisiones con las esquinas funcionen mucho mejor.

        // --- MOVIMIENTO Y COLISIÓN EJE X ---
        player->x += player->vx;

        // Comprobar colisión X con obstáculos
        for (int i = 0; i < NUM_OBSTACLES; ++i) {
            const Obstacle* obs = &obstacles[i];
            
            // Comprobación AABB (Cajas de colisión)
            if (checkCollision(player->x, player->y, player->w, player->h, obs->x, obs->y, obs->w, obs->h)) {
                // Hubo colisión. Averiguar de qué lado
                if (player->vx > 0) { // Se movía a la derecha
                    player->x = obs->x - player->w; // Atorar a la izquierda del obstáculo
                } else if (player->vx < 0) { // Se movía a la izquierda
                    player->x = obs->x + obs->w; // Atorar a la derecha
                }
                player->vx *= -1; // Rebotar
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
            const Obstacle* obs = &obstacles[i];

            // Comprobación AABB
            if (checkCollision(player->x, player->y, player->w, player->h, obs->x, obs->y, obs->w, obs->h)) {
                // Hubo colisión.
                if (player->vy > 0) { // Se movía hacia abajo
                    player->y = obs->y - player->h; // Atorar arriba
                } else if (player->vy < 0) { // Se movía hacia arriba
                    player->y = obs->y + obs->h; // Atorar abajo
                }
                player->vy *= -1; // Rebotar
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
        // (Se aplican después de todas las colisiones)
        player->vx *= FRICTION;
        player->vy *= FRICTION;

        if (abs(player->vx) + abs(player->vy) < 2) {
            player->vx = 0.0f;
            player->vy = 0.0f;
        }
    } // aca termina el if (!state.isAiming)

    //-------------------------------------------------------------------------------------------------------------------------
    
    // Comprobar colisión con el hoyo (VICTORIA)
    // Lo comprobamos ANTES que la colisión con el enemigo
    if (checkCollision(player->x, player->y, player->w, player->h, hole->x, hole->y, hole->w, hole->h))
    {
        // Opcional: Centrar al jugador en el hoyo
        player->vx = 0;
        player->vy = 0;
        player->x = hole->x + (hole->w / 2.0f) - (player->w / 2.0f);
        player->y = hole->y + (hole->h / 2.0f) - (player->h / 2.0f);
        
        state.isRunning = false; // Detenemos el bucle del juego
        cout << "¡Has llegado al hoyo! ¡Ganaste!" << endl;
    }


    // Comprobamos si los rectángulos se superponen (enemy y player) (DERROTA)
    if (state.isRunning && // Solo comprobar si el juego no ha terminado (por victoria)
        checkCollision(player->x, player->y, player->w, player->h, enemy->x, enemy->y, enemy->w, enemy->h))
    {
        // ggs skill issue
        state.isRunning = false; // Detenemos el bucle del juego
        cout << "¡El Uncanny Cat te ha atrapado! Fin del juego." << endl;
    }
}

// --- MODIFICADO --- Añadido const Obstacle obstacles[] y const Entity* hole
void render(SDL_Renderer* renderer, const GameState &state, const Entity* player, const Entity* enemy, const Obstacle obstacles[], const Entity* hole, SDL_Texture* background) {
    // (Esta función contiene toda la lógica de dibujo)
    
    // Dibuja el fondo
// ... (sin cambios en esta sección) ...
    if (background) {
        // Dibuja la textura de fondo para que cubra toda la pantalla
        // El primer NULL = usa la textura completa
        // El segundo NULL = estírala a toda la ventana
        SDL_RenderCopy(renderer, background, NULL, NULL);
    } else {
        // Si no hay fondo, usa el color gris
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255); // Usa el formato R, G, B, A por si lo quieres cambiar
        SDL_RenderClear(renderer);
    }

    // Dibujar los obstáculos
// ... (sin cambios en esta sección) ...
    // Los dibujamos después del fondo pero antes que el jugador y el enemigo
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255); // Color gris
    for (int i = 0; i < NUM_OBSTACLES; ++i) {
        SDL_Rect obsRect = {
            static_cast<int>(obstacles[i].x),
            static_cast<int>(obstacles[i].y),
            obstacles[i].w,
            obstacles[i].h
        };
        SDL_RenderFillRect(renderer, &obsRect);
    }

    //  Dibujar el hoyo
    // Lo dibujamos después de los obstáculos, pero antes que el jugador/enemigo
    SDL_Rect holeRenderRect = {
        static_cast<int>(hole->x),
        static_cast<int>(hole->y),
        hole->w,
        hole->h
    };
    if (hole->texture) {
        SDL_RenderCopy(renderer, hole->texture, NULL, &holeRenderRect);
    } else {
        // Si la textura falló, dibuja un cuadrado negro
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &holeRenderRect);
    }


    // Dibuja al enemigo
// ... (sin cambios en esta sección) ...
    SDL_Rect enemyRenderRect = {
        static_cast<int>(enemy->x),
        static_cast<int>(enemy->y),
        enemy->w,
        enemy->h
    };
    if (enemy->texture) {
        SDL_RenderCopy(renderer, enemy->texture, NULL, &enemyRenderRect);
    } else {
        // Si la textura falló, dibuja un cuadrado rojo
        // (para que al menos se vea algo)
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &enemyRenderRect);
    }

    // Dibujar al jugador
// ... (sin cambios en esta sección) ...
    // (Lo dibujamos después del enemigo para que aparezca por encima)
    // En el SDL no podemos usar floats pa dibujar, así que convertimos
    // nuestra posición (float) a un int.
    SDL_Rect playerRenderRect = {
        static_cast<int>(player->x), // x (convertido a int)
        static_cast<int>(player->y), // y (convertido a int)
        player->w,                  // ancho
        player->h                   // alto
    };
    
    // Esto "copia" la textura (g_player->texture) al renderizador
    // en la posición y tamaño de renderRect
    if (player->texture) {
        SDL_RenderCopy(renderer, player->texture, NULL, &playerRenderRect);
    } else {
        // Si la textura falló en cargar, dibujamos el cuadrado blanco
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &playerRenderRect);
    }

    // Esta parte es la linea para apuntar
// ... (sin cambios en esta sección) ...
    // Si estamos apuntando
    if (state.isAiming) {
        // Calculamos el centro del jugador (formulas por IA otra vez)
        int playerCenterX = static_cast<int>(player->x + player->w / 2);
        int playerCenterY = static_cast<int>(player->y + player->h / 2);

        // Calculamos el vector de arrastre (cuánto ha arrastrado el mouse)
        int pullDX = state.aimCurrentX - state.aimStartX;
        int pullDY = state.aimCurrentY - state.aimStartY;

        //Cambiamos el color del pincel a rojo
        // Y dibujamos la linea hacia el lado opuesto del cuadrado (aqui estuve un muy buen rato pq la fakin linea se seguia dibujando hacia el mismo lado de arrastre y no sabia como cambiarlo)
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Rojo
        SDL_RenderDrawLine(renderer, 
            playerCenterX,          // Punto de inicio X
            playerCenterY,          // Punto de inicio Y
            playerCenterX - pullDX, // Punto final X (El - es lo que lo hace opuesto)
            playerCenterY - pullDY  // Punto final Y
        );
    }

    //Esta función es para que se vea lo que ya se dibujo
    SDL_RenderPresent(renderer);
}

// --- MODIFICADO --- Añadido Obstacle obstacles[] y Entity* hole
void cleanup(SDL_Window* window, SDL_Renderer* renderer, Entity* player, Entity* enemy, Obstacle obstacles[], Entity* hole, SDL_Texture* background) {
    //La limpieza
    cout << "Cerrando el juego..." << endl;
    
    //Aca limpiamos la memoria dinamica que claramos arriba con el new
    // También destruimos la textura del jugador que creamos 
    if (player->texture){
        SDL_DestroyTexture(player->texture);
    }
    delete player;
    player = nullptr; // Ni se para que sirve, pero vi en las exposiciones de los chavos que es una buena practica
    
    // limpiamos al enemigo, es lo mismo que el limpiar al jugador
    if (enemy->texture){
        SDL_DestroyTexture(enemy->texture);
    }
    delete enemy;
    enemy = nullptr;

    //  Limpiar el hoyo
    if (hole->texture){
        SDL_DestroyTexture(hole->texture);
    }
    delete hole;
    hole = nullptr;

    //  Limpiar el array de obstáculos
    delete[] obstacles;
    obstacles = nullptr;

    //Limpiar la textura del fondo
    if (background) {
        SDL_DestroyTexture(background);
    }
    
    // Destruimos el pincel y la ventana
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    // Apagamos SDL_image
    IMG_Quit();
    // Apagamos SDL
    SDL_Quit();
}
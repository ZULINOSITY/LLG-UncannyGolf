#include <SDL2/SDL.h> //EL moto de bajo nivel
#include <SDL2/SDL_image.h> //Sprites
#include <iostream> //No me acuerdo
#include <cmath> // Para las mates (sqrt, abs)

using namespace std; // Pa no escribir std:: a cada rato

struct Player {
    float x, y;           // Estas variables son para la posición actual papu
    float vx, vy;         // Estas variables equivalen a la velocidad
    int w, h;             // Estas variables son el ancho y alto (del objeto)
    SDL_Texture* texture; // Variable para guardar nuestra imagen
};

// SDL_IMAGE
// Esta función carga una imagen y la convierte en textura
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

        // la liberamos
        SDL_FreeSurface(loadedSurface);
    }

    return newTexture;
}


    //Inicializa el sdl2, la verdad ni puta idea, pero todos los videos lo ponian, obviamente revisa si el sdl se inicio correctamente y si no que se detenga el programa,
    //pero la estructura esta algo complicada
int main(int argc, char* argv[]) {
    // Tenemos que inicializar SDL_VIDEO
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cerr << "Error al inicializar SDL: " << SDL_GetError() << endl;
        return 1;
    }

    // Ahora también inicializamos SDL_image (para los sprites)
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        cerr << "Error al inicializar SDL_image: " << IMG_GetError() << endl;
        SDL_Quit();
        return 1;
    }

    // Son las variables que definen el alto y ancho de la ventana
    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 600;

    //El evento que crea la ventana donde se va a cargar el jueguito, y funciona asi: [Nombre, x , y , alto, ancho, y si se sobrepone o no]
    SDL_Window* window = SDL_CreateWindow(
        "Uncanny Cat Golf", // <-- Título actualizado
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    //Si no se abre la ventana por cualquier cosa que se pare el programa, para ver los errores cawn
    if (!window) {
        cerr << "Error al crear la ventana: " << SDL_GetError() << endl;
        IMG_Quit(); // <-- Limpiamos SDL_image
        SDL_Quit();
        return 1;
    }

    // Este es el "Renderizador". Es el pincel del SDL.
    // Lo usas para dibujar cosas en la ventana .
    // SDL_RENDERER_ACCELERATED usa la tarjeta gráfica (modo rápido)
    // SDL_RENDERER_PRESENTVSYNC sincroniza el juego con el refresh del monitor (basicamente sincronización vertical)
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        // Si el pincel no se crea, tampoco podemos seguir
        cerr << "Error al crear el renderizador: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
        IMG_Quit(); // <-- Limpiamos SDL_image
        SDL_Quit();
        return 1;
    }

    // Aqui vamos a aplicar gestión de memoria, vease por el new, y aqui es donde le vamos a asignar las propiedades al futuro canny cat, estas son las variables que van
    // a estar cambiando usando las formulas que se van a ver mas abajo, pero inician con valores iniciales en este caso para que aparezca en el centro
    Player* g_player = new Player();
    g_player->w = 60; //Ancho 
    g_player->h = 60; //Alto
    g_player->x = (SCREEN_WIDTH / 2.0f) - (g_player->w / 2.0f); //Posición x (centrado)
    g_player->y = (SCREEN_HEIGHT / 2.0f) - (g_player->h / 2.0f); // POsición y (centrado)
    g_player->vx = 0.0f; // Velocidad horizontal
    g_player->vy = 0.0f; // Velocidad vertical
    
    // Esta path es relativo al ejecutable (juego.exe)
    // Las imagenes deben estar en la carpeta raiz (por si la quieres cambiar)
    g_player->texture = loadTexture("cat.png", renderer);
    if (g_player->texture == nullptr) {
        cerr << "Error: No se pudo cargar la textura 'cat.png'. Asegúrate de que está en la carpeta LLG-UncannyGolf." << endl;
        // Podríamos cerrar aquí, pero por ahora solo seguirá como un cuadrado invisible
    }

        SDL_Texture* g_backgroundTexture = loadTexture("fondo.png", renderer);
    if (g_backgroundTexture == nullptr) {
        cerr << "Error: No se pudo cargar 'fondo.png'. El fondo será gris." << endl;
    }

    //El game loop, para que el juego continue el tiempo que requerimos, esto nos va a servir para poder finalizar el programa sin la necesidad de tener que presionar la x
    //Este apartado nos va a servir para finalizar el programa en ciertas ocasiones como cuando el uncanny cat nos toqué (no esta programada todavia)
    bool isRunning = true;
    
    // Esta es la "caja" donde SDL va a meter los eventos (teclas, mouse, etc)
    SDL_Event event;

    // Aqui empiezan las mecanicas del golf, y esta es una puta mierda asi que presta atención porque es dificil de explicar
    bool isAiming = false;      // Aca declaramos la variable que utilizaremos para saber si basicamente el jugador esta presionando el click para apuntar (la función para eso esta mas adelante)

    //En el momento que haces click entran a la mesa las siguientes variables
    int aimStartX = 0;          // Posición X de donde empezó el clic
    int aimStartY = 0;          // Posición Y de donde empezó el clic
    int aimCurrentX = 0;        // Posición X actual del mouse
    int aimCurrentY = 0;        // Posición Y actual del mouse

    const float POWER_MULTIPLIER = 0.1f;  // Multiplicador de fuerza (qué tan fuerte es el golpe)
    const float FRICTION = 0.985f;         // Fricción (ej. 0.985 = 1.5% de freno por frame)


    //Es un cout wei, solo que se va a escribir en la consola
    cout << "Motor iniciado. Haz clic y arrastra para disparar." << endl;

    //Ahora si la chamba de los eventos, todos los eventos y funciones los vas a poder encontrar en SDL2-2.32.10\x86_66-w64-mingw32\include pero la neta para encontrarlos
    //y ademas saber la estructura para usarlos mejor buscalo en google

    //Pues mientras que efectivamente el programa este funcionando proceda
    while (isRunning) {
        
        //Mientras haya eventos pendientes y verifica esos eventos pendientes con el event si te acuerdas lo declaramos arriba
        while (SDL_PollEvent(&event)) {
            
            //Si el evento evaluado es igual a "evento salir" que finalice el programa
            //Por cierto a mucho de esto me voy a referir como evento pero la mayoria mas que nada son funciones del mismo sdl2
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
            
            // Checa si se presionó una tecla
            if (event.type == SDL_KEYDOWN) {
                // Si la tecla fue el esc que se cierre el programa
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    isRunning = false;
                }
            }

            
            // Aqui va la Logica del tiro de golf
            
            // Esto seria lo que es el inicio del tiro, en el momento que hacen click
            //Si el evento evaluado es "evento-hacer click" y es el click izquierdo
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                
                // Se añade una comprobación para verificar que el jugador esté quieto
                // (velocidad en X e Y sean 0) antes de permitir apuntar.
                if (g_player->vx == 0.0f && g_player->vy == 0.0f) {
                    // La pelota se detiene, quise hacerlo que siguiera avanzando o que no te dejara apuntar mientras se movia pero el juego se bugueaba bien feo
                    // g_player->vx = 0; // Estas líneas ya no son necesarias gracias a la comprobación
                    // g_player->vy = 0;
                    
                    //Ponemos el "esta apuntando en true" y actualizamos los valores de la posición actual (recuerda que las variables de posicionamiento y del tiro son diferentes ver "g_player")
                    isAiming = true;
                    aimStartX = event.button.x;
                    aimStartY = event.button.y;
                    aimCurrentX = event.button.x;
                    aimCurrentY = event.button.y;
                }
                // Si el jugador no está quieto, el clic simplemente se ignora.
            }

            // Esto seria el apuntado
            // Si el mouse se mueve 
            if (event.type == SDL_MOUSEMOTION) {
                // Y estamos apuntando (o sea, el clic está presionado)
                if (isAiming) {
                    // actualizamos la posición actual del mouse
                    aimCurrentX = event.motion.x;
                    aimCurrentY = event.motion.y;
                }
            }

            //Esta parte ya es para el momento que se suelta el click
            // Si el evento es "soltar botón" Y fue el izquierdo
            if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
                // y estábamos apuntando
                if (isAiming) {
                    // se deja de apuntar, para evitar que se hagan tiros doubles o triples y ganen mas momentum (asi es el juego tiene momentum, se puede hacer bunny hop como el l4d2)
                    isAiming = false;
                    
                    
                    //Formulas brindadas por IA, no hay mas que decir
                    float launchDX = (aimStartX - aimCurrentX);
                    float launchDY = (aimStartY - aimCurrentY);
                    
                    // Aplicamos la fuerza (el vector por el multiplicador de poder)
                    g_player->vx = launchDX * POWER_MULTIPLIER;
                    g_player->vy = launchDY * POWER_MULTIPLIER;
                }
            }
        } // Fin del bucle de eventos (SDL_PollEvent)
        
        // Aquí es donde ocurre la física, esto se ejecuta en cada momento del programa, (osea cuando la pelota se esta moviendo, esto hace que se mueva y se siga moviendo).
        
        // Actualizar posición basada en la velocidad
        // Esta era la parte que te decia que se bugueaba mucho, asi que de preferencia no le muevas
        if (!isAiming) {
            // A la posición 'x' le sumamos la velocidad 'x'
            g_player->x += g_player->vx;
            // A la posición 'y' le sumamos la velocidad 'y'
            g_player->y += g_player->vy;

            // La "fricción"
            // Multiplicamos la velocidad por la fricción (un número < 1)
            // Esto hace que se frene poco a poco, la fricción si gustas lo puedes cambiar (linea 83)
            g_player->vx *= FRICTION;
            g_player->vy *= FRICTION;

            // Este if hace que al sumar las velocidades de x Y y, si no suman cierta cantidad (en este caso 0.1(esto lo cambie a 2 para que se detenga mas rapido))
            if (abs(g_player->vx) + abs(g_player->vy) < 2) {
                // la detiene para evitar que se siga moviendo constantemente
                g_player->vx = 0.0f;
                g_player->vy = 0.0f;
            }


            // Si el borde izquierdo del jugador se pasa de 0
            if (g_player->x < 0) {
                g_player->x = 0;        // Lo atoramos en 0
                g_player->vx *= -1;     // e invertimos su velocidad X (para simular el rebote)
            } 
            // Si el borde derecho (x + ancho) se pasa del ancho de la pantalla...
            else if (g_player->x + g_player->w > SCREEN_WIDTH) {
                g_player->x = SCREEN_WIDTH - g_player->w; // Lo atoramos
                g_player->vx *= -1; // Rebote
            }

            // Lo mismo pero para arriba (y < 0)
            if (g_player->y < 0) {
                g_player->y = 0;
                g_player->vy *= -1; // Rebote
            } 
            // lo mismo pero para abajo (y + alto > alto de pantalla)
            else if (g_player->y + g_player->h > SCREEN_HEIGHT) {
                g_player->y = SCREEN_HEIGHT - g_player->h;
                g_player->vy *= -1; // Rebote
            }
        } // aca termina el if
        
        // Este apartado es la parte grafica, lo que se ve
        if (g_backgroundTexture) {
            // Dibuja la textura de fondo para que cubra toda la pantalla
            // El primer NULL = usa la textura completa
            // El segundo NULL = estírala a toda la ventana
            SDL_RenderCopy(renderer, g_backgroundTexture, NULL, NULL);
        } else {
            //Si no hay fondo que se use el gris por defecto (mas que nada para no borrar esta parte de codigo)
            // Pintamos o "limpiamos la pantalla" con el renderer que declaramos al principio
            // Le decimos al pincel (renderer) que use el color gris oscuro
            SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255); // Usa el formato R, G, B, A por si lo quieres cambiar
            // pinta toda la ventana con ese color
            SDL_RenderClear(renderer);
        }

        



        // Creamos un rectángulo para dibujar. es una paja porque al parecer el sdl2 no se puede hacer circulos tan faciles como el cuadrado, pero eso luego lo cambiamos
        //Si no, la neta que asi quede, eso de agregar radio a las variables esta culeron
        // En el SDL no podemos usar floats pa dibujar, así que convertimos
        // nuestra posición (float) a un int.
        SDL_Rect renderRect = {
            static_cast<int>(g_player->x), // x (convertido a int)
            static_cast<int>(g_player->y), // y (convertido a int)
            g_player->w,                  // ancho
            g_player->h                   // alto
        };
        
        // En lugar de SDL_RenderFillRect, ahora usamos SDL_RenderCopy
        // Esto "copia" la textura (g_player->texture) al renderizador
        // en la posición y tamaño de renderRect
        if (g_player->texture) {
            SDL_RenderCopy(renderer, g_player->texture, NULL, &renderRect);
        } else {
            // Si la textura falló en cargar, dibujamos el cuadrado blanco
            // como antes, para que al menos se vea algo
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &renderRect);
        }


        // Esta parte es la linea para apuntar
        // Si estamos apuntando
        if (isAiming) {
            // Calculamos el centro del jugador (formulas por IA otra vez)
            int playerCenterX = static_cast<int>(g_player->x + g_player->w / 2);
            int playerCenterY = static_cast<int>(g_player->y + g_player->h / 2);

            // Calculamos el vector de arrastre (cuánto ha arrastrado el mouse)
            int pullDX = aimCurrentX - aimStartX;
            int pullDY = aimCurrentY - aimStartY;

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


        //Esta función es para que se vea lo que ya dibujaste
        SDL_RenderPresent(renderer);

    } // Fin del Game Loop

    cout << "Cerrando el juego..." << endl;
    
    //Aca limpiamos la memoria dinamica que claramos arriba con el new
    // También destruimos la textura del jugador que creamos 
    if (g_player->texture){
        SDL_DestroyTexture(g_player->texture);
    }
    

    //Limpiar la textura del fondo
    if (g_backgroundTexture) {
        SDL_DestroyTexture(g_backgroundTexture);
    }
    delete g_player;
    g_player = nullptr; // Ni se para que sirve, pero vi en las exposiciones de los chavos que es una buena practica
    
    // Destruimos el pincel y la ventana
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    // Apagamos SDL_image
    IMG_Quit();
    // Apagamos SDLB
    SDL_Quit();

    return 0; // El programa termina
}
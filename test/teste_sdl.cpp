#include <SDL2/SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif


SDL_Window* window;
SDL_Renderer* renderer;
bool quit = false;

void gameLoop() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            quit = true;
        }
    }

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Rect rect = {50, 50, 100, 100};
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(renderer, &rect);

    SDL_RenderPresent(renderer);

    if (quit) 
    {
          #ifdef __EMSCRIPTEN__
        		emscripten_cancel_main_loop();  // Encerra o loop principal do emscripten
          #endif
    }
  
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("SDL Rectangle", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

   #ifdef __EMSCRIPTEN__
           emscripten_set_main_loop(gameLoop, 60, 1);  // Define o loop principal do emscripten
   #else
    
        while(!quit)
        {
         gameLoop();
        }
        
   #endif
 
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
        
    return 0;
}




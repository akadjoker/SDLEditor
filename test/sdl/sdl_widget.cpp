#include <SDL2/SDL.h>
const int screenWidth = 800;
const int screenHeight = 450;

int main(void)
{
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* window = SDL_CreateWindow("SDL!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    
    bool loop = true;
    SDL_Event event;
    while (loop)
    {
    	while (SDL_PollEvent(&event))
    	{
    	  switch(event.type)
    	  {
    	  	case SDL_QUIT:
    	  	{
    	  	    loop = false;
    	    	break;
    	  	}
    	  	
    	   }
    	}
    	
    	
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
   
     }
    
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}


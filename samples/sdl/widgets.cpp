#include <SDL2/SDL.h>


#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

const int screenWidth = 800;
const int screenHeight = 450;
static bool gameLoop=true;

SDL_Renderer* renderer; 

void mouse_pressed(int x, int y, int button)
{
SDL_Log(" mouse press\n");
}

void mouse_released(int x,int y,int buttom)
{
SDL_Log(" mouse released \n");
}

void mouse_move(int x,int y)
{
}

void UpdateDrawFrame(void)
{
	    

     SDL_Event event;
     while(SDL_PollEvent(&event))
      {
      	switch ( event.type)
      	{
      	   case SDL_QUIT:
      	   {
      	  		 gameLoop=false;
      	        break;
      	   }
      	   case SDL_MOUSEBUTTONDOWN:
      	   {
      	   	 mouse_pressed(event.motion.x,event.motion.y,event.button.button);
      	     break;
      	   }
      	   case SDL_MOUSEBUTTONUP:
      	   {
      	     mouse_released(event.motion.x,event.motion.y,event.button.button);
      	     break;
      	   }
      	   case SDL_MOUSEMOTION:
      	   {
      	      mouse_move(event.motion.x,event.motion.y);
      	      break;
      	   }
      	   
      	   default:
      	   {
      	    break;
      	   }
      	}
     }
   
           SDL_SetRenderDrawColor(renderer, 255, 0, 45, 255);
     SDL_RenderClear(renderer);
     SDL_RenderPresent(renderer);
}



int main(void)
{
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* window = SDL_CreateWindow("SDL !", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, 0);
  

     

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
   
    

   
    while(gameLoop)
    {
      
      
      UpdateDrawFrame();
      		
     
      
    }
    #endif

    
    
    
   SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}







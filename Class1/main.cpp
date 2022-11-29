#include <iostream>
#include "SDL.h"

#pragma comment(lib, "SDL2main.lib")
#pragma comment(lib, "SDL2.lib")

int SDL_main(int argc, char* argv[])
{
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window* MyWindow = SDL_CreateWindow("Test", 100, 100, 800, 600, SDL_WINDOW_OPENGL);
	SDL_Renderer* MyRenderer = SDL_CreateRenderer(MyWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);

	bool bRunning = true;
	int PlayerX = 100;
	int PlayerY = 100;

	while (bRunning)
	{
		//Input
		SDL_Event MyEvent;
		SDL_PollEvent(&MyEvent);
		if (MyEvent.type == SDL_QUIT)
		{
			bRunning = false;
		}
		else if (MyEvent.type == SDL_KEYDOWN)
		{
			switch (MyEvent.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				bRunning = false;
				break;
			case SDLK_w:
				PlayerY--;
				break;
			case SDLK_s:
				PlayerY++;
				break;
			case SDLK_a:
				PlayerX--;
				break;
			case SDLK_d:
				PlayerX++;
				break;
			}
		}

		//Process


		//Render
		SDL_SetRenderDrawColor(MyRenderer, 0, 0, 0, 0);
		SDL_RenderClear(MyRenderer);

		SDL_SetRenderDrawColor(MyRenderer, 0xff, 0, 0, 0);
		SDL_Rect MyRect = SDL_Rect{ PlayerX, PlayerY, 100, 100 };
		SDL_RenderFillRect(MyRenderer, &MyRect);


		SDL_RenderPresent(MyRenderer);

	}

	SDL_Quit();

	return 0;
}
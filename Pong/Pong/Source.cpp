#include <iostream>
#include <assert.h>

using namespace std;


#include "SDL2-2.0.8/include/SDL.h"

#pragma comment(lib, "SDL2-2.0.8\\\lib\\x86\\SDL2.lib")
#pragma comment(lib, "SDL2-2.0.8\\\lib\\x86\\SDL2main.lib")

#pragma comment(linker,"/subsystem:console")

struct Box_Pos
{
	float x, y;
};
struct Box_Speed
{
	float sx, sy;
};



int main(int argc, char **argv)
{
	SDL_Init(SDL_INIT_VIDEO);

	int screen_width = 800;
	int screen_height = 600;

	SDL_Window *window = SDL_CreateWindow("Pong Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_SHOWN);

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);



	Box_Pos *box_pos = new Box_Pos;
	Box_Speed *box_speed = new Box_Speed;

	float speed_coefficient = 0.5;

	{
		box_pos[0].x = screen_width / 2;
		box_pos[0].y = screen_height / 2;

		box_speed[0].sx = speed_coefficient;
		box_speed[0].sy = speed_coefficient;
	}

	bool done = false;
	while (!done)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				done = true;
			}
		}
		/*
				Game Code
				*/

				//update

		box_pos[0].x += box_speed[0].sx;
		box_pos[0].y += box_speed[0].sy;

		if (box_pos[0].x < 0 || box_pos[0].x + 4 > screen_width)
		{
			box_speed[0].sx *= -1.0;
		}
		if (box_pos[0].y < 0 || box_pos[0].y + 4 > screen_height)
		{
			box_speed[0].sy *= -1.0;
		}



		//Set Screen white
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		//clear screen with white
		SDL_RenderClear(renderer);

		//Set Screen Red
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);



		SDL_Rect rect;
		rect.x = box_pos[0].x;
		rect.y = box_pos[0].y;
		rect.w = 10;
		rect.h = 10;

		SDL_RenderFillRect(renderer, &rect);



		SDL_Rect padel1;
		padel1.x = 10;
		if (box_pos[0].x <= 390)
		{
			padel1.y = box_pos[0].y + -50;
		}
		else
		{
			padel1.y = 200;
		}
		padel1.h = 140;
		padel1.w = 10;
		SDL_RenderDrawRect(renderer, &padel1);


		SDL_Rect padel2;
		padel2.x = 780;
		if (box_pos[0].x >= 390)
		{
			padel2.y = box_pos[0].y + -50;
		}
		else
		{
			padel2.y = 200;
		}
		padel2.h = 140;
		padel2.w = 10;
		SDL_RenderDrawRect(renderer, &padel2);


		SDL_RenderPresent(renderer);
	}

	return 0;
}
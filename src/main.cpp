#include "main.unity.h"

#define WIN_WIDTH 1200
#define WIN_HEIGHT 900
global b32 app_running = false;

int main(int argc, char** argv) {
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    fprintf(stderr, "Error initing SDL2\n");
    return -1;
  }

  SDL_Window *window = SDL_CreateWindow("3D Renderer From Scratch",
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        WIN_WIDTH, WIN_HEIGHT,
                                        0);
  if (!window) {
    fprintf(stderr, "Error creating SDL2 window\n");
    return -1;
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
  if (!renderer) {
    fprintf(stderr, "Error creating SDL2 renderer\n");
    return -1;
  }

  // SETUP
  u32 *color_buff = (u32 *)malloc(sizeof(u32)*WIN_WIDTH*WIN_HEIGHT);
  if (!color_buff) {
    fprintf(stderr, "Error allocating with malloc\n");
    return -1;
  }
  SDL_Texture *cb_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIN_WIDTH, WIN_HEIGHT);

  app_running = true;
  while (app_running) {
    // INPUT
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type) {
      case SDL_QUIT: {
        app_running = false;
      } break;
      case SDL_KEYDOWN: {
        if(event.key.keysym.sym == SDLK_ESCAPE) {
          app_running = false;
        }
      } break;
    }

    // UPDATE

    // RENDER
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_UpdateTexture(cb_texture, 0, color_buff, (int)(WIN_WIDTH*sizeof(u32)));
    SDL_RenderCopy(renderer, cb_texture, 0, 0);

    for (u32 y = 0; y < WIN_HEIGHT; ++y) {
      for (u32 x = 0; x < WIN_WIDTH; ++x) {
        color_buff[(WIN_WIDTH * y) + x] = 0xFF616E8B;
      }
    }
    SDL_RenderPresent(renderer);
  }

  return 0;
}

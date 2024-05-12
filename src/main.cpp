#include "main.unity.h"

global b32 app_running = false;
global u32 WIN_WIDTH = 1200;
global u32 WIN_HEIGHT = 900;

static void draw_rect(u32 *color_buffer, u32 x, u32 y, u32 w, u32 h, u32 color) {
  neo_assert(x + w <= WIN_WIDTH);
  neo_assert(y + h <= WIN_HEIGHT);

  for (u32 rect_row = y; rect_row < (y + h); ++rect_row) {
    for (u32 rect_col = x; rect_col < (x + w); ++rect_col) {
      color_buffer[(rect_row*WIN_WIDTH) + rect_col] = color;
    }
  }
}

int main(int argc, char** argv) {
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    fprintf(stderr, "Error initing SDL2\n");
    return -1;
  }

#if 0
  SDL_DisplayMode display_mode;
  if (SDL_GetCurrentDisplayMode(0, &display_mode) != 0) {
    fprintf(stderr, "Error getting display\n");
    return -1;
  }
  WIN_WIDTH = display_mode.w;
  WIN_HEIGHT = display_mode.h;
#endif

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
  // NOTE: Turning this off due to screen flash switching display mode to full screen everytime
  // SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

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
    // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    // SDL_RenderClear(renderer);
    for (u32 y = 0; y < WIN_HEIGHT; ++y) {
      for (u32 x = 0; x < WIN_WIDTH; ++x) {
          color_buff[(WIN_WIDTH * y) + x] = 0xFF616E8B;
      }
    }

    draw_rect(color_buff, (WIN_WIDTH / 2) - (100 / 2), (WIN_HEIGHT / 2) - (100 / 2), 100, 100, 0xFFFF0000);
    SDL_UpdateTexture(cb_texture, 0, color_buff, (int)(WIN_WIDTH*sizeof(u32)));
    SDL_RenderCopy(renderer, cb_texture, 0, 0);

    SDL_RenderPresent(renderer);
  }

  return 0;
}

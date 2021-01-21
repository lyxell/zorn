#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

struct state {
    int zoom;
    bool color;
    bool quit;
    bool* canvas;
    int width;
    int height;
    bool drawing;
    int scroll_x;
    int scroll_y;
};

struct coord {
    int x;
    int y;
};

struct color {
    int r;
    int g;
    int b;
};

bool canvas[50000];

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCROLL_STEP = 5;

void fill_rect(SDL_Surface* surface,
                struct coord upper_left,
                struct coord lower_right,
                struct color c) {
    SDL_Rect rect = {
        upper_left.x,
        upper_left.y,
        lower_right.x - upper_left.x,
        lower_right.y - upper_left.y
    };
    Uint32 color = SDL_MapRGB(surface->format, c.r, c.g, c.b);
    SDL_FillRect(surface, &rect, color);
}

void render(struct state s, SDL_Window* w) {
    SDL_Surface* surface = SDL_GetWindowSurface(w);
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 180, 180, 180));
    fill_rect(surface,
            (struct coord) {MAX(0, -s.scroll_x*s.zoom), MAX(0, -s.scroll_y*s.zoom)},
            (struct coord) {(s.width-s.scroll_x)*s.zoom, (s.height-s.scroll_y)*s.zoom},
            (struct color) {255, 255, 255});
    for (int row = s.scroll_y; row < s.height; row++) {
        int y_draw_pos = row - s.scroll_y;
        for (int col = s.scroll_x; col < s.width; col++) {
            int x_draw_pos = col - s.scroll_x;
            if (row >= 0 && col >= 0 && s.canvas[row * s.width + col]) {
                fill_rect(surface,
                        (struct coord) {x_draw_pos*s.zoom,
                                        y_draw_pos*s.zoom},
                        (struct coord) {(x_draw_pos+1)*s.zoom,
                                        (y_draw_pos+1)*s.zoom},
                        (struct color) {0, 0, 0});
            }
        }
    }
    SDL_UpdateWindowSurface(w);
}

struct state handle_mousedown(struct state s) {
    printf("drawing\n");
    s.drawing = true;
    return s;
}

struct state handle_mouseup(struct state s) {
    printf("not drawing\n");
    s.drawing = false;
    return s;
}

struct state handle_motion(struct state s, struct coord c) {
    if (s.drawing) {
        c.x += s.scroll_x * s.zoom;
        c.y += s.scroll_y * s.zoom;
        c.x /= s.zoom;
        c.y /= s.zoom;
        if (c.x >= 0 && c.x < s.width && c.y >= 0 && c.y < s.height) {
            s.canvas[c.y * s.width + c.x] = s.color;
        }
    }
    return s;
}

struct state handle_keypress(struct state s, int key, int mod) {
    if (mod == KMOD_LSHIFT || mod == KMOD_RSHIFT) {
        switch (key) {
        case SDLK_EQUALS:
            s.zoom++;
            printf("zoom %d%%\n", s.zoom * 100);
            break;
        }
    } else {
        switch (key) {
        case SDLK_MINUS:
            if (s.zoom > 1) {
                s.zoom--;
            }
            printf("zoom %d%%\n", s.zoom * 100);
            break;
        case SDLK_k:
            s.scroll_y -= SCROLL_STEP;
            printf("scroll up\n");
            break;
        case SDLK_j:
            s.scroll_y += SCROLL_STEP;
            printf("scroll down\n");
            break;
        case SDLK_h:
            s.scroll_x -= SCROLL_STEP;
            printf("scroll left\n");
            break;
        case SDLK_l:
            s.scroll_x += SCROLL_STEP;
            printf("scroll right\n");
            break;
        case SDLK_x:
            s.color = !s.color;
            printf("changing color to %s\n", s.color ? "black" : "white");
            break;
        case SDLK_q:
            printf("exiting...\n");
            s.quit = true;
            break;
        }
    }
    return s;
}

int main(int argc, char* args[]) {

    struct state s = {
        .zoom = 12,
        .canvas = canvas,
        .width = 50,
        .height = 10,
        .color = true,
        .scroll_x = 0,
        .scroll_y = 0
    };

    SDL_Event event;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Error: %s\n", SDL_GetError());
        return 1;
    } 

    SDL_Window* window = SDL_CreateWindow("Bitmap editor",
                           SDL_WINDOWPOS_UNDEFINED,
                           SDL_WINDOWPOS_UNDEFINED,
                           SCREEN_WIDTH,
                           SCREEN_HEIGHT,
                           SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    if (window == NULL) {
        fprintf(stderr, "Error: %s\n", SDL_GetError());
        return 1;
    }

    while (true) {

        SDL_WaitEvent(&event);
        if (event.type == SDL_QUIT) {
            s.quit = true;
        } else if (event.type == SDL_KEYDOWN) {
            s = handle_keypress(s, event.key.keysym.sym, event.key.keysym.mod);
        } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            s = handle_mousedown(s);
        } else if (event.type == SDL_MOUSEBUTTONUP) {
            s = handle_mouseup(s);
        } else if (event.type == SDL_MOUSEMOTION) {
            s = handle_motion(s, (struct coord) {event.motion.x, event.motion.y});
        }

        render(s, window);

        if (s.quit) {
            break;
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}


#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>

struct state {
    int zoom;
    bool color;
    bool quit;
    bool* canvas;
    int width;
    int height;
    bool drawing;
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

bool canvas[10000];

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

void fill_rect(SDL_Surface* surface,
                struct coord upper_left,
                struct coord lower_right,
                struct color c) {
//    SDL_LockSurface(surface);
    SDL_Rect rect = {
        upper_left.x,
        upper_left.y,
        lower_right.x - upper_left.x,
        lower_right.y - upper_left.y
    };
    Uint32 color = SDL_MapRGB(surface->format, c.r, c.g, c.b);
    SDL_FillRect(surface, &rect, color);
//    SDL_UnlockSurface(surface);
}

void render(struct state s, SDL_Window* w) {
    SDL_Surface* surface = SDL_GetWindowSurface(w);
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 255, 255, 255));
    for (int row = 0; row < s.height; row++) {
        for (int col = 0; col < s.width; col++) {
            if (s.canvas[row * s.width + col]) {
                struct coord upper_left = {col*s.zoom, row*s.zoom};
                struct coord lower_right = {(col+1)*s.zoom,(row+1)*s.zoom};
                struct color color = {0, 0, 0};
                fill_rect(surface, upper_left, lower_right, color);
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
        printf("drawing at (%d, %d)\n", c.x/s.zoom, c.y/s.zoom);
        s.canvas[(c.y/s.zoom) * s.width + (c.x/s.zoom)] = s.color;
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
        .width = 100,
        .height = 100,
        .color = true
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


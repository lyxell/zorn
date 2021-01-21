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

bool canvas[65536];

const int SCREEN_HEIGHT = 480;
const int SCREEN_WIDTH  = 640;
const int SCROLL_STEP   = 5;
const int UI_HEIGHT     = 30;
const int UI_PADDING    = 10;
const struct color COLOR_BLACK  = {0, 0, 0};
const struct color COLOR_GREY   = {180, 180, 180};
const struct color COLOR_WHITE  = {255, 255, 255};

void fill_rect(SDL_Surface* surface,
                struct coord start,
                struct coord size,
                struct color c) {
    SDL_Rect rect = {
        start.x,
        start.y,
        size.x,
        size.y
    };
    SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, c.r, c.g, c.b));
}

void draw_background(struct state s, SDL_Surface* surface) {
    struct coord origin = {0, 0};
    struct coord size = {surface->w, surface->h};
    fill_rect(surface, origin, size, COLOR_GREY);
}

void draw_canvas(struct state s, SDL_Surface* surface) {
    struct coord canvas_start = {
        -s.scroll_x * s.zoom,
        -s.scroll_y * s.zoom
    };
    struct coord canvas_size = {
        s.width * s.zoom,
        s.height * s.zoom
    };
    fill_rect(surface, canvas_start, canvas_size, COLOR_WHITE);
    struct coord pixel_size = {
        s.zoom,
        s.zoom
    };
    for (int row = 0; row < s.height; row++) {
        for (int col = 0; col < s.width; col++) {
            if (s.canvas[row * s.width + col]) {
                struct coord pixel_start = {
                    canvas_start.x + col * s.zoom,
                    canvas_start.y + row * s.zoom
                };
                fill_rect(surface, pixel_start, pixel_size, COLOR_BLACK);
            }
        }
    }
}

void draw_ui(struct state s, SDL_Surface* surface, struct coord win_size) {
    struct coord ui_start = {UI_PADDING, win_size.y - UI_HEIGHT - UI_PADDING};
    struct coord color_block = {UI_HEIGHT, UI_HEIGHT};
    fill_rect(surface, ui_start, color_block,
                s.color ? COLOR_BLACK : COLOR_WHITE);
}

struct state handle_mousedown(struct state s) {
    s.drawing = true;
    return s;
}

struct state handle_mouseup(struct state s) {
    s.drawing = false;
    return s;
}

struct state handle_motion(struct state s, struct coord c) {
    if (s.drawing) {
        c.x /= s.zoom;
        c.y /= s.zoom;
        c.x += s.scroll_x;
        c.y += s.scroll_y;
        if (c.x >= 0 && c.y >= 0 && c.x < s.width && c.y < s.height) {
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
        SDL_Event event;
        SDL_WaitEvent(&event);
        switch (event.type) {
        case SDL_QUIT:
            s.quit = true;
            break;
        case SDL_KEYDOWN:
            s = handle_keypress(s, event.key.keysym.sym, event.key.keysym.mod);
            break;
        case SDL_MOUSEBUTTONDOWN:
            s = handle_mousedown(s);
            s = handle_motion(s, (struct coord) {event.button.x,
                                                 event.button.y});
            break;
        case SDL_MOUSEBUTTONUP:
            s = handle_mouseup(s);
            break;
        case SDL_MOUSEMOTION:
            s = handle_motion(s, (struct coord) {event.motion.x,
                                                 event.motion.y});
            break;
        }
        if (s.quit) {
            break;
        }
        SDL_Surface* surface = SDL_GetWindowSurface(window);
        struct coord win_size;
        SDL_GetWindowSize(window, &win_size.x, &win_size.y);
        draw_background(s, surface);
        draw_canvas(s, surface);
        draw_ui(s, surface, win_size);
        SDL_UpdateWindowSurface(window);
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}


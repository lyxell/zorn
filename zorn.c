#define _POSIX_C_SOURCE 2
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

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

const char* WINDOW_NAME         = "zorn";
const int INITIAL_CANVAS_HEIGHT = 64;
const int INITIAL_CANVAS_WIDTH  = 64;
const int INITIAL_ZOOM          = 16;
const int SCREEN_HEIGHT         = 480;
const int SCREEN_WIDTH          = 640;
const int SCROLL_STEP           = 1;
const int UI_HEIGHT             = 30;
const int UI_PADDING            = 10;
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

struct coord get_canvas_start(struct state s, struct coord win_size) {
    struct coord canvas_size = {
        s.width * s.zoom,
        s.height * s.zoom
    };
    struct coord canvas_start = {
        (win_size.x - canvas_size.x) / 2,
        (win_size.y - canvas_size.y) / 2
    };
    if (canvas_size.x > win_size.x || canvas_size.y > win_size.y) {
        canvas_start.x -= s.scroll_x * s.zoom;
        canvas_start.y -= s.scroll_y * s.zoom;
    }
    return canvas_start;
}

void draw_background(struct state s, SDL_Surface* surface) {
    struct coord origin = {0, 0};
    struct coord size = {surface->w, surface->h};
    fill_rect(surface, origin, size, COLOR_GREY);
}

void draw_canvas(struct state s, SDL_Surface* surface, struct coord win_size) {
    struct coord canvas_size = {
        s.width * s.zoom,
        s.height * s.zoom
    };
    struct coord canvas_start = get_canvas_start(s, win_size);
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

struct state handle_motion(struct state s,
        struct coord c, struct coord win_size) {
    if (s.drawing) {
        struct coord canvas_start = get_canvas_start(s, win_size);
        c.x -= canvas_start.x;
        c.y -= canvas_start.y;
        c.x /= s.zoom;
        c.y /= s.zoom;
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
            s.zoom <<= 1;
            break;
        }
    } else {
        switch (key) {
        case SDLK_MINUS:
            if (s.zoom > 1) {
                s.zoom >>= 1;
            }
            break;
        case SDLK_k:
            s.scroll_y -= SCROLL_STEP;
            break;
        case SDLK_j:
            s.scroll_y += SCROLL_STEP;
            break;
        case SDLK_h:
            s.scroll_x -= SCROLL_STEP;
            break;
        case SDLK_l:
            s.scroll_x += SCROLL_STEP;
            break;
        case SDLK_x:
            s.color = !s.color;
            break;
        case SDLK_q:
            s.quit = true;
            break;
        }
    }
    return s;
}

struct state load_file(struct state s, char* filename) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        fprintf(stderr, "Unable to open %s\n", filename);
        exit(EXIT_FAILURE);
    }
    char res[64];
    fscanf(f, "%63s", res);
    if (strcmp(res, "P4") == 0) {
        fscanf(f, "%63s", res);
        s.width = atoi(res);
        fscanf(f, "%63s", res);
        s.height = atoi(res);
        fgetc(f);
        int unread = s.width * s.height;
        bool* canvasp = s.canvas;
        while (unread > 0) {
            char curr = fgetc(f);
            for (int i = 0; i < 8; i++) {
                canvasp[7-i] = curr & 1;
                curr >>= 1;
            }
            canvasp += 8;
            unread -= 8;
        }
    } else {
        fprintf(stderr, "%s is not a PBM file\n", filename);
        exit(EXIT_FAILURE);
    }
    printf(res);
    return s;
}

struct state parse_argv(struct state s, int argc, char* argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "w:h:")) != -1) {
        switch (opt) {
        case 'h':
            s.height = atoi(optarg);
            break;
        case 'w':
            s.width = atoi(optarg);
            break;
        default:
            fprintf(stderr, "Usage: %s [-w width] [-h height] name\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    if (optind < argc) {
        s = load_file(s, argv[optind]);
    }
    return s;
}

int main(int argc, char* argv[]) {
    struct state s = {
        .zoom       = INITIAL_ZOOM,
        .canvas     = canvas,
        .width      = INITIAL_CANVAS_WIDTH,
        .height     = INITIAL_CANVAS_HEIGHT,
        .color      = true,
        .scroll_x   = 0,
        .scroll_y   = 0
    };
    s = parse_argv(s, argc, argv);
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    } 
    SDL_Window* window = SDL_CreateWindow(WINDOW_NAME, SDL_WINDOWPOS_UNDEFINED,
                           SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT,
                           SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (window == NULL) {
        fprintf(stderr, "Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    while (true) {
        struct coord win_size;
        SDL_GetWindowSize(window, &win_size.x, &win_size.y);
        SDL_Event e;
        SDL_WaitEvent(&e);
        switch (e.type) {
        case SDL_QUIT:
            s.quit = true;
            break;
        case SDL_KEYDOWN:
            s = handle_keypress(s, e.key.keysym.sym, e.key.keysym.mod);
            break;
        case SDL_MOUSEBUTTONDOWN:
            s = handle_mousedown(s);
            s = handle_motion(s, (struct coord) {e.button.x,
                                                 e.button.y}, win_size);
            break;
        case SDL_MOUSEBUTTONUP:
            s = handle_mouseup(s);
            break;
        case SDL_MOUSEMOTION:
            s = handle_motion(s, (struct coord) {e.motion.x,
                                                 e.motion.y}, win_size);
            break;
        }
        if (s.quit) {
            break;
        }
        SDL_Surface* surface = SDL_GetWindowSurface(window);
        draw_background(s, surface);
        draw_canvas(s, surface, win_size);
        draw_ui(s, surface, win_size);
        SDL_UpdateWindowSurface(window);
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}


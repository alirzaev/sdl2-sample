#pragma once

#include <memory>
#include <SDL.h>

namespace sample::sdl {
    class window_deleter {
    public:
        void operator()(SDL_Window *win) {
            SDL_DestroyWindow(win);
        }
    };

    class renderer_deleter {
    public:
        void operator()(SDL_Renderer *ren) {
            SDL_DestroyRenderer(ren);
        }
    };

    using win_ptr = std::unique_ptr<SDL_Window, window_deleter>;
    using ren_ptr = std::unique_ptr<SDL_Renderer, renderer_deleter>;

    void clear_render(const sdl::ren_ptr &renderer) {
        SDL_SetRenderDrawColor(renderer.get(), 0xff, 0xff, 0xff, 0);
        SDL_RenderClear(renderer.get());
    }

    void set_render_color(const sdl::ren_ptr &renderer, int r, int g, int b, int a) {
        SDL_SetRenderDrawColor(renderer.get(), r, g, b, a);
    }
}
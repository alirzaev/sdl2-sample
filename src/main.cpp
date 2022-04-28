#include <iostream>
#include <limits>
#include <memory>
#include <vector>
#include <SDL.h>

#include "widget.h"

using namespace sample;

std::vector<symbol> create_digits() {
    std::vector<symbol> digits = {
            // symbol "0"
            symbol{
                    // first curve
                    bezier_curve{{
                                         {0, 90},
                                         {0, 0},
                                         {90, 0},
                                         {90, 90}
                                 }},
                    // second curve
                    bezier_curve{{
                                         {90, 90},
                                         {90, 180},
                                         {0, 180},
                                         {0, 90}
                                 }}
            },
            // symbol "1"
            symbol{
                    // first curve
                    bezier_curve{{
                                         {0, 90},
                                         {60, 75},
                                         {75, 60},
                                         {90, 0}
                                 }},
                    // second curve
                    bezier_curve{{
                                         {90, 180},
                                         {75, 120},
                                         {75, 60},
                                         {90, 0},
                                 }}
            }
    };

    int max_x = 0, max_y = 0;

    for (const auto &symbol: digits) {
        for (const auto &curve: symbol) {
            for (auto [x, y]: curve) {
                max_x = std::max(max_x, x);
                max_y = std::max(max_y, y);
            }
        }
    }

    for (auto &symbol: digits) {
        for (auto &curve: symbol) {
            for (auto &[x, y]: curve) {
                x -= max_x;
                y -= max_y;
            }
        }
    }

    return digits;
}

std::vector<symbol> make_transition_frames(const symbol &from, const symbol &to) {
    if (from == to) {
        return {11, from};
    }

    std::vector<symbol> frames(11);

    for (int i = 0; i < 11; ++i) {
        for (int j = 0; j < 2; ++j) {
            for (int p = 0; p < 4; ++p) {
                frames[i][j][p] = {
                        from[j][p].first + (to[j][p].first - from[j][p].first) * i / 10,
                        from[j][p].second + (to[j][p].second - from[j][p].second) * i / 10
                };
            }
        }
    }

    return frames;
}

template<class T>
std::vector<std::vector<T>> combine(const std::initializer_list<std::vector<T>> &il) {
    auto min_size = std::numeric_limits<typename std::vector<T>::size_type>::max();
    for (const auto &vec: il) {
        min_size = std::min(min_size, vec.size());
    }
    std::vector<std::vector<T>> combined;

    for (std::size_t i = 0; i < min_size; ++i) {
        std::vector<symbol> items;

        for (const auto &vec: il) {
            items.push_back(vec[i]);
        }

        combined.push_back(items);
    }

    return combined;
}

template<class T>
std::vector<T> concat(const std::initializer_list<std::vector<T>> &il) {
    typename std::vector<T>::size_type total_size = 0;

    for (const auto &vec: il) {
        total_size += vec.size();
    }

    std::vector<T> output;
    output.reserve(total_size);

    for (const auto &vec: il) {
        for (const auto &item: vec) {
            output.push_back(item);
        }
    }

    return output;
}

std::vector<std::vector<symbol>> create_frames() {
    const auto digits = create_digits();

    std::vector<symbol> trs[2][2] = {
            // "0" -> "0", "0" -> "1"
            {make_transition_frames(digits[0], digits[0]), make_transition_frames(digits[0], digits[1])},
            // "1" -> "0", "1" -> "1"
            {make_transition_frames(digits[1], digits[0]), make_transition_frames(digits[1], digits[1])}
    };

    return concat({
                          // "000" -> "001"
                          combine({
                                          trs[0][0], trs[0][0], trs[0][1]
                                  }),
                          // "001" -> "010"
                          combine({
                                          trs[0][0], trs[0][1], trs[1][0]
                                  }),
                          // "010" -> "011"
                          combine({
                                          trs[0][0], trs[1][1], trs[0][1]
                                  }),
                          // "011" -> "100"
                          combine({
                                          trs[0][1], trs[1][0], trs[1][0]
                                  }),
                          // "100" -> "101"
                          combine({
                                          trs[1][1], trs[0][0], trs[0][1]
                                  }),
                          // "101" -> "110"
                          combine({
                                          trs[1][1], trs[0][1], trs[1][0]
                                  }),
                          // "110" -> "111"
                          combine({
                                          trs[1][1], trs[1][1], trs[0][1]
                                  }),
                          // "111" -> "000"
                          combine({
                                          trs[1][0], trs[1][0], trs[1][0]
                                  }),
                  });
}

void main_loop() {
    sdl::win_ptr window(SDL_CreateWindow("sdl2-sample", 100, 100, 1024, 850, SDL_WINDOW_SHOWN));
    sdl::ren_ptr renderer(SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED));

    sdl::clear_render(renderer);
    sdl::set_render_color(renderer, 255, 0, 0, 0);

    widget w(create_frames());
    w.move({320, 240});

    bool stop = false;

    while (!stop) {
        SDL_Event e;
        SDL_WaitEventTimeout(&e, 2);

        if (e.type == SDL_KEYDOWN && e.key.state == SDL_PRESSED) {
            switch (e.key.keysym.sym) {
                case SDLK_LEFT:
                    w.move_relative({-10, 0});
                    break;
                case SDLK_RIGHT:
                    w.move_relative({10, 0});
                    break;
                case SDLK_UP:
                    w.move_relative({0, -10});
                    break;
                case SDLK_DOWN:
                    w.move_relative({0, 10});
                    break;
                case SDLK_PAGEDOWN:
                    w.rotate(5);
                    break;
                case SDLK_PAGEUP:
                    w.rotate(-5);
                    break;
                case SDLK_HOME:
                    w.scale_up(1.2);
                    break;
                case SDLK_END:
                    w.scale_down(1.2);
                    break;
                case SDLK_ESCAPE:
                    stop = true;
                    break;
            }
        }

        sdl::clear_render(renderer);
        sdl::set_render_color(renderer, 255, 0, 0, 0);
        w.draw(renderer);

        SDL_Delay(80);
    }
}

int main(int, char **) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    main_loop();

    SDL_Quit();
    return 0;
}
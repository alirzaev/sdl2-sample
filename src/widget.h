#pragma once

#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif

#include <array>
#include <cmath>
#include <utility>
#include <vector>
#include <SDL.h>

#include "sdl_helpers.h"
#include "transformation.h"

namespace sample {
    using bezier_curve = std::array<std::pair<int, int>, 4>;
    using symbol = std::array<bezier_curve, 2>;

    class widget {
    public:
        void scale_up(double s) {
            _scale *= s;
            _scale_tr = make_scale_transform(_scale, _center);
        }

        void scale_down(double s) {
            _scale /= s;
            _scale_tr = make_scale_transform(_scale, _center);
        }

        void move(const std::pair<int, int> &point) {
            _center = point;
            _move_tr = make_move_transform(_center);
        }

        void move_relative(const std::pair<int, int> &point) {
            _center.first += point.first;
            _center.second += point.second;
            _move_tr = make_move_transform(_center);
        }

        void rotate(unsigned int delta) {
            _angle = (_angle + delta + 360) % 360;
            _angle_tr = make_rotate_transform(_angle * 2 * M_PI / 360);
        }

        void draw(const sdl::ren_ptr &renderer) {
            const auto &frames = _symbol_frames[_counter];

            for (decltype(frames.size()) i = 0; i < frames.size(); ++i) {
                const auto bx = static_cast<int>(30 + i * 120);
                const auto by = 30;

                for (const auto &curve: frames[i]) {
                    draw_bezier_curve(renderer, curve, bx, by);
                }
            }
            SDL_RenderPresent(renderer.get());
            _counter = (_counter + 1) % _symbol_frames.size();
        }

        explicit widget(std::vector<std::vector<symbol>> symbol_frames) : _scale(1), _angle(0), _center({0, 0}),
                                                                          _counter(0),
                                                                          _symbol_frames(std::move(symbol_frames)) {
        }

    private:
        double _scale;
        unsigned int _angle;
        std::pair<int, int> _center;
        std::vector<std::vector<symbol>> _symbol_frames;
        transformation _move_tr, _scale_tr, _angle_tr;
        typename std::vector<std::vector<symbol>>::size_type _counter;

        static SDL_Point make_point(int x, int y) {
            SDL_Point p;
            p.x = x;
            p.y = y;
            return p;
        }

        static void draw_circle(const sdl::ren_ptr &ren, int x, int y, double r) {
            std::vector<SDL_Point> points;

            for (int i = 0; i <= 360; i += 36) {
                int x1 = static_cast<int>(x + r * std::cos(i * 2 * M_PI / 360));
                int y1 = static_cast<int>(y + r * std::sin(i * 2 * M_PI / 360));
                points.push_back(make_point(x1, y1));
            }

            SDL_RenderDrawLines(ren.get(), points.data(), static_cast<int>(points.size()));
            points.clear();
        }

        void draw_bezier_curve(const sdl::ren_ptr &ren, const bezier_curve &curve, int bx, int by) {
            auto round = [](double val) -> int { return (int) std::ceil(val - 0.5); };
            double t = 0, step = 0.05;

            const auto first_point = transformation::apply_chain(
                    {curve[0].first + bx, curve[0].second + by},
                    {_scale_tr, _angle_tr, _move_tr}
            );
            draw_circle(ren, first_point.first, first_point.second, 2 * _scale);

            while (t < 1) {
                const auto [p0_x, p0_y] = curve[0];
                const auto [p1_x, p1_y] = curve[1];
                const auto [p2_x, p2_y] = curve[2];
                const auto [p3_x, p3_y] = curve[3];

                const auto q0_x = p0_x + round(t * (p1_x - p0_x));
                const auto q0_y = p0_y + round(t * (p1_y - p0_y));
                const auto q1_x = p1_x + round(t * (p2_x - p1_x));
                const auto q1_y = p1_y + round(t * (p2_y - p1_y));
                const auto q2_x = p2_x + round(t * (p3_x - p2_x));
                const auto q2_y = p2_y + round(t * (p3_y - p2_y));

                const auto r0_x = q0_x + round(t * (q1_x - q0_x));
                const auto r0_y = q0_y + round(t * (q1_y - q0_y));
                const auto r1_x = q1_x + round(t * (q2_x - q1_x));
                const auto r1_y = q1_y + round(t * (q2_y - q1_y));

                const auto c_x = r0_x + round(t * (r1_x - r0_x));
                const auto c_y = r0_y + round(t * (r1_y - r0_y));

                t += step;
                auto [x, y] = transformation::apply_chain(
                        {c_x + bx, c_y + by},
                        {_scale_tr, _angle_tr, _move_tr}
                );
                draw_circle(ren, x, y, 2 * _scale);
            }
        }
    };
}
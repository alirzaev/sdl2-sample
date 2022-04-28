#pragma once

#include <cmath>
#include <initializer_list>
#include <utility>
#include <SDL.h>

namespace sample {
    class transformation {
    public:
        double a11, a12;
        [[maybe_unused]] double a13;
        double a21, a22;
        [[maybe_unused]] double a23;
        double a31, a32;
        [[maybe_unused]] double a33;

        transformation() :
                a11(1), a12(0), a13(0),
                a21(0), a22(1), a23(0),
                a31(0), a32(0), a33(1) {}

        [[nodiscard]] std::pair<int, int> apply(const std::pair<int, int> &point) const {
            auto [x, y] = point;
            return {(int) (a11 * x + a21 * y + a31), (int) (a12 * x + a22 * y + a32)};
        }

        [[nodiscard]] static std::pair<int, int>
        apply_chain(const std::pair<int, int> &point, const std::initializer_list<transformation> &il) {
            auto result = point;

            for (const auto &tr: il) {
                result = tr.apply(result);
            }

            return result;
        }
    };

    transformation make_move_transform(const std::pair<int, int> &point) {
        transformation tr;
        tr.a31 = point.first;
        tr.a32 = point.second;
        return tr;
    }

    transformation make_rotate_transform(double angle) {
        transformation tr;
        tr.a11 = std::cos(angle);
        tr.a12 = std::sin(angle);
        tr.a21 = -std::sin(angle);
        tr.a22 = std::cos(angle);
        return tr;
    }

    transformation make_scale_transform(double s, const std::pair<int, int> &center) {
        transformation tr;
        tr.a11 = s;
        tr.a22 = s;
        return tr;
    }
}

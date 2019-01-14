#pragma once
#include <SDL.h>
#include <fstream>
#include <functional>
#include <memory>
#include <vector>
#include <cmath>
#include <array>

class window_deleter {
public:
	void operator ()(SDL_Window *win) {
		SDL_DestroyWindow(win);
	}
};

class renderer_deleter {
public:
	void operator ()(SDL_Renderer *ren) {
		SDL_DestroyRenderer(ren);
	}
};

typedef std::unique_ptr < SDL_Window, window_deleter > win_ptr;
typedef std::unique_ptr < SDL_Renderer, renderer_deleter > ren_ptr;
typedef std::pair < int, int > pii;
typedef std::vector<std::vector<pii> > Digit;

class Counter {
public:
	std::array<pii, 3> Current() const {
		std::array<pii, 3> ret;
		ret[0] = pii(_prev % 2,     _prev % 2     == _current % 2     ? 0 : _frame);
		ret[1] = pii(_prev / 2 % 2, _prev / 2 % 2 == _current / 2 % 2 ? 0 : _frame);
		ret[2] = pii(_prev / 4 % 2, _prev / 4 % 2 == _current / 4 % 2 ? 0 : _frame);
		return ret;
	}
	void Next() {
		_frame = (_frame + 1) % 11;
		if (_frame == 0) {
			_prev = _current;
			++_current;
		}
	}
	void Initialize() {
		_current = 1;
		_prev = 0;
		_frame = 0;
	}
private:
	unsigned int _current;
	unsigned int _prev;
	unsigned int _frame;
};

class Transformation {
public:
	double a11, a12, a13;
	double a21, a22, a23;
	double a31, a32, a33;
	Transformation() : 
		a11(1), a12(0), a13(0),
		a21(0), a22(1), a23(0),
		a31(0), a32(0), a33(1) { }
	pii operator() (pii point) const {
		int x = point.first, y = point.second;
		return pii((int)(a11 * x + a21 * y + a31), (int)(a12 * x + a22 * y + a32));
	}
};

SDL_Point MakePoint(int x, int y) {
	SDL_Point p; p.x = x; p.y = y;
	return p;
}

SDL_Point MakePoint(pii point) {
	SDL_Point p; p.x = point.first; p.y = point.second;
	return p;
}

Transformation MakeMoveTranform(pii point) {
	Transformation tr;
	tr.a31 = point.first; tr.a32 = point.second;
	return tr;
}

Transformation MakeRotateTranform(double angle) {
	Transformation tr;
	tr.a11 = std::cos(angle);
	tr.a12 = std::sin(angle);
	tr.a21 = - std::sin(angle);
	tr.a22 = std::cos(angle);
	return tr;
}

Transformation MakeScaleTranform(double s, pii center) {
	Transformation tr;
	int x = center.first, y = center.second;
	tr.a11 = s;			 tr.a22 = s;
	//tr.a31 = -x * s + x; tr.a32 = -y * s + y;
	return tr;
}

class Widget {
public:
	void ScaleUp(double s) {
		_scale *= s;
		_scale_tr = MakeScaleTranform(_scale, _center);
	}
	void ScaleDown(double s) {
		_scale /= s;
		_scale_tr = MakeScaleTranform(_scale, _center);
	}
	void Move(pii point) {
		_center = point;
		_move_tr = MakeMoveTranform(_center);
	}
	void MoveRel(pii point) {
		_center.first += point.first;
		_center.second += point.second;
		_move_tr = MakeMoveTranform(_center);
	}
	void Rotate(unsigned int delta) {
		_angle = (_angle + delta + 360) % 360;
		_angle_tr = MakeRotateTranform(_angle * 2 * 3.14 / 360);
	}
	void Draw(const ren_ptr& renderer) {
		std::array<pii, 3> c = _counter.Current();
		for (int i = 0; i < 3; ++i) {
			_DrawSpline(renderer, _digit_frames[c[3 - i - 1].first][c[3 - i - 1].second][0], 30 + i * 120, 30);
			_DrawSpline(renderer, _digit_frames[c[3 - i - 1].first][c[3 - i - 1].second][1], 30 + i * 120, 30);
		}
		SDL_RenderPresent(renderer.get());
		_counter.Next();
	}
	void Initialize(const std::array<std::vector<Digit>, 2>& digit_frames) {
		_scale = 1;
		_angle = 0;
		_center = pii(0, 0);
		_digit_frames = digit_frames;
		_counter.Initialize();
	}
private:
	double		 _scale;
	unsigned int _angle;
	pii			 _center;
	std::array<std::vector<Digit>, 2> _digit_frames;
	Transformation _move_tr, _scale_tr, _angle_tr;
	Counter _counter;


	void _DrawCircle(const ren_ptr& ren, int x, int y, double r) {
		std::vector<SDL_Point> points;

		for (int i = 0; i <= 360; i += 36) {
			int x1 = x + r * std::cos(i * 6.28 / 360);
			int y1 = y + r * std::sin(i * 6.28 / 360);
			points.push_back(MakePoint(x1, y1));
		}
		
		SDL_RenderDrawLines(ren.get(), points.data(), points.size());
		points.clear();
		
	}

	void _DrawRect(const ren_ptr& renderer, int x1, int y1, int x2, int y2) {
		std::vector<SDL_Point> points;
		points.push_back(MakePoint(_move_tr(_angle_tr(_scale_tr(pii(x1, y1))))));
		points.push_back(MakePoint(_move_tr(_angle_tr(_scale_tr(pii(x2, y1))))));
		points.push_back(MakePoint(_move_tr(_angle_tr(_scale_tr(pii(x2, y2))))));
		points.push_back(MakePoint(_move_tr(_angle_tr(_scale_tr(pii(x1, y2))))));
		points.push_back(MakePoint(_move_tr(_angle_tr(_scale_tr(pii(x1, y1))))));
		SDL_RenderDrawLines(renderer.get(), points.data(), points.size());
	}

	void _DrawSpline(const ren_ptr& ren, const std::vector<pii>& points, int bx, int by) {
		auto round = [](double val) -> int { return (int)std::ceil(val - 0.5); };
		std::vector<pii> R = points, P = points, spline_points;
		int m = points.size();
		double t = 0, step = 0.05;

		pii current_point(P[0]);
		pii point = _move_tr(_angle_tr(_scale_tr(
			pii(current_point.first + bx, current_point.second + by))));
		_DrawCircle(ren, point.first, point.second, 2 * _scale);
		while (t < 1) {
			R = P;
			for (int j = m; j > 1; --j) {
				for (int i = 0; i < j - 1; ++i) {
					R[i].first = R[i].first + round(t * (R[i + 1].first - R[i].first));
					R[i].second = R[i].second + round(t * (R[i + 1].second - R[i].second));
				}
			}

			t += step;
			current_point = R[0];
			pii point = _move_tr(_angle_tr(_scale_tr(
				pii(current_point.first + bx, current_point.second + by))));
			_DrawCircle(ren, point.first, point.second, 2 * _scale);
		}
	}
};
#include <iostream>
#include <fstream>
#include <functional>
#include <memory>
#include <vector>
#include <cmath>
#include <SDL.h>
#include "CounterWidget.h"

std::vector<Digit> LoadDigits() {
	std::ifstream file("points");
	std::vector<Digit> digits(2);

	for (int k = 0; k < 2; ++k) {
		int n; file >> n;
		digits[k].resize(n);
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < 4; ++j) {
				int x, y; file >> x >> y;
				x -= 180, y -= 120;
				digits[k][i].emplace_back(x, y);
			}
		}
	}

	return digits;
}

std::vector<Digit> MakeFrames(const Digit& first, const Digit& second) {
	std::vector<Digit> frames(11);
	for (int i = 0; i < 11; ++i) {
		frames[i].resize(2);
		for (int j = 0; j < 2; ++j) {
			frames[i][j].resize(4);
			for (int p = 0; p < 4; ++p) {
				frames[i][j][p] = pii(
					first[j][p].first + (second[j][p].first - first[j][p].first) * i / 10,
					first[j][p].second + (second[j][p].second - first[j][p].second) * i / 10
					);
			}
		}
	}

	return frames;
}

void ClearRender(const ren_ptr& renderer) {
	SDL_SetRenderDrawColor(renderer.get(), 0xff, 0xff, 0xff, 0);
	SDL_RenderClear(renderer.get());
}

void SetRenderColor(const ren_ptr& renderer, int r, int g, int b, int a) {
	SDL_SetRenderDrawColor(renderer.get(), r, g, b, a);
}

bool WaitTimeout(int timeout, SDL_EventType type) {
	SDL_Event e;
	if (SDL_WaitEventTimeout(&e, timeout)) {
		if (e.type == type) {
			return true;
		}
	}
	return false;
}

void WaitUntill(SDL_EventType type) {
	SDL_Event e;
	SDL_WaitEvent(&e);
	while (e.type != type) {
		SDL_WaitEvent(&e);
	}
}

void MainLoop() {
	win_ptr window( SDL_CreateWindow("sdl2-sample", 100, 100, 1024, 850, SDL_WINDOW_SHOWN) );
	ren_ptr renderer(SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED));

	ClearRender(renderer);
	SetRenderColor(renderer, 255, 0, 0, 0);

	std::vector<Digit> digits = LoadDigits();
	std::array<std::vector<Digit>, 2> digit_frames = { MakeFrames(digits[0], digits[1]), MakeFrames(digits[1], digits[0]) };

	Widget w;
	w.Initialize(digit_frames);
	w.Move(pii(320, 240));
	bool stop = false;
	while (!stop) {
		SDL_Event e;
		SDL_WaitEventTimeout(&e, 2);
		if (e.type = SDL_KEYDOWN && e.key.state == SDL_PRESSED) {
			switch (e.key.keysym.sym) {
			case SDLK_LEFT:
				w.MoveRel(pii(-10, 0));
				break;
			case SDLK_RIGHT:
				w.MoveRel(pii(10, 0));
				break;
			case SDLK_UP:
				w.MoveRel(pii(0, -10));
				break;
			case SDLK_DOWN:
				w.MoveRel(pii(0, 10));
				break;
			case SDLK_PAGEDOWN:
				w.Rotate(5);
				break;
			case SDLK_PAGEUP:
				w.Rotate(-5);
				break;
			case SDLK_HOME:
				w.ScaleUp(1.2);
				break;
			case SDLK_END:
				w.ScaleDown(1.2);
				break;
			case SDLK_ESCAPE:
				stop = true;
				break;
			}
		}
		
		ClearRender(renderer);
		SetRenderColor(renderer, 255, 0, 0, 0);
		w.Draw(renderer);
		SDL_Delay(80);
	}
}

int main(int, char**) {
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return 1;
	}
	
	MainLoop();

	SDL_Quit();
	return 0;
}
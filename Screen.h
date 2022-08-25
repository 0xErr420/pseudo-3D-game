#pragma once

#include "SDL.h"

namespace graphics {

class Screen
{
public:
	Screen();
	bool init();
	void close();
	void update();
	void drawPixel(int x, int y, Uint8 red, Uint8 green, Uint8 blue);
	void drawFill(int x1, int y1, int x2, int y2, Uint8 red, Uint8 green, Uint8 blue);
	void setScreen(Uint8 red, Uint8 green, Uint8 blue);
	bool processEvents();
private:
	SDL_Window* m_window;
	SDL_Renderer* m_renderer;
	SDL_Texture* m_texture;
	Uint32* m_buffer1; // screen main buffer
	size_t mScreenWidth;	/* Using `size_t` type because it's better for array-	*/
	size_t mScreenHeight;	/* index manipulation (like `buffer[height * width]`)	*/
public:
	size_t ScreenWidth();
	size_t ScreenHeight();
private:
	void clip(int& x, int& y);
};

} /* namespace graphics */


#include "Screen.h"

#include <iostream>


graphics::Screen::Screen() :
	m_window(NULL), m_renderer(NULL), m_texture(NULL), m_buffer1(NULL), mScreenWidth(800), mScreenHeight(600) {

}

/* \returns - true if successful, else returns false */
bool graphics::Screen::init()
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)   // Check if failed, else continue 
        { SDL_Quit(); return false; }

    // Initialize window
    m_window = SDL_CreateWindow("SDL window", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, ScreenWidth(), ScreenHeight(), SDL_WINDOW_SHOWN);
    if (m_window == NULL)   // Check if failed, else continue
        { SDL_Quit(); return false; }

    // Initialize renderer
    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_PRESENTVSYNC);
    if (m_renderer == NULL) // Check if failed, else continue
        { SDL_DestroyWindow(m_window); SDL_Quit(); return false; }

    // Initialize texture
    m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, ScreenWidth(), ScreenHeight());
    if (m_texture == NULL)  // Check if failed, else continue
        { SDL_DestroyRenderer(m_renderer); SDL_DestroyWindow(m_window); SDL_Quit(); return false; }

    // Create screen buffer size of (width x height)
    m_buffer1 = new Uint32[ScreenWidth() * ScreenHeight()];
    // and set buffer to 0 (black)
    memset(m_buffer1, 0, ScreenWidth() * ScreenHeight() * sizeof(Uint32));

    return true; // All successful
}

void graphics::Screen::close()
{
    delete[] m_buffer1;
    SDL_DestroyTexture(m_texture);
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

void graphics::Screen::update()
{
    SDL_UpdateTexture(m_texture, NULL, m_buffer1, ScreenWidth() * sizeof(Uint32));
    SDL_RenderClear(m_renderer);
    SDL_RenderCopy(m_renderer, m_texture, NULL, NULL);
    SDL_RenderPresent(m_renderer);
}

void graphics::Screen::drawPixel(int x, int y, Uint8 red, Uint8 green, Uint8 blue)
{
    if (x >= 0 && x < ScreenWidth() && y >= 0 && y < ScreenHeight()) {
        Uint32 color = 0;

        color += red;
        color <<= 8;
        color += green;
        color <<= 8;
        color += blue;
        color <<= 8;
        color += 0xFF;

        m_buffer1[(y * ScreenWidth()) + x] = color;
    }
}

void graphics::Screen::drawFill(int x1, int y1, int x2, int y2, Uint8 red, Uint8 green, Uint8 blue)
{
    clip(x1, y1);
    clip(x2, y2);
    for (int x = x1; x < x2; x++)
        for (int y = y1; y < y2; y++)
            drawPixel(x, y, red, green, blue);

}

void graphics::Screen::setScreen(Uint8 red, Uint8 green, Uint8 blue)
{
    Uint32 color = 0;

    color += red;
    color <<= 8;
    color += green;
    color <<= 8;
    color += blue;
    color <<= 8;
    color += 0xFF;

    for (int y = 0; y < Screen::ScreenHeight(); y++) {
        for (int x = 0; x < Screen::ScreenWidth(); x++) {
            m_buffer1[(y * ScreenWidth()) + x] = color;
        }
    }
}

bool graphics::Screen::processEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT) {
            return false;
        }
    }
    return true;
}

size_t graphics::Screen::ScreenWidth()
{
    return mScreenWidth;
}

size_t graphics::Screen::ScreenHeight()
{
    return mScreenHeight;
}

void graphics::Screen::clip(int& x, int& y)
{
    if (x < 0) x = 0;
    if (x >= mScreenWidth) x = mScreenWidth;
    if (y < 0) y = 0;
    if (y >= mScreenHeight) y = mScreenHeight;
}

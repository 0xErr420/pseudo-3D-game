#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>

#include "SDL.h"
#include "Screen.h"

using namespace graphics;

/* - - - - - - - - - - - - Global variables - - - - - - - - - - */

Screen screen;

float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerDirection = 0.0f;

enum key { left, right, up, forward=2, down, backward=3, z, x }; // up/forward and down/backward are the same
bool keystate[6]; // indexes: 0-left, 1-right, 2-forward, 3-backward, 4-z, 5-x

size_t nMapWidth = 16;
size_t nMapHeight = 16;

float fFOV = 3.14159f / 3.0f;
float fDepth = 16.0f;

bool processEvents(float& fElapsedTime);


int main(int argc, char* args[])
{

    if (screen.init() == false) // Check if failed, else continue
        { std::cout << "Error: SDL init failed: " << SDL_GetError() << std::endl; }
    
    // Initialize map buffer
    std::string map;
    // Push map into buffer
    map += "################";
    map += "#              #";
    map += "#    #         #";
    map += "#    #      ####";
    map += "#           #  #";
    map += "#           #  #";
    map += "#     ###   #  #";
    map += "#     ####### ##";
    map += "#      #    #  #";
    map += "#           #  #";
    map += "#           #  #";
    map += "#           #  #";
    map += "#           #  #";
    map += "#              #";
    map += "#              #";
    map += "################";

    // Initialize timestamps (for delta time)
    auto tp1 = std::chrono::system_clock::now();
    auto tp2 = std::chrono::system_clock::now();
    

    while (true)
    {
        // Get 'delta time' (how many milliseconds passed)
        tp2 = std::chrono::system_clock::now();
        std::chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();

        /* - - - - - - - - - - PROCESS EVENTS - - - - - - - - - - */

            // Movement controls
        if (processEvents(fElapsedTime) == false) break;
        if (keystate[left]) { fPlayerDirection -= 2.0f * fElapsedTime; }
        if (keystate[right]) { fPlayerDirection += 2.0f * fElapsedTime; }
        if (keystate[forward]) { 
            fPlayerX += sinf(fPlayerDirection) * 6.0f * fElapsedTime;
            fPlayerY += cosf(fPlayerDirection) * 6.0f * fElapsedTime;
            // Collision detection
            if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
                // Simply cancels previous two lines
                fPlayerX -= sinf(fPlayerDirection) * 6.0f * fElapsedTime;
                fPlayerY -= cosf(fPlayerDirection) * 6.0f * fElapsedTime;
            }
        }
        if (keystate[backward]) { 
            fPlayerX -= sinf(fPlayerDirection) * 6.0f * fElapsedTime;
            fPlayerY -= cosf(fPlayerDirection) * 6.0f * fElapsedTime;
            // Collision detection
            if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
                // Simply cancels previous two lines
                fPlayerX += sinf(fPlayerDirection) * 6.0f * fElapsedTime;
                fPlayerY += cosf(fPlayerDirection) * 6.0f * fElapsedTime;
            }
        }
            // FOV controls
        if (keystate[z]) { fFOV += 0.1f; }
        if (keystate[x]) { fFOV -= 0.1f; }

        /* - - - - - - - - - - RENDER - - - - - - - - - - */

        // Loop for each column on the screen
        for (int x = 0; x < screen.ScreenWidth(); x++) {
            // Calculate the projected ray angle into world space
            // NOTE: Divide field-of-view angle into two parts and then chop it to little bits (amount of ScreenWidth)
            float fRayAngle = (fPlayerDirection - fFOV / 2.0f) + ((float)x / (float)screen.ScreenWidth()) * fFOV;

            float fDistanceToWall = 0;
            bool bHitWall = false;
            bool bBoundary = false;

            float fEyeX = sinf(fRayAngle);
            float fEyeY = cosf(fRayAngle);

            // Ray tracing to calculate distance to wall
            while (!bHitWall && fDistanceToWall < fDepth) {
                fDistanceToWall += 0.1f;

                int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
                int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

                if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
                {
                    bHitWall = true;
                    fDistanceToWall = fDepth;
                }
                else
                {
                    // NOTE: Ray is inbounds so test to see if the ray cell is a wall block
                    if (map[nTestY * nMapWidth + nTestX] == '#') {
                        bHitWall = true;

                        // Some code to detect bounds
                        std::vector<std::pair<float, float>> p;
                        for (int tx = 0; tx < 2; tx++) {
                            for (int ty = 0; ty < 2; ty++) {
                                float vy = (float)nTestY + ty - fPlayerY;
                                float vx = (float)nTestX + tx - fPlayerX;
                                float d = sqrt(vx * vx + vy * vy);
                                float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
                                p.push_back(std::make_pair(d, dot));
                            }
                        }
                        // Sort pairs from closest to farthest (lambda function used)
                        std::sort(p.begin(), p.end(), 
                            [](const std::pair<float, float>& left, const std::pair<float, float>& right) {return left.first < right.first; }
                        );

                        float fBound = 0.01f;
                        if (acos(p.at(0).second) < fBound) bBoundary = true;
                        if (acos(p.at(1).second) < fBound) bBoundary = true;
                        //if (acos(p.at(2).second) < fBound) bBoundary = true;
                    }
                }
            }

            int nCeiling = (float)(screen.ScreenHeight() / 2) - screen.ScreenHeight() / ((float)fDistanceToWall);
            int nFloor = screen.ScreenHeight() - nCeiling;

            Uint8 red = 0;
            Uint8 green = 0;
            Uint8 blue = 0;

            // Draw pixel
            for (int y = 0; y < screen.ScreenHeight(); y++) {
                // Draw the ceiling
                if (y < nCeiling) { // shade of red color
                    screen.drawPixel(x, y, 0x44, 0x00, 0x00);
                }
                // Draw the walls
                else if (y > nCeiling && y <= nFloor) {
                    if (fDistanceToWall <= fDepth / 4.0f) { red = 255; green = 255; blue = 255; }
                    else if (fDistanceToWall < fDepth / 3.0f) { red = 192; green = 192; blue = 192; }
                    else if (fDistanceToWall < fDepth / 2.0f) { red = 129; green = 129; blue = 129; }
                    else if (fDistanceToWall < fDepth) { red = 66; green = 66; blue = 66; }
                    else { red = 0; green = 0; blue = 0; }
                    // Or draw the bound on wall
                    if (bBoundary) { red = 0; green = 0; blue = 0; }

                    screen.drawPixel(x, y, red, green, blue);
                }
                // Draw the floor
                else {  // shades of blue
                    float b = 1.0f - (((float)y - screen.ScreenHeight() / 2.0f) / ((float)screen.ScreenHeight() / 2.0f));
                    if (b < 0.25) { red = 0; green = 0; blue = 255; }
                    else if (b < 0.5) { red = 0; green = 0; blue = 192; }
                    else if (b < 0.75) { red = 0; green = 0; blue = 129; }
                    else if (b < 0.9) { red = 0; green = 0; blue = 66; }
                    else { red = 0; green = 0; blue = 0; }

                    screen.drawPixel(x, y, red, green, blue);
                }
            }
        }

        Uint8 red = 0;
        Uint8 green = 0;
        Uint8 blue = 0;
        int size = 10;

        // Display minimap
        for (int nx = 0; nx < nMapWidth; nx++)
            for (int ny = 0; ny < nMapHeight; ny++) {

                if (map[ny * nMapWidth + nx] == '#') { red = 0; green = 66; blue = 0; }
                if (map[ny * nMapWidth + nx] == ' ') { red = 0; green = 33; blue = 0; } 
                
                int x1 = nx;
                x1 *= size;
                int y1 = ny;
                y1 *= size;

                int x2 = x1 + size;
                int y2 = y1 + size;

                screen.drawFill(x1, y1, x2, y2, red, green, blue);
                screen.drawPixel(x1, y1, 66, 129, 66);
            }

        // Display player on minimap
        screen.drawFill((int)fPlayerX * size, (int)fPlayerY * size, (int)fPlayerX * size + size, (int)fPlayerY * size + size, 0xFF, 0x00, 0x00);


        screen.update();
    }
    screen.close();

    return 0;
}

bool processEvents(float &fElapsedTime)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) { return false; }
        /* Look for a keypress */
        else if (event.type == SDL_KEYDOWN) {
            /* Check the SDLKey values and move change the coords */
            switch (event.key.keysym.sym) {
                // Arrow keys
            case SDLK_LEFT: keystate[left] = true; break;
            case SDLK_RIGHT: keystate[right] = true; break;
            case SDLK_UP: keystate[up] = true; break;
            case SDLK_DOWN: keystate[down] = true; break;
                // WASD keys
            case SDLK_a: keystate[left] = true; break;
            case SDLK_d: keystate[right] = true; break;
            case SDLK_w: keystate[up] = true; break;
            case SDLK_s: keystate[down] = true; break;

            case SDLK_z: keystate[z] = true; break;
            case SDLK_x: keystate[x] = true; break;
            default: break;
            }
        }
        /* Look for key unpress */
        else if (event.type == SDL_KEYUP) {
            switch (event.key.keysym.sym) {
                // Arrow keys
            case SDLK_LEFT: keystate[left] = false; break;
            case SDLK_RIGHT: keystate[right] = false; break;
            case SDLK_UP: keystate[up] = false; break;
            case SDLK_DOWN: keystate[down] = false; break;
                // WASD keys
            case SDLK_a: keystate[left] = false; break;
            case SDLK_d: keystate[right] = false; break;
            case SDLK_w: keystate[up] = false; break;
            case SDLK_s: keystate[down] = false; break;

            case SDLK_z: keystate[z] = false; break;
            case SDLK_x: keystate[x] = false; break;
            default: break;
            }
        }
    }
    return true;
}

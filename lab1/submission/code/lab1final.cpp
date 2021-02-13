// Introduction lab that covers:
// * C++
// * SDL
// * 2D graphics
// * Plotting pixels
// * Video memory
// * Color representation
// * Linear interpolation
// * glm::vec3 and std::vector

// *neode.command* setgo gcc -O3 -I/usr/include/SDL -I/usr/include/glm -lstdc++ -lm -lSDL lab1final.cpp -o lab1final && ./lab1final

#include "SDL.h"
#include <iostream>
#include <glm/glm.hpp>
#include <vector>
#include "SDLauxiliary.h"

using namespace std;
using glm::ivec2;
using glm::vec2;
using glm::vec3;

// --------------------------------------------------------
// GLOBAL VARIABLES

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
SDL_Surface* screen;

vector<vec3> stars(1000);

unsigned int t;
float velocity = 0.0005f;

float focalLength = float(SCREEN_HEIGHT)/2.0f;

// --------------------------------------------------------
// FUNCTION DECLARATIONS

void Draw();

// --------------------------------------------------------
// FUNCTION DEFINITIONS

void interpolate(float a, float b, vector<float>& result)
{
    int size = result.size();
    float dx = (b-a)/max(1.0f, float(size - 1));
    
    float val = a;
    
    for (int i = 0; i < size; ++i)
    {
        result[i] = val;
        val += dx;
    }
}

void interpolate(ivec2 a, ivec2 b, vector<ivec2>& result)
{
    int size = result.size();
    
    vec2 step = vec2(b - a) / max(1.0f, float(size - 1));
    vec2 current = a;
    
    for (int i = 0; i < size; ++i)
    {
        result[i] = current;
        current += step;
    }
}

void interpolate(vec3 a, vec3 b, vector<vec3>& result)
{
    int size = result.size();
    
    vec3 step = (b - a) / max(1.0f, float(size - 1));
    vec3 current = a;
    
    for (int i = 0; i < size; ++i)
    {
        result[i] = current;
        current += step;
    }
}

void update()
{
    unsigned int t2 = SDL_GetTicks();
    float dt = float(t2 - t);
    
    for (vec3& v : stars)
    {
        v.z = v.z - velocity * dt;
        
        if (v.z < 0.01f)
        {
            v.z += 1;
        }
    }
    
    t = t2;
}

int main( int argc, char* argv[] )
{
    for (int i = 0; i < stars.size(); ++i)
    {
        float x = 3*(float(rand()) / float(RAND_MAX)) - 1.5f;
        float y = 2*(float(rand()) / float(RAND_MAX)) - 1.f;
        float z = float(rand()) / float(RAND_MAX);
        
        stars[i] = vec3(x,y,z);
    }
    
	screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );
    
    t = SDL_GetTicks();
    
	while( NoQuitMessageSDL() )
	{
        update();
		Draw();
	}
	SDL_SaveBMP( screen, "screenshot.bmp" );
	return 0;
}

void Draw()
{
	SDL_FillRect(screen, 0, 0);
    
	if( SDL_MUSTLOCK(screen) )
		SDL_LockSurface(screen);
    
    for (vec3& v : stars)
    {
        vec3 color = min(vec3(1,1,1), vec3(1,1,1) / ((2*v.z)*(2*v.z)));
        
        ivec2 p1(focalLength * (v.x/(v.z + 0.02f)) + SCREEN_WIDTH/2,
                 focalLength * (v.y/(v.z + 0.02f)) + SCREEN_HEIGHT/2);
        
        ivec2 p2(focalLength * (v.x/v.z) + SCREEN_WIDTH/2,
                 focalLength * (v.y/v.z) + SCREEN_HEIGHT/2);
        
        int pixels = min(200, max(1, max(abs(p1.x - p2.x), abs(p1.y - p2.y))));
        
        if (pixels > 2)
        {
            vector<ivec2> line(pixels);
            interpolate(p1, p2, line);
            
            vector<vec3> linecolor(pixels);
            interpolate(vec3(0,0,0), color, linecolor);
            
            for (int i = 0; i < line.size(); ++i)
            {
                PutPixelSDL(screen, line[i].x, line[i].y, linecolor[i]);
                PutPixelSDL(screen, line[i].x+1, line[i].y, linecolor[i]);
                PutPixelSDL(screen, line[i].x, line[i].y+1, linecolor[i]);
                PutPixelSDL(screen, line[i].x+1, line[i].y+1, linecolor[i]);
            }
        }
        else
        {
            PutPixelSDL(screen, p2.x, p2.y, color);
            PutPixelSDL(screen, p2.x+1, p2.y, color);
            PutPixelSDL(screen, p2.x, p2.y+1, color);
            PutPixelSDL(screen, p2.x+1, p2.y+1, color);
        }
    }
    
	if( SDL_MUSTLOCK(screen) )
		SDL_UnlockSurface(screen);

	SDL_UpdateRect( screen, 0, 0, 0, 0 );
    
    SDL_Delay(16);
}

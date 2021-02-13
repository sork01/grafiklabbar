// Introduction lab that covers:
// * C++
// * SDL
// * 2D graphics
// * Plotting pixels
// * Video memory
// * Color representation
// * Linear interpolation
// * glm::vec3 and std::vector

// *neode.command* setgo gcc -O3 -I/usr/include/SDL -I./glm -lstdc++ -lm -lSDL starfield.cpp -o starfield; ./starfield

#include "SDL.h"
#include <iostream>
#include <glm/glm.hpp>
#include <vector>
#include "SDLauxiliary.h"

using namespace std;
using glm::vec3;

// --------------------------------------------------------
// GLOBAL VARIABLES

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
SDL_Surface* screen;

vector<vec3> stars(2000);

unsigned int t;
float velocity = 0.0001f;
float phase = 0.0f;

float focalLength = float(SCREEN_HEIGHT)/4.0f;

float zbuffer[SCREEN_WIDTH * SCREEN_HEIGHT];

// --------------------------------------------------------
// FUNCTION DECLARATIONS

void Draw();

// --------------------------------------------------------
// FUNCTION DEFINITIONS

void interpolate(float a, float b, vector<float>& result)
{
    int size = result.size();
    float dx = (b-a)/((float) (size - 1));
    // printf("dx: %.3f\n", dx);
    
    float val = a;
    
    for (int i = 0; i < size; ++i)
    {
        result[i] = val;
        val += dx;
    }
}

void interpolate(vec3 a, vec3 b, vector<vec3>& result)
{
    int size = result.size();
    
    float dx = (b.x - a.x)/((float) (size - 1));
    float dy = (b.y - a.y)/((float) (size - 1));
    float dz = (b.z - a.z)/((float) (size - 1));
    
    // printf("dx: %.3f\n", dx);
    // printf("dy: %.3f\n", dy);
    // printf("dz: %.3f\n", dz);
    
    float x = a.x;
    float y = a.y;
    float z = a.z;
    
    for (int i = 0; i < size; ++i)
    {
        result[i].x = x;
        result[i].y = y;
        result[i].z = z;
        
        x += dx;
        y += dy;
        z += dz;
    }
}

void update()
{
    unsigned int t2 = SDL_GetTicks();
    float dt = float(t2 - t);
    
    for (vec3& v : stars)
    {
        v.z = v.z - velocity * dt;
        
        if (v.z < 0)
        {
            v.z += 1;
        }
        
        // float x = v.x * cos(0.1) + v.y*-sin(0.1);
        // float y = v.x * sin(0.1) +  v.y*cos(0.1);
        
        // v.x = x;
        // v.y = y;
        
        // phase += 0.1;
    }
    
    t = t2;
}

int main( int argc, char* argv[] )
{
    for (int i = 0; i < stars.size(); ++i)
    {
        float x = 2*(float(rand()) / float(RAND_MAX)) - 1;
        float y = 2*(float(rand()) / float(RAND_MAX)) - 1;
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
    memset(zbuffer, 0, sizeof(zbuffer));
    
	SDL_FillRect(screen, 0, 0);
    
	if( SDL_MUSTLOCK(screen) )
		SDL_LockSurface(screen);
    
    for (vec3 v : stars)
    {
        vec3 color = 0.2f * vec3(1,1,1) / (v.z*v.z);
        
        // PutPixelSDL(screen,
            // focalLength * (v.x/v.z) + SCREEN_WIDTH/2,
            // focalLength * (v.y/v.z) + SCREEN_HEIGHT/2, color);
        
        for (int i = 20; i >= 0; --i)
        {
            vec3 color2;
            color2.x = color.x / float(i+1);
            color2.y = color.y / float(i+1);
            color2.z = color.z / float(i+1);
            
            float z = (v.z + velocity * i);
            
            float x = focalLength * (v.x/z) + SCREEN_WIDTH/2;
            float y = focalLength * (v.y/z) + SCREEN_HEIGHT/2;
            
            // printf("%.2f")
            
            // int pixel = y * SCREEN_WIDTH + x;
            
            // if (zbuffer[4*pixel] == 0.0f || zbuffer[4*pixel] > z)
            // {
                PutPixelSDL(screen, x, y, color2);
                PutPixelSDL(screen, x+1, y, color2);
                PutPixelSDL(screen, x, y+1, color2);
                PutPixelSDL(screen, x+1, y+1, color2);
                
                // zbuffer[pixel] = z;
            // }
        }
    }
    
	if( SDL_MUSTLOCK(screen) )
		SDL_UnlockSurface(screen);

	SDL_UpdateRect( screen, 0, 0, 0, 0 );
    
    SDL_Delay(16);
}
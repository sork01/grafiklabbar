// Introduction lab that covers:
// * C++
// * SDL
// * 2D graphics
// * Plotting pixels
// * Video memory
// * Color representation
// * Linear interpolation
// * glm::vec3 and std::vector

// *neode.command* setgo gcc -I/usr/include/SDL bilinear.cpp -lstdc++ -lm -lSDL -o bilinear;./bilinear

#include "SDL.h"
#include <iostream>
#include <glm/glm.hpp>
#include <vector>
#include "SDLauxiliary.h"

using namespace std;
using glm::vec3;

// --------------------------------------------------------
// GLOBAL VARIABLES

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
SDL_Surface* screen;

vec3 topLeft(1,0,0);
vec3 topRight(0,0,1);
vec3 bottomLeft(0,1,0);
vec3 bottomRight(1,1,0);

vector<vec3> leftSide(SCREEN_HEIGHT);
vector<vec3> rightSide(SCREEN_HEIGHT);

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

int main( int argc, char* argv[] )
{
    interpolate(topLeft, bottomLeft, leftSide);
    interpolate(topRight, bottomRight, rightSide);
    
	screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );
	while( NoQuitMessageSDL() )
	{
		Draw();
	}
	SDL_SaveBMP( screen, "screenshot.bmp" );
	return 0;
}

void Draw()
{
	for( int y=0; y<SCREEN_HEIGHT; ++y )
	{
        vector<vec3> pixels(SCREEN_WIDTH);
        interpolate(leftSide[y], rightSide[y], pixels);
        
		for( int x=0; x<SCREEN_WIDTH; ++x )
		{
			PutPixelSDL(screen, x, y, pixels[x]);
		}
	}

	if( SDL_MUSTLOCK(screen) )
		SDL_UnlockSurface(screen);

	SDL_UpdateRect( screen, 0, 0, 0, 0 );
}
// Introduction lab that covers:
// * C++
// * SDL
// * 2D graphics
// * Plotting pixels
// * Video memory
// * Color representation
// * Linear interpolation
// * glm::vec3 and std::vector

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
vector<vec3> stars( 1000 );
long t;
float v = 0.001; 
// --------------------------------------------------------
// FUNCTION DECLARATIONS

void Draw();
void Update()
{
    long t2 = SDL_GetTicks();
    float dt = float(t2-t);
    printf("%d = t ",t);
    printf("%d = t2 ",t2);
    printf("%f = dt ",dt);

    for( int s=0; s<stars.size(); ++s )
    {
        stars[s].z = stars[s].z - v * dt;
        // Add code for update of stars
        if( stars[s].z <= 0 )
            stars[s].z += 1;
        if( stars[s].z > 1 )
            stars[s].z -= 1;
    }
    t = t2;
}

// --------------------------------------------------------
// FUNCTION DEFINITIONS

int main( int argc, char* argv[] )
{
    for( int i = 0; i<stars.size(); ++i)
    {
        float z = float(rand()) / float(RAND_MAX);
        float x = ((float(rand()) / float(RAND_MAX))*2)-1;
        float y = ((float(rand()) / float(RAND_MAX))*2)-1;
        stars[i].z = z;
        stars[i].x = x;
        stars[i].y = y;
        
    }
	screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );
    t = SDL_GetTicks();
	while( NoQuitMessageSDL() )
	{
        Update();
		Draw();
	}
	SDL_SaveBMP( screen, "screenshot.bmp" );
	return 0;
}

float focal = float(SCREEN_WIDTH) / 2;

void Draw()
{
    SDL_FillRect( screen, 0, 0 );
    

	if( SDL_MUSTLOCK(screen) )
		SDL_UnlockSurface(screen);
    vec3 color = vec3(1,1,1);
    for( size_t s=0; s<stars.size(); ++s )
    {
        int x = (focal * stars[s].x / stars[s].z) + SCREEN_WIDTH / 2;
        int y = (focal * stars[s].y / stars[s].z) + SCREEN_HEIGHT / 2;
        PutPixelSDL(screen, x, y, color);
    }
	SDL_UpdateRect( screen, 0, 0, 0, 0 );
}
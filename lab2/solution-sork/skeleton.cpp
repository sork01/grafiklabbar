#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"

using namespace std;
using glm::vec3;
using glm::mat3;

// ----------------------------------------------------------------------------
// GLOBAL VARIABLES

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
SDL_Surface* screen;
int t;
float focallength = SCREEN_HEIGHT / 2.0f;
float yaw;

vec3 cameraPos(0.0f,0.0f,-2.0f);
mat3 rotation(vec3(1.0f,0.0f,0.0f), vec3(0.0f,1.0f,0.0f), vec3(0.0f,0.0f,1.0f));
vec3 lightPos( 0, -0.5, -0.7 );
vec3 lightColor = 14.f * vec3( 1, 1, 1 );

struct Intersection
{
    vec3 position;
    float distance;
    int triangleIndex;
};

vector<Triangle> triangles;

vec3 indirectLight = 0.5f*vec3( 1, 1, 1 );

// ----------------------------------------------------------------------------
// FUNCTIONS

void Update();
void Draw();

vec3 DirectLight( const Intersection& i );

bool ClosestIntersection(
            vec3 start,
            vec3 dir,
            const vector<Triangle>& triangles,
            Intersection& closestIntersection)
{
    bool found = false;
    closestIntersection.distance = std::numeric_limits<float>::max();
    for (int i =  0; i < triangles.size(); ++i) 
    {
        vec3 v0 = triangles[i].v0;
        vec3 v1 = triangles[i].v1;
        vec3 v2 = triangles[i].v2;
        vec3 e1 = v1 - v0;
        vec3 e2 = v2 - v0;
        vec3 b = start - v0;
        mat3 A( -dir, e1, e2 );
        vec3 res = glm::inverse( A ) * b;
        if (res.x >= 0 && res.y >= 0 && res.z >= 0 && res.y + res.z <= 1)
        {
            float dist = glm::length(res.x * dir);
            
            if (dist < closestIntersection.distance)
            {
                found = true;
                closestIntersection.distance = dist;
                closestIntersection.position = start + res.x*dir;
                closestIntersection.triangleIndex = i;
                // cout << "triangleindex i closeint = " << closestIntersection.triangleIndex << " distance = " << closestIntersection.distance << endl; 
                // cout << "i close " << &closestIntersection << endl;
            }
        }
    }
    return found;
}

int main( int argc, char* argv[] )
{
	screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );
	t = SDL_GetTicks();	// Set start value for timer.
    LoadTestModel(triangles);
    // Triangle(vec3 (0,-1,1),
    // vec3 (1,1,1),
    // vec3 (-1,1,1),
    // vec3 (1,1,1));
    // vec3 start(0,0,0);
    // vec3 dir(0,0,1);
    // Intersection test;
    // bool result = ClosestIntersection(start, dir, triangles, test);
    // cout << "result = " << result << endl;
    // if (test.triangleIndex != 10948 )
    // {
        // cout << "tindex = " << test.triangleIndex << " distance = " << test.distance << endl; 
    // }

	while( NoQuitMessageSDL() )
	{
		Update();
		Draw();
	}

	SDL_SaveBMP( screen, "screenshot.bmp" );
	return 0;
}

void Update()
{
	// Compute frame time:
	int t2 = SDL_GetTicks();
	float dt = float(t2-t);
	t = t2;
	cout << "Render time: " << dt << " ms." << endl;
    
    Uint8* keystate = SDL_GetKeyState( 0 );
    if( keystate[SDLK_UP] )
    {
        // Move camera forward
        cameraPos = cameraPos + vec3(0.0f,0.0f,1.0f);
    }
    if( keystate[SDLK_DOWN] )
    {
        // Move camera backward
        cameraPos = cameraPos + vec3(0.0f,0.0f,-1.0f);
    }
    if( keystate[SDLK_LEFT] )
    {
        // Rotate camera to the left
        yaw += 0.1f;
        rotation = mat3(vec3(cos(yaw), 0.0f, sin(yaw)), vec3(0.0f, 1.0f, 0.0f), vec3(-sin(yaw), 0.0f, cos(yaw)));
    }
    if( keystate[SDLK_RIGHT] )
    {
        // Rotate camera to the right
        yaw -= 0.1f;
        rotation = mat3(vec3(cos(yaw), 0.0f, sin(yaw)), vec3(0.0f, 1.0f, 0.0f), vec3(-sin(yaw), 0.0f, cos(yaw)));       
    }
    if( keystate[SDLK_w] )
    {
        lightPos += vec3(0.0f,0.0f,0.1f);
    }
    
    if( keystate[SDLK_s] )
    {
        lightPos += vec3(0.0f,0.0f,-0.1f);
    }
    
    if( keystate[SDLK_a] )
    {
        lightPos += vec3(-0.1f,0.0f,0.0f);
    }
    
    if( keystate[SDLK_d] )
    {
        lightPos += vec3(0.1f,0.0f,0.0f);
    }
    
    if( keystate[SDLK_i] )
    {
        lightPos += vec3(0.0f,-0.1f,0.0f);
    }
    
    if( keystate[SDLK_k] )
    {
        lightPos += vec3(0.0f,0.1f,0.0f);
    }
}

void Draw()
{
	if( SDL_MUSTLOCK(screen) )
		SDL_LockSurface(screen);
    
    Intersection colli;

	for( int y=0; y<SCREEN_HEIGHT; ++y   )
	{
		for( int x=0; x<SCREEN_WIDTH; ++x )
		{
            
            vec3 color( 0, 0, 0 );
            vec3 d(x - SCREEN_WIDTH / 2, y - SCREEN_HEIGHT / 2, focallength);
            d = rotation * d;
            if(ClosestIntersection(cameraPos, d, triangles, colli))
            {
                // cout << "triangleindex i colli = " << colli.triangleIndex << endl;
                // cout << "i colli " << &colli << endl;
                color = triangles[colli.triangleIndex].color * (DirectLight(colli) + indirectLight);
            }
			PutPixelSDL( screen, x, y, color );
		}
	}

	if( SDL_MUSTLOCK(screen) )
		SDL_UnlockSurface(screen);

	SDL_UpdateRect( screen, 0, 0, 0, 0 );
}

vec3 DirectLight( const Intersection& i )
{
    vec3 normal = triangles[i.triangleIndex].normal;
    
    vec3 r = (lightPos - i.position);
    float distance = glm::distance(lightPos, i.position);
    vec3 b = lightColor / (4*3.14159f*distance*distance);
    // float dotprod = glm::dot(normal, r);
    float dotprod = (normal.x*r.x+normal.y*r.y+normal.z*r.z);
    vec3 d = b * dotprod;
    if (dotprod < 0)
    {
        dotprod = 0;
    }
    Intersection colli;
    if (ClosestIntersection(i.position + 0.001f*r, r, triangles, colli))
    {
        float newdist = glm::distance(colli.position, i.position);
        if (newdist < distance)
        {
            return vec3(0,0,0);
        }
    }
    return d;
}
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

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
SDL_Surface* screen;
int t;

vector<Triangle> triangles;
float focal_length = 480.0f;
vec3 camera_pos(0.0f,0.0f,-2.00f);

// ----------------------------------------------------------------------------
// FUNCTIONS

void Update();
void Draw();

struct intersection
{
    vec3 position;
    float distance;
    int triangle_index;
};

bool findClosestIntersection(vec3 start, vec3 dir, vector<Triangle> triangles, intersection& result)
{
    bool wasfound = false;
    result.distance = std::numeric_limits<float>::max();
    
    for (int i = 0; i < triangles.size(); ++i)
    {
        vec3 e1 = triangles[i].v1 - triangles[i].v0;
        vec3 e2 = triangles[i].v2 - triangles[i].v0;
        vec3 b = start - triangles[i].v0;
        mat3 A(-dir, e1, e2);
        vec3 x = glm::inverse(A) * b;
        
        // printf("x = (%.2f, %.2f, %.2f)\n", x.x, x.y, x.z);
        
        // inside triangle: x.y >= 0, x.z >= 0, x.y+x.z <= 1
        
        if (x.x < 0 || x.y < 0 || x.z < 0 || x.y+x.z > 1)
        {
            continue;
        }
        
        vec3 position = start + x.x*dir;
        float distance = glm::length(position);
        
        if (distance < result.distance)
        {
            wasfound = true;
            result.distance = distance;
            result.position = position;
            result.triangle_index = i;
        }
    }
    
    return wasfound;
}

int main(int argc, char* argv[])
{
    /* // INTERSECTION TEST
    triangles.push_back(Triangle(vec3(1,0,-0.5), vec3(1,-0.5,0.5), vec3(1,0.5,0.5), vec3(1,0,0)));
    triangles.push_back(Triangle(vec3(0.5,0,-0.5), vec3(0.5,-0.5,0.5), vec3(0.5,0.5,0.5), vec3(0.5,0,0)));
    
    intersection test;
    
    if (findClosestIntersection(vec3(0,0,0), vec3(1,0,0), triangles, test))
    {
        printf("intersection at (%.2f, %.2f, %.2f), distance = %.2f\n", test.position.x, test.position.y, test.position.z, test.distance);
    }
    */
    
    printf("fov is %.2f degrees\n", (2*atan(float(SCREEN_HEIGHT) / (2.0*focal_length))) * (180.0 / 3.1415));
    // return 0;
    LoadTestModel(triangles);
    
    // triangles.push_back(
        // vec3()
    // );
    
    screen = InitializeSDL(SCREEN_WIDTH, SCREEN_HEIGHT);
    t = SDL_GetTicks();    // Set start value for timer.
    
    Draw();
    Update();
    
    while (NoQuitMessageSDL())
    {
        // Update();
        // Draw();
        
        SDL_Delay(20);
    }
    
    SDL_SaveBMP(screen, "screenshot.bmp");
    return 0;
}

void Update()
{
    // Compute frame time:
    int t2 = SDL_GetTicks();
    float dt = float(t2 - t);
    t = t2;
    cout << "Render time: " << dt << " ms." << endl;
}

void Draw()
{
    if (SDL_MUSTLOCK(screen))
    {
        SDL_LockSurface(screen);
    }
    
    intersection in;
    
    for (int y = 0; y < SCREEN_HEIGHT; ++y)
    {
        for (int x = 0; x < SCREEN_WIDTH; ++x)
        {
            vec3 color(0,0,0);
            vec3 ray(x - SCREEN_WIDTH/2, y - SCREEN_HEIGHT/2, focal_length);
            
            if (findClosestIntersection(camera_pos, ray, triangles, in))
            {
                color = triangles[in.triangle_index].color;
            }
            
            PutPixelSDL(screen, x, y, color);
        }
    }
    
    if (SDL_MUSTLOCK(screen))
    {
        SDL_UnlockSurface(screen);
    }
    
    SDL_UpdateRect(screen, 0, 0, 0, 0);
}

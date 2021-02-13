// *neode.command* setgo gcc -O3 -I/usr/include/SDL -I./glm -lstdc++ -lm -lSDL skeleton.cpp -o skeleton; ./skeleton

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
const int SCREEN_HEIGHT = 400;
SDL_Surface* screen;
int t;

vector<Triangle> triangles;
float focal_length = SCREEN_HEIGHT / 2.0f;
vec3 camera_pos(0.0f,0.0f,-2.50f);

mat3 camera_rot(vec3(1.0f, 0.0f, 0.0f),
                vec3(0.0f, 1.0f, 0.0f),
                vec3(0.0f, 0.0f, 1.0f));

vec3 camera_right(1.0f, 0.0f, 0.0f);
vec3 camera_down(0.0f, 1.0f, 0.0f);
vec3 camera_forward(0.0f, 0.0f, 1.0f);

float camera_yaw = 0.0f;

vec3 light_pos(0.0f, -0.5f, -0.7f);
vec3 light_color = 14.0f * vec3(1.0f, 1.0f, 1.0f);
vec3 indirect_light = 0.5f * vec3(1, 1, 1);

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

// cramers rule:
// Ax = b has solution x with x_i = det(A_i)/det(A) where A_i is A having replaced column i with b
// 
// A = a11 a12 a13
//     a21 a22 a23
//     a31 a32 a33
// 
// det(A) = a11*((a22*a33) - (a23*a32)) - a12*((a21*a33)-(a23*a31)) + a13*((a21*a32)-(a22*a31))
// 
// A_1 = b1 a12 a13     A_2 = a11 b1 a13     A_3 = a11 a12 b1
//       b2 a22 a23           a21 b2 a23           a21 a22 b2
//       b3 a32 a33           a31 b3 a33           a31 a32 b3
//
// det(A_1) = det(A) with a11 = b1, a21 = b2, a31 = b3
//          = b1*((a22*a33) - (a23*a32)) - a12*((b2*a33)-(a23*b3)) + a13*((b2*a32)-(a22*b3))
// 
// det(A_2) = det(A) with a12 = b1, a22 = b2, a32 = b3
//          = a11*((b2*a33) - (a23*b3)) - b1*((a21*a33)-(a23*a31)) + a13*((a21*b3)-(b2*a31))
// 
// det(A_3) = det(A) with a13 = b1, a23 = b3, a33 = b3
//          = a11*((a22*b3) - (b2*a32)) - a12*((a21*b3)-(b2*a31)) + b1*((a21*a32)-(a22*a31))


bool findClosestIntersectionCramer(vec3 start, vec3 dir, vector<Triangle> triangles, intersection& result)
{
    bool wasfound = false;
    result.distance = std::numeric_limits<float>::max();
    
    for (int i = 0; i < triangles.size(); ++i)
    {
        vec3 e1 = triangles[i].v1 - triangles[i].v0;
        vec3 e2 = triangles[i].v2 - triangles[i].v0;
        vec3 b = start - triangles[i].v0;
        
        // float a11 = -dir.x;
        // float a21 = -dir.y;
        // float a31 = -dir.z;
        
        // float a12 = e1.x;
        // float a22 = e1.y;
        // float a32 = e1.z;
        
        // float a13 = e2.x;
        // float a23 = e2.y;
        // float a33 = e2.z;
        
        // float b1 = b.x;
        // float b2 = b.y;
        // float b3 = b.z;
        
        // float detA = a11*((a22*a33) - (a23*a32)) - a12*((a21*a33)-(a23*a31)) + a13*((a21*a32)-(a22*a31));
        // float detA1 = b1*((a22*a33) - (a23*a32)) - a12*((b2*a33)-(a23*b3)) + a13*((b2*a32)-(a22*b3));
        // float detA2 = a11*((b2*a33) - (a23*b3)) - b1*((a21*a33)-(a23*a31)) + a13*((a21*b3)-(b2*a31));
        // float detA3 = a11*((a22*b3) - (b2*a32)) - a12*((a21*b3)-(b2*a31)) + b1*((a21*a32)-(a22*a31));
        
        float detA = -dir.x*((e1.y*e2.z) - (e2.y*e1.z)) - e1.x*((-dir.y*e2.z)-(e2.y*-dir.z)) + e2.x*((-dir.y*e1.z)-(e1.y*-dir.z));
        
        float detA2 = -dir.x*((b.y*e2.z) - (e2.y*b.z)) - b.x*((-dir.y*e2.z)-(e2.y*-dir.z)) + e2.x*((-dir.y*b.z)-(b.y*-dir.z));
        float u = detA2/detA;
        
        if (u < 0 || u > 1)
            continue;
        
        float detA3 = -dir.x*((e1.y*b.z) - (b.y*e1.z)) - e1.x*((-dir.y*b.z)-(b.y*-dir.z)) + b.x*((-dir.y*e1.z)-(e1.y*-dir.z));
        float v = detA3/detA;
        
        if (v < 0 || v > 1 || u + v > 1)
            continue;
        
        float detA1 = b.x*((e1.y*e2.z) - (e2.y*e1.z)) - e1.x*((b.y*e2.z)-(e2.y*b.z)) + e2.x*((b.y*e1.z)-(e1.y*b.z));
        float t = detA1/detA;
        
        if (t < 0)
            continue;
        
        // vec3 x(detA1/detA, detA2/detA, detA3/detA);
        
        // if (x.x >= 0 && x.y >= 0 && x.z >= 0 && x.y + x.z <= 1)
        // {
            // float distance = glm::length(x.x*dir); // why bug if take length of start + x.x*dir?
            // float distance = glm::length(t*dir); // why bug if take length of start + x.x*dir?
            
            vec3 r = t*dir;
            float distance = sqrt(r.x*r.x + r.y*r.y + r.z*r.z);
            
            if (distance < result.distance)
            {
                wasfound = true;
                result.distance = distance;
                result.position = start + t*dir;
                result.triangle_index = i;
            }
        // }
    }
    
    return wasfound;
}

/*
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
        
        // inside triangle: x.x >= 0, x.y >= 0, x.z >= 0, x.y+x.z <= 1
        if (x.x >= 0 && x.y >= 0 && x.z >= 0 && x.y + x.z <= 1)
        {
            vec3 position = start + x.x*dir;
            float distance = glm::length(x.x*dir); // why bug if take length of start + x.x*dir?
            
            if (distance < result.distance)
            {
                wasfound = true;
                result.distance = distance;
                result.position = position;
                result.triangle_index = i;
            }
        }
    }
    
    return wasfound;
}
*/

void updateCameraRotation()
{
    camera_rot = mat3(vec3(cos(camera_yaw), 0.0f, sin(camera_yaw)),
                      vec3(0.0f, 1.0f, 0.0f),
                      vec3(-sin(camera_yaw), 0.0f, cos(camera_yaw)));
    
    camera_right = camera_rot[0];
    camera_down = camera_rot[1];
    camera_forward = camera_rot[2];
}

vec3 directLight(const intersection& i)
{
    vec3 vr = light_pos - i.position;
    float r = sqrt(vr.x*vr.x + vr.y*vr.y + vr.z*vr.z);
    
    // cout << "surface point is (" << i.position.x << ", " << i.position.y << ", " << i.position.z << ")" << endl;
    // cout << "light pos is (" << light_pos.x << ", " << light_pos.y << ", " << light_pos.z << ")" << endl;
    // cout << "surface-to-light vector is (" << vr.x << ", " << vr.y << ", " << vr.z << ")" << endl;
    // cout << "surface-to-light vector has length " << r << endl;
    
    intersection shadowtest;
    
    // if (findClosestIntersectionCramer(i.position, vr, triangles, shadowtest))
    // {
        // cout << "got shadowtest intersection at distance " << shadowtest.distance << endl;
        
    // }
    // else
    // {
        // cout << "did not get shadowtest intersection" << endl;
    // }
    
    if (findClosestIntersectionCramer(i.position + 0.01f * vr, vr, triangles, shadowtest) && shadowtest.distance < r)
    {
        return vec3(0,0,0);
    }
    else
    {
        float thedot = glm::dot(triangles[i.triangle_index].normal, glm::normalize(vr));
        
        vec3 result = (light_color * thedot) / (4.0f * 3.1415f * r*r);
        
        if (thedot > 0)
        {
            return result;
        }
        else
        {
            return -result;
        }
    }
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
    
    LoadTestModel(triangles);
    screen = InitializeSDL(SCREEN_WIDTH, SCREEN_HEIGHT);
    t = SDL_GetTicks();    // Set start value for timer.
    
    Draw();
    Update();
    
    // return 0;
    
    while (NoQuitMessageSDL())
    {
        Update();
        Draw();
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
    // cout << "Forward vector: (" << camera_forward.x << ", " << camera_forward.y << ", " << camera_forward.z << ")" << endl;
    Uint8* keystate = SDL_GetKeyState(0);
    
    if (keystate[SDLK_UP])
    {
        // camera_pos = camera_pos + vec3(0.0f, 0.0f, 0.1f);
        camera_pos = camera_pos + 0.1f * camera_forward;
    }
    
    if (keystate[SDLK_DOWN])
    {
        // camera_pos = camera_pos - vec3(0.0f, 0.0f, 0.1f);
        camera_pos = camera_pos - 0.1f * camera_forward;
    }
    
    if (keystate[SDLK_LEFT])
    {
        // camera_pos = camera_pos - vec3(0.1f, 0.0f, 0.0f);
        camera_yaw += 0.1f;
        updateCameraRotation();
    }
    
    if (keystate[SDLK_RIGHT])
    {
        // camera_pos = camera_pos + vec3(0.1f, 0.0f, 0.0f);
        camera_yaw -= 0.1f;
        updateCameraRotation();
    }
    
    if (keystate[SDLK_w])
    {
        light_pos += vec3(0.0f, 0.0f, 0.1f);
    }
    
    if (keystate[SDLK_s])
    {
        light_pos -= vec3(0.0f, 0.0f, 0.1f);
    }
    
    if (keystate[SDLK_a])
    {
       light_pos -= vec3(0.1f, 0.0f, 0.0f);
    }
    
    if (keystate[SDLK_d])
    {
        light_pos += vec3(0.1f, 0.0f, 0.0f);
    }
    
    if (keystate[SDLK_q])
    {
       light_pos += vec3(0.0f, 0.1f, 0.0f);
    }
    
    if (keystate[SDLK_e])
    {
        light_pos -= vec3(0.0f, 0.1f, 0.0f);
    }
}

void Draw()
{
    if (SDL_MUSTLOCK(screen))
    {
        SDL_LockSurface(screen);
    }
    
    intersection in;
    
    for (int y = 0; y < SCREEN_HEIGHT; y += 2)
    // for (int y = SCREEN_HEIGHT/2; y < SCREEN_HEIGHT/2 + 1; y += 2)
    {
        for (int x = 0; x < SCREEN_WIDTH; x += 1)
        {
            vec3 color(0,0,0);
            vec3 ray(x - SCREEN_WIDTH/2, y - SCREEN_HEIGHT/2, focal_length);
            
            ray = camera_rot * ray;
            
            if (findClosestIntersectionCramer(camera_pos, ray, triangles, in))
            {
                // color = triangles[in.triangle_index].color;
                // color = directLight(in);
                color = triangles[in.triangle_index].color * (directLight(in) + indirect_light);
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

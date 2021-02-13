// *neode.command* setgo gcc -O3 -I./glm -I/usr/include/SDL -lstdc++ -lm -lSDL wireframe_ivec2.cpp -o wireframe_ivec2; ./wireframe_ivec2
#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"

using namespace std;
using glm::vec2;
using glm::vec3;
using glm::ivec2;
using glm::mat3;

// ----------------------------------------------------------------------------
// GLOBAL VARIABLES
float frameTime = 0;

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;
float focalLength = float(SCREEN_HEIGHT)/1.0f;

SDL_Surface* screen;
int t;
vector<Triangle> triangles;

vec3 cameraPos(0, 0, -3.01);

vec3 cameraRight(1.0f, 0.0f, 0.0f);
vec3 cameraDown(0.0f, 1.0f, 0.0f);
vec3 cameraForward(0.0f, 0.0f, 1.0f);

float cameraPitch = 0.0f;
float cameraYaw = 0.0f;

mat3 cameraRotation(vec3(1,0,0), vec3(0,1,0), vec3(0,0,1));

const float MOVEMENT_SPEED = 0.008f;
const float ROTATION_SPEED = 0.003f;
const float MOUSE_SENSITIVITY = 2.0f;

bool enableMouse = false;

// ----------------------------------------------------------------------------
// FUNCTIONS

void Update();
void Draw();
void UpdateCameraRotation();
void VertexShader(const vec3& v, ivec2& p);
void Interpolate(ivec2 a, ivec2 b, vector<ivec2>& result);
void ComputeTriangleRows(ivec2 v0, ivec2 v1, ivec2 v2, vector<ivec2>& leftPixels, vector<ivec2>& rightPixels);
void InterpolateLine(const ivec2& a, const ivec2& b, vector<ivec2>& line);
void DrawLineSDL(SDL_Surface* surface, ivec2 a, ivec2 b, vec3 color);
void DrawRows(SDL_Surface* surface, const vector<ivec2>& leftPixels, const vector<ivec2>& rightPixels, vec3 color);

int main(int argc, char* argv[])
{
	LoadTestModel(triangles);
	screen = InitializeSDL(SCREEN_WIDTH, SCREEN_HEIGHT);
    
    // this can cause problems if the program hangs
    // SDL_WM_GrabInput(SDL_GRAB_ON);
    SDL_WarpMouse(250,250);
    
    UpdateCameraRotation();
    
	t = SDL_GetTicks();	// Set start value for timer.
    
    Update();
    Draw();
    enableMouse = true;
    
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
	frameTime = t2 - t;
	t = t2;
	cout << "Render time: " << frameTime << " ms." << endl;
    
    bool cameraRotated = false;
    
	Uint8* keystate = SDL_GetKeyState(0);

	if (keystate[SDLK_RIGHT])
	{
        cameraYaw -= frameTime * ROTATION_SPEED;
        cameraRotated = true;
    }
    
	if (keystate[SDLK_LEFT])
    {
        cameraYaw += frameTime * ROTATION_SPEED;
        cameraRotated = true;
    }

	if (keystate[SDLK_UP])
	{
        cameraPitch -= frameTime * ROTATION_SPEED;
        cameraRotated = true;
    }
    
	if (keystate[SDLK_DOWN])
    {
        cameraPitch += frameTime * ROTATION_SPEED;
        cameraRotated = true;
    }
    
	if (keystate[SDLK_w])
	{
        cameraPos += cameraForward * frameTime * MOVEMENT_SPEED;
    }
    
	if (keystate[SDLK_s])
	{
        cameraPos -= cameraForward * frameTime * MOVEMENT_SPEED;
    }
    
	if (keystate[SDLK_d])
	{
        cameraPos += cameraRight * frameTime * MOVEMENT_SPEED;
    }
    
	if (keystate[SDLK_a])
	{
        cameraPos -= cameraRight * frameTime * MOVEMENT_SPEED;
    }
    
	if (keystate[SDLK_e])
	{
        cameraPos += cameraDown * frameTime * MOVEMENT_SPEED;
    }
    
	if (keystate[SDLK_q])
	{
        cameraPos -= cameraDown * frameTime * MOVEMENT_SPEED;
    }
    
    if (enableMouse)
    {
        int dx;
        int dy;
        SDL_GetRelativeMouseState(&dx, &dy);
        
        if ((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)) == 0)
        {
            if (dy > 0)
            {
                cameraPitch += dy * MOUSE_SENSITIVITY * ROTATION_SPEED;
                cameraRotated = true;
            }
            
            if (dy < 0)
            {
                cameraPitch -= -dy * MOUSE_SENSITIVITY * ROTATION_SPEED;
                cameraRotated = true;
            }
            
            if (dx > 0)
            {
                cameraYaw -= dx * MOUSE_SENSITIVITY * ROTATION_SPEED;
                cameraRotated = true;
            }
            
            if (dx < 0)
            {
                cameraYaw += -dx * MOUSE_SENSITIVITY * ROTATION_SPEED;
                cameraRotated = true;
            }
        }
    }
    
    if (cameraRotated)
    {
        UpdateCameraRotation();
    }
}

void Draw()
{
    SDL_FillRect(screen, 0, 0);
    
    if (SDL_MUSTLOCK(screen))
    {
        SDL_LockSurface(screen);
    }
    
    for (int i=0; i<triangles.size(); ++i)
    {
        if (((triangles[i].v0 - cameraPos)*cameraRotation).z < 1 || ((triangles[i].v1 - cameraPos)*cameraRotation).z < 1 || ((triangles[i].v2 - cameraPos)*cameraRotation).z < 1)
        {
            continue; // fixme inefficient
        }
        
        vector<ivec2> proj(3);
        VertexShader(triangles[i].v0, proj[0]);
        VertexShader(triangles[i].v1, proj[1]);
        VertexShader(triangles[i].v2, proj[2]);
        
        vec3 white(1,1,1);
        vec3 gray(0.33,0.33,0.33);
        
        DrawLineSDL(screen, proj[0], proj[1], gray);
        DrawLineSDL(screen, proj[1], proj[2], gray);
        DrawLineSDL(screen, proj[2], proj[0], gray);
        
        PutPixelSDL(screen, proj[0].x, proj[0].y, white);
        PutPixelSDL(screen, proj[1].x, proj[1].y, white);
        PutPixelSDL(screen, proj[2].x, proj[2].y, white);
    }

    if (SDL_MUSTLOCK(screen))
    {
        SDL_UnlockSurface(screen);
    }
    
    SDL_UpdateRect(screen, 0, 0, 0, 0);
}

void UpdateCameraRotation()
{
    // these are the wikipedia standard rotation matrices (in row-major form, wysiwyg)
    //
    // Rx =  1    0    0
    //       0    cos  -sin
    //       0    sin  cos
    //
    // Ry =  cos  0    sin
    //       0    1    0
    //       -sin 0    cos
    //
    // Rz =  cos  -sin 0
    //       sin  cos  0
    //       0    0    1
    
    // since mat3 takes arguments as three columns, this becomes the transpose of Ry
    mat3 RyT(vec3(cos(cameraYaw), 0.0f, sin(cameraYaw)),
             vec3(0.0f, 1.0f, 0.0f),
             vec3(-sin(cameraYaw), 0.0f, cos(cameraYaw)));
    
    // transpose of Rx
    mat3 RxT(vec3(1.0f, 0.0f, 0.0f),
             vec3(0.0f, cos(cameraPitch), -sin(cameraPitch)),
             vec3(0.0f, sin(cameraPitch), cos(cameraPitch)));
    
    cameraRotation = RyT; // apply yaw
    
    // why does this work?
    cameraRight = cameraRotation[0];
    cameraDown = cameraRotation[1];
    cameraForward = cameraRotation[2];
    
    cameraRotation *= RxT; // apply pitch
}

void VertexShader(const vec3& v, ivec2& p)
{
    vec3 vr = (v - cameraPos) * cameraRotation;
    
    p.x = focalLength * (vr.x / vr.z) + SCREEN_WIDTH / 2;
    p.y = focalLength * (vr.y / vr.z) + SCREEN_HEIGHT / 2;
}

void Interpolate(ivec2 a, ivec2 b, vector<ivec2>& result)
{
    int N = result.size();
    vec2 step = vec2(b - a) / float(max(N - 1, 1));
    
    vec2 current(a);
    
    for (int i = 0; i < N; ++i)
    {
        result[i].x = round(current.x);
        result[i].y = round(current.y);
        current += step;
    }
}

void DrawLineSDL(SDL_Surface* surface, ivec2 a, ivec2 b, vec3 color)
{
    ivec2 delta = glm::abs(a - b);
    int pixels = glm::max(delta.x, delta.y) + 1;
    
    /*
    if (a.x < 0 || a.y < 0 || b.x < 0 || b.y < 0) // fixme clipping
    {
        return;
    }
    
    if (pixels > 1000) // fixme hack
    {
        return;
    }
    */
    
    vector<ivec2> line(pixels);
    Interpolate(a, b, line);
    
    for (int i = 0; i < pixels; ++i)
    {
        PutPixelSDL(screen, line[i].x, line[i].y, color);
    }
}

void InterpolateLine(const ivec2& a, const ivec2& b, vector<ivec2>& line)
{
    ivec2 delta = glm::abs(a - b);
    int pixels = glm::max(delta.x, delta.y) + 1;
    
    line.resize(pixels);
    Interpolate(a, b, line);
}

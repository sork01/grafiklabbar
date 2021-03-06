
// *neode.command* setgo gcc -O3 -I./glm -I/usr/include/SDL -lstdc++ -lm -lSDL pixel_lighting.cpp -o pixel_lighting; ./pixel_lighting
#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"
#include "objloader.h"

using namespace std;
using glm::vec2;
using glm::vec3;
using glm::ivec2;
using glm::mat3;

// ----------------------------------------------------------------------------
// GLOBAL VARIABLES
bool debug = false;
int debugwait = 0;

float frameTime = 0;

const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 600;
float focalLength = float(SCREEN_HEIGHT)/1.0f;

SDL_Surface* screen;
int t;
vector<Triangle> triangles;

vec3 lightPos(0,-0.5,-0.7);
vec3 lightPosInCameraSpace;

vec3 lightPower = 15.1f * vec3(1, 1, 1);
vec3 indirectLightPowerPerArea = 0.0f * vec3(1, 1, 1);
float lightPhase = 0.0f;

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

float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH];

struct Vertex
{
    vec3 position;
    
    Vertex() : position(vec3(0,0,0)) {}
    Vertex(vec3 pos) : position(pos) {}
};

struct Pixel
{
    int x;
    int y;
    float zinv;
    vec3 pos3d;
    
    Pixel()
    {
        x = 0;
        y = 0;
        zinv = 0;
        pos3d = vec3(0, 0, 0);
    }
    
    Pixel(int _x, int _y, float _zinv, vec3 _pos3d) : x(_x), y(_y), zinv(_zinv), pos3d(_pos3d) {}
};

vec3 currentNormal;
vec3 currentReflectance;

// ----------------------------------------------------------------------------
// FUNCTIONS

void Update();
void Draw();
void UpdateCameraRotation();
void VertexShader(const Vertex& v, Pixel& p);
void PixelShader(const Pixel& p);
void Interpolate(Pixel a, Pixel b, vector<Pixel>& result);
void ComputeTriangleRows(const Pixel& v0, const Pixel& v1, const Pixel& v2, vector<Pixel>& leftPixels, vector<Pixel>& rightPixels);
void InterpolateLine(const Pixel& a, const Pixel& b, vector<Pixel>& line);
void DrawLineSDL(SDL_Surface* surface, Pixel a, Pixel b, vec3 color);
void DrawRows(SDL_Surface* surface, const vector<Pixel>& leftPixels, const vector<Pixel>& rightPixels, vec3 color);

int main(int argc, char* argv[])
{
	// LoadTestModel(triangles);
    // LoadOBJModel("../wt_teapot.obj", triangles, 1.0f, true);
    LoadOBJModel("/home/lur/obj/bunny.obj", triangles, 1nä5.0f, true);
    
    // triangles.push_back(Triangle(
        // vec3(0, -1, 2),
        
        // vec3(1, 1, 2),
        // vec3(-1, 1, 2),
        // vec3(1,1,1)
        // ));
    
	screen = InitializeSDL(SCREEN_WIDTH, SCREEN_HEIGHT);
    
    // this can cause problems if the program hangs
    // SDL_WM_GrabInput(SDL_GRAB_ON);
    SDL_WarpMouse(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
    
    UpdateCameraRotation();
    
	t = SDL_GetTicks();	// Set start value for timer.
    
    Update();
    Draw();
    
	while (NoQuitMessageSDL())
	{
		Update();
		Draw();
        
        if (SDL_GetTicks() > 500)
        {
            enableMouse = true;
            break;
        }
	}
    
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
        
        if (cameraPitch < -0.8)
        {
            cameraPitch = -0.8;
        }
        
        cameraRotated = true;
    }
    
	if (keystate[SDLK_DOWN])
    {
        cameraPitch += frameTime * ROTATION_SPEED;
        
        if (cameraPitch > 0.8)
        {
            cameraPitch = 0.8;
        }
        
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
    
	if (keystate[SDLK_1])
	{
        lightPos -= frameTime * 0.005f * vec3(1,0,0);
    }
    
	if (keystate[SDLK_2])
	{
        lightPos += frameTime * 0.005f * vec3(1,0,0);
    }
    
	if (keystate[SDLK_3])
	{
        lightPos -= frameTime * 0.005f * vec3(0,1,0);
    }
    
	if (keystate[SDLK_4])
	{
        lightPos += frameTime * 0.005f * vec3(0,1,0);
    }
    
	if (keystate[SDLK_5])
	{
        lightPos -= frameTime * 0.005f * vec3(0,0,1);
    }
    
	if (keystate[SDLK_6])
	{
        lightPos += frameTime * 0.005f * vec3(0,0,1);
    }
    
	if (debugwait <= 0 && !debug && keystate[SDLK_k])
	{
        debugwait = 10;
        debug = true;
    }
	else if (debugwait <= 0 && debug && keystate[SDLK_k])
	{
        debugwait = 10;
        debug = false;
    }
    
    --debugwait;
    
    int dx;
    int dy;
    SDL_GetRelativeMouseState(&dx, &dy);
    
    if (enableMouse)
    {
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
    
    // moving the light for fun
    // lightPos.y += frameTime * sin(lightPhase) / 1250.0f;
    // lightPos.x += frameTime * cos(2*lightPhase) / 1250.0f;
    // lightPower.x = abs(125.0f * sin(lightPhase));
    // lightPower.y = abs(125.0f * sin(lightPhase + 120.0f * 3.14159562f / 180.0f));
    // lightPower.z = abs(125.0f * sin(lightPhase + 240.0f * 3.14159562f / 180.0f));
    // lightPhase += frameTime * 0.0002f;
    
    // mat3 RyT(vec3(cos(0.005f), 0.0f, sin(0.005f)),
             // vec3(0.0f, 1.0f, 0.0f),
             // vec3(-sin(0.005f), 0.0f, cos(0.005f)));
    
    // for (int i = 0; i < triangles.size(); ++i)
    // {
        // triangles[i].v0 = triangles[i].v0 * RyT;
        // triangles[i].v1 = triangles[i].v1 * RyT;
        // triangles[i].v2 = triangles[i].v2 * RyT;
        // triangles[i].normal = triangles[i].normal * RyT;
    // }
    
    lightPosInCameraSpace = (lightPos - cameraPos) * cameraRotation;
    // lightPosInCameraSpace = lightPos;
}

void Draw()
{
    memset(depthBuffer, 0, sizeof(depthBuffer));
    SDL_FillRect(screen, 0, 0);
    
    if (SDL_MUSTLOCK(screen))
    {
        SDL_LockSurface(screen);
    }
    
    for (int i=0; i<triangles.size(); ++i)
    {
        // how to only draw "front-facing" faces of triangles?
        // let A be the vector starting at the camera position and ending at the first vertex of a triangle T
        // T has a normal N
        // if the absolute value of the angle (in the AN-plane) between A and N is greater than pi/2
        //   this isn't how dot-cos angle works? dot-cos becomes some kind of absolute/direction-independent angle measure?
        
        // cos t = A dot N / |A|*|N|
        // if normalized vectors becomes cos t = A dot N
        
        vec3 A = glm::normalize(triangles[i].v0 - cameraPos);
        vec3 N = glm::normalize(triangles[i].normal);
        
        // cout << glm::dot(A, N) << endl;
        
        if (glm::dot(A, N) > 0.0f)
        {
            continue;
        }
        
        if (((triangles[i].v0 - cameraPos)*cameraRotation).z < 1 || ((triangles[i].v1 - cameraPos)*cameraRotation).z < 1 || ((triangles[i].v2 - cameraPos)*cameraRotation).z < 1)
        {
            continue; // fixme inefficient
        }
        
        currentNormal = triangles[i].normal * cameraRotation; // nice bug when left out
        currentReflectance = vec3(1,1,1);
        
        vector<Pixel> proj(3);
        VertexShader(Vertex(triangles[i].v0), proj[0]);
        VertexShader(Vertex(triangles[i].v1), proj[1]);
        VertexShader(Vertex(triangles[i].v2), proj[2]);
        
        vector<Pixel> leftPixels;
        vector<Pixel> rightPixels;
        ComputeTriangleRows(proj[0], proj[1], proj[2], leftPixels, rightPixels);
        DrawRows(screen, leftPixels, rightPixels, triangles[i].color);
        
        if (((lightPos - cameraPos)*cameraRotation).z >= 1)
        {
            VertexShader(Vertex(lightPos), proj[0]);
            PutPixelSDL(screen, proj[0].x, proj[0].y, vec3(1,1,0));
            PutPixelSDL(screen, proj[0].x+1, proj[0].y, vec3(1,1,0));
            PutPixelSDL(screen, proj[0].x, proj[0].y+1, vec3(1,1,0));
            PutPixelSDL(screen, proj[0].x+1, proj[0].y+1, vec3(1,1,0));
        }
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

void VertexShader(const Vertex& v, Pixel& p)
{
    vec3 vr = (v.position - cameraPos) * cameraRotation;
    
    p.x = focalLength * (vr.x / vr.z) + SCREEN_WIDTH / 2;
    p.y = focalLength * (vr.y / vr.z) + SCREEN_HEIGHT / 2;
    p.zinv = 1.0f / vr.z;
    
    if (debug)
    {
        p.pos3d = v.position;
    }
    else
    {
        p.pos3d = vr * p.zinv;
    }
    
    // p.pos3d = v.position;
    // p.pos3d = vr * p.zinv;
}

void PixelShader(const Pixel& p, vec3 color)
{
    if (p.x < 0 || p.x >= SCREEN_WIDTH || p.y < 0 || p.y >= SCREEN_HEIGHT || p.zinv < depthBuffer[p.y][p.x])
    {
        return;
    }
    
    depthBuffer[p.y][p.x] = p.zinv;
    
    // if (debug)
    // {
        // cout << p.pos3d.z << " vs " << 1.0f / p.zinv << endl;
    // }
    
    vec3 r;
    
    if (debug)
    {
        r = lightPos - p.pos3d;
    }
    else
    {
        r = lightPosInCameraSpace - (p.pos3d / p.zinv);
    }
    
    // vec3 r = lightPosInCameraSpace - (p.pos3d / p.zinv);
    // vec3 r = lightPos - vec3(p.pos3d.x, p.pos3d.y, p.pos3d.z);
    float rmagn = sqrt(r.x*r.x + r.y*r.y + r.z*r.z);
    float thedot = glm::dot(currentNormal, glm::normalize(r));
    
    vec3 illumination = currentReflectance * ((lightPower * max(thedot, 0.0f)) / (4.0f * 3.14159265f * rmagn*rmagn)) + indirectLightPowerPerArea;
    // vec3 illumination = lightPower / (4.0f * 3.14159265f * rmagn*rmagn);
    
    PutPixelSDL(screen, p.x, p.y, color * illumination);
}

void Interpolate(Pixel a, Pixel b, vector<Pixel>& result)
{
    int N = result.size();
    
    vec3 step = vec3(b.x - a.x, b.y - a.y, b.zinv - a.zinv) / float(max(N - 1, 1));
    vec3 step_pos = (b.pos3d - a.pos3d) / float(max(N - 1, 1));
    
    vec3 current(a.x, a.y, a.zinv);
    vec3 current_pos(a.pos3d);
    
    for (int i = 0; i < N; ++i)
    {
        result[i].x = round(current.x);
        result[i].y = round(current.y);
        result[i].zinv = current.z;
        
        result[i].pos3d = current_pos;
        
        current += step;
        current_pos += step_pos;
    }
}

void DrawLineSDL(SDL_Surface* surface, Pixel a, Pixel b, vec3 color)
{
    vector<Pixel> line;
    InterpolateLine(a, b, line);
    
    for (int i = 0; i < line.size(); ++i)
    {
        PixelShader(line[i], color);
    }
}

void InterpolateLine(const Pixel& a, const Pixel& b, vector<Pixel>& line)
{
    vec2 delta(glm::abs(a.x - b.x), glm::abs(a.y - b.y));
    
    int pixels = glm::max(delta.x, delta.y) + 1;
    
    if (pixels > 1000)
    {
        // cout << pixels << endl;
        return;
    }
    
    line.resize(pixels);
    Interpolate(a, b, line);
}

void ComputeTriangleRows(const Pixel& v0, const Pixel& v1, const Pixel& v2, vector<Pixel>& leftPixels, vector<Pixel>& rightPixels)
{
    // 1. Find max and min y-value of the triangle
    // and compute the number of rows it occupies.
    
    int vmin = min(v0.y, min(v1.y, v2.y));
    int vmax = max(v0.y, max(v1.y, v2.y));
    
    int rows = vmax - vmin + 1;
    
    // 2. Resize leftPixels and rightPixels
    // so that they have an element for each row.
    
    leftPixels.resize(rows);
    rightPixels.resize(rows);
    
    // 3. Initialize the x-coordinates in leftPixels
    // to some really large value and the x-coordinates
    // in rightPixels to some really small value.
    
    for (int i = 0; i < rows; ++i)
    {
        leftPixels[i].x = +numeric_limits<int>::max();
        rightPixels[i].x = -numeric_limits<int>::max();
    }
    
    // 4. Loop through all edges of the triangle and use
    // linear interpolation to find the x-coordinate for
    // each row it occupies. Update the corresponding
    // values in rightPixels and leftPixels.
    
    vector<Pixel> e0;
    vector<Pixel> e1;
    vector<Pixel> e2;
    
    InterpolateLine(v0, v1, e0);
    InterpolateLine(v1, v2, e1);
    InterpolateLine(v2, v0, e2);
    
    vector<vector<Pixel>> edges(3);
    edges[0] = e0;
    edges[1] = e1;
    edges[2] = e2;
    
    for (int i = 0; i < 3; ++i)
    {
        vector<Pixel> edge = edges[i];
        
        for (int i = 0; i < edge.size(); ++i)
        {
            int row = edge[i].y - vmin;
            
            if (edge[i].x < leftPixels[row].x)
            {
                leftPixels[row] = edge[i];
            }
            
            if (edge[i].x > rightPixels[row].x)
            {
                rightPixels[row] = edge[i];
            }
        }
    }
}

void DrawRows(SDL_Surface* surface, const vector<Pixel>& leftPixels, const vector<Pixel>& rightPixels, vec3 color)
{
    int rows = leftPixels.size();
    
    for (int i = 0; i < rows; ++i)
    {
        DrawLineSDL(surface, leftPixels[i], rightPixels[i], color);
    }
}

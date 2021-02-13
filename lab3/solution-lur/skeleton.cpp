// *neode.command* setgo gcc -O3 -I./glm -I/usr/include/SDL -lstdc++ -lm -lSDL skeleton.cpp -o ThirdLab; ./ThirdLab
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
SDL_Surface* screen;
int t;
vector<Triangle> triangles;

vec3 cameraPos(0, 0, -3.01);

vec3 cameraRight(1.0f, 0.0f, 0.0f);
vec3 cameraDown(0.0f, 1.0f, 0.0f);
vec3 cameraForward(0.0f, 0.0f, 1.0f);

vec3 cameraPYR(0.0f, 0.0f, 0.0f); // x=pitch, y=yaw, z=roll

// arguments are columns
mat3 cameraRot(vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f));

float focalLength = float(SCREEN_HEIGHT)/1.0f;

const float MOVEMENT_SPEED = 0.005f;
const float ROTATION_SPEED = 0.003f;
const float MOUSE_SENSITIVITY = 5;

bool enableMouse = false;

float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH];

struct Pixel
{
    int x;
    int y;
    float zinv;
};

// ----------------------------------------------------------------------------
// FUNCTIONS

void Update();
void Draw();
void UpdateCameraRotation();
void VertexShader(const vec3& v, Pixel& p);
void Interpolate(Pixel a, Pixel b, vector<Pixel>& result);
void ComputeTriangleRows(const Pixel& v0, const Pixel& v1, const Pixel& v2, vector<Pixel>& leftPixels, vector<Pixel>& rightPixels);
void InterpolateLine(const Pixel& a, const Pixel& b, vector<Pixel>& line);
void DrawLineSDL(SDL_Surface* surface, Pixel a, Pixel b, vec3 color);
void DrawRows(SDL_Surface* surface, const vector<Pixel>& leftPixels, const vector<Pixel>& rightPixels, vec3 color);

// -- pre-Pixel --
// void VertexShader(const vec3& v, ivec2& p);
// void Interpolate(ivec2 a, ivec2 b, vector<ivec2>& result);
// void ComputeTriangleRows(ivec2 v0, ivec2 v1, ivec2 v2, vector<ivec2>& leftPixels, vector<ivec2>& rightPixels);
// void InterpolateLine(const ivec2& a, const ivec2& b, vector<ivec2>& line);
// void DrawLineSDL(SDL_Surface* surface, ivec2 a, ivec2 b, vec3 color);
// void DrawRows(SDL_Surface* surface, const vector<ivec2>& leftPixels, const vector<ivec2>& rightPixels, vec3 color);

int main(int argc, char* argv[])
{
	LoadTestModel(triangles);
	screen = InitializeSDL(SCREEN_WIDTH, SCREEN_HEIGHT);
    
    // this can cause problems if the program hangs
    // SDL_WM_GrabInput(SDL_GRAB_ON);
    // SDL_WarpMouse(250,250);
    
    UpdateCameraRotation();
    
	t = SDL_GetTicks();	// Set start value for timer.
    
	while (NoQuitMessageSDL())
	{
		Update();
		Draw();
        // enableMouse = true;
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
    
	if (keystate[SDLK_UP])
	{
        cameraPYR.x -= frameTime * ROTATION_SPEED * cos(cameraPYR.z);
        cameraPYR.y -= frameTime * ROTATION_SPEED * -sin(cameraPYR.z);
        cameraRotated = true;
    }
    
	if (keystate[SDLK_DOWN])
	{
        cameraPYR.x += frameTime * ROTATION_SPEED * cos(cameraPYR.z);
        cameraPYR.y += frameTime * ROTATION_SPEED * -sin(cameraPYR.z);
        cameraRotated = true;
    }
    
	if (keystate[SDLK_RIGHT])
	{
        cameraPYR.y -= frameTime * ROTATION_SPEED * cos(cameraPYR.z);
        cameraPYR.x -= frameTime * ROTATION_SPEED * sin(cameraPYR.z);
        cameraRotated = true;
    }
    
	if (keystate[SDLK_LEFT])
    {
        cameraPYR.y += frameTime * ROTATION_SPEED * cos(cameraPYR.z);
        cameraPYR.x += frameTime * ROTATION_SPEED * sin(cameraPYR.z);
        cameraRotated = true;
    }
    
	if (keystate[SDLK_RSHIFT])
		;
    
	if (keystate[SDLK_RCTRL])
		;
    
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
        cameraPYR.z -= frameTime * ROTATION_SPEED;
        cameraRotated = true;
    }
    
	if (keystate[SDLK_q])
	{
        cameraPYR.z += frameTime * ROTATION_SPEED;
        cameraRotated = true;
    }
    
    if (enableMouse)
    {
        int dx;
        int dy;
        SDL_GetRelativeMouseState(&dx, &dy);
        
        if ((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)) == 0)
        {
            if (dy < 0)
            {
                cameraPYR.x += -dy * MOUSE_SENSITIVITY * frameTime * ROTATION_SPEED;
                cameraRotated = true;
            }
            
            if (dy > 0)
            {
                cameraPYR.x -= dy * MOUSE_SENSITIVITY * frameTime * ROTATION_SPEED;
                cameraRotated = true;
            }
            
            if (dx > 0)
            {
                cameraPYR.y += dx * MOUSE_SENSITIVITY * frameTime * ROTATION_SPEED;
                cameraRotated = true;
            }
            
            if (dx < 0)
            {
                cameraPYR.y -= -dx * MOUSE_SENSITIVITY * frameTime * ROTATION_SPEED;
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
    memset(depthBuffer, 0, sizeof(depthBuffer));
    SDL_FillRect(screen, 0, 0);
    
    if (SDL_MUSTLOCK(screen))
    {
        SDL_LockSurface(screen);
    }
    
    for (int i=0; i<triangles.size(); ++i)
    {
        if (((triangles[i].v0 - cameraPos)*cameraRot).z < 1 || ((triangles[i].v1 - cameraPos)*cameraRot).z < 1 || ((triangles[i].v2 - cameraPos)*cameraRot).z < 1)
        {
            continue; // fixme inefficient
        }
        
        // original
        // vector<vec3> vertices(3);
        // vertices[0] = triangles[i].v0;
        // vertices[1] = triangles[i].v1;
        // vertices[2] = triangles[i].v2;
        
        // for (int v = 0; v < 3; ++v)
        // {
            // ivec2 projPos;
            // VertexShader(vertices[v], projPos);
            // vec3 color(1,1,1);
            // PutPixelSDL(screen, projPos.x, projPos.y, color);
        // }
        
        // projection without depth
        // vector<ivec2> proj(3);
        // VertexShader(triangles[i].v0, proj[0]);
        // VertexShader(triangles[i].v1, proj[1]);
        // VertexShader(triangles[i].v2, proj[2]);
        
        // projection with depth
        vector<Pixel> proj(3);
        VertexShader(triangles[i].v0, proj[0]);
        VertexShader(triangles[i].v1, proj[1]);
        VertexShader(triangles[i].v2, proj[2]);
        
        // wireframe
        // vec3 white(1,1,1);
        // vec3 gray(0.33,0.33,0.33);
        
        // DrawLineSDL(screen, proj[0], proj[1], gray);
        // DrawLineSDL(screen, proj[1], proj[2], gray);
        // DrawLineSDL(screen, proj[2], proj[0], gray);
        
        // PutPixelSDL(screen, proj[0].x, proj[0].y, white);
        // PutPixelSDL(screen, proj[1].x, proj[1].y, white);
        // PutPixelSDL(screen, proj[2].x, proj[2].y, white);
        
        
        // flat without depth
        // vector<ivec2> leftPixels;
        // vector<ivec2> rightPixels;
        // ComputeTriangleRows(proj[0], proj[1], proj[2], leftPixels, rightPixels);
        // DrawRows(screen, leftPixels, rightPixels, triangles[i].color);
        
        
        // flat with depth
        vector<Pixel> leftPixels;
        vector<Pixel> rightPixels;
        ComputeTriangleRows(proj[0], proj[1], proj[2], leftPixels, rightPixels);
        DrawRows(screen, leftPixels, rightPixels, triangles[i].color);
        
        // for (int j = 0; j < leftPixels.size(); ++j)
        // {
            // cout << leftPixels[j].x << ", " << leftPixels[j].y << ", " << leftPixels[j].zinv << " " << rightPixels[j].x << ", " << rightPixels[j].y << ", " << rightPixels[j].zinv << endl;
        // }
        
        // exit(0);
    }
    
    // testing drawline
    // DrawLineSDL(screen, ivec2(10, 10), ivec2(30, 30), vec3(1,1,1));
    
    // testing draw filled triangle
    // vector<ivec2> leftPixels;
    // vector<ivec2> rightPixels;
    // ComputeTriangleRows(ivec2(10,20), ivec2(30,10), ivec2(20,40), leftPixels, rightPixels);
    // DrawRows(screen, leftPixels, rightPixels, vec3(1,1,1));
    
    if (SDL_MUSTLOCK(screen))
    {
        SDL_UnlockSurface(screen);
    }
    
    SDL_UpdateRect(screen, 0, 0, 0, 0);
}

void UpdateCameraRotation()
{
    // these are the wikipedia standard rotation matrices
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
    
    // mat3 rx(vec3(1.0f, 0.0f, 0.0f),
            // vec3(0.0f, cos(cameraPYR.x), sin(cameraPYR.x)),
            // vec3(0.0f, -sin(cameraPYR.x), cos(cameraPYR.x)));
    
    // mat3 ry(vec3(cos(cameraPYR.y), 0.0f, -sin(cameraPYR.y)),
            // vec3(0.0f, 1.0f, 0.0f),
            // vec3(sin(cameraPYR.y), 0.0f, cos(cameraPYR.y)));
    
    // mat3 rz(vec3(cos(cameraPYR.z), sin(cameraPYR.z), 0.0f),
            // vec3(-sin(cameraPYR.z), cos(cameraPYR.z), 0.0f),
            // vec3(0.0f, 0.0f, 1.0f));
    
    // transposes (for multiplying with a row vector to the left, x' = xA)
    //
    // RxT =  1    0    0
    //        0    cos  sin
    //        0    -sin cos
    //
    // RyT =  cos  0    -sin
    //        0    1    0
    //        sin  0    cos
    //
    // RzT =  cos  sin  0
    //        -sin cos  0
    //        0    0    1
    
    mat3 rxt(vec3(1.0f, 0.0f, 0.0f),
             vec3(0.0f, cos(cameraPYR.x), -sin(cameraPYR.x)),
             vec3(0.0f, sin(cameraPYR.x), cos(cameraPYR.x)));
    
    mat3 ryt(vec3(cos(cameraPYR.y), 0.0f, sin(cameraPYR.y)),
             vec3(0.0f, 1.0f, 0.0f),
             vec3(-sin(cameraPYR.y), 0.0f, cos(cameraPYR.y)));
    
    mat3 rzt(vec3(cos(cameraPYR.z), -sin(cameraPYR.z), 0.0f),
             vec3(sin(cameraPYR.z), cos(cameraPYR.z), 0.0f),
             vec3(0.0f, 0.0f, 1.0f));
    
    cameraRot = ryt*rxt;
    
    // why does this work?
    cameraRight = cameraRot[0];
    cameraDown = cameraRot[1];
    cameraForward = cameraRot[2];
}

void VertexShader(const vec3& v, ivec2& p)
{
    vec3 vv;
    vec3 rot;
    
    rot = (v * cameraRot) - cameraPos;
    
    // vv.x = v.x - cameraPos.x;
    // vv.y = v.y - cameraPos.y;
    // vv.z = v.z - cameraPos.z;
    
    // rot = vv*cameraRot;
    
    // printf("input point: %.2f, %.2f, %.2f\n", v.x, v.y, v.z);
    // printf("output point: %.2f, %.2f, %.2f\n", vv.x, vv.y, vv.z);
    
    p.x = focalLength * (rot.x / rot.z) + SCREEN_WIDTH/2;
    p.y = focalLength * (rot.y / rot.z) + SCREEN_HEIGHT/2;
}

void VertexShader(const vec3& v, Pixel& p)
{
    vec3 vv;
    vec3 rot;
    
    // vv.x = v.x - cameraPos.x;
    // vv.y = v.y - cameraPos.y;
    // vv.z = v.z - cameraPos.z;
    
    // rot = vv*cameraRot;
    
    rot = (v - cameraPos) * cameraRot;
    
    mat3 rzt(vec3(cos(cameraPYR.z), -sin(cameraPYR.z), 0.0f),
             vec3(sin(cameraPYR.z), cos(cameraPYR.z), 0.0f),
             vec3(0.0f, 0.0f, 1.0f));
    
    rot = rot * rzt;
    
    // cout << "vert in camspace: " << rot.x << ", " << rot.y << ", " << rot.z << endl;
    
    // printf("input point: %.2f, %.2f, %.2f\n", v.x, v.y, v.z);
    // printf("output point: %.2f, %.2f, %.2f\n", rot.x, rot.y, rot.z);
    
    p.x = focalLength * (rot.x / rot.z) + SCREEN_WIDTH/2;
    p.y = focalLength * (rot.y / rot.z) + SCREEN_HEIGHT/2;
    p.zinv = 1.0f/rot.z;
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

void Interpolate(Pixel a, Pixel b, vector<Pixel>& result)
{
    int N = result.size();
    
    vec3 step = vec3(b.x - a.x, b.y - a.y, b.zinv - a.zinv) / float(max(N - 1, 1));
    
    vec3 current(a.x, a.y, a.zinv);
    
    for (int i = 0; i < N; ++i)
    {
        result[i].x = round(current.x);
        result[i].y = round(current.y);
        result[i].zinv = current.z;
        
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

void DrawLineSDL(SDL_Surface* surface, Pixel a, Pixel b, vec3 color)
{
    vector<Pixel> line;
    InterpolateLine(a, b, line);
    
    // cout << a.x << ", " << a.y << "  " << b.x << ", " << b.y << " " << line.size() << endl;
    // exit(0);
    
    for (int i = 0; i < line.size(); ++i)
    {
        int x = line[i].x;
        int y = line[i].y;
        float zinv = line[i].zinv;
        
        if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT || zinv < depthBuffer[y][x])
        {
            continue;
        }
        
        PutPixelSDL(screen, x, y, color);
        depthBuffer[y][x] = zinv;
    }
}

void InterpolateLine(const ivec2& a, const ivec2& b, vector<ivec2>& line)
{
    ivec2 delta = glm::abs(a - b);
    int pixels = glm::max(delta.x, delta.y) + 1;
    
    line.resize(pixels);
    Interpolate(a, b, line);
    
    // cout << "InterpolateLine complete" << endl;
}

void InterpolateLine(const Pixel& a, const Pixel& b, vector<Pixel>& line)
{
    Pixel delta{glm::abs(a.x - b.x), glm::abs(a.y - b.y), glm::abs(a.zinv - b.zinv)};
    
    int pixels = glm::max(delta.x, delta.y) + 1;
    
    line.resize(pixels);
    Interpolate(a, b, line);
    
    // for (int i = 0; i < line.size(); ++i)
    // {
        // cout << line[i].x << ", " << line[i].y << ", " << line[i].zinv << endl;
    // }
    // exit(0);
    
    // cout << "InterpolateLine complete" << endl;
}

void ComputeTriangleRows(const ivec2& v0, const ivec2& v1, const ivec2& v2, vector<ivec2>& leftPixels, vector<ivec2>& rightPixels)
{
    // 1. Find max and min y-value of the triangle
    // and compute the number of rows it occupies.
    
    int vmin = min(v0.y, min(v1.y, v2.y));
    int vmax = max(v0.y, max(v1.y, v2.y));
    
    int rows = vmax - vmin + 1;
    
    // printf("min: %d, max: %d, rows: %d\n", vmin, vmax, rows);
    
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
    
    vector<ivec2> e0;
    vector<ivec2> e1;
    vector<ivec2> e2;
    
    InterpolateLine(v0, v1, e0);
    InterpolateLine(v1, v2, e1);
    InterpolateLine(v2, v0, e2);
    
    for (int i = 0; i < e0.size(); ++i)
    {
        int row = e0[i].y - vmin;
        if (row < 0)
        {
            cout << "e0 fail" << endl;
            exit(0);
        }
        
        // cout << "ComputeTriangleRows e0 row " << row << endl;
        
        if (e0[i].x < leftPixels[row].x)
        {
            leftPixels[row].x = e0[i].x;
            leftPixels[row].y = e0[i].y;
        }
        
        if (e0[i].x > rightPixels[row].x)
        {
            rightPixels[row].x = e0[i].x;
            rightPixels[row].y = e0[i].y;
        }
    }
    
    for (int i = 0; i < e1.size(); ++i)
    {
        int row = e1[i].y - vmin;
        if (row < 0)
        {
            cout << "e1 fail" << endl;
            exit(0);
        }
        
        // cout << "ComputeTriangleRows e1 row " << row << endl;
        
        if (e1[i].x < leftPixels[row].x)
        {
            leftPixels[row].x = e1[i].x;
            leftPixels[row].y = e1[i].y;
        }
        
        if (e1[i].x > rightPixels[row].x)
        {
            rightPixels[row].x = e1[i].x;
            rightPixels[row].y = e1[i].y;
        }
    }
    
    for (int i = 0; i < e2.size(); ++i)
    {
        int row = e2[i].y - vmin;
        if (row < 0)
        {
            cout << "e2[" << i << "] = " << e2[i].x << ", " << e2[i].y << " fail" << endl;
            cout << "e0 has size " << e0.size() << endl;
            cout << "e1 has size " << e1.size() << endl;
            cout << "e2 has size " << e2.size() << endl;
            cout << "v0: " << v0.x << ", " << v0.y << endl;
            cout << "v1: " << v1.x << ", " << v1.y << endl;
            cout << "v2: " << v2.x << ", " << v2.y << endl;
            
            for (int j = 0; j < e2.size(); ++j)
            {
                cout << "e2[" << j << "] = " << e2[j].x << ", " << e2[j].y << endl;
            }
            
            exit(0);
        }
        
        // cout << "ComputeTriangleRows e2 row " << row << endl;
        
        if (e2[i].x < leftPixels[row].x)
        {
            leftPixels[row].x = e2[i].x;
            leftPixels[row].y = e2[i].y;
        }
        
        if (e2[i].x > rightPixels[row].x)
        {
            rightPixels[row].x = e2[i].x;
            rightPixels[row].y = e2[i].y;
        }
    }
    
    // cout << "ComputeTriangleRows complete" << endl;
}

void ComputeTriangleRows(const Pixel& v0, const Pixel& v1, const Pixel& v2, vector<Pixel>& leftPixels, vector<Pixel>& rightPixels)
{
    // 1. Find max and min y-value of the triangle
    // and compute the number of rows it occupies.
    
    int vmin = min(v0.y, min(v1.y, v2.y));
    int vmax = max(v0.y, max(v1.y, v2.y));
    
    int rows = vmax - vmin + 1;
    
    // printf("min: %d, max: %d, rows: %d\n", vmin, vmax, rows);
    
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
    
    for (int i = 0; i < e0.size(); ++i)
    {
        int row = e0[i].y - vmin;
        if (row < 0)
        {
            cout << "e0 fail" << endl;
            exit(0);
        }
        
        // cout << "ComputeTriangleRows e0 row " << row << endl;
        
        if (e0[i].x < leftPixels[row].x)
        {
            leftPixels[row].x = e0[i].x;
            leftPixels[row].y = e0[i].y;
            leftPixels[row].zinv = e0[i].zinv;
        }
        
        if (e0[i].x > rightPixels[row].x)
        {
            rightPixels[row].x = e0[i].x;
            rightPixels[row].y = e0[i].y;
            rightPixels[row].zinv = e0[i].zinv;
        }
    }
    
    for (int i = 0; i < e1.size(); ++i)
    {
        int row = e1[i].y - vmin;
        if (row < 0)
        {
            cout << "e1 fail" << endl;
            exit(0);
        }
        
        // cout << "ComputeTriangleRows e1 row " << row << endl;
        
        if (e1[i].x < leftPixels[row].x)
        {
            leftPixels[row].x = e1[i].x;
            leftPixels[row].y = e1[i].y;
            leftPixels[row].zinv = e1[i].zinv;
        }
        
        if (e1[i].x > rightPixels[row].x)
        {
            rightPixels[row].x = e1[i].x;
            rightPixels[row].y = e1[i].y;
            rightPixels[row].zinv = e1[i].zinv;
        }
    }
    
    for (int i = 0; i < e2.size(); ++i)
    {
        int row = e2[i].y - vmin;
        if (row < 0)
        {
            cout << "e2[" << i << "] = " << e2[i].x << ", " << e2[i].y << " fail" << endl;
            cout << "e0 has size " << e0.size() << endl;
            cout << "e1 has size " << e1.size() << endl;
            cout << "e2 has size " << e2.size() << endl;
            cout << "v0: " << v0.x << ", " << v0.y << endl;
            cout << "v1: " << v1.x << ", " << v1.y << endl;
            cout << "v2: " << v2.x << ", " << v2.y << endl;
            
            for (int j = 0; j < e2.size(); ++j)
            {
                cout << "e2[" << j << "] = " << e2[j].x << ", " << e2[j].y << endl;
            }
            
            exit(0);
        }
        
        // cout << "ComputeTriangleRows e2 row " << row << endl;
        
        if (e2[i].x < leftPixels[row].x)
        {
            leftPixels[row].x = e2[i].x;
            leftPixels[row].y = e2[i].y;
            leftPixels[row].zinv = e2[i].zinv;
        }
        
        if (e2[i].x > rightPixels[row].x)
        {
            rightPixels[row].x = e2[i].x;
            rightPixels[row].y = e2[i].y;
            rightPixels[row].zinv = e2[i].zinv;
        }
    }
    
    // cout << "ComputeTriangleRows complete" << endl;
}

void DrawRows(SDL_Surface* surface, const vector<ivec2>& leftPixels, const vector<ivec2>& rightPixels, vec3 color)
{
    int rows = leftPixels.size();
    
    for (int i = 0; i < rows; ++i)
    {
        DrawLineSDL(surface, leftPixels[i], rightPixels[i], color);
    }
}

void DrawRows(SDL_Surface* surface, const vector<Pixel>& leftPixels, const vector<Pixel>& rightPixels, vec3 color)
{
    int rows = leftPixels.size();
    
    // cout << rows << endl;
    
    for (int i = 0; i < rows; ++i)
    {
        DrawLineSDL(surface, leftPixels[i], rightPixels[i], color);
    }
}

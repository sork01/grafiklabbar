#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"

using namespace std;
using glm::vec3;
using glm::vec2;
using glm::ivec2;
using glm::mat3;



struct Pixel
{
    int x;
    int y;
    float zinv;
    vec3 pos3d;
};

struct Vertex
{
    vec3 position;
};
// ----------------------------------------------------------------------------
// GLOBAL VARIABLES

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;
SDL_Surface* screen;
int t;
vector<Triangle> triangles;
vec3 cameraPos( 0, 0, -3.001 );
float focallength = SCREEN_HEIGHT / 2.0f;
float yaw;
mat3 R = glm::mat3(1.0f);
const float mov = 0.02f;
vec3 currentColor;
mat3 rotation(vec3(1.0f,0.0f,0.0f), vec3(0.0f,1.0f,0.0f), vec3(0.0f,0.0f,1.0f));
float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH];


//light
vec3 lightPos(0,-0.5,-0.7);
vec3 lightPower = 14.f*vec3( 1, 1, 1 );
vec3 indirectLightPowerPerArea = 0.5f*vec3( 1, 1, 1 );


vec3 currentNormal;
vec3 currentReflectance;
// ----------------------------------------------------------------------------
// FUNCTIONS

void Update();
void Draw();

void VertexShader( const vec3& v, Pixel& p );
void PixelShader( Pixel& p );
void Interpolate( Pixel a, Pixel b, vector<Pixel>& result );
void drawline(ivec2 a, ivec2 b, vec3 colors);
void ComputePolygonRows(
const vector<Pixel>& vertexPixels,
vector<Pixel>& leftPixels,
vector<Pixel>& rightPixels
);
void DrawRows(const vector<Pixel>& leftPixels,
const vector<Pixel>& rightPixels);
void DrawPolygon( const vector<Vertex>& vertices );

int main( int argc, char* argv[] )
{
    // vector<Pixel> vertexPixels(3);
    // vertexPixels[0] = ivec2(10, 5);
    // vertexPixels[1] = ivec2( 5,10);
    // vertexPixels[2] = ivec2(15,15);
    // vector<Pixel> leftPixels;
    // vector<Pixel> rightPixels;
    // ComputePolygonRows( vertexPixels, leftPixels, rightPixels );
    // for( int row=0; row<leftPixels.size(); ++row )
    // {
    // cout << "Start: ("
    // << leftPixels[row].x << ","
    // << leftPixels[row].y << "). "
    // << "End: ("
    // << rightPixels[row].x << ","
    // << rightPixels[row].y << "). " << endl;
    // }
    // return 0;
    
	LoadTestModel( triangles );
	
     // single test triangle
    // triangles.push_back(Triangle(
        // vec3(0, -1, 2),
        // vec3(1, 1, 2),
        // vec3(-1, 1, 2),
        // vec3(1,1,1)));
    
    // ivec2 a;
    // ivec2 b;
    // ivec2 c;
        
    // VertexShader(triangles[0].v0,a);
    // VertexShader(triangles[0].v1,b);
    // VertexShader(triangles[0].v2,c);
    
    // vector<ivec2> shitstain;
    // shitstain.push_back(a);
    // shitstain.push_back(b);
    // shitstain.push_back(c);
    // vector<ivec2> leftPixels;
    // vector<ivec2> rightPixels;
    // ComputePolygonRows( shitstain, leftPixels, rightPixels );
    // for(int i = 0; i < leftPixels.size(); ++i)
    // {
        
        // cout << "left x : " << leftPixels[i].x << " right x : " << rightPixels[i].x << endl;
        // cout << "left y : " << leftPixels[i].y << " right y : " << rightPixels[i].y << endl;
    
    // }
    // return 0;
        
    
    screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );
	t = SDL_GetTicks();	// Set start value for timer.

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

	Uint8* keystate = SDL_GetKeyState(0);

	if( keystate[SDLK_UP] )
	    cameraPos.z += mov;

	if( keystate[SDLK_DOWN] )
		cameraPos.z -= mov;;

	if( keystate[SDLK_RIGHT] )
		yaw += 0.001f;
        R = mat3(vec3(cos(yaw), 0.0f, sin(yaw)), vec3(0.0f, 1.0f, 0.0f), vec3(-sin(yaw), 0.0f, cos(yaw)));

	if( keystate[SDLK_LEFT] )
		yaw -= 0.001f;
        R = mat3(vec3(cos(yaw), 0.0f, sin(yaw)), vec3(0.0f, 1.0f, 0.0f), vec3(-sin(yaw), 0.0f, cos(yaw)));

	if( keystate[SDLK_RSHIFT] )
		cameraPos.x += mov;;

	if( keystate[SDLK_RCTRL] )
		cameraPos.x -= mov;;

	if( keystate[SDLK_w] )
		lightPos.z +=  mov;;

	if( keystate[SDLK_s] )
		lightPos.z -= mov;;

	if( keystate[SDLK_d] )
		lightPos.x += mov;;

	if( keystate[SDLK_a] )
		lightPos.x -= mov;;

	if( keystate[SDLK_e] )
		lightPos.y -= mov;

	if( keystate[SDLK_q] )
		lightPos.y += mov;
}

void Draw()
{
	SDL_FillRect( screen, 0, 0 );

	if( SDL_MUSTLOCK(screen) )
		SDL_LockSurface(screen);
	for( int y=0; y<SCREEN_HEIGHT; ++y )
        {
            for( int x=0; x<SCREEN_WIDTH; ++x )
            {
                depthBuffer[y][x] = 0;
            }
        }  
    
	for( int i=0; i<triangles.size(); ++i )
	{
        
        // vector<vec3> vertices(3);
        vector<Vertex> biss(3);
        // biss[i].position = triangles[i];
        // biss[0].normal = triangles[i].normal;
        // biss[1].normal = triangles[i].normal;
        // biss[2].normal = triangles[i].normal;
        // biss[0].reflectance = triangles[i].color;
        // biss[1].reflectance = triangles[i].color;
        // biss[2].reflectance = triangles[i].color;
        // currentColor = triangles[i].color;
		biss[0].position = triangles[i].v0;
		biss[1].position = triangles[i].v1;
		biss[2].position = triangles[i].v2;

        
        currentNormal = triangles[i].normal;
        currentReflectance = triangles[i].color;
        DrawPolygon(biss);

	}
	
	if ( SDL_MUSTLOCK(screen) )
		SDL_UnlockSurface(screen);

	SDL_UpdateRect( screen, 0, 0, 0, 0 );
}

void VertexShader( Vertex vertex, Pixel& p )
{
    vec3 pprime = (vertex.position - cameraPos)*R;
    p.x = focallength*pprime.x / pprime.z + SCREEN_WIDTH / 2;
    p.y = focallength*pprime.y / pprime.z + SCREEN_HEIGHT / 2;
    p.zinv =  1.0f / pprime.z;
    
    p.pos3d = vertex.position;
    p.pos3d = p.pos3d * p.zinv;
    // vec3 r = (lightPos - vertex.position);
    // float distance = glm::distance(lightPos, vertex.position);
    // vec3 B = lightPower / (4 * 3.1416f * distance*distance);
    // float dot = glm::dot(vertex.normal, r);
    // if (dot < 0)
    // {
        // dot = 0;
    // }
    // vec3 D = B*dot;
    // p.illumination = vertex.reflectance * D;
    // cout << "x: " << p.illumination.x << " y: " << p.illumination.y << " z: " << p.illumination.z << endl;
    // cout << "refl x: " << vertex.reflectance.x << " y: " << vertex.reflectance.y << " z: " << vertex.reflectance.z << endl;
    // cout << "D x: " << D.x << " y: " << D.y << " z: " << D.z << endl;
    
}


void Interpolate( Pixel a, Pixel b, vector<Pixel>& result )
{
    int N = result.size();
    // vector<vec3> BAJS(N);
    float nosteps = float(glm::max(N - 1, 1));
    vec3 illsteps = (b.pos3d - a.pos3d) / nosteps;
    float zsteps = (b.zinv - a.zinv) / nosteps;
    float currentzinv = a.zinv;
    vec3 currentpos3d = a.pos3d;
    // cout << "what is result.illum?: " << a.illumination << endl;
    // cout << "a efter inter: " << a.zinv << endl;
    vec2 step = vec2(b.x-a.x,b.y-a.y) / float(max(N-1,1));
    vec2 current(a.x,a.y);
    for( int i=0; i<N; ++i )
    {
        result[i].zinv = currentzinv;
        result[i].pos3d = currentpos3d;
        currentzinv += zsteps;        
        currentpos3d +=  illsteps;
        result[i].x = glm::round(current.x);
        result[i].y = glm::round(current.y);
        current += step;
        // result[i].pos3d = BAJS[i] / result[i].zinv;
        // cout << currentpos3d.x << " " << currentpos3d.y << " " << currentpos3d.z << endl;
    }
}

void drawline(Pixel a, Pixel b, vec3 colors)
{
    // cout << "a: "<< a.zinv << endl;
    // cout << "b: "<< b.zinv << endl;
    ivec2 delta = glm::abs( ivec2(a.x-b.x,a.y-b.y) );
    int pixels = glm::max( delta.x, delta.y ) + 1;
    vector<Pixel> line( pixels );
    Interpolate( a, b, line);
    for(int i = 0; i < line.size(); ++i )
    {
        if (line[i].x >= 0 && line[i].x < SCREEN_WIDTH && line[i].y >= 0 && line[i].y < SCREEN_HEIGHT && line[i].zinv > depthBuffer[line[i].y][line[i].x])
        {
        // cout << "shit: " << line[i].zinv << " and dbuff: " << depthBuffer[line[i].y][line[i].x] << endl;
            if (line[i].zinv > depthBuffer[line[i].y][line[i].x])
            {
                depthBuffer[line[i].y][line[i].x] = line[i].zinv;
                // cout << "dbuff: " << depthBuffer[line[i].y][line[i].x] << endl;
                PixelShader(line[i]);
                // PutPixelSDL(screen, line[i].x, line[i].y, currentColor );
            }
        }
    }
}

void PixelShader( Pixel& p )
{
    int x = p.x;
    int y = p.y;
    p.pos3d = (p.pos3d / p.zinv);
    vec3 r = (lightPos - p.pos3d);
    float distance = glm::distance(lightPos, p.pos3d);
    float dot = std::fmax(glm::dot(glm::normalize(r), currentNormal), 0);
    vec3 D = (lightPower * dot) / (4*3.1416f * distance*distance);
    
    
    // p.illumination = vertex.reflectance * D;
    depthBuffer[y][x] = p.zinv;
    // cout << "har skriver jag ut en pixel pa x: " << x << " och y: " << y  << endl;
    // cout << "what is this p.illum: " << p.illumination[2] << endl;
    PutPixelSDL( screen, x, y, currentReflectance * (D + indirectLightPowerPerArea) );
    // }
}

void ComputePolygonRows(const vector<Pixel>& vertexPixels,
vector<Pixel>& leftPixels,vector<Pixel>& rightPixels)
{
    // 1. Find max and min y-value of the polygon
    // and compute the number of rows it occupies.
    int min = numeric_limits<int>::max();
    int max = numeric_limits<int>::min();
    for(int i = 0 ; i < vertexPixels.size(); ++i)
    {
       
        // cout << "compute: " << vertexPixels[i].zinv << endl;
        // if (vertexPixels[i].zinv > depthBuffer[y][x])
        // {
            // depthBuffer[y][x] = vertexPixels[i].zinv;
        // }
        if (vertexPixels[i].y < min)
        {
            min = vertexPixels[i].y;
        }
        if (vertexPixels[i].y > max)
        {
            max = vertexPixels[i].y;
        }
    }
    int rows = max - min + 1;
    // 2. Resize leftPixels and rightPixels
    // so that they have an element for each row.
    leftPixels.resize(rows);
    rightPixels.resize(rows);
    // 3. Initialize the x-coordinates in leftPixels
    // to some really large value and the x-coordinates
    // in rightPixels to some really small value.
    
    
    
    for(int i = 0; i < rows; ++i)
    {
        leftPixels[i].x = numeric_limits<int>::max();
        rightPixels[i].x = numeric_limits<int>::min();
        leftPixels[i].y = min + i;
        rightPixels[i].y = min + i;
    }
    // 4. Loop through all edges of the polygon and use
    // linear interpolation to find the x-coordinate for
    // each row it occupies. Update the corresponding
    // values in rightPixels and leftPixels.
    for(int i = 0; i < vertexPixels.size(); ++i)
    {
        int j = (( i + 1 ) % vertexPixels.size());
        Pixel start = vertexPixels[i];
        Pixel end = vertexPixels[j];
        ivec2 delta = glm::abs( ivec2(start.x-end.x, start.y-end.y) );
        int pixels = glm::max( delta.x, delta.y ) + 1;
        vector<Pixel> line( pixels );
        // cout << "a innan inter: " << start.zinv << endl;
        Interpolate(start,end,line);
        // cout << "start x :" << start.x <<  "start y :" << start.y <<" end x :" << end.x <<" end y :" << end.y << endl;

        for(int k = 0; k < line.size(); ++k)
        {
            int rowshit = line[k].y - min;
            // cout << " k is :" << k << endl << "rowshit is : " << rowshit << endl;

            // if (rowshit == -1)
            // {
                // continue;
            // }
            if (rowshit < 0 || rowshit > max)
            {
                // cout << "you suck mankey balls and rowshit is: " << rowshit << endl;
                // rowshit = 0;
            }
            if (line[k].x < leftPixels[rowshit].x)
            {
                leftPixels[rowshit].x = line[k].x;
                leftPixels[rowshit].y = line[k].y;
                leftPixels[rowshit].zinv = line[k].zinv;
                leftPixels[rowshit].pos3d = line[k].pos3d;
                // leftPixels[rowshit].illumination = line[k].illumination;
            }
               
            if (line[k].x > rightPixels[rowshit].x)
            {
                rightPixels[rowshit].x = line[k].x;
                rightPixels[rowshit].y = line[k].y;
                rightPixels[rowshit].zinv = line[k].zinv;
                rightPixels[rowshit].pos3d = line[k].pos3d;
                // rightPixels[rowshit].illumination = line[k].illumination;
            }
            // leftPixels[rowshit].zinv = vertexPixels[i].zinv;
            // rightPixels[rowshit].zinv = vertexPixels[j].zinv;
            // cout << " most left " << leftPixels[rowshit].x << endl;
            // cout << " most right " << rightPixels[rowshit].x << endl; 
            // cout << "leftcompute: " << leftPixels[i].zinv << endl;
            // cout << "left innan drawrows: " << leftPixels[i].zinv << endl;
        }
    }
    
    
}

void DrawRows( const vector<Pixel>& leftPixels,
const vector<Pixel>& rightPixels )
{
    for(int i = 0; i < leftPixels.size(); ++i)
    {
        // cout << "left efter drawrows: " << leftPixels[i].zinv << endl;
        // PutPixelSDL(screen, leftPixels[i].x, rightPixels[i].x, currentColor);
        
        drawline(leftPixels[i], rightPixels[i], currentColor);
    }
}

void DrawPolygon( const vector<Vertex>& vertices )
{
    int V = vertices.size();
    vector<Pixel> vertexPixels( V );
    for( int i=0; i<V; ++i )
    {
        VertexShader( vertices[i], vertexPixels[i] );
        // cout << "shit: " << vertexPixels[i].zinv << endl;
    }
    vector<Pixel> leftPixels;
    vector<Pixel> rightPixels;
    ComputePolygonRows( vertexPixels, leftPixels, rightPixels );
    DrawRows( leftPixels, rightPixels );
}


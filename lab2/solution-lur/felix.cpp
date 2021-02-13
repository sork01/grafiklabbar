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
int spacing = 1;
const bool autoFps = true;
const int targetDt = 64;
const bool autoFill = true;
const int autoFillRatio = 3;
SDL_Surface* screen;
int t;
vector<Triangle> triangles;
const float focalLength = SCREEN_WIDTH / 2;
vec3 cameraPos(0,0,-2);
vec3 cameraRight;
vec3 cameraDown;
vec3 cameraForward;
const float cameraSpeed = 0.005;
mat3 rotation;
float yaw = 0;
bool yawWasChanged = false;
const float yawspeed = 0.1;

// Illumination
vec3 lightPos(0, -0.5, -0.7);
vec3 lightColor = 14.f * vec3(1,1,1);
vec3 indirectLight = 0.5f*vec3( 1, 1, 1 );

// STRUCTS
struct Intersection{
	vec3 position;
	float distance;
	long triangleIndex;
};



// ----------------------------------------------------------------------------
// FUNCTIONS

void Update();
void Draw();
bool closestIntersection(vec3 start,vec3 dir,const std::vector<Triangle>& triangles,Intersection& closestIntersection);

vec3 directLight(const Intersection& i){
	vec3 rhat = i.position - lightPos;
	float distance = glm::length(rhat);
	rhat = rhat / distance;

	Intersection temp;
	if(closestIntersection(lightPos,rhat,triangles, temp) && temp.triangleIndex != i.triangleIndex){
		return vec3(0,0,0);
	}

	vec3 normal = glm::cross(triangles[i.triangleIndex].v1 - triangles[i.triangleIndex].v0, triangles[i.triangleIndex].v2 - triangles[i.triangleIndex].v0);
	normal = normal / glm::length(normal);
	float scalarproduct = normal[0] * rhat[0] + normal[1] * rhat[1] + normal[2] * rhat[2];
	float multiplier = 0;
	if(scalarproduct > 0){
		multiplier = scalarproduct;
	}
	return (lightColor * multiplier) / float(4 * 3.1415 * distance * distance);
}

bool closestIntersection(
	vec3 start,
	vec3 dir,
	const std::vector<Triangle>& triangles,
	Intersection& closestIntersection
){
	bool found = false;
	closestIntersection.distance = std::numeric_limits<float>::max(); // Distance over 10 is not possible, so all found distances will be lower
	for(int i = 0; i < triangles.size(); i++){
		const mat3 a(-dir, triangles[i].v1 - triangles[i].v0, triangles[i].v2 - triangles[i].v0);
		const vec3 b = start - triangles[i].v0;

		const float adet =   a[0][0]*(a[1][1]*a[2][2] - a[1][2] * a[2][1])
											 + a[0][1]*(a[1][2]*a[2][0] - a[1][0] * a[2][2])
											 + a[0][2]*(a[1][0]*a[2][1] - a[1][1] * a[2][0]);
		const float a1det =  a[0][0]*(b[1]*a[2][2] - b[2] * a[2][1])
											 + a[0][1]*(b[2]*a[2][0] - b[0] * a[2][2])
											 + a[0][2]*(b[0]*a[2][1] - b[1] * a[2][0]);
		float u = a1det / adet;
		if(u >= 0 && u <= 1){
			const float a0det =  b[0]*(a[1][1]*a[2][2] - a[1][2] * a[2][1])
												 + b[1]*(a[1][2]*a[2][0] - a[1][0] * a[2][2])
												 + b[2]*(a[1][0]*a[2][1] - a[1][1] * a[2][0]);
			const float a2det =  a[0][0]*(a[1][1]*b[2] - a[1][2] * b[1])
												 + a[0][1]*(a[1][2]*b[0] - a[1][0] * b[2])
												 + a[0][2]*(a[1][0]*b[1] - a[1][1] * b[0]);
      float v, multiplier;
			v = a2det / adet;
			multiplier = a0det / adet;
			float m = glm::length(multiplier * dir);
			if(v >= 0 && v +u <= 1 && multiplier > 0 && m < closestIntersection.distance){
				found = true;
				closestIntersection.distance = m;
				closestIntersection.position = start + multiplier * dir;
				closestIntersection.triangleIndex = i;
			}

		}

	}

	return found;
}

void updateRotation(float yaw, mat3 & rotation){
	rotation = mat3(vec3(cos(yaw),0,-sin(yaw)), vec3(0,1,0), vec3(sin(yaw), 0, cos(yaw)));
	cameraRight = rotation[0];
	cameraDown = rotation[1];
	cameraForward = rotation[2];
}

int main( int argc, char* argv[] )
{
	screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );
	t = SDL_GetTicks();	// Set start value for timer.
  LoadTestModel(triangles);
	updateRotation(yaw, rotation);
	while( NoQuitMessageSDL() ){
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
	if(autoFps && dt > targetDt){
		SDL_FillRect( screen, 0, 0 );
		spacing++;
		//cout << "Changed spacing: " << spacing << endl;
	}else if(autoFps && dt < float(targetDt) / 4 && spacing != 1){
		SDL_FillRect( screen, 0, 0 );
		spacing--;
	}else{
		cout << "Render time: " << dt << " ms." << endl;
	}
	Uint8* keystate = SDL_GetKeyState( 0 );
	if( keystate[SDLK_w] ){
		// Move camera forward
		cameraPos += cameraForward*cameraSpeed * dt;
	}
	if( keystate[SDLK_s] ){
		// Move camera backward
		cameraPos -= cameraForward*cameraSpeed * dt;
	}
	if( keystate[SDLK_a] )
	{
		// Move camera to the left
		cameraPos -= cameraRight*cameraSpeed * dt;
	}
	if( keystate[SDLK_d] )
	{
		// Move camera to the right
		cameraPos += cameraRight*cameraSpeed * dt;
	}
	if(keystate[SDLK_q]){
		//Rotate camera to the left
		yaw -= yawspeed;
		yawWasChanged = true;
	}
	if(keystate[SDLK_e]){
		//Rotate camera to the left
		yaw += yawspeed;
		yawWasChanged = true;
	}
	if(yawWasChanged){
		yaw = fmod(yaw,2 * 3.14f);
		updateRotation(yaw, rotation);
		yawWasChanged = false;
	}
    
    if (keystate[SDLK_UP])
    {
        lightPos += vec3(0.0f, 0.0f, 0.1f);
    }
    if (keystate[SDLK_DOWN])
    {
        lightPos -= vec3(0.0f, 0.0f, 0.1f);
    }
    if (keystate[SDLK_LEFT])
    {
        lightPos -= vec3(0.1f, 0.0f, 0.0f);
    }
    if (keystate[SDLK_RIGHT])
    {
        lightPos += vec3(0.1f, 0.0f, 0.0f);
    }
}

void Draw()
{
	if( SDL_MUSTLOCK(screen) )
		SDL_LockSurface(screen);
	Intersection inter;
	if(autoFill){
		SDL_FillRect( screen, 0, 0 );
	}
	for( int y=0; y<SCREEN_HEIGHT; y += spacing )
	{
		for( int x=0; x<SCREEN_WIDTH; x += spacing )
		{

			vec3 d = vec3(x - float(SCREEN_WIDTH) / 2, y - float(SCREEN_HEIGHT)/2, focalLength);
			//cout << "pred: " << d[0] << ", " << d[1] << ", " << d[2];
			d = rotation * d;
			vec3 color = vec3(0,0,0);
			//cout << " postd: " << d[0] << ", " << d[1] << ", " << d[2] << endl;
			if(closestIntersection(cameraPos, d, triangles, inter)){
				color = triangles[inter.triangleIndex].color * (directLight(inter) + indirectLight);
				 //triangles[inter.triangleIndex].color );
				//cout << "(" << color.x << ", " << color.y << ", " << color.z << ")"<< endl;
			}
			if(autoFill){
				for(int i = 0; i < spacing; i += 1){
					PutPixelSDL(screen, x + i, y, color);

				}
				for(int i = 1; i < spacing; i++){
					PutPixelSDL(screen, x, y + i, color);
				}


			}else{
				PutPixelSDL( screen, x, y, color);
			}
		}
		//cout << y << " / " << SCREEN_HEIGHT << endl;
	}
	//cout << "KLAR" << endl;
	if( SDL_MUSTLOCK(screen) )
		SDL_UnlockSurface(screen);

	SDL_UpdateRect( screen, 0, 0, 0, 0 );
}
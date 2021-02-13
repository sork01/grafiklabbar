
// *neode.command* setgo gcc -O3 -I./glm -I/usr/include/SDL -lstdc++ -lm -lSDL test.cpp -o test; ./test
#include <iostream>
#include <glm/glm.hpp>

using namespace std;
using glm::vec2;
using glm::vec3;
using glm::ivec2;
using glm::mat3;

int main(int argc, char* argv[])
{
    vec3 a(1,2,3);
    vec3 b = a;
    a.x = 10;
    cout << a.x << ", " << a.y << ", " << a.z << endl;
    cout << b.x << ", " << b.y << ", " << b.z << endl;
    
	return 0;
}


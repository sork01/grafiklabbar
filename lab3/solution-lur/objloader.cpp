
// *neode.command* setgo gcc -lstdc++ -lm objloader.cpp -o objloader; ./objloader
#ifndef HAVE_OBJLOADER_H
#define HAVE_OBJLOADER_H

#include <string>
#include <cstring>
#include <iostream>
#include <fstream>

#include "TestModel.h"

using namespace std;

void LoadOBJModel(string filename, vector<Triangle>& triangles)
{
    ifstream data;
    data.open(filename);
    
    string op, face_elem0, face_elem1, face_elem2;
    char c;
    
    float v0, v1, v2;
    
    int face_vertex, face_texture, face_normal;
    
    vector<glm::vec3> vertices;
    vector<glm::vec3> faces;
    
    while (data.good())
    {
        data >> op;
        
        if (data.fail())
        {
            data >> c;
            
            if (data.eof())
            {
                data.clear();
                break;
            }
        }
        
        if (op != "" && op.at(0) == '#')
        {
            // cout << "comment" << endl;
            while (data.good() && data.get(c) && c != '\n');
        }
        else if (op == "v")
        {
            // cout << "vertex" << endl;
            data >> v0 >> v1 >> v2;
            vertices.push_back(glm::vec3(v0, v1, v2));
        }
        else if (op == "f")
        {
            // cout << "face" << endl;
            
            face_elem0 = "";
            face_elem1 = "";
            face_elem2 = "";
            
            data >> face_elem0 >> face_elem1 >> face_elem2;
            
            string face_elements[3] = {face_elem0, face_elem1, face_elem2};
            int face_vertices[3] = {0,0,0};
            
            for (int i = 0; i < 3; ++i)
            {
                if (sscanf(face_elements[i].c_str(), "%d/%d/%d", &face_vertex, &face_texture, &face_normal) == 3)
                {
                    face_vertices[i] = face_vertex;
                }
                else if (sscanf(face_elements[i].c_str(), "%d//%d", &face_vertex, &face_normal) == 2)
                {
                    face_vertices[i] = face_vertex;
                }
                else if (sscanf(face_elements[i].c_str(), "%d", &face_vertex) == 1)
                {
                    face_vertices[i] = face_vertex;
                }
                else
                {
                    data.setstate(data.failbit);
                    break;
                }
            }
            
            if (data.good())
            {
                faces.push_back(glm::vec3(face_vertices[0], face_vertices[1], face_vertices[2]));
            }
        }
        else
        {
            // cout << "unknown: " << op << endl;
            while (data.good() && data.get(c) && c != '\n');
        }
        
        op = "";
    }
    
    if (data.fail())
    {
        cout << "failed" << endl;
    }
    else
    {
        triangles.reserve(faces.size());
        
        for (int i = 0; i < faces.size(); ++i)
        {
            glm::vec3 face = faces[i] - glm::vec3(1,1,1);
            
            if (face.x < 0 || face.x >= vertices.size() || face.y < 0 || face.y >= vertices.size() || face.z < 0 || face.z >= vertices.size())
            {
                // error: vertex index out of range
                break;
            }
            
            triangles.push_back(Triangle(vertices[face.x], vertices[face.y], vertices[face.z], glm::vec3(1,1,1)));
        }
    }
    
    data.close();
}

int main(int argc, char** argv)
{
    vector<Triangle> triangles;
    LoadOBJModel("../wt_teapot.obj", triangles);
}

#endif

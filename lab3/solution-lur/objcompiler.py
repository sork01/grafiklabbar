
# *neode.command* setgo python3 objcompiler.py
#!/usr/bin/env python3
# -*- coding: utf-8 -*-

vertices = list()
vertices.append(None)

faces = list()

# print(list(map(lambda x: x.strip(), open("../wt_teapot.obj").readlines()))[3])

for line in map(lambda x: x.strip(), open("../wt_teapot.obj").readlines()):
    if line[0] == "#":
        continue
    
    line = line.split(" ")
    kw = line[0]
    
    if kw == "v":
        vertices.append([line[1], -float(line[2]), line[3]])
    
    elif kw == "f":
        faces.append([line[1].split("/")[0], line[2].split("/")[0], line[3].split("/")[0]])

print("using glm::vec3;")
print("vec3 white(0.75f, 0.75f, 0.75f);")
print("triangles.clear();")
print("triangles.reserve(" + str(len(faces)) + ");")

for face in faces:
    v0 = repr(vertices[int(face[0])]).replace("'", "").replace("[", "").replace("]", "")
    v1 = repr(vertices[int(face[1])]).replace("'", "").replace("[", "").replace("]", "")
    v2 = repr(vertices[int(face[2])]).replace("'", "").replace("[", "").replace("]", "")
    
    print("triangles.push_back(Triangle(vec3(" + v0 + "), vec3(" + v1 + "), vec3(" + v2 + "), white));")
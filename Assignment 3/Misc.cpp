#include "Misc.h"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include <map>

Model loadOBJ(const char* objPath)
{
	// function to load the obj file
	// Note: this simple function cannot load all obj files.

	struct V {
		// struct for identify if a vertex has showed up
		unsigned int index_position, index_uv, index_normal;
		bool operator == (const V& v) const {
			return index_position == v.index_position && index_uv == v.index_uv && index_normal == v.index_normal;
		}
		bool operator < (const V& v) const {
			return (index_position < v.index_position) ||
				(index_position == v.index_position && index_uv < v.index_uv) ||
				(index_position == v.index_position && index_uv == v.index_uv && index_normal < v.index_normal);
		}
	};

	std::vector<glm::vec3> temp_positions;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	std::map<V, unsigned int> temp_vertices;

	Model model;
	unsigned int num_vertices = 0;

	std::cout << "\nLoading OBJ file " << objPath << "..." << std::endl;

	std::ifstream file(objPath);

	// Check for Error
	if (file.fail()) 
    {
		std::cerr << "Impossible to open the file! Do you use the right path? See Tutorial 6 for details" << std::endl;
		exit(1);
	}
    
    for(std::string line; std::getline(file, line); )
    {
        std::istringstream in(line);
        std::vector<std::string> line_vec=std::vector<std::string>(std::istream_iterator<std::string>(in), std::istream_iterator<std::string>());
        
        if(line_vec.size()==0) continue;

		// process the object file
		const char *lineHeader=line_vec[0].c_str();

		if (strcmp(lineHeader, "v") == 0) 
        {
			// geometric vertices
            //std::assert(line_vec.size()==4);
			glm::vec3 position = glm::vec3(std::atof(line_vec[1].c_str()), std::atof(line_vec[2].c_str()), std::atof(line_vec[3].c_str()));
			temp_positions.push_back(position);
		}
		else if (strcmp(lineHeader, "vt") == 0) 
        {
			// texture coordinates
            //std::assert(line_vec.size()==3);
			glm::vec2 uv = glm::vec2(std::atof(line_vec[1].c_str()), std::atof(line_vec[2].c_str()));
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) 
        {
			// vertex normals
            //std::assert(line_vec.size()==4);
			glm::vec3 normal = glm::vec3(std::atof(line_vec[1].c_str()), std::atof(line_vec[2].c_str()), std::atof(line_vec[3].c_str()));
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) 
        {
			// Face elements
            //std::assert((line_vec.size()==4 || line_vec.size()==5));
            int n =line_vec.size()-1;
            if(n!=3 && n!=4)
            {
                std::cerr << "There may exist some errors while loading the obj file."<<std::endl;
                std::cerr << "Error content: ["<<line<<std::endl;
                std::cerr << "Can only handle triangles or quads in the obj file for now."<<std::endl;
                exit(1);
            }

            std::vector<V> vertices(n);
			for (int i = 0; i < n; i++) 
            {
                std::stringstream ss(line_vec[i+1]);
                std::string item;
                char delim='/';
                getline(ss, item, delim); int ip=std::atoi(item.c_str());
                getline(ss, item, delim); int it=std::atoi(item.c_str());
                getline(ss, item, delim); int in=std::atoi(item.c_str());
                vertices[i].index_position = ip;
                vertices[i].index_uv = it;
                vertices[i].index_normal = in;
			}
            
            std::vector<int> idxs;
			for (int i = 0; i < n; i++) 
            {
				if (temp_vertices.find(vertices[i]) == temp_vertices.end()) 
                {
					// the vertex never shows before
					Vertex vertex;
					vertex.position = temp_positions[vertices[i].index_position - 1];
					vertex.uv = temp_uvs[vertices[i].index_uv - 1];
					vertex.normal = temp_normals[vertices[i].index_normal - 1];

					model.vertices.push_back(vertex);
                    idxs.push_back(num_vertices);
					temp_vertices[vertices[i]] = num_vertices;
					num_vertices += 1;
				}
				else 
                {
					// reuse the existing vertex
					unsigned int index = temp_vertices[vertices[i]];
                    idxs.push_back(index);
				}
			} // for
            if(n==3) 
            {
                model.indices.push_back(idxs[0]);
                model.indices.push_back(idxs[1]);
                model.indices.push_back(idxs[2]);
            }
            else
            {   // split a quad into two triangles
                model.indices.push_back(idxs[0]);
                model.indices.push_back(idxs[1]);
                model.indices.push_back(idxs[2]);

                model.indices.push_back(idxs[0]);
                model.indices.push_back(idxs[2]);
                model.indices.push_back(idxs[3]);
            }
		} // else if
		else 
        {
			// skip it. it's not a vertex, texture coordinate, normal or face
            std::cout << "skipped: ["<<line<<"]"<<std::endl;
		}
	}
    // NOTE: vertices with the same position but different uv or normal
    // are counted as different vertices during the OBJ loading
	std::cout << "There are " << num_vertices << " vertices and " << model.indices.size()/3 << " triangles in the obj file.\n" << std::endl;
    
	return model;
}

void normalize_to_unit_bbox(std::vector<Vertex>& verts)
{
    float INF=1e+6;
    glm::vec3 p1(INF, INF, INF);
    glm::vec3 p2=-p1;

    for(int i=0; i<verts.size(); i++)
    {
        p1.x = glm::min(p1.x, verts[i].position.x);
        p1.y = glm::min(p1.y, verts[i].position.y);
        p1.z = glm::min(p1.z, verts[i].position.z);

        p2.x = glm::max(p2.x, verts[i].position.x);
        p2.y = glm::max(p2.y, verts[i].position.y);
        p2.z = glm::max(p2.z, verts[i].position.z);
    }
    
    glm::vec3 center = 0.5f*(p1+p2);
    glm::vec3 bbox = p2-p1;

    float S=glm::max(glm::max(bbox.x, bbox.y), bbox.z);
    for(int i=0; i<verts.size(); i++)
        verts[i].position = (verts[i].position - center)/S; 
}


void calc_bbox_and_center(const std::vector<Vertex>& verts)
{
    float INF=1e+6;
    glm::vec3 p1(INF, INF, INF);
    glm::vec3 p2=-p1;

    for(int i=0; i<verts.size(); i++)
    {
        p1.x = glm::min(p1.x, verts[i].position.x);
        p1.y = glm::min(p1.y, verts[i].position.y);
        p1.z = glm::min(p1.z, verts[i].position.z);

        p2.x = glm::max(p2.x, verts[i].position.x);
        p2.y = glm::max(p2.y, verts[i].position.y);
        p2.z = glm::max(p2.z, verts[i].position.z);
    }
    
    glm::vec3 center = 0.5f*(p1+p2);
    glm::vec3 bbox = p2-p1;
    printf("Center %f %f %f\n", center.x, center.y, center.z);
    printf("DX %f DY %f DZ %f\n", bbox.x, bbox.y, bbox.z);
}



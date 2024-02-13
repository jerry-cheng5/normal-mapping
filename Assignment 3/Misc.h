#pragma once

#include "./Dependencies/glm/glm.hpp"

#include <vector>

// struct for storing the obj file
struct Vertex {
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
};

struct Model {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

Model loadOBJ(const char* objPath);

void calc_bbox_and_center(const std::vector<Vertex>& verts);

void normalize_to_unit_bbox(std::vector<Vertex>& verts);


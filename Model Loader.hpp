#pragma once
#include <fstream>
#include <vector>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct sphereStruct {
	glm::vec4 pos;
	// color.w contains radius
	glm::vec4 col;
};

// retruns largest of three values
float max(float a, float b, float c) {
	float d = a > b ? a : b;
	return d > c ? d : c;
}

// return smallest of three values
float min(float a, float b, float c) {
	float d = a < b ? a : b;
	return d < c ? d : c;
}

struct triangleStruct {
	glm::vec4 p[3];
	glm::vec4 color;
	// pos.w contains volume of bounding volume for triangle
	glm::vec4 boundingVolume;
	glm::vec4 boundingCube;
};

class ModelLoader {
public:
	void load(std::vector<triangleStruct>&triangles) {
		std::fstream read("model.txt");
		std::vector<glm::vec3>vertices;
		std::vector<unsigned int>indices;
		char type;
		float p[3];

		while (read >> type) {
			if (type == 'v' || type == 'f') {
				read >> p[0] >> p[1] >> p[2];
				
				if (type == 'v') {
					glm::vec3 t;
					t = glm::vec3(p[0], p[1], p[2]);
					vertices.push_back(t);
				}

				else if (type == 'f') {
					indices.push_back(int(p[0]));
					indices.push_back(int(p[1]));
					indices.push_back(int(p[2]));
				}

			// skip to next line of file
			read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			}
		}

		for (int i = 0; i < indices.size(); i+=3) {
			triangleStruct temp;
			temp.p[0] = glm::vec4(vertices[indices[i] - 1], 0);
			temp.p[1] = glm::vec4(vertices[indices[i + 1] - 1], 0);
			temp.p[2] = glm::vec4(vertices[indices[i + 2] - 1], 0);
			temp.color = glm::vec4(1);
			triangles.push_back(temp);
		}
	}

	// calculate radius of bounding volume of a triangle
	inline void triangleBoundingVolume(triangleStruct& t) {
		// mid point of triangle
		glm::vec3 midPoint;
		midPoint.x = (t.p[0].x + t.p[1].x + t.p[2].x) / 3;
		midPoint.y = (t.p[0].y + t.p[1].y + t.p[2].y) / 3;
		midPoint.z = (t.p[0].z + t.p[1].z + t.p[2].z) / 3;
		t.boundingVolume = glm::vec4(midPoint, 0);

		float a = sqrt(pow(t.p[0].x, 2) + pow(t.p[0].y, 2) + pow(t.p[0].z, 2));
		float b = sqrt(pow(t.p[1].x, 2) + pow(t.p[1].y, 2) + pow(t.p[1].z, 2));
		float c = sqrt(pow(t.p[2].x, 2) + pow(t.p[2].y, 2) + pow(t.p[2].z, 2));
		
		// radius of bounding volume
		t.boundingVolume.w = (a > b) ? ((a > c) ? a : c) : ((b > c) ? b : c);

		// calculate width, height and depth of bounding cube
		glm::vec4 cube;
		cube.x = std::abs(max(t.p[0].x - midPoint.x, t.p[1].x - midPoint.x, t.p[2].x - midPoint.x)) + std::abs(min(t.p[0].x - midPoint.x, t.p[1].x - midPoint.x, t.p[2].x - midPoint.x));
		cube.y = std::abs(max(t.p[0].y - midPoint.y, t.p[1].y - midPoint.y, t.p[2].y - midPoint.y)) + std::abs(min(t.p[0].y - midPoint.y, t.p[1].y - midPoint.y, t.p[2].y - midPoint.y));
		cube.z = std::abs(max(t.p[0].z - midPoint.z, t.p[1].z - midPoint.z, t.p[2].z - midPoint.z)) + std::abs(min(t.p[0].y - midPoint.z, t.p[1].y - midPoint.z, t.p[2].y - midPoint.z));
		cube.w = 0;

		t.boundingCube = cube;
	}

	// returns radius of bounding volume
	float boundingVolume(std::vector<triangleStruct>&triangles) {
		float largestVertice = 0;
		for (int i = 0; i < triangles.size(); i++) {
			triangleBoundingVolume(triangles[i]);
			if (triangles[i].boundingVolume.w > largestVertice)
				largestVertice = triangles[i].boundingVolume.w;
		}


		return largestVertice;
	}
};
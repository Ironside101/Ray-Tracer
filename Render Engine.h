#pragma once


class Engine {
public:
	// mesh loader class
	ModelLoader loader;
	// mesh data
	std::vector<triangleStruct>triangles;
	// lights data
	std::vector<sphereStruct>lights;
	std::vector<sphereStruct>spheres;
	
	unsigned int texture;
	unsigned int lightSSBO, sphereSSBO, triangleSSBO;

	float boundingVolume;

	const int TEXTURE_WIDTH = 1024, TEXTURE_HEIGHT = 1024;

	// complete all setup in constructor
	Engine();

	// called per frame
	void render();
};

Engine::Engine() {
	// load mesh data into "triangles"	
	loader.load(triangles);
	boundingVolume = loader.boundingVolume(triangles);


	// load lights data into "lights"
	std::fstream read("lights.txt");
	float d[6];
	int i = 0;
	while (read >> d[i]) {
		i++;
		if (i == 6) {
			sphereStruct t;

			t.pos = glm::vec4(d[0], d[1], d[2], 0);
			t.col = glm::vec4(d[3], d[4], d[5], 0.1);
			lights.push_back(t);

			i = 0;
		}
	}

	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);

	glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);



	glGenBuffers(1, &lightSSBO);
	glGenBuffers(1, &sphereSSBO);
	glGenBuffers(1, &triangleSSBO);

	// send data to gpu
	if (lights.size() > 0) {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(sphereStruct) * lights.size(), &lights[0], GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, lightSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	if (spheres.size() > 0) {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, sphereSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(sphereStruct) * spheres.size(), &spheres[0], GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, sphereSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	if (triangles.size() > 0) {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangleSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(triangleStruct) * triangles.size(), &triangles[0], GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, triangleSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
}

void Engine::render() {}
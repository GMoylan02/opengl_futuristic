/*

#ifndef CHUNKLOADER_H
#define CHUNKLOADER_H

#define CHUNK_SIZE 512
#include <vector>

#include "asset.h"
#include "cube.h"
#include "ground.h"

class Chunk {
public:
    int x, z;
    Plane plane;
    std::vector<Cube> buildings;
    std::vector<Asset> assets;
    Chunk(int x, int z);
    ~Chunk();
    void render(glm::mat4 cameraMatrix);
    void renderDepth(GLuint programID, GLuint lightMatID, GLuint tranMatID, const glm::mat4& lightSpaceMatrix);
    void unload();
};
double distance(const std::pair<int, int>& p1, const std::pair<int, int>& p2);
std::vector<std::pair<int, int>> generateRandomPoints();



#endif //CHUNKLOADER_H

*/

/*

#include "chunkLoader.h"

#include <iostream>
#include <random>
using namespace std;

random_device rd{};
mt19937 gen(rd());
uniform_int_distribution<> numBuildingsRand(1, 3);
uniform_int_distribution<> buildingTexture(0, 4);
uniform_int_distribution<> buildingHeight(100, 300);
uniform_int_distribution<> coinFlip(0,1);

// if center pos is x, z and length is 512, radius 256
//buildings will be 40 accross, between 100 and 300 tall so
// x+-220, z +- 220, both z and x of every block will have to be 60 apart as well

string texture_paths[] = {"../final/assets/building1.jpg","../final/assets/building2.jpg","../final/assets/building3.jpg",
    "../final/assets/building4.jpg","../final/assets/building5.jpg",};

Chunk::Chunk(int x, int z) : x(x), z(z), plane(glm::vec3(x, 0, z), glm::vec3(CHUNK_SIZE, 10, CHUNK_SIZE),
    "../final/assets/ground.jpg") {
    cout << plane.position.y << "\n";
    int numBuildings = numBuildingsRand(gen);
    vector<pair<int, int>> points = generateRandomPoints();

    for (int i = 0; i < 5; i++) {
        string texturePath = texture_paths[buildingTexture(gen)];
        int height = buildingHeight(gen);
        //todo 512 - x etc
        if (i < numBuildings) {
            buildings.emplace_back(plane.programID, glm::vec3(points[i].first, height, points[i].second),
            glm::vec3(40, height, 40), texturePath.c_str());
        }
        else {
            if (coinFlip(gen)) {
                assets.emplace_back(plane.programID,
                    glm::vec3(points[i].first, 0, points[i].second), glm::vec3(15, 15, 15),
                    "../final/assets/tree_small_02/tree_small_02_1k.gltf");
            }
            else {
                assets.emplace_back(plane.programID,
                     glm::vec3(points[i].first, buildingHeight(gen)/4, points[i].second), glm::vec3(15, 15, 15),
                     "../final/assets/car2/scene.gltf");
            }
        }
    }
}

Chunk::~Chunk() {
    unload(); // Free GPU resources on destruction
}

void Chunk::render(glm::mat4 cameraMatrix) {
    plane.render(cameraMatrix);
    //for (Cube building : buildings) {
    //    building.render(cameraMatrix);
    //}
    //for (Asset asset : assets) {
    //    asset.render(cameraMatrix);
    //}
}

void Chunk::renderDepth(GLuint programID, GLuint lightMatID, GLuint tranMatID, const glm::mat4 &lightSpaceMatrix) {
    plane.renderDepth(programID, lightMatID, tranMatID, lightSpaceMatrix);
    for (Cube building : buildings) {
        building.renderDepth(programID, lightMatID, tranMatID, lightSpaceMatrix);
    }
}

void Chunk::unload() {
    plane.cleanup();
}

double distance(const std::pair<int, int>& p1, const std::pair<int, int>& p2) {
    return std::sqrt((p1.first - p2.first) * (p1.first - p2.first) +
                     (p1.second - p2.second) * (p1.second - p2.second));
}

std::vector<std::pair<int, int>> generateRandomPoints() {
    const int LARGE_SQUARE_SIZE = 512;
    const int SQUARE_SIZE = 40;
    const int MIN_DISTANCE = 15 + SQUARE_SIZE;

    std::vector<std::pair<int, int>> points;
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    while (points.size() < 5) {
        int x = std::rand() % (LARGE_SQUARE_SIZE - SQUARE_SIZE) + SQUARE_SIZE / 2;
        int y = std::rand() % (LARGE_SQUARE_SIZE - SQUARE_SIZE) + SQUARE_SIZE / 2;

        std::pair<int, int> candidate = {x, y};
        bool valid = true;

        for (const auto& p : points) {
            if (distance(candidate, p) < MIN_DISTANCE) {
                valid = false;
                break;
            }
        }

        if (valid) {
            points.push_back(candidate);
        }
    }

    return points;
}

*/
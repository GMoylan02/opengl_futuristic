#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <skybox.h>
#include <asset.h>

// GLTF model loader
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#include <render/shader.h>

#include <vector>
#include <iostream>
#define _USE_MATH_DEFINES
#include <chrono>
#include <iomanip>
#include <math.h>

#include <cube.h>
#include <ground.h>
#include <random>
#include <sstream>
#include <thread>

#include "instance.h"
#include "lighting.h"

#include <MyBot.cpp>
#define CHUNK_SIZE 512

const char* groundFilePath = "../final/assets/ground.jpg";

std::chrono::time_point<std::chrono::steady_clock> lastCheckTime = std::chrono::steady_clock::now();
const std::chrono::milliseconds CHECK_INTERVAL(500); // 500ms

std::random_device rd{};
std::mt19937 gen(rd());
std::uniform_int_distribution<> numBuildingsRand(1, 3);
std::uniform_int_distribution<> buildingTexture(0, 4);
std::uniform_int_distribution<> buildingHeight(100, 300);
std::uniform_int_distribution<> coinFlip(0,1);


static GLFWwindow *window;
static int windowWidth = 1024;
static int windowHeight = 768;

glm::mat4 projectionMatrix;


// Shadow mapping
static glm::vec3 lightUp(0, 0, 1);
static int shadowMapWidth = 2048;
static int shadowMapHeight = 2048;

static float depthFoV = 120.0f;
static float depthNear = 10.0f;
static float depthFar = 4000.0f;



float yaw = -90.0f; // Horizontal angle
float pitch = 0.0f; // Vertical angle
float lastX = 400.0f; // Initial mouse X position (center of the screen)
float lastY = 300.0f; // Initial mouse Y position
float sensitivity = 0.1f; // Mouse sensitivity
float cameraSpeed = 150.0f;
bool firstMouse = true; // To ignore the large delta on first input

glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);

// View control
static float viewAzimuth = 0.f;
static float viewPolar = 0.f;
static float viewDistance = 300.0f;

std::vector<Asset> assets;
std::vector<Cube> cubes;
std::vector<Plane> planes;
std::unordered_map<std::pair<int, int>, Plane, PairHash> pointToPlane;
std::unordered_map<Plane, std::vector<Asset>, PlaneHash> planeToAssets;
std::unordered_map<Plane, std::vector<Cube>, PlaneHash> planeToCubes;
std::unordered_map<Plane, lighting, PlaneHash> planeToLight;
std::unordered_map<Plane, std::vector<Plane>, PlaneHash> adjacent;


std::string texture_paths[] = {"../final/assets/building1.jpg","../final/assets/building2.jpg","../final/assets/building3.jpg",
	"../final/assets/building4.jpg","../final/assets/building5.jpg",};


void onChunkChanged(int currentChunkX, int currentChunkZ);
std::vector<int> generateRandomHeights();
std::vector<std::pair<int, int>> generateRandomPoints();

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	// Calculate mouse delta
	float xOffset = xpos - lastX;
	float yOffset = lastY - ypos; // Reversed since y-coordinates range from bottom to top
	lastX = xpos;
	lastY = ypos;

	// Apply sensitivity
	xOffset *= sensitivity;
	yOffset *= sensitivity;

	// Update yaw and pitch
	yaw += xOffset;
	pitch += yOffset;

	// Constrain pitch to avoid screen flipping
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	// Update camera direction
	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}


static void processInput(GLFWwindow* window, float deltaTime) {
	float velocity = cameraSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += velocity * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= velocity * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * velocity;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * velocity;
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		cameraPos += velocity * cameraUp;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		cameraPos -= velocity * cameraUp;

}

int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW." << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(windowWidth, windowHeight, "debug", NULL, NULL);
	if (window == NULL)
	{
		std::cerr << "Failed to open a GLFW window." << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetCursorPosCallback(window, mouse_callback);

	// Load OpenGL functions, gladLoadGL returns the loaded version, 0 on error.
	int version = gladLoadGL(glfwGetProcAddress);
	if (version == 0)
	{
		std::cerr << "Failed to initialize OpenGL context." << std::endl;
		return -1;
	}

	int currentChunkX = static_cast<int>(cameraPos.x) / CHUNK_SIZE;
	int currentChunkZ = static_cast<int>(cameraPos.z) / CHUNK_SIZE;

	int lastChunkX = currentChunkX;
	int lastChunkZ = currentChunkZ;

	// Prepare shadow map size for shadow mapping. Usually this is the size of the window itself, but on some platforms like Mac this can be 2x the size of the window. Use glfwGetFramebufferSize to get the shadow map size properly.
	glfwGetFramebufferSize(window, &shadowMapWidth, &shadowMapHeight);

	// Background
	glClearColor(0.2f, 0.2f, 0.25f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	Asset car(0, glm::vec3(20, 40, -100), glm::vec3(15, 15, 15), "../final/assets/car2/scene.gltf");
	assets.push_back(car);
	onChunkChanged(0,0);

	std::vector<Instance> instances;
	instances.emplace_back(glm::vec3(170.0f, 0.0f, -50.0f), glm::vec3(15), glm::vec3(0.0f));
	instances.emplace_back(glm::vec3(80.0f, 0.0f, -130.0f), glm::vec3(15), glm::vec3(0.0f));
	instances.emplace_back(glm::vec3(-120.0f, 0.0f, -50.0f), glm::vec3(15), glm::vec3(0.0f));

    Asset tree(planes[0].programID, glm::vec3(20, 0, 100), glm::vec3(15, 15, 15), "../final/assets/tree_small_02/tree_small_02_1k.gltf");

	SkyBox skybox;
	skybox.initialize(glm::vec3(0,0,0), glm::vec3(1,1,1));

	MyBot bot;
	bot.initialize();

	eye_center.y = viewDistance * cos(viewPolar);
	eye_center.x = viewDistance * cos(viewAzimuth);
	eye_center.z = viewDistance * sin(viewAzimuth);

	glm::mat4 viewMatrix;
	glm::float32 FoV = 45;
	glm::float32 zNear = 0.1f;
	glm::float32 zFar = 1000.0f;
	projectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, zNear, zFar);

	// Time and frame rate tracking
	static double lastTime = glfwGetTime();
	float time = 0.0f;			// Animation time
	float fTime = 0.0f;			// Time for measuring fps
	unsigned long frames = 0;

	for (int i = -1; i < 1; i++) {
		for (int j = -1; j < 1; j++) {
			onChunkChanged(i, j);
		}
	}
	do
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Update states for animation
        double currentTime = glfwGetTime();
        float deltaTime = float(currentTime - lastTime);
		lastTime = currentTime;
		if (deltaTime <= 0.0f) {
			deltaTime = 0.01f;
		}
		auto currTime = std::chrono::steady_clock::now();
		if (currTime - lastCheckTime >= CHECK_INTERVAL) {
			currentChunkX = static_cast<int>(std::floor((cameraPos.x + CHUNK_SIZE / 2) / CHUNK_SIZE));
			currentChunkZ = static_cast<int>(std::floor((cameraPos.z + CHUNK_SIZE / 2) / CHUNK_SIZE));

			if (currentChunkX != lastChunkX || currentChunkZ != lastChunkZ) {
				onChunkChanged(currentChunkX, currentChunkZ);
				lastChunkX = currentChunkX;
				lastChunkZ = currentChunkZ;
			}
			lastCheckTime = currTime;
		}

		if (playAnimation) {
			time += deltaTime * playbackSpeed;
			bot.update(time);
		}
		processInput(window, deltaTime);

		// Rendering
		viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		glm::mat4 vp = projectionMatrix * viewMatrix;

		skybox.position = cameraPos;
		glDepthMask(GL_FALSE); // Disable depth writes
		skybox.render(vp);     // Render the skybox
		glDepthMask(GL_TRUE);  // Re-enable depth writes

		// render instances
		for (const auto& instance : instances) {
			tree.renderInstance(instance.modelMatrix, vp);
		}
		assets[0].render(vp);
		bot.render(vp);

		try {
			for (int dx = -2; dx <= 2; ++dx) {
				for (int dz = -2; dz <= 2; ++dz) {
					Plane& plane = pointToPlane.at(std::pair<int, int>(currentChunkX + dx, currentChunkZ + dz));
					plane.render(vp);
					planeToCubes.at(plane)[0].render(vp);
				}
			}
		} catch (const std::out_of_range& e) {
			std::cerr << "Key not found: " << e.what() << std::endl;
		}

		frames++;
		fTime += deltaTime;
		if (fTime > 2.0f) {
			float fps = frames / fTime;
			frames = 0;
			fTime = 0;

			std::stringstream stream;
			stream << std::fixed << std::setprecision(2) << "debug | Frames per second (FPS): " << fps;
			glfwSetWindowTitle(window, stream.str().c_str());
		}

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (!glfwWindowShouldClose(window));

	// Clean up
	skybox.cleanup();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

void onChunkChanged(int currentChunkX, int currentChunkZ) {
	for (int i = -2; i <= 2; i++) {
		for (int j = -2; j <= 2; j++) {

			int chunkX = currentChunkX + i;
			int chunkZ = currentChunkZ + j;

			// if plane isnt in hashmap yet
			if (pointToPlane.find(std::pair<int, int>(chunkX, chunkZ)) == pointToPlane.end()) {
				Plane p(glm::vec3(chunkX*CHUNK_SIZE, 0, chunkZ*CHUNK_SIZE),
					glm::vec3(CHUNK_SIZE, 0.0, CHUNK_SIZE), groundFilePath);
				planes.push_back(p);
				pointToPlane.emplace(std::pair<int, int>(chunkX, chunkZ), p);
				std::vector<int> heights = generateRandomHeights();

				std::vector<std::pair<int,int>> positions = generateRandomPoints();

				std::string texturePath = texture_paths[buildingTexture(gen)];
				Cube cube(p.programID, p.position + glm::vec3(0,heights[0],0),
					glm::vec3(40, heights[0], 40), (texturePath.c_str()));
				planeToCubes[p].push_back(cube);

				lighting light(p.programID, shadowMapWidth, shadowMapHeight);
				glm::vec3 lightPosition = p.position + glm::vec3(-100.0f, 200.0f, -100.0f);
				light.setLightPosition(lightPosition, lightIntensity, 2.0f);
				planeToLight.emplace(p, light);

				glm::mat4 lightView, lightProjection;
				lightProjection = glm::perspective(glm::radians(depthFoV), static_cast<float>(shadowMapWidth) / static_cast<float>(shadowMapHeight), depthNear, depthFar);
				lightView = glm::lookAt(lightPosition, glm::vec3(lightPosition.x, lightPosition.y - 1, lightPosition.z), lightUp);

				glm::mat4 lightSpaceMatrix = lightProjection * lightView;
				if (chunkX == 0 && chunkZ == 0) {
					//for demonstration purposes
					assets[0].programID = p.programID;
					assets[0].cameraMatrixID = glGetUniformLocation(assets[0].programID, "camera");
					assets[0].transformMatrixID = glGetUniformLocation(assets[0].programID, "transform");

					// Get a handle for our Model Matrix uniform
					assets[0].modelMatrixID = glGetUniformLocation(assets[0].programID, "modelMatrix");
					assets[0].textureSamplerID = glGetUniformLocation(assets[0].programID, "textureSampler");
					assets[0].baseColorFactorID = glGetUniformLocation(assets[0].programID, "baseColorFactor");
					assets[0].isLightID = glGetUniformLocation(assets[0].programID, "isLight");
					assets[0].cameraPosID = glGetUniformLocation(assets[0].programID, "cameraPos");
					assets[0].primitiveObjects = assets[0].bindModel(assets[0].model);
					assets[0].textureSamplerID = glGetUniformLocation(assets[0].programID, "textureSampler");
					for (auto prim: assets[0].primitiveObjects) {
						prim.shininessID = glGetUniformLocation(assets[0].programID, "shininess");
					}
					light.shadowPass(lightSpaceMatrix, assets, {planeToCubes.at(p)}, {p});
					glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
					light.prepareLighting();
				}
				else {
					light.shadowPass(lightSpaceMatrix, {}, {planeToCubes.at(p)}, {p});
					glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
					light.prepareLighting();
				}
			}
		}
	}
}

double distance(const std::pair<int, int>& p1, const std::pair<int, int>& p2) {
	return std::sqrt((p1.first - p2.first) * (p1.first - p2.first) +
					 (p1.second - p2.second) * (p1.second - p2.second));
}

std::vector<int> generateRandomHeights() {
	std::vector<int> heights;
	heights.reserve(7);
for (int i = 0; i < 7; i++) {
		heights.push_back(buildingHeight(gen));
	}
	return heights;
}

std::vector<std::pair<int, int>> generateRandomPoints() {
	const int SQUARE_SIZE = 40;
	const int MIN_DISTANCE = 15 + SQUARE_SIZE;

	std::vector<std::pair<int, int>> points;
	std::srand(static_cast<unsigned>(std::time(nullptr)));

	while (points.size() < 7) {
		int x = std::rand() % (CHUNK_SIZE-100 - SQUARE_SIZE) + SQUARE_SIZE / 2;
		int y = std::rand() % (CHUNK_SIZE-100 - SQUARE_SIZE) + SQUARE_SIZE / 2;

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







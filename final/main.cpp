#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <skybox.h>
#include <asset.h>
#include <utility>

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
#include <glm/gtc/matrix_transform.hpp>

#include "lighting.h"


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

// Lighting control
const glm::vec3 wave500(0.0f, 255.0f, 146.0f);
const glm::vec3 wave600(255.0f, 190.0f, 0.0f);
const glm::vec3 wave700(205.0f, 0.0f, 0.0f);
//static glm::vec3 lightIntensity = 5.0f * (8.0f * wave500 + 15.6f * wave600 + 18.4f * wave700);
static glm::vec3 lightIntensity = glm::vec3(0.3,0.3,0.3);
static glm::vec3 lightPosition(-100.0f, 200.0f, -100.0f);
static float exposure = 2.0f;

// Shadow mapping
static glm::vec3 lightUp(0, 0, 1);
static int shadowMapWidth = 2048;
static int shadowMapHeight = 2048;

static float depthFoV = 120.0f;
static float depthNear = 10.0f;
static float depthFar = 4000.0f;


// Camera
static glm::vec3 eye_center(0.0f, 100.0f, 300.0f);
static glm::vec3 lookat(0.0f, 0.0f, 0.0f);
static glm::vec3 up(0.0f, 1.0f, 0.0f);
static float FoV = 45.0f;
static float zNear = 100.0f;
static float zFar = 1500.0f;

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

std::string texture_paths[] = {"../final/assets/building1.jpg","../final/assets/building2.jpg","../final/assets/building3.jpg",
	"../final/assets/building4.jpg","../final/assets/building5.jpg",};

// Animation 
static bool playAnimation = true;
static float playbackSpeed = 2.0f;

void onChunkChanged(int currentChunkX, int currentChunkZ);
void generateChunkStructures(Plane plane, std::vector<Cube>& buildings, std::vector<Asset>& assets);
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

	//MyBot bot;
	//bot.initialize();


	Plane start(glm::vec3(0,0,0), glm::vec3(512, 10, 512), groundFilePath);

	SkyBox skybox;
	skybox.initialize(glm::vec3(0,0,0), glm::vec3(1,1,1));

	//lighting sceneLight(start.programID, shadowMapWidth, shadowMapHeight);
	//sceneLight.setLightPosition(lightPosition, lightIntensity, 2.0f);


	planes.push_back(start);

	//load 3x3 grid of chunks around starting area
	onChunkChanged(0,0);
	for (const auto& pair : pointToPlane) {
		const auto& key = pair.first;
		const Plane& plane = pair.second;

		std::cout << "Key: (" << key.first << ", " << key.second << ") - ";
	}

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

	glm::mat4 lightView, lightProjection;
	lightProjection = glm::perspective(glm::radians(depthFoV), static_cast<float>(shadowMapWidth) / static_cast<float>(shadowMapHeight), depthNear, depthFar);
	lightView = glm::lookAt(lightPosition, glm::vec3(lightPosition.x, lightPosition.y - 1, lightPosition.z), lightUp);

	glm::mat4 lightSpaceMatrix = lightProjection * lightView;
	//sceneLight.shadowPass(lightSpaceMatrix, assets, cubes, planes);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	//sceneLight.prepareLighting();

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
		}

		processInput(window, deltaTime);

		// Rendering
		viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		glm::mat4 vp = projectionMatrix * viewMatrix;

		skybox.position = cameraPos;
		glDepthMask(GL_FALSE); // Disable depth writes
		skybox.render(vp);     // Render the skybox
		glDepthMask(GL_TRUE);  // Re-enable depth writes

		//store planes in hashmap so you can easily render 9 adjacent planes without slow for loop
		//start.render(vp);
        //tree.render(vp);
		//tree2.render(vp);
		//tree3.render(vp);
		//cube.render(vp);

		/*
		// unrolled loop for better locality and performance
		try {
			Plane& p1 = pointToPlane.at(std::pair<int, int>(currentChunkX, currentChunkZ));   // Center
			Plane& p2 = pointToPlane.at(std::pair<int, int>(currentChunkX-1, currentChunkZ));
			Plane& p3 = pointToPlane.at(std::pair<int, int>(currentChunkX + 1, currentChunkZ));      // Right
			Plane& p4 = pointToPlane.at(std::pair<int, int>(currentChunkX, currentChunkZ - 1));      // Front
			Plane& p5 = pointToPlane.at(std::pair<int, int>(currentChunkX - 1, currentChunkZ - 1));  // Front-Left
			Plane& p6 = pointToPlane.at(std::pair<int, int>(currentChunkX + 1, currentChunkZ - 1));  // Front-Right
			Plane& p7 = pointToPlane.at(std::pair<int, int>(currentChunkX, currentChunkZ + 1));      // Back
			Plane& p8 = pointToPlane.at(std::pair<int, int>(currentChunkX - 1, currentChunkZ + 1));  // Back-Left
			Plane& p9 = pointToPlane.at(std::pair<int, int>(currentChunkX + 1, currentChunkZ + 1));  // Back-Right

			p1.render(vp);
			p2.render(vp);
			p3.render(vp);
			p4.render(vp);
			p5.render(vp);
			p6.render(vp);
			p7.render(vp);
			p8.render(vp);
			p9.render(vp);

			//this may kill performance


		} catch (const std::out_of_range& e) {
			//std::cerr << "Key not found: " << e.what() << std::endl;
		}
		*/



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
/*
void onChunkChanged(int currentChunkX, int currentChunkZ, std::vector<glm::mat4>& planes,
	std::vector<glm::mat4>& assets, std::vector<glm::mat4>& buildings, lighting& lights) {
	planes.clear();
	buildings.clear();
	assets.clear();
	lights.pruneLights(currentChunkX, currentChunkZ);
	//todo add scenelight
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			int chunkX = currentChunkX + i;
			int chunkZ = currentChunkZ + j;
			Plane p(glm::vec3(chunkX*CHUNK_SIZE, 0, chunkZ*CHUNK_SIZE),
		glm::vec3(CHUNK_SIZE, 0.0, CHUNK_SIZE), groundFilePath);

			// If chunk has not already been loaded (ie if plane not already in hashmap)


		}
	}
}
*/
/*
void generateChunkStructures(Plane plane, std::vector<Cube>& buildings, std::vector<Asset>& assets) {
	int numBuildings = numBuildingsRand(gen);
	std::vector<std::pair<int, int>> points = generateRandomPoints();
	for (int i = 0; i < 5; i++) {
		std::string texturePath = texture_paths[buildingTexture(gen)];
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
*/
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







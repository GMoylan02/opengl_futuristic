#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <skybox.h>

// GLTF model loader
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#include <render/shader.h>

#include <vector>
#include <iostream>
#define _USE_MATH_DEFINES
#include <iomanip>
#include <math.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

static GLFWwindow *window;
static int windowWidth = 1024;
static int windowHeight = 768;


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

//tmp
// View control
static float viewAzimuth = 0.f;
static float viewPolar = 0.f;
static float viewDistance = 300.0f;

// Lighting  
static glm::vec3 lightIntensity(5e6f, 5e6f, 5e6f);
static glm::vec3 lightPosition(-275.0f, 500.0f, 800.0f);


// Animation 
static bool playAnimation = true;
static float playbackSpeed = 2.0f;

struct MyBot {
	// Shader variable IDs
	GLuint mvpMatrixID;
	GLuint jointMatricesID;
	GLuint lightPositionID;
	GLuint lightIntensityID;
	GLuint programID;
	GLuint jointIndicesID;
	GLuint jointWeightsID;

	tinygltf::Model model;

	// Each VAO corresponds to each mesh primitive in the GLTF model
	struct PrimitiveObject {
		GLuint vao;
		std::map<int, GLuint> vbos;
	};
	std::vector<PrimitiveObject> primitiveObjects;

	// Skinning 
	struct SkinObject {
		// Transforms the geometry into the space of the respective joint
		std::vector<glm::mat4> inverseBindMatrices;  

		// Transforms the geometry following the movement of the joints
		std::vector<glm::mat4> globalJointTransforms;

		// Combined transforms
		std::vector<glm::mat4> jointMatrices;
	};
	std::vector<SkinObject> skinObjects;

	// Animation 
	struct SamplerObject {
		std::vector<float> input;
		std::vector<glm::vec4> output;
		int interpolation;
	};
	struct ChannelObject {
		int sampler;
		std::string targetPath;
		int targetNode;
	}; 
	struct AnimationObject {
		std::vector<SamplerObject> samplers;	// Animation data
	};
	std::vector<AnimationObject> animationObjects;
	//std::vector<glm::mat4> globalTransforms;

	glm::mat4 getNodeTransform(const tinygltf::Node& node) {
		glm::mat4 transform(1.0f); 

		if (node.matrix.size() == 16) {
			transform = glm::make_mat4(node.matrix.data());
		} else {
			if (node.translation.size() == 3) {
				transform = glm::translate(transform, glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
			}
			if (node.rotation.size() == 4) {
				glm::quat q(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
				transform *= glm::mat4_cast(q);
			}
			if (node.scale.size() == 3) {
				transform = glm::scale(transform, glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
			}
		}
		return transform;
	}

	void computeLocalNodeTransform(const tinygltf::Model& model, 
		int nodeIndex, 
		std::vector<glm::mat4> &localTransforms)
	{
		if (nodeIndex >= model.nodes.size()) {
			return;
		}
		localTransforms.push_back(getNodeTransform(model.nodes[nodeIndex]));
		computeLocalNodeTransform(model, nodeIndex+1, localTransforms);
	}

	void computeGlobalNodeTransform(const tinygltf::Model& model, 
		const std::vector<glm::mat4> &localTransforms,
		int nodeIndex, const glm::mat4& parentTransform, 
		std::vector<glm::mat4> &globalTransforms)
	{
		glm::mat4 currentTransform = parentTransform * localTransforms[nodeIndex];
		globalTransforms[nodeIndex] = currentTransform;

		const tinygltf::Node& currentNode = model.nodes[nodeIndex];
		for (int childIndex : currentNode.children) {
			computeGlobalNodeTransform(model, localTransforms, childIndex, currentTransform, globalTransforms);
		}
	}

	std::vector<SkinObject> prepareSkinning(const tinygltf::Model &model) {
		std::vector<SkinObject> skinObjects;

		// In our Blender exporter, the default number of joints that may influence a vertex is set to 4, just for convenient implementation in shaders.

		for (size_t i = 0; i < model.skins.size(); i++) {
			SkinObject skinObject;

			const tinygltf::Skin &skin = model.skins[i];

			// Read inverseBindMatrices
			const tinygltf::Accessor &accessor = model.accessors[skin.inverseBindMatrices];
			assert(accessor.type == TINYGLTF_TYPE_MAT4);
			const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
			const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
			const float *ptr = reinterpret_cast<const float *>(
            	buffer.data.data() + accessor.byteOffset + bufferView.byteOffset);
			
			skinObject.inverseBindMatrices.resize(accessor.count);
			for (size_t j = 0; j < accessor.count; j++) {
				float m[16];
				memcpy(m, ptr + j * 16, 16 * sizeof(float));
				skinObject.inverseBindMatrices[j] = glm::make_mat4(m);
			}

			assert(skin.joints.size() == accessor.count);
			skinObject.globalJointTransforms.resize(skin.joints.size(), glm::mat4(1.0f));
			skinObject.jointMatrices.resize(skin.joints.size());

			int rootNodeIndex = skin.joints[0];
			std::vector<glm::mat4> localNodeTransforms(skin.joints.size());
			computeLocalNodeTransform(model, rootNodeIndex, localNodeTransforms);

			glm::mat4 parentTransform(1.0f);
			std::vector<glm::mat4> globalNodeTransforms(skin.joints.size());
			computeGlobalNodeTransform(model, localNodeTransforms, rootNodeIndex, parentTransform, globalNodeTransforms);

			for (size_t j = 0; j < skin.joints.size(); j++) {
				int jointNodeIndex = skin.joints[j];
				skinObject.globalJointTransforms[j] = globalNodeTransforms[jointNodeIndex];
			}

			for (size_t j = 0; j < skin.joints.size(); j++) {
				skinObject.jointMatrices[j] = skinObject.globalJointTransforms[j] * skinObject.inverseBindMatrices[j];
			}
			skinObjects.push_back(skinObject);
		}

		// Compute local transforms at each node
		return skinObjects;
	}

	int findKeyframeIndex(const std::vector<float>& times, float animationTime) 
	{
		int left = 0;
		int right = times.size() - 1;

		while (left <= right) {
			int mid = (left + right) / 2;

			if (mid + 1 < times.size() && times[mid] <= animationTime && animationTime < times[mid + 1]) {
				return mid;
			}
			else if (times[mid] > animationTime) {
				right = mid - 1;
			}
			else { // animationTime >= times[mid + 1]
				left = mid + 1;
			}
		}

		// Target not found
		return times.size() - 2;
	}

	std::vector<AnimationObject> prepareAnimation(const tinygltf::Model &model) 
	{
		std::vector<AnimationObject> animationObjects;
		for (const auto &anim : model.animations) {
			AnimationObject animationObject;
			
			for (const auto &sampler : anim.samplers) {
				SamplerObject samplerObject;

				const tinygltf::Accessor &inputAccessor = model.accessors[sampler.input];
				const tinygltf::BufferView &inputBufferView = model.bufferViews[inputAccessor.bufferView];
				const tinygltf::Buffer &inputBuffer = model.buffers[inputBufferView.buffer];

				assert(inputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
				assert(inputAccessor.type == TINYGLTF_TYPE_SCALAR);

				// Input (time) values
				samplerObject.input.resize(inputAccessor.count);

				const unsigned char *inputPtr = &inputBuffer.data[inputBufferView.byteOffset + inputAccessor.byteOffset];
				const float *inputBuf = reinterpret_cast<const float*>(inputPtr);

				// Read input (time) values
				int stride = inputAccessor.ByteStride(inputBufferView);
				for (size_t i = 0; i < inputAccessor.count; ++i) {
					samplerObject.input[i] = *reinterpret_cast<const float*>(inputPtr + i * stride);
				}
				
				const tinygltf::Accessor &outputAccessor = model.accessors[sampler.output];
				const tinygltf::BufferView &outputBufferView = model.bufferViews[outputAccessor.bufferView];
				const tinygltf::Buffer &outputBuffer = model.buffers[outputBufferView.buffer];

				assert(outputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

				const unsigned char *outputPtr = &outputBuffer.data[outputBufferView.byteOffset + outputAccessor.byteOffset];
				const float *outputBuf = reinterpret_cast<const float*>(outputPtr);

				int outputStride = outputAccessor.ByteStride(outputBufferView);
				
				// Output values
				samplerObject.output.resize(outputAccessor.count);
				
				for (size_t i = 0; i < outputAccessor.count; ++i) {

					if (outputAccessor.type == TINYGLTF_TYPE_VEC3) {
						memcpy(&samplerObject.output[i], outputPtr + i * 3 * sizeof(float), 3 * sizeof(float));
					} else if (outputAccessor.type == TINYGLTF_TYPE_VEC4) {
						memcpy(&samplerObject.output[i], outputPtr + i * 4 * sizeof(float), 4 * sizeof(float));
					} else {
						std::cout << "Unsupport accessor type ..." << std::endl;
					}

				}

				animationObject.samplers.push_back(samplerObject);			
			}

			animationObjects.push_back(animationObject);
		}
		return animationObjects;
	}

		void updateAnimation(
		const tinygltf::Model &model,
		const tinygltf::Animation &anim,
		const AnimationObject &animationObject,
		float time,
		std::vector<glm::mat4> &nodeTransforms)
	{
		// There are many channels so we have to accumulate the transforms
		for (const auto &channel : anim.channels) {

			int targetNodeIndex = channel.target_node;
			const auto &sampler = anim.samplers[channel.sampler];

			// Access output (value) data for the channel
			const tinygltf::Accessor &outputAccessor = model.accessors[sampler.output];
			const tinygltf::BufferView &outputBufferView = model.bufferViews[outputAccessor.bufferView];
			const tinygltf::Buffer &outputBuffer = model.buffers[outputBufferView.buffer];

			// Calculate current animation time (wrap if necessary)
			const std::vector<float> &times = animationObject.samplers[channel.sampler].input;
			float animationTime = fmod(time, times.back());

			int keyframeIndex = findKeyframeIndex(times, animationTime);

			const unsigned char *outputPtr = &outputBuffer.data[outputBufferView.byteOffset + outputAccessor.byteOffset];
			const float *outputBuf = reinterpret_cast<const float*>(outputPtr);
			int nextKeyframeIndex = (keyframeIndex + 1) % times.size();
			float t0 = times[keyframeIndex];
			float t1 = times[nextKeyframeIndex];
			float factor = (animationTime - t0) / (t1 - t0); // Linear interpolation factor

			if (channel.target_path == "translation") {
				glm::vec3 translation0, translation1;
				memcpy(&translation0, outputPtr + keyframeIndex * 3 * sizeof(float), 3 * sizeof(float));
				memcpy(&translation1, outputBuf + nextKeyframeIndex * 3, 3 * sizeof(float));

				//glm::vec3 translation = translation0;
				glm::vec3 translation = glm::mix(translation0, translation1, factor);
				nodeTransforms[targetNodeIndex] = glm::translate(nodeTransforms[targetNodeIndex], translation);
			} else if (channel.target_path == "rotation") {
				glm::quat rotation0, rotation1;
				memcpy(&rotation0, outputBuf + keyframeIndex * 4, 4 * sizeof(float));
				memcpy(&rotation1, outputBuf + nextKeyframeIndex * 4, 4 * sizeof(float));

				glm::quat rotation = glm::slerp(rotation0, rotation1, factor);
				nodeTransforms[targetNodeIndex] *= glm::mat4_cast(rotation);
			} else if (channel.target_path == "scale") {
				glm::vec3 scale0, scale1;
				memcpy(&scale0, outputBuf + keyframeIndex * 3, 3 * sizeof(float));
				memcpy(&scale1, outputBuf + nextKeyframeIndex * 3, 3 * sizeof(float));

				glm::vec3 scale = glm::mix(scale0, scale1, factor);
				nodeTransforms[targetNodeIndex] = glm::scale(nodeTransforms[targetNodeIndex], scale);
			}
		}
	}

	void updateSkinning(const std::vector<glm::mat4> &nodeTransforms) {
		// -------------------------------------------------
		// TODO: Recompute joint matrices
		// -------------------------------------------------
		for (int j = 0; j < skinObjects.size(); ++j) {
			SkinObject& skinObject = skinObjects[j];
			const tinygltf::Skin &skin = model.skins[j];
			//recompute joint matrices
			for (size_t i = 0; i < skinObject.globalJointTransforms.size(); ++i) {
				int jointNodeIndex = skin.joints[i];
				skinObject.jointMatrices[i] = nodeTransforms[jointNodeIndex] * skinObject.inverseBindMatrices[i];
			}
		}
	}

	void update(float time) {

		// return;

		if (model.animations.size() > 0) {
			const tinygltf::Animation &animation = model.animations[0];
			const AnimationObject &animationObject = animationObjects[0];
			const tinygltf::Skin &skin = model.skins[0];
			std::vector<glm::mat4> nodeTransforms(skin.joints.size());
			for (size_t i = 0; i < nodeTransforms.size(); ++i) {
				nodeTransforms[i] = glm::mat4(1.0);
			}
			updateAnimation(model, animation, animationObject, time, nodeTransforms);

			// TODO
			// Recompute global transforms
			int rootNodeIndex = model.skins[0].joints[0];
			std::vector<glm::mat4> globalNodeTransforms(model.nodes.size(), glm::mat4(1.0f));
			computeGlobalNodeTransform(model, nodeTransforms, rootNodeIndex, glm::mat4(1.0f), globalNodeTransforms);

			updateSkinning(globalNodeTransforms); // Update joint matrices
		}
	}

	bool loadModel(tinygltf::Model &model, const char *filename) {
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;

		bool res = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
		if (!warn.empty()) {
			std::cout << "WARN: " << warn << std::endl;
		}

		if (!err.empty()) {
			std::cout << "ERR: " << err << std::endl;
		}

		if (!res)
			std::cout << "Failed to load glTF: " << filename << std::endl;
		else
			std::cout << "Loaded glTF: " << filename << std::endl;

		return res;
	}

	void initialize() {
		// Modify your path if needed
		if (!loadModel(model, "../lab4/assets/tree_small_02_1k.gltf")) {
			return;
		}

		// Prepare buffers for rendering 
		primitiveObjects = bindModel(model);

		// Prepare joint matrices
		skinObjects = prepareSkinning(model);

		// Prepare animation data 
		animationObjects = prepareAnimation(model);

		// Create and compile our GLSL program from the shaders
		programID = LoadShadersFromFile("../lab4/shader/bot.vert", "../lab4/shader/bot.frag");
		if (programID == 0)
		{
			std::cerr << "Failed to load shaders." << std::endl;
		}

		// Get a handle for GLSL variables
		mvpMatrixID = glGetUniformLocation(programID, "MVP");
		jointMatricesID = glGetUniformLocation(programID, "jointMatrices");
		lightPositionID = glGetUniformLocation(programID, "lightPosition");
		lightIntensityID = glGetUniformLocation(programID, "lightIntensity");
	}

	void bindMesh(std::vector<PrimitiveObject> &primitiveObjects,
				tinygltf::Model &model, tinygltf::Mesh &mesh) {

		std::map<int, GLuint> vbos;
		for (size_t i = 0; i < model.bufferViews.size(); ++i) {
			const tinygltf::BufferView &bufferView = model.bufferViews[i];

			int target = bufferView.target;
			
			if (bufferView.target == 0) { 
				// The bufferView with target == 0 in our model refers to 
				// the skinning weights, for 25 joints, each 4x4 matrix (16 floats), totaling to 400 floats or 1600 bytes. 
				// So it is considered safe to skip the warning.
				//std::cout << "WARN: bufferView.target is zero" << std::endl;
				continue;
			}

			const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
			GLuint vbo;
			glGenBuffers(1, &vbo);
			glBindBuffer(target, vbo);
			glBufferData(target, bufferView.byteLength,
						&buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
			
			vbos[i] = vbo;
		}

		// Each mesh can contain several primitives (or parts), each we need to 
		// bind to an OpenGL vertex array object
		for (size_t i = 0; i < mesh.primitives.size(); ++i) {

			tinygltf::Primitive primitive = mesh.primitives[i];
			tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

			GLuint vao;
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);

			for (auto &attrib : primitive.attributes) {
				tinygltf::Accessor accessor = model.accessors[attrib.second];
				int byteStride =
					accessor.ByteStride(model.bufferViews[accessor.bufferView]);
				glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);

				int size = 1;
				if (accessor.type != TINYGLTF_TYPE_SCALAR) {
					size = accessor.type;
				}

				int vaa = -1;
				if (attrib.first.compare("POSITION") == 0) vaa = 0;
				if (attrib.first.compare("NORMAL") == 0) vaa = 1;
				if (attrib.first.compare("TEXCOORD_0") == 0) vaa = 2;
				if (attrib.first.compare("JOINTS_0") == 0) vaa = 3;
				if (attrib.first.compare("WEIGHTS_0") == 0) vaa = 4;
				if (vaa > -1) {
					glEnableVertexAttribArray(vaa);
					glVertexAttribPointer(vaa, size, accessor.componentType,
										accessor.normalized ? GL_TRUE : GL_FALSE,
										byteStride, BUFFER_OFFSET(accessor.byteOffset));
				} else {
					std::cout << "vaa missing: " << attrib.first << std::endl;
				}
			}

			// Record VAO for later use
			PrimitiveObject primitiveObject;
			primitiveObject.vao = vao;
			primitiveObject.vbos = vbos;
			primitiveObjects.push_back(primitiveObject);

			glBindVertexArray(0);
		}
	}

	void bindModelNodes(std::vector<PrimitiveObject> &primitiveObjects, 
						tinygltf::Model &model,
						tinygltf::Node &node) {
		// Bind buffers for the current mesh at the node
		if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
			bindMesh(primitiveObjects, model, model.meshes[node.mesh]);
		}

		// Recursive into children nodes
		for (size_t i = 0; i < node.children.size(); i++) {
			assert((node.children[i] >= 0) && (node.children[i] < model.nodes.size()));
			bindModelNodes(primitiveObjects, model, model.nodes[node.children[i]]);
		}
	}

	std::vector<PrimitiveObject> bindModel(tinygltf::Model &model) {
		std::vector<PrimitiveObject> primitiveObjects;

		const tinygltf::Scene &scene = model.scenes[model.defaultScene];
		for (size_t i = 0; i < scene.nodes.size(); ++i) {
			assert((scene.nodes[i] >= 0) && (scene.nodes[i] < model.nodes.size()));
			bindModelNodes(primitiveObjects, model, model.nodes[scene.nodes[i]]);
		}

		return primitiveObjects;
	}

	void drawMesh(const std::vector<PrimitiveObject> &primitiveObjects,
				tinygltf::Model &model, tinygltf::Mesh &mesh) {
		
		for (size_t i = 0; i < mesh.primitives.size(); ++i) 
		{
			GLuint vao = primitiveObjects[i].vao;
			std::map<int, GLuint> vbos = primitiveObjects[i].vbos;

			glBindVertexArray(vao);

			tinygltf::Primitive primitive = mesh.primitives[i];
			tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos.at(indexAccessor.bufferView));

			glDrawElements(primitive.mode, indexAccessor.count,
						indexAccessor.componentType,
						BUFFER_OFFSET(indexAccessor.byteOffset));

			glBindVertexArray(0);
		}
	}

	void drawModelNodes(const std::vector<PrimitiveObject>& primitiveObjects,
						tinygltf::Model &model, tinygltf::Node &node) {
		// Draw the mesh at the node, and recursively do so for children nodes
		if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
			drawMesh(primitiveObjects, model, model.meshes[node.mesh]);
		}
		for (size_t i = 0; i < node.children.size(); i++) {
			drawModelNodes(primitiveObjects, model, model.nodes[node.children[i]]);
		}
	}
	void drawModel(const std::vector<PrimitiveObject>& primitiveObjects,
				tinygltf::Model &model) {
		// Draw all nodes
		const tinygltf::Scene &scene = model.scenes[model.defaultScene];
		for (size_t i = 0; i < scene.nodes.size(); ++i) {
			drawModelNodes(primitiveObjects, model, model.nodes[scene.nodes[i]]);
		}
	}

	void render(glm::mat4 cameraMatrix) {
		glUseProgram(programID);
		// Set camera
		glm::mat4 mvp = cameraMatrix;
		glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

		SkinObject skinObject = skinObjects[0];
		//printJointMatrices(skinObject.jointMatrices);

		glUniformMatrix4fv(jointMatricesID, skinObject.jointMatrices.size(), GL_FALSE, &skinObject.jointMatrices[0][0][0]);
		// Set light data 
		glUniform3fv(lightPositionID, 1, &lightPosition[0]);
		glUniform3fv(lightIntensityID, 1, &lightIntensity[0]);

		// Draw the GLTF model
		drawModel(primitiveObjects, model);
	}

	void printJointMatrices(const std::vector<glm::mat4>& jointMatrices) {
		for (size_t i = 0; i < jointMatrices.size(); ++i) {
			std::cout << "Matrix " << i << ":" << std::endl;
			for (int row = 0; row < 4; ++row) {
				for (int col = 0; col < 4; ++col) {
					std::cout << jointMatrices[i][row][col] << " ";
				}
				std::cout << std::endl; // End of a matrix row
			}
			std::cout << std::endl; // Separation between matrices
		}
	}

	void cleanup() {
		glDeleteProgram(programID);
	}
};

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


void processInput(GLFWwindow* window, float deltaTime) {
	float velocity = cameraSpeed * deltaTime;
	//std::cout << glm::to_string(cameraPos) << std::endl;
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
	window = glfwCreateWindow(windowWidth, windowHeight, "Lab 4", NULL, NULL);
	if (window == NULL)
	{
		std::cerr << "Failed to open a GLFW window." << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Load OpenGL functions, gladLoadGL returns the loaded version, 0 on error.
	int version = gladLoadGL(glfwGetProcAddress);
	if (version == 0)
	{
		std::cerr << "Failed to initialize OpenGL context." << std::endl;
		return -1;
	}

	// Background
	glClearColor(0.2f, 0.2f, 0.25f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Skybox setu


	// Our 3D character
	//MyBot bot;
	//bot.initialize();

	SkyBox skybox;
	skybox.initialize(glm::vec3(0,0,0), glm::vec3(200,200,200));

	//tmp
	eye_center.y = viewDistance * cos(viewPolar);
	eye_center.x = viewDistance * cos(viewAzimuth);
	eye_center.z = viewDistance * sin(viewAzimuth);

	glm::mat4 viewMatrix, projectionMatrix;
	glm::float32 FoV = 45;
	glm::float32 zNear = 0.1f;
	glm::float32 zFar = 1000.0f;
	projectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, zNear, zFar);

	// Camera setup
    //glm::mat4 viewMatrix, projectionMatrix;
	//projectionMatrix = glm::perspective(glm::radians(FoV), (float)windowWidth / windowHeight, zNear, zFar);

	// Time and frame rate tracking
	static double lastTime = glfwGetTime();
	float time = 0.0f;			// Animation time
	float fTime = 0.0f;			// Time for measuring fps
	unsigned long frames = 0;


	// Main loop
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

		if (playAnimation) {
			time += deltaTime * playbackSpeed;
			//bot.update(time);
		}

		processInput(window, deltaTime);


		//std::cout << "Camera Position: " << glm::to_string(cameraPos) << std::endl;

		// Rendering
		viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		glm::mat4 vp = projectionMatrix * viewMatrix;
		//bot.render(vp);
		glDisable(GL_CULL_FACE); // Temporarily disable face culling
		glDepthFunc(GL_LEQUAL); // Change depth function to pass when values are <= current depth
		skybox.render(vp);
		glEnable(GL_CULL_FACE);  // Re-enable face culling
		glDepthFunc(GL_LESS); // Reset depth function back to default
		//skybox.render(vp);	//render skybox

		// FPS tracking
		// Count number of frames over a few seconds and take average


		frames++;
		fTime += deltaTime;
		if (fTime > 2.0f) {
			float fps = frames / fTime;
			frames = 0;
			fTime = 0;

			std::stringstream stream;
			stream << std::fixed << std::setprecision(2) << "Lab 4 | Frames per second (FPS): " << fps;
			glfwSetWindowTitle(window, stream.str().c_str());
		}

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (!glfwWindowShouldClose(window));

	// Clean up
	//bot.cleanup();
	skybox.cleanup();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}








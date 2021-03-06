#define _USE_MATH_DEFINES

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../Headers/stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "../Headers/logging.h"
#include "../Headers/mstack.h"
#include "../Headers/shader.h"
#include "../Headers/camera.h"
#include "../Headers/light.h"
#include "../Headers/fog.h"
#include "../Headers/cylinder.h"
#include "../Headers/boid.h"

#include <vector>
#include <iostream>
#include <string>
#include <cmath>
#include <ctime>
#include <random>

void shaderSetting(Shader shader);
void showUI();
void setViewMatrix();
void setProjectionMatrix();
void setViewport();
void geneObejectData();
void geneSphereData();
void drawFloor();
void drawCube();
void drawPlane(Shader shader, glm::vec3 position, float size_w, float size_h, int method);
void drawFish(Shader shader, glm::vec3 position, float size);
void drawGrass(Shader shader, glm::vec3 position, float size);
void drawBox(Shader shader);
void drawAxis(Shader shader);
void updateROVFront();
void drawSphere();
void drawCone();
void setFullScreen();
void frameBufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void scrollCallback(GLFWwindow* window, double xpos, double ypos);
void errorCallback(int error, const char* description);
unsigned int loadTexture(char const* path);
unsigned int loadCubemap(std::vector<std::string> faces);
glm::mat4 GetPerspectiveProjMatrix(float fovy, float ascept, float znear, float zfar);
glm::mat4 GetOrthoProjMatrix(float left, float right, float bottom, float top, float near, float far);

// ========== Global Variable ==========

// Window parameters
GLFWwindow* window;
bool isfullscreen = false;
const std::string WINDOW_TITLE = "Boids Flocking";
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;
std::vector <int> window_position{ 0, 0 };
std::vector <int> window_size{ 0, 0 };

// Matrix stack paramters
StackArray modelMatrix;

// Transform Matrices
glm::mat4 model = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);

// Time parameters
float deltaTime = 0.0f;
float lastTime = 0.0f;

// Camera parameter
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = (float)SCR_WIDTH / 2.0f;
float lastY = (float)SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool moveCameraDirection = false;
bool showAxis = false;

// Projection parameters
static bool isPerspective = true;
float aspect_wh = (float)SCR_WIDTH / (float)SCR_HEIGHT;
float aspect_hw = (float)SCR_HEIGHT / (float)SCR_WIDTH;
static float global_left = 0.0f;
static float global_right = 0.0f;
static float global_bottom = 0.0f;
static float global_top = 0.0f;
static float global_near = 0.1f;
static float global_far = 250.0f;

// Light Parameters
Light dirLight(glm::vec4(-0.2f, -1.0f, -0.3f, 0.0f), true);
std::vector<Light> pointLights = {
	Light(glm::vec3(10.0f, 10.0f, 35.0f), true),
	Light(glm::vec3(-45.0f, 5.0f, 30.0f), true),
	Light(glm::vec3(38.0f, 2.0f, -40.0f), true),
	Light(glm::vec3(-50.0f, 15.0f, -45.0f), true),
};
std::vector<Light> spotLights = {
	Light(camera.Position, camera.Front, true),
};
static bool useBlinnPhong = true;
static bool useSpotExponent = false;
static bool useLighting = true;
static bool useDiffuseTexture = true;
static bool useSpecularTexture = true;
static bool useEmission = true;
static bool useGamma = true;
// GammaValue = 1.0f / 2.2f;
static float GammaValue = 0.868;

// Foggy Setting
Fog fog(glm::vec4(0.266f, 0.5f, 0.609f, 1.0f), true, global_near, global_far);

// Object Data
std::vector<float> cubeVertices;
std::vector<int> cubeIndices;
unsigned int cubeVAO, cubeVBO, cubeEBO;

std::vector<float> floorVertices;
std::vector<unsigned int> floorIndices;
unsigned int floorVAO, floorVBO, floorEBO;

std::vector<float> planeVertices;
unsigned int planeVAO, planeVBO;

std::vector<float> sphereVertices;
std::vector<unsigned int> sphereIndices;
unsigned int sphereVAO, sphereVBO, sphereEBO;

Cylinder cone(0.0f, 0.2f, 0.8f, 40, 20);
unsigned int coneVAO, coneVBO, coneEBO;

static bool enableBillboard = true;

// Texture parameter
unsigned int seaTexture, sandTexture, grassTexture, boxTexture, boxSpecularTexture, fishTexture, skyTexture;

std::vector<glm::vec3> boxposition, plasticposition, grassposition, fishposition;
std::vector<float> grassSize, fishSize;

// Boids Flocking
std::vector<Boid> boids;
static float separation = 1.0f, alignment = 1.0f, cohesion = 1.0f;

int main() {

	// Initialize GLFW
	if (!glfwInit()) {
		logging::loggingMessage(logging::LogType::ERROR, "Failed to initialize GLFW.");
		glfwTerminate();
		return -1;
	} else {
		logging::loggingMessage(logging::LogType::DEBUG, "Initialize GLFW successful.");
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 32);

	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, WINDOW_TITLE.c_str(), NULL, NULL);
	if (!window) {
		logging::loggingMessage(logging::LogType::ERROR, "Failed to create GLFW window.");
		glfwTerminate();
		return -1;
	} else {
		logging::loggingMessage(logging::LogType::DEBUG, "Create GLFW window successful.");
	}

	// Register callbacks
	glfwMakeContextCurrent(window);
	glfwSetErrorCallback(errorCallback);
	glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetScrollCallback(window, scrollCallback);

	// Initialize GLAD (Must behind the create window)
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		logging::loggingMessage(logging::LogType::ERROR, "Failed to initialize GLAD.");
		glfwTerminate();
		return -1;
	} else {
		logging::loggingMessage(logging::LogType::DEBUG, "Initialize GLAD successful.");
	}

	// Initialize ImGui and bind to GLFW and OpenGL3(glad)
	std::string glsl_version = "#version 330";
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version.c_str());
	ImGui::StyleColorsDark();

	// Show version info
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	logging::showInitInfo(renderer, version);

	// Setting OpenGL
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Create shader program
	Shader myShader("Shaders/lighting.vs", "Shaders/lighting.fs");
	Shader instanceShader("Shaders/instance.vs", "Shaders/lighting.fs");
	Shader normalShader("Shaders/normal_visualization.vs", "Shaders/normal_visualization.fs", "Shaders/normal_visualization.gs");
	
	// Create object data
	geneObejectData();

	// Setting amount of fishes, boxed and grass. 
	std::mt19937_64 rand_generator;
	std::uniform_real_distribution<float> unif_g(-80.0, 80.0);
	std::uniform_real_distribution<float> unif_gsize(0.2, 2.0);
	std::uniform_real_distribution<float> unif_b(-30.0, 30.0);

	std::uniform_real_distribution<float> unif_boid_position(-1, 1);
	std::uniform_real_distribution<float> unif_boid_direction(-0.01, 0.01);
	
	for (int i = 0; i < 20; i++) {
		boxposition.push_back(glm::vec3(unif_b(rand_generator), 0.0f, unif_b(rand_generator)));
	}

	for (int i = 0; i < 10; i++) {
		plasticposition.push_back(glm::vec3(unif_b(rand_generator), 0.0f, unif_b(rand_generator)));
	}

	for (int i = 0; i < 600; i++) {
		grassposition.push_back(glm::vec3(unif_g(rand_generator), 0.0f, unif_g(rand_generator)));
		grassSize.push_back(unif_gsize(rand_generator));
	}

	constexpr float radius_max = 10.0f;
	float x, y, z, rotate_angle;
	glm::vec3 boid_position;
	glm::vec3 boid_direction;
	for (int i = 0; i < 50; i++) {
		// fishposition.push_back(glm::vec3(unif_f(generator), 0.0f, unif_f(generator)));
		// fishSize.push_back(unif_fsize(generator));
		
		glm::mat4 model(1.0f);

		do {
			x = unif_boid_position(rand_generator) * radius_max;
			y = unif_boid_position(rand_generator) * radius_max;
			z = unif_boid_position(rand_generator) * radius_max;
		} while (x * x + y * y + z * z > radius_max);
		boid_position = glm::vec3(x, y, z);

		boid_direction = glm::vec3(unif_boid_direction(rand_generator), unif_boid_direction(rand_generator), unif_boid_direction(rand_generator));

		model = glm::translate(model, boid_position);

		Boid temp_boid(boid_position, boid_direction);
		temp_boid.setModel(model);
		boids.push_back(temp_boid);
	}

	// Initial Light Setting
	spotLights[0].Cutoff = 25.0f;
	spotLights[0].OuterCutoff = 40.0f;

	// Loading textures
	seaTexture = loadTexture("Resources\\Textures\\sea.jpg");
	sandTexture = loadTexture("Resources\\Textures\\sand.jpg");
	grassTexture = loadTexture("Resources\\Textures\\grass.png");
	boxTexture = loadTexture("Resources\\Textures\\container2.png");
	boxSpecularTexture = loadTexture("Resources\\Textures\\container2_specular.png");
	fishTexture = loadTexture("Resources\\Textures\\fish.png");
	skyTexture = loadTexture("Resources\\Textures\\sky.jpg");

	// Loading Cubemap
	std::vector<std::string> faces{
		"Resources/Textures/skybox/right.jpg",
		"Resources/Textures/skybox/left.jpg",
		"Resources/Textures/skybox/top.jpg",
		"Resources/Textures/skybox/bottom.jpg",
		"Resources/Textures/skybox/front.jpg",
		"Resources/Textures/skybox/back.jpg",
	};
	unsigned int cubemapTexture = loadCubemap(faces);

	// The main loop
	while (!glfwWindowShouldClose(window)) {
		
		// Calculate the deltaFrame
		float currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		// Process Input (Moving camera)
		processInput(window);

		// Clear the buffer
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// feed inputs to dear imgui start new frame;
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		showUI();
		// ImGui::ShowDemoWindow();

		setViewMatrix();
		setProjectionMatrix();
		setViewport();

		// Enable Shader and setting view & projection matrix
		shaderSetting(myShader);

		// Render on the screen;

		// ==================== Draw origin and 3 axes ====================
		if (showAxis) {
			drawAxis(myShader);
		}

		// ==================== Draw Skybox (Using Cubemap) ====================
		glDepthFunc(GL_LEQUAL);
		myShader.setBool("isCubeMap", true);
		modelMatrix.push();
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(5.0f)));
		myShader.setMat4("model", modelMatrix.top());
		drawCube();
		modelMatrix.pop();
		myShader.setBool("isCubeMap", false);
		glDepthFunc(GL_LESS);

		/*
		// ==================== Draw Sea ====================
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, seaTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, NULL);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, NULL);
		myShader.setBool("material.enableColorTexture", true);
		myShader.setBool("material.enableSpecularTexture", true);
		myShader.setBool("material.enableEmission", false);
		myShader.setBool("material.enableEmissionTexture", false);
		myShader.setFloat("material.shininess", 64.0f);
		myShader.setMat4("model", modelMatrix.top());
		drawFloor();
		*/

		// ==================== Draw Boids ====================
		/*
		modelMatrix.push();
		for (unsigned int i = 0; i < boids.size(); i++) {
			boids[i].edges(20, 20, 20);
			boids[i].flock(boids, separation, alignment, cohesion);
			boids[i].update(deltaTime);
			myShader.setMat4("model", modelMatrix.top());
			drawFish(myShader, boids[i].Position, boids[i].Size);
		}
		modelMatrix.pop();
		*/

		std::vector<glm::mat4> boidsMatrices;
		for (unsigned int i = 0; i < boids.size(); i++) {
			
			boids[i].flock(boids, separation, alignment, cohesion);

			//boids[i].ApplyForce(boids[i].Cohesion(boids, cohesion));
			//boids[i].ApplyForce(boids[i].Alignment(boids, alignment));
			//boids[i].ApplyForce(boids[i].Separation(boids, separation));
			// boids[i].ApplyForce(boids[i].Edges());

			boids[i].Update(deltaTime);
			boids[i].ResetForce();

			boidsMatrices.push_back(boids[i].getModel());
			// instanceShader.setMat4("model", boids[i].getModel());
		}

		unsigned int buffer;
		GLsizei vec4Size = sizeof(glm::vec4);
		glGenBuffers(1, &buffer);		
		glBindVertexArray(coneVAO);
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
			glBufferData(GL_ARRAY_BUFFER, boidsMatrices.size() * sizeof(glm::mat4), boidsMatrices.data(), GL_STATIC_DRAW);
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
			glEnableVertexAttribArray(4);
			glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size));
			glEnableVertexAttribArray(5);
			glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
			glEnableVertexAttribArray(6);
			glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));
			glVertexAttribDivisor(3, 1);
			glVertexAttribDivisor(4, 1);
			glVertexAttribDivisor(5, 1);
			glVertexAttribDivisor(6, 1);
		glBindVertexArray(0);

		shaderSetting(instanceShader);
		instanceShader.use();
		instanceShader.setBool("material.enableColorTexture", false);
		instanceShader.setBool("material.enableSpecularTexture", false);
		instanceShader.setBool("material.enableEmission", false);
		instanceShader.setBool("material.enableEmissionTexture", false);
		instanceShader.setVec4("material.ambient", glm::vec4(0.02f, 0.02f, 0.02f, 1.0));
		instanceShader.setVec4("material.diffuse", glm::vec4(0.60f, 0.20f, 0.0f, 1.0));
		instanceShader.setVec4("material.specular", glm::vec4(0.40f, 0.10f, 0.0f, 1.0));
		instanceShader.setFloat("material.shininess", 16.0f);
		instanceShader.setMat4("model", modelMatrix.top());
		drawCone();

		/*
		normalShader.use();
		normalShader.setMat4("view", view);
		normalShader.setMat4("projection", projection);
		modelMatrix.push();
		normalShader.setMat4("model", modelMatrix.top());
		drawCone();
		modelMatrix.pop();
		*/

		/*
		// ==================== Draw obstacles ====================
		modelMatrix.push();
		for (unsigned int i = 0; i < boxposition.size(); i++) {
			modelMatrix.push();
			modelMatrix.save(glm::translate(modelMatrix.top(), glm::vec3(boxposition[i].x, sin(currentTime * 3 + boxposition[i].z) / 4, boxposition[i].z)));
			myShader.setMat4("model", modelMatrix.top());
			drawBox(myShader);
			modelMatrix.pop();
		}
		modelMatrix.pop();
		*/

		/*
		// ==================== Draw Plastic Object ====================
		myShader.use();
		myShader.setBool("material.enableColorTexture", false);
		myShader.setBool("material.enableSpecularTexture", false);
		myShader.setBool("material.enableEmission", false);
		myShader.setBool("material.enableEmissionTexture", false);
		myShader.setVec4("material.ambient", glm::vec4(0.02f, 0.02f, 0.02f, 1.0));
		myShader.setVec4("material.diffuse", glm::vec4(0.1f, 0.35f, 0.1f, 1.0));
		myShader.setVec4("material.specular", glm::vec4(0.45f, 0.55f, 0.45f, 1.0));
		myShader.setFloat("material.shininess", 16.0f);
		modelMatrix.push();
		for (unsigned int i = 0; i < plasticposition.size(); i++) {
			modelMatrix.push();
			modelMatrix.save(glm::translate(modelMatrix.top(), glm::vec3(plasticposition[i].x, sin(currentTime * 3 + plasticposition[i].z) / 4, plasticposition[i].z)));
			myShader.setMat4("model", modelMatrix.top());
			drawCube();
			modelMatrix.pop();
		}
		modelMatrix.pop();
		*/

		

		// ==================== draw light ball ====================
		myShader.use();
		myShader.setBool("material.enableColorTexture", false);
		myShader.setBool("material.enableSpecularTexture", false);
		myShader.setBool("material.enableEmission", true);
		myShader.setBool("material.enableEmissionTexture", false);
		for (unsigned int i = 0; i < pointLights.size(); i++) {
			if (!pointLights[i].Enable) {
				continue;
			}
			modelMatrix.push();
			modelMatrix.save(glm::translate(modelMatrix.top(), pointLights[i].Position));
			modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(0.5f)));
			myShader.setVec4("material.ambient", glm::vec4(pointLights[i].Ambient.x, pointLights[i].Ambient.y, pointLights[i].Ambient.z, 1.0f));
			myShader.setVec4("material.diffuse", glm::vec4(pointLights[i].Diffuse.x, pointLights[i].Diffuse.y, pointLights[i].Diffuse.z, 1.0f));
			myShader.setVec4("material.specular", glm::vec4(pointLights[i].Specular.x, pointLights[i].Specular.y, pointLights[i].Specular.z, 1.0f));
			myShader.setFloat("material.shininess", 32.0f);
			myShader.setMat4("model", modelMatrix.top());
			drawSphere();
			modelMatrix.pop();
		}
		myShader.setBool("material.enableEmission", false);

		// render on the screen
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Swap Buffers and Trigger event
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &cubeEBO);
	
	glDeleteVertexArrays(1, &floorVAO);
	glDeleteBuffers(1, &floorVBO);
	glDeleteBuffers(1, &floorEBO);

	glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &planeVBO);

	glDeleteVertexArrays(1, &sphereVAO);
	glDeleteBuffers(1, &sphereVBO);
	glDeleteBuffers(1, &sphereEBO);

	glDeleteVertexArrays(1, &coneVAO);
	glDeleteBuffers(1, &coneVBO);
	glDeleteBuffers(1, &coneEBO);

	// Release the resources.
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	return 0;
}

void shaderSetting(Shader shader) {
	shader.use();
	
	// Transform matrices setting
	shader.setMat4("view", view);
	shader.setMat4("projection", projection);

	// Cubemap setting
	shader.setInt("skybox", 3);
	shader.setBool("isCubeMap", false);

	// Global parameters setting
	shader.setVec3("viewPos", camera.Position);
	shader.setBool("useBlinnPhong", useBlinnPhong);
	shader.setBool("useSpotExponent", useSpotExponent);
	shader.setBool("useLighting", useLighting);
	shader.setBool("useDiffuseTexture", useDiffuseTexture);
	shader.setBool("useSpecularTexture", useSpecularTexture);
	shader.setBool("useEmission", useEmission);
	shader.setBool("useGamma", useGamma);
	shader.setFloat("GammaValue", GammaValue);
	
	// Material setting
	shader.setInt("material.diffuse_texture", 0);
	shader.setInt("material.specular_texture", 1);
	shader.setInt("material.emission_texture", 2);

	shader.setVec4("material.ambient", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	shader.setVec4("material.diffuse", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	shader.setVec4("material.specular", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	shader.setFloat("material.shininess", 64.0f);

	// Lightning setting (Directional Light)
	shader.setVec3("lights[0].direction", dirLight.Direction);
	shader.setVec3("lights[0].ambient", dirLight.Ambient);
	shader.setVec3("lights[0].diffuse", dirLight.Diffuse);
	shader.setVec3("lights[0].specular", dirLight.Specular);
	shader.setBool("lights[0].enable", dirLight.Enable);
	shader.setInt("lights[0].caster", dirLight.Caster);

	// Lightning setting (Point Light)
	for (unsigned int i = 0; i < pointLights.size(); i++) {
		shader.setVec3("lights[" + std::to_string(i + 1) + "].position", pointLights[i].Position);
		shader.setVec3("lights[" + std::to_string(i + 1) + "].ambient", pointLights[i].Ambient);
		shader.setVec3("lights[" + std::to_string(i + 1) + "].diffuse", pointLights[i].Diffuse);
		shader.setVec3("lights[" + std::to_string(i + 1) + "].specular", pointLights[i].Specular);
		shader.setFloat("lights[" + std::to_string(i + 1) + "].constant", pointLights[i].Constant);
		shader.setFloat("lights[" + std::to_string(i + 1) + "].linear", pointLights[i].Linear);
		shader.setFloat("lights[" + std::to_string(i + 1) + "].quadratic", pointLights[i].Quadratic);
		shader.setFloat("lights[" + std::to_string(i + 1) + "].enable", pointLights[i].Enable);
		shader.setInt("lights[" + std::to_string(i + 1) + "].caster", pointLights[i].Caster);
	}

	// Lightning setting (Spotlight)
	spotLights[0].Position = camera.Position;
	spotLights[0].Direction = camera.Front;
	for (unsigned int i = 0; i < spotLights.size(); i++) {
		shader.setVec3("lights[" + std::to_string(i + 5) + "].position", spotLights[i].Position);
		shader.setVec3("lights[" + std::to_string(i + 5) + "].direction", spotLights[i].Direction);
		shader.setVec3("lights[" + std::to_string(i + 5) + "].ambient", spotLights[i].Ambient);
		shader.setVec3("lights[" + std::to_string(i + 5) + "].diffuse", spotLights[i].Diffuse);
		shader.setVec3("lights[" + std::to_string(i + 5) + "].specular", spotLights[i].Specular);
		shader.setFloat("lights[" + std::to_string(i + 5) + "].constant", spotLights[i].Constant);
		shader.setFloat("lights[" + std::to_string(i + 5) + "].linear", spotLights[i].Linear);
		shader.setFloat("lights[" + std::to_string(i + 5) + "].quadratic", spotLights[i].Quadratic);
		shader.setFloat("lights[" + std::to_string(i + 5) + "].cutoff", glm::cos(glm::radians(spotLights[i].Cutoff)));
		shader.setFloat("lights[" + std::to_string(i + 5) + "].outerCutoff", glm::cos(glm::radians(spotLights[i].OuterCutoff)));
		shader.setFloat("lights[" + std::to_string(i + 5) + "].exponent", spotLights[i].Exponent);
		shader.setBool("lights[" + std::to_string(i + 5) + "].enable", spotLights[i].Enable);
		shader.setInt("lights[" + std::to_string(i + 5) + "].caster", spotLights[i].Caster);
	}

	// Fog setting
	fog.Density = 0.003f;
	shader.setVec4("fog.color", fog.Color);
	shader.setFloat("fog.density", fog.Density);
	shader.setInt("fog.mode", fog.Mode);
	shader.setInt("fog.depthType", fog.DepthType);
	shader.setBool("fog.enable", fog.Enable);
	shader.setFloat("fog.f_start", fog.F_start);
	shader.setFloat("fog.f_end", fog.F_end);
}

void showUI() {
	ImGui::Begin("Control Panel");
	ImGuiTabBarFlags tab_bar_flags = ImGuiBackendFlags_None;
	if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags)) {

		if (ImGui::BeginTabItem("Camera")) {
			ImGui::TextColored(ImVec4(1.0f, 0.5f, 1.0f, 1.0f), "Ghost Camera");
			ImGui::Text("Position = (%.2f, %.2f, %.2f)", camera.Position.x, camera.Position.y, camera.Position.z);
			ImGui::Text("Front = (%.2f, %.2f, %.2f)", camera.Front.x, camera.Front.y, camera.Front.z);
			ImGui::Text("Right = (%.2f, %.2f, %.2f)", camera.Right.x, camera.Right.y, camera.Right.z);
			ImGui::Text("Up = (%.2f, %.2f, %.2f)", camera.Up.x, camera.Up.y, camera.Up.z);
			ImGui::Text("Pitch = %.2f deg, Yaw = %.2f deg", camera.Pitch, camera.Yaw);
			ImGui::EndTabItem();
		}
		
		if (ImGui::BeginTabItem("Projection")) {
			ImGui::TextColored(ImVec4(1.0f, 0.5f, 1.0f, 1.0f), (isPerspective) ? "Perspective Projection" : "Orthogonal Projection");
			ImGui::Text("Parameters");
			ImGui::BulletText("FoV = %.2f deg, Aspect = %.2f", camera.Zoom, aspect_wh);
			ImGui::BulletText("left: %.2f, right: %.2f ", global_left, global_right);
			ImGui::BulletText("bottom: %.2f, top: %.2f ", global_bottom, global_top);
			ImGui::SliderFloat("Near", &global_near, 0.1, 10);
			ImGui::SliderFloat("Far", &global_far, 10, 250);
			ImGui::Spacing();

			if (ImGui::TreeNode("Projection Matrix")) {
				setProjectionMatrix();
				glm::mat4 proj = projection;
				ImGui::Columns(4, "mycolumns");
				ImGui::Separator();
				for (int i = 0; i < 4; i++) {
					ImGui::Text("%.2f", proj[0][i]); ImGui::NextColumn();
					ImGui::Text("%.2f", proj[1][i]); ImGui::NextColumn();
					ImGui::Text("%.2f", proj[2][i]); ImGui::NextColumn();
					ImGui::Text("%.2f", proj[3][i]); ImGui::NextColumn();
					ImGui::Separator();
				}
				ImGui::Columns(1);
				ImGui::TreePop();
			}
			ImGui::Spacing();
			ImGui::EndTabItem();
		}
		
		if (ImGui::BeginTabItem("Illustration")) {
			ImGui::Text("Projection Mode: %s", isPerspective ? "Perspective" : "Orthogonal");
			ImGui::Text("Showing Axes: %s", showAxis ? "True" : "false");
			ImGui::Text("Full Screen:  %s", isfullscreen ? "True" : "false");
			ImGui::Spacing();

			if (ImGui::TreeNode("General")) {
				ImGui::BulletText("Press X to show / hide the axes");
				ImGui::BulletText("Press Y to switch the projection");
				ImGui::BulletText("Press F11 to Full Screen");
				ImGui::BulletText("Press ESC to close the program");
				ImGui::TreePop();
			}
			ImGui::Spacing();
			
			if (ImGui::TreeNode("Camera Illustration")) {
				ImGui::BulletText("WSAD to move camera");
				ImGui::BulletText("Hold mouse right button to rotate");
				ImGui::BulletText("Press Left Shift to speed up");
				ImGui::TreePop();
			}
			ImGui::Spacing();

			if (ImGui::TreeNode("Illumination Illustration")) {
				ImGui::BulletText("F to turn off/on the spot light");
				ImGui::BulletText("L to switch lighting model");
				ImGui::TreePop();
			}
			ImGui::Spacing();

			ImGui::EndTabItem();
		}
		
		if (ImGui::BeginTabItem("Illumination")) {
			ImGui::Text("Lighting Model: %s", useBlinnPhong ? "Blinn-Phong" : "Phong");
			ImGui::Checkbox("use Exponent", &useSpotExponent);
			ImGui::Checkbox("Lighting", &useLighting);
			ImGui::Checkbox("DiffuseTexture", &useDiffuseTexture);
			ImGui::Checkbox("SpecularTexture", &useSpecularTexture);
			ImGui::Checkbox("Emission", &useEmission);
			ImGui::Checkbox("Gamma Correction", &useGamma);
			ImGui::SliderFloat("Gamma Value", &GammaValue, 1.0f / 2.2f, 2.2f);
			ImGui::Spacing();
			
			if (ImGui::TreeNode("Direction Light")) {
				ImGui::SliderFloat3("Direction", (float*)&dirLight.Direction, -10.0f, 10.0f);
				ImGui::SliderFloat3("Ambient", (float*)&dirLight.Ambient, 0.0f, 1.0f);
				ImGui::SliderFloat3("Diffuse", (float*)&dirLight.Diffuse, 0.0f, 1.0f);
				ImGui::SliderFloat3("Specular", (float*)&dirLight.Specular, 0.0f, 1.0f);
				ImGui::Checkbox("Enable", &dirLight.Enable);
				ImGui::TreePop();
			}
			ImGui::Spacing();

			for (unsigned int i = 0; i < pointLights.size(); i++) {
				std::stringstream ss;
				ss << i;
				std::string index = ss.str();

				if (ImGui::TreeNode(std::string("Point Light " + index).c_str())) {
					ImGui::SliderFloat3(std::string("Position").c_str(), glm::value_ptr(pointLights[i].Position), -50.0f, 50.0f);
					ImGui::SliderFloat3(std::string("Ambient").c_str(), (float*)&pointLights[i].Ambient, 0.0f, 1.0f);
					ImGui::SliderFloat3(std::string("Diffuse").c_str(), (float*)&pointLights[i].Diffuse, 0.0f, 1.0f);
					ImGui::SliderFloat3(std::string("Specular").c_str(), (float*)&pointLights[i].Specular, 0.0f, 1.0f);
					ImGui::SliderFloat(std::string("Linear").c_str(), (float*)&pointLights[i].Linear, 0.00014f, 0.7f);
					ImGui::SliderFloat(std::string("Quadratic").c_str(), (float*)&pointLights[i].Quadratic, 0.00007, 0.5f);
					ImGui::Checkbox(std::string("Enable").c_str(), &pointLights[i].Enable);
					ImGui::Spacing();
					ImGui::TreePop();
				}
				ImGui::Spacing();
			}

			for (unsigned int i = 0; i < spotLights.size(); i++) {
				std::stringstream ss;
				ss << i;
				std::string index = ss.str();

				if (ImGui::TreeNode(std::string("Spot Light " + index).c_str())) {
					ImGui::Text(std::string("Position: (%.2f, %.2f, %.2f)").c_str(), spotLights[i].Position.x, spotLights[i].Position.y, spotLights[i].Position.z);
					ImGui::Text(std::string("Direction: (%.2f, %.2f, %.2f)").c_str(), spotLights[i].Direction.x, spotLights[i].Direction.y, spotLights[i].Direction.z);
					ImGui::SliderFloat3(std::string("Ambient").c_str(), (float*)&spotLights[i].Ambient, 0.0f, 1.0f);
					ImGui::SliderFloat3(std::string("Diffuse").c_str(), (float*)&spotLights[i].Diffuse, 0.0f, 1.0f);
					ImGui::SliderFloat3(std::string("Specular").c_str(), (float*)&spotLights[i].Specular, 0.0f, 1.0f);
					ImGui::SliderFloat(std::string("Linear").c_str(), (float*)&spotLights[i].Linear, 0.00014f, 0.7f);
					ImGui::SliderFloat(std::string("Quadratic").c_str(), (float*)&spotLights[i].Quadratic, 0.00007, 0.5f);
					ImGui::SliderFloat(std::string("Cutoff").c_str(), (float*)&spotLights[i].Cutoff, 0.0f, spotLights[i].OuterCutoff - 1);
					ImGui::SliderFloat(std::string("OuterCutoff").c_str(), (float*)&spotLights[i].OuterCutoff, spotLights[i].Cutoff + 1, 40.0f);
					ImGui::SliderFloat(std::string("Exponent").c_str(), (float*)&spotLights[i].Exponent, 0.0f, 256.0f);
					ImGui::Checkbox(std::string("Enable").c_str(), &spotLights[i].Enable);
					ImGui::Spacing();
					ImGui::TreePop();
				}
				ImGui::Spacing();
			}
			ImGui::EndTabItem();
		}
		
		if (ImGui::BeginTabItem("Texture")) {
			ImGui::Checkbox("Billboard", &enableBillboard);
			ImGui::SliderFloat("Separation", &separation, 0, 10);
			ImGui::SliderFloat("Alignment", &alignment, 0, 10);
			ImGui::SliderFloat("Cohesion", &cohesion, 0, 10);
			ImGui::Spacing();

			ImGui::EndTabItem();
		}
		
		if (ImGui::BeginTabItem("Fog")) {
			ImGui::SliderFloat4(std::string("Color").c_str(), static_cast<float*>(&fog.Color.x), 0.0f, 1.0f);
			ImGui::SliderFloat(std::string("Density").c_str(), (float*)&fog.Density, 0.0f, 1.0f);

			const char* items_a[] = { "LINEAR", "EXP", "EXP2" };
			const char* items_b[] = { "PLANE_BASED", "RANGE_BASED" };
			ImGui::Combo("Mode", (int*)&fog.Mode, items_a, IM_ARRAYSIZE(items_a));
			ImGui::Combo("Depth Type", (int*)&fog.DepthType, items_b, IM_ARRAYSIZE(items_b));

			if (fog.Mode == 0) {
				ImGui::SliderFloat(std::string("F_Start").c_str(), &fog.F_start, global_near, fog.F_end);
				ImGui::SliderFloat(std::string("F_End").c_str(), &fog.F_end, fog.F_start, global_far);
			}
			
			ImGui::Checkbox(std::string("Enable").c_str(), &fog.Enable);
			ImGui::Spacing();

			ImGui::EndTabItem();
		}
		
		ImGui::EndTabBar();
	}
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::End();
}

void setViewMatrix() {
	view = camera.GetViewMatrix();
}

void setProjectionMatrix() {
	aspect_wh = (float)SCR_WIDTH / (float)SCR_HEIGHT;
	aspect_hw = (float)SCR_HEIGHT / (float)SCR_WIDTH;
	if (isPerspective) {
		projection = GetPerspectiveProjMatrix(glm::radians(camera.Zoom), aspect_wh, global_near, global_far);
	} else {
		float length = tan(glm::radians(camera.Zoom / 2)) * global_near * 50;
		if (SCR_WIDTH > SCR_HEIGHT) {
			projection = GetOrthoProjMatrix(-length, length, -length * aspect_hw, length * aspect_hw, global_near, global_far);
		} else {
			projection = GetOrthoProjMatrix(-length * aspect_wh, length * aspect_wh, -length, length, global_near, global_far);
		}
	}
}

void setViewport() {
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
}

void geneObejectData() {
	// ========== Generate Cube vertex data ==========
	cubeVertices = {
		// Positions			// Normals 			// Texture coords
		 0.5f,  0.5f,  0.5f,	0.0f, 0.0f, 1.0f,	1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,	0.0f, 0.0f, 1.0f,	1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,	0.0f, 0.0f, 1.0f,	0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,	0.0f, 0.0f, 1.0f,	0.0f, 1.0f,

		0.5f,  0.5f,  0.5f,		1.0f, 0.0f, 0.0f,	1.0f, 1.0f,
		0.5f,  0.5f, -0.5f,		1.0f, 0.0f, 0.0f,	1.0f, 0.0f,
		0.5f, -0.5f, -0.5f,		1.0f, 0.0f, 0.0f,	0.0f, 0.0f,
		0.5f, -0.5f,  0.5f,		1.0f, 0.0f, 0.0f,	0.0f, 1.0f,

		-0.5f,  0.5f,  0.5f,	-1.0f, 0.0f, 0.0f,	1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,	-1.0f, 0.0f, 0.0f,	1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,	-1.0f, 0.0f, 0.0f,	0.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,	-1.0f, 0.0f, 0.0f,	0.0f, 1.0f,

		 0.5f,  0.5f,  0.5f,	0.0f, 1.0f, 0.0f,	1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,	0.0f, 1.0f, 0.0f,	1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,	0.0f, 1.0f, 0.0f,	0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,	0.0f, 1.0f, 0.0f,	0.0f, 1.0f,

		 0.5f, -0.5f,  0.5f,	0.0f, -1.0f, 0.0f,	1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,	0.0f, -1.0f, 0.0f,	1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,	0.0f, -1.0f, 0.0f,	0.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,	0.0f, -1.0f, 0.0f,	0.0f, 1.0f,

		 0.5f,  0.5f, -0.5f,	0.0f, 0.0f, -1.0f,	1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,	0.0f, 0.0f, -1.0f,	1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,	0.0f, 0.0f, -1.0f,	0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,	0.0f, 0.0f, -1.0f,	0.0f, 1.0f,
	};
	cubeIndices = {
		0, 1, 3,
		1, 2, 3,

		4, 5, 7,
		5, 6, 7,

		8, 9, 11,
		9, 10, 11,

		12, 13, 15,
		13, 14, 15,

		16, 17, 19,
		17, 18, 19,

		20, 21, 23,
		21, 22, 23,
	};
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glGenBuffers(1, &cubeEBO);
	glBindVertexArray(cubeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeVertices.size() * sizeof(float), cubeVertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeIndices.size() * sizeof(unsigned int), cubeIndices.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);
	// ==================================================


	// ========== Generate floor vertex data ==========
	floorVertices = {
		// Positions			// Normals			// Texture Coords
		-100.0,  0.0,  100.0,	0.0f, 1.0f, 0.0f,	25.0f,  0.0f,
		 100.0,  0.0,  100.0,	0.0f, 1.0f, 0.0f,	25.0f, 25.0f,
		 100.0,  0.0, -100.0,	0.0f, 1.0f, 0.0f,	 0.0f, 25.0f,
		-100.0,  0.0, -100.0,	0.0f, 1.0f, 0.0f,	 0.0f,  0.0f,
	};
	floorIndices = {
		0, 1, 2,
		0, 2, 3,
	};
	glGenVertexArrays(1, &floorVAO);
	glGenBuffers(1, &floorVBO);
	glGenBuffers(1, &floorEBO);
	glBindVertexArray(floorVAO);
		glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
		glBufferData(GL_ARRAY_BUFFER, floorVertices.size() * sizeof(float), floorVertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, floorIndices.size() * sizeof(unsigned int), floorIndices.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);
	// ==================================================


	// ========== Generate grass vertex data ==========
	planeVertices = {
		// Positions		// Normals			// Texture coords
		 0.0,  1.0, 0.0,	0.0, 0.0, 1.0,		0.0, 0.0,
		 0.0,  0.0, 0.0,	0.0, 0.0, 1.0,		0.0, 1.0,
		 1.0,  0.0, 0.0,	0.0, 0.0, 1.0,		1.0, 1.0,

		 0.0,  1.0, 0.0,	0.0, 0.0, 1.0,		0.0, 0.0,
		 1.0,  0.0, 0.0,	0.0, 0.0, 1.0,		1.0, 1.0,
		 1.0,  1.0, 0.0,	0.0, 0.0, 1.0,		1.0, 0.0,
	};
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
		glBufferData(GL_ARRAY_BUFFER, planeVertices.size() * sizeof(float), planeVertices.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);
	// ==================================================
	
	// ========== Generate sphere vertex data ==========
	geneSphereData();
	// ==================================================


	// ========== Generate cylinder vertex data ==========
	glGenVertexArrays(1, &coneVAO);
	glGenBuffers(1, &coneVBO);
	glGenBuffers(1, &coneEBO);
	glBindVertexArray(coneVAO);
		glBindBuffer(GL_ARRAY_BUFFER, coneVBO);
		glBufferData(GL_ARRAY_BUFFER, cone.getVertexSize(), cone.getVertices(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, coneEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, cone.getIndexSize(), cone.getIndices(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);
	// ==================================================
}

void geneSphereData() {
	float radius = 1.0f;
	unsigned int latitude = 30;
	unsigned int longitude = 30;

	for (int i = 0; i <= latitude; i++) {
		float theta = M_PI * i / latitude;
		float sinTheta = sin(theta);
		float cosTheta = cos(theta);
		for (int j = 0; j <= longitude; j++) {
			float phi = 2.0f * M_PI * j / longitude;
			float sinPhi = sin(phi);
			float cosPhi = cos(phi);
			
			float x = cosPhi * sinTheta;
			float y = cosTheta;
			float z = sinPhi * sinTheta;

			sphereVertices.push_back(radius * x);
			sphereVertices.push_back(radius * y);
			sphereVertices.push_back(radius * z);

			// Generate normal vectors
			glm::vec3 normal = glm::vec3(2 * radius * x, 2 * radius * y, 2 * radius * z);
			normal = glm::normalize(normal);
			sphereVertices.push_back(normal.x);
			sphereVertices.push_back(normal.y);
			sphereVertices.push_back(normal.z);

			// Generate texture coordinate
			float u = 1 - (j / longitude);
			float v = 1 - (i / latitude);
			sphereVertices.push_back(u);
			sphereVertices.push_back(-v);
		}
	}

	for (int i = 0; i < latitude; i++) {
		for (int j = 0; j < longitude; j++) {
			int first = (i * (longitude + 1)) + j;
			int second = first + longitude + 1;

			sphereIndices.push_back(first);
			sphereIndices.push_back(second);
			sphereIndices.push_back(first + 1);

			sphereIndices.push_back(second);
			sphereIndices.push_back(second + 1);
			sphereIndices.push_back(first + 1);
		}
	}

	glGenVertexArrays(1, &sphereVAO);
	glGenBuffers(1, &sphereVBO);
	glGenBuffers(1, &sphereEBO);
	glBindVertexArray(sphereVAO);
		glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
		glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);
}

void drawFloor() {
	modelMatrix.push();
	glBindVertexArray(floorVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	modelMatrix.pop();
}

void drawCube() {
	modelMatrix.push();
	glBindVertexArray(cubeVAO);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	modelMatrix.pop();
}

void drawPlane(Shader shader, glm::vec3 position, float size_w, float size_h, int method) {
	glm::mat4 view_model = view * modelMatrix.top();
	
	glm::vec3 billboard_x = glm::vec3(0.0f);
	glm::vec3 billboard_y = glm::vec3(0.0f);
	glm::vec3 billboard_z = glm::vec3(0.0f);
	
	if (enableBillboard) {
		billboard_z = glm::vec3(view_model[0][2], view_model[1][2], view_model[2][2]);
		if (method == 0) {
			// 方法1：Y軸固定
			billboard_x = glm::vec3(billboard_z.z, 0.0f, -billboard_z.x);
			billboard_y = glm::vec3(0.0f, 1.0f, 0.0f);
		} else {
			// 方法2：Y軸不固定
			billboard_x = glm::vec3(view_model[0][0], view_model[1][0], view_model[2][0]);
			billboard_y = glm::vec3(view_model[0][1], view_model[1][1], view_model[2][1]);
		}
	} else {
		billboard_z = glm::vec3(0.0f, 0.0f, -1.0f);
		billboard_x = glm::vec3(billboard_z.z, 0.0f, -billboard_z.x);
		billboard_y = glm::vec3(0.0f, 1.0f, 0.0f);
	}
	
	// 計算頂點
	glm::vec3 v0 = position - (size_w * billboard_x / 2.0f);
	glm::vec3 v1 = position + (size_w * billboard_x / 2.0f);
	glm::vec3 v2 = v1 + (size_h * billboard_y);
	glm::vec3 v3 = v0 + (size_h * billboard_y);

	// 這邊材質座標的y軸記得要上下顛倒
	planeVertices = {
		// Positions		// Normals			// Texture coords
		 v0.x, v0.y, v0.z,	0.0, 0.0, 1.0,		0.0, 1.0,
		 v3.x, v3.y, v3.z,	0.0, 0.0, 1.0,		0.0, 0.0,
		 v2.x, v2.y, v2.z,	0.0, 0.0, 1.0,		1.0, 0.0,

		 v0.x, v0.y, v0.z,	0.0, 0.0, 1.0,		0.0, 1.0,
		 v2.x, v2.y, v2.z,	0.0, 0.0, 1.0,		1.0, 0.0,
		 v1.x, v1.y, v1.z,	0.0, 0.0, 1.0,		1.0, 1.0,
	};

	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, planeVertices.size() * sizeof(float), planeVertices.data(), GL_STATIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void drawFish(Shader shader, glm::vec3 position, float size) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fishTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, NULL);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, NULL);
	shader.setBool("material.enableColorTexture", true);
	shader.setBool("material.enableSpecularTexture", false);
	shader.setBool("material.enableEmission", false);
	shader.setBool("material.enableEmissionTexture", false);
	shader.setFloat("material.shininess", 16.0f);
	drawPlane(shader, position, size, size * 0.5, 1);
}

void drawGrass(Shader shader, glm::vec3 position, float size) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, grassTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, NULL);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, NULL);
	shader.setBool("material.enableColorTexture", true);
	shader.setBool("material.enableSpecularTexture", false);
	shader.setBool("material.enableEmission", false);
	shader.setBool("material.enableEmissionTexture", false);
	shader.setFloat("material.shininess", 16.0f);
	drawPlane(shader, position, size, size, 0);
}

void drawBox(Shader shader) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, boxTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, boxSpecularTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, NULL);
	shader.setBool("material.enableColorTexture", true);
	shader.setBool("material.enableSpecularTexture", true);
	shader.setBool("material.enableEmission", false);
	shader.setBool("material.enableEmissionTexture", false);
	shader.setFloat("material.shininess", 64.0f);
	drawCube();
}

void drawAxis(Shader shader) {

	shader.setBool("material.enableColorTexture", false);
	shader.setBool("material.enableSpecularTexture", false);
	shader.setBool("material.enableEmission", true);
	shader.setBool("material.enableEmissionTexture", false);

	// 繪製世界坐標系原點（0, 0, 0）
	modelMatrix.push();
		modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(0.2f, 0.2f, 0.2f)));
		shader.setVec4("material.ambient", glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
		shader.setVec4("material.diffuse", glm::vec4(0.2f, 0.2f, 0.2f, 1.0f));
		shader.setVec4("material.specular", glm::vec4(0.4f, 0.4f, 0.4f, 1.0f));
		shader.setFloat("material.shininess", 64.0f);
		shader.setMat4("model", modelMatrix.top());
		drawSphere();
	modelMatrix.pop();

	// 繪製三個軸
	modelMatrix.push();
		modelMatrix.push();
			modelMatrix.save(glm::translate(modelMatrix.top(), glm::vec3(1.5f, 0.0f, 0.0f)));
			modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(3.0f, 0.1f, 0.1f)));
			shader.setVec4("material.ambient", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
			shader.setVec4("material.diffuse", glm::vec4(1.0f, 0.0f, 0.0f, 1.0));
			shader.setVec4("material.specular", glm::vec4(1.0f, 0.0f, 0.0f, 1.0));
			shader.setFloat("material.shininess", 64.0f);
			shader.setMat4("model", modelMatrix.top());
			drawCube();
		modelMatrix.pop();


		modelMatrix.push();
			modelMatrix.save(glm::translate(modelMatrix.top(), glm::vec3(0.0f, 1.5f, 0.0f)));
			modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(0.1f, 3.0f, 0.1f)));
			shader.setVec4("material.ambient", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
			shader.setVec4("material.diffuse", glm::vec4(0.0f, 1.0f, 0.0f, 1.0));
			shader.setVec4("material.specular", glm::vec4(0.0f, 1.0f, 0.0f, 1.0));
			shader.setFloat("material.shininess", 64.0f);
			shader.setMat4("model", modelMatrix.top());
			drawCube();
		modelMatrix.pop();

		modelMatrix.push();
			modelMatrix.save(glm::translate(modelMatrix.top(), glm::vec3(0.0f, 0.0f, 1.5f)));
			modelMatrix.save(glm::scale(modelMatrix.top(), glm::vec3(0.1f, 0.1f, 3.0f)));
			shader.setVec4("material.ambient", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
			shader.setVec4("material.diffuse", glm::vec4(0.0f, 0.0f, 1.0f, 1.0));
			shader.setVec4("material.specular", glm::vec4(0.0f, 0.0f, 1.0f, 1.0));
			shader.setFloat("material.shininess", 64.0f);
			shader.setMat4("model", modelMatrix.top());
			drawCube();
		modelMatrix.pop();
	modelMatrix.pop();

	shader.setBool("material.enableEmission", false);
}

void drawSphere() {
	glBindVertexArray(sphereVAO);
	glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void drawCone() {
	glBindVertexArray(coneVAO);
	glDrawElementsInstanced(GL_TRIANGLES, cone.getIndexCount(), GL_UNSIGNED_INT, 0, boids.size());
	glBindVertexArray(0);
}

void setFullScreen() {
	// Create Window
	if (isfullscreen) {
		glfwGetWindowPos(window, &window_position[0], &window_position[1]);
		glfwGetWindowSize(window, &window_size[0], &window_size[1]);
		GLFWmonitor* myMonitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(myMonitor);
		glfwSetWindowMonitor(window, myMonitor, 0, 0, mode->width, mode->height, 0);
	} else {
		glfwSetWindowMonitor(window, nullptr, window_position[0], window_position[1], window_size[0], window_size[1], 0);
	}
}

// Handle window size change.
void frameBufferSizeCallback(GLFWwindow* window, int width, int height) {

	// Set new width and height
	SCR_WIDTH = width;
	SCR_HEIGHT = height;

	// Reset projection matrix and viewport
	setProjectionMatrix();
	setViewport();
}

// Handle the input which in the main loop
void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		camera.MovementSpeed = 25.0f;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE) {
		camera.MovementSpeed = 10.0f;
	}
}

// Handle the key callback
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	// Only handle press events
	if (action == GLFW_RELEASE) {
		return;
	}

	// Exit the program
	if (key == GLFW_KEY_ESCAPE) {
		glfwSetWindowShouldClose(window, true);
	}

	// Full screen switch 
	if (key == GLFW_KEY_F11) {
		if (isfullscreen) {
			isfullscreen = false;
			setFullScreen();
			logging::loggingMessage(logging::LogType::INFO, "Fullscreen: off.");
		} else {
			isfullscreen = true;
			setFullScreen();
			logging::loggingMessage(logging::LogType::INFO, "Fullscreen: on.");
		}
	}

	if (key == GLFW_KEY_X) {
		if (showAxis) {
			showAxis = false;
			logging::loggingMessage(logging::LogType::INFO, "Hidding Axis.");
		} else {
			showAxis = true;
			logging::loggingMessage(logging::LogType::INFO, "Showing Axis.");
		}
	}
	
	if (key == GLFW_KEY_Y) {
		if (isPerspective) {
			isPerspective = false;
			logging::loggingMessage(logging::LogType::INFO, "Using Orthogonal Projection");
		} else {
			isPerspective = true;
			logging::loggingMessage(logging::LogType::INFO, "Using Perspective Projection");
		}
	}

	// 手電筒開關
	if (key == GLFW_KEY_F) {
		if (spotLights[0].Enable) {
			spotLights[0].Enable = false;
			logging::loggingMessage(logging::LogType::INFO, "Spot Light is turn off.");
		} else {
			spotLights[0].Enable = true;
			logging::loggingMessage(logging::LogType::INFO, "Spot Light is turn on.");
		}
	}

	// Blinn-Phong 與 Phong 光照模型的切換
	if (key == GLFW_KEY_L) {
		if (useBlinnPhong) {
			useBlinnPhong = false;
		} else {
			useBlinnPhong = true;
		}
	}
}

// Handle mouse movement (cursor's position)
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {

	// In the first time u create a window, ur cursor may not in the middle of the window.
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	// Allow u to move the direction of camera
	if (moveCameraDirection) {
		camera.ProcessMouseMovement(xoffset, yoffset);
	}
}

// Handle mouse button (like: left middle right)
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		moveCameraDirection = true;
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		moveCameraDirection = false;
	}
}

// Handle mouse scroll
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(yoffset);
}

// Handle GLFW Error Callback
void errorCallback(int error, const char* description) {
	logging::loggingMessage(logging::LogType::ERROR, description);
}

// Loading Texture
unsigned int loadTexture(char const* path) {
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data) {
		GLenum format;
		if (nrComponents == 1) {
			format = GL_RED;
		}
		else if (nrComponents == 3) {
			format = GL_RGB;
		}
		else if (nrComponents == 4) {
			format = GL_RGBA;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else {
		std::cout << "Failed to load texture at path:" << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

// Loading Cubemap
unsigned int loadCubemap(std::vector<std::string> faces) {

	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++) {
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else {
			std::cout << "Failed to load Cubemap texture at path:" << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

glm::mat4 GetPerspectiveProjMatrix(float fovy, float ascept, float znear, float zfar) {

	glm::mat4 proj = glm::mat4(1.0f);

	proj[0][0] = 1 / (tan(fovy / 2) * ascept);
	proj[1][0] = 0;
	proj[2][0] = 0;
	proj[3][0] = 0;

	proj[0][1] = 0;
	proj[1][1] = 1 / (tan(fovy / 2));
	proj[2][1] = 0;
	proj[3][1] = 0;

	proj[0][2] = 0;
	proj[1][2] = 0;
	proj[2][2] = -(zfar + znear) / (zfar - znear);
	proj[3][2] = (-2 * zfar * znear) / (zfar - znear);

	proj[0][3] = 0;
	proj[1][3] = 0;
	proj[2][3] = -1;
	proj[3][3] = 0;

	return proj;
}

glm::mat4 GetOrthoProjMatrix(float left, float right, float bottom, float top, float near, float far) {
	glm::mat4 proj = glm::mat4(1.0f);

	proj[0][0] = 2 / (right - left);
	proj[1][0] = 0;
	proj[2][0] = 0;
	proj[3][0] = -(right + left) / (right - left);

	proj[0][1] = 0;
	proj[1][1] = 2 / (top - bottom);
	proj[2][1] = 0;
	proj[3][1] = -(top + bottom) / (top - bottom);

	proj[0][2] = 0;
	proj[1][2] = 0;
	proj[2][2] = -2 / (far - near);
	proj[3][2] = -(far + near) / (far - near);

	proj[0][3] = 0;
	proj[1][3] = 0;
	proj[2][3] = 0;
	proj[3][3] = 1;

	return proj;
}
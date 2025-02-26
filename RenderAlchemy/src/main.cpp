#include <cmath>
#include <fstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "clut/clut.h"
#include "fonts/IconsLucide.h"
#include "renderer/Framebuffer.h"
#include "renderer/Geometry.h"
#include "renderer/GLUtils.h"
#include "renderer/Shader.h"
#include "ui/ImGuiUtils.h" // Add the new header

// Forward declarations
GLFWwindow* initializeWindow();
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void initImGui(GLFWwindow* window);
void renderImGuiInterface(std::map<std::string, CLUT>& clutLibrary, CLUT& currentClut, const char*& currentPreset, GLuint& clut1DTexture, GLuint& clut3DTexture, bool& use3DCLUT, char* customLutName, bool& editingMode, float* cameraPos);

// Global variables
int windowWidth = 1280;
int windowHeight = 720;
float rotationAngle = 0.0f;
bool framebufferResized = false;

// Scene settings
int sceneType = 0; // 0: Cube, 1: Sphere, 2: Plane
float lightPos[3] = { 3.0f, 3.0f, 3.0f };
float lightColor[3] = { 1.0f, 1.0f, 1.0f };
float lightIntensity = 1.0f;
float ambientStrength = 0.1f;
float modelRotation[3] = { 0.0f, 0.0f, 0.0f };

// Auto-rotation settings
bool autoRotateModel = false;
float modelRotationSpeed = 0.5f;
bool autoRotateLight = false;
float lightRotationSpeed = 0.3f;
float lightRotationRadius = 3.0f;
float lightRotationAngle = 0.0f;

// CLUT and tone mapping settings
float exposure = 1.0f;
float clutStrength = 0.7f;
int tonemapOperator = 0;
bool applyClut = true;
bool splitScreen = false;
float splitPosition = 0.5f;
bool showClut = true;

// CLUT editing
float clutContrast = 1.0f;
float clutSaturation = 1.0f;
float clutTemperature = 0.0f;
float clutTint = 0.0f;

// Window and UI state variables
bool showHelpWindow = false;
bool showAboutWindow = false;
bool showToolsWindow = true;
bool showPreferencesWindow = false;

int main()
{
	// Initialize window
	GLFWwindow* window = initializeWindow();
	if (!window)
		return -1;

	// Initialize ImGui
	initImGui(window);

	// Create shader programs
	std::cout << "Creating shader programs..." << std::endl;
	std::cout << "Scene shader compiling" << std::endl;
	Shader sceneShader;
	sceneShader.createFromPaths("shaders/scene.vert", "shaders/scene.frag");
	std::cout << "Tonemap shader compiling" << std::endl;
	Shader tonemapShader;
	tonemapShader.createFromPaths("shaders/tonemap.vert", "shaders/tonemap.frag");

	// Create CLUT library with presets
	std::map<std::string, CLUT> clutLibrary;
	CLUT::createPreset1DCLUTs(clutLibrary);
	CLUT::createPreset3DCLUTs(clutLibrary);

	// Set initial CLUT to Neutral 1D
	CLUT currentClut = clutLibrary["Neutral (1D)"];
	GLuint clut1DTexture = currentClut.create1DTexture();

	// Create an initial 3D CLUT texture too
	CLUT neutral3D = clutLibrary["Neutral (3D)"];
	neutral3D.setIs3DCLUT(true);
	GLuint clut3DTexture = neutral3D.create3DTexture();

	// Track which type of CLUT is active
	bool use3DCLUT = false;

	// Create geometry
	Geometry cube;
	cube.createCube();

	Geometry sphere;
	sphere.createSphere();

	Geometry plane;
	plane.createPlane();

	Geometry screenQuad;
	screenQuad.createScreenQuad();

	// Create framebuffer for HDR rendering
	Framebuffer hdrFramebuffer;
	hdrFramebuffer.create(windowWidth, windowHeight, true);

	// Camera settings
	float cameraPos[3] = { 0.0f, 0.0f, 3.0f };
	float cameraTarget[3] = { 0.0f, 0.0f, 0.0f };

	// Selected preset for combo box
	const char* currentPreset = "Neutral (1D)";
	char customLutName[128] = "MyCustomLUT";
	bool editingMode = false;

	// For tracking frame time
	float lastFrameTime = static_cast<float>(glfwGetTime());

	// Wireframe mode
	bool wireframeMode = false;

	// Main render loop
	while (!glfwWindowShouldClose(window))
	{
		// Process input
		processInput(window);

		// Calculate delta time for smooth animation
		float currentTime = static_cast<float>(glfwGetTime());
		float deltaTime = currentTime - lastFrameTime;
		lastFrameTime = currentTime;

		// Handle auto-rotation of model
		if (autoRotateModel)
		{
			modelRotation[1] += modelRotationSpeed * deltaTime;
			if (modelRotation[1] > 2.0f * 3.14159f)
			{
				modelRotation[1] -= 2.0f * 3.14159f;
			}
		}

		// Handle auto-rotation of light
		if (autoRotateLight)
		{
			lightRotationAngle += lightRotationSpeed * deltaTime;
			if (lightRotationAngle > 2.0f * 3.14159f)
			{
				lightRotationAngle -= 2.0f * 3.14159f;
			}

			// Update light position based on rotation angle
			lightPos[0] = lightRotationRadius * cos(lightRotationAngle);
			lightPos[2] = lightRotationRadius * sin(lightRotationAngle);
		}

		// Check if framebuffer needs to be resized
		if (framebufferResized)
		{
			hdrFramebuffer.resize(windowWidth, windowHeight);
			framebufferResized = false;
		}

		glPolygonMode(GL_FRONT_AND_BACK, wireframeMode ? GL_LINE : GL_FILL);

		// First pass: Render scene to HDR framebuffer
		hdrFramebuffer.bind();
		GLUtils::checkGLError("binding HDR framebuffer");

		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		// Use scene shader
		sceneShader.use();
		GLUtils::checkGLError("using scene shader program");

		// Set light properties
		sceneShader.setVec3("lightPos", lightPos[0], lightPos[1], lightPos[2]);
		sceneShader.setVec3("lightColor", lightColor[0], lightColor[1], lightColor[2]);
		sceneShader.setFloat("lightIntensity", lightIntensity);
		sceneShader.setFloat("ambientStrength", ambientStrength);
		sceneShader.setInt("sceneType", sceneType);
		GLUtils::checkGLError("setting scene shader uniforms");

		// View matrix (camera)
		float viewMatrix[16] = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -cameraPos[0], -cameraPos[1], -cameraPos[2], 1.0f };

		// Projection matrix
		float aspectRatio = static_cast<float>(windowWidth) / windowHeight;
		float fov = 45.0f * 3.14159f / 180.0f;
		float near = 0.1f;
		float far = 100.0f;
		float tanHalfFov = tan(fov / 2.0f);

		float projectionMatrix[16] = { 1.0f / (aspectRatio * tanHalfFov), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f / tanHalfFov, 0.0f, 0.0f, 0.0f, 0.0f, -(far + near) / (far - near), -1.0f, 0.0f, 0.0f, -(2.0f * far * near) / (far - near), 0.0f };

		// Model matrix with rotation
		float cosY = cos(modelRotation[1]);
		float sinY = sin(modelRotation[1]);
		float cosX = cos(modelRotation[0]);
		float sinX = sin(modelRotation[0]);
		float cosZ = cos(modelRotation[2]);
		float sinZ = sin(modelRotation[2]);

		float rotationMatrixY[16] = { cosY, 0.0f, sinY, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -sinY, 0.0f, cosY, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

		float rotationMatrixX[16] = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, cosX, -sinX, 0.0f, 0.0f, sinX, cosX, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

		float rotationMatrixZ[16] = { cosZ, -sinZ, 0.0f, 0.0f, sinZ, cosZ, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

		// Combined rotation matrix (Y * X * Z)
		float modelMatrix[16];
		// For simplicity, using identity matrix with translation
		for (int i = 0; i < 16; i++)
		{
			modelMatrix[i] = 0.0f;
		}
		modelMatrix[0] = cosY * cosZ;
		modelMatrix[1] = cosY * sinZ;
		modelMatrix[2] = -sinY;
		modelMatrix[4] = sinX * sinY * cosZ - cosX * sinZ;
		modelMatrix[5] = sinX * sinY * sinZ + cosX * cosZ;
		modelMatrix[6] = sinX * cosY;
		modelMatrix[8] = cosX * sinY * cosZ + sinX * sinZ;
		modelMatrix[9] = cosX * sinY * sinZ - sinX * cosZ;
		modelMatrix[10] = cosX * cosY;
		modelMatrix[15] = 1.0f;

		// Set matrices
		sceneShader.setMat4("model", modelMatrix);
		sceneShader.setMat4("view", viewMatrix);
		sceneShader.setMat4("projection", projectionMatrix);
		GLUtils::checkGLError("setting matrix uniforms");

		// Draw the appropriate geometry based on scene type
		if (sceneType == 0)
		{
			cube.draw();
		}
		else if (sceneType == 1)
		{
			sphere.draw();
		}
		else
		{
			plane.draw();
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLUtils::checkGLError("binding default framebuffer");
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		GLUtils::checkGLError("clearing default framebuffer");
		glDisable(GL_DEPTH_TEST);
		GLUtils::checkGLError("disabling depth test");

		// Use tone mapping shader
		tonemapShader.use();
		GLUtils::checkGLError("using tonemap shader program");

		// Ensure the VAO is properly bound before drawing
		glBindVertexArray(screenQuad.getVAO());
		GLUtils::checkGLError("binding quad VAO");

		// Bind HDR texture to texture unit 0
		glActiveTexture(GL_TEXTURE0);
		GLUtils::checkGLError("activating texture unit 0");
		glBindTexture(GL_TEXTURE_2D, hdrFramebuffer.getColorTexture());
		GLUtils::checkGLError("binding HDR texture");
		tonemapShader.setInt("hdrBuffer", 0);
		GLUtils::checkGLError("setting hdrBuffer uniform");

		// Bind appropriate CLUT texture to texture unit 1
		glActiveTexture(GL_TEXTURE1);
		GLUtils::checkGLError("activating texture unit 1");

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_1D, clut1DTexture);
		tonemapShader.setInt("colorLUT1D", 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_3D, clut3DTexture);
		tonemapShader.setInt("colorLUT3D", 2);

		// Set tone mapping parameters
		GLint exposureLoc = tonemapShader.getUniformLocation("exposure");
		if (exposureLoc != -1)
			glUniform1f(exposureLoc, exposure);

		GLint clutStrengthLoc = tonemapShader.getUniformLocation("clutStrength");
		if (clutStrengthLoc != -1)
			glUniform1f(clutStrengthLoc, clutStrength);

		GLint tonemapOperatorLoc = tonemapShader.getUniformLocation("tonemapOperator");
		if (tonemapOperatorLoc != -1)
			glUniform1i(tonemapOperatorLoc, tonemapOperator);

		GLint splitScreenLoc = tonemapShader.getUniformLocation("splitScreen");
		if (splitScreenLoc != -1)
			glUniform1i(splitScreenLoc, splitScreen ? 1 : 0);

		GLint splitPositionLoc = tonemapShader.getUniformLocation("splitPosition");
		if (splitPositionLoc != -1)
			glUniform1f(splitPositionLoc, splitPosition);

		GLint showClutLoc = tonemapShader.getUniformLocation("showClut");
		if (showClutLoc != -1)
			glUniform1i(showClutLoc, showClut ? 1 : 0);

		GLint applyClutLoc = tonemapShader.getUniformLocation("applyClut");
		if (applyClutLoc != -1)
			glUniform1i(applyClutLoc, applyClut ? 1 : 0);

		GLint use3DCLUTLoc = tonemapShader.getUniformLocation("use3DCLUT");
		if (use3DCLUTLoc != -1)
			glUniform1i(use3DCLUTLoc, use3DCLUT ? 1 : 0);

		GLint clutSizeLoc = tonemapShader.getUniformLocation("clutSize");
		if (clutSizeLoc != -1)
			glUniform1i(clutSizeLoc, currentClut.getSize());

		GLUtils::checkGLError("setting tonemap shader uniforms");

		// Draw fullscreen quad
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		GLUtils::checkGLError("drawing post-process quad");

		// Unbind VAO and program
		glBindVertexArray(0);
		GLUtils::checkGLError("unbinding VAO");
		glUseProgram(0);
		GLUtils::checkGLError("unbinding shader program");

		// Start ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Render the modern ImGui interface with dockspace
		renderImGuiInterface(clutLibrary, currentClut, currentPreset, clut1DTexture, clut3DTexture, use3DCLUT, customLutName, editingMode, cameraPos);

		// Render ImGui
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Swap buffers and poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// Delete both 1D and 3D CLUT textures
	glDeleteTextures(1, &clut1DTexture);
	glDeleteTextures(1, &clut3DTexture);

	glfwTerminate();
	return 0;
}

GLFWwindow* initializeWindow()
{
	// Initialize GLFW
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return nullptr;
	}

	// Configure GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create window
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Render Alchemy", NULL, NULL);
	if (!window)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	// Initialize GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Failed to initialize GLEW" << std::endl;
		return nullptr;
	}

	// Enable depth testing
	glEnable(GL_DEPTH_TEST);

	return window;
}

void initImGui(GLFWwindow* window)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	// Enable docking
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// Enable keyboard navigation
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	// Setup custom fonts and styling
	ImGuiUtils::InitializeImGui(io);

	// Apply Gruvbox theme
	ImGuiUtils::SetGruvboxTheme();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	// Update viewport when window is resized
	glViewport(0, 0, width, height);
	windowWidth = width;
	windowHeight = height;
	framebufferResized = true;
}

void processInput(GLFWwindow* window)
{
	// Close window on ESC key press
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// Render the modern ImGui interface
void renderImGuiInterface(std::map<std::string, CLUT>& clutLibrary, CLUT& currentClut, const char*& currentPreset, GLuint& clut1DTexture, GLuint& clut3DTexture, bool& use3DCLUT, char* customLutName, bool& editingMode, float* cameraPos)
{
	// Tools panel (control panel)
	if (showToolsWindow)
	{
		ImGui::SetNextWindowSize(ImVec2(350, 500), ImGuiCond_FirstUseEver); // Default size to ensure enough space
		ImGui::Begin("CLUT Controls", &showToolsWindow, ImGuiWindowFlags_NoCollapse);

		// Calculate consistent widths for controls to avoid overlaps and ensure enough room for labels
		float windowWidth = ImGui::GetWindowWidth();
		float fullControlWidth = windowWidth - 30.0f;          // Full-width slider with more padding
		float halfControlWidth = (windowWidth - 40.0f) / 2.0f; // Half-width slider with more padding

		// Create tabs at the top of the window
		ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
		if (ImGui::BeginTabBar("ControlTabs", tab_bar_flags))
		{
			// SCENE TAB
			if (ImGui::BeginTabItem("Scene"))
			{
				ImGui::Spacing();

				// Scene settings with proper spacing
				{
					const char* sceneItems[] = { "Cube", "Sphere", "Plane" };
					ImGui::Text("Scene Type"); // Add separate label
					ImGui::SetNextItemWidth(fullControlWidth);
					ImGui::Combo("##SceneType", &sceneType, sceneItems, IM_ARRAYSIZE(sceneItems));

					// Add tooltip
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Choose which 3D object to display");

					ImGui::Spacing();

					// Camera controls group with proper width
					ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.25f, 0.25f, 0.25f, 0.5f));
					ImGui::BeginGroup();
					ImGuiUtils::Icon(ICON_LC_CAMERA);
					ImGui::SameLine();
					ImGui::Text("Camera");
					ImGui::Text("Position"); // Add separate label
					ImGui::SetNextItemWidth(fullControlWidth);
					ImGui::SliderFloat3("##CameraPosition", cameraPos, -5.0f, 5.0f, "%.1f");
					ImGui::EndGroup();
					ImGui::PopStyleColor();

					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();

					// Model rotation controls - with proper spacing
					ImGui::BeginGroup();
					ImGuiUtils::Icon(ICON_LC_ROTATE_3D);
					ImGui::SameLine();
					ImGui::TextColored(ImVec4(0.72f, 0.73f, 0.15f, 1.00f), "Model Rotation");

					ImGui::Checkbox("Auto-Rotate Model", &autoRotateModel);

					if (autoRotateModel)
					{
						ImGui::Text("Rotation Speed"); // Add separate label
						ImGui::SetNextItemWidth(fullControlWidth);
						ImGui::SliderFloat("##ModelSpeed", &modelRotationSpeed, 0.1f, 2.0f, "%.2f");
					}
					else
					{
						ImGui::BeginDisabled(autoRotateModel);
						ImGui::Text("Rotation Axes"); // Add separate label
						ImGui::SetNextItemWidth(fullControlWidth);
						ImGui::SliderFloat3("##ModelRotation", modelRotation, -3.14159f, 3.14159f, "%.2f");
						ImGui::EndDisabled();
					}
					ImGui::EndGroup();

					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();

					// Light controls - with proper spacing
					ImGui::BeginGroup();
					ImGuiUtils::Icon(ICON_LC_SUN);
					ImGui::SameLine();
					ImGui::TextColored(ImVec4(0.98f, 0.75f, 0.18f, 1.00f), "Lighting");

					ImGui::Checkbox("Auto-Rotate Light", &autoRotateLight);

					if (autoRotateLight)
					{
						// Better layout for light rotation controls with separate labels
						ImGui::BeginGroup();
						ImGui::Text("Speed");
						ImGui::SetNextItemWidth(halfControlWidth);
						ImGui::SliderFloat("##LightSpeed", &lightRotationSpeed, 0.1f, 2.0f, "%.2f");
						ImGui::EndGroup();

						ImGui::SameLine(halfControlWidth + 20.0f);

						ImGui::BeginGroup();
						ImGui::Text("Radius");
						ImGui::SetNextItemWidth(halfControlWidth);
						ImGui::SliderFloat("##LightRadius", &lightRotationRadius, 1.0f, 10.0f, "%.1f");
						ImGui::EndGroup();
					}
					else
					{
						ImGui::BeginDisabled(autoRotateLight);
						ImGui::Text("Position"); // Add separate label
						ImGui::SetNextItemWidth(fullControlWidth);
						ImGui::SliderFloat3("##LightPosition", lightPos, -5.0f, 5.0f, "%.1f");
						ImGui::EndDisabled();
					}

					// Light color controls with separate label
					ImGui::Text("Light Color");
					ImGui::SetNextItemWidth(fullControlWidth);
					ImGui::ColorEdit3("##LightColor", lightColor);

					ImGui::Spacing();

					// Fixed layout for intensity and ambient with separate labels
					ImGui::BeginGroup();
					ImGui::Text("Intensity");
					ImGui::SetNextItemWidth(halfControlWidth);
					ImGui::SliderFloat("##LightIntensity", &lightIntensity, 0.0f, 5.0f, "%.2f");
					ImGui::EndGroup();

					ImGui::SameLine(halfControlWidth + 20.0f);

					ImGui::BeginGroup();
					ImGui::Text("Ambient");
					ImGui::SetNextItemWidth(halfControlWidth);
					ImGui::SliderFloat("##AmbientStrength", &ambientStrength, 0.0f, 1.0f, "%.2f");
					ImGui::EndGroup();

					ImGui::EndGroup();
				}

				ImGui::EndTabItem();
			}

			// TONE MAPPING TAB
			if (ImGui::BeginTabItem("Tone Mapping"))
			{
				ImGui::Spacing();

				// Header with icon
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.31f, 0.65f, 0.66f, 1.00f));
				ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[ImGuiUtils::FONT_MEDIUM]);
				ImGuiUtils::Icon(ICON_LC_IMAGE);
				ImGui::SameLine();
				ImGui::Text("Tone Mapping");
				ImGui::PopFont();
				ImGui::PopStyleColor();
				ImGui::Separator();
				ImGui::Spacing();

				// Tone mapping section with proper spacing
				{
					ImGui::Text("Exposure");
					ImGui::SetNextItemWidth(fullControlWidth);
					ImGui::SliderFloat("##Exposure", &exposure, 0.1f, 5.0f, "%.2f");

					ImGui::Spacing();

					ImGui::Text("Tone Mapping Operator");
					ImGui::SetNextItemWidth(fullControlWidth);
					const char* tonemapItems[] = { "Reinhard", "ACES" };
					ImGui::Combo("##TonemapOp", &tonemapOperator, tonemapItems, IM_ARRAYSIZE(tonemapItems));
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Different algorithms for compressing HDR to LDR");

					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();

					// CLUT controls with proper layout
					ImGui::BeginGroup();
					ImGui::Checkbox("Apply CLUT", &applyClut);

					if (applyClut)
					{
						ImGui::Text("CLUT Strength");
						ImGui::SetNextItemWidth(fullControlWidth);
						ImGui::SliderFloat("##ClutStrength", &clutStrength, 0.0f, 1.0f, "%.2f");
					}
					ImGui::EndGroup();

					ImGui::Spacing();

					// Split screen controls with proper layout
					ImGui::BeginGroup();
					ImGui::Checkbox("Split Screen Comparison", &splitScreen);

					if (splitScreen)
					{
						ImGui::Text("Split Position");
						ImGui::SetNextItemWidth(fullControlWidth);
						ImGui::SliderFloat("##SplitPosition", &splitPosition, 0.0f, 1.0f, "%.2f");
					}
					ImGui::EndGroup();

					ImGui::Spacing();
					ImGui::Checkbox("Show CLUT Preview", &showClut);
				}

				ImGui::EndTabItem();
			}

			// CLUT SELECTION TAB
			if (ImGui::BeginTabItem("CLUT Selection"))
			{
				ImGui::Spacing();

				// Header with icon
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.83f, 0.46f, 0.78f, 1.00f));
				ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[ImGuiUtils::FONT_MEDIUM]);
				ImGuiUtils::Icon(ICON_LC_PALETTE);
				ImGui::SameLine();
				ImGui::Text("CLUT Selection");
				ImGui::PopFont();
				ImGui::PopStyleColor();
				ImGui::Separator();
				ImGui::Spacing();

				// CLUT preset selection with proper width and label
				ImGui::Text("Select CLUT Preset");
				ImGui::SetNextItemWidth(fullControlWidth);
				if (ImGui::BeginCombo("##ClutPreset", currentPreset))
				{
					for (const auto& preset: clutLibrary)
					{
						bool isSelected = (currentPreset == preset.first.c_str());

						// Add indicator for 1D vs 3D CLUTs
						std::string displayName = preset.first;
						if (preset.second.is3DCLUT())
						{
							displayName += " [3D]";
						}
						else
						{
							displayName += " [1D]";
						}

						if (ImGui::Selectable(displayName.c_str(), isSelected))
						{
							currentPreset = preset.first.c_str();
							currentClut = preset.second;

							// Update use3DCLUT flag based on the selected preset
							use3DCLUT = preset.second.is3DCLUT();

							// Update appropriate texture based on CLUT type
							if (use3DCLUT)
							{
								glDeleteTextures(1, &clut3DTexture);
								clut3DTexture = CLUT::create3DCLUT(currentClut.getData(), currentClut.getSize());
							}
							else
							{
								glDeleteTextures(1, &clut1DTexture);
								clut1DTexture = CLUT::create1DCLUT(currentClut.getData(), currentClut.getSize());
							}

							// Reset CLUT editing parameters when selecting a preset
							clutContrast = 1.0f;
							clutSaturation = 1.0f;
							clutTemperature = 0.0f;
							clutTint = 0.0f;
							editingMode = false;
						}
						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				ImGui::Spacing();

				// Display info about current CLUT in a framed group with a header
				ImGui::Text("Current CLUT Information");
				ImGui::BeginChild("CLUTInfo", ImVec2(fullControlWidth, 80), true);
				ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "Name: %s", currentPreset);
				ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "Type: %s", use3DCLUT ? "3D" : "1D");
				ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "Size: %d", currentClut.getSize());
				ImGui::EndChild();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// File loading controls with proper spacing and labels
				ImGui::TextColored(ImVec4(0.54f, 0.76f, 0.49f, 1.00f), "Load CLUT From File");

				ImGui::Text("File Path");
				static char loadPath[256] = "lut.cube";
				ImGui::SetNextItemWidth(fullControlWidth);
				ImGui::InputText("##LoadPath", loadPath, IM_ARRAYSIZE(loadPath));

				ImGui::Spacing();

				// Buttons with proper layout and more space
				if (ImGui::Button("Browse...", ImVec2(halfControlWidth, 25)))
				{
					ImGuiUtils::Icon(ICON_LC_FOLDER_OPEN);
					// Could show file browser dialog here
				}

				ImGui::SameLine(halfControlWidth + 20.0f);

				if (ImGui::Button("Load CLUT", ImVec2(halfControlWidth, 25)))
				{
					ImGuiUtils::Icon(ICON_LC_IMPORT);
					try
					{
						CLUT loadedClut = loadedClut.loadFromFile(loadPath);
						currentClut = loadedClut;
						clutLibrary[loadedClut.getName()] = loadedClut;
						currentPreset = loadedClut.getName().c_str();

						// Update CLUT texture based on type
						if (loadedClut.is3DCLUT())
						{
							use3DCLUT = true;
							glDeleteTextures(1, &clut3DTexture);
							clut3DTexture = CLUT::create3DCLUT(loadedClut.getData(), loadedClut.getSize());
						}
						else
						{
							use3DCLUT = false;
							glDeleteTextures(1, &clut1DTexture);
							clut1DTexture = CLUT::create1DCLUT(loadedClut.getData(), loadedClut.getSize());
						}

						ImGui::OpenPopup("CLUT Loaded");
					}
					catch (const std::exception& e)
					{
						ImGui::OpenPopup("Load Error");
					}
				}

				// Popup modals - keep as is
				if (ImGui::BeginPopupModal("CLUT Loaded", NULL, ImGuiWindowFlags_AlwaysAutoResize))
				{
					ImGui::Text("CLUT loaded successfully!");
					ImGui::Text("Type: %s", currentClut.is3DCLUT() ? "3D CLUT" : "1D CLUT");
					ImGui::Text("Size: %d", currentClut.getSize());
					ImGui::Spacing();
					if (ImGui::Button("OK", ImVec2(120, 0)))
					{
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}

				if (ImGui::BeginPopupModal("Load Error", NULL, ImGuiWindowFlags_AlwaysAutoResize))
				{
					ImGui::Text("Failed to load CLUT file!");
					ImGui::Spacing();
					if (ImGui::Button("OK", ImVec2(120, 0)))
					{
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}

				ImGui::EndTabItem();
			}

			// CLUT EDITING TAB
			if (ImGui::BeginTabItem("CLUT Editing"))
			{
				ImGui::Spacing();

				// Header with icon
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.83f, 0.46f, 0.78f, 1.00f));
				ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[ImGuiUtils::FONT_MEDIUM]);
				ImGuiUtils::Icon(ICON_LC_PAINTBRUSH);
				ImGui::SameLine();
				ImGui::Text("CLUT Editing");
				ImGui::PopFont();
				ImGui::PopStyleColor();
				ImGui::Separator();
				ImGui::Spacing();

				// CLUT editing controls
				ImGui::BeginGroup();
				ImGui::Checkbox("Edit CLUT", &editingMode);

				if (editingMode)
				{
					ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));

					ImGui::Spacing();

					// Better layout for the sliders with separate labels
					ImGui::BeginGroup();
					ImGui::Text("Contrast");
					ImGui::SetNextItemWidth(halfControlWidth);
					ImGui::SliderFloat("##Contrast", &clutContrast, 0.5f, 2.0f, "%.2f");
					ImGui::EndGroup();

					ImGui::SameLine(halfControlWidth + 20.0f);

					ImGui::BeginGroup();
					ImGui::Text("Saturation");
					ImGui::SetNextItemWidth(halfControlWidth);
					ImGui::SliderFloat("##Saturation", &clutSaturation, 0.0f, 2.0f, "%.2f");
					ImGui::EndGroup();

					ImGui::Spacing();
					ImGui::Spacing(); // Extra spacing between rows

					ImGui::BeginGroup();
					ImGui::Text("Temperature");
					ImGui::SetNextItemWidth(halfControlWidth);
					ImGui::SliderFloat("##Temperature", &clutTemperature, -1.0f, 1.0f, "%.2f");
					ImGui::EndGroup();

					ImGui::SameLine(halfControlWidth + 20.0f);

					ImGui::BeginGroup();
					ImGui::Text("Tint");
					ImGui::SetNextItemWidth(halfControlWidth);
					ImGui::SliderFloat("##Tint", &clutTint, -1.0f, 1.0f, "%.2f");
					ImGui::EndGroup();

					ImGui::PopStyleColor();

					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();

					ImGui::Text("Custom Name");
					ImGui::SetNextItemWidth(fullControlWidth);
					ImGui::InputText("##CustomName", customLutName, IM_ARRAYSIZE(customLutName));

					ImGui::Spacing();
					ImGui::Spacing(); // Extra spacing before buttons

					// Buttons with proper layout and more height
					if (ImGui::Button("Apply Changes", ImVec2(halfControlWidth, 28)))
					{
						// Generate new CLUT based on parameters and type
						if (use3DCLUT)
						{
							currentClut = clutLibrary["Neutral (3D)"];
							glDeleteTextures(1, &clut3DTexture);
							clut3DTexture = CLUT::create3DCLUT(currentClut.getData(), currentClut.getSize());
						}
						else
						{
							currentClut = clutLibrary["Neutral (1D)"];
							glDeleteTextures(1, &clut1DTexture);
							clut1DTexture = CLUT::create1DCLUT(currentClut.getData(), currentClut.getSize());
						}

						// Update current preset name
						currentPreset = use3DCLUT ? "Custom 3D" : "Custom 1D";
					}

					ImGui::SameLine(halfControlWidth + 20.0f);

					if (ImGui::Button("Save Custom CLUT", ImVec2(halfControlWidth, 28)))
					{
						// Save current CLUT to library
						currentClut.setName(customLutName);
						clutLibrary[customLutName] = currentClut;
						currentPreset = customLutName;

						// Save to file
						currentClut.saveToFile(std::string(customLutName) + ".cube");
						ImGui::OpenPopup("CLUT Saved");
					}

					if (ImGui::BeginPopupModal("CLUT Saved", NULL, ImGuiWindowFlags_AlwaysAutoResize))
					{
						ImGui::Text("CLUT saved as %s.cube", customLutName);
						ImGui::Spacing();
						if (ImGui::Button("OK", ImVec2(120, 0)))
						{
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
					}
				}
				else
				{
					// Display help text when editing is disabled
					ImGui::Spacing();
					ImGui::BeginChild("EditingHelp", ImVec2(fullControlWidth, 100), true);
					ImGui::TextWrapped("Enable \"Edit CLUT\" to modify the current color lookup table. "
					                   "You can adjust contrast, saturation, color temperature, and tint, "
					                   "then save your custom CLUT for future use.");
					ImGui::EndChild();
				}
				ImGui::EndGroup();

				ImGui::EndTabItem();
			}

			// HELP TAB
			if (ImGui::BeginTabItem("Help"))
			{
				ImGui::Spacing();

				// Header with icon
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 0.4f, 1.0f));
				ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[ImGuiUtils::FONT_MEDIUM]);
				ImGuiUtils::Icon(ICON_LC_SHIELD_QUESTION);
				ImGui::SameLine();
				ImGui::Text("Help & Information");
				ImGui::PopFont();
				ImGui::PopStyleColor();
				ImGui::Separator();
				ImGui::Spacing();

				// Display help text in a scrollable area
				ImGui::BeginChild("HelpText", ImVec2(fullControlWidth, 200), true);
				ImGui::TextWrapped("This application demonstrates color lookup table (CLUT) tone mapping techniques for HDR rendering."
				                   "\n\nNavigation:"
				                   "\n- Scene Tab: Configure the 3D scene, lighting and camera"
				                   "\n- Tone Mapping Tab: Adjust exposure and tone mapping settings"
				                   "\n- CLUT Selection Tab: Choose from preset CLUTs or load from file"
				                   "\n- CLUT Editing Tab: Create and modify your own color tables"
				                   "\n\nShortcuts:"
				                   "\n- Esc: Exit application");
				ImGui::EndChild();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::BeginChild("AboutInfo", ImVec2(fullControlWidth, 80), true);
				ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "About");
				ImGui::TextWrapped("CLUT Demo v1.0.0\nCopyright © 2023");
				ImGui::EndChild();

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::End();
	}
}

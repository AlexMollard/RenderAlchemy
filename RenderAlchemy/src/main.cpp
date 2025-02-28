#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>

#include "clut/clut.h"
#include "fonts/IconsLucide.h"
#include "imgui/imgui_impl_bgfx.h"
#include "renderer/BgfxUtils.h"
#include "renderer/Framebuffer.h"
#include "renderer/Geometry.h"
#include "renderer/Shader.h"
#include "ui/ImGuiUtils.h"

// Forward declarations
void framebufferSizeCallback(int width, int height);
void processInput(GLFWwindow* window);
void initImGui();
void renderImGuiInterface(std::map<std::string, CLUT>& clutLibrary, CLUT& currentClut, const char*& currentPreset, bgfx::TextureHandle& clut1DTexture, bgfx::TextureHandle& clut3DTexture, bool& use3DCLUT, char* customLutName, bool& editingMode, float* cameraPos);

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

// For bgfx view IDs
constexpr uint8_t kSceneView = 0;
constexpr uint8_t kPostProcessView = 1;

static void* glfwNativeWindowHandle(GLFWwindow* _window)
{
	return glfwGetWin32Window(_window);
}

int main()
{
	// Create a native window for bgfx using glfw
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return -1;
	}

	// Set OpenGL version to 4.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a windowed mode window and its OpenGL context
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "RenderAlchemy", NULL, NULL);
	if (!window)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// Make the window's context current
	glfwMakeContextCurrent(window);

	// Initialize bgfx
	if (!BgfxUtils::init(windowWidth, windowHeight, glfwNativeWindowHandle(window)))
	{
		std::cerr << "Failed to initialize bgfx" << std::endl;
		return -1;
	}

	// Initialize ImGui
	initImGui();

	// Create shader programs
	std::cout << "Creating shader programs..." << std::endl;
	std::cout << "Scene shader compiling" << std::endl;
	Shader sceneShader("shaders/scene.vert.sc", "shaders/scene.frag.sc");
	std::cout << "Tonemap shader compiling" << std::endl;
	Shader tonemapShader("shaders/tonemap.vert.sc", "shaders/tonemap.frag.sc");

	// Create CLUT library with presets
	std::map<std::string, CLUT> clutLibrary;
	CLUT::createPreset1DCLUTs(clutLibrary);
	CLUT::createPreset3DCLUTs(clutLibrary);

	// Set initial CLUT to Neutral 1D
	CLUT currentClut = clutLibrary["Neutral (1D)"];
	bgfx::TextureHandle clut1DTexture = BGFX_INVALID_HANDLE;
	
	// For 1D CLUT (Creating the bgfx texture)
	std::vector<float> clutDataWithAlpha(currentClut.getSize() * 4);
	for (int i = 0; i < currentClut.getSize(); ++i) {
		clutDataWithAlpha[i * 4 + 0] = currentClut.getData()[i * 3 + 0];
		clutDataWithAlpha[i * 4 + 1] = currentClut.getData()[i * 3 + 1];
		clutDataWithAlpha[i * 4 + 2] = currentClut.getData()[i * 3 + 2];
		clutDataWithAlpha[i * 4 + 3] = 1.0f; // Set alpha to 1.0
	}
	const bgfx::Memory* mem1D = bgfx::copy(clutDataWithAlpha.data(), clutDataWithAlpha.size() * sizeof(float));
	clut1DTexture = bgfx::createTexture2D(currentClut.getSize(), 1, false, 1, bgfx::TextureFormat::RGBA32F, BGFX_TEXTURE_NONE, mem1D);

	// Create an initial 3D CLUT texture too
	CLUT neutral3D = clutLibrary["Neutral (3D)"];
	neutral3D.setIs3DCLUT(true);
	bgfx::TextureHandle clut3DTexture = BGFX_INVALID_HANDLE;

	// For 3D CLUT (Creating the bgfx texture)
	std::vector<float> clutDataWithAlpha3D(neutral3D.getSize() * neutral3D.getSize() * neutral3D.getSize() * 4);
	for (int i = 0; i < neutral3D.getSize() * neutral3D.getSize() * neutral3D.getSize(); ++i) {
		clutDataWithAlpha3D[i * 4 + 0] = neutral3D.getData()[i * 3 + 0];
		clutDataWithAlpha3D[i * 4 + 1] = neutral3D.getData()[i * 3 + 1];
		clutDataWithAlpha3D[i * 4 + 2] = neutral3D.getData()[i * 3 + 2];
		clutDataWithAlpha3D[i * 4 + 3] = 1.0f; // Set alpha to 1.0
	}
	const bgfx::Memory* mem3D = bgfx::copy(clutDataWithAlpha3D.data(), clutDataWithAlpha3D.size() * sizeof(float));
	clut3DTexture = bgfx::createTexture3D(neutral3D.getSize(), neutral3D.getSize(), neutral3D.getSize(), false, bgfx::TextureFormat::RGBA32F, BGFX_TEXTURE_NONE, mem3D);

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
	const bgfx::Stats* stats = bgfx::getStats();
	float lastFrameTime = stats->cpuTimeBegin;

	// Wireframe mode
	bool wireframeMode = false;

	// Main render loop
	while (true)
	{
		// Process input
		processInput(window);

		// Calculate delta time for smooth animation
		stats = bgfx::getStats();
		const double toMsCpu = 1000.0 / stats->cpuTimerFreq;
		const double toMsGpu = 1000.0 / stats->gpuTimerFreq;
		const double frameMs = double(stats->cpuTimeFrame) * toMsCpu;
		int64_t deltaTime = stats->cpuTimeFrame;
		lastFrameTime = frameMs;

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
			BgfxUtils::resize(windowWidth, windowHeight);
			framebufferResized = false;
		}

		// Begin bgfx frame
		BgfxUtils::beginFrame();

		// Set wireframe mode if needed
		uint64_t state = BGFX_STATE_DEFAULT;
		if (wireframeMode)
		{
			state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_MSAA | BGFX_STATE_PT_LINES;
		}

		// First pass: Render scene to HDR framebuffer
		hdrFramebuffer.bind();

		// Clear framebuffer
		bgfx::setViewClear(kSceneView, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x0c0c0cff, 1.0f, 0);
		bgfx::setViewRect(kSceneView, 0, 0, windowWidth, windowHeight);

		// View matrix (camera)
		float view[16] = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -cameraPos[0], -cameraPos[1], -cameraPos[2], 1.0f };

		// Projection matrix
		float aspectRatio = static_cast<float>(windowWidth) / windowHeight;
		float fov = 45.0f * 3.14159f / 180.0f;
		float nearplane = 0.1f;
		float farplane = 100.0f;
		float tanHalfFov = tan(fov / 2.0f);

		float proj[16] = { 1.0f / (aspectRatio * tanHalfFov), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f / tanHalfFov, 0.0f, 0.0f, 0.0f, 0.0f, -(farplane + nearplane) / (farplane - nearplane), -1.0f, 0.0f, 0.0f, -(2.0f * farplane * nearplane) / (farplane - nearplane), 0.0f };

		bgfx::setViewTransform(kSceneView, view, proj);

		// Model matrix with rotation
		float cosY = cos(modelRotation[1]);
		float sinY = sin(modelRotation[1]);
		float cosX = cos(modelRotation[0]);
		float sinX = sin(modelRotation[0]);
		float cosZ = cos(modelRotation[2]);
		float sinZ = sin(modelRotation[2]);

		float model[16] = { 0 };
		model[0] = cosY * cosZ;
		model[1] = cosY * sinZ;
		model[2] = -sinY;
		model[4] = sinX * sinY * cosZ - cosX * sinZ;
		model[5] = sinX * sinY * sinZ + cosX * cosZ;
		model[6] = sinX * cosY;
		model[8] = cosX * sinY * cosZ + sinX * sinZ;
		model[9] = cosX * sinY * sinZ - sinX * cosZ;
		model[10] = cosX * cosY;
		model[15] = 1.0f;

		// Set uniforms for scene shader
		sceneShader.setUniform("lightPos", lightPos);
		sceneShader.setUniform("lightColor", lightColor);
		sceneShader.setUniform("lightIntensity", &lightIntensity);
		sceneShader.setUniform("ambientStrength", &ambientStrength);
		sceneShader.setUniform("sceneType", &sceneType, 1);

		// Set transform for the model
		bgfx::setTransform(model);

		// Draw the appropriate geometry based on scene type
		if (sceneType == 0)
		{
			cube.draw(sceneShader);
		}
		else if (sceneType == 1)
		{
			sphere.draw(sceneShader);
		}
		else
		{
			plane.draw(sceneShader);
		}

		// Second pass: Apply tone mapping and CLUT to the HDR image
		hdrFramebuffer.unbind();

		// Clear the backbuffer
		bgfx::setViewClear(kPostProcessView, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000ff, 1.0f, 0);
		bgfx::setViewRect(kPostProcessView, 0, 0, windowWidth, windowHeight);

		// Set the orthographic projection for post-processing
		float orthoProj[16] = { 2.0f / windowWidth, 0.0f, 0.0f, 0.0f, 0.0f, -2.0f / windowHeight, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f, 1.0f };

		float identity[16] = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

		bgfx::setViewTransform(kPostProcessView, identity, orthoProj);

		// Set HDR framebuffer texture
		tonemapShader.setTexture("hdrBuffer", hdrFramebuffer.getColorTexture(), 0);

		// Set CLUT textures
		tonemapShader.setTexture("colorLUT1D", clut1DTexture, 1);
		tonemapShader.setTexture("colorLUT3D", clut3DTexture, 2);

		// Set tone mapping parameters
		tonemapShader.setUniform("exposure", &exposure);
		tonemapShader.setUniform("clutStrength", &clutStrength);

		int tonemapOp = tonemapOperator;
		tonemapShader.setUniform("tonemapOperator", &tonemapOp, 1);

		int splitScreenInt = splitScreen ? 1 : 0;
		tonemapShader.setUniform("splitScreen", &splitScreenInt, 1);

		tonemapShader.setUniform("splitPosition", &splitPosition);

		int showClutInt = showClut ? 1 : 0;
		tonemapShader.setUniform("showClut", &showClutInt, 1);

		int applyClutInt = applyClut ? 1 : 0;
		tonemapShader.setUniform("applyClut", &applyClutInt, 1);

		int use3DCLUTInt = use3DCLUT ? 1 : 0;
		tonemapShader.setUniform("use3DCLUT", &use3DCLUTInt, 1);

		int clutSize = currentClut.getSize();
		tonemapShader.setUniform("clutSize", &clutSize, 1);

		// Draw screen quad with tonemap shader
		screenQuad.draw(tonemapShader);

		// Start ImGui frame
		ImGui_ImplBgfx_NewFrame();
		ImGui::NewFrame();

		// Render the modern ImGui interface with dockspace
		renderImGuiInterface(clutLibrary, currentClut, currentPreset, clut1DTexture, clut3DTexture, use3DCLUT, customLutName, editingMode, cameraPos);

		// Render ImGui
		ImGui::Render();
		ImGui_ImplBgfx_RenderDrawData(ImGui::GetDrawData());

		// End bgfx frame
		BgfxUtils::endFrame();

		// Poll events
		// Replace glfwPollEvents() with a custom event polling mechanism if needed
	}

	// Cleanup
	ImGui_ImplBgfx_Shutdown();
	ImGui::DestroyContext();

	// Delete CLUT textures
	if (bgfx::isValid(clut1DTexture))
	{
		bgfx::destroy(clut1DTexture);
	}

	if (bgfx::isValid(clut3DTexture))
	{
		bgfx::destroy(clut3DTexture);
	}

	// Clean up geometry
	cube.~Geometry();
	sphere.~Geometry();
	plane.~Geometry();
	screenQuad.~Geometry();

	// Clean up framebuffer
	hdrFramebuffer.cleanup();

	// Shutdown bgfx
	BgfxUtils::shutdown();

	return 0;
}

void initImGui()
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

	// Setup Platform/Renderer backends - using bgfx instead of OpenGL
	ImGui_ImplBgfx_Init(kPostProcessView); // Use bgfx renderer
}

void framebufferSizeCallback(int width, int height)
{
	// Update viewport when window is resized
	windowWidth = width;
	windowHeight = height;
	framebufferResized = true;
}

void processInput(GLFWwindow* window)
{
	glfwPollEvents();

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	// If the user wants to close the window
	if (glfwWindowShouldClose(window))
	{
		glfwTerminate();
		exit(EXIT_SUCCESS);
	}
}

// Render the modern ImGui interface - updated parameter types for bgfx
void renderImGuiInterface(std::map<std::string, CLUT>& clutLibrary, CLUT& currentClut, const char*& currentPreset, bgfx::TextureHandle& clut1DTexture, bgfx::TextureHandle& clut3DTexture, bool& use3DCLUT, char* customLutName, bool& editingMode, float* cameraPos)
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
								bgfx::destroy(clut3DTexture);
								std::vector<float> clutDataWithAlpha3D(currentClut.getSize() * currentClut.getSize() * currentClut.getSize() * 4);
								for (int i = 0; i < currentClut.getSize() * currentClut.getSize() * currentClut.getSize(); ++i)
								{
									clutDataWithAlpha3D[i * 4 + 0] = currentClut.getData()[i * 3 + 0];
									clutDataWithAlpha3D[i * 4 + 1] = currentClut.getData()[i * 3 + 1];
									clutDataWithAlpha3D[i * 4 + 2] = currentClut.getData()[i * 3 + 2];
									clutDataWithAlpha3D[i * 4 + 3] = 1.0f; // Set alpha to 1.0
								}
								const bgfx::Memory* mem3D = bgfx::copy(clutDataWithAlpha3D.data(), clutDataWithAlpha3D.size() * sizeof(float));
								clut3DTexture = bgfx::createTexture3D(currentClut.getSize(), currentClut.getSize(), currentClut.getSize(), false, bgfx::TextureFormat::RGBA32F, BGFX_TEXTURE_NONE, mem3D);
							}
							else
							{
								bgfx::destroy(clut1DTexture);
								std::vector<float> clutDataWithAlpha(currentClut.getSize() * 4);
								for (int i = 0; i < currentClut.getSize(); ++i) {
									clutDataWithAlpha[i * 4 + 0] = currentClut.getData()[i * 3 + 0];
									clutDataWithAlpha[i * 4 + 1] = currentClut.getData()[i * 3 + 1];
									clutDataWithAlpha[i * 4 + 2] = currentClut.getData()[i * 3 + 2];
									clutDataWithAlpha[i * 4 + 3] = 1.0f; // Set alpha to 1.0
								}
								const bgfx::Memory* mem1D = bgfx::copy(clutDataWithAlpha.data(), clutDataWithAlpha.size() * sizeof(float));
								clut1DTexture = bgfx::createTexture2D(currentClut.getSize(), 1, false, 1, bgfx::TextureFormat::RGBA32F, BGFX_TEXTURE_NONE, mem1D);
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
							bgfx::destroy(clut3DTexture);
							std::vector<float> clutDataWithAlpha3D(currentClut.getSize() * currentClut.getSize() * currentClut.getSize() * 4);
							for (int i = 0; i < currentClut.getSize() * currentClut.getSize() * currentClut.getSize(); ++i)
							{
								clutDataWithAlpha3D[i * 4 + 0] = currentClut.getData()[i * 3 + 0];
								clutDataWithAlpha3D[i * 4 + 1] = currentClut.getData()[i * 3 + 1];
								clutDataWithAlpha3D[i * 4 + 2] = currentClut.getData()[i * 3 + 2];
								clutDataWithAlpha3D[i * 4 + 3] = 1.0f; // Set alpha to 1.0
							}
							const bgfx::Memory* mem3D = bgfx::copy(clutDataWithAlpha3D.data(), clutDataWithAlpha3D.size() * sizeof(float));
							clut3DTexture = bgfx::createTexture3D(currentClut.getSize(), currentClut.getSize(), currentClut.getSize(), false, bgfx::TextureFormat::RGBA32F, BGFX_TEXTURE_NONE, mem3D);
						}
						else
						{
							use3DCLUT = false;
							bgfx::destroy(clut1DTexture);
							std::vector<float> clutDataWithAlpha(currentClut.getSize() * 4);
							for (int i = 0; i < currentClut.getSize(); ++i) {
								clutDataWithAlpha[i * 4 + 0] = currentClut.getData()[i * 3 + 0];
								clutDataWithAlpha[i * 4 + 1] = currentClut.getData()[i * 3 + 1];
								clutDataWithAlpha[i * 4 + 2] = currentClut.getData()[i * 3 + 2];
								clutDataWithAlpha[i * 4 + 3] = 1.0f; // Set alpha to 1.0
							}
							const bgfx::Memory* mem1D = bgfx::copy(clutDataWithAlpha.data(), clutDataWithAlpha.size() * sizeof(float));
							clut1DTexture = bgfx::createTexture2D(currentClut.getSize(), 1, false, 1, bgfx::TextureFormat::RGBA32F, BGFX_TEXTURE_NONE, mem1D);
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
							bgfx::destroy(clut3DTexture);
							std::vector<float> clutDataWithAlpha3D(currentClut.getSize() * currentClut.getSize() * currentClut.getSize() * 4);
							for (int i = 0; i < currentClut.getSize() * currentClut.getSize() * currentClut.getSize(); ++i)
							{
								clutDataWithAlpha3D[i * 4 + 0] = currentClut.getData()[i * 3 + 0];
								clutDataWithAlpha3D[i * 4 + 1] = currentClut.getData()[i * 3 + 1];
								clutDataWithAlpha3D[i * 4 + 2] = currentClut.getData()[i * 3 + 2];
								clutDataWithAlpha3D[i * 4 + 3] = 1.0f; // Set alpha to 1.0
							}
							const bgfx::Memory* mem3D = bgfx::copy(clutDataWithAlpha3D.data(), clutDataWithAlpha3D.size() * sizeof(float));
							clut3DTexture = bgfx::createTexture3D(currentClut.getSize(), currentClut.getSize(), currentClut.getSize(), false, bgfx::TextureFormat::RGBA32F, BGFX_TEXTURE_NONE, mem3D);
						}
						else
						{
							currentClut = clutLibrary["Neutral (1D)"];
							bgfx::destroy(clut1DTexture);
							std::vector<float> clutDataWithAlpha(currentClut.getSize() * 4);
							for (int i = 0; i < currentClut.getSize(); ++i) {
								clutDataWithAlpha[i * 4 + 0] = currentClut.getData()[i * 3 + 0];
								clutDataWithAlpha[i * 4 + 1] = currentClut.getData()[i * 3 + 1];
								clutDataWithAlpha[i * 4 + 2] = currentClut.getData()[i * 3 + 2];
								clutDataWithAlpha[i * 4 + 3] = 1.0f; // Set alpha to 1.0
							}
							const bgfx::Memory* mem1D = bgfx::copy(clutDataWithAlpha.data(), clutDataWithAlpha.size() * sizeof(float));
							clut1DTexture = bgfx::createTexture2D(currentClut.getSize(), 1, false, 1, bgfx::TextureFormat::RGBA32F, BGFX_TEXTURE_NONE, mem1D);
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
				ImGui::TextWrapped("CLUT Demo v1.0.0\nBrought to you by bingus and friends");
				ImGui::EndChild();

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::End();
	}
}

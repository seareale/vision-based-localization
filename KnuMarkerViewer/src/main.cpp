// dear imgui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>

// About OpenGL function loaders: modern OpenGL doesn't have a standard header file and requires individual function pointers to be loaded manually.
// Helper libraries are often used for this purpose! Here we are supporting a few common ones: gl3w, glew, glad.
// You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>    // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>    // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>  // Initialize with gladLoadGL()
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

#include "yeAssimp.h"
#include "knuCamera.h"
#include "knuGlModel.h"

static void glfw_error_callback(int error, const char* description) {
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main(int argc, char** argv) {
	// default : camera = 0, markId = 0
	int cameraId = 0;
	int markId = 0;
	int threadMode = 0;

	// ./viewer (markId) (cameraId)
	if( argc >= 2 ) {
		markId= atoi(argv[1]);
		if(argc == 3){
			threadMode = atoi(argv[2]);
		}
		else if(argc == 4){
			cameraId = atoi(argv[3]);
		}
		else if(argc > 4){
			cout << "parameter error" << endl;
			exit(0);
		}
	} 

	// Setup window
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit()) return 1;

	// Decide GL+GLSL versions
#if __APPLE__
	// GL 3.2 + GLSL 150
	const char* glsl_version = "#version 150";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

	// Create window with graphics context
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
	if (window == NULL)
		return 1;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	// Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
	bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
	bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
	bool err = gladLoadGL() == 0;
#else
	bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
	if (err) {
		fprintf(stderr, "Failed to initialize OpenGL loader!\n");
		return 1;
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Our state
	//bool show_demo_window = true;
	bool show_demo_window = false;
	bool show_another_window = false;

	// imgui 변수
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	static float f = 0.5f;
	static int clickButton = 0;

	////////////////////////////////////////////////////////////////////////////////////////////////

	KCvCamera cvCam;
	KGlCamera glCam;
	glm::mat4 matView = glm::mat4(0.0f);


	// 사용할 Opengl shader program loading.
	SYE::Shader shaderGrid("./shader/grid.vs", "./shader/grid.fs");
	SYE::Shader shaderTriangle("./shader/sample.vertex","./shader/sample.fragment");

	// 사용할 Opengl object들을 생성.
	KOglGrid objGrid(&shaderGrid);

	KOglObject objTriangle;
	objTriangle.initialize(&shaderTriangle, KObjTriangle); // KObjTriangle, KObjRectangle

	KOglObject objRectangle;
	objRectangle.initialize(&shaderTriangle, KObjRectangle); // KObjTriangle, KObjRectangle


	cv::Mat imgShow;
	unsigned int uiCamImgId = 0;
	glGenTextures(1, &uiCamImgId);

	//if(!cvCam.init(0, true)) {		// Thread mode로 사용 시
	if(!cvCam.init(cameraId, threadMode)) {
	}

	// camera, openCV window
	namedWindow("OPEN_CAM", WINDOW_NORMAL);
	resizeWindow("OPEN_CAM", 640,480);
	
	// Main loop
	while (!glfwWindowShouldClose(window)) {
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			ImGui::Begin("Setting Box");                          // Create a window called "Hello, world!" and append into it.
			
			ImGui::Text("Zoom");
			ImGui::SliderFloat("", &f, 0.1f, 5.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			
			ImGui::Text("bg-color");
			ImGui::ColorEdit3("", (float*)&clear_color); // Edit 3 floats representing a color

			ImGui::Text("Object");
			if (ImGui::Button("Triangle"))
				clickButton = 0;
			ImGui::SameLine();
			if (ImGui::Button("Rectangle"))
				clickButton = 1;
				
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

			if(!imgShow.empty()) {
				ImGui::Image((void *)(intptr_t)uiCamImgId, ImVec2(imgShow.cols, imgShow.rows));
			}

			ImGui::End();
		}

		// 3. Show another simple window.
		if (show_another_window) {
			// Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Begin("Another Window", &show_another_window);   
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}


		// Set opengl view projection matrix.
		if(cvCam.getCamImage(imgShow)){
			if(cvCam.isOpened() && cvCam.getMarkerPose(markId, matView, imgShow)) {
				// zoom 적용
				matView[3][2] *= f;
			} else {
				// Set default view projection matrix.
				matView = glm::mat4(1.0f);
				matView[3][2] = -100.0f;
			}
			imshow("OPEN_CAM",imgShow);
		}


		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Opengl - Projection, View, Model matrix 필요
		// FOV 45'
		glm::mat4 matProj = glm::perspective(45.0f, (float)display_w / (float)display_h, 0.1f, 1000.0f);

		// Rendering Back-Ground scene. ////////////////////////////////////////////////////////////
		objGrid.render(matView, matProj);

		// Rendering Opengl objects. 	////////////////////////////////////////////////////////////
		switch(clickButton){
			case 0:
				objTriangle.render(matView, matProj);	
				break;

			case 1:
				objRectangle.render(matView, matProj);
				break;
			
			default :
				break;
		}

		////////////////////////////////////////////////////////////////////////////////////////////

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
		//  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}

		glfwSwapBuffers(window);

		if(waitKey(10) == 27) break;
	}

	cvCam.close();

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

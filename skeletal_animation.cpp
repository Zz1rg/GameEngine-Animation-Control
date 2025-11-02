#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/animator.h>
#include <learnopengl/model_animation.h>



#include <iostream>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec3 characterPosition = glm::vec3(0.0f, -0.6f, 0.0f);


enum AnimState {
	IDLE = 1,
	IDLE_WALK,
	WALK_IDLE,
	WALK,
	RUN,
	IDLE_RUN,
	RUN_IDLE,
	ROLL_IDLE,
	IDLE_ROLL
};

bool roll = false;

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
	stbi_set_flip_vertically_on_load(true);

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader ourShader("anim_model.vs", "anim_model.fs");


	// load models
	// -----------
	Model ourModel(FileSystem::getPath("resources/objects/wiz/idle-mixamo/Warrior Idle.dae"));
	Animation idleAnimation(FileSystem::getPath("resources/objects/wiz/idle-mixamo/Warrior Idle.dae"), &ourModel);
	Animation walkAnimation(FileSystem::getPath("resources/objects/wiz/walk-inplace-mixamo/Walking.dae"), &ourModel);
	Animation runAnimation(FileSystem::getPath("resources/objects/wiz/run-inplace-mixamo/Fast Run.dae"), &ourModel);
	//Animation stepAnimation(FileSystem::getPath("resources/objects/wiz/step-mixamo/Standing Dodge Backward.dae"), &ourModel);
	Animation rollAnimation(FileSystem::getPath("resources/objects/wiz/roll-inplace-mixamo/Standing Dive Forward.dae"), &ourModel);
	Animator animator(&idleAnimation);
	enum AnimState charState = IDLE;
	float blendAmount = 0.0f;
	float blendRate = 0.055f;

	// draw in wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);
		if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
			animator.PlayAnimation(&idleAnimation, NULL, 0.0f, 0.0f, 0.0f);
		if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
			animator.PlayAnimation(&walkAnimation, NULL, 0.0f, 0.0f, 0.0f);
		if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
			animator.PlayAnimation(&runAnimation, NULL, 0.0f, 0.0f, 0.0f);
		if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
			animator.PlayAnimation(&rollAnimation, NULL, 0.0f, 0.0f, 0.0f);

		float walkSpeed = 2.0f;
		float rollSpeed = 5.0f;
		float runSpeed = 5.0f;

		switch (charState) {
		case IDLE:
			if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
				blendAmount = 0.0f;
				animator.PlayAnimation(&idleAnimation, &walkAnimation, animator.m_CurrentTime, 0.0f, blendAmount);
				charState = IDLE_WALK;
			}
			else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
				blendAmount = 0.0f;
				animator.PlayAnimation(&idleAnimation, &rollAnimation, animator.m_CurrentTime, 0.0f, blendAmount);
				charState = IDLE_ROLL;
			}
			else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
				blendAmount = 0.0f;
				animator.PlayAnimation(&idleAnimation, &runAnimation, animator.m_CurrentTime, 0.0f, blendAmount);
				charState = IDLE_RUN;
			}
			printf("idle \n");
			break;
		case IDLE_WALK:
			blendAmount += blendRate;
			blendAmount = fmod(blendAmount, 1.0f);
			animator.PlayAnimation(&idleAnimation, &walkAnimation, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
			characterPosition += glm::vec3(0.0f, 0.0f, -1.0f) * walkSpeed * deltaTime;
			if (blendAmount > 0.9f) {
				blendAmount = 0.0f;
				float startTime = animator.m_CurrentTime2;
				animator.PlayAnimation(&walkAnimation, NULL, startTime, 0.0f, blendAmount);
				charState = WALK;
			}
			printf("idle_walk \n");
			break;
		case WALK:
			animator.PlayAnimation(&walkAnimation, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
			//characterPosition += glm::vec3(0.0f, 0.0f, -1.0f) * walkSpeed * deltaTime;
			if (glfwGetKey(window, GLFW_KEY_E) != GLFW_PRESS) {
				charState = WALK_IDLE;
			}
			printf("walking\n");
			break;
		case WALK_IDLE:
			blendAmount += blendRate;
			blendAmount = fmod(blendAmount, 1.0f);
			animator.PlayAnimation(&walkAnimation, &idleAnimation, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
			if (blendAmount > 0.9f) {
				blendAmount = 0.0f;
				float startTime = animator.m_CurrentTime2;
				animator.PlayAnimation(&idleAnimation, NULL, startTime, 0.0f, blendAmount);
				charState = IDLE;
			}
			printf("walk_idle \n");
			break;
		case IDLE_RUN:
			blendAmount += blendRate;
			blendAmount = fmod(blendAmount, 1.0f);
			animator.PlayAnimation(&idleAnimation, &runAnimation, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
			if (blendAmount > 0.9f) {
				blendAmount = 0.0f;
				float startTime = animator.m_CurrentTime2;
				animator.PlayAnimation(&idleAnimation, NULL, startTime, 0.0f, blendAmount);
				charState = RUN;
			}
			printf("idle_run \n");
			break;
		case RUN:
			animator.PlayAnimation(&runAnimation, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
			//characterPosition += glm::vec3(0.0f, 0.0f, -1.0f) * runSpeed * deltaTime;
			if (glfwGetKey(window, GLFW_KEY_R) != GLFW_PRESS) {
				charState = RUN_IDLE;
			}
			printf("running\n");
			break;
		case RUN_IDLE:
			blendAmount += blendRate;
			blendAmount = fmod(blendAmount, 1.0f);
			animator.PlayAnimation(&runAnimation, &idleAnimation, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
			if (blendAmount > 0.9f) {
				blendAmount = 0.0f;
				float startTime = animator.m_CurrentTime2;
				animator.PlayAnimation(&idleAnimation, NULL, startTime, 0.0f, blendAmount);
				charState = IDLE;
			}
			printf("run_idle \n");
			break;
		case IDLE_ROLL:
			blendAmount += blendRate;
			blendAmount = fmod(blendAmount, 1.0f);
			animator.PlayAnimation(&idleAnimation, &rollAnimation, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
			if (blendAmount > 0.7f) {
				blendAmount = 0.0f;
				float startTime = animator.m_CurrentTime2;
				animator.PlayAnimation(&rollAnimation, NULL, startTime, 0.0f, blendAmount);
				charState = ROLL_IDLE;
			}
			printf("idle_roll\n");
			break;
		case ROLL_IDLE:
			if (animator.m_CurrentTime > 0.7f) {
				blendAmount += blendRate;
				blendAmount = fmod(blendAmount, 1.0f);
				animator.PlayAnimation(&rollAnimation, &idleAnimation, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
				if (blendAmount > 0.9f) {
					blendAmount = 0.0f;
					float startTime = animator.m_CurrentTime2;
					animator.PlayAnimation(&idleAnimation, NULL, startTime, 0.0f, blendAmount);
					charState = IDLE;
				}
				printf("roll_idle \n");
			}
			else {
				// punching
				printf("rolling \n");
			}
			break;
		}



		animator.UpdateAnimation(deltaTime);

		// render
		// ------
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// don't forget to enable shader before setting uniforms
		ourShader.use();

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);

		auto transforms = animator.GetFinalBoneMatrices();
		for (int i = 0; i < transforms.size(); ++i)
			ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);


		// render the loaded model
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, characterPosition); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(.75f, .75f, .75f));	// it's a bit too big for our scene, so scale it down
		ourShader.setMat4("model", model);
		ourModel.Draw(ourShader);


		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

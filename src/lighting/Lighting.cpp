#include "Lighting.h"

void Lighting::updateLightPos() {
	double newTime = glfwGetTime();
	if (newTime - currentTime > 0.01) {
		lightPos.x = sin(newTime) * lightRadius;
		lightPos.z = cos(newTime) * lightRadius;
		currentTime = newTime;
	}
}

Lighting::Lighting() {
	camera = Camera(glm::vec3(0.0f, 0.0f, 4.0f), glm::vec3(0.0, 1.0, 0.0));
	lightPos = glm::vec3(0.0f, -0.5f, 0.0f);
}

void Lighting::createLightingScene() {
	GLFWwindow* window = CreateWindow::createWindow();
	GLuint VBO, cubeVAO, lampVAO;
	bool menu = true;
	bool phongOrGouraud = true;
	glEnable(GL_DEPTH_TEST);
	Shader* shaderPtr = nullptr;
	Shader lightingShader("./src/lighting/lighting_cube.vs", "./src/lighting/lighting_cube.fs");
	Shader gougraudShader("./src/lighting/Gouraud_cube.vs", "./src/lighting/Gouraud_cube.fs");
	Shader lampShader("./src/lighting/lamp.vs", "./src/lighting/lamp.fs");
	// bind VBO and cubeVAO
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &VBO);
	
	 //bind lampVAO
	glGenVertexArrays(1, &lampVAO);
	glBindVertexArray(lampVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// ImgUI
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");
	ImGui::StyleColorsClassic();
	while (!glfwWindowShouldClose(window)) {		
		glfwPollEvents();
		if (!phongOrGouraud) {
			shaderPtr = &gougraudShader;
		} else {
			shaderPtr = &lightingShader;
		}
		// cube
		glBindVertexArray(cubeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		// lamp
		glBindVertexArray(lampVAO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// draw the cube
		shaderPtr->use();
		// set object color and light color
		shaderPtr->setVec3("objectColor", glm::vec3(1.0f, 0.5f, 0.31f));
		shaderPtr->setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
		shaderPtr->setVec3("lightPos", lightPos);
		shaderPtr->setVec3("viewPos", camera.position);
		shaderPtr->setFloat("ambientStrength", ambient);
		shaderPtr->setFloat("diffuseStrength", diffuse);
		shaderPtr->setFloat("specularStrength", specular);
		shaderPtr->setInt("specularPow", specularPow);
		// perspective projection
		glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(camera.zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		shaderPtr->setMat4("projection", projection);
		// camera view
		glm::mat4 view = glm::mat4(1.0f);
		view = camera.getViewMat();
		shaderPtr->setMat4("view", view);
		// model
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0, -1, 0));
		model = glm::rotate(model, glm::radians(22.5f), glm::vec3(0, 1, 0));
		shaderPtr->setMat4("model", model);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// draw the lamp
		lampShader.use();
		lampShader.setMat4("projection", projection);
		lampShader.setMat4("view", view);
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f));
		lampShader.setMat4("model", model);

		glBindVertexArray(lampVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// imgui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::Begin("Menu", &menu, ImGuiWindowFlags_MenuBar);
		ImGui::Checkbox("Phong or Gouraud", &phongOrGouraud);
		ImGui::SliderFloat("camera x", &camx, -10.0f, 10.0f);
		ImGui::SliderFloat("camera y", &camy, -10.0f, 10.0f);
		ImGui::SliderFloat("camera z", &camz, -10.0f, 10.0f);
		ImGui::SliderFloat("ambient", &ambient, 0.0f, 1.0f);
		ImGui::SliderFloat("diffuse", &diffuse, 0.0f, 1.0f);
		ImGui::SliderFloat("specular", &specular, 0.0f, 1.0f);
		ImGui::SliderInt("specularPow", &specularPow, 2, 64); 
		ImGui::End();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		camera.position = glm::vec3(camx, camy, camz);
		updateLightPos();
		glfwMakeContextCurrent(window);
		glfwSwapBuffers(window);
	}
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &lampVAO);
	glDeleteBuffers(1, &VBO);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
}
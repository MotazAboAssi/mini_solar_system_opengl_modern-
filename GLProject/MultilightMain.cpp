#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "tools/Polygon.h"
#include "tools/Cube.h"
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <random>
#include <chrono>
#include <cmath>
using namespace std;
using namespace glm;

int SCR_WIDTH = 1280;
int SCR_HEIGHT = 720;

Camera camera(vec3(0, 0, 30));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;

bool isKOSOF = false;
bool isKHOSOF = false;

bool blinn = false;

void processInput(GLFWwindow *window, float &timeFactor, float sceneTime, vec3 moonPlanet, vec3 earthPlanet, Shader &shader);
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

vector<GLuint> loadTextures(vector<string> paths, GLuint wrapOption = GL_REPEAT, GLuint filterOption = GL_LINEAR)
{
    vector<GLuint> textures = {};

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapOption);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapOption);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterOption);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterOption);

    for (string path : paths)
    {
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        int width, height, nrChannels;
        unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
            std::cout << "Failed to load texture" << std::endl;
        stbi_image_free(data);

        textures.push_back(texture);
    }

    return textures;
}

int main()
{
    glfwInit();
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Lecture 5", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    Shader allShader("./shaders/vs/L5.vs", "./shaders/fs/L5-Multi.fs");
    Shader lightSourceShader("./shaders/vs/LightSource.vs", "./shaders/fs/LightSource.fs");

    Model earth("./models/source/planet.obj");
    Model moon("./models/source/planet.obj");
    Model sun("./models/source/planet.obj");
    
    glEnable(GL_DEPTH_TEST);

    stbi_set_flip_vertically_on_load(true);

    vector<string> texturePaths = {};
    texturePaths.push_back("./models/textures/earth.jpeg");
    texturePaths.push_back("./models/textures/moon.jpeg");
    texturePaths.push_back("./models/textures/sun.jpeg");
    vector<GLuint> textures = loadTextures(texturePaths);

    float timeFactor = 1.0f;
    float sceneTime = 0.0f;
    float lastFrame = 0.0f;
    
    while (!glfwWindowShouldClose(window))
    {
        vec3 diffusSun = isKOSOF ? vec3(0.01f, 0.01f, 0.01f) : vec3(0.9f, 0.9f, 0.9f);
        vec3 specularSun = isKOSOF ? vec3(0.01f, 0.01f, 0.01f) : vec3(0.5f, 0.5f, 0.5f);
        vec3 diffusMoon = isKOSOF ? vec3(0.01f, 0.01f, 0.01f) : vec3(0.6f, 0.6f, 0.6f);
        vec3 specularMoon = isKOSOF ? vec3(0.01f, 0.01f, 0.01f) : vec3(0.025f, 0.025f, 0.025f);
        vec3 colorMoon = isKHOSOF ? vec3(0.5f, 0.5f, 0.5f) : vec3(1.0f, 1.0f, 1.0f);

        allShader.use();
        allShader.setVec3("viewPos", camera.Position);

#pragma region Light Settings
        // Directional light
        allShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        allShader.setVec3("dirLight.ambient", 0.0f, 0.0f, 0.0f);
        allShader.setVec3("dirLight.diffuse", 0.05f, 0.05f, 0.05f);
        allShader.setVec3("dirLight.specular", 0.2f, 0.2f, 0.2f);
        // sun
        allShader.setVec3("pointLights[0].position", vec3(sun.matrix_transformation[3]));
        allShader.setVec3("pointLights[0].ambient", 0.2f, 0.2f, 0.2f);
        allShader.setVec3("pointLights[0].diffuse", diffusSun);
        allShader.setVec3("pointLights[0].specular", specularSun);
        allShader.setFloat("pointLights[0].constant", 1.0f);
        allShader.setFloat("pointLights[0].linear", 0.14f);
        allShader.setFloat("pointLights[0].quadratic", 0.07f);
        // moon
        allShader.setVec3("pointLights[1].position", vec3(moon.matrix_transformation[3]));
        allShader.setVec3("pointLights[1].ambient", 0.02f, 0.02f, 0.02f);
        allShader.setVec3("pointLights[1].diffuse", diffusMoon);
        allShader.setVec3("pointLights[1].specular", specularMoon);
        allShader.setFloat("pointLights[1].constant", 1.0f);
        allShader.setFloat("pointLights[1].linear", 0.14f);
        allShader.setFloat("pointLights[1].quadratic", 0.07f);

#pragma endregion

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        sceneTime += deltaTime * timeFactor;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mat4 projection = mat4(1.0f);
        projection = perspective(radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        allShader.setMat4("projection", projection);

        mat4 view = mat4(1.0f);
        view = view = camera.GetViewMatrix();
        allShader.setMat4("view", view);

        allShader.setVec3("lightPos", vec3(earth.matrix_transformation[3])); // handel
        allShader.setVec3("objectColor", vec3(1.0f, 1.0f, 1.0f));

        // earth
        float aEarth = 15.0f;
        float bEarth = 12.0f;
        mat4 model = glm::mat4(1.0f);
        model = rotate(model, radians(25.0f), vec3(0, 0, 1));
        model = translate(model, vec3(aEarth * cos(sceneTime), bEarth * sin(sceneTime), 0.0f));
        model = rotate(model, sceneTime * 2, vec3(0, 0, 1));
        model = scale(model, vec3(0.5));
        allShader.setMat4("model", model);
        earth.Transformation(model);

        glBindTexture(GL_TEXTURE_2D, textures[0]);
        earth.Draw(allShader);

        // sun
        model = glm::mat4(1.0f);
        model = rotate(model, sceneTime * 0.5f, vec3(0, 0, 1));
        model = scale(model, vec3(0.8));
        sun.Transformation(model);

        lightSourceShader.use();
        lightSourceShader.setMat4("projection", projection);
        lightSourceShader.setMat4("view", view);
        lightSourceShader.setMat4("model", model);
        lightSourceShader.setVec3("objectColor", vec3(1.0f, 1.0f, 1.0f));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[2]);
        lightSourceShader.setInt("diffuseTexture", 0);
        sun.Draw(lightSourceShader);

        // moon
        float aMoon = 15.0f;
        float bMoon = 12.0f;
        model = glm::mat4(1.0f);
        model = rotate(model, radians(25.0f), vec3(0, 0, 1));
        model = translate(model, vec3(aMoon * cos(sceneTime), bMoon * sin(sceneTime), 0.0f));
        model = rotate(model, sceneTime * 1.5f, vec3(0, 0, 1));
        model = translate(model, vec3(earth.matrix_transformation[3][0] * 0.25, earth.matrix_transformation[3][1] * 0.25, 0));
        model = rotate(model, sceneTime, vec3(0, 0, 1));
        model = scale(model, vec3(0.1));
        moon.Transformation(model);

        lightSourceShader.use();
        lightSourceShader.setMat4("projection", projection);
        lightSourceShader.setMat4("view", view);
        lightSourceShader.setMat4("model", model);
        lightSourceShader.setVec3("objectColor", colorMoon);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textures[1]);
        lightSourceShader.setInt("diffuseTexture", 1);
        moon.Draw(lightSourceShader);

        processInput(window, timeFactor, sceneTime, vec3(moon.matrix_transformation[3]), vec3(earth.matrix_transformation[3]), allShader);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window, float &timeFactor, float sceneTime, vec3 moonPlanet, vec3 earthPlanet, Shader &shader)
{
    static bool pressG = false;
    static bool pressH = false;

    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
    {
        pressH = false;
        pressG = true;
        isKHOSOF = false;
        timeFactor = 5.0f;
    }
    if (pressG && timeFactor != 0.0f)
    {
        float isLine = floor((earthPlanet.y * moonPlanet.x) - (earthPlanet.x * moonPlanet.y));
        if (isLine == 0)
            if (abs(moonPlanet.y) < abs(earthPlanet.y))
            {
                cout << "KOSOF" << endl;

                isKOSOF = true;

                timeFactor = 0.0f;
            }
    }

    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
    {
        pressG = false;
        pressH = true;
        isKOSOF = false;
        timeFactor = 5.0f;
    }
    if (pressH && timeFactor != 0.0f)
    {
        float isLine = floor((earthPlanet.y * moonPlanet.x) - (earthPlanet.x * moonPlanet.y));
        if (isLine == 0)
            if (abs(moonPlanet.y) > abs(earthPlanet.y))
            {
                cout << "KHOSOF" << endl;

                isKHOSOF = true;

                timeFactor = 0.0f;
            }
    }

    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
    {
        pressG = false;
        pressH = false;
        isKOSOF = false;
        isKHOSOF = false;

        timeFactor = 1.0f;
    }

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

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

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

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

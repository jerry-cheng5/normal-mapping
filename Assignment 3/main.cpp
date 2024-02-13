/*
Student Information
Student ID: 1155210270
Student Name: Jerry Cheng
*/
#include "./Dependencies/glew/glew.h"
#include "./Dependencies/GLFW/glfw3.h"
#include "./Dependencies/glm/glm.hpp"
#include "./Dependencies/glm/gtc/matrix_transform.hpp"
#include "Shader.h"
#include "Texture.h"
#include "Misc.h"

#include <iostream>
#include <fstream>
#include <vector>

// Testing variables

// screen setting
const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

struct MouseController {
    bool LEFT_BUTTON = false;
    bool RIGHT_BUTTON = false;
    double MOUSE_Clickx = 0.0, MOUSE_Clicky = 0.0;
    double MOUSE_X = 0.0, MOUSE_Y = 0.0;
    int click_time = glfwGetTime();
};

struct KeyController {
    bool W_KEY = false;
    bool S_KEY = false;
    bool A_KEY = false;
    bool D_KEY = false;
};

// Controllers
MouseController mouseCtrl;
KeyController keyCtrl;

class Camera {
    
public:
    float R;
    float Theta;
    float Phi;
    
    glm::vec3 Position;
    glm::vec3 Target;
    glm::vec3 Up;
    
    float MouseSensitivity;
    float ScrollSensitivity;
    float KeySensitivity;
    
    // SphericalPosition is R, Theta, Phi
    Camera(glm::vec3 sphericalPosition = glm::vec3(0.0f, 0.0f, 0.0f), float sensitivity = 1.f, float keySensitivity = 0.05f,
           float scrollSensitivity = 0.1f, glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f))
    {
        R = sphericalPosition.x;
        Theta = glm::radians(sphericalPosition.y);
        Phi = glm::radians(sphericalPosition.z);
        Target = target;
        Up = up;
        MouseSensitivity = sensitivity;
        ScrollSensitivity = scrollSensitivity;
        KeySensitivity = keySensitivity;
        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Target, Up);
    }
    
    void ProcessMouseMovement_Left(float xoffset, float yoffset, GLboolean constrainPitch = true) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Phi += xoffset * 2 * M_PI / SCR_WIDTH;
        Theta += yoffset * M_PI / SCR_HEIGHT;
        
        clampPhi();
        clampTheta();
    
        updateCameraVectors();
    }
    
    void ProcessKeyPress() {
        if (keyCtrl.W_KEY)
            R -= KeySensitivity;
        if (keyCtrl.S_KEY)
            R += KeySensitivity;
        if (keyCtrl.A_KEY) {
            Phi += KeySensitivity * 20 * 2 * M_PI / SCR_WIDTH;
            clampPhi();
        }
        if (keyCtrl.D_KEY) {
            Phi -= KeySensitivity * 20 * 2 * M_PI / SCR_WIDTH;
            clampPhi();
        }
        updateCameraVectors();
    }
    
    void ProcessMouseScroll(float yoffset)
    {
        R -= yoffset * ScrollSensitivity;
        if (R < 5.0f)
            R = 5.0f;
        if (R > 20.0f)
            R = 20.0f;
        
        updateCameraVectors();
    }

private:
    
    void clampPhi() {
        Phi = Phi > 0 ? glm::min(Phi, float(Phi - 2*M_PI)) : Phi + 2*M_PI;
    }
    
    void clampTheta() {
        Theta = glm::min(Theta, float(M_PI_2) - 0.01f);
        Theta = glm::max(Theta, -float(M_PI_2) + 0.01f);
    }

    void updateCameraVectors() {
        Position.x = R * cos(Theta) * cos(Phi);
        Position.y = R * sin(Theta);
        Position.z = R * cos(Theta) * sin(Phi);
    }
};

// Shaders
Shader shader;
Shader skyboxShader;
Shader nmShader;

// Textures
Texture planetTexture;
Texture planetNormal;
Texture spacecraftTexture;
Texture skyboxTexture;
Texture ufoTexture;
Texture rockTexture;

// Normal Mapping
std::vector<glm::vec3> tangents;
std::vector<glm::vec3> biTangents;

// Models
Model planet;
Model spacecraft;
Model ufo;
Model rock;

// Lighting
float envLightIntensity = 0.8f;
glm::vec3 envLightPos = glm::vec3(0.0f, 10.0f, 10.0f);

// VAO
GLuint vao[4];
GLuint vao_skybox;

// Camera
Camera camera;

// Skybox Coords
GLfloat skyboxVertices[] =
{
    // Left
    1.0f, 1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f,
    
    // Right
    -1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,
    
    // Front
    1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,
    
    // Back
    -1.0f, 1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    
    // Top
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    
    // Bottom
    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, -1.0f,
    
    
};
int E_skybox = 36;

// Rock Variables
int rockCount = 400;
std::vector<glm::mat4> modelMatrices;

// Other Variables
float planetRotationSpeed = 0.2f;
float currentTime;
glm::vec3 spacecraftScale = glm::vec3(0.0008f);



void GetTangentsAndBitangents_Planet() {
    
    for (int i = 0; i < planet.vertices.size(); i++) {
        tangents.push_back(glm::vec3(1.0f));
        biTangents.push_back(glm::vec3(1.0f));
    }
    
    for (int i = 0; i < planet.indices.size(); i += 3) {
        unsigned int i0 = planet.indices[i];
        unsigned int i1 = planet.indices[i+1];
        unsigned int i2 = planet.indices[i+2];
        
        glm::vec3 &v0 = planet.vertices[i0].position;
        glm::vec3 &v1 = planet.vertices[i1].position;
        glm::vec3 &v2 = planet.vertices[i2].position;
        
        glm::vec2 &uv0 = planet.vertices[i0].uv;
        glm::vec2 &uv1 = planet.vertices[i1].uv;
        glm::vec2 &uv2 = planet.vertices[i2].uv;
        
        glm::vec3 dp1 = v1 - v0;
        glm::vec3 dp2 = v2 - v0;
        
        glm::vec2 duv1 = uv1 - uv0;
        glm::vec2 duv2 = uv2 - uv0;
        
        float r = 1.0f / (duv1.x * duv2.y - duv1.y * duv2.x);
        glm::vec3 tangent = (dp1 * duv2.y - dp2 * duv1.y) * r;
        glm::vec3 biTangent = (dp2 * duv1.x - dp1 * duv2.x) * r;
        
        tangents[i0] += tangent;
        biTangents[i0] += biTangent;
        
        tangents[i1] += tangent;
        biTangents[i1] += biTangent;
        
        tangents[i2] += tangent;
        biTangents[i2] += biTangent;
    }
    
    for (int i = 0; i < planet.vertices.size(); i++) {
        tangents[i] = glm::normalize(tangents[i]);
        biTangents[i] = glm::normalize(biTangents[i]);
    }
}

void CreateRand_ModelMatrices() {
    GLfloat radius = 6.0f;
    GLfloat offset = 0.5f;
    GLfloat displacement;
    
    for (int i = 0; i < rockCount; i++) {
        glm::mat4 model = glm::mat4(1.0f);
        
        GLfloat angle = (GLfloat)i / (GLfloat)rockCount * 360.0f;
        displacement = (rand() % (GLint)(2 * offset * 200)) / 100.0f - offset;
        GLfloat x = sin(angle) * radius + displacement;
        displacement = (rand() % (GLint)(2 * offset * 200)) / 100.0f - offset;
        GLfloat y = displacement * 0.4f + 1;
        displacement = (rand() % (GLint)(2 * offset * 200)) / 100.0f - offset;
        GLfloat z = cos(angle) * radius + displacement;
        
        model = glm::translate(model, glm::vec3(x, y, z));
        
        GLfloat scale = (rand() % 10) / 100.0f + 0.05;
        model = glm::scale(model, glm::vec3(scale));
        
        GLfloat rotateAngle = (rand() % 360);
        model = glm::rotate(model, rotateAngle, glm::vec3(0.4f, 0.5f, 0.8f));
        
        modelMatrices.push_back(model);
    }
}

void get_OpenGL_info()
{
	// OpenGL information
	const GLubyte* name = glGetString(GL_VENDOR);
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* glversion = glGetString(GL_VERSION);
	std::cout << "OpenGL company: " << name << std::endl;
	std::cout << "Renderer name: " << renderer << std::endl;
	std::cout << "OpenGL version: " << glversion << std::endl;
}



void loadMeshes()
{
    // TODO: load objects and norm vertices to unit bbox
    // TODO: bind the data to VAO & VBO
    // TODO: set up textures
    // TODO: prepare for normal mapping
    // TODO: create asteroid ring
    // TODO: initialize craft motion


    GLuint vbo[6];
    GLuint ebo[4];
    glGenBuffers(6, vbo);
    glGenBuffers(4, ebo);
        
    // Planet
    planet = loadOBJ("resources/object/planet.obj");
    GetTangentsAndBitangents_Planet();
    glGenVertexArrays(1, &vao[0]);
    glBindVertexArray(vao[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, planet.vertices.size() * sizeof(Vertex), &planet.vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, planet.indices.size() * sizeof(unsigned int), &planet.indices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
    glBufferData(GL_ARRAY_BUFFER, tangents.size() * sizeof(glm::vec3), &tangents[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
    glBufferData(GL_ARRAY_BUFFER, biTangents.size() * sizeof(glm::vec3), &biTangents[0], GL_STATIC_DRAW);

    // Position, UV Coords, Vertex Normals, Tangents, BiTangents
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    
    
    // Spacecraft
    spacecraft = loadOBJ("resources/object/spacecraft.obj");
    glGenVertexArrays(1, &vao[1]);
    glBindVertexArray(vao[1]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, spacecraft.vertices.size() * sizeof(Vertex), &spacecraft.vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, spacecraft.indices.size() * sizeof(unsigned int), &spacecraft.indices[0], GL_STATIC_DRAW);
    
    // Position, UV Coords, Vertex Normals
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    
    // Rock
    rock = loadOBJ("resources/object/rock.obj");
    glGenVertexArrays(1, &vao[2]);
    glBindVertexArray(vao[2]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, rock.vertices.size() * sizeof(Vertex), &rock.vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, rock.indices.size() * sizeof(unsigned int), &rock.indices[0], GL_STATIC_DRAW);
    
    // Position, UV Coords, Vertex Normals
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    
    // Set random model matrices for rocks
    CreateRand_ModelMatrices();
    
    // Ufos
    ufo = loadOBJ("resources/object/craft.obj");
    glGenVertexArrays(1, &vao[3]);
    glBindVertexArray(vao[3]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
    glBufferData(GL_ARRAY_BUFFER, ufo.vertices.size() * sizeof(Vertex), &ufo.vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ufo.indices.size() * sizeof(unsigned int), &ufo.indices[0], GL_STATIC_DRAW);
    
    // Position, UV Coords, Vertex Normals
    glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    

    //Load textures
    planetTexture.setupTexture("resources/texture/earthTexture.bmp");
    planetNormal.setupTexture("resources/texture/earthNormal.bmp");
    spacecraftTexture.setupTexture("resources/texture/spacecraftTexture.bmp");
    rockTexture.setupTexture("resources/texture/rockTexture.bmp");
    ufoTexture.setupTexture("resources/texture/craftTexture.bmp");
 }

void createSkybox()
{
    // TODO: load skybox and bind to VAO & VBO
    // TODO: set up skybox textures
    // TODO: set up skybox shader
    
    unsigned int VBO;
    glGenVertexArrays(1, &vao_skybox);
    glGenBuffers(1, &VBO);
    glBindVertexArray(vao_skybox);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    std::vector<std::string> texPaths;
    texPaths.push_back(std::string("resources/skybox/right.bmp"));
    texPaths.push_back(std::string("resources/skybox/left.bmp"));
    texPaths.push_back(std::string("resources/skybox/top.bmp"));
    texPaths.push_back(std::string("resources/skybox/bottom.bmp"));
    texPaths.push_back(std::string("resources/skybox/front.bmp"));
    texPaths.push_back(std::string("resources/skybox/back.bmp"));
    skyboxTexture.setupTextureCubemap(texPaths);
    
    skyboxShader.setupShader("skybox.vs", "skybox.fs");
}

void sendDataToOpenGL()
{
	//TODO
	//Load objects and bind to VAO and VBO
	//Load textures
    
    loadMeshes();
    createSkybox();
   
}

void initializedGL(void) //run only once
{
    if (glewInit() != GLEW_OK) {
        std::cout << "GLEW not OK." << std::endl;
    }

    get_OpenGL_info();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	sendDataToOpenGL();
    
    // set up the camera parameters
    camera = Camera(glm::vec3(18.0f, 15.0f, 90.0f), 0.2f, 0.01f);

    // set up vetex shader and fragment shader
    shader.setupShader("vert.glsl", "frag.glsl");
    
    // set up normal shaders
    nmShader.setupShader("nm.vs", "nm.fs");
    
}



void paintGL(void)  //always run
{
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f); //specify the background color, this is just an example
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //TODO: set transformation matrices
    //TODO: set planet and asteroid-ring rotation speed
    //TODO: set crafts locations
    //TODO: do texture mapping
    //TODO: do normal mapping
    //TODO: draw the elements
    
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = camera.GetViewMatrix();
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.5f, 100.0f);
    
    
    // Planet
    glBindVertexArray(vao[0]);
    
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.f), glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, currentTime * planetRotationSpeed, glm::vec3(0.0f, 0.0f, 1.0f));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, -1.05f, 0));
    
    nmShader.use();
    nmShader.setMat4("viewMatrix", viewMatrix);
    nmShader.setMat4("projectionMatrix", projectionMatrix);
    nmShader.setMat4("modelMatrix", modelMatrix);
    nmShader.setVec3("lightPos", envLightPos);
    nmShader.setVec3("viewPos", camera.Position);
    nmShader.setFloat("dirlightBrightness", envLightIntensity);
    
    planetTexture.bind(0);
    planetNormal.bind(1);
    nmShader.setInt("texColour", 0);
    nmShader.setInt("texNorm", 1);
    glDrawElements(GL_TRIANGLES, (GLsizei)planet.indices.size(), GL_UNSIGNED_INT, 0);
    planetTexture.unbind();
    planetNormal.unbind();
    
    
    shader.use();
    shader.setMat4("viewMatrix", viewMatrix);
    shader.setMat4("projectionMatrix", projectionMatrix);
    
    // Spacecraft
    glBindVertexArray(vao[1]);
    
    modelMatrix = glm::mat4(1.0f);
    glm::vec3 cameraPos = camera.Position - camera.Target;
    modelMatrix = glm::translate(modelMatrix, cameraPos);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, -1.0f, -1.4f));
    modelMatrix = glm::scale(modelMatrix, spacecraftScale);
    shader.setMat4("modelMatrix", modelMatrix);
    
    spacecraftTexture.bind(0);
    shader.setInt("tex1", 0);
    glDrawElements(GL_TRIANGLES, (GLsizei)spacecraft.indices.size(), GL_UNSIGNED_INT, 0);
    spacecraftTexture.unbind();
    
    
    // Astroids
    glBindVertexArray(vao[2]);
    
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::rotate(modelMatrix, currentTime * planetRotationSpeed, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 modelMatrixTemp;
    
    for (int i = 0; i < rockCount; i++) {
        modelMatrixTemp = modelMatrices[i];
        modelMatrixTemp = modelMatrix * modelMatrixTemp;
        shader.setMat4("modelMatrix", modelMatrixTemp);
        
        rockTexture.bind(0);
        shader.setInt("tex1", 0);
        glDrawElements(GL_TRIANGLES, (GLsizei)rock.indices.size(), GL_UNSIGNED_INT, 0);
        rockTexture.unbind();
    }
    
    
    // Ufo
    glBindVertexArray(vao[3]);
    
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(6.0f, 2.0f, -6.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f));
    shader.setMat4("modelMatrix", modelMatrix);
    
    ufoTexture.bind(0);
    shader.setInt("tex1", 0);
    glDrawElements(GL_TRIANGLES, (GLsizei)ufo.indices.size(), GL_UNSIGNED_INT, 0);
    ufoTexture.unbind();
    
    
    // Skybox
    glDepthFunc(GL_LEQUAL);
    skyboxShader.use();
    viewMatrix = glm::mat4(glm::mat3(viewMatrix));
    skyboxShader.setMat4("view", viewMatrix);
    skyboxShader.setMat4("projection", projectionMatrix);
    glBindVertexArray(vao_skybox);
    skyboxTexture.bind(0);
    glDrawArrays(GL_TRIANGLES, 0, E_skybox);
    glDepthFunc(GL_LESS);
    
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	//TODO: sets the mouse-button callback for the current window.	
    
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        mouseCtrl.LEFT_BUTTON = true;
        
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        
        mouseCtrl.MOUSE_Clickx = x;
        mouseCtrl.MOUSE_Clicky = y;
        mouseCtrl.MOUSE_X = x;
        mouseCtrl.MOUSE_Y = y;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        mouseCtrl.LEFT_BUTTON = false;
    }
}

void cursor_position_callback(GLFWwindow* window, double x, double y)
{
	//TODO: sets the cursor position callback for the current window
    
    if (mouseCtrl.LEFT_BUTTON) {
        float xoffset = x - mouseCtrl.MOUSE_X;
        float yoffset = y - mouseCtrl.MOUSE_Y;
        camera.ProcessMouseMovement_Left(xoffset, yoffset);
    }
    mouseCtrl.MOUSE_X = x;
    mouseCtrl.MOUSE_Y = y;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	//TODO: Sets the scoll callback for the current window.
    
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//TODO: Sets the Keyboard callback for the current window.
    
    // Key Press
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (key == GLFW_KEY_W && action == GLFW_PRESS)
        keyCtrl.W_KEY = true;
    if (key == GLFW_KEY_S && action == GLFW_PRESS)
        keyCtrl.S_KEY = true;
    if (key == GLFW_KEY_A && action == GLFW_PRESS)
        keyCtrl.A_KEY = true;
    if (key == GLFW_KEY_D && action == GLFW_PRESS)
        keyCtrl.D_KEY = true;
    
    
    // Key Release
    if (key == GLFW_KEY_W && action == GLFW_RELEASE)
        keyCtrl.W_KEY = false;
    if (key == GLFW_KEY_S && action == GLFW_RELEASE)
        keyCtrl.S_KEY = false;
    if (key == GLFW_KEY_A && action == GLFW_RELEASE)
        keyCtrl.A_KEY = false;
    if (key == GLFW_KEY_D && action == GLFW_RELEASE)
        keyCtrl.D_KEY = false;
}


int main(int argc, char* argv[])
{
	/* Initialize the glfw */
	if (!glfwInit()) {
		std::cout << "Failed to initialize GLFW" << std::endl;
		return -1;
	}
	/* glfw: configure; necessary for MAC */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	/* Create a windowed mode window and its OpenGL context */
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "HW3", NULL, NULL);
	if (!window) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	/*register callback functions*/
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);                                                                  
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

    if(GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    get_OpenGL_info();
	initializedGL();

	while (!glfwWindowShouldClose(window)) {
        //TODO: Get time information to make the planet, rocks and crafts moving across time
        //Hints: the function to get time -> float currentTIme = static_cast<float>(glfwGetTime());
        currentTime = static_cast<float>(glfwGetTime());
        camera.ProcessKeyPress();
        
		/* Render here */
		paintGL();

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}







#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"  // Image loading Utility functions
#include <glm/glm.hpp> // GLM Math Header inclusions
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <learnOpengl/camera.h> // Camera class
using namespace std; // Standard namespace
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source // Shader program Macro
#endif
namespace // Unnamed namespace
{
    const char* const WINDOW_TITLE = "7-1 Submit Your Project"; // Macro for window title
    const int WINDOW_WIDTH = 800; // Variables for window width and height
    const int WINDOW_HEIGHT = 600;
    struct GLMesh // Stores the GL data relative to a given mesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbo;         // Handle for the vertex buffer object
        GLuint nVertices;    // Number of indices of the mesh
    };
    GLFWwindow* gWindow = nullptr; // Main GLFW window
    GLMesh gMesh; // Triangle mesh data
    GLuint gTextureId; // Texture
    glm::vec2 gUVScale(5.0f, 5.0f);
    GLint gTexWrapMode = GL_REPEAT;
    GLuint gPyramidProgramId; // Shader programs
    GLuint gTableProgramId;
    GLuint gLampProgramId;
    Camera gCamera(glm::vec3(0.0f, 0.0f, 7.0f)); // Camera
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;
    float gDeltaTime = 0.0f; // Timing between current frame and last frame
    float gLastFrame = 0.0f;
    glm::vec3 gCubePosition(0.0f, 0.0f, 0.0f); // Subject position and scale
    glm::vec3 gCubeScale(2.0f);
    glm::vec3 gPyramidPosition(0.0f, 0.0f, 0.0f);
    glm::vec3 gPyramidScale(2.0f);
    glm::vec3 gObjectColor(1.0f, 0.0f, 1.0f); // Pyramid and light color
    glm::vec3 gKeyLightColor(0.8f, 0.2f, 0.8f);
    glm::vec3 gFillLightColor(0.5f, 0.5f, 0.5f);
    glm::vec3 gKeyLightPosition(1.5f, 0.5f, 3.0f); // Light position and scale
    glm::vec3 gFillLightPosition(-1.5f, 0.5f, -3.0f);
    glm::vec3 gLightScale(0.3f);
    bool gIsLampOrbiting = false; // Lamp animation
} // initialize the program, set the window size, redraw graphics on the window when resized, and render graphics on the screen
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
bool perspectiveProjection = true; // Variable to toggle between perspective and orthographic projections
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UCreateMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);

const GLchar* vertexShaderSource = GLSL(440, // Vertex Shader Source Code
    layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
layout(location = 1) in vec4 color;  // Color data from Vertex Attrib Pointer 1
out vec4 vertexColor; // variable to transfer color data to the fragment shader
uniform mat4 model;//Global variables for the  transform matrices
uniform mat4 view;
uniform mat4 projection;
    void main()
    {
        FragPos = vec3(model * vec4(aPos, 1.0)); // Calculate the vertex position in world space
        Normal = mat3(transpose(inverse(model))) * aNormal; // Transform the vertex normal to world space
        TexCoord = aTexCoord; // Pass the texture coordinates to the fragment shader
        gl_Position = projection * view * vec4(FragPos, 1.0); // Transform the vertex position to clip space
    }
    );
const GLchar* fragmentShaderSource = GLSL(440, // Fragment Shader Source Code
    in vec4 vertexColor; // Variable to hold incoming color data from vertex shader
out vec4 fragmentColor;
    void main()
    {
        float ambientStrength = 0.1; // Ambient lighting
        vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);
        vec3 lightDir = normalize(lightPos - FragPos); // Diffuse lighting
        float diff = max(dot(Normal, lightDir), 0.0);
        vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);
        float specularStrength = 0.5; // Specular lighting
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, Normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = specularStrength * spec * vec3(1.0, 1.0, 1.0);
        vec3 result = (ambient + diffuse + specular); // Combine ambient, diffuse, and specular lighting
        FragColor = texture(texture_diffuse1, TexCoord) * vec4(result, 1.0); // Output final color by combining with texture
    }
    );
const GLchar* cubeVertexShaderSource = GLSL(440, // Pyramid Vertex Shader Source Code
    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
layout(location = 1) in vec3 normal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;
out vec3 vertexNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;
uniform mat4 model; //Uniform / Global variables for the  transform matrices
uniform mat4 view;
uniform mat4 projection;
void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
    vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)
    vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
    vertexTextureCoordinate = textureCoordinate;
}
);
const GLchar* cubeFragmentShaderSource = GLSL(440, // Pyramid Fragment Shader Source Code
in vec3 vertexNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position
in vec2 vertexTextureCoordinate;
out vec4 fragmentColor; // For outgoing cube color to the GPU
// Uniform / Global variables for object color, light color, light position, and camera/view position
uniform vec3 objectColor;
uniform vec3 fillLightColor;
uniform vec3 fillLightPos;
uniform vec3 keyLightColor;
uniform vec3 keyLightPos;
uniform vec3 viewPosition;
uniform sampler2D uTexture; // Useful when working with multiple textures
uniform vec2 uvScale;
void main()
{   // Phong lighting model calculations to generate ambient, diffuse, and specular components
    // Calculate Key Lamp Ambient lighting
    float ambientStrength = 1.0f; // Set ambient or global lighting strength
    vec3 keyAmbient = ambientStrength * keyLightColor; // Generate ambient light color
    // Calculate Key Lamp Diffuse lighting
    vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
    vec3 lightDirection = normalize(keyLightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
    float impact = max(dot(norm, lightDirection), 0.1);// Calculate diffuse impact by generating dot product of normal and light
    vec3 keyDiffuse = impact * keyLightColor; // Generate diffuse light color
    // Calculate Key Lamp Specular lighting
    float specularIntensity = 5.0f; // Set specular light strength
    float highlightSize = 16.0f; // Set specular highlight size
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 reflectDir = reflect(-lightDirection, norm); // Calculate reflection vector
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize); //Calculate specular component
    vec3 keySpecular = specularIntensity * specularComponent * keyLightColor;
    //Calculate Fill Lamp Ambient lighting
    ambientStrength = 0.1f; // Set ambient or global lighting strength
    vec3 fillAmbient = ambientStrength * fillLightColor; // Generate ambient light color
    //Calculate Fill Lamp Diffuse lighting
    norm = normalize(vertexNormal); // Normalize vectors to 1 unit
    lightDirection = normalize(fillLightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
    impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
    vec3 fillDiffuse = impact * fillLightColor; // Generate diffuse light color
    // Calculate Fill Lamp Specular lighting
    specularIntensity = 0.1f; // Set specular light strength
    highlightSize = 16.0f; // Set specular highlight size
    viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    reflectDir = reflect(-lightDirection, norm); // Calculate reflection vector
    specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize); //Calculate specular component
    vec3 fillSpecular = specularIntensity * specularComponent * fillLightColor;
    vec3 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale).xyz; // Texture holds color for all three components
    vec3 fillResult = (fillAmbient + fillDiffuse + fillSpecular); // Calculate phong result
    vec3 keyResult = (keyAmbient + keyDiffuse + keySpecular);
    vec3 lightingResult = fillResult + keyResult;
    vec3 phong = lightingResult * textureColor;
    fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU
}
);
const GLchar* lampVertexShaderSource = GLSL(440, // Lamp Shader Source Code
    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
uniform mat4 model; //Uniform / Global variables for the  transform matrices
uniform mat4 view;
uniform mat4 projection;
void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
}
);
const GLchar* lampFragmentShaderSource = GLSL(440, // Fragment Shader Source Code
    out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU
void main()
{
    fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
}
);
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{ // Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;
        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}
int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;
    // Create the mesh
    UCreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object
    // Create the shader programs
    if (!UCreateShaderProgram(cubeVertexShaderSource, cubeFragmentShaderSource, gPyramidProgramId))
        return EXIT_FAILURE;
    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampProgramId))
        return EXIT_FAILURE;
    const char* texFilename = "../resources/textures/darkwood.jpg"; // Load texture
    if (!UCreateTexture(texFilename, gTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gPyramidProgramId);
    glUniform1i(glGetUniformLocation(gPyramidProgramId, "uTexture"), 0); // We set the texture as texture unit 0
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Sets the background color of the window to black (it will be implicitely used by glClear)
    while (!glfwWindowShouldClose(gWindow)) // render loop
    {   // per-frame timing
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;
        UProcessInput(gWindow); // input
        URender(); // Render this frame
        glfwPollEvents();
    }
    UDestroyMesh(gMesh); // Release mesh data
    UDestroyTexture(gTextureId); // Release texture
    UDestroyShaderProgram(gPyramidProgramId); // Release shader programs
    UDestroyShaderProgram(gLampProgramId); // Release shader programs
    exit(EXIT_SUCCESS); // Terminates the program successfully
}
bool UInitialize(int argc, char* argv[], GLFWwindow** window) // Initialize GLFW, GLEW, and create a window
{
    glfwInit(); // GLFW: initialize and configure
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL); // GLFW: window creation
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // tell GLFW to capture our mouse
    glewExperimental = GL_TRUE; // GLEW: initialize
    GLenum GlewInitResult = glewInit();
    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl; // Displays GPU OpenGL version
    return true;
}
void UProcessInput(GLFWwindow* window) // process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
{
    static const float cameraSpeed = 2.5f;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(UP, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) { // Toggle between perspective and orthographic projections
        perspectiveProjection = true;
        cout << "Switched to perspective projection" << endl;
    }
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
        perspectiveProjection = false;
        cout << "Switched to orthographic projection" << endl;
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && gTexWrapMode != GL_REPEAT)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);
        gTexWrapMode = GL_REPEAT;
        cout << "Current Texture Wrapping Mode: REPEAT" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && gTexWrapMode != GL_MIRRORED_REPEAT)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);
        gTexWrapMode = GL_MIRRORED_REPEAT;
        cout << "Current Texture Wrapping Mode: MIRRORED REPEAT" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && gTexWrapMode != GL_CLAMP_TO_EDGE)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
        gTexWrapMode = GL_CLAMP_TO_EDGE;
        cout << "Current Texture Wrapping Mode: CLAMP TO EDGE" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && gTexWrapMode != GL_CLAMP_TO_BORDER)
    {
        float color[] = { 1.0f, 0.0f, 0.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glBindTexture(GL_TEXTURE_2D, 0);
        gTexWrapMode = GL_CLAMP_TO_BORDER;
        cout << "Current Texture Wrapping Mode: CLAMP TO BORDER" << endl;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
    {
        gUVScale += 0.1f;
        cout << "Current scale (" << gUVScale[0] << ", " << gUVScale[1] << ")" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
    {
        gUVScale -= 0.1f;
        cout << "Current scale (" << gUVScale[0] << ", " << gUVScale[1] << ")" << endl;
    }
    static bool isLKeyDown = false; // Pause and resume lamp orbiting
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !gIsLampOrbiting)
        gIsLampOrbiting = true;
    else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && gIsLampOrbiting)
        gIsLampOrbiting = false;
} // glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
} // glfw: whenever the mouse moves, this callback is called
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }
    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top
    gLastX = xpos;
    gLastY = ypos;
    gCamera.ProcessMouseMovement(xoffset, yoffset);
} // glfw: whenever the mouse scroll wheel scrolls, this callback is called
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) // glfw: handle mouse button events
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}
void URender() // Functioned called to render a frame
{
    const float angularVelocity = glm::radians(45.0f); //Lamp orbits around the origin
    if (gIsLampOrbiting)
    {
        glm::vec4 newKeyPosition = glm::rotate(angularVelocity * gDeltaTime, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(gKeyLightPosition, 1.0f);
        gKeyLightPosition.x = newKeyPosition.x;
        gKeyLightPosition.y = newKeyPosition.y;
        gKeyLightPosition.z = newKeyPosition.z;
        glm::vec4 newFillPosition = glm::rotate(angularVelocity * gDeltaTime, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(gFillLightPosition, 1.0f);
        gFillLightPosition.x = newFillPosition.x;
        gFillLightPosition.y = newFillPosition.y;
        gFillLightPosition.z = newFillPosition.z;
    }
    glEnable(GL_DEPTH_TEST); // Enable z-depth
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Clear the frame and z buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(gMesh.vao); // Activate the cube VAO (used by cube and lamp)
    // CUBE, Set the shader to be used
    glUseProgram(gPyramidProgramId);
    glm::mat4 model = glm::translate(gPyramidPosition) * glm::scale(gPyramidScale); // Model matrix: transformations are applied right-to-left order
    glm::mat4 view = gCamera.GetViewMatrix(); // camera/view transformation
    glm::mat4 projection;// Creates either a perspective or orthographic projection based on the toggle
    if (perspectiveProjection) { // Creates a perspective projection
        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
    }
    else { // Creates an orthographic projection
        projection = glm::ortho(-3.0f, 3.0f, -3.0f, 3.0f, 0.1f, 100.0f);
    } // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gPyramidProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gPyramidProgramId, "view");
    GLint projLoc = glGetUniformLocation(gPyramidProgramId, "projection");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    // Reference matrix uniforms from the Cube Shader program for the cub color, light color, light position, and camera position
    GLint objectColorLoc = glGetUniformLocation(gPyramidProgramId, "objectColor");
    GLint keyLightColorLoc = glGetUniformLocation(gPyramidProgramId, "keyLightColor");
    GLint keyLightPositionLoc = glGetUniformLocation(gPyramidProgramId, "keyLightPos");
    GLint fillLightColorLoc = glGetUniformLocation(gPyramidProgramId, "fillLightColor");
    GLint fillLightPositionLoc = glGetUniformLocation(gPyramidProgramId, "fillLightPos");
    GLint viewPositionLoc = glGetUniformLocation(gPyramidProgramId, "viewPosition");
    GLint uvScaleLoc = glGetUniformLocation(gPyramidProgramId, "uvScale");
    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    glUniform3fv(objectColorLoc, 1, glm::value_ptr(gObjectColor));
    glUniform3fv(fillLightColorLoc, 1, glm::value_ptr(gFillLightColor));
    glUniform3fv(fillLightPositionLoc, 1, glm::value_ptr(gFillLightPosition));
    glUniform3fv(keyLightColorLoc, 1, glm::value_ptr(gKeyLightColor));
    glUniform3fv(keyLightPositionLoc, 1, glm::value_ptr(gKeyLightPosition));
    glUniform3fv(viewPositionLoc, 1, glm::value_ptr(gCamera.Position));
    glUniform2fv(uvScaleLoc, 1, glm::value_ptr(gUVScale));
    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(fillLightColorLoc, gFillLightColor.r, gFillLightColor.g, gFillLightColor.b);
    glUniform3f(fillLightPositionLoc, gFillLightPosition.x, gFillLightPosition.y, gFillLightPosition.z);
    glUniform3f(keyLightColorLoc, gKeyLightColor.r, gKeyLightColor.g, gKeyLightColor.b);
    glUniform3f(keyLightPositionLoc, gKeyLightPosition.x, gKeyLightPosition.y, gKeyLightPosition.z);
    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    GLint UVScaleLoc = glGetUniformLocation(gPyramidProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));
    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId);
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices); // Draws the triangles
    // KEY LAMP: draw lamp
    glUseProgram(gLampProgramId);
    model = glm::translate(gKeyLightPosition) * glm::scale(gLightScale); //Transform the smaller cube used as a visual que for the light source
    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");
    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);
    glUseProgram(gLampProgramId); // Fill Lamp
    model = glm::translate(gFillLightPosition) * glm::scale(gLightScale); //Transform the smaller cube used as a visual que for the light source
    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");
    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);
    // Deactivate the Vertex Array Object and shader program
    glBindVertexArray(0);
    glUseProgram(0);
    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}
void UCreateMesh(GLMesh& mesh) // Implements the UCreateMesh function
{
    GLfloat verts[] = { // Position and Texture data
        //Positions          //Normals                 //Textures
        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f, //Leg One Left Face
        -0.3f, -0.5f, -0.5f,    0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
        -0.3f,  0.0f, -0.5f,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        -0.3f,  0.0f, -0.5f,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        -0.5f,  0.0f, -0.5f,    0.0f, 0.0f, 1.0f,    0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f,

        -0.5f, -0.5f, -0.3f,    0.0f, 0.0f, 1.0f,    0.0f, 1.0f,// Right Face
        -0.3f, -0.5f, -0.3f,    0.0f, 0.0f, 1.0f,    0.0f, 1.0f,
        -0.3f,  0.0f, -0.3f,    0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
        -0.3f,  0.0f, -0.3f,    0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
        -0.5f,  0.0f, -0.3f,    0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
        -0.5f, -0.5f, -0.3f,    0.0f, 0.0f, 1.0f,    0.0f, 1.0f,

        -0.5f,  0.0f, -0.3f,    0.0f, 0.0f, 1.0f,    0.0f, 1.0f,// Front Face
        -0.5f,  0.0f, -0.5f,    0.0f, 0.0f, 1.0f,    0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
        -0.5f, -0.5f, -0.3f,    0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
        -0.5f,  0.0f, -0.3f,    0.0f, 0.0f, 1.0f,    0.0f, 1.0f,

        -0.3f,  0.0f, -0.3f,    0.0f, 0.0f, 1.0f,    0.0f, 1.0f,// Back Face
        -0.3f,  0.0f, -0.5f,    0.0f, 0.0f, 1.0f,    0.0f, 1.0f,
        -0.3f, -0.5f, -0.5f,    0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
        -0.3f, -0.5f, -0.5f,    0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
        -0.3f, -0.5f, -0.3f,    0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
        -0.3f,  0.0f, -0.3f,    0.0f, 0.0f, 1.0f,    0.0f, 1.0f,

        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f,// Bottom Face
        -0.3f, -0.5f, -0.5f,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        -0.3f, -0.5f, -0.3f,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        -0.3f, -0.5f, -0.3f,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        -0.5f, -0.5f, -0.3f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f,
        // Leg Two 
        0.0f, -0.5f, -0.5f,     0.0f, 0.0f, 1.0f,    0.0f, 0.0f,// Left Face
        0.2f, -0.5f, -0.5f,     0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
        0.2f,  0.0f, -0.5f,     0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        0.2f,  0.0f, -0.5f,     0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        0.0f,  0.0f, -0.5f,     0.0f, 0.0f, 1.0f,    0.0f, 1.0f,
        0.0f, -0.5f, -0.5f,     0.0f, 0.0f, 1.0f,    0.0f, 0.0f,

        0.0f, -0.5f, -0.3f,     0.0f, 0.0f, 1.0f,    0.0f, 0.0f,// Right Face
        0.2f, -0.5f, -0.3f,     0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
        0.2f,  0.0f, -0.3f,     0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        0.2f,  0.0f, -0.3f,     0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        0.0f,  0.0f, -0.3f,     0.0f, 0.0f, 1.0f,    0.0f, 1.0f,
        0.0f, -0.5f, -0.3f,     0.0f, 0.0f, 1.0f,    0.0f, 0.0f,

        0.0f,  0.0f, -0.3f,     0.0f, 0.0f, 1.0f,    0.0f, 1.0f,// Front Face
        0.0f,  0.0f, -0.5f,     0.0f, 0.0f, 1.0f,    0.0f, 1.0f,
        0.0f, -0.5f, -0.5f,     0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
        0.0f, -0.5f, -0.5f,     0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
        0.0f, -0.5f, -0.3f,     0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
        0.0f,  0.0f, -0.3f,     0.0f, 0.0f, 1.0f,    0.0f, 1.0f,

        0.2f,  0.0f, -0.3f,     0.0f, 0.0f, 1.0f,    1.0f, 1.0f,// Back Face
        0.2f,  0.0f, -0.5f,     0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        0.2f, -0.5f, -0.5f,     0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
        0.2f, -0.5f, -0.5f,     0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
        0.2f, -0.5f, -0.3f,     0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
        0.2f,  0.0f, -0.3f,     0.0f, 0.0f, 1.0f,    1.0f, 1.0f,

        0.0f, -0.5f, -0.5f,     0.0f, 0.0f, 1.0f,    0.0f, 0.0f,// Bottom Face
        0.2f, -0.5f, -0.5f,     0.0f, 0.0f, 1.0f,    0.0f, 0.0f,
        0.2f, -0.5f, -0.3f,     0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        0.2f, -0.5f, -0.3f,     0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        0.0f, -0.5f, -0.3f,     0.0f, 0.0f, 1.0f,    0.0f, 0.0f,
        0.0f, -0.5f, -0.5f,     0.0f, 0.0f, 1.0f,    0.0f, 0.0f,
        // Leg Three
        -0.5f, -0.5f, -0.1f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f, // Left Face
        -0.3f, -0.5f, -0.1f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f,
        -0.3f,  0.0f, -0.1f,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        -0.3f,  0.0f, -0.1f,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        -0.5f,  0.0f, -0.1f,    0.0f, 0.0f, 1.0f,    0.0f, 1.0f,
        -0.5f, -0.5f, -0.1f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f,

        -0.5f, -0.5f, 0.1f,     0.0f, 0.0f, 1.0f,    0.0f, 0.0f,// Right Face
        -0.3f, -0.5f, 0.1f,     0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
        -0.3f,  0.0f, 0.1f,     0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        -0.3f,  0.0f, 0.1f,     0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        -0.5f,  0.0f, 0.1f,     0.0f, 0.0f, 1.0f,    0.0f, 1.0f,
        -0.5f, -0.5f, 0.1f,     0.0f, 0.0f, 1.0f,    0.0f, 0.0f,

        -0.5f,  0.0f, 0.1f,     0.0f, 0.0f, 1.0f,    0.0f, 1.0f,// Front Face
        -0.5f,  0.0f, -0.1f,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        -0.5f, -0.5f, -0.1f,    0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
        -0.5f, -0.5f, -0.1f,    0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
        -0.5f, -0.5f, 0.1f,     0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
        -0.5f,  0.0f, 0.1f,     0.0f, 0.0f, 1.0f,    0.0f, 1.0f,

        -0.3f,  0.0f, 0.1f,     0.0f, 0.0f, 1.0f,    0.0f, 0.0f, // Back Face
        -0.3f,  0.0f, -0.1f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f,
        -0.3f, -0.5f, -0.1f,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        -0.3f, -0.5f, -0.1f,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        -0.3f, -0.5f, 0.1f,     0.0f, 0.0f, 1.0f,    0.0f, 1.0f,
        -0.3f,  0.0f, 0.1f,     0.0f, 0.0f, 1.0f,    0.0f, 0.0f,

        -0.5f, -0.5f, -0.1f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f, // Bottom Face
        -0.3f, -0.5f, -0.1f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f,
        -0.3f, -0.5f, 0.1f,     0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        -0.3f, -0.5f, 0.1f,     0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        -0.5f, -0.5f, 0.1f,     0.0f, 0.0f, 1.0f,    0.0f, 0.0f,
        -0.5f, -0.5f, -0.1f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f,
        // Leg Four
        0.0f, -0.5f, -0.1f,     0.0f, 0.0f, 1.0f,    0.0f, 0.0f, // Left Face
        0.2f, -0.5f, -0.1f,     0.0f, 0.0f, 1.0f,    0.0f, 0.0f,
        0.2f,  0.0f, -0.1f,     0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        0.2f,  0.0f, -0.1f,     0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        0.0f,  0.0f, -0.1f,     0.0f, 0.0f, 1.0f,    0.0f, 1.0f,
        0.0f, -0.5f, -0.1f,     0.0f, 0.0f, 1.0f,    0.0f, 0.0f,

        0.0f, -0.5f, 0.1f,      0.0f, 0.0f, 1.0f,    0.0f, 0.0f,  // Right Face
        0.2f, -0.5f, 0.1f,      0.0f, 0.0f, 1.0f,    0.0f, 0.0f,
        0.2f,  0.0f, 0.1f,      0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        0.2f,  0.0f, 0.1f,      0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        0.0f,  0.0f, 0.1f,      0.0f, 0.0f, 1.0f,    0.0f, 1.0f,
        0.0f, -0.5f, 0.1f,      0.0f, 0.0f, 1.0f,    0.0f, 0.0f,

        0.0f,  0.0f,  0.1f,     0.0f, 0.0f, 1.0f,    0.0f, 1.0f, // Front Face
        0.0f,  0.0f, -0.1f,     0.0f, 0.0f, 1.0f,    0.0f, 1.0f,
        0.0f, -0.5f, -0.1f,     0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        0.0f, -0.5f, -0.1f,     0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        0.0f, -0.5f,  0.1f,     0.0f, 0.0f, 1.0f,    0.0f, 0.0f,
        0.0f,  0.0f,  0.1f,     0.0f, 0.0f, 1.0f,    0.0f, 1.0f,

        0.2f,  0.0f,  0.1f,     0.0f, 0.0f, 1.0f,    0.0f, 1.0f,// Back Face
        0.2f,  0.0f, -0.1f,     0.0f, 0.0f, 1.0f,    0.0f, 1.0f,
        0.2f, -0.5f, -0.1f,     0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
        0.2f, -0.5f, -0.1f,     0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
        0.2f, -0.5f,  0.1f,     0.0f, 0.0f, 1.0f,    0.0f, 0.0f,
        0.2f,  0.0f,  0.1f,     0.0f, 0.0f, 1.0f,    0.0f, 1.0f,

        0.0f, -0.5f, -0.1f,     0.0f, 0.0f, 1.0f,    0.0f, 0.0f,// Bottom Face
        0.2f, -0.5f, -0.1f,     0.0f, 0.0f, 1.0f,    0.0f, 0.0f,
        0.2f, -0.5f,  0.1f,     0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        0.2f, -0.5f,  0.1f,     0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        0.0f, -0.5f,  0.1f,     0.0f, 0.0f, 1.0f,    0.0f, 0.0f,
        0.0f, -0.5f, -0.1f,     0.0f, 0.0f, 1.0f,    0.0f, 0.0f,
        // Table Top
        -0.5f, 0.0f, -0.5f,     0.0f, 1.0f, 0.0f,    0.0f, 1.0f,
         0.2f, 0.0f, -0.5f,     0.0f, 1.0f, 0.0f,    1.0f, 0.0f,
         0.2f, 0.0f,  0.1f,     0.0f, 1.0f, 0.0f,    1.0f, 1.0f,
         0.2f, 0.0f,  0.1f,     0.0f, 1.0f, 0.0f,    1.0f, 1.0f,
        -0.5f, 0.0f,  0.1f,     0.0f, 1.0f, 0.0f,    0.0f, 1.0f,
        -0.5f, 0.0f, -0.5f,     0.0f, 1.0f, 0.0f,    0.0f, 1.0f,
    };
    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;
    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);
    glGenBuffers(1, &mesh.vbo); // Create 1 buffer for the combined vertex, normal, and texture coordinate data
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex data to the GPU
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV); // Strides between vertex coordinates, normals, and texture coordinates
    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0); // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex)); // Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (floatsPerVertex + floatsPerNormal))); // Texture Coordinates
    glEnableVertexAttribArray(2);
}
void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
}
bool UCreateTexture(const char* filename, GLuint& textureId) // Generate and load the texture
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture
        return true;
    }
    return false; // Error loading the image
}
void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{ // Implements the UCreateShaders function
    int success = 0; // Compilation and linkage error reporting
    char infoLog[512];
    programId = glCreateProgram(); // Create a Shader program object.
    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);
    glCompileShader(vertexShaderId); // compile the vertex shader
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success); // check for shader compile errors
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        return false;
    }
    glCompileShader(fragmentShaderId); // compile the fragment shader
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success); // check for shader compile errors
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        return false;
    } // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
    glLinkProgram(programId);   // links the shader program
    glGetProgramiv(programId, GL_LINK_STATUS, &success); // check for linking errors
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        return false;
    }
    glUseProgram(programId); // Uses the shader program
    return true;
}
void UDestroyShaderProgram(GLuint programId) // Destroy Shader
{
    glDeleteProgram(programId);
}

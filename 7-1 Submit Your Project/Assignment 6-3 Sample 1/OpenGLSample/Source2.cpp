#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "Beginning a 3D scene"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbos[2];     // Handles for the vertex buffer objects
        GLuint nIndices;    // Number of indices of the mesh
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Triangle mesh data
    GLMesh gMesh;
    // Shader program
    GLuint gProgramId;
}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UCreateMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
layout(location = 1) in vec4 color;  // Color data from Vertex Attrib Pointer 1

out vec4 vertexColor; // variable to transfer color data to the fragment shader

//Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
    vertexColor = color; // references incoming color data
}
);


/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,
    in vec4 vertexColor; // Variable to hold incoming color data from vertex shader

out vec4 fragmentColor;

void main()
{
    fragmentColor = vec4(vertexColor);
}
);


int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the mesh
    UCreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object

    // Create the shader program
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        // input
        // -----
        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(gMesh);

    // Release shader program
    UDestroyShaderProgram(gProgramId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


// Functioned called to render a frame
void URender()
{
    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. Scales the object by 2
    glm::mat4 scale = glm::scale(glm::vec3(3.0f, 3.0f, 3.0f));
    // 2. Rotates the shape by 15 degrees in the x axis
    glm::mat4 rotation = glm::rotate(13.0f, glm::vec3(1.0, 0.5f, 0.0f));
    // 3. Places object at the origin
    glm::mat4 translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
    // Model matrix: Transformations are applied right-to-left.
    glm::mat4 model = translation * rotation * scale;

    // Transforms the camera: move the camera back (z axis)
    glm::mat4 view = glm::translate(glm::vec3(0.0f, 0.0f, -3.0f));

    // Creates a orthographic projection
    glm::mat4 projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);

    // Set the shader to be used
    glUseProgram(gProgramId);

    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, gMesh.nIndices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}


// Implements the UCreateMesh function
void UCreateMesh(GLMesh& mesh)
{
    // Position and Color data
    GLfloat verts[] = {
        // Vertex Positions    // Colors (r,g,b,a)
         0.9f,  0.0f,  0.0f,   1.0f, 0.0f, 0.0f, 1.0f, // 0 Table Top
         0.9f, -0.1f,  0.0f,   0.0f, 1.0f, 0.0f, 1.0f, // 1
         0.5f,  0.0f, -0.5f,   0.0f, 0.0f, 1.0f, 1.0f, // 2
         0.5f, -0.1f, -0.5f,   1.0f, 0.0f, 1.0f, 1.0f, // 3
        -0.9f,  0.0f,  0.0f,   0.5f, 0.5f, 1.0f, 1.0f, // 4
        -0.9f, -0.1f,  0.0f,   1.0f, 1.0f, 0.5f, 1.0f, // 5 
        -0.5f,  0.0f,  0.5f,   0.2f, 0.2f, 0.5f, 1.0f, // 6 
        -0.5f, -0.1f,  0.5f,   1.0f, 0.0f, 1.0f, 1.0f, // 7

        0.85f, -0.1f, -0.1f,   1.0f, 0.0f, 1.0f, 1.0f, // 8 Leg 1
        0.8f,  -0.1f,  0.0f,   1.0f, 0.0f, 1.0f, 1.0f, // 9
        0.85f, -0.1f,  0.1f,   1.0f, 0.0f, 1.0f, 1.0f, // 10

        0.9f,  -0.9f,  0.0f,   0.0f, 1.0f, 0.0f, 1.0f, // 11
        0.85f, -0.9f, -0.1f,   1.0f, 0.0f, 1.0f, 1.0f, // 12
        0.8f,  -0.9f,  0.0f,   1.0f, 0.0f, 1.0f, 1.0f, // 13
        0.85f, -0.9f,  0.1f,   1.0f, 0.0f, 1.0f, 1.0f, // 14

        0.45f, -0.1f, -0.45f,  0.0f, 0.0f, 1.0f, 1.0f, // 15 Leg 2
        0.5f,  -0.1f, -0.4f,   1.0f, 0.0f, 1.0f, 1.0f, // 16
        0.55f, -0.1f, -0.45f,  1.0f, 0.0f, 1.0f, 1.0f, // 17

        0.5f,  -0.9f,  0.0f,   1.0f, 0.0f, 1.0f, 1.0f, // 18
        0.45f, -0.9f, -0.1f,   1.0f, 0.0f, 1.0f, 1.0f, // 19
        0.5f,  -0.9f,  0.0f,   1.0f, 0.0f, 1.0f, 1.0f, // 20
        0.55f, -0.9f, -0.1f,   1.0f, 0.0f, 1.0f, 1.0f, // 21

       -0.85f, -0.1f,  0.1f,   0.0f, 0.0f, 1.0f, 1.0f, // 22 Leg 3
       -0.8f,  -0.1f, -0.0f,   1.0f, 0.0f, 1.0f, 1.0f, // 23
       -0.85f, -0.1f, -0.1f,   1.0f, 0.0f, 1.0f, 1.0f, // 24

       -0.9f,  -0.9f,  0.0f,   1.0f, 1.0f, 0.5f, 1.0f, // 25 
       -0.85f, -0.9f, -0.1f,   1.0f, 0.0f, 1.0f, 1.0f, // 26
       -0.8f,  -0.9f,  0.0f,   1.0f, 0.0f, 1.0f, 1.0f, // 27
       -0.85f, -0.9f,  0.1f,   1.0f, 0.0f, 1.0f, 1.0f, // 28    

       -0.55f, -0.1f,  0.45f,  0.0f, 0.0f, 1.0f, 1.0f, // 29 Leg 4
       -0.5f,  -0.1f,  0.4f,   1.0f, 0.0f, 1.0f, 1.0f, // 30
       -0.45f, -0.1f,  0.45f,  1.0f, 0.0f, 1.0f, 1.0f, // 31

       -0.5f,  -0.9f,  0.5f,   1.0f, 0.0f, 1.0f, 1.0f, // 32
       -0.55f, -0.9f,  0.45f,  0.0f, 0.0f, 1.0f, 1.0f, // 33 
       -0.5f,  -0.9f,  0.4f,   1.0f, 0.0f, 1.0f, 1.0f, // 34
       -0.45f, -0.9f,  0.45f,  1.0f, 0.0f, 1.0f, 1.0f, // 35
    };

    // Index data to share position data
    GLushort indices[] = {
        0, 1, 2,    //Table top
        0, 2, 3,   
        2, 3, 5,   
        2, 4, 5,   
        4, 5, 6,   
        5, 6, 7,   
        6, 7, 0,   
        0, 1, 7,   
        0, 2, 6,   
        2, 4, 6,   
        1, 2, 7,   
        2, 5, 7,    

        1, 8, 9,    //Leg 1
        1, 9, 10,
        1, 8, 11,
        8, 11, 12,
        8, 9, 12,
        9, 12, 13,
        9, 10, 13,
        10, 13, 14,
        10, 1, 14,
        1, 14, 11,
        11, 12, 14,
        12, 13, 14,

        3, 15, 16,   //Leg 2
        3, 16, 17,
        3, 15, 18,
        15, 18, 19,
        15, 16, 19,
        16, 19, 20,
        16, 17, 20,
        17, 20, 21,
        17, 3, 21,
        3, 21, 18,
        18, 19, 20,
        18, 20, 21,

        5, 22, 23,   //Leg 3
        5, 23, 24,
        5, 22, 25,
        22, 25, 26,
        22, 23, 26,
        23, 26, 27,
        23, 24, 27,
        24, 27, 28,
        24, 5, 28,
        5, 28, 25,
        25, 26, 27,
        25, 27, 28,

        7, 29, 30,   //Leg 4
        7, 30, 31,
        7, 29, 32,
        29, 32, 33,
        29, 30, 33,
        30, 33, 34,
        30, 31, 34,
        31, 34, 35,
        31, 7, 35,
        7, 35, 32,
        32, 33, 34,
        32, 34, 35

    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
}


void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(2, mesh.vbos);
}


// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}


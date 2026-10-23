/*
Completed By:   Alexander Mundt 101632886
Class:          GAME2012
Professor:      Connor Smiley
Assignment:     Assignment 2
*/
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <raymath.h>
#include <cassert>
#include <iostream>
#include <vector>
#include <random>
#include "Shader.h"

void APIENTRY DebugCallback(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

struct App
{
    int object_index;
};

//A line can hold 2 Vector3s of data to represent the beginning and end of a line
struct Line
{
    Vector3 begin;
    Vector3 end;

    Line()
    {
        begin = Vector3Zero();
        end = Vector3Zero();
    }

    Line(Vector3 b, Vector3 e)
    {
        begin = b;
        end = e;
    }
};

int main(void)
{
    App app{};

    assert(glfwInit() == GLFW_TRUE);

    // Request certain OpenGL features
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef NDEBUG
    // Don't make a debug context if in release mode
#else
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(1600, 900, "GAME-2012 Assignment 2 - Alexander Mundt", nullptr, nullptr);
    assert(window != nullptr);

    glfwSetWindowUserPointer(window, &app);
    glfwSetKeyCallback(window, KeyCallback);

    glfwMakeContextCurrent(window);
    assert(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == true);

#ifdef NDEBUG
#else
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(DebugCallback, nullptr);
#endif
    //Constants
    constexpr int VERTICES_PER_SQUARE = 8;
    constexpr int LINES_PER_SQUARE = 4;
    constexpr int SQUARE_COUNT = 12;
    constexpr int TOTAL_VERTEX_COUNT = SQUARE_COUNT * VERTICES_PER_SQUARE;

    //Instead of storing raw individual vertices, just store them as lines
    std::vector<Line> line_positions =
    {
        //First square of data
        Line({ -1.0f, -1.0f, 0.0f }, {  1.0f, -1.0f, 0.0f }), //bottom left, bottom right
        Line({  1.0f, -1.0f, 0.0f }, {  1.0f,  1.0f, 0.0f }), //bottom right, top right
        Line({  1.0f,  1.0f, 0.0f }, { -1.0f,  1.0f, 0.0f }), //top right, top left
        Line({ -1.0f,  1.0f, 0.0f }, { -1.0f, -1.0f, 0.0f }), //top left, bottom left
    };

    //Generate the rest of the lines
    //Start at the second square because the first one is already present above
    std::size_t squareOffset;
    std::vector<Vector3> midpoints;
    for (std::size_t s = 1; s < SQUARE_COUNT; s++)
    {
        //Calculate how far into the buffer we should be looking for our next line
        squareOffset = (s - 1) * LINES_PER_SQUARE;

        //Clean slate
        midpoints.clear();

        //For each line we find the midpoint of it
        for (std::size_t l = 0; l < LINES_PER_SQUARE; l++)
        {
            //Get the line to find the midpoint of
            Line line = line_positions[squareOffset + l];

            //Add that line's midpoint to the list of midpoints
            midpoints.push_back(Vector3Lerp(line.begin, line.end, 0.5f));
        }

        //Make the 4 lines based on the midpoints calculated above
        line_positions.push_back(Line(midpoints[0], midpoints[1]));
        line_positions.push_back(Line(midpoints[1], midpoints[2]));
        line_positions.push_back(Line(midpoints[2], midpoints[3]));
        line_positions.push_back(Line(midpoints[3], midpoints[0]));
    }

    //Line vertex colors
    std::vector<Vector3> line_colors;

    //For random colors
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> rgbVal(0, 255);

    //This will add as many vec3 color data elements as we have vertices in the positions buffer
    //SQUARE_COUNT * VERTICES_PER_SQUARE elements
    for (int s = 0; s < SQUARE_COUNT; s++)
    {
        float r = rgbVal(gen) / 255.0f;
        float g = rgbVal(gen) / 255.0f;
        float b = rgbVal(gen) / 255.0f;

        for (int v = 0; v < VERTICES_PER_SQUARE; v++)
        {
            line_colors.push_back(Vector3(r, g, b));
        }
    }

    //Line VAO
    GLuint line_vao = GL_NONE;
    glGenVertexArrays(1, &line_vao);
    glBindVertexArray(line_vao);

    //Line position VBO for line VAO
    GLuint line_positions_vbo = GL_NONE;
    glGenBuffers(1, &line_positions_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, line_positions_vbo);      // The following array (vertex) buffer will be described (line_positions_vbo):
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);// Index 0, 3 components, float data, no implicit normalization, no stride, no offset
    glBufferData(GL_ARRAY_BUFFER, TOTAL_VERTEX_COUNT * sizeof(Vector3), line_positions.data(), GL_STATIC_DRAW); // Transfer memory of line_positions into currently bound vbo
    glEnableVertexAttribArray(0);

    //Line color VBO for line VAO
    GLuint line_colors_vbo = GL_NONE;
    glGenBuffers(1, &line_colors_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, line_colors_vbo);         // The following array (vertex) buffer will be described (line_colors_vbo):
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);// Index 1, 3 components, float data, no implicit normalization, no stride, no offset
    glBufferData(GL_ARRAY_BUFFER, TOTAL_VERTEX_COUNT * sizeof(Vector3), line_colors.data(), GL_STATIC_DRAW); // Transfer memory of line_colors into currently bound vbo
    glEnableVertexAttribArray(1);

    glBindVertexArray(GL_NONE);                                 // "Stop recording vertex buffer state"

    GLuint vs_shader = LoadShader(GL_VERTEX_SHADER, "./assets/shaders/a2_lines.vert");
    GLuint fs_shader = LoadShader(GL_FRAGMENT_SHADER, "./assets/shaders/a2_lines.frag");
    GLuint shader_program = LoadProgram(vs_shader, fs_shader);
    
    Matrix world = MatrixIdentity();
    Matrix view = MatrixLookAt({ 0.0f, 0.0f, 10.0f }, { 0.0f, 0.0f, 0.0f }, Vector3UnitY);
    Matrix proj = MatrixOrtho(-1.05f, 1.05f, -1.05f, 1.05f, 0.1f, 100.0f);
    Matrix mvp = world * view * proj;

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);
        glBindVertexArray(line_vao);

        GLint u_mvp = glGetUniformLocation(shader_program, "u_mvp");
        glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(mvp));

        glLineWidth(2.0f);
        glDrawArrays(GL_LINES, 0, TOTAL_VERTEX_COUNT);

        glPointSize(10.0f);
        glDrawArrays(GL_POINTS, 0, TOTAL_VERTEX_COUNT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    App* app = (App*)glfwGetWindowUserPointer(window);

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
    {
        // Bound our array to be [0..4]
        ++app->object_index %= 5;
    }
}

void APIENTRY DebugCallback(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}
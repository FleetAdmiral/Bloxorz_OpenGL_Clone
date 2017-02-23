#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <irrKlang.h>
using namespace irrklang;
using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;
/* Variables declared here */
std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
std::vector< glm::vec3 > temp_vertices;
std::vector< glm::vec2 > temp_uvs;
std::vector< glm::vec3 > temp_normals;
GLuint programID;
FILE * file;
float blockx,blocky,blockz,posi,posback,posbackeep;
int xzflag = 0;
glm::mat4 rotateblocktemp = glm::mat4(1.0f); // glTranslatef
glm::mat4 left = glm::mat4(-10.0f);
glm::mat4 right = glm::mat4(10.0f);
glm::mat4 top = glm::mat4(10.0f);
glm::mat4 bottom = glm::mat4(-10.0f);
int triang_left = 0, triang_right = 0, triang_up = 0, triang_down = 0, turnstat=0,score=0;
int stageup=0;
float xstage[52] = {-3,-2,-1,0,0,0,0,0,1,1,1,1,2,2,2,2,2,2,2,3,3,3,3,3,3,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,6,6,6,6,6,7,7,7,7,7,5,3};
float zstage[52] = {0,0,0,-1,0,0,1,2,0,0,1,2,0,1,2,4,5,6,7,1,2,4,-3,6,7,1,4,5,6,7,-1,0,0,1,2,4,5,6,7,0,0,0,1,2,0,0,0,1,2,3,-2,3};
/* Function to load Shaders - Use it as it is */
int camera=0;
ISoundEngine* engine;
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 block
}

/**************************
 * Customizable functions *
 **************************/

float block_rot_dir = 1;
float rectangle_rot_dir = 1;
bool block_rot_status = true;
bool rectangle_rot_status = true;

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_C:
                rectangle_rot_status = !rectangle_rot_status;
                break;
            case GLFW_KEY_P:
                block_rot_status = !block_rot_status;
                break;
            case GLFW_KEY_X:
                // do something ..
                break;
            default:
                break;
        }
    }

    else if (action == GLFW_PRESS) {
      if(turnstat==0)
      {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            case GLFW_KEY_LEFT:
            engine->play2D("metal_clang.wav");
                triang_left = 1;
                break;
            case GLFW_KEY_RIGHT:
            engine->play2D("metal_clang.wav");
                triang_right = 1;
                break;
            case GLFW_KEY_UP:
            engine->play2D("metal_clang.wav");
                triang_up = 1;
                break;
           case GLFW_KEY_DOWN:
           engine->play2D("metal_clang.wav");
                triang_down = 1;
                break;
            case GLFW_KEY_X:
                camera = (camera+1)%4;
                break;
            default:
                break;
        }
      }
      if(turnstat==1)
      {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            case GLFW_KEY_LEFT:
            engine->play2D("metal_clang.wav");
                triang_up = 1;
                break;
            case GLFW_KEY_RIGHT:
            engine->play2D("metal_clang.wav");
                triang_down = 1;
                break;
            case GLFW_KEY_UP:
            engine->play2D("metal_clang.wav");
                triang_right = 1;
                break;
           case GLFW_KEY_DOWN:
           engine->play2D("metal_clang.wav");
                triang_left = 1;
                break;
            case GLFW_KEY_X:
                camera = (camera+1)%4;
                break;
            default:
                break;
        }
      }
        else if(turnstat==1)
        {

        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE)
                block_rot_dir *= -1;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
                rectangle_rot_dir *= -1;
            }
            break;
        default:
            break;
    }
}

void changePerspective()
{
  if(camera==2 || camera==3)
  {
    // cout<<"META META EI\n";
    GLfloat fov = 90.0f;
    Matrices.projection = glm::perspective (fov, 1.0f, 0.1f, 500.0f);
  }
  // Ortho projection for 2D views
  else
  Matrices.projection = glm::ortho(-8.0f, 8.0f, -8.0f, 8.0f, 0.1f, 500.0f);
}

/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);


	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);
  glEnable(GL_MULTISAMPLE);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    changePerspective();
}


VAO *block,  *rectangle, *stage, *block2, *block3, *stagewhite;

// Creates the block object used in this sample code
void createStage()
{
  static const GLfloat vertex_buffer_data [] = {
  /*  -0.5, 0.2, -0.5,
    0.5,0.2, -0.5,
    -0.5, -0.2, 0.5,

    0.5,0.2, -0.5,
    -0.5, -0.2, 0.5,
    0.5, 0.2, 0.5,

    -0.5,0.2,-0.5,
    -0.5,-0.2,-0.5,
    -0.5,0.2,0.5,

    -0.5,-0.2,-0.5,
    -0.5,0.2,0.5,
    -0.5,-0.2,0.5,

    0.5,0.2,-0.5,
    0.5,-0.2,-0.5,
    0.5,0.2,0.5,

    0.5,-0.2,-0.5,
    0.5,0.2,0.5,
    0.5,-0.2,0.5,

    -0.5,0.2,0.5,
    -0.5,-0.2,0.5,
    0.5,-0.2,0.5,

      -0.5,0.2,0.5,
      0.5,-0.2,0.5,
      0.5,0.2,0.5,

      -0.5,0.2,-0.5,
      -0.5,-0.2,-0.5,
      0.5,-0.2,-0.5,

        -0.5,0.2,-0.5,
        0.5,-0.2,-0.5,
        0.5,0.2,-0.5,

        -0.5,-0.2,-0.5,
        -0.5,-0.2,0.5,
        0.5,-0.2,-0.5,

        -0.5,-0.2,0.5,
        0.5,-0.2,-0.5,
        0.5,-0.2,0.5,*/
        -0.5, 0.2, 0.5,
        -0.5, -0.2, 0.5,
        0.5, -0.2, 0.5,
        -0.5, 0.2, 0.5,
        0.5, -0.2, 0.5,
        0.5, 0.2, 0.5,
        0.5, 0.2, 0.5,
        0.5, -0.2, 0.5,
        0.5, -0.2, -0.5,
        0.5, 0.2, 0.5,
        0.5, -0.2, -0.5,
        0.5, 0.2, -0.5,
        0.5, 0.2, -0.5,
        0.5, -0.2, -0.5,
        -0.5, -0.2, -0.5,
        0.5, 0.2, -0.5,
        -0.5, -0.2, -0.5,
        -0.5, 0.2, -0.5,
        -0.5, 0.2, -0.5,
        -0.5, -0.2, -0.5,
        -0.5, -0.2, 0.5,
        -0.5, 0.2, -0.5,
        -0.5, -0.2, 0.5,
        -0.5, 0.2, 0.5,
        -0.5, 0.2, -0.5,
        -0.5, 0.2, 0.5,
        0.5, 0.2, 0.5,
        -0.5, 0.2, -0.5,
        0.5, 0.2, 0.5,
        0.5, 0.2, -0.5,
        -0.5, -0.2, 0.5,
        -0.5, -0.2, -0.5,
        0.5, -0.2, -0.5,
        -0.5, -0.2, 0.5,
        0.5, -0.2, -0.5,
        0.5, -0.2, 0.5,
        -0.5, 0.2, 0.5,
        0.5, 0.2, -0.5,
        0.5, 0.2, -0.5,
  };
    static const GLfloat color_buffer_data [] = {
      0,0,0,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0,5,
      0.5,0.5,0,5,
      0.5,0.5,0,5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
    };
    stage = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createStageWhite()
{
  static const GLfloat vertex_buffer_data [] = {
  /*  -0.5, 0.2, -0.5,
    0.5,0.2, -0.5,
    -0.5, -0.2, 0.5,

    0.5,0.2, -0.5,
    -0.5, -0.2, 0.5,
    0.5, 0.2, 0.5,

    -0.5,0.2,-0.5,
    -0.5,-0.2,-0.5,
    -0.5,0.2,0.5,

    -0.5,-0.2,-0.5,
    -0.5,0.2,0.5,
    -0.5,-0.2,0.5,

    0.5,0.2,-0.5,
    0.5,-0.2,-0.5,
    0.5,0.2,0.5,

    0.5,-0.2,-0.5,
    0.5,0.2,0.5,
    0.5,-0.2,0.5,

    -0.5,0.2,0.5,
    -0.5,-0.2,0.5,
    0.5,-0.2,0.5,

      -0.5,0.2,0.5,
      0.5,-0.2,0.5,
      0.5,0.2,0.5,

      -0.5,0.2,-0.5,
      -0.5,-0.2,-0.5,
      0.5,-0.2,-0.5,

        -0.5,0.2,-0.5,
        0.5,-0.2,-0.5,
        0.5,0.2,-0.5,

        -0.5,-0.2,-0.5,
        -0.5,-0.2,0.5,
        0.5,-0.2,-0.5,

        -0.5,-0.2,0.5,
        0.5,-0.2,-0.5,
        0.5,-0.2,0.5,*/
        -0.5, 0.2, 0.5,
        -0.5, -0.2, 0.5,
        0.5, -0.2, 0.5,
        -0.5, 0.2, 0.5,
        0.5, -0.2, 0.5,
        0.5, 0.2, 0.5,
        0.5, 0.2, 0.5,
        0.5, -0.2, 0.5,
        0.5, -0.2, -0.5,
        0.5, 0.2, 0.5,
        0.5, -0.2, -0.5,
        0.5, 0.2, -0.5,
        0.5, 0.2, -0.5,
        0.5, -0.2, -0.5,
        -0.5, -0.2, -0.5,
        0.5, 0.2, -0.5,
        -0.5, -0.2, -0.5,
        -0.5, 0.2, -0.5,
        -0.5, 0.2, -0.5,
        -0.5, -0.2, -0.5,
        -0.5, -0.2, 0.5,
        -0.5, 0.2, -0.5,
        -0.5, -0.2, 0.5,
        -0.5, 0.2, 0.5,
        -0.5, 0.2, -0.5,
        -0.5, 0.2, 0.5,
        0.5, 0.2, 0.5,
        -0.5, 0.2, -0.5,
        0.5, 0.2, 0.5,
        0.5, 0.2, -0.5,
        -0.5, -0.2, 0.5,
        -0.5, -0.2, -0.5,
        0.5, -0.2, -0.5,
        -0.5, -0.2, 0.5,
        0.5, -0.2, -0.5,
        0.5, -0.2, 0.5,
        -0.5, 0.2, 0.5,
        0.5, 0.2, -0.5,
        0.5, 0.2, -0.5,
  };
    static const GLfloat color_buffer_data [] = {
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
      0.8,1,1,
    };
    stagewhite = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createblock ()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data [] = {
-0.5, 1, 0.5,
-0.5, -1, 0.5,
0.5, -1, 0.5,
-0.5, 1, 0.5,
0.5, -1, 0.5,
0.5, 1, 0.5,
0.5, 1, 0.5,
0.5, -1, 0.5,
0.5, -1, -0.5,
0.5, 1, 0.5,
0.5, -1, -0.5,
0.5, 1, -0.5,
0.5, 1, -0.5,
0.5, -1, -0.5,
-0.5, -1, -0.5,
0.5, 1, -0.5,
-0.5, -1, -0.5,
-0.5, 1, -0.5,
-0.5, 1, -0.5,
-0.5, -1, -0.5,
-0.5, -1, 0.5,
-0.5, 1, -0.5,
-0.5, -1, 0.5,
-0.5, 1, 0.5,
-0.5, 1, -0.5,
-0.5, 1, 0.5,
0.5, 1, 0.5,
-0.5, 1, -0.5,
0.5, 1, 0.5,
0.5, 1, -0.5,
-0.5, -1, 0.5,
-0.5, -1, -0.5,
0.5, -1, -0.5,
-0.5, -1, 0.5,
0.5, -1, -0.5,
0.5, -1, 0.5,
-0.5, 1, 0.5,
0.5, 1, -0.5,
0.5, 1, -0.5,
  };

  static const GLfloat color_buffer_data [] = {
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1, 0, 0,
1, 0, 0,
1, 0, 0,
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  block = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createblock2 ()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data [] = {
-0.5, 0.5, 1,
-0.5, -0.5, 1,
0.5, -0.5, 1,
-0.5, 0.5, 1,
0.5, -0.5, 1,
0.5, 0.5, 1,
0.5, 0.5, 1,
0.5, -0.5, 1,
0.5, -0.5, -1,
0.5, 0.5, 1,
0.5, -0.5, -1,
0.5, 0.5, -1,
0.5, 0.5, -1,
0.5, -0.5, -1,
-0.5, -0.5, -1,
0.5, 0.5, -1,
-0.5, -0.5, -1,
-0.5, 0.5, -1,
-0.5, 0.5, -1,
-0.5, -0.5, -1,
-0.5, -0.5, 1,
-0.5, 0.5, -1,
-0.5, -0.5, 1,
-0.5, 0.5, 1,
-0.5, 0.5, -1,
-0.5, 0.5, 1,
0.5, 0.5, 1,
-0.5, 0.5, -1,
0.5, 0.5, 1,
0.5, 0.5, -1,
-0.5, -0.5, 1,
-0.5, -0.5, -1,
0.5, -0.5, -1,
-0.5, -0.5, 1,
0.5, -0.5, -1,
0.5, -0.5, 1,
-0.5, 0.5, 1,
0.5, 0.5, -1,
0.5, 0.5, -1,
  };

  static const GLfloat color_buffer_data [] = {
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1, 0, 0,
    1, 0, 0,
    1, 0, 0,
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  block2 = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createblock3 ()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data [] = {
-1, 0.5, 0.5,
-1, -0.5, 0.5,
1, -0.5, 0.5,
-1, 0.5, 0.5,
1, -0.5, 0.5,
1, 0.5,0.5,
1, 0.5, 0.5,
1, -0.5, 0.5,
1, -0.5, -0.5,
1, 0.5, 0.5,
1, -0.5, -0.5,
1, 0.5, -0.5,
1, 0.5, -0.5,
1, -0.5, -0.5,
-1, -0.5, -0.5,
1, 0.5, -0.5,
-1, -0.5, -0.5,
-1, 0.5, -0.5,
-1, 0.5, -0.5,
-1, -0.5, -0.5,
-1, -0.5, 0.5,
-1, 0.5, -0.5,
-1, -0.5, 0.5,
-1, 0.5, 0.5,
-1, 0.5, -0.5,
-1, 0.5, 0.5,
1, 0.5, 0.5,
-1, 0.5, -0.5,
1, 0.5, 0.5,
1, 0.5, -0.5,
-1, -0.5, 0.5,
-1, -0.5, -0.5,
1, -0.5, -0.5,
-1, -0.5, 0.5,
1, -0.5, -0.5,
1, -0.5, 0.5,
-1, 0.5, 0.5,
1, 0.5, -0.5,
1, 0.5, -0.5,
  };

  static const GLfloat color_buffer_data [] = {
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1, 0, 0,
    1, 0, 0,
    1, 0, 0,
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  block3 = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createRectangle ()
{
  // GL3 accepts only blocks. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -1.2,-1,0, // vertex 1
    1.2,-1,0, // vertex 2
    1.2, 1,0, // vertex 3

    1.2, 1,0, // vertex 3
    -1.2, 1,0, // vertex 4
    -1.2,-1,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    0,0,1, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0.3,0.3,0.3, // color 4
    1,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float block_rotation = 0;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye;
  glm::vec3 target;
  changePerspective();
  if(camera==0)
  {
   eye = glm::vec3(3, 5, 5 );
   target = glm::vec3(2, 0, 0);
 }
  else if(camera==1)// top view
  {
  eye = glm::vec3(2, 3, 2.9);
  target = glm::vec3(2, -0.7, 2);
  }
  else if(camera==2)// block view
  {
    eye = glm::vec3(blockx,blocky+3,blockz);
    target=glm::vec3(blockx+5,blocky,blockz);
    turnstat=1;
  }
  else if(camera==3)// third person view/follow cam
  {
    eye = glm::vec3(blockx-1.5,blocky+3,blockz);
    target=glm::vec3(blockx+5,blocky,blockz);
    turnstat=1;
  }
  // Target - Where is the camera looking at.  Don't change unless you are sure!!

  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
   Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  //Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least awthe M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

  // Load identity to model matrix
  Matrices.model = glm::mat4(1.0f);

  /* Render your scene */
if(triang_left==1)
{

  block_rotation += 10;
  if(posback==1)
  {
    xzflag = 0;
    blockx-=0.16666;
    blocky-=0.0555;
    posbackeep = 3;
  }
  else if(posback==2) // ok
  {
    xzflag = 2;// rotation around z axis
    blockx-=0.11111;
    posbackeep = 2;
  }
  else if(posback==3)
  {
    xzflag = 0;
  blockx-=0.16666;
  blocky+=0.0555;
  posbackeep = 1;
}

}
if(triang_right==1)
{
    block_rotation -= 10;
    if(posback==1)
    {
      xzflag = 0;
      blockx+=0.16666;
      blocky-=0.0555;
      posbackeep = 3;
    }
    else if(posback==2) // ok
    {
      xzflag = 2;// rotation around z axis
      blockx+=0.11111;
      posbackeep = 2;
    }
    else if(posback==3)
    {
      xzflag = 0;
    blockx+=0.16666;
    blocky+=0.0555;
    posbackeep = 1;
    }
}
if(triang_up==1)
{
  block_rotation -= 10;
  if(posback==1)
  {
    xzflag = 1;
    blockz-=0.16666;
    blocky-=0.0555;
    posbackeep = 2;
  }
  else if(posback==2) // ok
  {
    xzflag = 1;// rotation around x axis
    blockz-=0.16666;
    blocky+=0.05555;
    posbackeep = 1;
  }
  else if(posback==3)
  {
    xzflag = 4;
  blockz-=0.11111;
  posbackeep = 3;
}
}
if(triang_down==1)
{
  block_rotation += 10;
  if(posback==1)
  {
    xzflag = 1;
    blockz+=0.16666;
    blocky-=0.0555;
    posbackeep = 2;
  }
  else if(posback==2) // ok
  {
    xzflag = 1;// rotation around z axis
    blockz+=0.16666;
    blocky+=0.05555;
    posbackeep = 1;
  }
  else if(posback==3)
  {
    xzflag = 4;
  blockz+=0.11111;
  posbackeep = 3;
}
}

// checking wether tilt/fall should take place



if(((int)block_rotation)%90!=0)
{

glm::mat4 translateblock = glm::translate (glm::vec3(blockx, blocky, blockz)); // glTranslatef
  glm::mat4 rotateblock;
  if(xzflag==0)
  rotateblock = glm::rotate((float)(block_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  else if(xzflag==1)
  {
  rotateblock = glm::rotate((float)((block_rotation)*M_PI/180.0f), glm::vec3(1,0,0));  // rotate about vector (1,0,0)
  }
  else if(xzflag==2)
  rotateblock = glm::rotate((float)(block_rotation*M_PI/180.0f), glm::vec3(0,0,1))*glm::rotate((float)(90*M_PI/180.0f), glm::vec3(1,0,0));  // rotate about vector (1,0,0)
  // rotateblock = glm::rotate((float)(block_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  else if(xzflag==3)
  rotateblock = glm::rotate((float)(block_rotation*M_PI/180.0f), glm::vec3(0,0,1))*glm::rotate((float)(90*M_PI/180.0f), glm::vec3(1,0,0));
  else if(xzflag==4)
  {
    rotateblock = glm::rotate((float)(block_rotation*M_PI/180.0f), glm::vec3(1,0,0))*glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1));
  }
  glm::mat4 scaleblock = glm::scale(glm::vec3(1,1,1));
  glm::mat4 blockTransform = translateblock * rotateblock * scaleblock;
  Matrices.model *= blockTransform;
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(block);
}
if(((int)block_rotation)%90==0)
{
  cout<<"The x value rn is "<<blockx<<endl;
  cout<<"The y value rn is "<<blocky<<endl;
  int counti=0;
  int tempindex;
  if(stageup==1)
  tempindex=52;
  else
  tempindex=51;
  if(posback==1)
  {
    counti=0;
    if(!(fabs(blockx-2.99976)<0.0001) && (fabs(blockz-4.9997)<0.001))
    {
    for (int k=0;k<tempindex;k++)
    {
      if(fabs(blockx-xstage[k])<0.5 && fabs(blockz-zstage[k])<0.5)
      {
        ++counti;
      }
    }
    if(counti==0)
    {

    blockx = 20;
    }
  }
    glm::mat4 translateblock23 = glm::translate (glm::vec3(blockx, blocky, blockz)); // glTranslatef
    glm::mat4 scaleblock23 = glm::scale(glm::vec3(1,1,1));
    glm::mat4 blockTransform23 = translateblock23 * scaleblock23;
    Matrices.model *= blockTransform23;
    MVP = VP * Matrices.model; // MVP = p * V * M
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(block);
    Matrices.model = glm::mat4(1.0f);
  }
  if (posback==2)
  {
    counti=0;
    int counti2=0;
    for (int k=0;k<tempindex;k++)
    {
      if(fabs(blockx-xstage[k])<0.5 && fabs((blockz-0.5)-zstage[k]<0.5) )
      {
        ++counti;
      }
      if (fabs(blockx-xstage[k])<0.5&& fabs((blockz+0.5)-zstage[k]<0.5))
      {
        ++counti2;
      }
    }
    if(counti==0 || counti2==0)
    {

    blockx=20;
  }
    glm::mat4 translateblock32 = glm::translate (glm::vec3(blockx, blocky, blockz)); // glTranslatef
    glm::mat4 scaleblock32 = glm::scale(glm::vec3(1,1,1));
    glm::mat4 blockTransform32 = translateblock32 * scaleblock32;
    Matrices.model *= blockTransform32;
    MVP = VP * Matrices.model; // MVP = p * V * M
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(block2);
    Matrices.model = glm::mat4(1.0f);
  }
  if (posback==3)
  {
    for (int k=0;k<tempindex;k++)
    {
      if(fabs(blockx-xstage[k])<0.5 && fabs(blockz-zstage[k]<0.5))
      {

        ++counti;
      }
    }
    if(counti<1)
    {
    blockx=20;

  }
    glm::mat4 translateblock34 = glm::translate (glm::vec3(blockx, blocky, blockz)); // glTranslatef
    glm::mat4 scaleblock34 = glm::scale(glm::vec3(1,1,1));
    glm::mat4 blockTransform34 = translateblock34 * scaleblock34;
    Matrices.model *= blockTransform34;
    MVP = VP * Matrices.model; // MVP = p * V * M
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(block3);
    Matrices.model = glm::mat4(1.0f);
  }


}

  // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
  // glPopMatrix ();
  Matrices.model = glm::mat4(1.0f);
for (int jk=0;jk<52;jk++)
{
  //cout<<"blockx is "<<blockx<<" and blockz is "<<blockz<<endl;
  if((fabs(blockx-2.99976)<0.0001) && (fabs(blockz-4.9997)<0.001))
  {
    ++score;

    cout<<"You have won and score is "<<score<<".\n";
    blocky=blocky-0.001;
    glm::mat4 translateblock234 = glm::translate (glm::vec3(blockx, blocky, blockz)); // glTranslatef
    glm::mat4 scaleblock234 = glm::scale(glm::vec3(1,1,1));
    glm::mat4 blockTransform234 = translateblock234 * scaleblock234;
    Matrices.model *= blockTransform234;
    MVP = VP * Matrices.model; // MVP = p * V * M
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(block);
    Matrices.model = glm::mat4(1.0f);
    if(blocky<-8)
    {
      blockx = -3;
      blocky= 0.5;
      blockz = 0;
      stageup=0;
    }
}
  if((fabs((5-blockx))<0.5) && (fabs((2-blockz))<0.6))
  {
    stageup=1;
  }
  if((fabs((5-blockx))<0.5) && (fabs((-2-blockz))<0.6))
  {
    stageup=0;
  }

  // al tiles/stages are put up here
  if(((jk==51)&&(stageup==1))||(jk<51))
  {
  glm::mat4 translateblock1 = glm::translate (glm::vec3(xstage[jk], -0.7, zstage[jk])); // glTranslatef
  glm::mat4 scaleblock1 = glm::scale(glm::vec3(1,1,1));
  glm::mat4 blockTransform1 = translateblock1 * scaleblock1;
  Matrices.model *= blockTransform1;
  MVP = VP * Matrices.model; // MVP = p * V * M
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  if((xstage[jk]==5) && (zstage[jk]==2))
  {
  draw3DObject(stagewhite);
  }
  else if((xstage[jk]==5) && (zstage[jk]==-2))
  {
  draw3DObject(stagewhite);
  }

  else
  draw3DObject(stage);
  Matrices.model = glm::mat4(1.0f);
}
}


glm::mat4 translateblock235 = glm::translate (glm::vec3(4, -0.7, 2)); // glTranslatef
glm::mat4 scaleblock235 = glm::scale(glm::vec3(1,1,1));
glm::mat4 blockTransform235 = translateblock235 * scaleblock235;
Matrices.model *= blockTransform235;
MVP = VP * Matrices.model; // MVP = p * V * M
glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
draw3DObject(stage);
Matrices.model = glm::mat4(1.0f);
  // Increment angles
  float increments = 1;

  //camera_rotation_angle++; // Simulating camera rotation
  if(((int)block_rotation)%90==0)
  {
  triang_left = 0;
  triang_right = 0;
  triang_up = 0;
  triang_down = 0;
  posback = posbackeep;
//  cout<<"Position now is "<<posback<<endl;
  //cout<<"It is "<<block_rotation<<endl;

}
  rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
//        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
//        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	// Create the models
	createblock (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
  createblock2();
  createblock3();
	createRectangle ();
  createStage();
  createStageWhite();

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

    // Background color of the scenew

	glClearColor (1, 1, 1, 1); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
  engine = createIrrKlangDevice();
  engine->play2D("start.wav");
	int width = 800;
	int height = 800;
blockx = -3;
blocky= 0.5;
blockz = 0;
posi = -1;
posback =1;
posbackeep = 1;
    GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;
    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
        draw();

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;
        }
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

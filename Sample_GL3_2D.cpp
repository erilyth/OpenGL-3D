#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <time.h>
#include <stdlib.h>

#include <thread>
#include <ao/ao.h>
#include <mpg123.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>
#include <FTGL/ftgl.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>

#include <GL/gl.h>
#include <GL/glu.h>

using namespace std;

struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;
	GLuint TextureBuffer;
	GLuint TextureID;

	GLenum PrimitiveMode; // GL_POINTS, GL_LINE_STRIP, GL_LINE_LOOP, GL_LINES, GL_LINE_STRIP_ADJACENCY, GL_LINES_ADJACENCY, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_TRIANGLES, GL_TRIANGLE_STRIP_ADJACENCY and GL_TRIANGLES_ADJACENCY
	GLenum FillMode; // GL_FILL, GL_LINE
	int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID; // For use with normal shader
	GLuint TexMatrixID; // For use with texture shader
} Matrices;

struct FTGLFont {
	FTFont* font;
	GLuint fontMatrixID;
	GLuint fontColorID;
} GL3Font;

typedef struct COLOR {
    float r;
    float g;
    float b;
} COLOR;

typedef struct Point {
    float x;
    float y;
    float z;
} Point;

typedef struct Triangle {
    int p1;
    int p2;
    int p3; //Indices of the points corresponding to the triangle
} Triangle;

struct Sprite {
    string name;
    float x,y,z;
    VAO* object;
    int status;
    float x_scale,y_scale,z_scale;
    float x_speed,y_speed,z_speed;
    float angle_x; //Current Angle (Actual rotated angle of the object)
    float angle_y;
    float angle_z;
    float rotation_x_offset;
    float rotation_y_offset;
    float rotation_z_offset;
    int inAir;
    float radius;
    int fixed;
    float friction; //Value from 0 to 1	
    int health;
    int isRotating;
    int direction_x; //0 for clockwise and 1 for anticlockwise for animation
    int direction_y;
    int direction_z;
    float remAngle; //the remaining angle to finish animation
    int isMovingAnim;
    int dx;
    int dy;
    int dz;
    float weight;
};
typedef struct Sprite Sprite;

map <string, Sprite> objects;
map <string, Sprite> playerObjects;
int player_moving_forward=0;
int player_moving_backward=0;
int player_moving_left=0;
int player_moving_right=0;
int player_rotating=0;
float camera_fov=1.3;
int currentLevel=0;
int height,width;
int camera_follow=0;
int camera_fps=0;
float fps_head_offset=0,fps_head_offset_x=0;
int head_tilting=0;

int elevatorStartLevel=0;
int elevatorFinishLevel=0;
int timeToStartLevel=0;
int timeToFinishLevel=0;

void* play_audio(string audioFile);

//The level specific map and trap map are loaded from files
int gameMap[10][10]={
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0}
};

//1 is not present, 2,3,4 are present
int gameMapTrap[10][10]={
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0}
};


string convertInt(int number)
{
    if (number == 0)
        return "0";
    string temp="";
    string returnvalue="";
    while (number>0)
    {
        temp+=number%10+48;
        number/=10;
    }
    for (int i=0;i<temp.length();i++)
        returnvalue+=temp[temp.length()-i-1];
    return returnvalue;
}

int check_collision_object(string name1,string name2){
	if(objects[name2].status==1 && objects[name1].y>=objects[name2].y-objects[name2].y_scale/2-objects[name1].y_scale/2-50 && objects[name1].y<=objects[name2].y+objects[name2].y_scale/2+objects[name1].y_scale/2+50 && objects[name1].x>=objects[name2].x-objects[name2].x_scale/2-objects[name1].x_scale/2-20 && objects[name1].x<=objects[name2].x+objects[name2].x_scale/2+objects[name1].x_scale/2+20 && objects[name1].z>=objects[name2].z-objects[name2].z_scale/2-objects[name1].z_scale/2-20 && objects[name1].z<=objects[name2].z+objects[name2].z_scale/2+objects[name1].z_scale/2+20 ){
		return 1;
	}
	return 0;
}

void goToNextLevel(GLFWwindow* window);

GLuint programID, fontProgramID, textureProgramID;

/* Function to load Shaders - Use it as it is */
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
	cout << "Compiling shader : " <<  vertex_file_path << endl;
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage( max(InfoLogLength, int(1)) );
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	cout << VertexShaderErrorMessage.data() << endl;

	// Compile Fragment Shader
	cout << "Compiling shader : " << fragment_file_path << endl;
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage( max(InfoLogLength, int(1)) );
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	cout << FragmentShaderErrorMessage.data() << endl;

	// Link the program
	cout << "Linking program" << endl;
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	cout << ProgramErrorMessage.data() << endl;

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	cout << "Error: " << description << endl;
}

void quit(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

glm::vec3 getRGBfromHue (int hue)
{
	float intp;
	float fracp = modff(hue/60.0, &intp);
	float x = 1.0 - abs((float)((int)intp%2)+fracp-1.0);

	if (hue < 60)
		return glm::vec3(1,x,0);
	else if (hue < 120)
		return glm::vec3(x,1,0);
	else if (hue < 180)
		return glm::vec3(0,1,x);
	else if (hue < 240)
		return glm::vec3(0,x,1);
	else if (hue < 300)
		return glm::vec3(x,0,1);
	else
		return glm::vec3(1,0,x);
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

struct VAO* create3DTexturedObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* texture_buffer_data, GLuint textureID, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;
	vao->TextureID = textureID;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->TextureBuffer));  // VBO - textures

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

	glBindBuffer (GL_ARRAY_BUFFER, vao->TextureBuffer); // Bind the VBO textures
	glBufferData (GL_ARRAY_BUFFER, 2*numVertices*sizeof(GLfloat), texture_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
						  2,                  // attribute 2. Textures
						  2,                  // size (s,t)
						  GL_FLOAT,           // type
						  GL_FALSE,           // normalized?
						  0,                  // stride
						  (void*)0            // array buffer offset
						  );

	return vao;
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
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

void draw3DTexturedObject (struct VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Bind Textures using texture units
	glBindTexture(GL_TEXTURE_2D, vao->TextureID);

	// Enable Vertex Attribute 2 - Texture
	glEnableVertexAttribArray(2);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->TextureBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle

	// Unbind Textures to be safe
	glBindTexture(GL_TEXTURE_2D, 0);
}

/* Create an OpenGL Texture from an image */
GLuint createTexture (const char* filename)
{
	GLuint TextureID;
	// Generate Texture Buffer
	glGenTextures(1, &TextureID);
	// All upcoming GL_TEXTURE_2D operations now have effect on our texture buffer
	glBindTexture(GL_TEXTURE_2D, TextureID);
	// Set our texture parameters
	// Set texture wrapping to GL_REPEAT
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Set texture filtering (interpolation)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Load image and create OpenGL texture
	int twidth, theight;
	unsigned char* image = SOIL_load_image(filename, &twidth, &theight, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, twidth, theight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D); // Generate MipMaps to use
	SOIL_free_image_data(image); // Free the data read from file after creating opengl texture
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess it up

	return TextureID;
}


/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = -1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
int inAir=0;
//Camera eye, target and up vector components
float eye_x,eye_y,eye_z;
float target_x=-50,target_y,target_z=-50;
float angle=0;
float camera_radius;
int left_mouse_clicked;
int right_mouse_clicked;
int camera_disable_rotation=0;

int playerOnFinishElevator(){
	int onElevator=0,i,j,k;
	for(i=0;i<10;i++){
		for(j=0;j<10;j++){
			objects["player"].y-=20;
			if(gameMapTrap[i][j]==5){
				string name="finishelevatorbottom";
				if(objects["player"].y>=objects[name].y-objects[name].y_scale/2-objects["player"].y_scale/2-50 && objects["player"].y<=objects[name].y+objects[name].y_scale/2+objects["player"].y_scale/2+50 && objects["player"].x>=objects[name].x-objects[name].x_scale/2-objects["player"].x_scale/2-20 && objects["player"].x<=objects[name].x+objects[name].x_scale/2+objects["player"].x_scale/2+20 && objects["player"].z>=objects[name].z-objects[name].z_scale/2-objects["player"].z_scale/2-20 && objects["player"].z<=objects[name].z+objects[name].z_scale/2+objects["player"].z_scale/2+20 ){
					onElevator=1;
				}
			}
			objects["player"].y+=20;
		}
	}
	return onElevator;
}

int playerOnStartElevator(){
	int onElevator=0,i,j,k;
	for(i=0;i<10;i++){
		for(j=0;j<10;j++){
			objects["player"].y-=20;
			if(gameMapTrap[i][j]==6){
				string name="startelevatorbottom";
				if(objects["player"].y>=objects[name].y-objects[name].y_scale/2-objects["player"].y_scale/2-50 && objects["player"].y<=objects[name].y+objects[name].y_scale/2+objects["player"].y_scale/2+50 && objects["player"].x>=objects[name].x-objects[name].x_scale/2-objects["player"].x_scale/2-20 && objects["player"].x<=objects[name].x+objects[name].x_scale/2+objects["player"].x_scale/2+20 && objects["player"].z>=objects[name].z-objects[name].z_scale/2-objects["player"].z_scale/2-20 && objects["player"].z<=objects[name].z+objects[name].z_scale/2+objects["player"].z_scale/2+20 ){
					onElevator=1;
				}
			}
			objects["player"].y+=20;
		}
	}
	return onElevator;
}

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.

	if (action == GLFW_RELEASE) {
		switch (key) {
			case GLFW_KEY_T:
				camera_disable_rotation=1;
				camera_follow=0;
				camera_fps=0;
				camera_radius=1; //Top view
				eye_x = objects["player"].x+camera_radius*cos(angle*M_PI/180);
				eye_z = objects["player"].z+camera_radius*sin(angle*M_PI/180);
				eye_y=600;
				target_x=objects["player"].x;
				target_y=0;
				target_z=objects["player"].z;
				fps_head_offset=0;
				fps_head_offset_x=0;
				break;
			case GLFW_KEY_Y:
				camera_disable_rotation=0;
				camera_follow=0;
				camera_fps=0;
				camera_radius=800; //Tower view
				eye_x = -50+camera_radius*cos(angle*M_PI/180);
				eye_z = -50+camera_radius*sin(angle*M_PI/180);
				eye_y=600;
				target_x=-50;
				target_y=0;
				target_z=-50;
				fps_head_offset=0;
				fps_head_offset_x=0;
				break;
			case GLFW_KEY_U:
				camera_disable_rotation=1;
				camera_fps=0;
				camera_follow=1;
				fps_head_offset=0;
				fps_head_offset_x=0;
				break;
			case GLFW_KEY_I:
				camera_disable_rotation=1;
				camera_follow=0;
				camera_fps=1;
				fps_head_offset=0;
				fps_head_offset_x=0;
				break;
			case GLFW_KEY_UP:
				player_moving_forward=0;
				break;
			case GLFW_KEY_DOWN:
				player_moving_backward=0;
				break;
			case GLFW_KEY_W:
				head_tilting=0;
				break;
			case GLFW_KEY_S:
				head_tilting=0;
				break;
			case GLFW_KEY_A:
				player_moving_left=0;
				break;
			case GLFW_KEY_D:
				player_moving_right=0;
				break;
			case GLFW_KEY_RIGHT:
				player_rotating=0;
				player_moving_right=0;
				break;
			case GLFW_KEY_LEFT:
				player_rotating=0;
				player_moving_left=0;
				break;
			case GLFW_KEY_C:
				rectangle_rot_status = !rectangle_rot_status;
				break;
			case GLFW_KEY_P:
				triangle_rot_status = !triangle_rot_status;
				break;
			case GLFW_KEY_X:
				// do something ..
				break;
			default:
				break;
		}
	}
	else if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_RIGHT:
				if(camera_fps==1){
					player_moving_right=1;
				}
				else{
					player_rotating=1;
				}
				break;
			case GLFW_KEY_LEFT:
				if(camera_fps==1){
					player_moving_left=1;
				}
				else{
					player_rotating=-1; //The left key has a slight problem when used together with up or down and space.
				}
				break;
			case GLFW_KEY_UP:
				player_moving_forward=1;
				break;
			case GLFW_KEY_DOWN:
				player_moving_backward=1;
				break;
			case GLFW_KEY_W:
				head_tilting=1;
				break;
			case GLFW_KEY_S:
				head_tilting=-1;
				break;
			case GLFW_KEY_A:
				player_moving_left=1;
				break;
			case GLFW_KEY_D:
				player_moving_right=1;
				break;
			case GLFW_KEY_SPACE:
				if(inAir==0 && (playerOnStartElevator()==0 && playerOnFinishElevator()==0)){
					//Dont let the person jump when inside the elevator
					objects["player"].y_speed=10;
					objects["player"].y+=5;
					inAir=1;
				}
				break;
			case GLFW_KEY_ESCAPE:
				quit(window);
				break;
			default:
				break;
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
			if (action == GLFW_PRESS){
				left_mouse_clicked=1;
				break;
			}
			if (action == GLFW_RELEASE){
				triangle_rot_dir *= -1;
				left_mouse_clicked=0;
				break;
			}
		case GLFW_MOUSE_BUTTON_RIGHT:
			if (action == GLFW_PRESS){
				right_mouse_clicked=1;
				break;
			}
			if (action == GLFW_RELEASE){
				triangle_rot_dir *= -1;
				right_mouse_clicked=0;
				break;
			}
			break;
		default:
			break;
	}
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;
	/* With Retina display on Mac OS X, GLFW's FramebufferSize
	 is different from WindowSize */
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = camera_fov; //Use from 1 to 2

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	 glLoadIdentity ();
	 gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	 Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 7000.0f);

	// Ortho projection for 2D views
	//Matrices.projection = glm::ortho(-1000.0f, 1000.0f, -1000.0f, 1000.0f, -1000.0f, 5000.0f);
}

void mousescroll(GLFWwindow* window, double xoffset, double yoffset)
{
    if (yoffset==-1) { 
        camera_fov*=1.1;
    }
    else if(yoffset==1){
        camera_fov/=1.1; //make it bigger than current size
    }
    if(camera_fov>=2){
    	camera_fov=2;
    }
    if(camera_fov<=0.5){
    	camera_fov=0.5;
    }
    reshapeWindow(window,700,700);
}

VAO *triangle, *skybox, *skybox1, *skybox2, *skybox3, *skybox4, *skybox5;

// Creates the triangle object used in this sample code
void createTriangle ()
{
	/* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

	/* Define vertex array as used in glBegin (GL_TRIANGLES) */
	static const GLfloat vertex_buffer_data [] = {
		0, 1,0, // vertex 0
		-1,-1,0, // vertex 1
		1,-1,0, // vertex 2
	};

	static const GLfloat color_buffer_data [] = {
		1,0,0, // color 0
		0,1,0, // color 1
		0,0,1, // color 2
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

// Creates the rectangle object used in this sample code
void createRectangle (GLuint textureID, GLfloat vertex_buffer[],string name)
{
	// GL3 accepts only Triangles. Quads are not supported
	GLfloat vertex_buffer_data [18];
	int i;
	for(i=0;i<18;i++){
		vertex_buffer_data[i]=vertex_buffer[i];
	}

	GLfloat color_buffer_data [] = {
		1,0,0, // color 1
		0,0,1, // color 2
		0,1,0, // color 3

		0,1,0, // color 3
		0,0,1, // color 4
		1,0,0  // color 1
	};

	// Texture coordinates start with (0,0) at top left of the image to (1,1) at bot right
	GLfloat texture_buffer_data [] = {
		0,1, // TexCoord 1 - bot left
		1,1, // TexCoord 2 - bot right
		1,0, // TexCoord 3 - top right

		1,0, // TexCoord 3 - top right
		0,0, // TexCoord 4 - top left
		0,1  // TexCoord 1 - bot left
	};

	// create3DTexturedObject creates and returns a handle to a VAO that can be used later
	if(name=="skybox")
		skybox = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data, texture_buffer_data, textureID, GL_FILL);
	if(name=="skybox1")
		skybox1 = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data, texture_buffer_data, textureID, GL_FILL);
	if(name=="skybox2")
		skybox2 = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data, texture_buffer_data, textureID, GL_FILL);
	if(name=="skybox3")
		skybox3 = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data, texture_buffer_data, textureID, GL_FILL);
	if(name=="skybox4")
		skybox4 = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data, texture_buffer_data, textureID, GL_FILL);
	if(name=="skybox5")
		skybox5 = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data, texture_buffer_data, textureID, GL_FILL);
}

void createModel (string name, float x_pos, float y_pos, float z_pos, float x_scale, float y_scale, float z_scale, string filename, string layer) //Create object from blender
{
    GLfloat vertex_buffer_data [100000] = {
    };
    GLfloat color_buffer_data [100000] = {
    };
    vector<Point> points;
    int len=0;
    string line;
    float a,b,c;
    int start=0;
    ifstream myfile;
    myfile.open(filename.c_str());
    if (myfile.is_open()){
        while (myfile >> line){
            if(line.length()==1 && line[0]=='v'){
                myfile >> a >> b >> c;
                Point cur_point = {};
                cur_point.x=a;
                cur_point.y=b;
                cur_point.z=c;
                points.push_back(cur_point);
            }
        }
        myfile.close();
    }
    int t[3],temp;
    int bcount=0,ccount=0;
    myfile.open(filename.c_str());
    if (myfile.is_open()){
        while (myfile >> line){
            if(line.length()==1 && line[0]=='f'){
                string linemod;
                getline(myfile, linemod);
                int j,ans=0,tt=0,state=0;
                for(j=0;j<linemod.length();j++){
                    if(linemod[j]==' '){
                        ans=0;
                        state=1;
                    }
                    else if(linemod[j]=='/' && ans!=0 && state==1){
                        t[tt]=ans;
                        tt++;
                        state=0;
                    }
                    else if(linemod[j]!='/'){
                        ans=ans*10+linemod[j]-'0';
                    }
                }
                t[tt]=ans;
                Triangle my_triangle = {};
                my_triangle.p1=t[0]-1;
                my_triangle.p2=t[1]-1;
                my_triangle.p3=t[2]-1;
                vertex_buffer_data[bcount]=points[my_triangle.p1].x*x_scale;
                vertex_buffer_data[bcount+1]=points[my_triangle.p1].y*y_scale;
                vertex_buffer_data[bcount+2]=points[my_triangle.p1].z*z_scale;
                vertex_buffer_data[bcount+3]=points[my_triangle.p2].x*x_scale;
                vertex_buffer_data[bcount+4]=points[my_triangle.p2].y*y_scale;
                vertex_buffer_data[bcount+5]=points[my_triangle.p2].z*z_scale;
                vertex_buffer_data[bcount+6]=points[my_triangle.p3].x*x_scale;
                vertex_buffer_data[bcount+7]=points[my_triangle.p3].y*y_scale;
                vertex_buffer_data[bcount+8]=points[my_triangle.p3].z*z_scale;
                bcount+=9;
            }
            if(line.length()==1 && line[0]=='c'){
                float r1,g1,b1,r2,g2,b2,r3,g3,b3;
                myfile >> r1 >> g1 >> b1 >> r2 >> g2 >> b2 >> r3 >> g3 >> b3;       
                color_buffer_data[ccount]=r1/255.0;
                color_buffer_data[ccount+1]=g1/255.0;
                color_buffer_data[ccount+2]=b1/255.0;
                color_buffer_data[ccount+3]=r2/255.0;
                color_buffer_data[ccount+4]=g2/255.0;
                color_buffer_data[ccount+5]=b2/255.0;
                color_buffer_data[ccount+6]=r3/255.0;
                color_buffer_data[ccount+7]=g3/255.0;
                color_buffer_data[ccount+8]=b3/255.0;
                ccount+=9;
            }
        }
        myfile.close();
    }
    VAO* myobject = create3DObject(GL_TRIANGLES, bcount/3, vertex_buffer_data, color_buffer_data, GL_FILL);
    Sprite vishsprite = {};
    vishsprite.name = name;
    vishsprite.object = myobject;
    vishsprite.x=x_pos;
    vishsprite.y=y_pos;
    vishsprite.z=z_pos;
    vishsprite.status=1;
    vishsprite.fixed=0;
    vishsprite.friction=0.4;
    vishsprite.health=100;
    vishsprite.weight=5;
    vishsprite.x_scale=x_scale;
    vishsprite.y_scale=y_scale;
    vishsprite.z_scale=z_scale;
    if(layer=="player")
    	playerObjects[name]=vishsprite;
    else
    	objects[name]=vishsprite;
}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;
double prev_mouse_x;
double prev_mouse_y;
float gravity=0.5;
float trapTimer=0;
int justInAir=0;
float player_speed=1.5;

//Collision checks for gravity, falling etc are only done with the main blocks in the game (The "floorcube" blocks)
int check_collision(GLFWwindow* window){
	int collided=0,i,j;
	player_speed=1.5;
	for(i=0;i<10;i++){
		for(j=0;j<10;j++){
			int p;
			//The floor cubes might be present at various depths
			for(p=0;p<gameMap[i][j];p++){
				string name = "floorcube";
				name.append(convertInt(i)+convertInt(j)+convertInt(p));
				//The character's legs are quite a bit lower, so we use -50 and +50 when checking y-collision
				if(check_collision_object("player",name)){
					collided=1;
				}
			}
			//Check for elevator collision
			if(gameMapTrap[i][j]==5){
				string name="finishelevatorback";
				if(check_collision_object("player",name)){
					collided=1;
				}
				name="finishelevatorleft";
				if(check_collision_object("player",name)){
					collided=1;
				}
				name="finishelevatorright";
				if(check_collision_object("player",name)){
					collided=1;
				}
				name="finishelevatortop";
				if(check_collision_object("player",name)){
					collided=1;
				}
				name="finishelevatorbottom";
				if(check_collision_object("player",name)){
					collided=1;
				}
			}
			if(gameMapTrap[i][j]==6){
				string name="startelevatorback";
				if(check_collision_object("player",name)){
					collided=1;
				}
				name="startelevatorleft";
				if(check_collision_object("player",name)){
					collided=1;
				}
				name="startelevatorright";
				if(check_collision_object("player",name)){
					collided=1;
				}
				name="startelevatortop";
				if(check_collision_object("player",name)){
					collided=1;
				}
				name="startelevatorbottom";
				if(check_collision_object("player",name)){
					collided=1;
				}
				//Check y-axis collisions in the draw function itself
			}
			//The traps/features etc are only on the top level of the floor
			if(gameMapTrap[i][j]==2){
				string name = "spike";
				name.append(convertInt(i)+convertInt(j));
				if(check_collision_object("player",name)){
					currentLevel--;
					goToNextLevel(window);
				}
			}
			//Check water collision
			if(gameMapTrap[i][j]==3){
				string name = "watertrap";
				name.append(convertInt(i)+convertInt(j));
				if(check_collision_object("player",name)){
					player_speed=0.8;
					cout << "WATER TRAP" << endl;
				}
			}
			//Check star collision
			if(gameMapTrap[i][j]==7){
				string name = "star";
				name.append(convertInt(i)+convertInt(j));
				if(check_collision_object("player",name)){
					objects[name].status=0;
					thread(play_audio,"Sounds/star.mp3").detach();
					cout << "FOUND A STAR" << endl;
				}
			}
		}
	}
	return collided;
}

float previous_mouse_y,previous_mouse_x;
float previous_mouse_y2,previous_mouse_x2;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw (GLFWwindow* window)
{
	if(objects["player"].y<-800){
		cout << "Player Died" << endl;
		currentLevel--;
		goToNextLevel(window);
	}
	if(camera_follow==1){
		target_x=objects["player"].x;
		target_y=objects["player"].y;
		target_z=objects["player"].z;
		eye_x=target_x-200*sin(objects["player"].angle_y*M_PI/180);
		eye_y=target_y+200;
		eye_z=target_z-200*cos(objects["player"].angle_y*M_PI/180);
	}
	if(camera_fps==1){
		double new_mouse_x,new_mouse_y;
		glfwGetCursorPos(window,&new_mouse_x,&new_mouse_y);
		if(abs(new_mouse_y-previous_mouse_y2)>=1){
			fps_head_offset-=(new_mouse_y-previous_mouse_y2)/13;
			previous_mouse_y2=new_mouse_y;
		}
		else if(new_mouse_y<=10 || new_mouse_y>=655){
			if(new_mouse_y<=10){
				fps_head_offset-=-0.3;
			}
			else{
				fps_head_offset-=0.3;
			}
		}
		if(abs(new_mouse_x-previous_mouse_x2)>=1){
			objects["player"].angle_y-=(new_mouse_x-previous_mouse_x2)/8;
			previous_mouse_x2=new_mouse_x;
		}
		else if(new_mouse_x<=10 || new_mouse_x>=1355){
			if(new_mouse_x<=10){
				objects["player"].angle_y+=1.5;
			}
			else{
				objects["player"].angle_y-=1.5;
			}
		}
		if(fps_head_offset>=30){
			fps_head_offset=30;
		}
		if(fps_head_offset<=-30){
			fps_head_offset=-30;
		}
		target_x=objects["player"].x+42*sin((objects["player"].angle_y)*M_PI/180);
		target_y=objects["player"].y+60+fps_head_offset;
		target_z=objects["player"].z+42*cos((objects["player"].angle_y)*M_PI/180);
		eye_x=objects["player"].x+42*sin(objects["player"].angle_y*M_PI/180)-10*sin(objects["player"].angle_y*M_PI/180);
		eye_y=objects["player"].y+60;
		eye_z=objects["player"].z+42*cos(objects["player"].angle_y*M_PI/180)-10*cos(objects["player"].angle_y*M_PI/180);
	}
	int i,j;
	for(i=0;i<10;i++){
		for(j=0;j<10;j++){
			string temp="star";
			temp.append(convertInt(i)+convertInt(j));
			objects[temp].angle_y+=2;
			if(objects[temp].isMovingAnim>=0 && objects[temp].isMovingAnim<=50){
				objects[temp].y+=0.4;
			}
			if(objects[temp].isMovingAnim>=51 && objects[temp].isMovingAnim<=100){
				objects[temp].y-=0.4;
			}
			objects[temp].isMovingAnim=(objects[temp].isMovingAnim+1)%101;
		}
	}
	if(elevatorStartLevel==1){
		objects["startelevatorbottom"].y-=2;
		objects["startelevatortop"].y-=2;
		objects["startelevatorright"].y-=2;
		objects["startelevatorleft"].y-=2;
		objects["startelevatorback"].y-=2;
		objects["startelevator"].y-=2;
		timeToStartLevel--;
		if(timeToStartLevel<=0){
			elevatorStartLevel=0;
		}
		objects["player"].y=objects["startelevatorbottom"].y+objects["startelevatorbottom"].y_scale/2+objects["player"].y_scale/2+55;
		objects["player"].x=objects["startelevatorbottom"].x;
		objects["player"].z=objects["startelevatorbottom"].z;
	}
	
	if(playerOnFinishElevator()==1 && elevatorFinishLevel==0){
		thread(play_audio,"Sounds/finish.mp3").detach();
		elevatorFinishLevel=100;
	}
	if(elevatorFinishLevel>1){
		elevatorFinishLevel--;
	}
	if(elevatorFinishLevel==1){
		if(timeToFinishLevel==0){
			timeToFinishLevel=150;
			thread(play_audio,"Sounds/teleport.mp3").detach();
		}
		timeToFinishLevel--;
		objects["finishelevatorbottom"].y+=2;
		objects["finishelevatortop"].y+=2;
		objects["finishelevatorright"].y+=2;
		objects["finishelevatorleft"].y+=2;
		objects["finishelevatorback"].y+=2;
		objects["finishelevator"].y+=2;
		if(timeToFinishLevel<=0){
			elevatorFinishLevel=0;
			goToNextLevel(window);
		}
		objects["player"].y=objects["finishelevatorbottom"].y+objects["finishelevatorbottom"].y_scale/2+objects["player"].y_scale/2+55;
		objects["player"].x=objects["finishelevatorbottom"].x;
		objects["player"].z=objects["finishelevatorbottom"].z;
	}

	if(inAir){
		objects["player"].y+=objects["player"].y_speed;
	}
	trapTimer+=1;
	int p;
	for(i=0;i<10;i++){
		for(j=0;j<10;j++){
			if(gameMapTrap[i][j]==2){
				string name="spike";
				name.append(convertInt(i)+convertInt(j));
				if(trapTimer>0 && trapTimer<=10){
					objects[name].y+=9;
				}
				else if(trapTimer>90 && trapTimer<=120){
					objects[name].y-=3;
				}
			}
		}
	}
	if(trapTimer>=260)
		trapTimer=0;
	objects["player"].y-=5;
	if(check_collision(window)!=1){
		inAir=1;
		playerObjects["playerhand"].angle_x=0;
		playerObjects["playerhand2"].angle_x=0;
		playerObjects["playerleg"].angle_x=0;
		playerObjects["playerleg2"].angle_x=0;
	}
	else if(justInAir){
		if(justInAir==1){
			justInAir=0;
		}
		playerObjects["playerhand"].angle_x=0;
		playerObjects["playerhand2"].angle_x=0;
		playerObjects["playerleg"].angle_x=0;
		playerObjects["playerleg2"].angle_x=0;
	}
	objects["player"].y+=5;

	if(inAir==1){
		objects["player"].y_speed-=gravity;
		if(objects["player"].y_speed<=-12.0){
			objects["player"].y_speed=-12.0;
		}
		//Check collision in the y-axis to detect if the player is in air or not
		int collided=0;
		for(i=0;i<10;i++){
			for(j=0;j<10;j++){
				for(p=0;p<gameMap[i][j];p++){
					objects["player"].y-=5;
					if(gameMapTrap[i][j]==5){
						string name = "finishelevatorbottom";
						if(check_collision_object("player",name)){
							collided=1;
							objects["player"].y=objects[name].y+objects[name].y_scale/2+objects["player"].y_scale/2+45+5;
							objects["player"].y_speed=0;
							inAir=0;
							justInAir=1;
						}
					}
					if(gameMapTrap[i][j]==6){
						string name = "startelevatorbottom";
						if(check_collision_object("player",name)){
							collided=1;
							objects["player"].y=objects[name].y+objects[name].y_scale/2+objects["player"].y_scale/2+45+5;
							objects["player"].y_speed=0;
							inAir=0;
							justInAir=1;
						}
					}
					string name = "floorcube";
					name.append(convertInt(i)+convertInt(j)+convertInt(p));
					//The character's legs are quite a bit lower, so we use -45 and +45 when checking y-collision
					if(check_collision_object("player",name)){
						collided=1;
						objects["player"].y=objects[name].y+objects[name].y_scale/2+objects["player"].y_scale/2+45+5;
						objects["player"].y_speed=0;
						inAir=0;
						justInAir=1;
					}
					objects["player"].y+=5;
				}
			}
		}
	}

	//Check for collisions with moving blocks even when you are not jumping
	for(i=0;i<10;i++){
		for(j=0;j<10;j++){
			for(p=0;p<gameMap[i][j];p++){
				objects["player"].y-=5;
				string name = "floorcube";
				name.append(convertInt(i)+convertInt(j)+convertInt(p));
				//The character's legs are quite a bit lower, so we use -45 and +45 when checking y-collision
				if(inAir==0){
					if(check_collision_object("player",name)){
						objects["player"].y=objects[name].y+objects[name].y_scale/2+objects["player"].y_scale/2+45+5;
						objects["player"].y_speed=0;
					}
				}
				if(objects[name].isMovingAnim!=0){
					objects[name].dy=(objects[name].dy+1)%objects[name].isMovingAnim;
					if(objects[name].dy<objects[name].isMovingAnim/2){
						objects[name].y+=1;
					}
					else{
						objects[name].y-=1;
						objects["player"].y-=1;
						//Check for collision and move the player down so that he doesn't keep jumping and stop jumping over and over again
						if(check_collision_object("player",name)){
							objects["player"].y-=1;
						}
						objects["player"].y+=1;
					}
				}
				objects["player"].y+=5;
			}
		}
	}

	if(player_rotating!=0){
		objects["player"].angle_y-=player_rotating*2;
	}
	if(player_moving_forward!=0){
		objects["player"].z+=player_speed*cos(objects["player"].angle_y*M_PI/180)*2;
		if(check_collision(window)==1){
			objects["player"].z-=player_speed*cos(objects["player"].angle_y*M_PI/180)*2;
		}
		objects["player"].x+=player_speed*sin(objects["player"].angle_y*M_PI/180)*2;
		if(check_collision(window)==1){
			objects["player"].x-=player_speed*sin(objects["player"].angle_y*M_PI/180)*2;
		}
	}
	else if(player_moving_backward!=0){
		objects["player"].z-=player_speed*cos(objects["player"].angle_y*M_PI/180)*2;
		if(check_collision(window)==1){
			objects["player"].z+=player_speed*cos(objects["player"].angle_y*M_PI/180)*2;
		}
		objects["player"].x-=player_speed*sin(objects["player"].angle_y*M_PI/180)*2;
		if(check_collision(window)==1){
			objects["player"].x+=player_speed*sin(objects["player"].angle_y*M_PI/180)*2;
		}
	}
	if(player_moving_left!=0){
		objects["player"].z+=player_speed*cos((objects["player"].angle_y+90)*M_PI/180)*2;
		if(check_collision(window)==1){
			objects["player"].z-=player_speed*cos((objects["player"].angle_y+90)*M_PI/180)*2;
		}
		objects["player"].x+=player_speed*sin((objects["player"].angle_y+90)*M_PI/180)*2;
		if(check_collision(window)==1){
			objects["player"].x-=player_speed*sin((objects["player"].angle_y+90)*M_PI/180)*2;
		}
	}
	else if(player_moving_right!=0){
		objects["player"].z-=player_speed*cos((objects["player"].angle_y+90)*M_PI/180)*2;
		if(check_collision(window)==1){
			objects["player"].z+=player_speed*cos((objects["player"].angle_y+90)*M_PI/180)*2;
		}
		objects["player"].x-=player_speed*sin((objects["player"].angle_y+90)*M_PI/180)*2;
		if(check_collision(window)==1){
			objects["player"].x+=player_speed*sin((objects["player"].angle_y+90)*M_PI/180)*2;
		}
	}
	if((player_moving_forward!=0 || player_moving_backward!=0) && !inAir){ //The player is not stationary
		if(playerObjects["playerhand"].direction_x==0){
			playerObjects["playerhand"].angle_x+=2;
			playerObjects["playerleg"].angle_x-=1.4;
			playerObjects["playerhand2"].angle_x-=2;
			playerObjects["playerleg2"].angle_x+=1.4;
			if(playerObjects["playerhand"].angle_x>=45){
				playerObjects["playerhand"].direction_x=1;
			}
		}
		if(playerObjects["playerhand"].direction_x==1){
			playerObjects["playerhand"].angle_x-=2;
			playerObjects["playerleg"].angle_x+=1.4;
			playerObjects["playerhand2"].angle_x+=2;
			playerObjects["playerleg2"].angle_x-=1.4;
			if(playerObjects["playerhand"].angle_x<=-45){
				playerObjects["playerhand"].direction_x=0;
			}
		}
	}
	else if(!inAir){ //Stop the player movement slowly
		playerObjects["playerhand"].angle_x-=playerObjects["playerhand"].angle_x/4;
		playerObjects["playerhand2"].angle_x-=playerObjects["playerhand2"].angle_x/4;
		playerObjects["playerleg"].angle_x-=playerObjects["playerleg"].angle_x/4;
		playerObjects["playerleg2"].angle_x-=playerObjects["playerleg2"].angle_x/4;
		if(playerObjects["playerhand"].angle_x<10 && playerObjects["playerhand"].angle_x>-10){
			playerObjects["playerhand"].angle_x=0;
		}
		if(playerObjects["playerhand2"].angle_x<10 && playerObjects["playerhand2"].angle_x>-10){
			playerObjects["playerhand2"].angle_x=0;
		}
		if(playerObjects["playerleg2"].angle_x<10 && playerObjects["playerleg2"].angle_x>-10){
			playerObjects["playerleg2"].angle_x=0;
		}
		if(playerObjects["playerleg"].angle_x<10 && playerObjects["playerleg"].angle_x>-10){
			playerObjects["playerleg"].angle_x=0;
		}
	}
	else if(inAir){
		playerObjects["playerhand"].angle_x=-180;
		playerObjects["playerhand2"].angle_x=-180;
		playerObjects["playerleg"].angle_x=0;
		playerObjects["playerleg2"].angle_x=0;
	}

	double new_mouse_x,new_mouse_y;
	glfwGetCursorPos(window,&new_mouse_x,&new_mouse_y);
	if(left_mouse_clicked==1 && camera_follow==0 && camera_disable_rotation==0){
		if(new_mouse_x<=350)
			angle+=1;
		else
			angle-=1;
		previous_mouse_x=new_mouse_x;
		eye_x = -50+camera_radius*cos(angle*M_PI/180);
		eye_z = -50+camera_radius*sin(angle*M_PI/180);
	}
	if(right_mouse_clicked && camera_follow==0 && camera_disable_rotation==0){
		if (abs(previous_mouse_y-new_mouse_y)>=35)
			previous_mouse_y = new_mouse_y;
		else
			eye_y += new_mouse_y - previous_mouse_y;
		previous_mouse_y=new_mouse_y;
	}
	prev_mouse_x=new_mouse_x;
	prev_mouse_y=new_mouse_y;
	// clear the color and depth in the frame buffer
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use the loaded shader program
	// Don't change unless you know what you are doing
	glUseProgram (programID);

	// Eye - Location of camera. Don't change unless you are sure!!
	glm::vec3 eye (eye_x, eye_y, eye_z);
	// Target - Where is the camera looking at.  Don't change unless you are sure!!
	glm::vec3 target (target_x, target_y, target_z);
	// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
	glm::vec3 up (0, 1, 0);

	// Compute Camera matrix (view)
	// Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
	//  Don't change unless you are sure!!
	static float c = 0;
	c++;
	//Matrices.view = glm::lookAt(glm::vec3(0,0,10), glm::vec3(0,0,0), glm::vec3(sinf(c*M_PI/180.0),3*cosf(c*M_PI/180.0),0)); // Fixed camera for 2D (ortho) in XY plane
	Matrices.view = glm::lookAt(eye, target, up); // Fixed camera for 2D (ortho) in XY plane
	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	//  Don't change unless you are sure!!
	glm::mat4 VP = Matrices.projection * Matrices.view;

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)
	//  Don't change unless you are sure!!
	glm::mat4 MVP;	// MVP = Projection * View * Model
	static int fontScale = 0;

	//Draw the objects
    for(map<string,Sprite>::iterator it=objects.begin();it!=objects.end();it++){
        string current = it->first; //The name of the current object
        if(objects[current].status==0)
            continue;
        glm::mat4 MVP;  // MVP = Projection * View * Model

        Matrices.model = glm::mat4(1.0f);

        glm::mat4 ObjectTransform;
        glm::mat4 rotateObject = glm::rotate((float)((objects[current].angle_y)*M_PI/180.0f), glm::vec3(0,1,0));  // rotate about vector (1,0,0)
        rotateObject*=glm::rotate((float)((objects[current].angle_x)*M_PI/180.0f), glm::vec3(1,0,0));
        rotateObject*=glm::rotate((float)((objects[current].angle_z)*M_PI/180.0f), glm::vec3(0,0,1));
        glm::mat4 translateObject = glm::translate (glm::vec3(objects[current].x, objects[current].y, objects[current].z)); // glTranslatef
        ObjectTransform=translateObject*rotateObject;
        Matrices.model *= ObjectTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M
        
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

        draw3DObject(objects[current].object);
        //glPopMatrix (); 
    }


    //Draw the player
    for(map<string,Sprite>::iterator it=playerObjects.begin();it!=playerObjects.end();it++){
        string current = it->first; //The name of the current object
        if(playerObjects[current].status==0)
            continue;
        glm::mat4 MVP;  // MVP = Projection * View * Model

        Matrices.model = glm::mat4(1.0f);

        glm::mat4 ObjectTransform;
        glm::mat4 rotateObject = glm::rotate((float)((objects["player"].angle_y)*M_PI/180.0f), glm::vec3(0,1,0));
    	rotateObject*=glm::rotate((float)((objects[current].angle_x)*M_PI/180.0f), glm::vec3(1,0,0));
        rotateObject*=glm::rotate((float)((objects[current].angle_z)*M_PI/180.0f), glm::vec3(0,0,1));
        glm::mat4 selfRotate = glm::rotate((float)((playerObjects[current].angle_y)*M_PI/180.0f), glm::vec3(0,1,0));
        selfRotate*=glm::rotate((float)((playerObjects[current].angle_x)*M_PI/180.0f), glm::vec3(1,0,0));
        selfRotate*=glm::rotate((float)((playerObjects[current].angle_z)*M_PI/180.0f), glm::vec3(0,0,1));
        glm::mat4 translateSelfOffset = glm::translate (glm::vec3(playerObjects[current].rotation_x_offset,playerObjects[current].rotation_y_offset,playerObjects[current].rotation_z_offset));
        glm::mat4 translateSelfOffsetBack = glm::translate (glm::vec3(-playerObjects[current].rotation_x_offset,-playerObjects[current].rotation_y_offset,-playerObjects[current].rotation_z_offset));
        glm::mat4 translateRelative = glm::translate (glm::vec3(playerObjects[current].x,playerObjects[current].y,playerObjects[current].z));
        glm::mat4 translateRelativeBack = glm::translate (glm::vec3(-playerObjects[current].x,-playerObjects[current].y,-playerObjects[current].z));
        glm::mat4 translateObject = glm::translate (glm::vec3(playerObjects[current].x+objects["player"].x, playerObjects[current].y+objects["player"].y, playerObjects[current].z+objects["player"].z)); // glTranslatef
        ObjectTransform=translateObject*translateRelativeBack*rotateObject*translateRelative*translateSelfOffsetBack*selfRotate*translateSelfOffset;

        //Read from right to left when the transformations are applied on points!
        //Move the object to the self rotation offset specified
        //Rotations wrt object first
        //Move the object back to the center
        //Move the object to the offset specified from its parent
        //Rotations wrt the parent object
        //Move the object back to the origin
        //Move the object to the location of its parent+offsets

        Matrices.model *= ObjectTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M
        
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

        draw3DObject(playerObjects[current].object);
        //glPopMatrix (); 
    }



	// Render with texture shaders now
	glUseProgram(textureProgramID);

	// Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
	// glPopMatrix ();
	Matrices.model = glm::mat4(1.0f);

	glm::mat4 translateRectangle = glm::translate (glm::vec3(2, 0, 0));        // glTranslatef
	glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= (translateRectangle * rotateRectangle);
	MVP = VP * Matrices.model;

	// Copy MVP to texture shaders
	glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);

	// Set the texture sampler to access Texture0 memory
	glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);

	// draw3DObject draws the VAO given to it using current MVP matrix
	draw3DTexturedObject(skybox);
	draw3DTexturedObject(skybox1);
	draw3DTexturedObject(skybox2);
	draw3DTexturedObject(skybox3);
	draw3DTexturedObject(skybox4);
	draw3DTexturedObject(skybox5);

	// Increment angles
	float increments = 0;

	// Render font on screen
	float fontScaleValue = 0.75 + 0.25*sinf(fontScale*M_PI/180.0f);
	glm::vec3 fontColor = getRGBfromHue (fontScale);

	// Use font Shaders for next part of code
	glUseProgram(fontProgramID);
	Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Transform the text
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText = glm::translate(glm::vec3(-3,2,0));
	glm::mat4 scaleText = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
	Matrices.model *= (translateText * scaleText);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);

	// Render font
	//GL3Font.font->Render("Round n Round we go !!");




	//camera_rotation_angle++; // Simulating camera rotation
	triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
	rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;

	// font size and color changes
	fontScale = (fontScale + 1) % 360;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
	GLFWwindow* window; // window desciptor/handle

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
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
	glfwSetScrollCallback(window, mousescroll); // mouse scroll

	return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	string levelmap = "Levels/gameMap"+convertInt(currentLevel+1);
	string trapmap = "Levels/gameMapTrap"+convertInt(currentLevel+1);
	levelmap+=".txt";
	trapmap+=".txt";

	cout << levelmap << endl;
	fstream myfile(levelmap.c_str());
	int a,i=0,j=0;
	if (myfile.is_open()){
		while(myfile >> a){
			if(i==10){
				i=0;
				j++;
			}
			gameMap[i][j]=a;
			i++;
		}
		myfile.close();
	}

	fstream myfile2(trapmap.c_str());
	i=0;
	j=0;
	if (myfile2.is_open()){
		while(myfile2 >> a){
			if(i==10){
				i=0;
				j++;
			}
			gameMapTrap[i][j]=a;
			i++;
		}
		myfile2.close();
	}

	// Load Textures
	// Enable Texture0 as current texture memory
	glActiveTexture(GL_TEXTURE0);
	// load an image file directly as a new OpenGL texture
	// GLuint texID = SOIL_load_OGL_texture ("Images/beach.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_TEXTURE_REPEATS); // Buggy for OpenGL3
	// check for an error during the load process
	//if(textureID == 0 )
	//	cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;

	// Create and compile our GLSL program from the texture shaders
	textureProgramID = LoadShaders( "TextureRender.vert", "TextureRender.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.TexMatrixID = glGetUniformLocation(textureProgramID, "MVP");


	/* Objects should be created before any other gl function and shaders */
	// Create the models
	//createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	
	//Creating the sky box
	float vertX[]={0,-1500,-1500,-1500,-1500,1500,1500,1500,1500};
	float vertY[]={0,-1500,-1500,1500,1500,-1500,-1500,1500,1500};
	float vertZ[]={0,-1500,1500,-1500,1500,-1500,1500,-1500,1500};
	GLfloat vertex_buffer_data [] = {
		vertX[1],vertY[1],vertZ[1],
		vertX[5],vertY[5],vertZ[5],
		vertX[7],vertY[7],vertZ[7],
		vertX[7],vertY[7],vertZ[7],
		vertX[3],vertY[3],vertZ[3],
		vertX[1],vertY[1],vertZ[1]
	};
	GLfloat vertex_buffer_data1 [] = {
		vertX[5],vertY[5],vertZ[5],
		vertX[6],vertY[6],vertZ[6],
		vertX[8],vertY[8],vertZ[8],
		vertX[8],vertY[8],vertZ[8],
		vertX[7],vertY[7],vertZ[7],
		vertX[5],vertY[5],vertZ[5]
	};
	GLfloat vertex_buffer_data2 [] = {
		vertX[6],vertY[6],vertZ[6],
		vertX[2],vertY[2],vertZ[2],
		vertX[4],vertY[4],vertZ[4],
		vertX[4],vertY[4],vertZ[4],
		vertX[8],vertY[8],vertZ[8],
		vertX[6],vertY[6],vertZ[6]
	};
	GLfloat vertex_buffer_data3 [] = {
		vertX[2],vertY[2],vertZ[2],
		vertX[1],vertY[1],vertZ[1],
		vertX[3],vertY[3],vertZ[3],
		vertX[3],vertY[3],vertZ[3],
		vertX[4],vertY[4],vertZ[4],
		vertX[2],vertY[2],vertZ[2]
	};
	GLfloat vertex_buffer_data4 [] = {
		vertX[4],vertY[4],vertZ[4],
		vertX[8],vertY[8],vertZ[8],
		vertX[7],vertY[7],vertZ[7],
		vertX[7],vertY[7],vertZ[7],
		vertX[3],vertY[3],vertZ[3],
		vertX[4],vertY[4],vertZ[4]
	};
	GLfloat vertex_buffer_data5 [] = {
		vertX[2],vertY[2],vertZ[2],
		vertX[6],vertY[6],vertZ[6],
		vertX[5],vertY[5],vertZ[5],
		vertX[5],vertY[5],vertZ[5],
		vertX[1],vertY[1],vertZ[1],
		vertX[2],vertY[2],vertZ[2]
	};

	GLuint textureID = createTexture("Images/top.png");
	GLuint textureID1 = createTexture("Images/left1.png");
	GLuint textureID2 = createTexture("Images/left2.png");
	GLuint textureID3 = createTexture("Images/right.png");
	GLuint textureID4 = createTexture("Images/bottom.png");
	GLuint textureID5 = createTexture("Images/middle.png");

	createRectangle (textureID5,vertex_buffer_data,"skybox");
	createRectangle (textureID3,vertex_buffer_data1,"skybox1");
	createRectangle (textureID1,vertex_buffer_data2,"skybox2");
	createRectangle (textureID2,vertex_buffer_data3,"skybox3");
	createRectangle (textureID,vertex_buffer_data4,"skybox4");
	createRectangle (textureID4,vertex_buffer_data5,"skybox5");

	float scale=0.7;

	int k;
	for(i=0;i<10;i++){
		for(j=0;j<10;j++){
			if(gameMap[i][j]!=0){
				//Spike traps
				if(gameMapTrap[i][j]==2){
					string name2 = "floortrap";
					name2.append(convertInt(i)+convertInt(j));
					createModel(name2,(j-5)*150,gameMap[i][j]*150-75+25,(i-5)*150,70,70,70,"Models/floortrap.data","");
					string new_name="spike";
					new_name.append(convertInt(i)+convertInt(j));
					createModel (new_name,(j-5)*150,gameMap[i][j]*150-75,(i-5)*150,70,70,70,"Models/spike.data","");
					objects[new_name].direction_y=1;
					//This must be created as the gravity/jump checks are done only with these blocks
					int p;
					for(p=0;p<gameMap[i][j];p++){
						string name = "floorcube";
						name.append(convertInt(i)+convertInt(j)+convertInt(p));
						createModel (name,(j-5)*150,p*150+150/2,(i-5)*150,150,150,150,"Models/cube.data","");
					}
				}
				//Water traps
				else if(gameMapTrap[i][j]==3){
					string name2 = "watertrap";
					name2.append(convertInt(i)+convertInt(j));
					createModel(name2,(j-5)*150,gameMap[i][j]*150-75,(i-5)*150,150,120,150,"Models/water.data","");
					int random=rand()%7;
					if(random<=1){
						string name3 = "lilypad";
						name3.append(convertInt(i)+convertInt(j));
						createModel(name3,(j-5)*150,gameMap[i][j]*150,(i-5)*150,30,30,30,"Models/lilypad.data","");
						objects[name3].angle_y=(rand()%360);
					}
					int p;
					for(p=0;p<gameMap[i][j]-1;p++){
						string name = "floorcube";
						name.append(convertInt(i)+convertInt(j)+convertInt(p));
						createModel (name,(j-5)*150,p*150+150/2,(i-5)*150,150,150,150,"Models/cube.data","");
					}
					string name = "floorcube";
					name.append(convertInt(i)+convertInt(j)+convertInt(p));
					createModel (name,(j-5)*150,p*150+150/2-75/2,(i-5)*150,150,75,150,"Models/cube.data","");
				}
				//Pebbles
				else if(gameMapTrap[i][j]==4){
					string new_name="stone";
					new_name.append(convertInt(i)+convertInt(j));
					createModel (new_name,(j-5)*150,(gameMap[i][j])*150+10,(i-5)*150,200,200,200,"Models/stone.data","");
					int p;
					for(p=0;p<gameMap[i][j];p++){
						string name = "floorcube";
						name.append(convertInt(i)+convertInt(j)+convertInt(p));
						createModel (name,(j-5)*150,p*150+150/2,(i-5)*150,150,150,150,"Models/cube.data","");
					}
				}
				else if(gameMapTrap[i][j]==5){ //Elevator needs 3 blocks space for doors to open
					string new_name = "finishelevator";
					createModel (new_name,(j-5)*150,(gameMap[i][j])*150+79,(i-5)*150,70,100,70,"Models/elevator.data","");
					string elevatorblock = new_name+"back";
					createModel (elevatorblock,(j-5)*150+51,(gameMap[i][j])*150+78,(i-5)*150,10,150,130,"Models/cube.data","");
					elevatorblock = new_name+"left";
					createModel (elevatorblock,(j-5)*150,(gameMap[i][j])*150+78,(i-5)*150+62,130,150,10,"Models/cube.data","");
					elevatorblock = new_name+"right";
					createModel (elevatorblock,(j-5)*150,(gameMap[i][j])*150+78,(i-5)*150-62,130,150,10,"Models/cube.data","");
					elevatorblock = new_name+"top";
					createModel (elevatorblock,(j-5)*150,(gameMap[i][j])*150+78+85,(i-5)*150,130,10,130,"Models/cube.data","");
					elevatorblock = new_name+"bottom";
					createModel (elevatorblock,(j-5)*150,(gameMap[i][j])*150+78-85,(i-5)*150,130,10,130,"Models/cube.data","");
					objects[new_name].angle_y=180;
					int p;
					for(p=0;p<gameMap[i][j];p++){
						string name = "floorcube";
						name.append(convertInt(i)+convertInt(j)+convertInt(p));
						createModel (name,(j-5)*150,p*150+150/2,(i-5)*150,150,150,150,"Models/cube.data","");
					}
				}
				else if(gameMapTrap[i][j]==6){ //Elevator needs 3 blocks space for doors to open
					string new_name = "startelevator";
					createModel (new_name,(j-5)*150,(gameMap[i][j])*150+79+300,(i-5)*150,70,100,70,"Models/elevator.data","");
					string elevatorblock = new_name+"back";
					createModel (elevatorblock,(j-5)*150+51,(gameMap[i][j])*150+78+300,(i-5)*150,10,150,130,"Models/cube.data","");
					elevatorblock = new_name+"left";
					createModel (elevatorblock,(j-5)*150,(gameMap[i][j])*150+78+300,(i-5)*150+62,130,150,10,"Models/cube.data","");
					elevatorblock = new_name+"right";
					createModel (elevatorblock,(j-5)*150,(gameMap[i][j])*150+78+300,(i-5)*150-62,130,150,10,"Models/cube.data","");
					elevatorblock = new_name+"top";
					createModel (elevatorblock,(j-5)*150,(gameMap[i][j])*150+78+85+300,(i-5)*150,130,10,130,"Models/cube.data","");
					elevatorblock = new_name+"bottom";
					createModel (elevatorblock,(j-5)*150,(gameMap[i][j])*150+78-85+300,(i-5)*150,130,10,130,"Models/cube.data","");
					objects[new_name].angle_y=180;
					int p;
					for(p=0;p<gameMap[i][j];p++){
						string name = "floorcube";
						name.append(convertInt(i)+convertInt(j)+convertInt(p));
						createModel (name,(j-5)*150,p*150+150/2,(i-5)*150,150,150,150,"Models/cube.data","");
					}
				}
				else if(gameMapTrap[i][j]==7){
					string new_name = "star";
					new_name.append(convertInt(i)+convertInt(j));
					createModel (new_name,(j-5)*150,gameMap[i][j]*150+75,(i-5)*150,20,20,20,"Models/star.data","");
					int p;
					for(p=0;p<gameMap[i][j];p++){
						string name = "floorcube";
						name.append(convertInt(i)+convertInt(j)+convertInt(p));
						createModel (name,(j-5)*150,p*150+150/2,(i-5)*150,150,150,150,"Models/cube.data","");
					}
				}
				else if(gameMapTrap[i][j]<0){
					int p;
					for(p=0;p<gameMap[i][j];p++){
						string name = "floorcube";
						name.append(convertInt(i)+convertInt(j)+convertInt(p));
						createModel (name,(j-5)*150,p*150+150/2,(i-5)*150,150,150,150,"Models/cube.data","");
						if(p==gameMap[i][j]-1){
							objects[name].isMovingAnim=-100*gameMapTrap[i][j];
						}
					}
				}
				else{
					int p;
					for(p=0;p<gameMap[i][j];p++){
						string name = "floorcube";
						name.append(convertInt(i)+convertInt(j)+convertInt(p));
						createModel (name,(j-5)*150,p*150+150/2,(i-5)*150,150,150,150,"Models/cube.data","");
					}
				}
			}
		}
	}

	createModel("player",0,300,100,63*scale,60*scale,42*scale,"Models/body.data",""); //The player's body
	createModel("playerhead",0,60*scale,0,28*scale,28*scale,28*scale,"Models/head.data","player");
	createModel("playerhand",49*scale,-2*scale,0,16*scale,32*scale,16*scale,"Models/hand.data","player");
	createModel("playerhand2",-49*scale,-2*scale,0,16*scale,32*scale,16*scale,"Models/hand.data","player");
	createModel("playerleg",15*scale,-68*scale,0,15*scale,48*scale,15*scale,"Models/leg.data","player");
	createModel("playerleg2",-15*scale,-68*scale,0,15*scale,48*scale,15*scale,"Models/leg.data","player");
	playerObjects["playerhand"].rotation_y_offset=-30*scale; //So that the rotation of the hand swinging is done on the top of the hand
	playerObjects["playerhand2"].rotation_y_offset=-30*scale;
	playerObjects["playerleg"].rotation_y_offset=-30*scale;
	playerObjects["playerleg2"].rotation_y_offset=-30*scale;

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL3.vert", "Sample_GL3.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);
	//glEnable(GL_LIGHTING);
//	glEnable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Initialise FTGL stuff
	//const char* fontfile = "/home/harsha/Classes/Two-2/Graphics/OGL3Sample2D/GLFW/GL3_Fonts_Textures/arial.ttf";
	const char* fontfile = "Fonts/arial.ttf";
	GL3Font.font = new FTExtrudeFont(fontfile); // 3D extrude style rendering

	if(GL3Font.font->Error())
	{
		cout << "Error: Could not load font `" << fontfile << "'" << endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// Create and compile our GLSL program from the font shaders
	fontProgramID = LoadShaders( "fontrender.vert", "fontrender.frag" );
	GLint fontVertexCoordAttrib, fontVertexNormalAttrib, fontVertexOffsetUniform;
	fontVertexCoordAttrib = glGetAttribLocation(fontProgramID, "vertexPosition");
	fontVertexNormalAttrib = glGetAttribLocation(fontProgramID, "vertexNormal");
	fontVertexOffsetUniform = glGetUniformLocation(fontProgramID, "pen");
	GL3Font.fontMatrixID = glGetUniformLocation(fontProgramID, "MVP");
	GL3Font.fontColorID = glGetUniformLocation(fontProgramID, "fontColor");

	GL3Font.font->ShaderLocations(fontVertexCoordAttrib, fontVertexNormalAttrib, fontVertexOffsetUniform);
	GL3Font.font->FaceSize(1);
	GL3Font.font->Depth(0);
	GL3Font.font->Outset(0, 0);
	GL3Font.font->CharMap(ft_encoding_unicode);

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

}

void goToNextLevel(GLFWwindow* window){
	currentLevel++; 
	//If next level does not exist, then the arrays will not be updated and you will be shown the last level again
	timeToStartLevel=150;
	timeToFinishLevel=0;
	objects.clear();
	playerObjects.clear();
	trapTimer=0;
	justInAir=0;
	player_speed=1.5;
	elevatorStartLevel=1;
    elevatorFinishLevel=0;
    timeToFinishLevel=0;
    thread(play_audio,"Sounds/teleport.mp3").detach();
	initGL(window,width,height);
}

//Set the audioFile parameter to the file name first before calling this function
void* play_audio(string audioFile){
	mpg123_handle *mh;
	unsigned char *buffer;
	size_t buffer_size;
	size_t done;
	int err;

	int driver;
	ao_device *dev;

	ao_sample_format format;
	int channels, encoding;
	long rate;

	/* initializations */
	ao_initialize();
	driver = ao_default_driver_id();
	mpg123_init();
	mh = mpg123_new(NULL, &err);
	buffer_size = mpg123_outblock(mh);
	buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

	/* open the file and get the decoding format */
	mpg123_open(mh, &audioFile[0]);
	mpg123_getformat(mh, &rate, &channels, &encoding);

	/* set the output format and open the output device */
	format.bits = mpg123_encsize(encoding) * 8;
	format.rate = rate;
	format.channels = channels;
	format.byte_format = AO_FMT_NATIVE;
	format.matrix = 0;
	dev = ao_open_live(driver, &format, NULL);

	/* decode and play */
	char *p =(char *)buffer;
	while (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK)
		ao_play(dev, p, done);

	/* clean up */
	free(buffer);
	ao_close(dev);
	mpg123_close(mh);
	mpg123_delete(mh);
}

time_t old_time;

int main (int argc, char** argv)
{
	srand(time(NULL));

	old_time = time(NULL);
    thread(play_audio,"Sounds/background.mp3").detach();

	width = 1400;
	height = 700;
	camera_radius=800;
	angle=135;
	eye_x = -50+camera_radius*cos(angle*M_PI/180);
	eye_y = 600;
    eye_z = -50+camera_radius*sin(angle*M_PI/180);

	GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

	currentLevel=-1;
	goToNextLevel(window);

	double last_update_time = glfwGetTime(), current_time;

	/* Draw in loop */
	while (!glfwWindowShouldClose(window)) {

		if(time(NULL)-old_time>=93){
			old_time=time(NULL);
			thread(play_audio,"Sounds/background.mp3").detach();
		}
		// OpenGL Draw commands
		draw(window);

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

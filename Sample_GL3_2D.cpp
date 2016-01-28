#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <cmath> 

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>
#include <FTGL/ftgl.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>

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
    float x_speed,y_speed;
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
int player_moving=0;
int player_rotating=0;

int gameMap[10][10]={
	{1,1,1,1,1,1,1,1,1,1},
	{1,1,1,2,1,1,1,1,1,1},
	{1,2,1,2,1,1,1,1,1,1},
	{1,2,1,1,2,2,2,2,2,2},
	{1,1,2,1,1,2,1,1,1,2},
	{1,1,2,1,1,1,1,1,1,2},
	{1,1,1,2,1,1,1,1,1,2},
	{1,1,1,1,2,1,1,1,1,1},
	{1,1,1,1,1,2,2,2,2,2},
	{1,1,1,1,1,1,1,1,1,1}
};

int gameMapPebbles[10][10]={
	{2,1,1,2,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1,1,1},
	{1,1,1,2,1,1,1,1,1,1},
	{1,1,1,1,1,2,1,1,2,1},
	{1,1,1,1,1,1,1,1,1,2},
	{1,1,2,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,2,1,2},
	{1,1,1,2,1,1,1,1,1,1}	
};


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

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.

	if (action == GLFW_RELEASE) {
		switch (key) {
			case GLFW_KEY_UP:
				player_moving=0;
				break;
			case GLFW_KEY_DOWN:
				player_moving=0;
				break;
			case GLFW_KEY_RIGHT:
				player_rotating=0;
				break;
			case GLFW_KEY_LEFT:
				player_rotating=0;
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
				player_rotating=1;
				break;
			case GLFW_KEY_LEFT:
				player_rotating=-1;
				break;
			case GLFW_KEY_UP:
				player_moving=1;
				break;
			case GLFW_KEY_DOWN:
				player_moving=-1;
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

float eye_x,eye_y,eye_z;
int left_mouse_clicked;

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
			if (action == GLFW_RELEASE) {
				rectangle_rot_dir *= -1;
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

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	 glLoadIdentity ();
	 gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	// Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

	// Ortho projection for 2D views
	Matrices.projection = glm::ortho(-400.0f, 400.0f, -400.0f, 400.0f, -1000.0f, 5000.0f);
}

VAO *triangle, *rectangle;

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
void createRectangle (GLuint textureID)
{
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		-140,-100,0, // vertex 1
		140,-100,0, // vertex 2
		140, 100,0, // vertex 3

		140, 100,0, // vertex 3
		-140, 100,0, // vertex 4
		-140,-100,0  // vertex 1
	};

	static const GLfloat color_buffer_data [] = {
		1,0,0, // color 1
		0,0,1, // color 2
		0,1,0, // color 3

		0,1,0, // color 3
		0.3,0.3,0.3, // color 4
		1,0,0  // color 1
	};

	// Texture coordinates start with (0,0) at top left of the image to (1,1) at bot right
	static const GLfloat texture_buffer_data [] = {
		0,1, // TexCoord 1 - bot left
		1,1, // TexCoord 2 - bot right
		1,0, // TexCoord 3 - top right

		1,0, // TexCoord 3 - top right
		0,0, // TexCoord 4 - top left
		0,1  // TexCoord 1 - bot left
	};

	// create3DTexturedObject creates and returns a handle to a VAO that can be used later
	rectangle = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data, texture_buffer_data, textureID, GL_FILL);
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
float angle=0;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw (GLFWwindow* window)
{
	if(player_rotating==1){
		objects["player"].angle_y-=2;
	}
	if(player_rotating==-1){
		objects["player"].angle_y+=2;
	}
	if(player_moving==1){
		objects["player"].z+=cos(objects["player"].angle_y*M_PI/180)*2;
		objects["player"].x+=sin(objects["player"].angle_y*M_PI/180)*2;
	}
	if(player_moving==-1){
		objects["player"].z-=cos(objects["player"].angle_y*M_PI/180)*2;
		objects["player"].x-=sin(objects["player"].angle_y*M_PI/180)*2;
	}
	if(player_moving==1 || player_moving==-1){ //The player is not stationary
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
	else{
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
	double new_mouse_x,new_mouse_y;
	glfwGetCursorPos(window,&new_mouse_x,&new_mouse_y);
	if(left_mouse_clicked==1){
		angle+=0.01;
		eye_x = 400*cos(angle);
    	eye_z = 400*sin(angle);
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
	glm::vec3 target (0, 0, 0);
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
	//draw3DTexturedObject(rectangle);



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
	GL3Font.font->Render("Round n Round we go !!");




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

	return window;
}

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

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	// Load Textures
	// Enable Texture0 as current texture memory
	glActiveTexture(GL_TEXTURE0);
	// load an image file directly as a new OpenGL texture
	// GLuint texID = SOIL_load_OGL_texture ("beach.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_TEXTURE_REPEATS); // Buggy for OpenGL3
	GLuint textureID = createTexture("beach2.png");
	// check for an error during the load process
	if(textureID == 0 )
		cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;

	// Create and compile our GLSL program from the texture shaders
	textureProgramID = LoadShaders( "TextureRender.vert", "TextureRender.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.TexMatrixID = glGetUniformLocation(textureProgramID, "MVP");


	/* Objects should be created before any other gl function and shaders */
	// Create the models
	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	createRectangle (textureID);


	int i,j,k;
	for(i=0;i<10;i++){
		for(j=0;j<10;j++){
			if(gameMap[i][j]!=0){
				string name = "floorcube";
				name.append(convertInt(i)+convertInt(j));
				createModel (name,(j-5)*50,gameMap[i][j]*50/2,(i-5)*50,50,gameMap[i][j]*50,50,"cube.data","");
				if(gameMapPebbles[i][j]==2){
					string new_name="stone";
					new_name.append(name);
					createModel (new_name,(j-5)*50,(gameMap[i][j]-1)*50+50,(i-5)*50,120,120,120,"stone.data","");
				}
			}
		}
	}

	createModel("player",0,200,50,62,60,40,"body.data",""); //The player's body
	createModel("playerhead",0,60,0,28,28,28,"head.data","player");
	createModel("playerhand",49,-2,0,16,32,16,"hand.data","player");
	createModel("playerhand2",-49,-2,0,16,32,16,"hand.data","player");
	createModel("playerleg",15,-58,0,30,74,30,"cube.data","player");
	createModel("playerleg2",-15,-58,0,30,74,30,"cube.data","player");
	playerObjects["playerhand"].rotation_y_offset=-30; //So that the rotation of the hand swinging is done on the top of the hand
	playerObjects["playerhand2"].rotation_y_offset=-30;
	playerObjects["playerleg"].rotation_y_offset=-30;
	playerObjects["playerleg2"].rotation_y_offset=-30;

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
//	glEnable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Initialise FTGL stuff
	//const char* fontfile = "/home/harsha/Classes/Two-2/Graphics/OGL3Sample2D/GLFW/GL3_Fonts_Textures/arial.ttf";
	const char* fontfile = "arial.ttf";
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

int main (int argc, char** argv)
{
	int width = 600;
	int height = 600;
	eye_x=400;
	eye_y=0;
	eye_z=-400;

	GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

	double last_update_time = glfwGetTime(), current_time;

	/* Draw in loop */
	while (!glfwWindowShouldClose(window)) {

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

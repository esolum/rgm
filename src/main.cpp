/* Base code for texture mapping lab */
/* includes three images and three meshes - Z. Wood 2016 */
#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "Texture.h"


using namespace std;
using namespace Eigen;

GLFWwindow *window; // Main application window
string RESOURCE_DIR = "../resources/"; // Where the resources are loaded from
shared_ptr<Program> prog0;
shared_ptr<Program> prog1;
shared_ptr<Program> prog2;
shared_ptr<Program> lampProg;
shared_ptr<Program> boardProg;
shared_ptr<Shape> world;
shared_ptr<Shape> shape;
shared_ptr<Shape> lamp;
shared_ptr<Shape> board;
shared_ptr<Shape> ball;


// OpenGL handle to texture data
Texture texture0;
GLint h_texture0;

Texture texture1;
GLint h_texture1;

Texture texture2;
GLint h_texture2;

Texture texture3;
GLint h_texture3;

int numLights;
float offset = 0.0;
int g_GiboLen;
int g_width, g_height;
float cTheta = 0;
float cHeight = 0;
float camOffsetX = 0;
float pitchScale;
float yawScale;
float phi = 0;
float theta = 0;
float radius = 20.0;
float lastYPos = -1;
float lastXPos = -1;
float startingXpos, startingYpos;
bool active = false;
Vector3f eye((float)0, (float) 3, (float)15);
Vector3f up((float)0.0, (float) 1.0, (float)0.0);
Vector3f la(0, 0, 0);
Vector3f w(3);
Vector3f u(3);
Vector3f v(3);

//global data for ground plane
GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;

static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
   }
    /*else if (key == GLFW_KEY_A && action == GLFW_PRESS) {
      cTheta += 5;
   } else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
      cTheta -= 5;
   } else if (key == GLFW_KEY_W && action == GLFW_PRESS) {
      cHeight += .5;
   } else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
      cHeight -= 0.5;
   }*/
    
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    //Move light to the left
    else if(key == GLFW_KEY_Q && action == GLFW_PRESS) {
        offset -= 1;
    }
    //Move light to the right
    else if(key == GLFW_KEY_E && action == GLFW_PRESS) {
        offset += 1;
    }

    //Move left
    else if(key == GLFW_KEY_A && action == GLFW_PRESS) {
        eye = eye - 0.5*u;
        la = la - 0.5*u;
    }
    //Move right
    else if(key == GLFW_KEY_D && action == GLFW_PRESS) {
        eye = eye + 0.5*u;
        la = la + 0.5*u;
    }
    //Zoom in
    else if(key == GLFW_KEY_W && action == GLFW_PRESS) {
        eye = eye - 0.5*w;
        la = la - 0.5*w;
    }
    //Zoom out
    else if(key == GLFW_KEY_S && action == GLFW_PRESS) {
        eye = eye + 0.5*w;
        la = la + 0.5*w;
    }
}

//Callback function that tracks the cursor position.
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    
    // Tracking position it's moved at least once before
    if(active && lastYPos != -1 && lastXPos != -1) {
        float increasePhi = (float) pitchScale * (ypos - lastYPos);
        float increaseTheta = (float) yawScale * (lastXPos - xpos);
        
        // Cap the movement at 160 degrees and add the changes.
        if(phi + increasePhi < 2.79253 && phi + increasePhi > -2.79253) {
            phi += increasePhi;
            lastYPos = ypos;
        }
        
        theta += increaseTheta;
        lastXPos = xpos;
        
        
        //Update the look at point after changing the view.
        la(0) = (float)radius*cos(phi)*cos(theta);
        la(1) = (float)radius*sin(phi);
        la(2) = (float)radius*cos(phi)*cos(90.0 - theta);
    }
    
    //Start tracking its last position if we haven't already.
    else if(active) {
        lastYPos = ypos;
        lastXPos = xpos;
    }
}

static void mouse_callback(GLFWwindow *window, int button, int action, int mods)
{
    //Start tracking the cursor position
    if (action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        
        startingXpos = xpos;
        startingYpos = ypos;
        active = true;
    }
    //Upon release, stop tracking the cursor and reset the last cursor positions
    if(action == GLFW_RELEASE) {
        
        active = false;
        
        lastXPos = -1;
        lastYPos = -1;
    }
}


float p2wx(double in_x, float left) {
	float c = (-2*left)/(g_width-1.0);
	float d = left;
   return c*in_x+d;
}

float p2wy(double in_y, float bot) {
	//flip y
  	in_y = g_height -in_y;
	float e = (-2*bot)/(g_height-1.0);
	float f = bot;
   return e*in_y + f;
}

static void resize_callback(GLFWwindow *window, int width, int height) {
	g_width = width;
	g_height = height;
	glViewport(0, 0, width, height);
}

/* code to define the ground plane */
static void initGeom() {

   float g_groundSize = 10;
   float g_groundY = -1.5;

  // A x-z plane at y = g_groundY of dimension [-g_groundSize, g_groundSize]^2
    float GrndPos[] = {
    -g_groundSize, g_groundY, -g_groundSize,
    -g_groundSize, g_groundY,  g_groundSize,
     g_groundSize, g_groundY,  g_groundSize,
     g_groundSize, g_groundY, -g_groundSize
    };

    float GrndNorm[] = {
     0, 1, 0,
     0, 1, 0,
     0, 1, 0,
     0, 1, 0,
     0, 1, 0,
     0, 1, 0
    };


  static GLfloat GrndTex[] = {
      0, 0, // back
      0, 30,
      30, 30,
      30, 0 };

    unsigned short idx[] = {0, 1, 2, 0, 2, 3};


   GLuint VertexArrayID;
	//generate the VAO
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

    g_GiboLen = 6;
    glGenBuffers(1, &GrndBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_STATIC_DRAW);

    glGenBuffers(1, &GrndNorBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GrndNorm), GrndNorm, GL_STATIC_DRAW);
    
	 glGenBuffers(1, &GrndTexBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GrndTex), GrndTex, GL_STATIC_DRAW);

    glGenBuffers(1, &GIndxBuffObj);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

}

static void init()
{
	GLSL::checkVersion();
    numLights = 1;
	// Set background color.
	glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Initialize mesh.
   /*shape = make_shared<Shape>();
   shape->loadMesh(RESOURCE_DIR + "dog.obj");
   shape->resize();
   shape->init();
   
	world = make_shared<Shape>();
   world->loadMesh(RESOURCE_DIR + "sphere.obj");
   world->resize();
   world->init(); */
    string filepathMTL = RESOURCE_DIR + "garl.mtl";
    
    lamp = make_shared<Shape>();
    lamp->loadMesh(RESOURCE_DIR + "garl.obj", RESOURCE_DIR);
    lamp->resize();
    lamp->init();
    
    board = make_shared<Shape>();
    board->loadMesh(RESOURCE_DIR + "cube.obj", RESOURCE_DIR);
    board->resize();
    board->init();
    
    ball = make_shared<Shape>();
    ball->loadMesh(RESOURCE_DIR + "sphere.obj", RESOURCE_DIR);
    ball->resize();
    ball->init();
    
    //Load material files


	// Initialize the GLSL programs
	prog0 = make_shared<Program>();
	prog0->setVerbose(true);
	prog0->setShaderNames(RESOURCE_DIR + "tex_vert.glsl", RESOURCE_DIR + "tex_frag0.glsl");
	prog0->init();
	
	prog1 = make_shared<Program>();
	prog1->setVerbose(true);
	prog1->setShaderNames(RESOURCE_DIR + "tex_vert.glsl", RESOURCE_DIR + "tex_frag1.glsl");
	prog1->init();
  
	prog2 = make_shared<Program>();
	prog2->setVerbose(true);
	prog2->setShaderNames(RESOURCE_DIR + "tex_vert.glsl", RESOURCE_DIR + "tex_frag2.glsl");
	prog2->init();
    
    lampProg = make_shared<Program>();
    lampProg->setVerbose(true);
    lampProg->setShaderNames(RESOURCE_DIR + "lamp_vert.glsl", RESOURCE_DIR + "lamp_frag.glsl");
    lampProg->init();
    
    boardProg = make_shared<Program>();
    boardProg->setVerbose(true);
    boardProg->setShaderNames(RESOURCE_DIR + "board_vert.glsl", RESOURCE_DIR + "board_frag.glsl");
    boardProg->init();
	
	texture0.setFilename(RESOURCE_DIR + "fur3.bmp");
  	texture1.setFilename(RESOURCE_DIR + "world.bmp");
  	texture2.setFilename(RESOURCE_DIR + "grass.bmp");
    texture3.setFilename(RESOURCE_DIR + "wood.bmp");
    
   //////////////////////////////////////////////////////
   // Intialize textures
   //////////////////////////////////////////////////////
   texture0.setUnit(0);
   texture0.setName("Texture0");
   texture0.init();
	
	texture1.setUnit(1);
   texture1.setName("Texture1");
   texture1.init();
	
	texture2.setUnit(2);
   texture2.setName("Texture2");
   texture2.init();
    
    texture3.setUnit(3);
    texture3.setName("Texture3");
    texture3.init();

	GLSL::printError();

	/// Add uniform and attributes to each of the programs
	prog0->addUniform("P");
	prog0->addUniform("M");
    prog0->addUniform("V");
	prog0->addUniform("Texture0");
	prog0->addAttribute("vertPos");
    prog0->addAttribute("vertNor");
	prog0->addAttribute("vertTex");
    prog0->addAttribute("lightPosition");
	prog0->addTexture(&texture0);
	
	prog1->addUniform("P");
	prog1->addUniform("M");
    prog1->addUniform("V");
	prog1->addUniform("Texture1");
	prog1->addAttribute("vertPos");
    prog1->addAttribute("vertNor");
	prog1->addAttribute("vertTex");
    prog1->addAttribute("lightPosition");
	prog1->addTexture(&texture1);
	
	prog2->addUniform("P");
	prog2->addUniform("M");
    prog2->addUniform("V");
	prog2->addUniform("Texture2");
	prog2->addAttribute("vertPos");
    prog2->addAttribute("vertNor");
	prog2->addAttribute("vertTex");
    prog2->addAttribute("lightPosition");
	prog2->addTexture(&texture2);
    
    lampProg->addUniform("P");
    lampProg->addUniform("M");
    lampProg->addUniform("V");
    //lampProg->addUniform("Texture3");
    //lampProg->addUniform("textOn");
    lampProg->addAttribute("lightPosition");
    lampProg->addUniform("MatAmb");
    lampProg->addUniform("MatDif");
    lampProg->addUniform("Spec");
    lampProg->addUniform("Shine");
    lampProg->addAttribute("vertPos");
    lampProg->addAttribute("vertNor");
    lampProg->addAttribute("vertTex");
    lampProg->addAttribute("normalShowing");
    //lampProg->addTexture(&texture3);
    
    boardProg->addUniform("P");
    boardProg->addUniform("M");
    boardProg->addUniform("V");
    //boardProg->addUniform("Texture3");
    //boardProg->addUniform("textOn");
    boardProg->addAttribute("lightPosition");
    boardProg->addUniform("MatAmb");
    boardProg->addUniform("MatDif");
    boardProg->addUniform("Spec");
    boardProg->addUniform("Shine");
    boardProg->addAttribute("vertPos");
    boardProg->addAttribute("vertNor");
    boardProg->addAttribute("vertTex");
    boardProg->addAttribute("normalShowing");
    //boardProg->addTexture(&texture3);

}


/****DRAW
This is the most important function in your program - this is where you
will actually issue the commands to draw any geometry you have set up to
draw
********/
static void render()
{
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float aspect = width/(float)height;
	glViewport(0, 0, width, height);
    float lightPosition[] = {(float)3.0 + offset, (float)2.0, (float)-1};
    //cout << "EYE POS: " << eye.x() << " " << eye.y() << " " << eye.z() << endl;
    //cout << "LA: " << la.x() << " " << la.y() << " " << la.z() << endl;
    //Compute the scale for the pitch and yaw depending on the height
    pitchScale = (float)180/height * (float)(M_PI/180);
    yawScale = (float)180/width * (float)(M_PI/180);
    
    //Compute gaze and the camera's coordinate frame
    Vector3f gaze = la - eye;
    gaze.normalize();
    w(0) = -gaze.x();
    w(1) = -gaze.y();
    w(2) = -gaze.z();
    
    u = up.cross(w);
    u.normalize();
    
    v = w.cross(u);

	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// Create the matrix stacks 
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();
    auto M = make_shared<MatrixStack>();
    auto V = make_shared<MatrixStack>();
	P->pushMatrix();
   P->perspective(45.0f, aspect, 0.01f, 100.0f);
    
    V->pushMatrix();
    V->lookAt(eye, la, up);
    
    
	
    //Draw the first lamp
    lampProg->bind();
    glUniformMatrix4fv(lampProg->getUniform("P"), 1, GL_FALSE, P->topMatrix().data());
    glUniformMatrix4fv(lampProg->getUniform("V"), 1, GL_FALSE, V->topMatrix().data());
    glVertexAttrib3fv(lampProg->getAttribute("lightPosition"), lightPosition);
    M->pushMatrix();
    
        M->loadIdentity();
        M->translate(Vector3f(6, 0.32, -3));
        M->rotate(180, Vector3f(0, 1, 0));
        M->scale(Vector3f(7, 7, 7));
        //MV->rotate(cTheta, Vector3f(0, 1, 0));
        glUniformMatrix4fv(lampProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
    
        lamp->draw(lampProg);
    M->popMatrix();
    
    M->pushMatrix();
    
        M->loadIdentity();
        M->translate(Vector3f(6, 0.32, 3));
        M->rotate(180, Vector3f(0, 1, 0));
        M->scale(Vector3f(7, 7, 7));
        //MV->rotate(cTheta, Vector3f(0, 1, 0));
        glUniformMatrix4fv(lampProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
        
        lamp->draw(lampProg);
    M->popMatrix();
    
    M->pushMatrix();
    
        M->loadIdentity();
        M->translate(Vector3f(-6, 0.32, 3));
        M->rotate(180, Vector3f(0, 1, 0));
        M->scale(Vector3f(7, 7, 7));
        //MV->rotate(cTheta, Vector3f(0, 1, 0));
        glUniformMatrix4fv(lampProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
        
        lamp->draw(lampProg);
    M->popMatrix();
    
    M->pushMatrix();
    
        M->loadIdentity();
        M->translate(Vector3f(-6, 0.32, -3));
        M->rotate(180, Vector3f(0, 1, 0));
        M->scale(Vector3f(7, 7, 7));
        //MV->rotate(cTheta, Vector3f(0, 1, 0));
        glUniformMatrix4fv(lampProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
        
        lamp->draw(lampProg);
    M->popMatrix();
    lampProg->unbind();
    
    
    
    
    
    //Draw marble track
    /*marbleProg->bind();
    glUniformMatrix4fv(marbleProg->getUniform("P"), 1, GL_FALSE, P->topMatrix().data());
    //glUniformMatrix4fv(marbleProg->getUniform("V"), 1, GL_FALSE, V->topMatrix().data());
    //glVertexAttrib3fv(marbleProg->getAttribute("lightPosition"), lightPosition);
    MV->pushMatrix();
    
    MV->loadIdentity();
    MV->translate(Vector3f(1, 0, -4));
    //MV->rotate(180, Vector3f(0, 1, 0));
    MV->scale(Vector3f(1, 1, 1));
    //MV->rotate(cTheta, Vector3f(0, 1, 0));
    glUniformMatrix4fv(marbleProg->getUniform("MV"), 1, GL_FALSE, MV->topMatrix().data());
    
    marbletrack->draw(marbleProg);
    MV->popMatrix();
    marbleProg->unbind();*/
    
    
    //draw the dog mesh
	/*prog0->bind();
	glUniformMatrix4fv(prog0->getUniform("P"), 1, GL_FALSE, P->topMatrix().data());

	MV->pushMatrix();

	MV->loadIdentity();
   MV->translate(Vector3f(0, cHeight, -5));
	
	MV->pushMatrix();
   MV->translate(Vector3f(-1, 0, 0));
   MV->rotate(cTheta, Vector3f(0, 1, 0));
	glUniformMatrix4fv(prog0->getUniform("MV"), 1, GL_FALSE, MV->topMatrix().data());

   shape->draw(prog0);
	MV->popMatrix();
	prog0->unbind();

	//draw the world sphere	
	prog1->bind();
	glUniformMatrix4fv(prog1->getUniform("P"), 1, GL_FALSE, P->topMatrix().data());
	
	MV->pushMatrix();
	MV->translate(Vector3f(1, 0, 0));
   MV->rotate(cTheta, Vector3f(0, 1, 0));
	glUniformMatrix4fv(prog1->getUniform("MV"), 1, GL_FALSE, MV->topMatrix().data());
    
	world->draw(prog1);
	
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	MV->popMatrix();
	prog1->unbind();
     */
	//draw the ground plane	
	prog2->bind();
    M->pushMatrix();
    M->loadIdentity();
	glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, P->topMatrix().data());
    glUniformMatrix4fv(lampProg->getUniform("V"), 1, GL_FALSE, V->topMatrix().data());
	glUniformMatrix4fv(prog2->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
    glVertexAttrib3fv(lampProg->getAttribute("lightPosition"), lightPosition);

	glEnableVertexAttribArray(0);
   glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(1);
   glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	 
	glEnableVertexAttribArray(2);
   glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
   glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

   // draw!
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
   glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_SHORT, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	
	prog2->unbind();

	// Pop matrix stacks.
    M->popMatrix();
    V->popMatrix();
	P->popMatrix();
}

int main(int argc, char **argv)
{

	g_width = 960;
	g_height = 720;
	/* we will always need to load external shaders to set up where 
	if(argc < 2) {
      cout << "Please specify the resource directory." << endl;
      return 0;
   }
   RESOURCE_DIR = argv[1] + string("/");
	*/

	/* your main will always include a similar set up to establish your window
      and GL context, etc. */
    
    
    

	// Set error callback as openGL will report errors but they need a call back
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
	//request the highest possible version of OGL - important for mac
	
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(g_width, g_height, "textures", NULL, NULL);
	if(!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}

	glGetError();
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
    //set the mouse call back
    glfwSetMouseButtonCallback(window, mouse_callback);
    //set the cursor callback
    glfwSetCursorPosCallback(window, cursor_position_callback);
	//set the window resize call back
	glfwSetFramebufferSizeCallback(window, resize_callback);

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	init();
	initGeom();

	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		// Render scene.
		render();
		// Swap front and back buffers.
		glfwSwapBuffers(window);
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

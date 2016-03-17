/* Rube Goldberg Machine Simulation - Erica Solum
   Includes Base code for texture mapping lab
   includes three images and three meshes - Z. Wood 2016 */
#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "Texture.h"

#define PI 3.14159
using namespace std;
using namespace Eigen;

GLFWwindow *window; // Main application window
string RESOURCE_DIR = "../resources/"; // Where the resources are loaded from
shared_ptr<Program> prog0;
shared_ptr<Program> prog1;
shared_ptr<Program> prog2;
shared_ptr<Program> lampProg;
shared_ptr<Program> boardProg;
shared_ptr<Program> ballProg;
shared_ptr<Program> wallProg;
shared_ptr<Shape> world;
shared_ptr<Shape> shape;
shared_ptr<Shape> lamp;
shared_ptr<Shape> board;
shared_ptr<Shape> ball;
shared_ptr<Shape> cube;
shared_ptr<Shape> button;

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
float bRot = 0;
float t = 0;
Vector3f ballStart(-3.8, 1.83, 0);
Vector3f ballEnd(-3.8, 1.83, 0);
Vector3f ballPos(-3.8, 1.83, 0);
Vector3f lastBallPos(3);

//MARK: Lights
GLfloat light1[] = {6, 1.17, 3};
GLfloat light2[] = {6, 1.17, -3};
GLfloat light3[] = {-6, 1.17, 3};
GLfloat light4[] = {-6, 1.17, -3};
GLfloat lightOn[] = {1.0, 1.0, 1.0};
GLfloat lightOff[] = {0, 0, 0};


//Physics sturf
float mass;
float gravity = -9.8;

Vector3f eye((float)0, (float) 3, (float)15);
Vector3f up((float)0.0, (float) 1.0, (float)0.0);
Vector3f la(0, 0, 0);
Vector3f w(3);
Vector3f u(3);
Vector3f v(3);

//global data for ground plane
GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;

struct PointLight {
    Vector3f position;
    
    float constant;
    float linear;
    float quadratic;
    
    Vector3f ambient;
    Vector3f diffuse;
    Vector3f specular;
};

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
    //LET'S GET THE BALL ROLLING
    //But actually
    else if(key == GLFW_KEY_B && action == GLFW_PRESS) {
        ballEnd(0) = 3.8;
        ballEnd(1) = 0.48;
        ballEnd(2) = 0;
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

void checkError(string msg) {
    GLenum gr2 = glGetError();
    if(gr2 != GL_NO_ERROR) {
        cout << msg << ": OpenGL Error: " << gr2 << endl;
    }
}

void computePhysics() {
    Vector3f path = ballEnd - ballStart;
    lastBallPos = ballPos;
    ballPos = ballPos + t*path;
    
    float distance = sqrt(pow((ballPos.x()-ballStart.x()), 2) + pow((ballPos.y()-ballStart.y()), 2));
    
    bRot = -distance* 2 * (180/PI);
    
    if(ballPos.x() > -1.9 && ballPos.x() < 0 ) {
        numLights = 2;
    }
    if(ballPos.x() > 0 && ballPos.x() < 1.9 ) {
        numLights = 3;
    }
    if(ballPos.x() > 1.9 && ballPos.x() < 3.8 ) {
        numLights = 4;
    }
}

void setUpLights(const shared_ptr<Program> prog) {
    checkError("Before lights");
    
    
    if(numLights >= 2) {
        glUniform1f(prog->getUniform("lightColor2"), 1.0f);
    }
    else {
        glUniform1f(prog->getUniform("lightColor2"), 0.0f);;
    }
    
    if(numLights >= 3) {
        glUniform1f(prog->getUniform("lightColor3"), 1.0f);
    }
    else {
        glUniform1f(prog->getUniform("lightColor3"), 0.0f);
    }
    
    if(numLights >= 4) {
        glUniform1f(prog->getUniform("lightColor4"), 1.0f);
    }
    else {
        glUniform1f(prog->getUniform("lightColor4"), 0.0f);
    }
    checkError("After passing lights");
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
      0, 5,
      5, 5,
      5, 0 };

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
    ball->loadMesh(RESOURCE_DIR + "bs.obj", RESOURCE_DIR);
    ball->resize();
    ball->init();
    
    cube = make_shared<Shape>();
    cube->loadMesh(RESOURCE_DIR + "cube.obj", RESOURCE_DIR);
    cube->resize();
    cube->init();
    
    button = make_shared<Shape>();
    button->loadMesh(RESOURCE_DIR + "cube.obj", RESOURCE_DIR);
    button->resize();
    button->init();
    
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
    
    ballProg = make_shared<Program>();
    ballProg->setVerbose(true);
    ballProg->setShaderNames(RESOURCE_DIR + "ball_vert.glsl", RESOURCE_DIR + "ball_frag.glsl");
    ballProg->init();
    
    wallProg = make_shared<Program>();
    wallProg->setVerbose(true);
    wallProg->setShaderNames(RESOURCE_DIR + "wall_vert.glsl", RESOURCE_DIR + "wall_frag.glsl");
    wallProg->init();
	
	texture0.setFilename(RESOURCE_DIR + "fur3.bmp");
  	texture1.setFilename(RESOURCE_DIR + "world.bmp");
  	texture2.setFilename(RESOURCE_DIR + "wood.bmp");
    texture3.setFilename(RESOURCE_DIR + "bs_diff.bmp");
    
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
    
    // MARK: Lamp
    lampProg->addUniform("P");
    lampProg->addUniform("M");
    lampProg->addUniform("V");
    //lampProg->addUniform("Texture3");
    //lampProg->addUniform("textOn");
    lampProg->addUniform("pointLight1");
    lampProg->addUniform("pointLight2");
    lampProg->addUniform("pointLight3");
    lampProg->addUniform("pointLight4");
    lampProg->addUniform("lightColor2");
    lampProg->addUniform("lightColor3");
    lampProg->addUniform("lightColor4");
    lampProg->addUniform("MatAmb");
    lampProg->addUniform("MatDif");
    lampProg->addUniform("Spec");
    lampProg->addUniform("Shine");
    lampProg->addAttribute("vertPos");
    lampProg->addAttribute("vertNor");
    lampProg->addAttribute("vertTex");
    //lampProg->addTexture(&texture3);
    
    // MARK: Ramp
    boardProg->addUniform("P");
    boardProg->addUniform("M");
    boardProg->addUniform("V");
    boardProg->addUniform("pointLight1");
    boardProg->addUniform("pointLight2");
    boardProg->addUniform("pointLight3");
    boardProg->addUniform("pointLight4");
    boardProg->addUniform("lightColor2");
    boardProg->addUniform("lightColor3");
    boardProg->addUniform("lightColor4");
    //boardProg->addUniform("Texture3");
    //boardProg->addUniform("textOn");
    //boardProg->addAttribute("lightPosition");
    boardProg->addUniform("MatAmb");
    boardProg->addUniform("MatDif");
    boardProg->addUniform("Spec");
    boardProg->addUniform("Shine");
    boardProg->addAttribute("vertPos");
    boardProg->addAttribute("vertNor");
    boardProg->addAttribute("vertTex");
    //boardProg->addTexture(&texture3);
    
    // MARK: Ball
    ballProg->addUniform("P");
    ballProg->addUniform("M");
    ballProg->addUniform("V");
    ballProg->addUniform("Texture3");
    ballProg->addUniform("pointLight1");
    ballProg->addUniform("pointLight2");
    ballProg->addUniform("pointLight3");
    ballProg->addUniform("pointLight4");
    ballProg->addUniform("lightColor2");
    ballProg->addUniform("lightColor3");
    ballProg->addUniform("lightColor4");
    ballProg->addAttribute("vertPos");
    ballProg->addAttribute("vertNor");
    ballProg->addAttribute("vertTex");
    ballProg->addTexture(&texture3);
    
    // MARK: Wall
    wallProg->addUniform("P");
    wallProg->addUniform("M");
    wallProg->addUniform("V");
    wallProg->addUniform("pointLight1");
    wallProg->addUniform("pointLight2");
    wallProg->addUniform("pointLight3");
    wallProg->addUniform("pointLight4");
    wallProg->addUniform("lightColor2");
    wallProg->addUniform("lightColor3");
    wallProg->addUniform("lightColor4");
    //wallProg->addUniform("Texture3");
    //wallProg->addUniform("textOn");
    //wallProg->addAttribute("lightPosition");
    wallProg->addUniform("MatAmb");
    wallProg->addUniform("MatDif");
    wallProg->addUniform("Spec");
    wallProg->addUniform("Shine");
    wallProg->addAttribute("vertPos");
    wallProg->addAttribute("vertNor");
    wallProg->addAttribute("vertTex");
    //wallProg->addTexture(&texture3);

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
    t += 0.00001;
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
    
    computePhysics();

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
    // MARK: Lamps
    lampProg->bind();
    glUniformMatrix4fv(lampProg->getUniform("P"), 1, GL_FALSE, P->topMatrix().data());
    glUniformMatrix4fv(lampProg->getUniform("V"), 1, GL_FALSE, V->topMatrix().data());
    setUpLights(lampProg);
    
     M->loadIdentity();
     M->pushMatrix();
        M->translate(Vector3f(6, 1.17, -3));
        M->rotate(180, Vector3f(0, 1, 0));
        M->scale(Vector3f(10, 10, 10));
        //MV->rotate(cTheta, Vector3f(0, 1, 0));
    
        glUniformMatrix4fv(lampProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
    
        lamp->draw(lampProg);
    M->popMatrix();
    
    M->pushMatrix();
        M->translate(Vector3f(6, 1.17, 3));
        M->rotate(180, Vector3f(0, 1, 0));
        M->scale(Vector3f(10, 10, 10));
        //MV->rotate(cTheta, Vector3f(0, 1, 0));
    
        glUniformMatrix4fv(lampProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
        
        lamp->draw(lampProg);
    M->popMatrix();
    
    M->pushMatrix();
        M->translate(Vector3f(-6, 1.17, 3));
        M->rotate(180, Vector3f(0, 1, 0));
        M->scale(Vector3f(10, 10, 10));
        //MV->rotate(cTheta, Vector3f(0, 1, 0));
        glUniformMatrix4fv(lampProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
        
        lamp->draw(lampProg);
    M->popMatrix();
    
    M->pushMatrix();
        M->translate(Vector3f(-6, 1.17, -3));
        M->rotate(180, Vector3f(0, 1, 0));
        M->scale(Vector3f(10, 10, 10));
        //MV->rotate(cTheta, Vector3f(0, 1, 0));
    
        glUniformMatrix4fv(lampProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
        
        lamp->draw(lampProg);
    M->popMatrix();
    lampProg->unbind();
    
    M->pushMatrix();
        //Global translation in case I want to move it later
        M->translate(Vector3f(0, -0.5, 0));
        
        /* DRAW THE RAMP */
        // MARK: Ramp
        boardProg->bind();
            glUniformMatrix4fv(boardProg->getUniform("P"), 1, GL_FALSE, P->topMatrix().data());
            glUniformMatrix4fv(boardProg->getUniform("V"), 1, GL_FALSE, V->topMatrix().data());
            //glVertexAttrib3fv(boardProg->getAttribute("lightPosition"), lightPosition);
            setUpLights(boardProg);
            glUniform3f(boardProg->getUniform("MatAmb"), 0.000000, 0.000000, 0.000000);
            glUniform3f(boardProg->getUniform("MatDif"), 0.696471, 0.097255, 0.097255);
            glUniform3f(boardProg->getUniform("Spec"), 0.500000, 0.500000, 0.500000);
            glUniform1f(boardProg->getUniform("Shine"), 96.078431);
            M->pushMatrix();
                M->loadIdentity();
    
    
        
                M->pushMatrix();
                    M->rotate(-10, Vector3f(0, 0, 1));
                    M->scale(Vector3f(4, 0.1, 1));
    
                    glUniformMatrix4fv(boardProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
                    board->draw(boardProg);
                M->popMatrix();
        
                //Code for the legs
                M->pushMatrix();
        
                    M->pushMatrix();
                        M->translate(Vector3f(-3.8, -0.2, 0.8));
                        M->scale(Vector3f(0.1, 0.8, 0.1));
                        glUniformMatrix4fv(boardProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
                        board->draw(boardProg);
                    M->popMatrix();
                    M->translate(Vector3f(-3.8, -0.2, -0.8));
                    M->scale(Vector3f(0.1, 0.8, 0.1));
        
                    glUniformMatrix4fv(boardProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
                    board->draw(boardProg);
                M->popMatrix();
        
        
        
            M->popMatrix();
            M->pushMatrix();
    
    
                M->rotate(-10, Vector3f(0, 0, 1));
                M->pushMatrix();
                    M->translate(Vector3f(-1.9, 0.6, -0.1));
                    M->scale(Vector3f(0.1, 0.1, 0.1));
                    if(numLights >= 2) {
                        glUniform3f(boardProg->getUniform("MatAmb"), 0.5, 0.5, 0.5);
                        glUniform3f(boardProg->getUniform("MatDif"), 0.696471, 0.097255, 0.097255);
                        glUniform3f(boardProg->getUniform("Spec"), 0.500000, 0.500000, 0.500000);
                        glUniform1f(boardProg->getUniform("Shine"), 96.078431);
                    }
                    else {
                        glUniform3f(boardProg->getUniform("MatAmb"), 0.3, 0.3, 0.3);
                        glUniform3f(boardProg->getUniform("MatDif"), 0.0, 0.0, 0.0);
                        glUniform3f(boardProg->getUniform("Spec"), 0.500000, 0.500000, 0.500000);
                        glUniform1f(boardProg->getUniform("Shine"), 96.078431);
                    }
                    glUniformMatrix4fv(boardProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
                    button->draw(boardProg);
                M->popMatrix();
                M->pushMatrix();
                    M->translate(Vector3f(0, 0.6, -0.1));
                    M->scale(Vector3f(0.1, 0.1, 0.1));
                    if(numLights >= 3) {
                        glUniform3f(boardProg->getUniform("MatAmb"), 0.5, 0.5, 0.5);
                        glUniform3f(boardProg->getUniform("MatDif"), 0.696471, 0.097255, 0.097255);
                        glUniform3f(boardProg->getUniform("Spec"), 0.500000, 0.500000, 0.500000);
                        glUniform1f(boardProg->getUniform("Shine"), 96.078431);
                    }
                    else {
                        glUniform3f(boardProg->getUniform("MatAmb"), 0.3, 0.3, 0.3);
                        glUniform3f(boardProg->getUniform("MatDif"), 0.0, 0.0, 0.0);
                        glUniform3f(boardProg->getUniform("Spec"), 0.500000, 0.500000, 0.500000);
                        glUniform1f(boardProg->getUniform("Shine"), 96.078431);
                    }
                    glUniformMatrix4fv(boardProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
                    button->draw(boardProg);
                M->popMatrix();
                M->pushMatrix();
                    M->translate(Vector3f(1.9, 0.6, -0.1));
                    M->scale(Vector3f(0.1, 0.1, 0.1));
                    if(numLights >= 4) {
                        glUniform3f(boardProg->getUniform("MatAmb"), 0.5, 0.5, 0.5);
                        glUniform3f(boardProg->getUniform("MatDif"), 0.696471, 0.097255, 0.097255);
                        glUniform3f(boardProg->getUniform("Spec"), 0.500000, 0.500000, 0.500000);
                        glUniform1f(boardProg->getUniform("Shine"), 96.078431);
                    }
                    else {
                        glUniform3f(boardProg->getUniform("MatAmb"), 0.3, 0.3, 0.3);
                        glUniform3f(boardProg->getUniform("MatDif"), 0.0, 0.0, 0.0);
                        glUniform3f(boardProg->getUniform("Spec"), 0.500000, 0.500000, 0.500000);
                        glUniform1f(boardProg->getUniform("Shine"), 96.078431);
                    }
                    glUniformMatrix4fv(boardProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
                    button->draw(boardProg);
                M->popMatrix();
            M->popMatrix();
        boardProg->unbind();
    
        GLSL::printError();
        checkError("Before program");
    
        // MARK: Ball
    
        ballProg->bind();
            //Code for the ball
            //ballzzz to the wallzzzz
            M->pushMatrix();
                glUniformMatrix4fv(ballProg->getUniform("P"), 1, GL_FALSE, P->topMatrix().data());
                glUniformMatrix4fv(ballProg->getUniform("V"), 1, GL_FALSE, V->topMatrix().data());
                setUpLights(ballProg);
                M->translate(ballPos);
                M->rotate(bRot, Vector3f(0, 0, 1));
                M->translate(Vector3f(0, 0.4, 0));
                M->scale(Vector3f(1.0, 1.0, 1.0));
                
                glUniformMatrix4fv(ballProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
                ball->draw(ballProg);
            M->popMatrix();
        ballProg->unbind();
    
        checkError("After program");
    M->popMatrix();
    
    // MARK: Walls
    wallProg->bind();
        glUniformMatrix4fv(wallProg->getUniform("P"), 1, GL_FALSE, P->topMatrix().data());
        glUniformMatrix4fv(wallProg->getUniform("V"), 1, GL_FALSE, V->topMatrix().data());
        setUpLights(wallProg);
        glUniform3f(wallProg->getUniform("MatAmb"), 0.000000, 0.000000, 0.000000);
        glUniform3f(wallProg->getUniform("MatDif"), 0.016, 0.2, 0.09);
        glUniform3f(wallProg->getUniform("Spec"), 0.35, 0.35, 0.35);
        glUniform1f(wallProg->getUniform("Shine"), 32);
        M->pushMatrix();
            M->translate(Vector3f(-10, 4.5, 0));
            M->scale(Vector3f(0.05, 6, 10));
            glUniformMatrix4fv(wallProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
            cube->draw(wallProg);
        M->popMatrix();
        M->pushMatrix();
            M->translate(Vector3f(0, 4.5, -10));
            M->scale(Vector3f(10, 6, 0.05));
            glUniformMatrix4fv(wallProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
            cube->draw(wallProg);
        M->popMatrix();
        M->pushMatrix();
            M->translate(Vector3f(10, 4.5, 0));
            M->scale(Vector3f(0.05, 6, 10));
            glUniformMatrix4fv(wallProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
            cube->draw(wallProg);
        M->popMatrix();
        if(eye.z() < 10) {
            M->pushMatrix();
                M->translate(Vector3f(0, 4.5, 10));
                M->scale(Vector3f(10, 6, 0.05));
                glUniformMatrix4fv(wallProg->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
                cube->draw(wallProg);
            M->popMatrix();
        }
    
    
    wallProg->unbind();
    
	//draw the ground plane	
	prog2->bind();
    M->pushMatrix();
    M->loadIdentity();
	glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, P->topMatrix().data());
    glUniformMatrix4fv(prog2->getUniform("V"), 1, GL_FALSE, V->topMatrix().data());
	glUniformMatrix4fv(prog2->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
    glVertexAttrib3fv(prog2->getAttribute("lightPosition"), lightPosition);

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

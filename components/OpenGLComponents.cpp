#include "wx/wx.h"
#include "wx/glcanvas.h"
#include "OpenGL/gl3.h"
#include <iostream>
#include <fstream>

#define GLDisplayErrors() GLDisplayErrors_(__FILE__,__LINE__)

static float x_pos = -0.0f;
static float y_pos = -0.0f;
static const int numPointsOnCircumference = 200;
static const float playerRadius = 0.1;
static float permitableDistance[2][2] = { /*{
    {Left,Right}
    {Down,Up}
    }*/
    {1.0f - (playerRadius),1.0f - (playerRadius)},
    {1.0f - (playerRadius),1.0f - (playerRadius)}
};
enum Direction {HORIZONTAL=0,VERTICAL=1};
struct Obstacle{
    float x_uleft;    //Upper left x co-ordinate
    float y_uleft;    //Upper left y co-ordinate
    float x_bright;   //Bottom right x co-ordinate
    float y_bright;   //Bottom right y co-ordinate
};

Obstacle* obstacles = new Obstacle[1];


static bool withinBounds(Obstacle* obstacles,int obstacleCount,float new_x,float new_y)
{
    //horizontalCollisionZone_1
    // float permitableDistance[2][2] = { /*{
    //     {Left,Right}
    //     {Down,Up}
    //     }*/
    //     {1.0f - (playerRadius),1.0f - (playerRadius)},
    //     {1.0f - (playerRadius),1.0f - (playerRadius)}
    // };
    for(int i = 0; i < obstacleCount;i++){
        bool horizontallyContained = ((obstacles+i)->x_uleft <= new_x) && ((obstacles+i)->x_bright >= new_x);
        bool verticallyContained = ((obstacles+i)->y_bright <= new_y) && ((obstacles+i)->y_uleft >= new_y);
        if(horizontallyContained && verticallyContained){
            return false;
        };
    };
    return true;
};

static void GLClearError(){
    while (glGetError());
};

static void GLDisplayErrors_(const char* file, int line){
    std::string errorMessage;
    while (GLenum errorCode = glGetError())
    {
        switch (errorCode)
        {
            case GL_NO_ERROR:                      errorMessage = "No Errors";
            case GL_INVALID_VALUE:                 errorMessage = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             errorMessage = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                errorMessage = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               errorMessage = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 errorMessage = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: errorMessage = "INVALID_FRAMEBUFFER_OPERATION"; break;
            default: break;
        }
        std::cout << "Open GL error: " << errorMessage  << " on " << line << '\n';
    };

    
};

static uint32_t* makeCircleIB(int numVertices)
{
    if(numVertices > 500 or numVertices < 2){
        throw std::invalid_argument("Inappropriate angular seperation: Not within range (0,500]");
    };

    uint32_t* indexBufferArray = new uint32_t[3*numVertices];
    for(int i = 0;i<(numVertices-1);i++){
        *(indexBufferArray + 3*i) = 0;
        *(indexBufferArray + 3*i + 1) = i+1; 
        *(indexBufferArray + 3*i + 2) = (i != numVertices - 2) ? i+2 : 1; 
    };
    /* Returns 3x the number of triangles
    Number of triangles formed = numVertices - 2 */
    return indexBufferArray;
};

static float* makeCircleVB(float x_origin, float y_origin, float radius, int angularSeperation)
{
    /* Uses the r^2 = x^2 + y^2  equation to calculate the displcement of 
    a circle from its origin */
    (angularSeperation % 2) == 1 ? angularSeperation += 1 : angularSeperation;
    if(angularSeperation > 500 or angularSeperation < 2){
        throw std::invalid_argument("Inappropriate angular seperation: Not within range (0,500]");
    };
    float* vertexBufferArray = new float[2 * (angularSeperation+1)];

    *(vertexBufferArray) = x_origin;
    *(vertexBufferArray+1) = y_origin;

    for(int i=2; i < 2*(angularSeperation + 1);i+=2)
    {
        
        // DO NOT USE 2*pi as incrementing index 'i' are all multiples of 2
        *(vertexBufferArray + i) = x_origin + radius*sinf(M_PI*(i/float (angularSeperation)));
        *(vertexBufferArray + i + 1) = y_origin + radius*cosf(M_PI*(i/float (angularSeperation)));
    };
    //21 pairs of vertex buffers are returned due to 20 points on the circumference and 1 origin
    return vertexBufferArray;
};

static std::string readFile(std::string filePath)
{
    std::string contents,line = "";
    std::ifstream file(filePath);
    while(std::getline(file,line)){
        contents += (line + "\n");
    };
    return contents;
};

static unsigned int compileShaders(const std::string &source, GLuint type)
{
    GLuint id = glCreateShader(type);
    const char* src = &source[0];
    glShaderSource(id,1,&src,nullptr);
    glCompileShader(id);

    int success;
    glGetShaderiv(id,GL_COMPILE_STATUS,&success);

    if(!success){
        int length;
        glGetShaderiv(id,GL_INFO_LOG_LENGTH,&length);
        char* message = (char *) alloca(length * sizeof(char));
        glGetShaderInfoLog(id,length,&length,message);
        std::cout << "Failed to compile" << (type == GL_VERTEX_SHADER ? " vertex " : " fragement ") << "shader \n";
        std::cout << message;
        std::cout << glGetString(GL_VERSION) << "\n";
        std::cout << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
        // std::cout << source << "\n";
    };


    return id;
};

static unsigned int makeShaders(const std::string &vertexShader, const std::string &fragmentShader)
{
    GLuint program = glCreateProgram();
    GLuint vs = compileShaders(vertexShader,GL_VERTEX_SHADER);
    GLuint fs = compileShaders(fragmentShader,GL_FRAGMENT_SHADER);

    glAttachShader(program,vs);
    glAttachShader(program,fs);
    glLinkProgram(program);
    int linkSuccess;
    glGetProgramiv(program, GL_LINK_STATUS, &linkSuccess);
    if (!linkSuccess) {
        int length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*) alloca(length * sizeof(char));
        glGetProgramInfoLog(program, length, &length, message);
        std::cout << "Failed to link program:\n" << message;
        glValidateProgram(program);
    }

    return program;

};

class GLCanvas : public wxGLCanvas
{
    wxGLContext* context;
public:
    GLCanvas(wxFrame* parent, int* args);
    ~GLCanvas(){delete context;};

    void reDraw(wxSizeEvent& e){wxWindow::Refresh();};
    void render(wxPaintEvent& e);
    void make2DViewPort(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);
    float getHeight(){return (wxWindow::GetSize().y * wxWindow::GetContentScaleFactor());};
    float getWidth(){return (wxWindow::GetSize().x * wxWindow::GetContentScaleFactor());};


    // User Interactions
    void leftClick(wxMouseEvent& e);
    void mouseMoved(wxMouseEvent& e);
    void keyPress(wxKeyEvent& e);
};

GLCanvas::GLCanvas(wxFrame* parent, int* args) : wxGLCanvas(parent,wxID_ANY,args,wxDefaultPosition,wxDefaultSize,wxBORDER_DOUBLE){
    wxGLContextAttrs cxtAttrs;
    cxtAttrs.CoreProfile().OGLVersion(4,1).ForwardCompatible().EndList();
    context = new wxGLContext(this,nullptr,&cxtAttrs);
    this->SetCurrent(*context);

    std::cout << glGetString(GL_VERSION)<< std::endl;

    if(!context->IsOK()){
        std::cout << "Context not okay \n";
    };

    Bind(wxEVT_PAINT,&GLCanvas::render,this,wxID_ANY);
    Bind(wxEVT_SIZE,&GLCanvas::reDraw,this,wxID_ANY);
    Bind(wxEVT_KEY_DOWN,&GLCanvas::keyPress,this,wxID_ANY);
}

void GLCanvas::make2DViewPort(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y){
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // glEnable(GL_TEXTURE_2D);
    // glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	
    glViewport(topleft_x, topleft_y, bottomrigth_x-topleft_x, bottomrigth_y-topleft_y);
    // glMatrixMode(GL_PROJECTION);
    // glLoadIdentity();
    
    // gluOrtho2D(topleft_x, bottomrigth_x, bottomrigth_y, topleft_y);
    // glMatrixMode(GL_MODELVIEW);
    // glLoadIdentity();
};

static void drawScreen(wxGLCanvas *parent)
{
    glClear(GL_COLOR_BUFFER_BIT);
    float* positions = makeCircleVB(x_pos,y_pos,playerRadius,numPointsOnCircumference);

    //Delete this section
    float pllgramPos[] = {
        -0.5f,-0.5f,
        -1.0f,-0.5f,
        -1.0f,-1.0f,
        -0.5f,-1.0f
    };
    obstacles[0].x_bright = -0.5f;
    obstacles[0].y_bright = -1.0f;
    obstacles[0].x_uleft = -1.0f;
    obstacles[0].y_uleft = -0.5f;

    for(int i = 0; i<(sizeof(pllgramPos)/sizeof(float));i++){
        *(positions+(2*(numPointsOnCircumference+1))+i) = pllgramPos[i];
    };

    // for(int i = 0;i <2*(numPointsOnCircumference+1. +4); i+=2){
    //     std::cout << (i/2) <<": " << *(positions+i) << " , " << *(positions + i +1) << std::endl;
    // };

    //Until Here

    //There are 21 points including the center of the circle
    uint32_t* indexBuffer = makeCircleIB(numPointsOnCircumference + 1);

    //Delete this section
    uint32_t pllGramIB[] = {
        201,202,203,
        201,203,204
    };

    for(int i = 0; i<6;i++){
        *(indexBuffer+3*(numPointsOnCircumference)+i) = pllGramIB[i];
    };

    // for(int i = 0;i <3*(numPointsOnCircumference + 2); i+=3){
    //     std::cout << (i/3) <<": " << *(indexBuffer+i) << " , " << *(indexBuffer + i +1) << " , " << *(indexBuffer + i +2) << std::endl;
    // };

    //Until Here

    unsigned int vao;
    glGenVertexArrays(1,&vao);
    glBindVertexArray(vao);

    unsigned int buffer;
    glGenBuffers(1,&buffer);
    glBindBuffer(GL_ARRAY_BUFFER,buffer);
    glBufferData(GL_ARRAY_BUFFER,sizeof(float)*(2*(numPointsOnCircumference+1)+8),positions,GL_STATIC_DRAW);

    unsigned int ibo;
    glGenBuffers(1,&ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(uint32_t)*(3*numPointsOnCircumference + 6),indexBuffer,GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,2, GL_FLOAT,GL_FALSE, 2 * sizeof(float),0);


    /* Determines the number of triangles to draw based on the 
    number of points on the circumference */
    
    glDrawElements(GL_TRIANGLES,3*numPointsOnCircumference + 6,GL_UNSIGNED_INT,nullptr);

    // std::cout << getWidth() << '\n';
    // std::cout << getHeight() << '\n';
    // withinBounds(x_pos,(y_pos+playerRadius+0.02f),900,900);


    glFlush();
    parent->SwapBuffers();
    delete[] positions;
    delete[] indexBuffer;
    glDeleteVertexArrays(1,&vao);
    glDeleteBuffers(1,&ibo);
    glDeleteBuffers(1,&buffer);
};

void GLCanvas::render(wxPaintEvent& e){
    if(!wxWindowBase::IsShown()){return;};

    wxGLCanvas::SetCurrent(*context);
    wxPaintDC(this);

    make2DViewPort(0,0,getWidth(),getHeight());

    glLoadIdentity();

    std::string vertexShader = readFile("../shaders/vertexShader.txt");
    std::string fragmentShader = readFile("../shaders/fragmentShader.txt");
    unsigned int shader = makeShaders(vertexShader,fragmentShader);
    glUseProgram(shader);

    drawScreen(this);
    glDeleteProgram(shader);

};

void GLCanvas::keyPress(wxKeyEvent& e){
    wxChar keyVal = e.GetKeyCode();
    bool redrawScreen = false;
    float speed = 0.05;
    switch (keyVal)
    {
    case 87:
        y_pos += (((y_pos + playerRadius+0.02f) <= 1) && withinBounds(obstacles,1,x_pos,(y_pos+playerRadius+speed))) ? speed : 0;
        redrawScreen = true;
        // withinBounds( x_pos,(y_pos+playerRadius),450,450);
        break;
    case 83:
        y_pos -= (((y_pos - playerRadius) >= -1) && withinBounds(obstacles,1,x_pos,(y_pos-playerRadius-speed))) ? speed : 0;
        redrawScreen = true;
        // withinBounds( x_pos,(y_pos-playerRadius),450,450);
        break;
    case 65:
        x_pos -= (((x_pos - playerRadius) >= -1) && withinBounds(obstacles,1,(x_pos-playerRadius-speed),y_pos)) ? speed : 0;
        redrawScreen = true;
        // withinBounds((x_pos - playerRadius),y_pos,450,450);
        break;
    case 68:
        x_pos += (((x_pos + playerRadius) <= 1) && withinBounds(obstacles,1,(x_pos+playerRadius+speed),y_pos)) ? speed : 0;
        redrawScreen = true;
        // withinBounds((x_pos + playerRadius),y_pos,450,450);
        break;
    default:
        break;
    }

    if(redrawScreen){
        drawScreen(this);
    };
}
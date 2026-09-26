#include "wx/wx.h"
#include "wx/glcanvas.h"
#include "OpenGL/gl.h"
#include "OpenGL/glu.h"
#include <iostream>
#include <fstream>

static std::string readFile(std::string filePath)
{
    std::string contents,line = "";
    std::ifstream file(filePath);
    while(std::getline(file,line)){
        contents += line;
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
    glValidateProgram(program);

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
    float getHeight(){return wxWindow::GetSize().y;};
    float getWidth(){return wxWindow::GetSize().x;};

    // User Interactions
    void leftClick(wxMouseEvent& e);
    void mouseMoved(wxMouseEvent& e);
    void keyPress(wxKeyEvent& e);
};

GLCanvas::GLCanvas(wxFrame* parent, int* args) : wxGLCanvas(parent,wxID_ANY,args,wxDefaultPosition,wxDefaultSize,wxBORDER_DOUBLE){
    context = new wxGLContext(this);
    Bind(wxEVT_PAINT,&GLCanvas::render,this,wxID_ANY);
    Bind(wxEVT_SIZE,&GLCanvas::reDraw,this,wxID_ANY);
}

void GLCanvas::make2DViewPort(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y){
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	
    glViewport(topleft_x, topleft_y, bottomrigth_x-topleft_x, bottomrigth_y-topleft_y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    gluOrtho2D(topleft_x, bottomrigth_x, bottomrigth_y, topleft_y);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
};

void GLCanvas::render(wxPaintEvent& e){
    if(!wxWindowBase::IsShown()){return;};


    wxGLCanvas::SetCurrent(*context);
    wxPaintDC(this);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    make2DViewPort(0,0,getWidth(),getHeight());
    glLoadIdentity();


    float positions[6] = {
        0 , getHeight(),
        getWidth(),getHeight(),
        getWidth(),-getHeight()
    };

    unsigned int buffer;
    glGenBuffers(1,&buffer);
    glBindBuffer(GL_ARRAY_BUFFER,buffer);
    glBufferData(GL_ARRAY_BUFFER,sizeof(positions),positions,GL_STATIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,2, GL_INT,GL_TRUE, 2 * sizeof(float),0);

    std::string vertexShader = readFile("../shaders/vertexShader.txt");
    std::string fragmentShader = readFile("../shaders/fragmentShader.txt");
    unsigned int shader = makeShaders(vertexShader,fragmentShader);
    glUseProgram(shader);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    // std::cout << getWidth() << '\n';
    // std::cout << getHeight() << '\n';

    glFlush();
    SwapBuffers();
};
#include "wx/wx.h"
#include "wx/glcanvas.h"
#include "OpenGL/gl3.h"
#include <iostream>
#include <fstream>
#include <cmath>
// #include "mazeMaker.hpp"
#define GLDisplayErrors() GLDisplayErrors_(__FILE__,__LINE__)

//Here
static const int meshSize = 10;

enum cardinals {NORTH,SOUTH,EAST,WEST};
class mazeNode
{
private:
    int gridLocation[2];        //(x,y) Cartesian Coordinates on a plane
    bool locationLock = false;
    bool isTarget = false;
public:
    bool directions[4] = {false,false,false,false};
    mazeNode* nextPointer = nullptr;
    mazeNode* findLastNode();
    void setGridCoordinate(int x, int y);
    int* getGridCoordinates(){return gridLocation;};
    void exploredDirections(cardinals direction){directions[direction] = true;};
    void makeTarget(){isTarget = true;};
    void directionInit(){
        directions[0] = (gridLocation[0] == 0 ? true : false);
        directions[1] = (gridLocation[0] == 9 ? true : false);
        directions[2] = (gridLocation[1] == 9 ? true : false);
        directions[3] = (gridLocation[1] == 0 ? true : false);

    };
};

void mazeNode::setGridCoordinate(int x, int y){
    if(!locationLock){
        gridLocation[0] = x;
        gridLocation[1] = y;
        locationLock = true;
    };
};

static mazeNode* headPointer;

mazeNode* mazeNode::findLastNode(){
    //Return last node
    mazeNode* lastNode = headPointer;
    while (lastNode->nextPointer != nullptr){
        lastNode = lastNode->nextPointer;
    };
    return lastNode;
};

static mazeNode * findNode(int x, int y){
    mazeNode* currentNode = headPointer;
    if(currentNode == nullptr){return nullptr;};
    while(currentNode->nextPointer != nullptr){
        bool x_match = (currentNode->getGridCoordinates()[0] == x);
        bool y_match = (currentNode->getGridCoordinates()[1] == y);
        if(x_match && y_match){
            return currentNode;
            break;
        }
        currentNode = currentNode->nextPointer;
        //std::cout << currentNode->nextPointer << std::endl;

    };
    return nullptr;
};

static bool boxedIn(cardinals dir,mazeNode currentNode){
    int x = currentNode.getGridCoordinates()[0];
    int y = currentNode.getGridCoordinates()[1];
    int dirsAvailable,vGain,hGain = 0;
    cardinals opp_Dir;
    switch (dir){
        case (SOUTH):                           // SOUTH
            if ((currentNode.getGridCoordinates()[1]) != 9){
                vGain = 1;
                opp_Dir = NORTH;
            }else{
                vGain = -1;
                dir = (cardinals) 0;
                opp_Dir = SOUTH;
            };
            break;
        case NORTH:                            // NORTH
            if ((currentNode.getGridCoordinates()[1]) != 0){
                vGain = -1;
                opp_Dir = SOUTH;
            }else{
                vGain = 1;
                dir = (cardinals) 1;
                opp_Dir = NORTH;
            };
            break;
        case EAST:                             // East
            if ((currentNode.getGridCoordinates()[0]) != 9){
                hGain = 1;
                opp_Dir = WEST;
            }else{
                hGain = -1;
                dir = (cardinals) 3;
                opp_Dir = EAST;
            };
            break;
        case WEST:                             // West
            if ((currentNode.getGridCoordinates()[0]) != 0){
                hGain = -1;
                opp_Dir = EAST;
            }else{
                hGain = 1;
                dir = (cardinals) 2;
                opp_Dir = WEST;
            };
            break;
        };

        mazeNode* discoveredNode = (findNode(x+hGain, y+vGain));
        if(discoveredNode != nullptr){
            //std::cout <<"True triggered" <<std::endl;
            mazeNode nextNode = *(discoveredNode);
            nextNode.exploredDirections(opp_Dir);
            int soleDir = 0;
            for(int i = 0; i<4; i++){
                if(nextNode.directions[i]){
                    dirsAvailable += 1;
                    soleDir = i;
                }
            };

            if(dirsAvailable >= 2){
                return false;
            }else if(dirsAvailable == 1){
                return boxedIn((cardinals) soleDir,nextNode);
            }else{
                return true;
            };
        }else{
            //std::cout <<"False triggered" <<std::endl;
            return false;
        };
};

static void makeMaze(){


    //Choose an initial node and assign its pointer to head
    mazeNode initialNode;
    initialNode.setGridCoordinate(0,0);
    headPointer = &initialNode;


    //Create a node that can be regularly altered
    mazeNode *currentNode = (headPointer);

    //Choose a final grid position:
    srand(time(NULL));
    mazeNode finalNode;
    finalNode.setGridCoordinate(rand()%10, rand()%10);

    //std::cout << initialNode.nextPointer << " | " << *(initialNode.getGridCoordinates()) << " | " << *(initialNode.getGridCoordinates() + 1) << std::endl;
    //std::cout << finalNode.nextPointer << " | " << *(finalNode.getGridCoordinates()) << " | " << *(finalNode.getGridCoordinates() + 1) << std::endl;


    //Loop around until the final node has been found.
    bool x_match = *(currentNode->getGridCoordinates()) == *(finalNode.getGridCoordinates());
    bool y_match = *(currentNode->getGridCoordinates() + 1) == *(finalNode.getGridCoordinates() +1);

    int vGain, hGain,counter = 0;
    while(!(x_match && y_match)){
        counter+=1;
        //std::cout << *(currentNode->getGridCoordinates()) << " | " << *(currentNode->getGridCoordinates()+1) << std::endl;
        vGain = 0;
        hGain = 0;
        currentNode->directionInit();
        int randDir = rand()%4;
        while(currentNode->directions[randDir] || boxedIn((cardinals)randDir,*(currentNode))){
            // //std::cout << "My heart is breaking" << std::endl;
            for(int i =0; i < 4; i++){
                //std::cout << currentNode->directions[i];
            };
            //std::cout << std::endl;
            randDir = rand()%4;
        }

        //std::cout << randDir <<std::endl;

        cardinals opp_dir;
        switch (randDir){
        case (1):                           // SOUTH
            if (*(currentNode->getGridCoordinates() + 1) != 9){
                vGain = 1;
                opp_dir = NORTH;
            }else{
                vGain = -1;
                randDir = 0;
                opp_dir = SOUTH;
            };
            break;
        case 0:                            // NORTH
            if (*(currentNode->getGridCoordinates() + 1) != 0){
                vGain = -1;
                opp_dir = SOUTH;
            }else{
                vGain = 1;
                randDir = 1;
                opp_dir = NORTH;
            };
            break;
        case 2:                             // East
            if (*(currentNode->getGridCoordinates()) != 9){
                hGain = 1;
                opp_dir = WEST;
            }else{
                hGain = -1;
                randDir = 3;
                opp_dir = EAST;
            };
            break;
        case 3:                             // West
            if (*(currentNode->getGridCoordinates()) != 0){
                hGain = -1;
                opp_dir = EAST;
            }else{
                hGain = 1;
                randDir = 2;
                opp_dir = WEST;
            };
            break;
        };
        mazeNode* nodeExists = findNode((currentNode->getGridCoordinates()[0]) + hGain,(currentNode->getGridCoordinates()[1]) + vGain);
        mazeNode* newNode;
        bool condititon1 = nodeExists == nullptr;
        if(condititon1){
            newNode = new mazeNode;
        }else{
            newNode = nodeExists;
        };
        currentNode->exploredDirections((cardinals) randDir);
        newNode->exploredDirections(opp_dir);
        // //std::cout << "-----------------------------------" <<std::endl;
        // //std::cout << "Values: " << vGain << " | " << hGain << std::endl;
        // //std::cout << "-----------------------------------" <<std::endl;

        if(!condititon1){
            currentNode = nodeExists;
            continue;
        };
        
        newNode->setGridCoordinate((currentNode->getGridCoordinates()[0]) + hGain,(currentNode->getGridCoordinates()[1]) + vGain);

        //Update next pointer and loop conditions
        x_match = (currentNode->getGridCoordinates()[0]) == (finalNode.getGridCoordinates()[0]);
        y_match = (currentNode->getGridCoordinates()[1]) == (finalNode.getGridCoordinates()[1]);
        currentNode->nextPointer = newNode;
        currentNode = (newNode);

    };

    // //std::cout << initialNode.nextPointer << std::endl;
    // //std::cout << headPointer << std::endl;

    // mazeNode* nextNode = headPointer;
    // while(nextNode->nextPointer != nullptr){
    //     //std::cout << nextNode->nextPointer << " | " << *(nextNode->getGridCoordinates()) << " | " << *(nextNode->getGridCoordinates() + 1) << std::endl;
    //     nextNode = nextNode->nextPointer;
    // };
    // //std::cout << nextNode->nextPointer << " | " << *(nextNode->getGridCoordinates()) << " | " << *(nextNode->getGridCoordinates() + 1) << std::endl;

    

};
//Until Here

static float x_pos = -0.0f;
static float y_pos = -0.0f;
static const int numPointsOnCircumference = 200;
static const float playerRadius = 0.1;
enum Direction {HORIZONTAL=0,VERTICAL=1};
struct Obstacle{
    float x_uleft;    //Upper left x co-ordinate
    float y_uleft;    //Upper left y co-ordinate
    float x_bright;   //Bottom right x co-ordinate
    float y_bright;   //Bottom right y co-ordinate
};

Obstacle* obstacles = new Obstacle[4];


static bool withinBounds(Obstacle* obstacles,int obstacleCount,float new_x,float new_y)
{

    for(int i = 0; i < obstacleCount;i++){
        bool horizontallyContained = ((obstacles+i)->x_uleft <= new_x) && ((obstacles+i)->x_bright >= new_x);
        bool verticallyContained = ((obstacles+i)->y_bright <= new_y) && ((obstacles+i)->y_uleft >= new_y);
        if(horizontallyContained && verticallyContained){
            return false;
        };
    };
    return true;
};

template<typename A>
A** mergeArrays(A head, A** tail, int rows, int cols){
    A** resArray = new A*[rows];
    for(int i = 0; i< rows; i++){
        resArray[i] = new A[cols + 1];
        resArray[i][0] = head;
        for(int j = 1; j< (cols + 1);j++){
            resArray[i][j] = tail[i][j-1];
        };
    };
    return resArray;
};

template<typename T>
T** cartesianComb(T* list1, T* list2,int length){
    
    // std::cout << length <<std::endl;

    // Base Case: Both list1 and list 2 have one element each. Returns a 2D array of both elements in seperate arrays
    // Recursive Case: 
    if(length == 1){
        T** response = new T*[2];
        for(int i = 0; i<2; i++){
            response[i] = new T[1];
        };
        response[0][0] = list1[0];
        response[1][0] = list2[0];
        // std::cout << response[0][0] <<std::endl;
        // std::cout << response[1][0] <<std::endl;
        return response;
    }else{
        T tail1[length - 1];
        T tail2[length - 1];
        for(T i = 1; i< length;i++){
            tail1[(int) i - 1] = list1[(int) i];
            tail2[(int) i - 1] = list2[(int) i];
        };
        T** cartComb = cartesianComb(tail1,tail2,length - 1);
        T** resOne = mergeArrays(list1[0],cartComb,(int) pow(2,length-1),length - 1);
        T** resTwo = mergeArrays(list2[0],cartComb,(int) pow(2,length-1),length - 1);
        T** response = new T*[(int) pow(2,length) + 1];
        for(int i = 0; i< (int) (pow(2,length)) ;i++){
            response[i] = (i < pow(2,length - 1)) ? resOne[i] : resTwo[i - ((int)pow(2,length - 1))];
            for(int j = 0;j < length;j++){
                // std::cout << response[i][j] ;
            };
            // std::cout << "- " << i<< std::endl;
        };

        // std::cout << " ------------------------------ " << std::endl;

        return response;
    };

};
template <typename T>
static T* foldAppend2D(T** array, int rows, int cols){
    T* resultArr = new T[rows * cols];
    for(int i = 0; i< rows;i++){
        for(int j =0;j<cols;j++){
            resultArr[i*cols + j] = array[i][j];
            // std::cout << (array[i][j]) << std::endl;
        }
    };
    return resultArr;
};

template <typename T>
static T* foldAppend2D(T* array1, T* array2,int length1,int length2){
    T* resultArray = new T[length1+length2];
    for(int i = 0; i< length1; i++){
        resultArray[i] = array1[i];
    };
    for(int i = 0; i< length2; i++){
        resultArray[i + length1] = array2[i];
    }
    return resultArray;
};

static float* createCorners(){
    obstacles[0] = {0.5,1,1,0.5};
    obstacles[1] = {0.5,-0.5,1,-1};
    obstacles[2] = {-1,-0.5,-0.5,-1};
    obstacles[3] = {-1,1,-0.5,0.5};

    int cornerflen = 0;
    float* cornerf = new float[0];

    for(int i = 0 ; i< 4; i++){
        float arr1[2] = {obstacles[i].x_uleft,obstacles[i].y_uleft};
        float arr2[2] = {obstacles[i].x_bright,obstacles[i].y_bright};
        float** cornerArrays = cartesianComb(arr1,arr2,2);
        cornerf = foldAppend2D(cornerf,foldAppend2D(cornerArrays,4,2),cornerflen,8);
        cornerflen += 8;
    };

    return cornerf;

};

// void drawObstacles(Obstacle* obstacles, float &positions, uint32_t &indexes, mazeNode &currentNode){
//     obstacles[0] = {-1.0f,-1.0f,-0.5f,-0.5f};
//     obstacles[1] = {0.5,-1,1,-0.5};
//     obstacles[2] = {-1.0f,0.5f,-0.5f,1.0f};
//     obstacles[3] = {0.5f,0.5f,1.0f,1.0f};

//     for(int i = 0; i< 4;i++){
//         ;
//     };
// };

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
        //std::cout << "Open GL error: " << errorMessage  << " on " << line << '\n';
    };

    
};

static uint32_t* makeCircleIB(int numVertices)
{
    if(numVertices > 201 or numVertices < 2){
        throw std::invalid_argument("Inappropriate angular seperation: Not within range (0,201]");
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
        std::cout << source << "\n";
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
        //std::cout << "Failed to link program:\n" << message;
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
    makeMaze();
    float* corners = createCorners();
    for(int i = 0; i < 4;i++){
        for(int j = 0; j < 8; j+=2){
            std::cout << "(" << *(corners + i*8 + j) << "," << *(corners + i*8 + j +1) << ") | ";
        };
        std::cout << std::endl;
    };

    wxGLContextAttrs cxtAttrs;
    cxtAttrs.CoreProfile().OGLVersion(4,1).ForwardCompatible().EndList();
    context = new wxGLContext(this,nullptr,&cxtAttrs);
    this->SetCurrent(*context);

    //std::cout << glGetString(GL_VERSION)<< std::endl;

    if(!context->IsOK()){
        //std::cout << "Context not okay \n";
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
    float* corners = createCorners();

    for(int i = 0; i<32;i++){
        *(positions+(2*(numPointsOnCircumference+1))+i) = corners[i];
    };

    for(int i = 0; i < 2*(numPointsOnCircumference+1)+32;i+=2){
        std::cout << (i/2) << ": "<< *(positions+i) << " | " << *(positions +i +1) << std::endl;
    };

    //Until Here

    //There are 21 points including the center of the circle
    uint32_t* indexBuffer = makeCircleIB(numPointsOnCircumference + 1);

    //Delete this section
    uint32_t* pllGramIB = new uint32_t[24];
    for(int i = 1; i<= 4; i++){
        pllGramIB[6*(i-1) + 3] = numPointsOnCircumference + 4*(i);
        pllGramIB[6*(i-1)] = numPointsOnCircumference + 4*(i) - 3;
        pllGramIB[6*(i-1) + 1] = pllGramIB[6*(i-1) + 4] = numPointsOnCircumference + 4*(i) - 1;
        pllGramIB[6*(i-1) + 2] = pllGramIB[6*(i-1) + 5] = numPointsOnCircumference + 4*(i) - 2;
    };

    for(int i = 0; i<24;i++){
        *(indexBuffer+3*(numPointsOnCircumference)+i) = pllGramIB[i];
    };

    for(int i = 0; i < 3*(numPointsOnCircumference+8);i+=3){
        std::cout << (i/3)<< ": " << *(indexBuffer + i) << " | " << *(indexBuffer + i + 1) << " | " << *(indexBuffer + i +2) << std::endl;
    };

    //Until Here

    unsigned int vao;
    glGenVertexArrays(1,&vao);
    glBindVertexArray(vao);

    unsigned int buffer;
    glGenBuffers(1,&buffer);
    glBindBuffer(GL_ARRAY_BUFFER,buffer);
    glBufferData(GL_ARRAY_BUFFER,sizeof(float)*(2*(numPointsOnCircumference+1)+32),positions,GL_STATIC_DRAW);

    unsigned int ibo;
    glGenBuffers(1,&ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(uint32_t)*(3*numPointsOnCircumference + 24),indexBuffer,GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,2, GL_FLOAT,GL_FALSE, 2 * sizeof(float),0);


    /* Determines the number of triangles to draw based on the 
    number of points on the circumference */
    
    glDrawElements(GL_TRIANGLES,3*numPointsOnCircumference + 24,GL_UNSIGNED_INT,nullptr);


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
        y_pos += (((y_pos + playerRadius+0.02f) <= 1) && withinBounds(obstacles,4,x_pos,(y_pos+playerRadius))) ? speed : 0;
        redrawScreen = true;
        break;
    case 83:
        y_pos -= (((y_pos - playerRadius) >= -1) && withinBounds(obstacles,4,x_pos,(y_pos-playerRadius))) ? speed : 0;
        redrawScreen = true;
        break;
    case 65:
        x_pos -= (((x_pos - playerRadius) >= -1) && withinBounds(obstacles,4,(x_pos-playerRadius),y_pos)) ? speed : 0;
        redrawScreen = true;
        break;
    case 68:
        x_pos += (((x_pos + playerRadius) <= 1) && withinBounds(obstacles,4,(x_pos+playerRadius),y_pos)) ? speed : 0;
        redrawScreen = true;
        break;
    default:
        break;
    }

    if(redrawScreen){
        drawScreen(this);
    };
}
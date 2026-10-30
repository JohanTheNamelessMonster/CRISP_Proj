#include "wx/wx.h"
#include "OpenGL/gl.h"
#include "OpenGLComponents.cpp"

class mainWindow : public wxApp
{
public:
    virtual bool OnInit();
};

class mainFrame : public wxFrame
{
public:
    mainFrame(wxWindow* parent, const wxString title);
    static const int side_length = 900;
private:
    void OnHello(wxCommandEvent& e){wxLogMessage("The application is up");};
    void OnExit(wxCommandEvent& e){Close(true);};
};

mainFrame::mainFrame(wxWindow* parent, const wxString title) : wxFrame(parent,wxID_ANY,title,wxDefaultPosition,wxSize(side_length,side_length),wxDEFAULT_FRAME_STYLE & ~(wxMAXIMIZE_BOX | wxRESIZE_BORDER))
{

    wxMenu* menuFile = new wxMenu();
    menuFile->Append(1,"&Hello");
    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile,"&File");
    SetMenuBar(menuBar);
    Bind(wxEVT_MENU,&mainFrame::OnHello,this,1);
    Bind(wxEVT_MENU,&mainFrame::OnExit,this,wxID_EXIT);

    // this->ShowFullScreen(false);

};

bool mainWindow::OnInit()
{
    wxBoxSizer* testSizer = new wxBoxSizer(wxHORIZONTAL);
    int args[] = {WX_GL_RGBA,WX_GL_DOUBLEBUFFER,WX_GL_DEPTH_SIZE,16,0};


    wxGLAttributes canvasAttrs;
    canvasAttrs.PlatformDefaults().Defaults().EndList(); 

    mainFrame* firstFrame = new mainFrame(nullptr,wxT("SpermSplorer"));
    GLCanvas* canvas = new GLCanvas(firstFrame,args);

    testSizer->Add(canvas,1,wxGROW);
    firstFrame->SetSizer(testSizer);
    firstFrame->SetAutoLayout(true);

    firstFrame->Show(true);
    return true;
};
#ifndef _VAST_APP_H
#define _VAST_APP_H

#include <set>
#include <vector>
#include "time.h"

#include <QMainWindow>
#include <QKeyEvent>
#include <QString>

#include "GL/gle.h"
#include <QGLViewer/qglviewer.h>

#include "ui_VASTChallenge.h"

// forward declaration!
class VASTApp;
#include "Park.h"

//typedef gleDouble glePoint[3];


/// ===========================================================
/// VASTViewer
/// ===========================================================

class VASTViewer : public QGLViewer{

    VASTApp* parentApp;
    //GLuint sphereList;

public:

    //int showrideno;

    uint live_animation_time;
    float park_X, park_Y;

    GLuint maptextid;

    VASTViewer(QWidget *parent);

protected :

    virtual void draw();
    virtual void animate();
    virtual void keyPressEvent(QKeyEvent* event);

    virtual void init();
};

/// ===========================================================
/// VASTApp
/// ===========================================================

class VASTApp : public QMainWindow
{
    Q_OBJECT

private:

   VASTViewer *viewer;          // pointer to the viewer

public:
    Ui::VASTChallengeUI ui;

    Park aPark;

    /// ----------------------------------------------------
    /// constructor and destructor
    VASTApp(std::string filename);
    ~VASTApp(){}

protected slots:

    void on_pb_animPause_clicked(){
        if(viewer->animationIsStarted())
            viewer->toggleAnimation();
    }

    void on_pb_animResume_clicked(){
        if(!viewer->animationIsStarted())
            viewer->toggleAnimation();
    }

    void on_pb_animRestart_clicked(){
        viewer->live_animation_time = 0;
        viewer->updateGL();
    }
  void on_sb_animSpeed_valueChanged(int){             viewer->updateGL(); }
};
#endif

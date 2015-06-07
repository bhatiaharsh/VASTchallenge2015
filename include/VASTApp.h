#ifndef _VAST_APP_H
#define _VAST_APP_H

#include <set>
#include <vector>
#include "time.h"

#include <QMainWindow>
#include <QKeyEvent>
#include <QString>
#include <QGLWidget>
#include <QTimer>

#include "ui_VASTChallenge.h"

// forward declaration!
class VASTApp;
#include "Park.h"

//typedef gleDouble glePoint[3];


/// ===========================================================
/// VASTViewer
/// ===========================================================

class VASTViewer : public QGLWidget {
    Q_OBJECT
public:
    VASTApp* parentApp;
    uint live_animation_time;
    float park_X, park_Y;
    int animation_period;
    bool animating;
    QTimer * anim_timer;

    GLuint maptextid;

    VASTViewer(QWidget *parent);
    bool animationIsStarted() { return animating; }
    void toggleAnimation();

public slots:
    virtual void animate();

protected :
    void initializeGL();
    void paintEvent(QPaintEvent *event);
    virtual void drawNativeGL();
    virtual void qtPaint(QPainter *painter) { Q_UNUSED(painter); }
    void beginNativeGL();
    void endNativeGL();

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
        viewer->animate();
    }
  void on_sb_animSpeed_valueChanged(int){ viewer->animate(); }
};
#endif

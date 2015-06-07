/// ----------------------------
/// std header files

#include <cstdlib>
#include <cstdio>
#include <string>
#include <numeric>
#include <float.h>
#include <iostream>

/// ----------------------------
/// my header files

#include "VASTApp.h"

/// -----------------------------------------------------------------
using namespace std;

/// -----------------------------------------------------------------
VASTViewer::VASTViewer(QWidget *parent) :
    QGLWidget(QGLFormat(QGL::SampleBuffers), parent),
    parentApp((VASTApp *) parent),
    live_animation_time(0),
    park_X(500),
    park_Y(500),
    animation_period(50), // milliseconds
    animating(false),
    anim_timer(new QTimer)
{
    connect(anim_timer, SIGNAL(timeout()), this, SLOT(animate()));

    setMinimumSize(800, 800);
    setAutoFillBackground(false);
}

void VASTViewer::toggleAnimation()
{
    if (this->animating)
    {
        this->anim_timer->stop();
    }
    else
    {
        this->anim_timer->start(this->animation_period);
    }
    this->animating = !(this->animating);
}

void VASTViewer::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    // Clear
    qglClearColor(Qt::white);
    glClear(GL_COLOR_BUFFER_BIT);

    beginNativeGL();
    drawNativeGL();
    endNativeGL();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    qtPaint(&painter);
    painter.end();
}

void VASTViewer::initializeGL()
{
    glEnable(GL_MULTISAMPLE);
    glDisable(GL_DEPTH);
}

void VASTViewer::beginNativeGL()
{
    makeCurrent();
    glViewport(0, 0, width(), height());
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Switch for 2D drawing
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
}

void VASTViewer::endNativeGL()
{
    // Revert settings for painter
    glShadeModel(GL_FLAT);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);


    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}


void VASTViewer::drawNativeGL()
{
    glEnable(GL_TEXTURE_2D);
    QImage rawjpg;

    if(!rawjpg.load( "../ParkMap.jpg")){

        printf("Cannot load Park Map!\n");
        exit(1);
    }

    QImage txtimg = QGLWidget::convertToGLFormat( rawjpg );
    glGenTextures( 1, &maptextid );
    glBindTexture( GL_TEXTURE_2D, maptextid );
    glTexImage2D( GL_TEXTURE_2D, 0, 3, txtimg.width(), txtimg.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, txtimg.bits() );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_COLOR_MATERIAL);

    time_t currtime = live_animation_time+parentApp->aPark.time_beg;

    // -----------------------------------------------------------
    glDisable(GL_LIGHTING);

    // -----------------------------------------------------------
    static float grid_res = 100.0;
    static float oneover_grid_res = 1.0 / grid_res;

    glScalef(oneover_grid_res, oneover_grid_res, 1.0);

    // -----------------------------------------------------------
    glColor3f(1,1,1);
    glBindTexture( GL_TEXTURE_2D, maptextid );
    glBegin(GL_QUADS);

    glTexCoord2f(0,0);  glVertex3f(0,0,0);
    glTexCoord2f(0,1);  glVertex3f(0,grid_res,0);
    glTexCoord2f(1,1);  glVertex3f(grid_res,grid_res,0);
    glTexCoord2f(1,0);  glVertex3f(grid_res,0,0);

    glEnd();

    // -----------------------------------------------------------
    //uint num_events = this->parentApp->events.size();
    //uint num_custs = this->parentApp->customers.size();

    uint num_custs_in_park = 0;

    // draw customers!
    glPointSize(6);
    glColor3f(0,0,0);

    glBegin(GL_POINTS);

    map<unsigned long int, ParkCustomer>::const_iterator citer = parentApp->aPark.customers.begin();
    for(; citer != parentApp->aPark.customers.end(); citer++){

        // find the present position of this customer
        // skip all past positions
        // look for first timestamp >= the current animation time
        // the position before that is the present position

        const vector<CustomerEvent> &cevents = citer->second.all_events;

        for(uint i = 0; i < cevents.size(); i++){

            const CustomerEvent &e = cevents[i];
            if(e.timestamp < currtime)      continue;
            if(i == 0)                      break;

            i--;
            glVertex3f(e.location.first, e.location.second, 0);
            num_custs_in_park ++;
            break;
        }
    }
    glEnd();

    // draw rides

    float max_pointsize = 60;
    float size_factor = max_pointsize / (float)parentApp->aPark.max_ride_load;

    glColor3f(0,1,0);

    map<coords, ParkRide>::const_iterator riter = parentApp->aPark.rides.begin();
    for(; riter != parentApp->aPark.rides.end(); riter++){

        const ParkRide &ride = riter->second;

        int psize = ride.get_load(currtime) * size_factor;

        glPointSize(psize);
        glBegin(GL_POINTS);
            glVertex3f(riter->first.first, riter->first.second, 0);
        glEnd();
    }



    {   // ----- write the current time

        glColor3f(0.1,0.1,0.1);
        QString tag("Curr Time: ");
        tag.append(ctime(&currtime));
        this->renderText(10,38,tag);
        QString tag2("Num of people in the park: ");
        tag2.append(QString::number(num_custs_in_park));
        this->renderText(10,52,tag2);
    }
}

void VASTViewer::animate(){
    this->anim_timer->stop();
    live_animation_time = (live_animation_time + parentApp->ui.sb_animSpeed->value()) % (parentApp->aPark.time_end-parentApp->aPark.time_beg);
    repaint();
    this->anim_timer->start(this->animation_period);

}

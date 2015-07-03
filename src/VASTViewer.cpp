/// ----------------------------
/// std header files

#include <cstdlib>
#include <cstdio>
#include <string>
#include <numeric>
#include <float.h>

/// ----------------------------
/// my header files

#include "VASTApp.h"

/// -----------------------------------------------------------------
using namespace std;

/// -----------------------------------------------------------------
VASTViewer::VASTViewer(QWidget *parent) : QGLViewer(parent){

    parentApp = (VASTApp*)parent;
    this->live_animation_time = 0;
    this->park_X = 500;
    this->park_Y = 500;

    //showrideno = 0;
}

void VASTViewer::init(){

    this->setSceneCenter(qglviewer::Vec(0.5,0.5,0));
    //this->setSceneRadius(0.6);
    this->camera()->centerScene();

    this->setMouseBinding(Qt::NoModifier, Qt::LeftButton, NO_CLICK_ACTION);

    this->setFPSIsDisplayed(true);
    this->setTextIsEnabled(true);

    this->setBackgroundColor(Qt::white);
    this->setForegroundColor(Qt::black);

    this->setAnimationPeriod(1);        // milliseconds!

    cout << "\n -------------------------------------------------------- \n";
    cout << " | OpenGL Renderer: " << (char*) glGetString(GL_RENDERER) << "\n";
    cout << " | OpenGL Version: " << (char*) glGetString(GL_VERSION) << "\n";
    cout << " -------------------------------------------------------- \n\n";

/*    // create a precompiled list to draw spheres
    sphereList = glGenLists(1);
    glNewList(sphereList, GL_COMPILE);
        ViewerUtils::draw_sphere(1.0, 40, 40);
    glEndList();
*/

    // init the viewer

    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    glDisable(GL_CULL_FACE);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(true);
    glDepthFunc(GL_LEQUAL);
    //glDepthFunc(GL_LESS);
/*
    // lighting
    GLfloat ambientLight[] = {0.5f, 0.5f, 0.5f, 1.0f};
    GLfloat lightColor[] = {0.8f, 0.8f, 0.8f, 1.0f};

    GLfloat lightPos[] = {1.0, 1.0, 1.0, 1.0 };

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);

    //Diffuse (non-shiny) light component
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);

    //Specular (shiny) light component
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightColor);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    //Disable color materials, so that glMaterial calls work
    glDisable(GL_COLOR_MATERIAL);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
*/


    // load the park map and bind as opengl texture!
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
    }


  // Restore previous viewer state.
  //restoreStateFromFile();

    // Opens help window
  //help();
}
void VASTViewer::keyPressEvent(QKeyEvent* event){
/*
    switch(event->key()){

    case '2':   showrideno = (showrideno+1) % parentApp->aPark.rides.size();
                //parentApp->ui.groupBox_anim->setChecked( !parentApp->ui.groupBox_anim->isChecked() );
                break;
    case '1':   if(showrideno > 0)  showrideno--;
                else                showrideno = parentApp->aPark.rides.size()-1;
                //parentApp->ui.groupBox_anim->setChecked( !parentApp->ui.groupBox_anim->isChecked() );
                break;
    }
    updateGL();*/
}

void VASTViewer::draw(){

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

    map<int, ParkRide>::const_iterator riter = parentApp->aPark.rides.begin();
    for(; riter != parentApp->aPark.rides.end(); riter++){

        //if(showrideno != std::distance( (map<coords, ParkRide>::const_iterator)parentApp->aPark.rides.begin(), riter))
          //  continue;

        const ParkRide &ride = riter->second;
        int psize = ride.get_load(currtime) * size_factor;

        glPointSize(psize);
        glBegin(GL_POINTS);
            glVertex3f(ride.ride_coords.first, ride.ride_coords.second, 0);
        glEnd();

        //printf(" %d -- %d,%d\n", showrideno, riter->first.first, riter->first.second);
    }


    /*// draw events
    glBegin(GL_POINTS);
    for(uint i = 0; i < num_events; i++){

        const ParkEvent &e = parentApp->events[i];
        if(e.timestamp > currtime)
            break;
        glVertex3f(e.X, e.Y, 0.01);
    }
    glEnd();
*/
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
    live_animation_time = (live_animation_time + parentApp->ui.sb_animSpeed->value()) % (parentApp->aPark.time_end-parentApp->aPark.time_beg);
}

/******************************************
* Environment/Compiler:  XCode 15.4
*
* Interactions:
*                 North View: '1'
*                 East View: '2'
*                 South View: '3'
*                 West View: '4'
*                 Wire mode: 'm'
*                 Wing Flap: 'w/W'
*                 Fly: 'l/L'
*                 Apple Dropping: 'a/A'
*                 Lemon Dropping: 'e/E'
*                 Reset: 'r/R'
*                 Flight Path: 'v/V'
*                 Increase fly speed: 'up arrow'
*                 Decrease fly speed: 'down arrow'
*                 Increase wing speed: 'right arrow'
*                 Decrease wing speed: 'left arrow'
*                 Left click on bird to toggle fly
*                 Left click on lemon to toggle lemon dropping
*                 Left click on apple to toggle apple dropping
*
*                 Use NaiveColorPickingDemo.cpp as reference for color picking
*******************************************/

#include <iostream>
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

using namespace std;

//Globals

// Color Picking
bool selecting = false;
int xClick, yClick;
bool picking = false;
const GLubyte birdColor[3] = {0, 255, 0};    //Green
const GLubyte appleColor[3] = {255, 0, 0};   //Red
const GLubyte lemonColor[3] = {255, 255, 0}; //Yellow

// Wire Mode & Perspectives
bool wireMode = false;
int width = 1200, height = 600;
int view = 0; // 0 = North, 1 = East, 2 = South, 3 = West

// Flight path
bool showFlightPath = false;

bool wingFlap = false;    // Toggle wing flapping
float wingAngle = 0.0;    // Wing angle for flapping
bool wingGoingUp = true;  // Direction of wing flap

// Flight related variables
bool birdFlying = false;
float birdX = -8.3, birdY = 3.0, birdZ = -6.5;  // Initial bird position
int currentFlightSegment = 0;
float flightProgress = 0.0;
float flightSpeed = 0.009; // Initial flight speed
float wingSpeedMultiplier = 1.0; // Wing flap speed multiplier
bool wingsFlappingDuringFlight = true;

// Bird flight path (nest -> above trees -> nest)
const int flightPoints = 6;

float flightPath[flightPoints][3] = {
    {-8.3, 3.0, -6.5},
    {-8.3, 5.0, -6.5},
    {8.3, 5.0, -6.5},
    {8.3, 5.0, 6.5},
    {-8.3, 5.0, 6.5},
    {-8.3, 3.0, -6.5}
};

// Fruits
float apple1X = 8.0, apple1Y = 4.0, apple1Z = 7.0;
float apple2X = 8.5, apple2Y = 3.5, apple2Z = 7.0;
float lemonX = 8.0, lemonY = 3.5, lemonZ = -7.0;
bool appleDropping = false;
bool lemonDropping = false;


void drawBird()
{
    GLUquadric* quad = gluNewQuadric();
    gluQuadricDrawStyle(quad, wireMode ? GLU_LINE : GLU_FILL);
    glPushMatrix();
    glTranslated(0, 3, 0); // Raise bird above ground

    if (picking)
        glColor3ub(255, 0, 0);
    else
        glColor3f(0.0, 1.0, 0.0);
    if (wireMode)
        glutWireSphere(1.2, 20, 20);
    else
        glutSolidSphere(1.2, 20, 20);
    if (!picking)
    {
        // Eyes - left
        glColor3f(1.0, 1.0, 1.0);
        glPushMatrix();
        glTranslated(0.5, 0.8, 0.7);
        if (wireMode)
            glutWireSphere(0.2, 10, 10);
        else
            glutSolidSphere(0.2, 10, 10);
        
        // Pupil
        glColor3f(0.0, 0.0, 0.0);
        glTranslated(0.05, 0.05, 0.1);
        if (wireMode)
            glutWireSphere(0.15, 5, 5);
        else
            glutSolidSphere(0.13, 5, 5);
        glPopMatrix();
        
        // Eyes - right
        glColor3f(1.0, 1.0, 1.0);
        glPushMatrix();
        glTranslated(0.5, 0.8, -0.7);
        if (wireMode)
            glutWireSphere(0.2, 10, 10);
        else
            glutSolidSphere(0.2, 10, 10);
        glColor3f(0.0, 0.0, 0.0);
        glTranslated(0.05, 0.05, -0.1);
        if (wireMode)
            glutWireSphere(0.15, 5, 5);
        else
            glutSolidSphere(0.15, 5, 5);
        glPopMatrix();
        
        // Beak
        glColor3f(1.0, 0.8, 0.0);
        glPushMatrix();
        glTranslated(1.1, 0.5, 0);
        glRotated(90, 0, 1, 0);
        if (wireMode)
            glutWireCone(0.15, 0.4, 10, 10);
        else
            glutSolidCone(0.15, 0.4, 10, 10);
        glPopMatrix();
        
        // Wings - darker pink flattened ovals with flap animation
        glColor3f(0.8, 0.2, 0.5);
        
        // Wing flapping animation
        glPushMatrix();
        glTranslated(0, 0, -1.2);
        glRotated(wingAngle, 1, 0, 0);
        glScaled(0.8, 0.05, 0.4);
        if (wireMode)
            glutWireSphere(1.0, 20, 20);
        else
            glutSolidSphere(1.0, 20, 20);
        glPopMatrix();
        glPushMatrix();
        glTranslated(0, 0, 1.2);
        glRotated(-wingAngle, 1, 0, 0);
        glScaled(0.8, 0.05, 0.4);
        if (wireMode)
            glutWireSphere(1.0, 20, 20);
        else
            glutSolidSphere(1.0, 20, 20);
        glPopMatrix();
    }
    
    glPopMatrix();
    gluDeleteQuadric(quad);
}

void animateWing()
{
    if ((birdFlying && wingsFlappingDuringFlight) || wingFlap) {
        if (wingGoingUp) {
            wingAngle += 20.0 * wingSpeedMultiplier;
            if (wingAngle >= 35) wingGoingUp = false;
        } else {
            wingAngle -= 20.0 * wingSpeedMultiplier;
            if (wingAngle <= -35) wingGoingUp = true;
        }
    } else {
        wingAngle = 0.0; // wings rest flat when not flapping
    }
    glutPostRedisplay();
}

// used MJBclownW2timers.cpp as reference
void flightAnimation(int value) {
    if (birdFlying) {
        flightProgress += flightSpeed;
        if (flightProgress >= 1.0) {
            flightProgress = 0.0;
            currentFlightSegment++;
            if (currentFlightSegment >= flightPoints - 1) {
                birdFlying = false;
                currentFlightSegment = 0;
                wingFlap = false;
            }
        }
        
        birdX = (1 - flightProgress) * flightPath[currentFlightSegment][0] + flightProgress * flightPath[currentFlightSegment + 1][0];
        birdY = (1 - flightProgress) * flightPath[currentFlightSegment][1] + flightProgress * flightPath[currentFlightSegment + 1][1];
        birdZ = (1 - flightProgress) * flightPath[currentFlightSegment][2] + flightProgress * flightPath[currentFlightSegment + 1][2];
        glutPostRedisplay();
        glutTimerFunc(16, flightAnimation, 0);
    }
}

void identifyObject(int x, int y)
{
    unsigned char pixel[3];
    glReadPixels(x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
    
    cout << "R: " << (int)pixel[0]
         << " G: " << (int)pixel[1]
         << " B: " << (int)pixel[2] << endl;
    
    if (pixel[0] == 255 && pixel[1] == 0 && pixel[2] == 0) {
        if (!birdFlying) {
            birdFlying = true;
            wingFlap = true;
            flightProgress = 0.0;
            currentFlightSegment = 0;
            glutTimerFunc(0, flightAnimation, 0);
        }
    }
    selecting = false;
}

void drawApple() {
    glColor3f(1.0, 0.0, 0.0); // Red color
    if (wireMode)
        glutWireSphere(0.3, 15, 15);
    else
        glutSolidSphere(0.3, 15, 15);
}

void drawLemon() {
    glColor3f(1.0, 1.0, 0.0); // Yellow color
    glPushMatrix();
        glScalef(1.0, 0.7, 1.0); // Scale to create an ellipsoid
        if (wireMode)
            glutWireSphere(0.4, 15, 15);
        else
            glutSolidSphere(0.4, 15, 15);
    glPopMatrix();
}

void dropFruits(int value) {
    if (appleDropping) {
        if (apple1Y > 0.5) apple1Y -= 0.05;
        if (apple2Y > 0.5) apple2Y -= 0.05;
        if (apple1Y <= 0.5 && apple2Y <= 0.5) appleDropping = false;
    }
    
    if (lemonDropping) {
        if (lemonY > 0.5) lemonY -= 0.05;
        else lemonDropping = false;
    }
    glutPostRedisplay();
    
    if (appleDropping || lemonDropping)
        glutTimerFunc(16, dropFruits, 0);
}

void drawFlightPath()
{
    if (!showFlightPath) return;
    
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0x0F0F);
    
    glColor3f(0.0, 0.0, 0.0);
    glLineWidth(0.8);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < flightPoints; i++)
    {
        glVertex3f(flightPath[i][0], flightPath[i][1], flightPath[i][2]);
    }
    glEnd();
    
    glDisable(GL_LINE_STIPPLE);
}

void drawSceneContents()
{
    glEnable(GL_DEPTH_TEST);
    
    // Grass base (used LegoLookAt.cpp as reference for the coordinates)
    glColor3f(0.0, 0.8, 0.0);
    glPushMatrix();
    glTranslated(0, -0.5, 0);
    glScaled(20, 1, 20);
    if (wireMode)
        glutWireCube(1);
    else
        glutSolidCube(1);
    glPopMatrix();
    
    // Trees
    GLUquadric* quad = gluNewQuadric();
    gluQuadricDrawStyle(quad, wireMode ? GLU_LINE : GLU_FILL);
    double baseOffset = 8;
    double trunkHeight = 2.5;
    auto drawTree = [&](double x, double z, int type, double scaleX, double scaleY, double scaleZ, double coneRadius, double coneHeight) {
        glColor3f(0.5, 0.35, 0.05);
        glPushMatrix();
        glTranslated(x, 0, z);
        glRotated(-90, 1, 0, 0);
        gluCylinder(quad, 0.4, 0.4, trunkHeight, 20, 20);
        glPopMatrix();
        glColor3f(0.0, 0.6, 0.0);
        glPushMatrix();
        glTranslated(x, trunkHeight, z);
        if (type == 0) { // Sphere
            glTranslated(0, 0.8 * scaleY, 0);
            glScaled(scaleX, scaleY, scaleZ);
            if (wireMode)
                glutWireSphere(1.0, 20, 20);
            else
                glutSolidSphere(1.0, 20, 20);
        } else { // Cone
            glRotated(-90, 1, 0, 0);
            if (wireMode)
                glutWireCone(coneRadius, coneHeight, 20, 20);
            else
                glutSolidCone(coneRadius, coneHeight, 20, 20);
        }
        glPopMatrix();
    };
    drawTree(-baseOffset, -baseOffset, 0, 1.5, 1.5, 1.5, 0, 0);
    drawTree(baseOffset, -baseOffset, 1, 0, 0, 0, 2.0, 2.5);
    drawTree(baseOffset, baseOffset, 0, 1.2, 1.4, 1.0, 0, 0);
    drawTree(-baseOffset, baseOffset, 1, 0, 0, 0, 2.0, 4);
    gluDeleteQuadric(quad);
    
    // Sun
    glColor3f(1.0, 1.0, 0.0);
    glPushMatrix();
    glTranslated(10, 11, 0);
    if (wireMode)
        glutWireSphere(1.0, 20, 20);
    else
        glutSolidSphere(1.0, 20, 20);
    glPopMatrix();
    
    //Apples
    glColor3f(1.0, 0.0, 0.0);
    glPushMatrix();
    glTranslated(8.0, apple1Y, 7.0);
    glScaled(0.3, 0.3, 0.3);
    if (wireMode)
        glutWireSphere(1.0, 20, 20);
    else
        glutSolidSphere(1.0, 20, 20);
    glPopMatrix();
    glPushMatrix();
    glTranslated(8.5, apple2Y, 7.0);
    glScaled(0.3, 0.3, 0.3);
    if (wireMode)
        glutWireSphere(1.0, 20, 20);
    else
        glutSolidSphere(1.0, 20, 20);
    glPopMatrix();
    
    // Lemon
    glColor3f(1.0, 1.0, 0.0);
    glPushMatrix();
    glTranslated(8.0, lemonY, -7.0);
    glScaled(0.4, 0.3, 0.3);
    if (wireMode)
        glutWireSphere(1.0, 20, 20);
    else
        glutSolidSphere(1.0, 20, 20);
    glPopMatrix();
    
    // Nest
    quad = gluNewQuadric();
    gluQuadricDrawStyle(quad, wireMode ? GLU_LINE : GLU_FILL);
    glColor3f(0.55, 0.27, 0.07); // Brown
    glPushMatrix();
        glTranslated(-9, 4, -7);
        glRotated(-90, 1, 0, 0);
        gluCylinder(quad, 0.6, 0.8, 0.4, 20, 10);
        gluDisk(quad, 0, 0.6, 20, 1);
    glPopMatrix();
    
    glPushMatrix();
        glTranslated(-8.3, 4.0, -6.5);
        glRotated(-90, 1, 0, 0);
        gluCylinder(quad, 0.6, 0.8, 0.4, 20, 10);
        gluDisk(quad, 0, 0.6, 20, 1);
    glPopMatrix();
    
    float angle = atan2(flightPath[currentFlightSegment + 1][2] - flightPath[currentFlightSegment][2],
                        flightPath[currentFlightSegment + 1][0] - flightPath[currentFlightSegment][0]) * 180 / 3.14159;
    angle = 35 - angle;
    glPushMatrix();
        glTranslated(birdX, birdY, birdZ);
        glRotated(angle, 0, 1, 0);
        glScaled(0.6, 0.6, 0.6);
        drawBird();
    glPopMatrix();
    
    //gluDeleteQuadric(quad);
    drawFlightPath();

}
void drawScene(void)
{
    if (selecting)
    {
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(90, (float)width / height, 1, 50);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(0, 5, -20, 0, 0, 0, 0, 1, 0); // Match left view
        picking = true;
        drawSceneContents();
        picking = false;
        glFlush();

        unsigned char pixel[3];
        glReadPixels(xClick, yClick, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
        cout << "R: " << (int)pixel[0] << " G: " << (int)pixel[1] << " B: " << (int)pixel[2] << endl;
        
        if (pixel[0] == 255 && pixel[1] == 0 && pixel[2] == 0) {
            if (!birdFlying) {
                birdFlying = true;
                wingFlap = true;
                flightProgress = 0.0;
                currentFlightSegment = 0;
                glutTimerFunc(0, flightAnimation, 0);
            }
        }
        selecting = false;
        glutPostRedisplay();
    }
    
    else
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // ---- left viewport (used spaceTravel.cpp as reference for writing 2 viewports)
        glViewport(0, 0, width / 2, height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(90, (float)(width / 2) / height, 1, 50);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        if (view == 0) gluLookAt(0, 5, -20, 0, 0, 0, 0, 1, 0);      // North
        else if (view == 1) gluLookAt(20, 5, 0, 0, 0, 0, 0, 1, 0);  // East
        else if (view == 2) gluLookAt(0, 5, 20, 0, 0, 0, 0, 1, 0);  // South
        else if (view == 3) gluLookAt(-20, 5, 0, 0, 0, 0, 0, 1, 0); // West
        glPushMatrix();
            glScaled(1.1, 1.1, 1.1);
            drawSceneContents();
        glPopMatrix();
        
        // right viewport (overhead)
        glViewport(width / 2, 0, width / 2, height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(90, (float)(width / 2) / height, 1, 50);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(0, 25, 0, 0, 0, 0, 0, 0, -1);
        glPushMatrix();
            glScaled(1.1, 1.1, 1.1);
            drawSceneContents();
        glPopMatrix();
        
        // vertical line divider
        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(0, width, 0, height);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glDisable(GL_DEPTH_TEST);
        glColor3f(0.0, 0.0, 0.0);
        glLineWidth(2.0);
        glBegin(GL_LINES);
            glVertex2i(width / 2, 0);
            glVertex2i(width / 2, height);
        glEnd();
        glLineWidth(1.0);
        glutSwapBuffers();
    }
}

// Setup
void setup(void)
{
    glClearColor(0.53, 0.81, 0.92, 1.0);
    glEnable(GL_DEPTH_TEST);
}

// Reshape
void resize(int w, int h) {
    width = w;
    height = h;
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90, (float)w / (float)h, 1, 50);
    glMatrixMode(GL_MODELVIEW);
}

// Key input
void keyInput(unsigned char key, int x, int y)
{
    switch (key)
    {
        case '1': view = 0; glutPostRedisplay(); break; // North
        case '2': view = 1; glutPostRedisplay(); break; // East
        case '3': view = 2; glutPostRedisplay(); break; // South
        case '4': view = 3; glutPostRedisplay(); break; // West
        case 'm': wireMode = !wireMode; glutPostRedisplay(); break;
        case 'w':
        case 'W':
            wingFlap = !wingFlap;
            if (birdFlying) wingsFlappingDuringFlight = wingFlap; // allow toggling while flying
            glutPostRedisplay();
            break;
        case 'l':
        case 'L':
            if (!birdFlying) {
                birdFlying = true;
                wingFlap = true;    // Automatically enable wing flapping during flight
                flightProgress = 0.0;
                currentFlightSegment = 0;
                glutTimerFunc(0, flightAnimation, 0);
            }
            break;
        case 'a':
        case 'A':
            appleDropping = true;
            glutTimerFunc(0, dropFruits, 0);
            break;
        case 'e':
        case 'E':
            lemonDropping = true;
            glutTimerFunc(0, dropFruits, 0);
            break;
        case 'r':
        case 'R':
            birdX = -8.3; birdY = 3.0; birdZ = -6.5;
            currentFlightSegment = 0;
            flightProgress = 0.0;
            birdFlying = false;
            wingFlap = false;
            view = 0;
            // Reset fruit positions
            apple1Y = 4.0;
            apple2Y = 3.5;
            lemonY = 3.5;
            glutPostRedisplay();
            break;
        case 'v':
        case 'V':
            showFlightPath = !showFlightPath;
            glutPostRedisplay();
            break;

        case 27: exit(0); break;
    }
}

void specialKeyInput(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:
            flightSpeed += 0.003;
            cout << "Flight speed increased: " << flightSpeed << endl;
            break;
        case GLUT_KEY_DOWN:
            flightSpeed -= 0.003;
            if (flightSpeed < 0.002f) flightSpeed = 0.002f;
            cout << "Flight speed decreased: " << flightSpeed << endl;
            break;
        case GLUT_KEY_RIGHT:
            wingSpeedMultiplier += 0.2;
            cout << "Wing speed increased: " << wingSpeedMultiplier << endl;
            break;
        case GLUT_KEY_LEFT:
            wingSpeedMultiplier -= 0.2;
            if (wingSpeedMultiplier < 0.2f) wingSpeedMultiplier = 0.2f;
            cout << "Wing speed decreased: " << wingSpeedMultiplier << endl;
            break;
    }
    glutPostRedisplay();
}

void mouseClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        y = viewport[3] - y;

        GLubyte pixel[3];
        glReadPixels(x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);

        // Determine which object was clicked
        if (pixel[0] == birdColor[0] && pixel[1] == birdColor[1] && pixel[2] == birdColor[2]) {
            // Bird was clicked
            if (!birdFlying) {
                birdFlying = true;
                wingFlap = true;
                flightProgress = 0.0;
                currentFlightSegment = 0;
                glutTimerFunc(0, flightAnimation, 0);
            }
            
        } else if (pixel[0] == appleColor[0] && pixel[1] == appleColor[1] && pixel[2] == appleColor[2]) {
            // Apple was clicked
            appleDropping = true;
            glutTimerFunc(0, dropFruits, 0);
        } else if (pixel[0] == lemonColor[0] && pixel[1] == lemonColor[1] && pixel[2] == lemonColor[2]) {
            // Lemon was clicked
            lemonDropping = true;
            glutTimerFunc(0, dropFruits, 0);
        }
    }
}

void printInteraction(void)
{
    cout << "Controls:" << endl;
    cout << "L - Make bird fly" << endl;
    cout << "A - Drop apples" << endl;
    cout << "E - Drop lemon" << endl;
    cout << "R - Reset scene" << endl;
    cout << "V - Toggle flight path" << endl;
    cout << "W - Toggle wing flap" << endl;
    cout << "m - Toggle wireframe mode" << endl;
    cout << "Arrow Keys - Adjust speed" << endl;
    cout << "1 - North" << endl;
    cout << "2 - East" << endl;
    cout << "3 - South" << endl;
    cout << "4 - West" << endl;
}

// Main
int main(int argc, char **argv)
{
    printInteraction();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Assignment 3 - A Bird Flies over an Orchard");
    setup();
    glutDisplayFunc(drawScene);
    glutReshapeFunc(resize);
    glutKeyboardFunc(keyInput);
    glutIdleFunc(animateWing);
    glutMouseFunc(mouseClick);
    glutSpecialFunc(specialKeyInput);
    glutMainLoop();
    return 0;
}



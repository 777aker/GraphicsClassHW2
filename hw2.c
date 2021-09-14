/* Lorenz Attractor HW
* Hopefully, a fun visualization of the Lorenz Attractor
* Kelley Kelley
*/
/* Inputs:
* Arrow keys rotate view
* space resets view
* l changes between the four line modes
* s increments s value S decrements
* b increments b value B decrements
* r increments r value R decrements
* ESC quits the program
* 1, 2 some cool Lorenz parameters
*/
// a lot of imports
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#ifdef USEGLEW
#include <GL/glew.h>
#endif
//  OpenGL with prototypes for glext
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
//  Default resolution
//  For Retina displays compile with -DRES=2
#ifndef RES
#define RES 1
#endif
// my added includes
#include <stdbool.h>
#include <math.h>

// convenience function for raster text
void Print(const char* format, ...);
// error function
void Fatal(const char* format, ...);
// OpenGL error checker
void ErrCheck(const char* where);
// call this function when a key is pressed
void key(unsigned char ch, int x, int y);
// call this function for special keys (kinda weird tbh that
// it's not the same as just keys)
void special(int key, int x, int y);
// call this for fixing window resize
void reshape(int width, int height);
// displaying stuff (the important bit)
void display();
// so here comes the Lorenz part which is cool
// this lorenz just draws the entire lorenz
void lorenz();

// variable time
int th = 45; // aximuth of view angle
int ph = 45; // elevation of view angle
double dim = 50; // dimension of orthogonal box
// lorenz starting parameters
double s = 78;
double b = 2.6666;
double r = 28;
// time step
double dt = 0.001;

// so this is a struct that basically represents
// a pyramid. Stores its position and change in position
// pass it to lorenz lorenz gives it a new position
// and stores the change
struct point {
	double x;
	double y;
	double z;
	double dx;
	double dy;
	double dz;
};
// this is a function that will draw our object at 
// a specific point and rotation
// practiced modeling it in hw2modeling
void object(struct point pos);
// this is a version of lorenz that takes in a point
// and returns the next point. purely for drawing
// things along the lorenz path
struct point lorenzret(struct point cur_point);
// one global position to work with
const int lenpos = 50;
// this stores all 50 of our traveling pyramids
struct point positions[50];
// the origin, just a convenience thing for drawing
// and accessing and reseting all 50 of the other positions
struct point origin;
// there is no point if they are all on top of each other so
// this will keep track of how long till we spawned the last point
int timer = 0;
// this is for keeping track of how many of the positions we
// need to actually go through (makes more sense later)
int drawn = 1;
// which line drawing mode we are in
int lines = 3;
// so, I want to draw the lorenz in color where the color is how big
// the change is. So I needed some arbitrary value to divide the changes
// by so I can color it nicely. Anything bigger than 400 is 1, 0, 1
// anything smaller will be change/400, 0, 1
double greatest = 400;

// function that takes in a point and returns the next point
struct point lorenzret(struct point cur_point) {
	// this is just the lorenz part
	double dx = s*(cur_point.y-cur_point.x);
	double dy = cur_point.x*(r-cur_point.z)-cur_point.y;
	double dz = cur_point.x*cur_point.y - b*cur_point.z;
	// this I need the change in position so I can rotate
	// the object to point toward where it is going
	cur_point.dx = dx;
	cur_point.dy = dy;
	cur_point.dz = dz;
	// this really just scales it down so the lorenz isn't unreasonably
	// huge
	cur_point.x += dt*dx;
	cur_point.y += dt*dy;
	cur_point.z += dt*dz;
	return cur_point;
}

// ah, display, a classic
void display() {
	// if our timer reaches the arbitrary value of 10,
	// then spawn another man to go along the lorenz
	timer++;
	if(timer >= 10) {
		timer = 0;
		drawn++;
	}
	// clear the image
	glClear(GL_COLOR_BUFFER_BIT);
	// reset previous transforms
	glLoadIdentity();
	// set view angle
	glRotated(ph, 1, 0, 0);
	glRotated(th, 0, 1, 0);
	// draw the base lorenz
	lorenz();
	
	// I was using this while they were points and not objects
	// not longer needed
	//glPointSize(5);
	
	// a for loop that will draw each of our little men who go across the screen
	int i;
	for(i = 0; i < lenpos && i < drawn; i++) {
		positions[i] = lorenzret(positions[i]);
		//glColor3f(.5, 0, .5);
		//glBegin(GL_POINTS);
		//glVertex3d(positions[i].x, positions[i].y, positions[i].z);
		//glEnd();
		// replace points with objects
		// this is a function I built in hw2modeling
		// it draws a 3d object, places it, and rotates it
		object(positions[i]);
		glColor3f(1, 1, 1);
		// switch based off the line mode we are in
		// wait, a switch would've been better here.....oops
		// eh, no one ever cares about repeated code
		// note: draws no lines on lines == 0
		if(lines == 1) {
			// draw some lines to the origin and color based on distance to origin
			glBegin(GL_LINES);
			double distance = pow(pow(positions[i].x, 2) + pow(positions[i].y, 2) + pow(positions[i].z, 2), .5);
			distance /= dim/2;
			glColor3f(distance, 0, 1/distance);
			glVertex3d(1, 1, 1);
			glVertex3d(positions[i].x, positions[i].y, positions[i].z);
			glEnd();
		} else if(lines == 2) {
			// draw some lines to each other
			if(i != 0) {
				glBegin(GL_LINES);
				double distance = pow(pow(positions[i].x, 2) + pow(positions[i].y, 2) + pow(positions[i].z, 2), .5);
				distance /= dim/2;
				glColor3f(distance, 1, 1/distance);
				glVertex3d(positions[i-1].x, positions[i-1].y, positions[i-1].z);
				glVertex3d(positions[i].x, positions[i].y, positions[i].z);
				glEnd();
			}
		} else if(lines == 3) {
			// draw lines to the origin and each other
			// also color lines based on distance to origin
			if(i != 0) {
				glBegin(GL_LINE_STRIP);
				double distance = pow(pow(positions[i].x, 2) + pow(positions[i].y, 2) + pow(positions[i].z, 2), .5);
				distance /= dim/2;
				glColor3f(distance, 0, 1/distance);
				glVertex3d(positions[i-1].x, positions[i-1].y, positions[i-1].z);
				glVertex3d(positions[i].x, positions[i].y, positions[i].z);
				glVertex3d(1, 1, 1);
				glEnd();
			} else {
				glBegin(GL_LINES);
				double distance = pow(pow(positions[i].x, 2) + pow(positions[i].y, 2) + pow(positions[i].z, 2), .5);
				distance /= dim/2;
				glColor3f(distance, 0, 1/distance);
				glVertex3d(1, 1, 1);
				glVertex3d(positions[i].x, positions[i].y, positions[i].z);
				glEnd();
			}
		}
	}
	//position = lorenzret(position);
	//glVertex3d(position.x, position.y, position.z);
	
	// ripped code from ex6
	//  Draw axes in white
	glColor3f(1,1,1);
	glBegin(GL_LINES);
	glVertex3d(0,0,0);
	glVertex3d(45,0,0);
	glVertex3d(0,0,0);
	glVertex3d(0,45,0);
	glVertex3d(0,0,0);
	glVertex3d(0,0,45);
	glEnd();
	//  Label axes
	glRasterPos3d(48,0,0);
	Print("X");
	glRasterPos3d(0,48,0);
	Print("Y");
	glRasterPos3d(0,0,48);
	Print("Z");
	// display parameters
	glWindowPos2i(5, 65);
	Print("View Angle=%d, %d\n", th, ph);
	glWindowPos2i(5, 45);
	Print("Cool things: 1, 2");
	glWindowPos2i(5, 25);
	Print("Change parameters: s, b, r, S, B, R, l, space\n");
	glWindowPos2i(5, 5);
	Print("s = %f, b = %f, r = %f\n", s, b, r);
	//printf("%f %f %f", s, b, r);
	// flush and swap
	ErrCheck("display");
	glFlush();
	glutSwapBuffers();
	glutPostRedisplay();
}

void lorenz() {
	// for loop boy
	int i;
	// points
	double x = 1;
	double y = 1;
	double z = 1;
	// time step is now global
	//double dt = 0.001;

	// for finding the maximum z so we can reframe
	// to fit Lorenz attractor
	//double zmax = 1;

	//glColor3f(0, 0, 1);
	//glPointSize(10);
	// draw the lorenz attractor
	glBegin(GL_LINE_STRIP);
	for(i=0; i<50000; i++) {
		// parameters are global
		double dx = s*(y-x);
		double dy = x*(r-z)-y;
		double dz = x*y - b*z;
		// also color based on change in position
		double distance = pow(pow(dx, 2) + pow(dy, 2) + pow(dz, 2), .5);
		glColor3f(distance/greatest, 0, 1);
		//if(distance > greatest)
		//	greatest = distance;
		x += dt*dx;
		y += dt*dy;
		z += dt*dz;
		//if(z > zmax)
		//	zmax = z;
		glVertex3d(x, y, z);
	}
	glEnd();

	//printf("%f\n", greatest);
}

// this function handles a lot of input
void key(unsigned char ch, int x, int y) {
	// do we need to reset our positions?
	bool reset = 0;
	// exit on ESC
	if(ch == 27)
		exit(0);
	// change the lorenz paramters and reset our men
	// to go along the new lorenz
	else if(ch == 's') {
		s += 2;
		reset = 1;
	} else if(ch == 'S') {
		s -= 2;
		reset = 1;
	} else if(ch == 'b') {
		b += .5;
		reset = 1;
	} else if(ch == 'B') {
		b -= .5;
		reset = 1;
	} else if(ch == 'r') {
		r += 1.5;
		reset = 1;
	} else if(ch == 'R') {
		r -= 1.5;
		reset = 1;
	// reset to the original paramters
	} else if(ch == ' ') {
		s = 78;
		b = 2.6666;
		r = 28;
		reset = 1;
		th = 45;
		ph = 45;
		lines = 3;
	// which line mode we in
	} else if(ch == 'l') {
		lines++;
		lines %= 4;
	} else if(ch == '1') {
		s = -2;
		b = 2.6666;
		r = 28;
		reset = 1;
		th = 15;
		ph = 10;
	} else if(ch == '2') {
		s = 12;
		b = 6.1666;
		r = 32.5;
		reset = 1;
		th = 135;
		ph = 15;
	}
	// reset all of our traveling objects
	if(reset) {
		int i;
		drawn = 0;
		for(i = 0; i < lenpos; i++)
			positions[i] = origin;
	}
	glutPostRedisplay();
}

void special(int key, int x, int y) {
	//  Right arrow key - increase azimuth by 5 degrees
	if (key == GLUT_KEY_RIGHT)
  		th += 5;
	//  Left arrow key - decrease azimuth by 5 degrees
	else if (key == GLUT_KEY_LEFT)
		th -= 5;
	//  Up arrow key - increase elevation by 5 degrees
	else if (key == GLUT_KEY_UP)
  		ph += 5;
	//  Down arrow key - decrease elevation by 5 degrees
	else if (key == GLUT_KEY_DOWN)
  		ph -= 5;
	//  Keep angles to +/-360 degrees
	th %= 360;
	ph %= 360;
	//  Tell GLUT it is necessary to redisplay the scene
	glutPostRedisplay();
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[]) {
	// intialize some of our positions
	origin.x = 1;
	origin.y = 1;
	origin.z = 1;
	int i;
	for(i = 0; i < lenpos; i++)
		positions[i] = origin;
  //  Initialize GLUT and process user parameters
   glutInit(&argc,argv);
   //  Request double buffered, true color window 
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
   //  Request 500 x 500 pixel window
   glutInitWindowSize(500, 500);
   //  Create the window
   glutCreateWindow("Kelley Kelley HW2");
#ifdef USEGLEW
   //  Initialize GLEW
   if (glewInit()!=GLEW_OK) Fatal("Error initializing GLEW\n");
#endif
   // depth test and face cull since we doing some 3D stuff
   // and I want it to show up well, I tried making display
   // mode GLUT_DEPTH or whatever but it broke so moved on
   glEnable(GL_CULL_FACE);
   glEnable(GL_DEPTH_TEST);
   //  Tell GLUT to call "display" when the scene should be drawn
   glutDisplayFunc(display);
  //  Tell GLUT to call "reshape" when the window is resized
   glutReshapeFunc(reshape);
   //  Tell GLUT to call "special" when an arrow key is pressed
   glutSpecialFunc(special);
   //  Tell GLUT to call "key" when a key is pressed
   glutKeyboardFunc(key);
   //  Pass control to GLUT so it can interact with the user
   glutMainLoop();
   //  Return code
   return 0;
}

// drawing a 3D object of some sort
// it's a...double rectangular prism?
void object(struct point pos) {
   glPushMatrix();
   
   // make the object at the location and
   // rotate it to be pointing the direction we 
   // are going

   glTranslated(pos.x, pos.y, pos.z);

   // this took me way too long to figure out
   double xang = atan(pos.dy/pos.dz);
   xang *= 180/M_PI;
   glRotatef(-xang, 1, 0, 0);

   if(pos.dx >= 0) {
      double yang = atan(pos.dz/pos.dx);
      yang *= 180/M_PI;
      glRotatef(-yang, 0, 1, 0);
   } else {
      double yang = atan(pos.dx/pos.dz);
      yang *= 180/M_PI;
      if(pos.z <= 0)
         yang += 90;
      else
         yang -= 90;
      glRotatef(yang, 0, 1, 0);
   }

   // I also tried lots of homegeneous coordinate stuff
   // it never really worked out
   /*
   double magnitude = (dx**2 + dy**2 + dz**2)**.5;
   double matrix[] = {
   	{2, 0, 0, dx},
   	{0, 2, 0, dy},
   	{0, 0, 2, dz},
   	{0, 0, 0, 1},
   };
   glLoadMatrixd(matrix);
   */
   //glRotated(angle, Ux, Uy, Uz);
   glScaled(2, 2, 2);

   /*
   glColor3f(.5, .5, .5);
   glBegin(GL_QUADS);
   glVertex3d(0, -.5, -.5);
   glVertex3d(0, -.5, .5);
   glVertex3d(0, .5, .5);
   glVertex3d(0, .5, -.5);
   glEnd();
   */

   // draw our cool object (each face is a different color...uuuwwww)

   glColor3f(.75, 0, .75);
   glBegin(GL_TRIANGLES);
   glVertex3d(0, -.5, -.5);
   glVertex3d(1, 0, 0);
   glVertex3d(0, -.5, .5);
   glEnd();

   glColor3f(.25, 0, .25);
   glBegin(GL_TRIANGLES);
   glVertex3d(0, .5, -.5);
   glVertex3d(1, 0, 0);
   glVertex3d(0, -.5, -.5);
   glEnd();

   glColor3f(1, 0, 1);
   glBegin(GL_TRIANGLES);
   glVertex3d(0, .5, .5);
   glVertex3d(1, 0, 0);
   glVertex3d(0, .5, -.5);
   glEnd();

   glColor3f(.5, 0, .5);
   glBegin(GL_TRIANGLES);
   glVertex3d(0, -.5, .5);
   glVertex3d(1, 0, 0);
   glVertex3d(0, .5, .5);
   glEnd();

   glColor3f(.6, 0, .6);
   glBegin(GL_TRIANGLES);
   glVertex3d(0, -.5, .5);
   glVertex3d(-.5, 0, 0);
   glVertex3d(0, -.5, -.5);
   glEnd();
   
   glColor3f(.1, 0, .1);
   glBegin(GL_TRIANGLES);
   glVertex3d(0, -.5, -.5);
   glVertex3d(-.5, 0, 0);
   glVertex3d(0, .5, -.5);
   glEnd();

   glColor3f(.85, 0, .85);
   glBegin(GL_TRIANGLES);
   glVertex3d(0, .5, -.5);
   glVertex3d(-.5, 0, 0);
   glVertex3d(0, .5, .5);
   glEnd();

   glColor3f(.35, 0, .35);
   glBegin(GL_TRIANGLES);
   glVertex3d(0, .5, .5);
   glVertex3d(-.5, 0, 0);
   glVertex3d(0, -.5, .5);
   glEnd();

   glPopMatrix();
}

// these are all the boring functions stolen
// from ex6 that just fix stuff and do
// convenience stuff (super helpful, but boring)
void reshape(int width, int height) {
	//  Set the viewport to the entire window
   glViewport(0,0, RES*width,RES*height);
   //  Tell OpenGL we want to manipulate the projection matrix
   glMatrixMode(GL_PROJECTION);
   //  Undo previous transformations
   glLoadIdentity();
   //  Orthogonal projection box adjusted for the
   //  aspect ratio of the window
   double asp = (height>0) ? (double)width/height : 1;
   glOrtho(-asp*dim,+asp*dim, -dim,+dim, -dim,+dim);
   //  Switch to manipulating the model matrix
   glMatrixMode(GL_MODELVIEW);
   //  Undo previous transformations
   glLoadIdentity();
}

// OpenGL error checker
void ErrCheck(const char* where)
{
   int err = glGetError();
   if (err) fprintf(stderr,"ERROR: %s [%s]\n",gluErrorString(err),where);
}

// error printer
void Fatal(const char* format , ...) {
   va_list args;
   va_start(args,format);
   vfprintf(stderr,format,args);
   va_end(args);
   exit(1);
}

#define LEN 8192 // max buf
void Print(const char* format, ...) {
	char buf[LEN];
	char* ch=buf;
	va_list args;
	//  Turn the parameters into a character string
	va_start(args,format);
	vsnprintf(buf,LEN,format,args);
	va_end(args);
	//  Display the characters one at a time at the current raster position
	while (*ch)
  		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,*ch++);
}
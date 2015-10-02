//cs335 Spring 2015 Lab-1
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define MAX_PARTICLES 40000
#define GRAVITY .2

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures

struct Vec {
	float x, y, z;
};

struct Shape {
	float width, height;
	float radius;
	Vec center;
};

struct Particle {
	Shape s;
	Vec velocity;
};

struct Game {
	Shape box[5];
	Shape circle[2];
	Particle particle[MAX_PARTICLES];
	int n;
	int lastMousex;
	int lastMousey;
};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);


int main(void)
{
	int done=0;
	srand(time(NULL));
	initXWindows();
	init_opengl();
	//declare game object
	Game game;
	game.n=0;

	//declare a box shape
	game.box[0].width = 50;
	game.box[0].height = 10;
	game.box[0].center.x = 200 + 5*65;
	game.box[0].center.y = 480 - 5*60;

	game.box[1].width = 50;
	game.box[1].height = 10;
	game.box[1].center.x = 100 + 5*65;
	game.box[1].center.y = 500 - 5*60;

	game.box[2].width = 50;
	game.box[2].height = 10;
	game.box[2].center.x = 0 + 5*65;
	game.box[2].center.y = 520 - 5*60;

	game.box[3].width = 50;
        game.box[3].height = 10;
	game.box[3].center.x = -100 + 5*65;
	game.box[3].center.y = 540 - 5*60;

	game.box[4].width = 50;
        game.box[4].height = 10;
	game.box[4].center.x = -200 + 5*65;
	game.box[4].center.y = 560 - 5*60;

	game.circle[0].center.x = -400 + 5*65;
	game.circle[0].center.y = 50 + 5*65;
	game.circle[0].radius = 50;

	game.circle[1].center.x = 400 + 5*65;
        game.circle[1].center.y = -400 + 5*65;
	game.circle[1].radius = 200;



	//start animation
	while(!done) {
		while(XPending(dpy)) {
			XEvent e;
			XNextEvent(dpy, &e);
			check_mouse(&e, &game);
			done = check_keys(&e, &game);
		}
		movement(&game);
		render(&game);
		glXSwapBuffers(dpy, win);
	}
	cleanupXWindows();
	return 0;
}

void set_title(void)
{
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "335 Lab1   LMB for particle");
}

void cleanupXWindows(void) {
	//do not change
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

void initXWindows(void) {
	//do not change
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		std::cout << "\n\tcannot connect to X server\n" << std::endl;
		exit(EXIT_FAILURE);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if(vi == NULL) {
		std::cout << "\n\tno appropriate visual found\n" << std::endl;
		exit(EXIT_FAILURE);
	} 
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
							ButtonPress | ButtonReleaseMask |
							PointerMotionMask |
							StructureNotifyMask | SubstructureNotifyMask;
	win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
					InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
	//Set the screen background color
	glClearColor(0.1, 0.1, 0.1, 1.0);
}

#define rnd() (float)rand() / (float)RAND_MAX

void makeParticle(Game *game, int x, int y) {
	if (game->n >= MAX_PARTICLES)
		return;
	//std::cout << "makeParticle() " << x << " " << y << std::endl;
	//position of particle
	Particle *p = &game->particle[game->n];
	p->s.center.x = x;
	p->s.center.y = y;
	p->velocity.y =  rnd();
	p->velocity.x =  .5 + rnd() * rnd();
	game->n++;
}

void check_mouse(XEvent *e, Game *game)
{
	static int savex = 0;
	static int savey = 0;
//	static int n = 0;

	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button was pressed
			int y = WINDOW_HEIGHT - e->xbutton.y;
			for(int i=0; i < 1000; i++)
			    makeParticle(game, e->xbutton.x, y);
			return;
		}
		if (e->xbutton.button==3) {
			//Right button was pressed
			return;
		}
	}
	//Did the mouse move?
	if (savex != e->xbutton.x || savey != e->xbutton.y) {
		savex = e->xbutton.x;
		savey = e->xbutton.y;
		int y = WINDOW_HEIGHT - e->xbutton.y;

		for(int i=0; i < 10; i++)
			makeParticle(game, e->xbutton.x, y);
		
		game->lastMousex = e->xbutton.x;
		game->lastMousey = y;
//		if (++n < 10)
//			return;
	}
}

int check_keys(XEvent *e, Game *game)
{
	//Was there input from the keyboard?
	if (e->type == KeyPress) {
		int key = XLookupKeysym(&e->xkey, 0);
		if (key == XK_Escape) {
			return 1;
		}
		//You may check other keys here.

	}
	return 0;
}

void movement(Game *game)
{
	Particle *p;

	if (game->n <= 0)
		return;

	for(int i=0; i<100; i++)
	        makeParticle(game, game->lastMousex, game->lastMousey);

	for(int i = 0; i<game->n; i++){
		p = &game->particle[i];
		p->s.center.x += p->velocity.x;
		p->s.center.y += p->velocity.y;
		p->velocity.y -= GRAVITY;

		for(int j = 0; j<5; j++){
		Shape *s  = &game->box[j];

		//check for collision with box shapes...
		//Shape *s;
			if( p->s.center.y < s->center.y + s->height &&
			        p->s.center.y > s->center.y - s->height &&
		  	  	p->s.center.x >= s->center.x - s->width &&
		 	        p->s.center.x <= s->center.x + s->width){
	    	  	  p->velocity.y *= -.3;
			  p->velocity.y += rnd();
			  int pos = 0;
			  pos = rnd() - rnd();
			  if( pos < 0 )
			      pos *= -1;
			  p->velocity.x += pos;
			  p->s.center.y = s->center.y + s->height +.001;
			  }
		}

		//check for collision with circle
		
		for(int l=0; l<2; l++){
		    Shape *s = &game->circle[l];
		    float distance = 0, d1, d0;
		    d0 = p->s.center.x - s->center.x;
		    d1 = p->s.center.y - s->center.y;

		    distance = sqrt(d0*d0 + d1*d1);
		    if(distance <= s->radius){
			p->velocity.y *= -.5;
			if(p->s.center.x < s->center.x){
			p->velocity.x += -.1;
			}
			
			d0 /=distance;
			d1 /=distance;
			d0 *= s->radius *1.01;
			d1 *= s->radius *1.01;
			p->s.center.x = s->center.x + d0;
			p->s.center.y = s->center.y + d1;
			p->velocity.x += d0 * 0.00005;
		
			p->velocity.y += d1 * 0.00005;
			}
		}


		//check for off-screen
		if (p->s.center.y < 0.0 || p->s.center.y > WINDOW_HEIGHT ) {
			//std::cout << "off screen" << std::endl;
			memcpy(&game->particle[i],  &game->particle[game->n-1], sizeof(Particle));
			game->n--;
		}

	}
return;
}

void render(Game *game)
{
	float w, h;
	glClear(GL_COLOR_BUFFER_BIT);
	//Draw shapes...
	for(int i =0; i<5; i++){
		//draw box
		Shape *s;
		glColor3ub(90,140,90);
		s = &game->box[i];
		glPushMatrix();
		glTranslatef(s->center.x, s->center.y, s->center.z);
		w = s->width;
		h = s->height;
		glBegin(GL_QUADS);
			glVertex2i(-w,-h);
			glVertex2i(-w, h);
			glVertex2i( w, h);
			glVertex2i( w,-h);
		glEnd();
		glPopMatrix();
		}
	//Draw Circles
	for(int i=0; i<2; i++){
	        Shape *s;
		glColor3ub(200,0,0);
		s= &game->circle[i];

		int triangles=40000;

		glBegin(GL_TRIANGLE_FAN);
		glVertex2f(s->center.x, s->center.y);

		for(int i = 0; i <= triangles; i++){
		    glVertex2f(
			    s->center.x + (s->radius *cos(i*(2 * 3.14159)/triangles)),
			    s->center.y + (s->radius *sin(i*(2 * 3.14159)/triangles))
			    );
		    
		}
		glEnd();
	}

	//draw all particles here
	for(int i = 0; i<game->n; i++){
		glPushMatrix();
		glColor3ub(150,160,220);
		Vec *c = &game->particle[i].s.center;
		w = .5;
		h = .5;
		glBegin(GL_QUADS);
			glVertex2i(c->x-w, c->y-h);
			glVertex2i(c->x-w, c->y+h);
			glVertex2i(c->x+w, c->y+h);
			glVertex2i(c->x+w, c->y-h);
		glEnd();
		glPopMatrix();
	}
return;
}




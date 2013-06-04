#include "interface.h"

#include <GL/freeglut.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "graph.h"

#define GLUT_KEY_ESC    (27)
#define GLUT_WHEEL_UP   (3)
#define GLUT_WHEEL_DOWN (4)

// texture information

static int   winId;
static int   winWidth  = 1024;
static int   winHeight = 768;
static int   mouseX    = 0;
static int   mouseY    = 0;

static double viewX    = 0;
static double viewY    = 0;
static double viewZoom = 1;

static const double blockScale = 0.1;
static const double blockPadding = 20;

static struct block* fun;
static size_t        funlen;
static struct block* curBlock;

static size_t countChars(const char* str, char c)
{
	size_t ret = 0;
	for (; *str; str++)
		if (*str == c)
			ret++;
	return ret;
}

#define BUFSIZE   1024
#define FONT      GLUT_STROKE_MONO_ROMAN
static void displayBlock(struct block* b)
{
	if (!b || b->drawn) return;
	b->drawn = true;

	glPushMatrix();
	glTranslatef(b->x, b->y, 0);
	if (b == curBlock)
		glColor4f(1, 0, 0, 1);
	else
		glColor4f(1, 1, 1, 1);

	double maxWidth = 0;
	size_t lines = 0;

	glPushMatrix();
		glScalef(blockScale, -blockScale, blockScale);

		for (size_t k = 0; k < b->size; k++)
		{
			unsigned char glText[BUFSIZE];
			snprint_instr((char*) glText, BUFSIZE, b->start+k);

			double textWidth = glutStrokeLength(FONT, glText);
			if (textWidth > maxWidth)
				maxWidth = textWidth;
			lines += countChars((const char*) glText, '\n');
	
			glutStrokeString(FONT, glText);
		}
	glPopMatrix();

	double w = blockScale * maxWidth;
	double h = blockScale * lines * glutStrokeHeight(FONT);
	double p = blockPadding;
	glBegin(GL_LINE_LOOP);
		glVertex2f( -p,  -p);
		glVertex2f(w+p,  -p);
		glVertex2f(w+p, h+p);
		glVertex2f( -p, h+p);
	glEnd();

	glPopMatrix();

	if (b->next)
	{
		glBegin(GL_LINES);
			glVertex2f(b->x+w/2,   b->y+h+p);
			glVertex2f(b->next->x-p, b->next->y-p);
		glEnd();
	}
	if (b->branch)
	{
		glBegin(GL_LINES);
			glVertex2f(b->x+w/2,   b->y+h+p);
			glVertex2f(b->branch->x-p, b->branch->y-p);
		glEnd();
	}

	displayBlock(b->next);
	displayBlock(b->branch);
}

static void cb_displayFunc()
{
//	ftime(&cur);

	glClear(GL_COLOR_BUFFER_BIT);
	glPushMatrix();
	glTranslatef(0.375, 0.375, 0); // hack against pixel centered coordinates

	glTranslatef(winWidth / 2, winHeight / 2, 0);
	glScalef(viewZoom, viewZoom, viewZoom);
	glTranslatef(-viewX, -viewY, 0);

	for (size_t k = 0; k < funlen; k++)
		fun[k].drawn = false;
	displayBlock(fun);

	glPopMatrix();
	glutSwapBuffers();
}

static void cb_keyboardFunc(unsigned char key, int x, int y)
{
	(void) x;
	(void) y;

	if (key == 'r')
	{
		spreadNodes(fun, funlen);
		glutPostRedisplay();
	}
}

static void cb_mouseFunc(int button, int state, int _x, int _y)
{
	(void) state;

	if (button == GLUT_WHEEL_UP)
		viewZoom *= 1.1;
	else if (button == GLUT_WHEEL_DOWN)
		viewZoom /= 1.1;
	else if (button == GLUT_LEFT_BUTTON)
	{
// TODO
	glPushMatrix();
	glTranslatef(0.375, 0.375, 0); // hack against pixel centered coordinates

	glTranslatef(winWidth / 2, winHeight / 2, 0);
	glScalef(viewZoom, viewZoom, viewZoom);
	glTranslatef(-viewX, -viewY, 0);

		GLdouble modelMatrix[16];
		glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
		GLdouble projMatrix[16];
		glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
		int viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		double x, y, z;
		gluUnProject(_x, _y, 0, modelMatrix, projMatrix, viewport, &x, &y, &z);
	glPopMatrix();

		double d_min = -1;
		struct block* b_min = NULL;
		for (size_t k = 0; k < funlen; k++)
		{
			struct block* b = fun+k;
			double dx = x - b->x;
			double dy = y - b->y;
			double d = dx*dx + dy*dy;
			if (d_min == -1 || d < d_min)
			{
				d_min = d;
				b_min = b;
			}
		}
		curBlock = b_min;
	}

	glutPostRedisplay();
}

static void cb_passiveMotionFunc(int x, int y)
{
	if (mouseX == x && mouseY == y)
		return;

	viewX += (x - mouseX) / viewZoom;
	viewY += (y - mouseY) / viewZoom;
	mouseX = winWidth / 2;
	mouseY = winHeight / 2;
	glutWarpPointer(mouseX, mouseY);

	glutPostRedisplay();
}

static void cb_motionFunc(int x, int y)
{
	if (curBlock)
	{
		curBlock->x += (x - mouseX) / viewZoom;
		curBlock->y += (y - mouseY) / viewZoom;
		mouseX = winWidth / 2;
		mouseY = winHeight / 2;
	}

	cb_passiveMotionFunc(x, y);
}

static void glInit()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glPointSize(3);

	// two dimensionnal mode
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, winWidth, winHeight, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glDisable(GL_DEPTH_TEST);

	// enables transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void zui(int argc, char** argv, struct block* _fun, size_t len)
{
	fun = _fun; // TODO
	funlen = len;
	curBlock = fun;

	mouseX = winWidth/2;
	mouseY = winHeight/2;

	spreadNodes(fun, funlen);
	viewX = fun->x;
	viewY = fun->y;

	glutInit(&argc, argv);
	glutInitWindowSize(winWidth, winHeight);
	winId = glutCreateWindow("Execution graph");
//	glutSetCursor(GLUT_CURSOR_NONE);

	glutDisplayFunc      (&cb_displayFunc);
	glutKeyboardFunc     (&cb_keyboardFunc);
	glutMouseFunc        (&cb_mouseFunc);
	glutMotionFunc       (&cb_motionFunc);
	glutPassiveMotionFunc(&cb_passiveMotionFunc);

	glInit();
	glutMainLoop();
}

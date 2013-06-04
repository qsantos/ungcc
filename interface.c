#include "interface.h"

#include <GL/freeglut.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "graph.h"

#define CURSOR_SZ 1

#define GLUT_KEY_ESC    (27)
#define GLUT_WHEEL_UP   (3)
#define GLUT_WHEEL_DOWN (4)

// texture information

static int   winId;
static int   winWidth  = 800;
static int   winHeight = 600;
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
// sets the height and width of blocks
static inline void initDim(struct block* b, size_t n)
{
	for (; n; n--, b++)
	{
		double maxWidth = 0;
		size_t lines = 0;
		for (size_t k = 0; k < b->size; k++)
		{
			unsigned char glText[BUFSIZE];
			snprint_instr((char*) glText, BUFSIZE, b->start+k);

			double textWidth = glutStrokeLength(FONT, glText);
			if (textWidth > maxWidth)
				maxWidth = textWidth;
			lines += countChars((const char*) glText, '\n');
		}
		b->w = blockScale * maxWidth;
		b->h = blockScale * lines * glutStrokeHeight(FONT);
	}
}
static void displayBlock(struct block* b)
{
	if (!b || b->drawn) return;
	b->drawn = true;

	glPushMatrix();
	glTranslatef(b->x, b->y, 0);

	int unselected = b != curBlock;
	glColor4f(1, unselected, unselected, 1);

	const double p = blockPadding;
	glBegin(GL_LINE_LOOP);
		glVertex2f(    -p,     -p);
		glVertex2f(b->w+p,     -p);
		glVertex2f(b->w+p, b->h+p);
		glVertex2f(    -p, b->h+p);
	glEnd();

	glScalef(blockScale, -blockScale, blockScale);
	for (size_t k = 0; k < b->size; k++)
	{
		unsigned char glText[BUFSIZE];
		snprint_instr((char*) glText, BUFSIZE, b->start+k);
		glutStrokeString(FONT, glText);
	}

	glPopMatrix();

	glBegin(GL_LINES);
	if (b->next)
	{
		struct block* c = b->next;
		glVertex2f(b->x + b->w/2, b->y + b->h + p);
		glVertex2f(c->x + c->w/2, c->y - p);
	}
	if (b->branch)
	{
		struct block* c = b->branch;
		glVertex2f(b->x + b->w/2, b->y + b->h + p);
		glVertex2f(c->x + c->w/2, c->y - p);
	}
	glEnd();

	displayBlock(b->next);
	displayBlock(b->branch);
}

static void cb_displayFunc()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();

	glTranslatef(winWidth / 2, winHeight / 2, 0);
	glScalef(viewZoom, viewZoom, viewZoom);
	glTranslatef(-viewX, -viewY, 0);

	for (size_t k = 0; k < funlen; k++)
		fun[k].drawn = false;
	displayBlock(fun);

	glPushMatrix();
	glLoadIdentity();
	glColor4f(1, 1, 1, 1);
	glBegin(GL_QUADS);
		glVertex2f(winWidth/2 - CURSOR_SZ, winHeight/2 - CURSOR_SZ);
		glVertex2f(winWidth/2 + CURSOR_SZ, winHeight/2 - CURSOR_SZ);
		glVertex2f(winWidth/2 + CURSOR_SZ, winHeight/2 + CURSOR_SZ);
		glVertex2f(winWidth/2 - CURSOR_SZ, winHeight/2 + CURSOR_SZ);
	glEnd();
	glPopMatrix();

	glutSwapBuffers();
}

static void cb_reshapeFunc(int width, int height)
{
	winWidth = width;
	winHeight = height;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, winWidth, winHeight, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);

	mouseX = winWidth / 2;
	mouseY = winHeight / 2;
	glutWarpPointer(mouseX, mouseY);
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
		// get (x,y) world coordinates
		GLdouble modelviewM [16]; glGetDoublev(GL_MODELVIEW_MATRIX,  modelviewM);
		GLdouble projectionM[16]; glGetDoublev(GL_PROJECTION_MATRIX, projectionM);
		int viewport[4];          glGetIntegerv(GL_VIEWPORT, viewport);
		double x, y, z;
		gluUnProject(_x, _y, 0, modelviewM, projectionM, viewport, &x, &y, &z);

		// find closest block
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
	glDisable(GL_DEPTH_TEST);

	// enables transparency
//	glEnable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//	glTranslatef(0.375, 0.375, 0); // hack against pixel centered coordinates
}

void zui(int argc, char** argv, struct block* _fun, size_t len)
{
	fun      = _fun;
	funlen   = len;
	curBlock = fun;

	glutInit(&argc, argv);
	glutInitWindowSize(winWidth, winHeight);
	winId = glutCreateWindow("Execution graph");
	glutSetCursor(GLUT_CURSOR_NONE);

	glutDisplayFunc      (&cb_displayFunc);
	glutReshapeFunc      (&cb_reshapeFunc);
	glutKeyboardFunc     (&cb_keyboardFunc);
	glutMouseFunc        (&cb_mouseFunc);
	glutMotionFunc       (&cb_motionFunc);
	glutPassiveMotionFunc(&cb_passiveMotionFunc);

	glInit();
	initDim(fun, funlen);
	spreadNodes(fun, funlen);
	viewX = fun->x;
	viewY = fun->y;

	glutMainLoop();
}

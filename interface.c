#include "interface.h"

#include <GL/freeglut.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define GLUT_KEY_ESC    (27)
#define GLUT_WHEEL_UP   (3)
#define GLUT_WHEEL_DOWN (4)

// texture information

static int   winId;
static int   winWidth  = 1024;
static int   winHeight = 768;
static int   mouseX    = 0;
static int   mouseY    = 0;

static float viewX    = 0;
static float viewY    = 0;
static float viewZoom = 1;

static struct block* fun;
static size_t        funlen;

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
	if (b->drawn) return;
	b->drawn = true;

	glPushMatrix();
	glTranslatef(b->x, b->y, 0);

	float scale = 0.1;
	float maxWidth = 0;
	size_t lines = 0;

	glPushMatrix();
		glScalef(scale, -scale, scale);

		for (size_t k = 0; k < b->size; k++)
		{
			unsigned char glText[BUFSIZE];
			instr_print((char*) glText, BUFSIZE, b->start+k);

			float textWidth = glutStrokeLength(FONT, glText);
			if (textWidth > maxWidth)
				maxWidth = textWidth;
			lines += countChars((const char*) glText, '\n');
	
			glutStrokeString(FONT, glText);
		}
	glPopMatrix();

	float padding = 20;

	float w = scale * maxWidth;
	float h = scale * lines * glutStrokeHeight(FONT);
	float p = padding;
	glBegin(GL_LINE_LOOP);
		glVertex2f( -p,  -p);
		glVertex2f(w+p,  -p);
		glVertex2f(w+p, h+p);
		glVertex2f( -p, h+p);
	glEnd();

	glPopMatrix();
	glColor4f(1, 1, 1, 1);

	if (b->next)
	{
		glBegin(GL_LINES);
			glVertex2f(b->x+w/2,   b->y+h+p);
			glVertex2f(b->next->x, b->next->y);
		glEnd();
		displayBlock(b->next);
	}
	if (b->branch)
	{
		glBegin(GL_LINES);
			glVertex2f(b->x+w/2,   b->y+h+p);
			glVertex2f(b->branch->x, b->branch->y);
		glEnd();
		displayBlock(b->branch);
	}
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
	glColor4f(1, 0, 0, 1);
	displayBlock(fun);

	glPopMatrix();
	glutSwapBuffers();
//	glutPostRedisplay();
}

static void cb_mouseFunc(int button, int state, int x, int y)
{
	(void) state;
	(void) x;
	(void) y;

	if (button == GLUT_WHEEL_UP)
		viewZoom *= 1.1;
	else if (button == GLUT_WHEEL_DOWN)
		viewZoom /= 1.1;

	glutPostRedisplay();
}

static void cb_motionFunc(int x, int y)
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

static void cb_passiveMotionFunc(int x, int y)
{
	cb_motionFunc(x, y);
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

/*
static void attract(float* x, float* y, const struct block* a, const struct block* b)
{
	float dx = b->x - a->x;
	float dy = b->y - a->y;

	*x += dx / 100;
	*y += dy / 100;
}

static void pushAway(float* x, float* y, const struct block* a, const struct block* b)
{
	float dx = b->x - a->x;
	float dy = b->y - a->y;

	float d = dx*dx + dy*dy;

	if (d == 0)
	{
		*x += 1000;
		*y += 1000;
	}
	else
	{
		*x -= dx / d;
		*y -= dy / d;
	}
}
*/

void zui(int argc, char** argv, struct block* _fun, size_t len)
{
	fun = _fun; // TODO
	funlen = len;

	mouseX = winWidth/2;
	mouseY = winHeight/2;

	// node positionning
	fun[0].x = 0;
	fun[0].y = 0;
	for (size_t k = 1; k < funlen; k++)
	{
		fun[k].x = rand() % 10000;
		fun[k].y = rand() % 10000;
	}
/*
	for (size_t bla = 0; bla < 1000; bla++)
	{
		for (size_t k = 0; k < funlen; k++)
		{
			struct block* b = fun + k;
			float dx = 0;
			float dy = 0;
			for (size_t k = 0; k < funlen; k++)
				if (fun[k].next == b || fun[k].branch == b)
					attract(&dx, &dy, b, fun+k);
				else
					pushAway(&dx, &dy, b, fun+k);

			if (b->next)   attract(&dx, &dy, b, b->next);
			if (b->branch) attract(&dx, &dy, b, b->branch);
			b->x += dx;
			b->y += dy;
		}
	}
*/

	glutInit(&argc, argv);
	glutInitWindowSize(winWidth, winHeight);
	winId = glutCreateWindow("Execution graph");
//	glutSetCursor(GLUT_CURSOR_NONE);

	glutDisplayFunc      (&cb_displayFunc);
	glutMouseFunc        (&cb_mouseFunc);
	glutMotionFunc       (&cb_motionFunc);
	glutPassiveMotionFunc(&cb_passiveMotionFunc);

	glInit();
	glutMainLoop();
}

#include "interface.h"

#include <GL/freeglut.h>
#include <stdio.h>
#include <string.h>

#include "blist.h"
#include "print.h"

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

static flist_t* funList;
static size_t   curFunct;
static block_t* curBlock;
static blist_t  blocks;

static size_t countChars(const char* str, char c)
{
	size_t ret = 0;
	for (; *str; str++)
		if (*str == c)
			ret++;
	return ret;
}

#define BUFSIZE 1024
#define FONT    GLUT_STROKE_MONO_ROMAN
static inline void blist_setdim(blist_t* l)
{
	for (size_t i = 0; i < l->n; i++)
	{
		block_t* b = l->b+i;

		double maxWidth = 0;
		size_t lines = 0;
		if (i == 0)
		{
			char*  title = funList->f[curFunct].name;
			double width = glutStrokeLength(FONT, (unsigned char*) title);
			maxWidth = width;

			lines += 2;
		}
		for (expr_t* e = b->e; e; e = e->next)
		{
			char glText[BUFSIZE];
			print_stat(glText, BUFSIZE, e);

			double curWidth = glutStrokeLength(FONT, (unsigned char*) glText);
			if (curWidth > maxWidth)
				maxWidth = curWidth; 
			lines += countChars(glText, '\n');

			if (e->endBlck)
				break;
		}
		if (lines) lines--; // remove last blank line
		b->w = blockScale * maxWidth + 2 * blockPadding;
		b->h = blockScale * lines * glutStrokeHeight(FONT) + 2 * blockPadding;
	}
}

static void setCurFunct(size_t i)
{
	curFunct = i;
	blist_gen   (&blocks, funList->f[curFunct].expr);
	blist_setdim(&blocks);
	blist_spread(&blocks);

	curBlock = blocks.b;
	viewX = curBlock->x;
	viewY = curBlock->y;

	glutPostRedisplay();
}

static void blist_display(blist_t* l)
{
	for (size_t i = 0; i < l->n; i++)
	{
		block_t* b = l->b+i;
		glPushMatrix();
		glTranslatef(b->x, b->y, 0);

		int unselected = b != curBlock;
		glColor4f(1, unselected, unselected, 1);

		glBegin(GL_LINE_LOOP);
			glVertex2f(   0,    0);
			glVertex2f(b->w,    0);
			glVertex2f(b->w, b->h);
			glVertex2f(   0, b->h);
		glEnd();

		// prints the function name
		if (i == 0)
		{
			char*  title = funList->f[curFunct].name;
			double width = glutStrokeLength(FONT, (unsigned char*) title);
			width *= blockScale;

			glPushMatrix();
			glTranslatef(b->w/2 - width/2, blockPadding, 0);
			glScalef(blockScale, -blockScale, blockScale);

			glutStrokeString(FONT, (unsigned char*) title);

			glPopMatrix();
			glTranslatef(0, 2*glutStrokeHeight(FONT)*blockScale, 0);
		}

		glTranslatef(blockPadding, blockPadding, 0);
		glScalef(blockScale, -blockScale, blockScale);
		for (expr_t* e = b->e; e; e = e->next)
		{
			char glText[BUFSIZE];
			print_stat(glText, BUFSIZE, e);

			glutStrokeString(FONT, (unsigned char*) glText);

			if (e->endBlck)
				break;
		}

		glPopMatrix();

		glBegin(GL_LINES);
		if (b->next)
		{
			block_t* c = b->next;
			glVertex2f(b->x + b->w/2, b->y + b->h);
			glVertex2f(c->x + c->w/2, c->y);
		}
		if (b->branch)
		{
			block_t* c = b->branch;
			glColor4f(unselected, 1, 0, 1);
			glVertex2f(b->x + b->w/2, b->y + b->h);
			glVertex2f(c->x + c->w/2, c->y);
		}
		glEnd();
	}
}

static void cb_displayFunc()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();

	glTranslatef(winWidth / 2, winHeight / 2, 0);
	glScalef(viewZoom, viewZoom, viewZoom);
	glTranslatef(-viewX, -viewY, 0);

	blist_display(&blocks);

	glPushMatrix();
	glLoadIdentity();
	glTranslatef(20, 20, 0);
	glScalef(0.08, -0.08, 0.08);
	for (size_t i = 0; i < funList->n; i++)
	{
		if (i == curFunct)
			glColor4f(1, 0, 0, 1);
		else
			glColor4f(1, 1, 1, 1);

		function_t* f = funList->f + i;
		if (f->expr == NULL)
			continue; // TODO
		char glText[BUFSIZE];
		snprintf(glText, BUFSIZE, "%s\n", f->name);
		glutStrokeString(FONT, (unsigned char*) glText);
	}
	glPopMatrix();

	glPushMatrix();
	glLoadIdentity();
	glColor4f(1, 1, 1, 1);
	glBegin(GL_QUADS);
		glVertex2f(mouseX - CURSOR_SZ, mouseY - CURSOR_SZ);
		glVertex2f(mouseX + CURSOR_SZ, mouseY - CURSOR_SZ);
		glVertex2f(mouseX + CURSOR_SZ, mouseY + CURSOR_SZ);
		glVertex2f(mouseX - CURSOR_SZ, mouseY + CURSOR_SZ);
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
		blist_spread(&blocks);
		glutPostRedisplay();
	}
}

static void cb_specialFunc(int key, int x, int y)
{
	(void) x;
	(void) y;

	if (key == GLUT_KEY_PAGE_UP)
	{
		blist_del(&blocks);
		setCurFunct(curFunct ? curFunct - 1 : funList->n - 1);
	}
	else if (key == GLUT_KEY_PAGE_DOWN)
	{
		blist_del(&blocks);
		setCurFunct(curFunct < funList->n-1 ? curFunct + 1 : 0);
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
		_y = viewport[3] - _y;
		gluUnProject(_x, _y, 0, modelviewM, projectionM, viewport, &x, &y, &z);

		// amongst the clicked blocks, find the closest one
		double d_min = -1;
		block_t* b_min = NULL;
		for (size_t i = 0; i < blocks.n; i++)
		{
			block_t* b = blocks.b + i;;
			if (b->x <= x && x <= b->x+b->w &&
			    b->y <= y && y <= b->y+b->h)
			{
				double dx = x - b->x;
				double dy = y - b->y;
				double d = dx*dx + dy*dy;
				if (d_min == -1 || d < d_min)
				{
					d_min = d;
					b_min = b;
				}
			}
		}
		curBlock = b_min;
	}

	glutPostRedisplay();
}

static void cb_passiveMotionFunc(int x, int y)
{
	mouseX = x;
	mouseY = y;

	glutPostRedisplay();
}

static void cb_motionFunc(int x, int y)
{
	if (curBlock)
	{
		curBlock->x += (x - mouseX) / viewZoom;
		curBlock->y += (y - mouseY) / viewZoom;
	}
	else
	{
		viewX += (mouseX - x) / viewZoom;
		viewY += (mouseY - y) / viewZoom;
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

void zui(int argc, char** argv, flist_t* l)
{
	glutInit(&argc, argv);
	glutInitWindowSize(winWidth, winHeight);
	winId = glutCreateWindow("Execution graph");
	glutSetCursor(GLUT_CURSOR_NONE);

	glutDisplayFunc      (&cb_displayFunc);
	glutReshapeFunc      (&cb_reshapeFunc);
	glutKeyboardFunc     (&cb_keyboardFunc);
	glutSpecialFunc      (&cb_specialFunc);
	glutMouseFunc        (&cb_mouseFunc);
	glutMotionFunc       (&cb_motionFunc);
	glutPassiveMotionFunc(&cb_passiveMotionFunc);

	glInit();

	funList  = l;
	curFunct = 0;
	for (; funList->f[curFunct].expr == NULL; curFunct++); // TODO
	blist_gen   (&blocks, funList->f[curFunct].expr);
	blist_setdim(&blocks);
	blist_spread(&blocks);

	curBlock = blocks.b;
	viewX = curBlock->x;
	viewY = curBlock->y;

	glutMainLoop();
}

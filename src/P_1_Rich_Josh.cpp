#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#define _USE_MATH_DEFINES
#include <math.h>

#include <GL/glut.h>

/*!
 * structure for holding histogram data
 * based on a file
 */
struct DataFile {
	//! associated file name
	std::string name;
	//! data obtained from file
	std::vector<float> data;
	//! largest value in data 
	float max = -0xff, 
	//! smallest value in data
		min = 0xff;
	//! number of data points in file
	int amount;
};

//! data from files and the currently displayed data
DataFile *current, d_norm, d_expo, d_dat1, d_dat2;

float mu = 0.0, sigma = 1, beta = 0.1;
//! list of color schemes
float colorSchemes[4][4][3] = {
	{{ 0,1,0 }, { 1,0,0 }, { 1,1,1 },{ 0,0,0 }},
	{{ .1,.1,1. }, { 05,.5,.05 }, { 0,0,0 },{ 1,1,1 }},
	{{ 181. / 255.,137. / 255.,0. / 255. }, { 220. / 255.,50. / 255.,47. / 255. }, { 88. / 255.,110. / 255.,117. / 255. },{ 253. / 255.,246. / 255.,227. / 255. }},
	{{ 5. / 255.,255. / 255.,161. / 255. }, { 1. / 255.,205. / 255.,254. / 255. }, { 255. / 255.,113. / 255.,206. / 255. },{ .1,.1,.1 }},
};
//! selected color scheme index
int colorSchemeIdx = 0;
//! selected step value
float step = 0.01;
//! the highest bar value
float maxBar = 0; 
//! selected interval
int interval = 50; 
//! the histogram data
std::vector<float> bars;
//! for switching between showing normal and exponential distrobutions
bool drawNorm = true; 
//! these are for allowing for continuos movement when a key is held
bool upisDown = false, downisDown = false, rightisDown = false, leftisDown = false;

/*!
 * depending on the currently selected data, 
 * creates a histogram based on the selected 
 * interval
 * 
 */
void calcHistogram() {
	bars = std::vector<float>(interval+1);
	// find the normalized distance between min and max
	float step = (current->max - current->min) / interval;
	for (int i = 0;i < current->data.size();i++) { // for each data point
		int idx = 0;
		int Tidx = (int)roundf(abs(current->data[i] / step))-1;
		printf("%d -- ",Tidx);
		for (float c = current->min;c < current->max;c += step) {// determine which interval the data point lies
			if ((c) < current->data[i] && current->data[i] < (c + step)) {
				bars[idx] = bars[idx] + 1; // count the found data point
				printf("%d -- %d \n", idx,(idx-Tidx));
				break;
			}
			idx++;
		}
	}
	// find the maximum bar value
	maxBar = *std::max_element(bars.begin(), bars.end());
}
/*!
 * draw a string at a location in the context
 * 
 * \param s string to print to screen
 * \param x worldspace relative x axis location
 * \param y worldspace relative y axis location
 */
void drawString(std::string s, float x, float y) {
	void *font = GLUT_BITMAP_8_BY_13;
	glRasterPos2f(x, y);
	for (unsigned int i = 0; i < s.size(); i++)
		glutBitmapCharacter(font, s[i]);
}
/*!
 * draws the xy-axis and associated labels
 * 
 */
void drawAxis() {
	glColor3fv(colorSchemes[colorSchemeIdx][2]);
	// xy-axis lines
	glBegin(GL_LINE_STRIP); {
		glVertex2f(.05, .95);
		glVertex2f(.05, .05);
		glVertex2f(.95, .05);
	}glEnd();
	// max value notch
	glBegin(GL_LINES); {
		glVertex2f(.05, .85);
		glVertex2f(.06, .85);
	}glEnd();
	// finding the index of maxbar and multiplying it by the normalized distance to find its interval value
	float max = ((current->max - current->min) / interval)*(1+std::find(bars.begin(), bars.end(), maxBar) - bars.begin());
	drawString(std::to_string(max/8.), .06, .85);
	drawString("Probability Density", .06, .9);
	drawString("Data", .95, .075);

}
/*!
 * draw the histogram with associated data
 * 
 */
void drawHistogram() {
	glColor3fv(colorSchemes[colorSchemeIdx][0]);
	float x = .65, y = .875;
	drawString(std::string("File: ").append(current->name), x, y);
	drawString(std::string("Min: ").append(std::to_string(current->min)), x, y - .025);
	drawString(std::string("Max: ").append(std::to_string(current->max)), x, y - .05);
	drawString(std::string("Num of Intervals: ").append(std::to_string(interval)), x, y - .075);

	//draw each bar starting at start
	// the width of each bar depends upon the worldspace and the interval
	float start[2] = { .05,.05 }, width = 1./interval;
	for (int i = 0;i < (bars.size());i++) {
		glBegin(GL_LINE_LOOP); {
			glVertex2f(start[0] + width * (i + 1), start[1]);// scale the bar to be relative to worldspace
			glVertex2f(start[0] + width * (i + 1), start[1] + (bars[i] / maxBar)*.8);
			glVertex2f(start[0] + width * (i + 1) + width, start[1] + (bars[i] / maxBar)*.8);
			glVertex2f(start[0] + width * (i + 1) + width, start[1]);
		}glEnd();
	}
}

/*!
 * normal distrobution function
 * 
 * \param x expected value
 * \param sigma std deviation
 * \param mu mean
 * \return the z value of the normal distrobution
 */
float normal(float x, float sigma,float mu) {
	float expo = -(pow(x-mu,2.)/(2.*pow(sigma,2.)));
	return exp(-(pow(x - mu, 2.) / (2.*pow(sigma, 2.)))) / (sigma*sqrt(2.*M_PI));
}
/*!
 * exponential distrobution function
 * 
 * \param x expected value
 * \param beta 
 * \return value of distrobution
 */
float expn(float x, float beta) {
	return exp(-x / beta) / beta;
}
/*!
 * draw the normal distrobution and its labels
 * 
 */
void drawNormal() {
	glColor3fv(colorSchemes[colorSchemeIdx][1]);
	float x = .65, y = .75;
	drawString(std::string("Function Type: Normal"), x, y);
	drawString(std::string("Mu: ").append(std::to_string(mu)), x, y-.025);
	drawString(std::string("Sigma: ").append(std::to_string(sigma)), x, y - .05);
	glPushMatrix();
	glTranslatef(.05, .05, 0);
	glScalef(1. / 6., 1. / .5, 1);
	glBegin(GL_LINE_STRIP); {
		float y = 0;
		for (float x = 0;x <= 6.;x += 0.01) {
			y = normal(x, sigma,mu);
			glVertex2f(x, y);
		}
	}glEnd();
	glPopMatrix();
}
/*!
 * draw the exponential distrobution and its labels
 * 
 */
void drawExpo() {
	glColor3fv(colorSchemes[colorSchemeIdx][1]);
	float x = .65, y = .75;
	drawString(std::string("Function Type: Exponential"), x, y);
	drawString(std::string("Beta: ").append(std::to_string(beta)), x, y-.025);
	glPushMatrix();
	glTranslatef(.05, .05, 0);
	glScalef(1. / 5., 1. / 1.6, 1);
	glBegin(GL_LINE_STRIP); {
		float y = 0;
		for (float x = .1;x <= 5;x += 0.01) {
			y = expn(x, beta);
			glVertex2f(x, y);
		}
	}glEnd();
	glPopMatrix();
}
/*!
 * glut display callback
 * 
 */
void display(void) {
	glClearColor(colorSchemes[colorSchemeIdx][3][0],
				 colorSchemes[colorSchemeIdx][3][1],
				 colorSchemes[colorSchemeIdx][3][2], 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawAxis();
	drawHistogram();
	if (drawNorm)
		drawNormal();
	else drawExpo();


	glFlush();
}
/*!
 * openGL initial settings
 * 
 */
void init(void) {
	glClearColor(0.0, 0.0, 0.0, 0.0);


	glViewport(0, 0, 700, 600);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1, 0.0, 1, -2.0, 2.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_ALPHA_TEST);//enable transparency test
	glAlphaFunc(GL_NOTEQUAL, 0);
	glEnable(GL_BLEND);//and enable blending of transparencies
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);

}
/*!
 * glut window reshape callback
 * prevents resize
 * 
 * \param w
 * \param h
 */
void reshape(int w, int h) {
	glutReshapeWindow(700, 600);
}
/*!
 * glut key down callback
 * 
 * \param key
 * \param x
 * \param y
 */
void keyboardDown(int key, int x, int y) {
	switch (key) {
		case GLUT_KEY_UP:
			upisDown=true;
			break;
		case GLUT_KEY_DOWN:
			downisDown = true;
			break;
		case GLUT_KEY_RIGHT:
			rightisDown = true;
			break;
		case GLUT_KEY_LEFT:
			leftisDown = true;
			break;
	}
}
/*!
 * glut key up callback
 * 
 * \param key
 * \param x
 * \param y
 */
void keyboardUp(int key, int x, int y) {
	switch (key) {
		case GLUT_KEY_UP:
			upisDown = false;
			break;
		case GLUT_KEY_DOWN:
			downisDown = false;
			break;
		case GLUT_KEY_RIGHT:
			rightisDown = false;
			break;
		case GLUT_KEY_LEFT:
			leftisDown = false;
			break;
	}
}
/*!
 * listens for when a key is held and 
 * performs action based on the key
 * 
 */
void keyListener() {
	if (upisDown) {
		if (drawNorm)
			sigma += step;
		else beta += step;
	} else if (downisDown) {
		if (drawNorm)
			sigma -= step;
		else beta -= step;
	}
	if (rightisDown)
		mu += step;
	else if (leftisDown)
		mu -= step;
	// only update screen when key is pressed
	if (upisDown || downisDown || rightisDown || leftisDown) {
		calcHistogram();
		glutPostRedisplay();
	}
}
/*!
 * reads and parses data file placing contents in DataFile structure
 * 
 * \param file data file to process
 * \param df structure to fill
 */
void loadFile(std::string file,DataFile &df) {
	std::ifstream in;
	in.open(file);
	df.name = file;
	in >> df.amount;
	while (!in.eof()) {
		float d;
		in >> d;
		if (d > df.max)
			df.max = d;
		else if (d < df.min)
			df.min = d;
		df.data.push_back(d);
	}
	in.close();
}

/*!
 * menu interactions mapped to values
 * 
 */
enum menuEvent {
	//! Exit option event. exits program
	EXIT = 0,
	//! change interval amount to 30
	HIST30 = 1,
	//! change interval amount to 40
	HIST40 = 2,
	//! change interval amount to 50
	HIST50 = 3,
	//! draw normal distrobution
	NORM = 4,
	//! draw exponential distrobution
	EXPO = 5,
	//! show normal.dat file data
	fNORM = 6,
	//! show expo.dat file data
	fEXPO = 7,
	//! show first extra file data
	fDAT1 = 8,
	//! show second extra file data
	fDAT2 = 9,
	//! set step for distrobution variables to 0.01
	STEP1 = 10,
	//! set step for distrobution variables to 0.02
	STEP2 = 11,
	//! set step for distrobution variables to 0.03
	STEP3 = 12,
	//! defualt color scheme 
	csDEF = 13,
	//! light color scheme 
	csLIG = 14,
	//! solorized color scheme 
	csSOL = 15,
	//! vaporwave color scheme 
	csVAP = 16,
};
/*!
 * listens for menu interaction and performs the correct action
 * 
 * \param val interaction value
 */
void menuListener(int val) {
	switch (val) {
		case menuEvent::EXIT:
			exit(0);
			break;
		case menuEvent::fNORM:
			current = &d_norm;
			break;
		case menuEvent::fEXPO:
			current = &d_expo;
			break;
		case menuEvent::fDAT1:
			current = &d_dat1;
			break;
		case menuEvent::fDAT2:
			current = &d_dat2;
			break;
		case menuEvent::NORM:
			drawNorm = true;
			break;
		case menuEvent::EXPO:
			drawNorm = false;
			break;
		case menuEvent::HIST30:
			interval = 30;
			break;
		case menuEvent::HIST40:
			interval = 40;
			break;
		case menuEvent::HIST50:
			interval = 50;
			break;
		case menuEvent::STEP1:
			step = 0.01;
			break;
		case menuEvent::STEP2:
			step = 0.02;
			break;
		case menuEvent::STEP3:
			step = 0.03;
			break;
		case menuEvent::csDEF:
			colorSchemeIdx = 0;
			break;
		case menuEvent::csLIG:
			colorSchemeIdx = 1;
			break;
		case menuEvent::csSOL:
			colorSchemeIdx = 2;
			break;
		case menuEvent::csVAP:
			colorSchemeIdx = 3;
			break;
	}

	calcHistogram();
	glutPostRedisplay();
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);

	//load data files
	loadFile("normal.dat",d_norm);
	loadFile("expo.dat",d_expo);
	loadFile("1.dat",d_dat1);
	loadFile("19.dat",d_dat2);
	current = &d_norm;

	calcHistogram();// draw initial histogram

	//initial glut window settings
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(700, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Input Analysis");

	init();

	//set glut window callback 
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	glutSpecialFunc(keyboardDown);
	glutSpecialUpFunc(keyboardUp);
	glutIdleFunc(keyListener);// always listening for key press
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);

	// create menus
	int mfile = glutCreateMenu(menuListener);
	glutAddMenuEntry("normal.dat", menuEvent::fNORM);
	glutAddMenuEntry("expo.dat", menuEvent::fEXPO);
	glutAddMenuEntry("1.dat", menuEvent::fDAT1);
	glutAddMenuEntry("19.dat", menuEvent::fDAT2);

	int mdist = glutCreateMenu(menuListener);
	glutAddMenuEntry("Normal", menuEvent::NORM);
	glutAddMenuEntry("Exponential", menuEvent::EXPO);

	int mhist = glutCreateMenu(menuListener);
	glutAddMenuEntry("30", menuEvent::HIST30);
	glutAddMenuEntry("40", menuEvent::HIST40);
	glutAddMenuEntry("50", menuEvent::HIST50);

	int mpara = glutCreateMenu(menuListener);
	glutAddMenuEntry("0.01", menuEvent::STEP1);
	glutAddMenuEntry("0.02", menuEvent::STEP2);
	glutAddMenuEntry("0.03", menuEvent::STEP3);

	int mcolor = glutCreateMenu(menuListener);
	glutAddMenuEntry("defualt", menuEvent::csDEF);
	glutAddMenuEntry("light", menuEvent::csLIG);
	glutAddMenuEntry("solarized", menuEvent::csSOL);
	glutAddMenuEntry("vaporwave", menuEvent::csVAP);
	 
	int menu = glutCreateMenu(menuListener);
	glutAddSubMenu("File", mfile);
	glutAddSubMenu("Distrubtion", mdist);
	glutAddSubMenu("Histogram", mhist);
	glutAddSubMenu("Parameter Step", mpara);
	glutAddSubMenu("Color Scheme", mcolor);
	glutAddMenuEntry("Exit", menuEvent::EXIT);

	glutAttachMenu(GLUT_RIGHT_BUTTON);

	glutMainLoop();
	return 0;
}

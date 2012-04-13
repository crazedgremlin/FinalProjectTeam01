#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <GLUT/glut.h>
#include <sys/types.h>
#include <sys/socket.h>

/*

ARGUMENTS

    [--nVal (-n)]           Number of squares on each side of the board.
                            Default is 8.

*/

const int WIDTH = 640;
const int HEIGHT = 640;

int numSquaresOnSide = -1;

void init();
void drawScreen();
void drawBoard();
void drawPieces();


int main(int argc, char* argv[]) {
    int argNum;
    char* argLabel, *argVal;


    // read in all arguments
    argNum=1;
    while (argNum < argc) {
        argLabel = argv[argNum];
        argVal = argv[argNum+1];

        // strcmp(a,b) returns 0 if the strings are equal, so my shorthand
        // for checking if two strings are equal is !strcmp(a,b)
        if (!strcmp(argLabel, "--nVal") || !strcmp(argLabel, "-n")) {
            numSquaresOnSide = atoi(argVal);
        }

        argNum++;
    }

    // check to see that required arguments were specified
    if (numSquaresOnSide == -1 || numSquaresOnSide <= 1) {
        printf("Defaulting to n=8\n");
        numSquaresOnSide = 8;
    }

    // GLUT setup stuff
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Test!");

    // register display callback
    glutDisplayFunc(drawScreen);
        
    init();
    glutMainLoop();
}


/*
    Set up screen stuff
*/
void init() {
    // disable z axis
    glDisable(GL_DEPTH_TEST);

    // choose background color
    glClearColor(1.0, 1.0, 1.0, 0.0);

    // set drawing color
    glColor3f(0.0f, 0.0f, 0.0f);

    // set point size
    glPointSize(2.0);

    // load matrix mode
    glMatrixMode(GL_PROJECTION);

    // load identity matrix
    glLoadIdentity();

    //defines a 2D orthographic projection matrix
    gluOrtho2D(0.0, WIDTH, 0.0, HEIGHT);
}



/*
    Display the state of the game visually
*/
void drawScreen() {
    // clear the screen
    glClear(GL_COLOR_BUFFER_BIT);

    // draw the current state of the game
    drawBoard();
    drawPieces();

    // flushes all unfinished drawing commands
    glFlush();
}


/*
    Draw the checkerboard pattern
*/
void drawBoard() {
    int x, y;
    float xPos, yPos;
    int x1, y1, x2, y2;
    float sqrWidth = 1.*WIDTH / numSquaresOnSide;
    float sqrHeight = 1.*HEIGHT / numSquaresOnSide;
    bool evenCol, sqrOn;


    // represents which color this column should start with
    evenCol = false;


    // avoid the oncoming infinite loop if square width or height is zero
    if (sqrWidth == 0)
        sqrWidth = 1;
    if (sqrHeight == 0)
        sqrHeight = 1;


    // draw the checkerboard
    for (x = 0; x < numSquaresOnSide; x++) {
        
        xPos = x * sqrWidth;

        evenCol = !evenCol;
        sqrOn = evenCol;

        for (y = 0; y<numSquaresOnSide; y++) {

            yPos = y * sqrHeight;

            sqrOn = !sqrOn;

            if (sqrOn) {

                x1 = (int)(xPos + 0.5);
                x2 = (int)(xPos + sqrWidth + 0.5);
                y1 = (int)(yPos + 0.5);
                y2 = (int)(yPos + sqrHeight + 0.5);

                glBegin(GL_QUADS);
                glVertex2f(x1, y1);
                glVertex2f(x2, y1);
                glVertex2f(x2, y2);
                glVertex2f(x1, y2);
                glEnd();
            }
        }
    }
}

/*
    Draw the positions of the players' pieces
*/
void drawPieces() {


}

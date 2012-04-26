#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>

#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif


const int WIDTH = 640;
const int HEIGHT = 640;

const char* HELP_STR =
"ARGUMENTS\n\n"
"    [--nVal   (-n)]           Number of squares on each side of the board.\n"
"                            Default is 8.\n"
"    [--server (-s)]         Start in server mode.\n"
"    [--client (-c)]         Start in client mode\n"
"    [--port   (-p)]         Run server on specified port number.\n"
;


enum modeType {
    SERVER,
    CLIENT
};

enum modeType mode;
int numSquaresOnSide = -1;
int port = 1024;
char **board;

bool procArgs(int argc, char* argv[]);
void init();
char** initMatrix(int n, int m);
void drawScreen();
void drawBoard();
void drawPieces();
void drawCircle(int x, int y, int radius);

int main(int argc, char* argv[]) {

    bool goodArgs = procArgs(argc, argv);
    if (!goodArgs)
        return 1;

    // GLUT setup stuff
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Test!");

    // register display callback
    glutDisplayFunc(drawScreen);
        
    init();
    glutMainLoop();

    return 0;
}


/*
    Process command-line arguments. Return false if the program should
    quit right away. Return true if we should continue.
*/
bool procArgs(int argc, char* argv[]) {
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
        } else if (!strcmp(argLabel, "--help")) {
            printf("%s", HELP_STR);
            return false;
        } else if (!strcmp(argLabel, "--server") || !strcmp(argLabel, "-s")) {
            mode = SERVER;
        } else if (!strcmp(argLabel, "--client") || !strcmp(argLabel, "-c")) {
            mode = CLIENT;
        } else if (!strcmp(argLabel, "--port") || !strcmp(argLabel, "-p")) {
            port = atoi(argVal);
        }

        argNum++;
    }

    // check to see that required arguments were specified
    if (numSquaresOnSide == -1 || numSquaresOnSide <= 1) {
        printf("Defaulting to n=8\n");
        numSquaresOnSide = 8;
    }
    if (mode == SERVER) {
        printf("Starting in SERVER mode.\n");
        printf("Running on port %d\n", port);
    } else if (mode == CLIENT) {
        printf("Starting in CLIENT mode.\n");
        printf("Connecting to port %d\n", port);
    }


    return true;
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


    // antialiasing
    glEnable(GL_POLYGON_SMOOTH_HINT);
    glEnable(GL_LINE_SMOOTH_HINT);

    // allocate memory for n pointers to char arrays
    board = initMatrix(numSquaresOnSide, numSquaresOnSide);

    // fill in the board array
    //      X       represents player 1
    //      Y       represents player 2
    //      K       represents player 1 kinged
    //      L       represents player 2 kinged

    int x, y;
    bool evenCol, sqrOn;


    for (x = 0; x < numSquaresOnSide; x++) {        
        evenCol = !evenCol;
        sqrOn = evenCol;

        for (y = 0; y<numSquaresOnSide; y++) {
            sqrOn = !sqrOn;

            if (sqrOn) {
                if (numSquaresOnSide % 2 == 1) {
                    if (y < numSquaresOnSide/2) {
                        board[x][y] = 'X';
                    } else if (y > numSquaresOnSide/2){
                        board[x][y] = 'Y';
                    }
                } else {
                    if (y < numSquaresOnSide/2 - 1) {
                        board[x][y] = 'X';
                    } else if (y > numSquaresOnSide/2){
                        board[x][y] = 'Y';
                    }
                }
            }
        }
    }




}

/* Creates a 2D array of char and returns a pointer. */
char** initMatrix(int n, int m) {
    int i;

    // reserve space for n pointers to char arrays
    char** a = (char**) malloc(sizeof(char*)*n);

    // guarantee contiguous allocation
    a[0] = malloc(n*m * sizeof(char));

    // Make double subscripts work with some hardcoded
    // pointer arithmetic. Note that we don't have to
    // redo these pointers when receiving a new matrix
    // because the matrices are allocated contiguously!
    for (i=0; i<n; i++)
        a[i] = (char*)(a[0] + i*m);
 
    return a;
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
    int x1, y1, x2, y2;
    int cx, cy, r;
    float xPos, yPos;
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

                cx = (x1 + x2) / 2;
                cy = (y1 + y2) / 2;
                r = (int)(sqrWidth * 0.8);

                switch (board[x][y]) {
                    // player 1
                    case 'X':
                    case 'K':
                        glColor3f(1.0f, 0.0f, 0.0f);
                        break;

                    // player 2
                    case 'Y':
                    case 'L':
                        glColor3f(0.0f, 1.0f, 0.0f);
                        break;
                }
                
                drawCircle((x1+x2)/2, (y1+y2)/2, (x2-x1)/2);


                switch (board[x][y]) {
                    case 'K':
                        glColor3f(0.8f, 0.2f, 0.3f);
                        break;
                    case 'L':                    
                        glColor3f(0.2f, 0.8f, 0.3f);
                        break;
                }

                if (board[x][y] == 'K' || board[x][y] == 'L')
                    drawCircle((x1+x2)/2, (y1+y2)/2, (x2-x1)/3);



                glColor3f(0.0f, 0.0f, 0.0f);

            }
        }
    }
}

/*
    Draw the positions of the players' pieces
*/
void drawPieces() {


}


void drawCircle(int x, int y, int radius) {
    int theta;
    
    glBegin(GL_TRIANGLE_FAN);
    for (theta=0; theta<1000; theta+=1) {
        glVertex2f(x + sin(theta) * radius, y + cos(theta) * radius);
    }
    glEnd();

}



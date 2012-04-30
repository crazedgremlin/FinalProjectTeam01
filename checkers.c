#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif


const int WIDTH = 320;
const int HEIGHT = 320;
const int PI = 3.1415926f;
const char* HELP_STR =
"ARGUMENTS\n\n"
"    [--nVal   (-n)]           Number of squares on each side of the board.\n"
"                            Default is 8.\n"
"    [--server (-s)]         Start in server mode.\n"
"    [--client (-c)]         Start in client mode\n"
"    [--port   (-p)]         Run server on specified port number.\n"
"\n"
"KEYBOARD COMMANDS\n\n"
"    L                       draws labels over the checkers pieces\n"
"    P                       prints the board to STDOUT\n"
;



// different types of players
enum player {
    PLAYER_ONE,
    PLAYER_TWO,
    NO_PLAYER
};

// possible modes of operation
enum modeType {
    SERVER,
    CLIENT
};


typedef struct {
    // move from this player sent to server
    int x1, y1, x2, y2;
} MessageToServer;

typedef struct {
    // move from other player
    int x1, y1, x2, y2;

    // if isMyTurn is False, game is over
    bool isMyTurn;
} MessageFromServer;


// command line options
enum modeType mode = SERVER;
int numSquaresOnSide = -1;
int port = 9020;
char* serverAddr = "localhost";

// display options
bool drawLabels = false;

// used for dragging a piece with mouse
bool dragging = false;
char dragType = ' ';
int dragXFrom, dragYFrom;
int mouseX, mouseY;

// other globals
char **board;
char titleStr[255];
enum player me;

// communication
int serverSocket, playerOneSock, playerTwoSock;

bool procArgs(int argc, char* argv[]);
void init();
char** initMatrix(int n, int m);
void drawScreen();
void drawBoard();
void drawPiece(char pieceType, int x, int y);
void drawKing(float x, float y, int scale);
void drawReesesCup(int x, int y, int radius);
bool isValidMove(enum player p, bool isKing, int x1, int y1, int x2, int y2);
enum player determinePlayer(char piece);
void decideBoardCoords(int mouseX, int mouseY, int *x, int *y);
void motionFunc(int x, int y);
void mouseFunc(int button, int state, int x, int y);
void printBoard();
void keyPressed(unsigned char key, int x, int y);
void drawString(char* str, int x, int y);
int serverAddPlayer(char* playerTitle, int serverSocket, struct sockaddr_in clientAddr);
void initSockets();


bool isGameOver() {
    return false;
}

int whoWon() {
    return -1;
}


void sendMoveToServer( MessageToServer* mess) {
    int n = write(serverSocket,mess,sizeof(MessageToServer));
    if (n < 0)
        printf("ERROR sending move to server \n");
}

MessageFromServer* getMessageFromServer() {
    MessageFromServer* message = malloc(sizeof(MessageFromServer));
    int n;
    n = read(serverSocket,message,sizeof(MessageFromServer));
    if (n < 0)
            printf("ERROR receiving message\n");
    // get message

    return message;
}

int main(int argc, char* argv[]) {

    bool goodArgs = procArgs(argc, argv);
    if (!goodArgs)
        return 1;

    if (mode == CLIENT) {
        initSockets();

        // GLUT setup stuff
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
        glutInitWindowSize(WIDTH, HEIGHT);
        glutCreateWindow(titleStr);

        // set up everything
        init();

        // register display callback
        glutDisplayFunc(drawScreen);
        
        //player 2 listens for player 1 move 
        if(me == PLAYER_TWO)
        {
            getMessageFromServer();
        }
        
        glutMainLoop();

        // Listen for message
        // If message.isMyTurn
        //     let player move
        //     send move to server
        // Else
        //     Display game over message


    } else if (mode == SERVER) {
    
        initSockets();
        
        bool whoseTurn;
        bool gameOver = false;
        int winner;
        while (!gameOver) {
            whoseTurn = !whoseTurn;
            
            // make a message for player whose turn it is
            // send message
            // listen for reply
            // send that answer to the other player


            if (isGameOver()) {
                winner = whoWon();

                // send youWon message to winner
                // send youLose message to loser
            }
        }
    }




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
        } else if (!strcmp(argLabel, "--address") || !strcmp(argLabel, "-a")) {
            serverAddr = argVal;
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
    glClearColor(0.6, 0.1, 0.2, 0.0);

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

    // set mouse motion func
    glutMouseFunc(mouseFunc);
    glutMotionFunc(motionFunc);

    // set keyboard func
    glutKeyboardFunc(keyPressed);

    // allocate memory for n pointers to char arrays
    board = initMatrix(numSquaresOnSide, numSquaresOnSide);


    // fill in the board character array
    //      X       represents player 1
    //      Y       represents player 2
    //      K       represents player 1 kinged
    //      L       represents player 2 kinged
    //      _       means this square is off

    int x, y;
    bool evenCol, sqrOn;


    for (x = 0; x < numSquaresOnSide; x++) {        
        evenCol = !evenCol;
        sqrOn = evenCol;

        for (y = 0; y<numSquaresOnSide; y++) {
            sqrOn = !sqrOn;

            board[x][y] = ' ';

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


                glColor3f(.0f, .0f, .0f);

                glBegin(GL_QUADS);
                    glVertex2f(x1, y1);
                    glVertex2f(x2, y1);
                    glVertex2f(x2, y2);
                    glVertex2f(x1, y2);
                glEnd();

                cx = (x1 + x2) / 2;
                cy = (y1 + y2) / 2;
                drawPiece(board[x][y], cx, cy);


                if (drawLabels) {
                    // draw the board character at this position
                    glColor3f(1.0f, 1.0f, 1.0f);
                    glRasterPos2i(cx-9/2, cy - 15/2);
                    glutBitmapCharacter(GLUT_BITMAP_9_BY_15, board[x][y]);
                    glColor3f(0.0f, 0.0f, 0.0f);
                }

            }

        }
    }

    // optionally draw the piece that is being dragged
    if (dragging) {
        drawPiece(dragType, mouseX, mouseY);
    }
}

/*
    Draw the positions of the players' pieces
*/
void drawPiece(char pieceType, int screenX, int screenY) {

    int diam = WIDTH/numSquaresOnSide;

    switch (pieceType) {
        // player 1
        case 'X':
        case 'K':
            glColor3f(.87f, 0.64f, 0.32f);
            break;

        // player 2
        case 'Y':
        case 'L':
            glColor3f(0.8f, 0.3f, 0.2f);
            break;
    }
    
    drawReesesCup(screenX, screenY, diam/2.1);

    switch (pieceType) {
        case 'K':
            glColor3f(0.77f, 0.54, 0.22f);
            break;
        case 'L':                    
            glColor3f(0.7f, 0.2f, 0.1f);
            break;
    }

    if (pieceType == 'K' || pieceType == 'L')
        drawKing(screenX, screenY, diam/2.1);

    glColor3f(0.0f, 0.0f, 0.0f);
}

/*
    Draws a piece at (x, y) with the specified radius.
*/
void drawReesesCup(int x, int y, int radius) {
    bool in = false;
    float numPts = 2*PI*10;
    float theta;
    float r1, r2;
    r1 = radius;
    r2 = 0.8 * radius;

    //glBegin(GL_POINTS);
    glBegin(GL_POLYGON);
    //glBegin(GL_LINE_LOOP);
    //glBegin(GL_TRIANGLES);
    for (theta=0; theta<=2.*PI; theta+=2.*PI/numPts) {

        // decide which radius to use
        /*in = !in;
        if (in) radius = r2;
        else    radius = r1;
        */
        glVertex2f(x + sin(theta)*radius, y + cos(theta)*radius);
    }

    glVertex2f(x, y+r1);

    glEnd();

}

/*
 Draws a king at (x, y) with the specified radius.
 */
void drawKing(float x, float y, int scale) {

    x = x - 1.0/3.0*scale;
    y = y - 1.0/3.0*scale;

    glBegin(GL_POLYGON);
    glVertex2f(x,y);
    glVertex2f(x - 1.0/3.0*scale, y + 2.0/3.0*scale);
    glVertex2f(x, y + 0.5*scale);
    glVertex2f(x + 1.0/3.0*scale, y+1.0*scale);
    glVertex2f(x + 2.0/3.0*scale, y+0.5*scale);
    glVertex2f(x + 1.0*scale, y + 2.0/3.0*scale);
    glVertex2f(x + 2.0/3.0*scale, y);
    glEnd();
	
}

/*
    Figure out the coordinates of the square on the board that the mouse is
    inside.
*/
void decideBoardCoords(int mouseX, int mouseY, int *x, int *y) {
    *x = 1. * numSquaresOnSide * mouseX / WIDTH;
    *y = numSquaresOnSide - 1. * numSquaresOnSide * mouseY / HEIGHT;
}


/*
    Called when the mouse is moved while a button is down.
*/
void motionFunc(int x, int y) {
    mouseX = x;
    mouseY = HEIGHT - y;
    glutPostRedisplay();
}


enum player determinePlayer(char piece) {
    if (piece == 'X' || piece == 'K')
        return PLAYER_ONE;
    else if (piece == 'Y' || piece == 'L')
        return PLAYER_TWO;
    else
        return NO_PLAYER;
}

/*
    Determines if the move is valid.  If a piece has been jumped, overwrite
    that part of the board with a space.
     
    FIXME -- really really bad code
*/
bool isValidMove(enum player p, bool isKing, int x1, int y1, int x2, int y2) {
    int goal = 0;
    if (p == PLAYER_ONE)
        goal = 1;
    else if (p == PLAYER_TWO)
        goal = -1;

    bool goodSingleJump = (board[x2][y2] == ' ' && abs(x2-x1) == 1 && 
                          ((isKing && abs(y2-y1) == 1) || (!isKing && y2-y1 == goal)));

    // between jump coordinates -- the space that is passed during the jump
    int bx, by;
    char jumped;

    if (goodSingleJump)
        return true;
    else {
        goal *= 2;
        if (abs(x2-x1) == 2 && isKing && abs(y2-y1) == 2 || !isKing && y2-y1 == goal) {
            bx = (x2+x1)/2;
            by = (y2+y1)/2;
            jumped = board[bx][by];
            if ((p == PLAYER_ONE && (jumped == 'Y' || jumped == 'L')) ||
                (p == PLAYER_TWO && (jumped == 'X' || jumped == 'K'))) {

                printf("Piece at (%d, %d) was taken.\n", bx, by);
                board[bx][by] = ' ';
                return true;
            }
        }
    }
    return false;

}

/*
    Called when the user presses and releases mouse buttons.
*/
void mouseFunc(int button, int state, int x, int y) {

    int dragXTo, dragYTo;
    enum player playerNum;
    bool isKing;

    mouseX = x;
    mouseY = HEIGHT - y;

    if (button == GLUT_LEFT_BUTTON) {

        if (state == GLUT_DOWN) {

            if (!dragging) { // pick up a piece

                dragging = true;
                decideBoardCoords(x, y, &dragXFrom, &dragYFrom);
                dragType = board[dragXFrom][dragYFrom];

                // if this square is off
                if (dragType == ' ') {
                    dragging = false;
                    return;
                }

                board[dragXFrom][dragYFrom] = ' ';

                printf("dragging piece @ (%d, %d)\n", dragXFrom, dragYFrom);
            }
        } else {

            if (dragging) { // drop the piece that is being dragged
                
                dragging = false;
                decideBoardCoords(x, y, &dragXTo, &dragYTo);

                playerNum = determinePlayer(dragType);
                isKing = dragType == 'K' || dragType == 'L';
                // determine if this is a valid location to drop the piece
                if (isValidMove(playerNum, isKing, dragXFrom, dragYFrom, dragXTo, dragYTo)) {

                    // promote to king?
                    if (dragType == 'X' && dragYTo == numSquaresOnSide-1)
                        dragType = 'K';
                    else if (dragType == 'Y' && dragYTo == 0) {
                        dragType = 'L';
                    }

                    board[dragXTo][dragYTo] = dragType;



                    // TODO Send move to server
                    // TODO Listen for other player's move
                    MessageToServer* message = malloc(sizeof(MessageToServer));
                    message->x1 = dragXFrom;
                    message->y1 = dragYFrom;
                    message->x2 = dragXTo;
                    message->y2 = dragYTo;
                    sendMoveToServer(message);

                } else {
                    printf("INVALID!\n");
                    board[dragXFrom][dragYFrom] = dragType;
                }
            }
        }

    } else {
        
    }

    glutPostRedisplay();

}

void printBoard() {
    int x, y;
    for (y=0; y<numSquaresOnSide; y++) {
        for (x=0; x<numSquaresOnSide; x++) {
            printf("%c ", board[x][y]);
        }
        printf("\n");
    }

    int i;
    for (i=0; i<numSquaresOnSide*2; i++) 
        printf("-");
    printf("\n");
}
void keyPressed(unsigned char key, int x, int y) {
    switch (key) {
        case 'l':
        case 'L':
            drawLabels = !drawLabels;
            break;
        case 'p':
        case 'P':
            printBoard();
    }
    glutPostRedisplay();
}

void drawString(char* str, int x, int y) {
    int i;
    int len;
    
    // set color
    glColor3f(0.0, 1.0, 0.0);
    
    // set position
    glRasterPos2i(x, y);

    // draw each character
    len = strlen(str);
    for (i=0; i<len; i++) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, str[i]);
    }

}




/*
    Adds a player by listening for a connection from the client.
    
    Returns sockfd for the client
*/
int serverAddPlayer(char* playerTitle, int serverSocket, struct sockaddr_in clientAddr) {
    int clientSockFd;
    int clilen;
    int n;
    

    // listen for incoming connection from client 
    int backlog = 1;
    listen(serverSocket,backlog);
    clilen = sizeof(clientAddr);
    clientSockFd = accept(serverSocket, 
             (struct sockaddr *) &clientAddr, 
             &clilen);
    if (clientSockFd < 0) 
      printf("ERROR on accept\n");


    // send the window title to the client
    n = write(clientSockFd,playerTitle,255);
    if (n < 0) 
        printf("ERROR sending player title string\n");
    else
        printf("Sent title string '%s'\n", playerTitle);


    n = write(clientSockFd,&numSquaresOnSide, sizeof(int));
    if (n < 0)
        printf("ERROR sending board size to client");
    else
        printf("Sent board size to %s\n", playerTitle);

    return clientSockFd;

}

void initSockets() {
    struct hostent *server;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;

    int sockfd, n;
    char buffer[256];

    // set up sockets
    if (mode == CLIENT) {

        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) 
            printf("ERROR opening socket\n");

        server = gethostbyname(serverAddr);
        
        if (server == NULL) {
            fprintf(stderr,"ERROR, no such host\n");
            exit(0);
        }

        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr, 
             (char *)&serv_addr.sin_addr.s_addr,
             server->h_length);
        serv_addr.sin_port = htons(port);
        if (connect(serverSocket, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
            printf("ERROR connecting\n");
            exit(0);
        }


        // read in title string
        n = read(serverSocket,&titleStr[0],255);
        if (n < 0) 
            printf("ERROR receiving title string\n");
        else
            printf("Received title string\n");

        // get board size
        n = read(serverSocket,&numSquaresOnSide, sizeof(int));
        if (n < 0)
            printf("ERROR receiving board size\n");
        else
            printf("Received size of board: %d\n", numSquaresOnSide);

	if(!strcmp(titleStr,"Player One"))
	{
		me = PLAYER_ONE;
	}else 
	{
		me = PLAYER_TWO;
	}
        printf("RECEIVED: '%s'\n", titleStr);


    } else if (mode == SERVER) {
           
        // create server socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) 
        printf("ERROR opening socket\n");

        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(port);
        if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              printf("ERROR on binding\n");


         // --------- listen for connection from client 1
        playerOneSock = serverAddPlayer("Player One", sockfd, cli_addr);
        playerTwoSock = serverAddPlayer("Player Two", sockfd, cli_addr);
        me = NO_PLAYER;

    }
}



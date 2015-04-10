#ifndef PHYSICS_H
#define FPGA        true
#define PHYSICS_H   true

#define POS_LOC_IGNORE true

#if !FPGA
#include <stdio.h>
#include <fstream>
using namespace std;
#endif

#include <map>
#include "angles.hpp"
//#include "position_locator.hpp"

#define NUM_BALLS   3
#define NORMAL      1024
#define SHIFT       10    // remember to change this to log2(NORMAL)

#define FRICTION    0    // what should it be?
#define MAX_SPEED   (2     * NORMAL)

#define RADIUS      (2     * NORMAL)

#define EAST_WALL   (640   * NORMAL - RADIUS)
#define WEST_WALL   (0     * NORMAL + RADIUS)
#define NORTH_WALL  (480   * NORMAL - RADIUS)
#define SOUTH_WALL  (0     * NORMAL + RADIUS)

// Pocket boundaries
#define P1_UPPER  (640     * NORMAL)
#define P1_LOWER  (636     * NORMAL)
#define P2_UPPER  (322     * NORMAL)
#define P2_LOWER  (318     * NORMAL)
#define P3_UPPER  (4       * NORMAL)
#define P3_LOWER  (0       * NORMAL)

// These will be read in from peripherals
#define CUE_POS_X 10
#define CUE_POS_Y 10

#define BUTTON_ADDR       0xFFFFFFFF
#define BUTTON_VALID_BIT  0x00000001    // valid bit for pool cue button

enum OBJECT_TYPE  {SOLIDS,    STRIPES,  EIGHT,     CUE,     TEST};
enum GAME_STATE   {P1_TURN,   P2_TURN,  CALIBRATION};
enum WALL_DIR     {NORTH,     EAST,     SOUTH,      WEST};

class GameState;
class Ball {

public:
  // Physics properties
  int pos_x;
  int pos_y;
  int speed_x;
  int speed_y;

  // Other properties
  OBJECT_TYPE colour  = SOLIDS;
  int id;

  // State
  bool isMove       = false;
  bool exist        = true;

  // Double-buffered Drawing positions
  int t             = 0;  // For 'hit prediction'
  int pre_pos_x;
  int pre_pos_y;

  // Collision Handling ~ Pseudo Hardware
  int col_id        = 0;  // Each bit's index represents a ball
  int prev_col_id   = 0;  // Doubled buffered: previous frame's collisions
  // 17 - North Wall | 18 - East Wall | 19 - South Wall | 20 - West Wall

  // Multi-collisions
  int col_speed_x   = 0;
  int col_speed_y   = 0;
  int num_cols      = 0;  // use this to divide net momentum
};

class GameState {
public:
  GAME_STATE state;
  bool done = false;      // if game is finished
  int num_balls;
  Ball *ball[NUM_BALLS];   // List of pointers to all game balls

  // Data for frame updates
  int mov_ids[NUM_BALLS];         // List of moving balls
  int num_movs = 0;

  // Player States
  int turn_id = 0;
  int score[2] = {0,0};

  // Cue states for drawing the cue's position
  int cue_pos_x, cue_pos_y;
  bool cue_down = false;

  // Turn Statues
  bool scratch = 0;
  bool scored = 0;
  bool cue_first_hit = 0;
  bool can_hit_eight_ball[2] = {false, false};
};


// Derivations
unsigned int getSpeed(Ball ball_a);   // perhaps should force this to int?
int collisionAngle(Ball ball_a, Ball ball_b);
int relativeAngle(Ball ball_a, Ball ball_b, int hit_angle);
void getXYComponents(int &x, int &y, int angle, int magnitude);
int getNormalAngle(int hit_angle, Ball ball_a);

// Collision Calculations
bool isCircleCollision(int x, int y);
bool isCollision(Ball *ball_a, Ball *ball_b);
bool isStutter(Ball *ball_a, int id);
void imminentCollision(Ball * ball_a, Ball *ball_b);
void bufferPreviousCollisions(Ball *ball_a);
void setCollisionBit(Ball *ball_a, int id);

void headOnCollision(Ball *ball_a, Ball *ball_b);
void statCollision(Ball *ball_a, Ball *ball_b, int relative_angle, int hit_angle);
void dynamicCollision(Ball *ball_a, Ball *ball_b, int hit_angle);
void collisionForce(Ball *ball_a, Ball *ball_b);

// Game Rules: Scoring
bool isInPocket(Ball ball_a, int upper, int lower);
void incrementScore(GameState *g);
bool isPlayersBall(GameState *g, int ball_id);
bool isScore(Ball ball_a, WALL_DIR wall);

// Game State Status
bool isWon(GameState g);
bool isEndTurn(GameState g);
bool isMoving(Ball ball_a);

// Ball Status
void updateVelocity(Ball *ball_a, GameState *g);  // plus friction!
void updatePosition(Ball *ball_a, GameState *g);

// Moving Ball List Manager
void addMoveListEntry(GameState *g, int id);
void removeMoveListEntry(GameState *g, int id);
void removeBall(GameState *g, int id, int turn);

// Collision List Manager
void addColListEntry(Ball *ball_a);
void removeColList(GameState *g);

// Bound Ball Properties
void boundSpeed(Ball *ball_a);
void boundPosition(Ball *ball_a, GameState *g);

// Pool Cue
void cueSpeed(int * x_pos, int * y_pos, Ball *cue_ball);
bool cueHit(int cut_x, int cur_x, Ball *cue_ball);
void cuePollPosition(Ball *cue_ball, GameState *g);
void replaceCueBall(Ball *cue_ball, GameState *g);

// After calling "cuePollPosition", the cue ball will be moving.
// Calling it at the start of each players turn should be all that is needed.

// Per Frame Operation
void Step(GameState *g);

// Data Validation
bool dataValid(int address, int bits);

// Initialize Game
void initGameBalls(GameState *g);

// Manage Scoring
void incrementScore(GameState *g);

void initMoveList(GameState *g);
void scratched_cue(GameState *g);
bool all_balls_sunk (GameState *g, int turn_id);
#if !FPGA
// Debug functions
void printPosition(Ball *ball_a);
void printVelocity(Ball *ball_a);
void printBall(Ball *ball_a);
void printScore(GameState g, int id);
void readLine(fstream &infile, Ball *ball);
#endif
#endif
//TODO:
// Implement Collision Zones
// - needs to be loosely based on ball size and speed
// - needs to be divisible by 2 ?
//
// Implement Player Turns
// - Switch on no change in score
//
// Implement 8-Ball Rules
// - Automatic game loss if 8-ball sunk prematurely
// - Automatic win if sunk appropriately
//
// Implement Scratch rules
// - Scratch if hit non-player ball first
// - Scratch if sink non-player ball
//
// Implement Scratch ball placement
// - Place cue ball position somehow.
// - David?

#pragma once
#include "structs.h"

// Set metrics of the frame
#define FRAME_WIDTH 800
#define FRAME_HEIGHT 800
#define FRAME_UPPER_Y 100
#define FRAME_LOWER_Y (FRAME_UPPER_Y + FRAME_HEIGHT)
#define FRAME_LEFT_X 200
#define FRAME_RIGHT_X (FRAME_LEFT_X + FRAME_WIDTH)

// Set size of objects
#define SNACK_BLOCK_SIDE 20
#define FRUIT_SIZE 20

typedef enum { Up = 1, Down = 2, Right = 3, Left = 4 } Destination;

// Overall score
int score = 0;
int t = 0;

RECT rect;
// Create our snake
TSnake snake;
// Other objects (fruits)
PObject fruits = NULL;
int fruitsCnt = 0; // The current number of fruits
// Start new game flag
BOOL startNewGame = FALSE;
// Set the snake speed
float snakeSpeed = 1.5;
// My timer
int prevTime = 0;
int curTime = 0;

int collisionCnt = 0;


// Define functions
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
Tpoint point(float x, float y);
void WinInit();
void SnakeInit();
void ObjectInit(TObject* obj, float xPos, float yPos, float width, float height, char objType);
void WinMove();
void SnakeControl();
void ObjectMove(TObject* obj);
void TailMove(TObject* obj);
void WinShow(HDC dc);
void ObjectShow(TObject obj, HDC dc);
void FrameShow(HDC dc);
void CheckBoundaries();
void GenerateFruit();
void AddFruit(float xPos, float yPos);
BOOL ObjectCollision(TObject o1, TObject o2);
void DelFruits();
void UpdateSpeed();
void AddSnakeBlock();
void ChangeDirections();
void CheckSnakeCollision();
void SyncBlocks();
void InitTime();
void PeriodicDeleteFruits();
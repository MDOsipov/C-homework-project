#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <sys/timeb.h>
#include <stdio.h>

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

typedef struct Spoint
{
    float x, y;
} Tpoint;

typedef struct Sobject
{
    Tpoint pos;
    Tpoint size;
    COLORREF brush;
    Tpoint speed;
    char oType;
    BOOL isDel;
    float destMove;
} TObject, * PObject;

typedef struct SSnake
{
    int length;
    PObject parts;
} TSnake;

typedef enum { Up = 1, Down = 2, Right = 3, Left = 4 } Destination;

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

int collisionCnt = 0;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR szCmdLine, int iCmdShow)
{
    static TCHAR szAppName[] = TEXT("HelloWin");
    HWND         hwnd;
    MSG          msg;
    WNDCLASS     wndclass;

    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;



    if (!RegisterClass(&wndclass))
    {
        MessageBox(NULL, TEXT("This program requires Windows NT!"),
            szAppName, MB_ICONERROR);
        return 0;
    }
    hwnd = CreateWindow(szAppName,                  // window class name
        TEXT("The Hello Program"), // window caption
        WS_OVERLAPPEDWINDOW,        // window style
        1920 / 2 - 600,              // initial x position
        0,              // initial y position
        1200,              // initial x size
        1080,              // initial y size
        NULL,                       // parent window handle
        NULL,                       // window menu handle
        hInstance,                  // program instance handle
        NULL);                     // creation parameters

    // Create device context
    HDC dc = GetDC(hwnd);

    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);
    WinInit();

    while (1)
    {
        if (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);

        }
        else
        {
            t++;
            WinMove();
            for (int i = 0; i < fruitsCnt; i++)
            {
                if (ObjectCollision(*(fruits + i), *(snake.parts)) && (fruits + i)->isDel != TRUE)
                {
                    (fruits + i) -> isDel = TRUE;
                    score++;
                    AddSnakeBlock();
                }
            }
           
            ChangeDirections();
            WinShow(dc);
            CheckBoundaries();
            ChangeDirections();
            SyncBlocks();
            CheckSnakeCollision();
            DelFruits();
            ChangeDirections();

        }
    }
    ReleaseDC(hwnd, dc);
    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC         hdc;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_CREATE:
        // PlaySound(TEXT("hellowin.wav"), NULL, SND_FILENAME | SND_ASYNC);
        return 0;

    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);

        GetClientRect(hwnd, &rect);

        EndPaint(hwnd, &ps);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

Tpoint point(float x, float y)
{
    Tpoint pt;
    pt.x = x;
    pt.y = y;
    return pt;
}

void WinInit()
{
    collisionCnt = 0;
    startNewGame = FALSE;
    fruitsCnt = 0;
    score = 0;
    if (!fruits)
    {
        free(fruits);
    }
    fruits = NULL;
    snakeSpeed = 2;
    free(snake.parts);
    SnakeInit();
}

void SnakeInit()
{
    snake.length = 1;
    snake.parts = (TObject*)malloc(sizeof(TObject));
    ObjectInit(snake.parts, FRAME_LEFT_X + (FRAME_RIGHT_X - FRAME_LEFT_X) / 2, FRAME_UPPER_Y + (FRAME_LOWER_Y - FRAME_UPPER_Y) / 2, SNACK_BLOCK_SIDE, SNACK_BLOCK_SIDE, 's');
}

void ObjectInit(TObject* obj, float xPos, float yPos, float width, float height, char objType)
{
    int choose = 0;
    srand((int)time(NULL));

    obj->pos = point(xPos, yPos);
    obj->size = point(width, height);
    obj->brush = RGB(0, 255, 0);
    obj->speed = point(0, 0);
    obj->oType = objType;
    obj->isDel = FALSE;
    obj->oType = objType;
    obj->destMove = 0;
  
    if (objType == 's')
    {
        choose = rand() % 4;
        if (choose == 0)
        {
            obj->speed = point(snakeSpeed, 0);
        }
        else if (choose == 1)
        {
            obj->speed = point(-snakeSpeed, 0);
        }
        else if (choose == 2)
        {
            obj->speed = point(0, snakeSpeed);
        }
        else {
            obj->speed = point(0, -snakeSpeed);
        }
    }
    else if (objType == 'f')
    {
        COLORREF color = RGB(100 + rand() % 155, 100 + rand() % 155, 100 + rand() % 155);
        obj->brush = color;
    }
}

void WinMove()
{
    if (startNewGame)
    {
        WinInit();
    }

    SnakeControl();
    // Move head
    ObjectMove(snake.parts);
    // Move tail
    for (int i = 1; i < snake.length; i++)
    {
        ObjectMove(snake.parts + i);
    }
    GenerateFruit();
}

void SnakeControl()
{
    Tpoint prevSpeed = point(snake.parts[0].speed.x, snake.parts[0].speed.y);
    Destination prevHeadDest = FindDestination(snake.parts);
    
    if (GetKeyState('W') < 0 && abs(snake.parts[0].destMove) >= SNACK_BLOCK_SIDE)
    {
        if (prevSpeed.y > 0)
        {
            startNewGame = TRUE;
        }
        snake.parts[0].speed.y = -snakeSpeed;
        snake.parts[0].speed.x = 0;
        if (prevHeadDest != Up)
        {
            snake.parts[0].destMove = 0;
        }
    }
    else if (GetKeyState('S') < 0 && abs(snake.parts[0].destMove) >= SNACK_BLOCK_SIDE)
    {
        if (prevSpeed.y < 0)
        {
            startNewGame = TRUE;
        }
        snake.parts[0].speed.y = snakeSpeed;
        snake.parts[0].speed.x = 0;
        if (prevHeadDest != Down)
        {
            snake.parts[0].destMove = 0;
        }
    }
    else if (GetKeyState('A') < 0 && abs(snake.parts[0].destMove) >= SNACK_BLOCK_SIDE)
    {
        if (prevSpeed.x > 0)
        {
            startNewGame = TRUE;
        }
        snake.parts[0].speed.x = -snakeSpeed;
        snake.parts[0].speed.y = 0;
        if (prevHeadDest != Left)
        {
            snake.parts[0].destMove = 0;
        }
    }
    else if (GetKeyState('D') < 0 && abs(snake.parts[0].destMove) >= SNACK_BLOCK_SIDE)
    {
        if (prevSpeed.x < 0)
        {
            startNewGame = TRUE;
        }
        snake.parts[0].speed.x = snakeSpeed;
        snake.parts[0].speed.y = 0;
        if (prevHeadDest != Right)
        {
            snake.parts[0].destMove = 0;
        }
    }
    else 
    {
        if (snake.parts[0].speed.x > 0)
        {
            snake.parts[0].speed.x = snakeSpeed;
        }
        else if (snake.parts[0].speed.x < 0)
        {
            snake.parts[0].speed.x = -snakeSpeed;
        }
        else if (snake.parts[0].speed.y > 0)
        {
            snake.parts[0].speed.y = snakeSpeed;
        }
        else if (snake.parts[0].speed.y < 0)
        {
            snake.parts[0].speed.y = -snakeSpeed;
        }
    }

    for (int i = 1; i < snake.length; i++)
    {
        if (snake.parts[i].speed.x > 0)
        {
            snake.parts[i].speed.x = snakeSpeed;
        }
        else if (snake.parts[i].speed.x < 0)
        {
            snake.parts[i].speed.x = -snakeSpeed;
        }
        else if (snake.parts[i].speed.y > 0)
        {
            snake.parts[i].speed.y = snakeSpeed;
        }
        else if (snake.parts[i].speed.y < 0)
        {
            snake.parts[i].speed.y = -snakeSpeed;
        }
    }
}

void ObjectMove(TObject* obj)
{
    obj->pos.x += obj->speed.x;
    obj->pos.y += obj->speed.y;
    obj->destMove += (obj->speed.x + obj->speed.y);
}

void TailMove(TObject* obj)
{
    switch (FindDestination(snake.parts + snake.length - 2))
    {
        case Right:
            obj->speed = point(snakeSpeed, 0);
            break;
        case Left:
            obj->speed = point(-snakeSpeed, 0);
            break;
        case Up:
            obj->speed = point(0, -snakeSpeed);
            break;
        case Down:
            obj->speed = point(0, snakeSpeed);
            break;
    }
}

void WinShow(HDC dc)
{
    HDC memDC = CreateCompatibleDC(dc);
    HBITMAP memBM = CreateCompatibleBitmap(dc, rect.right - rect.left, rect.bottom - rect.top);
    SelectObject(memDC, memBM);

    SelectObject(memDC, GetStockObject(DC_BRUSH));
    SetDCBrushColor(memDC, RGB(0, 0, 0));
    
    FrameShow(memDC);
    ObjectShow(*snake.parts, memDC);

    for (int i = 1; i < snake.length; i++)
    {
        ObjectShow(*(snake.parts + i), memDC);
    }

    for (int i = 0; i < fruitsCnt; i++)
    {
        ObjectShow(fruits[i], memDC);
    }

    wchar_t bufferForScore[50];
    wchar_t bufferForSpeed[50];
    int buf_len = 0;

    buf_len = wsprintf(bufferForScore, TEXT("Score = %i"), score);
    TextOut(memDC, 400, 50, bufferForScore, buf_len);

    buf_len = swprintf(bufferForSpeed, 50, TEXT("Head dest move = %f"), snake.parts[0].destMove);
    TextOut(memDC, 10, 50, bufferForSpeed, buf_len);

    buf_len = swprintf(bufferForSpeed, 50, TEXT("Snake speed (module) = %f"), snakeSpeed);
    TextOut(memDC, 250, 50, bufferForSpeed, buf_len);

    buf_len = swprintf(bufferForSpeed, 50, TEXT("Snake speed (x,y) = (%f, %f)"), snake.parts[0].speed.x, snake.parts[0].speed.y);
    TextOut(memDC, 250, 80, bufferForSpeed, buf_len);

    buf_len = wsprintf(bufferForScore, TEXT("Fruits num = %i"), fruitsCnt);
    TextOut(memDC, 600, 50, bufferForScore, buf_len);

    buf_len = wsprintf(bufferForScore, TEXT("Snake length = %i"), snake.length);
    TextOut(memDC, 600, 80, bufferForScore, buf_len);

    buf_len = wsprintf(bufferForScore, TEXT("Snake head (x,y) = (%i, %i)"), (int)snake.parts[0].pos.x, (int)snake.parts[0].pos.y);
    TextOut(memDC, 600, 80, bufferForScore, buf_len);

    buf_len = wsprintf(bufferForScore, TEXT("Time = %i"), t);
    TextOut(memDC, 800, 50, bufferForScore, buf_len);

    buf_len = wsprintf(bufferForScore, TEXT("Collision count = %i"), collisionCnt);
    TextOut(memDC, 800, 80, bufferForScore, buf_len);

    buf_len = wsprintf(bufferForScore, TEXT("Milisecs = %i"), (int)time(NULL) * 1000 + getMiliseconds());
    TextOut(memDC, 950, 50, bufferForScore, buf_len);

    buf_len = wsprintf(bufferForScore, TEXT("Head destination = %i"), (int)FindDestination(snake.parts));
    TextOut(memDC, 950, 80, bufferForScore, buf_len);

    BitBlt(dc, 0, 0, rect.right - rect.left, rect.bottom - rect.top, memDC, 0, 0, SRCCOPY);
    DeleteDC(memDC);
    DeleteObject(memBM);
}

void ObjectShow(TObject obj, HDC dc)
{
    
    SelectObject(dc, GetStockObject(DC_PEN));
    SetDCPenColor(dc, RGB(0, 0, 0));
    SelectObject(dc, GetStockObject(DC_BRUSH));
    SetDCBrushColor(dc, obj.brush);

    BOOL(*shape)(HDC, int, int, int, int);

    if (obj.oType == 's')
    {
        shape = Rectangle;
    }
    else if (obj.oType == 'f')
    {
        shape = Ellipse;
    }
    else {
        shape = Rectangle;
    }

    shape(dc, (int)(obj.pos.x), (int)(obj.pos.y),
        (int)(obj.pos.x + obj.size.x), (int)(obj.pos.y + obj.size.y));

}

void FrameShow(HDC dc)
{
    SelectObject(dc, GetStockObject(DC_PEN));
    SetDCPenColor(dc, RGB(255, 255, 255));
    SelectObject(dc, GetStockObject(DC_BRUSH));
    SetDCBrushColor(dc, RGB(0, 0, 0));

    Rectangle(dc, FRAME_LEFT_X, FRAME_UPPER_Y, FRAME_RIGHT_X, FRAME_LOWER_Y);
}

void CheckBoundaries()
{
    if (snake.parts->pos.x + snake.parts->size.x >  FRAME_RIGHT_X || snake.parts->pos.x < FRAME_LEFT_X ||
        snake.parts->pos.y < FRAME_UPPER_Y || snake.parts->pos.y + snake.parts->size.y > FRAME_LOWER_Y)
    {
        startNewGame = TRUE;
    }
}

void GenerateFruit()
{
    srand((int)time(NULL) * 1000 + getMiliseconds());
    int xPos = FRAME_LEFT_X + rand() % FRAME_WIDTH;
    int yPos = FRAME_UPPER_Y + rand() % FRAME_HEIGHT;   
    BOOL check = FALSE;

    Sleep(1);

    int k = rand() % 200;
    if (k == 1)
    {
        fruitsCnt++;
        check = TRUE;
        AddFruit(xPos, yPos, fruitsCnt);
    }
}

void AddFruit(float xPos, float yPos, int frt_cnt)
{
    if (fruits = realloc(fruits, sizeof(*fruits) * frt_cnt))
    {
       ObjectInit(fruits + frt_cnt - 1, xPos, yPos, FRUIT_SIZE, FRUIT_SIZE, 'f');
    }
    else
    {
        return;
    }
}

BOOL ObjectCollision(TObject o1, TObject o2)
{
    if (o1.isDel == TRUE)
    {
        return FALSE;
    }
    return ((o1.pos.x + o1.size.x) > o2.pos.x) && (o1.pos.x < (o2.pos.x + o2.size.x)) &&
        ((o1.pos.y + o1.size.y) > o2.pos.y) && (o1.pos.y < (o2.pos.y + o2.size.y));
}

void DelFruits()
{
    int del_cntr = 0;
    for (int i = 0; i < fruitsCnt; i++)
    {
        if ((fruits + i) -> isDel == TRUE)
        {
            del_cntr++;
            fruitsCnt--;
            fruits[i] = fruits[fruitsCnt];
            fruits = realloc(fruits, sizeof(*fruits) * fruitsCnt);
        }
    }
    
    for (int i = 0; i < del_cntr; i++)
    {
        UpdateSpeed();
    }
}

void UpdateSpeed()
{
    snakeSpeed += 0.25;
}

void AddSnakeBlock()
{
    int xPrevPos = snake.parts[snake.length - 1].pos.x;
    int yPrevPos = snake.parts[snake.length - 1].pos.y;
    Tpoint prevSpeed = snake.parts[snake.length - 1].speed;

    snake.length++;
    snake.parts = (TObject*)realloc(snake.parts, snake.length * sizeof(TObject));

    switch (FindDestination(snake.parts + snake.length - 2))
    {
        case Right:
            ObjectInit(snake.parts + snake.length - 1, xPrevPos - SNACK_BLOCK_SIDE, yPrevPos, SNACK_BLOCK_SIDE, SNACK_BLOCK_SIDE, 't');
            break;
        case Left:
            ObjectInit(snake.parts + snake.length - 1, xPrevPos + SNACK_BLOCK_SIDE, yPrevPos, SNACK_BLOCK_SIDE, SNACK_BLOCK_SIDE, 't');
            break;
        case Up:
            ObjectInit(snake.parts + snake.length - 1, xPrevPos, yPrevPos + SNACK_BLOCK_SIDE, SNACK_BLOCK_SIDE, SNACK_BLOCK_SIDE, 't');
            break;
        case Down:
            ObjectInit(snake.parts + snake.length - 1, xPrevPos, yPrevPos - SNACK_BLOCK_SIDE, SNACK_BLOCK_SIDE, SNACK_BLOCK_SIDE, 't');
            break;
    }

    TailMove(snake.parts + snake.length - 1);

}

Destination FindDestination(PObject obj)
{
    if (obj->speed.x > 0)
    {
        return Right;
    }
    else if (obj->speed.x < 0)
    {
        return Left;
    }
    else if (obj->speed.y > 0)
    {
        return Down;
    }
    else if (obj->speed.y < 0)
    {
        return Up;
    }
}

int getMiliseconds() 
{
    struct _timeb timebuffer;
    _ftime(&timebuffer);
    return (int)timebuffer.millitm; 
}

void ChangeDirections()
{
    for (int i = 0; i < (snake.length - 1); i++)
    {
        if (FindDestination(snake.parts + i) != FindDestination(snake.parts + i + 1))
        {
            switch (FindDestination(snake.parts + i))
            {
                case Right:
                    if ((int)snake.parts[i].pos.x >= ((int)snake.parts[i + 1].pos.x + SNACK_BLOCK_SIDE))
                    {
                        (snake.parts + i + 1)->speed = point(snakeSpeed, 0);
                        snake.parts[i + 1].pos.y = snake.parts[i].pos.y;
                        snake.parts[i + 1].pos.x = snake.parts[i].pos.x - SNACK_BLOCK_SIDE;
                        snake.parts[i + 1].destMove = 0;
                    }
                    break;
                case Left:
                    if (((int)snake.parts[i].pos.x + SNACK_BLOCK_SIDE) <= (int)snake.parts[i + 1].pos.x)
                    {
                        (snake.parts + i + 1)->speed = point(-snakeSpeed, 0);
                        snake.parts[i + 1].pos.y = snake.parts[i].pos.y;
                        snake.parts[i + 1].pos.x = snake.parts[i].pos.x + SNACK_BLOCK_SIDE;
                        snake.parts[i + 1].destMove = 0;
                    }
                    break;
                case Up:
                    if ((int)snake.parts[i].pos.y <= ((int)snake.parts[i + 1].pos.y - SNACK_BLOCK_SIDE))
                    {
                        (snake.parts + i + 1)->speed = point(0, -snakeSpeed);
                        snake.parts[i + 1].pos.x = snake.parts[i].pos.x;
                        snake.parts[i + 1].pos.y = snake.parts[i].pos.y + SNACK_BLOCK_SIDE;
                        snake.parts[i + 1].destMove = 0;
                    }
                    break;
                case Down:
                    if ((int)snake.parts[i].pos.y >= ((int)snake.parts[i + 1].pos.y + SNACK_BLOCK_SIDE))
                    {
                        (snake.parts + i + 1)->speed = point(0, snakeSpeed);
                        snake.parts[i + 1].pos.x = snake.parts[i].pos.x;
                        snake.parts[i + 1].pos.y = snake.parts[i].pos.y - SNACK_BLOCK_SIDE;
                        snake.parts[i + 1].destMove = 0;
                    }
                    break;
            }
        }
    }
}

void CheckSnakeCollision()
{
    for (int i = 3; i < snake.length; i++)
    {
        if (ObjectCollision(snake.parts[0], snake.parts[i]) && FindDestination(snake.parts) != FindDestination(snake.parts + i))
        {
            startNewGame = TRUE;
            collisionCnt++;
        }
    }
}

void SyncBlocks()
{
    for (int i = 0; i < (snake.length - 1); i++)
    {
        if (FindDestination(snake.parts + i) != FindDestination(snake.parts + i + 1) && abs((snake.parts + i)->destMove) >= (SNACK_BLOCK_SIDE - 5))
        {
            switch (FindDestination(snake.parts + i))
            {
                case Right:
                    snake.parts[i + 1].pos.y = snake.parts[i].pos.y;
                    break;
                case Left:
                    snake.parts[i + 1].pos.y = snake.parts[i].pos.y;
                    break;
                case Up:
                    snake.parts[i + 1].pos.x = snake.parts[i].pos.x;
                    break;
                case Down:
                    snake.parts[i + 1].pos.x = snake.parts[i].pos.x;
                    break;
            }
        }
    }
}
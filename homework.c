#include <windows.h>
#include <stdlib.h>
#include <time.h>
#define FRAME_WIDTH 800
#define FRAME_HEIGHT 800
#define FRAME_UPPER_Y 100
#define FRAME_LOWER_Y (FRAME_UPPER_Y + FRAME_HEIGHT)
#define FRAME_LEFT_X 200
#define FRAME_RIGHT_X (FRAME_LEFT_X + FRAME_WIDTH)

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
} TObject, * PObject;

typedef struct SSnake
{
    int length;
    PObject parts;
} TSnake;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
Tpoint point(float x, float y);
void WinInit();
void SnakeInit();
void ObjectInit(TObject* obj, float xPos, float yPos, float width, float height, char objType);
void WinMove();
void SnakeControl();
void ObjectMove(TObject* obj);
void WinShow(HDC dc);
void ObjectShow(TObject obj, HDC dc);
void FrameShow(HDC dc);
void CheckBoundaries();
void GenerateFruit();
void AddFruit(float xPos, float yPos);
BOOL ObjectCollision(TObject o1, TObject o2);
void DelFruits();
void UpdateSpeed();

// Overall score
int score = 0;

RECT rect;
// Create our snake
TSnake snake;
// Other objects (fruits)
PObject fruits = NULL;
int fruitsCnt = 0; // The current number of fruits
BOOL startNewGame = FALSE;
float snakeSpeed = 2;




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
            WinMove();
            WinShow(dc);
            CheckBoundaries();
            Sleep(5);
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
        PlaySound(TEXT("hellowin.wav"), NULL, SND_FILENAME | SND_ASYNC);
        return 0;

    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);

        GetClientRect(hwnd, &rect);


        DrawText(hdc, TEXT("Hello, Windows 98!"), -1, &rect,
            DT_SINGLELINE | DT_CENTER | DT_VCENTER);
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
    startNewGame = FALSE;
    fruitsCnt = 0;
    fruits = NULL;
    snakeSpeed = 2;
    SnakeInit();
}

void SnakeInit()
{
    snake.length = 1;
    snake.parts = ((TObject*)malloc(sizeof(TObject))) ? (TObject*)malloc(sizeof(TObject)) : NULL;
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
        COLORREF color = RGB(rand() % 255, rand() % 255, rand() % 255);
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
    ObjectMove(snake.parts);
    GenerateFruit();
    DelFruits();
}

void SnakeControl()
{
    // snake.speed.x = 0;
    // snake.speed.y = 0;

    if (GetKeyState('W') < 0)
    {
        snake.parts[0].speed.y = -snakeSpeed;
        snake.parts[0].speed.x = 0;
    }
    if (GetKeyState('S') < 0)
    {
        snake.parts[0].speed.y = snakeSpeed;
        snake.parts[0].speed.x = 0;

    }
    if (GetKeyState('A') < 0)
    {
        snake.parts[0].speed.x = -snakeSpeed;
        snake.parts[0].speed.y = 0;
    }
    if (GetKeyState('D') < 0)
    {
        snake.parts[0].speed.x = snakeSpeed;
        snake.parts[0].speed.y = 0;
    }

}

void ObjectMove(TObject* obj)
{
    obj->pos.x += obj->speed.x;
    obj->pos.y += obj->speed.y;
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
    for (int i = 0; i < fruitsCnt; i++)
    {
        ObjectShow(fruits[i], memDC);
        if (ObjectCollision(fruits[i], *(snake.parts)))
        {
            score++;
            fruits[i].isDel = TRUE;
        }
        
    }

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
    srand(time(NULL));
    int xPos = FRAME_LEFT_X + rand() % FRAME_WIDTH;
    int yPos = FRAME_UPPER_Y + rand() % FRAME_HEIGHT;   

    int k = rand() % 7;
    if (k == 1)
    {
        AddFruit(xPos, yPos);
    }
    if (k == 2)
    {
        AddFruit(xPos, yPos);
    }
}

void AddFruit(float xPos, float yPos)
{
    fruitsCnt++;
    if (fruits = realloc(fruits, sizeof(*fruits) * fruitsCnt))
    {
       ObjectInit(fruits + fruitsCnt - 1, xPos, yPos, FRUIT_SIZE, FRUIT_SIZE, 'f');
    }
    else
    {
        return;
    }
}

BOOL ObjectCollision(TObject o1, TObject o2)
{
    return ((o1.pos.x + o1.size.x) > o2.pos.x) && (o1.pos.x < (o2.pos.x + o2.size.x)) &&
        ((o1.pos.y + o1.size.y) > o2.pos.y) && (o1.pos.y < (o2.pos.y + o2.size.y));
}

void DelFruits()
{
    int del_cntr = 0;
    for (int i = 0; i < fruitsCnt; i++)
    {
        if (fruits[i].isDel)
        {
            del_cntr++;
            fruitsCnt--;
            fruits[i] = fruits[fruitsCnt];
            fruits = realloc(fruits, sizeof(*fruits) * fruitsCnt);
        }
    }
    Sleep(5);
    for (int i = 0; i < del_cntr; i++)
    {
        UpdateSpeed();
    }
}

void UpdateSpeed()
{
    snakeSpeed += 0.02;
}

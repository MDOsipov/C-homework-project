#pragma once
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
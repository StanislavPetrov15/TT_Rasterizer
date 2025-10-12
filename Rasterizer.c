//(PRIVATE)

const int PIXEL_SIZE = 4;
const double PI = 3.14159265358979323846;
const double VERTEXOID_SHIFT = 0.01;
const unsigned int N_CROSSING = 0;
const unsigned int O_CROSSING = 1;
const unsigned int T_CROSSING = 2;
const unsigned int O_BEGIN = 8;
const unsigned int O_END = 14;
const unsigned int T_BEGIN = 15;
const unsigned int T_END = 21;
const unsigned int O_LEFT = 22;
const unsigned int T_LEFT = 23;
const unsigned int O_RIGHT = 24;
const unsigned int T_RIGHT = 25;
const unsigned int INITIAL_PIXEL_MARKER = 0b10000000000000000000000000000000;
const unsigned int EXTEROID = 0;
const unsigned int CONTUROID = 1;
const unsigned int INTEROID = 2;

unsigned int* MetaCanvas_S1 = NULL;
unsigned short* MetaCanvas_S2 = NULL;
int MetaCanvasWidth;
int MetaCanvasHeight;
int PreviousPixelX;
int PreviousPixelY;
int StringWidth = 0; //in pixels; used for horizontal (string gradients)
int StringHeight = 0; //in pixels; used for vertical (string gradients)
int StringBeginX =  0; //in pixels; used for horizontal (string gradients)
int StringBeginY = 0; //in pixels; used for vertical (string gradients)

/* two-stage drawing is needed (first in a meta-canvas byte array, then in the real canvas); this allows drawing over non-uniform background (
   consisting of many different colors) and also allows proper drawing of certain characters - for example Unicode codepoint Dx295 in
   Mistral (typeface) */

/* format of the elements in MetaCanvas_S1:
   - bits [0..6]: coverage
   - bits [8..14]: position of the right-most opening crossing
   - bits [15..21]: position of the right-most terminating crossing
   - bit [22]: opening crossing on the left side
   - bit [23]: terminating crossing on the left side
   - bit [24]: opening crossing on the right side
   - bit [25]: terminating crossing on the right side */

/* format of the elements in MetaCanvas_S2:
   - bits [0..7] pixel type (determined in stage 1): 0 :: exteroid | 1 :: conturoid | 2 :: interoid
   - bits [8..15] coverage */

enum GlyphColorizationMode
{
    //solid color
    GCM_SOLID,

    //horizontal gradient between two or more colors
    GCM_HORIZONTAL_GRADIENT,

    //vertical gradient between two or more colors
    GCM_VERTICAL_GRADIENT,

    //(INTERNAL)
    GCM_S_HORIZONTAL_GRADIENT,

    //(INTERNAL)
    GCM_S_VERTICAL_GRADIENT,
};

typedef enum GlyphColorizationMode GlyphColorizationMode;

//(PUBLIC)

enum ColorComponentOrder
{
    RGBA_ORDER,
    BGRA_ORDER
};

typedef enum ColorComponentOrder ColorComponentOrder;

enum StringColorizationMode
{
    //each (non-empty) glyph in the string has the same color
    SCM_SOLID_IDENTICAL,

    //each (non-empty) glyph in the string has an associated color; repeated in series
    SCM_SOLID_INDIVIDUAL,

    //each word in the string has an associated color; repeated in series
    SCM_SOLID_WORD,

    /* horizontal gradient between two or more colors; the gradient is applied to each glyph
       in the string, not to the string as a whole */
    SCM_HORIZONTAL_GRADIENT_GLYPH,

    /* vertical gradient between two or more colors; the gradient is applied to each glyph
       in the string, not to the string as a whole */
    SCM_VERTICAL_GRADIENT_GLYPH,

    //horizontal gradient between two or more colors; the gradient is applied to the string as a whole
    SCM_HORIZONTAL_GRADIENT_STRING,

    //vertical gradient between two or more colors; the gradient is applied to the string as a whole
    SCM_VERTICAL_GRADIENT_STRING
};

typedef enum StringColorizationMode StringColorizationMode;

struct Bitex
{
    double X;
    double Y;
};

typedef struct Bitex Bitex;

struct Contour
{
    bool IsFilled;
    short* X_Coordinates;
    short* Y_Coordinates;
    unsigned char* Flags;
    int NumberOfPoints;
    int OriginalIndex; //index before reordering
};

typedef struct Contour Contour;

struct tt_Rectangle
{
    int X;
    int Y;
    int Width;
    int Height;
};

typedef struct tt_Rectangle tt_Rectangle;

bool RectangleContainsPoint(const tt_Rectangle* _rectangle, int _x, int _y)
{
    return _x >= _rectangle->X && _x <= _rectangle->X + (_rectangle->Width - 1) &&
           _y >= _rectangle->Y && _y <= _rectangle->Y + (_rectangle->Height - 1);
}

bool RectangleContainsRectangle(const tt_Rectangle* _rectangle1, const tt_Rectangle* _rectangle2)
{
    //lower-left vertex
    Bitex p1;
    p1.X = _rectangle2->X;
    p1.Y = _rectangle2->Y;

    //lower-right vertex
    Bitex p2;
    p2.X = (p1.X + _rectangle2->Width) - 1;
    p2.Y = p1.Y;

    //upper-left vertex
    Bitex p3;
    p3.X = p1.X;
    p3.Y = (p1.Y + _rectangle2->Height) - 1;

    //upper-right vertex
    Bitex p4;
    p4.X = (p1.X + _rectangle2->Width) - 1;
    p4.Y = (p1.Y + _rectangle2->Height) - 1;

    return RectangleContainsPoint(_rectangle1, p1.X, p1.Y) &&
           RectangleContainsPoint(_rectangle1, p2.X, p2.Y) &&
           RectangleContainsPoint(_rectangle1, p3.X, p3.Y) &&
           RectangleContainsPoint(_rectangle1, p4.X, p4.Y);
}

struct ContourPair
{
    Contour* Contour;
    Contour* DirectContainer;
};

typedef struct ContourPair ContourPair;

struct tt_rgba
{
    unsigned char R;
    unsigned char G;
    unsigned char B;
    unsigned char A;
};

typedef struct tt_rgba tt_rgba;

unsigned char Colors[] =
        {
                240, 248, 255, 0, //AliceBlue
                250, 235, 215, 0, //AntiqueWhite
                0, 255, 255, 0, //Aqua
                127, 255, 212, 0, //Aquamarine
                240, 255, 255, 0, //Azure
                245, 245, 220, 0, //Beige
                255, 228, 196, 0, //Bisque
                0, 0, 0, 0, //Black
                255, 235, 205, 0, //BlanchedAlmond
                0, 0, 255, 0, //Blue
                138, 43, 226, 0, //BlueViolet
                165,  42,  42, 0, //Brown
                222, 184, 135, 0, //Burlywood
                95, 158, 160, 0, //CadetBlue
                127, 255, 0, 0, //Chartreuse
                210, 105, 30, 0, //Chocolate
                255, 127, 80, 0, //Coral
                100, 149, 237, 0, //CornflowerBlue
                255, 248, 220, 0, //Cornsilk
                220, 20, 60, 0, //Crimson
                0, 255, 255, 0, //Cyan
                0, 0, 139, 0, //DarkBlue
                0, 139, 139, 0, //DarkCyan
                184, 134, 11, 0, //DarkGoldenrod
                169, 169, 169, 0, //DarkGray
                0, 100, 0, 0, //DarkGreen
                189, 183, 107, 0, //DarkKhaki
                139, 0, 139, 0, //DarkMagenta
                85, 107, 47, 0, //DarkOliveGreen
                255, 140, 0, 0, //DarkOrange
                153, 50, 204, 0, //DarkOrchid
                139, 0, 0, 0, //DarkRed
                233, 150, 122, 0, //DarkSalmon
                143, 188, 143, 0, //DarkSeaGreen
                72, 61, 139, 0, //DarkSlateBlue
                47, 79, 79, 0, //DarkSlateGray
                0, 206, 209, 0, //DarkTurquoise
                148, 0, 211, 0, //DarkViolet
                255, 20, 147, 0, //DeepPink
                0, 191, 255, 0, //DeepSkyBlue
                105, 105, 105, 0, //DimGray
                30, 144, 255, 0, //DodgerBlue
                178, 34, 34, 0, //Firebrick
                255, 250, 240, 0, //FloralWhite
                34, 139, 34, 0, //ForestGreen
                255, 0, 255, 0, //Fuchsia
                220, 220, 220, 0, //Gainsboro
                248, 248, 255, 0, //GhostWhite
                255, 215, 0, 0, //Gold
                218, 165, 32, 0, //Goldenrod
                128, 128, 128, 0, //Gray
                0, 255, 0, 0, //Green
                173, 255, 47, 0, //Green
                240, 255, 240, 0, //HoneyDew
                255, 105, 180, 0, //HotPink
                255, 92, 92, 0, //IndianRed
                75, 0, 130, 0, //Indigo
                255, 255, 240, 0, //Ivory
                240, 230, 140, 0, //Khaki
                230, 230, 250, 0, //Lavender
                255, 240, 245, 0, //LavenderBlush
                124, 252, 0, 0, //LawnGreen
                255, 250, 205, 0, //LemonChiffon
                173, 216, 230, 0, //LightBlue
                240, 128, 128, 0, //LightCoral
                224, 255, 255, 0, //LightCyan
                250, 250, 210, 0, //LightGoldenrodYellow
                211, 211, 211, 0, //LightGray
                144, 238, 144, 0, //LightGreen
                255, 182, 193, 0, //LightPink
                255, 160, 122, 0, //LightSalmon
                32, 178, 170, 0, //LightSeaGreen
                135, 206, 250, 0, //LightSkyBlue
                119, 136, 153, 0, //LightSlateGray
                176, 196, 222, 0, //LightSteelBlue
                255, 255, 224, 0, //LightYellow
                0, 255, 0, 0, //Lime
                50, 205, 50, 0, //LimeGreen
                250, 240, 230, 0, //Linen
                255, 0, 255, 0, //Magenta
                128, 0, 0, 0, //Maroon
                102, 205, 170, 0, //MediumAquamarine
                0, 0, 205, 0, //MediumBlue
                186, 85, 211, 0, //MediumOrchid
                147, 112, 219, 0, //MediumPurple
                123, 104, 238, 0, //MediumSlateBlue
                0, 250, 154, 0, //MediumSpringGreen
                72, 209, 204, 0, //MediumTurquoise
                199, 21, 133, 0, //MediumVioletRed
                25, 25, 112, 0, //MidnightBlue
                245, 255, 250, 0, //MintCream
                255, 228, 225, 0, //MistyRose
                255, 228, 181, 0, //Moccasin
                255, 222, 173, 0, //NavajoWhite
                0, 0, 128, 0, //Navy
                253, 245, 230, 0, //OldLace
                128, 128, 0, 0, //Olive
                107, 142, 35, 0, //OliveDrab
                255, 165, 0, 0, //Orange
                255, 69, 0, 0, //OrangeRed
                218, 112, 214, 0, //Orchid
                238, 232, 170, 0, //PaleGoldenrod
                152, 251, 152, 0, //PaleGreen
                175, 238, 238, 0, //PaleTurquoise
                219, 112, 147, 0, //PaleVioletRed
                255, 239, 213, 0, //PapayaWhip
                255, 218, 185, 0, //PeachPuff
                205, 133, 63, 0, //Peru
                255, 192, 203, 0, //Pink
                221, 160, 221, 0, //Plum
                176, 224, 230, 0, //PowderBlue
                128, 0, 128, 0, //Purple
                255, 0, 0, 0, //Red
                188, 143, 143, 0, //RosyBrown
                65, 105, 225, 0, //RoyalBlue
                139, 69, 19, 0, //SaddleBrown
                250, 128, 114, 0, //Salmon
                244, 164, 96, 0, //SandyBrown
                46, 139, 87, 0, //SeaGreen
                255, 245, 238, 0, //SeaShell
                160, 82, 45, 0, //Sienna
                192, 192, 192, 0, //Silver
                135, 206, 235, 0, //SkyBlue
                106, 90, 205, 0, //SlateBlue
                112, 128, 144, 0, //SlateGray
                255, 250, 250, 0, //Snow
                0, 255, 127, 0, //SpringGreen
                70, 130, 180, 0, //SteelBlue
                210, 180, 140, 0, //Tan
                0, 128, 128, 0, //Teal
                216, 191, 216, 0, //Thistle
                255, 99, 71, 0, //Tomato
                64, 224, 208, 0, //Turquoise
                238, 130, 238, 0, //Violet
                245, 222, 179, 0, //Wheat
                255, 255, 255, 0, //White
                245, 245, 245, 0, //WhiteSmoke
                255, 255, 0, 0, //Yellow
                154, 205, 50, 0, //YellowGreen
                0, 0, 0, 0 //InvalidValue
        };

const tt_rgba* C_ALICE_BLUE = (tt_rgba*) &Colors[0];
const tt_rgba* C_ANTIQUE_WHITE = (tt_rgba*) &Colors[4];
const tt_rgba* C_AQUA = (tt_rgba*) &Colors[8];
const tt_rgba* C_AQUAMARINE = (tt_rgba*) &Colors[12];
const tt_rgba* C_AZURE = (tt_rgba*) &Colors[16];
const tt_rgba* C_BEIGE = (tt_rgba*) &Colors[20];
const tt_rgba* C_BISQUE = (tt_rgba*) &Colors[24];
const tt_rgba* C_BLACK = (tt_rgba*) &Colors[28];
const tt_rgba* C_BLANCHED_ALMOND = (tt_rgba*) &Colors[32];
const tt_rgba* C_BLUE = (tt_rgba*) &Colors[36];
const tt_rgba* C_BLUE_VIOLET = (tt_rgba*) &Colors[40];
const tt_rgba* C_BROWN = (tt_rgba*) &Colors[44];
const tt_rgba* C_BURLYWOOD = (tt_rgba*) &Colors[48];
const tt_rgba* C_CADET_BLUE = (tt_rgba*) &Colors[52];
const tt_rgba* C_CHARTREUSE = (tt_rgba*) &Colors[56];
const tt_rgba* C_CHOCOLATE = (tt_rgba*) &Colors[60];
const tt_rgba* C_CORAL = (tt_rgba*) &Colors[64];
const tt_rgba* C_CORNFLOWER_BLUE = (tt_rgba*) &Colors[68];
const tt_rgba* C_CORNSILK = (tt_rgba*) &Colors[72];
const tt_rgba* C_CRIMSON = (tt_rgba*) &Colors[76];
const tt_rgba* C_CYAN = (tt_rgba*) &Colors[80];
const tt_rgba* C_DARK_BLUE = (tt_rgba*) &Colors[84];
const tt_rgba* C_DARK_CYAN = (tt_rgba*) &Colors[88];
const tt_rgba* C_DARK_GOLDENROD = (tt_rgba*) &Colors[92];
const tt_rgba* C_DARK_GRAY = (tt_rgba*) &Colors[96];
const tt_rgba* C_DARK_GREEN = (tt_rgba*) &Colors[100];
const tt_rgba* C_DARK_KHAKI = (tt_rgba*) &Colors[104];
const tt_rgba* C_DARK_MAGENTA = (tt_rgba*) &Colors[108];
const tt_rgba* C_DARK_OLIVE_GREEN = (tt_rgba*) &Colors[112];
const tt_rgba* C_DARK_ORANGE = (tt_rgba*) &Colors[116];
const tt_rgba* C_DARK_ORCHID = (tt_rgba*) &Colors[120];
const tt_rgba* C_DARK_RED = (tt_rgba*) &Colors[124];
const tt_rgba* C_DARK_SALMON = (tt_rgba*) &Colors[128];
const tt_rgba* C_DARK_SEA_GREEN = (tt_rgba*) &Colors[132];
const tt_rgba* C_DARK_SLATE_BLUE = (tt_rgba*) &Colors[136];
const tt_rgba* C_DARK_SLATE_GRAY = (tt_rgba*) &Colors[140];
const tt_rgba* C_DARK_TURQUOISE = (tt_rgba*) &Colors[144];
const tt_rgba* C_DARK_VIOLET = (tt_rgba*) &Colors[148];
const tt_rgba* C_DEEP_PINK = (tt_rgba*) &Colors[152];
const tt_rgba* C_DEEP_SKY_BLUE = (tt_rgba*) &Colors[156];
const tt_rgba* C_DIM_GRAY = (tt_rgba*) &Colors[160];
const tt_rgba* C_DODGER_BLUE = (tt_rgba*) &Colors[164];
const tt_rgba* C_FIREBRICK = (tt_rgba*) &Colors[168];
const tt_rgba* C_FLORAL_WHITE = (tt_rgba*) &Colors[172];
const tt_rgba* C_FOREST_GREEN = (tt_rgba*) &Colors[176];
const tt_rgba* C_FUCHSIA = (tt_rgba*) &Colors[180];
const tt_rgba* C_GAINSBORO = (tt_rgba*) &Colors[184];
const tt_rgba* C_GHOST_WHITE = (tt_rgba*) &Colors[188];
const tt_rgba* C_GOLD = (tt_rgba*) &Colors[192];
const tt_rgba* C_GOLDENROD = (tt_rgba*) &Colors[196];
const tt_rgba* C_GRAY = (tt_rgba*) &Colors[200];
const tt_rgba* C_GREEN = (tt_rgba*) &Colors[204];
const tt_rgba* C_GREEN_YELLOW = (tt_rgba*) &Colors[208];
const tt_rgba* C_HONEYDEW = (tt_rgba*) &Colors[212];
const tt_rgba* C_HOT_PINK = (tt_rgba*) &Colors[216];
const tt_rgba* C_INDIAN_RED = (tt_rgba*) &Colors[220];
const tt_rgba* C_INDIGO = (tt_rgba*) &Colors[224];
const tt_rgba* C_IVORY = (tt_rgba*) &Colors[228];
const tt_rgba* C_KHAKI = (tt_rgba*) &Colors[232];
const tt_rgba* C_LAVENDER = (tt_rgba*) &Colors[236];
const tt_rgba* C_LAVENDER_BUSH = (tt_rgba*) &Colors[240];
const tt_rgba* C_LAWN_GREEN = (tt_rgba*) &Colors[244];
const tt_rgba* C_LEMON_CHIFFON = (tt_rgba*) &Colors[248];
const tt_rgba* C_LIGHT_BLUE = (tt_rgba*) &Colors[252];
const tt_rgba* C_LIGHT_CORAL = (tt_rgba*) &Colors[256];
const tt_rgba* C_LIGHT_CYAN = (tt_rgba*) &Colors[260];
const tt_rgba* C_LIGHT_GOLDENROD_YELLOW = (tt_rgba*) &Colors[264];
const tt_rgba* C_LIGHT_GRAY = (tt_rgba*) &Colors[268];
const tt_rgba* C_LIGHT_GREEN = (tt_rgba*) &Colors[272];
const tt_rgba* C_LIGHT_PINK = (tt_rgba*) &Colors[276];
const tt_rgba* C_LIGHT_SALMON = (tt_rgba*) &Colors[280];
const tt_rgba* C_LIGHT_SEA_GREEN = (tt_rgba*) &Colors[284];
const tt_rgba* C_LIGHT_SKY_BLUE = (tt_rgba*) &Colors[288];
const tt_rgba* C_LIGHT_SLATE_GRAY = (tt_rgba*) &Colors[292];
const tt_rgba* C_LIGHT_STEEL_BLUE = (tt_rgba*) &Colors[296];
const tt_rgba* C_LIGHT_YELLOW = (tt_rgba*) &Colors[300];
const tt_rgba* C_LIME = (tt_rgba*) &Colors[304];
const tt_rgba* C_LIME_GREEN = (tt_rgba*) &Colors[308];
const tt_rgba* C_LINEN = (tt_rgba*) &Colors[312];
const tt_rgba* C_MAGENTA = (tt_rgba*) &Colors[316];
const tt_rgba* C_MAROON = (tt_rgba*) &Colors[320];
const tt_rgba* C_MEDIUM_AQUAMARINE = (tt_rgba*) &Colors[324];
const tt_rgba* C_MEDIUM_BLUE = (tt_rgba*) &Colors[328];
const tt_rgba* C_MEDIUM_ORCHID = (tt_rgba*) &Colors[332];
const tt_rgba* C_MEDIUM_PURPLE = (tt_rgba*) &Colors[336];
const tt_rgba* C_MEDIUM_SLATE_BLUE = (tt_rgba*) &Colors[340];
const tt_rgba* C_MEDIUM_SPRING_GREEN = (tt_rgba*) &Colors[344];
const tt_rgba* C_MEDIUM_TURQUOISE = (tt_rgba*) &Colors[348];
const tt_rgba* C_MEDIUM_VIOLET_RED = (tt_rgba*) &Colors[352];
const tt_rgba* C_MIDNIGHT_BLUE = (tt_rgba*) &Colors[356];
const tt_rgba* C_MINT_CREAM = (tt_rgba*) &Colors[360];
const tt_rgba* C_MISTY_ROSE = (tt_rgba*) &Colors[364];
const tt_rgba* C_MOCCASIN = (tt_rgba*) &Colors[368];
const tt_rgba* C_NAVAJO_WHITE = (tt_rgba*) &Colors[372];
const tt_rgba* C_NAVY = (tt_rgba*) &Colors[376];
const tt_rgba* C_OLD_LICE = (tt_rgba*) &Colors[380];
const tt_rgba* C_OLIVE = (tt_rgba*) &Colors[384];
const tt_rgba* C_OLIVE_DRAB = (tt_rgba*) &Colors[388];
const tt_rgba* C_ORANGE = (tt_rgba*) &Colors[392];
const tt_rgba* C_ORANGE_RED = (tt_rgba*) &Colors[396];
const tt_rgba* C_ORCHID = (tt_rgba*) &Colors[400];
const tt_rgba* C_PALE_GOLDENROD = (tt_rgba*) &Colors[404];
const tt_rgba* C_PALE_GREEN = (tt_rgba*) &Colors[408];
const tt_rgba* C_PALE_TURQUOISE = (tt_rgba*) &Colors[412];
const tt_rgba* C_PALE_VIOLET_RED = (tt_rgba*) &Colors[416];
const tt_rgba* C_PAPAYA_WHIP = (tt_rgba*) &Colors[420];
const tt_rgba* C_PEACH_PUFF = (tt_rgba*) &Colors[424];
const tt_rgba* C_PERU = (tt_rgba*) &Colors[428];
const tt_rgba* C_PINK = (tt_rgba*) &Colors[432];
const tt_rgba* C_PLUM = (tt_rgba*) &Colors[436];
const tt_rgba* C_POWDER_BLUE = (tt_rgba*) &Colors[440];
const tt_rgba* C_PURPLE = (tt_rgba*) &Colors[444];
const tt_rgba* C_RED = (tt_rgba*) &Colors[448];
const tt_rgba* C_ROSY_BROWN = (tt_rgba*) &Colors[452];
const tt_rgba* C_ROYAL_BLUE = (tt_rgba*) &Colors[456];
const tt_rgba* C_SADDLE_BROWN = (tt_rgba*) &Colors[460];
const tt_rgba* C_SALMON = (tt_rgba*) &Colors[464];
const tt_rgba* C_SANDY_BROWN = (tt_rgba*) &Colors[468];
const tt_rgba* C_SEA_GREEN = (tt_rgba*) &Colors[472];
const tt_rgba* C_SEA_SHELL = (tt_rgba*) &Colors[476];
const tt_rgba* C_SIENNA = (tt_rgba*) &Colors[480];
const tt_rgba* C_SILVER = (tt_rgba*) &Colors[484];
const tt_rgba* C_SKY_BLUE = (tt_rgba*) &Colors[488];
const tt_rgba* C_SLATE_BLUE = (tt_rgba*) &Colors[492];
const tt_rgba* C_SLATE_GRAY = (tt_rgba*) &Colors[496];
const tt_rgba* C_SNOW = (tt_rgba*) &Colors[500];
const tt_rgba* C_SPRING_GREEN = (tt_rgba*) &Colors[504];
const tt_rgba* C_STEEL_BLUE = (tt_rgba*) &Colors[508];
const tt_rgba* C_TAN = (tt_rgba*) &Colors[512];
const tt_rgba* C_TEAL = (tt_rgba*) &Colors[516];
const tt_rgba* C_THISTLE = (tt_rgba*) &Colors[520];
const tt_rgba* C_TOMATO = (tt_rgba*) &Colors[524];
const tt_rgba* C_TURQUOISE = (tt_rgba*) &Colors[528];
const tt_rgba* C_VIOLET = (tt_rgba*) &Colors[532];
const tt_rgba* C_WHEAT = (tt_rgba*) &Colors[536];
const tt_rgba* C_WHITE = (tt_rgba*) &Colors[540];
const tt_rgba* C_WHITE_SMOKE = (tt_rgba*) &Colors[544];
const tt_rgba* C_YELLOW = (tt_rgba*) &Colors[548];
const tt_rgba* C_YELLOW_GREEN = (tt_rgba*) &Colors[552];

//(PRIVATE)
//_begin >= 0 || _end <= 31, _begin < _end ->
unsigned int GetBits(unsigned int _number, int _begin, int _end)
{
    _number = _number << (31 - _end);
    _number = _number >> ((31 - _end) + _begin);
    return _number;
}

//(PRIVATE)
//_index >= 0 || _index <= 31 ->
void SetBit(unsigned int* _number, int _index, bool _value)
{
    if (_value)
    {
        *_number |= (1 << _index);
    }
    else
    {
        *_number &= ~(1 << _index);
    }
}

//(PRIVATE)
//SetBits_USHORT(x = 236, 4, 7, 0b1011) -> x = 188
void SetBits_USHORT(unsigned short* _number, int _begin, int _end, unsigned short _value)
{
    unsigned short left = (_end < 15 ? *_number >> (_end + 1) : 0);
    left <<= (_end < 15 ? _end + 1 : 0);
    unsigned short right = _begin > 0 ? *_number << (15 - (_begin - 1)) : 0;
    right >>= 15 - (_begin - 1);
    unsigned short value = _value << _begin;
    unsigned short result = right;
    result |= left;
    result |= value;
    *_number = result;
}

//(PRIVATE)
//SetBits_UINT(x = 236, 4, 7, 0b1011) -> x = 188
void SetBits_UINT(unsigned int* _number, int _begin, int _end, unsigned int _value)
{
    unsigned int left = (_end < 31 ? *_number >> (_end + 1) : 0);
    left <<= (_end < 31 ? _end + 1 : 0);
    unsigned int right = _begin > 0 ? *_number << (31 - (_begin - 1)) : 0;
    right >>= 31 - (_begin - 1);
    unsigned int value = _value << _begin;
    unsigned int result = right;
    result |= left;
    result |= value;
    *_number = result;
}

//(PRIVATE)
int RoundDown(double _value)
{
    if (_value >= 0.0)
    {
        return _value;
    }
    else
    {
        return _value - 1.0;
    }
}

//(PRIVATE)
int RoundDown_L(double _value)
{
    if (_value >= 0.0)
    {
        return _value;
    }
    else
    {
        return _value - 1.0;
    }
}

//(PRIVATE)
int RoundUp(double _value)
{
    if (_value >= 0.0)
    {
        return _value + 1;
    }
    else
    {
        return _value;
    }
}

//(PRIVATE)
//N1 == N2 => N2
double SmallerOf(double N1, double N2)
{
    return N1 < N2 ? N1 : N2;
}

//(PRIVATE)
//N1 == N2 => N2
double LargerOf(double N1, double N2)
{
    return N1 > N2 ? N1 : N2;
}

//(PRIVATE)
double AverageOf(double N1, double N2)
{
    double larger = N1 > N2 ? N1 : N2;
    double smaller =  N1 < N2 ? N1 : N2;
    return smaller + ((larger - smaller) / 2.0);
}

//(PRIVATE)
double Absolute(double _number)
{
    if (_number >= 0)
    {
        return _number;
    }
    else
    {
        return _number - _number - _number;
    }
}

//(PRIVATE)
double FractionOf(double N)
{
    return Absolute(N - RoundDown_L(N));
}

//(PRIVATE)
//it copies segment (from _source to _destination) (beginning at _sourceBegin) and (with length specified by _length)
void copy_short(const short* _source, short* _destination, int _destinationLength, int _sourceBegin,
                int _destinationBegin, int _copyLength)
{
    for (int n = 0; ; n++)
    {
        if (n == _copyLength)
        {
            break;
        }
        else if (_destinationBegin + n == _destinationLength)
        {
            break;
        }

        _destination[_destinationBegin + n] = _source[_sourceBegin + n];
    }
}

//(PRIVATE)
//it copies segment (from _source to _destination) (beginning at _sourceBegin) and (with length specified by _length)
void copy_uchar(const unsigned char* _source, unsigned char* _destination, int _destinationLength, int _sourceBegin,
                int _destinationBegin, int _copyLength)
{
    for (int n = 0; ; n++)
    {
        if (n == _copyLength)
        {
            break;
        }
        else if (_destinationBegin + n == _destinationLength)
        {
            break;
        }

        _destination[_destinationBegin + n] = _source[_sourceBegin + n];
    }
}

//(PRIVATE)
//_length specifies the length of _array
void swap_short(short* _array, int _i1, int _i2)
{
    short c = _array[_i1];
    _array[_i1] = _array[_i2];
    _array[_i2] = c;
}

//(PRIVATE)
//_length specifies the length of _array
void swap_uchar(unsigned char* _array, int _i1, int _i2)
{
    unsigned char c = _array[_i1];
    _array[_i1] = _array[_i2];
    _array[_i2] = c;
}

//(PRIVATE)
//_length specifies the length of _array
void reverse_short(short* _array, int _length)
{
    for (int i = 0; i < _length / 2; i ++)
    {
        swap_short(_array, i, (_length - i) - 1);
    }
}

//(PRIVATE)
//_length specifies the length of _array
void reverse_uchar(unsigned char* _array, int _length)
{
    for (int i = 0; i < _length / 2; i ++)
    {
        swap_uchar(_array, i, (_length - i) - 1);
    }
}

//(PRIVATE)
//_length specifies the length of _array
//_array has atleast one free element at the end ->
void insert(Contour* _array, int _length, Contour* _value, int _index)
{
    //moving of (the elements after _index) one position to the right
    for (int i = _length - 1; i > _index; i--)
    {
        _array[i] = _array[i - 1];
    }

    _array[_index] = *_value;
}

//(PRIVATE)
//(LOCAL-TO DrawCharacter)
tt_rgba TT_GetPixel(unsigned char* _data, int _canvasWidth, int _x, int _y)
{
    int offset = (_y * _canvasWidth + _x) * PIXEL_SIZE;
    tt_rgba color;
    color.B = _data[offset];
    color.G = _data[offset + 1];
    color.R = _data[offset + 2];
    color.A = _data[offset + 3];
    return color;
}

//(PRIVATE)
//(LOCAL-TO Move)
double DegreesToRadians(double _degrees)
{
    if (_degrees < 0.0)
    {
        return 0.0;
    }
    else
    {
        return  (PI * _degrees) / 180.0;
    }
}

//(PRIVATE)
//(LOCAL-TO OrientationOf)
double RadiansToDegrees(double _radians)
{
    if (_radians < 0.0)
    {
        return 0.0;
    }
    else
    {
        return (180.0 * _radians) / PI;
    }
}

//(PRIVATE)
//(LOCAL-TO IsFilledContour && DrawCharacter)
//this function is using Bottom-Left coordinates
//originX == pointX && originY == pointY => 0
//originX != pointX || originY != pointY => 0..360
double OrientationOf(double originX, double originY, double pointX, double pointY)
{
    if (originX == pointX && originY == pointY)
    {
        return 0.0;
    }
        ///special cases - 0, 90, 180 and 270 degrees
    else if (originX == pointX && originY < pointY)
    {
        return 0.0;
    }
    else if (originY == pointY && originX < pointX)
    {
        return 90.0;
    }
    else if (originX == pointX && originY > pointY)
    {
        return 180.0;
    }
    else if (originY == pointY && originX > pointX)
    {
        return 270.0;
    }
        //if _point is in first quadrant in the local coordinate system with origin the point _origin
    else if (originX < pointX && originY < pointY)
    {
        double opposite = pointY - originY;
        double adjacent = pointX - originX;
        return 90.0 - RadiansToDegrees(atan(opposite / adjacent));
    }
        //if _point is in second quadrant
    else if (originX < pointX && originY > pointY)
    {
        double opposite = originY - pointY;
        double adjacent = pointX - originX;
        return 90.0 + RadiansToDegrees(atan(opposite / adjacent));
    }
        //if _point is in third quadrant
    else if (originX > pointX && originY > pointY)
    {
        double opposite = originY - pointY;
        double adjacent = originX - pointX;
        return 270.0 - RadiansToDegrees(atan(opposite / adjacent));
    }
        //if _point is in fourth quadrant
    else if (originX > pointX && originY < pointY)
    {
        double opposite = pointY - originY;
        double adjacent = originX - pointX;
        return 270.0 + RadiansToDegrees(atan(opposite / adjacent));
    }
}

//(PRIVATE)
//(LOCAL-TO DrawCharacter)
//this function is using Bottom-Left coordinates
//_delta == _point => 0
//_delta != _point => 0..360
void Move(double* _deltaX, double* _deltaY, double _orientation, double _magnitude)
{
    double x = *_deltaX;
    double y = *_deltaY;

    if (_orientation == 0.0 || _orientation == 360.0)
    {
        *_deltaY = y + _magnitude;
    }
    else if (_orientation == 90.0)
    {
        *_deltaX = x + _magnitude;
    }
    else if (_orientation == 180.0)
    {
        *_deltaY = y - _magnitude;
    }
    else if (_orientation == 270.0)
    {
        *_deltaX = x - _magnitude;
    }
    else if (_orientation < 90.0)
    {
        double hypotenuse = _magnitude;
        double adjacent = cos(DegreesToRadians(_orientation)) * hypotenuse;
        double opposite = sin(DegreesToRadians(_orientation)) * hypotenuse;
        *_deltaX = x + opposite;
        *_deltaY = y + adjacent;
    }
    else if (_orientation > 90.0 && _orientation < 180.0)
    {
        double hypotenuse = _magnitude;
        double adjacent = cos(DegreesToRadians(_orientation - 90.0)) * hypotenuse;
        double opposite = sin(DegreesToRadians(_orientation - 90.0)) * hypotenuse;
        *_deltaX = x + adjacent;
        *_deltaY = y - opposite;
    }
    else if (_orientation > 180.0 && _orientation < 270.0)
    {
        double hypotenuse = _magnitude;
        double adjacent = cos(DegreesToRadians(_orientation - 180.0)) * hypotenuse;
        double opposite = sin(DegreesToRadians(_orientation - 180.0)) * hypotenuse;
        *_deltaX = x - opposite;
        *_deltaY = y - adjacent;
    }
    else if (_orientation > 270.0 && _orientation < 360.0)
    {
        double hypotenuse = _magnitude;
        double adjacent = cos(DegreesToRadians(_orientation - 270.0)) * hypotenuse;
        double opposite = sin(DegreesToRadians(_orientation - 270.0)) * hypotenuse;
        *_deltaX = x - adjacent;
        *_deltaY = y + opposite;
    }
}

//(PRIVATE)
//(LOCAL-TO DistanceOf(Bitex&, Bitex&))
bool AreEqual(double _a, double _b)
{
    double smaller = _a < _b ? _a : _b;
    double larger = _a > _b ? _a : _b;
    return (larger - smaller) <= 0.0001;
}

//(PRIVATE)
//(LOCAL-TO DrawCharacter)
double DistanceOf(double x1, double y1, double x2, double y2)
{
    if (x1 == x2 && y1 == y2) return 0.0;

    int smallerX = x1 < x2 ? x1 : x2;
    int smallerY = y1 < y2 ? y1 : y2;
    int largerX = x1 > x2 ? x1 : x2;
    int largerY = y1 > y2 ? y1 : y2;

    //if Xv is aligned with Xr
    if (y1 == y2)
    {
        return largerX - smallerX;
    }
        //if Yv is aligned with Yr
    else if (x1 == x2)
    {
        return largerY - smallerY;
    }
    else
    {
        //the distance is the hypotenuse of a right triangle - side1 and side2 are the adjacent and the opposite side
        //the Pythagorean theorem is used for finding the hypotenuse

        double side1 = largerX - smallerX;
        double side2 = largerY - smallerY;
        return sqrt((side1 * side1) + (side2 * side2));
    }
}

//(PRIVATE)
//(LOCAL-TO DrawCharacter fm:CoverageOf)
double DistanceOf_Bitex(const Bitex* b1, const Bitex* b2)
{
    if (AreEqual(b1->X, b2->X) && AreEqual(b1->Y, b2->Y)) return 0.0;

    //if Xv is aligned with Xr
    if (AreEqual(b1->Y, b2->Y))
    {
        return LargerOf(b1->X, b2->X) - SmallerOf(b1->X, b2->X);
    }
        //if Yv is aligned with Yr
    else if (AreEqual(b1->X, b2->X))
    {
        return LargerOf(b1->Y, b2->Y) - SmallerOf(b1->Y, b2->Y);
    }
    else
    {
        //the distance is the hypotenuse of a right triangle - side1 and side2 are the adjacent and the opposite side
        //Pythagorean theorem is used for finding the hypotenuse

        double side1 = LargerOf(b1->X, b2->X) - SmallerOf(b1->X, b2->X);
        double side2 = LargerOf(b1->Y, b2->Y) - SmallerOf(b1->Y, b2->Y);
        return sqrt((side1 * side1) + (side2 * side2));
    }
}

//(PRIVATE)
//(LOCAL-TO DrawCharacter)
Bitex CentexOf(double x1, double y1, double x2, double y2)
{
    double smallerX = SmallerOf(x1, x2);
    double smallerY = SmallerOf(y1, y2);
    double largerX = LargerOf(x1, x2);
    double largerY = LargerOf(y1, y2);
    double width = largerX - smallerX;
    double height = largerY - smallerY;
    Bitex bitex;
    bitex.X = smallerX + (width / 2.0);
    bitex.Y = smallerY + (height / 2.0);
    return bitex;
}

//(PRIVATE)
//(LOCAL-TO DrawCharacter)
//determines the coverage of a segmentonom (singular crossing)
unsigned int SegmentonomCoverage(const Bitex* _enteringSamplex, const Bitex* _exitingSamplex, const Bitex* _nextEnteringSamplex, bool _isFilledContour)
{
    //(DEBUG-BLOCK)
    //(SHOULD-NOT-HAPPEN)
    if (_enteringSamplex->X < 0.0 || _enteringSamplex->Y < 0.0 || _exitingSamplex->Y < 0.0 || _exitingSamplex->Y < 0.0)
    {
        int breakpoint = 0;
    }
    if ((_enteringSamplex->X > MetaCanvasWidth) || (_enteringSamplex->Y > MetaCanvasHeight) ||
        (_exitingSamplex->X > MetaCanvasWidth) || (_exitingSamplex->Y > MetaCanvasHeight))
    {
        int breakpoint = 0;
    }
    //(END-DEBUG-BLOCK)

    double localEnteringX = _enteringSamplex->X - RoundDown(_enteringSamplex->X);
    double localEnteringY = _enteringSamplex->Y - RoundDown(_enteringSamplex->Y);
    double localExitingX = _exitingSamplex->X - RoundDown(_exitingSamplex->X);
    double localExitingY = _exitingSamplex->Y - RoundDown(_exitingSamplex->Y);

    int currentPixelX = RoundDown(_enteringSamplex->X);
    int currentPixelY = RoundDown(_enteringSamplex->Y);
    int nextPixelX = RoundDown(_nextEnteringSamplex->X);
    int nextPixelY = RoundDown(_nextEnteringSamplex->Y);

    //L :: left edge of the pixel
    //R :: right edge of the pixel
    //T :: upper edge of the pixel
    //B :: lower edge of the pixel

    unsigned int coverage;
    int position = currentPixelY * MetaCanvasWidth + currentPixelX;
    unsigned int marker = MetaCanvas_S1[position];
    int crossingType = N_CROSSING;
    int middleX = (((AverageOf(localEnteringX, localExitingX)) * 100) / 1.35) + 1/*to not be 0*/;

    //if there is corner crossing
    if (_enteringSamplex == _exitingSamplex)
    {
        const Bitex* samplexA = _enteringSamplex;
        const Bitex* samplexB = _nextEnteringSamplex;

        //(L->T) crossing
        if (PreviousPixelX < currentPixelX && PreviousPixelY == currentPixelY && nextPixelY > currentPixelY)
        {
            coverage = 99.0;
            crossingType = _isFilledContour ? O_CROSSING : T_CROSSING;
            SetBit(&coverage, _isFilledContour ? O_LEFT : T_LEFT, true);
        }
            //(Т->L) crossing
        else if (PreviousPixelY > currentPixelY && nextPixelX < currentPixelX && nextPixelY == currentPixelY)
        {
            coverage = 1.0;
            crossingType = _isFilledContour ? T_CROSSING : O_CROSSING;
            SetBit(&coverage, _isFilledContour ? T_LEFT : O_LEFT, true);
        }
            //(L->B) crossing
        else if (PreviousPixelX < currentPixelX && PreviousPixelY == currentPixelY && nextPixelY < currentPixelY)
        {
            coverage = 1.0;
            crossingType = _isFilledContour ? T_CROSSING : O_CROSSING;
            SetBit(&coverage, _isFilledContour ? T_LEFT : O_LEFT, true);
        }
            //(B->L) crossing
        else if (PreviousPixelY < currentPixelY && nextPixelX < currentPixelX && nextPixelY == currentPixelY)
        {
            coverage = 99.0;
            crossingType = _isFilledContour ? O_CROSSING : T_CROSSING;
            SetBit(&coverage, _isFilledContour ? O_LEFT : T_LEFT, true);
        }
            //(T->R) crossing
        else if (PreviousPixelY > currentPixelY && nextPixelX > currentPixelX && nextPixelY == currentPixelY)
        {
            coverage = 99.0;
            crossingType = _isFilledContour ? T_CROSSING : O_CROSSING;
            SetBit(&coverage, _isFilledContour ? T_RIGHT : O_RIGHT, true);
        }
            //(R->T) crossing
        else if (PreviousPixelX > currentPixelX && PreviousPixelY == currentPixelY && nextPixelY > currentPixelY)
        {
            coverage = 1.0;
            crossingType = _isFilledContour ? O_CROSSING : T_CROSSING;
            SetBit(&coverage, _isFilledContour ? O_RIGHT : T_RIGHT, true);
        }
            //(B->R) crossing
        else if (PreviousPixelY < currentPixelY && nextPixelX > currentPixelX && nextPixelY == currentPixelY)
        {
            coverage = 1.0;
            crossingType = _isFilledContour ? O_CROSSING : T_CROSSING;
            SetBit(&coverage, _isFilledContour ? O_RIGHT : T_RIGHT, true);
        }
            //(R->B) crossing
        else if (PreviousPixelX > currentPixelX && PreviousPixelY == currentPixelY && nextPixelY < currentPixelY)
        {
            coverage = 99.0;
            crossingType = _isFilledContour ? T_CROSSING : O_CROSSING;
            SetBit(&coverage, _isFilledContour ? T_RIGHT : O_RIGHT, true);
        }
    }
        //(B->T crossing)
    else if (PreviousPixelY < currentPixelY && nextPixelY > currentPixelY)
    {
        double middleX = AverageOf(localEnteringX, localExitingX);
        double width = 1.0 - middleX;
        double height = 1.0;
        coverage = width * height * 100.0f;
        crossingType = _isFilledContour ? O_CROSSING : T_CROSSING;
    }
        //(T->B crossing)
    else if (PreviousPixelY > currentPixelY && nextPixelY < currentPixelY)
    {
        double middleX = AverageOf(localEnteringX, localExitingX);
        double width = middleX;
        double height = 1.0;
        coverage = width * height * 100.0f;
        crossingType = _isFilledContour ? T_CROSSING : O_CROSSING;
    }
        //(L->R crossing)
    else if (PreviousPixelX < currentPixelX && nextPixelX > currentPixelX && PreviousPixelY == currentPixelY && nextPixelY == currentPixelY)
    {
        double middleY = AverageOf(localEnteringY, localExitingY);
        double width = 1.0;
        double height = middleY;
        coverage = width * height * 100.0f;
    }
        //(R->L crossing)
    else if (PreviousPixelX > currentPixelX && nextPixelX < currentPixelX && PreviousPixelY == currentPixelY && nextPixelY == currentPixelY)
    {
        double middleY = AverageOf(localEnteringY, localExitingY);
        double width = 1.0;
        double height = 1.0 - middleY;
        coverage = width * height * 100.0f;
    }
        //(L->B crossing)
    else if (PreviousPixelX < currentPixelX && PreviousPixelY == currentPixelY && nextPixelY < currentPixelY)
    {
        double width = _exitingSamplex->X - _enteringSamplex->X;
        double height = _enteringSamplex->Y - _exitingSamplex->Y;
        coverage = (width * height * 100.0f) / 2.0f;
        crossingType = _isFilledContour ? T_CROSSING : O_CROSSING;
        SetBit(&coverage, _isFilledContour ? T_LEFT : O_LEFT, true);
    }
        //(L->T crossing)
    else if (PreviousPixelX < currentPixelX && PreviousPixelY == currentPixelY && nextPixelY > currentPixelY)
    {
        double width = _exitingSamplex->X - _enteringSamplex->X;
        double height = _exitingSamplex->Y - _enteringSamplex->Y;
        coverage = 100.0 - ((width * height * 100.0f) / 2.0f);
        crossingType = _isFilledContour ? O_CROSSING : T_CROSSING;
        SetBit(&coverage, _isFilledContour ? O_LEFT : T_LEFT, true);
    }
        //(R->B crossing)
    else if (PreviousPixelX > currentPixelX && PreviousPixelY == currentPixelY && nextPixelY < currentPixelY)
    {
        double width = _enteringSamplex->X - _exitingSamplex->X;
        double height = _enteringSamplex->Y - _exitingSamplex->Y;
        coverage = 100.0f - ((width * height * 100.0f) / 2.0f);
        crossingType = _isFilledContour ? T_CROSSING : O_CROSSING;
        SetBit(&coverage, _isFilledContour ? T_RIGHT : O_LEFT, true);
    }
        //(R->T crossing)
    else if (PreviousPixelX > currentPixelX && PreviousPixelY == currentPixelY && nextPixelY > currentPixelY)
    {
        double width = _enteringSamplex->X - _exitingSamplex->X;
        double height = _exitingSamplex->Y - _enteringSamplex->Y;
        coverage = (width * height * 100.0f) / 2.0f;
        crossingType = _isFilledContour ? O_CROSSING : T_CROSSING;
        SetBit(&coverage, _isFilledContour ? O_RIGHT : T_RIGHT, true);
    }
        //(B->L crossing)
    else if (PreviousPixelY < currentPixelY && nextPixelX < currentPixelX && nextPixelY == currentPixelY)
    {
        double width = _enteringSamplex->X - _exitingSamplex->X;
        double height = _exitingSamplex->Y - _enteringSamplex->Y;
        coverage = 100.0f - ((width * height * 100.0f) / 2.0f);
        crossingType = _isFilledContour ? O_CROSSING : T_CROSSING;
        SetBit(&coverage, _isFilledContour ? O_LEFT : T_LEFT, true);
    }
        //(B->R crossing)
    else if (PreviousPixelY < currentPixelY && nextPixelX > currentPixelX && nextPixelY == currentPixelY)
    {
        double width = _exitingSamplex->X - _enteringSamplex->X;
        double height = _exitingSamplex->Y - _enteringSamplex->Y;
        coverage = (width * height * 100.0f) / 2.0f;
        crossingType = _isFilledContour ? O_CROSSING : T_CROSSING;
        SetBit(&coverage, _isFilledContour ? O_RIGHT : T_RIGHT, true);
    }
        //(T->L crossing)
    else if (PreviousPixelY > currentPixelY && nextPixelX < currentPixelX && nextPixelY == currentPixelY)
    {
        double width = _enteringSamplex->X - _exitingSamplex->X;
        double height = _enteringSamplex->Y - _exitingSamplex->Y;
        coverage = (width * height * 100.0f) / 2.0f;
        crossingType = _isFilledContour ? T_CROSSING : O_CROSSING;
        SetBit(&coverage, _isFilledContour ? T_LEFT : O_LEFT, true);
    }
        //(T->R crossing)
    else if (PreviousPixelY > currentPixelY && nextPixelX > currentPixelX && nextPixelY == currentPixelY)
    {
        double width = _exitingSamplex->X - _enteringSamplex->X;
        double height = _enteringSamplex->Y - _exitingSamplex->Y;
        coverage = 100.0f - ((width * height * 100.0f) / 2.0f);
        crossingType = _isFilledContour ? T_CROSSING : O_CROSSING;
        SetBit(&coverage, _isFilledContour ? T_RIGHT : O_RIGHT, true);
    }
        //L->L (up|down)
    else if (PreviousPixelX < currentPixelX && PreviousPixelX == nextPixelX)
    {
        return marker;
    }
        //R->R (up|down)
    else if (PreviousPixelX > currentPixelX && PreviousPixelX == nextPixelX)
    {
        return marker;
    }
        //T->T (to left | to right)
    else if (PreviousPixelY > currentPixelY && PreviousPixelY == nextPixelY)
    {
        return marker;
    }
        //B->B (to left | to right)
    else if (PreviousPixelY < currentPixelY && PreviousPixelY == nextPixelY)
    {
        return marker;
    }
        //(SHOULD-NOT-HAPPEN)
    else
    {
        int breakpoint = 0;
    }

    if (GetBits(coverage, 0, 6) < 1)
    {
        SetBit(&coverage, 0, true);
    }

    if (crossingType == O_CROSSING && middleX > GetBits(marker, O_BEGIN, O_END))
    {
        SetBits_UINT(&coverage, O_BEGIN, O_END, middleX);
        SetBits_UINT(&coverage, T_BEGIN, T_END,  GetBits(marker, T_BEGIN, T_END));
    }
    else if (crossingType == T_CROSSING && middleX > GetBits(marker, T_BEGIN, T_END))
    {
        SetBits_UINT(&coverage, O_BEGIN, O_END, GetBits(marker, O_BEGIN, O_END));
        SetBits_UINT(&coverage, T_BEGIN, T_END, middleX);
    }
    else
    {
        SetBits_UINT(&coverage, O_BEGIN, O_END, GetBits(marker, O_BEGIN, O_END));
        SetBits_UINT(&coverage, T_BEGIN, T_END,  GetBits(marker, T_BEGIN, T_END));
        SetBits_UINT(&coverage, 22, 25,  GetBits(marker, 22, 25));
    }

    return coverage;
}

//(PRIVATE)
//(LOCAL-TO DrawCharacter)
//determines the coverage of a segmentoid (singular crossing)
unsigned int SegmentoidCoverage(const Bitex* _segmentoidVertex, const Bitex* _enteringSamplex, const Bitex* _exitingSamplex,
                                const Bitex* _nextEnteringSamplex, bool _isFilledContour)
{
    //(DEBUG)
    //(SHOULD-NOT-HAPPEN)
    if (_enteringSamplex == _exitingSamplex)
    {
        int breakpoint = 0;
    }
    else if (_enteringSamplex == _segmentoidVertex)
    {
        int breakpoint = 0;
    }
    else if (_exitingSamplex == _segmentoidVertex)
    {
        int breakpoint = 0;
    }
    if (_enteringSamplex->X < 0.0 || _enteringSamplex->Y < 0.0 || _exitingSamplex->Y < 0.0 || _exitingSamplex->Y < 0.0)
    {
        int breakpoint = 0;
    }
    if ((_enteringSamplex->X > MetaCanvasWidth) || (_enteringSamplex->Y > MetaCanvasHeight) ||
        (_exitingSamplex->X > MetaCanvasWidth) || (_exitingSamplex->Y > MetaCanvasHeight))
    {
        int breakpoint = 0;
    }
    //(END-DEBUG-BLOCK)

    Bitex upperLeftPixeloid;
    upperLeftPixeloid.X = RoundDown(_segmentoidVertex->X);
    upperLeftPixeloid.Y = RoundUp(_segmentoidVertex->Y);
    Bitex bottomLeftPixeloid;
    bottomLeftPixeloid.X = RoundDown(_segmentoidVertex->X);
    bottomLeftPixeloid.Y = RoundDown(_segmentoidVertex->Y);
    Bitex upperRightPixeloid;
    upperRightPixeloid.X = RoundUp(_segmentoidVertex->X);
    upperRightPixeloid.Y = RoundUp(_segmentoidVertex->Y);
    Bitex bottomRightPixeloid;
    bottomRightPixeloid.X = RoundUp(_segmentoidVertex->X);
    bottomRightPixeloid.Y = RoundDown(_segmentoidVertex->Y);

    double localEnteringX = _enteringSamplex->X - RoundDown(_enteringSamplex->X);
    double localExitingX = _exitingSamplex->X - RoundDown(_enteringSamplex->X);

    int currentPixelX = RoundDown(_enteringSamplex->X);
    int currentPixelY = RoundDown(_enteringSamplex->Y);
    int nextPixelX = RoundDown(_nextEnteringSamplex->X);
    int nextPixelY = RoundDown(_nextEnteringSamplex->Y);

    unsigned int coverage;
    int position = currentPixelY * MetaCanvasWidth + currentPixelX;
    unsigned int marker = MetaCanvas_S1[position];
    int crossingType = N_CROSSING;
    int middleX = (((AverageOf(localEnteringX, localExitingX)) * 100) / 1.35) + 1/*to not be 0*/;

    //(L->R crossing)
    if (PreviousPixelX < currentPixelX && nextPixelX > currentPixelX && PreviousPixelY == currentPixelY && nextPixelY == currentPixelY)
    {
        double T1_A = _enteringSamplex->Y - bottomLeftPixeloid.Y;
        double T1_B = DistanceOf_Bitex(_enteringSamplex, _segmentoidVertex);
        double T1_C = DistanceOf_Bitex(_segmentoidVertex, &bottomLeftPixeloid);
        double T1_semiperimeter = (T1_A + T1_B + T1_C) / 2.0;
        double T1_area = sqrt(T1_semiperimeter * (T1_semiperimeter - T1_A) * (T1_semiperimeter - T1_B) * (T1_semiperimeter - T1_C));

        if (isnan(T1_area)) T1_area = 0.0;

        double T2_A = T1_C;
        double T2_B = 1.0;
        double T2_C = DistanceOf_Bitex(_segmentoidVertex, &bottomRightPixeloid);
        double T2_semiperimeter = (T2_A + T2_B + T2_C) / 2.0;
        double T2_area = sqrt(T2_semiperimeter * (T2_semiperimeter - T2_A) * (T2_semiperimeter - T2_B) * (T2_semiperimeter - T2_C));

        if (isnan(T2_area)) T2_area = 0.0;

        double T3_A = T2_C;
        double T3_B = DistanceOf_Bitex(_segmentoidVertex, _exitingSamplex);
        double T3_C = _exitingSamplex->Y - bottomRightPixeloid.Y;
        double T3_semiperimeter = (T3_A + T3_B + T3_C) / 2.0;
        double T3_area = sqrt(T3_semiperimeter * (T3_semiperimeter - T3_A) * (T3_semiperimeter - T3_B) * (T3_semiperimeter - T3_C));

        if (isnan(T3_area)) T3_area = 0.0;

        coverage = (T1_area + T2_area + T3_area) * 100.0;
    }
        //(R->L crossing)
    else if (PreviousPixelX > currentPixelX && nextPixelX < currentPixelX && PreviousPixelY == currentPixelY && nextPixelY == currentPixelY)
    {
        double T1_A = _enteringSamplex->Y - bottomRightPixeloid.Y;
        double T1_B = DistanceOf_Bitex(_enteringSamplex, _segmentoidVertex);
        double T1_C = DistanceOf_Bitex(_segmentoidVertex, &bottomRightPixeloid);
        double T1_semiperimeter = (T1_A + T1_B + T1_C) / 2.0;
        double T1_area = sqrt(T1_semiperimeter * (T1_semiperimeter - T1_A) * (T1_semiperimeter - T1_B) * (T1_semiperimeter - T1_C));

        if (isnan(T1_area)) T1_area = 0.0;

        double T2_A = T1_C;
        double T2_B = 1.0;
        double T2_C = DistanceOf_Bitex(_segmentoidVertex, &bottomLeftPixeloid);
        double T2_semiperimeter = (T2_A + T2_B + T2_C) / 2.0;
        double T2_area = sqrt(T2_semiperimeter * (T2_semiperimeter - T2_A) * (T2_semiperimeter - T2_B) * (T2_semiperimeter - T2_C));

        if (isnan(T2_area)) T2_area = 0.0;

        double T3_A = T2_C;
        double T3_B = _exitingSamplex->Y - bottomLeftPixeloid.Y;
        double T3_C = DistanceOf_Bitex(_exitingSamplex, _segmentoidVertex);
        double T3_semiperimeter = (T3_A + T3_B + T3_C) / 2.0;
        double T3_area = sqrt(T3_semiperimeter * (T3_semiperimeter - T3_A) * (T3_semiperimeter - T3_B) * (T3_semiperimeter - T3_C));

        if (isnan(T3_area)) T3_area = 0.0;

        coverage = (1.0 - (T1_area + T2_area + T3_area)) * 100.0;
    }
        //(B->T crossing)
    else if (PreviousPixelY < currentPixelY && nextPixelY > currentPixelY)
    {
        double T1_A = DistanceOf_Bitex(_enteringSamplex, _segmentoidVertex);
        double T1_B = bottomRightPixeloid.X - _enteringSamplex->X;
        double T1_C = DistanceOf_Bitex(_segmentoidVertex, &bottomRightPixeloid);
        double T1_semiperimeter = (T1_A + T1_B + T1_C) / 2.0;
        double T1_area = sqrt(T1_semiperimeter * (T1_semiperimeter - T1_A) * (T1_semiperimeter - T1_B) * (T1_semiperimeter - T1_C));

        if (isnan(T1_area)) T1_area = 0.0;

        double T2_A = T1_C;
        double T2_B = 1.0;
        double T2_C = DistanceOf_Bitex(&upperRightPixeloid, _segmentoidVertex);
        double T2_semiperimeter = (T2_A + T2_B + T2_C) / 2.0;
        double T2_area = sqrt(T2_semiperimeter * (T2_semiperimeter - T2_A) * (T2_semiperimeter - T2_B) * (T2_semiperimeter - T2_C));

        if (isnan(T2_area)) T2_area = 0.0;

        double T3_A = T2_C;
        double T3_B = DistanceOf_Bitex(_segmentoidVertex, _exitingSamplex);
        double T3_C = upperRightPixeloid.X - _exitingSamplex->X;
        double T3_semiperimeter = (T3_A + T3_B + T3_C) / 2.0;
        double T3_area = sqrt(T3_semiperimeter * (T3_semiperimeter - T3_A) * (T3_semiperimeter - T3_B) * (T3_semiperimeter - T3_C));

        if (isnan(T3_area)) T3_area = 0.0;

        coverage = (T1_area + T2_area + T3_area) * 100.0;

        crossingType = _isFilledContour ? O_CROSSING : T_CROSSING;
    }
        //(T->B crossing)
    else if (PreviousPixelY > currentPixelY && nextPixelY < currentPixelY)
    {
        double T1_A = DistanceOf_Bitex(_segmentoidVertex, _enteringSamplex);
        double T1_B = _enteringSamplex->X - upperLeftPixeloid.X;
        double T1_C = DistanceOf_Bitex(&upperLeftPixeloid, _segmentoidVertex);
        double T1_semiperimeter = (T1_A + T1_B + T1_C) / 2.0;
        double T1_area = sqrt(T1_semiperimeter * (T1_semiperimeter - T1_A) * (T1_semiperimeter - T1_B) * (T1_semiperimeter - T1_C));

        if (isnan(T1_area)) T1_area = 0.0;

        double T2_A = T1_C;
        double T2_B = DistanceOf_Bitex(_segmentoidVertex, &bottomLeftPixeloid);
        double T2_C = 1.0;
        double T2_semiperimeter = (T2_A + T2_B + T2_C) / 2.0;
        double T2_area = sqrt(T2_semiperimeter * (T2_semiperimeter - T2_A) * (T2_semiperimeter - T2_B) * (T2_semiperimeter - T2_C));

        if (isnan(T2_area)) T2_area = 0.0;

        double T3_A = T2_B;
        double T3_B =  _exitingSamplex->X - bottomLeftPixeloid.X;
        double T3_C = DistanceOf_Bitex(_exitingSamplex, _segmentoidVertex);
        double T3_semiperimeter = (T3_A + T3_B + T3_C) / 2.0;
        double T3_area = sqrt(T3_semiperimeter * (T3_semiperimeter - T3_A) * (T3_semiperimeter - T3_B) * (T3_semiperimeter - T3_C));

        if (isnan(T3_area)) T3_area = 0.0;

        coverage = (T1_area + T2_area + T3_area) * 100.0;

        crossingType = _isFilledContour ? T_CROSSING : O_CROSSING;
    }
        //(L->T crossing)
    else if (PreviousPixelX < currentPixelX && PreviousPixelY == currentPixelY && nextPixelY > currentPixelY)
    {
        double T1_A = upperLeftPixeloid.Y - _enteringSamplex->Y;
        double T1_B = DistanceOf_Bitex(_enteringSamplex, _segmentoidVertex);
        double T1_C = DistanceOf_Bitex(_segmentoidVertex, &upperLeftPixeloid);
        double T1_semiperimeter = (T1_A + T1_B + T1_C) / 2.0;
        double T1_area = sqrt(T1_semiperimeter * (T1_semiperimeter - T1_A) * (T1_semiperimeter - T1_B) * (T1_semiperimeter - T1_C));

        if (isnan(T1_area)) T1_area = 0.0;

        double T2_A = T1_C;
        double T2_B = DistanceOf_Bitex(_segmentoidVertex, _exitingSamplex);
        double T2_C = _exitingSamplex->X - upperLeftPixeloid.X;
        double T2_semiperimeter = (T2_A + T2_B + T2_C) / 2.0;
        double T2_area = sqrt(T2_semiperimeter * (T2_semiperimeter - T2_A) * (T2_semiperimeter - T2_B) * (T2_semiperimeter - T2_C));

        if (isnan(T2_area)) T2_area = 0.0;

        coverage = 100 - ((T1_area + T2_area) * 100.0);

        crossingType = _isFilledContour ? O_CROSSING : T_CROSSING;

        SetBit(&coverage, _isFilledContour ? O_LEFT : T_LEFT, true);
    }
        //(T->L crossing)
    else if (PreviousPixelY > currentPixelY && nextPixelX < currentPixelX && nextPixelY == currentPixelY)
    {
        double T1_A = upperLeftPixeloid.Y - _exitingSamplex->Y;
        double T1_B = DistanceOf_Bitex(_exitingSamplex, _segmentoidVertex);
        double T1_C = DistanceOf_Bitex(_segmentoidVertex, &upperLeftPixeloid);
        double T1_semiperimeter = (T1_A + T1_B + T1_C) / 2.0;
        double T1_area = sqrt(T1_semiperimeter * (T1_semiperimeter - T1_A) * (T1_semiperimeter - T1_B) * (T1_semiperimeter - T1_C));

        if (isnan(T1_area)) T1_area = 0.0;

        double T2_A = T1_C;
        double T2_B = _enteringSamplex->X - upperLeftPixeloid.X;
        double T2_C = DistanceOf_Bitex(_enteringSamplex, _segmentoidVertex);
        double T2_semiperimeter = (T2_A + T2_B + T2_C) / 2.0;
        double T2_area = sqrt(T2_semiperimeter * (T2_semiperimeter - T2_A) * (T2_semiperimeter - T2_B) * (T2_semiperimeter - T2_C));

        if (isnan(T2_area)) T2_area = 0.0;

        coverage = (T1_area + T2_area) * 100.0;

        crossingType = _isFilledContour ? T_CROSSING : O_CROSSING;

        SetBit(&coverage, _isFilledContour ? T_LEFT : O_LEFT, true);
    }
        //(L->B crossing)
    else if (PreviousPixelX < currentPixelX && PreviousPixelY == currentPixelY && nextPixelY < currentPixelY)
    {
        double T1_A = _enteringSamplex->Y - bottomLeftPixeloid.Y;
        double T1_B = DistanceOf_Bitex(_enteringSamplex, _segmentoidVertex);
        double T1_C = DistanceOf_Bitex(_segmentoidVertex, &bottomLeftPixeloid);
        double T1_semiperimeter = (T1_A + T1_B + T1_C) / 2.0;
        double T1_area = sqrt(T1_semiperimeter * (T1_semiperimeter - T1_A) * (T1_semiperimeter - T1_B) * (T1_semiperimeter - T1_C));

        if (isnan(T1_area)) T1_area = 0.0;

        double T2_A = T1_C;
        double T2_B = DistanceOf_Bitex(_segmentoidVertex, _exitingSamplex);
        double T2_C = _exitingSamplex->X - bottomLeftPixeloid.X;
        double T2_semiperimeter = (T2_A + T2_B + T2_C) / 2.0;
        double T2_area = sqrt(T2_semiperimeter * (T2_semiperimeter - T2_A) * (T2_semiperimeter - T2_B) * (T2_semiperimeter - T2_C));

        if (isnan(T2_area)) T2_area = 0.0;

        coverage = (T1_area + T2_area) * 100.0;

        crossingType = _isFilledContour ? T_CROSSING : O_CROSSING;

        SetBit(&coverage, _isFilledContour ? T_LEFT : O_LEFT, true);
    }
        //(B->L crossing)
    else if (PreviousPixelY < currentPixelY && nextPixelX < currentPixelX && nextPixelY == currentPixelY)
    {
        double T1_A = DistanceOf_Bitex(_enteringSamplex, _segmentoidVertex);
        double T1_B = DistanceOf_Bitex(_segmentoidVertex, &bottomLeftPixeloid);
        double T1_C = _enteringSamplex->X - bottomLeftPixeloid.X;
        double T1_semiperimeter = (T1_A + T1_B + T1_C) / 2.0;
        double T1_area = sqrt(T1_semiperimeter * (T1_semiperimeter - T1_A) * (T1_semiperimeter - T1_B) * (T1_semiperimeter - T1_C));

        if (isnan(T1_area)) T1_area = 0.0;

        double T2_A = T1_B;
        double T2_B = DistanceOf_Bitex(_segmentoidVertex, _exitingSamplex);
        double T2_C = _exitingSamplex->Y - bottomLeftPixeloid.Y;
        double T2_semiperimeter = (T2_A + T2_B + T2_C) / 2.0;
        double T2_area = sqrt(T2_semiperimeter * (T2_semiperimeter - T2_A) * (T2_semiperimeter - T2_B) * (T2_semiperimeter - T2_C));

        if (isnan(T2_area)) T2_area = 0.0;

        coverage = 100.0 - ((T1_area + T2_area) * 100.0);

        crossingType = _isFilledContour ? O_CROSSING : T_CROSSING;

        SetBit(&coverage, _isFilledContour ? O_LEFT : T_LEFT, true);
    }
        //(R->T crossing)
    else if (PreviousPixelX > currentPixelX && PreviousPixelY == currentPixelY && nextPixelY > currentPixelY)
    {
        double T1_A = DistanceOf_Bitex(_enteringSamplex, _segmentoidVertex);
        double T1_B = DistanceOf_Bitex(_segmentoidVertex, _exitingSamplex);
        double T1_C = DistanceOf_Bitex(_exitingSamplex, _enteringSamplex);
        double T1_semiperimeter = (T1_A + T1_B + T1_C) / 2.0;
        double T1_area = sqrt(T1_semiperimeter * (T1_semiperimeter - T1_A) * (T1_semiperimeter - T1_B) * (T1_semiperimeter - T1_C));

        if (isnan(T1_area)) T1_area = 0.0;

        double T2_A = T1_C;
        double T2_B = upperRightPixeloid.Y - _enteringSamplex->Y;
        double T2_C = upperRightPixeloid.X - _exitingSamplex->X;
        double T2_semiperimeter = (T2_A + T2_B + T2_C) / 2.0;
        double T2_area = sqrt(T2_semiperimeter * (T2_semiperimeter - T2_A) * (T2_semiperimeter - T2_B) * (T2_semiperimeter - T2_C));

        if (isnan(T2_area)) T2_area = 0.0;

        coverage = (T1_area + T2_area) * 100.0;

        crossingType = _isFilledContour ? O_CROSSING : T_CROSSING;

        SetBit(&coverage, _isFilledContour ? O_RIGHT : O_RIGHT, true);
    }
        //(T->R crossing)
    else if (PreviousPixelY > currentPixelY && nextPixelX > currentPixelX && nextPixelY == currentPixelY)
    {
        double T1_A = upperRightPixeloid.X - _enteringSamplex->X;
        double T1_B = DistanceOf_Bitex(_enteringSamplex, _segmentoidVertex);
        double T1_C = DistanceOf_Bitex(_segmentoidVertex, &upperRightPixeloid);
        double T1_semiperimeter = (T1_A + T1_B + T1_C) / 2.0;
        double T1_area = sqrt(T1_semiperimeter * (T1_semiperimeter - T1_A) * (T1_semiperimeter - T1_B) * (T1_semiperimeter - T1_C));

        if (isnan(T1_area)) T1_area = 0.0;

        double T2_A = T1_C;
        double T2_B = upperRightPixeloid.Y - _exitingSamplex->Y;
        double T2_C = DistanceOf_Bitex(_exitingSamplex, _segmentoidVertex);
        double T2_semiperimeter = (T2_A + T2_B + T2_C) / 2.0;
        double T2_area = sqrt(T2_semiperimeter * (T2_semiperimeter - T2_A) * (T2_semiperimeter - T2_B) * (T2_semiperimeter - T2_C));

        if (isnan(T2_area)) T2_area = 0.0;

        coverage = 100.0 - ((T1_area + T2_area) * 100.0);

        crossingType = _isFilledContour ? T_CROSSING : O_CROSSING;

        SetBit(&coverage, _isFilledContour ? T_RIGHT : T_LEFT, true);
    }
        //(R->B crossing)
    else if (PreviousPixelX > currentPixelX && PreviousPixelY == currentPixelY && nextPixelY < currentPixelY)
    {
        double T1_A = _enteringSamplex->Y - bottomRightPixeloid.Y;
        double T1_B = DistanceOf_Bitex(_enteringSamplex, _segmentoidVertex);
        double T1_C = DistanceOf_Bitex(_segmentoidVertex, &bottomRightPixeloid);
        double T1_semiperimeter = (T1_A + T1_B + T1_C) / 2.0;
        double T1_area = sqrt(T1_semiperimeter * (T1_semiperimeter - T1_A) * (T1_semiperimeter - T1_B) * (T1_semiperimeter - T1_C));

        if (isnan(T1_area)) T1_area = 0.0;

        double T2_A = T1_C;
        double T2_B = bottomRightPixeloid.X - _exitingSamplex->X;
        double T2_C = DistanceOf_Bitex(_exitingSamplex, _segmentoidVertex);
        double T2_semiperimeter = (T2_A + T2_B + T2_C) / 2.0;
        double T2_area = sqrt(T2_semiperimeter * (T2_semiperimeter - T2_A) * (T2_semiperimeter - T2_B) * (T2_semiperimeter - T2_C));

        if (isnan(T2_area)) T2_area = 0.0;

        coverage = 100.0 - ((T1_area + T2_area) * 100.0);

        crossingType = _isFilledContour ? T_CROSSING : O_CROSSING;

        SetBit(&coverage, _isFilledContour ? T_RIGHT : O_RIGHT, true);
    }
        //(B->R crossing)
    else if (PreviousPixelY < currentPixelY && nextPixelX > currentPixelX && nextPixelY == currentPixelY)
    {
        double T1_A = DistanceOf_Bitex(_enteringSamplex, _segmentoidVertex);
        double T1_B = DistanceOf_Bitex(_segmentoidVertex, _exitingSamplex);
        double T1_C = DistanceOf_Bitex(_exitingSamplex, _enteringSamplex);
        double T1_semiperimeter = (T1_A + T1_B + T1_C) / 2.0;
        double T1_area = sqrt(T1_semiperimeter * (T1_semiperimeter - T1_A) * (T1_semiperimeter - T1_B) * (T1_semiperimeter - T1_C));

        if (isnan(T1_area)) T1_area = 0.0;

        double T2_A = T1_C;
        double T2_B = _exitingSamplex->Y - bottomRightPixeloid.Y;
        double T2_C = bottomRightPixeloid.X - _enteringSamplex->X;
        double T2_semiperimeter = (T2_A + T2_B + T2_C) / 2.0;
        double T2_area = sqrt(T2_semiperimeter * (T2_semiperimeter - T2_A) * (T2_semiperimeter - T2_B) * (T2_semiperimeter - T2_C));

        if (isnan(T2_area)) T2_area = 0.0;

        coverage = (T1_area + T2_area) * 100.0;

        crossingType = _isFilledContour ? O_CROSSING : T_CROSSING;

        SetBit(&coverage, _isFilledContour ? O_RIGHT : T_RIGHT, true);
    }
        //L->L (up|down)
    else if (PreviousPixelX < currentPixelX && PreviousPixelX == nextPixelX)
    {
        //(L->L (down)
        if (_enteringSamplex->Y > _exitingSamplex->Y)
        {
            double T_A = DistanceOf_Bitex(_enteringSamplex, _segmentoidVertex);
            double T_B = DistanceOf_Bitex(_segmentoidVertex, _exitingSamplex);
            double T_C = DistanceOf_Bitex(_exitingSamplex, _enteringSamplex);
            double T_semiperimeter = (T_A + T_B + T_C) / 2.0;
            double T_area = sqrt(T_semiperimeter * (T_semiperimeter - T_A) * (T_semiperimeter - T_B) * (T_semiperimeter - T_C));

            if (isnan(T_area)) T_area = 0.0;

            coverage = T_area * 100.0;

            crossingType = _isFilledContour ? T_CROSSING : O_CROSSING;
            SetBit(&coverage, _isFilledContour ? T_LEFT : O_LEFT, true);
        }
            //(L->L (up)
        else
        {
            double T_A = DistanceOf_Bitex(_enteringSamplex, _segmentoidVertex);
            double T_B = DistanceOf_Bitex(_segmentoidVertex, _exitingSamplex);
            double T_C = DistanceOf_Bitex(_exitingSamplex, _enteringSamplex);
            double T_semiperimeter = (T_A + T_B + T_C) / 2.0;
            double T_area = sqrt(T_semiperimeter * (T_semiperimeter - T_A) * (T_semiperimeter - T_B) * (T_semiperimeter - T_C));

            if (isnan(T_area)) T_area = 0.0;

            coverage = 100.0 - (T_area * 100.0);

            crossingType = _isFilledContour ? O_CROSSING : T_CROSSING;
            SetBit(&coverage, _isFilledContour ? O_LEFT : T_LEFT, true);
        }
    }
        //R->R (up|down)
    else if (PreviousPixelX > currentPixelX && PreviousPixelX == nextPixelX)
    {
        //R->R (down)
        if (_enteringSamplex->Y > _exitingSamplex->Y)
        {
            double T_A = DistanceOf_Bitex(_enteringSamplex, _segmentoidVertex);
            double T_B = DistanceOf_Bitex(_segmentoidVertex, _exitingSamplex);
            double T_C = DistanceOf_Bitex(_exitingSamplex, _enteringSamplex);
            double T_semiperimeter = (T_A + T_B + T_C) / 2.0;
            double T_area = sqrt(T_semiperimeter * (T_semiperimeter - T_A) * (T_semiperimeter - T_B) * (T_semiperimeter - T_C));

            if (isnan(T_area)) T_area = 0.0;

            coverage = 100.0 - (T_area * 100.0);

            SetBit(&coverage, _isFilledContour ? T_RIGHT : O_RIGHT, true);
        }
            //R->R (up)
        else
        {
            double T_A = DistanceOf_Bitex(_enteringSamplex, _segmentoidVertex);
            double T_B = DistanceOf_Bitex(_segmentoidVertex, _exitingSamplex);
            double T_C = DistanceOf_Bitex(_exitingSamplex, _enteringSamplex);
            double T_semiperimeter = (T_A + T_B + T_C) / 2.0;
            double T_area = sqrt(T_semiperimeter * (T_semiperimeter - T_A) * (T_semiperimeter - T_B) * (T_semiperimeter - T_C));

            if (isnan(T_area)) T_area = 0.0;

            coverage = T_area * 100.0;
        }
    }
        //T->T (to left | to right)
    else if (PreviousPixelY > currentPixelY && PreviousPixelY == nextPixelY)
    {
        //T->T (to left)
        if (_enteringSamplex->X > _exitingSamplex->X)
        {
            double T_A = DistanceOf_Bitex(_enteringSamplex, _segmentoidVertex);
            double T_B = DistanceOf_Bitex(_segmentoidVertex, _exitingSamplex);
            double T_C = DistanceOf_Bitex(_exitingSamplex, _enteringSamplex);
            double T_semiperimeter = (T_A + T_B + T_C) / 2.0;
            double T_area = sqrt(T_semiperimeter * (T_semiperimeter - T_A) * (T_semiperimeter - T_B) * (T_semiperimeter - T_C));

            if (isnan(T_area)) T_area = 0.0;

            coverage = T_area * 100.0;
        }
            //Т->Т (to right)
        else
        {
            double T_A = DistanceOf_Bitex(_enteringSamplex, _segmentoidVertex);
            double T_B = DistanceOf_Bitex(_segmentoidVertex, _exitingSamplex);
            double T_C = DistanceOf_Bitex(_exitingSamplex, _enteringSamplex);
            double T_semiperimeter = (T_A + T_B + T_C) / 2.0;
            double T_area = sqrt(T_semiperimeter * (T_semiperimeter - T_A) * (T_semiperimeter - T_B) * (T_semiperimeter - T_C));

            if (isnan(T_area)) T_area = 0.0;

            coverage = 100.0 - (T_area * 100.0);
        }
    }
        //B->B (to left | to right)
    else if (PreviousPixelY < currentPixelY && PreviousPixelY == nextPixelY)
    {
        //B->B (to left)
        if (_enteringSamplex->X > _exitingSamplex->X)
        {
            double T_A = DistanceOf_Bitex(_enteringSamplex, _segmentoidVertex);
            double T_B = DistanceOf_Bitex(_segmentoidVertex, _exitingSamplex);
            double T_C = DistanceOf_Bitex(_exitingSamplex, _enteringSamplex);
            double T_semiperimeter = (T_A + T_B + T_C) / 2.0;
            double T_area = sqrt(T_semiperimeter * (T_semiperimeter - T_A) * (T_semiperimeter - T_B) * (T_semiperimeter - T_C));

            if (isnan(T_area)) T_area = 0.0;

            coverage = 100.0 - (T_area * 100.0);
        }
            //B->B (to right)
        else
        {
            double T_A = DistanceOf_Bitex(_enteringSamplex, _segmentoidVertex);
            double T_B = DistanceOf_Bitex(_segmentoidVertex, _exitingSamplex);
            double T_C = DistanceOf_Bitex(_exitingSamplex, _enteringSamplex);
            double T_semiperimeter = (T_A + T_B + T_C) / 2.0;
            double T_area = sqrt(T_semiperimeter * (T_semiperimeter - T_A) * (T_semiperimeter - T_B) * (T_semiperimeter - T_C));

            if (isnan(T_area)) T_area = 0.0;

            coverage = T_area * 100.0;
        }
    }
        //(SHOULD-NOT-HAPPEN)
    else
    {
        int breakpoint = 0;
    }

    if (GetBits(coverage, 0, 6) < 1)
    {
        SetBit(&coverage, 0, true);
    }

    if (crossingType == O_CROSSING && middleX > GetBits(marker, O_BEGIN, O_END))
    {
        SetBits_UINT(&coverage, O_BEGIN, O_END, middleX);
        SetBits_UINT(&coverage, T_BEGIN, T_END,  GetBits(marker, T_BEGIN, T_END));
    }
    else if (crossingType == T_CROSSING && middleX > GetBits(marker, T_BEGIN, T_END))
    {
        SetBits_UINT(&coverage, O_BEGIN, O_END, GetBits(marker, O_BEGIN, O_END));
        SetBits_UINT(&coverage, T_BEGIN, T_END, middleX);
    }
    else
    {
        SetBits_UINT(&coverage, O_BEGIN, O_END, GetBits(marker, O_BEGIN, O_END));
        SetBits_UINT(&coverage, T_BEGIN, T_END,  GetBits(marker, T_BEGIN, T_END));
        SetBits_UINT(&coverage, 22, 25,  GetBits(marker, 22, 25));
    }

    return coverage;
}

//(PRIVATE)
//(LOCAL-TO DrawCharacter)
//determines the coverage of multi-crossed segmentonom or segmentoid (this is the +1 crossing)
unsigned int MulticrossCoverage(unsigned int _oldMarker, unsigned int _newMarker)
{
    int oldCoverage = GetBits(_oldMarker, 0, 6);
    int newCoverage = GetBits(_newMarker, 0, 6);

    unsigned int realCoverage;

    if (oldCoverage + newCoverage < 100)
    {
        realCoverage = oldCoverage + newCoverage;
    }
    else
    {
        realCoverage = 100 - ((100 - oldCoverage) + (100 - newCoverage));
    }

    if (realCoverage == 0)
    {
        realCoverage = 1;
    }

    SetBits_UINT(&_oldMarker, 0, 6, realCoverage);
    SetBits_UINT(&_oldMarker, O_BEGIN, O_END, GetBits(_newMarker, O_BEGIN, O_END));
    SetBits_UINT(&_oldMarker, T_BEGIN, T_END, GetBits(_newMarker, T_BEGIN, T_END));

    if (GetBit(_newMarker, O_LEFT))
    {
        SetBit(&_oldMarker, O_LEFT, true);
    }

    if (GetBit(_newMarker, T_LEFT))
    {
        SetBit(&_oldMarker, T_LEFT, true);
    }

    if (GetBit(_newMarker, O_RIGHT))
    {
        SetBit(&_oldMarker, O_RIGHT, true);
    }

    if (GetBit(_newMarker, T_RIGHT))
    {
        SetBit(&_oldMarker, T_RIGHT, true);
    }

    return _oldMarker;
}

//(PRIVATE)
//(LOCAL-TO DrawCharacter)
//(SOURCE) https://stackoverflow.com/questions/1165647/how-to-determine-if-a-list-of-polygon-points-are-in-clockwise-order?noredirect=1&lq=1
bool IsFilledContour(const short* _x_coordinates, const short* _y_coordinates, int _pointCount)
{
    int signedArea = 0;

    for (int i = 0; i < _pointCount; i++)
    {
        int x1 = _x_coordinates[i];
        int y1 = _y_coordinates[i];
        int x2;
        int y2;

        if (i == _pointCount - 1)
        {
            x2 = _x_coordinates[0];
            y2 = _y_coordinates[0];
        }
        else
        {
            x2 = _x_coordinates[i + 1];
            y2 = _y_coordinates[i + 1];
        }

        signedArea += x1 * y2 - x2 * y1;
    }

    return signedArea < 0;
}

//(PRIVATE)
//(LOCAL-TO DrawCharacter)
/* returns the length of quadratic Bezier curve; the computed length can be relatively precise, even extremely precise (0.0001%) if the curve
   isn't too curved, but in case that is too curved, the calculation will be less precise - but even in that case the error is no more than 1%;
   in reality glyphs with such curves are either probably very rare in reality or does not exist at all */
double LengthOfBezierCurve(double _beginPointX, double _beginPointY, double _controlPointX, double _controlPointY, double _endPointX, double _endPointY)
{
    double length = 0.0;
    double previousPointX = _beginPointX;
    double previousPointY = _beginPointY;

    double a = _controlPointX - _beginPointX;
    double b = _controlPointY - _beginPointY;
    double d = _endPointX - _controlPointX;
    double e = _endPointY - _controlPointY;

    for (double percentage = 0.0; percentage <= 100.0; percentage += 0.5/*200 steps*/)
    {
        double p1_C_Interpolation_X = _beginPointX + ((a / 100.0) * percentage);
        double p1_C_Interpolation_Y = _beginPointY + ((b / 100.0) * percentage);
        double C_p2_Interpolation_X = _controlPointX + ((d / 100.0) * percentage);
        double C_p2_Interpolation_Y = _controlPointY + ((e / 100.0) * percentage);
        double x = p1_C_Interpolation_X + (((C_p2_Interpolation_X - p1_C_Interpolation_X) / 100.0) * percentage);
        double y = p1_C_Interpolation_Y + (((C_p2_Interpolation_Y - p1_C_Interpolation_Y) / 100.0) * percentage);

        double smallerX = previousPointX < x ? previousPointX : x;
        double largerX = previousPointX > x ? previousPointX : x;
        double smallerY = previousPointY < y ? previousPointY : y;
        double largerY = previousPointY > y ? previousPointY : y;
        double side1 = largerX - smallerX;
        double side2 = largerY - smallerY;
        length += sqrt((side1 * side1) + (side2 * side2));

        previousPointX = x;
        previousPointY = y;
    }

    return length;
}

//_fontSize is specified in pixels
double GetScale(const Font* _font, double _fontSize)
{
    HEAD_Table* head = (HEAD_Table*) GetTable(_font, HEAD_TABLE);
    return _fontSize / (double)head->UnitsPerEm;
}

tt_Rectangle GetEnclosingRectangle(const Contour* _contour)
{
    int minX = -1;
    int minY = -1;
    int maxX = -1;
    int maxY = -1;

    for (int i = 0; i < _contour->NumberOfPoints; i++)
    {
        int x = _contour->X_Coordinates[i];
        int y = _contour->Y_Coordinates[i];

        if (minX == -1)
        {
            minX = x;
            maxX = x;
        }
        else if (x < minX)
        {
            minX = x;
        }
        else if (x > maxX)
        {
            maxX = x;
        }

        if (minY == -1)
        {
            minY = y;
            maxY = y;
        }
        else if (y < minY)
        {
            minY = y;
        }
        else if (y > maxY)
        {
            maxY = y;
        }
    }

    tt_Rectangle rectangle;
    rectangle.X = minX;
    rectangle.Y = minY;
    rectangle.Width = (maxX - minX) + 1;
    rectangle.Height = (maxY - minY) + 1;
    return rectangle;
}

//(PRIVATE)
//(LOCAL-TO DrawCharacter)
unsigned char GetColorComponent(unsigned char _backgroundColorComponent, unsigned char _foregroundColorComponent, int _transparency)
{
    if (_backgroundColorComponent == _foregroundColorComponent)
    {
        return _backgroundColorComponent;
    }

    int larger = _backgroundColorComponent > _foregroundColorComponent ? _backgroundColorComponent : _foregroundColorComponent;
    int smaller = _backgroundColorComponent < _foregroundColorComponent ? _backgroundColorComponent : _foregroundColorComponent;

    unsigned char difference = larger - smaller;

    if (_foregroundColorComponent > _backgroundColorComponent)
    {
        return _foregroundColorComponent - ((difference / 100.0) * _transparency);
    }
    else
    {
        return _foregroundColorComponent + ((difference / 100.0) * _transparency);
    }
}

//(PUBLIC)
/* _characterIndex is a Unicode codepoint if it's a positive value, and glyph index (within the given font file) if it's a negative value;
  the function is non-validating - if _characterIndex is a Unicode codepoint, then it must be a valid Unicode codepoint and if
  _characterIndex is a glyph index, then it must be an index within the valid for the specific font range */
//_glyph is a Parser::SimpleGlyph or Parser::CompositeGlyph object; if this parameter is used, then _characterIndex is ignored
//_canvas is (a RGBA or BGRA pixel array) in which the character is drawn
//Y_Direction specifies the direction in which the Y-coordinates grow (top-to-bottom or bottom-up)
//_colorComponentOrder specifies if the pixels in _canvas are RGBA or BGRA
//_canvasWidth and _canvasHeight are the width and height of the canvas(i.e. _canvas) specified in pixels
//_horizontalPosition specifies the position (in pixels) of the left border of the EM-square; it can be negative or positive value
//_verticalPosition specifies the position (in pixels) of the baseline in the canvas; it can be negative or positive value
//_fontSize is the height of the line (not the actual character) in pixels
//_numberOfColors should be equal (or larger) to the number of elements in _colors
//_transparency = 0 means fully opaque string, and 100 means fully transparent string
/*_maxGraphemicX specifies a limiting X coordinate in the canvas (not an X coordinate in the string itself) - i.e. the part of the
   character after this coordinate will not be visualized; a value of -1 specifies that there is no horizontal limit;
   this coordinate is inclusive, i.e. the column matching the coordinate will also be visualized */
/* (!!!) this is a non-validating function; the font must contain the glyph that is represented by the specified _characterIndex
         value (if set) and the parameters must have correct values */
void DrawCharacter(
        int _characterIndex,
        void* _glyph,
        const Font* _font,
        unsigned char* _canvas,
        ColorComponentOrder _colorComponentOrder,
        int _canvasWidth,
        int _canvasHeight,
        double _horizontalPosition,
        double _verticalPosition,
        double _fontSize,
        GlyphColorizationMode _colorizationMode,
        const tt_rgba* _colors,
        int _numberOfColors,
        int _transparency,
        int _maxGraphemicX,
        double _composite_X_Offset, //(INTERNAL)
        double _composite_Y_Offset, //(INTERNAL)
        double _composite_X_Scale, //(INTERNAL)
        double _composite_Y_Scale) //(INTERNAL)
{
    double SCALE = GetScale(_font, _fontSize);

    void* glyph;

    //if the function receives a glyph, and not (an Unicode codepoint) or (glyph index)
    if (_glyph != NULL)
    {
        glyph = _glyph;
    }
        //ако &_characterIndex is an Unicode codepoint
    else if (_characterIndex > 0)
    {
        glyph = GetGlyph(_font, _characterIndex);
    }
        //(STATE) _characterIndex is a glyph index (in the table 'glyf')
    else
    {
        GLYF_Table* glyf = (GLYF_Table*) GetTable(_font, GLYF_TABLE);
        glyph = glyf->Glyphs[0 - _characterIndex];
    }

    ///IF THE GLYPH IS EMPTY (NON-CONTOUR GLYPH)
    if (Is(glyph, EMPTY_GLYPH))
    {
        return;
    }
        ///IF THE GLYPH IS SIMPLE
    else if (Is(glyph, SIMPLE_GLYPH))
    {
        SimpleGlyph* glyph_ = (SimpleGlyph*) glyph;

        int lsb;

        if (_glyph != NULL)
        {
            lsb = glyph_->MinX; //(C)
        }
        else
        {
            lsb = GetLeftSideBearing(_font, _characterIndex);
        }

        ///CONTOUR REORDERING

        int numberOfContours = glyph_->NumberOfContours;
        int numberOfRealContours = 0; //(E) it's needed because there are contours with one point
        Contour* unorderedContours = malloc(sizeof(Contour) * numberOfContours);
        Contour* orderedContours = malloc(sizeof(Contour) * numberOfContours);

        //for every contour
        for (int contourIndex = 0, nonEmptyContourCount = 0; contourIndex < numberOfContours; contourIndex++)
        {
            int numberOfPoints = contourIndex > 0 ? glyph_->EndPointsOfContours[contourIndex] - glyph_->EndPointsOfContours[contourIndex - 1] : glyph_->EndPointsOfContours[0] + 1;

            /* (E) it's possible that a contour contains only one point;
               (SOURCE) https://github.com/MicrosoftDocs/typography-issues/issues/720?) */
            if (numberOfPoints == 1)
            {
                continue;
            }

            int indexOfFirstPoint = contourIndex > 0 ? glyph_->EndPointsOfContours[contourIndex - 1] + 1 : 0;

            Contour* contour = &unorderedContours[nonEmptyContourCount++];
            contour->X_Coordinates = malloc(sizeof(short) * numberOfPoints);
            contour->Y_Coordinates = malloc(sizeof(short) * numberOfPoints);
            contour->Flags = malloc(sizeof(unsigned char) * numberOfPoints);
            copy_short(glyph_->X_Coordinates, contour->X_Coordinates, numberOfPoints, indexOfFirstPoint, 0, numberOfPoints);
            copy_short(glyph_->Y_Coordinates, contour->Y_Coordinates, numberOfPoints, indexOfFirstPoint, 0, numberOfPoints);
            copy_uchar(glyph_->Flags, contour->Flags, numberOfPoints, indexOfFirstPoint, 0, numberOfPoints);
            contour->NumberOfPoints = numberOfPoints;
            contour->OriginalIndex = contourIndex;
            contour->IsFilled = IsFilledContour(contour->X_Coordinates, contour->Y_Coordinates, numberOfPoints);

            //(POSSIBLE-CASE)
            if (numberOfContours == 1 && !contour->IsFilled)
            {
                contour->IsFilled = true;
                reverse_short(contour->X_Coordinates, numberOfPoints);
                reverse_short(contour->Y_Coordinates, numberOfPoints);
                reverse_uchar(contour->Flags, numberOfPoints);
            }

            //value -1.0 for _composite_X_Scale is used with horizontally-mirrored characters
            //value -1.0 for _composite_Y_Scale is used with vertically-mirrored characters
            if (_composite_X_Scale != 0.0 || _composite_Y_Scale != 0.0)
            {
                for (int i = 0; i < numberOfPoints; i++)
                {
                    contour->X_Coordinates[i] = (_composite_X_Scale < 0 ? _composite_X_Offset : 0) + (contour->X_Coordinates[i] * _composite_X_Scale);
                    contour->Y_Coordinates[i] = (_composite_Y_Scale < 0 ? _composite_Y_Offset : 0) + (contour->Y_Coordinates[i] * _composite_Y_Scale);
                }

                if ((_composite_X_Scale < 0.0 && _composite_Y_Scale >= 0.0) || (_composite_X_Scale >= 0.0 && _composite_Y_Scale < 0.0))
                {
                    reverse_short(contour->X_Coordinates, numberOfPoints);
                    reverse_short(contour->Y_Coordinates, numberOfPoints);
                    reverse_uchar(contour->Flags, numberOfPoints);
                }
            }

            numberOfRealContours++;
        }

        numberOfContours = numberOfRealContours;

        int F_Contours_Count = 0;

        for (int i = 0; i < numberOfContours; i++)
        {
            if (unorderedContours[i].IsFilled)
            {
                F_Contours_Count++;
            }
        }

        int N_Contours_Count = numberOfContours - F_Contours_Count;
        int orderedContoursCount = 0;

        if (F_Contours_Count == 1 && N_Contours_Count >= 0)
        {
            for (int i = 0; i < numberOfContours; i++)
            {
                if (unorderedContours[i].IsFilled)
                {
                    orderedContours[0] = unorderedContours[i];
                    break;
                }
            }

            for (int i = 0, n = 1; i < numberOfContours; i++)
            {
                if (!unorderedContours[i].IsFilled)
                {
                    orderedContours[n++] = unorderedContours[i];
                }
            }
        }
        else if (F_Contours_Count > 1 && N_Contours_Count == 0)
        {
            orderedContours = unorderedContours;
        }
            //(STATE) F_Contours_Count >= 2 && N_Contours_Count >= 2; a reordering of the contours must be performed
        else
        {
            //the first element of the pair is a contour, and the second element is his 'most-direct' container contour
            ContourPair* contourPairs = malloc(sizeof(ContourPair) * numberOfContours);

            for (int i = 0; i < numberOfContours; i++)
            {
                contourPairs[i].Contour = NULL;
                contourPairs[i].DirectContainer = NULL;
            }

            //determine the closest enclosing rectangles for every contour
            for (int i = 0; i < numberOfContours; i++)
            {
                //determine the the most-direct container contour for &__contour

                Contour* contour = &unorderedContours[i];
                tt_Rectangle rectangle = GetEnclosingRectangle(contour);
                int directArea = -1;
                Contour* directContainer = NULL;

                for (int n = 0; n < numberOfContours; n++)
                {
                    Contour* contour_ = &unorderedContours[n];

                    if (contour != contour_)
                    {
                        tt_Rectangle rectangle_ = GetEnclosingRectangle(contour_);

                        if (RectangleContainsRectangle(&rectangle_, &rectangle))
                        {
                            if (directContainer == NULL)
                            {
                                directContainer = contour_;
                                directArea = rectangle_.Width * rectangle_.Height;
                            }
                            else
                            {
                                int area_ = rectangle_.Width * rectangle_.Height;

                                if (area_ < directArea)
                                {
                                    directArea = area_;
                                    directContainer = contour_;
                                }
                            }
                        }
                    }
                }

                contourPairs[i].Contour = contour;
                contourPairs[i].DirectContainer = directContainer;
            }

            //adding the (contours with no containers) to &orderedContours

            for (int i = 0; i < numberOfContours; i++)
            {
                ContourPair* contourPair = &contourPairs[i];

                if (contourPair->DirectContainer == NULL)
                {
                    orderedContours[orderedContoursCount++] = *((Contour*) contourPair->Contour);
                }
            }

            //inserting the (contours which have containers) after the corresponding container contour

            for (int i = 0; i < numberOfContours; i++)
            {
                ContourPair* pair = &contourPairs[i];

                if (pair->DirectContainer != NULL)
                {
                    int containerIndex = -1;

                    for (int n = 0; n < orderedContoursCount; n++)
                    {
                        if (orderedContours[n].OriginalIndex == ((Contour*) pair->DirectContainer)->OriginalIndex)
                        {
                            containerIndex = n;
                        }
                    }

                    for (int i = 0; i < orderedContoursCount; i++)
                    {
                        if (orderedContours->OriginalIndex == ((Contour*) pair->DirectContainer)->OriginalIndex)
                        {
                            containerIndex = i;
                            break;
                        }
                    }

                    if (containerIndex != -1 && containerIndex < orderedContoursCount - 1)
                    {
                        insert(orderedContours, numberOfContours, pair->Contour, containerIndex + 1);
                    }
                    else
                    {
                        orderedContours[orderedContoursCount] = *((Contour*) pair->Contour);
                    }

                    orderedContoursCount++;
                }
            }

            free(contourPairs);
        }

        Bitex enteringSamplex;
        enteringSamplex.X = 0.0;
        enteringSamplex.Y = 0.0; //the entering samplex for a pixel

        Bitex implicitPoint;

        ///

        //(D) modifying the coordinates, so that the coordinate arrays won't have any negative values

        /* the lowest values are determined by taking in consideration the contours in &orderedContours, and not
           glyph_->Contours, as glyph_->Contours contains the original coordinates, which are potentially modified at this point
           because of _composite_X_Scale/_composite_Y_Scale */

        int lowestX = INT_MAX;
        int lowestY = INT_MAX;
        int highestX = INT_MIN;
        int highestY = INT_MIN;

        for (int contourIndex = 0; contourIndex < numberOfContours; contourIndex++)
        {
            Contour* contour = &orderedContours[contourIndex];

            for (int pointIndex = 0; pointIndex < contour->NumberOfPoints; pointIndex++)
            {
                int x_point = contour->X_Coordinates[pointIndex];
                int y_point = contour->Y_Coordinates[pointIndex];

                if (x_point < lowestX)
                {
                    lowestX = x_point;
                }

                if (y_point < lowestY)
                {
                    lowestY = y_point;
                }

                if (x_point > highestX)
                {
                    highestX = x_point;
                }

                if (y_point > highestY)
                {
                    highestY = y_point;
                }
            }
        }

        if (lsb < 0)
        {
            _horizontalPosition += lsb * SCALE;
        }
        else
        {
            _horizontalPosition += lowestX * SCALE;
        }

        _verticalPosition += lowestY * SCALE;

        double fx_shift = _horizontalPosition - RoundDown(_horizontalPosition);
        double fy_shift = _verticalPosition - RoundDown(_verticalPosition);
        /* (C) (t:SimpleGlyph : MinX, MinY, MaxX, MaxY) cannot be used here as there are errors (it seems) in some fonts - for example
           yMin in (DejaVuSans index 3013) does not correspond to the real lowest Y value */
        MetaCanvasWidth = RoundUp(((highestX - lowestX) * SCALE)) + 1;
        MetaCanvasHeight = RoundUp((highestY - lowestY) * SCALE) + 1;
        int size = MetaCanvasWidth * MetaCanvasHeight;
        MetaCanvas_S1 = malloc(sizeof(unsigned int) * size);
        MetaCanvas_S2 = malloc(sizeof(unsigned short) * size);
        memset((void*) MetaCanvas_S1, 0, size * sizeof(unsigned int));
        memset((void*) MetaCanvas_S2, 0, size * sizeof(unsigned short));

        //for every contour
        for (int contourIndex = 0; contourIndex < numberOfContours; contourIndex++)
        {
            PreviousPixelX = -1;
            PreviousPixelY = -1;

            Contour* contour = &orderedContours[contourIndex];
            int numberOfPoints = contour->NumberOfPoints;

            short* x_coordinates = contour->X_Coordinates;
            short* y_coordinates = contour->Y_Coordinates;
            unsigned char* flags = contour->Flags;

            //(D)
            for (int i = 0; i < numberOfPoints; i++)
            {
                x_coordinates[i] -= lowestX;
                y_coordinates[i] -= lowestY;
            }

            //(->)

            /* (A) the coverage of the begin/end pixel P is determined at the end of the contour iteration, as the entering samplex
                   is not known at the moment of exiting P; this variable stores the exiting samplex for the pixel, that will be
                   used later in combination with the already determined entering samplex */

            Bitex beginPixelEnteringSamplex;
            beginPixelEnteringSamplex.X = -1.0;
            beginPixelEnteringSamplex.Y = -1.0;
            Bitex beginPixelExitingSamplex;
            beginPixelExitingSamplex.X = -1.0;
            beginPixelExitingSamplex.Y = -1.0;
            Bitex beginPixelNextSamplex;
            beginPixelNextSamplex.X = -1.0;
            beginPixelNextSamplex.Y = -1.0;

            Bitex endSegmentoid;

            int currentPixelMinX;
            int currentPixelMaxX;
            int currentPixelMinY;
            int currentPixelMaxY;

            double deltaX;
            double deltaY;
            double oldDeltaX;
            double oldDeltaY;

            //(L)

            int beginIndex = INT_MAX;
            int endIndex = INT_MAX;

            //if the first point of the contour is OFF (such cases are very rare, but they exist)
            if (GetBit(flags[0], 0) == false)
            {
                /* as it is not certain whether the last point of the contour is ON, the position of the first ON point
                   in the contour must be determined (i.e. the first point of the next segment); the (point before the first ON point) is
                   the end point of the contour */
                for (int i = 0; i < numberOfPoints - 2; i++)
                {
                    if (GetBit(flags[i], 0) == true)
                    {
                        beginIndex = i;
                        endIndex = i - 1;
                        break;
                    }
                }

                /* there are cases in which the contour consists of only OFF points - this is a valid TrueType contour;
                   in such cases a median OFF point is inserted between every OFF-point pair; after the modification
                   the first point in the array is OFF, the second is ON, the third is OFF, fourth is ON and so on */
                if (beginIndex == INT_MAX)
                {
                    short* extended_x_coordinates = malloc(sizeof(short) * numberOfPoints * 2);
                    short* extended_y_coordinates = malloc(sizeof(short) * numberOfPoints * 2);
                    unsigned char* extendedFlags = malloc(sizeof(unsigned char) * numberOfPoints * 2);

                    //copying the coordinates and OFF points into the extended arrays
                    for (int s = 0, t = 0; s < numberOfPoints; s++, t += 2)
                    {
                        extended_x_coordinates[t] = x_coordinates[s];
                        extended_y_coordinates[t] = y_coordinates[s];
                        extendedFlags[t] = flags[s];
                    }

                    //generating implicit ON points (plus flags for them) into the extended arrays
                    for (int i = 1; i < numberOfPoints * 2; i += 2)
                    {
                        short previousX = extended_x_coordinates[i - 1];
                        short previousY = extended_y_coordinates[i - 1];

                        short nextX = extended_x_coordinates[i < (numberOfPoints * 2) - 1 ? i + 1 : 0];
                        short nextY = extended_y_coordinates[i < (numberOfPoints * 2) - 1 ? i + 1 : 0];

                        Bitex implicitPoint = CentexOf(previousX, previousY, nextX, nextY);

                        extended_x_coordinates[i] = implicitPoint.X;
                        extended_y_coordinates[i] = implicitPoint.Y;
                        extendedFlags[i] = 1 /* only the first bit is important - to set the point as ON */;
                    }

                    numberOfPoints *= 2;

                    free(x_coordinates);
                    free(y_coordinates);
                    free(flags);
                    x_coordinates = extended_x_coordinates;
                    y_coordinates = extended_y_coordinates;
                    flags = extendedFlags;

                    beginIndex = 1;
                    endIndex = 0;
                }
            }
            else
            {
                beginIndex = 0;
                endIndex = numberOfPoints - 1;
            }

            int beginContourPixelX = (x_coordinates[beginIndex] * SCALE) + fx_shift;
            int beginContourPixelY = (y_coordinates[beginIndex] * SCALE) + fy_shift;

            ///for every contour point
            for (int contourPointIndex = beginIndex; ; contourPointIndex++)
            {
                ///(L)

                if (endIndex < numberOfPoints - 1 && contourPointIndex == numberOfPoints)
                {
                    contourPointIndex = 0;
                }

                int nextContourPointIndex;

                if (contourPointIndex == numberOfPoints - 1)
                {
                    nextContourPointIndex = 0;
                }
                else
                {
                    nextContourPointIndex = contourPointIndex + 1;
                }

                ///

                short pointX = x_coordinates[contourPointIndex];
                short pointY = y_coordinates[contourPointIndex];

                short nextPointX = x_coordinates[nextContourPointIndex];
                short nextPointY = y_coordinates[nextContourPointIndex];

                unsigned char flags_ = flags[contourPointIndex];
                unsigned char nextFlags = flags[nextContourPointIndex];

                //(->)

                double scaledPointX = (pointX * SCALE) + fx_shift;
                double scaledPointY = (pointY * SCALE) + fy_shift;
                double scaledNextPointX = (nextPointX * SCALE) + fx_shift;
                double scaledNextPointY = (nextPointY * SCALE) + fy_shift;

                bool currentPointIsON = GetBit(flags_, 0) == true;
                bool nextPointIsON = GetBit(nextFlags, 0) == true;

                ///ON, ON :: LINE
                if (currentPointIsON && nextPointIsON)
                {
                    //writing the begin and end points of the current segment

                    double baseStep = 0.005; /* smaller distance between two semplices means more precise calculation of the
                coverage, but the smaller distance ofcourse also means that more semplices will be calculated
                for each pixel and therefore that will reflect in lower performance */

                    /* (B) check whether a (horizontal or vertical shift) of (the begin and end vertices) is needed;
                       this shift is needed in some cases because of the fundamental errors in the calculations with the type 'double'
                       that could generate wave-like/zig-zag movement of the delta point; if the begin and end vertices form
                       a horizontal or vertical line and they are very close to the pixel edge, it's possible that this wave-like
                       movement of the delta-point could cause multiple crossings between the current pixel and the closest (relative to
                       the delta point) pixel, and this will result in incorrect calculation of the coverage */

                    double scaledXFraction = FractionOf(scaledPointX);
                    double scaledYFraction = FractionOf(scaledPointY);

                    //if shifting the begin vertexoid to the right is needed
                    if (scaledXFraction <= VERTEXOID_SHIFT)
                    {
                        scaledPointX += VERTEXOID_SHIFT;
                    }
                        //if shifting the begin vertexoid to the left is needed
                    else if (scaledXFraction >= 0.99)
                    {
                        scaledPointX -= VERTEXOID_SHIFT;
                    }

                    //if shifting the begin vertexoid upwards is needed
                    if (scaledYFraction <= VERTEXOID_SHIFT)
                    {
                        scaledPointY += VERTEXOID_SHIFT;
                    }
                        //if shifting the begin vertexoid downwards is needed
                    else if (scaledYFraction >= 0.99)
                    {
                        scaledPointY -= VERTEXOID_SHIFT;
                    }

                    ///

                    double scaledNextXFraction = FractionOf(scaledNextPointX);
                    double scaledNextYFraction = FractionOf(scaledNextPointY);

                    //if shifting the end vertexoid to the right is needed
                    if (scaledNextXFraction <= VERTEXOID_SHIFT)
                    {
                        scaledNextPointX += VERTEXOID_SHIFT;
                    }
                        //if shifting the end vertexoid to the left is needed
                    else if (scaledNextXFraction >= 0.99)
                    {
                        scaledNextPointX -= VERTEXOID_SHIFT;
                    }

                    //if shifting the begin vertexoid upwards is needed
                    if (scaledNextYFraction <= VERTEXOID_SHIFT)
                    {
                        scaledNextPointY += VERTEXOID_SHIFT;
                    }
                        //if shifting the begin vertexoid downwards is needed
                    else if (scaledNextYFraction >= 0.99)
                    {
                        scaledNextPointY -= VERTEXOID_SHIFT;
                    }

                    //

                    double beginSegmentPixelX = RoundDown(scaledPointX);
                    double beginSegmentPixelY = RoundDown(scaledPointY);
                    double endSegmentPixelX = RoundDown(scaledNextPointX);
                    double endSegmentPixelY = RoundDown(scaledNextPointY);

                    endSegmentoid.X = scaledNextPointX;
                    endSegmentoid.Y = scaledNextPointY;

                    double lineLength = DistanceOf(scaledPointX, scaledPointY, scaledNextPointX, scaledNextPointY);

                    double LINE_ORIENTATION = OrientationOf(scaledPointX, scaledPointY, scaledNextPointX, scaledNextPointY);
                    currentPixelMinX = RoundDown(scaledPointX);
                    currentPixelMaxX = RoundUp(scaledPointX);
                    currentPixelMinY = RoundDown(scaledPointY);
                    currentPixelMaxY = RoundUp(scaledPointY);

                    deltaX = scaledPointX;
                    deltaY = scaledPointY;

                    while (true)
                    {
                        double distance = DistanceOf(scaledPointX, scaledPointY, deltaX, deltaY);

                        if (distance >= lineLength)
                        {
                            break;
                        }

                        oldDeltaX = deltaX;
                        oldDeltaY = deltaY;

                        double delta_x = deltaX;
                        double delta_y = deltaY;

                        Move(&delta_x, &delta_y, LINE_ORIENTATION, baseStep * 10);

                        //if there are 10 or less steps until crossing another pixel
                        if (delta_x < currentPixelMinX || delta_x >= currentPixelMaxX || delta_y < currentPixelMinY || delta_y >= currentPixelMaxY)
                        {
                            Move(&deltaX, &deltaY, LINE_ORIENTATION, baseStep);
                        }
                            /* (STATE) there are more than 10 steps until crossing another pixel, and there are more than 10 steps
                                    until the end of the line */
                        else
                        {
                            deltaX = delta_x;
                            deltaY = delta_y;
                        }

                        //if delta reaches the next pixel
                        if (deltaX < currentPixelMinX || deltaX >= currentPixelMaxX || deltaY < currentPixelMinY || deltaY >= currentPixelMaxY)
                        {
                            //(A) if the 'delta' pixel is the begin pixel of the contour
                            if (RoundDown(deltaX) == beginContourPixelX && RoundDown(deltaY) == beginContourPixelY)
                            {
                                beginPixelEnteringSamplex.X = deltaX;
                                beginPixelEnteringSamplex.Y = deltaY;
                            }

                            //if the current pixel is the begin pixel of the segment
                            if (currentPixelMinX == beginSegmentPixelX && currentPixelMinY == beginSegmentPixelY)
                            {
                                //(A) if the current pixel is the begin pixel of the contour
                                if (beginPixelExitingSamplex.X == -1)
                                {
                                    beginPixelExitingSamplex.X = oldDeltaX;
                                    beginPixelExitingSamplex.Y = oldDeltaY;
                                    beginPixelNextSamplex.X = deltaX;
                                    beginPixelNextSamplex.Y = deltaY;

                                    //this pixel must be marked as conturoid

                                    int position = currentPixelMinY * MetaCanvasWidth + currentPixelMinX;

                                    if (MetaCanvas_S1[position] == 0)
                                    {
                                        int value = INITIAL_PIXEL_MARKER; /* a value that is not 0; when the end of the contour is reached it will be replaced
                               with the real value */
                                        MetaCanvas_S1[position] = value;
                                    }
                                }
                                    //(STATE) the current pixel is the first for the segment, but not the first for the contour
                                else
                                {
                                    Bitex segmentoidVertex;
                                    segmentoidVertex.X = scaledPointX;
                                    segmentoidVertex.Y = scaledPointY;

                                    Bitex exitingSamplex;
                                    exitingSamplex.X = oldDeltaX;
                                    exitingSamplex.Y = oldDeltaY;

                                    Bitex nextSamplex;
                                    nextSamplex.X = deltaX;
                                    nextSamplex.Y = deltaY;

                                    unsigned int coverage = SegmentoidCoverage(&segmentoidVertex, &enteringSamplex,
                                                                               &exitingSamplex, &nextSamplex, contour->IsFilled);

                                    int position = currentPixelMinY * MetaCanvasWidth + currentPixelMinX;

                                    unsigned int marker = MetaCanvas_S1[position];

                                    //if the segmentoid is already crossed once (i.e. this is a +1 crossing)
                                    if (marker != 0 && marker != INITIAL_PIXEL_MARKER)
                                    {
                                        MetaCanvas_S1[position] = MulticrossCoverage(marker, coverage);
                                    }
                                        //(STATE) this is the first crossing of the segmentoid
                                    else
                                    {
                                        MetaCanvas_S1[position] = coverage;
                                    }
                                }

                                enteringSamplex.X = deltaX;
                                enteringSamplex.Y = deltaY;
                                PreviousPixelX = currentPixelMinX;
                                PreviousPixelY = currentPixelMinY; //(->)
                                currentPixelMinX = RoundDown(deltaX);
                                currentPixelMaxX = RoundUp(deltaX);
                                currentPixelMinY = RoundDown(deltaY);
                                currentPixelMaxY = RoundUp(deltaY);
                            }
                                //(STATE) the pixel is a segmentonom
                            else if (currentPixelMinX != endSegmentPixelX || currentPixelMinY != endSegmentPixelY)
                            {
                                int position = currentPixelMinY * MetaCanvasWidth + currentPixelMinX;

                                unsigned int marker_ = MetaCanvas_S1[position];

                                /* it's possible that the entering and the exiting semplices are equal - this can happen if a pixel is 'missed',
                                   i.e. if there is corner crossing */

                                Bitex exitingSamplex;
                                exitingSamplex.X = oldDeltaX;
                                exitingSamplex.Y = oldDeltaY;

                                Bitex nextSamplex;
                                nextSamplex.X = deltaX;
                                nextSamplex.Y = deltaY;

                                unsigned int coverage = SegmentonomCoverage(&enteringSamplex, &exitingSamplex, &nextSamplex, contour->IsFilled);

                                //if the segmentonom is already crossed once (i.e. this is a +1 crossing)
                                if (marker_ != 0 && marker_ != INITIAL_PIXEL_MARKER)
                                {
                                    MetaCanvas_S1[position] = MulticrossCoverage(marker_, coverage);
                                }
                                    //(STATE) this is the first crossing of the segmentonom
                                else
                                {
                                    MetaCanvas_S1[position] = coverage;
                                }

                                enteringSamplex.X = deltaX;
                                enteringSamplex.Y = deltaY;
                                PreviousPixelX = currentPixelMinX;
                                PreviousPixelY = currentPixelMinY; //(->)
                                currentPixelMinX = RoundDown(deltaX);
                                currentPixelMaxX = RoundUp(deltaX);
                                currentPixelMinY = RoundDown(deltaY);
                                currentPixelMaxY = RoundUp(deltaY);
                            }
                        }
                    }
                }
                    ///IF THE POINT IS A CONTROL POINT OF A BEZIER CURVE/SPLINE
                else if (!currentPointIsON)
                {
                    //(STATE) the begin point of the curve is (the previous point in the list) or (implicit point generated before that)

                    double beginPointX = 0.0;
                    double beginPointY = 0.0;

                    double controlPointX = scaledPointX;
                    double controlPointY = scaledPointY;

                    double endPointX = scaledNextPointX;
                    double endPointY = scaledNextPointY;

                    //(L)

                    int previousContourPointIndex;

                    if (contourPointIndex == 0)
                    {
                        previousContourPointIndex = numberOfPoints - 1;
                    }
                    else
                    {
                        previousContourPointIndex = contourPointIndex -1;
                    }

                    unsigned char previousFlags = flags[previousContourPointIndex ];

                    //if the previous point is ON
                    if (GetBit(previousFlags, 0) == true)
                    {
                        beginPointX = (x_coordinates[previousContourPointIndex] * SCALE) + fx_shift;
                        beginPointY = (y_coordinates[previousContourPointIndex] * SCALE) + fy_shift;
                    }
                        //(STATE) there is an implicit point generated before that and that point is the begin point of the curve
                    else
                    {
                        beginPointX = implicitPoint.X;
                        beginPointY = implicitPoint.Y;
                    }

                    //if the next point is the end point of the curve
                    if (nextPointIsON)
                    {
                        endPointX = scaledNextPointX;
                        endPointY = scaledNextPointY;
                    }
                        //(STATE) the next point is also a control point
                    else
                    {
                        //generating an implicit ON point
                        implicitPoint = CentexOf(scaledPointX, scaledPointY, scaledNextPointX, scaledNextPointY);
                        endPointX = implicitPoint.X;
                        endPointY = implicitPoint.Y;
                    }

                    double scaledXFraction = FractionOf(beginPointX);
                    double scaledYFraction = FractionOf(beginPointY);

                    //if shifting the begin vertexoid to the right is needed
                    if (scaledXFraction <= VERTEXOID_SHIFT)
                    {
                        beginPointX +=  VERTEXOID_SHIFT;
                    }
                        //if shifting the begin vertexoid to the left is needed
                    else if (scaledXFraction >= 0.99)
                    {
                        beginPointX -= VERTEXOID_SHIFT;
                    }

                    //if shifting the begin vertexoid upwards is needed
                    if (scaledYFraction <= VERTEXOID_SHIFT)
                    {
                        beginPointY += VERTEXOID_SHIFT;
                    }
                        //if shifting the begin vertexoid downwards is needed
                    else if (scaledYFraction >= 0.99)
                    {
                        beginPointY -= VERTEXOID_SHIFT;
                    }

                    ///

                    double scaledXFraction_ = FractionOf(endPointX);
                    double scaledYFraction_ = FractionOf(endPointY);

                    //if shifting the begin vertexoid to the right is needed
                    if (scaledXFraction_ <= VERTEXOID_SHIFT)
                    {
                        endPointX += VERTEXOID_SHIFT;
                    }
                        //if shifting the begin vertexoid to the left is needed
                    else if (scaledXFraction_ >= 0.99)
                    {
                        endPointX -= VERTEXOID_SHIFT;
                    }

                    //if shifting the begin vertexoid upwards is needed
                    if (scaledYFraction_ <= VERTEXOID_SHIFT)
                    {
                        endPointY += VERTEXOID_SHIFT;
                    }
                        //if shifting the begin vertexoid downwards is needed
                    else if (scaledYFraction_ >= 0.99)
                    {
                        endPointY -= VERTEXOID_SHIFT;
                    }

                    //

                    endSegmentoid.X = endPointX;
                    endSegmentoid.Y = endPointY;

                    //writing the begin and end points of the current segment

                    double beginSegmentPixelX = RoundDown(beginPointX);
                    double beginSegmentPixelY = RoundDown(beginPointY);

                    deltaX = scaledPointX;
                    deltaY = scaledPointY;

                    currentPixelMinX = RoundDown(beginPointX);
                    currentPixelMaxX = RoundUp(beginPointX);
                    currentPixelMinY = RoundDown(beginPointY);
                    currentPixelMaxY = RoundUp(beginPointY);

                    double curveLength = LengthOfBezierCurve(beginPointX, beginPointY, controlPointX, controlPointY, endPointX, endPointY);

                    curveLength += curveLength / 100.0;

                    double percentage = 0.0;
                    double onePixelPercentage = 100.0 / curveLength;
                    double baseStep = (onePixelPercentage / 100.0) / 2.0; /* ~0.01px; the base step must be smaller than
            VERTEXOID_SHIFT, so that a pixel will not be missed if there is corner crossing, i.e. to ensure
            that delta will really cross the pixel (not just logically) and that the pixel will be marked as conturoid;
            on the other hand the step has to be large enough to achieve better performance - in this case the difference
            between the step and VERTEXOID_SHIFT is ~0.005px; the value is approximate, as the distance between two semplices
            depends on the curvature of the curve (which is not constant) */;

                    double a = controlPointX - beginPointX;
                    double b = controlPointY - beginPointY;
                    double c = endPointX - controlPointX;
                    double d = endPointY - controlPointY;

                    while (true)
                    {
                        oldDeltaX = deltaX;
                        oldDeltaY = deltaY;

                        double percentage_ = percentage + (baseStep * 10);
                        double p1_C_Interpolation_X = beginPointX + ((a / 100.0) * percentage_);
                        double p1_C_Interpolation_Y = beginPointY + ((b / 100.0) * percentage_);
                        double C_p2_Interpolation_X = controlPointX + ((c / 100.0) * percentage_);
                        double C_p2_Interpolation_Y = controlPointY + ((d / 100.0) * percentage_);
                        double delta_x = p1_C_Interpolation_X + (((C_p2_Interpolation_X - p1_C_Interpolation_X) / 100.0) * percentage_);
                        double delta_y = p1_C_Interpolation_Y + (((C_p2_Interpolation_Y - p1_C_Interpolation_Y) / 100.0) * percentage_);

                        //if there are 10 or less steps until crossing another pixel
                        if (delta_x < currentPixelMinX || delta_x >= currentPixelMaxX || delta_y < currentPixelMinY || delta_y >= currentPixelMaxY)
                        {
                            double p1_C_Interpolation_X_ = beginPointX + ((a / 100.0) * percentage);
                            double p1_C_Interpolation_Y_ = beginPointY + ((b / 100.0) * percentage);
                            double C_p2_Interpolation_X_ = controlPointX + ((c / 100.0) * percentage);
                            double C_p2_Interpolation_Y_ = controlPointY + ((d / 100.0) * percentage);
                            deltaX = p1_C_Interpolation_X_ + (((C_p2_Interpolation_X_ - p1_C_Interpolation_X_) / 100.0) * percentage);
                            deltaY = p1_C_Interpolation_Y_ + (((C_p2_Interpolation_Y_ - p1_C_Interpolation_Y_) / 100.0) * percentage);
                            percentage += baseStep;
                        }
                            /* (STATE) there are more than 10 steps until crossing another pixel, and there are more than 10 steps
                                 until the end of the curve */
                        else
                        {
                            deltaX = delta_x;
                            deltaY = delta_y;
                            percentage += baseStep * 10;
                        }

                        //if delta reaches the next pixel
                        if (deltaX < currentPixelMinX || deltaX >= currentPixelMaxX || deltaY < currentPixelMinY || deltaY >= currentPixelMaxY)
                        {
                            //(A) if the 'delta' pixel is the begin pixel of the contour
                            if (RoundDown(deltaX) == beginContourPixelX && RoundDown(deltaY) == beginContourPixelY)
                            {
                                beginPixelEnteringSamplex.X = deltaX;
                                beginPixelEnteringSamplex.Y = deltaY;
                            }

                            //if the current pixel is the begin pixel of the segment
                            if (currentPixelMinX == beginSegmentPixelX && currentPixelMinY == beginSegmentPixelY)
                            {
                                //(A) if the current pixel is the begin pixel of the contour
                                if (beginPixelExitingSamplex.X == -1)
                                {
                                    beginPixelExitingSamplex.X = oldDeltaX;
                                    beginPixelExitingSamplex.Y = oldDeltaY;
                                    beginPixelNextSamplex.X = deltaX;
                                    beginPixelNextSamplex.Y = deltaY;

                                    //this pixel must be marked as conturoid

                                    int position = currentPixelMinY * MetaCanvasWidth + currentPixelMinX;

                                    if (MetaCanvas_S1[position] == 0)
                                    {
                                        int value = INITIAL_PIXEL_MARKER; /* a value that is not 0; when the end of the contour is reached it will be replaced
                               with the real value */
                                        MetaCanvas_S1[position] = value;
                                    }
                                }
                                    //(STATE) the current pixel is the first for the segment, but not the first for the contour
                                else
                                {
                                    Bitex segmentoidVertex;
                                    segmentoidVertex.X = beginPointX;
                                    segmentoidVertex.Y = beginPointY;

                                    Bitex exitingSamplex;
                                    exitingSamplex.X = oldDeltaX;
                                    exitingSamplex.Y = oldDeltaY;

                                    Bitex nextSamplex;
                                    nextSamplex.X = deltaX;
                                    nextSamplex.Y = deltaY;

                                    unsigned int coverage = SegmentoidCoverage(&segmentoidVertex, &enteringSamplex, &exitingSamplex,
                                                                               &nextSamplex, contour->IsFilled);

                                    int position = currentPixelMinY * MetaCanvasWidth + currentPixelMinX;

                                    unsigned int marker = MetaCanvas_S1[position];

                                    //if the segmentoid is already crossed once (i.e. this is a +1 crossing)
                                    if (marker != 0 && marker != INITIAL_PIXEL_MARKER)
                                    {
                                        MetaCanvas_S1[position] = MulticrossCoverage(marker, coverage);
                                    }
                                        //(STATE) this is the first crossing of the segmentoid
                                    else
                                    {
                                        MetaCanvas_S1[position] = coverage;
                                    }
                                }

                                enteringSamplex.X = deltaX;
                                enteringSamplex.Y = deltaY;
                                PreviousPixelX = currentPixelMinX;
                                PreviousPixelY = currentPixelMinY; //(->)
                                currentPixelMinX = RoundDown(deltaX);
                                currentPixelMaxX = RoundUp(deltaX);
                                currentPixelMinY = RoundDown(deltaY);
                                currentPixelMaxY = RoundUp(deltaY);
                            }
                                //(STATE) the pixel is a segmentonom
                            else if (currentPixelMinX != endPointX || currentPixelMinY != endPointY)
                            {
                                /* it's possible that the entering and the exiting semplices are equal - this can happen if a pixel is 'missed',
                                   i.e. if there is corner crossing */

                                Bitex exitingSamplex;
                                exitingSamplex.X = oldDeltaX;
                                exitingSamplex.Y = oldDeltaY;

                                Bitex nextSamplex;
                                nextSamplex.X = deltaX;
                                nextSamplex.Y = deltaY;

                                unsigned int coverage = SegmentonomCoverage(&enteringSamplex, &exitingSamplex, &nextSamplex, contour->IsFilled);

                                int position = currentPixelMinY * MetaCanvasWidth + currentPixelMinX;

                                unsigned int marker = MetaCanvas_S1[position];

                                //if the segmentonom is already crossed once (i.e. this is a +1 crossing)
                                if (marker != 0 && marker != INITIAL_PIXEL_MARKER)
                                {
                                    MetaCanvas_S1[position] = MulticrossCoverage(marker, coverage);
                                }
                                    //(STATE) this is the first crossing of the segmentonom
                                else
                                {
                                    MetaCanvas_S1[position] = coverage;
                                }

                                enteringSamplex.X = deltaX;
                                enteringSamplex.Y = deltaY;
                                PreviousPixelX = currentPixelMinX;
                                PreviousPixelY = currentPixelMinY; //(->)
                                currentPixelMinX = RoundDown(deltaX);
                                currentPixelMaxX = RoundUp(deltaX);
                                currentPixelMinY = RoundDown(deltaY);
                                currentPixelMaxY = RoundUp(deltaY);
                            }
                        }

                        if (percentage >= 100.0)
                        {
                            break;
                        }
                    }
                }

                if (endIndex < numberOfPoints - 1 && contourPointIndex == endIndex)
                {
                    break;
                }
                else if (endIndex == numberOfPoints - 1 && contourPointIndex == numberOfPoints - 1)
                {
                    break;
                }
            }

            //determine the coverage of the begin/end pixel of the contour

            if (beginPixelEnteringSamplex.X > -1)
            {
                unsigned int coverage = SegmentoidCoverage(&endSegmentoid, &beginPixelEnteringSamplex, &beginPixelExitingSamplex,
                                                           &beginPixelNextSamplex, contour->IsFilled);

                int position = beginContourPixelY * MetaCanvasWidth + beginContourPixelX;

                unsigned int marker = MetaCanvas_S1[position];

                if (marker != 0 && marker != INITIAL_PIXEL_MARKER)
                {
                    MetaCanvas_S1[position] = MulticrossCoverage(marker, coverage);
                }
                else
                {
                    MetaCanvas_S1[position] = coverage;
                }

                PreviousPixelX = currentPixelMinX;
                PreviousPixelY = currentPixelMinY;
            }

            free(x_coordinates);
            free(y_coordinates);
            free(flags);

            ///FILLING THE CONTOUR

            //for every row of the graphema
            for (int row = 0; row < MetaCanvasHeight; row++)
            {
                bool fillMode = false;

                //for every column of the graphema
                for (int column = 0; column < MetaCanvasWidth; column++)
                {
                    unsigned int marker = MetaCanvas_S1[row * MetaCanvasWidth + column];
                    unsigned int coverage = GetBits(marker, 0, 6);
                    unsigned int O_Crossing = GetBits(marker, O_BEGIN, O_END);
                    unsigned int T_Crossing = GetBits(marker, T_BEGIN, T_END);
                    bool is_O_LEFT = GetBit(marker, O_LEFT);
                    bool is_T_LEFT = GetBit(marker, T_LEFT);
                    bool is_O_RIGHT = GetBit(marker, O_RIGHT);
                    bool is_T_RIGHT = GetBit(marker, T_RIGHT);

                    if (coverage > 0)
                    {
                        //only O-crossing
                        if (O_Crossing > 0 && T_Crossing == 0)
                        {
                            fillMode = true;
                        }
                            //only T-crossing
                        else if (O_Crossing == 0 && T_Crossing > 0)
                        {
                            fillMode = false;
                        }
                            //(BLOCK) O-crossing и T-crossing
                        else if (is_O_LEFT && !is_T_LEFT)
                        {
                            fillMode = O_Crossing > T_Crossing;
                        }
                        else if (!is_O_RIGHT && is_T_RIGHT)
                        {
                            fillMode = false;
                        }
                        else if (is_T_LEFT && !is_O_LEFT && !is_O_RIGHT)
                        {
                            fillMode = O_Crossing > T_Crossing;
                        }
                        else if (is_T_LEFT && !is_O_LEFT)
                        {
                            fillMode = true;
                        }
                        else if (!is_T_RIGHT && is_O_RIGHT)
                        {
                            fillMode = true;
                        }
                        else if (is_O_LEFT && is_T_LEFT)
                        {
                            fillMode = O_Crossing > T_Crossing;
                        }
                        else if (is_O_RIGHT && is_T_RIGHT)
                        {
                            fillMode = false;
                        }
                        else if (O_Crossing > 0 && T_Crossing > 0 && !is_O_LEFT && !is_O_RIGHT && !is_T_LEFT && !is_T_RIGHT)
                        {
                            fillMode = O_Crossing > T_Crossing;
                        }
                    }

                    int pixelType = EXTEROID;

                    if (coverage > 0)
                    {
                        pixelType = CONTUROID;
                    }
                    else if (fillMode)
                    {
                        pixelType = INTEROID;
                    }

                    MetaCanvas_S1[row * MetaCanvasWidth + column] = 0;
                    unsigned char previousPixelType = GetBits(MetaCanvas_S2[row * MetaCanvasWidth + column], 0, 7);
                    unsigned char previousCoverage = GetBits(MetaCanvas_S2[row * MetaCanvasWidth + column], 8, 15);

                    if (pixelType == CONTUROID && coverage > previousCoverage)
                    {
                        SetBits_USHORT(&MetaCanvas_S2[row * MetaCanvasWidth + column], 8, 15, coverage);
                    }

                    if (previousPixelType == EXTEROID && pixelType != EXTEROID && contour->IsFilled)
                    {
                        SetBits_USHORT(&MetaCanvas_S2[row * MetaCanvasWidth + column], 0, 7, pixelType);
                    }
                    else if (previousPixelType == CONTUROID && pixelType == INTEROID)
                    {
                        SetBits_USHORT(&MetaCanvas_S2[row * MetaCanvasWidth + column], 0, 7, INTEROID);
                    }

                    if (previousPixelType != EXTEROID && previousPixelType != CONTUROID && !contour->IsFilled)
                    {
                        if (pixelType == EXTEROID)
                        {
                            pixelType = INTEROID;
                        }
                        else if (pixelType == INTEROID)
                        {
                            pixelType = EXTEROID;
                        }

                        SetBits_USHORT(&MetaCanvas_S2[row * MetaCanvasWidth + column], 0, 7, pixelType);
                    }
                }
            }
        }

        //for every row of the graphema
        for (int row = 0; row < MetaCanvasHeight; row++)
        {
            //for every column of the graphema
            for (int column = 0; column < MetaCanvasWidth; column++)
            {
                unsigned char pixelType = GetBits(MetaCanvas_S2[row * MetaCanvasWidth + column], 0, 7);
                unsigned char coverage = GetBits(MetaCanvas_S2[row * MetaCanvasWidth + column], 8, 14);

                int targetColumn = _horizontalPosition + column;
                int targetRow = _verticalPosition + row;

                if (targetColumn < 0 || targetColumn >= _canvasWidth || targetRow < 0 || targetRow >= _canvasHeight)
                {
                    continue;
                }

                if (_maxGraphemicX != -1 && targetColumn > _maxGraphemicX)
                {
                    continue;
                }

                tt_rgba backgroundColor = TT_GetPixel(_canvas, _canvasWidth, targetColumn, targetRow);

                int targetPixelPosition = (targetRow * _canvasWidth + targetColumn) * PIXEL_SIZE;

                unsigned char betaCoverage = 100.0 - coverage;

                tt_rgba color;
                tt_rgba foregroundColor;

                if (_colorizationMode == GCM_SOLID)
                {
                    foregroundColor = _colors[0];
                }
                else if (_colorizationMode == GCM_HORIZONTAL_GRADIENT)
                {
                    int maxGraphemicX = MetaCanvasWidth - 1;
                    int colorSegmentSize = maxGraphemicX / (_numberOfColors - 1);
                    int currentColorIndex = RoundDown(column / colorSegmentSize);
                    int currentSegmentPixel = column - (currentColorIndex * colorSegmentSize);

                    const tt_rgba* alphaColor = &_colors[currentColorIndex];
                    const tt_rgba* betaColor = &_colors[currentColorIndex + 1];

                    double r_step = (double)(betaColor->R - alphaColor->R) / colorSegmentSize;
                    double g_step = (double)(betaColor->G - alphaColor->G) / colorSegmentSize;
                    double b_step = (double)(betaColor->B - alphaColor->B) / colorSegmentSize;

                    foregroundColor.R = alphaColor->R + (r_step * currentSegmentPixel);
                    foregroundColor.G = alphaColor->G + (g_step * currentSegmentPixel);
                    foregroundColor.B = alphaColor->B + (b_step * currentSegmentPixel);
                }
                else if (_colorizationMode == GCM_VERTICAL_GRADIENT)
                {
                    int maxGraphemicY = MetaCanvasHeight - 1;
                    int colorSegmentSize = maxGraphemicY / (_numberOfColors - 1);
                    int currentColorIndex = RoundDown(row / colorSegmentSize);
                    int currentSegmentPixel = row - (currentColorIndex * colorSegmentSize);

                    const tt_rgba* alphaColor = &_colors[currentColorIndex];
                    const tt_rgba* betaColor = &_colors[currentColorIndex + 1];

                    double r_step = (double)(betaColor->R - alphaColor->R) / colorSegmentSize;
                    double g_step = (double)(betaColor->G - alphaColor->G) / colorSegmentSize;
                    double b_step = (double)(betaColor->B - alphaColor->B) / colorSegmentSize;

                    foregroundColor.R = alphaColor->R + (r_step * currentSegmentPixel);
                    foregroundColor.G = alphaColor->G + (g_step * currentSegmentPixel);
                    foregroundColor.B = alphaColor->B + (b_step * currentSegmentPixel);
                }
                else if (_colorizationMode == GCM_S_HORIZONTAL_GRADIENT)
                {
                    int stringColumn = (_horizontalPosition + column) - StringBeginX;
                    int maxGraphemicX = StringWidth - 1;
                    int colorSegmentSize = StringWidth / (_numberOfColors - 1);
                    int currentColorIndex = RoundDown(stringColumn / colorSegmentSize);
                    int currentSegmentPixel = stringColumn - (currentColorIndex * colorSegmentSize);

                    const tt_rgba* alphaColor = &_colors[currentColorIndex];
                    const tt_rgba* betaColor = &_colors[currentColorIndex + 1];

                    double r_step = (double)(betaColor->R - alphaColor->R) / colorSegmentSize;
                    double g_step = (double)(betaColor->G - alphaColor->G) / colorSegmentSize;
                    double b_step = (double)(betaColor->B - alphaColor->B) / colorSegmentSize;

                    foregroundColor.R = alphaColor->R + (r_step * currentSegmentPixel);
                    foregroundColor.G = alphaColor->G + (g_step * currentSegmentPixel);
                    foregroundColor.B = alphaColor->B + (b_step * currentSegmentPixel);
                }
                else if (_colorizationMode == GCM_S_VERTICAL_GRADIENT)
                {
                    int stringRow = (_verticalPosition + row) - StringBeginY;
                    int maxGraphemicY = StringBeginY + StringHeight;
                    int colorSegmentSize = StringHeight / (_numberOfColors - 1);
                    int currentColorIndex = RoundDown(stringRow / colorSegmentSize);
                    int currentSegmentPixel = stringRow - (currentColorIndex * colorSegmentSize);

                    const tt_rgba* alphaColor = &_colors[currentColorIndex];
                    const tt_rgba* betaColor = &_colors[currentColorIndex + 1];

                    double r_step = (double)(betaColor->R - alphaColor->R) / colorSegmentSize;
                    double g_step = (double)(betaColor->G - alphaColor->G) / colorSegmentSize;
                    double b_step = (double)(betaColor->B - alphaColor->B) / colorSegmentSize;

                    foregroundColor.R = alphaColor->R + (r_step * currentSegmentPixel);
                    foregroundColor.G = alphaColor->G + (g_step * currentSegmentPixel);
                    foregroundColor.B = alphaColor->B + (b_step * currentSegmentPixel);
                }

                if (pixelType == CONTUROID)
                {
                    //rounding to the nearest value of the (values of the color components) is not needed, as the effect will be neglible
                    color.R = ((foregroundColor.R / 100.0) * coverage) + ((backgroundColor.R / 100.0) * betaCoverage);
                    color.G = ((foregroundColor.G / 100.0) * coverage) + ((backgroundColor.G / 100.0) * betaCoverage);
                    color.B = ((foregroundColor.B / 100.0) * coverage) + ((backgroundColor.B / 100.0) * betaCoverage);
                }
                else if (pixelType == INTEROID)
                {
                    color.R = foregroundColor.R;
                    color.G = foregroundColor.G;
                    color.B = foregroundColor.B;
                }
                else
                {
                    color.R = backgroundColor.R;
                    color.G = backgroundColor.G;
                    color.B = backgroundColor.B;
                }

                if (_transparency == 0)
                {
                    if (_colorComponentOrder == RGBA_ORDER)
                    {
                        _canvas[targetPixelPosition] = color.R;
                        _canvas[targetPixelPosition + 1] = color.G;
                        _canvas[targetPixelPosition + 2] = color.B;
                    }
                    else
                    {
                        _canvas[targetPixelPosition] = color.B;
                        _canvas[targetPixelPosition + 1] = color.G;
                        _canvas[targetPixelPosition + 2] = color.R;
                    }
                }
                else
                {
                    if (_colorComponentOrder == RGBA_ORDER)
                    {
                        _canvas[targetPixelPosition] = GetColorComponent(backgroundColor.R, color.R, _transparency);
                        _canvas[targetPixelPosition + 1] = GetColorComponent(backgroundColor.G, color.G, _transparency);
                        _canvas[targetPixelPosition + 2] = GetColorComponent(backgroundColor.B, color.B, _transparency);
                    }
                    else
                    {
                        _canvas[targetPixelPosition] = GetColorComponent(backgroundColor.B, color.B, _transparency);
                        _canvas[targetPixelPosition + 1] = GetColorComponent(backgroundColor.G, color.G, _transparency);
                        _canvas[targetPixelPosition + 2] = GetColorComponent(backgroundColor.R, color.R, _transparency);
                    }
                }
            }
        }

        if (orderedContours != unorderedContours)
        {
            free(orderedContours);
        }
        //(->)
        free(unorderedContours);
    }
        ///(STATE) THE GLYPH IS COMPOSITE
    else
    {
        CompositeGlyph* glyph_ = (CompositeGlyph*) glyph;

        for (int i = 0; i < glyph_->NumberOfComponents; i++)
        {
            GlyphComponent* component = glyph_->Components[i];

            double x_scale;
            double y_scale;

            //2x2 scaling (not supported)
            if (component->Scale[2] > 0)
            {
                int breakpoint = 0;
            }
                //X&Y scaling (different values)
            else if (component->Scale[1] > 0)
            {
                int x_integer = GetBit(component->Scale[0], 14);
                int x_fraction = GetBits(component->Scale[0], 0, 13);

                int y_integer = GetBit(component->Scale[1], 14);
                int y_fraction = GetBits(component->Scale[1], 0, 13);

                if (GetBit(component->Scale[0], 15))
                {
                    x_integer = -x_integer;
                }

                if (GetBit(component->Scale[1], 15))
                {
                    y_integer = -y_integer;
                }

                x_scale = x_integer + (((double) x_fraction) / 16384);
                y_scale = y_integer + (((double) y_fraction) / 16384);
            }
                //X&Y scaling (same value)
            else
            {
                int integer = GetBit(component->Scale[0], 14);
                int fraction = GetBits(component->Scale[0], 0, 13);

                if (GetBit(component->Scale[0], 15))
                {
                    integer = -integer;
                }

                x_scale = integer + ((double) fraction) / 16384;
                y_scale = x_scale;
            }

            DrawCharacter(
                    -component->GlyphIndex,
                    NULL,
                    _font,
                    _canvas,
                    _colorComponentOrder,
                    _canvasWidth,
                    _canvasHeight,
                    _horizontalPosition + (x_scale != -1.0 ? component->Argument1 * SCALE : 0),
                    _verticalPosition + (y_scale != -1.0 ? component->Argument2 * SCALE : 0),
                    _fontSize,
                    _colorizationMode,
                    _colors,
                    _numberOfColors,
                    _transparency,
                    -1,
                    component->Argument1,
                    component->Argument2,
                    x_scale,
                    y_scale);
        }
    }

    if (MetaCanvas_S1 != NULL)
    {
        free(MetaCanvas_S1);
        free(MetaCanvas_S2);
        MetaCanvas_S1 = NULL;
        MetaCanvas_S2 = NULL;
    }
}

//(PUBLIC)
//returns the width of the string in pixels (with the left-side bearing of the first character and the right-side bearing of the last character)
double GetTypographicWidth(const Font* _font, const wchar_t* _string, double _fontSize)
{
    int currentWidth = 0;

    HMTX_Table* hmtx = (HMTX_Table*) GetTable(_font, HMTX_TABLE);

    //the first character in the string

    int index = GetGlyphIndex(_font, _string[0]);
    currentWidth += hmtx->HorizontalMetrics[index].AdvanceWidth;

    double firstCharacterLeftSideBearing = GetLeftSideBearing(_font, _string[0]);

    if (firstCharacterLeftSideBearing < 0)
    {
        currentWidth += Absolute(firstCharacterLeftSideBearing);
    }

    //characters (1..last)

    int stringLength = wcslen(_string);

    for (int i = 1; i < stringLength; i++)
    {
        int index = GetGlyphIndex(_font, _string[i]);
        currentWidth += hmtx->HorizontalMetrics[index].AdvanceWidth;
    }

    //the last character in the string

    double lastCharacterRightSideBearing = GetRightSideBearing(_font, _string[stringLength - 1]);

    if (lastCharacterRightSideBearing < 0)
    {
        currentWidth += Absolute(lastCharacterRightSideBearing);
    }

    //

    return currentWidth * GetScale(_font, _fontSize);
}

//(PUBLIC)
//returns the width of the string in pixels (without the left-side bearing of the first character and the right-side bearing of the last character)
//_fontSize is specified in pixels
//_string.length() >= 1 ->
double GetGraphemicWidth(const Font* _font, const wchar_t* _string, double _fontSize)
{
    //(STATE) the string has more than 1 character

    HMTX_Table* hmtx = (HMTX_Table*) GetTable(_font, HMTX_TABLE);

    double SCALE = GetScale(_font, _fontSize);

    int currentWidth = 0;
    int stringLength = wcslen(_string);

    ///characters (0..last)

    for (int i = 0; i < stringLength; i++)
    {
        int index = GetGlyphIndex(_font, _string[i]);
        double advanceWidth = hmtx->HorizontalMetrics[index].AdvanceWidth;

        if (i < stringLength - 1)
        {
            int kerning = GetKerning(_font, _string[i], _string[i + 1]);

            //if there is no kerning between the two characters
            if (kerning == INT_MIN)
            {
                currentWidth += advanceWidth;
            }
                //if there is negative kerning between the two characters
            else if (kerning < 0)
            {
                currentWidth += advanceWidth - (0 - kerning);
            }
                //if there is positive kerning between the two characters
            else
            {
                currentWidth += advanceWidth + kerning;
            }
        }
        else
        {
            currentWidth += advanceWidth;
        }
    }

    ///left-side bearing of the first character in the string

    double firstCharacterLeftSideBearing = GetLeftSideBearing(_font, _string[0]);

    //negative left-side bearing
    if (firstCharacterLeftSideBearing < 0)
    {
        currentWidth += Absolute(firstCharacterLeftSideBearing);
    }
        //positive left-side bearing
    else
    {
        currentWidth -= firstCharacterLeftSideBearing;
    }

    ///right-side bearing of the last character in the string

    double lastCharacterRightSideBearing = GetRightSideBearing(_font,  _string[stringLength - 1]);

    //negative right-side bearing
    if (lastCharacterRightSideBearing < 0)
    {
        currentWidth += Absolute(lastCharacterRightSideBearing);
    }
        //positive right-side bearing
    else
    {
        currentWidth -= lastCharacterRightSideBearing;
    }

    //

    return currentWidth * SCALE;
}

//(PUBLIC)
//returns the graphemic height of the string in pixels (the distance between the lowest and the highest graphemic point in the string)
//_fontSize is specified in pixels
double GetGraphemicHeight(const Font* _font, const wchar_t* _string, double _fontSize)
{
    int minY = -1;
    int maxY = -1;
    int stringLength = wcslen(_string);

    for (int i = 0; i < stringLength; i++)
    {
        void* glyph = GetGlyph(_font, _string[i]);

        if (Is(glyph, EMPTY_GLYPH))
        {
            continue;
        }
        else if (Is(glyph, SIMPLE_GLYPH))
        {
            SimpleGlyph* glyph_ = (SimpleGlyph*) glyph;

            if (minY == -1 || glyph_->MinY < minY)
            {
                minY = glyph_->MinY;
            }

            if (maxY == -1 || glyph_->MaxY > maxY)
            {
                maxY = glyph_->MaxY;
            }
        }
        else
        {
            CompositeGlyph* glyph_ = (CompositeGlyph*) glyph;

            if (minY == -1 || glyph_->MinY < minY)
            {
                minY = glyph_->MinY;
            }

            if (maxY == -1 || glyph_->MaxY > maxY)
            {
                maxY = glyph_->MaxY;
            }
        }
    }

    return (maxY - minY) * GetScale(_font, _fontSize);
}

//(PRIVATE)
//(LOCAL-TO DrawString)
bool IsWhitespace(int _character)
{
    //(SOURCE) https://en.wikipedia.org/wiki/Whitespace_character
    return _character == 9 || _character == 10 || _character == 11 || _character == 12 || _character == 13 || _character == 32 ||
           _character == 133 || _character == 160 || _character == 5760 || (_character >= 8192 && _character <= 8202) ||
           _character == 8232 || _character == 8233 || _character == 8239 || _character == 8287 || _character == 12288;
}

//(PUBLIC)
//_canvas is (a RGBA or BGRA pixel array) in which the character is drawn
//Y_Direction specifies the direction in which the Y-coordinates grow (top-to-bottom or bottom-up)
//_colorComponentOrder specifies if the pixels in _canvas are RGBA or BGRA
//_canvasWidth and _canvasHeight are the width and height of the canvas specified in pixels
//_horizonalPosition specifies the position (in pixels) of the leftmost graphemic point of the string
//_verticalPosition specifies the position (in pixels) of the baseline
//_fontSize is the height of the line in pixels
//_numberOfColors should be equal (or larger) to the number of elements in _colors
//_transparency = 0 means fully opaque string, and 100 means fully transparent string
/*_maxGraphemicX specifies a limiting X coordinate in the canvas (not an X coordinate in the string itself) - i.e. the part of the
   string after this coordinate will not be visualized; a value of -1 specifies that there is no horizontal limit;
   this coordinate is inclusive, i.e. the column matching the coordinate will also be visualized */
/* (!!!) this is a non-validating function; the font must contain all the (glyphs corresponding to the characters in the specified string)
         and the parameters must have correct values */
void DrawString(
        const wchar_t* _string,
        const Font* _font,
        unsigned char* _canvas,
        ColorComponentOrder _colorComponentOrder,
        int _canvasWidth,
        int _canvasHeight,
        double _horizontalPosition,
        double _verticalPosition,
        double _fontSize,
        StringColorizationMode _colorizationMode,
        const tt_rgba* _colors,
        int _numberOfColors,
        int _transparency,
        int _maxGraphemicX)
{
    double SCALE = GetScale(_font, _fontSize);
    HMTX_Table* hmtx = (HMTX_Table*) GetTable(_font, HMTX_TABLE);
    int stringLength = wcslen(_string);
    int lsb = GetLeftSideBearing(_font, _string[0]);
    _horizontalPosition -= lsb * SCALE;

    StringBeginX = _horizontalPosition;
    StringBeginY = _verticalPosition + GetDescent(_font, _string, stringLength) * SCALE;
    StringWidth = GetGraphemicWidth(_font, _string, _fontSize);
    StringHeight = GetGraphemicHeight(_font, _string, _fontSize);

    int groupElementIndex = 0;
    for (int i = 0; i < stringLength; i++)
    {
        const tt_rgba* colors;

        enum GlyphColorizationMode glyphColorizationMode;

        if (_colorizationMode == SCM_SOLID_IDENTICAL)
        {
            colors = &_colors[0];
            glyphColorizationMode = GCM_SOLID;
        }
        else if (_colorizationMode == SCM_SOLID_INDIVIDUAL)
        {
            //move to the next color if the glyph is not empty
            if (!IsWhitespace(_string[i]))
            {
                if (groupElementIndex + 1 < _numberOfColors)
                {
                    groupElementIndex++;
                }
                else
                {
                    groupElementIndex = 0;
                }
            }

            colors = &_colors[groupElementIndex];

            glyphColorizationMode = GCM_SOLID;
        }
        else if (_colorizationMode == SCM_SOLID_WORD)
        {
             if (IsWhitespace(_string[i]))
             {
                 if (groupElementIndex + 1 < _numberOfColors)
                 {
                     groupElementIndex++;
                 }
                 else
                 {
                     groupElementIndex = 0;
                 }
             }

             colors = &_colors[groupElementIndex];

            glyphColorizationMode = GCM_SOLID;
        }
        else if (_colorizationMode == SCM_HORIZONTAL_GRADIENT_GLYPH)
        {
            colors = _colors;
            glyphColorizationMode = GCM_HORIZONTAL_GRADIENT;
        }
        else if (_colorizationMode == SCM_VERTICAL_GRADIENT_GLYPH)
        {
            colors = _colors;
            glyphColorizationMode = GCM_VERTICAL_GRADIENT;
        }
        else if (_colorizationMode == SCM_HORIZONTAL_GRADIENT_STRING)
        {
            colors = _colors;
            glyphColorizationMode = GCM_S_HORIZONTAL_GRADIENT;
        }
        else if (_colorizationMode == SCM_VERTICAL_GRADIENT_STRING)
        {
            colors = _colors;
            glyphColorizationMode = GCM_S_VERTICAL_GRADIENT;
        }

        DrawCharacter(
            _string[i],
            NULL,
            _font,
            _canvas,
            _colorComponentOrder,
            _canvasWidth,
            _canvasHeight,
            _horizontalPosition,
            _verticalPosition,
            _fontSize,
            glyphColorizationMode,
            colors,
            _numberOfColors,
            _transparency,
            _maxGraphemicX,
            0.0,
            0.0,
            0.0,
            0.0);

        double scaledKerning = 0;

        if (i < stringLength - 1)
        {
            int kerning = GetKerning(_font, _string[i], _string[i + 1]);

            //if there is kerning between the two characters
            if (kerning != INT_MIN)
            {
                scaledKerning = kerning * SCALE;
            }

            int glyphIndex = GetGlyphIndex(_font, _string[i]);
            double advanceWidth = hmtx->HorizontalMetrics[glyphIndex].AdvanceWidth * SCALE;

            //if there is no kerning between the two characters
            if (kerning == INT_MIN)
            {
                _horizontalPosition += advanceWidth;
            }
                //if there is negative kerning between the two characters
            else if (scaledKerning < 0)
            {
                _horizontalPosition += advanceWidth - (0 - scaledKerning);
            }
                //if there is positive kerning between the two characters
            else
            {
                _horizontalPosition += advanceWidth + scaledKerning;
            }
        }
    }
}


#ifndef TT_RASTERIZER_C_RASTERIZER_H
#define TT_RASTERIZER_C_RASTERIZER_H

#include "Parser.h"

#define C_ALICE_BLUE &(tt_rgba){ 240, 248, 255, 0 }
#define C_ANTIQUE_WHITE &(tt_rgba){ 250, 235, 215, 0 }
#define C_AQUA &(tt_rgba){ 0, 255, 255, 0 }
#define C_AQUAMARINE &(tt_rgba){ 127, 255, 212, 0 }
#define C_AZURE &(tt_rgba){  240, 255, 255, 0 }
#define C_BEIGE &(tt_rgba){ 245, 245, 220, 0 }
#define C_BISQUE &(tt_rgba){ 255, 228, 196, 0 }
#define C_BLACK &(tt_rgba){ 0, 0, 0, 0 }
#define C_BLANCHED_ALMOND &(tt_rgba){ 255, 235, 205, 0 }
#define C_BLUE &(tt_rgba){ 0, 0, 255, 0 }
#define C_BLUE_VIOLET &(tt_rgba){ 138, 43, 226, 0 }
#define C_BROWN &(tt_rgba){ 165,  42,  42, 0 }
#define C_BURLYWOOD &(tt_rgba){ 222, 184, 135, 0 }
#define C_CADET_BLUE &(tt_rgba){ 95, 158, 160, 0 }
#define C_CHARTREUSE &(tt_rgba){ 127, 255, 0, 0 }
#define C_CHOCOLATE &(tt_rgba){ 210, 105, 30, 0 }
#define C_CORAL &(tt_rgba){ 255, 127, 80, 0 }
#define C_CORNFLOWER_BLUE &(tt_rgba){ 100, 149, 237, 0 }
#define C_CORNSILK &(tt_rgba){ 255, 248, 220, 0 }
#define C_CRIMSON &(tt_rgba){ 220, 20, 60, 0 }
#define C_CYAN &(tt_rgba){ 0, 255, 255, 0 }
#define C_DARK_BLUE &(tt_rgba){ 0, 0, 139, 0 }
#define C_DARK_CYAN &(tt_rgba){ 0, 139, 139, 0 }
#define C_DARK_GOLDENROD &(tt_rgba){ 184, 134, 11, 0 }
#define C_DARK_GRAY &(tt_rgba){ 169, 169, 169, 0 }
#define C_DARK_GREEN &(tt_rgba){ 0, 100, 0, 0 }
#define C_DARK_KHAKI &(tt_rgba){ 189, 183, 107, 0 }
#define C_DARK_MAGENTA &(tt_rgba){ 139, 0, 139, 0 }
#define C_DARK_OLIVE_GREEN &(tt_rgba){ 85, 107, 47, 0 }
#define C_DARK_ORANGE &(tt_rgba){ 255, 140, 0, 0 }
#define C_DARK_ORCHID &(tt_rgba){ 153, 50, 204, 0 }
#define C_DARK_RED &(tt_rgba){ 139, 0, 0, 0 }
#define C_DARK_SALMON &(tt_rgba){  233, 150, 122, 0 }
#define C_DARK_SEA_GREEN &(tt_rgba){ 143, 188, 143, 0 }
#define C_DARK_SLATE_BLUE &(tt_rgba){ 72, 61, 139, 0 }
#define C_DARK_SLATE_GRAY &(tt_rgba){ 47, 79, 79, 0 }
#define C_DARK_TURQUOISE &(tt_rgba){ 0, 206, 209, 0 }
#define C_DARK_VIOLET &(tt_rgba){ 148, 0, 211, 0 }
#define C_DEEP_PINK &(tt_rgba){ 255, 20, 147, 0 }
#define C_DEEP_SKY_BLUE &(tt_rgba){ 0, 191, 255, 0 }
#define C_DIM_GRAY &(tt_rgba){ 105, 105, 105, 0 }
#define C_DODGER_BLUE &(tt_rgba){ 30, 144, 255, 0 }
#define C_FIREBRICK &(tt_rgba){ 178, 34, 34, 0 }
#define C_FLORAL_WHITE &(tt_rgba){ 255, 250, 240, 0 }
#define C_FOREST_GREEN &(tt_rgba){ 34, 139, 34, 0 }
#define C_FUCHSIA &(tt_rgba){ 255, 0, 255, 0 }
#define C_GAINSBORO &(tt_rgba){ 220, 220, 220, 0 }
#define C_GHOST_WHITE &(tt_rgba){ 248, 248, 255, 0 }
#define C_GOLD &(tt_rgba){  255, 215, 0, 0 }
#define C_GOLDENROD &(tt_rgba){ 218, 165, 32, 0 }
#define C_GRAY &(tt_rgba){ 128, 128, 128, 0 }
#define C_GREEN &(tt_rgba){ 0, 255, 0, 0 }
#define C_GREEN_YELLOW &(tt_rgba){ 173, 255, 47, 0 }
#define C_HONEYDEW &(tt_rgba){ 240, 255, 240 }
#define C_HOT_PINK &(tt_rgba){ 255, 105, 180 }
#define C_INDIAN_RED &(tt_rgba){ 255, 92, 92, 0 }
#define C_INDIGO &(tt_rgba){ 75, 0, 130, 0 }
#define C_IVORY &(tt_rgba){ 255, 255, 240, 0 }
#define C_KHAKI &(tt_rgba){ 240, 230, 140, 0 }
#define C_LAVENDER &(tt_rgba){ 230, 230, 250, 0 }
#define C_LAVENDER_BUSH &(tt_rgba){ 255, 240, 245, 0 }
#define C_LAWN_GREEN &(tt_rgba){ 124, 252, 0, 0 }
#define C_LEMON_CHIFFON &(tt_rgba){ 255, 250, 205, 0 }
#define C_LIGHT_BLUE &(tt_rgba){ 173, 216, 230, 0 }
#define C_LIGHT_CORAL &(tt_rgba){ 240, 128, 128, 0 }
#define C_LIGHT_CYAN &(tt_rgba){ 224, 255, 255, 0 }
#define C_LIGHT_GOLDENROD_YELLOW &(tt_rgba){ 250, 250, 210, 0 }
#define C_LIGHT_GRAY &(tt_rgba){ 211, 211, 211, 0 }
#define C_LIGHT_GREEN &(tt_rgba){ 144, 238, 144, 0 }
#define C_LIGHT_PINK &(tt_rgba){ 255, 182, 193, 0 }
#define C_LIGHT_SALMON &(tt_rgba){ 255, 160, 122, 0 }
#define C_LIGHT_SEA_GREEN &(tt_rgba){ 32, 178, 170, 0 }
#define C_LIGHT_SKY_BLUE &(tt_rgba){ 135, 206, 250, 0 }
#define C_LIGHT_SLATE_GRAY &(tt_rgba){ 119, 136, 153, 0 }
#define C_LIGHT_STEEL_BLUE &(tt_rgba){ 176, 196, 222, 0 }
#define C_LIGHT_YELLOW &(tt_rgba){ 255, 255, 224, 0 }
#define C_LIME &(tt_rgba){ 0, 255, 0, 0 }
#define C_LIME_GREEN &(tt_rgba){ 50, 205, 50, 0 }
#define C_LINEN &(tt_rgba){ 250, 240, 230, 0 }
#define C_MAGENTA &(tt_rgba){ 255, 0, 255, 0 }
#define C_MAROON &(tt_rgba){  128, 0, 0, 0 }
#define C_MEDIUM_AQUAMARINE &(tt_rgba){ 102, 205, 170, 0 }
#define C_MEDIUM_BLUE &(tt_rgba){ 0, 0, 205, 0 }
#define C_MEDIUM_ORCHID &(tt_rgba){ 186, 85, 211, 0 }
#define C_MEDIUM_PURPLE &(tt_rgba){ 147, 112, 219, 0 }
#define C_MEDIUM_SLATE_BLUE &(tt_rgba){ 123, 104, 238, 0 }
#define C_MEDIUM_SPRING_GREEN &(tt_rgba){ 0, 250, 154, 0 }
#define C_MEDIUM_TURQUOISE &(tt_rgba){ 72, 209, 204, 0 }
#define C_MEDIUM_VIOLET_RED &(tt_rgba){ 199, 21, 133, 0 }
#define C_MIDNIGHT_BLUE &(tt_rgba){ 25, 25, 112, 0 }
#define C_MINT_CREAM &(tt_rgba){  245, 255, 250, 0 }
#define C_MISTY_ROSE &(tt_rgba){ 255, 228, 225, 0 }
#define C_MOCCASIN &(tt_rgba){ 255, 228, 181, 0 }
#define C_NAVAJO_WHITE &(tt_rgba){ 255, 222, 173, 0 }
#define C_NAVY &(tt_rgba){ 0, 0, 128, 0 }
#define C_OLD_LICE &(tt_rgba){ 253, 245, 230, 0 }
#define C_OLIVE &(tt_rgba){ 128, 128, 0, 0 }
#define C_OLIVE_DRAB &(tt_rgba){ 107, 142, 35, 0 }
#define C_ORANGE &(tt_rgba){ 255, 165, 0, 0 }
#define C_ORANGE_RED &(tt_rgba){ 255, 69, 0, 0 }
#define C_ORCHID &(tt_rgba){ 218, 112, 214, 0 }
#define C_PALE_GOLDENROD &(tt_rgba){ 238, 232, 170, 0 }
#define C_PALE_GREEN &(tt_rgba){ 152, 251, 152, 0 }
#define C_PALE_TURQUOISE &(tt_rgba){ 175, 238, 238, 0 }
#define C_PALE_VIOLET_RED &(tt_rgba){ 219, 112, 147, 0 }
#define C_PAPAYA_WHIP &(tt_rgba){ 255, 239, 213, 0 }
#define C_PEACH_PUFF &(tt_rgba){ 255, 218, 185, 0 }
#define C_PERU &(tt_rgba){ 205, 133, 63, 0 }
#define C_PINK &(tt_rgba){ 255, 192, 203, 0 }
#define C_PLUM &(tt_rgba){ 221, 160, 221, 0 }
#define C_POWDER_BLUE &(tt_rgba){ 176, 224, 230, 0 }
#define C_PURPLE &(tt_rgba){ 128, 0, 128, 0 }
#define C_RED &(tt_rgba){ 255, 0, 0, 0 }
#define C_ROSY_BROWN &(tt_rgba){ 188, 143, 143, 0 }
#define C_ROYAL_BLUE &(tt_rgba){ 65, 105, 225, 0 }
#define C_SADDLE_BROWN &(tt_rgba){ 139, 69, 19, 0 }
#define C_SALMON &(tt_rgba){ 250, 128, 114, 0 }
#define C_SANDY_BROWN &(tt_rgba){ 244, 164, 96, 0 }
#define C_SEA_GREEN &(tt_rgba){ 46, 139, 87, 0 }
#define C_SEA_SHELL &(tt_rgba){ 255, 245, 238, 0 }
#define C_SIENNA &(tt_rgba){ 160, 82, 45, 0 }
#define C_SILVER &(tt_rgba){  192, 192, 192, 0 }
#define C_SKY_BLUE &(tt_rgba){ 135, 206, 235 }
#define C_SLATE_BLUE &(tt_rgba){ 106, 90, 205 }
#define C_SLATE_GRAY &(tt_rgba){ 112, 128, 144, 0 }
#define C_SNOW &(tt_rgba){ 255, 250, 250, 0 }
#define C_SPRING_GREEN &(tt_rgba){ 0, 255, 127, 0 }
#define C_STEEL_BLUE &(tt_rgba){  70, 130, 180, 0 }
#define C_TAN &(tt_rgba){ 210, 180, 140, 0 }
#define C_TEAL &(tt_rgba){ 0, 128, 128, 0 }
#define C_THISTLE &(tt_rgba){ 216, 191, 216, 0 }
#define C_TOMATO &(tt_rgba){ 255, 99, 71, 0 }
#define C_TURQUOISE &(tt_rgba){ 64, 224, 208, 0 }
#define C_VIOLET &(tt_rgba){ 238, 130, 238, 0 }
#define C_WHEAT &(tt_rgba){ 245, 222, 179, 0 }
#define C_WHITE &(tt_rgba){ 255, 255, 255, 0 }
#define C_WHITE_SMOKE &(tt_rgba){ 245, 245, 245, 0 }
#define C_YELLOW &(tt_rgba){ 255, 255, 0, 0 }
#define C_YELLOW_GREEN &(tt_rgba){ 154, 205, 50, 0 }

enum ColorComponentOrder
{
    RGBA_ORDER,
    BGRA_ORDER
};

typedef enum ColorComponentOrder ColorComponentOrder;

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

struct tt_rgba
{
    unsigned char R;
    unsigned char G;
    unsigned char B;
    unsigned char A; //not used by the rasterizer, but it's good to provide it here as it may be used for 'outside' purposes
};

typedef struct tt_rgba tt_rgba;

/* _characterIndex is a Unicode codepoint if it's a positive value, and glyph index (within the given font file) if it's a negative value;
  the function is non-validating - if _characterIndex is a Unicode codepoint, then it must be a valid Unicode codepoint and if
  _characterIndex is a glyph index, then it must be an index within the valid for the specific font range */
//_glyph is a Parser::SimpleGlyph or Parser::CompositeGlyph object; if this parameter is used, then _characterIndex is ignored
//_canvas is (a RGBA or BGRA pixel array) in which the character is drawn
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
//the last 4 variables have to be set to 0.0
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
    int _callingMode,
    double _composite_X_Offset, //(INTERNAL)
    double _composite_Y_Offset, //(INTERNAL)
    double _composite_X_Scale, //(INTERNAL)
    double _composite_Y_Scale); //(INTERNAL)

//_canvas is (a RGBA or BGRA pixel array) in which the character is drawn
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
        int _maxGraphemicX);

//returns the width of the string in pixels (with the left-side bearing of the first character and the right-side bearing of the last character)
double GetTypographicWidth(const Font* _font, const wchar_t* _string, double _fontSize);

//returns the width of the string in pixels (without the left-side bearing of the first character and the right-side bearing of the last character)
//_fontSize is specified in pixels
//_string.length() >= 1 ->
double GetGraphemicWidth(const Font* _font, const wchar_t* _string, double _fontSize);

//returns the graphemic height of the string in pixels (the distance between the lowest and the highest graphemic point in the string)
//_fontSize is specified in pixels
double GetGraphemicHeight(const Font* _font, const wchar_t* _string, double _fontSize);

///UNDERLINE FUNCTIONALITY

extern tt_rgba* UnderlineColor;
extern int UnderlineThickness; //in pixels
extern int UnderlinePosition; //in pixels under the baseline

#endif

#ifndef TT_RASTERIZER_C_PARSER_H
#define TT_RASTERIZER_C_PARSER_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#define CMAP_TABLE_T 24
#define GLYF_TABLE_T 25
#define HEAD_TABLE_T 26
#define HHEA_TABLE_T 27
#define HMTX_TABLE_T 28
#define LOCA_TABLE_T 29
#define KERN_TABLE_T 30
#define MAXP_TABLE_T 31
#define OS2_TABLE_T 32
#define VHEA_TABLE_T 33
#define CMAP_SUBTABLE_FORMAT0_T 50
#define CMAP_SUBTABLE_FORMAT4_T 51
#define CMAP_SUBTABLE_FORMAT6_T 52
#define CMAP_SUBTABLE_FORMAT12_T 53
#define KERN_SUBTABLE_FORMAT0_T 54
#define EMPTY_GLYPH_T 100
#define SIMPLE_GLYPH_T 101
#define COMPOSITE_GLYPH_T 102

struct KerningPair
{
    unsigned short Left;
    unsigned short Right;
    short Value;
};

typedef struct KerningPair KerningPair;

struct CMAP_Table
{
    unsigned int Typograph; //(INTERNAL-CONSTANT)
    unsigned short Version;
    unsigned short NumberOfSubtables;
    void** Subtables;
};

typedef struct CMAP_Table CMAP_Table;

struct SimpleGlyph
{
    unsigned int Typograph; //(INTERNAL-CONSTANT)
    short NumberOfContours;
    short MinX;
    short MinY;
    short MaxX;
    short MaxY;
    unsigned short* EndPointsOfContours; //indexes specifying the last point of every contour
    unsigned char* Flags;
    short* X_Coordinates;
    short* Y_Coordinates;
    unsigned short NumberOfPoints; //= EndPointsOfContours[NumberOfContours - 1] + 1
};

typedef struct SimpleGlyph SimpleGlyph;

//representing a reference to a simple or composite glyph
struct GlyphComponent
{
    unsigned short Flags;
    unsigned short GlyphIndex;
    int Argument1;
    int Argument2;
    int ArgumentMode; /*
| 1 :: offset relative to own coordinates
| 0 :: Argument1 is an index to a point in the container glyph, and Argument2 is an index to a point in this component */
    //Flags:Scale == true -> [0] | Flags:X_AND_Y_SCALE == true -> [0], [1] | Flags:TWO_BY_TWO_TRANSFORMATION -> [0], [1], [2], [3]
    unsigned short Scale[4];
    bool UseMetrics; //if this is set, then the specified in this glyph advance-width and left-side-bearing are used for the composite
};

typedef struct GlyphComponent GlyphComponent;

struct CompositeGlyph
{
    unsigned int Typograph; //(INTERNAL-CONSTANT)
    short MinX;
    short MinY;
    short MaxX;
    short MaxY;
    GlyphComponent** Components; //[GlyphComponent]
    unsigned short NumberOfComponents;
};

typedef struct CompositeGlyph CompositeGlyph;


struct GLYF_Table
{
    unsigned int Typograph; //(INTERNAL-CONSTANT)
    short NumberOfContours;
    void** Glyphs; //[SimpleGlyph & CompositeGlyph]
    short NumberOfGlyphs;
    int X_Min;
    int Y_Min;
    int X_Max;
    int Y_Max;
};

typedef struct GLYF_Table GLYF_Table;


struct HEAD_Table
{
    unsigned int Typograph; //(INTERNAL-CONSTANT)
    unsigned short MajorVersion;
    unsigned short MinorVersion;
    unsigned short FontMajorRevision;
    unsigned short FontMinorRevision;
    unsigned int ChecksumAdjacement;
    unsigned int MagicNumber;
    unsigned short Flags;
    unsigned short UnitsPerEm;
    long long Created;
    long long Modified;
    short MinX;
    short MinY;
    short MaxX;
    short MaxY;
    unsigned short MacStyle;
    unsigned short LowestRecPPEM;
    short FontDirectionHint;
    short IndexToLocationFormat;
    short GlyphDataFormat;
};

typedef struct HEAD_Table HEAD_Table;


struct HHEA_Table
{
    unsigned int Typograph; //(INTERNAL-CONSTANT)
    unsigned short MajorVersion;
    unsigned short MinorVersion;
    short Ascender;
    short Descender;
    short LineGap;
    unsigned short AdvancedWidthMax;
    short MinLeftSideBearing;
    short MinRightSideBearing;
    short X_MaxExtent;
    short CaretSlopeRise;
    short CaretSlopeRun;
    short CaretOffset;
    short MetricDataFormat;
    unsigned short NumberOfHorizontalMetrics;
};

typedef struct HHEA_Table HHEA_Table;

struct KERN_Table
{
    unsigned int Typograph; //(INTERNAL-CONSTANT)
    unsigned short Version;
    unsigned short NumberOfSubtables;
    void** Subtables;
};

typedef struct KERN_Table KERN_Table;

struct VHEA_Table
{
    unsigned int Typograph; //(INTERNAL-CONSTANT)
};

typedef struct VHEA_Table VHEA_Table;

struct LongHorizontalMetric
{
    unsigned short AdvanceWidth;
    short LeftSideBearing;
};

typedef struct LongHorizontalMetric LongHorizontalMetric;

struct HMTX_Table
{
    unsigned int Typograph; //(INTERNAL-CONSTANT)
    LongHorizontalMetric* HorizontalMetrics;
    short* LeftSideBearings;
};

typedef struct HMTX_Table HMTX_Table;

struct LOCA_Table
{
    unsigned int Typograph; //(INTERNAL-CONSTANT)
    void* Offsets; //[uint16] | [uint32]
    int ArrayType; //0 :: [uint16] | 1 :: [uint32]
};

typedef struct LOCA_Table LOCA_Table;

struct MAXP_Table
{
    unsigned int Typograph; //(INTERNAL-CONSTANT)
    unsigned short MajorVersion;
    unsigned short MinorVersion;
    unsigned short NumberOfGlyphs;
    unsigned short MaxPoints;
    unsigned short MaxContours;
    unsigned short MaxCompositePoints;
    unsigned short MaxCompositeContours;
    unsigned short MaxZones;
    unsigned short MaxTwilightPoints;
    unsigned short MaxStorage;
    unsigned short MaxFunctionDefinitions;
    unsigned short MaxInstructionDefinitions;
    unsigned short MaxStackElements;
    unsigned short MaxSizeOfInstructions;
    unsigned short MaxComponentElements;
    unsigned short MaxComponentDepth;
};

typedef struct MAXP_Table MAXP_Table;


struct OS2_Table
{
    unsigned int Typograph; //(INTERNAL-CONSTANT)
    unsigned short Version;
    short X_AverageCharacterWidth;
    unsigned short US_WeightClass;
    unsigned short US_WidthClass;
    unsigned short FS_Type;
    short Y_SubscriptXSize;
    short Y_SubscriptYSize;
    short Y_SubscriptXOffset;
    short Y_SubscriptYOffset;
    short Y_SuperscriptXSize;
    short Y_SuperscriptYSize;
    short Y_SuperscriptXOffset;
    short Y_SuperscriptYOffset;
    short Y_StrikeoutSize;
    short Y_StrikeoutPosition;
    short S_FamilyClass;
    unsigned char Panose[10];
    unsigned int UL_UnicodeRange[4];
    unsigned char VendorID[4];
    unsigned short FS_Selection;
    unsigned short US_FirstCharIndex;
    unsigned short US_LastCharIndex;
    short S_TypographicAscender;
    short S_TypographicDescender;
    short S_TypographicLineGap;
    unsigned short US_WinAscent;
    unsigned short US_WinDescent;
    //additional fields for version 1
    unsigned int UL_CodePageRange1; //bits 0..31
    unsigned int UL_CodePageRange2; //bits 32..63
    //additional fields for version 2 (versions 3 and 4 have the same fields)
    short SX_Height;
    short S_CapHeight;
    unsigned short US_DefaultChar;
    unsigned short US_BreakChar;
    unsigned short US_MaxContext;
    //additional fields for version 5
    unsigned short US_LowerOpticalPointSize;
    unsigned short US_UpperOpticalPointSize;
};

typedef struct OS2_Table OS2_Table;

struct Font
{
    int SFNT_VERSION;
    int NumberOfTables;
    void** Tables;
};

typedef struct Font Font;

void* GetTable(const Font* _font, short _identifier);

//_file is a valid file object ->
Font* ParseFont(FILE* _file);

//(the glyph corresponding to the specified codepoint) does not exist in the file => -1
int GetGlyphIndex(const Font* _font, int _codepoint);

//the codepoint does not exist in the file => NULL
//_characterIndex < 0 :: index in the table glyf | _characterIndex >= 0 :: codepoint
void* GetGlyph(const Font* _font, int _characterIndex);

//the return value is in Funit-s
/* _characterIndex is a Unicode codepoint if it's a positive value, and glyph index (within the given font file) if it's a negative value;
   the function is non-validating - if _characterIndex is a Unicode codepoint, then it must be a valid Unicode codepoint and if
   _characterIndex is a glyph index, then it must be an index within the valid for the specific font range */
//the specified character (codepoint) exists in the file  ->
int GetLeftSideBearing(const Font* _font, int _characterCode);

//the return value is in Funit-s
//the specified character (codepoint) exists in the glyph ->
int GetRightSideBearing(const Font* _font, int _codepoint);

//returns the distance (in Funit-s) from the (baseline) to (the highest graphemic point of the character)
//returns negative value if the highest graphemic point of the character is below the baseline
//returns INT_MAX if the _codepoint is empty symbol
//_fontSize is specified in pixels
double GetCodepointAscent(const Font* _font, int _codepoint);

//returns the distance (in Funit-s) from the (baseline) to (the lowest graphemic point of the string)
//returns positive value if the lowest graphemic point of the string is above the baseline
//returns INT_MAX if the _codepoint represents an empty glyph
//_fontSize is specified in pixels
double GetCodepointDescent(const Font* _font, int _codepoint);

//returns the distance (in Funit-s) from the (baseline) to (the highest graphemic point of the string)
//returns negative value if the highest graphemic point of the string is below the baseline
//_fontSize is specified in pixels
//_stringLength is in characters
double GetAscent(const Font* _font, const wchar_t* _string, int _stringLength);

//returns the distance (in Funit-s) from the (baseline) to (the lowest graphemic point of the string)
//returns positive value if the lowest graphemic point of the string is above the baseline
//_fontSize is specified in pixels
//_stringLength is in characters
double GetDescent(const Font* _font, const wchar_t* _string, int _stringLength);

//the return values is in Funit-s
/* _characterIndex is a Unicode codepoint if it's a positive value, and glyph index (within the given font file) if it's a negative value;
   the function is non-validating - if _characterIndex is a Unicode codepoint, then it must be a valid Unicode codepoint and if
   _characterIndex is a glyph index, then it must be an index within the valid for the specific font range */
//the specified character (codepoint) exists in the file ->
int GetAdvanceWidth(const Font* _font, int _characterCode);

//the return value is in Funit-s
//the specified kerning-pair does not exist in the file => INT_MIN
//the specified character (codepoint) does not exist in the file ->
int GetKerning(const Font* _font, int _codepoint1, int _codepoint2);

bool ContainsGlyph(const Font* _font, int _codepoint);

void ReleaseFont(Font* _font);

#endif

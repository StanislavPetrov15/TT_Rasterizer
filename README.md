GENERAL NOTES
    
  - the library does not depend on third-party code; it depends only on few functions in the C standard library

  - it's C files only, no headers for now as i don't use them in my personal projects 

  - software rasterizer (it does not use hardware acceleration) 

  - it implements gray-scale anti-aliasing

  - the characters/string are 'drawn' in a byte array which then has to be visualized using (low-level graphic API's
    such as GDI, GDI+, Direct2D, etc) or (high-level graphic libraries such as Cairo, SDL, etc)

  - the bitmap in which the characters/strings are drawn can be:
    - RGBA (32bpp)
    - BGRA (32bpp)

  - it uses bottom-up coordinate system, i.e. Y coordinate 0 is the bottom row of the canvas

  - the position of the characters/strings is specified by using real numbers

  - the size of the characters is specified in pixels - the size specifies the line height

  - for now it does not support:
    - font collection files
    - right-to-left languages scripts (i.e. Hebrew, Arabic, Syriac, Persian, Uighur, Urdu, etc)
    - vertical languages/scripts
    - color emojis
    - character/string rotation (it will be implemented in near future)
    - hinting (highly unlikely that it will be implemented in future versions)
    - variable fonts (highly unlikely that it will be implemented in future versions)

  - the library is not intended for visualizing characters with size (i.e. line height) lower than 12px (atleast for now)

  - the library does not validate the font files - it should be used only on trusted font files

  - the library is tested on Windows only, but since it does not have any dependencies it should produce the same results
     on any other system that can produce C99

  - a character or string is positioned at the baseline; for example if a character/string has Y set to 109, then the baseline of
    the character/string has vertical position 109 (row with index 109) in the canvas

  - problems:
    - (MAJOR) performance-wise it's quite slow - it can draw (about ~1500 'R' characters with size ~30px) per second on а low-end machine,
          which is atleast few times slower than stb_truetype for example; performance will be addressed in the near future;
          next version will have support for threads (maybe) and pre-computed characters/strings so it should be considerably faster
    - (MAJOR) cannot properly visualize characters with self-intersecting contours (for example Unicode codepoint Dx181 in Castellar (typeface) or
      Unicode codepoint Dx198 in Viner Hand (typeface)); most likely this will be addressed in the next version
    - (MINOR) the library can potentially use a lot of memory if the visualized character is very large:
      - for example it uses (~4MB if the character is 1000x1000px) or (~16MB if the character is 2000x2000px)
        (*) for most real-world characters it will be no more than 1MB, as they will be much smaller
      - also the library makes a copy of the currently used font in the memory; very few fonts are very big though (over 10 MB)

PUBLIC API

        Font* ParseFont(FILE* _file)

        void* GetTable(const Font* _font, short _identifier)
        
        int GetGlyphIndex(const Font* _font, int _codepoint)

        void* GetGlyph(const Font* _font, int _characterIndex)
           
        int GetLeftSideBearing(const Font* _font, int _characterCode)
        
        int GetRightSideBearing(const Font* _font, int _codepoint)
        
        double GetAscent(const Font* _font, const wchar_t* _string)
        
        double GetDescent(const Font* _font, const wchar_t* _string)
        
        int GetAdvanceWidth(const Font* _font, int _characterCode)
        
        int GetKerning(const Font* _font, int _codepoint1, int _codepoint2)

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

        double GetGraphemicWidth(const Font* _font, const wchar_t* _string, double _fontSize)
   
        double GetGraphemicHeight(const Font* _font, const wchar_t* _string, double _fontSize)

        double GetTypographicWidth(const Font* _font, const wchar_t* _string, double _fontSize)
   
TERMINOLOGY 

   - (segment) :: segment of a contour - a line, Bezier curve or Bezier spline
   - (segmentoid) :: begin/end pixel of a segment
   - (segmentonom) :: intermediate pixel (between two segmentoids)
   - (vertexoid) :: the begin/end point a segment
   - (conturoid) :: a pixel that is crossed by a contour, i.e. a segmentoid or a segmentonom; it's really "conturoid", not "contouroid", it's not a typo
   - (exteroid) :: exterior pixel; a pixel that is not part of the currently drawn character
   - (interoid) :: interior pixel; a pixel that is a part of the currently drawn character
   - (f-conturoid) :: a conturoid that is part of filled contour
   - (n-conturoid) :: a conturoid that is part of non-filled contour
   - (delta-point | delta) :: a point that traverses the glyph contour
   - (samplex) :: a position of a delta-point at given moment in time
   - (entering samplex) :: the first samplex (chronologically) in a pixel, i.e. the first delta-value for the particular pixel
   - (exiting samplex) :: the last samplex (chronologically) in a pixel, i.e. the last delta-value for the particular pixel
   - (terminating segmentonom) :: a segmentonom that is the last pixel in a fill-sequence
   - (terminating segmentoid) :: a segmentoid that is the last pixel in a fill-sequence
   - (corner-crossing) :: crossing in which delta only crosses the corner region, i.e. a crossing in which there is only samplex -
       this samplex is always in one of the corners of the pixel
   - (pixeloid) :: 'vertex' of a pixel 
   - (fully-crossed pixel) :: a pixel that is traversed from one side to the side parallel to it (from left to right, bottom to top, etc)
   - (typographic width) ::
       (1) when applied to a single character, it means the width of the character with (the left-side bearing) and (the right-side bearing)
       (2) when applied to a character string, it means the width of the string with (the left-side bearing of the first character)
           and (the right-side bearing of the last character)
   - (graphemic pixels) :: the pixels of (a contour or a glyph) that are either contour or interior pixels
   - (graphemic width) ::
       (1) when applied to a single character, it means the width of the character without (the left-side bearing) and (the right-side bearing),
           i.e. the distance between the left-most and right-most graphemic points of the character
       (2) when applied to a character string, it means the width of the string without (the left-side bearing of the first character)
           and (the right-side bearing of the last character), i.e. the distance between the left-most and right-most graphemic points of the string
   - (graphemic height) ::
       (1) when applied to a single character, it means the distance between the highest and lowest point of the character
       (2) when applied to a character string, it means the distance between the highest and lowest graphemic points of the string
   - (graphema) all the graphemic pixels for (a contour or a glyph), i.e. the visual representation of (a contour or a glyph) on the screen



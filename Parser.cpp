namespace TT
{
    //supported tables - cmap, glyf, head, hhea, hmtx, kern, loca, maxp and OS/2

    /* typograph format:
      - bits 0..23 :: base-type flags
      - bits 24..31 :: real-type identifier (a value between 24 and 256) */

    //(PUBLIC)

    //identifiers of the real types in their typographs; used by Is() and GetTable()
    const short CMAP_TABLE = 24;
    const short GLYF_TABLE = 25;
    const short HEAD_TABLE = 26;
    const short HHEA_TABLE = 27;
    const short HMTX_TABLE = 28;
    const short LOCA_TABLE = 29;
    const short KERN_TABLE = 30;
    const short MAXP_TABLE = 31;
    const short OS2_TABLE = 32;
    const short VHEA_TABLE = 33;
    const short CMAP_SUBTABLE_FORMAT0 = 50;
    const short CMAP_SUBTABLE_FORMAT4 = 51;
    const short CMAP_SUBTABLE_FORMAT6 = 52;
    const short CMAP_SUBTABLE_FORMAT12 = 53;
    //cmap-subtable-format-2
    //cmap_subtable-format-8
    //cmap-subtаble-format-10
    //cmap-subtable-format-13
    //cmap-subtable-format-14
    const short EMPTY_GLYPH = 100;
    const short SIMPLE_GLYPH = 101;
    const short COMPOSITE_GLYPH = 102;

    //(PRIVATE)
    struct CMAP_Subtable_Format0
    {
        unsigned int Typograph = 0b00110010'00000000'00000000'00000000; //(INTERNAL)
        unsigned short Length; //length of the table in bytes
        unsigned short Language;
        unsigned char GlyphIndexArray[256];
    };

    //(PRIVATE)
    struct CMAP_Subtable_Format4
    {
        unsigned int Typograph = 0b00110011'00000000'00000000'00000000; //(INTERNAL)
        unsigned short Length; //length of the table in bytes
        unsigned short Language;
        unsigned short SegmentCountX2;
        unsigned short SearchRange;
        unsigned short EntrySelector;
        unsigned short RangeShift;
        unsigned short* EndCode; //length = SegmentCount; end character code for every segment last=0xFFFF
        unsigned short ReservedPad = 0;
        unsigned short* StartCode; //length = SegmentCount; begin character code for every segment
        unsigned short* IndexDelta; //length = SegmentCount; delta for every character in a segment
        unsigned short* IndexRangeOffsets;
        unsigned short* GlyphIndexArray;

        ~CMAP_Subtable_Format4()
        {
            delete [] EndCode;
            delete [] StartCode;
            delete [] IndexDelta;
            delete [] IndexRangeOffsets;
            delete [] GlyphIndexArray;
        }
    };

    //(PRIVATE)
    struct CMAP_Subtable_Format6
    {
        unsigned int Typograph = 0b00110100'00000000'00000000'00000000; //(INTERNAL)
        unsigned short Length; //length of the table in bytes
        unsigned short Language ; //(SEE LANGUAGE_USE)
        unsigned short FirstCode; //begin character code in the range
        unsigned short EntryCount; //number of the character codes in the range
        unsigned short* GlyphIndexArray;

        ~CMAP_Subtable_Format6()
        {
            delete [] GlyphIndexArray;
        }
    };

    //(PRIVATE)
    struct SequentialMapGroup
    {
        unsigned int StartCharacterCode; //first character code in the group
        unsigned int EndCharacterCode; //last character code in the group
        unsigned int StartGlyphIndex; //глифов индекс съответстващ на началния символен код
    };

    //(PRIVATE)
    struct CMAP_Subtable_Format12
    {
        unsigned int Typograph = 0b00110101'00000000'00000000'00000000; //(INTERNAL)
        unsigned int Length; //length of the table in bytes
        unsigned int Language; //(SEE LANGUAGE_USE)
        unsigned int NumberOfGroups;
        SequentialMapGroup** Groups;

        ~CMAP_Subtable_Format12()
        {
            for (int i = 0; i < NumberOfGroups; i++)
            {
                delete Groups[i];
            }

            delete [] Groups;
        }
    };

    //(PUBLIC)
    struct KerningPair
    {
        unsigned short Left;
        unsigned short Right;
        short Value;
    };

    //(PRIVATE)
    struct KERN_Subtable_Format0
    {
        unsigned int Typograph = 0b00110110'00000000'00000000'00000000; //(INTERNAL)
        unsigned int Version;
        unsigned short Length;
        unsigned short Coverage;
        unsigned short NumberOfPairs;
        unsigned short SearchRange;
        unsigned short EntrySelector;
        unsigned short RangeShift;
        KerningPair* Pairs;

        ~KERN_Subtable_Format0()
        {
            delete [] Pairs;
        }
    };

    //(PUBLIC)
    struct CMAP_Table
    {
        unsigned int Typograph = 0b00011000'00000000'00000000'00000000; //(INTERNAL)
        unsigned short Version;
        unsigned short NumberOfSubtables;
        void** Subtables;

        ~CMAP_Table()
        {
            for (int i = 0; i < NumberOfSubtables; i++)
            {
                delete Subtables[i];
            }

            delete [] Subtables;
        }
    };

    //(PUBLIC)
    struct EmptyGlyph
    {
        unsigned int Typograph = 0b01100100'00000000'00000000'00000000; //(INTERNAL)
    };

    //(PUBLIC)
    struct SimpleGlyph
    {
        unsigned int Typograph = 0b01100101'00000000'00000000'00000000; //(INTERNAL)
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

        ~SimpleGlyph()
        {
             delete [] EndPointsOfContours;
             delete [] X_Coordinates;
             delete [] Y_Coordinates;
             delete [] Flags;
        }
    };

    //(PUBLIC)
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
        bool UseMetrics = false; //if this is set, then the specified in this glyph advance-width and left-side-bearing are used for the composite

        GlyphComponent()
        {
            Scale[0] = 0;
            Scale[1] = 0;
            Scale[2] = 0;
            Scale[3] = 0;
        }
    };

    //(PUBLIC)
    struct CompositeGlyph
    {
         unsigned int Typograph = 0b01100110'00000000'00000000'00000000; //(INTERNAL)
         short MinX;
         short MinY;
         short MaxX;
         short MaxY;
         GlyphComponent** Components; //[GlyphComponent]
         unsigned short NumberOfComponents;

         ~CompositeGlyph()
         {
               for (int i = 0; i < NumberOfComponents; i++)
               {
                    delete Components[i];
               }

               delete [] Components;
         }
    };

    //(PUBLIC)
    struct GLYF_Table
    {
        unsigned int Typograph = 0b00011001'00000000'00000000'00000000; //(INTERNAL)
        short NumberOfContours;
        void** Glyphs; //[SimpleGlyph & CompositeGlyph]
        short NumberOfGlyphs;
        int X_Min;
        int Y_Min;
        int X_Max;
        int Y_Max;

        ~GLYF_Table()
        {
            for (int i = 0; i < NumberOfGlyphs; i++)
            {
                delete Glyphs[i];
            }

            delete [] Glyphs;
        }
    };

    //(PUBLIC)
    struct HEAD_Table
    {
        unsigned int Typograph = 0b00011010'00000000'00000000'00000000; //(INTERNAL)
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

    //(PUBLIC)
    struct HHEA_Table
    {
        unsigned int Typograph = 0b00011011'00000000'00000000'00000000; //(INTERNAL)
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

    //(PUBLIC)
    struct VHEA_Table
    {
        unsigned int Typograph = 0b00100001'00000000'00000000'00000000; //(INTERNAL)
    };

    //(PUBLIC)
    struct KERN_Table
    {
        unsigned int Typograph = 0b00011110'00000000'00000000'00000000; //(INTERNAL)
        unsigned short Version;
        unsigned short NumberOfSubtables;
        void** Subtables;

        ~KERN_Table()
        {
            for (int i = 0; i < NumberOfSubtables; i++)
            {
                delete Subtables[i];
            }

            delete [] Subtables;
        }
    };

    //(PUBLIC)
    struct LongHorizontalMetric
    {
        unsigned short AdvanceWidth;
        short LeftSideBearing;
    };

    //(PUBLIC)
    struct HMTX_Table
    {
        unsigned int Typograph = 0b00011100'00000000'00000000'00000000; //(INTERNAL)
        LongHorizontalMetric* HorizontalMetrics;
        short* LeftSideBearings;

        ~HMTX_Table()
        {
            delete [] HorizontalMetrics;
            delete [] LeftSideBearings;
        }
    };

    //(PUBLIC)
    struct LOCA_Table
    {
        unsigned int Typograph = 0b00011101'00000000'00000000'00000000; //(INTERNAL)
        void* Offsets; //[uint16] | [uint32]
        int ArrayType; //0 :: [uint16] | 1 :: [uint32]

        ~LOCA_Table()
        {
            delete [] Offsets;
        }
    };

    //(PUBLIC)
    struct MAXP_Table
    {
        unsigned int Typograph = 0b00011111'00000000'00000000'00000000; //(INTERNAL)
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

    //(PUBLIC)
    struct OS2_Table
    {
        unsigned int Typograph = 0b00100000'00000000'00000000'00000000; //(INTERNAL)
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

    //(PUBLIC)
    struct Font
    {
        int SFNT_VERSION;
        int NumberOfTables;
        void** Tables;

        ~Font()
        {
            for (int i = 0; i < NumberOfTables; i++)
            {
                delete Tables[i];
            }

            delete [] Tables;
        }
    };

    //(PRIVATE)
    bool Is(const void* _derived, int _typeIdentifier)
    {
        return ((*(unsigned int*)_derived) >> 24) == _typeIdentifier;
    }

    //_index >= 0 || _index <= 31 ->
    bool GetBit(unsigned int _number, int _index)
    {
        _number = _number << (31 - _index);
        _number = _number >> 31;
        return _number;
    }

    //(PRIVATE)
    unsigned char ReadI8(FILE* _file)
    {
        return fgetc(_file);
    }

    //(PRIVATE)
    unsigned short ReadI16(FILE* _file)
    {
        unsigned char byte1 = fgetc(_file);
        unsigned char byte2 = fgetc(_file);
        return byte2 | (byte1 << 8);
    }

    //(PRIVATE)
    unsigned int ReadI32(FILE* _file)
    {
        unsigned char byte1 = fgetc(_file);
        unsigned char byte2 = fgetc(_file);
        unsigned char byte3 = fgetc(_file);
        unsigned char byte4 = fgetc(_file);
        return (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4;
    }

    //(PRIVATE)
    unsigned long long ReadI64(FILE* _file)
    {
        unsigned char byte1 = fgetc(_file);
        unsigned char byte2 = fgetc(_file);
        unsigned char byte3 = fgetc(_file);
        unsigned char byte4 = fgetc(_file);
        unsigned char byte5 = fgetc(_file);
        unsigned char byte6 = fgetc(_file);
        unsigned char byte7 = fgetc(_file);
        unsigned char byte8 = fgetc(_file);
        return (static_cast<unsigned long long>(byte1) << 56) |
               (static_cast<unsigned long long>(byte2) << 48) |
               (static_cast<unsigned long long>(byte3) << 40) |
               (static_cast<unsigned long long>(byte4) << 32) |
               (static_cast<unsigned long long>(byte5) << 24) |
               (static_cast<unsigned long long>(byte6) << 16) |
               (static_cast<unsigned long long>(byte7) << 8) |
               static_cast<unsigned long long>(byte8);
    }

    //(PRIVATE)
    void* ExtractSimpleGlyph(FILE* _file, short _numberOfContours)
    {
        SimpleGlyph* glyph = new SimpleGlyph;

        glyph->NumberOfContours = _numberOfContours;
        glyph->MinX = ReadI16(_file);
        glyph->MinY = ReadI16(_file);
        glyph->MaxX = ReadI16(_file);
        glyph->MaxY = ReadI16(_file);
        glyph->NumberOfContours = _numberOfContours;

        glyph->EndPointsOfContours = new unsigned short[_numberOfContours];
        for (int i = 0; i < _numberOfContours; i++)
            glyph->EndPointsOfContours[i] = ReadI16(_file);

        int instructionLength = ReadI16(_file);

        //ignoring the instructions
        fseek(_file, ftell(_file) + instructionLength, SEEK_SET);

        ///reading the flags

        //total number of points for every contour in the glyph
        int numberOfPointsInContours = glyph->EndPointsOfContours[_numberOfContours - 1] + 1;

        glyph->NumberOfPoints = numberOfPointsInContours;

        int numberOfFlags = 0;

        glyph->Flags = new unsigned char[numberOfPointsInContours];

        while (true)
        {
            unsigned char flags = ReadI8(_file);
            glyph->Flags[numberOfFlags++] = flags;

            //if the flag is repeated
            if (GetBit(flags, 3) == true)
            {
                unsigned char numberOfRepetitions = ReadI8(_file);

                //repeating the flag
                for (int m = 0; m < numberOfRepetitions; m++)
                {
                    glyph->Flags[numberOfFlags++] = flags;
                }
            }

            if (numberOfFlags == numberOfPointsInContours) break;
        }

        ///reading the X-coordinates

        glyph->X_Coordinates = new short[numberOfPointsInContours];

        for (int n = 0; n < numberOfPointsInContours; n++)
        {
            //if the X-coordinate is 1 byte
            if (GetBit(glyph->Flags[n], 1) == true)
            {
                unsigned char value = ReadI8(_file);

                if (GetBit(glyph->Flags[n], 4) == false)
                {
                    glyph->X_Coordinates[n] = (n == 0 ? 0 : glyph->X_Coordinates[n - 1]) - value;
                }
                else
                {
                    glyph->X_Coordinates[n] = (n == 0 ? 0 : glyph->X_Coordinates[n - 1]) + value;
                }
            }
            //(STATE) the X-coordinate is 2 bytes
            else
            {
                if (GetBit(glyph->Flags[n], 4) == true)
                {
                    glyph->X_Coordinates[n] = n == 0 ? 0 : glyph->X_Coordinates[n - 1];
                }
                else
                {
                    glyph->X_Coordinates[n] = (n == 0 ? 0 : glyph->X_Coordinates[n - 1]) + ReadI16(_file);
                }
            }
        }

        //reading the Y-coordinates

        glyph->Y_Coordinates = new short[numberOfPointsInContours];

        for (int n = 0; n < numberOfPointsInContours; n++)
        {
            //if the Y-coordinate is 1 byte
            if (GetBit(glyph->Flags[n], 2) == true)
            {
                unsigned char value = ReadI8(_file);

                if (GetBit(glyph->Flags[n], 5) == false)
                {
                    glyph->Y_Coordinates[n] = (n == 0 ? 0 : glyph->Y_Coordinates[n - 1]) - value;
                }
                else
                {
                    glyph->Y_Coordinates[n] = (n == 0 ? 0 : glyph->Y_Coordinates[n - 1]) + value;
                }
            }
            //(STATE) if Y-coordinate is 2 bytes
            else
            {
                if (GetBit(glyph->Flags[n], 5) == true)
                {
                    glyph->Y_Coordinates[n] = n == 0 ? 0 : glyph->Y_Coordinates[n - 1];
                }
                else
                {
                    glyph->Y_Coordinates[n] = (n == 0 ? 0 : glyph->Y_Coordinates[n - 1]) + ReadI16(_file);
                }
            }
        }

        return reinterpret_cast<void*>(glyph);
    }

    //(LOCAL-TO extend)
    //it copies segment (from _source to _destination) (beginning at_sourceBegin and ending at _sourceEnd)
    void copyRange(GlyphComponent* _source, GlyphComponent* _destination, int _sourceLength, int _destinationLength,
                   int _sourceBegin, int _sourceEnd, int _destinationBegin)
    {
        for (int n = 0; ; n++)
        {
            if (_destinationBegin + n == _destinationLength)
            {
                break;
            }
            else if (_sourceBegin + n == _sourceLength)
            {
                break;
            }
            else if (_sourceBegin + n > _sourceEnd)
            {
                break;
            }

            _destination[_destinationBegin + n] = _source[_sourceBegin + n];
        }
    }

    //(LOCAL-TO ExtractCompositeGlyph)
    //_length specifies the length of _array
    //extend([1, 2, 3, 4, 5], 5, 2) >> [1, 2, 3, 4, 5, x, x]
    void extend(GlyphComponent* _array, int _length, int _extensor)
    {
        int newLength = _length + _extensor;

        GlyphComponent* newArray = new GlyphComponent[newLength];

        copyRange(_array, newArray, _length, newLength, 0, _length - 1, 0);

        free(_array);

        _array = newArray;
    }

    //(PRIVATE)
    //(RECURSIVE)
    void* ExtractCompositeGlyph(FILE* _file)
    {
        CompositeGlyph* glyph = new CompositeGlyph;

        glyph->MinX = ReadI16(_file);
        glyph->MinY = ReadI16(_file);
        glyph->MaxX = ReadI16(_file);
        glyph->MaxY = ReadI16(_file);

        int size = 20;
        GlyphComponent** components = new GlyphComponent*[size];
        int componentCount = 0;

        //while there are more sub-glyphs
        while (true)
        {
            GlyphComponent* component = new GlyphComponent;
            component->Flags = ReadI16(_file);
            component->GlyphIndex = ReadI16(_file);

            bool flags_bit_0 = GetBit(component->Flags, 0);
            bool flags_bit_1 = GetBit(component->Flags, 1);

            component->ArgumentMode = flags_bit_1;

            if (flags_bit_0 == false && flags_bit_1 == false)
            {
                component->Argument1 = ReadI8(_file);
                component->Argument2 = ReadI8(_file);
            }
            else if (flags_bit_0 == false && flags_bit_1 == true)
            {
                component->Argument1 = static_cast<char>(ReadI8(_file));
                component->Argument2 = static_cast<char>(ReadI8(_file));
            }
            else if (flags_bit_0 == true && flags_bit_1 == false)
            {
                component->Argument1 = ReadI16(_file);
                component->Argument2 = ReadI16(_file);
            }
            else if (flags_bit_0 == true && flags_bit_1 == true)
            {
                component->Argument1 = static_cast<short>(ReadI16(_file));
                component->Argument2 = static_cast<short>(ReadI16(_file));
            }

            //transformation fields

            //'simple' scaling (SOURCE:MICROSOFT)
            if (GetBit(component->Flags, 3) == true)
            {
                component->Scale[0] = ReadI16(_file);
            }
            //X and Y scaling
            else if (GetBit(component->Flags, 6) == true)
            {
                component->Scale[0] = ReadI16(_file);
                component->Scale[1] = ReadI16(_file);
            }
            //2x2 scaling
            else if (GetBit(component->Flags, 7) == true)
            {
                component->Scale[0] = ReadI16(_file);
                component->Scale[1] = ReadI16(_file);
                component->Scale[2] = ReadI16(_file);
                component->Scale[3] = ReadI16(_file);
            } //

            component->UseMetrics = GetBit(component->Flags, 9);

            components[componentCount++] = component;

            if (componentCount == size)
            {
                extend(*components, size, size / 2);
            }

            //if there are instructions for &glyph following (if present, they're always located after the last sub-glyph in the parent composite glyph)
            if (GetBit(component->Flags, 8) == true)
            {
                unsigned short numberOfInstructions = ReadI16(_file);

                //ignoring the instructions
                fseek(_file, ftell(_file) + numberOfInstructions, SEEK_SET);
            }

            //(STATE) if there are no more sub-glyphs
            if (GetBit(component->Flags, 5) == false)
            {
                break;
            }
        }

        glyph->Components = new GlyphComponent*[componentCount];

        for (int i = 0; i < componentCount; i++)
        {
        auto rggrg = components[i];
            glyph->Components[i] = components[i];
        }

        glyph->NumberOfComponents = componentCount;

        delete [] components;

        return reinterpret_cast<void*>(glyph);
    }

    //(PUBLIC)
    void* GetTable(const Font* _font, short _identifier)
    {
        for (int i = 0; i < _font->NumberOfTables; i++)
        {
            if (_font->Tables[i] != nullptr && Is(_font->Tables[i], _identifier))
            {
                return _font->Tables[i];
            }
        }

        return nullptr;
    }

    //(PUBLIC)
    //_file is a valid file object ->
    Font* ParseFont(FILE* _file)
    {
        Font* font = new Font;

        //reading the table font-directory

        font->SFNT_VERSION = ReadI32(_file); //65536 :: TrueType contours | 1330926671 :: CFF data
        font->NumberOfTables = ReadI16(_file);
        font->Tables = new void*[font->NumberOfTables];
        for (int i = 0; i < font->NumberOfTables; i++)
            font->Tables[i] = nullptr; //as some tables are not supported, null values have to be added to each table slot

        //position the file at the beginning of the table list
        fseek(_file, 12, SEEK_SET);

        //reading the tables
        for (int i = 0; i < font->NumberOfTables; i++)
        {
            unsigned char tagCharacter1 = ReadI8(_file);
            unsigned char tagCharacter2 = ReadI8(_file);
            unsigned char tagCharacter3 = ReadI8(_file);

            unsigned char tagCharacter4 = ReadI8(_file);
            fseek(_file, ftell(_file) + 4, SEEK_SET); //ignoring the Checksum field
            unsigned int tableOffset = ReadI32(_file);
            fseek(_file, ftell(_file) + 4, SEEK_SET); //ignoring the Length field

            //this variable is needed because a jump (to the header of the next table) has to be performed after extraction of this table
            int tableHeaderBegin = ftell(_file);

            fseek(_file, tableOffset, SEEK_SET);

            if (tagCharacter1 == 'c' && tagCharacter2 == 'm' && tagCharacter3 == 'a' && tagCharacter4 == 'p')
            {
                CMAP_Table* table = new CMAP_Table;

                table->Version = ReadI16(_file);
                table->NumberOfSubtables = ReadI16(_file);
                table->Subtables = new void*[table->NumberOfSubtables];

                for (int n = 0; n < table->NumberOfSubtables; n++)
                {
                    /* (NOTE) formats 8, 10 and 13 are not used often; not supported for now;
                              format 14 (Unicode Variation Sequences) is not so important and it also won't be supported for now */

                    //reading the sub-table descriptor
                    unsigned short platformID = ReadI16(_file);
                    unsigned short encodingID = ReadI16(_file);
                    unsigned int subtableOffset = ReadI32(_file);

                    int nextSubtableDescriptorPosition = ftell(_file);

                    //reading the sub-table

                    fseek(_file, tableOffset + subtableOffset, SEEK_SET);

                    unsigned short tableFormat = ReadI16(_file);

                    if (tableFormat == 0)
                    {
                        CMAP_Subtable_Format0* subtable = new CMAP_Subtable_Format0;
                        subtable->Length = ReadI16(_file);
                        subtable->Language = ReadI16(_file);

                        for (int x = 0; x < 256; x++)
                        {
                            subtable->GlyphIndexArray[x] = ReadI8(_file);
                        }

                        table->Subtables[n] = reinterpret_cast<void*>(subtable);

                        fseek(_file, nextSubtableDescriptorPosition, SEEK_SET);
                    }
                    else if (tableFormat == 4)
                    {
                        CMAP_Subtable_Format4* subtable = new CMAP_Subtable_Format4;
                        subtable->Length = ReadI16(_file);
                        subtable->Language = ReadI16(_file);
                        subtable->SegmentCountX2 = ReadI16(_file);
                        int segmentCount = subtable->SegmentCountX2 / 2;
                        subtable->SearchRange = ReadI16(_file);
                        subtable->EntrySelector = ReadI16(_file);
                        subtable->RangeShift = ReadI16(_file);
                        //
                        subtable->EndCode = new unsigned short[segmentCount];
                        for (int m = 0; m < segmentCount; m++)
                            subtable->EndCode[m] = ReadI16(_file);
                        //
                        subtable->ReservedPad = ReadI16(_file);
                        //
                        subtable->StartCode = new unsigned short[segmentCount];
                        for (int m = 0; m < segmentCount; m++)
                            subtable->StartCode[m] = ReadI16(_file);
                        //
                        subtable->IndexDelta = new unsigned short[segmentCount];
                        for (int m = 0; m < segmentCount; m++)
                            subtable->IndexDelta[m] = ReadI16(_file);
                        //
                        subtable->IndexRangeOffsets = new unsigned short[segmentCount];
                        for (int m = 0; m < segmentCount; m++)
                            subtable->IndexRangeOffsets[m] = ReadI16(_file);
                        //
                        int glyphIndexArrayCount = (subtable->Length -
                              ((16 /*8 2-byte fields*/) + (4 * (segmentCount * 2) /*4 arrays with (2-byte elements) with length = &segmentCount*/))) / 2;
                        subtable->GlyphIndexArray = new unsigned short[glyphIndexArrayCount];
                        for (int m = 0; m < glyphIndexArrayCount; m++)
                        {
                            subtable->GlyphIndexArray[m] = ReadI16(_file);
                        }

                        table->Subtables[n] = reinterpret_cast<void*>(subtable);

                        fseek(_file, nextSubtableDescriptorPosition, SEEK_SET);
                    }
                    else if (tableFormat == 6)
                    {
                        CMAP_Subtable_Format6* subtable = new CMAP_Subtable_Format6;
                        subtable->Length = ReadI16(_file);
                        subtable->Language = ReadI16(_file);
                        subtable->FirstCode = ReadI16(_file);
                        subtable->EntryCount = ReadI16(_file);
                        subtable->GlyphIndexArray = new unsigned short[subtable->EntryCount];
                        for (int m = 0; m < subtable->EntryCount; m++)
                            subtable->GlyphIndexArray[m] = ReadI16(_file);

                        table->Subtables[n] = reinterpret_cast<void*>(subtable);

                        fseek(_file, nextSubtableDescriptorPosition, SEEK_SET);
                    }
                    else if (tableFormat == 12)
                    {
                        CMAP_Subtable_Format12* subtable = new CMAP_Subtable_Format12;
                        subtable->Length = ReadI32(_file);
                        subtable->Language = ReadI32(_file);
                        subtable->NumberOfGroups = ReadI32(_file);
                        subtable->Groups = new SequentialMapGroup*[subtable->NumberOfGroups];

                        for (int m = 0; m < subtable->NumberOfGroups; m++)
                        {
                            SequentialMapGroup* group = new SequentialMapGroup;
                            group->StartCharacterCode = ReadI32(_file);
                            group->EndCharacterCode = ReadI32(_file);
                            group->StartGlyphIndex = ReadI32(_file);
                            subtable->Groups[m] = group;
                        }

                        table->Subtables[n] = reinterpret_cast<void*>(subtable);

                        fseek(_file, nextSubtableDescriptorPosition, SEEK_SET);
                    }
                    else if (tableFormat == 14)
                    {
                        table->Subtables[n] = nullptr;
                        fseek(_file, nextSubtableDescriptorPosition, SEEK_SET);
                    }
                    else
                    {
                        return nullptr;
                    }
                }

                font->Tables[i] = reinterpret_cast<void*>(table);
            }
            else if (tagCharacter1 == 'h' && tagCharacter2 == 'e' && tagCharacter3 == 'a' && tagCharacter4 == 'd')
            {
                HEAD_Table* table = new HEAD_Table;
                table->MajorVersion = ReadI16(_file);
                table->MinorVersion = ReadI16(_file);
                table->FontMajorRevision = ReadI16(_file);
                table->FontMinorRevision = ReadI16(_file);
                table->ChecksumAdjacement = ReadI32(_file);
                table->MagicNumber = ReadI32(_file);
                table->Flags = ReadI16(_file);
                table->UnitsPerEm = ReadI16(_file);
                table->Created = ReadI64(_file);
                table->Modified = ReadI64(_file);
                table->MinX = ReadI16(_file);
                table->MinY = ReadI16(_file);
                table->MaxX = ReadI16(_file);
                table->MaxY = ReadI16(_file);
                table->MacStyle = ReadI16(_file);
                table->LowestRecPPEM = ReadI16(_file);
                table->FontDirectionHint = ReadI16(_file);
                table->IndexToLocationFormat = ReadI16(_file);
                table->GlyphDataFormat = ReadI16(_file);
                font->Tables[i] = reinterpret_cast<void*>(table);
            }
            else if (tagCharacter1 == 'h' && tagCharacter2 == 'h' && tagCharacter3 == 'e' && tagCharacter4 == 'a')
            {
                HHEA_Table* table = new HHEA_Table;
                table->MajorVersion = ReadI16(_file);
                table->MinorVersion = ReadI16(_file);
                table->Ascender = ReadI16(_file);
                table->Descender = ReadI16(_file);
                table->LineGap = ReadI16(_file);
                table->AdvancedWidthMax = ReadI16(_file);
                table->MinLeftSideBearing = ReadI16(_file);
                table->MinRightSideBearing = ReadI16(_file);
                table->X_MaxExtent = ReadI16(_file);
                table->CaretSlopeRise = ReadI16(_file);
                table->CaretSlopeRun = ReadI16(_file);
                table->CaretOffset = ReadI16(_file);
                fseek(_file, ftell(_file) + 8, SEEK_SET); //ignoring a reserved segment
                table->MetricDataFormat = ReadI16(_file);
                table->NumberOfHorizontalMetrics = ReadI16(_file);
                font->Tables[i] = reinterpret_cast<void*>(table);
            }
            else if (tagCharacter1 == 'v' && tagCharacter2 == 'h' && tagCharacter3 == 'e' && tagCharacter4 == 'a')
            {
                font->Tables[i] = reinterpret_cast<void*>(new VHEA_Table);
            }
            else if (tagCharacter1 == 'k' && tagCharacter2 == 'e' && tagCharacter3 == 'r' && tagCharacter4 == 'n')
            {
                //(NOTE) kern-subtable-format-2 (not used in Windows) and it won't be supported for now

                KERN_Table* table = new KERN_Table;
                table->Version = ReadI16(_file);
                table->NumberOfSubtables = ReadI16(_file);
                table->Subtables = new void*[table->NumberOfSubtables];

                //extracting the sub-tables
                for (int i = 0; i < table->NumberOfSubtables; i++)
                {
                    unsigned int version = ReadI16(_file);
                    unsigned int length = ReadI16(_file);
                    unsigned short coverage = ReadI16(_file);

                    //kern-subtable-format-0
                    if (version == 0)
                    {
                        KERN_Subtable_Format0* subtable = new KERN_Subtable_Format0;
                        subtable->Length = length;
                        subtable->Coverage = coverage;
                        subtable->NumberOfPairs = ReadI16(_file);
                        subtable->SearchRange = ReadI16(_file);
                        subtable->EntrySelector = ReadI16(_file);
                        subtable->RangeShift = ReadI16(_file);
                        subtable->Pairs = new KerningPair[subtable->NumberOfPairs];

                        //extracting the kerning-pairs
                        for (int i = 0; i < subtable->NumberOfPairs; i++)
                        {
                            KerningPair pair;
                            pair.Left = ReadI16(_file);
                            pair.Right = ReadI16(_file);
                            pair.Value = ReadI16(_file);
                            subtable->Pairs[i] = pair;
                        }

                        table->Subtables[i] = reinterpret_cast<void*>(subtable);
                    }
                }

                font->Tables[i] = reinterpret_cast<void*>(table);
            }
            else if (tagCharacter1 == 'm' && tagCharacter2 == 'a' && tagCharacter3 == 'x' && tagCharacter4 == 'p')
            {
                MAXP_Table* table = new MAXP_Table;
                table->MajorVersion = ReadI16(_file);
                table->MinorVersion = ReadI16(_file);
                table->NumberOfGlyphs = ReadI16(_file);
                table->MaxPoints = ReadI16(_file);
                table->MaxContours = ReadI16(_file);
                table->MaxCompositePoints = ReadI16(_file);
                table->MaxCompositeContours = ReadI16(_file);
                table->MaxZones = ReadI16(_file);
                table->MaxTwilightPoints = ReadI16(_file);
                table->MaxStorage = ReadI16(_file);
                table->MaxFunctionDefinitions = ReadI16(_file);
                table->MaxInstructionDefinitions = ReadI16(_file);
                table->MaxStackElements = ReadI16(_file);
                table->MaxSizeOfInstructions = ReadI16(_file);
                table->MaxComponentElements = ReadI16(_file);
                table->MaxComponentDepth = ReadI16(_file);
                font->Tables[i] = reinterpret_cast<void*>(table);
            }
            else if (tagCharacter1 == 'O' && tagCharacter2 == 'S' && tagCharacter3 == '/' && tagCharacter4 == '2')
            {
                OS2_Table* table = new OS2_Table;
                table->Version = ReadI16(_file);
                table->X_AverageCharacterWidth = ReadI16(_file);
                table->US_WeightClass = ReadI16(_file);
                table->US_WidthClass = ReadI16(_file);
                table->FS_Type = ReadI16(_file);
                table->Y_SubscriptXSize = ReadI16(_file);
                table->Y_SubscriptYSize = ReadI16(_file);
                table->Y_SubscriptXOffset = ReadI16(_file);
                table->Y_SubscriptYOffset = ReadI16(_file);
                table->Y_SuperscriptXSize = ReadI16(_file);
                table->Y_SuperscriptYSize = ReadI16(_file);
                table->Y_SuperscriptXOffset = ReadI16(_file);
                table->Y_SuperscriptYOffset = ReadI16(_file);
                table->Y_StrikeoutSize = ReadI16(_file);
                table->Y_StrikeoutPosition = ReadI16(_file);
                table->S_FamilyClass = ReadI16(_file);
                for (int n = 0; n < 10; n++)
                    table->Panose[i] = ReadI8(_file);
                table->UL_UnicodeRange[0] = ReadI32(_file);
                table->UL_UnicodeRange[1] = ReadI32(_file);
                table->UL_UnicodeRange[2] = ReadI32(_file);
                table->UL_UnicodeRange[3] = ReadI32(_file);
                for (int n = 0; n < 4; n++)
                    table->VendorID[i] = ReadI8(_file);
                table->FS_Selection = ReadI16(_file);
                table->US_FirstCharIndex = ReadI16(_file);
                table->US_LastCharIndex = ReadI16(_file);
                table->S_TypographicAscender = ReadI16(_file);
                table->S_TypographicDescender = ReadI16(_file);
                table->S_TypographicLineGap = ReadI16(_file);
                table->US_WinAscent = ReadI16(_file);
                table->US_WinDescent = ReadI16(_file);

                if (table->Version > 0)
                {
                    table->UL_CodePageRange1 = ReadI32(_file);
                    table->UL_CodePageRange2 = ReadI32(_file);
                }

                if (table->Version > 1)
                {
                    table->SX_Height = ReadI16(_file);
                    table->S_CapHeight = ReadI16(_file);
                    table->US_DefaultChar = ReadI16(_file);
                    table->US_BreakChar = ReadI16(_file);
                    table->US_MaxContext = ReadI16(_file);
                }

                if (table->Version > 4)
                {
                    table->US_LowerOpticalPointSize = ReadI16(_file);
                    table->US_UpperOpticalPointSize = ReadI16(_file);
                }

                font->Tables[i] = reinterpret_cast<void*>(table);
            }

            //position the file at the beginning for the next table header
            fseek(_file, tableHeaderBegin, SEEK_SET);
        }

        ///EXTRACTING THE DATA FROM TABLES hmtx AND loca

        /* (A) (hmtx depends on hhea and maxp), (loca depends on head and maxp), and there is no guarantee that (tested with ROCKWELL.ttf in Windows 11),
            hhea, head and maxp will be located before hmtx/loca in the file; that's why the data from these two tables must be extracted
            after extraction of the other tables (with exception of glyf) */

        LOCA_Table* locaTable = new LOCA_Table;

        //position the file at the beginning of the of the table list
        fseek(_file, 12, SEEK_SET);

        for (int i = 0; i < font->NumberOfTables; i++)
        {
            unsigned char tagCharacter1 = ReadI8(_file);
            unsigned char tagCharacter2 = ReadI8(_file);
            unsigned char tagCharacter3 = ReadI8(_file);
            unsigned char tagCharacter4 = ReadI8(_file);
            fseek(_file, ftell(_file) + 4, SEEK_SET); //ignoring the Checksum field
            unsigned int tableOffset = ReadI32(_file);
            fseek(_file, ftell(_file) + 4, SEEK_SET); //ignoring the Length field

            //this variable is needed because a jump (to the header of the next table) has to be performed after extraction of this table
            int tableHeaderBegin = ftell(_file);

            fseek(_file, tableOffset, SEEK_SET);

            if (tagCharacter1 == 'h' && tagCharacter2 == 'm' && tagCharacter3 == 't' && tagCharacter4 == 'x')
            {
                HMTX_Table* table = new HMTX_Table;

                unsigned short numberOfHorizontalMetrics = reinterpret_cast<HHEA_Table*>(GetTable(font, HHEA_TABLE))->NumberOfHorizontalMetrics;
                unsigned short numberOfGlyphs = reinterpret_cast<MAXP_Table*>(GetTable(font, MAXP_TABLE))->NumberOfGlyphs;

                table->HorizontalMetrics = new LongHorizontalMetric[numberOfGlyphs];
                table->LeftSideBearings = new short[numberOfGlyphs - numberOfHorizontalMetrics];

                int lastAdvanceWidth;

                //extracting horizontal metrics
                for (int n = 0; n < numberOfHorizontalMetrics; n++)
                {
                     LongHorizontalMetric metric;
                     metric.AdvanceWidth = ReadI16(_file);
                     metric.LeftSideBearing = ReadI16(_file);
                     table->HorizontalMetrics[n] = metric;
                     lastAdvanceWidth = metric.AdvanceWidth;
                }

                //extracting left side bearings
                for (int n = numberOfHorizontalMetrics; n < numberOfGlyphs; n++)
                {
                    table->LeftSideBearings[n - numberOfHorizontalMetrics] = ReadI16(_file);
                    table->HorizontalMetrics[n].AdvanceWidth = lastAdvanceWidth;
                    table->HorizontalMetrics[n].LeftSideBearing = table->LeftSideBearings[n - numberOfHorizontalMetrics];
                }

                font->Tables[i] = reinterpret_cast<void*>(table);
            }
            else if (tagCharacter1 == 'l' && tagCharacter2 == 'o' && tagCharacter3 == 'c' && tagCharacter4 == 'a')
            {
                short tableType = reinterpret_cast<HEAD_Table*>(GetTable(font, HEAD_TABLE))->IndexToLocationFormat;
                unsigned short numberOfGlyphs = reinterpret_cast<MAXP_Table*>(GetTable(font, MAXP_TABLE))->NumberOfGlyphs;

                //'short' version
                if (tableType == 0)
                {
                    unsigned short* array = new unsigned short[numberOfGlyphs + 1];

                    for (int i = 0; i < numberOfGlyphs + 1; i++)
                    {
                        array[i] = ReadI16(_file);
                    }

                    locaTable->Offsets = reinterpret_cast<void*>(array);
                    locaTable->ArrayType = 0;
                }
                //'long' version
                else
                {
                    unsigned int* array = new unsigned int[numberOfGlyphs + 1];

                    for (int i = 0; i < numberOfGlyphs + 1; i++)
                    {
                        array[i] = ReadI32(_file);
                    }

                    locaTable->Offsets = reinterpret_cast<void*>(array);
                    locaTable->ArrayType = 1;
                }

                font->Tables[i] = reinterpret_cast<void*>(locaTable);
            }

            //positioning the file at the beginning of the next table header
            fseek(_file, tableHeaderBegin, SEEK_SET);
        }

        ///EXTRACTING THE DATA FROM TABLE glyf

        /* (A) (glyf depends on loca and maxp), and there is no guarantee (tested with ROCKWELL.ttf in Windows 11), that loca and maxp
               will be located before table glyf in the file; that's why the data from these two tables must be extracted after the
               extraction of the other tables */

        //position the file at the beginning of the of the table list
        fseek(_file, 12, SEEK_SET);

        for (int i = 0; i < font->NumberOfTables; i++)
        {
            unsigned char tagCharacter1 = ReadI8(_file);
            unsigned char tagCharacter2 = ReadI8(_file);
            unsigned char tagCharacter3 = ReadI8(_file);
            unsigned char tagCharacter4 = ReadI8(_file);
            fseek(_file, ftell(_file) + 4, SEEK_SET); //ignoring the Checksum field
            unsigned int tableOffset = ReadI32(_file);
            fseek(_file, ftell(_file) + 4, SEEK_SET); //ignoring the Length field

            //this variable is needed because a jump (to the header of the next table) has to be performed after extraction of this table
            int tableHeaderBegin = ftell(_file);

            fseek(_file, tableOffset, SEEK_SET);

            if (tagCharacter1 == 'g' && tagCharacter2 == 'l' && tagCharacter3 == 'y' && tagCharacter4 == 'f')
            {
                GLYF_Table* table = new GLYF_Table;

                unsigned short numberOfGlyphs = reinterpret_cast<MAXP_Table*>(GetTable(font, MAXP_TABLE))->NumberOfGlyphs;

                table->NumberOfGlyphs = numberOfGlyphs;
                table->Glyphs = new void*[numberOfGlyphs];

                //for every glyph in the table
                for (int n = 0; n < numberOfGlyphs; n++)
                {
                    //determine the position (relative to the beginning of table glyf) of glyph (index)'n'

                    int glyphOffset;
                    int nextGlyphOffset;

                    //uint16 values
                    if (n < numberOfGlyphs && locaTable->ArrayType == 0)
                    {
                         glyphOffset = reinterpret_cast<unsigned short*>(locaTable->Offsets)[n] * 2;
                         nextGlyphOffset = reinterpret_cast<unsigned short*>(locaTable->Offsets)[n + 1] * 2;
                    }
                    //uint32 values
                    else if (n < numberOfGlyphs)
                    {
                         glyphOffset = reinterpret_cast<unsigned int*>(locaTable->Offsets)[n];
                         nextGlyphOffset = reinterpret_cast<unsigned int*>(locaTable->Offsets)[n + 1];
                    }

                    //if the contour has no glyphs
                    if (glyphOffset == nextGlyphOffset)
                    {
                        table->Glyphs[n] = reinterpret_cast<void*>(new EmptyGlyph);
                        continue;
                    }

                    fseek(_file, tableOffset + glyphOffset, SEEK_SET);

                    ///reading the glyph

                    //reading the header

                    short numberOfContours = ReadI16(_file);

                    //if the glyph is simple
                    if (numberOfContours >= 0)
                    {
                         table->Glyphs[n] = ExtractSimpleGlyph(_file, numberOfContours);
                    }
                    //(STATE) the glyph is composite
                    else
                    {
                         table->Glyphs[n] = ExtractCompositeGlyph(_file);
                    }
                }

                font->Tables[i] = reinterpret_cast<void*>(table);
            }

            //position the file at the beginning for the next table header
            fseek(_file, tableHeaderBegin, SEEK_SET);
        }

        return font;
    }

    //(PUBLIC)
    //(the glyph corresponding to the specified codepoint) does not exist in the file => -1
    int GetGlyphIndex(const Font* _font, int _codepoint)
    {
          CMAP_Table* cmap = reinterpret_cast<CMAP_Table*>(GetTable(_font, CMAP_TABLE));

          for (int i = cmap->NumberOfSubtables - 1; i > -1; i--)
          {
              if (cmap->Subtables[i] == nullptr)
              {
                 continue;
              }
              else if (Is(cmap->Subtables[i], CMAP_SUBTABLE_FORMAT12))
              {
                  CMAP_Subtable_Format12* subtable = reinterpret_cast<CMAP_Subtable_Format12*>(cmap->Subtables[i]);

                  //(NEED-TESTING)

                  for (int i = 0; i < subtable->NumberOfGroups; i++)
                  {
                      if (_codepoint >= subtable->Groups[i]->StartCharacterCode && _codepoint <= subtable->Groups[i]->EndCharacterCode)
                      {
                          return subtable->Groups[i]->StartGlyphIndex + _codepoint - subtable->Groups[i]->StartCharacterCode;
                      }
                  }

                  return -1;
              }
            else if (Is(cmap->Subtables[i], CMAP_SUBTABLE_FORMAT6))
            {
                CMAP_Subtable_Format6* subtable = reinterpret_cast<CMAP_Subtable_Format6*>(cmap->Subtables[i]);

                //(NEED-TESTING)

                if (_codepoint < subtable->FirstCode || _codepoint > (subtable->FirstCode + subtable->EntryCount) - 1)
                {
                    return -1;
                }
                else
                {
                    return subtable->GlyphIndexArray[_codepoint - subtable->FirstCode];
                }
            }
               else if (Is(cmap->Subtables[i], CMAP_SUBTABLE_FORMAT4))
               {
                   CMAP_Subtable_Format4* subtable = reinterpret_cast<CMAP_Subtable_Format4*>(cmap->Subtables[i]);

                   int segmentCount = subtable->SegmentCountX2 / 2;

                    for (int n = 0; n < segmentCount; n++)
                    {
                        if (subtable->EndCode[n] >= _codepoint)
                        {
                            int correspondingStartCode = subtable->StartCode[n];

                            /* (SOURCE Apple) "If the corresponding startCode is less than or equal to the character code, then use the corresponding idDelta and
                                idRangeOffset to map the character code to the glyph index. Otherwise, the missing character glyph is returned. " */
                            if (correspondingStartCode <= _codepoint)
                            {
                                /* (SOURCE Apple) "If the idRangeOffset value for the segment is not 0, the mapping of the character codes
                                    relies on the glyphIndexArray." */
                                if (subtable->IndexRangeOffsets[n] != 0)
                                {
                                    unsigned short glyphIndex = subtable->GlyphIndexArray[
                                             (subtable->IndexRangeOffsets[n] / 2 /*as these indexes are in bytes*/) - (segmentCount - n) + (_codepoint - subtable->StartCode[n])];

                                    /* (SOURCE Apple) "Once the glyph indexing operation is complete, the glyph ID at the indicated address is checked.
                                        If it's not 0 (that is, if it's not the missing glyph), the value is added to idDelta[i] to get the actual glyph ID to use." */
                                    if (glyphIndex != 0)
                                    {
                                        return glyphIndex + subtable->IndexDelta[n];
                                    }
                                    else
                                    {
                                        return -1; //missing character/glyph
                                    }
                                }
                                //(SOURCE Apple) "If the idRangeOffset is 0, the idDelta value is added directly to the character code to get the corresponding glyph index."
                                else
                                {
                                    return (subtable->IndexDelta[n] + _codepoint) % 65536;
                                }
                            }
                            else
                            {
                                return -1; //missing character/glyph
                            }
                        }
                    }
               }
               else if (Is(cmap->Subtables[i], CMAP_SUBTABLE_FORMAT0) && _codepoint < 256)
               {
                   CMAP_Subtable_Format0* subtable = reinterpret_cast<CMAP_Subtable_Format0*>(cmap->Subtables[i]);
                   return subtable->GlyphIndexArray[_codepoint];
               }
               //format 8|10|13|14
               else
               {
                   continue;
               }
          }

          return -1;
    }

    //(PUBLIC)
    //the codepoint does not exist in the file => nullptr
    //_characterIndex < 0 :: index in the table glyf | _characterIndex >= 0 :: codepoint
    void* GetGlyph(const Font* _font, int _characterIndex)
    {
        GLYF_Table* glyf = reinterpret_cast<GLYF_Table*>(GetTable(_font, GLYF_TABLE));

        int glyphIndex;

        if (_characterIndex >= 0)
        {
            glyphIndex = GetGlyphIndex(_font, _characterIndex);
        }
        else
        {
            glyphIndex = -_characterIndex;
        }

        if (glyphIndex != -1)
        {
            return glyf->Glyphs[glyphIndex];
        }
        else
        {
            return nullptr;
        }
    }


    //(PUBLIC)
    //the return value is in Funit-s
    /* _characterIndex is a Unicode codepoint if it's a positive value, and glyph index (within the given font file) if it's a negative value;
       the function is non-validating - if _characterIndex is a Unicode codepoint, then it must be a valid Unicode codepoint and if
       _characterIndex is a glyph index, then it must be an index within the valid for the specific font range */
    //the specified character (codepoint) exists in the file  ->
    int GetLeftSideBearing(const Font* _font, int _characterCode)
    {
         HMTX_Table* hmtx = reinterpret_cast<HMTX_Table*>(GetTable(_font, HMTX_TABLE));

         if (_characterCode < 0)
         {
             return hmtx->HorizontalMetrics[0 - _characterCode].LeftSideBearing;
         }
         else
         {
             return hmtx->HorizontalMetrics[GetGlyphIndex(_font, _characterCode)].LeftSideBearing;
         }
    }

    //(PUBLIC)
    //the return value is in Funit-s
    //the specified character (codepoint) exists in the glyph ->
    int GetRightSideBearing(const Font* _font, int _codepoint)
    {
       HMTX_Table* hmtx = reinterpret_cast<HMTX_Table*>(GetTable(_font, HMTX_TABLE));
       GLYF_Table* glyf = reinterpret_cast<GLYF_Table*>(GetTable(_font, GLYF_TABLE));
       int glyphIndex = GetGlyphIndex(_font, _codepoint);
       int advanceWidth = hmtx->HorizontalMetrics[glyphIndex].AdvanceWidth;
       void* glyph = glyf->Glyphs[glyphIndex];

       if (Is(glyph, EMPTY_GLYPH))
       {
           return 0;
       }
       else if (Is(glyph, SIMPLE_GLYPH))
       {
            SimpleGlyph* glyph_ = reinterpret_cast<SimpleGlyph*>(glyph);

            if (glyph_->MinX < 0)
            {
               return advanceWidth - (glyph_->MaxX - (0 - glyph_->MinX));
            }
            else
            {
               return advanceWidth - glyph_->MaxX;
            }
       }
       else
       {
           CompositeGlyph* glyph_ = reinterpret_cast<CompositeGlyph*>(glyph);

           if (glyph_->MinX < 0)
           {
               return advanceWidth - (glyph_->MaxX - (0 - glyph_->MinX));
           }
           else
           {
               return advanceWidth - glyph_->MaxX;
           }
       }
    }

    //(PUBLIC)
    //returns the distance (in Funit-s) from the (baseline) to (the highest graphemic point of the character)
    //returns negative value if the highest graphemic point of the character is below the baseline
    //returns INT_MAX if the _codepoint is empty symbol
    //_fontSize is specified in pixels
    double GetAscent(Font* _font, int _codepoint)
    {
       HEAD_Table* head = reinterpret_cast<HEAD_Table*>(GetTable(_font, HEAD_TABLE));
       GLYF_Table* glyf = reinterpret_cast<GLYF_Table*>(GetTable(_font, GLYF_TABLE));

       void* glyph = GetGlyph(_font, _codepoint);

       if (Is(glyph, EMPTY_GLYPH))
       {
           return INT_MAX;
       }
       else if (Is(glyph, SIMPLE_GLYPH))
       {
           return reinterpret_cast<SimpleGlyph*>(glyph)->MaxY;
       }
       else
       {
           return reinterpret_cast<CompositeGlyph*>(glyph)->MaxY;
       }
    }

    //(PUBLIC)
    //returns the distance (in Funit-s) from the (baseline) to (the lowest graphemic point of the string)
    //returns positive value if the lowest graphemic point of the string is above the baseline
    //returns INT_MAX if the _codepoint represents an empty glyph
    //_fontSize is specified in pixels
    double GetDescent(Font* _font, const int _codepoint)
    {
       HEAD_Table* head = reinterpret_cast<HEAD_Table*>(GetTable(_font, HEAD_TABLE));
       GLYF_Table* glyf = reinterpret_cast<GLYF_Table*>(GetTable(_font, GLYF_TABLE));

       void* glyph = GetGlyph(_font, _codepoint);

       if (Is(glyph, EMPTY_GLYPH))
       {
           return INT_MAX;
       }
       else if (Is(glyph, SIMPLE_GLYPH))
       {
           return reinterpret_cast<SimpleGlyph*>(glyph)->MinY;
       }
       else
       {
           return reinterpret_cast<CompositeGlyph*>(glyph)->MinY;
       }
    }

    //(PUBLIC)
    //returns the distance (in Funit-s) from the (baseline) to (the highest graphemic point of the string)
    //returns negative value if the highest graphemic point of the string is below the baseline
    //_fontSize is specified in pixels
    double GetAscent(Font* _font, const std::wstring& _string)
    {
       HEAD_Table* head = reinterpret_cast<HEAD_Table*>(GetTable(_font, HEAD_TABLE));
       GLYF_Table* glyf = reinterpret_cast<GLYF_Table*>(GetTable(_font, GLYF_TABLE));

       int maxY = -1;

       for (int i = 0; i < _string.length(); i++)
       {
           void* glyph = GetGlyph(_font, _string[i]);

           if (Is(glyph, EMPTY_GLYPH))
           {
               continue;
           }
           else if (Is(glyph, SIMPLE_GLYPH))
           {
               SimpleGlyph* glyph_ = reinterpret_cast<SimpleGlyph*>(glyph);

               if (maxY == -1 || glyph_->MaxY > maxY)
               {
                   maxY = glyph_->MaxY;
               }
           }
           else
           {
               CompositeGlyph* glyph_ = reinterpret_cast<CompositeGlyph*>(glyph);

               if (maxY == -1 || glyph_->MaxY > maxY)
               {
                   maxY = glyph_->MaxY;
               }
           }
       }

       return maxY;
    }

    //(PUBLIC)
    //returns the distance (in Funit-s) from the (baseline) to (the lowest graphemic point of the string)
    //returns positive value if the lowest graphemic point of the string is above the baseline
    //_fontSize is specified in pixels
    double GetDescent(Font* _font, const std::wstring& _string)
    {
       HEAD_Table* head = reinterpret_cast<HEAD_Table*>(GetTable(_font, HEAD_TABLE));
       GLYF_Table* glyf = reinterpret_cast<GLYF_Table*>(GetTable(_font, GLYF_TABLE));

       int minY = -1;

       for (int i = 0; i < _string.length(); i++)
       {
           void* glyph = GetGlyph(_font, _string[i]);

           if (Is(glyph, EMPTY_GLYPH))
           {
               continue;
           }
           else if (Is(glyph, SIMPLE_GLYPH))
           {
               SimpleGlyph* glyph_ = reinterpret_cast<SimpleGlyph*>(glyph);

               if (minY == -1 || glyph_->MinY < minY)
               {
                   minY = glyph_->MinY;
               }
           }
           else
           {
               CompositeGlyph* glyph_ = reinterpret_cast<CompositeGlyph*>(glyph);

               if (minY == -1 || glyph_->MinY < minY)
               {
                   minY = glyph_->MinY;
               }
           }
       }

       return minY;
    }

    //(PUBLIC)
    //the return values is in Funit-s
    /* _characterIndex is a Unicode codepoint if it's a positive value, and glyph index (within the given font file) if it's a negative value;
       the function is non-validating - if _characterIndex is a Unicode codepoint, then it must be a valid Unicode codepoint and if
       _characterIndex is a glyph index, then it must be an index within the valid for the specific font range */
    //the specified character (codepoint) exists in the file ->
    int GetAdvanceWidth(const Font* _font, int _characterCode)
    {
       HMTX_Table* hmtx = reinterpret_cast<HMTX_Table*>(GetTable(_font, HMTX_TABLE));

        if (_characterCode < 0)
        {
            return hmtx->HorizontalMetrics[0 - _characterCode].AdvanceWidth;
        }
        else
        {
            return hmtx->HorizontalMetrics[GetGlyphIndex(_font, _characterCode)].AdvanceWidth;
        }
    }

    //(PUBLIC)
    //the return value is in Funit-s
    //the specified kerning-pair does not exist in the file => INT_MIN
    //the specified character (codepoint) does not exist in the file ->
    int GetKerning(const Font* _font, int _codepoint1, int _codepoint2)
    {
       KERN_Table* kern = reinterpret_cast<KERN_Table*>(GetTable(_font, KERN_TABLE));

       if (kern == nullptr)
       {
           return INT_MIN;
       }

       KERN_Subtable_Format0* subtable = reinterpret_cast<KERN_Subtable_Format0*>(kern->Subtables[0]);

       int glyphIndex1 = GetGlyphIndex(_font, _codepoint1);
       int glyphIndex2 = GetGlyphIndex(_font, _codepoint2);

       //(POTENTIAL-OPTIMIZATION)
       for (int i = 0; i < subtable->NumberOfPairs; i++)
       {
           if (subtable->Pairs[i].Left == glyphIndex1 && subtable->Pairs[i].Right == glyphIndex2)
           {
               return subtable->Pairs[i].Value;
           }
       }

       return INT_MIN;
    }

    //(PUBLIC)
    bool ContainsGlyph(const Font* _font, int _codepoint)
    {
        return GetGlyphIndex(_font, _codepoint) != -1;
    }
}

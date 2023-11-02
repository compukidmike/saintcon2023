#ifndef __ANIMATEDSPRITE__
#define __ANIMATEDSPRITE__
#include <Arduino.h>


/* GIF Defines and variables */
#define MAX_CHUNK_SIZE 255
//
// These 2 macros can be changed to limit the amount of RAM
// required by the decoder. For example, decoding 1-bit images to
// a 128x32 display will not need a max code size of 12 nor a palette
// with 256 entries
//
#define MAX_CODE_SIZE 12
#define MAX_COLORS 256
#define LZW_BUF_SIZE (6*MAX_CHUNK_SIZE)
#define LZW_HIGHWATER (4*MAX_CHUNK_SIZE)
#define MAX_WIDTH 320
// This buffer is used to store the pixel sequence in reverse order
// it needs to be large enough to hold the longest possible
// sequence (1<<MAX_CODE_SIZE)
#define FILE_BUF_SIZE (1<<MAX_CODE_SIZE)

#define PIXEL_FIRST 0
#define PIXEL_LAST (1<<MAX_CODE_SIZE)
#define LINK_UNUSED 5911 // 0x1717 to use memset
#define LINK_END 5912
#define MAX_HASH 5003

enum {
   GIF_PALETTE_RGB565_LE = 0, // little endian (default)
   GIF_PALETTE_RGB565_BE,     // big endian
   GIF_PALETTE_RGB888         // original 24-bpp entries
};
// for compatibility with older code
#define LITTLE_ENDIAN_PIXELS GIF_PALETTE_RGB565_LE
#define BIG_ENDIAN_PIXELS GIF_PALETTE_RGB565_BE
//
// Draw callback pixel type
// RAW = 8-bit palettized pixels requiring transparent pixel handling
// COOKED = 16 or 24-bpp fully rendered pixels ready for display
//
enum {
   GIF_DRAW_RAW = 0,
   GIF_DRAW_COOKED
};

enum {
   GIF_SUCCESS = 0,
   GIF_DECODE_ERROR,
   GIF_TOO_WIDE,
   GIF_INVALID_PARAMETER,
   GIF_UNSUPPORTED_FEATURE,
   GIF_FILE_NOT_OPEN,
   GIF_EARLY_EOF,
   GIF_EMPTY_FRAME,
   GIF_BAD_FILE,
   GIF_ERROR_MEMORY
};

typedef struct gif_file_tag
{
  int32_t iPos; // current file position
  int32_t iSize; // file size
  uint8_t *pData; // memory file pointer
  void * fHandle; // class pointer to File/SdFat or whatever you want
} GIFFILE;

typedef struct gif_info_tag
{
  int32_t iFrameCount; // total frames in file
  int32_t iDuration; // duration of animation in milliseconds
  int32_t iMaxDelay; // maximum frame delay
  int32_t iMinDelay; // minimum frame delay
} GIFINFO;

typedef struct gif_draw_tag
{
    int iX, iY; // Corner offset of this frame on the canvas
    int y; // current line being drawn (0 = top line of image)
    int iWidth, iHeight; // size of this frame
    void *pUser; // user supplied pointer
    uint8_t *pPixels; // 8-bit source pixels for this line
    uint16_t *pPalette; // little or big-endian RGB565 palette entries (default)
    uint8_t *pPalette24; // RGB888 palette (optional)
    uint8_t ucTransparent; // transparent color
    uint8_t ucHasTransparency; // flag indicating the transparent color is in use
    uint8_t ucDisposalMethod; // frame disposal method
    uint8_t ucBackground; // background color
    uint8_t ucIsGlobalPalette; // Flag to indicate that a global palette, rather than a local palette is being used
} GIFDRAW;

// Callback function prototypes
typedef int32_t (GIF_READ_CALLBACK)(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen);
typedef int32_t (GIF_SEEK_CALLBACK)(GIFFILE *pFile, int32_t iPosition);
typedef void (GIF_DRAW_CALLBACK)(GIFDRAW *pDraw);
typedef void * (GIF_OPEN_CALLBACK)(const char *szFilename, int32_t *pFileSize);
typedef void (GIF_CLOSE_CALLBACK)(void *pHandle);
typedef void * (GIF_ALLOC_CALLBACK)(uint32_t iSize);
typedef void (GIF_FREE_CALLBACK)(void *buffer);
//
// our private structure to hold a GIF image decode state
//
typedef struct gif_image_tag
{
    int iWidth, iHeight, iCanvasWidth, iCanvasHeight;
    int iX, iY; // GIF corner offset
    int iBpp;
    int iError; // last error
    int iFrameDelay; // delay in milliseconds for this frame
    int iRepeatCount; // NETSCAPE animation repeat count. 0=forever
    int iXCount, iYCount; // decoding position in image (countdown values)
    int iLZWOff; // current LZW data offset
    int iLZWSize; // current quantity of data in the LZW buffer
    int iCommentPos; // file offset of start of comment data
    short sCommentLen; // length of comment
    GIF_READ_CALLBACK *pfnRead;
    GIF_SEEK_CALLBACK *pfnSeek;
    GIF_DRAW_CALLBACK *pfnDraw;
    GIF_OPEN_CALLBACK *pfnOpen;
    GIF_CLOSE_CALLBACK *pfnClose;
    GIFFILE GIFFile;
    void *pUser;
    unsigned char *pFrameBuffer;
    unsigned char *pPixels, *pOldPixels;
    unsigned char ucLineBuf[MAX_WIDTH]; // current line
    unsigned char ucFileBuf[FILE_BUF_SIZE]; // holds temp data and pixel stack
    unsigned short pPalette[(MAX_COLORS * 3)/2]; // can hold RGB565 or RGB888 - set in begin()
    unsigned short pLocalPalette[(MAX_COLORS * 3)/2]; // color palettes for GIF images
    unsigned char ucLZW[LZW_BUF_SIZE]; // holds 6 chunks (6x255) of GIF LZW data packed together
    //unsigned short usGIFTable[1<<MAX_CODE_SIZE];
    //unsigned char ucGIFPixels[(PIXEL_LAST*2)];
    unsigned char bEndOfFrame;
    unsigned char ucGIFBits, ucBackground, ucTransparent, ucCodeStart, ucMap, bUseLocalPalette;
    unsigned char ucPaletteType; // RGB565 or RGB888
    unsigned char ucDrawType; // RAW or COOKED
} GIFIMAGE;

#define MAX_GIFS    6

class AnimatedSprite {
public:
    AnimatedSprite();
    ~AnimatedSprite();

    void SetGif(uint8_t *pData, int iDataSize);
    void SetGif(String name);
    virtual bool Draw(int x, int y, int disolve=0);

private:
    int decoder_id;

};

#endif // __ANIMATEDSPRITE__

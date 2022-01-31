#pragma once

#include <zlib.h>
#include <png.h>

#include <string>
using std::string;

enum ImageFormat
{
	FORMAT_4444	= 1,
	FORMAT_8888	= 2,
	FORMAT_565	= 513,
	FORMAT_BIN	= 517
};

class PNGWriter{
public:
	static void Initialize();
	static void Deinitialize();
	static bool Write(string sFilename, int iHeight, int iWidth, int iStreamLength, unsigned char* pStream, ImageFormat format);

	static int maxHeight;
	static int maxOutBuf;
	static int maxWriteBuf;
	static unsigned char* outBuf;
	static unsigned char* writeBuf;
	static png_bytep* row_pointers;

	static png_text pngtext[2];
};
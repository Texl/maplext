#include "PNGWriter.h"

int PNGWriter::maxHeight = 600;
int PNGWriter::maxOutBuf = 800 * 600 * 2;
int PNGWriter::maxWriteBuf = 800 * 600 * 2 * 2;

unsigned char* PNGWriter::outBuf	= NULL;
unsigned char* PNGWriter::writeBuf	= NULL;
png_bytep* PNGWriter::row_pointers	= NULL;
png_text PNGWriter::pngtext[2]		= {0};

void PNGWriter::Initialize()
{
	outBuf			= new unsigned char[maxOutBuf];
	writeBuf		= new unsigned char[maxWriteBuf];
	row_pointers	= new png_bytep[maxHeight];
}

void PNGWriter::Deinitialize()
{
	delete[] outBuf;
	delete[] writeBuf;
	delete[] row_pointers;
}

bool PNGWriter::Write(string sFilename, int iHeight, int iWidth, int iStreamLength, unsigned char* pStream, ImageFormat format)
{
	int sizeUncompressed;
	int size8888;

	switch(format){
	case FORMAT_4444:
	case FORMAT_565:
		sizeUncompressed = 	iHeight * iWidth * 2;
		break;
	case FORMAT_8888:
		sizeUncompressed = 	iHeight * iWidth * 4;
		break;
	case FORMAT_BIN:
		sizeUncompressed = iHeight * iWidth / 128;
		break;
	}
	size8888 = iHeight * iWidth * 4;

	if(sizeUncompressed > maxOutBuf)
	{
		maxOutBuf = sizeUncompressed;
		delete[] outBuf;
		outBuf = new unsigned char[maxOutBuf];
	}

	if(size8888 > maxWriteBuf)
	{
		maxWriteBuf = size8888;
		delete[] writeBuf;
		writeBuf = new unsigned char[maxWriteBuf];
	}

	if(iHeight > maxHeight)
	{
		maxHeight = iHeight;
		delete[] row_pointers;
		row_pointers = new png_bytep[maxHeight];
	}

	//create zlib stream
	z_stream strm;
	strm.zalloc		= Z_NULL;
    strm.zfree		= Z_NULL;
    strm.opaque		= Z_NULL;
	strm.avail_in	= iStreamLength;
	strm.next_in	= pStream;
	strm.avail_out	= sizeUncompressed;
	strm.next_out	= outBuf;

	//init inflate stream
	if(inflateInit(&strm) != Z_OK)
		return false;

	//attempt to inflate zlib stream
	inflate(&strm, Z_NO_FLUSH);

	//tidy up zlib
    inflateEnd(&strm);

	//some code to debug image formats
//	FILE* rawFile = fopen((sFilename + ".raw").c_str(), "wb");
//	fwrite(outBuf, 1, sizeUncompressed, rawFile);
//	fclose(rawFile);

	//transcode image data to 8888
	if(format == FORMAT_4444)
	{
		for(int i = 0; i < sizeUncompressed; i++)
		{
			unsigned char low	= outBuf[i] & 0x0F;
			unsigned char high	= outBuf[i] & 0xF0;

			writeBuf[(i << 1)] = (low << 4) | low;
			writeBuf[(i << 1) + 1] = high | (high >> 4);
		}
	}
	else if(format == FORMAT_8888)
	{
		memcpy(writeBuf, outBuf, sizeUncompressed);
	}
	else if(format == FORMAT_565)
	{
		for(int i = 0; i < sizeUncompressed; i+=2)
		{
			unsigned char bBits = (outBuf[i] & 0x1F) << 3;
			unsigned char gBits = ((outBuf[i + 1] & 0x07) << 5) | ((outBuf[i] & 0xE0) >> 3);
			unsigned char rBits = outBuf[i + 1] & 0xF8;

			writeBuf[(i << 1)]		= bBits | (bBits >> 5);
			writeBuf[(i << 1) + 1]	= gBits | (gBits >> 6);
			writeBuf[(i << 1) + 2]	= rBits | (rBits >> 5);
			writeBuf[(i << 1) + 3]	= 0xFF;
		}
	}
	else if(format == FORMAT_BIN)
	{
		unsigned char byte = 0;
		int pixelIndex = 0;
		for(int i = 0; i < sizeUncompressed; i++)
		{
			for(int j = 0; j < 8; j++)
			{
				byte = ((outBuf[i] & (0x01 << (7 - j))) >> (7 - j)) * 255;
				for(int k = 0; k < 16; k++)
				{
					pixelIndex = (i << 9) + (j << 6) + (k << 2);
					writeBuf[pixelIndex]		= byte;
					writeBuf[pixelIndex + 1]	= byte;
					writeBuf[pixelIndex + 2]	= byte;
					writeBuf[pixelIndex + 3]	= 0xFF;
				}
			}
		}
	}
	else
	{
		//unknown format
	}

	for(int iRow = 0; iRow < iHeight; iRow++)
		row_pointers[iRow] = &writeBuf[iRow * iWidth * 4];

	//create PNG file
	FILE* pngFile = fopen(sFilename.c_str(), "wb");
	if(!pngFile)
		return false;

	//create png write and info structs
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr)
		return false;

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr)
		return false;

	info_ptr->text = pngtext;
	info_ptr->num_text = 2;
	pngtext[0].compression = PNG_TEXT_COMPRESSION_NONE;
	pngtext[0].key = "Author";
	pngtext[0].text = "Alan Horne (www.alanhorne.com)";
	pngtext[1].compression = PNG_TEXT_COMPRESSION_NONE;
	pngtext[1].key = "Copyright";
	pngtext[1].text = "Wizet / Nexon";

	//initialize IO
	if(setjmp(png_jmpbuf(png_ptr)))
		return false;
	png_init_io(png_ptr, pngFile);

	//write PNG header
	if(setjmp(png_jmpbuf(png_ptr)))
		return false;
	png_set_bgr(png_ptr);
	png_set_IHDR(png_ptr, info_ptr, iWidth, iHeight,
			8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_write_info(png_ptr, info_ptr);

	//write image data
	if(setjmp(png_jmpbuf(png_ptr)))
		return false;
	png_write_image(png_ptr, row_pointers);

	//write PNG end and close file
	if (setjmp(png_jmpbuf(png_ptr)))
		return false;
	png_write_end(png_ptr, NULL);

	png_destroy_write_struct(&png_ptr, &info_ptr);

	fclose(pngFile);

	return true;
}
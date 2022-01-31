#include <string>
using std::string;

int main(unsigned int argc, char *argv[])
{
	//open files
	FILE* filePatch;
	FILE* fileInfo;
	FILE* fileBase;
	FILE* fileOut;

	if(argc < 2)
	{
		printf("%s <data.patch> [info.txt]\r\n", argv[0]);
		return 1;
	}
	
	filePatch = fopen(argv[1], "rb");
	
	if(!filePatch)
	{
		printf("Failed to open patch file %s!\r\n", argv[1]);
		return 1;
	}
	
	if(argc > 2)
	{
		fileInfo = fopen(argv[2], "rb");
		
		if(!fileInfo)
		{
			if(filePatch)
				fclose(filePatch);
			
			printf("Failed to open info file %s!\r\n", argv[2]);
			return 1;
		}
	}
	else
	{
		fileInfo = NULL;
	}
	
	fileBase = fopen("ManualPatch.base", "rb");
	
	if(!fileBase)
	{
		if(filePatch)
			fclose(filePatch);
		if(fileInfo)
			fclose(fileInfo);
		
		printf("Failed to open exe file %s!\r\n", "ManualPatch.base");
		return 1;
	}
	
	fileOut = fopen("ManualPatch.exe", "wb");
	
	if(!fileOut)
	{
		if(filePatch)
			fclose(filePatch);
		if(fileInfo)
			fclose(fileInfo);
		if(fileBase)
			fclose(fileBase);
		
		printf("Failed to open output file %s!\r\n", "ManualPatch.exe");
		return 0;
	}
	
	//write base EXE
	fseek(fileBase, 0, SEEK_END);
	unsigned int fileBaseSize = ftell(fileBase);
	fseek(fileBase, 0, SEEK_SET);
	printf("Base EXE size is %u\r\n", fileBaseSize);
	unsigned char* pBuffer = new unsigned char[fileBaseSize];
	fread(pBuffer, 1, fileBaseSize, fileBase);
	fwrite(pBuffer, 1, fileBaseSize, fileOut);
	delete[] pBuffer;
	
	//write patch data
	fseek(filePatch, 0, SEEK_END);
	unsigned int filePatchSize = ftell(filePatch);
	fseek(filePatch, 0, SEEK_SET);
	printf("Patch size is %u\r\n", filePatchSize);
	
	pBuffer = new unsigned char[filePatchSize];
	fread(pBuffer, 1, filePatchSize, filePatch);
	fwrite(pBuffer, 1, filePatchSize, fileOut);
	delete[] pBuffer;

	//write info data
	unsigned int fileInfoSize;
	if(fileInfo)
	{
		fseek(fileInfo, 0, SEEK_END);
		fileInfoSize = ftell(fileInfo);
		fseek(fileInfo, 0, SEEK_SET);
		printf("Info size is %u\r\n", fileInfoSize);
		pBuffer = new unsigned char[fileInfoSize];
		fread(pBuffer, 1, fileInfoSize, fileInfo);
		fwrite(pBuffer, 1, fileInfoSize, fileOut);
		delete[] pBuffer;
	}

	char* note = "\r\nTEXEL\r\n";
	fwrite(note, 1, strlen(note), fileOut);
	printf("Note size is %u\r\n", strlen(note));
	
	fileInfoSize += (unsigned int)strlen(note);
	
	fwrite(&filePatchSize, 4, 1, fileOut);
	fwrite(&fileInfoSize, 4, 1, fileOut);
	unsigned int patchcrc = 0xF2F7FBF3;
	fwrite(&patchcrc, 4, 1, fileOut);
	
	printf("Total EXE size is %u\r\n", ftell(fileOut));
	
	if(filePatch)
		fclose(filePatch);
	if(fileInfo)
		fclose(fileInfo);
	if(fileBase)
		fclose(fileBase);
	if(fileOut)
		fclose(fileOut);
	
	printf("All done, ManualPatch.exe created.\r\n");
}
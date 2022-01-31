//Alan Horne 2006
//maplext -[flags] <input file> <filter>
//maplext				: display usage
//S : string decode mode (ignores other options)
//l	: log file directory
//f	: extract files
//i	: extract images
//s	: extract sounds
//x	: export XML

#include "Globals.h"
#include "WZFile.h"
#include "Tokenizer.h"

WZFile g_WZFile;

void DecodeStrings()
{
	Log::Write("todo: string decoding\n");
}

void PrintUsage()
{
	Log::Write("usage:\n");
	Log::Write("maplext -[flags] <input file> <filter>\n");
	Log::Write("S: string decode mode (ignores other options)\n");
	Log::Write("l: list directory\n");
	Log::Write("f: extract files\n");
	Log::Write("i: extract images\n");
	Log::Write("s: extract sounds\n");
	Log::Write("x: export XML\n");
}

int main(int argc, char **argv)
{
	CreateDirectory("maplext", NULL);

	if(!Log::Init("maplext\\log.txt", true))
		return 1;

	if(argc < 3)
	{
		PrintUsage();
		return 1;
	}

	bool bDecodeStrings	= false;
	bool bListDirectory	= false;
	bool bExportXML		= false;
	bool bExtractImages	= false;
	bool bExtractSounds	= false;
	bool bExtractFiles	= false;

	//get actions
	if(argv[1][0] != '-')
	{
		PrintUsage();
		return 1;
	}
	for(unsigned int iFlag = 1; iFlag < strlen(argv[1]); iFlag++)
	{
		switch(argv[1][iFlag]){
			case 'S':
				bDecodeStrings	= true;
				break;
			case 'l':
				bListDirectory	= true;
				break;
			case 'x':
				bExportXML		= true;
				break;
			case 'i':
				bExtractImages	= true;
				break;
			case 's':
				bExtractSounds	= true;
				break;
			case 'f':
				bExtractFiles	= true;
				break;
			default:
				Log::Write("invalid action: %c\n", argv[2][iFlag]);
				PrintUsage();
				break;
		}
	}

	//get filename
	string fileName = argv[2];
	Log::Write("opening %s...\n", argv[2]);
	
	//get filter
	string filter = "";
	if(argc == 4)
		filter = string(argv[3]);

	//follow instruction flags
	{PROFILE("main");

		{PROFILE("file open");
			g_WZFile.Open(fileName, filter);
		}

		SetCurrentDirectory("maplext");

		if(bDecodeStrings)
		{
			DecodeStrings();
		}
		else
		{
			if(bListDirectory)
			{
				PROFILE("list directory");
				g_WZFile.ListDirectory();
			}
			if(bExportXML)
			{
				PROFILE("xml export");
				CreateDirectory("xml", NULL);
				SetCurrentDirectory("xml");
	
				FILE* fileOut = fopen("export.xml", "w");
				g_WZFile.ExportXML(fileOut);
				fclose(fileOut);

				SetCurrentDirectory("..");
			}
			if(bExtractImages)
			{
				PROFILE("extract images");
				CreateDirectory("images", NULL);
				SetCurrentDirectory("images");

				g_WZFile.ExtractImages();

				SetCurrentDirectory("..");
			}
			if(bExtractSounds)
			{
				PROFILE("extract sounds");
				CreateDirectory("sounds", NULL);
				SetCurrentDirectory("sounds");

				g_WZFile.ExtractSounds();

				SetCurrentDirectory("..");
			}
			if(bExtractFiles)
			{
				PROFILE("extract files");
				CreateDirectory("files", NULL);
				SetCurrentDirectory("files");

				g_WZFile.ExtractFiles();

				SetCurrentDirectory("..");
			}
		}

		{PROFILE("file close");
			g_WZFile.Close();
		}
	}
	Profiler::Update();
	Profiler::Output();

	SetCurrentDirectory("..");
	return 0;
}

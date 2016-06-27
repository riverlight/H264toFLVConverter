#include <iostream>
#include <fstream>

#include "splitter.h"
#include "converter.h"

using namespace std;
using namespace Cnvt;

fstream g_fileIn;
char g_flvFile[260];
int g_mode = 0;
CConverter g_cnvt;

unsigned char *g_pBufferIn, *g_pBufferOut;
int g_nFileSize = 0;

int Initialize(int argc, char *argv[]);
int Release();
int ConvertH264();
int ConvertAAC();


int main(int argc, char *argv[])
{
	cout << "Hi, this is a VideoJJ SEI splitter sample!\n";

	if (argc != 4)
	{
		cout << "Usage:\n\t" << "converter.exe" << " [mode] [h.264 or aac file]" << " [output file]" << endl;
		cout << "\tmode = 1 is h.264 to flv" << endl;
		cout << "\tmode = 2 is aac to flv\n" << endl;
		return 0;
	}
	if (Initialize(argc, argv) == 0)
		return 0;

	if (g_mode==1)
		ConvertH264();
	if (g_mode == 2)
		ConvertAAC();

	Release();

	return 1;
}

int Initialize(int argc, char *argv[])
{
	g_mode = atoi(argv[1]);
	if (g_mode != 1 && g_mode != 2)
	{
		cout << "mode must be 1 or 2" << endl;
		return 0;
	}

	g_fileIn.open(argv[2], ios::binary | _IOS_Nocreate | ios::in);
	if (!g_fileIn)
	{
		cout << argv[1] << " can not be open!\n";
		return 0;
	}
	
	strcpy_s(g_flvFile, argv[3]);

	g_fileIn.seekg(0, ios::end);
	std::streampos ps = g_fileIn.tellg();
	g_nFileSize = ps;
	g_fileIn.seekg(0, ios_base::beg);

	g_pBufferIn = new unsigned char[g_nFileSize];
	g_pBufferOut = new unsigned char[g_nFileSize];
	if (g_pBufferIn == NULL && g_pBufferOut == NULL)
		return 0;

	g_fileIn.read((char *)g_pBufferIn, g_nFileSize);
	if (g_nFileSize != g_fileIn.gcount())
		return 0;

	return 1;
}

int Release()
{
	delete g_pBufferIn;
	delete g_pBufferOut;
	
	g_fileIn.close();

	return 1;
}

int ConvertH264()
{
	int nOffset = 0;
	int count = 0;

	if (g_cnvt.Open(g_flvFile) == 0)
		return 0;

	unsigned int nTimeStamp = 0;
	while (1)
	{
		int nNaluSize = 0;
		if (Cnvt::GetOneNalu(g_pBufferIn + nOffset, g_nFileSize - nOffset, g_pBufferOut, nNaluSize) == 0)
			break;

		g_cnvt.ConvertH264((char *)g_pBufferOut, nNaluSize, nTimeStamp);

		if (g_pBufferOut[4] != 0x67 && g_pBufferOut[4]!=0x68)
			nTimeStamp += 33;
		nOffset += nNaluSize;
		if (nOffset >= g_nFileSize - 4)
			break;
		count++;
	}
	g_cnvt.Close();

	return 1;
}

int ConvertAAC()
{
	int nOffset = 0;
	int count = 0;

	if (g_cnvt.Open(g_flvFile, 1, 0) == 0)
		return 0;

	unsigned int nTimeStamp = 0;
	while (1)
	{
		int nAACFrameSize = 0;
		if (Cnvt::GetOneAACFrame(g_pBufferIn + nOffset, g_nFileSize - nOffset, g_pBufferOut, nAACFrameSize) == 0)
			break;

		printf("nAACFrameSize = %d\n", nAACFrameSize);
		g_cnvt.ConvertAAC((char *)g_pBufferOut, nAACFrameSize, nTimeStamp);

		nTimeStamp += double(1024*1000) / double(44100);
		nOffset += nAACFrameSize;
		if (nOffset >= g_nFileSize - 4)
			break;
		count++;
	}
	g_cnvt.Close();

	return 1;
}

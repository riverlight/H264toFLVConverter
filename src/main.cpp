#include <iostream>
#include <fstream>

#include "splitter.h"
#include "converter.h"

using namespace std;
using namespace Cnvt;

fstream g_fileIn;
int g_mode = 0;
CConverter g_cnvt;

unsigned char *g_pBufferIn, *g_pBufferOut;
int g_nFileSize = 0;

int Initialize(int argc, char *argv[]);
int Release();
int ConvertH264();



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
	
	if (g_cnvt.Open(argv[3]) == 0)
		return 0;

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

	g_cnvt.Close();
	g_fileIn.close();

	return 1;
}

int ConvertH264()
{
	int nOffset = 0;
	int count = 0;

	while (1)
	{
		int nNaluSize = 0;
		if (Cnvt::GetOneNalu(g_pBufferIn + nOffset, g_nFileSize - nOffset, g_pBufferOut, nNaluSize) == 0)
			break;

		g_cnvt.ConvertH264((char *)g_pBufferOut, nNaluSize);

		nOffset += nNaluSize;
		if (nOffset >= g_nFileSize - 4)
			break;
		count++;
	}

	return 1;
}

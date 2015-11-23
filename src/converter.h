#ifndef CONVERTER_H
#define CONVERTER_H

#include <fstream>

namespace Cnvt
{
	class u4
	{
	public:
		u4(unsigned int i) { _u[0] = i >> 24; _u[1] = (i >> 16) & 0xff; _u[2] = (i >> 8) & 0xff; _u[3] = i & 0xff; }

	public:
		unsigned char _u[4];
	};
	class u3
	{
	public:
		u3(unsigned int i) { _u[0] = i >> 16; _u[1] = (i >> 8) & 0xff; _u[2] = i & 0xff; }

	public:
		unsigned char _u[3];
	};
	class u2
	{
	public:
		u2(unsigned int i) { _u[0] = i >> 8; _u[1] = i & 0xff; }

	public:
		unsigned char _u[2];
	};

	class CConverter
	{
	public:
		CConverter();
		virtual ~CConverter();

		int Open(std::string strFlvFile);
		int Close();

		int Convert(char *pNalu, int nNaluSize);

	private:
		void MakeFlvHeader(unsigned char *pFlvHeader);
		void WriteHeader();
		void WriteFrame(char *pNalu, int nNaluSize);
		void WriteEndofSeq();

		void Write(unsigned char u) { _fileOut.write((char *)&u, 1); }
		void Write(u4 u) { _fileOut.write((char *)u._u, 4); }
		void Write(u3 u) { _fileOut.write((char *)u._u, 3); }
		void Write(u2 u) { _fileOut.write((char *)u._u, 2); }

	private:
		unsigned char _FlvHeader[9];
		unsigned char *_pSPS, *_pPPS;
		int _nSPSSize, _nPPSSize;
		int _bWriteAVCSeqHeader;
		int _nPrevTagSize;
		unsigned int _nTimeStamp;
		int _nStreamID;

	private:
		std::fstream _fileOut;

	};

}

#endif // CONVERTER_H

#include <iostream>

using namespace std;

#include "converter.h"

namespace Cnvt
{
	CConverter::CConverter()
	{
		_pSPS = NULL;
		_pPPS = NULL;
		_nSPSSize = 0;
		_nPPSSize = 0;
		_bWriteAVCSeqHeader = 0;
		_nPrevTagSize = 0;
		_nTimeStamp = 0;
		_nStreamID = 0;
	}

	CConverter::~CConverter()
	{

	}

	int CConverter::Open(std::string strFlvFile)
	{
		_fileOut.open(strFlvFile, std::ios_base::out | std::ios_base::binary);
		if (!_fileOut)
			return 0;

		MakeFlvHeader(_FlvHeader);

		return 1;
	}

	int CConverter::Close()
	{
		if (_pSPS != NULL)
			delete _pSPS;
		if (_pPPS != NULL)
			delete _pPPS;

		WriteEndofSeq();
		_fileOut.close();

		return 1;
	}

	int CConverter::Convert(char *pNalu, int nNaluSize)
	{
		if (pNalu == NULL || nNaluSize <= 4)
			return 0;

		int nNaluType = pNalu[4] & 0x1f;
		if (_pSPS == NULL && nNaluType == 0x07)
		{
			_pSPS = new unsigned char[nNaluSize];
			_nSPSSize = nNaluSize;
			memcpy(_pSPS, pNalu, nNaluSize);
		}
		if (_pPPS == NULL && nNaluType == 0x08)
		{
			_pPPS = new unsigned char[nNaluSize];
			_nPPSSize = nNaluSize;
			memcpy(_pPPS, pNalu, nNaluSize);
		}
		if (_pSPS != NULL && _pPPS != NULL && _bWriteAVCSeqHeader == 0)
		{
			WriteHeader();
			_bWriteAVCSeqHeader = 1;
		}
		if (_bWriteAVCSeqHeader == 0)
			return 1;

		WriteFrame(pNalu, nNaluSize);

		//_fileOut.write((char *)pNalu, nNaluSize);

		return 1;
	}

	void CConverter::MakeFlvHeader(unsigned char *pFlvHeader)
	{
		pFlvHeader[0] = 'F';
		pFlvHeader[1] = 'L';
		pFlvHeader[2] = 'V';
		pFlvHeader[3] = 0x01;
		pFlvHeader[4] = 0x01;

		unsigned int size = 9;
		u4 size_u4(size);
		memcpy(pFlvHeader + 5, size_u4._u, sizeof(unsigned int));
	}

	void CConverter::WriteHeader()
	{
		_fileOut.write((char *)_FlvHeader, 9);

		u4 prev_u4(_nPrevTagSize);
		_fileOut.write((char *)prev_u4._u, 4);

		char cTagType = 0x09;
		_fileOut.write(&cTagType, 1);
		int nDataSize = 1 + 1 + 3 + 6 + 2 + (_nSPSSize - 4) + 1 + 2 + (_nPPSSize - 4);

		u3 datasize_u3(nDataSize);
		_fileOut.write((char *)datasize_u3._u, 3);

		u3 tt_u3(_nTimeStamp);
		_fileOut.write((char *)tt_u3._u, 3);

		unsigned char cTTex = _nTimeStamp >> 24;
		_fileOut.write((char *)&cTTex, 1);

		u3 sid_u3(_nStreamID);
		_fileOut.write((char *)sid_u3._u, 3);

		unsigned char cVideoParam = 0x17;
		_fileOut.write((char *)&cVideoParam, 1);
		unsigned char cAVCPacketType = 0; /* seq header */
		_fileOut.write((char *)&cAVCPacketType, 1);

		u3 CompositionTime_u3(0);
		_fileOut.write((char *)CompositionTime_u3._u, 3);

		Write(1);
		Write(_pSPS[5]);
		Write(_pSPS[6]);
		Write(_pSPS[7]);
		Write(unsigned char(0xff));
		Write(unsigned char(0xE1));

		u2 spssize_u2(_nSPSSize - 4);
		_fileOut.write((char *)spssize_u2._u, 2);
		_fileOut.write((char *)(_pSPS + 4), _nSPSSize - 4);
		Write(unsigned char(0x01));

		u2 ppssize_u2(_nPPSSize - 4);
		_fileOut.write((char *)ppssize_u2._u, 2);
		_fileOut.write((char *)(_pPPS + 4), _nPPSSize - 4);

		_nPrevTagSize = 11 + nDataSize;
	}

	void CConverter::WriteFrame(char *pNalu, int nNaluSize)
	{
		int nNaluType = pNalu[4] & 0x1f;
		if (nNaluType == 7 || nNaluType == 8)
			return;

		u4 prev_u4(_nPrevTagSize);
		Write(prev_u4);

		Write(0x09);
		int nDataSize;
		nDataSize = 1 + 1 + 3 + 4 + (nNaluSize - 4);
		u3 datasize_u3(nDataSize);
		Write(datasize_u3);
		u3 tt_u3(_nTimeStamp);
		Write(tt_u3);
		Write(unsigned char(_nTimeStamp >> 24));

		u3 sid(_nStreamID);
		Write(sid);

		if (nNaluType == 5)
			Write(0x17);
		else
			Write(0x27);
		Write(unsigned char(1));
		u3 com_time_u3(0);
		Write(com_time_u3);

		u4 nalusize_u4(nNaluSize - 4);
		Write(nalusize_u4);

		_fileOut.write((char *)(pNalu + 4), nNaluSize - 4);

		if (nNaluType == 1 || nNaluType == 5)
			_nTimeStamp += 40;
		_nPrevTagSize = 11 + nDataSize;
	}

	void CConverter::WriteEndofSeq()
	{
		u4 prev_u4(_nPrevTagSize);
		Write(prev_u4);

		Write(0x09);
		int nDataSize;
		nDataSize = 1 + 1 + 3;
		u3 datasize_u3(nDataSize);
		Write(datasize_u3);
		u3 tt_u3(_nTimeStamp);
		Write(tt_u3);
		Write(unsigned char(_nTimeStamp >> 24));

		u3 sid(_nStreamID);
		Write(sid);

		Write(0x27);
		Write(0x02);

		u3 com_time_u3(0);
		Write(com_time_u3);
	}
}

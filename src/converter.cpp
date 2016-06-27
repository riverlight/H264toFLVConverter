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

		_pAudioSpecificConfig = NULL;
		_nAudioConfigSize = 0;
		_aacProfile = 0;
		_sampleRateIndex = 0;
		_channelConfig = 0;
		_bWriteAACSeqHeader = 0;
	}

	CConverter::~CConverter()
	{

	}

	int CConverter::Open(std::string strFlvFile, int bHaveAudio, int bHaveVideo)
	{
		_fileOut.open(strFlvFile, std::ios_base::out | std::ios_base::binary);
		if (!_fileOut)
			return 0;

		_bHaveAudio = bHaveAudio;
		_bHaveVideo = bHaveVideo;

		MakeFlvHeader(_FlvHeader);

		return 1;
	}

	int CConverter::Close()
	{
		if (_pSPS != NULL)
			delete _pSPS;
		if (_pPPS != NULL)
			delete _pPPS;

		WriteH264EndofSeq();
		_fileOut.close();

		return 1;
	}

	int CConverter::ConvertH264(char *pNalu, int nNaluSize)
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
			WriteH264Header();
			_bWriteAVCSeqHeader = 1;
		}
		if (_bWriteAVCSeqHeader == 0)
			return 1;

		WriteH264Frame(pNalu, nNaluSize);

		return 1;
	}

	void CConverter::MakeFlvHeader(unsigned char *pFlvHeader)
	{
		pFlvHeader[0] = 'F';
		pFlvHeader[1] = 'L';
		pFlvHeader[2] = 'V';
		pFlvHeader[3] = 0;
		if (_bHaveVideo!=0)
			pFlvHeader[3] |= 0x01;
		if (_bHaveAudio != 0)
			pFlvHeader[3] |= 0x04;
		pFlvHeader[4] = 0x01;

		unsigned int size = 9;
		u4 size_u4(size);
		memcpy(pFlvHeader + 5, size_u4._u, sizeof(unsigned int));

		_fileOut.write((char *)_FlvHeader, 9);
	}

	void CConverter::WriteH264Header()
	{
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

	void CConverter::WriteH264Frame(char *pNalu, int nNaluSize)
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

	void CConverter::WriteH264EndofSeq()
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

	int CConverter::ConvertAAC(char *pAAC, int nAACFrameSize)
	{
		if (pAAC == NULL || nAACFrameSize <= 7)
			return 0;

		if (_pAudioSpecificConfig == NULL)
		{
			_pAudioSpecificConfig = new unsigned char[2];
			_nAudioConfigSize = 2;

			unsigned char *p = (unsigned char *)pAAC;
			_aacProfile = (p[2] >> 6) + 1;
			_sampleRateIndex = (p[2] >> 2) & 0x0f;
			_channelConfig = ((p[2] & 0x01) << 2) + (p[3]>>6);

			_pAudioSpecificConfig[0] = (_aacProfile << 3) + (_sampleRateIndex>>1);
			_pAudioSpecificConfig[1] = ((_sampleRateIndex&0x01)<<7) + (_channelConfig<<3);
		}
		if (_pAudioSpecificConfig != NULL & _bWriteAACSeqHeader == 0)
		{
			WriteAACHeader();
			_bWriteAACSeqHeader = 1;
		}
		if (_bWriteAACSeqHeader == 0)
			return 1;

		WriteAACFrame(pAAC, nAACFrameSize);

		return 1;
	}

	void CConverter::WriteAACHeader()
	{
		u4 prev_u4(_nPrevTagSize);
		_fileOut.write((char *)prev_u4._u, 4);

		char cTagType = 0x08;
		_fileOut.write(&cTagType, 1);
		int nDataSize = 1 + 1 + 2;

		u3 datasize_u3(nDataSize);
		_fileOut.write((char *)datasize_u3._u, 3);

		u3 tt_u3(_nTimeStamp);
		_fileOut.write((char *)tt_u3._u, 3);

		unsigned char cTTex = _nTimeStamp >> 24;
		_fileOut.write((char *)&cTTex, 1);

		u3 sid_u3(_nStreamID);
		_fileOut.write((char *)sid_u3._u, 3);

		unsigned char cAudioParam = 0xAF;
		_fileOut.write((char *)&cAudioParam, 1);
		unsigned char cAACPacketType = 0; /* seq header */
		_fileOut.write((char *)&cAACPacketType, 1);

		_fileOut.write((char *)_pAudioSpecificConfig, 2);

		_nPrevTagSize = 11 + nDataSize;
	}

	void CConverter::WriteAACFrame(char *pFrame, int nFrameSize)
	{
		u4 prev_u4(_nPrevTagSize);
		Write(prev_u4);

		Write(0x08);
		int nDataSize;
		nDataSize = 1 + 1 + (nFrameSize - 7);
		u3 datasize_u3(nDataSize);
		Write(datasize_u3);
		u3 tt_u3(_nTimeStamp);
		Write(tt_u3);
		Write(unsigned char(_nTimeStamp >> 24));

		u3 sid(_nStreamID);
		Write(sid);

		unsigned char cAudioParam = 0xAF;
		_fileOut.write((char *)&cAudioParam, 1);
		unsigned char cAACPacketType = 1; /* AAC raw data */
		_fileOut.write((char *)&cAACPacketType, 1);

		_fileOut.write((char *)pFrame + 7, nFrameSize - 7);

		_nTimeStamp += double(1024 * 1000) / double(44100);
		_nPrevTagSize = 11 + nDataSize;
	}
}

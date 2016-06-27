#include <string.h>
#include <memory.h>

#include "splitter.h"

namespace Cnvt
{
	int GetOneNalu(unsigned char *pBufIn, int nInSize, unsigned char *pNalu, int &nNaluSize)
	{
		unsigned char *p = pBufIn;
		int nStartPos = 0, nEndPos = 0;
		while (1)
		{
			if (p[0] == 0 && p[1] == 0 && p[2] == 0 && p[3] == 1)
			{
				nStartPos = p - pBufIn;
				break;
			}
			p++;
			if (p - pBufIn >= nInSize - 4)
				return 0;
		}
		p++;
		while (1)
		{
			if (p[0] == 0 && p[1] == 0 && p[2] == 0 && p[3] == 1)
			{
				nEndPos = p - pBufIn;
				break;
			}
			p++;
			if (p - pBufIn >= nInSize - 4)
			{
				nEndPos = nInSize;
				break;
			}
		}
		nNaluSize = nEndPos - nStartPos;
		memcpy(pNalu, pBufIn + nStartPos, nNaluSize);

		return 1;
	}

	int IsVideojjSEI(unsigned char *pNalu, int nNaluSize)
	{
		unsigned char *p = pNalu;

		if (p[3] != 1 || p[4] != 6 || p[5] != 5)
			return 0;
		p += 6;
		while (*p++==0xff) ;
		const char *szVideojjUUID = "VideojjLeonUUID";
		char *pp = (char *)p;
		for (int i = 0; i < strlen(szVideojjUUID); i++)
		{
			if (pp[i] != szVideojjUUID[i])
				return 0;
		}
		
		return 1;
	}

	int GetOneAACFrame(unsigned char *pBufIn, int nInSize, unsigned char *pAACFrame, int &nAACFrameSize)
	{
		unsigned char *p = pBufIn;

		if (nInSize <= 7)
			return 0;

		int nFrameSize = ((p[3] & 0x3) << 11) + (p[4] << 3) + (p[5] >> 5);
		if (nInSize < nFrameSize)
			return 0;

		nAACFrameSize = nFrameSize;
		memcpy(pAACFrame, pBufIn, nFrameSize);

		return 1;
	}
}

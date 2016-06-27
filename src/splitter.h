#ifndef SPLITTER_H
#define SPLITTER_H

namespace Cnvt
{
	int GetOneNalu(unsigned char *pBufIn, int nInSize, unsigned char *pNalu, int &nNaluSize);
	int IsVideojjSEI(unsigned char *pNalu, int nNaluSize);

	int GetOneAACFrame(unsigned char *pBufIn, int nInSize, unsigned char *pAACFrame, int &nAACFrameSize);
}


#endif // SPLITTER_H

#ifndef SPLITTER_H
#define SPLITTER_H

namespace Cnvt
{
	int GetOneNalu(unsigned char *pBufIn, int nInSize, unsigned char *pNalu, int &nNaluSize);
	int IsVideojjSEI(unsigned char *pNalu, int nNaluSize);
}


#endif // SPLITTER_H

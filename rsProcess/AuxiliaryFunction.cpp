#include"stdafx.h"
#include"AuxiliaryFunction.h"

//дENVIͷ�ļ����ļ���
void WriteENVIHeader(const char* pathHeader, ENVIHeader mImgHeader)
{
	FILE* fpHead = NULL;
	errno_t err = fopen_s(&fpHead, pathHeader, "w");
	if (err != 0)
		exit(-1);

	if (!fpHead)
		exit(-1);

	fprintf(fpHead, "%s\n%s\n%s\n", "ENVI", "description = {", "File Imported into ENVI.}");
	fprintf(fpHead, "samples = %d\n", mImgHeader.imgWidth);					//д������Ԫ��
	fprintf(fpHead, "lines = %d\n", mImgHeader.imgHeight);				    //д������
	fprintf(fpHead, "bands = %d\n", mImgHeader.imgBands);					//д�벨����
	fprintf(fpHead, "file type = %s\n", "ENVI Standard");				    //д���ʾ
	fprintf(fpHead, "data type = %d\n", mImgHeader.datatype);
	fprintf(fpHead, "interleave = %s\n", mImgHeader.interleave.c_str());
	fprintf(fpHead, "sensor type = %s\n", "Unknown");
	fprintf(fpHead, "byte order = %d\n", mImgHeader.byteorder);
	fprintf(fpHead, "wavelength units = %s\n", "Unknown");

	//ͶӰ������ϵͳ
	if (mImgHeader.bimgGeoProjection)
	{
		fprintf(fpHead, "map info = { %s,", mImgHeader.mapInfo.projection.c_str());
		fprintf(fpHead, "%lf,", mImgHeader.mapInfo.adfGeotransform[1]);
		fprintf(fpHead, "%lf,", mImgHeader.mapInfo.adfGeotransform[5]);
		fprintf(fpHead, "%lf,", mImgHeader.mapInfo.adfGeotransform[0]);
		fprintf(fpHead, "%lf,", mImgHeader.mapInfo.adfGeotransform[3]);
		fprintf(fpHead, "%lf,", mImgHeader.mapInfo.adfGeotransform[2]);
		fprintf(fpHead, "%lf,", mImgHeader.mapInfo.adfGeotransform[4]);
		fprintf(fpHead, "%d,", mImgHeader.mapInfo.zone);
		fprintf(fpHead, "%s\n", mImgHeader.mapInfo.directions.c_str());
		fprintf(fpHead, "%s\n", mImgHeader.coordianteSys.c_str());
	}

	//����
	if (mImgHeader.bimgWaveLength)
	{
		fprintf(fpHead, "wavelength = {");
		for (int i = 0; i < mImgHeader.imgWaveLength.size(); ++i)
		{
			for (int j = 0; j + i < mImgHeader.imgWaveLength.size(); ++j)
				fprintf(fpHead, "%lf", mImgHeader.imgWaveLength[j + i]);
			fprintf(fpHead, "\n");
		}
		fprintf(fpHead, "}\n");
	}

	//�벨��
	if (mImgHeader.bimgFWHM)
	{
		fprintf(fpHead, "fwhm = {");
		for (int i = 0; i < mImgHeader.imgFWHM.size(); ++i)
		{
			for (int j = 0; j + i < mImgHeader.imgFWHM.size(); ++j)
				fprintf(fpHead, "%lf", mImgHeader.imgFWHM[j + i]);
			fprintf(fpHead, "\n");
		}
		fprintf(fpHead, "}\n");
	}

	//������
	if (mImgHeader.bimgBandNames)
	{
		fprintf(fpHead, "band names = {");
		for (int i = 0; i < mImgHeader.imgBandNnames.size(); ++i)
		{
			for (int j = 0; j + i < mImgHeader.imgBandNnames.size(); ++j)
				fprintf(fpHead, "%s", mImgHeader.imgBandNnames[j + i].c_str());
			fprintf(fpHead, "\n");
		}
		fprintf(fpHead, "}\n");
	}

	fclose(fpHead);		//�ر��ļ�
}

//��ȡ��ֵ�ͱ�׼��
void GetAveDev(unsigned char *pBuffer, int nSamples, int nLines, int nBand, float &fAverage, float &fDeviate)
{
	double dSum = 0, dMul = 0;
	__int64 nCount = 0;
	int i = 0, j = 0;
	int nOffset = 0;
	fAverage = 0;
	fDeviate = 0;
	int nPix = nLines*nSamples;
	for (i = 0; i<nPix; i++)
	{
		nOffset = nBand*nPix + i;
		if (pBuffer[nOffset] != 0)
		{
			dSum += pBuffer[nOffset];
			dMul += pBuffer[nOffset] * pBuffer[nOffset];
			nCount++;
		}
	}
	if (nCount == 0)
	{
		return ;
	}
	fAverage = (float)dSum / nCount;
	dMul /= nCount;
	dMul -= fAverage*fAverage;
	fDeviate = (float)sqrt(dMul);
	return ;
}
void GetAveDev(unsigned short *pBuffer, int nSamples, int nLines, int nBand, float &fAverage, float &fDeviate)
{
	double dSum = 0, dMul = 0;
	__int64 nCount = 0;
	int i = 0, j = 0;
	int nOffset = 0;
	fAverage = 0;
	fDeviate = 0;
	int nPix = nLines*nSamples;
	for (i = 0; i<nPix; i++)
	{
		nOffset = nBand*nPix + i;
		if (pBuffer[nOffset] != 0)
		{
			dSum += pBuffer[nOffset];
			dMul += pBuffer[nOffset] * pBuffer[nOffset];
			nCount++;
		}
	}
	if (nCount == 0)
	{
		return;
	}
	fAverage = dSum / nCount;
	dMul /= nCount;
	dMul -= fAverage*fAverage;
	fDeviate = (float)sqrt(dMul);
	return;
}

//�����ݽ��в���
void GetImgSample(unsigned short *pImgBuffer, DPOINT &minPt, DPOINT &maxPt, THREEDPOINT *pGoundPt, float fGSDX, float fGSDY, int nSamples, int nLines, int nReSamples, int nReLines, unsigned short *pRegBuffer)
{
	int i = 0, j = 0;			//����ѭ������
	DPOINT originPnt;			//Ӱ�����ϵ�
	originPnt.dX = minPt.dX;
	originPnt.dY = maxPt.dY;

	float *fDGrey = NULL;			//�Ҷȴ洢����
	float *fDItem = NULL;			//Ȩֵ�洢����
	try
	{
		fDGrey = new float[nReSamples*nReLines];
	}
	catch (bad_alloc)
	{
		exit(-1);
	}
	try
	{
		fDItem = new float[nReSamples*nReLines];
	}
	catch (bad_alloc )
	{
		exit(-1);
	}
	memset(fDGrey, 0, nReSamples*nReLines*sizeof(float));	//��ʼ����
	memset(fDItem, 0, nReSamples*nReLines*sizeof(float));	//��ʼ����

	for (i = 0; i<nLines; i++)
	{
#pragma omp parallel for
		for (j = 0; j<nSamples; j++)
		{
			unsigned short fDN = 0;
			float fTempGrey[4];						//����Ȩֵ����	
			int nC = 0, nY = 0;						//����ȡ������
			double fDX = 0, fDY = 0;				//����ȡ��������
			DPOINT presentPnt;
			presentPnt.dX = 0;
			presentPnt.dY = 0;

			long lPixOffset = i*nSamples + j;
			//����ԭʼӰ��(i,j)����Ϊ(x,y)����Ӧ��ͼ��λ��Ϊ
			presentPnt.dX = fabs(pGoundPt[lPixOffset].dX - originPnt.dX) / fGSDX;
			presentPnt.dY = fabs(pGoundPt[lPixOffset].dY - originPnt.dY) / fGSDY;
			//����presentPnt��������������ȡ��
			nC = (int)presentPnt.dX;
			nY = (int)presentPnt.dY;
			//����presentPnt��������������ȡ��
			fDX = presentPnt.dX - nC;
			fDY = presentPnt.dY - nY;
			fDN = pImgBuffer[lPixOffset];		//��ȡ��ǰ���ԭʼDNֵ

			fTempGrey[0] = (float)(1 - fDX)*(1 - fDY)*fDN;
			fTempGrey[1] = (float)fDX*(1 - fDY)*fDN;
			fTempGrey[2] = (float)(1 - fDX)*fDY*fDN;
			fTempGrey[3] = (float)fDX*fDY*fDN;

			if (nC >= 0 && nC<nReSamples && nY >= 0 && nY<nReLines)
			{
				long lOffset = 0;
				lOffset = nY*nReSamples + nC;
				fDGrey[lOffset] += fTempGrey[0];
				fDItem[lOffset] += (1 - fDX)*(1 - fDY);						//���ϵ�
				if (nC < nReSamples - 1)	//δ�����ұ߽�
				{
					fDGrey[lOffset + 1] += (float)fTempGrey[1];
					fDItem[lOffset + 1] += (float)fDX*(1 - fDY);					//���ϵ�
				}
				if (nY < nReLines - 1)	//δ�����±߽�
				{
					fDGrey[lOffset + nReSamples] += (float)fTempGrey[2];
					fDItem[lOffset + nReSamples] += (float)(1 - fDX)*fDY;		//���µ�
				}
				if (nC<nReSamples - 1 && nY<nReLines - 1)
				{
					fDGrey[lOffset + nReSamples + 1] += fTempGrey[3];
					fDItem[lOffset + nReSamples + 1] += fDX*fDY;			//���µ�
				}
			}
		}
	}
	for (i = 0; i<nReLines; i++)
	{
#pragma omp parallel for
		for (j = 0; j<nReSamples; j++)
		{
			long lOffset = i*nReSamples + j;
			if (fDItem[lOffset] != 0)
			{
				pRegBuffer[lOffset] = unsigned short(fDGrey[lOffset] / fDItem[lOffset]);
			}
			else	//�޸��ڵ�
			{
				if (i>0 && i<nReLines - 1 && j>0 && j<nReSamples - 1)	//�����ڱ߽�λ��
				{
					float fSumValues = 0;
					int nCount = 0;

					if (fDItem[lOffset - nReSamples - 1] != 0)	//����
					{
						nCount++;
						fSumValues += fDGrey[lOffset - nReSamples - 1] / fDItem[lOffset - nReSamples - 1];
					}
					if (fDItem[lOffset - nReSamples] != 0)		//��
					{
						nCount++;
						fSumValues += fDGrey[lOffset - nReSamples] / fDItem[lOffset - nReSamples];
					}
					if (fDItem[lOffset - nReSamples + 1] != 0)	//����
					{
						nCount++;
						fSumValues += fDGrey[lOffset - nReSamples + 1] / fDItem[lOffset - nReSamples + 1];
					}
					if (fDItem[lOffset - 1] != 0)					//��
					{
						nCount++;
						fSumValues += fDGrey[lOffset - 1] / fDItem[lOffset - 1];
					}
					if (fDItem[lOffset + 1] != 0)					//��
					{
						nCount++;
						fSumValues += fDGrey[lOffset + 1] / fDItem[lOffset + 1];
					}
					if (fDItem[lOffset + nReSamples - 1] != 0)	//����
					{
						nCount++;
						fSumValues += fDGrey[lOffset + nReSamples - 1] / fDItem[lOffset + nReSamples - 1];
					}
					if (fDItem[lOffset + nReSamples] != 0)		//��
					{
						nCount++;
						fSumValues += fDGrey[lOffset + nReSamples] / fDItem[lOffset + nReSamples];
					}
					if (fDItem[lOffset + nReSamples + 1] != 0)	//����
					{
						nCount++;
						fSumValues += fDGrey[lOffset + nReSamples + 1] / fDItem[lOffset + nReSamples + 1];
					}
					if (nCount >= 5)	//�����Χ��������ϲ��Ǻڵ�ͽ��о�ֵ����
					{
						pRegBuffer[lOffset] = unsigned short(fSumValues / nCount);
					}
				}
			}
		}
	}
	if (fDGrey)
	{
		delete[]fDGrey;
		fDGrey = NULL;
	}
	if (fDItem)
	{
		delete[]fDItem;
		fDGrey = NULL;
	}
}
void GetImgSample(float *pImgBuffer, DPOINT &minPt, DPOINT &maxPt, THREEDPOINT *pGoundPt, float fGSDX, float fGSDY, int nSamples, int nLines, int nReSamples, int nReLines, float *pRegBuffer)
{
	int i = 0, j = 0;			//����ѭ������
	DPOINT originPnt;			//Ӱ�����ϵ�
	originPnt.dX = minPt.dX;
	originPnt.dY = maxPt.dY;

	float *fDGrey = NULL;			//�Ҷȴ洢����
	float *fDItem = NULL;			//Ȩֵ�洢����
	try
	{
		fDGrey = new float[nReSamples*nReLines];
	}
	catch (bad_alloc )
	{
		exit(-1);
	}
	try
	{
		fDItem = new float[nReSamples*nReLines];
	}
	catch (bad_alloc )
	{
		exit(-1);
	}
	memset(fDGrey, 0, nReSamples*nReLines*sizeof(float));	//��ʼ����
	memset(fDItem, 0, nReSamples*nReLines*sizeof(float));	//��ʼ����

	for (i = 0; i<nLines; i++)
	{
#pragma omp parallel for
		for (j = 0; j<nSamples; j++)
		{
			float fDN = 0;
			float fTempGrey[4];						//����Ȩֵ����	
			int nC = 0, nY = 0;						//����ȡ������
			double fDX = 0, fDY = 0;				//����ȡ��������
			DPOINT presentPnt;
			presentPnt.dX = 0;
			presentPnt.dY = 0;

			long lPixOffset = i*nSamples + j;
			//����ԭʼӰ��(i,j)����Ϊ(x,y)����Ӧ��ͼ��λ��Ϊ
			presentPnt.dX = fabs(pGoundPt[lPixOffset].dX - originPnt.dX) / fGSDX;
			presentPnt.dY = fabs(pGoundPt[lPixOffset].dY - originPnt.dY) / fGSDY;
			//����presentPnt��������������ȡ��
			nC = (int)presentPnt.dX;
			nY = (int)presentPnt.dY;
			//����presentPnt��������������ȡ��
			fDX = presentPnt.dX - nC;
			fDY = presentPnt.dY - nY;
			fDN = pImgBuffer[lPixOffset];		//��ȡ��ǰ���ԭʼDNֵ

			fTempGrey[0] = (float)(1 - fDX)*(1 - fDY)*fDN;
			fTempGrey[1] = (float)fDX*(1 - fDY)*fDN;
			fTempGrey[2] = (float)(1 - fDX)*fDY*fDN;
			fTempGrey[3] = (float)fDX*fDY*fDN;

			if (nC >= 0 && nC<nReSamples && nY >= 0 && nY<nReLines)
			{
				long lOffset = 0;
				lOffset = nY*nReSamples + nC;
				fDGrey[lOffset] += fTempGrey[0];
				fDItem[lOffset] += (1 - fDX)*(1 - fDY);						//���ϵ�
				if (nC < nReSamples - 1)	//δ�����ұ߽�
				{
					fDGrey[lOffset + 1] += fTempGrey[1];
					fDItem[lOffset + 1] += fDX*(1 - fDY);					//���ϵ�
				}
				if (nY < nReLines - 1)	//δ�����±߽�
				{
					fDGrey[lOffset + nReSamples] += fTempGrey[2];
					fDItem[lOffset + nReSamples] += (1 - fDX)*fDY;		//���µ�
				}
				if (nC<nReSamples - 1 && nY<nReLines - 1)
				{
					fDGrey[lOffset + nReSamples + 1] += (float)fTempGrey[3];
					fDItem[lOffset + nReSamples + 1] += (float)fDX*fDY;			//���µ�
				}
			}
		}
	}
	for (i = 0; i<nReLines; i++)
	{
#pragma omp parallel for
		for (j = 0; j<nReSamples; j++)
		{
			long lOffset = i*nReSamples + j;
			if (fDItem[lOffset] != 0)
			{
				pRegBuffer[lOffset] = float(fDGrey[lOffset] / fDItem[lOffset]);
			}
			else	//�޸��ڵ�
			{
				if (i>0 && i<nReLines - 1 && j>0 && j<nReSamples - 1)	//�����ڱ߽�λ��
				{
					float fSumValues = 0;
					int nCount = 0;

					if (fDItem[lOffset - nReSamples - 1] != 0)	//����
					{
						nCount++;
						fSumValues += fDGrey[lOffset - nReSamples - 1] / fDItem[lOffset - nReSamples - 1];
					}
					if (fDItem[lOffset - nReSamples] != 0)		//��
					{
						nCount++;
						fSumValues += fDGrey[lOffset - nReSamples] / fDItem[lOffset - nReSamples];
					}
					if (fDItem[lOffset - nReSamples + 1] != 0)	//����
					{
						nCount++;
						fSumValues += fDGrey[lOffset - nReSamples + 1] / fDItem[lOffset - nReSamples + 1];
					}
					if (fDItem[lOffset - 1] != 0)					//��
					{
						nCount++;
						fSumValues += fDGrey[lOffset - 1] / fDItem[lOffset - 1];
					}
					if (fDItem[lOffset + 1] != 0)					//��
					{
						nCount++;
						fSumValues += fDGrey[lOffset + 1] / fDItem[lOffset + 1];
					}
					if (fDItem[lOffset + nReSamples - 1] != 0)	//����
					{
						nCount++;
						fSumValues += fDGrey[lOffset + nReSamples - 1] / fDItem[lOffset + nReSamples - 1];
					}
					if (fDItem[lOffset + nReSamples] != 0)		//��
					{
						nCount++;
						fSumValues += fDGrey[lOffset + nReSamples] / fDItem[lOffset + nReSamples];
					}
					if (fDItem[lOffset + nReSamples + 1] != 0)	//����
					{
						nCount++;
						fSumValues += fDGrey[lOffset + nReSamples + 1] / fDItem[lOffset + nReSamples + 1];
					}
					if (nCount >= 5)	//�����Χ��������ϲ��Ǻڵ�ͽ��о�ֵ����
					{
						pRegBuffer[lOffset] = float(fSumValues / nCount);
					}
				}
			}
		}
	}
	if (fDGrey)
	{
		delete[]fDGrey;
		fDGrey = NULL;
	}
	if (fDItem)
	{
		delete[]fDItem;
		fDGrey = NULL;
	}
}

//��ȡֱ��ͼƥ����ֱ��ͼ
//Ӱ��2��ֱ��ͼΪ��׼
void GetImgHistroMatch(double* img1, double *img2, int xsize1, int ysize1, int xsize2, int ysize2, int minPix, int maxPix, int* histroMap)
{
	int *ihistro1 = NULL;
	double* fhistro1 = NULL;
	int *ihistro2 = NULL;
	double* fhistro2 = NULL;
	try
	{
		ihistro1 = new int[maxPix-minPix];
		fhistro1 = new double[maxPix - minPix];
		ihistro2 = new int[maxPix - minPix];
		fhistro2 = new double[maxPix - minPix];

		memset(ihistro1, 0, sizeof(int)*(maxPix - minPix));
		memset(ihistro2, 0, sizeof(int)*(maxPix - minPix));
		memset(fhistro1, 0, sizeof(double)*(maxPix - minPix));
		memset(fhistro2, 0, sizeof(double)*(maxPix - minPix));
	}
	catch (bad_alloc)
	{
		printf("allocate memory error\n");
		exit(-1);
	}

	for (int i = 0; i<xsize1*ysize1; ++i)
		ihistro1[int(img1[i])-minPix]++;
	for (int i = 0; i<xsize2*ysize2; ++i)
		ihistro2[int(img2[i]) - minPix]++;
	for (int i = minPix; i<maxPix; ++i)
	{
		fhistro1[i] = double(ihistro1[i]) / xsize1 / ysize1;
		fhistro2[i] = double(ihistro2[i]) / xsize2 / ysize2;
	}
	//�ۻ�ֱ��ͼ
	for (int i = 1; i<maxPix; ++i)
	{
		fhistro1[i] = fhistro1[i - 1] + fhistro1[i];
		fhistro2[i] = fhistro2[i - 1] + fhistro2[i];
	}



	//ֱ��ͼƥ��
	double m_diffA, m_diffB;  int k = 0;
	for (int i = 0; i < maxPix - minPix; i++)
	{
		m_diffB = 1;
		for (int j = k; j < maxPix - minPix; j++)
		{
			m_diffA = abs(fhistro1[i] - fhistro2[j]);
			if (m_diffA - m_diffB < 1.0E-5)
			{
				m_diffB = m_diffA;
				k = j;
			}
			else
			{
				k = j - 1;
				break;
			}
		}
		if (k == maxPix - minPix - 1)
		{
			for (int l = i; l < maxPix - minPix; l++)
				histroMap[l] = k;
			break;
		}
		histroMap[i] = k;
	}

	//�����ڴ�
	delete[]ihistro1;
	delete[]ihistro2;
	delete[]fhistro1;
	delete[]fhistro2;
}
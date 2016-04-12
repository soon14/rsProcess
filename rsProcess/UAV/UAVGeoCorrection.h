#pragma once

#include"..\GeoPOSProcess.h"
#include"..\AerialProduct\Level2Process.h"

//���˻�Ӱ��POS���ݴ���
class UAVPOSProcess : public GeoPOSProcess
{
public:
	//1.��ȡӰ������
	long GeoPOSProc_ReadPartPOS(const char *pPOSFile, long nLines, double &dB, double &dL, double &dH, int nbeginLine);

	//2.���˻�Ӱ��һ�㲻��Ҫ����SBET�ļ������˻�POS���ݿ���ֱ�ӻ�ȡ
	//2016/04/10 ���ڷ���Ӱ���ȡ��ʱ���GPSʱ����ڲ��죬���Ǹ���Ӱ��ʱ���ڲ�GPSʱ�����
	long GeoPOSProc_ExtractSBET(const char* pathSBET, const char* pathEvent, const char* pathPOS, float fOffsetGPS);

	//3.�ɵ���POS���ݽ���EOԪ�أ����˻������ǰ��ýǺͰ���ʸ��
	long GeoPOSProc_ExtractEO(POS m_perpos, EO &m_pereo);
};

//UAV�ļ���У������ɨʽ��Ӱ�񼸺�У���бȽϴ�Ĳ���
//UAV��һ��Ӱ��ֻ��1��POS��Ϣ
//1.��У��������Ӧ�ý����������Ϊ��֪��
//2.��POS���ݽ����EOԪ��
//3.ͨ��EOԪ�ؽ���У��
class UAVGeoCorrection : public Level2Process
{
public:
	//�������صĺ���
	long Level2Proc_Product2A(const char *pFile, const char *pCFile, const char *pEOFile, float fGSDX, float fGSDY, double fElevation, int nEOOffset,
		float fFov, float fIFov, float fFocalLen, bool bIGM, const char *pIGMFile, bool bInverse = false) {
		printf("UAV Image not product\n"); 
		return 0;
	}
	long Level2Proc_Product2B(const char *pFile, const char *pCFile, const char *pEOFile, float fGSDX, float fGSDY, double fElevation, int nEOOffset,
		float fFov, float fIFov, float fFocalLen, const char* pDEMFile, bool bIGM, const char *pIGMFile, bool bInverse = false) {
		printf("UAV Image not product\n");
		return 0;
	}

	//1.��Ӱ�񼯽���У��
	long UAVGeoCorrection_GeoCorrect(const char* pathSrcDir, const char* pathDstDir, const char* pathPOS, const char* pathDEM,
																			int IMGbegline, int POSbegline, int lines,double fLen , double fGSD, double AvgHeight);

	//2.�Ե���Ӱ�����У��
	long UAVGeoCorrection_GeoCorrect(const char* pathSrc, const char* pathDst, EO  m_preeo, double dL, double dB, double fLen, double fGSD, double AvgHeight, char* pathDEM = NULL);

	//3.����ÿ����ĵ�������
	long UAVGeoCorrection_GeoPntsProb(double dB, double dL, double dH, EO pEO, double fLen, int width, int height, THREEDPOINT *pGoundPnt, DPOINT* pMapImgPnt, int pntNum);

	//4.��ȷ����ÿ���������Ȼ�����У��
	long UAVGeoCorrection_GeoPntsAccu(double dB, double dL, double dH, EO pEO, double fLen, int width, int height, THREEDPOINT *pGoundPnt,const char* pathDEM);

	//5.����Ӱ���ȡ��ʱ�䣬�о�����ʱ���GPSʱ��֮���в��죬������ͬʱ��
	long UAVGeoCorrection_ExifTime(const char* pathDir, int begImgNum, int imageNumbers, const char* pathTime);

};
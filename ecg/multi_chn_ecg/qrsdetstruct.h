/******************************************************************************

    文件名: qrsdetstruct.h
    描述： 此头文件定义QRS复合波查找函数所要用到的滤波器的结构体及复合波查找
	       所用中间量的结构体
	引用： qrsdet.h
	被引用： run.h
	其他： 无
	函数列表： 无

******************************************************************************/
#ifndef qrsdetstruct_h__
#define qrsdetstruct_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "qrsdet.h"
//将中值数组（或平均数组）长度定义为宏，程序灵活性更大
#define arrayLen  8

//低通滤波结构体
typedef struct
{
	int dataoutone;//y（n-1），初始化为0
	int dataouttwo;//y（n-2），初始化为0
	int datain[LPBUFFER_LGTH];//输入值队列，初始化为0
	int lptr;//队列下标，初始化为0
}qrsLpStruct;

//高通滤波结构体
typedef struct
{
	int dataoutone;//y（n-1），初始化为0
	int datain[HPBUFFER_LGTH];//输入值队列，初始化为0
	int hptr;//队列下标，初始化为0
}qrsHpStruct;

//差分滤波结构体
typedef struct
{
	int datain[DERIV_LENGTH];//输入值队列，初始化为0
	int derptr;//队列下标，初始化为0
}qrsDerStruct;

//积分滤波结构体
typedef struct
{
	int sum;//初始化为0
	int datain[WINDOW_WIDTH];//输入值队列，初始化为0
	int mvptr;//队列下标，初始化为0
}qrsMvStruct;

//将以上结构体定义到总结构体中
typedef struct
{
	qrsLpStruct qrsLpStr;
	qrsHpStruct qrsHpStr;
	qrsDerStruct qrsDerStr[2];//这个要考虑一下
	qrsMvStruct qrsMvStr;
}qrsFilterStruct;

//qrsdet.c中peak函数所使用的结构体
typedef struct 
{
	int maxvalue;//初始化为0
	int timeSinceMax;//初始化为0
	int lastDatum;//初始化为0
}qrsPeakStruct;

//qrsdet.c中qrsdet函数所使用的结构体
typedef struct  
{
	int det_thresh;//初始化为0
	int qpkcnt;//初始化为0
	int rsetCount;//初始化为0
	int nmedian;//初始化为0
	int qmedian;//初始化为0
	int rrmedian;//初始化为0
	int countnum;//初始化为0
	// sbpeak: search back peak
	int sbpeak;//初始化为0
	// sbloc: search back location
	int sbloc;//初始化为0
	// sbcount: search back count
	int sbcount;//初始化为MS1500
	int initBlank;//初始化为0
	int initMax;//初始化为0
	int preBlankCnt;//初始化为0
	int tempPeak;//初始化为0
	int qrsbuf[arrayLen];//初始化为0
	int noise[arrayLen];//初始化为0
	int rsetBuff[arrayLen];//初始化为0
	int rrbuf[arrayLen];//初始化为MS1000
	qrsPeakStruct qrsPeakStr;
	qrsFilterStruct qrsFilterStr;
}qrsDetStruct;


#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif // qrsdetstruct_h__
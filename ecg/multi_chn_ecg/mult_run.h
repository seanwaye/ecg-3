#ifndef mult_run_h__
#define mult_run_h__

#ifdef __cplusplus
extern "C" {
#endif

#include "qrsdet.h"
#include "predefinefilter.h"
#include "qrsdetstruct.h"

//使得实时心率不会变化太大，策略，将长度定义为宏
#define HrArLen 5

//定义每通道心电AD转换精度及
//算法通道数
//在初始化时传入，由调用者赋值给此结构体
typedef struct
{
	//心电AD转换精度
	real precision[8];
	//算法通道数
	int algoChanArray[8];
}InitVarStruct;


//维护心电数据的结构体，实际为一数组
typedef struct 
{
	//存放滤波后滤波值的循环数组
	int ecgbuf[bufferlength];
	//存到当前信号点数循环数组中的下标
	int ecgptr;
	//当前信号采样点数（此时发生delay）
	int ecgnumber;
	////上一QRS复合波delay发生点
	//int lastecgnumber;
	////上一次QRS复合波终点与delay发生点之间的偏移量
	//int lastOffSft;
	////上次之上次QRS复合波终点与delay发生点之间的偏移量
	//int thiOffSft;
}EcgqueueStruct;


//显示屏输出的结果结构体
typedef struct 
{
	int filterEcgValue;
}OutDataStruct;


typedef struct  
{
	int qrsLoc;

	int heartRate;
	int HrArray[HrArLen];//目的是使得实时心率不会变化太大，策略 
	int lastHRate;
	int qrsnum;

	int delayflag;

	int algoChan;

	EcgqueueStruct Ecgqueue;
	OutDataStruct OutDate;
	//Displaydatastruct Displaydata;
	FilterDataStruct FilterData;
	qrsDetStruct qrsDetStr;

}ChannelAnalysisStruct;

typedef struct
{
	ChannelAnalysisStruct *ChannelAnalysis;
	int channelNum;
}TotalAnalysisStruct;

int mult_initvar(TotalAnalysisStruct* TotalStruct, 
	ChannelAnalysisStruct* ChannelStruct, int channelNum, InitVarStruct* InitVarStr);

int mult_run(TotalAnalysisStruct* TotalStruct, int* proArray, int lenArray);


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif // mult_run_h__

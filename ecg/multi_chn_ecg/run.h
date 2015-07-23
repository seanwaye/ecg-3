#ifndef __RUN_H__
#define __RUN_H__

#include "qrsdet.h"
#ifdef __cplusplus
extern "C" {
#endif
struct AnalysisData 
{
	int qrsLoc;
	int qrsOnLoc;
	int qrsOffLoc;
	int qrsWide;
	int stDif;
	char beaType;
	int rhyme;
	int filterEcgValue;
	int delay;
	//二次滤波的数据
	int maintainrcgvalue;

	int heartRate;
	int HrArray[5];
	int lastHRate;

	int qrsnum;

	int sumRR;
	int sumRefStVa;
	int refStNum;
	int sumStValue;
	int sumStNum;
	int abnorBeatNum;
	int delayflag;//长时间未找到QRS复合波标志位
	int asystoleflag;

// 	int qrsArray[4000];
// 	int stDifArray[4000];
// 	char beaTypeArray[4000];
// 	int heartRateArray[4000];
};

struct Ecgqueue
{
	//存放滤波后滤波值的循环数组
	int ecgbuf[bufferlength];
	//存到当前信号点数循环数组中的下标
	int ecgptr;
	//当前信号采样点数（此时发生delay）
	int ecgnumber;
	//上一QRS复合波delay发生点
	int lastecgnumber;
	//上一次QRS复合波终点与delay发生点之间的偏移量
	int lastOffSft;
	//上次之上次QRS复合波终点与delay发生点之间的偏移量
	int thiOffSft;
};

//存放的显示数组
/*
struct Displaydata
{
	int displaybuf[displaylength];
	int saveptr;
	int displayptr;
	int delayflag;
	int lastdelayflag;
	int oncedelayflag;
	int bufferlengthflag;
	int judgeflag;
	int flagOfdefg;  //flag of delayflag
	//维护下标，目的是维护显示数据所需的在displaybuf循环数组的下标
	int maintainptr;
};*/

struct IndexHeart
{
	int score;
	int avrHeart;
	int avrStValue;
	int healthLev;

	int heartJudge;
	int typeJudge;
	int stJudge;
	int detailtype;
};

int initvar(struct Ecgqueue* queue, struct AnalysisData* dataout, 
            struct IndexHeart* heartIndex);

int run(struct AnalysisData* outStruct, int* proArray,
	    int lenArray, struct Ecgqueue* queue);

#ifdef __cplusplus
}  /* extern "C" */
#endif
#endif /* __RUN_H__ */

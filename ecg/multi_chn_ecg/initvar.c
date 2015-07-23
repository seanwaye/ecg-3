/******************************************************************************

    函数名： initvar()
	语法： int initvar(struct Ecgqueue* queue, struct AnalysisData* dataout, 
	            struct IndexHeart* heartIndex, struct datadisplay* datadisplay)
	描述： 对三个结构体中的变量进行初始化及初始化两个函数
	原理： 
	调用： qrsdet()、arrhyalgone()
	被调用：主函数
	输入参数： 四结构体
	输出参数： 无
	返回值： 0表示初始化正常
	其他： 暂时未对异常判断，如判断指针是否为空，空间大小是否足够等等
	       后面添加
    注意： 
	 
******************************************************************************/
#include <string.h>
#include "assessHeart.h"
#include "run.h"
#include "detect.h"


int initvar(struct Ecgqueue* queue, struct AnalysisData* dataout, 
            struct IndexHeart* heartIndex)
{
	int liv_ctr = 0;
	//这里初始化buf
	for (queue->ecgptr = 0; queue->ecgptr < bufferlength; ++(queue->ecgptr))
	{
		queue->ecgbuf[queue->ecgptr] = 0;
	}
	queue->ecgptr = 0;
	queue->ecgnumber = 0;
	queue->lastecgnumber = 0;
	queue->lastOffSft = 0;
	queue->thiOffSft = 0;

	dataout->qrsLoc = 0;
	dataout->qrsOnLoc = 0;
	dataout->qrsOffLoc = 0;
	dataout->qrsWide = 0;
	dataout->stDif = 0;
	dataout->heartRate = 0;
	dataout->lastHRate = 0;
	for (liv_ctr = 0; liv_ctr < 5; ++liv_ctr)
	{
		dataout->HrArray[liv_ctr] = 0;
	}
	dataout->filterEcgValue = 0;
	dataout->maintainrcgvalue = 0;
	dataout->delay = 0;
	dataout->qrsnum = 0;
	dataout->sumRR = 0;
	dataout->sumRefStVa = 0;
	dataout->sumStValue = 0;
	dataout->refStNum = 0;
	dataout->sumStNum = 0;
	dataout->abnorBeatNum = 0;
	dataout->delayflag = 0;
	dataout->asystoleflag = 0;

	heartIndex->avrHeart = 0;
	heartIndex->avrStValue = 0;
	heartIndex->score = 0;
	heartIndex->healthLev = 0;
	heartIndex->heartJudge = 0;
	heartIndex->typeJudge = 0;
	heartIndex->stJudge = 0;
	heartIndex->detailtype = 0;

/*
	memset(datadisplay->displaybuf, 0, sizeof(datadisplay->displaybuf));
	datadisplay->saveptr = 0;
	datadisplay->displayptr = 0;
	datadisplay->delayflag = 0;
	datadisplay->lastdelayflag = 0;
	datadisplay->oncedelayflag = 0;
	datadisplay->bufferlengthflag = 0;
	datadisplay->judgeflag = 0;
	datadisplay->flagOfdefg = 0;
	datadisplay->maintainptr = 0;*/

	qrsdet( 0, 1 );
	subpowerline250(0, 1);
	sub_dc_offset_250(0, 1);
	sub_hf_250_fir_1(0, 1);
	sub_hf_250_fir_2(0, 1);
	smooth_5_points(0, 1);
	arrhyalgone( 0, 0, &dataout->beaType, &dataout->rhyme, 1);

	return(0);
}

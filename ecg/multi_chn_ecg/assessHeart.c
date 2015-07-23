/******************************************************************************

    函数名： assessHeart()
	语法： int assessHeart(struct IndexHeart* heartIndex,
	                                          struct AnalysisData* outStruct)
	描述： 对心率、心律及ST段各自做出打分，并由打分值做出健康水平的判断
	原理： 参考一些心电知识及心电产品的标准
	调用： 无
	被调用：主函数
	输入参数： outStruct结构体
	输出参数： heartIndex结构体，两结构体具体定义将相关头文件
	返回值： 0表示输出正常
	其他： 暂时未对异常判断，如判断指针是否为空，空间大小是否足够等等
	       后面添加
    注意：

******************************************************************************/
#include <math.h>
#include "assessHeart.h"
#include "run.h"
#include "qrsdet.h"
#include <stdlib.h>

int assessHeart(struct IndexHeart* heartIndex, struct AnalysisData* outStruct)
{
	int qrsnum = 0;
	int liv_ctr = 0;
	int refStnum = 0;
	int realRefStnum = 0;
	int sumAvrSt = 0;
	int refStValue = 0;
	int avrStValue = 0;
	int absStValue = 0;
	int avrStnum = 0;
	int avrHeart = 0;
	double avrRR = 0;
	int sumRR = 0;
	int heartScore = 0;
	int stScore = 0;
	int arrScore = 0;
	int norBeatNum = 0;
	int abnorBeatNum = 0;
	int sumScore = 0;
	int heaLevel = 0;
	int heartJudge = 0;
	int typeJudge = 0;
	int stJudge = 0;
	int detailtype = 0;


	if (outStruct->refStNum > 0)
	{
		refStValue = outStruct->sumRefStVa / outStruct->refStNum;
	}
	else
	{
		refStValue = 0;
	}

	if (outStruct->sumStNum > 0)
	{
		avrStValue = outStruct->sumStValue / outStruct->sumStNum;
	}
	else
	{
		avrStValue = 0;
		refStValue = 0;
	}

	avrStValue = avrStValue - refStValue;//第一个输出值
	//若采用ADS129x，则精度为5*21，若为数据库则为5，若为nerosky则为5*【3or4】
	avrStValue = avrStValue / 20;//注意此处亦与数据相关--采样精度

	//第二个输出值
	if (outStruct->qrsnum > 1)
	{
		avrRR = outStruct->sumRR / (outStruct->qrsnum - 1);
		avrRR = avrRR / SAMPLE_RATE;
		avrRR = 60 / avrRR;
		avrHeart = (int)avrRR;
	}
	else
	{
		avrHeart = 0;
	}

    abnorBeatNum = outStruct->abnorBeatNum;//第三个输出值


	//以下打分
	//心率分值
	if ((avrHeart <= 100) && (avrHeart >= 60))
	{
		heartScore = 30;
		heartJudge = 1;
	}
	else if ((avrHeart <= 50) || (avrHeart >= 120))
	{
		heartScore = 0;
		if (avrHeart <= 50)
		{
			heartJudge = 4;
		}
		else
		{
			heartJudge = 5;
		}
	}
	else if ((avrHeart > 50) && (avrHeart < 60))
	{
		heartScore = 30 - 3 * (60 - avrHeart);
		heartJudge = 2;
	}
	else if ((avrHeart > 100) && (avrHeart < 120))
	{
		heartScore = 180 - 1.5 * avrHeart;
		heartJudge = 3;
	}

	//ST段值
	absStValue = abs(avrStValue);
	if (absStValue <= 10)
	{
		stScore = 40;
		stJudge = 1;
	}
	else if (absStValue >= 20)
	{
		stScore = 0;
		if (avrStValue >= 20)
		{
			stJudge = 4;
		}
		else
		{
			stJudge = 5;
		}
	}
	else
	{
		stScore = 80 - 4 * absStValue;
		if (avrStValue > 10)
		{
			stJudge = 2;
		}
		else
		{
			stJudge = 3;
		}
	}
	//心律不齐打分
	//前5个心跳默认为N，故要从第六跳开始计算


	if (abnorBeatNum <= 0)
	{
		arrScore = 30;
		typeJudge = 1;
	}
	else if (abnorBeatNum >= 6)
	{
		arrScore = 0;
		typeJudge = 4;
	}
	else
	{
		arrScore = 30 - 5 * abnorBeatNum;
		if (abnorBeatNum >= 3)
		{
			typeJudge = 3;
		}
		else
		{
			typeJudge = 2;
		}
	}

	detailtype = outStruct->rhyme;

	/*************************************************************************/
	//2013年7月2日添加，目的是处理心率为0左右时，即未找到QRS波时出现的情况
	if (avrHeart <= 30 || outStruct->asystoleflag)
	{
		heartScore = 0;
		heartJudge = 6;
		stScore = 0;
		stJudge = 6;
		arrScore = 0;
		typeJudge = 6;
		detailtype = 6;
	}

	if (detailtype == 10)
	{
		arrScore = 20;
	}


	/*************************************************************************/

	sumScore = heartScore + stScore + arrScore;

	if (sumScore >= 90)
	{
		heaLevel = 5;
	}
	else if (sumScore <= 70)
	{
		heaLevel = 3;
	}
	else
	{
		heaLevel = 4;
	}

	/*************************************************************************/
	//2013年7月2日添加，目的是处理心率为0左右时，即未找到QRS波时出现的情况
	if (avrHeart <= 30 || outStruct->asystoleflag)
	{
		heaLevel = 6;
	}
	/*************************************************************************/

	heartIndex->score = sumScore;
	heartIndex->avrHeart = avrHeart;
	heartIndex->avrStValue = avrStValue;
	heartIndex->healthLev = heaLevel;
	heartIndex->heartJudge = heartJudge;
	heartIndex->typeJudge = typeJudge;
	heartIndex->stJudge = stJudge;
	heartIndex->detailtype = detailtype;

	return(0);
}

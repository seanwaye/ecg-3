/******************************************************************************

    函数名： statfun()
	语法： int statfun( ChannelAnalysisStruct* ChanAnaStr, int lastqrsLOc )
	描述： 统计函数，统计在run中之前调用的函数得到的心电特征量，以传递给
	       assessHeart函数使用，由之前版本的run中独立代码发展而来，减少run
		   函数的代码量
	原理： 
	调用：标准库中的一些函数
	被调用：int run(TotalAnalysisStruct* TotalStruct, 
	                int* proArray, int lenArray)
	输入参数： 上一QRS复合波发生处
	输出参数： 结构outStruct，以上结构具体定义见run.h
	返回值：具体意义未明确定下
	其他： 以后还可对返回值进行更详细的定义
    注意： 
	 
******************************************************************************/
#include <string.h>
#include "qrsdet.h"
#include "mult_run.h"

int statfun( ChannelAnalysisStruct* ChanAnaStr, int lastqrsLOc )
{
	int lastHRate = 0;
	real rrInterval = 0;
	real rrInter = 0;
	real heartRate = 0;
	int HRate = 0;
	const int memmovelen = (HrArLen - 1) * sizeof(int);
	int zeroFlag = 0, flagCtr = 0;
	int sumHRate = 0, plusTimes = 0;
	
	//得输出心率
	if (lastqrsLOc && ChanAnaStr->qrsLoc)
	{
		rrInterval = (real)(ChanAnaStr->qrsLoc - lastqrsLOc);
		rrInter = rrInterval / SAMPLE_RATE;
		heartRate = 60 / rrInter;
		HRate = (int)heartRate;
		if (ChanAnaStr->lastHRate == 0)
		{
			ChanAnaStr->lastHRate = HRate;
		} 
		else if (((0.7 * ChanAnaStr->heartRate <= HRate) 
			&& (HRate <= 1.4 * ChanAnaStr->heartRate)) 
			|| (ChanAnaStr->heartRate == 0) || (HRate <= 201 && HRate >= 40))
		{
			ChanAnaStr->lastHRate = HRate;
		}
		memmove(ChanAnaStr->HrArray, 
			&(ChanAnaStr->HrArray[1]), memmovelen);
		ChanAnaStr->HrArray[HrArLen - 1] = ChanAnaStr->lastHRate;
		zeroFlag = 0;//赋初始值
		for (flagCtr = 0; flagCtr < HrArLen; ++flagCtr)
		{
			if (ChanAnaStr->HrArray[flagCtr] == 0)
			{
				zeroFlag = zeroFlag + 1;
			}
		}
		if (zeroFlag >= 5)
		{
			ChanAnaStr->heartRate = 0;
		} 
		else
		{
			sumHRate = 0;//赋初始值
			plusTimes = 0;//赋初始值
			for (; zeroFlag < 5; ++zeroFlag)
			{
				++plusTimes;
				sumHRate = sumHRate + ChanAnaStr->HrArray[zeroFlag];
			}
			ChanAnaStr->heartRate = sumHRate / plusTimes;
		}
	}
	else
	{
		ChanAnaStr->heartRate = 0;
	}
	

	return 0;
}
/******************************************************************************

    函数名： pwavedet()
	语法： int pwavedet( int *pbuf, int *pBegin, int *pEnd, int *pLoc)
	描述： 此函数是用来寻找P波的起点，终点，主波点
	参考论文： 主要基于以下两篇论文
		   1、一种基于最优区间函数拟合的P波识别算法
		   2、Q-onset and T-end delineation: assessment of the performance
		      of an automated method with the use of a reference database
	调用： 标准库中的abs（绝对值）函数
	被调用： 在主函数中调用
	输入参数： pbuffer，来自qrsandonoffset()函数
	输出参数： P波起点、终点及P波峰值点在pbuf中的坐标
	返回值： 目前未定义好返回值，后续版本定义好，如：返回0，表明找到的P波
	         有效，返回1，表明找的无效
	其他： 此版本简单完成P波的查找，默认认为每一QRS波必有一P波
	       先找到主波后再找起终点更合理
	 
******************************************************************************/
#include "qrsdet.h"
#include <math.h>

int pwavedet( int *pbuf, int *pBegin, int *pEnd, int *pLoc)
{
	int loopone = 0, looptwo = 0, loopthree = 0;//循环及坐标量
	int crit = 0;//标准（找起终点）
	int maxValue = 0, minValue = 0;//最大、最小值
	int maxLoc = 0, minLoc = 0;//最大、最小值坐标
	int maxLeft = 0, maxRight = 0;//最大坐标左右MS10点
	int minLeft = 0, minRight = 0;//最小坐标左右MS10点
	int maxLeftValue = 0, maxRightValue = 0;//最大坐标左右MS10点值
	int minLeftValue = 0, minRightValue = 0;//最小坐标左右MS10点值
	int maxValuePlus = 0, minValuePlus = 0;//斜率和

	int looptwoLeft = 0;//寻找begin时左边一点
	int conTimeTwo = 0;//连续时间（）目前认为超过MS20时认为找到起点
	int loopthreeRight = 0;//寻找end时右边一点
	int conTimeThree = 0;//连续时间（）目前认为超过MS20时认为找到终点

	//寻找最大最小值及坐标
	maxValue = minValue = pbuf[0];
	for (loopone = 0; loopone < MS220; ++loopone)
	{
		if (maxValue < pbuf[loopone])
		{
			maxValue = pbuf[loopone];
			maxLoc = loopone;
		}
		if (minValue < pbuf[loopone])
		{
			minValue = pbuf[loopone];
			minLoc = loopone;
		}
	}

	crit = 0.01 * (maxValue - minValue);//经验值
	maxLeft = maxLoc - MS10;
	maxRight = maxLoc + MS10;
	minLeft = minLoc - MS10;
	minRight = minLoc + MS10;
	if (maxLeft < 0)
	{
		maxLeft = 0;
	}
	if (maxRight >= MS220)
	{
		maxRight = MS220 - 1;
	}
	if (minLeft < 0)
	{
		minLeft = 0;
	}
	if (minRight >= MS220)
	{
		minRight = MS220 - 1;
	}

	maxLeftValue = abs(pbuf[maxLeft] - pbuf[maxLoc]);
	maxRightValue = abs(pbuf[maxRight] - pbuf[maxLoc]);
	minLeftValue = abs(pbuf[minLeft] - pbuf[minLoc]);
	minRightValue = abs(pbuf[minRight] - pbuf[minLoc]);
	maxValuePlus = maxLeftValue + maxRightValue;
	minValuePlus = minLeftValue + minRightValue;
	if (maxValuePlus > 0.8 * minValuePlus)
	{
		*pLoc = maxLoc;
	}
	else
	{
		*pLoc = minLoc;
	}

	//寻找pbegin
	for (looptwo = *pLoc; looptwo >= 0; --looptwo)
	{
		looptwoLeft = looptwo - 1;
		if (looptwoLeft < 0)
		{
			*pBegin = 0;
			break;
		}
		if (abs(pbuf[looptwo] - pbuf[looptwoLeft]) <= crit)
		{
			conTimeTwo = conTimeTwo + 1;
		}
		else
		{
			conTimeTwo = 0;
		}

		//连续时间满足一定条件则记下左查找区间点
		if (conTimeTwo >= MS20)
		{
			*pBegin = looptwo + MS15;//是否减，以后要看效果
			break;
		}
	}

	//寻找pend
	for (loopthree = *pLoc; loopthree < MS220; ++loopthree)
	{
		loopthreeRight = loopthree + 1;
		if (loopthreeRight >= MS220)
		{
			*pEnd = MS220 - 1;
			break;
		}
		if (abs(pbuf[loopthree] - pbuf[loopthreeRight]) <= crit)
		{
			conTimeThree = conTimeThree + 1;
		}
		else
		{
			conTimeThree = 0;
		}

		//连续时间满足一定条件则记下左查找区间点
		if (conTimeThree >= MS20)
		{
			*pEnd = loopthree - MS15;//是否减，以后要看效果
			break;
		}
	}
return (1);
}

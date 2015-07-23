/******************************************************************************

    函数名： subpowerline()
	语法： int subpowerline(FilterDataStruct *FilterDate, int datum)
	描述： 去工频噪声函数
	参考论文： 主要基于以下两篇论文，论文机构一致
	           1、Subtraction of 50 Hz interference from the
	              electrocardiogram
	           2、New approach to the digital elimination of
	              50 Hz interference from the electrocardiogram
	调用： 标准库中的abs（绝对值）函数
	被调用：主函数或上层函数
	输入参数：滤波器结构体变量和待滤波数据值
	输出参数： 无
	返回值： 被滤波信号
	其他： 这是一种非线性滤波器，与另外几种不同的地方只会有subpowerlineOrder.h
	       头文件不会有系数头文件，所以在.c文件中会引用阶数头文件，
		   在阶数头文件中定义的宏比另几种滤波器多，这些
		   宏主要针对的是50Hz工频干扰，目前不能去除60Hz工频干扰
    注意：

******************************************************************************/
#include <math.h>
#include <string.h>
#include "qrsdet.h"
#include "filter.h"


#include "subpowerlineorder_250.h"


int subpowerline(FilterDataStruct *FilterDate, int datum)
{
	int intfdatum = 0;//输出值
	real fdatum = 0;//输出值
	real subPrecision = 0;//为提高精度的中间值
	int realPtr = 0;
	real sumOfdata = 0;
	int sumLoop = 0, sumPtr = 0;
	real alterError = 0;
	int	alterPtr = 0;
	int crit = 0;
	int diffOne = 0, diffTwo = 0, criterion = 0;
	int diffOnePtr = 0, diffTwoPtr = 0, ptrMinusOne = 0;

	crit = (int)(150 * FilterDate->subPL.crit);
	FilterDate->subPL.dataIn[FilterDate->subPL.ptr] = datum;

	if (FilterDate->subPL.jVar == numOfCircle)
	{
		FilterDate->subPL.jVar = 0;
	}

	diffOnePtr = FilterDate->subPL.ptr - numOfCircle;
	diffTwoPtr = FilterDate->subPL.ptr - numOfCircle - 1;
	ptrMinusOne = FilterDate->subPL.ptr - 1;
	if (diffOnePtr < 0)
	{
		diffOnePtr = diffOnePtr + datainlen;
	}
	if (diffTwoPtr < 0)
	{
		diffTwoPtr = diffTwoPtr + datainlen;
	}
	if (ptrMinusOne < 0)
	{
		ptrMinusOne = ptrMinusOne + datainlen;
	}

	diffOne = FilterDate->subPL.dataIn[diffOnePtr] -
		      FilterDate->subPL.dataIn[FilterDate->subPL.ptr];
	diffTwo = FilterDate->subPL.dataIn[diffTwoPtr] -
		      FilterDate->subPL.dataIn[ptrMinusOne];
	criterion = abs(diffOne - diffTwo);

	if (criterion < crit)
	{
		FilterDate->subPL.ctrVar = FilterDate->subPL.ctrVar - 1;
		if (FilterDate->subPL.ctrVar == 0)
		{
			FilterDate->subPL.ctrVar = 1;
			for (sumLoop = 1; sumLoop <= numOfCircle; ++sumLoop)
			{
				sumPtr = FilterDate->subPL.ptr - sumLoop;
				if (sumPtr < 0)
				{
					sumPtr = sumPtr + datainlen;
				}
				sumOfdata = sumOfdata + FilterDate->subPL.dataIn[sumPtr];
			}

			alterPtr = FilterDate->subPL.ptr - numOfCircle;
			if (alterPtr < 0)
			{
				alterPtr = alterPtr + datainlen;
			}
			alterError = (FilterDate->subPL.dataIn[FilterDate->subPL.ptr] -
				          FilterDate->subPL.dataIn[alterPtr]) / 2;
			sumOfdata = sumOfdata - alterError;
			fdatum = sumOfdata / numOfCircle;

			realPtr = FilterDate->subPL.ptr - realptrShift;
			if (realPtr < 0)
			{
				realPtr = realPtr + datainlen;
			}
			//是否应该这样？这个考虑改一下
			FilterDate->subPL.interference[FilterDate->subPL.jVar] =
				        FilterDate->subPL.dataIn[realPtr] - fdatum;
		}
		else
		{
			realPtr = FilterDate->subPL.ptr - realptrShift;
			if (realPtr < 0)
			{
				realPtr = realPtr + datainlen;
			}
			fdatum = FilterDate->subPL.dataIn[realPtr] -
				     FilterDate->subPL.interference[FilterDate->subPL.jVar];
		}
	}
	else
	{
		FilterDate->subPL.ctrVar = numOfCircle;
		realPtr = FilterDate->subPL.ptr - realptrShift;
		if (realPtr < 0)
		{
			realPtr = realPtr + datainlen;
		}
		fdatum = FilterDate->subPL.dataIn[realPtr] -
			     FilterDate->subPL.interference[FilterDate->subPL.jVar];
	}

	FilterDate->subPL.jVar = FilterDate->subPL.jVar + 1;
	++FilterDate->subPL.ptr;
	if (FilterDate->subPL.ptr == datainlen)
	{
		FilterDate->subPL.ptr = 0;
	}

	intfdatum = (int)fdatum;
	subPrecision = fdatum - intfdatum;

	if (subPrecision >= 0.5000)
	{
		intfdatum = intfdatum + 1;
	}
	if (subPrecision < -0.5000)
	{
		intfdatum = intfdatum - 1;
	}
	return(intfdatum);
}

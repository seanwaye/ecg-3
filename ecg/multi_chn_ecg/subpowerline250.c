/******************************************************************************

    函数名： subpowerline250()
	语法： int subpowerline250(int datum, int init)
	描述： 去工频噪声函数
	参考论文： 主要基于以下两篇论文，论文机构一致
	           1、Subtraction of 50 Hz interference from the
	              electrocardiogram
	           2、New approach to the digital elimination of
	              50 Hz interference from the electrocardiogram
	调用： 标准库中的abs（绝对值）函数
	被调用：主函数或上层函数
	输入参数：待滤波信号及初始化标志量
	输出参数： 无
	返回值： 被滤波信号
	其他： 找到了一直与matlab版本不一致的原因：matlab输出与中间量是浮点型，
	       而在次均为int型，至于选何种类型还要考虑，看后续运用
		   实验证可对受任意幅度的工频干扰（基本如此）的信号进行滤波处理；
		   且效果不错
		   延迟5点

		   上面的函数输出为整型的函数，下面的函数输出为double型的函数

    注意： 两个注意问题
	       1、此函数不像matlab版本一样，可以处理任意采样率和两种工频干扰
		   （50、60）的信号，这里处理采样率为500Hz，工频干扰为50Hz的信号
		   为ADS1298-FE采集的信号
		   2、未处理前面的突变
	 
******************************************************************************/
#include <math.h>
/*#define ABS(x) (x>=0?x:-x)*/

int subpowerline250(int datum, int init)
{
	int intfdatum = 0;//输出值
	double fdatum = 0;//输出值
	double subPrecision = 0;//为提高精度的中间值
	static int data[7], ptr;
	static double interference[5];//交流误差项
	static int jVar, ctrVar;//两控制量，具体意义见论文
	int realPtr = 0;
	int numOfCircle = 5;//一周期内采样六点
	int halfNumOfCir = 2;
	int crit = 3000;//经验值，未具体定下
	double sumOfdata = 0;
	int sumLoop = 0, sumPtr = 0;
	double alterError = 0;
	int	alterPtr = 0;
	int diffOne = 0, diffTwo = 0, criterion = 0;
	int diffOnePtr = 0, diffTwoPtr = 0, ptrMinusOne = 0;

	if (init)
	{
		for (ptr = 0; ptr < 7; ++ptr)
		{
			data[ptr] = 0;
		}
		for (ptr = 0; ptr < 5; ++ptr)
		{
			interference[ptr] = 0;
		}
		ptr = 0;
		jVar = 0;
		ctrVar = 1;
		return 0;
	}

	data[ptr] = datum;
	if (jVar == numOfCircle)
	{
		jVar = 0;
	}

	diffOnePtr = ptr - numOfCircle;
	diffTwoPtr = ptr - numOfCircle - 1;
	ptrMinusOne = ptr - 1;
	if (diffOnePtr < 0)
	{
		diffOnePtr = diffOnePtr + 7;
	}
	if (diffTwoPtr < 0)
	{
		diffTwoPtr = diffTwoPtr + 7;
	}
	if (ptrMinusOne < 0)
	{
		ptrMinusOne = ptrMinusOne + 7;
	}

	diffOne = data[diffOnePtr] - data[ptr];
	diffTwo = data[diffTwoPtr] - data[ptrMinusOne];

	criterion = abs(diffOne - diffTwo);
	if (criterion < crit)
	{
		ctrVar = ctrVar - 1;
		if (ctrVar == 0)
		{
			ctrVar = 1;
			for (sumLoop = 1; sumLoop <= numOfCircle; ++sumLoop)
			{
				sumPtr = ptr - sumLoop;
				if (sumPtr < 0)
				{
					sumPtr = sumPtr + 7;
				}
				sumOfdata = sumOfdata + data[sumPtr];
			}

			alterPtr = ptr - numOfCircle;
			if (alterPtr < 0)
			{
				alterPtr = alterPtr + 7;
			}
			alterError = (data[ptr] - data[alterPtr]) / 2;
			sumOfdata = sumOfdata - alterError;
			fdatum = sumOfdata / numOfCircle;

			realPtr = ptr - 2;
			if (realPtr < 0)
			{
				realPtr = realPtr + 7;
			}
			//是否应该这样？这个考虑改一下
			interference[jVar] = data[realPtr] - fdatum;
		} 
		else
		{
			realPtr = ptr - 2;
			if (realPtr < 0)
			{
				realPtr = realPtr + 7;
			}
			fdatum = data[realPtr] - interference[jVar];
		}
	} 
	else
	{
		ctrVar = numOfCircle;
		realPtr = ptr - 2;
		if (realPtr < 0)
		{
			realPtr = realPtr + 7;
		}
		fdatum = data[realPtr] - interference[jVar];
	}
	jVar = jVar + 1;
	++ptr;
	if (ptr == 7)
	{
		ptr = 0;
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


//#include <math.h>
//
//double subpowerline250(int datum, int init)
//{
//	int intfdatum = 0;//输出值
//	double fdatum = 0;//输出值
//	double subPrecision = 0;//为提高精度的中间值
//	static int data[7], ptr;
//	static double interference[5];//交流误差项
//	static int jVar, ctrVar;//两控制量，具体意义见论文
//	int realPtr = 0;
//	int numOfCircle = 5;//一周期内采样六点
//	int halfNumOfCir = 2;
//	int crit = 3000;//经验值，未具体定下
//	double sumOfdata = 0;
//	int sumLoop = 0, sumPtr = 0;
//	double alterError = 0;
//	int	alterPtr = 0;
//	int diffOne = 0, diffTwo = 0, criterion = 0;
//	int diffOnePtr = 0, diffTwoPtr = 0, ptrMinusOne = 0;
//
//	if (init)
//	{
//		for (ptr = 0; ptr < 7; ++ptr)
//		{
//			data[ptr] = 0;
//		}
//		for (ptr = 0; ptr < 5; ++ptr)
//		{
//			interference[ptr] = 0;
//		}
//		ptr = 0;
//		jVar = 0;
//		ctrVar = 1;
//		return 0;
//	}
//
//	data[ptr] = datum;
//	if (jVar == numOfCircle)
//	{
//		jVar = 0;
//	}
//
//	diffOnePtr = ptr - numOfCircle;
//	diffTwoPtr = ptr - numOfCircle - 1;
//	ptrMinusOne = ptr - 1;
//	if (diffOnePtr < 0)
//	{
//		diffOnePtr = diffOnePtr + 7;
//	}
//	if (diffTwoPtr < 0)
//	{
//		diffTwoPtr = diffTwoPtr + 7;
//	}
//	if (ptrMinusOne < 0)
//	{
//		ptrMinusOne = ptrMinusOne + 7;
//	}
//
//	diffOne = data[diffOnePtr] - data[ptr];
//	diffTwo = data[diffTwoPtr] - data[ptrMinusOne];
//
//	criterion = abs(diffOne - diffTwo);
//	if (criterion < crit)
//	{
//		ctrVar = ctrVar - 1;
//		if (ctrVar == 0)
//		{
//			ctrVar = 1;
//			for (sumLoop = 1; sumLoop <= numOfCircle; ++sumLoop)
//			{
//				sumPtr = ptr - sumLoop;
//				if (sumPtr < 0)
//				{
//					sumPtr = sumPtr + 7;
//				}
//				sumOfdata = sumOfdata + data[sumPtr];
//			}
//
//			alterPtr = ptr - numOfCircle;
//			if (alterPtr < 0)
//			{
//				alterPtr = alterPtr + 7;
//			}
//			alterError = (data[ptr] - data[alterPtr]) / 2;
//			sumOfdata = sumOfdata - alterError;
//			fdatum = sumOfdata / numOfCircle;
//
//			realPtr = ptr - 2;
//			if (realPtr < 0)
//			{
//				realPtr = realPtr + 7;
//			}
//			//是否应该这样？这个考虑改一下
//			interference[jVar] = data[realPtr] - fdatum;
//		} 
//		else
//		{
//			realPtr = ptr - 2;
//			if (realPtr < 0)
//			{
//				realPtr = realPtr + 7;
//			}
//			fdatum = data[realPtr] - interference[jVar];
//		}
//	} 
//	else
//	{
//		ctrVar = numOfCircle;
//		realPtr = ptr - 2;
//		if (realPtr < 0)
//		{
//			realPtr = realPtr + 7;
//		}
//		fdatum = data[realPtr] - interference[jVar];
//	}
//	jVar = jVar + 1;
//	++ptr;
//	if (ptr == 7)
//	{
//		ptr = 0;
//	}
//	intfdatum = (int)fdatum;
//	subPrecision = fdatum - intfdatum;
//	if (subPrecision >= 0.5000)
//	{
//		intfdatum = intfdatum + 1;
//	}
//	if (subPrecision < -0.5000)
//	{
//		intfdatum = intfdatum - 1;
//	}
//	return(fdatum);
//}
/******************************************************************************
    文件名: qrsdet.c
	描述： 此文件中包含的是为检测QRS波的检测函数，以及检测函数所需的子函数
	qrsdet()函数会被其它函数调用，其他函数仅是内部函数,并调用两个外部函数
	函数列表：
	1、qrsdet()     本文件中主函数，调用其他函数，当检测到一个QRS波输出检测延迟
	2、peak()       检测峰值函数
	3、median()     中值函数
	4、thresh()     阈值函数
	5、blscheck()   基线漂移检测函数
	6、qrsfilter()  滤波函数，外部函数
	7、deriv1()     差分函数，外部函数
******************************************************************************/



#include <string.h>		// memmove函数所在库文件
#include <math.h>
#include "qrsdet.h"
#include <stdlib.h>

// 外部函数声明

int qrsfilter(int datum, int init);
int deriv1( int x0, int init );

// 内部函数声明

int peak( int datum, int init );
int median(int *array, int datnum);
int thresh(int qmedian, int nmedian);
int blscheck(int *dBuf,int dbPtr);    //baseline shift check基线偏移检测


double TH = 0.4; //阈值系数

int ddBuffer[DER_DELAY], ddPtr;	//存放原始信号差值的缓存
int Dly = 0;

const int memmovelen = 7 * sizeof(int);


/******************************************************************************

    函数名： qrsdet()
	语法： int qrsdet( int datum, int init )
	描述： 滤波总函数，调用各滤波器，得到经过滤波处理的信号值
	调用：
	       1、qrsfilter()   滤波函数
	       2、deriv1()      差值函数
	       3、peak()        峰值检测函数
	       4、median()      中值函数
	       5、thresh()      阈值函数
	       6、blscheck()    基线漂移检测函数
	被调用：上层函数（主函数）
	输入参数： 原始信号量及是否初始化变量
	输出参数： 若检测到一个QRS波，则输出检测延时，未检测到，则输出0值
	返回值： 若检测到一个QRS波，则输出检测延时，未检测到，则输出0值
	其他： 若init非0时，一些量赋初始值，qrsfilter()和peak()函数初始化

******************************************************************************/

int qrsdet( int datum, int init )
{
	static int det_thresh, qpkcnt = 0;
	static int qrsbuf[8], noise[8], rrbuf[8];
	static int rsetBuff[8], rsetCount = 0;
	static int nmedian, qmedian, rrmedian;

	// sbpeak: search back peak
	// sbloc: search back location
	// sbcount: search back count
	static int count, sbpeak = 0, sbloc, sbcount = MS1500;
	static int initBlank, initMax;
	static int preBlankCnt, tempPeak;

	int fdatum, qrsDelay = 0;
	int i, newPeak, aPeak;

	//初始化
	if ( init )
	{
		for (i = 0; i < 8; ++i)
		{
			noise[i] = 0;    	// 初始化噪声缓存
			rrbuf[i] = MS1000;  // 初始化RR间期缓存
		}

		qpkcnt = count = sbpeak = 0;
		initBlank = initMax = preBlankCnt = ddPtr = 0;
		sbcount = MS1500;
		qrsfilter(0, 1);	// 初始化滤波函数
		peak(0, 1);         // 初始化峰值检测函数
	}

	fdatum = qrsfilter(datum, 0);	//将原始数据滤波
	aPeak = peak(fdatum, 0);        //峰值检测

	// 这段函数的作用是体现心跳的不应期，以MS200为不应期
	newPeak = 0;
	if (aPeak && !preBlankCnt)			// 若200ms不应期已到且产生一个peak值
	{								    // 则存储此peak值且重置不应期计数变量
		tempPeak = aPeak;
		preBlankCnt = PRE_BLANK;
	}
	else if(!aPeak && preBlankCnt)	   // 当不应期已过且无新peak产生时，才赋值
	{								   // newpeak，认为有可能产生QRS波
		if(--preBlankCnt == 0)         // 这也是为什么FILTER_DELAY中加上了
		{                              // PRE_BLANK的缘故
			newPeak = tempPeak;
		}
	}
	else if (aPeak)					   // 如果在不应期内有重新产生了新的peak
	{
		if (aPeak > tempPeak)		   // 若此peak值大于前面的peak值
		{                              // 则重复一次上面的的操作
			tempPeak = aPeak;
			preBlankCnt = PRE_BLANK;
		}
		else if (--preBlankCnt == 0)
		{
			newPeak = tempPeak;
		}
	}

	//存储原始信号的斜率，存放的数组即基线检测所需数组
	ddBuffer[ddPtr] = deriv1( datum, 0 );
	if (++ddPtr == DER_DELAY)
	{
		ddPtr = 0;
	}

	//前8个（实际为8s，因为满足MS1000时qpkcnt加1）
	if ( qpkcnt < 8 )
	{
		++count;

		if(newPeak > 0)
		{
			count = WINDOW_WIDTH;
		}
		++initBlank;
		if(initBlank == MS1000)
		{
			initBlank = 0;
			qrsbuf[qpkcnt] = initMax;
			initMax = 0;
			++qpkcnt;

			if(qpkcnt == 8)
			{
				qmedian = median(qrsbuf, 8);
				nmedian = 0;
				rrmedian = MS1000;
				sbcount = MS1500 + MS150;
				det_thresh = thresh(qmedian, nmedian);
			}
		}

		if ( newPeak > initMax )
		{
			initMax = newPeak;
		}
	}

	else	// 此时才开始检测QRS波
	{
		++count;
		if (newPeak > 0)
		{
			// 检测peak是否由于极限偏移引起，不是则认为有可能产生QRS波
			if (!blscheck(ddBuffer, ddPtr))
			{

				// 若peak值大于探测阈值，则认为有可能产生QRS波
				if (newPeak > det_thresh)
				{
					memmove(&qrsbuf[1], qrsbuf, memmovelen);
					qrsbuf[0] = newPeak;
					qmedian = median(qrsbuf, 8);
					det_thresh = thresh(qmedian, nmedian);
					memmove(&rrbuf[1], rrbuf, memmovelen);
					rrbuf[0] = count - WINDOW_WIDTH;
					rrmedian = median(rrbuf, 8);
					sbcount = rrmedian + (rrmedian >> 1) + WINDOW_WIDTH;
					count = WINDOW_WIDTH;
					sbpeak = 0;
					qrsDelay = WINDOW_WIDTH + FILTER_DELAY;
					initBlank = initMax = rsetCount = 0;
				}

				// 认为是噪声引起的peak，或有可能是与之前
				// 不匹配QRS（能量小）引起
				else
				{
					memmove(&noise[1], noise, memmovelen);
					noise[0] = newPeak;
					nmedian = median(noise, 8);
					det_thresh = thresh(qmedian, nmedian);

					// Don't include early peaks (which might be T-waves)
					// in the search back process.  A T-wave can mask
					// a small following QRS.

					if ((newPeak > sbpeak) &&
						             ((count - WINDOW_WIDTH) >= MS360))
					{
						sbpeak = newPeak;
						sbloc = count - WINDOW_WIDTH;
					}
				}

				//不行就去掉
				//if ((count > sbcount) && (sbpeak > (det_thresh >> 1)))
				//{
				//	memmove(&qrsbuf[1], qrsbuf, memmovelen);
				//	qrsbuf[0] = sbpeak;
				//	qmedian = median(qrsbuf, 8);
				//	det_thresh = thresh(qmedian, nmedian);
				//	memmove(&rrbuf[1], rrbuf, memmovelen);
				//	rrbuf[0] = sbloc;
				//	rrmedian = median(rrbuf, 8);
				//	sbcount = rrmedian + (rrmedian >> 1) + WINDOW_WIDTH;
				//	qrsDelay = count = count - sbloc;
				//	qrsDelay += FILTER_DELAY;
				//	sbpeak = 0;
				//	initBlank = initMax = rsetCount = 0;
				//}
			}
		}

		//回溯机制
		if ((count > sbcount) && (sbpeak > (det_thresh >> 1)))
		{
			memmove(&qrsbuf[1], qrsbuf, memmovelen);
			qrsbuf[0] = sbpeak;
			qmedian = median(qrsbuf, 8);
			det_thresh = thresh(qmedian, nmedian);
			memmove(&rrbuf[1], rrbuf, memmovelen);
			rrbuf[0] = sbloc;
			rrmedian = median(rrbuf, 8);

			// sbcount = 1.5 * RR间期
			sbcount = rrmedian + (rrmedian >> 1) + WINDOW_WIDTH;
			qrsDelay = count = count - sbloc;
			qrsDelay += FILTER_DELAY;
			sbpeak = 0;
			initBlank = initMax = rsetCount = 0;
		}
	}

	//若在8s内未检测到QRS波则重新初始化一些值而qrsbuf赋值为每1s的最大值
/*
	if ( qpkcnt == 8 )
	{
		if (++initBlank == MS1000)
		{
			initBlank = 0;
			rsetBuff[rsetCount] = initMax;
			initMax = 0;
			++rsetCount;

			//满足8s条件
			if (rsetCount == 8)
			{
				for (i = 0; i < 8; ++i)
				{
					qrsbuf[i] = rsetBuff[i];
					noise[i] = 0;
				}
				qmedian = median( rsetBuff, 8 );
				nmedian = 0;
				rrmedian = MS1000;
				sbcount = MS1500 + MS150;
				det_thresh = thresh(qmedian, nmedian);
				initBlank = initMax = rsetCount = 0;
                sbpeak = 0;
			}
		}

		if ( newPeak > initMax )
		{
			initMax = newPeak;
		}
	}*/

	return(qrsDelay);
}


/******************************************************************************

    函数名： peak()
	语法: int peak( int datum, int init )
	描述: 峰值检测函数，输入预处理后的信号，该信号与上个信号，
	      以及最大值比较，若超过，更新最大值，保存当前信号值，
	      返回0，否则，若低于最大值的一半，返回最大值，Dly 为0，
	      否则，判定当前信号位置到最大值距离，超过95ms，
	      返回最大值，Dly = 3，除此之外，返回 0。
	调用： 无
	被调用：  qrsdet()
	输入参数： 预处理后的信号及是否初始化变量
	输出参数： 峰值
	返回值： 峰值及Dly
	其他： Dly未具体利用

******************************************************************************/

int peak( int datum, int init )
{
	static int max = 0, timeSinceMax = 0, lastDatum;
	int pk = 0;

	if (init)
	{
		max = timeSinceMax = 0;
	}

	if (timeSinceMax > 0)
	{
		++timeSinceMax;
	}

	if ((datum > lastDatum) && (datum > max))
	{
		max = datum;

		//满足此条件和上面的大条件时才赋值timeSinceMax
		if (max > 2)
		{
			timeSinceMax = 1;
		}
	}
	else if (datum < (max >> 1))
	{
		pk = max;
		max = 0;
		timeSinceMax = 0;
		Dly = 0;
	}
	else if (timeSinceMax > MS95)
	{
		pk = max;
		max = 0;
		timeSinceMax = 0;
		Dly = 3;
	}

	lastDatum = datum;
	return(pk);
}


/******************************************************************************

    函数名： median()
	语法: int median(int *array, int datnum)
	描述: 中值函数
	调用： 无
	被调用：  qrsdet()
	输入参数： 一数组及数组长度
	输出参数： 此数组中值
	返回值： 此数组中值
	其他： 无

******************************************************************************/

int median(int *array, int datnum)
{
	int i, j, k, temp, sort[20];

	for (i = 0; i < datnum; ++i)
	{
		sort[i] = array[i];
	}

	for (i = 0; i < datnum; ++i)
	{
		temp = sort[i];

		for (j = 0; (temp < sort[j]) && (j < i) ; ++j);

		for (k = i - 1 ; k >= j ; --k)
		{
			sort[k + 1] = sort[k];
		}

		sort[j] = temp;
	}
	return(sort[datnum >> 1]);
}


/******************************************************************************

    函数名： thresh()
	语法: int thresh(int qmedian, int nmedian)
	描述: Detection_Threshold = Median_Noise_Peak +
	                               TH*(Median_QRS_Peak  - Median_Noise_Peak)
	调用： 无
	被调用：  qrsdet()
	输入参数： 处理后的QRS波峰值及处理后的噪声峰值
	输出参数： 阈值
	返回值： 阈值
	其他： TH为阈值系数，在文件顶部有定义

******************************************************************************/

int thresh(int qmedian, int nmedian)
{
	int thrsh, dmed;
	double temp;
	dmed = qmedian - nmedian;
	temp = dmed;
	temp *= TH;
	dmed = temp ;
	thrsh = nmedian + dmed; //公式体现
	return(thrsh);
}


/******************************************************************************

    函数名： blscheck()
	语法: int blscheck(int *dBuf,int dbPtr)
	描述: 在220ms的窗内查找最大值和最小值，这里的最大值和最小值是指原信号的
	      差分值，即斜率，若最大值和最小值反向（并满足一定条件），且在150ms内
		  发生则认为有可能有QRS波产生
	调用： 无
	被调用：  qrsdet()
	输入参数： 原始信号斜率缓存数组，当前斜率点坐标
	输出参数： 满足条件时返回0，不满足条件时为1
	返回值： 满足条件时返回0，不满足条件时为1
	其他： 无

******************************************************************************/

int blscheck(int *dBuf, int dbPtr)
{
	int max, min, maxt, mint, t, x;
	max = min = 0;

	return 0; //在此直接返回0值，下面均不计算了，此时se会明显提高，pe相对下降
	          //一些，总体会好一些，至于bls函数的优化后续进行

	for (t = 0; t < MS220; ++t)
	{
		x = dBuf[dbPtr];

		if (x > max)
		{
			maxt = t ;
			max = x ;
		}
		else if (x < min)
		{
			mint = t ;
			min = x;
		}

		if (--dbPtr <= 0)
		{
			dbPtr = DER_DELAY - 1;
		}
	}

	min = -min;


    //若在150ms内出现正负斜率则认为有可能有QRS波产生
	if ((max > (min>>3)) && (min > (max>>3)) &&
		(abs(maxt - mint) < MS150))
	{
		return(0);
	}
	else
	{
		return(1);
	}

}

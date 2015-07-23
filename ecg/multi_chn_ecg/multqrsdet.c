/******************************************************************************
    文件名: qrsdet.c
	描述： 此文件中包含的是为检测QRS波的检测函数，以及检测函数所需的子函数
	qrsdet()函数会被其它函数调用，其他函数仅是内部函数,并调用两个外部函数
	函数列表： 
	    1、qrsdet()     本文件中主函数，调用其他函数，当检测到一个QRS复合波
		                输出检测延迟
	    2、multpeak()       检测峰值函数
	    3、multmedian()     中值函数
	    4、multthresh()     阈值函数
	    5、multblscheck()   基线漂移检测函数
	    6、qrsfilter()  滤波函数，外部函数
	    7、deriv1()     差分函数，外部函数
	被调用： 
	输入参数： 
	输出参数： 
	返回值： 
	其他： multmedian函数一参数为所要寻找的中值数组长度，这为后面更改查找算法
	       留了一点空间
	注意：2013_1_29日发现，函数无拒绝高尖T波的能力
******************************************************************************/
#include <string.h>		// memmove函数所在库文件
#include <math.h>
#include "qrsdet.h"
#include "fundecla.h"


// 内部函数声明
int multpeak( qrsDetStruct* qrsDetStr ,int datum );
int multmedian(int *array, int datnum);
int multthresh(int qmedian, int nmedian);
int multblscheck(int *dBuf,int dbPtr);    //baseline shift check基线偏移检测

real multTH = 0.4f; //阈值系数

int multddBuffer[DER_DELAY] = {0}, multddPtr = 0;	//存放原始信号差值的缓存
int multDly = 0;

const int multmemmovelen = (arrayLen - 1) * sizeof(int);

/******************************************************************************

    函数名： qrsdet()
	语法： int qrsdet( qrsDetStruct* qrsDetStr, int datum )
	描述： 滤波总函数，调用各滤波器，得到经过滤波处理的信号值
	调用： 
	       1、qrsfilter()   滤波函数
	       2、deriv1()      差值函数
	       3、multpeak()        峰值检测函数
	       4、multmedian()      中值函数
	       5、multthresh()      阈值函数
	       6、multblscheck()    基线漂移检测函数
	被调用：上层函数（主函数）
	输入参数： 原始信号量及是否初始化变量
	输出参数： 若检测到一个QRS波，则输出检测延时，未检测到，则输出0值
	返回值： 若检测到一个QRS波，则输出检测延时，未检测到，则输出0值
	其他： 
	 
******************************************************************************/
int multqrsdet( qrsDetStruct* qrsDetStr, int datum )
{
	int fdatum, qrsDelay = 0;
	int i, newPeak, aPeak;

	fdatum = multqrsfilter(&qrsDetStr->qrsFilterStr, datum);	//将原始数据滤波
	aPeak = multpeak(qrsDetStr, fdatum);        //峰值检测

	// 这段函数的作用是体现心跳的不应期，以MS200为不应期
	newPeak = 0;
	if (aPeak && !qrsDetStr->preBlankCnt)// 若200ms不应期已到且产生一个multpeak值
	{								     // 则存储此multpeak值且重置不应期计数变量
		qrsDetStr->tempPeak = aPeak;
		qrsDetStr->preBlankCnt = PRE_BLANK;			
	}
	else if(!aPeak && qrsDetStr->preBlankCnt)//当不应期已过且无新multpeak产生时
	{								         //，才赋值newpeak，认为有可能产生
		if(--qrsDetStr->preBlankCnt == 0)    //QRS波这也是为什么FILTER_DELAY中
		{                                    //加上了PRE_BLANK的缘故
			newPeak = qrsDetStr->tempPeak;
		}
	}
	else if (aPeak)					   // 如果在不应期内有重新产生了新的multpeak
	{								   
		if (aPeak > qrsDetStr->tempPeak)    // 若此multpeak值大于前面的multpeak值
		{                                   // 则重复一次上面的的操作
			qrsDetStr->tempPeak = aPeak;
			qrsDetStr->preBlankCnt = PRE_BLANK;        
		}
		else if (--qrsDetStr->preBlankCnt == 0)
		{
			newPeak = qrsDetStr->tempPeak;
		}
	}

	//存储原始信号的斜率，存放的数组即基线检测所需数组
	multddBuffer[multddPtr] = multderiv1( &qrsDetStr->qrsFilterStr, datum );
	if (++multddPtr == DER_DELAY)
	{
		multddPtr = 0;
	}

	//前8个（实际为8s，因为满足MS1000时qpkcnt加1）
	if ( qrsDetStr->qpkcnt < 8 )
	{
		++qrsDetStr->countnum;

		if(newPeak > 0) 
		{
			qrsDetStr->countnum = WINDOW_WIDTH;
		}

		if(++qrsDetStr->initBlank == MS1000)
		{
			qrsDetStr->initBlank = 0;
			qrsDetStr->qrsbuf[qrsDetStr->qpkcnt] = qrsDetStr->initMax;
			qrsDetStr->initMax = 0;
			++qrsDetStr->qpkcnt;

			if(qrsDetStr->qpkcnt == 8)
			{
				qrsDetStr->qmedian = multmedian(qrsDetStr->qrsbuf, 8);
				qrsDetStr->nmedian = 0;
				qrsDetStr->rrmedian = MS1000;
				qrsDetStr->sbcount = MS1500 + MS150;
				qrsDetStr->det_thresh = 
					       multthresh(qrsDetStr->qmedian, qrsDetStr->nmedian);
			}
		}

		if ( newPeak > qrsDetStr->initMax )
		{
			qrsDetStr->initMax = newPeak;
		}
	}

	else	// 此时才开始检测QRS波
	{
		++qrsDetStr->countnum;
		if (newPeak > 0)
		{
			// 检测multpeak是否由于极限偏移引起，不是则认为有可能产生QRS波
			if (!multblscheck(multddBuffer, multddPtr))
			{

				// 若multpeak值大于探测阈值，则认为有可能产生QRS波
				if (newPeak > qrsDetStr->det_thresh)
				{
					memmove(&qrsDetStr->qrsbuf[1], 
						                qrsDetStr->qrsbuf, multmemmovelen);
					qrsDetStr->qrsbuf[0] = newPeak;
					qrsDetStr->qmedian = multmedian(qrsDetStr->qrsbuf, 8);
					qrsDetStr->det_thresh = 
						       multthresh(qrsDetStr->qmedian, qrsDetStr->nmedian);
					memmove(&qrsDetStr->rrbuf[1], 
						                qrsDetStr->rrbuf, multmemmovelen);
					qrsDetStr->rrbuf[0] = qrsDetStr->countnum - WINDOW_WIDTH;
					qrsDetStr->rrmedian = multmedian(qrsDetStr->rrbuf, 8);
					qrsDetStr->sbcount = qrsDetStr->rrmedian + 
						             (qrsDetStr->rrmedian >> 1) + WINDOW_WIDTH;
					qrsDetStr->countnum = WINDOW_WIDTH;
					qrsDetStr->sbpeak = 0;
					qrsDelay = WINDOW_WIDTH + FILTER_DELAY;
					qrsDetStr->initBlank = 
						       qrsDetStr->initMax = qrsDetStr->rsetCount = 0;
				}

				// 认为是噪声引起的multpeak，或有可能是与之前
				// 不匹配QRS（能量小）引起
				else
				{
					memmove(&qrsDetStr->noise[1], 
						     qrsDetStr->noise, multmemmovelen);
					qrsDetStr->noise[0] = newPeak;
					qrsDetStr->nmedian = multmedian(qrsDetStr->noise, 8);
					qrsDetStr->det_thresh = 
						       multthresh(qrsDetStr->qmedian, qrsDetStr->nmedian);

					// Don't include early peaks (which might be T-waves)
					// in the search back process.  A T-wave can mask
					// a small following QRS.

					if ((newPeak > qrsDetStr->sbpeak) && 
						((qrsDetStr->countnum - WINDOW_WIDTH) >= MS360))
					{
						qrsDetStr->sbpeak = newPeak;
						qrsDetStr->sbloc = qrsDetStr->countnum - WINDOW_WIDTH;
					}
				}

				//不行就去掉
				/*if ((qrsDetStr->countnum > qrsDetStr->sbcount) && 
					(qrsDetStr->sbpeak > (qrsDetStr->det_thresh >> 1)))
				{
				memmove(&qrsDetStr->qrsbuf[1], qrsDetStr->qrsbuf, multmemmovelen);
				qrsDetStr->qrsbuf[0] = qrsDetStr->sbpeak;
				qrsDetStr->qmedian = multmedian(qrsDetStr->qrsbuf, 8);
				qrsDetStr->det_thresh = 
					       multthresh(qrsDetStr->qmedian, qrsDetStr->nmedian);
				memmove(&qrsDetStr->rrbuf[1], qrsDetStr->rrbuf, multmemmovelen);
				qrsDetStr->rrbuf[0] = qrsDetStr->sbloc;
				qrsDetStr->rrmedian = multmedian(qrsDetStr->rrbuf, 8);
				qrsDetStr->sbcount = qrsDetStr->rrmedian + 
					                 (qrsDetStr->rrmedian >> 1) + WINDOW_WIDTH;
				qrsDelay = qrsDetStr->countnum = 
					       qrsDetStr->countnum - qrsDetStr->sbloc;
				qrsDelay += FILTER_DELAY;
				qrsDetStr->sbpeak = 0;
				qrsDetStr->initBlank = 
					       qrsDetStr->initMax = qrsDetStr->rsetCount = 0;
				}*/
			}
		}

		//回溯机制
		if ((qrsDetStr->countnum > qrsDetStr->sbcount) && 
			(qrsDetStr->sbpeak > (qrsDetStr->det_thresh >> 1)))
		{
			memmove(&qrsDetStr->qrsbuf[1], qrsDetStr->qrsbuf, multmemmovelen);
			qrsDetStr->qrsbuf[0] = qrsDetStr->sbpeak;
			qrsDetStr->qmedian = multmedian(qrsDetStr->qrsbuf, 8);
			qrsDetStr->det_thresh = 
				       multthresh(qrsDetStr->qmedian, qrsDetStr->nmedian);
			memmove(&qrsDetStr->rrbuf[1], qrsDetStr->rrbuf, multmemmovelen);
			qrsDetStr->rrbuf[0] = qrsDetStr->sbloc;
			qrsDetStr->rrmedian = multmedian(qrsDetStr->rrbuf, 8);

			// sbcount = 1.5 * RR间期
			qrsDetStr->sbcount = qrsDetStr->rrmedian + 
				                 (qrsDetStr->rrmedian >> 1) + WINDOW_WIDTH;
			qrsDelay = qrsDetStr->countnum = 
				       qrsDetStr->countnum - qrsDetStr->sbloc;
			qrsDelay += FILTER_DELAY;
			qrsDetStr->sbpeak = 0;
			qrsDetStr->initBlank = 
				       qrsDetStr->initMax = qrsDetStr->rsetCount = 0;
		}
	}

	//若在8s内未检测到QRS波则重新初始化一些值而qrsbuf赋值为每1s的最大值
/*
	if ( qrsDetStr->qpkcnt == 8 )
	{
		if (++qrsDetStr->initBlank == MS1000)
		{
			qrsDetStr->initBlank = 0;
			qrsDetStr->rsetBuff[qrsDetStr->rsetCount] = qrsDetStr->initMax;
			qrsDetStr->initMax = 0;
			++qrsDetStr->rsetCount;

			//满足8s条件
			if (qrsDetStr->rsetCount == 8)
			{
				for (i = 0; i < 8; ++i)
				{
					qrsDetStr->qrsbuf[i] = qrsDetStr->rsetBuff[i];
					qrsDetStr->noise[i] = 0;
				}
				qrsDetStr->qmedian = multmedian( qrsDetStr->rsetBuff, 8 );
				qrsDetStr->nmedian = 0;
				qrsDetStr->rrmedian = MS1000;
				qrsDetStr->sbcount = MS1500 + MS150;
				qrsDetStr->det_thresh = 
					       multthresh(qrsDetStr->qmedian, qrsDetStr->nmedian);
				qrsDetStr->initBlank = 
					       qrsDetStr->initMax = qrsDetStr->rsetCount = 0;
                qrsDetStr->sbpeak = 0;
			}
		}

		if ( newPeak > qrsDetStr->initMax )
		{
			qrsDetStr->initMax = newPeak;
		}
	}*/

	return(qrsDelay);
}


/******************************************************************************

    函数名： multpeak()
	语法: int multpeak( qrsDetStruct* qrsDetStr, int datum )
	描述: 峰值检测函数，输入预处理后的信号，该信号与上个信号，
	      以及最大值比较，若超过，更新最大值，保存当前信号值，
	      返回0，否则，若低于最大值的一半，返回最大值，multDly 为0，
	      否则，判定当前信号位置到最大值距离，超过95ms，
	      返回最大值，multDly = 3，除此之外，返回 0。
	调用： 无
	被调用：  qrsdet()
	输入参数： 预处理后的信号及是否初始化变量
	输出参数： 峰值
	返回值： 峰值及multDly
	其他： multDly未具体利用

******************************************************************************/
int multpeak( qrsDetStruct* qrsDetStr, int datum )
{
	int pk = 0;	
	if (qrsDetStr->qrsPeakStr.timeSinceMax > 0)
	{
		++qrsDetStr->qrsPeakStr.timeSinceMax;
	}
	if ((datum > qrsDetStr->qrsPeakStr.lastDatum) && 
		                              (datum > qrsDetStr->qrsPeakStr.maxvalue))
	{
		qrsDetStr->qrsPeakStr.maxvalue = datum;

		//满足此条件和上面的大条件时才赋值timeSinceMax
		if (qrsDetStr->qrsPeakStr.maxvalue > 2)  
		{
			qrsDetStr->qrsPeakStr.timeSinceMax = 1;
		}
	}
	else if (datum < (qrsDetStr->qrsPeakStr.maxvalue >> 1))
	{
		pk = qrsDetStr->qrsPeakStr.maxvalue;
		qrsDetStr->qrsPeakStr.maxvalue = 0;
		qrsDetStr->qrsPeakStr.timeSinceMax = 0;
		multDly = 0;
	}
	else if (qrsDetStr->qrsPeakStr.timeSinceMax > MS95)
	{
		pk = qrsDetStr->qrsPeakStr.maxvalue;
		qrsDetStr->qrsPeakStr.maxvalue = 0;
		qrsDetStr->qrsPeakStr.timeSinceMax = 0;
		multDly = 3;
	}

	qrsDetStr->qrsPeakStr.lastDatum = datum;
	return(pk);
}


/******************************************************************************

    函数名： multmedian()
	语法: int multmedian(int *array, int datnum)
	描述: 中值函数
	调用： 无
	被调用：  qrsdet()
	输入参数： 一数组及数组长度
	输出参数： 此数组中值
	返回值： 此数组中值
	其他： 无

******************************************************************************/
int multmedian(int *array, int datnum)
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

    函数名： multthresh()
	语法: int multthresh(int qmedian, int nmedian)
	描述: Detection_Threshold = Median_Noise_Peak + 
	                               multTH*(Median_QRS_Peak  - Median_Noise_Peak)  
	调用： 无
	被调用：  qrsdet()
	输入参数： 处理后的QRS波峰值及处理后的噪声峰值
	输出参数： 阈值
	返回值： 阈值
	其他： multTH为阈值系数，在文件顶部有定义

******************************************************************************/
int multthresh(int qmedian, int nmedian)
{
	int thrsh, dmed;
	real temp;
	dmed = qmedian - nmedian;
	temp = dmed;
	temp *= multTH;
	dmed = temp ;
	thrsh = nmedian + dmed; //公式体现
	return(thrsh);
}


/******************************************************************************

    函数名： multblscheck()
	语法: int multblscheck(int *dBuf,int dbPtr)
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
int multblscheck(int *dBuf, int dbPtr)
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
/******************************************************************************
    文件名: multqrsfilter.c
	语法： int multqrsfilter( qrsFilterStruct* qrsFilterStr, int datum )
	描述： 此文件中包含的是为检测QRS波所需的滤波函数，其中qrsfilt()和
	deriv1()函数会被其它函数调用，其他函数仅是内部函数
	函数列表： 
	    1、multqrsfilter() 本文件中主函数，调用其他函数，输出心电信号处理（能量）值
	    2、multlpfilt()    低通滤波函数
	    3、multhpfilt()    高通滤波函数
	    4、multderiv1()    差分函数
	    5、multderiv2()    差分函数
	    6、multmvwint()    积分窗求和函数
	    7、abs()       库中自带绝对值函数
	被调用： qrsdet.c
	输入参数： 
	输出参数： 
	返回值： 
	其他： 
	注意： 
******************************************************************************/
#include <math.h>
#include "qrsdet.h"
#include "qrsdetstruct.h"

/******************************************************************************

    函数名： qrsfilter()
	语法： int qrsfilter(qrsFilterStruct* qrsFilterStr, int datum)
	描述： 滤波总函数，调用各滤波器，得到经过滤波处理的信号值
	调用： 
	       1、lpfilt()    低通滤波函数
	       2、hpfilt()    高通滤波函数
	       3、deriv1()    差分函数
	       4、deriv2()    差分函数
	       5、mvwint()    积分窗求和函数
	       6、abs()       库中自带绝对值函数
	被调用： qrsdet()     QRS波检测函数
	输入参数： 原始信号量及是否初始化变量
	输出参数： 经过滤波后得到的信号值
	返回值： 经过滤波后得到的信号值
	其他： 若init非0时，滤波器缓存及静态变量被重置
	 
******************************************************************************/

int multqrsfilter( qrsFilterStruct* qrsFilterStr, int datum )
{
	int fdatum ;

	fdatum = multlpfilt( qrsFilterStr, datum );   //低通滤波
	fdatum = multhpfilt( qrsFilterStr, fdatum );  //高通滤波
	fdatum = multderiv2( qrsFilterStr, fdatum );  //差分滤波
	fdatum = abs( fdatum );    //取绝对值
	fdatum = multmvwint( qrsFilterStr, fdatum );  //积分窗求和
	
	return(fdatum);
}


/******************************************************************************

    函数名： lpfilt()
	语法: int lpfilt( qrsFilterStruct* qrsFilterStr, int datum)
	描述: 低通滤波函数，时域差分表达式为
	      y[n] = 2*y[n-1] - y[n-2] + x[n] - 2*x[t-24 ms] + x[t-48 ms]
		  y[n] = y[n] / ((LPBUFFER_LGTH * LPBUFFER_LGTH) / 4)
	调用： 无
	被调用：  qrsfilter()
	输入参数： 整形值及是否初始化变量
	输出参数： 滤波后信号量
	返回值： 滤波后信号量
	其他： 滤波延迟为（(LPBUFFER_LGTH / 2) - 1）个点

******************************************************************************/

int multlpfilt( qrsFilterStruct* qrsFilterStr, int datum )
{
	int y0 ;
	int output, halfPtr ;
	
	// Use halfPtr to index
	halfPtr = qrsFilterStr->qrsLpStr.lptr - (LPBUFFER_LGTH / 2) ;	
	
	if (halfPtr < 0)							// to x[n-6].
	{
		halfPtr += LPBUFFER_LGTH;
	}

	//y[n] = 2*y[n-1] - y[n-2] + x[n] - 2*x[t-24 ms] + x[t-48 ms]，即这句话
	//的翻译，这里的乘除都是通过以为实现的
	y0 = (qrsFilterStr->qrsLpStr.dataoutone << 1) - 
		  qrsFilterStr->qrsLpStr.dataouttwo + datum - 
		  (qrsFilterStr->qrsLpStr.datain[halfPtr] << 1) + 
		  qrsFilterStr->qrsLpStr.datain[qrsFilterStr->qrsLpStr.lptr] ; 

	qrsFilterStr->qrsLpStr.dataouttwo = qrsFilterStr->qrsLpStr.dataoutone;                                                         
	qrsFilterStr->qrsLpStr.dataoutone = y0;
	output = y0 / ((LPBUFFER_LGTH * LPBUFFER_LGTH) / 4);

	// Stick most recent sample into
	qrsFilterStr->qrsLpStr.datain[qrsFilterStr->qrsLpStr.lptr] = datum ;			
	// the circular buffer and update
	if (++qrsFilterStr->qrsLpStr.lptr == LPBUFFER_LGTH)	
	{
		qrsFilterStr->qrsLpStr.lptr = 0;             // the buffer pointer.
	}	
	return(output);
}

/******************************************************************************

    函数名： hpfilt()
	语法： int hpfilt( qrsFilterStruct* qrsFilterStr, int datum )
	描述： 高通滤波函数，时域差分表达式为
	       y[n] = y[n-1] + x[n] - x[n-128 ms] 
           z[n] = x[n-64 ms] - y[n]/HPBUFFER_LGTH
	调用： 无
	被调用： qrsfilter()
	输入参数： 整形值及是否初始化变量
	输出参数： 经过滤波后得到的信号值
	返回值： 经过滤波后得到的信号值
	其他： 滤波延迟为（(HPBUFFER_LGTH - 1) / 2）个点

******************************************************************************/

int multhpfilt( qrsFilterStruct* qrsFilterStr, int datum )
{
	int z, halfPtr ;

	qrsFilterStr->qrsHpStr.dataoutone += datum - 
		          qrsFilterStr->qrsHpStr.datain[qrsFilterStr->qrsHpStr.hptr];

	halfPtr = qrsFilterStr->qrsHpStr.hptr - (HPBUFFER_LGTH / 2) ;
	
	if (halfPtr < 0)
	{
		halfPtr += HPBUFFER_LGTH;
	}
	z = qrsFilterStr->qrsHpStr.datain[halfPtr] - 
		           (qrsFilterStr->qrsHpStr.dataoutone / HPBUFFER_LGTH);

	qrsFilterStr->qrsHpStr.datain[qrsFilterStr->qrsHpStr.hptr] = datum ;
	
	if (++qrsFilterStr->qrsHpStr.hptr == HPBUFFER_LGTH)
	{
		qrsFilterStr->qrsHpStr.hptr = 0;
	}

	return(z);
}


/******************************************************************************

    函数名： deriv1()和deriv2() 
	语法： int deriv1( qrsFilterStruct* qrsFilterStr, int datum ) or 
	       int deriv2( qrsFilterStruct* qrsFilterStr, int datum )
	描述： 差分函数，两函数实现功能一样，之所以写两个，是因为deriv1会被其他函数
	       调用，防止内部静态数组的值被其它函数更改
		   时域差分表达式为:y[n] = x[n] - x[n - 10ms]
	调用： 无
	被调用： deriv2()被qrsfilter()调用，deriv1()被qrsdet()调用
	输入参数： 整形值及是否初始化变量
	输出参数： 经过滤波后得到的信号值
	返回值： 经过滤波后得到的信号值
	其他： 滤波延迟为（DERIV_LENGTH / 2）个点

******************************************************************************/

int multderiv1( qrsFilterStruct* qrsFilterStr, int datum )
{
	int y;

	y = datum - 
		qrsFilterStr->qrsDerStr[0].datain[qrsFilterStr->qrsDerStr[0].derptr];

	qrsFilterStr->qrsDerStr[0].datain[qrsFilterStr->qrsDerStr[0].derptr] = 
		                                                         datum;
	
	if (++qrsFilterStr->qrsDerStr[0].derptr == DERIV_LENGTH)
	{
		qrsFilterStr->qrsDerStr[0].derptr = 0;
	}
	return(y) ;
}

int multderiv2( qrsFilterStruct* qrsFilterStr, int datum )
{
	int y;

	y = datum - 
		qrsFilterStr->qrsDerStr[1].datain[qrsFilterStr->qrsDerStr[1].derptr];

	qrsFilterStr->qrsDerStr[1].datain[qrsFilterStr->qrsDerStr[1].derptr] = 
		datum;

	if (++qrsFilterStr->qrsDerStr[1].derptr == DERIV_LENGTH)
	{
		qrsFilterStr->qrsDerStr[1].derptr = 0;
	}
	return(y) ;
}


/******************************************************************************

    函数名： mvwint()
	语法： int mvwint( qrsFilterStruct* qrsFilterStr, int datum )
	描述： 积分窗求和函数
	调用： 无
	被调用： qrsfilter()
	输入参数： 整形值及是否初始化变量
	输出参数： 经过滤波后得到的信号值
	返回值： 经过滤波后得到的信号值
	其他： 

******************************************************************************/

int multmvwint( qrsFilterStruct* qrsFilterStr, int datum )
{
	int output;

	qrsFilterStr->qrsMvStr.sum += datum;
	qrsFilterStr->qrsMvStr.sum -= 
		        qrsFilterStr->qrsMvStr.datain[qrsFilterStr->qrsMvStr.mvptr];
	//构成了一个循环
	qrsFilterStr->qrsMvStr.datain[qrsFilterStr->qrsMvStr.mvptr] = datum;                 

	if (++qrsFilterStr->qrsMvStr.mvptr == WINDOW_WIDTH)
	{
		qrsFilterStr->qrsMvStr.mvptr = 0;
	}

	//这里有一个防止溢出/防噪声的判断条件
	if ((qrsFilterStr->qrsMvStr.sum / WINDOW_WIDTH) > 32000)    
	{
			output = 32000;
	}
	else
	{
		output = qrsFilterStr->qrsMvStr.sum / WINDOW_WIDTH ;
	}

	return(output);
}
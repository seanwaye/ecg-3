/******************************************************************************
    文件名: qrsfilt.c
	描述： 此文件中包含的是为检测QRS波所需的滤波函数，其中qrsfilt()和
	deriv1()函数会被其它函数调用，其他函数仅是内部函数
	函数列表：
	1、qrsfilter() 本文件中主函数，调用其他函数，输出心电信号处理（能量）值
	2、lpfilt()    低通滤波函数
	3、hpfilt()    高通滤波函数
	4、deriv1()    差分函数
	5、deriv2()    差分函数
	6、mvwint()    积分窗求和函数
	7、abs()       库中自带绝对值函数
******************************************************************************/

#include <math.h>
#include "qrsdet.h"
#include <stdio.h>
#include <stdlib.h>
// 内部函数声明
int lpfilt( int datum ,int init );
int hpfilt( int datum, int init );
int deriv1( int x0, int init );
int deriv2( int x0, int init );
int mvwint( int datum, int init);

/******************************************************************************

    函数名： qrsfilter()
	语法： int qrsfilter(int datum, int init)
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

int qrsfilter( int datum, int init )
{
	int fdatum ;

	//初始化滤波器
	if(init)
	{
	    hpfilt( 0, 1 ) ;
		lpfilt( 0, 1 ) ;
		mvwint( 0, 1 ) ;
		deriv1( 0, 1 ) ;
		deriv2( 0, 1 ) ;
	}

	fdatum = lpfilt( datum, 0 );    //低通滤波
	fdatum = hpfilt( fdatum, 0 );    //高通滤波
	fdatum = deriv2( fdatum, 0 );    //差分滤波
	fdatum = abs( fdatum );    //取绝对值
	fdatum = mvwint( fdatum, 0 );    //积分窗求和

	return(fdatum);
}


//下面是为了看处理值而在一些处理步骤后生成了文件，保存这些处理结果
//int qrsfilter( int datum, int init )
//{
//	int fdatum ;
//	FILE *hp, *lp, *de, *ab, *mv;
//	int ihp, ilp, ide, iab, imv;
//	ihp = 0;
//	ilp = 0;
//	ide = 0;
//	iab = 0;
//	imv = 0;
//	//初始化滤波器
//	if(init)
//	{
//		ihp = hpfilt( 0, 1 ) ;
//		hp = fopen( "hp.txt", "a+" );
//		fprintf(hp, "%d\n", ihp);
//		fclose( hp );
//
//		ilp = lpfilt( 0, 1 ) ;
//		lp = fopen( "lp.txt", "a+" );
//		fprintf(lp, "%d\n", ilp);
//		fclose( lp );
//
//		imv = mvwint( 0, 1 ) ;
//		mv = fopen( "mv.txt", "a+" );
//		fprintf(hp, "%d\n", imv);
//		fclose( mv );
//
//		deriv1( 0, 1 ) ;
//
//		ide = deriv2( 0, 1 ) ;
//		de = fopen( "de.txt", "a+" );
//		fprintf(de, "%d\n", ide);
//		fclose( de );
//	}
//
//	ilp = lpfilt( datum, 0 );    //低通滤波
//	lp = fopen( "lp.txt", "a+" );
//	fprintf(lp, "%d\n", ilp);
//	fclose( lp );
//
//	ihp = hpfilt( ilp, 0 );    //高通滤波
//	hp = fopen( "hp.txt", "a+" );
//	fprintf(hp, "%d\n", ihp);
//	fclose( hp );
//
//	ide = deriv2( ihp, 0 );    //差分滤波
//	de = fopen( "de.txt", "a+" );
//	fprintf(de, "%d\n", ide);
//	fclose( de );
//
//	iab = abs( ide );    //取绝对值
//	ab = fopen( "ab.txt", "a+" );
//	fprintf(ab, "%d\n", iab);
//	fclose( ab );
//
//	imv = mvwint( iab, 0 );    //积分窗求和
//	mv = fopen( "mv.txt", "a+" );
//	fprintf(hp, "%d\n", imv);
//	fclose( mv );
//
//	fdatum = imv;
//
//	return(fdatum) ;
//}

/******************************************************************************

    函数名： lpfilt()
	语法: int lpfilt( int datum, int init)
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

int lpfilt( int datum, int init)
{
    static long y1 = 0, y2 = 0 ;
	static int data[LPBUFFER_LGTH], ptr = 0 ;
	long y0 ;
	int output, halfPtr ;
	if (init)
	{
		for(ptr = 0; ptr < LPBUFFER_LGTH; ++ptr)
		{
			data[ptr] = 0;
		}

		y1 = y2 = 0 ;
		ptr = 0 ;
	}
	halfPtr = ptr-(LPBUFFER_LGTH/2) ;	// Use halfPtr to index

	if (halfPtr < 0)							// to x[n-6].
	{
		halfPtr += LPBUFFER_LGTH;
	}

	//y[n] = 2*y[n-1] - y[n-2] + x[n] - 2*x[t-24 ms] + x[t-48 ms]，即这句话
	//的翻译，这里的乘除都是通过以为实现的
	y0 = (y1 << 1) - y2 + datum - (data[halfPtr] << 1) + data[ptr] ;
	y2 = y1;
	y1 = y0;
	output = y0 / ((LPBUFFER_LGTH*LPBUFFER_LGTH)/4);
	data[ptr] = datum ;			// Stick most recent sample into

	if (++ptr == LPBUFFER_LGTH)	// the circular buffer and update
	{
		ptr = 0;                 // the buffer pointer.
	}

	return(output) ;
}


/******************************************************************************

    函数名： hpfilt()
	语法： int hpfilt( int datum, int init )
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

int hpfilt( int datum, int init )
{
	static long y=0 ;
	static int data[HPBUFFER_LGTH], ptr = 0 ;
	int z, halfPtr ;

	if (init)
	{
		for(ptr = 0; ptr < HPBUFFER_LGTH; ++ptr)
		{
			data[ptr] = 0;
		}

		ptr = 0 ;
		y = 0 ;
	}

	y += datum - data[ptr];
	halfPtr = ptr - (HPBUFFER_LGTH / 2) ;

	if (halfPtr < 0)
	{
		halfPtr += HPBUFFER_LGTH;
	}
	z = data[halfPtr] - (y / HPBUFFER_LGTH);
	data[ptr] = datum ;

	if (++ptr == HPBUFFER_LGTH)
	{
		ptr = 0;
	}

	return( z );
}

/******************************************************************************

    函数名： deriv1()和deriv2()
	语法： int deriv1( int x, int init ) or int deriv2( int x, int init )
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

int deriv1( int x, int init )
{
	static int derBuff[DERIV_LENGTH], derI = 0 ;
	int y ;

	if (init != 0)
	{
		for(derI = 0; derI < DERIV_LENGTH; ++derI)
		{
			derBuff[derI] = 0;
		}

		derI = 0 ;
		return(0) ;
	}

	y = x - derBuff[derI] ;
	derBuff[derI] = x ;

	if (++derI == DERIV_LENGTH)
	{
		derI = 0;
	}

	return(y) ;
}

int deriv2( int x, int init )
{
	static int derBuff[DERIV_LENGTH], derI = 0 ;
	int y ;

	if (init != 0)
	{
		for(derI = 0; derI < DERIV_LENGTH; ++derI)
		{
			derBuff[derI] = 0;
		}

		derI = 0 ;
		return(0) ;
	}

	y = x - derBuff[derI] ;
	derBuff[derI] = x ;

	if (++derI == DERIV_LENGTH)
	{
		derI = 0;
	}

	return(y) ;
}




/******************************************************************************

    函数名： mvwint()
	语法： int mvwint( int datum, int init )
	描述： 积分窗求和函数
	调用： 无
	被调用： qrsfilter()
	输入参数： 整形值及是否初始化变量
	输出参数： 经过滤波后得到的信号值
	返回值： 经过滤波后得到的信号值
	其他：

******************************************************************************/

int mvwint( int datum, int init )
{
	static long sum = 0 ;
	static int data[WINDOW_WIDTH], ptr = 0 ;
	int output;

	if (init)
	{
		for (ptr = 0; ptr < WINDOW_WIDTH ; ++ptr)
		{
			data[ptr] = 0;
		}
		sum = 0 ;
		ptr = 0 ;
	}

	sum += datum ;
	sum -= data[ptr] ;
	data[ptr] = datum ;                 //构成了一个循环

	if (++ptr == WINDOW_WIDTH)
	{
		ptr = 0 ;
	}

	if ((sum / WINDOW_WIDTH) > 32000)    //这里有一个防止溢出/防噪声的判断条件
	{
			output = 32000 ;
	}
	else
	{
		output = sum / WINDOW_WIDTH ;
	}

	return(output) ;
}

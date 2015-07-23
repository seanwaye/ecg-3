/******************************************************************************

    函数名： qrsandonoffset()
	语法： int qrsandonoffset(int *buffer, int delay, int bufferptr,
	                          int samplepoint, int *lastSample,
	                          int *lastOffshift, int *thiOffshift,
							  int *qrsOnset,
	                          int *qrsOffset, int *stDif,
	                          int *pbuffer, int *outecgbuf, int *bufLength)
	描述： 利用qrsdet函数得到的延迟，以及输入的相应变量，准确得到QRS波的
	       主波位置以及QRS波起点与终点
	原理： 在一定范围内，找到最大值和最小值及它们的坐标，为两个潜在主波点
	       而后先分别计算两点左右一点斜率，斜率之积均为非负时，两者才可能
		   都是潜在主波点，在比较左右MS20点的斜率和，大的为主波
		   原理本身并不复杂，难的是经验数值的选取
	调用： 1、qrsonset()函数，QRS波起点查找函数
	       2、qrsoffset()函数，QRS波终点查找函数
	被调用：上层函数（主函数）
	输入参数： ecgbuffer：存放ecg信号；delay：qrsdet函数返回的检测延时
	           bufferptr：当前buffer元素的坐标；samplepoint：ecg信号坐标
	输出参数： qrsOnset、qrsOffset及stDif（ST段与等电位点的差值）
	返回值： QRS复合波主波精确位置
	其他： 由于是存放在一个循环数组中，对于各种情况都要考虑好边界条件
	       在这里先默认qrsdet有一个delay传来则有一个QRS波
    注意： 最初程序写好时，找主波一直有问题，经过调试，发现是数组下标问题
	       将条件（liv_max_locat_right == bufferlength）
		   改为（liv_max_locat_right >= bufferlength）即可
		   查找范围为经验数据，难以适应所有的心电数据，这个以后考虑加强
		   目前所选取的数值，适应性在100序列良好

******************************************************************************/
#include "qrsdet.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>
int qrsonset( int *buf, int maxvalue, int minvalue, int *isopoint );
int qrsoffset( int *buf, int maxvalue, int minvalue, int *stpoint );
int qrsmooth(int *buf);
/*int pro_newecgbuf( int *outbuf, int *buf, int bufLen, int onsetflag);*/

int qrsandonoffset(int *buffer, int delay, int bufferptr,int samplepoint,
	               int *lastSample, int *lastOffshift, int *thiOffshift, int *qrsOnset,
	               int *qrsOffset, int *stDif, int *pbuffer, int *outecgbuf, int *bufLength)
{
	int liv_ptr = 0;//查找的中心坐标
	int liv_begin = 0;
	int liv_control = 0;

	int liv_max = 0, liv_max_locat = 0;//最大值及(在ecgbuffer中的)位置
	int liv_max_locat_left = 0, liv_max_locat_right = 0;//左右一点坐标
	int liv_max_locat_left_slope = 0, liv_max_locat_right_slope = 0;
	int liv_max_locat_mult_slope = 0;//左右一点斜率之积
	int liv_max_locat_left_MS20 = 0, liv_max_locat_right_MS20 = 0;//坐标
	int liv_max_locat_left_MS20_slope = 0, liv_max_locat_right_MS20_slope = 0;
	int liv_max_locat_MS20_plus_slope = 0;//左右MS20点斜率绝对值之和

	int liv_min = 0, liv_min_locat = 0;//最小值及(在ecgbuffer中的)位置
	int liv_min_locat_left = 0, liv_min_locat_right = 0;//左右一点坐标
	int liv_min_locat_left_slope = 0, liv_min_locat_right_slope = 0;
	int liv_min_locat_mult_slope = 0;//左右一点斜率之积
	int liv_min_locat_left_MS20 = 0, liv_min_locat_right_MS20 = 0;//坐标
	int liv_min_locat_left_MS20_slope = 0, liv_min_locat_right_MS20_slope = 0;
	int liv_min_locat_MS20_plus_slope = 0;//左右MS20点斜率绝对值之和

	int liv_max_or_min_loc = 0;//buffer中，主波偏移量
	int qrs_shift = 0;//QRS波偏移量
	int qrs_position = 0;//QRS波位置
	int onsetShift = 0, offsetShift = 0;//QRS波起点终点偏移量

	int qrsbufferorg[MS280] = {0};//QRS左右段缓存，以主波位置为MS125点
	int qrsbuffer[MS280] = {0};//QRS左右段缓存，以主波位置为MS125点
	int qrsbuffer2[MS280] = {0};//QRS左右段缓存，以主波位置为MS125点
	int ecgbufferbegin = 0;//QRS主波在ecgbuffer中的坐标
	int liv_loop = 0;//赋值qrsbuffer的坐标变量

	int isoLoc = 0;//等电位点在qrsbuf中的坐标
	int stLoc = 0;//ST段在qrsbuf中的坐标
	int liv_iso = 0;//循环量（坐标）
	int isoTimes = 0, sumIso = 0, isoValue = 0;//次数、总和及平均值
	int liv_st = 0;////循环量（坐标）
	int stTimes = 0, sumSt = 0, stValue = 0;//次数、总和及平均值

	int pbufferbegin = 0;//P波缓存坐标
	int pbuf_loop = 0;//赋值pbuffer的坐标变量


/*
	int newecgbuf[bufferlength] = {0};
	int offsetPtrShift = 0;
	int onsetPtrShift = 0;
	int lastOffsetPtrShift = 0;
	int bufLen = 0;
	int newecgbufbegin = 0;
	int newecgbuf_loop = 0;
	int onsetFlag = 0;
*/



	//调整中心坐标
	liv_ptr = bufferptr - delay - MS70;//liv_ptr = bufferptr - delay - MS10;
	if (liv_ptr < 0)
	{
		liv_ptr = liv_ptr + bufferlength;
	}

	liv_begin = liv_ptr - MS80;//MS70
	if (liv_begin < 0)
	{
		liv_begin = liv_begin + bufferlength;
	}

	//寻找最大最小值及坐标
	liv_max = liv_min = buffer[liv_begin];
	//2013_5_3 13:29发现bug，大小值坐标应该先赋值为liv_begin，否则
	//没有满足条件的则liv_max_locat * liv_min_locat = 0
	liv_max_locat = liv_min_locat = liv_begin;

	for (liv_control = 0; liv_control < 2 * MS100; ++liv_control)
	{
		if (++liv_begin >= bufferlength)
		{
			liv_begin =0;
		}

		if (liv_max < buffer[liv_begin])
		{
			liv_max = buffer[liv_begin];
			liv_max_locat = liv_begin;
		}

		if (liv_min > buffer[liv_begin])
		{
			liv_min = buffer[liv_begin];
			liv_min_locat = liv_begin;
		}
	}

	//在这里处理谁是主波
	//先将左右一点坐标定好，目的主波斜率左右积为非正数
	liv_max_locat_left = liv_max_locat - 1;
	if (liv_max_locat_left < 0)
	{
		liv_max_locat_left = liv_max_locat_left + bufferlength;
	}

	liv_max_locat_right = liv_max_locat + 1;
	if (liv_max_locat_right >= bufferlength)
	{
		liv_max_locat_right = 0;
	}

	liv_min_locat_left = liv_min_locat - 1;
	if (liv_min_locat_left < 0)
	{
		liv_min_locat_left = liv_min_locat_left + bufferlength;
	}

	liv_min_locat_right = liv_min_locat + 1;
	if (liv_min_locat_right >= bufferlength)
	{
		liv_min_locat_right = 0;
	}

	//将左右MS20点坐标定好,目的斜率确定主波
	liv_max_locat_left_MS20 = liv_max_locat - MS20;
	if (liv_max_locat_left_MS20 < 0)
	{
		liv_max_locat_left_MS20 = liv_max_locat_left_MS20 + bufferlength;
	}

	liv_max_locat_right_MS20 = liv_max_locat + MS20;
	if (liv_max_locat_right_MS20 >= bufferlength)
	{
		liv_max_locat_right_MS20 = 0;
	}

	liv_min_locat_left_MS20 = liv_min_locat - MS20;
	if (liv_min_locat_left_MS20 < 0)
	{
		liv_min_locat_left_MS20 = liv_min_locat_left_MS20 + bufferlength;
	}

	liv_min_locat_right_MS20 = liv_min_locat + MS20;
	if (liv_min_locat_right_MS20 >= bufferlength)
	{
		liv_min_locat_right_MS20 = 0;
	}

	//计算左右一点斜率
	liv_max_locat_left_slope = buffer[liv_max_locat]
	                               - buffer[liv_max_locat_left];
	liv_max_locat_right_slope = buffer[liv_max_locat_right]
	                               - buffer[liv_max_locat];
	liv_max_locat_mult_slope = liv_max_locat_left_slope
		                           * liv_max_locat_right_slope;

	liv_min_locat_left_slope = buffer[liv_min_locat]
	                               - buffer[liv_min_locat_left];
	liv_min_locat_right_slope = buffer[liv_min_locat_right]
	                               - buffer[liv_min_locat];
	liv_min_locat_mult_slope = liv_min_locat_left_slope
		                           * liv_min_locat_right_slope;

	//计算左右MS20点斜率
	liv_max_locat_left_MS20_slope = abs(buffer[liv_max_locat]
	                                - buffer[liv_max_locat_left_MS20]);
    liv_max_locat_right_MS20_slope = abs(buffer[liv_max_locat_right_MS20]
	                                - buffer[liv_max_locat]);
    liv_max_locat_MS20_plus_slope = liv_max_locat_left_MS20_slope
		                            + liv_max_locat_right_MS20_slope;

	liv_min_locat_left_MS20_slope = abs(buffer[liv_min_locat]
	                                - buffer[liv_min_locat_left_MS20]);
	liv_min_locat_right_MS20_slope = abs(buffer[liv_min_locat_right_MS20]
	                                - buffer[liv_min_locat]);
	liv_min_locat_MS20_plus_slope = liv_min_locat_left_MS20_slope
		                            + liv_min_locat_right_MS20_slope;

	//一定条件下，决定主波
	if ((liv_max_locat_mult_slope <= 0) && (liv_min_locat_mult_slope <= 0))
	{
		if (liv_max_locat_MS20_plus_slope
			                      >=  0.80 * liv_min_locat_MS20_plus_slope)
		{
			liv_max_or_min_loc = liv_max_locat;
		}
		else
		{
			liv_max_or_min_loc = liv_min_locat;
		}
	}
	else if (liv_max_locat_mult_slope <= 0)
	{
		liv_max_or_min_loc = liv_max_locat;
	}
	else if (liv_min_locat_mult_slope <= 0)
	{
		liv_max_or_min_loc = liv_min_locat;
	}
	else
	{
		liv_max_or_min_loc = liv_max_locat;
	}


	//此处得到QRS波主主波位置
	qrs_shift = bufferptr - liv_max_or_min_loc;
	if (qrs_shift < 0)
	{
		qrs_shift = qrs_shift + bufferlength;
	}
	//QRS波位置
	qrs_position = samplepoint - qrs_shift + 1;

	//获取qrsbuffer
	ecgbufferbegin = liv_max_or_min_loc - MS125 + 1;
	if (ecgbufferbegin < 0)
	{
		ecgbufferbegin = ecgbufferbegin + bufferlength;
	}

	for (liv_loop = 0; liv_loop < MS280; ++liv_loop)
	{
		qrsbuffer[liv_loop] = buffer[ecgbufferbegin];
		if (++ecgbufferbegin >= bufferlength)
		{
			ecgbufferbegin = 0;
		}
	}

	for (liv_loop = 0; liv_loop < MS280; ++liv_loop)
	{
		qrsbufferorg[liv_loop] = qrsbuffer[liv_loop];
	}

	//平滑函数，是否效果更好，还未定
	//两次调用smooth，利用此时的buf来找起终点的起终点
	qrsmooth(qrsbuffer);
	qrsmooth(qrsbuffer);

	//目的是再保存一组只滤过两次的qrsbuf数据
	for (liv_loop = 0; liv_loop < MS280; ++liv_loop)
	{
		qrsbuffer2[liv_loop] = qrsbuffer[liv_loop];
	}
	//起点检测
	onsetShift = qrsonset( qrsbuffer, liv_max, liv_min , &isoLoc);
	*qrsOnset = qrs_position - onsetShift + 1;
	//终点检测
	offsetShift = qrsoffset( qrsbuffer2, liv_max, liv_min, &stLoc );
	*qrsOffset = qrs_position + offsetShift;//是否要加1？

	//在qrsbufferorg中求等电位点的数值和ST段的数值
	//为消除噪声的影响，均是取一段值的平均值
	for (liv_iso = isoLoc - MS10;
		                    liv_iso <= isoLoc + MS10; ++liv_iso, ++isoTimes)
	{
		sumIso = sumIso + qrsbufferorg[liv_iso];
	}
	isoValue = sumIso / isoTimes;

	for (liv_st = stLoc; liv_st < MS280; ++liv_st, ++stTimes)
	{
		sumSt = sumSt + qrsbufferorg[liv_st];
	}
	stValue = sumSt / stTimes;
	*stDif = stValue - isoValue;



	/*onsetPtrShift = qrs_shift + onsetShift;//qrs起点与当前bufptr的偏移值
	offsetPtrShift = qrs_shift - offsetShift;//qrs终点与当前bufptr的偏移值
	//上一个qrs终点与当前bufptr的偏移值
	lastOffsetPtrShift = *lastOffshift + samplepoint - *lastSample;
	//上一个qrs终点与当前qrs起点的偏移值
	onsetFlag = lastOffsetPtrShift - onsetPtrShift;

	//处理两qrs波太长的情况
	//2013_1_25此处可能有bug，要改为bufferlength-1
	if (onsetFlag >= bufferlength)
	{
		onsetFlag = bufferlength - 1;
	}
	//得出上一个qrs终点与当前qrs终点的长度为新buf的有效长度
	bufLen = lastOffsetPtrShift - offsetPtrShift;//是否加1？//1706修改去掉+1

	//2013_5_6更改传递数组的长度和数据
	//前后均加上MS30点数据
	bufLen = bufLen + MS60;

	if (bufLen >= bufferlength)
	{
		bufLen = bufferlength - 1;
	}
	//找出新buf的第一点在传递进来的ecgbuf中的坐标
	newecgbufbegin = bufferptr - lastOffsetPtrShift;

	//2013_5_6更改传递数组的长度和数据
	//将前一段数据也传递进来
	newecgbufbegin = bufferptr - lastOffsetPtrShift - MS30;

	if (newecgbufbegin < 0)
	{
		newecgbufbegin = newecgbufbegin + bufferlength;
	}
	if (newecgbufbegin < 0)   //目的是不使两delay之间太长的情况报错
	{
		newecgbufbegin = 0;
	}
	//得到newecgbuf
	for (newecgbuf_loop = 0; newecgbuf_loop < bufLen; ++newecgbuf_loop)
	{
		newecgbuf[newecgbuf_loop] = buffer[newecgbufbegin];
		if ( ++newecgbufbegin >= bufferlength )
		{
			newecgbufbegin = 0;
		}
	}
	//处理新产生的buf
	pro_newecgbuf(outecgbuf, newecgbuf, bufLen, onsetFlag);

	//注意要在这里减去此前加上的MS60
	*bufLength = bufLen - MS60;//将此参数传递到主函数，用来写数据
	*lastSample = samplepoint;//将当前采样点数赋值为过去点数
	*thiOffshift = *lastOffshift;//作用？
	*lastOffshift = offsetPtrShift;//将此次offset与ptr的偏移量存储下来
	memset(newecgbuf, 0, sizeof(newecgbuf));//清空buf
*/

	//在这里输出pbuffer最好
	//是不是不应该减去MS125
	pbufferbegin = liv_max_or_min_loc  - onsetShift - MS220 + 1;
	if (pbufferbegin < 0)
	{
		pbufferbegin = pbufferbegin + bufferlength;
	}
	for (pbuf_loop = 0; pbuf_loop < MS220; ++pbuf_loop)
	{
		pbuffer[pbuf_loop] = buffer[pbufferbegin];
		if (++pbufferbegin >= bufferlength)
		{
			pbufferbegin = 0;
		}
	}

	return(qrs_position);
}

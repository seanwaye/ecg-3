/******************************************************************************

    函数名： accurloc()
	语法： int accurloc( ChannelAnalysisStruct* ChanAnaStr,
	                                      int delay, QrsBufStruct* QrsBufStr)
	描述： accurloc--accurate location（精确位置查找函数）
	       利用qrsdet函数得到的延迟，以及输入的相应变量，准确得到QRS波的
	       主波位置以及QRS波起点与终点
	原理： 在一定范围内，找到最大值和最小值及它们的坐标，为两个潜在主波点
	       而后先分别计算两点左右一点斜率，斜率之积均为非负时，两者才可能
		   都是潜在主波点，在比较左右MS20点的斜率和，大的为主波
		   原理本身并不复杂，难的是经验数值的选取
	调用： 标准库中的abs（绝对值）函数
	被调用：int run(TotalAnalysisStruct* TotalStruct,
	                                     int* proArray, int lenArray)
	输入参数： 算法通道结构体，此通道QRS检测延迟，中间结构体
	输出参数： 算法通道结构体，此通道QRS检测延迟，中间结构体
	返回值： 具体意义未明确定下
	其他： 由于是存放在一个循环数组中，对于各种情况都要考虑好边界条件
	       在这里先默认qrsdet有一个delay传来则有一个QRS波
    注意： 最初程序写好时，找主波一直有问题，经过调试，发现是数组下标问题
	       将条件（liv_max_locat_right == bufferlength）
		   改为（liv_max_locat_right >= bufferlength）即可
		   查找范围为经验数据，难以适应所有的心电数据，这个以后考虑加强
		   目前所选取的数值，适应性在100序列良好

******************************************************************************/
#include <string.h>
#include <math.h>
#include "qrsdet.h"
#include "mult_run.h"
#include <stdlib.h>

int accurloc( ChannelAnalysisStruct* ChanAnaStr, int delay )
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
//	int qrs_position = 0;//QRS波位置

//	int ecgbufferbegin = 0;//QRS主波在ecgbuffer中的坐标
//	int liv_loop = 0;//赋值qrsbuffer的坐标变量

	//int pbufferbegin = 0;//P波缓存坐标
	//int pbuf_loop = 0;//赋值pbuffer的坐标变量

	/*int newecgbuf[bufferlength] = {0};
	int offsetPtrShift = 0;
	int onsetPtrShift = 0;
	int lastOffsetPtrShift = 0;
	int bufLen = 0;
	int newecgbufbegin = 0;
	int newecgbuf_loop = 0;
	int onsetFlag = 0;*/

	//调整中心坐标
	liv_ptr = ChanAnaStr->Ecgqueue.ecgptr - delay - MS70;//liv_ptr = bufferptr - delay - MS10;
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
	liv_max = liv_min = ChanAnaStr->Ecgqueue.ecgbuf[liv_begin];
	//2013_5_3 13:29发现bug，大小值坐标应该先赋值为liv_begin，否则
	//没有满足条件的则liv_max_locat * liv_min_locat = 0
	liv_max_locat = liv_min_locat = liv_begin;

	for (liv_control = 0; liv_control < 2 * MS100; ++liv_control)
	{
		if (++liv_begin >= bufferlength)
		{
			liv_begin =0;
		}

		if (liv_max < ChanAnaStr->Ecgqueue.ecgbuf[liv_begin])
		{
			liv_max = ChanAnaStr->Ecgqueue.ecgbuf[liv_begin];
			liv_max_locat = liv_begin;
		}

		if (liv_min > ChanAnaStr->Ecgqueue.ecgbuf[liv_begin])
		{
			liv_min = ChanAnaStr->Ecgqueue.ecgbuf[liv_begin];
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
	liv_max_locat_left_slope = ChanAnaStr->Ecgqueue.ecgbuf[liv_max_locat] -
		                       ChanAnaStr->Ecgqueue.ecgbuf[liv_max_locat_left];
	liv_max_locat_right_slope =
		                ChanAnaStr->Ecgqueue.ecgbuf[liv_max_locat_right] -
		                ChanAnaStr->Ecgqueue.ecgbuf[liv_max_locat];
	liv_max_locat_mult_slope = liv_max_locat_left_slope *
		                       liv_max_locat_right_slope;

	liv_min_locat_left_slope = ChanAnaStr->Ecgqueue.ecgbuf[liv_min_locat] -
		                       ChanAnaStr->Ecgqueue.ecgbuf[liv_min_locat_left];
	liv_min_locat_right_slope =
		                ChanAnaStr->Ecgqueue.ecgbuf[liv_min_locat_right] -
						ChanAnaStr->Ecgqueue.ecgbuf[liv_min_locat];
	liv_min_locat_mult_slope = liv_min_locat_left_slope  *
		                       liv_min_locat_right_slope;

	//计算左右MS20点斜率
	liv_max_locat_left_MS20_slope =
		           abs(ChanAnaStr->Ecgqueue.ecgbuf[liv_max_locat] -
					   ChanAnaStr->Ecgqueue.ecgbuf[liv_max_locat_left_MS20]);
	liv_max_locat_right_MS20_slope =
		           abs(ChanAnaStr->Ecgqueue.ecgbuf[liv_max_locat_right_MS20] -
					   ChanAnaStr->Ecgqueue.ecgbuf[liv_max_locat]);
	liv_max_locat_MS20_plus_slope = liv_max_locat_left_MS20_slope +
		                            liv_max_locat_right_MS20_slope;

	liv_min_locat_left_MS20_slope =
		           abs(ChanAnaStr->Ecgqueue.ecgbuf[liv_min_locat] -
				       ChanAnaStr->Ecgqueue.ecgbuf[liv_min_locat_left_MS20]);
	liv_min_locat_right_MS20_slope =
		           abs(ChanAnaStr->Ecgqueue.ecgbuf[liv_min_locat_right_MS20] -
				       ChanAnaStr->Ecgqueue.ecgbuf[liv_min_locat]);
	liv_min_locat_MS20_plus_slope = liv_min_locat_left_MS20_slope +
		                            liv_min_locat_right_MS20_slope;

	//一定条件下，决定主波
	if ((liv_max_locat_mult_slope <= 0) && (liv_min_locat_mult_slope <= 0))
	{
		if (liv_max_locat_MS20_plus_slope >=
			                       0.80 * liv_min_locat_MS20_plus_slope)
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
	qrs_shift = ChanAnaStr->Ecgqueue.ecgptr - liv_max_or_min_loc;
	if (qrs_shift < 0)
	{
		qrs_shift = qrs_shift + bufferlength;
	}
	//QRS波位置
	ChanAnaStr->qrsLoc = ChanAnaStr->Ecgqueue.ecgnumber - qrs_shift + 1;

	return 0;
}

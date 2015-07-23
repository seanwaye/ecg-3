/******************************************************************************

    函数名： run()
	语法： int run(struct AnalysisData* outStruct, int* proArray,
	               int lenArray, struct Ecgqueue* queue, 
				   struct Displaydata* displaydata)
	描述： 输入一段信号，调用各函数对这段信号进行分析，输出参数存放在结构
	       outStruct中
	原理： 
	调用： qrsandonoffset()、arrhyalgone()函数
	被调用：主函数
	输入参数： proArray、lenArray为信号数组及数组长度，结构queue中存放的的是
	           两秒钟信号，当前循环数组中的坐标及总采样数
	输出参数： 结构outStruct，以上结构具体定义见run.h
	返回值：输出最近的检测到的输入信号的QRS波位置，若未找到新的QRS波，则返回-1 
	其他： 以后还可对返回值进行更详细的定义
    注意： 由于run中有一个循环，注意每次在循环中一些变量要赋初始值
	 
******************************************************************************/
#include "mult_run.h"
#include "filter.h"
#include "fundecla.h"
#include <stdio.h>

int mult_run(TotalAnalysisStruct* TotalStruct, int* proArray, int lenArray)
{
	int ecgvalue = 0;
	int delay = 0;
	int lastqrsLOc = 0;
	int liv_loopctrone = 0;

	/*FILE *values;
	char filename[50] = {"threeasystolefil1.txt"};
	FILE *qrsLocfil;
	char qrsLocfilename[50] = {"threeasystoleqrsLoc21.txt"};*/

	for (liv_loopctrone = 0; liv_loopctrone < lenArray; ++liv_loopctrone)
	{
		//记下当前各通道的采样点数值
		++(TotalStruct->ChannelAnalysis[liv_loopctrone].Ecgqueue.ecgnumber);
		//滤波不需要判断各导联均进行
		ecgvalue = proArray[liv_loopctrone];
		ecgvalue = hpFilter(&TotalStruct->ChannelAnalysis[liv_loopctrone].FilterData, ecgvalue);
		ecgvalue = lpFilter(&TotalStruct->ChannelAnalysis[liv_loopctrone].FilterData, ecgvalue);
		ecgvalue = subpowerline(&TotalStruct->ChannelAnalysis[liv_loopctrone].FilterData, ecgvalue);
		ecgvalue = smooth(&TotalStruct->ChannelAnalysis[liv_loopctrone].FilterData, ecgvalue);

		TotalStruct->ChannelAnalysis[liv_loopctrone].OutDate.filterEcgValue = ecgvalue;
		TotalStruct->ChannelAnalysis[liv_loopctrone].Ecgqueue.ecgbuf[TotalStruct->ChannelAnalysis[liv_loopctrone].Ecgqueue.ecgptr] = ecgvalue;
		if (++(TotalStruct->ChannelAnalysis[liv_loopctrone].Ecgqueue.ecgptr) >= bufferlength)
		{
			TotalStruct->ChannelAnalysis[liv_loopctrone].Ecgqueue.ecgptr = 0;
		}

		/*if (liv_loopctrone == 0)
		{
			values = fopen(filename, "a+");
			fprintf(values, "%d", ecgvalue);
			fprintf(values, ",");
			fclose(values);
		} 
		else if (liv_loopctrone == 1)
		{
			values = fopen(filename, "a+");
			fprintf(values, "%d", ecgvalue);
			fprintf(values, ",");
			fclose(values);
		}
		else
		{
			values = fopen(filename, "a+");
			fprintf(values, "%d\n", ecgvalue);
			fclose(values);
		}*/

		//根据算法通道设置判断决定
		if (TotalStruct->ChannelAnalysis[liv_loopctrone].algoChan)
		{
			//后续程序均在此判断中完成
			delay = multqrsdet( &TotalStruct->ChannelAnalysis[liv_loopctrone].qrsDetStr, ecgvalue );
			if (delay != 0)
			{
				TotalStruct->ChannelAnalysis[liv_loopctrone].delayflag = 0;
				lastqrsLOc = TotalStruct->ChannelAnalysis[liv_loopctrone].qrsLoc;
				accurloc( &TotalStruct->ChannelAnalysis[liv_loopctrone], delay );	
				
				++TotalStruct->ChannelAnalysis[liv_loopctrone].qrsnum;
				statfun( &TotalStruct->ChannelAnalysis[liv_loopctrone], lastqrsLOc );

				/*qrsLocfil = fopen(qrsLocfilename, "a+");
				fprintf(qrsLocfil, "%d", TotalStruct->ChannelAnalysis[liv_loopctrone].qrsLoc);
				fprintf(qrsLocfil, ",");
				fclose(qrsLocfil);*/
			}
			++TotalStruct->ChannelAnalysis[liv_loopctrone].delayflag;
			if (TotalStruct->ChannelAnalysis[liv_loopctrone].delayflag >= MS3000)
			{
				TotalStruct->ChannelAnalysis[liv_loopctrone].heartRate = 0;
			}

			
		}

		
		delay = 0;//每次循环后将此值清零，否则其他文件中也会有非零值
	}

	return 0;
}
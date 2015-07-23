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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "run.h"
#include "detect.h"
#include "qrsdet.h"

int run(struct AnalysisData* outStruct, int* proArray,
	    int lenArray, struct Ecgqueue* queue)
{
	int qrsposition = 0;
	int onset = 0, offset = 0, qrswide = 0;
	int stVaDif = 0;
	int pbuffer[MS220] = {0};
	int loopCtr = 0;
	int ecgValue = 0;
	int delay = 0;
	int lastqrsLOc = 0, lastHRate = 0;
	double rrInterval = 0;
	double rrInter = 0;
	double heartRate = 0;
	int HRate = 0;
	int zeroFlag = 0, flagCtr = 0;
	int sumHRate = 0, plusTimes = 0;
	const int memmovelen = 4 * sizeof(int);
	//int errornum = 0;//调试用

	int outBuf[bufferlength] = {0};
	int lenBuf = 0;
	int bufloop = 0;
	int ecgptr = 0;
	int bufLen_loop = 0;

	/*FILE *values, *offloc;
	char offlocname[50] = {"testsinusarrqrsloc.txt"};
	char filename[50] = {"testsinusarrfil.txt"};*/

	for (loopCtr = 0; loopCtr < lenArray; ++loopCtr)
	{
		++(queue->ecgnumber);
		ecgValue = proArray[loopCtr];
		ecgValue = sub_dc_offset_250(ecgValue, 0);
		ecgValue = sub_hf_250_fir_1(ecgValue, 0);
		ecgValue = sub_hf_250_fir_2(ecgValue, 0);
		ecgValue = subpowerline250(ecgValue, 0);
		//循环存储ECG信号
		ecgValue = smooth_5_points(ecgValue, 0);

		/*values = fopen(filename, "a+");
		fprintf(values, "%d\n", ecgValue);
		fclose(values);*/

		outStruct->filterEcgValue = ecgValue;
		queue->ecgbuf[queue->ecgptr] = ecgValue;

		if (++(queue->ecgptr) == bufferlength)
		{
			queue->ecgptr = 0;
		}

		delay = qrsdet( ecgValue, 0 );

		if (delay != 0)
		{

			/*displaydata->flagOfdefg = 0;
			displaydata->judgeflag = 0;
			displaydata->oncedelayflag = 1;
			displaydata->bufferlengthflag = 0;
			if (displaydata->lastdelayflag == 0)
			{
				displaydata->delayflag = 1;
			}
			if ((displaydata->lastdelayflag == 1) || (displaydata->lastdelayflag == 2))
			{
				displaydata->delayflag = 2;
			}*/


			//++errornum;//调试用
			outStruct->delayflag = 0;
			lastqrsLOc = outStruct->qrsLoc;
			lastHRate = outStruct->lastHRate;
			qrsposition = qrsandonoffset(queue->ecgbuf, delay, queue->ecgptr,  
				        queue->ecgnumber, &(queue->lastecgnumber), 
						&(queue->lastOffSft), &(queue->thiOffSft), &onset, &offset,
						&stVaDif, pbuffer, outBuf, &lenBuf);

			/*offloc = fopen(offlocname, "a+");
			fprintf(offloc, "%d\n", outStruct->qrsLoc);
			fclose(offloc);*/

			qrswide = offset - onset;
			arrhyalgone( qrsposition, qrswide, &(outStruct->beaType),
				                                         &(outStruct->rhyme), 0);
			outStruct->delay = delay;
			outStruct->qrsLoc = qrsposition;
			outStruct->qrsOnLoc = onset;
			outStruct->qrsOffLoc = offset;
			outStruct->qrsWide = qrswide;
			outStruct->stDif = stVaDif;
			++(outStruct->qrsnum);
			//得输出心率
			if (lastqrsLOc && qrsposition)
			{
				rrInterval = qrsposition - lastqrsLOc;
				rrInter = rrInterval / SAMPLE_RATE;
				heartRate = 60 / rrInter;
				HRate = (int)heartRate;
				if (lastHRate == 0)
				{
					outStruct->lastHRate = HRate;
				} 
				else if (((0.7 * outStruct->heartRate <= HRate) 
					        && (HRate <= 1.4 * outStruct->heartRate)) 
							           || (outStruct->heartRate) == 0 
							                 || (HRate <= 201 && HRate >= 40))
				{
					outStruct->lastHRate = HRate;
				}
				memmove(outStruct->HrArray, 
					                     &(outStruct->HrArray[1]), memmovelen);
				outStruct->HrArray[4] = outStruct->lastHRate;
				zeroFlag = 0;//赋初始值
				for (flagCtr = 0; flagCtr < 5; ++flagCtr)
				{
					if (outStruct->HrArray[flagCtr] == 0)
					{
						zeroFlag = zeroFlag + 1;
					}
				}
				if (zeroFlag >= 5)
				{
					outStruct->heartRate = 0;
				} 
				else
				{
					sumHRate = 0;//赋初始值
					plusTimes = 0;//赋初始值
					for (; zeroFlag < 5; ++zeroFlag)
					{
						++plusTimes;
						sumHRate = sumHRate + outStruct->HrArray[zeroFlag];
					}
					outStruct->heartRate = sumHRate / plusTimes;
				}
			}
			else
			{
				outStruct->heartRate = 0;
			}
			//得全部RR间期和
			if (outStruct->qrsnum > 1)
			{
				//这样得到的RR间期和不太合理，因为信号原因长时间未检测到QRS波时
				//两QRS之间有很长一段时间
				outStruct->sumRR = outStruct->sumRR + qrsposition - lastqrsLOc;
			}
			//得不正常心跳数
			if (outStruct->qrsnum > 5)
			{
				if (outStruct->beaType != 'N')
				{
					++(outStruct->abnorBeatNum);
				}
			}
			//得ST段值
			if ((outStruct->qrsnum > 5) && (outStruct->qrsnum <= 15))
			{
				if (outStruct->beaType == 'N')
				{
					++(outStruct->refStNum);
					outStruct->sumRefStVa = outStruct->sumRefStVa 
						                                    + outStruct->stDif;
				}
			}
			if (outStruct->qrsnum > 15)
			{
				if (outStruct->beaType == 'N')
				{
					++(outStruct->sumStNum);
					outStruct->sumStValue = outStruct->sumStValue
						                                    + outStruct->stDif;
				}
			}

			/*if ((displaydata->lastdelayflag == 1) && (displaydata->delayflag == 2))
			{
				for (bufloop = __max(queue->thiOffSft - 1, 0); bufloop < lenBuf; ++bufloop)
				{
					displaydata->displaybuf[displaydata->saveptr] = outBuf[bufloop];
					++displaydata->saveptr;
					if (displaydata->saveptr >= displaylength)
					{
						displaydata->saveptr = 0;
					}
				}
			}

			if ((displaydata->lastdelayflag == 2) && (displaydata->delayflag == 2))
			{
				for (bufloop = 0; bufloop < lenBuf; ++bufloop)
				{
					displaydata->displaybuf[displaydata->saveptr] = outBuf[bufloop];
					++displaydata->saveptr;
					if (displaydata->saveptr >= displaylength)
					{
						displaydata->saveptr = 0;
					}
				}
			}

			displaydata->lastdelayflag = displaydata->delayflag;
*/

		}

		++outStruct->delayflag;
		if (outStruct->delayflag >= MS3000)
		{
			outStruct->heartRate = 0;
			if (queue->ecgnumber >= MS20000)
			{
				outStruct->asystoleflag = 1;
			}
		}

/*
		//初始学习阶段，来一点存一点
		if ((displaydata->oncedelayflag == 0) && (displaydata->delayflag == 0) && (displaydata->lastdelayflag == 0))
		{
			displaydata->displaybuf[displaydata->saveptr] = ecgValue;
			++displaydata->saveptr;
			if (displaydata->saveptr >= displaylength)
			{
				displaydata->saveptr = 0;
			}
		}

		//displaydata.judgeflag的作用是确保下面的if和else if只进入一次
		//是否是减1,根据信号分类，这种情况为找到了两个或两个以上的qrs波时
		if (((queue->ecgnumber - (queue->lastecgnumber - queue->lastOffSft)) >= bufferlength) && ((displaydata->delayflag == 2) || (displaydata->flagOfdefg == 1)) && (displaydata->judgeflag != 2))
		{
			displaydata->flagOfdefg = 1;
			displaydata->delayflag = 0;
			displaydata->lastdelayflag = 0;
			displaydata->judgeflag = 1;
			if (displaydata->bufferlengthflag)
			{
				displaydata->displaybuf[displaydata->saveptr] = ecgValue;
				++displaydata->saveptr;
				if (displaydata->saveptr >= displaylength)
				{
					displaydata->saveptr = 0;
				}
			}

			if (displaydata->oncedelayflag && (!displaydata->bufferlengthflag))//若原来已找到过，存到此前bufferlength长度
			{
				displaydata->bufferlengthflag = 1;
				ecgptr = queue->ecgptr;
				for (bufLen_loop = 0; bufLen_loop < bufferlength; ++bufLen_loop)
				{
					displaydata->displaybuf[displaydata->saveptr] = queue->ecgbuf[ecgptr];
					++displaydata->saveptr;
					++ecgptr;
					if (displaydata->saveptr >= displaylength)
					{
						displaydata->saveptr = 0;
					}
					if (ecgptr >= bufferlength)
					{
						ecgptr = 0;
					}
				}
			}

		}

		//是否是减1,根据信号分类，这种情况为找到了未找到两个或两个以上的qrs波时
		//2013_5_3改
		//将(i - last_i) >= bufferlength)改为(i - last_i + 1) >= bufferlength)
		else if (((queue->ecgnumber - queue->lastecgnumber + 1) >= bufferlength) && ((displaydata->delayflag == 1) || (displaydata->flagOfdefg == 2)) && (displaydata->judgeflag != 1))//是否是减1
		{
			displaydata->flagOfdefg = 2;
			displaydata->delayflag = 0;
			displaydata->lastdelayflag = 0;
			displaydata->judgeflag = 2;

			if (displaydata->bufferlengthflag)
			{
				displaydata->displaybuf[displaydata->saveptr] = ecgValue;
				++displaydata->saveptr;
				if (displaydata->saveptr >= displaylength)
				{
					displaydata->saveptr = 0;
				}
			}

			if (displaydata->oncedelayflag && (!displaydata->bufferlengthflag))//若原来已找到过，存到此前bufferlength长度
			{
				displaydata->bufferlengthflag = 1;
				ecgptr = queue->ecgptr - 1;
				//新加2_4_11_17
				if (ecgptr < 0)
				{
					ecgptr = bufferlength - 1;
				}
				for (bufLen_loop = 0; bufLen_loop < bufferlength; ++bufLen_loop)
				{
					displaydata->displaybuf[displaydata->saveptr] = queue->ecgbuf[ecgptr];
					++displaydata->saveptr;
					++ecgptr;
					if (displaydata->saveptr >= displaylength)
					{
						displaydata->saveptr = 0;
					}
					if (ecgptr >= bufferlength)
					{
						ecgptr = 0;
					}
				}
			}

		}*/
		
/*
		if (queue->ecgnumber <= bufferlength)
		{
			outStruct->maintainrcgvalue = 0;
		} 
		else
		{
			outStruct->maintainrcgvalue = displaydata->displaybuf[displaydata->maintainptr];
			++displaydata->maintainptr;
			if (displaydata->maintainptr >= displaylength)
			{
				displaydata->maintainptr = 0;
			}
		}*/

		/*values = fopen(filename, "a+");
		fprintf(values, "%d\n", outStruct->filterEcgValue);
		fclose(values);*/

	}
	
	if (delay == 0)
	{
		return(-1);
	} 
	else
	{
		return(outStruct->qrsLoc);
	}
} 
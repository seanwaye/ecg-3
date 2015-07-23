/******************************************************************************

    函数名： initvar()
	语法： int initvar(TotalAnalysisStruct* TotalStruct, 
	                   ChannelAnalysisStruct* ChannelStruct, int channelNum)
	描述： 
	原理： 
	调用： qrsdet()、arrhyalgone()
	被调用：主函数
	输入参数： 
	输出参数： 无
	返回值： 0表示初始化正常
	其他： 暂时未对异常判断，如判断指针是否为空，空间大小是否足够等等
	       后面添加
    注意： 
	 
******************************************************************************/
#include "mult_run.h"

int mult_initvar(TotalAnalysisStruct* TotalStruct, 
	        ChannelAnalysisStruct* ChannelStruct, int channelNum, InitVarStruct* InitVarStr)
{
	int liv_loopctrone, liv_loopctrtwo;
	TotalStruct->ChannelAnalysis = ChannelStruct;
	TotalStruct->channelNum = channelNum;

	for (liv_loopctrone = 0; liv_loopctrone < channelNum; ++liv_loopctrone)
	{
		//对算法通道的定义
		ChannelStruct[liv_loopctrone].algoChan = 
			                         InitVarStr->algoChanArray[liv_loopctrone];
		for (liv_loopctrtwo = 0; liv_loopctrtwo < hpNL; ++liv_loopctrtwo)
		{
			ChannelStruct[liv_loopctrone].FilterData.hpDataIn[liv_loopctrtwo] = 0;
		}

		for (liv_loopctrtwo = 0; liv_loopctrtwo < hpDL; ++liv_loopctrtwo)
		{
			ChannelStruct[liv_loopctrone].FilterData.hpDataOut[liv_loopctrtwo] = 0;
		}

		for (liv_loopctrtwo = 0; liv_loopctrtwo < lpNL; ++liv_loopctrtwo)
		{
			ChannelStruct[liv_loopctrone].FilterData.lpDataIn[liv_loopctrtwo] = 0;
		}

		for (liv_loopctrtwo = 0; liv_loopctrtwo < lpDL; ++liv_loopctrtwo)
		{
			ChannelStruct[liv_loopctrone].FilterData.lpDataOut[liv_loopctrtwo] = 0;
		}

		for (liv_loopctrtwo = 0; liv_loopctrtwo < smoothBL; ++liv_loopctrtwo)
		{
			ChannelStruct[liv_loopctrone].FilterData.smoothDataIn[liv_loopctrtwo] = 0;
		}
		for (liv_loopctrtwo = 0; liv_loopctrtwo < datainlen; ++liv_loopctrtwo)
		{
			ChannelStruct[liv_loopctrone].FilterData.subPL.dataIn[liv_loopctrtwo] = 0;
		}
		for (liv_loopctrtwo = 0; liv_loopctrtwo < interferencelen; ++liv_loopctrtwo)
		{
			ChannelStruct[liv_loopctrone].FilterData.subPL.interference[liv_loopctrtwo] = 0;
		}
		ChannelStruct[liv_loopctrone].FilterData.subPL.ptr = 0;
		ChannelStruct[liv_loopctrone].FilterData.subPL.jVar = 0;
		ChannelStruct[liv_loopctrone].FilterData.subPL.ctrVar = 1;
		ChannelStruct[liv_loopctrone].FilterData.subPL.crit = InitVarStr->precision[liv_loopctrone];//此处可以考虑不同通道的精度不一样

		//QRS复合波查找相关量初始化
		//低通
		ChannelStruct[liv_loopctrone].qrsDetStr.qrsFilterStr.qrsLpStr.dataoutone = 0;
		ChannelStruct[liv_loopctrone].qrsDetStr.qrsFilterStr.qrsLpStr.dataouttwo = 0;
		ChannelStruct[liv_loopctrone].qrsDetStr.qrsFilterStr.qrsLpStr.lptr = 0;
		for (liv_loopctrtwo = 0; liv_loopctrtwo < LPBUFFER_LGTH; ++liv_loopctrtwo)
		{
			ChannelStruct[liv_loopctrone].qrsDetStr.qrsFilterStr.qrsLpStr.datain[liv_loopctrtwo] = 0;
		}
		//高通
		ChannelStruct[liv_loopctrone].qrsDetStr.qrsFilterStr.qrsHpStr.dataoutone = 0;
		ChannelStruct[liv_loopctrone].qrsDetStr.qrsFilterStr.qrsHpStr.hptr = 0;
		for (liv_loopctrtwo = 0; liv_loopctrtwo < HPBUFFER_LGTH; ++liv_loopctrtwo)
		{
			ChannelStruct[liv_loopctrone].qrsDetStr.qrsFilterStr.qrsHpStr.datain[liv_loopctrtwo] = 0;
		}
		//差分
		ChannelStruct[liv_loopctrone].qrsDetStr.qrsFilterStr.qrsDerStr[0].derptr = 0;
		for (liv_loopctrtwo = 0; liv_loopctrtwo < DERIV_LENGTH; ++liv_loopctrtwo)
		{
			ChannelStruct[liv_loopctrone].qrsDetStr.qrsFilterStr.qrsDerStr[0].datain[liv_loopctrtwo] = 0;
		}
		ChannelStruct[liv_loopctrone].qrsDetStr.qrsFilterStr.qrsDerStr[1].derptr = 0;
		for (liv_loopctrtwo = 0; liv_loopctrtwo < DERIV_LENGTH; ++liv_loopctrtwo)
		{
			ChannelStruct[liv_loopctrone].qrsDetStr.qrsFilterStr.qrsDerStr[1].datain[liv_loopctrtwo] = 0;
		}
		//积分
		ChannelStruct[liv_loopctrone].qrsDetStr.qrsFilterStr.qrsMvStr.sum = 0;
		ChannelStruct[liv_loopctrone].qrsDetStr.qrsFilterStr.qrsMvStr.mvptr = 0;
		for (liv_loopctrtwo = 0; liv_loopctrtwo < WINDOW_WIDTH; ++liv_loopctrtwo)
		{
			ChannelStruct[liv_loopctrone].qrsDetStr.qrsFilterStr.qrsMvStr.datain[liv_loopctrtwo] = 0;
		}
		//peak查找，qrsDetStruct
		ChannelStruct[liv_loopctrone].qrsDetStr.qrsPeakStr.maxvalue = 0;
		ChannelStruct[liv_loopctrone].qrsDetStr.qrsPeakStr.timeSinceMax = 0;
		ChannelStruct[liv_loopctrone].qrsDetStr.qrsPeakStr.lastDatum = 0;
		//qrsdet.c中所用，qrsDetStruct
		ChannelStruct[liv_loopctrone].qrsDetStr.det_thresh = 0;
		ChannelStruct[liv_loopctrone].qrsDetStr.qpkcnt = 0;
		ChannelStruct[liv_loopctrone].qrsDetStr.rsetCount = 0;
		ChannelStruct[liv_loopctrone].qrsDetStr.nmedian = 0;
		ChannelStruct[liv_loopctrone].qrsDetStr.qmedian = 0;
		ChannelStruct[liv_loopctrone].qrsDetStr.rrmedian = 0;
		ChannelStruct[liv_loopctrone].qrsDetStr.countnum = 0;
		ChannelStruct[liv_loopctrone].qrsDetStr.sbpeak = 0;
		ChannelStruct[liv_loopctrone].qrsDetStr.sbloc = 0;
		ChannelStruct[liv_loopctrone].qrsDetStr.sbcount = MS1500;
		ChannelStruct[liv_loopctrone].qrsDetStr.initBlank = 0;
		ChannelStruct[liv_loopctrone].qrsDetStr.initMax = 0;
		ChannelStruct[liv_loopctrone].qrsDetStr.preBlankCnt = 0;
		ChannelStruct[liv_loopctrone].qrsDetStr.tempPeak = 0;
		for (liv_loopctrtwo = 0; liv_loopctrtwo < arrayLen; ++liv_loopctrtwo)
		{
			ChannelStruct[liv_loopctrone].qrsDetStr.qrsbuf[liv_loopctrtwo] = 0;
		}
		for (liv_loopctrtwo = 0; liv_loopctrtwo < arrayLen; ++liv_loopctrtwo)
		{
			ChannelStruct[liv_loopctrone].qrsDetStr.noise[liv_loopctrtwo] = 0;
		}
		for (liv_loopctrtwo = 0; liv_loopctrtwo < arrayLen; ++liv_loopctrtwo)
		{
			ChannelStruct[liv_loopctrone].qrsDetStr.rsetBuff[liv_loopctrtwo] = 0;
		}
		for (liv_loopctrtwo = 0; liv_loopctrtwo < arrayLen; ++liv_loopctrtwo)
		{
			ChannelStruct[liv_loopctrone].qrsDetStr.rrbuf[liv_loopctrtwo] = MS1000;
		}

		//EcgqueueStruct
		ChannelStruct[liv_loopctrone].Ecgqueue.ecgptr = 0;
		ChannelStruct[liv_loopctrone].Ecgqueue.ecgnumber = 0;
		for (liv_loopctrtwo = 0; liv_loopctrtwo < bufferlength; ++liv_loopctrtwo)
		{
			ChannelStruct[liv_loopctrone].Ecgqueue.ecgbuf[liv_loopctrtwo] = 0;
		}

		//OutDataStruct
		ChannelStruct[liv_loopctrone].OutDate.filterEcgValue = 0;

		//ChannelAnalysisStruct
		ChannelStruct[liv_loopctrone].qrsLoc = 0;
		ChannelStruct[liv_loopctrone].heartRate = 0;
		for (liv_loopctrtwo = 0; liv_loopctrtwo < HrArLen; ++liv_loopctrtwo)
		{
			ChannelStruct[liv_loopctrone].HrArray[liv_loopctrtwo] = 0;
		}
		ChannelStruct[liv_loopctrone].lastHRate = 0;
		ChannelStruct[liv_loopctrone].qrsnum = 0;
		ChannelStruct[liv_loopctrone].delayflag = 0;

	}


	return 0;
}
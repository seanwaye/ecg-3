/******************************************************************************

    函数名： arrhyalgone()
	语法： int arrhyalgone( int qrsposition, int qrswide,
	                                        char *type, int *rhythm, int init)
	描述： 利用RR间期及QRS波宽度检测心律不齐
	       参考论文：
		   1、基于决策树与动态阈值的嵌入式心电监护系统
		   2、基于嵌入式微机的便携心电监护仪设计
	调用： 标准库中的memcpy（字符串复制）函数
	被调用：上层函数（主函数）
	输入参数： QRS复合波位置，QRS波宽度及是否初始化变量
	输出参数： 分三种心拍和四种节律类型
	           上一时刻QRS波的心拍类型
	           N（正常心拍）、V（室性早搏心拍）、O（其他早搏心拍如房性、
			   室上性等）
			   及节律类型
			   (N(正常节律)、(T(心动过速)、(B(心动过缓)、(A(停搏)
	返回值： 返回值意义还未定义, 暂时未对异常判断，如判断指针是否为空，
	         空间大小是否足够等等后面添加
	其他： 若init非0时，一些量赋初始值
	       由于心律不齐检测分为心拍检测和心律检测，所以考虑输出两种量，
		   心拍与节律，由于节律一般都是要有多个心拍，所以有延迟产生，
		   如何解决这个问题需要考虑
		   试想输出三种心拍：正常、室性早搏及其他类早搏
		   输出四种节律：正常、心动过速、心动过缓及停搏
		   停搏是属于心拍还是节律，最好还是当做节律
		   这种利用RR间期来判别停搏的方法并不是很好，考虑一种特殊情况：在某一
		   时间后，无搏动，因为不会调用此函数，此时怎能判别停搏？
		   最好节律输出与数据库中类似，并不是每个心拍都输出，而是与之前节律
		   不一致时，才输出，这个在主程序中控制
		   关于正常RR间期的替换（改进）：1、当前心拍正常且上一心拍也正常才行
		   2、针对漏波，当前RR间期小于正常周期均值的1.6倍左右时，才能替换，
		   以防有漏波心拍，使得下一心拍判断成早搏心拍
	调试程序：决定效果好坏的两大原因：1、rr间期的经验系数,这个在目前情况下影响
	          不是很大，如针对100信号，没找到异常的有两个，主要是系数由0.8变为
			  0.83左右即可，后面可以考虑转换为0.85左右，应该都没有什么问题
			  （当做这个问题已经解决）
		      2、目前处理的信号均为原始信号，所以查找的QRS波宽度不准确，这是
			  影响

			  每得到一个qrs便得到此时刻的心拍类型

******************************************************************************/

#include <string.h>
#include "qrsdet.h"

int arrhyalgone( int qrsposition, int qrswide,
	                                  char *type, int *rhythm, int init)
{
	static int rr1 = 0, rr2 = 0, rr3 = 0, rr4 = 0, rr5 = 0;//RR间期
	static int arr1 = 0, arr2 = 0, arr3 = 0, arr4 = 0;//正常心拍的RR间期
	static int ave_rr = 0, cur_rr = 0;//平均RR间期和当前RR间期
	static int qrsp1 = 0, qrsp2 = 0, qrsp3 = 0,
		                        qrsp4 = 0, qrsp5 = 0, qrsp6 = 0;//QRS波位置
	static int ta_flag = 0, br_flag = 0;//心动过速及过缓标志位
	static char lastype = '\0', thirdtype = '\0', fourthtype = '\0';
	static char fifthtype = '\0', sixthtype = '\0';
	int qrswidth = 0;//QRS波宽度
	int intRR = 0;
	real realRR = 0;
//	int testone = 0;
/*
	int va_flag = 0;//室性心动过速标志
	int aa_flag = 0;//房性心动过速标志
	int vb_flag = 0;//室性二联律标志
	int vt_flag = 0;//室性三联律标志
	int pp_flag = 0;//成对PVCs标志
	int missbeat_flag = 0;//窦性停搏标志
	int pva_flag = 0;//室性早搏标志
	int atr_flag = 0;//房性早搏标志
	int normal_flag = 0;//正常心率标志*/
	int rhythm_flag = 0;

	if (init)
	{
		rr1 = rr2 = rr3 = rr4 = 0;

		ave_rr = arr1 = arr2 = arr3 = arr4 = cur_rr = 0;
		qrsp1 = qrsp2 = qrsp3 = qrsp4 = qrsp5 = qrsp6 = 0;
		ta_flag = br_flag = 0;
		lastype = 'N';
		thirdtype = 'N';
		fourthtype = 'N';
		fifthtype = 'N';
		sixthtype = 'N';
		*rhythm = 7;
	}
	qrsp1 = qrsp2;
	qrsp2 = qrsp3;
	qrsp3 = qrsp4;
	qrsp4 = qrsp5;
	qrsp5 = qrsp6;
	qrsp6 = qrsposition;
	qrswidth = qrswide;
	rr1 = qrsp2 - qrsp1;
	rr2 = qrsp3 - qrsp2;
	rr3 = qrsp4 - qrsp3;
	rr4 = qrsp5 - qrsp4;
	rr5 = qrsp6 - qrsp5;

	if (qrsp1 && qrsp2 && qrsp3 && qrsp4 && qrsp5 && qrsp6)
	{
		cur_rr = rr5;
		//心拍类型
		*type = 'N';
		//由于宽度查找太宽，而更改宽度查找函数更麻烦
		//（后续解决这个问题肯定还是要），所以先改阈值MS120为MS135
		intRR = ((int)(0.855 * ave_rr + 0.56));

		realRR = (0.855 * ave_rr + 0.56) - intRR;
		if (realRR >= 0.5000)
		{
			intRR = intRR + 1;
		}
		if (realRR <= -0.5000)
		{
			intRR = intRR - 1;
		}
//		testone = MS135;
		if (cur_rr <= intRR && (qrswidth >= MS135))
		{
			*type = 'V';
		}
		if (cur_rr <= intRR && (qrswidth < MS135))
		{
			*type = 'O';
		}

		//更改ave_rr
		if ((*type == 'N') && (lastype == 'N') &&
			               (cur_rr < 1.5 * ave_rr) && (cur_rr > 0.7 * ave_rr))
		{
			arr1 = arr2;
			arr2 = arr3;
			arr3 = arr4;
			arr4 = cur_rr;
			ave_rr = (arr1 + arr2 + arr3 + arr4) / 4;
		}


		//处理细分各类型程序
		if ((thirdtype == 'V') && (lastype == 'V') && (*type == 'V'))
		{
			rhythm_flag = 15;
		}
		else if((thirdtype == 'O') && (lastype == 'O') && (*type == 'O'))
		{
			rhythm_flag = 14;
		}
		else if((fourthtype == 'N') && (thirdtype == 'V')
			                        && (lastype == 'N') && (*type == 'V'))
		{
			rhythm_flag = 13;
		}
		else if((sixthtype == 'N') && (fifthtype == 'N') &&
			    (fourthtype == 'V') && (thirdtype == 'N') &&
				(lastype == 'N') && (*type == 'V'))
		{
			rhythm_flag = 12;
		}
		else if((sixthtype == 'V') && (fifthtype == 'V') &&
			    (fourthtype == 'N') && (thirdtype == 'V') &&
			    (lastype == 'V') && (*type == 'N'))
		{
			rhythm_flag = 12;
		}
		else if((lastype == 'V') && (*type == 'V'))
		{
			rhythm_flag = 11;
		}
		else if(cur_rr >= MS1500)
		{
			rhythm_flag = 10;
		}
		else if(*type == 'V')
		{
			rhythm_flag = 9;
		}
		else if(*type == 'O')
		{
			rhythm_flag = 8;
		}
		else
		{
			rhythm_flag = 7;
		}

		if (rhythm_flag > (*rhythm))
		{
			*rhythm = rhythm_flag;
		}

		sixthtype = fifthtype;
		fifthtype = fourthtype;
		fourthtype = thirdtype;
		thirdtype = lastype;
		lastype = *type;

/*
		//节律类型
		memcpy(rhythm,"(N",4);
		if (cur_rr <= MS600)
		{
			++ta_flag;
		}
		else
		{
			ta_flag = 0;
		}
		if ((cur_rr >= MS1200) && (cur_rr < MS2400))
		{
			++br_flag;
		}
		else
		{
			br_flag = 0;
		}
		if (ta_flag > 3)
		{
			memcpy(rhythm,"(T",4);
		}
		if (br_flag > 3)
		{
			memcpy(rhythm,"(B",4);
		}
		if (cur_rr >= MS2400)
		{
			memcpy(rhythm,"(A",4);
		}*/

	}
	else
	{
		arr1 = rr2;
		arr2 = rr3;
		arr3 = rr4;
		arr4 = rr5;
		ave_rr = (arr1 + arr2 + arr3 + arr4) / 4;
		sixthtype = fifthtype;
		fifthtype = fourthtype;
		fourthtype = thirdtype;
		thirdtype = lastype;
		lastype = *type;
		*type = 'N';
		*rhythm = 7;
	}
	return(0);
}

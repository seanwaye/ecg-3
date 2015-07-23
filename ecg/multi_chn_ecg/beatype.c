/******************************************************************************

    函数名： beatype()
	语法： char beatype( int qrsposition, int init )
	描述： 利用RR间期将心拍分类
	       参考论文：
		   1、An arrhythmia classification system based on the
		      RR-interval signal
		   2、Arrhythmia classification using the RR-interval duration signal
	调用： 标准库中的abs（绝对值）函数     
	被调用：上层函数（主函数）
	输入参数： QRS复合波位置及是否初始化变量
	输出参数： 上一时刻QRS波的心拍类型
	返回值： 上一时刻QRS波的心拍类型
	         N（正常心拍）、V（室性早搏心拍）、A（房性早搏心拍）
	其他： 若init非0时，一些量赋初始值
	 
******************************************************************************/

#include <math.h>
#include "qrsdet.h"

char beatype( int qrsposition, int init ) 
{
	static int rr1 = 0, rr2 = 0, rr3 = 0; //RR间期
	static int qrsp1 = 0, qrsp2 = 0, qrsp3 = 0, qrsp4 = 0; //QRS波位置
	int condone, condtwo, condthree; //条件
	int condfour, condfive, condsix;
	char beatcategory; //心拍类型

	if (init)
	{
		rr1 = rr2 = rr3 = 0;
		qrsp1 = qrsp2 = qrsp3 = qrsp4 = 0;
	}

	qrsp1 = qrsp2;
	qrsp2 = qrsp3;
	qrsp3 = qrsp4;
	qrsp4 = qrsposition;
	
	if (qrsp1 && qrsp2 && qrsp3 && qrsp4)
	{
		rr1 = qrsp2 - qrsp1;
		rr2 = qrsp3 - qrsp2;
		rr3 = qrsp4 - qrsp3;
		condone = (rr2 <= (0.88 * rr1)) && ((0.95 * rr3) >= rr1);
		condtwo = (rr2 + rr3) < 2 * rr1;  
		condthree = (rr2 + rr3) >= 2 * rr1;

		condfour = (abs(rr2 - rr1) < MS200) && ((rr1 < MS750) 
			&& (rr2 < MS750)) && (rr3 > 0.6 * (rr1 + rr2));
		condfive = (abs(rr2 - rr3) < MS200) && ((rr3 < MS750)
			&& (rr2 < MS750)) && (rr1 > 0.6 * (rr3 + rr2));
		condsix = (rr2 < MS600) && ((1.8 * rr2 < rr1) || (1.8 * rr2 < rr3));

		//这里室性早搏优先级高于房性早搏
		beatcategory = 'N';
		if (condone && condthree)
		{
			beatcategory = 'V';
		}
		if (condone && condtwo)
		{
				beatcategory = 'A';
		}
		if (condfour || condfive || condsix)
		{
				beatcategory = 'V';
		}

	}
	else
	{
		beatcategory = 'Q';
	}

	return(beatcategory);
}
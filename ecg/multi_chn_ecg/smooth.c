/******************************************************************************

    函数名： smooth()
	语法： int smooth(FilterDataStruct *FilterDate, int datum)
	描述： 去高频噪声函数
	       差分方程：y(n) = b(1)*x(n) + b(2)*x(n-1) + ... + b(nb+1)*x(n-nb)
	                                  - a(2)*y(n-1) - ... - a(na+1)*y(n-na)
	参考论文： 主要基于以下论文Online digital filters for biological signals 
	                           some fast designs for a small computer
	调用： 无
	被调用：主函数或上层函数
	输入参数：滤波器结构体变量和待滤波数据值
	输出参数： 无
	返回值： 被滤波信号
	其他： 滤波器类型是FIR型
    注意：
	 
******************************************************************************/
#include <string.h>
#include "qrsdet.h"
#include "filter.h"
#include "qrsdet.h"


#include "smoothCof_250.h"


int smooth(FilterDataStruct *FilterDate, int datum)
{
	int fdatum = 0;//输出滤波值
	real tmp = 0;//中间量
	int loop = 0;//循环控制
	real subPrecision = 0;//提高滤波值用到的变量

	//滤波器y(n) = b(1)*x(n) + b(2)*x(n-1) + ... + b(nb+1)*x(n-nb)
	//                       - a(2)*y(n-1) - ... - a(na+1)*y(n-na)
	tmp = smB[0] * (real)datum;
	
	for (loop = 1; loop < smoothBL; ++loop)
	{
		tmp += smB[loop] * (real)FilterDate->smoothDataIn[loop - 1];
	}
	

	//中间值的替换
	memmove(&FilterDate->smoothDataIn[1],
		    &FilterDate->smoothDataIn, (smoothBL - 1) * sizeof(int));
	FilterDate->smoothDataIn[0] = datum;


	//在输出类型定下为int型时，适当提高输出精度
	fdatum = (int)(tmp);
	subPrecision = tmp - fdatum;
	if (subPrecision >= 0.5000)
	{
		fdatum = fdatum + 1;
	}
	if (subPrecision <= -0.5000)
	{
		fdatum = fdatum - 1;
	}

	return fdatum;
}
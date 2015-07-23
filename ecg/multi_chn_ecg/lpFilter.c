/******************************************************************************

    函数名： lpFilter()
	语法： int lpFilter(FilterDataStruct *FilterDate, int datum)
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
	其他： 滤波器类型是IIR型中一种，自定义为I型，由于250Hz时此滤波器滤得的结果
	       由于滤波器不稳定有问题，将其系数改为FIR型的，所以这里针对分母有一
		   判断条件，使得此滤波器也满足FIR类型滤波器
    注意：
	 
******************************************************************************/
#include <string.h>
#include "qrsdet.h"
#include "filter.h"


#include "lpFilterCof_250.h"


int lpFilter(FilterDataStruct *FilterDate, int datum)
{
	int fdatum = 0;//输出滤波值
	real tmp = 0;//中间量
	int loop = 0;//循环控制
	real subPrecision = 0;//提高滤波值用到的变量

	//滤波器y(n) = b(1)*x(n) + b(2)*x(n-1) + ... + b(nb+1)*x(n-nb)
	//                       - a(2)*y(n-1) - ... - a(na+1)*y(n-na)
	tmp = lpNum[0] * (real)datum;
	
	for (loop = 1; loop < lpNL; ++loop)
	{
		tmp += lpNum[loop] * (real)FilterDate->lpDataIn[loop - 1];
	}
	
	//中间值的替换
	memmove(&FilterDate->lpDataIn[1],
		    &FilterDate->lpDataIn, (lpNL - 1) * sizeof(int));
	FilterDate->lpDataIn[0] = datum;

	if (lpDL > 1)
	{
		for (loop = 1; loop < lpDL; ++loop)
		{
			tmp -= lpDen[loop] * FilterDate->lpDataOut[loop - 1];
		}
		memmove(&FilterDate->lpDataOut[1], 
			&FilterDate->lpDataOut, (lpDL - 1) * sizeof(real));
		FilterDate->lpDataOut[0] = tmp;
	}
	

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
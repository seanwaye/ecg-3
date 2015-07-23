/******************************************************************************

    文件名: filter.h
    描述： 此头文件声明滤波函数
	引用：
	被引用： 
	其他： 无
	函数列表： 无

******************************************************************************/
#ifndef filter_h__
#define filter_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "predefinefilter.h"

int hpFilter(FilterDataStruct *FilterDate, int datum);
int lpFilter(FilterDataStruct *FilterDate, int datum);
int smooth(FilterDataStruct *FilterDate, int datum);
int subpowerline(FilterDataStruct *FilterDate, int datum);

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif // filter_h__
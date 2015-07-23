/******************************************************************************

    文件名: fundecla.h
    描述： fundecla--function declare函数声明头文件，作用：使调用者只能看到
	       run.h中的函数，而不能看到程序中使用的其它函数
	引用： 1、mult_run.h
	被引用： mult_run.c
	其他： 无
	函数列表： 无

******************************************************************************/
#ifndef fundecla_h__
#define fundecla_h__

#ifdef __cplusplus
extern "C" {
#endif

#include "mult_run.h"
//QRS复合波查找所用函数
int multlpfilt( qrsFilterStruct* qrsFilterStr, int datum );
int multhpfilt( qrsFilterStruct* qrsFilterStr, int datum );
int multderiv1( qrsFilterStruct* qrsFilterStr, int datum );
int multderiv2( qrsFilterStruct* qrsFilterStr, int datum );
int multmvwint( qrsFilterStruct* qrsFilterStr, int datum );
int multqrsfilter( qrsFilterStruct* qrsFilterStr, int datum );
int multqrsdet( qrsDetStruct* qrsDetStr, int datum );

//精确位置查找相关函数及ST段函数
int accurloc( ChannelAnalysisStruct* ChanAnaStr, int delay );
//统计函数
int statfun( ChannelAnalysisStruct* ChanAnaStr, int lastqrsLOc );

#ifdef __cplusplus
}  /* extern "C" */
#endif
#endif // fundecla_h__
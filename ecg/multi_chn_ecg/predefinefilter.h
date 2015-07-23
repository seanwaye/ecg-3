/******************************************************************************

    文件名: predefinefilter.h
    描述： 此头文件定义滤波器中间值结构体
	引用：
	被引用：
	其他： 无
	函数列表： 无

******************************************************************************/
#ifndef predefinefilter_h__
#define predefinefilter_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "qrsdet.h"

#include "hpfilterorder_250.h"
#include "lpfilterorder_250.h"
#include "smoothorder_250.h"
#include "subpowerlineorder_250.h"


//去工频滤波器所用结构体声明
typedef struct
{
	//输入数据存储数组
	int dataIn[datainlen];
	//工频干扰量存储数组
	int interference[interferencelen];
	//dataIn中的循环下标，初始化为0
	int ptr;
	//与ctrVar的具体意义要见论文，初始化为0
	int jVar;
	//注意初始化为1
	int ctrVar;
	//评判标准（即论文中M）
	real crit;
}subPLStruct;

//滤波器整体所用结构体声明
typedef struct
{
	//高通滤波器的中间量
	int hpDataIn[hpNL];
	real hpDataOut[hpDL];
	//低通滤波器的中间量
	int lpDataIn[lpNL];
	real lpDataOut[lpDL];
	//平滑滤波器的中间量
	int smoothDataIn[smoothBL];
	//去工频结构体
	subPLStruct subPL;
}FilterDataStruct;


#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif // predefinefilter_h__


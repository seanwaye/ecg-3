/******************************************************************************

    文件名: smoothCof_250.h
	描述： 平滑滤波器阶数，这里用到的就是4点平滑，250Hz下转换到频域上相当于
	       -3dB截止频率为30Hz，第一处陷波频率为62.5Hz的低通滤波器
	其他： 滤波器类型是FIR型
	函数列表： 无

******************************************************************************/
#ifndef smoothCof_250_h__
#define smoothCof_250_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "qrsdet.h"
#include "smoothorder_250.h"

const real smB[smoothBL] = {0.250000000f,    0.250000000f,    
	                        0.250000000f,    0.250000000f};

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif // smoothCof_250_h__


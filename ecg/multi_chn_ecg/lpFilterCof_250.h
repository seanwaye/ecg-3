/******************************************************************************

    文件名: lpFilterCof_250.h
    描述： 低通滤波器滤波系数
	       针对250Hz，-3dB截止频率为30Hz设计,下降比较平缓
	其他： 由于设计为IIR型不稳定改设计为FIR型
	函数列表： 无

******************************************************************************/
#ifndef lpFilterCof_250_h__
#define lpFilterCof_250_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "qrsdet.h"
#include "lpfilterorder_250.h"

const real lpNum[lpNL] = {
   0.08333333582f,   0.25f,   0.3333333433f,   0.25f,  0.08333333582f
};

const real lpDen[lpDL] = {
	1
};


#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif // lpFilterCof_250_h__


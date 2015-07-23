/******************************************************************************

    文件名: hpFilterOrder_250.h
    描述： 高通滤波器滤波阶数
	       针对250Hz，-3dB截止频率为0.5Hz设计
	其他： 由于设计为IIR型不稳定改设计为FIR型
	函数列表： 无

******************************************************************************/
#ifndef hpfilterorder_250_h__
#define hpfilterorder_250_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define hpNL 250

#define hpDL 1

#ifdef __cplusplus
};
#endif /* __cplusplus */
#endif // hpfilterorder_250_h__


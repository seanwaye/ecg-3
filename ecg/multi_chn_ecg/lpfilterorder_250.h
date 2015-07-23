/******************************************************************************

    文件名: lpFilterOrder_250.h
    描述： 低通滤波器滤波阶数
	       针对250Hz，-3dB截止频率为30Hz设计,下降比较平缓
	其他： 由于设计为IIR型不稳定改设计为FIR型
	函数列表： 无

******************************************************************************/
#ifndef lpfilterorder_250_h__
#define lpfilterorder_250_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define  lpNL 5

#define lpDL 1

#ifdef __cplusplus
};
#endif /* __cplusplus */
#endif // lpfilterorder_250_h__



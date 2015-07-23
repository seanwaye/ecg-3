/******************************************************************************

    文件名: subpowerlineorder_250.h
    描述： 去工频噪声函数所需宏
	其他： 
	函数列表： 无
	参考论文： 主要基于以下两篇论文，论文机构一致
	           1、Subtraction of 50 Hz interference from the
	           electrocardiogram
	           2、New approach to the digital elimination of
	           50 Hz interference from the electrocardiogram
	
	其他： 这是一种非线性滤波器，与另外几种不同的地方只会有subpowerlineOrder.h
	       头文件不会有系数头文件，所以在.c文件中会引用阶数头文件，
	       在阶数头文件中定义的宏比另几种滤波器多，这些
	       宏主要针对的是50Hz工频干扰，目前不能去除60Hz工频干扰
	注意： 在不同的采样频率和工频干扰下定义的宏需要在相应文件下更改

******************************************************************************/
#ifndef subpowerlineorder_250_h__
#define subpowerlineorder_250_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//输入数据存储数组长度
#define datainlen 7
//工频干扰量存储数组长度
#define interferencelen 5
//一周期内采样点数（实际为采样频率除以工频干扰所得）
#define numOfCircle 5
//半周期内采样点数（实际为【采样频率除以工频干扰 - 1】/2）
#define halfNumOfCir 2
//在输入数据存储数组中所要用到的下标偏移值
#define realptrShift 2

#ifdef __cplusplus
};
#endif /* __cplusplus */
#endif // subpowerlineorder_250_h__





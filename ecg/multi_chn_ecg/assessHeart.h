#ifndef assessHeart_h__
#define assessHeart_h__

#include "run.h"

/*
将三个judge量的含义阐述如下：
heartJudge：1，心率正常
            2，心率稍缓
			3，心率稍快
			4，心率过缓
			5，心率过快
			6，此次检测无意义

typeJudge：1，未发现心律不齐
           2，疑似心律不齐
		   3，心律不齐可能性很大
		   4，心律不齐
		   6，此次检测无意义

detailtype:6，此次检测无意义
           7，未发现心律不齐
           8，出现房性早搏
           9，出现室性早搏
           10，窦性停搏
           11，成对室性早搏
           12，室性早搏三联律
           13，室性早搏二联律
           14，房性心动过速
           15，室性心动过速

stJudge：1，ST段电压正常
         2，ST段电压轻度抬高
		 3，ST段电压轻度压低
		 4，ST段电压抬高
		 5，ST段电压压低
		 6，此次检测无意义

将三个healthLev值的含义阐述如下：
healthLev：3，容易发病，请前往医院作进一步诊断
           4，较易发病，多运动，保持心情舒畅
		   5，不易发病，多运动，继续保持
		   6，信号质量不好，此次检测无意义，请贴好电极重新检测

*/
#ifdef __cplusplus
extern "C" {
#endif



int assessHeart(struct IndexHeart* heartIndex, struct AnalysisData* outStruct);


#ifdef __cplusplus
}  /* extern "C" */
#endif
#endif // assessHeart_h__
#ifndef qrsdet_h__
#define qrsdet_h__

/******************************************************************************

    文件名: qrsdet.h
    描述： 声明与采样频率相关的符号常量，转化为点数
	其他： 无
	函数列表： 无

******************************************************************************/


#define SAMPLE_RATE	250	/* Sample rate in Hz. */
#define MS_PER_SAMPLE	( (double) 1000/ (double) SAMPLE_RATE)
#define MS5	((int) (5/ MS_PER_SAMPLE + 0.5))
#define MS10	2//((int) (10/MS_PER_SAMPLE + 0.5))
#define MS15	((int) (15/MS_PER_SAMPLE + 0.5))
#define MS20	((int) (20/MS_PER_SAMPLE + 0.5))
#define MS25	6//((int) (25/MS_PER_SAMPLE + 0.5))
#define MS30	((int) (30/MS_PER_SAMPLE + 0.5))
#define MS35	((int) (35/MS_PER_SAMPLE + 0.5))
#define MS40	((int) (40/MS_PER_SAMPLE + 0.5))
#define MS50	((int) (50/MS_PER_SAMPLE + 0.5))
#define MS60	((int) (60/MS_PER_SAMPLE + 0.5))
#define MS70	((int) (70/MS_PER_SAMPLE + 0.5))
#define MS80	20//((int) (80/MS_PER_SAMPLE + 0.5))
#define MS90	((int) (90/MS_PER_SAMPLE + 0.5))
#define MS95	((int) (95/MS_PER_SAMPLE + 0.5))
#define MS100	25//((int) (100/MS_PER_SAMPLE + 0.5))
#define MS110	((int) (110/MS_PER_SAMPLE + 0.5))
#define MS120	((int) (120/MS_PER_SAMPLE + 0.5))
#define MS125	31//((int) (125/MS_PER_SAMPLE + 0.5))
#define MS135	((int) (135/MS_PER_SAMPLE + 0.5))
#define MS150	((int) (150/MS_PER_SAMPLE + 0.5))
#define MS160	((int) (160/MS_PER_SAMPLE + 0.5))
#define MS175	((int) (175/MS_PER_SAMPLE + 0.5))
#define MS195	((int) (195/MS_PER_SAMPLE + 0.5))
#define MS200	((int) (200/MS_PER_SAMPLE + 0.5))
#define MS220	55//((int) (220/MS_PER_SAMPLE + 0.5))
#define MS250	((int) (250/MS_PER_SAMPLE + 0.5))
#define MS280	70//((int) (280/MS_PER_SAMPLE + 0.5))
#define MS300	((int) (300/MS_PER_SAMPLE + 0.5))
#define MS360	((int) (360/MS_PER_SAMPLE + 0.5))
#define MS450	((int) (450/MS_PER_SAMPLE + 0.5))
#define MS600	((int) (600/MS_PER_SAMPLE + 0.5))
#define MS750	((int) (750/MS_PER_SAMPLE + 0.5))
#define MS1000	SAMPLE_RATE
#define MS1500	((int) (1500/MS_PER_SAMPLE))
#define MS1600	((int) (1600/MS_PER_SAMPLE))
#define MS1200	((int) (1200/MS_PER_SAMPLE))
#define MS2000	(2 * SAMPLE_RATE)
#define MS3000	(3 * SAMPLE_RATE)
#define MS2400	((int) (2400/MS_PER_SAMPLE))
#define MS4000	(4 * SAMPLE_RATE)
#define MS9000	(9 * SAMPLE_RATE)
#define MS20000	(20 * SAMPLE_RATE)
#define DERIV_LENGTH	MS10             /* 差分长度？ */
#define LPBUFFER_LGTH ((int) (2*MS25))   /* 低通长度？ */
#define HPBUFFER_LGTH MS125              /* 高通长度？ */

#define PRE_BLANK   MS250

#define WINDOW_WIDTH	MS80			// Moving window integration width.

#define	FILTER_DELAY (int) (((double) DERIV_LENGTH/2) + \
	((double) LPBUFFER_LGTH/2 - 1) + (((double) HPBUFFER_LGTH-1)/2) \
	+ PRE_BLANK) /* filter delays plus 200 ms blanking delay */

#define DER_DELAY	WINDOW_WIDTH + FILTER_DELAY + MS100 \
	/* 检测延迟  MS100是什么？ PRE_BLANK在QRSDET中有定义 */

#define  bufferlength  MS4000 //为查找各特征的缓存长度，之所以这么长
                              //因为最大延迟有极少数可达到MS1800

#define  displaylength  MS9000

typedef float real;

#endif // qrsdet_h__

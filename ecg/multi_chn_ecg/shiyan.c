#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "run.h"
#include "mult_run.h"

// 原始信号存放绝对路径，不同机器上运行时请修改
// ubuntu 14
#define ECG_PATH  "/home/xgx/mitdb/"
#define  lenArr 1
#define  channel 3
void main ()
{
	FILE *stream;
	FILE *values;
	int sampleNum, ecgOrgVal, ecgFltVal,beaType,heartRate,qrsPosition,qrsPositionOld,qrsPositionOldOld,qrsSetOn,qrsSetOff;
	char line[10];

	char filename[50] = {"/home/xgx/mitdb/vvannot201txt"};// 输出滤波文件名
	char adsname[100] = {'\0'};
	int m;
	m = 0;
    qrsPositionOld=0;
	struct AnalysisData dataOut;
	struct Ecgqueue ecgque;
	struct IndexHeart indexOut;
	int proArray[lenArr] = { 0 };
    int beatCnt=0;
//	TotalAnalysisStruct TotalStruct;
//	ChannelAnalysisStruct ChannelStruct[channel];
	InitVarStruct InitVarStr;
	int channelArray[channel] = { 0 };

	InitVarStr.precision[0] = 20;//ADC¾«¶È
	InitVarStr.precision[1] = 20;
	InitVarStr.precision[2] = 20;
	InitVarStr.algoChanArray[0] = 1;//µÚÒ»µ¼ÁªÅÜËã·¨
	InitVarStr.algoChanArray[1] = 0;
	InitVarStr.algoChanArray[2] = 0;



	memset(adsname, '\0', sizeof(adsname));
	strcat(adsname, ECG_PATH);
	strcat(adsname, "mit201.dat"); // 心电原始数据文件
	printf("%s\n",adsname);

	initvar(&ecgque, &dataOut, &indexOut);
// 初始化各滤波函数
    values = fopen(filename, "wt");
    fclose(values);
	// 整个if模块为读取心电信号文件，并在其中进行滤波
	if( (stream = fopen( adsname, "r" )) != NULL )
	{
        printf("open mit123.dat succesful\n");
		for(sampleNum = 0; !feof( stream ); )
		{
			if( fgets( line, 10, stream ) == NULL)
			{
			}
			else
			{
				sampleNum++;// 读取到一个有效心电采样点数据
				ecgOrgVal = atoi(line);//Z 将文本数据转换为int型
				run(&dataOut, &ecgOrgVal, lenArr, &ecgque);

				ecgFltVal = dataOut.filterEcgValue;
				qrsPosition = dataOut.qrsLoc;
				qrsSetOn = dataOut.qrsOnLoc;
				qrsSetOff = dataOut.qrsOffLoc;
				heartRate = dataOut.heartRate;
                beaType=0;

				if((qrsPosition!=qrsPositionOld)&&(sampleNum>108000)){
                    qrsPositionOld=qrsPosition;
                    if(dataOut.beaType=='N')
                        beaType=1;
                    if(dataOut.beaType=='V')
                        beaType=10;
                    if(dataOut.beaType=='O')
                        beaType=13;
                         // do not counter first point
                    if(beatCnt>0){
                    values = fopen(filename, "a+");
                    fprintf(values, "%d\t%d\n", qrsPosition,beaType);
                    fclose(values);
                    }

                    beatCnt++;
				}else{


				}

//			 	if(dataOut.beaType=='N')
//                   beaType=1;
//		 	if(dataOut.beaType=='V')
 //                   beaType=10;
//			 	if(dataOut.beaType=='O')
 //                   beaType=48;
 //               values = fopen(filename, "a+");
 //               fprintf(values, "%d,%d,%d,%d,%d\n", ecgFltVal,qrsPosition,qrsSetOn,qrsSetOff,beaType);
//                fclose(values);

				// 输出心电滤波数据为文件，输出路径为当前源代码存放路径

			}
		}
		fclose( stream );

	}else{
        printf("can not open mit107.dat\n");
        return 0;
	}
//	printf("%d\n",sampleNum);
    printf("Beat counter is: %d\n",beatCnt-1);
	system("pause");
	return(0);
}

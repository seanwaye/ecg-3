#ifndef input_h__
#define input_h__

#define MITDB0
//#define MITDB1

#ifdef MITDB0

//以下与函数6和8对应，利用数据库信号文件和qrsdet函数，寻找QRS发生时间
//文件是数据库信号文件
char Records[][20] = {"m1000","m1010","m1020","m1030","m1040","m1050","m1060",
	                  "m1070","m1080","m1090","m1110","m1120","m1130","m1140",
					  "m1150","m1160","m1170","m1180","m1190","m1210","m1220",
					  "m1230","m1240","m2000","m2010","m2020","m2030","m2050",
	                  "m2070","m2080","m2090","m2100","m2120","m2130","m2140",
					  "m2150","m2170","m2190","m2200","m2210","m2220","m2230",
					  "m2280","m2300","m2310","m2320","m2330","m2340"};//
#define REC_COUNT	1

//
//以下与函数7对应，利用数据库标注时间和beatype函数，对心拍分类
//文件是数据库QRS波标注时间
//char Records[][20] ={"m100qrs","m101qrs","m102qrs","m103qrs","m104qrs",
//	                 "m105qrs","m106qrs","m107qrs","m108qrs","m109qrs",
//					 "m111qrs","m112qrs","m113qrs","m114qrs","m115qrs",
//					 "m116qrs","m117qrs","m118qrs","m119qrs","m121qrs",
//					 "m122qrs","m123qrs","m124qrs","m200qrs","m201qrs",
//					 "m202qrs","m203qrs","m205qrs","m207qrs","m208qrs",
//					 "m209qrs","m210qrs","m212qrs","m213qrs","m214qrs",
//					 "m215qrs","m217qrs","m219qrs","m220qrs","m221qrs",
//					 "m222qrs","m223qrs","m228qrs","m230qrs","m231qrs",
//					 "m232qrs","m233qrs","m234qrs"};
//#define REC_COUNT	23

#define ECG_PATH  "E:\\mit\\mitdb\\txt\\"

#endif

#ifdef MITDB1
char Records[][20] = {"m1001","m1011","m1021","m1031","m1041","m1051","m1061",
	                  "m1071","m1081","m1091","m1111","m1121","m1131","m1141",
					  "m1151","m1161","m1171","m1181","m1191","m1211","m1221",
					  "m1231","m1241","m2001","m2011","m2021","m2031","m2051",
					  "m2071","m2081","m2091","m2101","m2121","m2131","m2141",
					  "m2151","m2171","m2191","m2201","m2211","m2221","m2231",
					  "m2281","m2301","m2311","m2321","m2331","m2341"};
#define REC_COUNT	48
#define ECG_PATH  "E:\\mit\\mitdb\\txt\\"

#endif

#endif // input_h__

#ifndef detect_h__
#define detect_h__

int sub_dc_offset_250(int datum, int init);
int sub_hf_250(int datum, int init);
int sub_hf_250_fir_1(int datum, int init);
int sub_hf_250_fir_2(int datum, int init);
int subpowerline250(int datum, int init);
int sub_dc_offset_500(int datum, int init);
int subpowerline500(int datum, int init);
int sub_hf_500(int datum, int init);
int qrsdet( int datum, int init );
int smooth_5_points(int datum, int init);
int qrsandonoffset(int *buffer, int delay, int bufferptr,int samplepoint, 
	int *lastSample, int *lastOffshift, int *thiOffshift, int *qrsOnset, 
	int *qrsOffset, int *stDif, int *pbuffer, int *outecgbuf, int *bufLength);
int arrhyalgone( int qrsposition, int qrswide, 
	                                 char *type, int *rhythm, int init);
#endif // detect_h__

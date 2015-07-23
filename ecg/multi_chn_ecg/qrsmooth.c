/******************************************************************************

    函数名： qrsonset()
	语法： int qrsmooth(int *buf)
	描述： 采用5点平滑方法,数组长度是固定的MS280
	参考论文： 与matlab中smooth中的moving方法类似
	调用： 无
	被调用：qrsandonoffset()函数
	输入参数： 在qrsandonoffset（）函数中的qrsbuffer
	输出参数： 滤波后的qrsbuffer
	返回值： 0表示正常，暂时未对异常判断，如判断指针是否为空，空间大小
	         是否足够等等后面添加
	其他： 由于数组操作和常值（const int）的困难及想简便快速完成这个功能，
	       所以此函数只适用于这种情况，不能满足其他情况
	 
******************************************************************************/

#include "qrsdet.h"

int qrsmooth(int *buf)
{
	int tmp_buf[MS280] = {0};
	int liv_loop = 2;
	for (; liv_loop < MS280 - 2; ++liv_loop)
	{
		tmp_buf[liv_loop] = (buf[liv_loop - 2] + buf[liv_loop -1] 
		          + buf[liv_loop] + buf[liv_loop + 1] + buf[liv_loop + 2]) / 5;
	}
	tmp_buf[0] = buf[0];
	tmp_buf[1] = buf[1];
	tmp_buf[liv_loop] = buf[liv_loop];
	++liv_loop;
	tmp_buf[liv_loop] = buf[liv_loop];
	for (liv_loop = 0; liv_loop < MS280; ++liv_loop)
	{
		buf[liv_loop]= tmp_buf[liv_loop];
	}

	return(0);
}
/******************************************************************************

    函数名： sub_dc_offset_250()
	语法： int sub_dc_offset_250(int datum, int init)
	描述： 去工频噪声函数
	       差分方程：Y(n) = Y(n-1) + （249 / 250） * X(n) - X(n-1) + 
		   （1 / 250） * X(n-250)
	参考论文： 主要基于以下论文Digital Filters for Real-Time ECG Signal 
               Processing Using Microprocessors.
	调用： 无
	被调用：主函数或上层函数
	输入参数：待滤波信号及初始化标志量
	输出参数： 无
	返回值： 被滤波信号
	其他： 找到了一直与matlab版本不一致的原因：matlab输出与中间量是浮点型，
	       而在次均为int型，至于选何种类型还要考虑，看后续运用
    注意： 两个注意问题
	       1、此函数不像matlab版本一样，可以处理任意采样率的信号，
		   这里只能处理采样率为250Hz的信号
		   针对ADS1298-FE低速率下采集的信号,此处的截止频率为0.5Hz
		   2、输入输出量类型问题，中间量肯定为浮点类型，但输入与输出量的类型
		   还需与前后程序顺序综合考虑
	 
******************************************************************************/

int sub_dc_offset_250(int datum, int init)
{
	static double lastFdatum;//即Y(n-1)
	static int data[251], ptr;
	double fdatum = 0;//“输出量”
	int intfdatum = 0;//“输出量”
	double subPrecision = 0;//提高精度的中间量
	int ptrMinusOne = 0, ptrMinusOrder = 0;//坐标
	double medineOne = 0, medineTwo = 0;
	//初始化
	if (init)
	{
		lastFdatum = 0;
		for (ptr = 0; ptr < 251; ++ptr)
		{
			data[ptr] = 0;
		}
		ptr = 0;
		return 0;
	}

	data[ptr] = datum;
	ptrMinusOne = ptr - 1;
	ptrMinusOrder = ptr - 250;
	if (ptrMinusOne < 0)
	{
		ptrMinusOne = ptrMinusOne + 251;
	}
	if (ptrMinusOrder < 0)
	{
		ptrMinusOrder = ptrMinusOrder + 251;
	}

	medineOne = (249  * (double)data[ptr] / 250);
	medineTwo = (1  * (double)data[ptrMinusOrder] / 250);
	fdatum = lastFdatum + medineOne
	         - data[ptrMinusOne] + medineTwo;//差分方程
	lastFdatum = fdatum;

	++ptr;
	if (ptr == 251)
	{
		ptr = 0;
	}

	intfdatum = (int)(fdatum);
	//在输出类型定下为int型时，适当提高输出精度
	subPrecision = fdatum - intfdatum;
	if (subPrecision >= 0.5000)
	{
		intfdatum = intfdatum + 1;
	}
	if (subPrecision <= -0.5000)
	{
		intfdatum = intfdatum - 1;
	}
	return intfdatum;
}
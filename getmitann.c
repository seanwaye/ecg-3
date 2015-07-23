#include "stdio.h"
#include "wfdb/wfdb.h"
main()
{
    int i,beatyp,tim;
    WFDB_Siginfo s[2];
    WFDB_Sample v[2];
	WFDB_Anninfo a;
	WFDB_Annotation annot;
	FILE *values;
	long cnt=0,nncnt=0;
	beatyp=0;
    values = fopen("/home/xgx/mit105.dat", "w+");
    if(values==NULL){
		printf("can not create mit105.dat\n");
		exit(0);
	}
	fclose(values);
	
	a.name ="atr";
	a.stat =WFDB_READ;
    if(isigopen("100",s,2)<2)
        exit(1);

    for (i=0;i<650000;i++){
        if(getvec(v)<0)
            break;
        values = fopen("/home/xgx/mit105.dat", "a+");
        if(annopen("100",&a,1)<0)
			exit(1);
		while (getann(0,&annot)==0){
			tim=(int)annot.time;
			if(tim==i){
	//			printf("qrs position %d\n",(int)annot.time);
				beatyp=(int)annot.anntyp;
				break;
			}
		}

		fprintf(values, "%d\n", v[0]);
        fclose(values);
		beatyp=0;
	}
	if(annopen("100",&a,1)<0)
		exit(1);
	printf("find counter!\n");
	while (getann(0,&annot)==0){
		if(annot.time>108000){
			cnt++;
			if(annot.anntyp==1)
			nncnt++;
		}
		printf("%ld %s\n",annot.time,annstr(annot.anntyp));
	}
	printf("QRS counter is : %ld\n",cnt);
	printf("N beat counter is : %ld\n",nncnt);
	exit(0);

}

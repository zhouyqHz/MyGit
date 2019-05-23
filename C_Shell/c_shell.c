#include<stdio.h>	
#include<stdlib.h>  //c语言调用shell命令头文件所需,int转char需要
#include <string.h> //字符串拼接头文件
#include <time.h> 	//时间相关头文件(sleep)
#include <unistd.h> //socket close函数所在头文件 以及sleep所需（树莓派跟linux有点不一样）
//摄像的shell头："raspivid -o 2019523_1.h264 -t 10000 -w 640 -h 480"
//把h264转成MP4 “MP4Box -fps 25 -add 2019523_1.h264 2019523_1.mp4”
char Take_Video_Head[] = "raspivid -o ";
char Take_Video[100] = "";
char Transform_Video_Head[] = "MP4Box -fps 20 -add ";
char Transform_Video[100] = "";
//时间记录数组
char Mysql_Num_Data[50] = "";
char Time_buf[50] = "";
int  Time_Len = 0;
int  Time_Count;
//主函数
int main()
{
	//获取当前时间
	time_t now;			//实例化time结构
	struct tm *timenow; //实例化tm结构指针
	time(&now);
	timenow = localtime(&now);//把国际标准时间转换成本地时间
	//把英文日期转换成数字的日期
	strftime(Time_buf, sizeof(Time_buf), "%F %P %X", timenow);
	Time_buf[49] = '\0';
	Time_Len = strlen(Time_buf);
	printf("buf:%s\n", Time_buf);
	printf("len:%d\n", Time_Len);
	//把空格 换成 -
	for(Time_Count=0; Time_Count<=Time_Len; Time_Count++)
	{
		if(Time_buf[Time_Count]==0x20||Time_buf[Time_Count]==0x3a)
			Time_buf[Time_Count] = 0x2d;
	}
	printf("buf:%s\n", Time_buf);
	//asctime返回char* strcpy实现把char*复制给字符数组
	//strcpy(Mysql_Num_Data, asctime(timenow));
	//拼接文件名字符串
	stpcpy(Take_Video, Take_Video_Head);
	strcat(Take_Video, Time_buf);
	strcat(Take_Video, ".h264 -t 10000 -w 500 -h 330");
	//字符数组结束符，把字符数组变成字符串
	Take_Video[99] ='\0';
	printf("Take_Video:%s\n",Take_Video);
	//拼接mp4名字符串
	stpcpy(Transform_Video, Transform_Video_Head);
	strcat(Transform_Video, Time_buf);
	strcat(Transform_Video, ".h264 ");
	strcat(Transform_Video, Time_buf);
	strcat(Transform_Video, ".mp4");
	//字符数组结束符，把字符数组变成字符串
	Transform_Video[99] ='\0';	
	printf("Take video Begin, Waiting 10s\n");
	//调用shell命令,生成h.264文件
	system(Take_Video);
	printf("Transform_Video:%s\n",Transform_Video);
	//sleep(15);
	//调用生成MP4的shell命令
	system(Transform_Video);
}
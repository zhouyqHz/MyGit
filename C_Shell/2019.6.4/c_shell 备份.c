#include <stdio.h>	//
#include <stdlib.h>  //c语言调用shell命令头文件所需,int转char需要
#include <string.h> //字符串拼接头文件
#include <time.h> 	//时间相关头文件(sleep)
#include <unistd.h> //socket close函数所在头文件 以及sleep所需（树莓派跟linux有点不一样）
#include <sys/socket.h> 	//建立socket相关头文件
#include <netinet/in.h> 	//地址struct相关头文件
#include <netinet/ip.h>		//不知道干嘛的
#include <arpa/inet.h>		//不知道干嘛的
#include <sys/types.h>		//不知道干嘛的
#include <fcntl.h>          //文件读写相关库
#include <wiringPi.h>       //gpio读写库
//摄像的shell头："raspivid -o 2019523_1.h264 -t 10000 -w 640 -h 480"
//把h264转成MP4 “MP4Box -fps 25 -add 2019523_1.h264 2019523_1.mp4”
char Take_Video_Head[] = "raspivid -o ";
char Take_Video[100] = "";
char Transform_Video_Head[] = "MP4Box -fps 20 -add ";
char Transform_Video[100] = "";
//时间处理相关变量
char Mysql_Num_Data[50] = "";
char Time_buf[50] = "";
int  Time_Len = 0;
int  Time_Count;
//socket相关变量
int socketfd = 0;
void Socket_Inil();
//主函数
int main()
{
	//gpio初始化
	
	wiringPiSetup () ; 
	pinMode (29, OUTPUT) ; 
	//digitalWrite(21, HIGH);
	for (;;) // 无限循环
    {
       digitalWrite (29, HIGH);  // GPIO.0 输出高
       delay (500);  // 延时500毫秒
       digitalWrite (29, LOW); // GPIO.0 输出低
       delay (500); 
	   printf("Gpio\n");
    }
	
	
	Socket_Inil();
	//获取当前时间
	time_t now;			//实例化time结构
	struct tm *timenow; //实例化tm结构指针
	time(&now);
	timenow = localtime(&now);//把国际标准时间转换成本地时间
	//把英文日期转换成数字的日期
	strftime(Time_buf, sizeof(Time_buf), "%F %P %X", timenow);
	Time_buf[49] = '\0';
	Time_Len = strlen(Time_buf);
	//printf("buf:%s\n", Time_buf);
	//printf("len:%d\n", Time_Len);
	//把空格和：换成 - ，打包MP4的命令 不能有乱七八糟的符号
	for(Time_Count=0; Time_Count<=Time_Len; Time_Count++)
	{
		if(Time_buf[Time_Count]==0x20||Time_buf[Time_Count]==0x3a)
			Time_buf[Time_Count] = 0x2d;
	}
	printf("buf:%s\n", Time_buf);
	//asctime返回char* strcpy实现把char*复制给字符数组
	//strcpy(Mysql_Num_Data, asctime(timenow));
	//拼接h.264文件名字符串
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
	strcat(Transform_Video, ".mp4 ");
	//字符数组结束符，把字符数组变成字符串
	Transform_Video[99] ='\0';	
	printf("Take video Begin, Waiting 10s\n");
	//调用shell命令,生成h.264文件（会等待生成完毕才会往下走）
	//system(Take_Video);
	printf("Transform_Video:%s\n",Transform_Video);
	//调用生成MP4的shell命令
	//system(Transform_Video);
	//打开文件，变成文件流的形式
	/*FILE* pf = fopen("/home/pi/Desktop/Take_Video/2019-05-23-pm-16-12-26.mp4", "rb");
	if(pf == NULL) 
	{  
		printf("open file failed!\n");  
    }
	else
		printf("open file succeed!\n"); 
	//定义发送缓存大小，512KB
	char pack[1024*512] = {'\0'};
	int len;
	int count = 0;
	//把文件流通过write函数发掉
	while((len = fread(pack, sizeof(char), 1024*512, pf)) > 0)  
	{  
		//system("clear");  
		//printf("send data size = %d \t", len);  
		write(socketfd, pack, len);  
		//清空缓存区
		bzero(pack,1024*512);  
		count++;
	}
	//打印发文件流的次数
	printf("count= %d \n", count); 
	count = 0;*/
	while(1);
}

void Socket_Inil()
{
	//创建一个服务端地址结构体
	struct sockaddr_in Sever_Add;
	//创建一个socket
	socketfd = socket(AF_INET,SOCK_STREAM,0);
	//补充服务端地址结构体信息
	Sever_Add.sin_family = AF_INET;
    Sever_Add.sin_port = htons(6280);
    Sever_Add.sin_addr.s_addr=inet_addr("47.100.43.231");
    bzero(&(Sever_Add.sin_zero), 8);
	//创建与服务端的连接
	connect(socketfd,(struct sockaddr*)&Sever_Add,sizeof(struct sockaddr));
}
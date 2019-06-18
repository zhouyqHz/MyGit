//本工程的接收缓冲（1024）与发送缓冲（1024*512）是分开的
#include <stdio.h>			//c标准库
#include <stdlib.h>  		//c语言调用shell命令头文件所需,int转char需要
#include <string.h> 		//字符串拼接头文件
#include <time.h> 			//时间相关头文件(sleep)
#include <unistd.h> 		//socket close函数所在头文件 以及sleep所需（树莓派跟linux有点不一样）
#include <sys/socket.h> 	//建立socket相关头文件
#include <netinet/in.h> 	//地址struct相关头文件
#include <netinet/ip.h>		//不知道干嘛的
#include <arpa/inet.h>		//不知道干嘛的
#include <fcntl.h>          //文件读写相关库
#include <wiringPi.h>       //gpio读写库
#include <pthread.h>		//多线程头文件（监听人体，与网络连接为不同的线程）
#include <sys/types.h>		//遍历目录需要用到的头文件
#include <dirent.h>         //读取文件列表的头文件
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
int  socketfd = 0;
char Receive_Buff[1024];//接收缓冲数组
char Send_Buff[1024];//发送缓冲数组
int  Receive_Len = 0;   //接收长度
//常函数声明
void Socket_Inil();
void Gpio_Inil();
void Take_And_Transform_Video();
int  readFileList(char *basePath);
void Read_And_Send_Video(char *File_name);
//线程函数声明
pthread_t id;
void *Socket_Connect(void *arg);
pthread_t id_Human;
void *Human_Detection();
//********************************主函数
int main()
{	
	//用shell命令校准时间，非常重要
	system("sudo ntpd -s -d");
	//socket客户端初始化
	Socket_Inil();	
	pthread_create(&id, NULL, Socket_Connect, &socketfd);
	pthread_create(&id_Human, NULL, Human_Detection, NULL);
	//读取目录 
	//readFileList("/home/pi/Desktop/Take_Video/");
		
	while(1);
}
//********常函数实现
void Gpio_Inil()
{
	//gpio初始化（之前一直弄不出来，端口号不对）	
	wiringPiSetup ();
	//gpio设置，29.28号设置成输入
	pinMode (29, INPUT); 
	pinMode (28, INPUT); 
	printf("Gpio_Inil ok\n");
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
    Sever_Add.sin_addr.s_addr=inet_addr("106.14.34.209");
    bzero(&(Sever_Add.sin_zero), 8);
	//创建与服务端的连接,成功返回0
	if(connect(socketfd,(struct sockaddr*)&Sever_Add,sizeof(struct sockaddr))==0)
	{
		printf("Socket_Inil ok\n");
		//发送FS，更新本端点的socketfd
		send(socketfd, "FS", 2, 0);
	}
	else 
	{
		printf("Socket_Connect Error\n");
		exit(0);
	}
}
//这个由28号gpio控制
void Take_And_Transform_Video()
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
	//-t 表示时间，（10000表示10s） -w表示分辨率
	strcat(Take_Video, ".h264 -t 10000 -w 640 -h 480");
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
	system(Take_Video);
	printf("Transform_Video:%s\n",Transform_Video);
	//调用生成MP4的shell命令
	system(Transform_Video);
}
void Read_And_Send_Video(char *File_name)
{
	char File_Name_All[100] = "";
	//拼接出要打开的文件
	stpcpy(File_Name_All, "/home/pi/Desktop/Take_Video/");
	strcat(File_Name_All, File_name);
	File_Name_All[99] = '\0';
	printf("File_Name_All:%s\n",File_Name_All);
	//打开文件，变成文件流的形式
	//FILE* pf = fopen("/home/pi/Desktop/Take_Video/2019-05-27-pm-16-18-43.mp4", "rb");
	//打开文件
	FILE* pf = fopen(File_Name_All, "rb");
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
		system("clear");  
		//printf("send data size = %d \t", len);  
		//将本次缓冲数据发送出去
		write(socketfd, pack, len);  
		//清空缓存区
		bzero(pack,1024*512);  
		count++;
	}
	//打印发文件流的次数
	printf("count= %d \n", count); 
	//发送结束文件发送的指令(不知道为什么没发过去)
	write(socketfd, "VEND", 4);
	count = 0;
}
//获取指定目录下的所有文件名字(这里要更新目录，则需要统计)
int readFileList(char *basePath)
 {
    DIR *dir;	//声明一个句柄（不太懂是什么）
    struct dirent *ptr; //保存readdir 返回的数据
    char base[1000];
	int FN_Num = 0;  //一定要初始化初始化
	char FN_Num_Char[5] = "";
    if ((dir=opendir(basePath)) == NULL)
    {
		perror("Open dir error...");
    }
    while ((ptr=readdir(dir)) != NULL)
	{
		if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
			continue;
		else if(ptr->d_type == 8)    //file（8就是文件的标示，本工程只找文件）
		{
			bzero(Send_Buff, sizeof(Send_Buff));
			//把int转成char
			sprintf(FN_Num_Char, "%d", FN_Num);
			//打印文件名
			printf("File_Name:%s\n",ptr->d_name);
			stpcpy(Send_Buff, "FN");//把FN拼进去
			//小于10，则拼个0进去保持位数
			if(FN_Num<10)
				strcat(Send_Buff, "0");
			strcat(Send_Buff, FN_Num_Char);
			strcat(Send_Buff, ptr->d_name);
			Send_Buff[1024] = '\0';
			//小于0 则出错。。为什么write不能发送，只能用send发送
			//不是正确的文件，不发送
			if(Send_Buff[4]=='2'&&Send_Buff[27]!='h')
			{
				send(socketfd, Send_Buff, sizeof(Send_Buff), 0);
				printf("%s\n", Send_Buff);
				//只有当服务端有非空消息返回的时候才会继续发
				while(recv(socketfd, Receive_Buff, sizeof(Receive_Buff), 0)<=0);
				//表示文件个数
				FN_Num++;
			}
		}
		bzero(FN_Num_Char, sizeof(FN_Num_Char));
	}
	closedir(dir);
	return 1;
 }
//*****下面是线程函数实现
//socket传输线程,void *arg的意思是，有传入参数
void *Socket_Connect(void *arg)
{
	//socketfd传入线程
	int socketfd = *(int*)arg;
	while(1)	
	{
		//printf("Client_Receive_Ready\n");
		//本函数会堵塞
		Receive_Len = recv(socketfd, Receive_Buff, sizeof(Receive_Buff), 0);
		//这里在接受时做了成功与否的判断，最好在前面的发送那边也要做判断，exit(0);什么意思不太知道
		if(Receive_Len<=0)
		{
			printf("Disconnect With Server\n");
			exit(0);
		}
		//数据有效性验证
		else if(Receive_Len>0)
		{
			//增加结束位，转变成字符串
			Receive_Buff[Receive_Len]  = '\0';
			//printf("Received_Message : %s \n",Receive_Buff);
			//发送文件名
			if(Receive_Buff[0]=='F'&&Receive_Buff[1]=='N')
			{
				//读取目录_并通过socket发送出去(返回一个非空，才会继续发下一个)
				readFileList("/home/pi/Desktop/Take_Video/");//本函数也会堵塞
				//发送结束，防止服务端继续发东西过来,要用这个指令结束本次数据更新
				send(socketfd, "END", 3, 0);
				printf("Send Video_Nme Succeed\n");
			}
			//服务器发来要上传文件的命令
			else if(Receive_Buff[0]=='W'&&Receive_Buff[1]=='U')
			{
				char Socket_File_Name[50] = "";
				strncpy(Socket_File_Name, Receive_Buff+2,26);
				Socket_File_Name[49] = '\0';
				printf("Socket_File_Name:%s\n",Socket_File_Name);
				//读取以及发送文件
				Read_And_Send_Video(Socket_File_Name);
			}
		}
	}
	
}
//人体检测，录像线程
//在这个线程中，拍视屏与视频转换的功能都会堵塞，所以，在拍完以及转换完之前都会堵塞着
void *Human_Detection()
{
	Gpio_Inil();
	while(1)
	{
		//28号GPIO默认为0
		if(digitalRead (28) == 1)
		{
			delay (500);  //延时500ms
			if(digitalRead (28) == 1)
			{
				Take_And_Transform_Video();//本函数的shell命令会造成堵本线程塞
				printf("Take_And_Transform_Video Succeed\n");
			}
		}
	}
}
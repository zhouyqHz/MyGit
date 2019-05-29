#include <stdio.h>			//c��׼��
#include <stdlib.h>  		//c���Ե���shell����ͷ�ļ�����,intתchar��Ҫ
#include <string.h> 		//�ַ���ƴ��ͷ�ļ�
#include <time.h> 			//ʱ�����ͷ�ļ�(sleep)
#include <unistd.h> 		//socket close��������ͷ�ļ� �Լ�sleep���裨��ݮ�ɸ�linux�е㲻һ����
#include <sys/socket.h> 	//����socket���ͷ�ļ�
#include <netinet/in.h> 	//��ַstruct���ͷ�ļ�
#include <netinet/ip.h>		//��֪�������
#include <arpa/inet.h>		//��֪�������
#include <fcntl.h>          //�ļ���д��ؿ�
#include <wiringPi.h>       //gpio��д��
#include <pthread.h>		//���߳�ͷ�ļ����������壬����������Ϊ��ͬ���̣߳�
#include <sys/types.h>		//����Ŀ¼��Ҫ�õ���ͷ�ļ�
#include <dirent.h>         //��ȡ�ļ��б��ͷ�ļ�
//�����shellͷ��"raspivid -o 2019523_1.h264 -t 10000 -w 640 -h 480"
//��h264ת��MP4 ��MP4Box -fps 25 -add 2019523_1.h264 2019523_1.mp4��
char Take_Video_Head[] = "raspivid -o ";
char Take_Video[100] = "";
char Transform_Video_Head[] = "MP4Box -fps 20 -add ";
char Transform_Video[100] = "";
//ʱ�䴦����ر���
char Mysql_Num_Data[50] = "";
char Time_buf[50] = "";
int  Time_Len = 0;
int  Time_Count;
//socket��ر���
int  socketfd = 0;
char Receive_Buff[1024];//���ջ�������
char Send_Buff[1024];//���ջ�������
int  Receive_Len = 0;   //���ճ���
//����������
void Socket_Inil();
void Gpio_Inil();
void Take_And_Transform_Video();
int  readFileList(char *basePath);
void Read_And_Send_Video();
//�̺߳�������
pthread_t id;
void *Socket_Connect(void *arg);
pthread_t id_Human;
void *Human_Detection();
//********************************������
int main()
{	
	//socket�ͻ��˳�ʼ��
	Socket_Inil();	
	pthread_create(&id, NULL, Socket_Connect, &socketfd);
	pthread_create(&id_Human, NULL, Human_Detection, NULL);
	//��ȡĿ¼ 
	//readFileList("/home/pi/Desktop/Take_Video/");
		
	while(1);
}
//********������ʵ��
void Gpio_Inil()
{
	//gpio��ʼ����֮ǰһֱŪ���������˿ںŲ��ԣ�	
	wiringPiSetup ();
	//gpio���ã�29.28�����ó�����
	pinMode (29, INPUT); 
	pinMode (28, INPUT); 
	printf("Gpio_Inil ok\n");
}

void Socket_Inil()
{
	//����һ������˵�ַ�ṹ��
	struct sockaddr_in Sever_Add;
	//����һ��socket
	socketfd = socket(AF_INET,SOCK_STREAM,0);
	//�������˵�ַ�ṹ����Ϣ
	Sever_Add.sin_family = AF_INET;
    Sever_Add.sin_port = htons(6280);
    Sever_Add.sin_addr.s_addr=inet_addr("192.168.1.161");
    bzero(&(Sever_Add.sin_zero), 8);
	//���������˵�����
	connect(socketfd,(struct sockaddr*)&Sever_Add,sizeof(struct sockaddr));
	printf("Socket_Inil ok\n");
}
//�����28��gpio����
void Take_And_Transform_Video()
{
	//��ȡ��ǰʱ��
	time_t now;			//ʵ����time�ṹ
	struct tm *timenow; //ʵ����tm�ṹָ��
	time(&now);
	timenow = localtime(&now);//�ѹ��ʱ�׼ʱ��ת���ɱ���ʱ��
	//��Ӣ������ת�������ֵ�����
	strftime(Time_buf, sizeof(Time_buf), "%F %P %X", timenow);
	Time_buf[49] = '\0';
	Time_Len = strlen(Time_buf);
	//printf("buf:%s\n", Time_buf);
	//printf("len:%d\n", Time_Len);
	//�ѿո�ͣ����� - �����MP4������ ���������߰���ķ���
	for(Time_Count=0; Time_Count<=Time_Len; Time_Count++)
	{
		if(Time_buf[Time_Count]==0x20||Time_buf[Time_Count]==0x3a)
			Time_buf[Time_Count] = 0x2d;
	}
	printf("buf:%s\n", Time_buf);
	//asctime����char* strcpyʵ�ְ�char*���Ƹ��ַ�����
	//strcpy(Mysql_Num_Data, asctime(timenow));
	//ƴ��h.264�ļ����ַ���
	stpcpy(Take_Video, Take_Video_Head);
	strcat(Take_Video, Time_buf);
	//-t ��ʾʱ�䣬��10000��ʾ10s�� -w��ʾ�ֱ���
	strcat(Take_Video, ".h264 -t 10000 -w 640 -h 480");
	//�ַ���������������ַ��������ַ���
	Take_Video[99] ='\0';
	printf("Take_Video:%s\n",Take_Video);
	//ƴ��mp4���ַ���
	stpcpy(Transform_Video, Transform_Video_Head);
	strcat(Transform_Video, Time_buf);
	strcat(Transform_Video, ".h264 ");
	strcat(Transform_Video, Time_buf);
	strcat(Transform_Video, ".mp4 ");
	//�ַ���������������ַ��������ַ���
	Transform_Video[99] ='\0';	
	printf("Take video Begin, Waiting 10s\n");
	//����shell����,����h.264�ļ�����ȴ�������ϲŻ������ߣ�
	system(Take_Video);
	printf("Transform_Video:%s\n",Transform_Video);
	//��������MP4��shell����
	system(Transform_Video);
}
void Read_And_Send_Video()
{
	//���ļ�������ļ�������ʽ
	FILE* pf = fopen("/home/pi/Desktop/Take_Video/2019-05-27-pm-16-18-43.mp4", "rb");
	if(pf == NULL) 
	{  
		printf("open file failed!\n");  
    }
	else
		printf("open file succeed!\n"); 
	//���巢�ͻ����С��512KB
	char pack[1024*512] = {'\0'};
	int len;
	int count = 0;
	//���ļ���ͨ��write��������
	while((len = fread(pack, sizeof(char), 1024*512, pf)) > 0)  
	{  
		//system("clear");  
		//printf("send data size = %d \t", len);  
		write(socketfd, pack, len);  
		//��ջ�����
		bzero(pack,1024*512);  
		count++;
	}
	//��ӡ���ļ����Ĵ���
	printf("count= %d \n", count); 
	count = 0;
}
//��ȡָ��Ŀ¼�µ������ļ�����(����Ҫ����Ŀ¼������Ҫͳ��)
int readFileList(char *basePath)
 {
    DIR *dir;	//����һ���������̫����ʲô��
    struct dirent *ptr; //����readdir ���ص�����
    char base[1000];
	int FN_Num = 0;  //һ��Ҫ��ʼ����ʼ��
	char FN_Num_Char[5] = "";
    if ((dir=opendir(basePath)) == NULL)
    {
		perror("Open dir error...");
    }
    while ((ptr=readdir(dir)) != NULL)
	{
		if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
			continue;
		else if(ptr->d_type == 8)    //file��8�����ļ��ı�ʾ��������ֻ���ļ���
		{
			bzero(Send_Buff, sizeof(Send_Buff));
			sprintf(FN_Num_Char, "%d", FN_Num);
			//��ӡ�ļ���
			printf("File_Name:%s\n",ptr->d_name);
			stpcpy(Send_Buff, "FN");//��FNƴ��ȥ
			//С��10����ƴ��0��ȥ����λ��
			if(FN_Num<10)
				strcat(Send_Buff, "0");
			strcat(Send_Buff, FN_Num_Char);
			strcat(Send_Buff, ptr->d_name);
			Send_Buff[1024] = '\0';
			//С��0 �������Ϊʲôwrite���ܷ��ͣ�ֻ����send����
			send(socketfd, Send_Buff, sizeof(Send_Buff), 0);
			printf("%s\n", Send_Buff);
			//ֻ�е����������Ϣ���ص�ʱ��Ż������
			while(recv(socketfd, Receive_Buff, sizeof(Receive_Buff), 0)<=0);
		}
		FN_Num++;
		bzero(FN_Num_Char, sizeof(FN_Num_Char));
	}
	closedir(dir);
	return 1;
 }
//*****�������̺߳���ʵ��
//socket�����߳�,void *arg����˼�ǣ��д������
void *Socket_Connect(void *arg)
{
	//socketfd�����߳�
	int socketfd = *(int*)arg;
	//Read_And_Send_Video();
	while(1)	
	{
		printf("Client_Ready\n");
		//�����������
		Receive_Len = recv(socketfd, Receive_Buff, sizeof(Receive_Buff), 0);
		//�����ڽ���ʱ���˳ɹ������жϣ������ǰ��ķ����Ǳ�ҲҪ���жϣ�exit(0);ʲô��˼��̫֪��
		if(Receive_Len==-1)
		{
			printf("Recv Error\n");
			exit(1);//�������˳���
		}
		//������Ч����֤
		else if(Receive_Len>0)
		{
			//���ӽ���λ��ת����ַ���
			Receive_Buff[Receive_Len]  = '\0';
			printf("Received_Message : %s \n",Receive_Buff);
			//�����ļ���
			if(Receive_Buff[0]=='F'&&Receive_Buff[1]=='N')
			{
				//��ȡĿ¼_��ͨ��socket���ͳ�ȥ
				readFileList("/home/pi/Desktop/Take_Video/");
				printf("Send Video Succeed\n");
			}
		}
	}
	
}
//�����⣬¼���߳�
//������߳��У�����������Ƶת���Ĺ��ܶ�����������ԣ��������Լ�ת����֮ǰ���������
void *Human_Detection()
{
	Gpio_Inil();
	while(1)
	{
		//28��GPIOĬ��Ϊ0
		if(digitalRead (28) == 1)
		{
			delay (500);  //��ʱ500ms
			if(digitalRead (28) == 1)
			{
				//Take_And_Transform_Video();//��������shell�������ɶ���
				printf("Take_And_Transform_Video Succeed\n");
			}
		}
	}
}
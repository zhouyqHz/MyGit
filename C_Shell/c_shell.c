#include <stdio.h>	//
#include <stdlib.h>  //c���Ե���shell����ͷ�ļ�����,intתchar��Ҫ
#include <string.h> //�ַ���ƴ��ͷ�ļ�
#include <time.h> 	//ʱ�����ͷ�ļ�(sleep)
#include <unistd.h> //socket close��������ͷ�ļ� �Լ�sleep���裨��ݮ�ɸ�linux�е㲻һ����
#include <sys/socket.h> 	//����socket���ͷ�ļ�
#include <netinet/in.h> 	//��ַstruct���ͷ�ļ�
#include <fcntl.h>  //�ļ���д��ؿ�
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
int socketfd = 0;
void Socket_Inil();
//������
int main()
{
	Socket_Inil();
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
	strcat(Take_Video, ".h264 -t 10000 -w 500 -h 330");
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
	//system(Take_Video);
	printf("Transform_Video:%s\n",Transform_Video);
	//��������MP4��shell����
	//system(Transform_Video);
	//���ļ�������ļ�������ʽ
	FILE* pf = fopen("/home/pi/Desktop/Take_Video/2019-05-23-pm-17-05-41.mp4", "rb");
	if(pf == NULL) 
	{  
		printf("open file failed!\n");  
    }
	else
		printf("open file succeed!\n"); 
	char pack[1024*512] = {'\0'};
}

void Socket_Inil()
{

}
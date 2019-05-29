#include <stdio.h>
#include <stdlib.h>			//intתchar��Ҫ
#include <unistd.h>     	//socket close��������ͷ�ļ�
#include <sys/socket.h> 	//����socket���ͷ�ļ�
#include <netinet/in.h> 	//��ַstruct���ͷ�ļ�
#include <mysql/mysql.h>    //���ݿ����ͷ�ļ�
#include <string.h>         //�ַ���ƴ��ͷ�ļ�
#include <time.h> 			//ʱ�����ͷ�ļ�(sleep)
#include <pthread.h>		//���߳�ͷ�ļ�
#include <fcntl.h>          //�ļ���д��ؿ�
//���ݱ���������
//mysql�������
char Mysql_Insert_FileList_Head[] = "insert into FileList(date, FileName) values('";
char Mysql_Insert_FileList[100] = "";
char Mysql_Value_Data[30] = ""; //��ʱ�������
//
char Mysql_Updata_Head[] = "UPDATE socketfd SET socketfd='";
char Mysql_Updata_Websocketfd_Head[]    = "UPDATE socketfd SET socketfd_web='";
char Mysql_Updata_Clientsocketfd_Head[] = "UPDATE socketfd SET socketfd_client='";
char Mysql_Updata_Appsocketfd_Head[]    = "UPDATE socketfd SET socketfd_app='";
char Mysql_Updata_State_Head[] = "UPDATE socketfd SET state='1' WHERE id='0'";
char Mysql_Updata_table_0[100];
char Mysql_Value_Id[5] = "";
char Mysql_Value_Socketfd[10] = "";
char Mysql_client_Socketfd[10] = "";
//
char Mysql_Select_Head[]="SELECT socketfd FROM socketfd  WHERE id='0'";
char Mysql_Select_all_Head[]="SELECT * FROM socketfd  WHERE id='0'";
char *Mysql_Get_Socketfd;
char *Mysql_Get_appSocketfd;
char Buff_Led_state[22] = "";
//�뱾���̹����޹�

char Mysql_Num_Data[50] = "";
//****Mysql�������
int res;
//�̱߳�־��
pthread_t id;
//mysql��ر���
MYSQL 		*mysql_main;
MYSQL_RES 	*mysqlResult ;
MYSQL_FIELD *mysqlField ; //�����ֶ�������Ϣ	
MYSQL_ROW 	mysqlRow;
//socket��ر���
int socketfd;
int count = 0;
int count_flag;
int socketfd_rec;
int socketfd_send;
char Buff[1024];//socket���ݽ��ջ���
char Buff_Video[1024*512];//socket�������ջ���
int  rec_n;		//socket���ձ�־
//����һ��socket��������
int listenfd;
//����������
void Mysql_Inil();
//�̴߳����������������ݴ������̺߳����
void *thread(void *arg);
//������
int  main()
{
	//�󶨵�ַ����ṹ�壨�����Ҫ�������棬�������ͨ������
	struct sockaddr_in sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	sockaddr.sin_port = htons(5000);
	listenfd = socket(AF_INET,SOCK_STREAM,0);//IPV4,TCP
	//����bind���󶨶˿�
	bind(listenfd,(struct sockaddr *) &sockaddr,sizeof(sockaddr));
	//������������������5��
	listen(listenfd,5);
	printf("Listen succeed Waiting Connect\n");
	//Ȼ��ʼʼ�յȴ�����
	while(1)
	{
		//�ȴ����ӣ�����������������
		if((socketfd = accept(listenfd,(struct sockaddr*)NULL,NULL))==-1)
		{
			printf("Accpet socket error\n");
			continue;
		}
		else
		{
			printf("New connection,sicketfd=%d\n",socketfd);
			//�������̣߳�����socketfd�������߳�
			pthread_create(&id, NULL, thread, &socketfd);
			printf("creat_thread_succeed,threadid=%d\n",id);
		}		
		//�رյ�ǰsocket���ӣ�socketfdΪint�ͣ�����Ϊ��ǰsocket��id��
		//close(socketfd); //��������Զ��Ͽ�
	}
}
//������
void Mysql_Inil()
{
	//mysql��ʼ����ʼ
	printf("mysql_Begin init\n");
	mysql_main = mysql_init(NULL);	//	
	printf("mysql init ok\n");
	//��ʼ�����ɹ�
	if(!mysql_main)
	{
		printf("Inil failed\n");
		fprintf(stderr,"mysql_init failed\n");
		exit(0);
	}
}
//���̴߳����������������ļ���������
void *thread(void *arg)
{
	int socketfd = *(int*)arg;
	unsigned int R_len = 0;
	printf("socketfd in pthread=%d\n",socketfd);
	//����abc��mp4�ļ�
	FILE* pf = fopen("abc.mp4", "wb+");
	if(pf == NULL)  
	{  
		printf("Open file error!\n");  
	} 
	while(1)
	{	
		//������������
		bzero(Buff, sizeof(Buff));
		//���̶߳��������recv����Ҳ�������
		rec_n = recv(socketfd,Buff,1024*512,0);
		//������Ч
		if(rec_n>0)
		{	
			//BUff����������������Ϊ�ַ���
			Buff[rec_n] = '\0';
			//������ļ������ϴ�
			if(Buff[0]=='F'&&Buff[1]=='N')
			{
				//mysql��ʼ��
				Mysql_Inil();
				//����mysql
				mysql_main = mysql_real_connect(mysql_main,"localhost","root","1206","lamptest",0,NULL,0);
				if(mysql_main)
				{
					//���ӳɹ�
					printf("Connection succeed\n");
					//������Ϣ
					//��ȡ��BUff�е����м�λ����
					strncpy(Mysql_Value_Data, Buff+4, 10);
					//ƴ�Ӳ����ַ���
					stpcpy(Mysql_Insert_FileList, Mysql_Insert_FileList_Head);
					strcat(Mysql_Insert_FileList, Mysql_Value_Data);
					strcat(Mysql_Insert_FileList, "', '");
					strcat(Mysql_Insert_FileList, Buff);
					strcat(Mysql_Insert_FileList, "')");
					printf("mysql:%s\n",Mysql_Insert_FileList);
					//���������
					//res=mysql_query(mysql_main,Mysql_Insert_FileList);
				}
			}
			//send(socketfd, Buff, sizeof(Buff), 0);
			//printf("rev:%d send:%d",socketfd_rec, socketfd_send);
			//д�ļ�
			//fwrite(Buff_Video, sizeof(char), rec_n, pf);
		}		
		//if(count>=2)
			//�ر��ļ������Ǹ�ʲôʱ�����
		//	fclose(pf);
		//rec_n=0��˵���ͻ��˶Ͽ�
		else if(rec_n==0)
		{
			printf("socket close\n");
			//�ر�socket���˳����߳�
			close(socketfd);
			pthread_exit(NULL);
		}
		
	}
}
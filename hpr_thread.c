#include <stdio.h>
#include <stdlib.h>			//intתchar��Ҫ
#include <unistd.h>     	//socket close��������ͷ�ļ�
#include <sys/socket.h> 	//����socket���ͷ�ļ�
#include <netinet/in.h> 	//��ַstruct���ͷ�ļ�
#include <mysql/mysql.h>    //���ݿ����ͷ�ļ�
#include <string.h>         //�ַ���ƴ��ͷ�ļ�
#include <time.h> 			//ʱ�����ͷ�ļ�(sleep)
#include <pthread.h>		//���߳�ͷ�ļ�
//���ݱ���������
//mysql�������
char Mysql_Insert_Head[] = "insert into Table_0(id, value, num) values('1' ,'";
char Mysql_Insert_table_0[100];
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
char Mysql_Value_Data[22] = "";
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
char Buff[1024];//socket����
int  rec_n;		//socket���ձ�־
//����һ��socket��������
int listenfd;
//�̴߳����������������ݴ������̺߳����
void *thread(void *arg);
//������
int  main()
{
	//�󶨵�ַ����ṹ�壨�����Ҫ�������棬�������ͨ������
	struct sockaddr_in sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	sockaddr.sin_port = htons(6280);
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
//���̴߳�����
void *thread(void *arg)
{
	int socketfd = *(int*)arg;
	printf("socketfd in pthread=%d\n",socketfd);
	
	count++;
	printf("count:%d",count);
	while(1)
	{	
		//������������
		bzero(Buff, sizeof(Buff));
		printf("count:%d");
		if(count!=count_flag)
		{
			socketfd_rec = socketfd;
		}
		//���̶߳��������recv����Ҳ�������
		rec_n = recv(socketfd,Buff,1024,0);
		//������Ч
		if(rec_n>0)
		{
			if(Buff[0]==0x30&&Buff[1]==0x31)
			{
				socketfd_send = socketfd;
				count_flag=count;
				
			}
			
			send(socketfd_rec, Buff, sizeof(Buff), 0);
			printf("rev:%d send:%d",socketfd_rec, socketfd_send);
		}
		//rec_n=0��˵���ͻ��˶Ͽ�
		
	}
}
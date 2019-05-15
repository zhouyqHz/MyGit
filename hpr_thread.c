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
char Buff[1024];//socket����
int  rec_n;		//socket���ձ�־�Լ�λ��
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
//���̴߳�����
void *thread(void *arg)
{
	//����������socketfdת���ɱ��߳��еı���
	int socketfd = *(int*)arg;
	printf("socketfd in pthread=%d\n",socketfd);
	while(1)
	{	
		//������������
		bzero(Buff, sizeof(Buff));
		//���̶߳��������recv����Ҳ�������
		rec_n = recv(socketfd,Buff,1024,0);
		//������Ч
		if(rec_n>0)
		{
			//1.��λ��������
			if(rec_n==16&&Buff[14]==0x42&&Buff[15]==0x01)
			{
				printf("Heart Bite Coming\n");
			}
			//2.���Э�鳤����75��������λ��ÿ��Сʱ�ϴ�������
			else if( (rec_n==75)&&(Buff[14]==0x04)&&(Buff[15]==0x01))
			{
				printf("Client Data Coming\n");
			}
			//3����Ǵ���ҳ������У׼ʱ������
			else if( (rec_n==20)&&(Buff[0]=='f')&&(Buff[5]=='7')&&(Buff[6]=='0')&&(Buff[7]=='0'))	
			{
				printf("Web Check Time Coming\n");
			}
			 //4����Ǵ���ҳ������ˢ����������(�Ѻ�)
            else if( (rec_n==8)&&(Buff[0]=='f')&&(Buff[5]=='2'))
			{
				printf("Web Refash Data Coming\n");
			}
			//5����Ǵ���ҳ���������ÿ�������(�Ѿ��ɹ�)
            else if( (rec_n==8)&&(Buff[0]=='f')&&(Buff[5]=='4')&&(Buff[6]=='0')&&((Buff[7]=='0')||(Buff[7]=='1')))	
			{
				printf("Web Set Switch Coming\n");
			}
			//6����Ǵ���ҳ���������ù���ʱ������(��ʱ��)
            else if( (rec_n==16)&&(Buff[0]=='f')&&(Buff[14]!=0x42))
			{
				printf("Web Set Work Time Coming\n");
			}
			//7����Ǵ���ҳ���������ñ�����Χ����
            else if( (rec_n==35)&&(Buff[0]=='f')&&(Buff[5]=='6'))
			{
				printf("Web Set Alarm Range Coming\n");
			}
			//8.����Ǵ���ҳ���������õ绰����
            else if( (rec_n==21)&&(Buff[0]=='s')&&(Buff[18]=='f'))
			{
				printf("Web Set Telephone Number Coming\n");
			}
			//9.����Ǵ���ҳ�������ϴ�ͼƬ����
            else if( (rec_n==21)&&(Buff[0]=='s')&&(Buff[18]=='a'))
			{
				printf("Web Request Picture Coming\n");
			}
			//10.���Э�鳤����8λ��Ӧ��
            else if( (Buff[0]==0x66)&&(Buff[1]==0x00)&&(Buff[14]!=0x42)&&(rec_n!=19))
			{
				printf("Web 8Bite Request Coming\n");
			}
			
			
			send(socketfd, Buff, sizeof(Buff), 0);
		}
		//rec_n=0��˵���ͻ��˶Ͽ�
		else if(rec_n==0)
		{
			printf("Socket Close\n");
			//�ر�socket���˳����߳�
			close(socketfd);
			pthread_exit(NULL);
		}
		
	}
}
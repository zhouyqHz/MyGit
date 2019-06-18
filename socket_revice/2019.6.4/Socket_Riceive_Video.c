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
char Mysql_Select_Date_All[]="SELECT date FROM FileList";
char Mysql_Select_all_Head[]="SELECT * FROM socketfd  WHERE id='0'";
char *Mysql_Get_Socketfd;
char *Mysql_Get_appSocketfd;
char Buff_Led_state[22] = "";
//�ļ������õ�����
char File_Name[50] = "";
FILE* pf;
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
int Socketfd_Video = 0;   //����˵�socketfd�ļ�¼
char Buff[1024*512];      //socket���ݽ��ջ���
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
//���̴߳������������ļ��б����������ļ���������
void *thread(void *arg)
{
	int Cmp_Bit = 1;
	int socketfd = *(int*)arg;
	printf("socketfd in pthread=%d\n",socketfd);	
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
			printf("recdata:%s\n",Buff);
			//����˵�socketfd�ĸ���
			if(Buff[0]=='F'&&Buff[1]=='S')
			{
				Socketfd_Video = socketfd;
				printf("Vicdeo_socketfd:%d\n", Socketfd_Video);
			}
			//������ļ������ϴ�
			else if(Buff[0]=='F'&&Buff[1]=='N')
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
					//��ȡ��BUff�е����м�λ����(ֻ��ȡ��������)
					strncpy(Mysql_Value_Data, Buff+4, 22);
					//���濪ʼ�Ƚ����ݿ����Ƿ��м�¼
					res=mysql_query(mysql_main,Mysql_Select_Date_All); 
					if(res)
					{							
						printf("select_sqlerror\n");
					}
					else
					{
						printf("select_sqlok\n");
						//��ȡ������ݼ�
						mysqlResult = mysql_store_result(mysql_main);
						//Ϊ�����ѯʧ��
						if (mysqlResult == NULL)
						{
							printf(">���ݲ�ѯʧ��! %d:%s\n", mysql_errno(mysql_main), mysql_error(mysql_main));
							return;
						}
						//�����Ǽ��������ݿ��Ƿ�����ݮ�ɷ������������¼
						while((mysqlRow = mysql_fetch_row(mysqlResult))&&Cmp_Bit==1)
						{
							printf("Row:%s\n", mysqlRow[0]);
							printf("Mysql_Value_Data:%s\n", Mysql_Value_Data);
							//�����������ݿ��Ѿ��м�¼
							if(strcmp(Mysql_Value_Data, mysqlRow[0])==0)
							{
								Cmp_Bit = 0;
								printf("The FileList Already Have\n");
							}
							else 
							{
								Cmp_Bit = 1;
							}
						}
						//�ͷ����ݼ�
						mysql_free_result(mysqlResult);
						//�������ڴ˼�¼ʱ
						if(Cmp_Bit==1)
						{
							//ƴ�Ӳ����ַ���
							stpcpy(Mysql_Insert_FileList, Mysql_Insert_FileList_Head);
							strcat(Mysql_Insert_FileList, Mysql_Value_Data);
							strcat(Mysql_Insert_FileList, "', '");
							strcat(Mysql_Insert_FileList, Mysql_Value_Data);
							strcat(Mysql_Insert_FileList, ".mp4')");
							printf("mysql:%s\n",Mysql_Insert_FileList);
							//���������
							res=mysql_query(mysql_main,Mysql_Insert_FileList);
							if(res)
							{							
								printf("sqlerror\n");
								exit(0);
							}
							else
							{
								printf("sqlok\n");
								
							}
						}
						//�ѱȽ�λ������Ϊ1(�ǳ���Ҫ,�����ж��ļ��Ƿ����ֻ���ж�һ��)
						Cmp_Bit = 1;
						//�·��ɹ����գ���Ҫ��������͵�ָ��ǿռ��ɣ�
						send(Socketfd_Video, "FN", 2, 0);
					}

				}
			}
			//��ݮ�ɽ����ļ������ݽ�������
			else if(Buff[0]=='E'&&Buff[1]=='N'&&Buff[2]=='D')
			{
				//�ر����ݿ⣨������Ҫ�رգ��ǳ���Ҫ��
				mysql_close(mysql_main);
				printf("File_Name_Update_Done\n");
			}
			//��ݮ�ɷ����Ľ��������ļ���ָ��
			else if(Buff[0]=='V'&&Buff[1]=='E'&&Buff[2]=='N'&&Buff[3]=='D')
			{
				//�ر��ļ�
				fclose(pf);	
			}
			//��ҳ����������ļ���
			else if(Buff[0]=='W'&&Buff[1]=='N')
			{
				printf("Web Request Name\n");
				send(Socketfd_Video, "FN", 2, 0);
			}
			//��ҳ�����������ļ�
			else if(Buff[0]=='W'&&Buff[1]=='U')
			{
				strncpy(File_Name, Buff+2, 26);
				File_Name[49]='\0';
				printf("File_Name:%s\n",File_Name);
				//����mp4�ļ�
				pf	= fopen(File_Name, "wb+");
				if(pf == NULL)  
				{  
					printf("Open file error!\n");  
				}
				//����Ҫ�ϴ����ļ��·�����ݮ��
				send(Socketfd_Video, Buff, sizeof(Buff), 0);
			}
			//send(socketfd, Buff, sizeof(Buff), 0);
			//printf("rev:%d send:%d",socketfd_rec, socketfd_send);
			//д�ļ�
			//fwrite(Buff_Video, sizeof(char), rec_n, pf);
			//��û���κα�־λ��ʱ������ļ�
			else
			{
				//д�ļ�
				fwrite(Buff, sizeof(char), rec_n, pf);
			}
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
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
char *Mysql_Get_clientSocketfd;
char *Mysql_Get_webSocketfd;
char Buff_Led_state[22] = "";
char Buff_Back_state[22] = "";
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
			//��ȡ��ǰʱ��
			time_t now;			//ʵ����time�ṹ
			struct tm *timenow; //ʵ����tm�ṹָ��
			time(&now);
			timenow = localtime(&now);//�ѹ��ʱ�׼ʱ��ת���ɱ���ʱ��
			//asctime����char* strcpyʵ�ְ�char*���Ƹ��ַ�����
			strcpy(Mysql_Num_Data, asctime(timenow));
			printf("time:%s\n", Mysql_Num_Data);
			//BUff���鴦��
			Buff[rec_n] = '\0';
			printf("recv from socketfd%d:%s lenth:%d\n",socketfd, Buff, rec_n);	
			//��ͷ���������00�Ļ�,BUffsocket����
			if(Buff[0]==0x30&&Buff[1]==0x30)	
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
					return 0;
				}
				bzero(Mysql_Value_Data, sizeof(Mysql_Value_Data));
				//��ȡ��BUff�е����м�λ����
				strncpy(Mysql_Value_Data, Buff+2, 22);
				//�ַ��������������Ҫ�ӣ�
				Mysql_Value_Data[22] = '\0';
				//��ȡ��ǰʱ��
				time_t now;			//ʵ����time�ṹ
				struct tm *timenow; //ʵ����tm�ṹָ��
				time(&now);
				timenow = localtime(&now);//�ѹ��ʱ�׼ʱ��ת���ɱ���ʱ��
				//asctime����char* strcpyʵ�ְ�char*���Ƹ��ַ�����
				strcpy(Mysql_Num_Data, asctime(timenow));
				printf("time:%s", Mysql_Num_Data);
				//�������ݿ�
				mysql_main = mysql_real_connect(mysql_main,"localhost","root","Abc123!_","Mr_zhou",0,NULL,0);
				//���ӳɹ�
				if(mysql_main)
				{
					printf("Connection succeed\n");
					bzero(Mysql_Insert_table_0, sizeof(Mysql_Insert_table_0));
					//ƴ���ַ���
					stpcpy(Mysql_Insert_table_0, Mysql_Insert_Head);
					strcat(Mysql_Insert_table_0, Mysql_Value_Data);
					strcat(Mysql_Insert_table_0, "','");
					strcat(Mysql_Insert_table_0, Mysql_Num_Data);//ƴʱ�䲿��tm��ʽ
					strcat(Mysql_Insert_table_0, "')");
					//��ӡMysql���
					printf("insert_head:%s\n",Mysql_Insert_table_0);
					//���������
					res=mysql_query(mysql_main,Mysql_Insert_table_0); 
					//��ӡ���
					printf("res:%d\n",res);
					if(res)
					{							
						printf("sqlerror\n");
					}
					else
					{
						printf("sqlok\n");
					}
				}
				//���Ӳ��ɹ�
				else 
				{
					printf("Connection failed\n");
				}
				//�������ݸ���λ��
				send(socketfd, "hello client", 12, 0);
				//�ر����ݿ⣨������Ҫ�رգ��ǳ���Ҫ��
				mysql_close(mysql_main);
			}
			//��ͷ��01(�����ն�id ��Ӧ�� socketfd)			
			else if(Buff[0]==0x30&&Buff[1]==0x31)
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
					return 0;
				}
				bzero(Mysql_Value_Id, sizeof(Mysql_Value_Id));
				//��ȡ��BUff�е�����2λ����
				strncpy(Mysql_Value_Id, Buff+2, 2);
				//�ַ��������������Ҫ�ӣ�
				Mysql_Value_Id[5] = '\0';
				//�������ݿ�
				mysql_main = mysql_real_connect(mysql_main,"localhost","root","Abc123!_","Mr_zhou",0,NULL,0);
				//���ӳɹ�
				if(mysql_main)
				{
					printf("Connection succeed\n");
					bzero(Mysql_Value_Socketfd, sizeof(Mysql_Value_Socketfd));
					//intת����char
					sprintf(Mysql_Value_Socketfd, "%d", socketfd);
					Mysql_Value_Socketfd[10] = '\0';
					bzero(Mysql_Updata_table_0, sizeof(Mysql_Updata_table_0));
					//ƴ���ַ���
					stpcpy(Mysql_Updata_table_0, Mysql_Updata_Head);
					strcat(Mysql_Updata_table_0, Mysql_Value_Socketfd);
					strcat(Mysql_Updata_table_0, "' WHERE id='0'");
					//��ӡMysql���
					printf("insert_head:%s\n",Mysql_Updata_table_0);
					res=mysql_query(mysql_main,Mysql_Updata_table_0); 
					//��ӡ���
					printf("res:%d\n",res);
					if(res)
					{							
						printf("sqlerror\n");
					}
					else
					{
						printf("sqlok\n");
					}
				}
				//�ر����ݿ⣨������Ҫ�رգ��ǳ���Ҫ��
				mysql_close(mysql_main);
			}
			//��ͷ��02 03 04,��ҳ�����������ն˷�������������			
			else if((Buff[0]==0x30&&Buff[1]==0x32)||(Buff[0]==0x30&&Buff[1]==0x33)||(Buff[0]==0x30&&Buff[1]==0x34))
			{
				//mysql���
				char Mysql_Updata_socketfd[100];
				bzero(Mysql_Updata_socketfd, sizeof(Mysql_Updata_socketfd));
				//��ǰfd
				bzero(Mysql_client_Socketfd, sizeof(Mysql_client_Socketfd));
				//intת����char
				sprintf(Mysql_client_Socketfd, "%d", socketfd);
				//��ҳ������
				if(Buff[0]==0x30&&Buff[1]==0x32)
				{	
					//ƴ���ַ���
					stpcpy(Mysql_Updata_socketfd, Mysql_Updata_Websocketfd_Head);
				}
				//�ն˷�����
				else if(Buff[0]==0x30&&Buff[1]==0x33)
				{
					//ƴ���ַ���
					stpcpy(Mysql_Updata_socketfd, Mysql_Updata_Clientsocketfd_Head);
				}
				//app������
				else if(Buff[0]==0x30&&Buff[1]==0x34)
				{
					//ƴ���ַ���
					stpcpy(Mysql_Updata_socketfd, Mysql_Updata_Appsocketfd_Head);
				}
				strcat(Mysql_Updata_socketfd, Mysql_client_Socketfd);
				strcat(Mysql_Updata_socketfd, "' WHERE id='0'");
				//mysql��ʼ����ʼ
				printf("mysql_Begin init\n");
				mysql_main = mysql_init(NULL);	//	
				printf("mysql init ok\n");
				//��ʼ�����ɹ�
				if(!mysql_main)
				{
					printf("Inil failed\n");
					fprintf(stderr,"mysql_init failed\n");
					return 0;
				}
				bzero(Buff_Led_state, sizeof(Buff_Led_state));
				//��ȡ��BUff�е�����5λ�ƵĲ�������
				strncpy(Buff_Led_state, Buff+2, 22);
				//�ַ��������������Ҫ�ӣ�
				Buff_Led_state[22] = '\0';
				printf("Buff_Led_state:%s\n",Buff_Led_state);
				//�������ݿ�
				mysql_main = mysql_real_connect(mysql_main,"localhost","root","Abc123!_","Mr_zhou",0,NULL,0);
				
				
				//���ӳɹ�
				if(mysql_main)
				{
					printf("Connection succeed\n");
					//���¸����ն˵�socketfd
					printf("updata_head:%s\n",Mysql_Updata_socketfd);
					res=mysql_query(mysql_main,Mysql_Updata_socketfd); 
					//��ӡ���
					printf("updatares:%d\n",res);
					if(res)
					{							
						printf("updatasqlerror\n");
					}
					else
					{
						printf("updatasqlok\n");
					}
					//��ӡMysql���
					printf("slect_head:%s\n",Mysql_Select_Head);
					//��ȡidΪ0�����ӵ�socketfd
					res=mysql_query(mysql_main,Mysql_Select_Head); 
					//��ӡ���
					printf("res:%d\n",res);
					if(res)
					{							
						printf("sqlerror\n");
					}
					else
					{
						printf("sqlok\n");
						//��ȡ������ݼ�
						mysqlResult = mysql_store_result(mysql_main);
						//Ϊ�����ѯʧ��
						if (mysqlResult == NULL)
						{
							printf(">���ݲ�ѯʧ��! %d:%s\n", mysql_errno(mysql_main), mysql_error(mysql_main));
							return;
						}
						else
						{
							if(mysqlRow = mysql_fetch_row(mysqlResult))
							{
								//��ȡ����
								Mysql_Get_Socketfd = mysqlRow[0];
								printf("get socketfd:%d\n", atoi(Mysql_Get_Socketfd));
								//����ҳ�����������ݣ�ת����id=0��socket����
								send(atoi(Mysql_Get_Socketfd), Buff_Led_state, sizeof(Buff_Led_state), 0);
								//printf("go to client0\n");
							}
							//�ͷ����ݼ�
							mysql_free_result(mysqlResult);
						}
					}
				}
				//�ر����ݿ⣨������Ҫ�رգ��ǳ���Ҫ��
				mysql_close(mysql_main);
			}
			//��ͷ��λ��11��Ӧ��
			else if(Buff[0]==0x31&&Buff[1]==0x31)
			{
				printf("Set Succeed\n");
				//��ȡ��BUff�е����м�λ����
				strncpy(Buff_Back_state, Buff+2, 22);
				//������Ҫ��
				Buff_Back_state[22] = '\0';
				//mysql��ʼ����ʼ
				printf("mysql_Begin init\n");
				mysql_main = mysql_init(NULL);	//	
				printf("mysql init ok\n");
				//��ʼ�����ɹ�
				if(!mysql_main)
				{
					printf("Inil failed\n");
					fprintf(stderr,"mysql_init failed\n");
					return 0;
				}
				//�������ݿ�
				mysql_main = mysql_real_connect(mysql_main,"localhost","root","Abc123!_","Mr_zhou",0,NULL,0);
				//���ӳɹ�
				if(mysql_main)
				{
					printf("Connection succeed\n");
					//��ӡMysql���
					printf("slect_head:%s\n",Mysql_Select_all_Head);
					res=mysql_query(mysql_main, Mysql_Select_all_Head); 
					//��ӡ���
					printf("res:%d\n",res);
					if(res)
					{							
						printf("sqlerror\n");
					}
					else
					{
						printf("sqlok\n");
						//��ȡ������ݼ�
						mysqlResult = mysql_store_result(mysql_main);
						//Ϊ�����ѯʧ��
						if (mysqlResult == NULL)
						{
							printf(">���ݲ�ѯʧ��! %d:%s\n", mysql_errno(mysql_main), mysql_error(mysql_main));
							return;
						}
						else
						{
							if(mysqlRow = mysql_fetch_row(mysqlResult))
							{
								//��ȡ����
								Mysql_Get_appSocketfd    = mysqlRow[3];
								Mysql_Get_clientSocketfd = mysqlRow[4];
								Mysql_Get_webSocketfd    = mysqlRow[5];
								//��ӡ�����������ն˵�socketfd
								printf("appsocketfd:%d,clientsocketfd:%d,webSocketfd:%d\n", atoi(Mysql_Get_appSocketfd), atoi(Mysql_Get_clientSocketfd), atoi(Mysql_Get_webSocketfd));
								//��Ӧ��ת���������ն�
								send(atoi(Mysql_Get_appSocketfd),    Buff_Back_state, sizeof(Buff_Back_state), 0);
								send(atoi(Mysql_Get_clientSocketfd), Buff_Back_state, sizeof(Buff_Back_state), 0);
								send(atoi(Mysql_Get_webSocketfd),    Buff_Back_state, sizeof(Buff_Back_state), 0);
								//printf("go to client0\n");
							}
							//�ͷ����ݼ�
							mysql_free_result(mysqlResult);
						}
					}
				}
				//�ر����ݿ⣨������Ҫ�رգ��ǳ���Ҫ��
				mysql_close(mysql_main);
			}
			//ǰ��λ����00,ҲҪ�������ݸ���λ��
			//else
			//{	
				//write �����������Ҳ������������
			//	send(socketfd, "Recerve ok", 12, 0);
			//}
		}
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
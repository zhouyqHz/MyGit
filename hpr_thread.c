#include <stdio.h>
#include <stdlib.h>			//int转char需要
#include <unistd.h>     	//socket close函数所在头文件
#include <sys/socket.h> 	//建立socket相关头文件
#include <netinet/in.h> 	//地址struct相关头文件
#include <mysql/mysql.h>    //数据库相关头文件
#include <string.h>         //字符串拼接头文件
#include <time.h> 			//时间相关头文件(sleep)
#include <pthread.h>		//多线程头文件
//数据变量定义区
//mysql操作语句
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
//与本工程功能无关
char Mysql_Value_Data[22] = "";
char Mysql_Num_Data[50] = "";
//****Mysql操作结果
int res;
//线程标志符
pthread_t id;
//mysql相关变量
MYSQL 		*mysql_main;
MYSQL_RES 	*mysqlResult ;
MYSQL_FIELD *mysqlField ; //保存字段名字信息	
MYSQL_ROW 	mysqlRow;
//socket相关变量
int socketfd;
int count = 0;
int count_flag;
int socketfd_rec;
int socketfd_send;
char Buff[1024];//socket缓冲
int  rec_n;		//socket接收标志
//建立一个socket监听变量
int listenfd;
//线程处理函数（基本上数据处理都在线程函数里）
void *thread(void *arg);
//主进程
int  main()
{
	//绑定地址所需结构体（这个需要放在里面，否则编译通不过）
	struct sockaddr_in sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	sockaddr.sin_port = htons(6280);
	listenfd = socket(AF_INET,SOCK_STREAM,0);//IPV4,TCP
	//建立bind，绑定端口
	bind(listenfd,(struct sockaddr *) &sockaddr,sizeof(sockaddr));
	//建立监听（最大监听数5）
	listen(listenfd,5);
	printf("Listen succeed Waiting Connect\n");
	//然后开始始终等待连接
	while(1)
	{
		//等待连接，主进程阻塞在这里
		if((socketfd = accept(listenfd,(struct sockaddr*)NULL,NULL))==-1)
		{
			printf("Accpet socket error\n");
			continue;
		}
		else
		{
			printf("New connection,sicketfd=%d\n",socketfd);
			//创建子线程，并把socketfd传入子线程
			pthread_create(&id, NULL, thread, &socketfd);
			printf("creat_thread_succeed,threadid=%d\n",id);
		}		
		//关闭当前socket连接，socketfd为int型，就是为当前socket的id号
		//close(socketfd); //接收完毕自动断开
	}
}
//子线程处理函数
void *thread(void *arg)
{
	int socketfd = *(int*)arg;
	printf("socketfd in pthread=%d\n",socketfd);
	
	count++;
	printf("count:%d",count);
	while(1)
	{	
		//接收数组清零
		bzero(Buff, sizeof(Buff));
		printf("count:%d");
		if(count!=count_flag)
		{
			socketfd_rec = socketfd;
		}
		//子线程堵塞在这里（recv函数也会堵塞）
		rec_n = recv(socketfd,Buff,1024,0);
		//数据有效
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
		//rec_n=0则说明客户端断开
		
	}
}
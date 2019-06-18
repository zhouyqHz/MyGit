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
char *Mysql_Get_clientSocketfd;
char *Mysql_Get_webSocketfd;
char Buff_Led_state[22] = "";
char Buff_Back_state[22] = "";
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
	sockaddr.sin_port = htons(5000);
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
	while(1)
	{	
		//接收数组清零
		bzero(Buff, sizeof(Buff));
		//子线程堵塞在这里（recv函数也会堵塞）
		rec_n = recv(socketfd,Buff,1024,0);
		//数据有效
		if(rec_n>0)
		{
			//获取当前时间
			time_t now;			//实例化time结构
			struct tm *timenow; //实例化tm结构指针
			time(&now);
			timenow = localtime(&now);//把国际标准时间转换成本地时间
			//asctime返回char* strcpy实现把char*复制给字符数组
			strcpy(Mysql_Num_Data, asctime(timenow));
			printf("time:%s\n", Mysql_Num_Data);
			//BUff数组处理
			Buff[rec_n] = '\0';
			printf("recv from socketfd%d:%s lenth:%d\n",socketfd, Buff, rec_n);	
			//开头两个如果是00的话,BUffsocket处理
			if(Buff[0]==0x30&&Buff[1]==0x30)	
			{
				//mysql初始化开始
				printf("mysql_Begin init\n");
				mysql_main = mysql_init(NULL);	//	
				printf("mysql init ok\n");
				//初始化不成功
				if(!mysql_main)
				{
					printf("Inil failed\n");
					fprintf(stderr,"mysql_init failed\n");
					return 0;
				}
				bzero(Mysql_Value_Data, sizeof(Mysql_Value_Data));
				//截取出BUff中的其中几位数据
				strncpy(Mysql_Value_Data, Buff+2, 22);
				//字符串结束符（最好要加）
				Mysql_Value_Data[22] = '\0';
				//获取当前时间
				time_t now;			//实例化time结构
				struct tm *timenow; //实例化tm结构指针
				time(&now);
				timenow = localtime(&now);//把国际标准时间转换成本地时间
				//asctime返回char* strcpy实现把char*复制给字符数组
				strcpy(Mysql_Num_Data, asctime(timenow));
				printf("time:%s", Mysql_Num_Data);
				//连接数据库
				mysql_main = mysql_real_connect(mysql_main,"localhost","root","Abc123!_","Mr_zhou",0,NULL,0);
				//连接成功
				if(mysql_main)
				{
					printf("Connection succeed\n");
					bzero(Mysql_Insert_table_0, sizeof(Mysql_Insert_table_0));
					//拼接字符串
					stpcpy(Mysql_Insert_table_0, Mysql_Insert_Head);
					strcat(Mysql_Insert_table_0, Mysql_Value_Data);
					strcat(Mysql_Insert_table_0, "','");
					strcat(Mysql_Insert_table_0, Mysql_Num_Data);//拼时间部分tm格式
					strcat(Mysql_Insert_table_0, "')");
					//打印Mysql语句
					printf("insert_head:%s\n",Mysql_Insert_table_0);
					//做插入操作
					res=mysql_query(mysql_main,Mysql_Insert_table_0); 
					//打印结果
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
				//连接不成功
				else 
				{
					printf("Connection failed\n");
				}
				//发送数据给下位机
				send(socketfd, "hello client", 12, 0);
				//关闭数据库（操作完要关闭，非常必要）
				mysql_close(mysql_main);
			}
			//开头是01(更新终端id 对应的 socketfd)			
			else if(Buff[0]==0x30&&Buff[1]==0x31)
			{
				//mysql初始化开始
				printf("mysql_Begin init\n");
				mysql_main = mysql_init(NULL);	//	
				printf("mysql init ok\n");
				//初始化不成功
				if(!mysql_main)
				{
					printf("Inil failed\n");
					fprintf(stderr,"mysql_init failed\n");
					return 0;
				}
				bzero(Mysql_Value_Id, sizeof(Mysql_Value_Id));
				//截取出BUff中的其中2位数据
				strncpy(Mysql_Value_Id, Buff+2, 2);
				//字符串结束符（最好要加）
				Mysql_Value_Id[5] = '\0';
				//连接数据库
				mysql_main = mysql_real_connect(mysql_main,"localhost","root","Abc123!_","Mr_zhou",0,NULL,0);
				//连接成功
				if(mysql_main)
				{
					printf("Connection succeed\n");
					bzero(Mysql_Value_Socketfd, sizeof(Mysql_Value_Socketfd));
					//int转换成char
					sprintf(Mysql_Value_Socketfd, "%d", socketfd);
					Mysql_Value_Socketfd[10] = '\0';
					bzero(Mysql_Updata_table_0, sizeof(Mysql_Updata_table_0));
					//拼接字符串
					stpcpy(Mysql_Updata_table_0, Mysql_Updata_Head);
					strcat(Mysql_Updata_table_0, Mysql_Value_Socketfd);
					strcat(Mysql_Updata_table_0, "' WHERE id='0'");
					//打印Mysql语句
					printf("insert_head:%s\n",Mysql_Updata_table_0);
					res=mysql_query(mysql_main,Mysql_Updata_table_0); 
					//打印结果
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
				//关闭数据库（操作完要关闭，非常必要）
				mysql_close(mysql_main);
			}
			//开头是02 03 04,网页或者是其他终端发来的设置命令			
			else if((Buff[0]==0x30&&Buff[1]==0x32)||(Buff[0]==0x30&&Buff[1]==0x33)||(Buff[0]==0x30&&Buff[1]==0x34))
			{
				//mysql语句
				char Mysql_Updata_socketfd[100];
				bzero(Mysql_Updata_socketfd, sizeof(Mysql_Updata_socketfd));
				//当前fd
				bzero(Mysql_client_Socketfd, sizeof(Mysql_client_Socketfd));
				//int转换成char
				sprintf(Mysql_client_Socketfd, "%d", socketfd);
				//网页发来的
				if(Buff[0]==0x30&&Buff[1]==0x32)
				{	
					//拼接字符串
					stpcpy(Mysql_Updata_socketfd, Mysql_Updata_Websocketfd_Head);
				}
				//终端发来的
				else if(Buff[0]==0x30&&Buff[1]==0x33)
				{
					//拼接字符串
					stpcpy(Mysql_Updata_socketfd, Mysql_Updata_Clientsocketfd_Head);
				}
				//app发来的
				else if(Buff[0]==0x30&&Buff[1]==0x34)
				{
					//拼接字符串
					stpcpy(Mysql_Updata_socketfd, Mysql_Updata_Appsocketfd_Head);
				}
				strcat(Mysql_Updata_socketfd, Mysql_client_Socketfd);
				strcat(Mysql_Updata_socketfd, "' WHERE id='0'");
				//mysql初始化开始
				printf("mysql_Begin init\n");
				mysql_main = mysql_init(NULL);	//	
				printf("mysql init ok\n");
				//初始化不成功
				if(!mysql_main)
				{
					printf("Inil failed\n");
					fprintf(stderr,"mysql_init failed\n");
					return 0;
				}
				bzero(Buff_Led_state, sizeof(Buff_Led_state));
				//截取出BUff中的其中5位灯的操作数据
				strncpy(Buff_Led_state, Buff+2, 22);
				//字符串结束符（最好要加）
				Buff_Led_state[22] = '\0';
				printf("Buff_Led_state:%s\n",Buff_Led_state);
				//连接数据库
				mysql_main = mysql_real_connect(mysql_main,"localhost","root","Abc123!_","Mr_zhou",0,NULL,0);
				
				
				//连接成功
				if(mysql_main)
				{
					printf("Connection succeed\n");
					//更新各个终端的socketfd
					printf("updata_head:%s\n",Mysql_Updata_socketfd);
					res=mysql_query(mysql_main,Mysql_Updata_socketfd); 
					//打印结果
					printf("updatares:%d\n",res);
					if(res)
					{							
						printf("updatasqlerror\n");
					}
					else
					{
						printf("updatasqlok\n");
					}
					//打印Mysql语句
					printf("slect_head:%s\n",Mysql_Select_Head);
					//获取id为0的连接的socketfd
					res=mysql_query(mysql_main,Mysql_Select_Head); 
					//打印结果
					printf("res:%d\n",res);
					if(res)
					{							
						printf("sqlerror\n");
					}
					else
					{
						printf("sqlok\n");
						//获取结果数据集
						mysqlResult = mysql_store_result(mysql_main);
						//为空则查询失败
						if (mysqlResult == NULL)
						{
							printf(">数据查询失败! %d:%s\n", mysql_errno(mysql_main), mysql_error(mysql_main));
							return;
						}
						else
						{
							if(mysqlRow = mysql_fetch_row(mysqlResult))
							{
								//获取数据
								Mysql_Get_Socketfd = mysqlRow[0];
								printf("get socketfd:%d\n", atoi(Mysql_Get_Socketfd));
								//将网页发过来的数据，转发给id=0的socket连接
								send(atoi(Mysql_Get_Socketfd), Buff_Led_state, sizeof(Buff_Led_state), 0);
								//printf("go to client0\n");
							}
							//释放数据集
							mysql_free_result(mysqlResult);
						}
					}
				}
				//关闭数据库（操作完要关闭，非常必要）
				mysql_close(mysql_main);
			}
			//开头两位是11，应答
			else if(Buff[0]==0x31&&Buff[1]==0x31)
			{
				printf("Set Succeed\n");
				//截取出BUff中的其中几位数据
				strncpy(Buff_Back_state, Buff+2, 22);
				//结束符要加
				Buff_Back_state[22] = '\0';
				//mysql初始化开始
				printf("mysql_Begin init\n");
				mysql_main = mysql_init(NULL);	//	
				printf("mysql init ok\n");
				//初始化不成功
				if(!mysql_main)
				{
					printf("Inil failed\n");
					fprintf(stderr,"mysql_init failed\n");
					return 0;
				}
				//连接数据库
				mysql_main = mysql_real_connect(mysql_main,"localhost","root","Abc123!_","Mr_zhou",0,NULL,0);
				//连接成功
				if(mysql_main)
				{
					printf("Connection succeed\n");
					//打印Mysql语句
					printf("slect_head:%s\n",Mysql_Select_all_Head);
					res=mysql_query(mysql_main, Mysql_Select_all_Head); 
					//打印结果
					printf("res:%d\n",res);
					if(res)
					{							
						printf("sqlerror\n");
					}
					else
					{
						printf("sqlok\n");
						//获取结果数据集
						mysqlResult = mysql_store_result(mysql_main);
						//为空则查询失败
						if (mysqlResult == NULL)
						{
							printf(">数据查询失败! %d:%s\n", mysql_errno(mysql_main), mysql_error(mysql_main));
							return;
						}
						else
						{
							if(mysqlRow = mysql_fetch_row(mysqlResult))
							{
								//获取数据
								Mysql_Get_appSocketfd    = mysqlRow[3];
								Mysql_Get_clientSocketfd = mysqlRow[4];
								Mysql_Get_webSocketfd    = mysqlRow[5];
								//打印出各个连接终端的socketfd
								printf("appsocketfd:%d,clientsocketfd:%d,webSocketfd:%d\n", atoi(Mysql_Get_appSocketfd), atoi(Mysql_Get_clientSocketfd), atoi(Mysql_Get_webSocketfd));
								//将应答转发给各个终端
								send(atoi(Mysql_Get_appSocketfd),    Buff_Back_state, sizeof(Buff_Back_state), 0);
								send(atoi(Mysql_Get_clientSocketfd), Buff_Back_state, sizeof(Buff_Back_state), 0);
								send(atoi(Mysql_Get_webSocketfd),    Buff_Back_state, sizeof(Buff_Back_state), 0);
								//printf("go to client0\n");
							}
							//释放数据集
							mysql_free_result(mysqlResult);
						}
					}
				}
				//关闭数据库（操作完要关闭，非常必要）
				mysql_close(mysql_main);
			}
			//前两位不是00,也要发送数据给下位机
			//else
			//{	
				//write 这个函数好像也能用来发数据
			//	send(socketfd, "Recerve ok", 12, 0);
			//}
		}
		//rec_n=0则说明客户端断开
		else if(rec_n==0)
		{
			printf("socket close\n");
			//关闭socket，退出子线程
			close(socketfd);
			pthread_exit(NULL);
		}
	}
}
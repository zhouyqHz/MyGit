#include <stdio.h>
#include <stdlib.h>			//int转char需要
#include <unistd.h>     	//socket close函数所在头文件
#include <sys/socket.h> 	//建立socket相关头文件
#include <netinet/in.h> 	//地址struct相关头文件
#include <mysql/mysql.h>    //数据库相关头文件
#include <string.h>         //字符串拼接头文件
#include <time.h> 			//时间相关头文件(sleep)
#include <pthread.h>		//多线程头文件
#include <fcntl.h>          //文件读写相关库
//数据变量定义区
//mysql操作语句
char Mysql_Insert_FileList_Head[] = "insert into FileList(date, FileName) values('";
char Mysql_Insert_FileList[100] = "";
char Mysql_Value_Data[30] = ""; //放时间的数组
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
//文件下载用的数组
char File_Name[50] = "";
FILE* pf;
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
int Socketfd_Video = 0;   //摄像端点socketfd的记录
char Buff[1024*512];      //socket数据接收缓冲
char Buff_Video[1024*512];//socket视屏接收缓冲
int  rec_n;		//socket接收标志
//建立一个socket监听变量
int listenfd;
//常函数声明
void Mysql_Inil();
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
//常函数
void Mysql_Inil()
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
		exit(0);
	}
}
//子线程处理函数，接收文件列表，接收视屏文件，并保存
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
		//接收数组清零
		bzero(Buff, sizeof(Buff));
		//子线程堵塞在这里（recv函数也会堵塞）
		rec_n = recv(socketfd,Buff,1024*512,0);
		//数据有效
		if(rec_n>0)
		{	
			//BUff数组加入结束符，成为字符串
			Buff[rec_n] = '\0';
			printf("recdata:%s\n",Buff);
			//摄像端点socketfd的更新
			if(Buff[0]=='F'&&Buff[1]=='S')
			{
				Socketfd_Video = socketfd;
				printf("Vicdeo_socketfd:%d\n", Socketfd_Video);
			}
			//如果是文件名的上传
			else if(Buff[0]=='F'&&Buff[1]=='N')
			{
				//mysql初始化
				Mysql_Inil();
				//连接mysql
				mysql_main = mysql_real_connect(mysql_main,"localhost","root","1206","lamptest",0,NULL,0);
				if(mysql_main)
				{
					//连接成功
					printf("Connection succeed\n");
					//处理信息
					//截取出BUff中的其中几位数据(只截取出了日期)
					strncpy(Mysql_Value_Data, Buff+4, 22);
					//下面开始比较数据库中是否有记录
					res=mysql_query(mysql_main,Mysql_Select_Date_All); 
					if(res)
					{							
						printf("select_sqlerror\n");
					}
					else
					{
						printf("select_sqlok\n");
						//获取结果数据集
						mysqlResult = mysql_store_result(mysql_main);
						//为空则查询失败
						if (mysqlResult == NULL)
						{
							printf(">数据查询失败! %d:%s\n", mysql_errno(mysql_main), mysql_error(mysql_main));
							return;
						}
						//这里是检索，数据库是否有树莓派发上来的这个记录
						while((mysqlRow = mysql_fetch_row(mysqlResult))&&Cmp_Bit==1)
						{
							printf("Row:%s\n", mysqlRow[0]);
							printf("Mysql_Value_Data:%s\n", Mysql_Value_Data);
							//当发现有数据库已经有记录
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
						//释放数据集
						mysql_free_result(mysqlResult);
						//当不存在此记录时
						if(Cmp_Bit==1)
						{
							//拼接插入字符串
							stpcpy(Mysql_Insert_FileList, Mysql_Insert_FileList_Head);
							strcat(Mysql_Insert_FileList, Mysql_Value_Data);
							strcat(Mysql_Insert_FileList, "', '");
							strcat(Mysql_Insert_FileList, Mysql_Value_Data);
							strcat(Mysql_Insert_FileList, ".mp4')");
							printf("mysql:%s\n",Mysql_Insert_FileList);
							//做插入操作
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
						//把比较位重新设为1(非常重要,否则判断文件是否存在只会判断一次)
						Cmp_Bit = 1;
						//下发成功接收，并要求继续发送的指令（非空即可）
						send(Socketfd_Video, "FN", 2, 0);
					}

				}
			}
			//树莓派结束文件名数据交换命令
			else if(Buff[0]=='E'&&Buff[1]=='N'&&Buff[2]=='D')
			{
				//关闭数据库（操作完要关闭，非常必要）
				mysql_close(mysql_main);
				printf("File_Name_Update_Done\n");
			}
			//树莓派发来的结束传送文件的指令
			else if(Buff[0]=='V'&&Buff[1]=='E'&&Buff[2]=='N'&&Buff[3]=='D')
			{
				//关闭文件
				fclose(pf);	
			}
			//网页端请求更新文件名
			else if(Buff[0]=='W'&&Buff[1]=='N')
			{
				printf("Web Request Name\n");
				send(Socketfd_Video, "FN", 2, 0);
			}
			//网页端请求下载文件
			else if(Buff[0]=='W'&&Buff[1]=='U')
			{
				strncpy(File_Name, Buff+2, 26);
				File_Name[49]='\0';
				printf("File_Name:%s\n",File_Name);
				//创建mp4文件
				pf	= fopen(File_Name, "wb+");
				if(pf == NULL)  
				{  
					printf("Open file error!\n");  
				}
				//把需要上传的文件下发到树莓派
				send(Socketfd_Video, Buff, sizeof(Buff), 0);
			}
			//send(socketfd, Buff, sizeof(Buff), 0);
			//printf("rev:%d send:%d",socketfd_rec, socketfd_send);
			//写文件
			//fwrite(Buff_Video, sizeof(char), rec_n, pf);
			//当没有任何标志位的时候就是文件
			else
			{
				//写文件
				fwrite(Buff, sizeof(char), rec_n, pf);
			}
		}		
		//if(count>=2)
			//关闭文件，但是该什么时候关呢
		//	fclose(pf);
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
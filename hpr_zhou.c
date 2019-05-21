//除了下位机每半小时的数据格式，其他已经成功
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <mysql.h>   //数据库
#include <time.h>
#include <signal.h>

//kill命令相关
#include<sys/types.h>
#include<signal.h>

//select 函数相关
#include<sys/time.h>
#include<unistd.h>


//消息相关
#include <sys/msg.h>
#include <errno.h>

#define PORT  		5000
#define MAX  10
#define BUFFER_SIZE 1024
#define MSGBUFSIZ 22
//消息测试结构体
struct msg_st
{
    long int msg_type;
    char text[18];
};

#define MAX_TEXT 22


unsigned int sockarr[1000][2]= {0};

int m_receivelength=0;
unsigned char m_cethrebuf[120];
unsigned char msgbuffer[1024];
unsigned char Switch_Message_Rboot[90];
unsigned char msgcommd[30];

////////////////////////////////////////////////////////////////////////
int 	i,j,k,sockfd,newsockfd,is_connected[MAX],fd,writefd,sockclosefd,lampsockfd,lamppid,killret,msgctlret,msgnum;		/*定义相关的变量。*/
int		xtbflag = 0;
struct 	sockaddr_in addr;
int 	addr_len = sizeof(struct sockaddr_in);
fd_set 	myreadfds;
int 	m_picflag=0;
char 	m_picpath[50]="/hp/pic";
int 	m_picfilefid;
char 	chensqlstrout[600];		//刷新和心跳包所用sqlstring
char	xtbsql[100];
char    Mysql_Updata_Location_Head[] = "UPDATE lamp_location SET longitude='10',latitude='10' Where lamp_id='1231'";
char    Mysql_Updata_Location_All[100];
//char 	xtbsqlpre[]="insert into tb_xtb_record(txr_lamp_id,txr_xtb_time,txr_lamp_sockfd,txr_lamp_pid) values(";
char 	Mysql_Updata_Heartbit_Time[100];
char 	Mysql_Updata_Heartbit_Time_Head[] = "UPDATE lamp_state_record SET lamp_heartbit_time='";
char	xtbselectpre[]= "SELECT txr_lamp_id,txr_xtb_time,txr_lamp_sockfd,txr_lamp_pid FROM tb_xtb_record WHERE txr_lamp_id=";
char	xtbselectsuffix[]= " ORDER BY txr_xtb_time DESC LIMIT 1;";
char 	msg[] ="This is the message from server.Connected.\n";
int 	m_aimlength=0;
char 	m_beginpicflag=0;
int 	m_totallen=0;
unsigned char m_flag=0;
//和时间相关
char 	str_t[26] = {0};
struct 	tm* p_time;

char 	fd_arr[4];		//fd的个十百千
char 	m_dizhi[5]; 	//地址
char 	xtb_arr[4];		//心跳包sql地址
char	sockfd_arr[4];	//数据库操作用
char	pid_arr[4];		//数据库操作用
char 	sqlm_dizhi[5]; 	//图片sql地址
char 	m_tel[11];  	//报警电话
char 	m_worktime[12]; //工作时间
int 	m_status=0;		//设置开关状态标志
char 	m_alarm[11];   	//报警上下限设置
char 	m_checktime[12];//时间校准
//经度，纬度
char 	lamp_location_longitude[11] = "";
char 	lamp_location_lantitude[11] = "";

//数据库相关
int 	res;
int 	first_row = 1;
char 	buffer[]="chenxiai test";
char 	chensqlstr[]= "insert into  lamp_data_record(tlr_lamp_id,tlr_air_temperature,tlr_state,tlr_photovoltaic_current,tlr_photovoltaic_voltage,tlr_battery_current,tlr_battery_voltage,tlr_load_current,tlr_air_humidity,tlr_water_temperature,tlr_upload_time,tlr_waring,tlr_alarm_type,tlr_co2_concentation,tlr_soil_moisture)  values(";
char*	txr_lamp_id;
char*	txr_xtb_time;
char*	txr_lamp_sockfd;
char*	txr_lamp_pid;
////////////////////////////////////////////////////////////////////////



void mychulizero( )
{
    //去零
    bzero(m_cethrebuf,sizeof(m_cethrebuf));
    printf("mychuli bzero m_cethrebuf...\n");
    /////////////////////////////////////////////////////
    //地址码即机器编码（tlr_lamp_id）的处理
    /////////////////////////////////////////////////////
    m_cethrebuf[0] = msgbuffer[6];
    m_cethrebuf[1] = msgbuffer[7];
    m_cethrebuf[2] = msgbuffer[8];
    m_cethrebuf[3] = msgbuffer[9];
    m_cethrebuf[4] = msgbuffer[10];
    m_cethrebuf[5] = msgbuffer[11];
    m_cethrebuf[6] = msgbuffer[12];
    m_cethrebuf[7] = msgbuffer[13];
    m_cethrebuf[8] = ',';

    /////////////////////////////////////////////////////
    //空气温度的处理
    /////////////////////////////////////////////////////
    m_cethrebuf[9]  = msgbuffer[39];
    m_cethrebuf[10] = msgbuffer[40];
    m_cethrebuf[11] = '.';
    m_cethrebuf[12] = msgbuffer[41];
    m_cethrebuf[13] = ',';
    /////////////////////////////////////////////////////
    //紫外灯（开/关）的处理
    /////////////////////////////////////////////////////
    m_cethrebuf[14] = msgbuffer[16];
    m_cethrebuf[15] = ',';
    /////////////////////////////////////////////////////
    //光伏电流的处理
    /////////////////////////////////////////////////////
    m_cethrebuf[16] = msgbuffer[24];
    m_cethrebuf[17] = msgbuffer[25];
    m_cethrebuf[18] = '.';
    m_cethrebuf[19] = msgbuffer[26];
    m_cethrebuf[20] = ',';
    /////////////////////////////////////////////////////
    //光伏电压的处理
    /////////////////////////////////////////////////////
    m_cethrebuf[21] = msgbuffer[27];
    m_cethrebuf[22] = msgbuffer[28];
    m_cethrebuf[23] = '.';
    m_cethrebuf[24] = msgbuffer[29];
    m_cethrebuf[25] = ',';
    /////////////////////////////////////////////////////
    //蓄电池电流的处理
    /////////////////////////////////////////////////////
    m_cethrebuf[26] = msgbuffer[30];
    m_cethrebuf[27] = msgbuffer[31];
    m_cethrebuf[28] = '.';
    m_cethrebuf[29] = msgbuffer[32];
    m_cethrebuf[30] = ',';
    /////////////////////////////////////////////////////
    //蓄电池电压的处理
    /////////////////////////////////////////////////////
    m_cethrebuf[31] = msgbuffer[33];
    m_cethrebuf[32] = msgbuffer[34];
    m_cethrebuf[33] = '.';
    m_cethrebuf[34] = msgbuffer[35];
    m_cethrebuf[35] = ',';
    /////////////////////////////////////////////////////
    //负载电流的处理
    /////////////////////////////////////////////////////
    m_cethrebuf[36] = msgbuffer[36];
    m_cethrebuf[37] = msgbuffer[37];
    m_cethrebuf[38] = '.';
    m_cethrebuf[39] = msgbuffer[38];
    m_cethrebuf[40] = ',';
    /////////////////////////////////////////////////////
    //空气湿度的处理
    /////////////////////////////////////////////////////
    m_cethrebuf[41] = msgbuffer[42];
    m_cethrebuf[42] = msgbuffer[43];
    m_cethrebuf[43] = '.';
    m_cethrebuf[44] = msgbuffer[44];
    m_cethrebuf[45] = ',';
    /////////////////////////////////////////////////////
    //水温的处理
    /////////////////////////////////////////////////////
    m_cethrebuf[46] = msgbuffer[45];
    m_cethrebuf[47] = msgbuffer[46];
    m_cethrebuf[48] = '.';
    m_cethrebuf[49] = msgbuffer[47];
    m_cethrebuf[50] = ',';
    /////////////////////////////////////////////////////
    //年分的处理
    /////////////////////////////////////////////////////
    m_cethrebuf[51] = msgbuffer[108];
    m_cethrebuf[52] = msgbuffer[109];
    m_cethrebuf[53] = msgbuffer[110];
    m_cethrebuf[54] = msgbuffer[111];

    /////////////////////////////////////////////////////
    //月日的处理
    /////////////////////////////////////////////////////
    m_cethrebuf[55] = msgbuffer[112];
    m_cethrebuf[56] = msgbuffer[113];
    m_cethrebuf[57] = msgbuffer[114];
    m_cethrebuf[58] = msgbuffer[115];

    /////////////////////////////////////////////////////
    //时分的处理
    /////////////////////////////////////////////////////
    m_cethrebuf[59] = msgbuffer[116];
    m_cethrebuf[60] = msgbuffer[117];
    m_cethrebuf[61] = msgbuffer[118];
    m_cethrebuf[62] = msgbuffer[119];
    m_cethrebuf[63] = '0';
    m_cethrebuf[64] = '0';
    m_cethrebuf[65] = ',';
    /////////////////////////////////////////////////////
    //报警的处理(上面已经都修改完毕)
    /////////////////////////////////////////////////////
    m_cethrebuf[66] = msgbuffer[8];
    m_cethrebuf[67] = ',';

    /////////////////////////////////////////////////////
    //报警种类的处理
    /////////////////////////////////////////////////////
    m_cethrebuf[68] = '\'';
    m_cethrebuf[69] = msgbuffer[9];
    m_cethrebuf[70] = 'a';
    m_cethrebuf[71] = msgbuffer[10];
    m_cethrebuf[72] = 'a';
    m_cethrebuf[73] = msgbuffer[11];
    m_cethrebuf[74] = 'a';
    m_cethrebuf[75] = msgbuffer[12];
    m_cethrebuf[76] = 'a';
    m_cethrebuf[77] = msgbuffer[13];
    m_cethrebuf[78] = 'a';
    m_cethrebuf[79] = msgbuffer[14];
    m_cethrebuf[80] = 'a';
    m_cethrebuf[81] = msgbuffer[15];
    m_cethrebuf[82] = 'a';
    m_cethrebuf[83] = msgbuffer[16];
    m_cethrebuf[84] = 'a';
    m_cethrebuf[85] = msgbuffer[17];
    m_cethrebuf[86] = 'a';
    m_cethrebuf[87] = msgbuffer[18];
    m_cethrebuf[88] = 'a';
    m_cethrebuf[89] = msgbuffer[19];
    m_cethrebuf[90] = 'a';
    m_cethrebuf[91] = msgbuffer[20];
    m_cethrebuf[92] = 'a';
    m_cethrebuf[93] = msgbuffer[21];
    m_cethrebuf[94] = 'a';
    m_cethrebuf[95] = msgbuffer[22];
    m_cethrebuf[96] = '\'';
    m_cethrebuf[97] = ',';

    //CO2浓度
    m_cethrebuf[98] =  msgbuffer[84];
    m_cethrebuf[99] =  msgbuffer[85];
    m_cethrebuf[100] = msgbuffer[86];
    m_cethrebuf[101] = msgbuffer[87];
    m_cethrebuf[102] = ',';

    //土壤湿度
    m_cethrebuf[103] = msgbuffer[48];
    m_cethrebuf[104] = msgbuffer[49];
    m_cethrebuf[105] = '.';
    m_cethrebuf[106] = msgbuffer[50];
    //结束符号
    m_cethrebuf[107] = ')';
	

}
//进程函数
int processthread(int sockfd)
{
    pid_t pid = getpid();
	int m_receivelength;
    //消息相关
    extern int errno;
    fd_set read_sock;   //select
    struct timeval tout;       //select
    int msgrcvflag = 0;
    int msgid = -1;
    struct msg_st data;
    long int msgtype = -1;   // 注意1，接受数据时，类型为0，啥都接受
    struct msqid_ds msg_info;	//msgctl()也可能是msgid_ds
    int m_tpisocket=0;
    int m_ma;
    int m_sendmsid=1;
    tout.tv_sec=120;        //select
    tout.tv_usec=0;         //select

    //数据库相关
    MYSQL 		*mysql_main;
    MYSQL_RES 	*mysqlResult ;
    MYSQL_FIELD *mysqlField ; //保存字段名字信息
    MYSQL_ROW 	mysqlRow;
    mysql_main = mysql_init(NULL);	//??
    if(!mysql_main)
    {
        printf("Connection failed\n");
        fprintf(stderr,"mysql_init failed\n");
        return EXIT_FAILURE;
    }
    mysql_main = mysql_real_connect(mysql_main,"localhost","root","1206","lamptest",0,NULL,0);	//不能放外面
    FD_ZERO(&read_sock);
    FD_SET(sockfd,&read_sock);

    while(1)
    {
        // memset(msgbuffer,0,sizeof(msgbuffer));
        bzero(msgbuffer,sizeof(msgbuffer));	/*清空字符串。*/
        //判断是否退出while(1)
        int ret=0;
		//判断端口可读性
        if((ret=(select(sockfd+1,&read_sock,NULL,NULL,&tout)))<0)
        {
            printf("ERROR SELECT\n");
            break;
        }
        else if(ret==0)
        {
            printf("SELECT TIME OUT\n");
            break;
        }
        else if(ret>0)
        {
            tout.tv_sec=120;        //select
            tout.tv_usec=0;
            printf("READ SELECT\n");
            int m_receivelength = read(sockfd,msgbuffer,sizeof(msgbuffer));			   //read函数是一个阻塞函数
            if(m_receivelength<1)
            {
                if(errno==EINTR)
                {
                    printf("ERROR EINTR");
                    break;
                }
                else
                {
                    printf("ERROR Another Reason");
                    break;
                }
            }
			//加入字符串结束符（很有必要送）
			msgbuffer[m_receivelength] = '\0';
            printf("\nchildprocess msgbuffer: %s\nm_receivelength:%d\nsockfd: %d\n",msgbuffer,m_receivelength,sockfd);


            // //不收图
            // if(m_picflag==0)
            // {
            //1.从终端发来的普通心跳包的处理
            if( (m_receivelength==16)&&(msgbuffer[14]==0x42)&&(msgbuffer[15]==0x01))
            {
                printf("Heart Beat Coming\n");
                //往消息队列中发送数据
                struct msg_st data;
                data.msg_type = 1; // 注意2,发送数据时，类型为1
                xtb_arr[0]=msgbuffer[10];
                xtb_arr[1]=msgbuffer[11];
                xtb_arr[2]=msgbuffer[12];
                xtb_arr[3]=msgbuffer[13];	//多余V1235000

                //往数据库插入心跳包记录

                bzero(xtbsql,sizeof(xtbsql));	/*清空字符串。*/

                struct timeval tv;
                time_t time;
                bzero(str_t,sizeof(str_t));	/*清空字符串。*/
                gettimeofday (&tv, NULL);
                time = tv.tv_sec;
                p_time = localtime(&time);
                strftime(str_t, 26, "%Y-%m-%d %H:%M:%S", p_time);
                printf ("str_t=%s\n", str_t);


                //xtbsql="insert into tb_xtb_record(txr_lamp_id,txr_xtb_time) values(1234,'2018-04-28 09:06:09');";
                sockfd_arr[0]=sockfd/1000+0x30;
                sockfd_arr[1]=(sockfd%1000)/100+0x30;
                sockfd_arr[2]=(sockfd%100)/10+0x30;
                sockfd_arr[3]=sockfd%10+0x30;

                pid_arr[0]=(pid%100000000)/100000000+0x30;
                pid_arr[1]=(pid%10000000)/1000000+0x30;
                pid_arr[2]=(pid%1000000)/100000+0x30;
                pid_arr[3]=(pid%100000)/10000+0x30;
                pid_arr[4]=(pid%10000)/1000+0x30;
                pid_arr[5]=(pid%1000)/100+0x30;
                pid_arr[6]=(pid%100)/10+0x30;
                pid_arr[7]=pid%10+0x30;


				//字符串拼接
               /* strncat(xtbsql, xtbsqlpre,89);
                strncat(xtbsql, xtb_arr,4);
                strncat(xtbsql, ",'",2);
                strncat(xtbsql, str_t,19);
                strncat(xtbsql,"',",2);
                strncat(xtbsql, sockfd_arr,4);
                strncat(xtbsql,",",1);
                strncat(xtbsql, pid_arr,8);
                strncat(xtbsql,");",2);*/
				//字符串拼接
				stpcpy(Mysql_Updata_Heartbit_Time, Mysql_Updata_Heartbit_Time_Head);
				strcat(Mysql_Updata_Heartbit_Time, str_t);
				strcat(Mysql_Updata_Heartbit_Time, "' WHERE lamp_id=");
				strcat(Mysql_Updata_Heartbit_Time, xtb_arr);
				//打印Mysql语句
                printf("Mysql_Updata_Heartbit_Time=%s\n",Mysql_Updata_Heartbit_Time);

				//更新心跳包时间
                if(mysql_main)
                {
                    printf("mysql Connection success\n");
                    m_flag=1;
                    res=mysql_query(mysql_main,Mysql_Updata_Heartbit_Time);
                    printf("res:%d\n",res);
                    if(res)
                    {
                        printf("sqlerror\n\n\n");
                    }
                    else
                    {
                        printf("xtbsqlok\n\n");
                    }

                }
				//本工程中不能关掉，因为不是每个判断都在连接的，本工程只做一次连接
                //mysql_close(mysql_main);
            }

            //2.如果协议长度是154，则是下位机每半小时上传的数据
            else if( (m_receivelength==154)&&(msgbuffer[14]==0x04)&&(msgbuffer[15]==0x01))
            {
				//截取出BUff中的其中GPS定位数据
				//strncpy(lamp_location_longitude, msgbuffer+129, 10);
				//strncpy(lamp_location_lantitude, msgbuffer+142, 10);
				//字符串结束符
				//lamp_location_longitude[10] = '\0';
				//lamp_location_lantitude[10] = '\0';
				
				
                printf("%d\r\n",m_receivelength);
                int i;
                for(i=0; i<81; i++)
                    {
                        printf("msgbuffer:%x\n",msgbuffer[i]);
                    }
                // printf("sdlampfa75 data\n");
                // printf("%x\r\n",msgbuffer[57]);
                // printf("%x\r\n",msgbuffer[58]);
                // printf("%x\r\n",msgbuffer[59]);
                // printf("%x\r\n",msgbuffer[60]);
                
                bzero(chensqlstrout,sizeof(chensqlstrout));	/*清空字符串。*/
                
				mychulizero();
                strcat(chensqlstrout, chensqlstr);
                strcat(chensqlstrout, m_cethrebuf);
                printf("chensqlstrout:%s\n",chensqlstrout);
                printf("m_cethrebuf:%s\n",m_cethrebuf);

                if(mysql_main)
                {
                    printf("mysql Connection success\n");
                    m_flag=1;
                    res=mysql_query(mysql_main,chensqlstrout);
                    printf("res=%d\n",res);
                    if(res)
                    {
                        printf("75sqlerror\n\n\n");
                    }
                    else
                    {
                        printf("75sqlok\n\n\n");
                    }
                }
                //mysql_close(mysql_main);
            }

            //3如果是从网页发来的校准时间命令
            else if( (m_receivelength==20)&&(msgbuffer[0]=='f')&&(msgbuffer[5]=='7')&&(msgbuffer[6]=='0')&&(msgbuffer[7]=='0'))
            {
                printf("2\r\n",m_receivelength,msgbuffer);
                printf("sdwebchecktime\n");
                clock_t starttime, finishtime;
                int duration;
                starttime = clock();

                xtb_arr[0]=msgbuffer[1];
                xtb_arr[1]=msgbuffer[2];
                xtb_arr[2]=msgbuffer[3];
                xtb_arr[3]=msgbuffer[4];

                //读取数据库stm32的sockfd
                strncat(xtbsql,xtbselectpre,98);
                strncat(xtbsql, xtb_arr,4);		//	xtb_arr可能有问题
                strncat(xtbsql, xtbselectsuffix,36);

                printf("xtbsql=%s\n",xtbsql);

                if(mysql_main)
                {
                    printf("mysql Connection success\n");
                    res=mysql_query(mysql_main,xtbsql);
                    //printf("res:%d\n",res);
                    if(res)
                    {
                        printf("sqlerror\n\n\n");
                    }
                    else
                    {
                        printf("xtbsqlok\n");

                        int iNumRow = 0;
                        int iNumField = 0;
                        int i = 0;
                        mysqlResult = mysql_store_result(mysql_main);
                        //printf("mysqlResult:%d\n",mysqlResult);
                        if (mysqlResult == NULL)
                        {
                            printf(">数据查询失败! %d:%s\n", mysql_errno(mysql_main), mysql_error(mysql_main));
                            return;
                        }
                        else
                        {
                            if(mysqlRow = mysql_fetch_row(mysqlResult))
                            {
                                txr_lamp_id 	= mysqlRow[0];
                                txr_xtb_time 	= mysqlRow[1];
                                txr_lamp_sockfd = mysqlRow[2];
                                txr_lamp_pid 	= mysqlRow[3];

                                int inttxr_lamp_id 		= 0;
                                int inttxr_lamp_sockfd	= 0;
                                int inttxr_lamp_pid 	= 0;

                                inttxr_lamp_id 		= atoi(txr_lamp_id);
                                inttxr_lamp_sockfd 	= atoi(txr_lamp_sockfd);
                                inttxr_lamp_pid 	= atoi(txr_lamp_pid);

                                printf("inttxr_lamp_id=%d\ntxr_xtb_time=%s\ninttxr_lamp_sockfd=%d\ninttxr_lamp_pid=%d\n",inttxr_lamp_id,txr_xtb_time,inttxr_lamp_sockfd,inttxr_lamp_pid);

                                mysql_free_result(mysqlResult);

                                //获取心跳包记录时间
                                struct tm tm_time;
                                int xtbsqltime;
                                strptime(txr_xtb_time, "%Y-%m-%d %H:%M:%S", &tm_time);
                                xtbsqltime = mktime(&tm_time);
                                printf("xtbsqltime=%d\n",xtbsqltime);

                                //获取当前时间戳
                                time_t t;
                                t = time(NULL);
                                int nowtime = time(&t);
                                printf("nowtime = %d\n", nowtime);

                                //心跳包记录时间与当前时间比较，若>181秒退出进程
                                double duration;
                                duration = (double)(nowtime - xtbsqltime) / CLOCKS_PER_SEC;
                                if(duration>181)
                                {
                                    //退出杀虫灯进程和网页进程
                                    printf("设备不在线\n\n\n");

                                }
                                else
                                {
                                    //给stm32发命令
                                    bzero(Switch_Message_Rboot,sizeof(Switch_Message_Rboot));
                                    Switch_Message_Rboot[0] = 0x66;
                                    Switch_Message_Rboot[1] = 0x00;
                                    Switch_Message_Rboot[2] = 0x00;
                                    Switch_Message_Rboot[3] = 0x00;
                                    Switch_Message_Rboot[4] = 0x00;
                                    Switch_Message_Rboot[5] = 0x00;

                                    Switch_Message_Rboot[6]  = 0x30;
                                    Switch_Message_Rboot[7]  = 0x30;
                                    Switch_Message_Rboot[8]  = 0x30;
                                    Switch_Message_Rboot[9]  = 0x30;
                                    Switch_Message_Rboot[10] = msgbuffer[1];
                                    Switch_Message_Rboot[11] = msgbuffer[2];
                                    Switch_Message_Rboot[12] = msgbuffer[3];
                                    Switch_Message_Rboot[13] = msgbuffer[4];
                                    Switch_Message_Rboot[14] = 0x10;
                                    Switch_Message_Rboot[15] = 0x04;
                                    Switch_Message_Rboot[16] = msgbuffer[8];
                                    Switch_Message_Rboot[17] = msgbuffer[9];
                                    Switch_Message_Rboot[18] = msgbuffer[10];
                                    Switch_Message_Rboot[19] = msgbuffer[11];
                                    Switch_Message_Rboot[20] = msgbuffer[12];
                                    Switch_Message_Rboot[21] = msgbuffer[13];
                                    Switch_Message_Rboot[22] = msgbuffer[14];
                                    Switch_Message_Rboot[23] = msgbuffer[15];
                                    Switch_Message_Rboot[24] = msgbuffer[16];
                                    Switch_Message_Rboot[25] = msgbuffer[17];
                                    Switch_Message_Rboot[26] = msgbuffer[18];
                                    Switch_Message_Rboot[27] = msgbuffer[19];
                                    //发出命令
                                    int i;
                                    for(i=0; i<28; i++)
                                    {
                                        printf("switch:%x\n",Switch_Message_Rboot[i]);
                                    }
                                    int rew = write(inttxr_lamp_sockfd,Switch_Message_Rboot,28);

                                    //bzero(Switch_Message_Rboot,sizeof(Switch_Message_Rboot));

                                    printf("inttxr_lamp_sockfd=%d\n",inttxr_lamp_sockfd);
                                    printf("rew=%d\n",rew);
                                    if(rew!=28)
                                    {
                                        //write失败时退出进程
                                        printf("checktime to stm32 failed\n");
                                        break;
                                    }
                                    else
                                    {
                                        printf("checktime to stm32 success\n\n");	//设备不在线也能发送成功？？？
                                    }
                                }
                            }
                        }
                    }
                }

                //获取当前时间戳
                time_t t;
                t = time(NULL);
                int currenttime = time(&t);
                printf("currenttime = %d\n", currenttime);

                while(1)
                {
                    //获取当前时间戳
                    // time_t t;
                    t = time(NULL);
                    int finishtime = time(&t);
                    duration = (finishtime - currenttime);
                    printf("duration = %d\n", duration);
                    if(duration>8)
                    {
                        printf("duration=%f\n",duration);
                        //返回断开信息给网页 并结束stm32进程 待处理goto

                        int rew = write(sockfd,"dn",2);
                        printf("rew=%d\n",rew);

                        printf("empty data.text: %sover\n", data.text);
                        bzero(data.text,sizeof(data.text));
                        printf("web sockfd: %d will be closed\n", sockfd);
                        break;
                        /*
                        sockclosefd=shutdown(sockfd,2);
                         printf("web sockclosefd: %d has been closed\n", sockclosefd);
                         killret=kill(lamppid,SIGKILL);	// 成功执行时，返回0。失败返回-1
                         printf("new killret=%d\n\n\n",killret);
                         exit(1);
                         */

                    }
                    else
                    {
                        m_ma= 1000+(msgbuffer[1]-0x30)*1000 + (msgbuffer[2]-0x30)*100 +  (msgbuffer[3]-0x30)*10 +  (msgbuffer[4]-0x30);
                        msgid = msgget((key_t)m_ma, 0666 | IPC_CREAT);
                        printf("%d/r/n",m_ma);
                        // if (msgrcv(msgid, (void *)&data, BUFSIZ, -200, 0) == -1)
                        if (msgrcv(msgid, (void *)&data, BUFSIZ, -200, IPC_NOWAIT) == -1)
                        {
                            //printf("msgrcv failed width erro\n");
                        }
                        else
                        {
                            int rew = write(sockfd,"su",2);
                            printf("web read response rew=%d\n",rew);
                            break;
                        }
                    }

                }
                printf("after web m_ma=%d msgid:%d msgtype=%d\n\n\n",m_ma,msgid,msgtype);

                bzero(msgbuffer,sizeof(msgbuffer));
                break;	//退出网页进程
            }

            //4如果是从网页发来的刷新数据命令(已好)
            else if( (m_receivelength==8)&&(msgbuffer[0]=='f')&&(msgbuffer[5]=='2'))
            {
                printf("3\r\n",m_receivelength,msgbuffer);
                printf("sdwebrefresh\n");

                clock_t starttime, finishtime;
                double duration;
                starttime = clock();

                xtb_arr[0]=msgbuffer[1];
                xtb_arr[1]=msgbuffer[2];
                xtb_arr[2]=msgbuffer[3];
                xtb_arr[3]=msgbuffer[4];

                //读取数据库stm32的sockfd
                strncat(xtbsql,xtbselectpre,98);
                strncat(xtbsql, xtb_arr,4);		//	xtb_arr可能有问题
                strncat(xtbsql, xtbselectsuffix,36);

                printf("xtbsql=%s\n",xtbsql);

                if(mysql_main)
                {
                    printf("mysql Connection success\n");
                    res=mysql_query(mysql_main,xtbsql);
                    //printf("res:%d\n",res);
                    if(res)
                    {
                        printf("sqlerror\n\n\n");
                    }
                    else
                    {
                        printf("xtbsqlok\n");

                        int iNumRow = 0;
                        int iNumField = 0;
                        int i = 0;
                        mysqlResult = mysql_store_result(mysql_main);
                        //printf("mysqlResult:%d\n",mysqlResult);
                        if (mysqlResult == NULL)
                        {
                            printf(">数据查询失败! %d:%s\n", mysql_errno(mysql_main), mysql_error(mysql_main));
                            return;
                        }
                        else
                        {
                            if(mysqlRow = mysql_fetch_row(mysqlResult))
                            {
                                txr_lamp_id 	= mysqlRow[0];
                                txr_xtb_time 	= mysqlRow[1];
                                txr_lamp_sockfd = mysqlRow[2];
                                txr_lamp_pid 	= mysqlRow[3];

                                int inttxr_lamp_id 		= 0;
                                int inttxr_lamp_sockfd	= 0;
                                int inttxr_lamp_pid 	= 0;

                                inttxr_lamp_id 		= atoi(txr_lamp_id);
                                inttxr_lamp_sockfd 	= atoi(txr_lamp_sockfd);
                                inttxr_lamp_pid 	= atoi(txr_lamp_pid);

                                printf("inttxr_lamp_id=%d\ntxr_xtb_time=%s\ninttxr_lamp_sockfd=%d\ninttxr_lamp_pid=%d\n",inttxr_lamp_id,txr_xtb_time,inttxr_lamp_sockfd,inttxr_lamp_pid);

                                mysql_free_result(mysqlResult);

                                //获取心跳包记录时间
                                struct tm tm_time;
                                int xtbsqltime;
                                strptime(txr_xtb_time, "%Y-%m-%d %H:%M:%S", &tm_time);
                                xtbsqltime = mktime(&tm_time);
                                printf("xtbsqltime=%d\n",xtbsqltime);

                                //获取当前时间戳
                                time_t t;
                                t = time(NULL);
                                int nowtime = time(&t);
                                printf("nowtime = %d\n", nowtime);

                                //心跳包记录时间与当前时间比较，若>181秒退出进程
                                double duration;
                                duration = (double)(nowtime - xtbsqltime) / CLOCKS_PER_SEC;
                                if(duration>181)
                                {
                                    //退出杀虫灯进程和网页进程
                                    printf("设备不在线\n\n\n");

                                }
                                else
                                {
                                    //给stm32发命令

                                    //数据采集
                                    Switch_Message_Rboot[0] = 0x66;
                                    Switch_Message_Rboot[1] = 0x00;
                                    Switch_Message_Rboot[2] = 0x00;
                                    Switch_Message_Rboot[3] = 0x00;
                                    Switch_Message_Rboot[4] = 0x00;
                                    Switch_Message_Rboot[5] = 0x00;

                                    Switch_Message_Rboot[6]  = 0x30;
                                    Switch_Message_Rboot[7]  = 0x30;
                                    Switch_Message_Rboot[8]  = 0x30;
                                    Switch_Message_Rboot[9]  = 0x30;
                                    Switch_Message_Rboot[10] = msgbuffer[1];
                                    Switch_Message_Rboot[11] = msgbuffer[2];
                                    Switch_Message_Rboot[12] = msgbuffer[3];
                                    Switch_Message_Rboot[13] = msgbuffer[4];
                                    Switch_Message_Rboot[14] = 0x04;
                                    Switch_Message_Rboot[15] = 0x01;
                                    int i;
                                    for(i=0; i<16; i++)
                                    {
                                        printf("switch:%x\n",Switch_Message_Rboot[i]);
                                    }
                                    int rew = write(inttxr_lamp_sockfd,Switch_Message_Rboot,16);
                                    printf("rew=%d\n",rew);

                                    bzero(Switch_Message_Rboot,sizeof(Switch_Message_Rboot));

                                    if(rew!=16)
                                    {
                                        //write失败时退出进程
                                        printf("checktime to stm32 failed\n");
                                        break;
                                    }
                                    else
                                    {
                                        printf("checktime to stm32 success\n\n");	//设备不在线也能发送成功？？？
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                printf("after web m_ma=%d msgid:%d msgtype=%d\n\n\n",m_ma,msgid,msgtype);

                bzero(msgbuffer,sizeof(msgbuffer));
                break;
            }

            //5如果是从网页发来的设置开关命令(已经成功)
            else if( (m_receivelength==8)&&(msgbuffer[0]=='f')&&(msgbuffer[5]=='4')&&(msgbuffer[6]=='0')&&((msgbuffer[7]=='0')||(msgbuffer[7]=='1')))
            {
                printf("4\r\n",m_receivelength,msgbuffer);
                printf("sdwebstatus\nweblength:%d\nwebmessage:%s \n",m_receivelength,msgbuffer);
                clock_t starttime, finishtime;
                double duration;
                starttime = clock();

                xtb_arr[0]=msgbuffer[1];
                xtb_arr[1]=msgbuffer[2];
                xtb_arr[2]=msgbuffer[3];
                xtb_arr[3]=msgbuffer[4];

                //读取数据库stm32的sockfd
                strncat(xtbsql,xtbselectpre,98);
                strncat(xtbsql, xtb_arr,4);		//	xtb_arr可能有问题
                strncat(xtbsql, xtbselectsuffix,36);

                printf("xtbsql=%s\n",xtbsql);

                if(mysql_main)
                {
                    printf("mysql Connection success\n");
                    res=mysql_query(mysql_main,xtbsql);
                    //printf("res:%d\n",res);
                    if(res)
                    {
                        printf("sqlerror\n\n\n");
                    }
                    else
                    {
                        printf("xtbsqlok\n");

                        int iNumRow = 0;
                        int iNumField = 0;
                        int i = 0;
                        mysqlResult = mysql_store_result(mysql_main);
                        //printf("mysqlResult:%d\n",mysqlResult);
                        if (mysqlResult == NULL)
                        {
                            printf(">数据查询失败! %d:%s\n", mysql_errno(mysql_main), mysql_error(mysql_main));
                            return;
                        }
                        else
                        {
                            if(mysqlRow = mysql_fetch_row(mysqlResult))
                            {
                                txr_lamp_id 	= mysqlRow[0];
                                txr_xtb_time 	= mysqlRow[1];
                                txr_lamp_sockfd = mysqlRow[2];
                                txr_lamp_pid 	= mysqlRow[3];

                                int inttxr_lamp_id 		= 0;
                                int inttxr_lamp_sockfd	= 0;
                                int inttxr_lamp_pid 	= 0;

                                inttxr_lamp_id 		= atoi(txr_lamp_id);
                                inttxr_lamp_sockfd 	= atoi(txr_lamp_sockfd);
                                inttxr_lamp_pid 	= atoi(txr_lamp_pid);

                                printf("inttxr_lamp_id=%d\ntxr_xtb_time=%s\ninttxr_lamp_sockfd=%d\ninttxr_lamp_pid=%d\n",inttxr_lamp_id,txr_xtb_time,inttxr_lamp_sockfd,inttxr_lamp_pid);

                                mysql_free_result(mysqlResult);

                                //获取心跳包记录时间
                                struct tm tm_time;
                                int xtbsqltime;
                                strptime(txr_xtb_time, "%Y-%m-%d %H:%M:%S", &tm_time);
                                xtbsqltime = mktime(&tm_time);
                                printf("xtbsqltime=%d\n",xtbsqltime);

                                //获取当前时间戳
                                time_t t;
                                t = time(NULL);
                                int nowtime = time(&t);
                                printf("nowtime = %d\n", nowtime);

                                //心跳包记录时间与当前时间比较，若>181秒退出进程
                                double duration;
                                duration = (double)(nowtime - xtbsqltime) / CLOCKS_PER_SEC;
                                if(duration>181)
                                {
                                    //退出杀虫灯进程和网页进程
                                    printf("设备不在线\n\n\n");

                                }
                                else
                                {
                                    //给stm32发命令
                                    bzero(Switch_Message_Rboot,sizeof(Switch_Message_Rboot));
                                    Switch_Message_Rboot[0] = 0x66;
                                    Switch_Message_Rboot[1] = 0x00;
                                    Switch_Message_Rboot[2] = 0x00;
                                    Switch_Message_Rboot[3] = 0x00;
                                    Switch_Message_Rboot[4] = 0x00;
                                    Switch_Message_Rboot[5] = 0x00;

                                    Switch_Message_Rboot[6]  = 0x30;
                                    Switch_Message_Rboot[7]  = 0x30;
                                    Switch_Message_Rboot[8]  = 0x30;
                                    Switch_Message_Rboot[9]  = 0x30;
                                    Switch_Message_Rboot[10] = msgbuffer[1];
                                    Switch_Message_Rboot[11] = msgbuffer[2];
                                    Switch_Message_Rboot[12] = msgbuffer[3];
                                    Switch_Message_Rboot[13] = msgbuffer[4];
                                    Switch_Message_Rboot[14] = 0x05;
                                    Switch_Message_Rboot[15] = 0x01;
                                    Switch_Message_Rboot[16] = msgbuffer[7];
                                    //发出命令
                                    int i;
                                    for(i=0; i<17; i++)
                                    {
                                        printf("switch:%x\n",Switch_Message_Rboot[i]);
                                    }
                                    int rew = write(inttxr_lamp_sockfd,Switch_Message_Rboot,17);

                                    //bzero(Switch_Message_Rboot,sizeof(Switch_Message_Rboot));

                                    printf("inttxr_lamp_sockfd=%d\n",inttxr_lamp_sockfd);
                                    printf("rew=%d\n",rew);
                                    if(rew!=17)
                                    {
                                        //write失败时退出进程
                                        printf("checktime to stm32 failed\n");
                                        break;
                                    }
                                    else
                                    {
                                        printf("checktime to stm32 success\n\n");	//设备不在线也能发送成功？？？
                                    }
                                }
                            }
                        }
                    }
                }

                //获取当前时间戳
                time_t t;
                t = time(NULL);
                int currenttime = time(&t);
                printf("currenttime = %d\n", currenttime);

                while(1)
                {
                    //获取当前时间戳
                    // time_t t;
                    t = time(NULL);
                    int finishtime = time(&t);
                    duration = (finishtime - currenttime) ;

                    if(duration>8)
                    {
                        printf("duration=%f\n",duration);
                        //返回断开信息给网页 并结束stm32进程 待处理goto

                        int rew = write(sockfd,"dn",2);
                        printf("rew=%d\n",rew);

                        printf("empty data.text: %sover\n", data.text);
                        bzero(data.text,sizeof(data.text));
                        printf("web sockfd: %d will be closed\n", sockfd);
                        break;
                        // sockclosefd=shutdown(sockfd,2);
                        // printf("web sockclosefd: %d has been closed\n", sockclosefd);
                        // killret=kill(lamppid,SIGKILL);	// 成功执行时，返回0。失败返回-1
                        // printf("new killret=%d\n\n\n",killret);
                        // exit(1);

                    }
                    else
                    {
                        m_ma= 1000+(msgbuffer[1]-0x30)*1000 + (msgbuffer[2]-0x30)*100 +  (msgbuffer[3]-0x30)*10 +  (msgbuffer[4]-0x30);
                        //printf("%d",m_ma);
                        msgid = msgget((key_t)m_ma, 0666 | IPC_CREAT);

                        // if (msgrcv(msgid, (void *)&data, BUFSIZ, -200, 0) == -1)
                        if (msgrcv(msgid, (void *)&data, BUFSIZ, -200, IPC_NOWAIT) == -1)
                        {
                            //printf("msgrcv failed width erro\n");
                        }
                        else
                        {
                            int rew = write(sockfd,"su",2);
                            printf("web read response rew=%d\n",rew);
                            break;
                        }
                    }

                }
                printf("after web m_ma=%d msgid:%d msgtype=%d\n\n\n",m_ma,msgid,msgtype);

                bzero(msgbuffer,sizeof(msgbuffer));

                break;
            }

            //6如果是从网页发来的设置工作时间命令(仅时分)
            else if( (m_receivelength==16)&&(msgbuffer[0]=='f')&&(msgbuffer[14]!=0x42))
            {
                printf("5\r\n",m_receivelength,msgbuffer);
                printf("sdwebworktime weblength:%d\nwebmessage:%s \n\n",m_receivelength,msgbuffer);
                clock_t starttime, finishtime;
                double duration;
                starttime = clock();

                xtb_arr[0]=msgbuffer[1];
                xtb_arr[1]=msgbuffer[2];
                xtb_arr[2]=msgbuffer[3];
                xtb_arr[3]=msgbuffer[4];

                //读取数据库stm32的sockfd
                strncat(xtbsql,xtbselectpre,98);
                strncat(xtbsql, xtb_arr,4);		//	xtb_arr可能有问题
                strncat(xtbsql, xtbselectsuffix,36);

                printf("xtbsql=%s\n",xtbsql);

                if(mysql_main)
                {
                    printf("mysql Connection success\n");
                    res=mysql_query(mysql_main,xtbsql);
                    //printf("res:%d\n",res);
                    if(res)
                    {
                        printf("sqlerror\n\n\n");
                    }
                    else
                    {
                        printf("xtbsqlok\n");

                        int iNumRow = 0;
                        int iNumField = 0;
                        int i = 0;
                        mysqlResult = mysql_store_result(mysql_main);
                        //printf("mysqlResult:%d\n",mysqlResult);
                        if (mysqlResult == NULL)
                        {
                            printf(">数据查询失败! %d:%s\n", mysql_errno(mysql_main), mysql_error(mysql_main));
                            return;
                        }
                        else
                        {
                            if(mysqlRow = mysql_fetch_row(mysqlResult))
                            {
                                txr_lamp_id 	= mysqlRow[0];
                                txr_xtb_time 	= mysqlRow[1];
                                txr_lamp_sockfd = mysqlRow[2];
                                txr_lamp_pid 	= mysqlRow[3];

                                int inttxr_lamp_id 		= 0;
                                int inttxr_lamp_sockfd	= 0;
                                int inttxr_lamp_pid 	= 0;

                                inttxr_lamp_id 		= atoi(txr_lamp_id);
                                inttxr_lamp_sockfd 	= atoi(txr_lamp_sockfd);
                                inttxr_lamp_pid 	= atoi(txr_lamp_pid);

                                printf("inttxr_lamp_id=%d\ntxr_xtb_time=%s\ninttxr_lamp_sockfd=%d\ninttxr_lamp_pid=%d\n",inttxr_lamp_id,txr_xtb_time,inttxr_lamp_sockfd,inttxr_lamp_pid);

                                mysql_free_result(mysqlResult);

                                //获取心跳包记录时间
                                struct tm tm_time;
                                int xtbsqltime;
                                strptime(txr_xtb_time, "%Y-%m-%d %H:%M:%S", &tm_time);
                                xtbsqltime = mktime(&tm_time);
                                printf("xtbsqltime=%d\n",xtbsqltime);

                                //获取当前时间戳
                                time_t t;
                                t = time(NULL);
                                int nowtime = time(&t);
                                printf("nowtime = %d\n", nowtime);

                                //心跳包记录时间与当前时间比较，若>181秒退出进程
                                double duration;
                                duration = (double)(nowtime - xtbsqltime) / CLOCKS_PER_SEC;
                                if(duration>181)
                                {
                                    //退出杀虫灯进程和网页进程
                                    printf("设备不在线\n\n\n");

                                }
                                else
                                {
                                    //给stm32发命令
                                    bzero(Switch_Message_Rboot,sizeof(Switch_Message_Rboot));
                                    Switch_Message_Rboot[0] = 0x66;
                                    Switch_Message_Rboot[1] = 0x00;
                                    Switch_Message_Rboot[2] = 0x00;
                                    Switch_Message_Rboot[3] = 0x00;
                                    Switch_Message_Rboot[4] = 0x00;
                                    Switch_Message_Rboot[5] = 0x00;

                                    Switch_Message_Rboot[6]  = 0x30;
                                    Switch_Message_Rboot[7]  = 0x30;
                                    Switch_Message_Rboot[8]  = 0x30;
                                    Switch_Message_Rboot[9]  = 0x30;
                                    Switch_Message_Rboot[10] = msgbuffer[1];
                                    Switch_Message_Rboot[11] = msgbuffer[2];
                                    Switch_Message_Rboot[12] = msgbuffer[3];
                                    Switch_Message_Rboot[13] = msgbuffer[4];
                                    Switch_Message_Rboot[14] = 0x10;
                                    Switch_Message_Rboot[15] = 0x01;
                                    Switch_Message_Rboot[16] = msgbuffer[8] ;
                                    Switch_Message_Rboot[17] = msgbuffer[9] ;
                                    Switch_Message_Rboot[18] = msgbuffer[10] ;
                                    Switch_Message_Rboot[19] = msgbuffer[11] ;
                                    Switch_Message_Rboot[20] = msgbuffer[12] ;
                                    Switch_Message_Rboot[21] = msgbuffer[13] ;
                                    Switch_Message_Rboot[22] = msgbuffer[14] ;
                                    Switch_Message_Rboot[23] = msgbuffer[15] ;
                                    //发出命令
                                    int i;
                                    for(i=0; i<24; i++)
                                    {
                                        printf("switch:%x\n",Switch_Message_Rboot[i]);
                                    }
                                    int rew = write(inttxr_lamp_sockfd,Switch_Message_Rboot,24);

                                    //bzero(Switch_Message_Rboot,sizeof(Switch_Message_Rboot));

                                    printf("inttxr_lamp_sockfd=%d\n",inttxr_lamp_sockfd);
                                    printf("rew=%d\n",rew);
                                    if(rew!=24)
                                    {
                                        //write失败时退出进程
                                        printf("checktime to stm32 failed\n");
                                        break;
                                    }
                                    else
                                    {
                                        printf("checktime to stm32 success\n\n");	//设备不在线也能发送成功？？？
                                    }
                                }
                            }
                        }
                    }
                }

                //获取当前时间戳
                time_t t;
                t = time(NULL);
                int currenttime = time(&t);
                printf("currenttime = %d\n", currenttime);

                while(1)
                {
                    //获取当前时间戳
                    // time_t t;
                    t = time(NULL);
                    int finishtime = time(&t);
                    duration = (finishtime - currenttime) ;

                    if(duration>8)
                    {
                        printf("duration=%f\n",duration);
                        //返回断开信息给网页 并结束stm32进程 待处理goto

                        int rew = write(sockfd,"dn",2);
                        printf("rew=%d\n",rew);

                        printf("empty data.text: %sover\n", data.text);
                        bzero(data.text,sizeof(data.text));
                        printf("web sockfd: %d will be closed\n", sockfd);
                        break;
                        // sockclosefd=shutdown(sockfd,2);
                        // printf("web sockclosefd: %d has been closed\n", sockclosefd);
                        // killret=kill(lamppid,SIGKILL);	// 成功执行时，返回0。失败返回-1
                        // printf("new killret=%d\n\n\n",killret);
                        // exit(1);

                    }
                    else
                    {
                        m_ma= 1000+(msgbuffer[1]-0x30)*1000 + (msgbuffer[2]-0x30)*100 +  (msgbuffer[3]-0x30)*10 +  (msgbuffer[4]-0x30);
                        msgid = msgget((key_t)m_ma, 0666 | IPC_CREAT);

                        // if (msgrcv(msgid, (void *)&data, BUFSIZ, -200, 0) == -1)
                        if (msgrcv(msgid, (void *)&data, BUFSIZ, -200, IPC_NOWAIT) == -1)
                        {
                            //printf("msgrcv failed width erro\n");
                        }
                        else
                        {
                            int rew = write(sockfd,data.text,2);
                            printf("web read response rew=%d\n",rew);
                            break;
                        }
                    }
                }
                printf("after web m_ma=%d msgid:%d msgtype=%d\n\n\n",m_ma,msgid,msgtype);

                bzero(msgbuffer,sizeof(msgbuffer));

                break;
            }

            //7如果是从网页发来的设置报警范围命令
            else if( (m_receivelength==35)&&(msgbuffer[0]=='f')&&(msgbuffer[5]=='6'))
            {
                printf("6\r\n",m_receivelength,msgbuffer);
                printf("sdwebthreshold\nweblength:%d\n,webmessage:%s \n",m_receivelength,msgbuffer);
                clock_t starttime, finishtime;
                double duration;
                starttime = clock();

                xtb_arr[0]=msgbuffer[1];
                xtb_arr[1]=msgbuffer[2];
                xtb_arr[2]=msgbuffer[3];
                xtb_arr[3]=msgbuffer[4];

                //读取数据库stm32的sockfd
                strncat(xtbsql,xtbselectpre,98);
                strncat(xtbsql, xtb_arr,4);		//	xtb_arr可能有问题
                strncat(xtbsql, xtbselectsuffix,36);

                printf("xtbsql=%s\n",xtbsql);

                if(mysql_main)
                {
                    printf("mysql Connection success\n");
                    res=mysql_query(mysql_main,xtbsql);
                    //printf("res:%d\n",res);
                    if(res)
                    {
                        printf("sqlerror\n\n\n");
                    }
                    else
                    {
                        printf("xtbsqlok\n");

                        int iNumRow = 0;
                        int iNumField = 0;
                        int i = 0;
                        mysqlResult = mysql_store_result(mysql_main);
                        //printf("mysqlResult:%d\n",mysqlResult);
                        if (mysqlResult == NULL)
                        {
                            printf(">数据查询失败! %d:%s\n", mysql_errno(mysql_main), mysql_error(mysql_main));
                            return;
                        }
                        else
                        {
                            if(mysqlRow = mysql_fetch_row(mysqlResult))
                            {
                                txr_lamp_id 	= mysqlRow[0];
                                txr_xtb_time 	= mysqlRow[1];
                                txr_lamp_sockfd = mysqlRow[2];
                                txr_lamp_pid 	= mysqlRow[3];

                                int inttxr_lamp_id 		= 0;
                                int inttxr_lamp_sockfd	= 0;
                                int inttxr_lamp_pid 	= 0;

                                inttxr_lamp_id 		= atoi(txr_lamp_id);
                                inttxr_lamp_sockfd 	= atoi(txr_lamp_sockfd);
                                inttxr_lamp_pid 	= atoi(txr_lamp_pid);

                                printf("inttxr_lamp_id=%d\ntxr_xtb_time=%s\ninttxr_lamp_sockfd=%d\ninttxr_lamp_pid=%d\n",inttxr_lamp_id,txr_xtb_time,inttxr_lamp_sockfd,inttxr_lamp_pid);

                                mysql_free_result(mysqlResult);

                                //获取心跳包记录时间
                                struct tm tm_time;
                                int xtbsqltime;
                                strptime(txr_xtb_time, "%Y-%m-%d %H:%M:%S", &tm_time);
                                xtbsqltime = mktime(&tm_time);
                                printf("xtbsqltime=%d\n",xtbsqltime);

                                //获取当前时间戳
                                time_t t;
                                t = time(NULL);
                                int nowtime = time(&t);
                                printf("nowtime = %d\n", nowtime);

                                //心跳包记录时间与当前时间比较，若>181秒退出进程
                                double duration;
                                duration = (double)(nowtime - xtbsqltime) / CLOCKS_PER_SEC;
                                if(duration>181)
                                {
                                    //退出杀虫灯进程和网页进程
                                    printf("设备不在线\n\n\n");

                                }
                                else
                                {
                                    //给stm32发命令
                                    bzero(Switch_Message_Rboot,sizeof(Switch_Message_Rboot));
                                    Switch_Message_Rboot[0] = 0x66;
                                    Switch_Message_Rboot[1] = 0x00;
                                    Switch_Message_Rboot[2] = 0x00;
                                    Switch_Message_Rboot[3] = 0x00;
                                    Switch_Message_Rboot[4] = 0x00;
                                    Switch_Message_Rboot[5] = 0x00;

                                    Switch_Message_Rboot[6]  = 0x30;
                                    Switch_Message_Rboot[7]  = 0x30;
                                    Switch_Message_Rboot[8]  = 0x30;
                                    Switch_Message_Rboot[9]  = 0x30;
                                    Switch_Message_Rboot[10] = msgbuffer[1];
                                    Switch_Message_Rboot[11] = msgbuffer[2];
                                    Switch_Message_Rboot[12] = msgbuffer[3];
                                    Switch_Message_Rboot[13] = msgbuffer[4];
                                    Switch_Message_Rboot[14] = 0x10;
                                    Switch_Message_Rboot[15] = 0x03;

                                    Switch_Message_Rboot[16] = 0x30;
                                    Switch_Message_Rboot[17] = 0x35;
                                    Switch_Message_Rboot[18] = 0x30;

                                    Switch_Message_Rboot[19] = 0x31;
                                    Switch_Message_Rboot[20] = 0x30;
                                    Switch_Message_Rboot[21] = 0x38;

                                    Switch_Message_Rboot[22] = 0x31;
                                    Switch_Message_Rboot[23] = 0x34;
                                    Switch_Message_Rboot[24] = 0x34;


                                    Switch_Message_Rboot[25] = msgbuffer[17];
                                    Switch_Message_Rboot[26] = msgbuffer[18];
                                    Switch_Message_Rboot[27] = msgbuffer[19];

                                    Switch_Message_Rboot[28] = msgbuffer[20];
                                    Switch_Message_Rboot[29] = msgbuffer[21];
                                    Switch_Message_Rboot[30] = msgbuffer[22];

                                    Switch_Message_Rboot[31] = msgbuffer[23];
                                    Switch_Message_Rboot[32] = msgbuffer[24];
                                    Switch_Message_Rboot[33] = msgbuffer[25];

                                    Switch_Message_Rboot[34] = msgbuffer[26];
                                    Switch_Message_Rboot[35] = msgbuffer[27];
                                    Switch_Message_Rboot[36] = msgbuffer[28];

                                    Switch_Message_Rboot[37] = msgbuffer[29];
                                    Switch_Message_Rboot[38] = msgbuffer[30];
                                    Switch_Message_Rboot[39] = msgbuffer[31];

                                    Switch_Message_Rboot[40] = msgbuffer[32];
                                    Switch_Message_Rboot[41] = msgbuffer[33];
                                    Switch_Message_Rboot[42] = msgbuffer[34];

                                    Switch_Message_Rboot[43] = 0x30;//PH下
                                    Switch_Message_Rboot[44] = 0x35;
                                    Switch_Message_Rboot[45] = 0x30;

                                    Switch_Message_Rboot[46] = 0x31;//PH上
                                    Switch_Message_Rboot[47] = 0x32;
                                    Switch_Message_Rboot[48] = 0x30;

                                    Switch_Message_Rboot[49] = 0x31;//土壤下
                                    Switch_Message_Rboot[50] = 0x35;
                                    Switch_Message_Rboot[51] = 0x35;

                                    Switch_Message_Rboot[52] = 0x33;//土壤上
                                    Switch_Message_Rboot[53] = 0x32;
                                    Switch_Message_Rboot[54] = 0x30;

                                    Switch_Message_Rboot[55] = 0x31;//N下
                                    Switch_Message_Rboot[56] = 0x30;
                                    Switch_Message_Rboot[57] = 0x30;

                                    Switch_Message_Rboot[58] = 0x38;
                                    Switch_Message_Rboot[59] = 0x30;//N上
                                    Switch_Message_Rboot[60] = 0x30;

                                    Switch_Message_Rboot[61] = 0x38;//P下
                                    Switch_Message_Rboot[62] = 0x30;
                                    Switch_Message_Rboot[63] = 0x30;

                                    Switch_Message_Rboot[64] = 0x38;
                                    Switch_Message_Rboot[65] = 0x30;//P上
                                    Switch_Message_Rboot[66] = 0x30;

                                    Switch_Message_Rboot[67] = 0x31;//K下
                                    Switch_Message_Rboot[68] = 0x30;
                                    Switch_Message_Rboot[69] = 0x30;

                                    Switch_Message_Rboot[70] = 0x38;//K上
                                    Switch_Message_Rboot[71] = 0x30;
                                    Switch_Message_Rboot[72] = 0x30;



                                    Switch_Message_Rboot[73] = 0x31;//CO2下
                                    Switch_Message_Rboot[74] = 0x30;
                                    Switch_Message_Rboot[75] = 0x30;
                                    Switch_Message_Rboot[76] = 0x30;

                                    Switch_Message_Rboot[77] = 0x34;
                                    Switch_Message_Rboot[78] = 0x30;//CO2上
                                    Switch_Message_Rboot[79] = 0x30;
                                    Switch_Message_Rboot[80] = 0x30;




                                    //发出命令
                                    int i;
                                    for(i=0; i<81; i++)
                                    {
                                        printf("switch:%x\n",Switch_Message_Rboot[i]);
                                    }
                                    int rew = write(inttxr_lamp_sockfd,Switch_Message_Rboot,81);

                                    //bzero(Switch_Message_Rboot,sizeof(Switch_Message_Rboot));

                                    printf("inttxr_lamp_sockfd=%d\n",inttxr_lamp_sockfd);
                                    printf("rew=%d\n",rew);
                                    if(rew!=81)
                                    {
                                        //write失败时退出进程
                                        printf("checktime to stm32 failed\n");
                                        break;
                                    }
                                    else
                                    {
                                        printf("checktime to stm32 success\n\n");	//设备不在线也能发送成功？？？
                                    }
                                }
                            }
                        }
                    }
                }

                //获取当前时间戳
                time_t t;
                t = time(NULL);
                int currenttime = time(&t);
                printf("currenttime = %d\n", currenttime);

                while(1)
                {
                    //获取当前时间戳
                    // time_t t;
                    t = time(NULL);
                    int finishtime = time(&t);
                    duration = (finishtime - currenttime);

                    if(duration>8)
                    {
                        printf("duration=%f\n",duration);
                        //返回断开信息给网页 并结束stm32进程 待处理goto

                        int rew = write(sockfd,"dn",2);
                        printf("rew=%d\n",rew);

                        printf("empty data.text: %sover\n", data.text);
                        bzero(data.text,sizeof(data.text));
                        printf("web sockfd: %d will be closed\n", sockfd);
                        break;
                        // sockclosefd=shutdown(sockfd,2);
                        // printf("web sockclosefd: %d has been closed\n", sockclosefd);
                        // killret=kill(lamppid,SIGKILL);	// 成功执行时，返回0。失败返回-1
                        // printf("new killret=%d\n\n\n",killret);
                        // exit(1);

                    }
                    else
                    {
                        m_ma= 1000+(msgbuffer[1]-0x30)*1000 + (msgbuffer[2]-0x30)*100 +  (msgbuffer[3]-0x30)*10 +  (msgbuffer[4]-0x30);
                        msgid = msgget((key_t)m_ma, 0666 | IPC_CREAT);

                        // if (msgrcv(msgid, (void *)&data, BUFSIZ, -200, 0) == -1)
                        if (msgrcv(msgid, (void *)&data, BUFSIZ, -200, IPC_NOWAIT) == -1)
                        {
                            //printf("msgrcv failed width erro\n");
                        }
                        else
                        {
                            int rew = write(sockfd,data.text,2);
                            printf("web read response rew=%d\n",rew);
                            break;
                        }
                    }
                }
                printf("after web m_ma=%d msgid:%d msgtype=%d\n\n\n",m_ma,msgid,msgtype);

                bzero(msgbuffer,sizeof(msgbuffer));

                break;
            }

            //8.如果是从网页发来的设置电话命令
            else if( (m_receivelength==21)&&(msgbuffer[0]=='s')&&(msgbuffer[18]=='f'))
            {
                printf("7\r\n",m_receivelength,msgbuffer);
                printf("sdwebfatel\nweblength:%d\nwebmessage:%s\n",m_receivelength,msgbuffer);
                clock_t starttime, finishtime;
                double duration;
                starttime = clock();

                xtb_arr[0]=msgbuffer[1];
                xtb_arr[1]=msgbuffer[2];
                xtb_arr[2]=msgbuffer[3];
                xtb_arr[3]=msgbuffer[4];

                //读取数据库stm32的sockfd
                strncat(xtbsql,xtbselectpre,98);
                strncat(xtbsql, xtb_arr,4);		//	xtb_arr可能有问题
                strncat(xtbsql, xtbselectsuffix,36);

                printf("xtbsql=%s\n",xtbsql);

                if(mysql_main)
                {
                    printf("mysql Connection success\n");
                    res=mysql_query(mysql_main,xtbsql);
                    //printf("res:%d\n",res);
                    if(res)
                    {
                        printf("sqlerror\n\n\n");
                    }
                    else
                    {
                        printf("xtbsqlok\n");

                        int iNumRow = 0;
                        int iNumField = 0;
                        int i = 0;
                        mysqlResult = mysql_store_result(mysql_main);
                        //printf("mysqlResult:%d\n",mysqlResult);
                        if (mysqlResult == NULL)
                        {
                            printf(">数据查询失败! %d:%s\n", mysql_errno(mysql_main), mysql_error(mysql_main));
                            return;
                        }
                        else
                        {
                            if(mysqlRow = mysql_fetch_row(mysqlResult))
                            {
                                txr_lamp_id 	= mysqlRow[0];
                                txr_xtb_time 	= mysqlRow[1];
                                txr_lamp_sockfd = mysqlRow[2];
                                txr_lamp_pid 	= mysqlRow[3];

                                int inttxr_lamp_id 		= 0;
                                int inttxr_lamp_sockfd	= 0;
                                int inttxr_lamp_pid 	= 0;

                                inttxr_lamp_id 		= atoi(txr_lamp_id);
                                inttxr_lamp_sockfd 	= atoi(txr_lamp_sockfd);
                                inttxr_lamp_pid 	= atoi(txr_lamp_pid);

                                printf("inttxr_lamp_id=%d\ntxr_xtb_time=%s\ninttxr_lamp_sockfd=%d\ninttxr_lamp_pid=%d\n",inttxr_lamp_id,txr_xtb_time,inttxr_lamp_sockfd,inttxr_lamp_pid);

                                mysql_free_result(mysqlResult);

                                //获取心跳包记录时间
                                struct tm tm_time;
                                int xtbsqltime;
                                strptime(txr_xtb_time, "%Y-%m-%d %H:%M:%S", &tm_time);
                                xtbsqltime = mktime(&tm_time);
                                printf("xtbsqltime=%d\n",xtbsqltime);

                                //获取当前时间戳
                                time_t t;
                                t = time(NULL);
                                int nowtime = time(&t);
                                printf("nowtime = %d\n", nowtime);

                                //心跳包记录时间与当前时间比较，若>181秒退出进程
                                double duration;
                                duration = (double)(nowtime - xtbsqltime) / CLOCKS_PER_SEC;
                                if(duration>181)
                                {
                                    //退出杀虫灯进程和网页进程
                                    printf("设备不在线\n\n\n");

                                }
                                else
                                {
                                    //给stm32发命令
                                    bzero(Switch_Message_Rboot,sizeof(Switch_Message_Rboot));
                                    Switch_Message_Rboot[0] = 0x66;
                                    Switch_Message_Rboot[1] = 0x00;
                                    Switch_Message_Rboot[2] = 0x00;
                                    Switch_Message_Rboot[3] = 0x00;
                                    Switch_Message_Rboot[4] = 0x00;
                                    Switch_Message_Rboot[5] = 0x00;

                                    Switch_Message_Rboot[6]  = 0x30;
                                    Switch_Message_Rboot[7]  = 0x30;
                                    Switch_Message_Rboot[8]  = 0x30;
                                    Switch_Message_Rboot[9]  = 0x30;
                                    Switch_Message_Rboot[10] = msgbuffer[1];
                                    Switch_Message_Rboot[11] = msgbuffer[2];
                                    Switch_Message_Rboot[12] = msgbuffer[3];
                                    Switch_Message_Rboot[13] = msgbuffer[4];
                                    Switch_Message_Rboot[14] = 0x10;
                                    Switch_Message_Rboot[15] = 0x02;
                                    Switch_Message_Rboot[16] = msgbuffer[6];
                                    Switch_Message_Rboot[17] = msgbuffer[7];
                                    Switch_Message_Rboot[18] = msgbuffer[8];
                                    Switch_Message_Rboot[19] = msgbuffer[9];
                                    Switch_Message_Rboot[20] = msgbuffer[10];
                                    Switch_Message_Rboot[21] = msgbuffer[11];
                                    Switch_Message_Rboot[22] = msgbuffer[12];
                                    Switch_Message_Rboot[23] = msgbuffer[13];
                                    Switch_Message_Rboot[24] = msgbuffer[14];
                                    Switch_Message_Rboot[25] = msgbuffer[15];
                                    Switch_Message_Rboot[26] = msgbuffer[16];

                                    //发出命令
                                    int i;
                                    for(i=0; i<27; i++)
                                    {
                                        printf("switch:%x\n",Switch_Message_Rboot[i]);
                                    }
                                    int rew = write(inttxr_lamp_sockfd,Switch_Message_Rboot,27);

                                    //bzero(Switch_Message_Rboot,sizeof(Switch_Message_Rboot));

                                    printf("inttxr_lamp_sockfd=%d\n",inttxr_lamp_sockfd);
                                    printf("rew=%d\n",rew);
                                    if(rew!=27)
                                    {
                                        //write失败时退出进程
                                        printf("checktime to stm32 failed\n");
                                        break;
                                    }
                                    else
                                    {
                                        printf("checktime to stm32 success\n\n");	//设备不在线也能发送成功？？？
                                    }
                                }
                            }
                        }
                    }
                }

                //获取当前时间戳
                time_t t;
                t = time(NULL);
                int currenttime = time(&t);
                printf("currenttime = %d\n", currenttime);

                while(1)
                {
                    //获取当前时间戳
                    // time_t t;
                    t = time(NULL);
                    int finishtime = time(&t);
                    duration = (finishtime - currenttime);

                    if(duration>8)
                    {
                        printf("duration=%f\n",duration);
                        //返回断开信息给网页 并结束stm32进程 待处理goto

                        int rew = write(sockfd,"dn",2);
                        printf("rew=%d\n",rew);

                        printf("empty data.text: %sover\n", data.text);
                        bzero(data.text,sizeof(data.text));
                        printf("web sockfd: %d will be closed\n", sockfd);
                        break;
                        // sockclosefd=shutdown(sockfd,2);
                        // printf("web sockclosefd: %d has been closed\n", sockclosefd);
                        // killret=kill(lamppid,SIGKILL);	// 成功执行时，返回0。失败返回-1
                        // printf("new killret=%d\n\n\n",killret);
                        // exit(1);

                    }
                    else
                    {
                        m_ma= 1000+(msgbuffer[1]-0x30)*1000 + (msgbuffer[2]-0x30)*100 +  (msgbuffer[3]-0x30)*10 +  (msgbuffer[4]-0x30);
                        msgid = msgget((key_t)m_ma, 0666 | IPC_CREAT);

                        // if (msgrcv(msgid, (void *)&data, BUFSIZ, -200, 0) == -1)
                        if (msgrcv(msgid, (void *)&data, BUFSIZ, -200, IPC_NOWAIT) == -1)
                        {
                            //printf("msgrcv failed width erro\n");
                        }
                        else
                        {
                            int rew = write(sockfd,data.text,2);
                            printf("web read response rew=%d\n",rew);
                            break;
                        }
                    }
                }
                printf("after web m_ma=%d msgid:%d msgtype=%d\n\n\n",m_ma,msgid,msgtype);

                bzero(msgbuffer,sizeof(msgbuffer));

                break;
            }

            //9.如果是从网页发来的上传图片命令
            else if( (m_receivelength==21)&&(msgbuffer[0]=='s')&&(msgbuffer[18]=='a'))
            {
                printf("8\r\n",m_receivelength,msgbuffer);
                printf("sdwebfatu\n");
                clock_t starttime, finishtime;
                double duration;
                starttime = clock();

                xtb_arr[0]=msgbuffer[1];
                xtb_arr[1]=msgbuffer[2];
                xtb_arr[2]=msgbuffer[3];
                xtb_arr[3]=msgbuffer[4];

                //读取数据库stm32的sockfd
                strncat(xtbsql,xtbselectpre,98);
                strncat(xtbsql, xtb_arr,4);		//	xtb_arr可能有问题
                strncat(xtbsql, xtbselectsuffix,36);

                printf("xtbsql=%s\n",xtbsql);

                if(mysql_main)
                {
                    printf("mysql Connection success\n");
                    res=mysql_query(mysql_main,xtbsql);
                    //printf("res:%d\n",res);
                    if(res)
                    {
                        printf("sqlerror\n\n\n");
                    }
                    else
                    {
                        printf("xtbsqlok\n");

                        int iNumRow = 0;
                        int iNumField = 0;
                        int i = 0;
                        mysqlResult = mysql_store_result(mysql_main);
                        //printf("mysqlResult:%d\n",mysqlResult);
                        if (mysqlResult == NULL)
                        {
                            printf(">数据查询失败! %d:%s\n", mysql_errno(mysql_main), mysql_error(mysql_main));
                            return;
                        }
                        else
                        {
                            if(mysqlRow = mysql_fetch_row(mysqlResult))
                            {
                                txr_lamp_id 	= mysqlRow[0];
                                txr_xtb_time 	= mysqlRow[1];
                                txr_lamp_sockfd = mysqlRow[2];
                                txr_lamp_pid 	= mysqlRow[3];

                                int inttxr_lamp_id 		= 0;
                                int inttxr_lamp_sockfd	= 0;
                                int inttxr_lamp_pid 	= 0;

                                inttxr_lamp_id 		= atoi(txr_lamp_id);
                                inttxr_lamp_sockfd 	= atoi(txr_lamp_sockfd);
                                inttxr_lamp_pid 	= atoi(txr_lamp_pid);

                                printf("inttxr_lamp_id=%d\ntxr_xtb_time=%s\ninttxr_lamp_sockfd=%d\ninttxr_lamp_pid=%d\n",inttxr_lamp_id,txr_xtb_time,inttxr_lamp_sockfd,inttxr_lamp_pid);

                                mysql_free_result(mysqlResult);

                                //获取心跳包记录时间
                                struct tm tm_time;
                                int xtbsqltime;
                                strptime(txr_xtb_time, "%Y-%m-%d %H:%M:%S", &tm_time);
                                xtbsqltime = mktime(&tm_time);
                                printf("xtbsqltime=%d\n",xtbsqltime);

                                //获取当前时间戳
                                time_t t;
                                t = time(NULL);
                                int nowtime = time(&t);
                                printf("nowtime = %d\n", nowtime);

                                //心跳包记录时间与当前时间比较，若>181秒退出进程
                                double duration;
                                duration = (double)(nowtime - xtbsqltime) / CLOCKS_PER_SEC;
                                if(duration>181)
                                {
                                    //退出杀虫灯进程和网页进程
                                    printf("设备不在线\n\n\n");

                                }
                                else
                                {
                                    //给stm32发命令
                                    bzero(Switch_Message_Rboot,sizeof(Switch_Message_Rboot));
                                    Switch_Message_Rboot[0] = 0x66;
                                    Switch_Message_Rboot[1] = 0x00;
                                    Switch_Message_Rboot[2] = 0x00;
                                    Switch_Message_Rboot[3] = 0x00;
                                    Switch_Message_Rboot[4] = 0x00;
                                    Switch_Message_Rboot[5] = 0x00;

                                    Switch_Message_Rboot[6]  = 0x30;
                                    Switch_Message_Rboot[7]  = 0x30;
                                    Switch_Message_Rboot[8]  = 0x30;
                                    Switch_Message_Rboot[9]  = 0x30;
                                    Switch_Message_Rboot[10] = msgbuffer[1];
                                    Switch_Message_Rboot[11] = msgbuffer[2];
                                    Switch_Message_Rboot[12] = msgbuffer[3];
                                    Switch_Message_Rboot[13] = msgbuffer[4];
                                    Switch_Message_Rboot[14] = 0x41;
                                    Switch_Message_Rboot[15] = 0x01;
                                    //发出命令
                                    int i;
                                    for(i=0; i<16; i++)
                                    {
                                        printf("switch:%x\n",Switch_Message_Rboot[i]);
                                    }
                                    int rew = write(inttxr_lamp_sockfd,Switch_Message_Rboot,16);

                                    //bzero(Switch_Message_Rboot,sizeof(Switch_Message_Rboot));

                                    printf("inttxr_lamp_sockfd=%d\n",inttxr_lamp_sockfd);
                                    printf("rew=%d\n",rew);
                                    if(rew!=16)
                                    {
                                        //write失败时退出进程
                                        printf("checktime to stm32 failed\n");
                                        break;
                                    }
                                    else
                                    {
                                        printf("checktime to stm32 success\n\n");	//设备不在线也能发送成功？？？
                                    }
                                }
                            }
                        }
                    }
                }
                printf("after web m_ma=%d msgid:%d msgtype=%d\n\n\n",m_ma,msgid,msgtype);
                bzero(msgbuffer,sizeof(msgbuffer));
                break;	//结束网页链接
            }

            //10.如果协议长度是8位的应答
            //else if( (m_receivelength==8)&&(msgbuffer[0]=='f')&&(((msgbuffer[6]=='s')&&(msgbuffer[7]=='u'))||((msgbuffer[6]=='e')&&(msgbuffer[7]=='r'))))
            else if( (msgbuffer[0]==0x66)&&(msgbuffer[1]==0x00)&&(msgbuffer[14]!=0x42)&&(m_receivelength!=19))
            {
                printf("9\r\n",m_receivelength,msgbuffer);
                printf("response 8\nweblength:%d\nwebmessage:%s\n",m_receivelength,msgbuffer);

                m_dizhi[0] = msgbuffer[10];
                m_dizhi[1] = msgbuffer[11];
                m_dizhi[2] = msgbuffer[12];
                m_dizhi[3] = msgbuffer[13];


                //向网页返回数据
                msgcommd[0]=msgbuffer[0];
                msgcommd[1]=msgbuffer[1];
                msgcommd[2]=msgbuffer[2];
                msgcommd[3]=msgbuffer[3];
                msgcommd[4]=msgbuffer[4];
                msgcommd[5]=msgbuffer[5];
                msgcommd[6]='s';
                msgcommd[7]='u';

                for(j=0; j<10; j++)
                {
                    printf("data.text[%d]=%c\n",j,data.text[j]);
                }

                bzero(msgbuffer,sizeof(msgbuffer));	/*清空字符串。*/
                //bzero(data.text,sizeof(data.text));	/*清空字符串。*/
                //往消息队列中发送数据
                struct msg_st data;
                data.msg_type = 1; // 注意2,发送数据时，类型为1 ??
                data.text[0] = msgcommd[6];
                data.text[1] = msgcommd[7];

                // 向队列里发送数据
                m_ma= 1000+(m_dizhi[0]-0x30)*1000 + (m_dizhi[1]-0x30)*100 +  (m_dizhi[2]-0x30)*10 +  (m_dizhi[3]-0x30);
                printf("after 8 response send to web m_ma=%d\n",m_ma);
                msgid = msgget((key_t)m_ma, 0666 | IPC_CREAT);

                if (msgsnd(msgid, (void *)&data, MAX_TEXT, 0) == -1)   //msgflg 0?
                {
                    printf("msgrcv failed width erro\n");
                    exit(EXIT_FAILURE);
                }
                else
                {
                    printf("after 8 response send to web data.text: %s\n", data.text);
                }

                //bzero(data.text,sizeof(data.text));	/*清空字符串。*/
                printf("empty data.text: %sover\n\n\n", data.text);
            }

            //11.如果是从网页发来的视频开关命令
            else if((m_receivelength==1)&&((msgbuffer[0]=='g')||(msgbuffer[0]=='t')))
            {
                printf("10\r\n",m_receivelength,msgbuffer);
                printf("sdwebvideostatus\nweblength:%d\nwebmessage:%s \n",m_receivelength,msgbuffer);
                // clock_t starttime, finishtime;
                // double duration;
                // starttime = clock();

                xtb_arr[0]=msgbuffer[1];
                xtb_arr[1]=msgbuffer[2];
                xtb_arr[2]=msgbuffer[3];
                xtb_arr[3]=msgbuffer[4];

                //读取数据库stm32的sockfd
                strncat(xtbsql,xtbselectpre,98);
                strncat(xtbsql, "1234",4);		//	xtb_arr可能有问题
                strncat(xtbsql, xtbselectsuffix,36);

                printf("xtbsql=%s\n",xtbsql);

                if(mysql_main)
                {
                    printf("mysql Connection success\n");
                    res=mysql_query(mysql_main,xtbsql);
                    //printf("res:%d\n",res);
                    if(res)
                    {
                        printf("sqlerror\n\n\n");
                    }
                    else
                    {
                        printf("xtbsqlok\n");

                        int iNumRow = 0;
                        int iNumField = 0;
                        int i = 0;
                        mysqlResult = mysql_store_result(mysql_main);
                        //printf("mysqlResult:%d\n",mysqlResult);
                        if (mysqlResult == NULL)
                        {
                            printf(">数据查询失败! %d:%s\n", mysql_errno(mysql_main), mysql_error(mysql_main));
                            return;
                        }
                        else
                        {
                            if(mysqlRow = mysql_fetch_row(mysqlResult))
                            {
                                txr_lamp_id 	= "1234";
                                txr_xtb_time 	= mysqlRow[1];
                                txr_lamp_sockfd = mysqlRow[2];
                                txr_lamp_pid 	= mysqlRow[3];

                                int inttxr_lamp_id 		= 0;
                                int inttxr_lamp_sockfd	= 0;
                                int inttxr_lamp_pid 	= 0;

                                inttxr_lamp_id 		= atoi(txr_lamp_id);
                                inttxr_lamp_sockfd 	= atoi(txr_lamp_sockfd);
                                inttxr_lamp_pid 	= atoi(txr_lamp_pid);

                                printf("inttxr_lamp_id=%d\ntxr_xtb_time=%s\ninttxr_lamp_sockfd=%d\ninttxr_lamp_pid=%d\n",inttxr_lamp_id,txr_xtb_time,inttxr_lamp_sockfd,inttxr_lamp_pid);

                                mysql_free_result(mysqlResult);

                                //获取心跳包记录时间
                                struct tm tm_time;
                                int xtbsqltime;
                                strptime(txr_xtb_time, "%Y-%m-%d %H:%M:%S", &tm_time);
                                xtbsqltime = mktime(&tm_time);
                                printf("xtbsqltime=%d\n",xtbsqltime);

                                //获取当前时间戳
                                time_t t;
                                t = time(NULL);
                                int nowtime = time(&t);
                                printf("nowtime = %d\n", nowtime);

                                //心跳包记录时间与当前时间比较，若>181秒退出进程
                                double duration;
                                duration = (double)(nowtime - xtbsqltime) / CLOCKS_PER_SEC;
                                if(duration>181)
                                {
                                    //退出杀虫灯进程和网页进程
                                    printf("设备不在线\n\n\n");

                                }
                                else
                                {
                                    //给stm32发命令
                                    int rew = write(inttxr_lamp_sockfd,msgbuffer,1);
                                    printf("rew=%d\n",rew);
                                    if(rew!=1)
                                    {
                                        //write失败时退出进程
                                        printf("checktime to stm32 failed\n");
                                        break;
                                    }
                                    else
                                    {
                                        printf("checktime to stm32 success\n\n");	//设备不在线也能发送成功？？？
                                    }
                                }
                            }
                        }
                    }
                }


                printf("after web m_ma=%d msgid:%d msgtype=%d\n\n\n",m_ma,msgid,msgtype);

                bzero(msgbuffer,sizeof(msgbuffer));

                break;
            }

            // }

            // if(m_picflag==1)
            // {
            printf("22222222\n");
            printf("m_beginpicflag=%d\n",m_beginpicflag);
            printf("msgbuffer[0]=%d\n",msgbuffer[0]);
            printf("m_receivelength=%d\n",m_receivelength);
            //1.先收图标志位为0，接受图片长度
            if( (m_beginpicflag==0)&&(msgbuffer[0]==0x66)&&(m_receivelength==19) )
            {
                printf("msgbuffer[0]=%d\n",msgbuffer[0]);
                printf("msgbuffer[1]=%c\n",msgbuffer[1]);
                printf("msgbuffer[2]=%c\n",msgbuffer[2]);
                printf("msgbuffer[3]=%c\n",msgbuffer[3]);

                printf("pan tu length\n");
                m_totallen=0;
                m_totallen = 0;
                m_aimlength = msgbuffer[16]*65536+msgbuffer[17]*256+msgbuffer[18]; //获取得到
                printf("m_aimlength:%d\n",m_aimlength);

                m_dizhi[0]=msgbuffer[10];
                m_dizhi[1]=msgbuffer[11];
                m_dizhi[2]=msgbuffer[12];
                m_dizhi[3]=msgbuffer[13];



                printf("m_dizhi[0]=%c\n",m_dizhi[0]);
                printf("m_dizhi[1]=%c\n",m_dizhi[1]);
                printf("m_dizhi[2]=%c\n",m_dizhi[2]);
                printf("m_dizhi[3]=%c\n",m_dizhi[3]);

                sleep(1);
                struct timeval tv;
                time_t time;

                bzero(str_t,sizeof(str_t));	/*清空字符串。*/
                gettimeofday (&tv, NULL);

                time = tv.tv_sec;

                p_time = localtime(&time);
                strftime(str_t, 26, "%Y%m%d%H%M%S", p_time);

                printf ("str_t=%s\n", str_t);

                strncat(m_picpath, m_dizhi,4);
                strncat(m_picpath, str_t,20);
                strncat(m_picpath, ".jpeg",8);

                pid_t pid = getpid();
                //创建文件
                creat(m_picpath,0777);
                m_picfilefid = open(m_picpath,O_WRONLY);
                m_beginpicflag=1;
                printf("create picfile m_picpath:%s pid=%d m_aimlength=%d,msgbuffer[6]%d,msgbuffer[7]%d,msgbuffer[8]%c,msgbuffer[9]%c\n",m_picpath,pid,m_aimlength,msgbuffer[6],msgbuffer[7],msgbuffer[8],msgbuffer[9]);
            }
            //2.再收图标志位为1，开始收图并写入文件
            else if( m_beginpicflag==1)
            {
                //写入
                printf("xieru\n");

                write(m_picfilefid,msgbuffer,m_receivelength);
                if(m_totallen<m_aimlength)
                {
                    m_totallen = m_totallen +m_receivelength;
                    printf("ysfile len %d\n",m_totallen);
                }
                if(m_totallen>=m_aimlength-400)
                {
                    //2018-01-29 17:19:10
                    strftime(str_t, 26, "%Y-%m-%d %H:%M:%S", p_time);
                    printf ("trans_pic finish str_t:%s\n", str_t);
                    printf("file finish\n");
                    pid_t pid = getpid();
                    memmove(sqlm_dizhi,m_dizhi,4);

                    char chenteststr[400]= "insert into  tb_lamp_record_20171011(tlr_lamp_id,tlr_picture_path,tlr_upload_time) values(";
                    strncat(chenteststr, "'",1);
                    strncat(chenteststr, sqlm_dizhi,4);
                    strncat(chenteststr, "'",1);
                    strncat(chenteststr, ",'",5);
                    strncat(chenteststr, m_picpath,50);
                    strncat(chenteststr, "','",3);
                    strncat(chenteststr, str_t,50);
                    strncat(chenteststr, "')",2);

                    printf("m_picpath:%s pid=%d\n\n",m_picpath,pid);
                    printf("chenteststr:%s\n\n",chenteststr);

                    //往消息队列中发送数据
                    struct msg_st data;
                    bzero(data.text,sizeof(data.text));	/*清空字符串。*/
                    data.msg_type = 1; // 注意2,发送数据时，类型为1 ??


                    if(mysql_main)
                    {
                        res=mysql_query(mysql_main,chenteststr);
                        if(!res)
                        {
                            printf("picture sqlok\n");
                            data.text[0]='s';
                            data.text[1]='u';
                        }
                        else
                        {
                            printf("picture sqlerror\n");
                            data.text[0]='e';
                            data.text[1]='r';
                        }
                    }
                    else
                    {
                        printf("mysql_main error\n\n");
                        return EXIT_FAILURE;
                    }

                    m_aimlength=0;
                    m_picflag =0;
                    m_totallen=0;
                    m_beginpicflag=0;
                    //关闭文件
                    close(m_picfilefid);
                    //mysql_close(mysql_main);
                    bzero(m_picpath,sizeof(m_picpath));
                    strcat(m_picpath, "/hp/pic");

                    printf("m_picflag=%d\n\n\n",m_picflag);
                }
            }

            // }
        }
    }
    printf("sockclosefd: %d sockfd will be closed\n", sockfd);
    sockclosefd=shutdown(sockfd,2);
    printf("sockclosefd: %d sockfd has been closed\n\n\n\n\n\n\n\n\n\n\n\n\n", sockclosefd);
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    if ((sockfd = socket(AF_INET,SOCK_STREAM,0))<0)  	/*建立一个socket。*/
    {
        perror("socket");
        exit(1);
    }
    else
    {
        printf("socket created success.\n");				/*socket建立成功，输出提示。*/
    }
    //定义sockaddr_in
    struct sockaddr_in addr;
    bzero(&addr,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    //bind成功返回0，出错返回-1
    if(bind(sockfd,(struct sockaddr *)&addr,sizeof(addr))==-1)
    {
        perror("bind");
        exit(1);//1为异常退出
    }
    else
    {
        printf("bind success.\nlocal port:%d\n",PORT);
    }
    printf("after bind sockfd:%d\n",sockfd);
    //listen成功返回0，出错返回-1，允许同时帧听的连接数为QUEUE_SIZE
    if(listen(sockfd,MAX) == -1)
    {
        perror("listen");
        exit(1);
    }
    else
    {
        printf("listenning......\n\n\n");
    }
    for(fd=0; fd<MAX; fd++)
    {
        is_connected[fd]=0;						/*设置所有的标记为0。*/
    }

    for(;;)
    {
        struct sockaddr_in client_addr;
        socklen_t length = sizeof(client_addr);
        //进程阻塞在accept上，成功返回非负描述字，出错返回-1
        newsockfd = accept(sockfd, (struct sockaddr*)&client_addr,&length);
        if(newsockfd<0)
        {
            perror("connect");
            exit(1);
        }
        //write(newsockfd,msg,sizeof(msg));		/*给客户端发送一段信息。*/
        printf("new client accepted.\n");

        pid_t childid;
        if(childid=fork()==0)   //子进程       //fork() 开子进程 不需要参数
        {
            printf("child process pid: %d created.\n\n", getpid());
            processthread(newsockfd);			//处理监听的连接
            exit(0);                               //exit 关闭所有文件，终止正在执行的进程，exit(0)表示正常退出
        }
        signal(SIGCHLD,SIG_IGN);
    }

    printf("closed.\n");
    close(sockfd);
    return 0;
}
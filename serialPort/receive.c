#include <stdio.h>    	//输入输出相关函数 
#include <stdlib.h>  	//类型转换 随机 内存分配 进程控制 搜索排序 简单数学计算等函 数
#include <unistd.h>  	//unix standard缩写，针对unix系统调用的封装
#include <sys/stat.h> 
#include <sys/types.h>  //一些标准的linux数据类型定义
#include <fcntl.h>  	// 文件操作相关
#include <termios.h> 	//终端调用相关
#include <errno.h>  	//定义了通过错误码来回报错误资讯的宏

#define FALSE -1
#define TRUE 0

int speed_arr[] = { B38400, B19200, B9600, B4800, B2400, B1200, B300,
          B38400, B19200, B9600, B4800, B2400, B1200, B300};
int name_arr[] = {38400,  19200,  9600,  4800,  2400,  1200,  300, 38400,  
          19200,  9600, 4800, 2400, 1200,  300};
void set_speed(int fd ,int speed) //设置波特率
{
  int i;
  int status;
  struct termios opt;
  tcgetattr(fd,&opt);	//获取终端相关参数
  for(i = 0;i<sizeof(speed_arr)/sizeof(int);i++){
    if(speed == name_arr[i]){
      tcflush(fd,TCIOFLUSH); //清空终端未完成输入/输出，tctoflush清除正在写入数据
      cfsetispeed(&opt,speed_arr[i]); //设置波特率 
     
      //设置终端参数,TCSANOW 不等数据传送完毕立即改变属性,返回0成功，-1失败
      status = tcsetattr(fd,TCSANOW,&opt); 

      if(status != 0){
        perror("tcsetattr fd");//perror打印引号内的参数和错误原因 
        return;
      }
      tcflush(fd,TCIOFLUSH);  //清空终端未完成的输入输出请求及数据
                              //TCIFLUSH 清除正收到的数据
    }
  }
}

/*
*@brief 设置串口数据位，停止位和校验位 
*@param fd        打开的串口文件句柄
*@param databits  数据位数
*@param stopbits  停止位数
*@param parity    校验类型 

*/
int set_parity(int fd,int databits,int stopbits,int parity)
{
  struct termios option;
  if(tcgetattr(fd,&option) != 0){
    perror("SetupSerial 1");
    return(FALSE);
  }
  option.c_cflag &= ~CSIZE; //传送或接收字元时用的位数,即字节数
  switch(databits)
  {
    case 7:
           option.c_cflag |= CS7; 
	   break;
    case 8:
           option.c_cflag |= CS8;
	   break;
    default:
      	   fprintf(stderr,"Unsupported data size\n");
           return (FALSE);
  }
  switch(parity)
  {
    case 'n':
    case 'N':
             option.c_cflag &= ~PARENB; //不允许产生奇偶信息以及输入的奇偶校验
             option.c_iflag &= ~INPCK ; //不启用奇偶校验
	     break;
    case 'o':
    case 'O':
             option.c_cflag |= (PARODD | PARENB);//奇校验
             option.c_iflag |= INPCK;
             break;
    case 'e':
    case 'E':
             option.c_cflag |= PARENB;
             option.c_cflag &= ~PARODD; //偶校验
             option.c_iflag |= INPCK;
             break;
    case 's':
    case 'S':
             option.c_cflag &= ~PARENB;
             option.c_cflag &= ~CSTOPB;//不设置两个停止位
             break;
    default:
             fprintf(stderr,"Unsupported stop bits\n");
             return (FALSE);    
  }
  switch(stopbits)// 设置停止位
  {
    case 1:
           option.c_cflag &= ~CSTOPB;
           break;
    case 2:
           option.c_cflag |= CSTOPB;
           break;
    defalut:
           fprintf(stderr,"unsupported stop bits\n");
           return (FALSE);
  }
  if(parity != 'n')//如果不禁止奇偶校验，就启动奇偶校验
    option.c_iflag |= INPCK;
  tcflush(fd,TCIFLUSH); //丢已要写入对象但未传输的数据 刷新数据但是不读
  option.c_cc[VTIME] = 150; //模式读时的单位 单位： 十分之一秒
  option.c_cc[VMIN] = 0;   //模式读的最小字符数，能满足read最小字元数
  if(tcsetattr(fd,TCSANOW,&option) != 0)
  {
    perror("SetupSerial 3");
    return (FALSE);
  }
  return (TRUE);
}

int OpenDev(char *Dev)
{
  int fd = open(Dev,O_RDWR);  //以可读可写的方式打开
  if(fd == -1)
  {
    perror("can't Open Serial Port");
    return -1;
  }
  else
    return fd;
}
int main(int argc, char **argv){
  int fd;
  int nread;
  char buff[512];
  char *dev = "/dev/pts/7"; //PORT 2
  fd = OpenDev(dev);
  set_speed(fd,19200);
  if(set_parity(fd,8,1,'N') == FALSE){
   printf("Set Parity Error\n");
    exit(0);
  }
  int count = 0;
  while(1)
  {
    //Only read once can receive all buff data.
    while((nread = read(fd,buff,512))>0) //512 表示需要读取的字节数
    {
    //  printf("\nlen %d\n",nread);
      buff[nread] = '\0';
      printf("%s",buff);
    }
  }
}



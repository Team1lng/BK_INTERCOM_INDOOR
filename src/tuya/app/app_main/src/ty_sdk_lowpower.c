/*********************************************************************************
  *Copyright(C),2015-2020, 
  *TUYA 
  *www.tuya.comm
  *FileName:    lowpower demo
**********************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "utilities/uni_log.h"
#include "tuya_iot_config.h"
#include "tuya_ipc_sdk_init.h"
#include "ty_sdk_common.h"

#ifdef ENABLE_DEMO_LOWPOWER

/*
---------------------------------------------------------------------------------
Low power access reference code
en:TRUE is sleep      FALSE is wake
---------------------------------------------------------------------------------
*/
#define PR_TRACE
#define MAXBUF 512

OPERATE_RET TUYA_APP_LOW_POWER_START(CHAR_T *devbuf, CHAR_T *keybuf, TUYA_IP_ADDR_T ip, INT_T port, CHAR_T *domain_name)
{
    INT_T ret=1;
    INT_T i=0;
    INT_T low_power_socket =-1;
    CHAR_T wakeData[36] = {0};
    INT_T wake_data_len = SIZEOF(wakeData);
    INT_T fail_cnt = 0;

    // demo 写死 ipv4 模式，如果是IPV6  请按情况适配下面API；优先使用域名解析地址
    TUYA_IP_ADDR_T tmp_ip_addr = {0};
    ret = tal_net_gethostbyname(domain_name,TY_AF_INET,&tmp_ip_addr);
    if (0 != ret){
      PR_ERR(" domain resolution failed [%s]", domain_name);  
    }else{
      ret = tuya_ipc_low_power_server_connect(tmp_ip_addr, port, devbuf, strlen(devbuf), keybuf, strlen(keybuf));  
      if (0 != ret){
        PR_ERR(" retry connect failed [%u]", tmp_ip_addr.u_addr.ip4);  
      }
    }
    // 如果域名解析 IP 连接失败，尝试使用接口返回IP地址
    while(0 != ret && fail_cnt < 3)
    {
        ret = tuya_ipc_low_power_server_connect(ip, port, devbuf, strlen(devbuf), keybuf, strlen(keybuf));
        fail_cnt++;
    }

    if (0 != ret){
      PR_ERR(" ip connect file , break");
      return -1;
    }
    PR_DEBUG("power_server_connect over.\n");

    while(low_power_socket == -1)
    {
       low_power_socket= tuya_ipc_low_power_socket_fd_get();
    }

    tuya_ipc_low_power_wakeup_data_get(wakeData, &wake_data_len);
    
    PR_DEBUG("wake up date is { ");
    for(i=0;i<wake_data_len;i++)
    {
        PR_DEBUG("0x%x ",wakeData[i]);
    }
    PR_DEBUG(" }\n");

    CHAR_T heart_beat[12] = {0};
    INT_T heart_beat_len = SIZEOF(heart_beat);
    tuya_ipc_low_power_heart_beat_get(heart_beat,&heart_beat_len);
    PR_DEBUG("heart beat data is { ");

    for(i=0;i<heart_beat_len;i++)
    {
        PR_DEBUG("0x%x ",heart_beat[i]);
    }
    PR_DEBUG(" }\n");

    fd_set rfds;
    struct timeval tv;
    INT_T retval, maxfd = -1;

    INT_T len=0;
    CHAR_T recBuf[MAXBUF]={0};
    INT_T heart_timeout=5;
    INT_T user_set_timeout=10;
    while(1)
    {
        FD_ZERO(&rfds);
        FD_SET(0,&rfds);
        maxfd=0;
        FD_SET(low_power_socket,&rfds);
        if (low_power_socket > maxfd)
        {
          maxfd = low_power_socket;
        }
        tv.tv_sec = user_set_timeout;//default 10 seconds;
        tv.tv_usec = 0;

        retval = select(maxfd+1, &rfds, NULL, NULL, &tv);
        if (retval == -1)
        {
          PR_DEBUG("Will exit and the select is error! %s", strerror(errno));
          break;
        }
        else if (retval == 0)
        {
          PR_DEBUG("============send heart beat==============\n");
          len = send(low_power_socket, heart_beat, heart_beat_len, 0);
          if (len < 0)
          {
              PR_DEBUG("socket =%d %d\n",low_power_socket,errno);
              break;
          }
          else
          {
            PR_DEBUG("News: %d \t send, sent a total of %d bytes!\n",
                    heart_beat_len, len);
          }

          continue;
        }
        else
        {
            if (FD_ISSET(low_power_socket, &rfds))
            {
              bzero(recBuf, MAXBUF);
              PR_DEBUG("============recv data==============\n");
              len = recv(low_power_socket, recBuf, MAXBUF, 0);
              if (len > 0)
              {
                  PR_DEBUG("Successfully received the message: is {");
                  for(i=0;i<len;i++)
                      PR_DEBUG("0x%02x ",recBuf[i]);
                  PR_DEBUG("}\n");
                  if(strncmp(recBuf,wakeData,wake_data_len)==0)
                  {
                      //TODO  启动SDK
                      PR_DEBUG("recve data is wake up\n");
                  }

              }
              else
              {
                if (len < 0)
                    PR_DEBUG("Failed to receive the message! \
                          The error code is %d, error message is '%s'\n",
                          errno, strerror(errno));
                else
                    PR_DEBUG("Chat to terminate len=0x%x!\n",len);

                break;
              }
            }

        }
    }

    return 0;
}

#endif
VOID TUYA_IPC_low_power_sample()
{
  TUYA_IP_ADDR_T ip={0};
  int port=0;
  char domain_name[64] = {0};

  int ret = tuya_ipc_get_low_power_server_v2(domain_name, sizeof(domain_name), &ip, &port);
  // note 建议优先使用 IP 连接保活服务器，如果连接异常，可以尝试使用 domain_name 解析出 IP 地址进行连接尝试
  if(ret != 0)
  {
      PR_ERR("get low power ip  error %d\n",ret);
      return;
  }
  #define COMM_LEN 30
  char devid[COMM_LEN]={0};
  int id_len=COMM_LEN;
  ret = tuya_ipc_get_device_id(devid, &id_len);
  if(ret != 0)
  {
      PR_ERR("get devide error %d\n",ret);
      return;
  }
  char local_key[COMM_LEN]={0};
  int key_len=COMM_LEN;
  ret = tuya_ipc_get_local_key(local_key, &key_len);
  if(ret != 0)
  {
      PR_ERR("get local key  error %d\n",ret);
      return;
  }
#ifdef ENABLE_DEMO_LOWPOWER
  TUYA_APP_LOW_POWER_START(devid, local_key, ip, port, domain_name);  
#endif
  return;
}


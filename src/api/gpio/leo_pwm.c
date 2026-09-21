#include "leo_pwm.h"
#include"stdio.h"
#include"string.h"

int bl_pwm_duty_set(const int pwm,unsigned int duty_cycle,unsigned int period){

    char path[64] = {0};
	char buffer[64];
	int fd = -1;
	int len = 0;
    sprintf(&path[0], "/sys/class/pwm/pwmchip0/pwm%d", pwm);
    if(access(path, F_OK) != 0){
		memset(path, 0, 64);
		sprintf(&path[0], "/sys/class/pwm/pwmchip0/export");
		fd = open(path, O_WRONLY);                      
		if (fd < 0){
			printf("\"%s\" open failed \n", path); 
			return -1;                     
		}  

		
		len = snprintf(buffer, sizeof(buffer), "%d", pwm);
		if (write(fd, buffer, len) < 0){
			printf("write failed to export pwmd%d!\n",pwm);
		}
		close(fd);
	}

	memset(path,0,sizeof(path));
	memset(buffer,0,sizeof(buffer));
  	sprintf(&path[0], "/sys/class/pwm/pwmchip0/pwm%d/enable",pwm);
	fd = open(path, O_WRONLY);                      
	if (fd < 0){
		printf("\"%s\" open failed \n", path); 
		return -1;                     
	}  

	memset(buffer,0,sizeof(buffer));
	len = snprintf(buffer, sizeof(buffer), "%d", duty_cycle?1:0);
	if (write(fd, buffer, len) < 0){
		printf("write failed to pwmd%d - duty enable : %d!\n",pwm,duty_cycle);
	}
	close(fd);

	if(duty_cycle == 0){
		return 0;
	}

	
	memset(path,0,sizeof(path));
  	sprintf(&path[0], "/sys/class/pwm/pwmchip0/pwm%d/period",pwm);
	fd = open(path, O_WRONLY);                      
	if (fd < 0){
		printf("\"%s\" open failed \n", path); 
		return -1;                     
	}  

	memset(buffer,0,sizeof(buffer));
	len = snprintf(buffer, sizeof(buffer), "%d", period);
	if (write(fd, buffer, len) < 0){
		printf("write failed to pwmd%d - period : %d!\n",pwm,period);
	}
	close(fd);

	memset(path,0,sizeof(path));
  	sprintf(&path[0], "/sys/class/pwm/pwmchip0/pwm%d/duty_cycle",pwm);
	fd = open(path, O_WRONLY);                      
	if (fd < 0){
		printf("\"%s\" open failed \n", path); 
		return -1;                     
	}  

	memset(buffer,0,sizeof(buffer));
	len = snprintf(buffer, sizeof(buffer), "%d", duty_cycle);
	if (write(fd, buffer, len) < 0){
		printf("write failed to pwmd%d - duty cycle : %d!\n",pwm,duty_cycle);
	}
	close(fd);
    return 0;
}
int pwm1_duty_set(const int pwm,unsigned int duty_cycle,unsigned int period){

    char path[64] = {0};
	char buffer[64];
	int fd = -1;
	int len = 0;
    sprintf(&path[0], "/sys/class/pwm/pwmchip1/pwm%d", pwm);
    if(access(path, F_OK) != 0){
		memset(path, 0, 64);
		sprintf(&path[0], "/sys/class/pwm/pwmchip1/export");
		fd = open(path, O_WRONLY);                      
		if (fd < 0){
			printf("\"%s\" open failed \n", path); 
			return -1;                     
		}  

		
		len = snprintf(buffer, sizeof(buffer), "%d", pwm);
		if (write(fd, buffer, len) < 0){
			printf("write failed to export pwmd%d!\n",pwm);
		}
		close(fd);
	}

	memset(path,0,sizeof(path));
	memset(buffer,0,sizeof(buffer));
  	sprintf(&path[0], "/sys/class/pwm/pwmchip1/pwm%d/enable",pwm);
	fd = open(path, O_WRONLY);                      
	if (fd < 0){
		printf("\"%s\" open failed \n", path); 
		return -1;                     
	}  

	memset(buffer,0,sizeof(buffer));
	len = snprintf(buffer, sizeof(buffer), "%d", duty_cycle?1:0);
	if (write(fd, buffer, len) < 0){
		printf("write failed to pwmd%d - duty enable : %d!\n",pwm,duty_cycle);
	}
	close(fd);

	if(duty_cycle == 0){
		return 0;
	}

	
	memset(path,0,sizeof(path));
  	sprintf(&path[0], "/sys/class/pwm/pwmchip1/pwm%d/period",pwm);
	fd = open(path, O_WRONLY);                      
	if (fd < 0){
		printf("\"%s\" open failed \n", path); 
		return -1;                     
	}  

	memset(buffer,0,sizeof(buffer));
	len = snprintf(buffer, sizeof(buffer), "%d", period);
	if (write(fd, buffer, len) < 0){
		printf("write failed to pwmd%d - period : %d!\n",pwm,period);
	}
	close(fd);

	memset(path,0,sizeof(path));
  	sprintf(&path[0], "/sys/class/pwm/pwmchip1/pwm%d/duty_cycle",pwm);
	fd = open(path, O_WRONLY);                      
	if (fd < 0){
		printf("\"%s\" open failed \n", path); 
		return -1;                     
	}  

	memset(buffer,0,sizeof(buffer));
	len = snprintf(buffer, sizeof(buffer), "%d", duty_cycle);
	if (write(fd, buffer, len) < 0){
		printf("write failed to pwmd%d - duty cycle : %d!\n",pwm,duty_cycle);
	}
	close(fd);
    return 0;
}

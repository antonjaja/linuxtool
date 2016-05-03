#include "efm32fn.h"

/* Create mcu.h from mcu.bin file
 * $ xxd -i mcu.bin mcu.h
 */ 
#include "mcu.h"

#define RTC_DEV "/dev/rtc0"
#define MCU_BIN mcu_bin
#define MCU_BIN_LEN mcu_bin_len

// efm32_update source_path
int Update_EFM32Fn(char* dev) {
	char *buffer = NULL;
	int count = 0;
	int i, rtc, ret;
	int retry_times = 0;
	
	buffer = (char*) calloc( 16, sizeof(char) );

	rtc = open(dev, O_RDWR);
	for(i = 0 ;i < 3 ; i++){
		ret = ioctl(rtc, EFM_UPDATE_SAVETIME, buffer);
		if(ret == 0)break;
	}
    debug_msg("EFM_UPDATE_SAVETIME ret %d\n", ret);
    
    if( ret < 0 ) return -1;

    for(i = 0 ;i < 3 ; i++){
		ret = ioctl(rtc, EFM_UPDATE_START, buffer);
        if(ret == 0)break;
    }
    
	debug_msg("EFM_UPDATE_START ret %d\n", ret);
	if( ret < 0 ) return -1;

	while (count < MCU_BIN_LEN ) {
/*	show the write data
  		printf("send data :");
		for(i = 0 ;i < 16 ; i++){
			buffer[i] = mcu_bin[count+i];
			printf(" %02x", buffer[i]);
		}
		printf("\n");
*/
		for(i = 0 ;i < 3 ; i++){
			ret = ioctl(rtc, EFM_SEND, &MCU_BIN[count]);
			if(ret == 0)break;
		}
		count = count + 16;
		if(i == 3){
			debug_err("Failed EFM_SEND ret %d\n", ret);
			for(i = 0 ;i < 3 ; i++){
				ret = ioctl(rtc, EFM_UPDATE_START, buffer);
				if(ret == 0)break;
			}
			count = 0;
			++retry_times;
		}
		if( retry_times >= 10 ){
			debug_eng("EFM_UPDATE Fail retry_times : %d\n", retry_times);
			free(buffer);
			close(rtc);
			return -1;
		}
	}

    for(i = 0 ;i < 3 ; i++){
		ret = ioctl(rtc, EFM_UPDATE_FINISHED, buffer);
            if(ret == 0)break;
    }
	debug_msg("EFM_UPDATE_FINISHED %d\n", ret);
    if( ret < 0 ) return -1;

	debug_eng("%02d%02d%02d\n", buffer[1], buffer[2], buffer[3]);

	free(buffer);
	close(rtc);

	return 0;
}

/*main(int argc, char **argv)
 * 	
 *       
 */  
int main(int argc, char **argv)
{
	int ret = 0;
	FILE *fp;
	char result_buf[32];
	int printk[4];
	char send[1024];

	//remember printk level
	fp = popen("cat /proc/sys/kernel/printk", "r");
	if(NULL == fp){
		goto bypass;
	}
	
	while(fgets(result_buf, sizeof(result_buf), fp) != NULL){
		sscanf(result_buf,"%d %d %d %d", &printk[0],&printk[1],&printk[2],&printk[3]);
	}
		
	ret = pclose(fp);
	if( ret == -1 ){
		goto bypass;
	}

	//disable printk message
	system("echo 1 4 1 7 > /proc/sys/kernel/printk");

bypass:
	//update
	ret = Update_EFM32Fn( RTC_DEV );	

	//restore printk level
	sprintf(send ,"echo %d %d %d %d > /proc/sys/kernel/printk", printk[0],printk[1],printk[2],printk[3]);
	system(send);

fail:			
	return ret ;
							
}

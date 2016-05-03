#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/ioctl.h>

#include "getopt.h"

#define veb_on
//#define msg_on
//#define war_on
#define err_on
//#define eng_on

#ifdef veb_on
#define debug_veb(format, arg...) (printf("[veb]%s[%d]:%s : ", __FILE__ , __LINE__ , __FUNCTION__  ), printf(format ,## arg))
#else
#define debug_veb(format, arg...)
#endif

#ifdef msg_on
#define debug_msg(format, arg...) (printf("[msg]%s[%d]:%s : ", __FILE__ , __LINE__ , __FUNCTION__  ), printf(format ,## arg))
#else
#define debug_msg(format, arg...)
#endif

#ifdef war_on
#define debug_war(format, arg...) (printf("[war]%s[%d]:%s : ", __FILE__ , __LINE__ , __FUNCTION__  ), printf(format ,## arg))
#else
#define debug_war(format, arg...)
#endif

#ifdef err_on
#define debug_err(format, arg...) (printf("[err]%s[%d]:%s : ", __FILE__ , __LINE__ , __FUNCTION__  ), printf(format ,## arg))
#else
#define debug_err(format, arg...)
#endif

#ifdef eng_on
#define debug_eng(format, arg...) (printf("[eng]%s[%d]:%s : ", __FILE__ , __LINE__ , __FUNCTION__  ), printf(format ,## arg))
#else
#define debug_eng(format, arg...)
#endif

#define EFM_SEND			_IO('p', 0x11)  /* EFM32 send data			*/
#define EFM32_VER_GET		_IO('p', 0x12)  /* EFM32 get boot/main and version*/
#define EFM_UPDATE_START	_IO('p', 0x13)  /* EFM32 update start		*/
#define EFM_UPDATE_FINISHED	_IO('p', 0x14)  /* EFM32 update finished	*/
#define EFM_SEND_ALARM		_IO('p', 0x15)  /* EFM32 send Alarm time	*/
#define EFM_GET_ALARM		_IO('p', 0x16)  /* EFM32 get Alarm time		*/
#define EFM_UPDATE_SAVETIME	_IO('p', 0x17)  /* EFM32 get save time		*/
#define EFM_UPDATE_STATUS	_IO('p', 0x18)  /* EFM32 set power status	*/

#define O_RDWR		     	02

/* options args */
static const char *short_options = "?h";

static struct option long_options[] = {
  {"help"           , no_argument,       0, 'h'                      },
  {"path"           , required_argument, 0, 'p'                      },
  {0                , no_argument,       0, 0                        }
};

static void show_usage(){
        printf("Options:\n");
        printf("        --path   bin file full path\n");
        printf("        --help   help\n");
}

// efm32_update source_path
int Update_EFM32Fn(char* dev, char* path) {
	char *buffer = NULL;
	size_t read;
	int i, ret, rtc1 ,j;
	int rtc = 0;
	FILE* fin = NULL;
	int retry_times = 0;
	int retry_times_max = 3;
	unsigned char status = 0;
	int status_retry = 0;
	int status_retry_max = 10;
	int count = 0;
	char cTemp[64];
	
	buffer = (char*) calloc( 16, sizeof(char) );

	fin = fopen(path, "rb");
	if (fin == NULL) {
		debug_err("can't open %s for write\n", path);
		return -1;
	}
	fseek(fin, 0, SEEK_END);
	debug_veb("update file name : %s ; fin length : %d\n",path, (int)ftell(fin));
	fseek(fin, 0, SEEK_SET);

	rtc = open(dev, O_RDWR);
	if( rtc <= 0 ){
		debug_err("Can't open %s\n", dev);			
		return -2;
	}
	
	for(i = 0 ;i < 3 ; i++){
		ret = ioctl(rtc, EFM_UPDATE_SAVETIME, buffer);
		if(ret == 0)break;
	}
    debug_msg("EFM_UPDATE_SAVETIME ret %d\n", ret);
    
    if( ret < 0 ) goto fail;

    for(i = 0 ;i < 3 ; i++){
		ret = ioctl(rtc, EFM_UPDATE_START, buffer);
        if(ret == 0)break;
    }
    
	debug_veb("EFM_UPDATE_START ret %d\n", ret);
	if( ret < 0 ) goto fail;

nextdata:	
	while ((read = fread(buffer, 1, 16, fin)) > 0) {
		status_retry = 0;
		for(i = 0 ;i < 3 ; i++){
			ret = ioctl(rtc, EFM_SEND, buffer);
//			ret = 0;
//			usleep(300000);
#ifdef msg_on
			count++;
			for(j=0 ;j < 16; j++){
				sprintf(&cTemp[j*2], "%02x", *(buffer+j));
			}
			debug_msg("EFM_SEND count %d, buffer : %s, EFM_SEND ret : 0x%x\n", count, cTemp, ret);
#endif
			if(ret == 0){
				break;
			}else if(ret == -1){
readstatus:
				status = 0x16;
				ret = ioctl(rtc, EFM_UPDATE_STATUS, &status);
				status_retry++;	
				if(status_retry > status_retry_max) break;
				if(status == 0x15 && ret == 0){
					debug_war("EFM_SEND current data retry : %d\n", i+1);
				}else if(status == 0 && ret == 0){
					debug_war("EFM_SEND next data. status_retry : %d\n", status_retry);
					goto nextdata;
				}else {
					debug_war("Get boot_read status delay 50ms.\n");
					usleep(10000);
					goto readstatus;
				}
			}
		}
		
		if(i == 3 || status_retry > status_retry_max){
			count = 0;
			debug_war("Failed EFM_SEND i : %d; status_retry : %d\n", i, status_retry);
			for(i = 0 ;i < 3 ; i++){
				ret = ioctl(rtc, EFM_UPDATE_START, buffer);
				if(ret == 0)break;
			}
			++retry_times;
			fseek(fin, 0, SEEK_SET);
			debug_msg("retry_times %d ; EFM_UPDATE_START ret %d\n",retry_times, ret);
			if( ret < 0 ) goto fail;
		}
		if( retry_times >= retry_times_max ){
			debug_err("EFM_UPDATE Fail retry_times : %d\n", retry_times);
			goto fail;
		}
	}

    for(i = 0 ;i < 3 ; i++){
		ret = ioctl(rtc, EFM_UPDATE_FINISHED, buffer);
            if(ret == 0)break;
    }
	debug_veb("EFM_UPDATE_FINISHED %d\n", ret);
    if( ret < 0 ) goto fail;

	debug_veb("%02d%02d%02d\n", buffer[1], buffer[2], buffer[3]);

fail:
	if (fclose(fin) != 0) {
		fin = NULL;
	}
	free(buffer);
	close(rtc);
	return ret;
}

int main(int argc, char **argv)
{
        int c ='?';
        int option_index = 0;
//		int i = 0;

        while( (c = getopt_long(argc, argv, short_options,long_options, &option_index)) != EOF ){
                switch( c ){
                        case 'p':
//				for( i = 0; i < 3; i++ ){
//                                	if( Update_EMF32Fn(optarg) == 0 ) break;
//					sleep(1);
//				}
				if( Update_EFM32Fn("/dev/rtc0", optarg) != 0 )
					printf("EFM_UPDATE_FAIL!!!\n");
				else
					printf("EFM_UPDATE_SUCCESS!!!\n");

                                break;
                        case 'h':
                                show_usage();
                                break;
                        default:
                                show_usage();
                                break;
                }
	}

	return 0;
}

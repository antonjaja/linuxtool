#include "efm32fn.h"
/*
Coordinated Universal Time（UTC）：世界標准時間，格林威治標准時間（Greenwich Mean Time，GMT）。比如，中國內地的時間與UTC的時差為+8，也就是UTC+8。美國是UTC-5
Calendar Time：日歷時間是"相對時間"，是用「從一個標准時間點到此時的時間經過的秒數」來表示的時間
epoch：時間點。時間點在標准C/C++中是一個整數，它用此時的時間和標准時間點相差的秒數（即日歷時間）來表示。
clock tick：時鐘計時單元，一個時鐘計時單元的時間長短是由CPU控制的。一個clock tick不是CPU的時鐘周期，而是C/C++的一個基本計時單位。

typedef long time_t; 
typedef long clock_t;

clock_t clock( void );
double difftime(time_t time1, time_t time0);
time_t mktime(struct tm * timeptr);
time_t time(time_t * timer);
char * asctime(const struct tm * timeptr);
char * ctime(const time_t *timer);
struct tm * gmtime(const time_t *timer);
struct tm * localtime(const time_t * timer);
* 
strftime(str,100,"It is now %I %p",ptr);
printf(str);
* 
Read more: http://csie-tw.blogspot.tw/2008/10/cc-timetstruct-tm.html
*/

void usage(void)
{
	get_usage("[--setalarm agrv] [--getalarm agrv] [--status agrv] [--update agrv] [--getcr2032 agrv]\n"
"example:\n"
"	--setclock	/dev/rtc* YYYYMMDDHHMMSS\n"
"	--getclock	/dev/rtc*\n"
"	--setalarm	/dev/rtc* 60\n"
"	--getalarm	/dev/rtc*\n"
"	--status	/dev/rtc* off|reset|keep|ignore|get|version\n"
"	--update /dev/rtc* path\n"
);
	exit(2);
}

/*efm_setclock
 * 	for efm set clock
 *      --setclock /dev/rtc0 20130101010101
 */  
int efm_setclock(char* dev, const char *date_str) {
	int i, ret;
	int rtc = 0 ;
	struct rtc_time ptm;
	
	rtc = open(dev, O_RDWR);
	if( rtc <= 0 ){
		debug_err("Can't open %s\n", dev);			
		return 128;
	}
	
/*	for(i = 0 ;i < 3 ; i++){
		ret = ioctl(rtc, RTC_UIE_ON, ptm);
		if(ret == 0){
			break;
		}else{
			debug_war("RTC_UIE_ON retry:  %d\n", i);			
		}
	}
    
	if( i >= 3 ){
		debug_err("RTC_UIE_ON fail\n");			
		goto fail;		
	}
*/	
	if ( sscanf(date_str, "%4u%2u%2u%2u%2u%2u",
					&ptm.tm_year,
					&ptm.tm_mon,
					&ptm.tm_mday,
					&ptm.tm_hour,
					&ptm.tm_min,
					&ptm.tm_sec) == 6) {
			ptm.tm_year -= 1900; /* Adjust years */
			ptm.tm_mon -= 1; /* Adjust month from 1-12 to 0-11 */
			ptm.tm_isdst = -1;
		} 
		
    debug_eng("set time :  %04d/%02d/%02d %02d:%02d:%02d\n", 
    ptm.tm_year+1900, ptm.tm_mon+1, ptm.tm_mday, ptm.tm_hour, ptm.tm_min, ptm.tm_sec);
	
	for(i = 0 ;i < 3 ; i++){
		ret = ioctl(rtc, RTC_SET_TIME, &ptm);
		if(ret == 0){
			break;
		}else{
			debug_war("set clock time retry:  %d\n", i);			
		}
	}
	
	if( i >= 3 ){
		debug_err("set clock time fail\n");			
		goto fail;		
	}
	
	close(rtc);
	
	return 0;
	
fail:
	close(rtc);
	
	return 129;
}

/*efm_getclock
 * 	for efm get clock
 *      --getclock /dev/rtc0
 */  
int efm_getclock(char* dev) {
	int i, ret;
	int rtc = 0 ;
	struct rtc_time *ptm;
	
	rtc = open(dev, O_RDWR);
	if( rtc <= 0 ){
		debug_err("Can't open %s\n", dev);			
		return 128;
	}

	ptm = malloc(sizeof(*ptm));
	memset(ptm, 0, sizeof(*ptm));

	for(i = 0 ;i < 3 ; i++){
		ret = ioctl(rtc, RTC_RD_TIME, ptm);
		if(ret == 0){
			break;
		}else{
			debug_war("read clocktime retry:  %d\n", i);			
		}
	}
    
	if( i >= 3 ){
		debug_err("read clocktime fail\n");			
		goto fail;		
	}
	
    debug_eng("get clocktime :  %04d/%02d/%02d %02d:%02d:%02d\n", 
    ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	
	free(ptm);
	close(rtc);
	
	return 0;
	
fail:
	free(ptm);
	close(rtc);
	
	return 129;
}

/*efm_setalarm
 * 	for efm set alarm clock
 *      --setalarm /dev/rtc0 30
 */  
int efm_setalarm(char* dev, int alarmtime) {
	int i, ret;
	int rtc = 0 ;
	struct rtc_time *ptm;
	struct rtc_wkalrm *ptmalarm;
	time_t t;
	
	rtc = open(dev, O_RDWR);
	if( rtc <= 0 ){
		debug_err("Can't open %s\n", dev);			
		return 128;
	}
	
	ptmalarm = malloc(sizeof(*ptmalarm));
	memset(ptmalarm, 0, sizeof(*ptmalarm));
	ptm = malloc(sizeof(*ptm));
	memset(ptm, 0, sizeof(*ptm));

	for(i = 0 ;i < 3 ; i++){
		ret = ioctl(rtc, RTC_RD_TIME, ptm);
		if(ret == 0){
			break;
		}else{
			debug_war("read time retry:  %d\n", i);			
		}
	}
    
	if( i >= 3 ){
		debug_err("read time fail\n");			
		goto fail;		
	}
	
    debug_veb("get time : %04d/%02d/%02d %02d:%02d:%02d\n", 
    ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	
	if(alarmtime == 0){
		ptm->tm_year = 70;
		ptm->tm_mon = 0;
		ptm->tm_mday = 1;
	}
		
	t = mktime(ptm);   
    t = t + alarmtime ;
    ptm = gmtime(&t);
	memcpy(&ptmalarm->time, ptm, sizeof(struct rtc_time));
/*    ptmalarm->time.tm_sec = ptm->tm_sec;
    ptmalarm->time.tm_min = ptm->tm_min;
    ptmalarm->time.tm_hour = ptm->tm_hour;
    ptmalarm->time.tm_mday = ptm->tm_mday;
    ptmalarm->time.tm_mon = ptm->tm_mon;
    ptmalarm->time.tm_year = ptm->tm_year;
    ptmalarm->time.tm_wday = ptm->tm_wday;
    ptmalarm->time.tm_yday = ptm->tm_yday;
	*/
	ptmalarm->time.tm_isdst = -1;

    ptmalarm->enabled = 1;
    ptmalarm->pending = 0;

    debug_eng("alarm time : %04d/%02d/%02d %02d:%02d:%02d\n", 
    ptmalarm->time.tm_year+1900, ptmalarm->time.tm_mon+1, ptmalarm->time.tm_mday, ptmalarm->time.tm_hour
    , ptmalarm->time.tm_min, ptmalarm->time.tm_sec);
    
	for(i = 0 ;i < 3 ; i++){
		ret = ioctl(rtc, RTC_WKALM_SET, ptmalarm);
		if(ret == 0){
			break;
		}else{
			debug_war("set alarm time retry:  %d\n", i);			
		}
	}
	
	if( i >= 3 ){
		debug_err("set alarm time fail\n");			
		goto fail;		
	}
disablealarm:	
	free(ptmalarm);
	close(rtc);
	
	return 0;
	
fail:
	free(ptmalarm);
	close(rtc);
	
	return 129;
}

/*efm_getalarm
 * 	for efm get alarm clock
 *      --getalarm /dev/rtc0
 */  
int efm_getalarm(char* dev) {
	int i, ret;
	int rtc = 0 ;
	struct rtc_time *ptm;
	
	rtc = open(dev, O_RDWR);
	if( rtc <= 0 ){
		debug_err("Can't open %s\n", dev);			
		return 128;
	}

	ptm = malloc(sizeof(*ptm));
	memset(ptm, 0, sizeof(*ptm));

	for(i = 0 ;i < 3 ; i++){
		ret = ioctl(rtc, RTC_ALM_READ, ptm);
		if(ret == 0){
			break;
		}else{
			debug_war("read alarmtime retry:  %d\n", i);			
		}
	}
    
	if( i >= 3 ){
		debug_err("read alarmtime fail\n");			
		goto fail;		
	}
	
    debug_eng("get alarmtime : %04d/%02d/%02d %02d:%02d:%02d\n", 
    ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	
	free(ptm);
	close(rtc);
	
	return 0;
	
fail:
	free(ptm);
	close(rtc);
	
	return 129;
}

/*efm_status
 * 	for efm status set/get
 *      --status /dev/rtc0 get|keep|ignore|version
 */  
int efm_status(char* dev, char* choise) {
	int ret;
	int rtc = 0 ;
	unsigned char powersta = 0;
	unsigned char ver[4];

	rtc = open(dev, O_RDWR);
	if( rtc <= 0 ){
		debug_err("Can't open %s\n", dev);			
		return 128;
	}

	if(strncmp(choise,"off",3) == 0){//power off
		powersta = 0x00;
		ret = ioctl(rtc, EFM_UPDATE_STATUS, &powersta);			
	}else if( strncmp(choise,"reset",5) == 0 ){//reset system
		powersta = 0x01;
		ret = ioctl(rtc, EFM_UPDATE_STATUS, &powersta);	
	}else if(strncmp(choise,"keep",4) == 0){//keep power status
		powersta = 0x02;
		ret = ioctl(rtc, EFM_UPDATE_STATUS, &powersta);			
	}else if(strncmp(choise,"ignore",6) == 0){//ignore power status
		powersta = 0x03;
		ret = ioctl(rtc, EFM_UPDATE_STATUS, &powersta);			
	}else if(strncmp(choise,"get",3) == 0){
		powersta = 0x12;
		ret = ioctl(rtc, EFM_UPDATE_STATUS, &powersta);
		debug_eng("get status : %s\n", ret==0?"ignore":"keep");							
	}else if(strncmp(choise,"version",7) == 0){
		ret = ioctl(rtc, EFM32_VER_GET, ver);
		debug_eng("mcu_version= %02d%02d%02d\n", ver[1], ver[2], ver[3]);							
	}else{
		debug_err("status function not support: %s\n", choise);			
 		return 129;
	}

	debug_msg("status get/set :  %d ;\n", ret);			
	
	close(rtc);
	
	return ret;
	
}

// efm32_update source_path
int Update_EFM32Fn(char* dev, char* path) {
	char *buffer = NULL;
	size_t read;
	int i, ret, j;
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
	debug_veb("update file name : %s ; fin lenght : %d\n",path, (int)ftell(fin));
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
		for(i = 0 ;i < 3 ; i++){
			ret = ioctl(rtc, EFM_SEND, buffer);
//			ret = 0;
			count++;
//			usleep(300000);
			for(j=0 ;j < 16; j++){
				sprintf(&cTemp[j*2], "%02x", *(buffer+j));
			}
			debug_msg("EFM_SEND count %d, buffer : %s, EFM_SEND ret : 0x%x\n", count, cTemp, ret);
			if(ret == 0){
				break;
			}else if(ret == -1){
				status_retry = 0;
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

/*main(int argc, char **argv)
 * 	
 *       
 */  
int main(int argc, char **argv)
{
	int ch;
	int long_index = 0;
	int ret = 0;
	
	debug_veb("arc: %d, argv[1]: %s, argv[2]: %s\n", argc, argv[1], argv[2]);
	while ((ch = getopt_long(argc, argv, EFM_SHORT_OPTSTR, EFM_LONG_OPTSTR, &long_index)) != EOF) {
		switch(ch) {	
		case 1: //setalarm
			{
				argv++ ;
				int itime = 60 ;
				if( argv[2] != NULL ){
					itime = strtol(argv[2] ,NULL, 0);
				}
				if (strncmp( argv[1], "/dev/",5) == 0){
					ret = efm_setalarm(argv[1], itime) ;
				}else{
					usage() ;	
				}
				return ret ;
			break ;
			}
			
		case 2: //getalarm
				argv++ ;
				ret = efm_getalarm(argv[1]) ;
				return ret ;
			break ;
			
		case 3: //status
				argv++ ;
				ret = efm_status(argv[1], argv[2]) ;
				return ret ;			
			break;
			
		case 4: //update
				argv++ ;
				ret = Update_EFM32Fn(argv[1], argv[2]);	
				return ret ;			
			break ;

		case 5: //setclock
				argv++ ;
				ret = efm_setclock(argv[1], argv[2]) ;
				return ret ;
			break ;
			
		case 6: //getclock
				argv++ ;
				ret = efm_getclock(argv[1]) ;
				return ret ;
			break ;
			
		default:
			usage() ;
		}
	}

	return 0 ;
}

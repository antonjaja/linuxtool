#include "mp.h"

void get_version(void)
{
	get_ver("\n"
"   	Create date is %s\n"
	"	Build number is %s\n"
			, TOOL_DATE, TOOL_VERSION);
	exit(2);
}

void usage(void)
{
	get_usage("[--gpio agrv] [--rs232 agrv] [--rs485 agrv] [--display agrv] [--version]\n"
"   		[-a -b agrv] [-c agrv] [-d agrv]\n"
"example:\n"
"	--gpio		gpio test\n"
"	--rs232		rs232 test\n"
"	--rs485		rs485 test\n"
"	--display	video out test\n"
"	--devopen	opne device node\n"
"	--cat		get cpu temperatual\n"
"	-v|--version	get version\n"
);
	exit(2);
}

void usage_gpio(void)
{
	get_usage("--gpio in|out|pair num1 num2 num3 num4 .....(max.%d)\n"
"example:\n"
"	--gpio in 13 33 88 133 44\n"
"	--gpio out 22 33 25 66 77 111 68\n"
"	--gpio pair 13 44 55 66 33 101\n"
	, ARRAY_MAX);
	exit(2);
}

void usage_rs232(void)
{
	get_usage("--rs232 client|server|repeat port_node\n"
"example:\n"
"	--rs232 client /dev/ttymxc0\n"
"	--rs232 server /dev/ttymxc0\n"
"	--rs232 repeat /dev/ttymxc2\n"
);
	exit(2);
}

void usage_rs485(void)
{
	get_usage("--rs485 client|server port_node\n"
"example:\n"
"	--rs485 client /dev/ttymxc3\n"
"	--rs485 server /dev/ttymxc3\n"
	);
	exit(2);
}

void usage_display(void)
{
	get_usage("--display device times\n"
"example:\n"
"	--display /dev/fb0 5\n"
"	--display /dev/fb1 25\n"
	);
	exit(2);
}

void usage_devopen(void)
{
	get_usage("--open device node\n"
"example:\n"
"	--devopen /dev/rtc0 10\n"
	);
	exit(2);
}

void usage_cat(void)
{
	get_usage("--cat for loop\n"
"example:\n"
"	--cat /sys/devices/platform/anatop_thermal.0/thermal_degree 0\n"
"	--cat /sys/devices/platform/anatop_thermal.0/thermal_degree 100\n"
	);
	exit(2);
}

/*gpio_in
 * 	gpio input pin
 *       
 */  
int gpio_in(int nums, int port[ARRAY_MAX])
{
	int i;
	int ret;
	char send[1024];
    char result_buf[32];
	char dip_fir[ARRAY_MAX] ;
	char dip_sec[ARRAY_MAX] ;
	char dip_res[ARRAY_MAX] ;
	int time_out = 10 ;
	int time_count = 0 ;
	int pin_finish = 130 ;
	char *sysgpio="/sys/class/gpio";
	char *export="/sys/class/gpio/export";
	char *unexport="/sys/class/gpio/unexport";
	FILE *fp;

	for ( i = 0; i < nums ; i++) {
		debug_msg(" [%d] = %d\n", i , port[i]) ;
		sprintf(send, "echo %d > %s", port[i], export);
		system(send);
		dip_res[i] = 0;
	}
	
	for ( i = 0; i < nums ; i++) {
		sprintf(send, "%s/gpio%d/value", sysgpio, port[i]);
		fp = fopen(send, "r");
		if(NULL == fp){
			debug_err("fopen %s/gpio%d/value fail！\n", sysgpio, port[i]);
			pin_finish = 128 ;
			goto fail;
		}
	
		while(fgets(result_buf, sizeof(result_buf), fp) != NULL){
			dip_fir[i] = strtol(result_buf, NULL, 0);
		}
		
	 	ret = fclose(fp);
		if( ret == -1 ){
			debug_err("fclose %s/gpio%d/value fail！\n", sysgpio, port[i]);
			pin_finish = 129 ;
			goto fail;
		}
	}
	
	while (( pin_finish == 130 ) && ( time_count < time_out )){
		sleep(1);
		time_count++;
		for ( i = 0 ; i < nums ; i++ ){
			sprintf(send, "%s/gpio%d/value", sysgpio, port[i]);
			fp = fopen(send, "r");
			if( fp == NULL ){
				debug_err("fopen %s/gpio%d/value fail！\n", sysgpio, port[i]);
				pin_finish = 128 ;
				goto fail;
			}
	
			while(fgets(result_buf, sizeof(result_buf), fp) != NULL){
				dip_sec[i] = strtol(result_buf, NULL, 0);
			}

			ret = fclose(fp);
			if( ret == -1 ){
				debug_err("fclose %s/gpio%d/value fail！\n", sysgpio, port[i]);
				pin_finish = 129 ;
				goto fail;
			}

			if ( (dip_fir[i] != dip_sec[i]) && (dip_res[i] == 0) ){
				dip_res[i] = 1 ;
				debug_msg("pin [%d] status pass: first %d , second %d!\n", port[i], dip_fir[i], dip_sec[i]);
			}
			usleep(200000); //200ms
		}
		for ( i = 0 ; i < nums ; i++ ){
			if ( dip_res[i] == 1 ){
				pin_finish = 0 ;
			}else{
				pin_finish = 130 ;
				debug_veb("pin number %d need to switch!\n" , port[i]);
				break ;
			}
		}
	}
						
	if ( time_count >= time_out ){
		debug_err("gpio pin input test Fail!!!\n" );
		for ( i = 0 ; i < nums ; i++ ){
			if ( dip_res[port[i]] = 0 ){
				debug_err("pin number %d can't work\n" , port[i]);
			}
		}
		pin_finish = 131 ;
		goto fail;
	}

fail:
	for ( i = 0; i < nums ; i++) {
		debug_msg(" [%d] unexport %d\n", i , port[i]) ;
		sprintf(send, "echo %d > %s", port[i], unexport);
		system(send);
	}

return pin_finish;
}

/*gpio_out
 * 	gpio output pin
 *       
 */  
int gpio_out(int nums, int port[ARRAY_MAX])
{
	int i;
	int pin_finish = 1 ;
	char keyin[32];
	char send[1024];
	char *sysgpio="/sys/class/gpio";
	char *export="/sys/class/gpio/export";
	char *unexport="/sys/class/gpio/unexport";

	for ( i = 0; i < nums ; i++) {
		sprintf(send, "echo %d > %s", port[i], export);
		system(send);
		usleep(1000);
		sprintf(send, "echo low > %s/gpio%d/direction", sysgpio, port[i]);
		system(send);
	}
	
	while ( pin_finish == 1 ) {
		for ( i = 0; i < nums ; i++) {
			debug_msg(" [%d] = pin numbers %d set high!\n", i , port[i]) ;
			sprintf(send, "echo high > %s/gpio%d/direction", sysgpio, port[i]);
			system(send);
			usleep(200000);
		}
		sleep(1);
		for ( i = 0; i < nums ; i++) {
			sprintf(send, "echo low > %s/gpio%d/direction", sysgpio, port[i]);
			system(send);
			usleep(200000);
		}

		printf(" Please keyin the result pass/ng or retry:") ;
		scanf("%s\0",keyin);
		if(strncmp(keyin,"pa",2) == 0){
			pin_finish = 0;
		}else if(strncmp(keyin,"ng",2) == 0){
			pin_finish = 128;
		}
	}

	for ( i = 0; i < nums ; i++) {
		debug_msg(" [%d] unexport %d\n", i , port[i]) ;
		sprintf(send, "echo %d > %s", port[i], unexport);
		system(send);
	}
	
return pin_finish;
}

/*gpio_pair
 * 	gpio pair pin it need even numbers
 *       
 */  
int gpio_pair(int nums, int port[ARRAY_MAX])
{
	int i;
	int ret;
	int pin_finish = 0 ;
	char send[1024];
    char result_buf[32];
	int pin_low[ARRAY_MAX];
	int pin_high[ARRAY_MAX];
	char *sysgpio="/sys/class/gpio";
	char *export="/sys/class/gpio/export";
	char *unexport="/sys/class/gpio/unexport";
	FILE *fp;

	for ( i = 0; i < nums ; i=i+2) {
		debug_msg(" [%d] = %d:%d\n", i , port[i], port[i+1]) ;

		sprintf(send, "echo %d > %s", port[i], export);
		system(send);
		sprintf(send, "echo %d > %s", port[i+1], export);
		system(send);
		
		sprintf(send, "echo low > %s/gpio%d/direction", sysgpio, port[i]);
		system(send);

		sprintf(send, "echo in > %s/gpio%d/direction", sysgpio, port[i+1]);
		system(send);
	
	}
	
	for ( i = 0; i < nums ; i=i+2) {
		sprintf(send, "%s/gpio%d/value", sysgpio, port[i+1]);
		fp = fopen(send, "r");
		if(NULL == fp){
			debug_err("fopen %s/gpio%d/value fail！\n", sysgpio, port[i+1]);
			pin_finish = 128 ;
			goto fail;
		}
	
		while(fgets(result_buf, sizeof(result_buf), fp) != NULL){
			pin_low[i+1] = strtol(result_buf, NULL, 0);
		}
		
		sprintf(send, "echo high > %s/gpio%d/direction", sysgpio, port[i]);
		system(send);

		while(fgets(result_buf, sizeof(result_buf), fp) != NULL){
			pin_high[i+1] = strtol(result_buf, NULL, 0);
		}
		
	 	ret = fclose(fp);
		if( ret == -1 ){
			debug_err("fclose %s/gpio%d/value fail！\n", sysgpio, port[i+1]);
			pin_finish = 129 ;
			goto fail;
		//}else{
		//	debug_msg("命令【%s】子进程结束状态【%d】命令返回值【%d】\r\n", send, ret, WEXITSTATUS(ret));
		}
	}
		
	for ( i = 0; i < nums ; i=i+2) {
		sprintf(send, "echo low > %s/gpio%d/direction", sysgpio, port[i+1]);
		system(send);

		sprintf(send, "echo in > %s/gpio%d/direction", sysgpio, port[i]);
		system(send);
	
	}

	for ( i = 0; i < nums ; i=i+2) {
		sprintf(send, "%s/gpio%d/value", sysgpio, port[i]);
		fp = fopen(send, "r");
		if(NULL == fp){
			debug_err("fopen %s/gpio%d/value fail！\n", sysgpio, port[i]);
			pin_finish = 128 ;
			goto fail;
		}
	
		while(fgets(result_buf, sizeof(result_buf), fp) != NULL){
			pin_low[i] = strtol(result_buf, NULL, 0);
		}
		
		sprintf(send, "echo high > %s/gpio%d/direction", sysgpio, port[i+1]);
		system(send);

		while(fgets(result_buf, sizeof(result_buf), fp) != NULL){
			pin_high[i] = strtol(result_buf, NULL, 0);
		}
		
	 	ret = fclose(fp);
		if( ret == -1 ){
			debug_err("fclose %s/gpio%d/value fail！\n", sysgpio, port[i]);
			pin_finish = 129 ;
			goto fail;
		}
	}

	for ( i = 0; i < nums ; i++) {
		if (pin_low[i] == 1 || pin_high[i] == 0 ){
			debug_err("pin_numbers[%d] can't work!\r\n", port[i]);
			pin_finish = 130 ;
		}
	}	
	
fail:
	for ( i = 0; i < nums ; i++) {
		debug_msg(" [%d] unexport %d\n", i , port[i]) ;
		sprintf(send, "echo %d > %s", port[i], unexport);
		system(send);
	}
	
return pin_finish;
}

/*rs232_client
 * 	rs232 test at client
 *      --rs232 /dev/ttymxc0 
 */  
int rs232_client(char *port)
{
    char buf_r[7],buf_w[]="hello1",passwd[]="hello2";
	int i,result,nread,uart1_fd;
#if 1
	int w_iTIOCM_bit = 0;
	int r_iTIOCM_bit = 0;
#else
    fd_set inputs,testfds;
    struct timeval timeout;
#endif
	struct termios w_TERMIOS = {0x00,};

	uart1_fd = open( port, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if(uart1_fd==-1)				{				
        debug_err("can't open UART %s\n", port);    				
        return -1;				
    }

	w_TERMIOS.c_iflag = IGNPAR ;
	w_TERMIOS.c_cflag = CS8 | CLOCAL | CREAD;
	w_TERMIOS.c_oflag = 0;
	w_TERMIOS.c_lflag = 0;
	w_TERMIOS.c_cc[VMIN] = 0;
	w_TERMIOS.c_cc[VTIME] = 0;
	cfsetspeed( &w_TERMIOS, B115200 );
	tcflush( uart1_fd, TCIFLUSH );
	tcsetattr( uart1_fd, TCSANOW, &w_TERMIOS );

//    FD_ZERO(&inputs);
//    FD_SET(uart1_fd,&inputs);

	debug_eng(" %s is client!\n", port) ;
	for(i=0;i<5;i++){

		w_iTIOCM_bit = TIOCM_DTR;
		#ifdef DTR_LowActive
		ioctl( uart1_fd, TIOCMBIS, &w_iTIOCM_bit );
		#else
		ioctl( uart1_fd, TIOCMBIC, &w_iTIOCM_bit );//clear DTR for DCD low active
		#endif

		ioctl( uart1_fd, TIOCMGET, &r_iTIOCM_bit );
		debug_msg("DCD=%s, CTS=%s\n",(r_iTIOCM_bit & TIOCM_CD)?"true":"false",(r_iTIOCM_bit & TIOCM_CTS)?"true":"false");
		if(((r_iTIOCM_bit & TIOCM_CD)) && (r_iTIOCM_bit & TIOCM_CTS)){
			nread=write(uart1_fd,buf_w,7);
			debug_msg("wrt %d bytes: %s\n",nread,buf_w);

			w_iTIOCM_bit = TIOCM_RTS;
			ioctl( uart1_fd, TIOCMBIS, &w_iTIOCM_bit );//set RTS

			nread=read(uart1_fd,buf_r,7);
			debug_veb("read %d bytes: %s\n",nread,buf_r);

			if(nread > 0){
				w_iTIOCM_bit = TIOCM_RTS;
				ioctl( uart1_fd, TIOCMBIC, &w_iTIOCM_bit );//set RTS

				buf_r[6]='\0';
				if(strcmp(passwd, buf_r) == 0){						
					debug_msg("passwd %d bytes: %s\n",sizeof(passwd),passwd);
					w_iTIOCM_bit = TIOCM_DTR;
					#ifdef DTR_LowActive
					ioctl( uart1_fd, TIOCMBIC, &w_iTIOCM_bit );
					#else
					ioctl( uart1_fd, TIOCMBIS, &w_iTIOCM_bit );//set DTR for DCD high inactive
					#endif

					return 0;
				}
			}
		}
		usleep(1000000);
		debug_veb("retry %d times\n",i);

	}

	w_iTIOCM_bit = TIOCM_DTR;
	#ifdef DTR_LowActive
	ioctl( uart1_fd, TIOCMBIC, &w_iTIOCM_bit );
	#else
	ioctl( uart1_fd, TIOCMBIS, &w_iTIOCM_bit );//set DTR for DCD high inactive
	#endif

	close(uart1_fd);
	return -3;
}

/*rs232_server
 * 	rs232 test at server
 *       
 */  
int rs232_server(char *port)
{
    char buf_r[7],buf_w[]="hello2",passwd[]="hello1";
	int result,nread,uart1_fd;
#if 1
	int w_iTIOCM_bit = 0;
	int r_iTIOCM_bit = 0;
	int i = 0;
#else
    fd_set inputs,testfds;
    struct timeval timeout;
#endif
	struct termios w_TERMIOS = {0x00,};

	uart1_fd = open( port, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if(uart1_fd==-1)				{				
        debug_err("can't open UART %s\n", port);    				
        exit(-1);				
    }

	w_TERMIOS.c_iflag = IGNPAR ;
	w_TERMIOS.c_cflag = CS8 | CLOCAL | CREAD;
	w_TERMIOS.c_oflag = 0;
	w_TERMIOS.c_lflag = 0;
	w_TERMIOS.c_cc[VMIN] = 0;
	w_TERMIOS.c_cc[VTIME] = 0;
	cfsetspeed( &w_TERMIOS, B115200 );
	tcflush( uart1_fd, TCIFLUSH );
	tcsetattr( uart1_fd, TCSANOW, &w_TERMIOS );

//    FD_ZERO(&inputs);
//    FD_SET(uart1_fd,&inputs);

	w_iTIOCM_bit = TIOCM_DTR;
	#ifdef DTR_LowActive
	ioctl( uart1_fd, TIOCMBIS, &w_iTIOCM_bit );
	#else
	ioctl( uart1_fd, TIOCMBIC, &w_iTIOCM_bit );//clear DTR for DCD low active
	#endif

	debug_eng(" %s is server!\n", port) ;
	while(1){

		ioctl( uart1_fd, TIOCMGET, &r_iTIOCM_bit );
		if(r_iTIOCM_bit & TIOCM_CD){
			w_iTIOCM_bit = TIOCM_RTS;
			ioctl( uart1_fd, TIOCMBIS, &w_iTIOCM_bit );//set RTS

			nread=read(uart1_fd,buf_r,7);
			debug_msg("read %d bytes: %s\n",nread,buf_r);

			if(nread > 0){
				w_iTIOCM_bit = TIOCM_RTS;
				ioctl( uart1_fd, TIOCMBIC, &w_iTIOCM_bit );//clear RTS

				buf_r[6]='\0';
				debug_msg("passwd %d bytes: %s\n",sizeof(passwd),passwd);
				if(strcmp(passwd, buf_r) == 0){
					for(i=0;i<5;i++){
						ioctl( uart1_fd, TIOCMGET, &r_iTIOCM_bit );
						if(r_iTIOCM_bit & TIOCM_CTS){						
							nread=write(uart1_fd,buf_w,7);
							debug_veb("wrt %d bytes: %s\n",nread,buf_w);
							break;
						}else{
							debug_err("no CTS\n");
							usleep(1000000);
						}
					}
				}
			}
		}else{
			debug_err("no DCD\n");
		}
		usleep(1000000);

	}
}

/*rs232_repeat
 * 	rs232 test for repeat send message
 *       
 */  
int rs232_repeat(char *port)
{
    char *mesg="Hello, this is RTX test. run times:" ;
	char i = 0;
	int uart1_fd ;
	char send[1024];
	struct termios w_TERMIOS = {0x00,};

	uart1_fd = open( port, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if(uart1_fd==-1)				{				
        debug_err("can't open UART %s\n", port);    				
        exit(-1);				
    }

	w_TERMIOS.c_iflag = IGNPAR ;
	w_TERMIOS.c_cflag = CS8 | CLOCAL | CREAD;
	w_TERMIOS.c_oflag = 0;
	w_TERMIOS.c_lflag = 0;
	w_TERMIOS.c_cc[VMIN] = 0;
	w_TERMIOS.c_cc[VTIME] = 0;
	cfsetspeed( &w_TERMIOS, B115200 );
	tcflush( uart1_fd, TCIFLUSH );
	tcsetattr( uart1_fd, TCSANOW, &w_TERMIOS );

//    FD_ZERO(&inputs);
//    FD_SET(uart1_fd,&inputs);

	while(1){
		i++;
		sprintf(send, "echo %s %d > %s", mesg, i, port);
		debug_veb("send_cmd: %s\n",send);
		system(send);
		sleep(1);
	}
}

/*rs485_client
 * 	for rs485 test at client
 *      --rs485 /dev/ttymxc3 
 */  
int rs485_client(char *port)
{
    char buf_r[7],buf_w[]="hello1",passwd[]="hello2";
	int i,result,nread,uart1_fd;

    fd_set inputs,testfds;
    struct timeval timeout;

	struct termios w_TERMIOS = {0x00,};
	
	uart1_fd = open( port, O_RDWR | O_NOCTTY );//Bishop:ttymxc0, AEG-200:ttymxc1
	if(uart1_fd==-1)				{				
        debug_err("can't open UART %s\n", port);    				
        return -1;				
    }

	w_TERMIOS.c_iflag = IGNPAR ;
	w_TERMIOS.c_cflag = CS8 | CLOCAL | CREAD;
	w_TERMIOS.c_oflag = 0;
	w_TERMIOS.c_lflag = 0;
	w_TERMIOS.c_cc[VMIN] = 0;
	w_TERMIOS.c_cc[VTIME] = 0;
	cfsetspeed( &w_TERMIOS, B115200 );
	tcflush( uart1_fd, TCIFLUSH );
	tcsetattr( uart1_fd, TCSANOW, &w_TERMIOS );

    FD_ZERO(&inputs);
    FD_SET(uart1_fd,&inputs);

	debug_eng(" %s is client!\n", port) ;
	for(i=0;i<5;i++){
		nread=write(uart1_fd,buf_w,7);
		debug_msg("wrt %d bytes: %s\n",nread,buf_w);
		usleep(500000);
		testfds = inputs;
		timeout.tv_sec = 1;//2;
		timeout.tv_usec = 0;//500000;

		result = select(uart1_fd+1,&testfds,NULL,NULL,&timeout);
		debug_msg("result=%d\n",result);

		switch(result){
		case 0:
			debug_err("timeout\n");
			break;
		case -1:
			debug_err("sel err\n");
			exit(-2);
		default:
			if(FD_ISSET(uart1_fd,&testfds)){
				nread=read(uart1_fd,buf_r,7);
				if(nread > 0){
					debug_veb("read %d %d bytes: %s\n",nread,sizeof(buf_r),buf_r);
					buf_r[6]='\0';
					if(strcmp(passwd, buf_r) == 0){						
						debug_msg("passwd %d bytes: %s\n",sizeof(passwd),passwd);
						return 0;
					}
				}		
			}
			break;
		}
	}
	close(uart1_fd);
	return -3;
}
 
/*rs485_server
 * 	for rs485 test at server
 *       
 */  
int rs485_server(char *port)
{
    char buf_r[7],buf_w[]="hello2",passwd[]="hello1";
	int result,nread,uart1_fd;

    fd_set inputs,testfds;
    struct timeval timeout;

	struct termios w_TERMIOS = {0x00,};

	uart1_fd = open( port, O_RDWR | O_NOCTTY );//Bishop:ttymxc0, AEG-200:ttymxc1
	if(uart1_fd==-1)				{				
        debug_err("can't open UART %s\n", port);    				
        exit(-1);				
    }

	w_TERMIOS.c_iflag = IGNPAR ;
	w_TERMIOS.c_cflag = CS8 | CLOCAL | CREAD;
	w_TERMIOS.c_oflag = 0;
	w_TERMIOS.c_lflag = 0;
	w_TERMIOS.c_cc[VMIN] = 0;
	w_TERMIOS.c_cc[VTIME] = 0;
	cfsetspeed( &w_TERMIOS, B115200 );
	tcflush( uart1_fd, TCIFLUSH );
	tcsetattr( uart1_fd, TCSANOW, &w_TERMIOS );

    FD_ZERO(&inputs);
    FD_SET(uart1_fd,&inputs);

	debug_eng(" %s is server!\n", port) ;
	while(1){
		testfds = inputs;
		timeout.tv_sec = 1;//2;
		timeout.tv_usec = 0;//500000;

		result = select(uart1_fd+1,&testfds,NULL,NULL,&timeout);
		debug_msg("result=%d\n",result);

		switch(result){
		case 0:
			debug_err("timeout\n");
			break;
		case -1:
			debug_err("sel err\n");
			exit(-2);
		default:
			if(FD_ISSET(uart1_fd,&testfds)){
				nread=read(uart1_fd,buf_r,7);
				if(nread > 0){
					buf_r[6]='\0';
					debug_msg("read %d %d bytes: %s\n",nread,sizeof(buf_r),buf_r);
					debug_msg("passwd %d bytes: %s\n",sizeof(passwd),passwd);
					if(strcmp(passwd, buf_r) == 0){
						nread=write(uart1_fd,buf_w,7);
						debug_veb("wrt %d bytes: %s",nread,buf_w);
					}
				}		
			}
			break;
		}
	}
}

/*startup_func
 * 	for startup test
 *      --startup ./test.txt
 */  
int startup_func(char *path)
{
	int ret;
	int count = 0;
	char str[128];
	char result_buf[256];
	FILE *fp;
	
	fp = fopen(path, "r");
	if( fp == NULL ){
		goto create;
	}
	while(fgets(result_buf, sizeof(result_buf), fp) != NULL){
		sscanf(result_buf, "%s%d", str, &count);
		debug_war("result_buf : %s, %s, %d\n", result_buf, str, count) ;
	}
	ret = fclose(fp);
	if( fp == NULL ){
		return 128;
	}

create:
	fp = fopen(path, "w+");
	count++;
	fprintf(fp, "starupcount= %d\n", count);
	ret = fclose(fp);
	if( fp == NULL ){
		return 128;
	}
	
	debug_eng("startup times : %d\n", count) ;
	return 0;
}

/*display_show
 * 	for video output test
 *      --display /dev/fb0
 */  
int display_show(char *dev, int time_out)
{
    int fbfd = 0, fbsize = 0; 
    int i=0; 
    struct fb_var_screeninfo vinfo; 
    struct fb_fix_screeninfo finfo; 
    long int screensize = 0,cnt=0; 
    unsigned char *fbp;
    int time_count = 0;
    int dis_finish = 128 ;
	char keyin[32];
 	
    fbfd = open( dev, O_RDWR); 
 
    /* Get fixed screen information */         
    ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo); 

    /* Get variable screen information */ 
    ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo); 
    /* Figure out the size of the screen in bytes */ 
    fbsize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;   
    screensize = vinfo.xres * vinfo.yres ; 
    
    /* Map the device to memory */ 
    fbp=(unsigned char*)mmap(0, fbsize, PROT_READ|PROT_WRITE, MAP_SHARED, fbfd, 0); 
    
    debug_msg("x:%d y:%d bpp:%d\n",vinfo.xres,vinfo.yres,vinfo.bits_per_pixel); 
    // 16bit : high: RRRRRGGG  low: GGGBBBBB      
retry:
	time_count = 0;
	while( time_count < time_out ) { 
		for(i=0;i<screensize/2;i++){ 
			if(cnt%3==0){ 
				*(fbp+2*i) = 0x00; 
				*(fbp+2*i+1) = 0xf8; 
				*(fbp+2*i+screensize) = 0x1f; 
				*(fbp+2*i+1+screensize) = 0x00; 
			}else if(cnt%3==1) { 
				*(fbp+2*i) = 0xe0; 
				*(fbp+2*i+1) = 0x07; 
				*(fbp+2*i+screensize) = 0x00; 
				*(fbp+2*i+1+screensize) = 0xf8; 
			}else{ 
				*(fbp+2*i) = 0x1f; 
				*(fbp+2*i+1) = 0x00; 
				*(fbp+2*i+screensize) = 0xe0; 
				*(fbp+2*i+1+screensize) = 0x07; 
			} 
		} 
		time_count++;
		sleep(1); 
		debug_msg("time count : %d\n", time_count); 
		cnt++;                                                           
	}
	
	printf(" Please keyin the result pass/ng or retry:") ;
	scanf("%s\0",keyin);
	if(strncmp(keyin,"pa",2) == 0){
		dis_finish = 0;
	}else if(strncmp(keyin,"ng",2) == 0){
		dis_finish = 128;
	}else{
		goto retry;
	}

	munmap(fbp,screensize); 
	close(fbfd);
	      
	return dis_finish;
}  

/*open_device
 * 	to Open device at /dev/*
 *      --devopen /dev/watchdog 60
 */  
int open_device(char *dev, int time_out)
{
    int fd=NULL; 
 	int i;
 	
    fd = open( dev, O_RDWR); 
 
	if( fd == NULL ){
		debug_err(" %s fail\n", dev) ;
		return 128;
	}
	
	for (i = 1; i < time_out+1; i++){
		sleep(1);
		debug_eng("delay time is %d, count is %d\n", time_out, i) ;		
	}
	
 	close(fd);
	      
	return 0;
}  

/*cat_device node
 * 	to cat node at /sys/devices/platform/anatop_thermal.0/thermal_degree times
 *      --cat /sys/devices/platform/anatop_thermal.0/thermal_degree 60
 */  
int cat_device(char *dev, int time_out)
{
 	int i, ret, data;
    char send[1024];
    char result_buf[32];
	FILE *fp;

	debug_eng("cat %s\n", dev);
	for (i = 1; i < time_out+1; i++){
forevery:
		fp = fopen(dev, "r");
		if(NULL == fp){
			debug_err("fopen %s fail！\n", dev);
			goto fail;
		}
		while(fgets(result_buf, sizeof(result_buf), fp) != NULL){
			debug_eng("cat times is %d, count is %d, degree(C): %s", time_out, i, result_buf) ;		
		}
		ret = fclose(fp);
		if( ret == -1 ){
			debug_err("fclose %s fail！\n", dev);
			goto fail;
		}
		sleep(1);
	}
	if( time_out == 0) goto forevery;

fail:
	return 0;
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
	while ((ch = getopt_long(argc, argv, COMMON_SHORT_OPTSTR, COMMON_LONG_OPTSTR, &long_index)) != EOF) {
		switch(ch) {
		case 0:	//gpio
			{
				int nums=argc-3;
				int pin_num[ARRAY_MAX];		
				int iCount = 0 ;

				for( iCount = 3; iCount < argc ; iCount++)
				{
					pin_num[iCount-3] = strtol(argv[iCount] ,NULL, 0);
				}

				argv++;
				if (strcmp( argv[1], "in") == 0){
					ret = gpio_in(nums, pin_num) ;
				}else if (strcmp( argv[1], "out") == 0){
					ret = gpio_out(nums, pin_num) ;					
				}else if (strcmp( argv[1], "pair") == 0){
					ret = gpio_pair(nums, pin_num) ;					
				}else{
					usage_gpio() ;	
				}
				return ret ;
			break ;
			}
			
		case 1: //rs232
				argv++ ;
				if (strcmp( argv[1], "client") == 0){
					ret = rs232_client(argv[2]) ;
				}else if (strcmp( argv[1], "server") == 0){
					ret = rs232_server(argv[2]) ;					
				}else if (strcmp( argv[1], "repeat") == 0){
					ret = rs232_repeat(argv[2]) ;					
				}else{
					usage_rs232() ;	
				}
				return ret ;
			break ;
			
		case 2: //rs485
				argv++ ;
				if (strcmp( argv[1], "client") == 0){
					ret = rs485_client(argv[2]) ;
				}else if (strcmp( argv[1], "server") == 0){
					ret = rs485_server(argv[2]) ;					
				}else{
					usage_rs485() ;	
				}
				return ret ;
			break ;
			
		case 3: //startup
				argv++ ;
				ret = startup_func(argv[1]) ;
				return ret ;
			break ;
			
		case 4: //display
		case 'd':
			{
				if(argc < 3){
					usage_display() ;
				}else{	
					int show_time = 10 ;
					if( argv[3] ) show_time = strtol(argv[3] ,NULL, 0);
					argv++ ;
					ret = display_show(argv[1],show_time) ;
				}
				return ret ;
			break ;
			}
			
		case 5: //devopen
			{	
				if(argc < 3){
					usage_devopen() ;
				}else{	
					int wait_time = 10 ;
					if( argv[3] ) wait_time = strtol(argv[3] ,NULL, 0);
					argv++;
					ret = open_device(argv[1], wait_time) ;
				}
				return ret ;
			break ;
			}
			
		case 6: //cat
			{	
				if(argc < 3){
					usage_cat() ;
				}else{	
					int cat_time = 0 ;
					if( argv[3] ) cat_time = strtol(argv[3] ,NULL, 0);
					argv++;
					ret = cat_device(argv[1], cat_time) ;
				}
				return ret ;
			break ;
			}
			
		case 'v': //get version
		{
			get_version();	
			break ;
		}

		case 'z': //test
		{
			int i1, i2, i3, i4 ;
			if (sscanf(optarg, "%u.%u.%u.%u", &i1, &i2, &i3, &i4) == 4) {
				debug_msg("%d,%d,%d,%d\n", i1, i2, i3, i4) ;
			}	
			break ;
		}

		default:
			usage() ;
		}
	}

	return 0 ;
}


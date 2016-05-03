#include "mp.h"

/* options args */
static const char *short_options = "?hl";

static struct option long_options[] = {
  {"help"           , no_argument,       0, 'h'                      },
  {"clean"        	, no_argument, 		 0, 'c'                      },
  {"write"          , required_argument, 0, 'w'                      },
  {"read"	        , required_argument, 0, 'r'                      },
  {"delete"         , required_argument, 0, 'd'                      },
  {"memory"         , no_argument,       0, 'm'                      },
  {0                , no_argument,       0, 0                        }
};

static void show_usage(){
	printf("Options:\n");
	printf("        --clean     clean all variable\n");
	printf("        --write  	variable=String\n");
	printf("        --delete  	variable\n");
    printf("        --read     	variable\n");
    printf("        --memory  	memory calibration at first startup\n");
	printf("        --help    	help\n");
}

static void show_read_usage(){
	printf("read variable:\n");
	printf("        --read all\n");
	printf("        --read ubMagicCode\n");
	printf("        --read ubMAC01\n");
	printf("        --read ubMAC02\n");
	printf("        --read ubMAC03\n");
	printf("        --read ubProductName\n");
	printf("        --read ubProductSerialNO\n");
	printf("        --read ubBSPVersion\n");
//	printf("        --read ulPasswordLen\n");
//	printf("        --read ubPassword\n");
	printf("        --read ulFunction\n");
	printf("        --read ulCmd\n");
	printf("        --read ulStatus\n");
}

static void show_write_usage(){
	printf("write variable:\n");
	printf("        --write ubMagicCode=5aa5aa55\n");
	printf("        --write ubMAC01=00:12:33:ab:bb:df\n");
	printf("        --write ubMAC02=00:12:33:ab:bb:cf\n");
	printf("        --write ubMAC03=00:12:33:ab:bb:ef\n");
	printf("        --write ubProductName=RTX-R064Roymark-Digitalsinige(max128)\n");
	printf("        --write ubProductSerialNO=RTX001258746(max32)\n");
	printf("        --write ubBSPVersion=\n");
//	printf("        --write ulPasswordLen=\n");
//	printf("        --write ubPassword=\n");
	printf("        --write ulFunction=\n");
	printf("        --write ulCmd=\n");
	printf("        --write ulStatus=\n");
}


typedef struct __BOOTSEL_INFO__ {
	unsigned long ulCheckCode ;
	unsigned char ubMagicCode[16] ;
	unsigned char ubMAC01[24] ;
	unsigned char ubMAC02[24] ;
	unsigned char ubMAC03[24] ;
	unsigned char ubProductName[128] ;
	unsigned char ubProductSerialNO[32] ;
	unsigned char ubBSPVersion[32] ;
	unsigned long ulPasswordLen ;
	unsigned char ubPassword[32] ;
	unsigned long ulFunction ;
	unsigned long ulCmd ;
	unsigned long ulStatus ;
	unsigned char ubRecv[180] ;
} bootselinfo ;

#define BLOCK_SKIP (768 * 1024)
#define BLOCK_SKIP1 (3 * 512)

#ifdef KERNEL_ANDROID
static	const char path_utf[] =  "/dev/block/mmcblk0";
#else
static	const char path_utf[] =  "/dev/mmcblk0";
#endif
static int g_mmcblk0_fd = 0;
static bootselinfo bootselinfodata ;

#define REV_SUCCESS 1
#define REV_FAIL    0

static unsigned long quick_atoi(char *a, unsigned long slen)
{
	unsigned long i, num = 0, digit;

	for (i = 0; i < slen; i++) {
		if (a[i] >= '0' && a[i] <= '9') {
			digit = a[i] - '0';
		} else if (a[i] >= 'a' && a[i] <= 'f') {
			digit = a[i] - 'a' + 10;
		} else if (a[i] >= 'A' && a[i] <= 'F') {
			digit = a[i] - 'A' + 10;
		} else {
			printf("ERROR: %c\n", a[i]);
			return -1;
		}
		num = (num * 16) + digit;
	}

    return num;
}

static int Init_Ubootlist(){
	int read_size = 0;
	int val = REV_SUCCESS;
	
	memset( (void *)&bootselinfodata , 0 , sizeof(bootselinfo) ) ;

	g_mmcblk0_fd = open( path_utf, O_RDWR );

	if( g_mmcblk0_fd <= 0 ){
		printf( "[Create List] Error opening file: %d\n", g_mmcblk0_fd );
		return REV_FAIL;
	}

	lseek( g_mmcblk0_fd, BLOCK_SKIP, SEEK_SET );

	read_size = read( g_mmcblk0_fd, &bootselinfodata, sizeof(bootselinfo) );

	if( read_size != sizeof(bootselinfo) ){
		printf( "read_size = %d\n", read_size );

		val = REV_FAIL;
	}

	close( g_mmcblk0_fd );
	g_mmcblk0_fd = 0;
	
	return val;
}

static int Clean_Ubootlist(){
	int write_size = 0;
	int val = REV_SUCCESS;
	
	memset( (void *)&bootselinfodata , 0 , sizeof(bootselinfo) ) ;

	g_mmcblk0_fd = open( path_utf, O_RDWR );

	if( g_mmcblk0_fd <= 0 ){
		printf( "[Create List] Error opening file: %d\n", g_mmcblk0_fd );
		return REV_FAIL;
	}

	lseek( g_mmcblk0_fd, BLOCK_SKIP, SEEK_SET );

	write_size = write( g_mmcblk0_fd, &bootselinfodata, sizeof(bootselinfo) );

	if( write_size != sizeof(bootselinfo) ){
		printf( "write_size = %d\n", write_size );
		val = REV_FAIL;
	}

	close( g_mmcblk0_fd );
	g_mmcblk0_fd = 0;
	
	return val;
}

static int Write_Ubootlist(char* var){
	int write_size = 0;
	int val = REV_SUCCESS;
	int i;
    char* variable = NULL;
    char* data = NULL;
	char rdata[32];
	unsigned long value;
	
    variable = strtok( var, "=" );
    data = strtok( NULL, "\r\n" );
    
	g_mmcblk0_fd = open( path_utf, O_RDWR );

	if( g_mmcblk0_fd <= 0 ){
		printf( "[Create List] Error opening file: %d\n", g_mmcblk0_fd );
		return REV_FAIL;
	}
	
	printf("var:%s; variable: %s; data:%s\n", var, variable, data);

	if( strncmp(var, "ulCheck", 7) == 0 ){
		value = strtoul(data, NULL, 16);
		bootselinfodata.ulCheckCode = value;
	}else if( strncmp(var, "ubMagic", 7) == 0 ){
		for(i = 0; i < 16; i++){
			memcpy(rdata, data, 2);
			rdata[3] = '\0';
			value = quick_atoi(rdata, 2);	
			if( value != -1 )bootselinfodata.ubMagicCode[i] = value;
			if ((++data)[0] == '\0') {
				printf("ERROR: Odd string input\n");
				val = REV_FAIL;
				break;
			}
			if ((++data)[0] == '\0') {
				printf("Successful\n");
				break;
			}
		}
	}else if( strncmp(var, "ubMAC01", 7) == 0 ){
		memcpy(bootselinfodata.ubMAC01, data, strlen(data));
	}else if( strncmp(var, "ubMAC02", 7) == 0 ){
		memcpy(bootselinfodata.ubMAC02, data, strlen(data));
	}else if( strncmp(var, "ubMAC03", 7) == 0 ){
		memcpy(bootselinfodata.ubMAC03, data, strlen(data));
	}else if( strncmp(var, "ubProductName", 13) == 0 ){
		memcpy(bootselinfodata.ubProductName, data, strlen(data));
	}else if( strncmp(var, "ubProductSerialNO", 17) == 0 ){
		memcpy(bootselinfodata.ubProductSerialNO, data, strlen(data));
	}else if( strncmp(var, "ubBSPVe", 7) == 0 ){
		memcpy(bootselinfodata.ubBSPVersion, data, strlen(data));
/*	}else if( strncmp(var, "ulPasswordLen", 13) == 0 ){
		value = strtoul(data, NULL, 10);
		bootselinfodata.ulPasswordLen = value;
	}else if( strncmp(var, "ubPassword", 10) == 0 ){
		memcpy(bootselinfodata.ubPassword, data, strlen(data));
*/	}else if( strncmp(var, "ulFunction", 10) == 0 ){
		value = strtoul(data, NULL, NULL);
		bootselinfodata.ulFunction = value;
	}else if( strncmp(var, "ulCmd", 5) == 0 ){
		value = strtoul(data, NULL, 10);
		bootselinfodata.ulCmd = value;
	}else if( strncmp(var, "ulStatus", 8) == 0 ){
		value = strtoul(data, NULL, 10);
		bootselinfodata.ulStatus = value;
	}else{
		show_write_usage();
		val = REV_FAIL;
	}

	lseek( g_mmcblk0_fd, BLOCK_SKIP, SEEK_SET );

	write_size = write( g_mmcblk0_fd, &bootselinfodata, sizeof(bootselinfo) );

	if( write_size != sizeof(bootselinfo) ){
		printf( "write_size = %d\n", write_size );
		val = REV_FAIL;
	}

	close( g_mmcblk0_fd );
	g_mmcblk0_fd = 0;
	
	return val;
}

static int Read_Ubootlist(char* var){
	int write_size = 0;
	int val = REV_SUCCESS;
	int i;
	
	if( strncmp(var, "all", 3) == 0 ){
		printf("        ulCheckCode:0x%08x\n", bootselinfodata.ulCheckCode);
		printf("        ubMagicCode:");
		for(i = 0; i < sizeof(bootselinfodata.ubMagicCode); i++){
			printf(" 0x%02x", bootselinfodata.ubMagicCode[i]);			
		}
		printf("\n");
		printf("        ubMAC01:%s\n", strlen(bootselinfodata.ubMAC01)>15?bootselinfodata.ubMAC01:"null");
		printf("        ubMAC02:%s\n", strlen(bootselinfodata.ubMAC02)>15?bootselinfodata.ubMAC02:"null");
		printf("        ubMAC03:%s\n", strlen(bootselinfodata.ubMAC03)>15?bootselinfodata.ubMAC03:"null");
		printf("        ubProductName:%s\n", strlen(bootselinfodata.ubProductName)>3?bootselinfodata.ubProductName:"null");
		printf("        ubProductSerialNO:%s\n", strlen(bootselinfodata.ubProductSerialNO)>3?bootselinfodata.ubProductSerialNO:"null");
		printf("        ubBSPVersion:%s\n", strlen(bootselinfodata.ubBSPVersion)>3?bootselinfodata.ubBSPVersion:"null");
		//printf("        ulPasswordLen:%lu\n", bootselinfodata.ulPasswordLen);
		//printf("        ubPassword:%s\n", strlen(bootselinfodata.ubPassword)>3?bootselinfodata.ubPassword:"null");
		printf("        ulFunction:0x%08x\n", bootselinfodata.ulFunction);
		printf("        ulCmd:%lu\n", bootselinfodata.ulCmd);
		printf("        ulStatus:%lu\n", bootselinfodata.ulStatus);
	}else if( strncmp(var, "ulCheck", 7) == 0 ){
		printf("        ulCheckCode:0x%08x\n", bootselinfodata.ulCheckCode);
	}else if( strncmp(var, "ubMagic", 7) == 0 ){
		printf("        ubMagicCode:");
		for(i = 0; i < sizeof(bootselinfodata.ubMagicCode); i++){
			printf(" 0x%02x", bootselinfodata.ubMagicCode[i]);			
		}
		printf("\n");
	}else if( strncmp(var, "ubMAC01", 7) == 0 ){
		printf("        ubMAC01:%s\n", strlen(bootselinfodata.ubMAC01)>15?bootselinfodata.ubMAC01:"null");
	}else if( strncmp(var, "ubMAC02", 7) == 0 ){
		printf("        ubMAC02:%s\n", strlen(bootselinfodata.ubMAC02)>15?bootselinfodata.ubMAC02:"null");
	}else if( strncmp(var, "ubMAC03", 7) == 0 ){
		printf("        ubMAC03:%s\n", strlen(bootselinfodata.ubMAC03)>15?bootselinfodata.ubMAC03:"null");
	}else if( strncmp(var, "ubProductName", 13) == 0 ){
		printf("        ubProductName:%s\n", strlen(bootselinfodata.ubProductName)>3?bootselinfodata.ubProductName:"null");
	}else if( strncmp(var, "ubProductSerialNO", 17) == 0 ){
		printf("        ubProductSerialNO:%s\n", strlen(bootselinfodata.ubProductSerialNO)>3?bootselinfodata.ubProductSerialNO:"null");
	}else if( strncmp(var, "ubBSPVe", 7) == 0 ){
		printf("        ubBSPVersion:%s\n", strlen(bootselinfodata.ubBSPVersion)>3?bootselinfodata.ubBSPVersion:"null");
/*	}else if( strncmp(var, "ulPasswordLen", 13) == 0 ){
		printf("        ulPasswordLen:%lu\n", bootselinfodata.ulPasswordLen);
	}else if( strncmp(var, "ubPassword", 10) == 0 ){
		printf("        ubPassword:%s\n", strlen(bootselinfodata.ubPassword)>3?bootselinfodata.ubPassword:"null");
*/	}else if( strncmp(var, "ulFunction", 10) == 0 ){
		printf("        ulFunction:0x%08x\n", bootselinfodata.ulFunction);
	}else if( strncmp(var, "ulCmd", 5) == 0 ){
		printf("        ulCmd:%lu\n", bootselinfodata.ulCmd);
	}else if( strncmp(var, "ulStatus", 8) == 0 ){
		printf("        ulStatus:%lu\n", bootselinfodata.ulStatus);
	}else{
		show_read_usage();
		val = REV_FAIL;
	}
		
	return val;
}

static int Delete_Ubootlist(char* var){
	int write_size = 0;
	int val = REV_SUCCESS;
	int i;
	unsigned long value;
	    
	g_mmcblk0_fd = open( path_utf, O_RDWR );

	if( g_mmcblk0_fd <= 0 ){
		printf( "[Create List] Error opening file: %d\n", g_mmcblk0_fd );
		return REV_FAIL;
	}
	
	if( strncmp(var, "ulCheck", 7) == 0 ){
		bootselinfodata.ulCheckCode = 0;
	}else if( strncmp(var, "ubMagic", 7) == 0 ){
		memset(bootselinfodata.ubMagicCode , 0, sizeof( bootselinfodata.ubMagicCode[i] ));
	}else if( strncmp(var, "ubMAC01", 7) == 0 ){
		memset(bootselinfodata.ubMAC01, 0, sizeof(bootselinfodata.ubMAC01));
	}else if( strncmp(var, "ubMAC02", 7) == 0 ){
		memset(bootselinfodata.ubMAC02, 0, sizeof(bootselinfodata.ubMAC02));
	}else if( strncmp(var, "ubMAC03", 7) == 0 ){
		memset(bootselinfodata.ubMAC03, 0, sizeof(bootselinfodata.ubMAC03));
	}else if( strncmp(var, "ubProductName", 13) == 0 ){
		memset(bootselinfodata.ubProductName, 0, sizeof(bootselinfodata.ubProductName));
	}else if( strncmp(var, "ubProductSerialNO", 17) == 0 ){
		memset(bootselinfodata.ubProductSerialNO, 0, sizeof(bootselinfodata.ubProductSerialNO));
	}else if( strncmp(var, "ubBSPVe", 7) == 0 ){
		memset(bootselinfodata.ubBSPVersion, 0, sizeof(bootselinfodata.ubBSPVersion));
/*	}else if( strncmp(var, "ulPasswordLen", 13) == 0 ){
		bootselinfodata.ulPasswordLen = 0;
	}else if( strncmp(var, "ubPassword", 10) == 0 ){
		memset(bootselinfodata.ubPassword, 0, sizeof(bootselinfodata.ubPassword));
*/	}else if( strncmp(var, "ulFunction", 10) == 0 ){
		bootselinfodata.ulFunction = 0;
	}else if( strncmp(var, "ulCmd", 5) == 0 ){
		bootselinfodata.ulCmd = 0;
	}else if( strncmp(var, "ulStatus", 8) == 0 ){
		bootselinfodata.ulStatus = 0;
	}else{
		show_read_usage();
		val = REV_FAIL;
	}

	lseek( g_mmcblk0_fd, BLOCK_SKIP, SEEK_SET );

	write_size = write( g_mmcblk0_fd, &bootselinfodata, sizeof(bootselinfo) );

	if( write_size != sizeof(bootselinfo) ){
		printf( "write_size = %d\n", write_size );
		val = REV_FAIL;
	}

	close( g_mmcblk0_fd );
	g_mmcblk0_fd = 0;
	
	return val;
}

static int Memory_Calibration(){
	int read_size = 0;
	int write_size = 0;
	int val = REV_SUCCESS;
	
	memset( (void *)&bootselinfodata , 0 , sizeof(bootselinfo) ) ;

	g_mmcblk0_fd = open( path_utf, O_RDWR );

	if( g_mmcblk0_fd <= 0 ){
		printf( "[Create List] Error opening file: %d\n", g_mmcblk0_fd );
		return REV_FAIL;
	}

	lseek( g_mmcblk0_fd, BLOCK_SKIP1, SEEK_SET );

	read_size = read( g_mmcblk0_fd, &bootselinfodata, sizeof(bootselinfo) );

	if( read_size != sizeof(bootselinfo) ){
		printf( "read_size = %d\n", read_size );
		val = REV_FAIL;
	    goto done;

	}

	lseek( g_mmcblk0_fd, BLOCK_SKIP1, SEEK_SET );

	if(bootselinfodata.ulCheckCode == 0x01 && bootselinfodata.ubMagicCode[0] == 0x98 && bootselinfodata.ubMagicCode[1] == 0x07 ){
		bootselinfodata.ulCheckCode &= ~0x01;
		write_size = write( g_mmcblk0_fd, &bootselinfodata, sizeof(bootselinfo) );

		if( write_size != sizeof(bootselinfo) ){
			printf( "write_size = %d\n", write_size );
		    printf( "Memory calibration Fail\n");
			val = REV_FAIL;
		}else{
			printf("Memory calibration Pass\n");
		 }
	}else{
		printf( "Memory calibration Fail\n");
	}

done:
	close( g_mmcblk0_fd );
	g_mmcblk0_fd = 0;
	
	return val;
}

int main(int argc, char **argv)
{
    int c ='?';
    int option_index = 0;
	char* variable = NULL;
	char* data = NULL;
	int error = 0;
	int result = 0;

	error = Init_Ubootlist();

	if(error == REV_FAIL){
		goto error_message;
	}
        while( (c = getopt_long(argc, argv, short_options,long_options, &option_index)) != EOF ){
		switch( c ){
			case 'c':
				error = Clean_Ubootlist();
				if( error == REV_SUCCESS ) result = 1;
				break;
			case 'w':
				Delete_Ubootlist(optarg);
				error = Write_Ubootlist(optarg);
				if( error == REV_SUCCESS ) result = 2;
				break;
			case 'r':
				error = Read_Ubootlist(optarg);
				if( error == REV_SUCCESS ) result = 3;
				break;
			case 'd':
				error = Delete_Ubootlist( optarg );
				if( error == REV_SUCCESS ) result = 4;
				break;
			case 'm':
				error = Memory_Calibration();
				break;
			case 'h':
				error = 0;
				break;
			default:
				error = -1;
				break;
		}

		if( result == 1 ) printf("Clean All Uboot parameter Pass\n");
		else if( result == 2 ) printf("Write Uboot parameter Pass\n");
		//else if( result == 3 ) printf("Read Uboot parameter Pass\n");
		else if( result == 4 ) printf("Delet Uboot parameter Pass\n");
		
error_message:
		if( error <= 0 ){
			show_usage();
			break;
		}else if( error == 1 ){
			break;
		}
		
	}

	return 0;
}


#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>

/*************************************************
stdio.h 的函數 scanf() 從標準輸入裝置接收格式化字串。有如下的可指定的轉換格式

%d	有正負號的十進位整數	int *
%i	有正負號的十進位整數	int *
%u	無正負號的十進位整數	int *
%o	無正負號的八進位整數	int *
%x, %X	無正負號的十六進位整數	int *
%c	字元	char
%s	字串	char *
%f	浮點數	double
%p	記憶體位址的編碼	void *
%%	百分比符號	%

scanf() 的第一個參數為接受使用者輸入的格式化字串，隨後可接多個參數，依格式化字串中的轉換格式的數量而定。例如要求使用者輸入兩個整數，如下
scanf("%d%d", &n1, &n2);
格式化字串中只需標明轉換格式， scanf() 會自動依輸入的順序將值存入其後的參數中。這裡要注意，其後的參數必須是指標。
以下程式示範使用 scanf() 的結果
**************************************************/

static int check_if_hex(char *a)
{
	int i = 0;

    for(i = 0; a[i] != '\0'; i++)
    {
        if(! isxdigit(a[i]))
        {
			return 1;
		}
        //printf("%c is a hexadecimal digits\n", a[i]);
	}
    return 0;
}

static int check_if_number(char *a)
{
	int i = 0;

    for(i = 0; a[i] != '\0'; i++)
    {
        if(! isdigit(a[i]))
        {
			return 1;
		}
        //printf("%c is a number digits\n", a[i]);
	}
    return 0;
}


int main (int argc, char** agrv)
{
	char cMac[18]={0};
	char cMacinput01[18]={0};
	char cMacinput02[18]={0};
	char cID[18]={0};
	char cIDinput01[18]={0};
	char cIDinput02[18]={0};
	int i , ret = 0;
	int iMacOk = 0;
	int iIDOk = 0;
	int retry = 0;
	char send[512];
	int iMaclen = 12;
	int iIDlen = 10;

inputagain:
	cMac[0] = '\0';
	cID[0] = '\0';
	iMacOk = '\0';
	iIDOk = '\0';
	printf("\nPlease input Mac：");
    scanf("%s", cMacinput01);
	printf("Please input Mac again：");
    scanf("%s", cMacinput02);
    
	printf("Please input Product ID：");
    scanf("%s", cIDinput01);
	printf("Please input Product ID again：");
    scanf("%s", cIDinput02);

	if ( !check_if_hex(cMacinput01) && strlen(cMacinput01) == iMaclen && strncmp(cMacinput01,cMacinput02, iMaclen) == 0 )
	{
		for( i = 1; i < iMaclen+6; i++ )
		{
			if (i%3){
				cMac[i-1] = cMacinput01[(i-1) - i/3];
			}else{
				cMac[i-1] = ':';
			}
		}
		cMac[i-1] = '\0';
		iMacOk = 1;
	}

	if ( strlen(cIDinput01) == iIDlen && strncmp(cIDinput01,cIDinput02, iIDlen) == 0 )
	{
		for( i = 0; i < iIDlen; i++ )
		{
			cID[i] = cIDinput01[i];
		}
		cID[i] = '\0';
		iIDOk = 1;
	}

	if ( iMacOk == 1 && iIDOk == 1 )
	{
		printf("input Mac is %s：",cMac);
		printf("input ProductID is %s：",cID);
		sprintf(send, "/bin/rtx_uboot --write  ubMAC01=%s", cMac);
		system(send);
		sprintf(send, "/bin/rtx_uboot --write  ubProductSerialNO=%s", cID);
		system(send);
		sync();
		sleep(1);
		sync();
		ret = 0 ;
	}else{
		if ( iMacOk == 0 )
		{
			printf("input Mac is Wrong!");
		}
		if ( iIDOk == 0 )
		{
			printf("input Product ID is Wrong!");
		}
		ret = 1 ;
		retry++ ;
		if ( retry < 3 ) goto inputagain;
	}
	printf("\n");
	return ret;
}


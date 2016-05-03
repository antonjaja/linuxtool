#include "pidof.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <libgen.h>

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/timeb.h>

/* checks if the string is purely an integer
 * we can do it with `strtol' also
 */
int check_if_number (char *str)
{
  int i;
  for (i=0; str[i] != '\0'; i++)
  {
    if (!isdigit (str[i]))
    {
      return 0;
    }
  }
  return 1;
}
 
#define MAX_BUF 1024
#define PID_LIST_BLOCK 32
 
/*monitor /proc/---/cmdline equals pname then
 * kill pname process if it VmRSS size over size_limit
 * 	
 */  
int *pidof (char *pname, int size_limit)
{
  DIR *dirp;
  FILE *fp;
  struct dirent *entry;
  int *pidlist = NULL ;
  int pidlist_index = 0, pidlist_realloc_count = 1;
  char path[MAX_BUF], read_buf[MAX_BUF];
  char size_buf[256];
  char rss[128] ;
  int size_rss = 0;
  char size_check = 0 ;

  dirp = opendir ("/proc/");
  if (dirp == NULL)
  {
    perror ("Fail");
    return NULL;
  }
 
  pidlist = malloc (sizeof (int) * PID_LIST_BLOCK);
  if (pidlist == NULL)
  {
    return NULL;
  }
 
  while ((entry = readdir (dirp)) != NULL)
  {
    if (check_if_number (entry->d_name))
    {
      strcpy (path, "/proc/");
      strcat (path, entry->d_name);
      strcat (path, "/cmdline");
 
      /* A file may not exist, it may have been removed.
       * dut to termination of the process. Actually we need to
       * make sure the error is actually file does not exist to
       * be accurate.
       */
      fp = fopen (path, "r");
      if (fp != NULL)
      {
        fscanf (fp, "%s", read_buf);
        if (strcmp (read_buf, pname) == 0)
        {
		  fclose (fp);
          strcpy (path, "/proc/");
		  strcat (path, entry->d_name);
		  strcat (path, "/status");
          fp = fopen (path, "r");
          if (fp != NULL)
          {
		    while( fgets(size_buf, sizeof(size_buf), fp) != NULL ){
			  if( sscanf(size_buf, "%s %d", rss, &size_rss ) == 2 ){
				if( strncmp( rss, "VmRSS", 5) == 0 ){
					if ( size_rss < size_limit )
					{
						fclose (fp);
                        printf("Tom size_rss: %d ; size_limit: %d \n", size_rss, size_limit );
						goto finish ;
					}else{
						size_check = 1 ;
					}
				}
			  }
		    }
		  }else{
			printf("Can't open %s\n", path);	  
            goto finish ;
		  }
		  
		  if ( size_check == 0 )
		  {
			printf("Can't read size_rss!\n");	  
            fclose (fp);
            goto finish ;
		  }
		  
          /* add to list and expand list if needed */
          pidlist[pidlist_index++] = atoi (entry->d_name);
          if (pidlist_index == PID_LIST_BLOCK * pidlist_realloc_count)
          {
            pidlist_realloc_count++;
            pidlist = realloc (pidlist, sizeof (int) * PID_LIST_BLOCK * pidlist_realloc_count); //Error check todo
            if (pidlist == NULL)
            {
			  fclose (fp);
			  pidlist_index = 0 ;
              goto finish ;
            }
          }
        }
        fclose (fp);
      }else{
		printf("Can't open %s\n", path);
		goto finish ;
	  }
    }
  }
 
finish:
  closedir (dirp);
  pidlist[pidlist_index] = -1; /* indicates end of list */
  return pidlist;
}

/*monitor /proc/meninfo. 
 * release cached if MemFree: < size_limit
 */ 
void meminfo ( int size_limit )
{
  FILE *fp;
  char size_buf[256] ;
  char MemFree[128] ;
  int size_free = 0 ;

  fp = fopen ("/proc/meminfo", "r");
  if (fp != NULL)
  {
	while( fgets(size_buf, sizeof(size_buf), fp) != NULL ) {
		if( sscanf(size_buf, "%s %d", MemFree, &size_free ) == 2 ) {
			if( strncmp( MemFree, "MemFree", 7) == 0 ) {
				if ( size_free < size_limit ) {
					fclose (fp);
					fp = fopen ("/proc/sys/vm/drop_caches", "wb");
					if (fp != NULL){
						fwrite("1", 1, 1, fp);
						printf("Tom MemFree: %d ; size_limit: %d \n", size_free, size_limit );
					}else{
						printf("Can't open /proc/sys/vm/drop_caches\n");
					}
				}
				break ;
			}
		}
	}
  }else{
	printf("Can't open /proc/meminfo\n");
  }
  
  if( fp != NULL ){
	fclose (fp);
  }
  
  return ;
}

/* int *pidalive (char *pname)
 * whether the process is alive
 */ 
int *pidalive (char *pname)
{
  DIR *dirp;
  FILE *fp;
  struct dirent *entry;
  int *pidlist = NULL ;
  int pidlist_index = 0, pidlist_realloc_count = 1;
  char path[MAX_BUF], read_buf[MAX_BUF];
  char size_buf[256];
  char rss[128] ;
  int size_rss = 0;
  char size_check = 0 ;

  dirp = opendir ("/proc/");
  if (dirp == NULL)
  {
    perror ("Fail");
    return NULL;
  }
 
  pidlist = malloc (sizeof (int) * PID_LIST_BLOCK);
  if (pidlist == NULL)
  {
    return NULL;
  }
 
  while ((entry = readdir (dirp)) != NULL)
  {
    if (check_if_number (entry->d_name))
    {
      strcpy (path, "/proc/");
      strcat (path, entry->d_name);
      strcat (path, "/cmdline");
 
      /* A file may not exist, it may have been removed.
       * dut to termination of the process. Actually we need to
       * make sure the error is actually file does not exist to
       * be accurate.
       */
      fp = fopen (path, "r");
      if (fp != NULL)
      {
        fscanf (fp, "%s", read_buf);
        if (strcmp (read_buf, pname) == 0)
        {	  
          /* add to list and expand list if needed */
          pidlist[pidlist_index++] = atoi (entry->d_name);
          if (pidlist_index == PID_LIST_BLOCK * pidlist_realloc_count)
          {
            pidlist_realloc_count++;
            pidlist = realloc (pidlist, sizeof (int) * PID_LIST_BLOCK * pidlist_realloc_count); //Error check todo
            if (pidlist == NULL)
            {
			  fclose (fp);
			  pidlist_index = 0 ;
              goto finish ;
            }
          }
        }
        fclose (fp);
      }else{
		printf("Can't open %s\n", path);
		goto finish ;
	  }
    }
  }
 
finish:
  closedir (dirp);
  pidlist[pidlist_index] = -1; /* indicates end of list */
  return pidlist;
}

int main (int argc, char *argv[])
{
  int *list, i;
  //struct timeb tom, tom1 ;
  int kill_size , free_size;
  char retry_count = 0 ;
  
  if (argc < 2)
  {
    printf ("args number fail argc=%d\n", argc);
    return 0;
  }
  
  if( strncmp(argv[1], "pidsize", 7 ) == 0){
	  //ftime(&tom);
	  if (argc < 4)
	  {
		printf ("Usage: %s proc_name\n", argv[1]);
		return 0;
	  }
	  
	  kill_size = strtol(argv[3], NULL, 10) ;
	  list = pidof (argv[2], kill_size);
	  
	  for (i=0; list[i] != -1; i++)
	  {
		kill(list[i], SIGKILL);
		//usleep(30000); //delay 30 ms for system to create new process
		printf ("old:%d\n", list[i]);
		free (list);
	  }
	  /*
	  while (1){
		retry_count++;
		if(retry_count > 500)
		{
			printf("pidof: NG\n");
			break ; //create new process must success in 5s.
		}
		list = pidof (argv[2], 0);
		if( list[0] == -1 ){
			usleep(10000); //delay 10 ms	 
			continue;
		}
		break;
	  }
	  //ftime(&tom1);
	
	  for (i=0; list[i] != -1; i++)
	  {
		printf ("current pid:%d\n", list[i]);
	  }
	  free (list);
	  */
	  //printf("time:%d,%d\n", tom.millitm, tom1.millitm);
	  
	  //clear cache if free_size below argv[4] value
	  if ( argv[4] != NULL ){
		free_size = strtol(argv[4], NULL, 10) ;
		meminfo( free_size ) ;
	  }
  }else if( strncmp(argv[1], "pidalive", 8 ) == 0){
	  if (argc < 3)
	  {
		printf ("%s command fail\n", argv[1]);
		return 0;
	  }
	  
	  list = pidalive (argv[2]);
	  
	  if( list[0] == -1 ){
		printf ("%s died\n", argv[2]);
	  }else{
		printf ("%s alive\n", argv[2]);
	  }
	
	  free (list);
  }
  
  
  return 0;
}

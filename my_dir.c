#include "20161615.h"


int instruction_dir(){ // 현재 디렉토리의 파일들을 출력한다.
	DIR* dp = NULL;
	struct dirent* entry = NULL;
	struct stat buf;

	if((dp = opendir(".")) == NULL){
		printf("Error!\n");
		return -1;
	}

	int print_line = 0;
	while ((entry = readdir(dp)) != NULL){
		lstat(entry->d_name, &buf);

		if (S_ISDIR(buf.st_mode))
			printf("\t%s/", entry->d_name);
		else if ( S_ISREG(buf.st_mode)){
			if((S_IEXEC & buf.st_mode) != 0)
				printf("\t%s*",entry->d_name);
			else
				printf("\t%s", entry->d_name);
		}

		if(print_line++ >= 3){
			printf("\n");
			print_line = 0;
		}
	}
	printf("\n");
	closedir(dp);

	return 0;
}

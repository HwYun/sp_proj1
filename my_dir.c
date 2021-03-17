#include "20161615.h"

int instruction_dir(){
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
			printf("%s/\t", entry->d_name);
		else if ( S_ISREG(buf.st_mode)){
			if((S_IEXEC & buf.st_mode) != 0)
				printf("%s*\t",entry->d_name);
			else
				printf("%s\t", entry->d_name);
		}

		if(print_line++ >= 3){
			printf("\n");
			print_line = 0;
		}
	}
	closedir(dp);

	return 0;
}

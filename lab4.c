#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>

struct FileMapping {
	int file;
	size_t fsize;
	unsigned char* dataPtr;
};

void parentProcess(int* pipe_fd, int m, char *fname, char *fproc, int* dataPtr) {
	int d;
	int file = open(fproc, O_WRONLY, 0);
	if(file < 0) {
		printf("FileMappingOpen - open failed, fname = %s \n", fproc);
		exit(-1);
	}
	for (int i = 0; i < m; ++i) {
		 dataPtr[i] = i + 1;
	}
	close(file);
}
void childProcess(int* pipe_fd, char *fname, char *fproc, int* dataPtr, int fsize) {
	int d;
	int fd;
	fd = open(fname, O_CREAT | O_APPEND | O_WRONLY, S_IWUSR | S_IRUSR);
	int file = open(fproc, O_RDONLY, 0);
	if(file < 0) {
		printf("FileMappingOpen - open failed, fname = %s \n", fproc);
		exit(-1);
	}
	dup2(fd, 1);
	int k = 0;
	for(k; k < fsize; ++k){
		if (dataPtr[k] != 0)
			printf("%i ", dataPtr[k]);
	}
	printf("\n");
	close(fd);
	close(file);
}

int main(int argc, char const *argv[]) {
	int pipe_fd[2];
	pid_t pid;
	char name_file[20];
	char proc_file[20];
	int count_process;
	int m;
	int err = 0;
	printf("Enter name of out file\n");
	scanf ("%s", name_file);
	printf("Enter name of memory file\n");
	scanf("%s", proc_file);
	printf("Enter n\n");
	scanf ("%d", &count_process);
	printf("Enter m\n");
	scanf ("%d", &m);
	int i = 0;
	int file = open(proc_file, O_CREAT | O_APPEND | O_RDWR, S_IWUSR | S_IRUSR);
	size_t fsize = 100;
	ftruncate(file, fsize);
	int* dataPtr = (int*)mmap(NULL, fsize, PROT_READ | PROT_WRITE, MAP_SHARED, file, 0); //Создаем отображение файла в память
// PROT_READ страницы могут быть прочитаны
// PROT_WRITE стр могут быть описаны
//  MAP_SHARED стр могут сипользоваться совместно с др процессами, которые также проектируют этот объект в память
	if (dataPtr == MAP_FAILED) 
	{
		perror("Map");
		printf("FileMappingCreate - open failed 2, fname = %s \n", proc_file);
		close(file);
		exit(-1);
	}
	for (i; i < count_process; ++i) {
		if (pipe(pipe_fd) == -1) {
			perror("PIPE");
			err = -2;
		}
		pid = fork();
		if (pid == -1) {
			perror("FORK");
			err = -1;
		}
		else if (pid == 0) {
			childProcess(pipe_fd, name_file, proc_file, dataPtr, m);
			break;
		} else
			parentProcess(pipe_fd, m, name_file, proc_file, dataPtr);

	}
	return err;
}

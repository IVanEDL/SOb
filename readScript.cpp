#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <iostream>
#include <vector>
#include <string>
#include <semaphore.h>
#include <fcntl.h>
//#include una cerveza por favor :(

int procProd(const char* input_file){
	int script = 2048;
	char line[256];
	int cont = 0;
	char contcpy[120];
	char scrname[120];
	char route[80] = "./scripts";
	char read[120];
	FILE *file = fopen(input_file, "r");
	if (file == NULL) {
		printf("Error opening file!\n");
		return 1;
	}
	while (!feof(file)) {
		strcpy(route, "./scripts.txt"); 
		fgets(line, sizeof(line), file);
		if (line[0] == '/') {
			if (line[1] == '/') {
				fgets(line, sizeof(line), file);
			}
		}
		else {
			sprintf(route, "./scriptN%d.txt", cont);
			FILE * f2 = fopen(route, "a+");
			if (f2 == NULL) {
                printf("Error creating subfile!\n");
                fclose(file);
                exit(1);
            }
			while (line[0] != '/' && line[1] != '/' && !feof(file)) {
				fputs(line, f2);
				fgets(line, sizeof(line), file);
			}
			fclose(f2);
			cont++;
		}
	}
	fclose(file);
	printf("%d scripts generados.", cont);
	return cont;
}

void procCon(int id, int i, int total_scripts) {
    char route[80];
    char command[256];
    char log_file[80];

    sprintf(log_file, "logCon.txt");

    sprintf(route, "./scriptN%d.txt", i);
    FILE* file = fopen(route, "r");
    if (file == NULL) {
        printf("Proceso Consumidor Número %d: No se pudo abrir %s\n", id, route);
    }

    FILE* log = fopen(log_file, "a");
    if (log == NULL) {
        printf("Proceso Consumidor Número %d: Error al abrir el archivo de log correspondiente, no sas malo.\n", id);
        fclose(file);
    }

    fprintf(log, "Proceso Consumidor Número %d ejecutando %s\n", id, route);
    fclose(log);
        
    while (fgets(command, sizeof(command), file)) {
        command[strcspn(command, "\n")] = 0;
        printf("Proceso Consumidor Número %d ejecutando: %s", id, command);
        system(command); //Ver README
    }

    fclose(file);
}

int main() {
    const char* input_file = "./scripts.txt";
    sem_t *sem;
    int shm_fd;
    sem = sem_open("sem.txt", O_CREAT, 644, 1);
    
    shm_fd = shm_open("shm.txt", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, 4096);
    int *NumeroScripts = (int*)mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (NumeroScripts == MAP_FAILED) {
        perror("Error creando memoria compartida");
        return 1;
    }

    pid_t pidProd = fork();
    if (pidProd == 0) {
        *NumeroScripts = procProd(input_file); 
        return 0;
    }
    wait(nullptr);

    pid_t pidCon1 = fork();
    if (pidCon1 == 0) {
        sem_wait(sem);
        for (int i = 0; i < *NumeroScripts; i+=2){
            procCon(0, i, *NumeroScripts);
            sem_post(sem);
        }
        return 0;
    }
    pid_t pidCon2 = fork();
    if (pidCon2 == 0) {
        sem_wait(sem);
        for (int i = 1; i < *NumeroScripts; i+=2){
            procCon(1, i, *NumeroScripts);
            sem_post(sem);
        }
        return 0;
    }
    wait(NULL);
    wait(NULL);


    shm_unlink("shm.txt");
    sem_close(sem);
    sem_unlink("sem.txt");

    printf("VAMO ARRIBA COMPILÓ\n");
    return 0;
}
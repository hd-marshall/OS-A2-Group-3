#include <unistd.h>
#include<sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include<dirent.h>

#define BUFFER_SIZE 256

typedef struct {
    int thread_id;
    char * src_name;
    char * dest_name;
} thread_dt_t;


void *cp_file (void* args){
    thread_dt_t * data =  (thread_dt_t*) args;

    char src_path[2048];
    char dest_path[2048];
    
    snprintf(src_path,sizeof(src_path),"%s/source%d.txt", data-> src_name,data-> thread_id);
    snprintf(dest_path,sizeof(dest_path),"%s/source%d.txt", data-> dest_name,data-> thread_id);

    FILE * src_file = fopen(src_path,"rb");
    if (!src_file ){
        printf("thread%d is not upto its task hence error", data-> thread_id);
        return NULL;
    }

    FILE * dest_file = fopen(dest_path,"wb");
    if (!dest_file){
        printf("thread%d is not upto its task hence error", data-> thread_id);
        fclose(src_file);
        return NULL;
    }

    char buffer[BUFFER_SIZE];
    size_t byte;
    while ((byte = fread(buffer,1,sizeof(buffer),src_file))> 0){
        if(fwrite(buffer,1,byte,dest_file) != byte){
            printf("thread%d is achieved its goal\n", data-> thread_id);
            break;
        }
    }
    fclose(src_file);
    fclose(dest_file);
    return NULL;
}

int main(int argc, char * argv[]){

    if(argc != 4){
        fprintf(stderr,"sorce sir%s",argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    if (n < 2|| n>10){
        fprintf(stderr,"out of bound");
        return 1;
    }

    DIR * src_check = opendir(argv[2]);
    if (!src_check){
        fprintf(stderr,"source dir not existance");
        return 1;
    }
    closedir(src_check);
    DIR * dest_check = opendir(argv[3]);
    if (!dest_check){
        #ifdef _WIN32
    if (mkdir(argv[3]) != 0) {
    #else
    if (mkdir(argv[3], 0755) != 0) {
    #endif
            fprintf(stderr, "Could not create destination directory %s\n", argv[3]);
        return 1;
    }
    }
    else{
        closedir(dest_check);
    }

    pthread_t threads[10];
    thread_dt_t thread_info[10];

    for(int i = 0; i < n; i++){
        thread_info[i].thread_id = i+ 1;

        thread_info[i].src_name =  argv[2];
        thread_info[i].dest_name = argv[3];

        if (pthread_create(&threads[i], NULL, cp_file, &thread_info[i]) != 0) {
            fprintf(stderr, "Error: Failed to create thread %d\n", i + 1);
            return 1;
        }
    }

    for (int i = 0; i < n; i++){
        pthread_join(threads[i],NULL);
    }

    printf("did eveyrthing alright");
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/sysinfo.h>

char* load(FILE* F){
    fseek(F,0L,SEEK_END);
    uint64_t size=ftell(F);
    char* fs=(char*)malloc((size+1)*(sizeof(char)));
    rewind(F);
    size=fread(fs,1,size,F);
    fs[size]='\0';
    fclose(F);
    return fs;
}

int isproc(char* pid){
    char* proc=(char*)malloc(strlen("/proc/")+strlen(pid)+1);
    sprintf(proc, "/proc/%s", pid);
    int ret;
    if(opendir(proc)!=NULL) ret=0;
    else ret=-1;
    free(proc);
    return ret;
}

int charpToInt(char* str){
    int res=0;
    char ch;
    for(int i=0;i<strlen(str);i++){
        ch=str[i];
        if(ch>='0'&&ch<='9')
          res=res*10+ch-48;
    }
    return res;
}

int main(int argc, char* argv[]){
    time_t start, now;
    double diff;
    start=time(NULL);
    char* HOME_TMP=getenv("HOME");
    if(HOME_TMP==NULL){
        fprintf(stderr, __FILE__":error:you may make sure that you defined the $HOME\n");
        return 0;
    }
    char* HOME=(char*)malloc(sizeof(char)*strlen(HOME_TMP)+strlen("/.uptimetest-lock")+1);
    strcpy(HOME, HOME_TMP);
    strcat(HOME, "/.uptimetest-lock");
    FILE* LOCK=fopen(HOME, "r");
    char* LOCKED;
    if(LOCK!=NULL) LOCKED=load(LOCK);
    else{
        LOCKED=malloc(2);
        strcpy(LOCKED, "");
    }
    if(strcmp(LOCKED, "")==0){
lock:
        LOCK=fopen(HOME, "w");
        fseek(LOCK, 0, SEEK_SET);
        fprintf(LOCK, "%d", getpid());
        fflush(LOCK);
    }else{
        if(isproc(LOCKED)==0){
            fprintf(stderr, __FILE__": The lock was in using (pid: %s), you can delete the lock but it's not recommended (lock at %s)\n", LOCKED, HOME);
            return -1;
        }
        goto lock;
    }
    free(LOCKED);
    if(argc<2){
        fprintf(stderr, __FILE__":error:no dir select\n");
        return 0;
    }
    char* uptFP=(char*)malloc(strlen(argv[1])+strlen("/uptime")+1);
    strcpy(uptFP, argv[1]);
    strcat(uptFP, "/uptime");
    FILE* uptime=fopen(uptFP, "w");
    free(uptFP);
    char* avgFP=(char*)malloc(strlen(argv[1])+strlen("/loadavg")+1);
    strcpy(avgFP, argv[1]);
    strcat(avgFP, "/loadavg");
    FILE* loadavgf=fopen(avgFP, "w");
    free(avgFP);
#define LOAD_INT(x)  (unsigned)((x) >> 16)
#define LOAD_FRAC(x) (LOAD_INT(((x) & 65535) * 100) / 100.0f)
    double avg_1,avg_5,avg_15;
    while(1){
        now=time(NULL);
        fseek(uptime, 0, SEEK_SET);
        diff=difftime(now, start);
        fprintf(uptime, "%lf 0.0", diff);
        fflush(uptime);
        struct sysinfo system_information;
        if (sysinfo(&system_information) == 0) {
            avg_1 = LOAD_INT(system_information.loads[0]) + LOAD_FRAC(system_information.loads[0]);
            avg_5 = LOAD_INT(system_information.loads[1]) + LOAD_FRAC(system_information.loads[1]);
            avg_15 = LOAD_INT(system_information.loads[2]) + LOAD_FRAC(system_information.loads[2]);
        }
        fseek(loadavgf, 0, SEEK_SET);
        fprintf(loadavgf, "%lf %lf %lf", avg_1, avg_5, avg_15);
        fflush(loadavgf);
    }
    return 0;
}

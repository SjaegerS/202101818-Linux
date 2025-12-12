#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define timequantum    3
#define cpuburst  1
#define childprocess    10
#define maxio     5
#define READY   0
#define RUNNING 1
#define SLEEP   2
#define DONE    3

typedef struct {
    pid_t pid;
    int state;
    int iorest;
    int timequantumrest;
    int totalwait;
    int setcpuburst;
    int cpuburstrest;
} PCB;

PCB pcb[childprocess];
int nowprocess = -1;
int lastprocess = -1;
int doneprocess = 0;
int timecount = 0;
int mycpuburst = 0;
int mysetcpuburst = 0;
int returncount = 0;

void handler(int s) {
    if (returncount == 1) {
        mycpuburst = mysetcpuburst;
        returncount = 0;
    }
    mycpuburst -= cpuburst;
    if (mycpuburst <= 0) {
        int action = rand() % 2;
        if (action == 0) {
            exit(0);
        } else {
            returncount = 1;
            kill(getppid(), SIGUSR2);
        }
    }
}

void processloop() {
    srand(getpid()); 
    mysetcpuburst = (rand() % 10) + 1;
    mycpuburst = mysetcpuburst;
    signal(SIGUSR1, handler);
    while (1) {
        pause();
    }
}

int setnextprocess() {
    int start_idx = (lastprocess + 1) % childprocess;
    int i;
    for (i = 0; i < childprocess; i++) {
        int idx = (start_idx + i) % childprocess;
        if (pcb[idx].state == READY) {
            return idx;
        }
    }
    return -1; 
}

void IOrequest(int s) {
    if (nowprocess != -1) {
        pcb[nowprocess].state = SLEEP;
        pcb[nowprocess].iorest = (rand() % maxio) + 1;
        pcb[nowprocess].cpuburstrest = pcb[nowprocess].setcpuburst;
        printf("[현 시간 이 후] 프로세스 %d I/O 요청 (%d 초 대기)\n", pcb[nowprocess].pid, pcb[nowprocess].iorest);
        lastprocess = nowprocess;
        nowprocess = -1;
    }
}

void processdone(int s) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        int i;
        for (i = 0; i < childprocess; i++) {
            if (pcb[i].pid == pid) {
                pcb[i].state = DONE;
                doneprocess++;
                printf("[현 시간 이 후] 프로세스 %d 종료 (완료: %d/%d)\n", pid, doneprocess, childprocess);
                if (i == nowprocess) {
                    lastprocess = nowprocess;
                    nowprocess = -1;
                }
                break;
            }
        }
    }
}

void timecheck(int s) {
    int i;
    timecount++;
    printf("\n[타임 %d] ", timecount);
    for (i = 0; i < childprocess; i++) {
        if (pcb[i].state == SLEEP) {
            pcb[i].iorest--;
            if (pcb[i].iorest <= 0) {
                pcb[i].state = READY;
                printf("IO 완료 : 프로세스 %d\n", pcb[i].pid);
            }
        }
        else if (pcb[i].state == READY) {
            pcb[i].totalwait++; 
        }
    }
    if (nowprocess != -1) {
        printf("실행중인 프로세스 %d (남은 cpu 버스트: %d, 남은 timequantum : %d)", pcb[nowprocess].pid, pcb[nowprocess].cpuburstrest, pcb[nowprocess].timequantumrest);
        pcb[nowprocess].timequantumrest--;
        kill(pcb[nowprocess].pid, SIGUSR1);
        if (pcb[nowprocess].cpuburstrest > 0)
             pcb[nowprocess].cpuburstrest--;
        if (nowprocess != -1) {
            if (pcb[nowprocess].timequantumrest <= 0 && pcb[nowprocess].cpuburstrest > 0) {
                printf(" \n => 다음 타임에 timequantum 만료 후 수행 프로세스 변경");
                pcb[nowprocess].state = READY;
                lastprocess = nowprocess;
                nowprocess = -1; 
            }
        }
        printf("\n");
    } 
    else {
        int next = setnextprocess();
        if (next != -1) {
            nowprocess = next;
            pcb[next].state = RUNNING;
            pcb[next].totalwait--;
            pcb[next].timequantumrest = timequantum;
            printf("프로세스 문맥 교환 진행, 프로세스 %d 로 변경 후 수행 시작 (남은 cpu 버스트 : %d, 남은 timequantum : %d)\n", pcb[next].pid, pcb[next].cpuburstrest, timequantum);
            pcb[next].timequantumrest--;
            if (pcb[next].cpuburstrest > 0){
                pcb[next].cpuburstrest--;
            }
            kill(pcb[next].pid, SIGUSR1);
        } else {
            printf("대기 프로세스 없음\n"); 
        }
    }
    if (doneprocess >= childprocess) {
        double total_wait = 0;
        printf("\n 시뮬레이션 완료\n\n");
        for (i = 0; i < childprocess; i++) {
            total_wait += pcb[i].totalwait;
            printf("프로세스 %d 대기시간 : %d\n", pcb[i].pid, pcb[i].totalwait);
        }
        printf("\n평균 대기시간 : %.2f 초\n", total_wait / childprocess);
        exit(0);
    }
    alarm(1);
}
int main() {
    int i;
    pid_t pid;
    setvbuf(stdout, NULL, _IONBF, 0);
    signal(SIGALRM, timecheck);
    signal(SIGCHLD, processdone);
    signal(SIGUSR2, IOrequest);
    printf("시뮬레이션 시작 (설정 timequantum: %d)\n", timequantum);
    for (i = 0; i < childprocess; i++) {
        pid = fork();
        if (pid == 0) {
            processloop();
            exit(0);
        } else {
            pcb[i].pid = pid;
            pcb[i].state = READY; 
            pcb[i].iorest = 0;
            pcb[i].timequantumrest = 0;
            pcb[i].totalwait = 0;
            srand(pid); 
            pcb[i].setcpuburst = (rand() % 10) + 1;
            pcb[i].cpuburstrest = pcb[i].setcpuburst;
            printf("   자식프로세스 %d 생성 (설정 cpu버스트 : %d)\n", pid, pcb[i].setcpuburst);
        }
    }
    sleep(1); 
    alarm(1); 
    while (1) {
        pause();
    }
    return 0;
}


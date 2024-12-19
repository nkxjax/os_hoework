#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME_LENGTH 20

// 定义进程控制块（PCB）
typedef struct {
    int pid;                 // 进程ID
    char name[MAX_NAME_LENGTH]; // 进程名
    char state[20];          // 进程状态
    int burstTime;           // 服务时间
    int remainingTime;       // 剩余时间
} PCB;

// 队列节点
typedef struct QueueNode {
    PCB process;
    struct QueueNode* next;
} QueueNode;

// 队列结构
typedef struct {
    QueueNode* front;
    QueueNode* rear;
} Queue;

// 初始化队列
void initQueue(Queue* q) {
    q->front = q->rear = NULL;
}

// 判断队列是否为空
int isEmpty(Queue* q) {
    return q->front == NULL;
}

// 入队
void enqueue(Queue* q, PCB process) {
    QueueNode* newNode = (QueueNode*)malloc(sizeof(QueueNode));
    newNode->process = process;
    newNode->next = NULL;
    if (isEmpty(q)) {
        q->front = q->rear = newNode;
    } else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
}

// 出队
PCB dequeue(Queue* q) {
    if (isEmpty(q)) {
        fprintf(stderr, "Queue is empty!\n");
        exit(0);
    } else {
        QueueNode* temp = q->front;
        PCB process = temp->process;
        q->front = q->front->next;
        free(temp);
        return process;
    }
}

// 打印队列
void printQueue(const char* title, Queue* q) {
    printf("-------------------- %s --------------------\n", title);
    printf("进程名\t进程ID\t服务时间\t剩余时间\t状态\n");
    QueueNode* temp = q->front;
    while (temp) {
        PCB* p = &temp->process;
        printf("%s\t%d\t%d\t\t%d\t\t%s\n", p->name, p->pid, p->burstTime, p->remainingTime, p->state);
        temp = temp->next;
    }
    printf("-------------------------------------------------------\n");
}

// 全局队列
Queue readyQueue;      // 活动就绪队列
Queue blockedQueue;    // 活动阻塞队列
Queue suspendedQueue;  // 静止阻塞队列
Queue runningQueue;    // 运行队列
Queue completedQueue;  // 完成队列

int processCounter = 0;  // 进程ID计数器

// 初始化操作系统的原始进程
void initProcesses() {
    for (int i = 0; i < 3; i++) {
        PCB process;
        process.pid = ++processCounter;
        snprintf(process.name, sizeof(process.name), "P%d", process.pid);
        strcpy(process.state, "READY");
        process.burstTime = rand() % 10 + 1; // 服务时间随机生成
        process.remainingTime = process.burstTime;
        enqueue(&readyQueue, process);
    }
}

// 新建进程
void createProcess() {
    PCB process;
    process.pid = ++processCounter;
    printf("请输入新进程的名称：");
    scanf("%s", process.name);
    strcpy(process.state, "READY");
    printf("请输入新进程的服务时间：");
    scanf("%d", &process.burstTime);
    process.remainingTime = process.burstTime;
    enqueue(&readyQueue, process);
    printf("新建进程成功！\n");
}

// 进程调度（先来先服务算法）
void schedule() {
    while (!isEmpty(&readyQueue) || !isEmpty(&blockedQueue) || !isEmpty(&suspendedQueue)) {
        printQueue("就绪队列", &readyQueue);
        printQueue("阻塞队列", &blockedQueue);
        printQueue("静止队列", &suspendedQueue);
        printQueue("完成队列", &completedQueue);

        printf("\n请输入操作：\n");
        printf("1. 新建进程  2. 执行进程  3. 请求I/O事件  4. I/O完成\n");
        printf("5. 时间片到期  6. 阻塞事件挂起  7. 挂起完成  0. 退出\n");
        int choice;
        scanf("%d", &choice);

        switch (choice) {
            case 1:  // 新建进程
                createProcess();
                break;
            case 2:  // 执行进程
                if (!isEmpty(&readyQueue)) {
                    PCB process = dequeue(&readyQueue);
                    strcpy(process.state, "RUNNING");
                    printf("正在运行进程：%s\n", process.name);
                    process.remainingTime--;
                    if (process.remainingTime == 0) {
                        strcpy(process.state, "COMPLETED");
                        enqueue(&completedQueue, process);
                    } else {
                        strcpy(process.state, "READY");
                        enqueue(&readyQueue, process);
                    }
                } else {
                    printf("无进程可运行！\n");
                }
                break;
            case 3:  // 请求I/O事件
                if (!isEmpty(&readyQueue)) {
                    PCB process = dequeue(&readyQueue);
                    strcpy(process.state, "BLOCKED");
                    enqueue(&blockedQueue, process);
                }
                break;
            case 4:  // I/O完成
                if (!isEmpty(&blockedQueue)) {
                    PCB process = dequeue(&blockedQueue);
                    strcpy(process.state, "READY");
                    enqueue(&readyQueue, process);
                }
                break;
            case 5:  // 时间片到期
                if (!isEmpty(&readyQueue)) {
                    PCB process = dequeue(&readyQueue);
                    enqueue(&readyQueue, process);  // 将当前进程放回队列尾部
                }
                break;
            case 6:  // 阻塞事件挂起
                if (!isEmpty(&blockedQueue)) {
                    PCB process = dequeue(&blockedQueue);
                    strcpy(process.state, "SUSPENDED");
                    enqueue(&suspendedQueue, process);
                }
                break;
            case 7:  // 挂起完成
                if (!isEmpty(&suspendedQueue)) {
                    PCB process = dequeue(&suspendedQueue);
                    strcpy(process.state, "BLOCKED");
                    enqueue(&blockedQueue, process);
                }
                break;
            case 0:  // 退出
                printf("退出程序。\n");
                return;
            default:
                printf("无效的选项，请重新输入！\n");
        }
    }
}

int main() {
    // 初始化队列
    initQueue(&readyQueue);
    initQueue(&blockedQueue);
    initQueue(&suspendedQueue);
    initQueue(&runningQueue);
    initQueue(&completedQueue);

    // 初始化原始进程
    initProcesses();

    // 开始调度
    schedule();

    return 0;
}


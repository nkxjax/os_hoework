#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_PROCESSES 100

typedef struct {
	int pid;
	char name[20];
	char* state;
	int priority;
	int burstTime;
	int remainingTime;
} PCB;

typedef struct QueueNode {
	PCB process;
	struct QueueNode* next;
} QueueNode;

typedef struct {
	QueueNode* front;
	QueueNode* rear;
} Queue;

void initQueue (Queue* q) {
	q->front = q->rear = NULL;
	printf("创建成功\n");
}

int isEmpty (Queue* q) {
	return q->front == NULL;
}

void enqueue(Queue* q, PCB process) {
	QueueNode* newNode = (QueueNode*) malloc (sizeof(QueueNode));
	newNode->process = process;
	newNode->next = NULL;
	if(isEmpty(q)) {
		q->front = q->rear = newNode;
	} else {
		q->rear->next = newNode;
		q->rear = newNode;
	}
}

PCB dequeue(Queue* q) {
	if(isEmpty(q)) {
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

Queue readyQueue; //就绪队列
Queue blockedQueue; //阻塞队列
Queue suspendsQueue; //挂起队列

void initProcess() {
	for(int i = 0; i < 5; i++) {
		PCB process;
		process.pid = i + 1;
		snprintf(process.name, sizeof(process.name), "Process%d", process.pid);
		process.state = "READY";
		process.priority = rand() % 10;
		process.burstTime = rand() % 20 + 1;
		process.remainingTime = process.burstTime;
		enqueue(&readyQueue, process);
	}
}

void suspendProcess(PCB* process) {
	printf("%s is being suspended.\n", process->name);
	enqueue(&suspendsQueue, *process);
}

void resumeProcess() {
	while(!isEmpty(&suspendsQueue)) {
		PCB process = dequeue(&suspendsQueue);
		printf("%s is being resumed.\n", process.name);
		if(process.state == "Readys") process.state = "Readya";
		else process.state = "Blockeda";
		enqueue(&readyQueue, process);
	}
}

void completeIO () {
    if(!isEmpty(&blockedQueue)) {
        PCB process = dequeue(&blockedQueue);
		int num = random() % 10;
		if(num % 2) {
			printf("%s becomes blockeds", process.name);
			process.state = "Blockeds";
			enqueue(&suspendsQueue, process);
			return;
		}
        printf("%s I/O结束...", process.name);
        process.state = "READY";
        enqueue(&readyQueue, process);
	}
}

void schedule() {
	while(!isEmpty(&readyQueue) || !isEmpty(&blockedQueue) || !isEmpty(&suspendsQueue)) {
		PCB currentProcess = dequeue(&readyQueue);
		currentProcess.state = "RUNNING";
		printf("Running: %s\n", currentProcess.name);
		int number = random() % 10;
		for(int timeSlice = 0; timeSlice < 5 && currentProcess.remainingTime > 0; timeSlice ++) {
			currentProcess.remainingTime --;
			if(number % 2) {
				currentProcess.state = "Readys";
				suspendProcess(&currentProcess);
			}
			if(timeSlice == 2) {
				printf("%s requesting I/O...\n", currentProcess.name);
				currentProcess.state = "BLOCKED";
				enqueue(&blockedQueue, currentProcess);
				break;
			}
		}
		resumeProcess();
		if(currentProcess.remainingTime == 0) {
			currentProcess.state = "COMPLITE";
			printf("%s completed.\n", currentProcess.name);
		} else {
			currentProcess.state = "READY";
			enqueue(&readyQueue, currentProcess);
		}

		completeIO();
	}

}

int main() {
	initQueue(&readyQueue);
	initQueue(&blockedQueue);
	initQueue(&suspendsQueue);

	initProcess();
	//检查是否创建成功
	QueueNode* temp = readyQueue.front;
	while(temp) {
		printf("pid:%d\n", temp->process.pid);
		temp = temp->next;
	}

	PCB process = dequeue(&readyQueue);
	process.state = "Readys";
	enqueue(&suspendsQueue, process);

	schedule();
}



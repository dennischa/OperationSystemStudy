#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#define SEMPERM 0600
#define TRUE 1
#define FALSE 0
typedef union   _semun {
	int val;
	struct semid_ds *buf;
	ushort *array;
} semun;

int initsem(key_t semkey, int n)
{
	int status = 0, semid;
	if ((semid = semget(semkey, 1, SEMPERM | IPC_CREAT | IPC_EXCL)) == -1)
	{
		if (errno == EEXIST)
			semid = semget(semkey, 1, 0);
	}
	else
	{
		semun arg;
		arg.val = n;
		status = semctl(semid, 0, SETVAL, arg);
	}
	if (semid == -1 || status == -1)
	{
		perror("initsem failed");
		return (-1);
	}
	return (semid);
}

int p(int semid)
{
	struct sembuf p_buf;
	p_buf.sem_num = 0;
	p_buf.sem_op = -1;
	p_buf.sem_flg = SEM_UNDO;
	if (semop(semid, &p_buf, 1) == -1)
	{
		perror("p(semid) failed");
		exit(1);
	}
	return (0);
}

int v(int semid)
{
	struct sembuf v_buf;
	v_buf.sem_num = 0;
	v_buf.sem_op = 1;
	v_buf.sem_flg = SEM_UNDO;
	if (semop(semid, &v_buf, 1) == -1)
	{
		perror("v(semid) failed");
		exit(1);
	}
	return (0);
}

int L1, L2, L3;
int C1, C2, C3;

pid_t pid; //process id 
time_t t;	//

//lock.Acquire()
void Acquire(int lock) 
{
	p(lock);
}
//lock.Release()
void Release(int lock)
{
	v(lock);
}
//Wait() Atomically release lock and go to sleep.
//Re - acquire lock later, before returning.
void Wait(int okTo, int lock)
{
	Release(lock);
	p(okTo);
	Acquire(lock);
}
//Signal() Wake up one waiter, if any
void Signal(int okTo)
{
	v(okTo);
}
//Broadcast() Wake up all waiters 다시 짜야됨
void Broadcast(int okTo)
{
	/*int j =GetValue("WR.txt");
	int i = 0;
	for (i = 0; i < j; i++)
		Signal(okTo);*/
}

//change counters last value
void Change(char* counter_name, int ch)
{
	FILE *f = fopen(counter_name, "a+");
	time(&t);
	fprintf(f, "\n ID %d 시간 : %s",getpid(), ctime(&t));
	fprintf(f, "\n %d", ch);
	fclose(f);
}
int GetValue(char* counter_name)
{
	FILE *f = fopen(counter_name, "r");
	int res;
	char buf[100];
	while (fgets(buf, 100, f) != NULL)
	{
	}
	res = atoi(buf);
	fclose(f);
	return res;
}
//print current state on Result.txt
void PrintState(char* R, char* state)
{
	FILE *f = fopen("Result.txt", "a+");
	int res;
	res = GetValue(R);
	time(&t);
	fprintf(f, "\n 시간 : %s   %d%s", ctime(&t), getpid(), state);
	fprintf(f, "\n %s : %d", R, res);
	fclose(f);
}
//Print_Phil
void Print_Phil(char* state)
{
	FILE *f = fopen("Result.txt", "a+");
	time(&t);
	fprintf(f, "\n 시간 : %s   %d%s", ctime(&t), getpid(), state);
	fclose(f);
}
//return last value


Take_R1() {
	Acquire(L1);
	// R1이 1이면 젓가락이 있고 0이면 젓가락이 없어서 기다려야 함
	while (GetValue("R1.txt") == 0) {
		PrintState("R1.txt", "가 R1을 기다림");
		// print message: getpid()가 R1을 기다림
		Wait(C1, L1);
		PrintState("R1.txt", "가 R1을 기다리다가 깨어남");
		// print message: getpid()가 R1을 기다리다가 깨어남
	}
	Change("R1.txt", 0);
	PrintState("R1.txt", "가  R1을 가져옴");
	// print message: getpid()가  R1을 가져옴

	Release(L1);
}

Take_R2() {
	Acquire(L2);
	// R2이 1이면 젓가락이 있고 0이면 젓가락이 없어서 기다려야 함
	while (GetValue("R2.txt") == 0) {
		PrintState("R2.txt", "가 R2을 기다림");
		// print message: getpid()가 R2을 기다림
		Wait(C2, L2);
		PrintState("R2.txt", "가 R2을 기다리다가 깨어남");
		// print message: getpid()가 R2을 기다리다가 깨어남
	}
	Change("R2.txt", 0);
	PrintState("R2.txt", "가  R2을 가져옴");
	// print message: getpid()가  R2을 가져옴

	Release(L2);
}

Take_R3() {
	Acquire(L3);
	// R3이 1이면 젓가락이 있고 0이면 젓가락이 없어서 기다려야 함
	while (GetValue("R3.txt") == 0) {
		PrintState("R3.txt", "가 R3을 기다림");
		// print message: getpid()가 R3을 기다림
		Wait(C3, L3);
		PrintState("R3.txt", "가 R3을 기다리다가 깨어남");
		// print message: getpid()가 R3을 기다리다가 깨어남
	}
	Change("R3.txt", 0);
	PrintState("R3.txt", "가  R3을 가져옴");
	// print message: getpid()가  R3을 가져옴

	Release(L3);
}

Put_R1() {
	Acquire(L1);
	Change("R1.txt", 1);
	Signal(C1);
	PrintState("R1.txt", "가  R1을 내려둠");
	// print message: getpid()가 R1을 내려둠
	Release(L1);
}

Put_R2() {
	Acquire(L2);
	Change("R2.txt", 1);
	Signal(C2);
	PrintState("R2.txt", "가  R2을 내려둠");
	// print message: getpid()가 R2을 내려둠
	Release(L2);
}

Put_R3() {
	Acquire(L3);
	Change("R3.txt", 1);
	Signal(C2);
	PrintState("R3.txt", "가  R3을 내려둠");
	// print message: getpid()가 R3을 내려둠
	Release(L3);
}

Think() {
	sleep(2);
}

Eating() {
	sleep(2);
}
Phil_A() {
	Take_R1();
	Print_Phil("가  생각을 시작함[Phil_A]");
	// print message: getpid()가  생각을 시작함
	Think(); // sleep()으로 구현
	Print_Phil("가  생각을 멈춤[Phil_A]");
	 // print message: getpid()가  생각을 멈춤
	Take_R2();
	Print_Phil("가  먹기 시작함[Phil_A]");
	// print message: getpid()가  먹기 시작함
	Eating(); // sleep()으로 구현
	Print_Phil("가  먹기를 멈춤[Phil_A]");
			 // print message: getpid()가  먹기를 멈춤
	Put_R1();
	Put_R2();
}

Phil_B() {
	Take_R2();
	Print_Phil("가  생각을 시작함[Phil_B]");
	// print message: getpid()가  생각을 시작함
	Think(); // sleep()으로 구현
	Print_Phil("가  생각을 멈춤[Phil_B]");
	// print message: getpid()가  생각을 멈춤
	Take_R3();
	Print_Phil("가  먹기 시작함[Phil_B]");
	// print message: getpid()가  먹기 시작함
	Eating(); // sleep()으로 구현
	Print_Phil("가  먹기를 멈춤[Phil_B]");
	// print message: getpid()가  먹기를 멈춤
	Put_R2();
	Put_R3();
}

Phil_C() {
	Take_R2();
	Print_Phil("가  생각을 시작함[Phil_C]");
	// print message: getpid()가  생각을 시작함
	Think(); // sleep()으로 구현
	Print_Phil("가  생각을 멈춤[Phil_C]");
	// print message: getpid()가  생각을 멈춤
	Take_R3();
	Print_Phil("가  먹기 시작함[Phil_C]");
	// print message: getpid()가  먹기 시작함
	Eating(); // sleep()으로 구현
	Print_Phil("가  먹기를 멈춤[Phil_C]");
	// print message: getpid()가  먹기를 멈춤
	Put_R2();
	Put_R3();
}


void main(int argc, char *argv[])
{
	key_t semkey = 201223420 ;
	key_t semkey2 = 2012232420 + 1;
	key_t semkey3 = 201223420 + 2;
	key_t semkey4 = 2012322420 + 3;
	key_t semkey5 = 201232420 + 4;
	key_t semkey6 = 201234220 + 5;
	
	
	L1 = initsem(semkey, 1);
	L2 = initsem(semkey2, 1);
	L3 = initsem(semkey3, 1);

	C1 = initsem(semkey4, 0);
	C2= initsem(semkey5, 0);
	C3= initsem(semkey6, 0);

	if (strcmp(argv[1], "Phil_A") == 0)
		Phil_A();
	else if (strcmp(argv[1], "Phil_B") == 0)
		Phil_B();
	else if (strcmp(argv[1], "Phil_C") == 0)
		Phil_C();
}

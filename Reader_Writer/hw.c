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

int lock;
int okTo;
int okToRead;
int okToWrite;

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
//Broadcast() Wake up all waiters
void Broadcast(int okTo)
{
	int j =GetValue("WR.txt");
	int i = 0;
	for (i = 0; i < j; i++)
		Signal(okTo);
}

//change counters last value
void Change(char* counter_name, int ch)
{
	FILE *f = fopen(counter_name, "a+");
	int res = GetValue(counter_name) + ch;
	time(&t);
	fprintf(f, "\n ID %d 시간 : %s",getpid(), ctime(&t));
	fprintf(f, "\n %d", res);
	fclose(f);
}
//WW, WR ,AW, AW return last value
int GetValue(char* counter_name)
{
	FILE *f = fopen(counter_name, "r");
	int res;
	char buf[100];
	while(fgets(buf,100,f) !=NULL)
	{}
	res = atoi(buf);	
	fclose(f);
	return res;
}
// Reader()
void Reader(int sl1, int sl2)
{
	sleep(sl1);
	Acquire(lock); 
	while (GetValue("AW.txt") + GetValue("WW.txt") > 0) //Is it safe to read?
	{
		Change("WR.txt", 1);							//No. Writers exist
		Wait(okToRead, lock);							//Sleep on cond var
		Change("WR.txt", -1);							// No longer waiting
	}
	Change("AR.txt", 1);								// Now we are active!
	Release(lock);
	
	// Perform actual read-only access
	//AccessDatabase(ReadOnly);
	sleep(sl2);
	// Now, check out of system
	Acquire(lock);
	Change("AR.txt", -1);								// No longer active
	if (GetValue("AR.txt") == 0 && GetValue("WW.txt") > 0) // No other active readers
		Signal(okToWrite);								// Wake up one writer
	Release(lock);
}

void Writer(int sl1, int sl2)
{
	sleep(sl1);
	Acquire(lock);
	while (GetValue("AW.txt") + GetValue("AR.txt") > 0)	// Is it safe to write?
	{
		Change("WW.txt", 1);							// No. Active users exist
		Wait(okToWrite, lock);							// Sleep on cond var
		Change("WW.txt", -1);							// No longer waiting
	}
	Change("AW.txt", 1);								// Now we are active!
	Release(lock);
	// Perform actual read/write access
	//AccessDatabase(ReadWrite);
	sleep(sl2);
	// Now, check out of system
	Acquire(lock);
	Change("AW.txt", -1);								// No longer active
	if (GetValue("WW.txt") > 0)							// Give priority to writers
		Signal(okToWrite);								// Wake up one writer
	else if (GetValue("WR.txt") > 0)					// Otherwise, wake reader
		Broadcast(okToRead);							// Wake all readers
	Release(lock);
}

void main(int argc, char *argv[])
{
	key_t semkey = 20123420 ;
	key_t semkey2 = 201223420 + 1;
	key_t semkey3 = 20123420 + 2;
	
	
	lock = initsem(semkey, 1);
	okToRead = initsem(semkey2, 0);
	okToWrite = initsem(semkey3, 0);

	if (strcmp(argv[1], "reader") == 0) {
		Reader(atoi(argv[2]), atoi(argv[3]));
	}
	else if (strcmp(argv[1], "writer") == 0) {
		Writer(atoi(argv[2]), atoi(argv[3]));
	}
}

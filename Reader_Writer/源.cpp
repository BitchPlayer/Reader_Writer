#include <stdio.h>
#include <Windows.h>

#define MAX_THREAD 10
typedef struct
{
	int number;
	char thread_name[3];
	unsigned int require_moment;
	unsigned int persist_time;
}TEST_INFO;

TEST_INFO test_data[MAX_THREAD] =
{
	{1,"r1",0,1},
	{2,"r2",1,1},
	{3,"w1",3,3},
	{4,"r3",4,2},
	{5,"w2",5,6},
	{6,"w3",6,10},
	{7,"r4",7,8},
	{8,"r5",9,2},
	{9,"w4",10,18},
	{10,"w5",12,2}
};

CRITICAL_SECTION CS_DATA;//�ٽ���
HANDLE h_mutex_read_count = CreateMutex(NULL, FALSE, TEXT("mutex_read_count"));
HANDLE h_mutex_write_count = CreateMutex(NULL, FALSE, TEXT("mutex_write_count"));
HANDLE h_mutex_print = CreateMutex(NULL, FALSE, TEXT("mutex_print"));

int read_count = 0;//��������
int write_count = 0;//д������
void gotoxy(int x, int y)
{
	HANDLE hCon;
	hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD Pos;
	Pos.X = x;
	Pos.Y = y;
	SetConsoleCursorPosition(hCon, Pos);
}
int color(int num)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), num);
	return 0;
}

void RF_reader_thread(void* data)
{
	char thread_name[3];
	strcpy_s(thread_name, ((TEST_INFO*)data)->thread_name);
	Sleep(((TEST_INFO*)data)->require_moment * 1000);

	WaitForSingleObject(h_mutex_read_count, -1);//����
	read_count++;
	if (read_count == 1)
		EnterCriticalSection(&CS_DATA);//�����ٽ���������ʹ����EnterCriticalSection������LeaveCriticalSection�˳��ٽ���
									   //��Ϊ�˱�֤����һ�������߳̽�����߳�ʱ��д�߽��̾ͽ���ȥ������Դ���ˣ���������Ȼ���Լ��������������Դ��
									   //����Ϊ��EnterCriticalSection��LeaveCriticalSection�Ž�������䣩
	ReleaseMutex(h_mutex_read_count);

	WaitForSingleObject(h_mutex_print, -1);//����
	gotoxy(0, 2 * (int)(((TEST_INFO*)data)->number));
	color(10);
	printf("���߳�%s���ڶ�", thread_name);
	ReleaseMutex(h_mutex_print);

	Sleep(((TEST_INFO*)data)->persist_time * 1000);



	WaitForSingleObject(h_mutex_read_count, -1);//����
	read_count--;
	WaitForSingleObject(h_mutex_print, -1);//����
	gotoxy(25, 2 * (int)(((TEST_INFO*)data)->number));
	color(10);
	printf("���߳�%s�뿪", thread_name);
	ReleaseMutex(h_mutex_print);
	if (read_count == 0)
		LeaveCriticalSection(&CS_DATA);//�˳��ٽ���
	ReleaseMutex(h_mutex_read_count);
}


void RF_writer_thread(void* data)
{
	Sleep(((TEST_INFO*)data)->require_moment * 1000);
	EnterCriticalSection(&CS_DATA);

	WaitForSingleObject(h_mutex_print, -1);
	gotoxy(0, 2 * (int)(((TEST_INFO*)data)->number));
	color(12);
	printf("д�߳�%s����д", ((TEST_INFO*)data)->thread_name);
	ReleaseMutex(h_mutex_print);
	Sleep(((TEST_INFO*)data)->persist_time * 1000);
	LeaveCriticalSection(&CS_DATA);

	WaitForSingleObject(h_mutex_print, -1);
	gotoxy(25, 2 * (int)(((TEST_INFO*)data)->number));
	color(12);
	printf("д�߳�%s�뿪", ((TEST_INFO*)data)->thread_name);
	ReleaseMutex(h_mutex_print);
}

void reader_first()//��������
{
	int i = 0;
	HANDLE h_thread[MAX_THREAD];
	printf("�������������:");
	for (i = 0; i < MAX_THREAD; i++)
	{
		printf("%s", test_data[i].thread_name);
	}
	printf("\n");
	printf("�����Ȳ�������:\n");
	InitializeCriticalSection(&CS_DATA);//InitializeCriticalSection�����ĳ�ʼ�������ʹ�ã����ұ���ȷ�������߳��е��κ���ͼ���ʴ˹�����Դ�Ĵ��붼���ڴ��ٽ����ı���֮��
	for (i = 0; i < MAX_THREAD; i++)
	{
		if (test_data[i].thread_name[0] == 'r')
			h_thread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(RF_reader_thread), &test_data[i], 0, NULL);
		else
			h_thread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(RF_writer_thread), &test_data[i], 0, NULL);
	}
	WaitForMultipleObjects(MAX_THREAD, h_thread, TRUE, -1);
	printf("\n");
}

void main()
{
	reader_first();
}

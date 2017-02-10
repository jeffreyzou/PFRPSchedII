#pragma once

#ifdef _THIS_IS_LINUX
#define INT64 int64_t
#include <string.h>
#else
#include <windows.h>
#endif

#include <fstream>
#include <iostream>
#include <vector>
#include <string>
using namespace std;

#define TS_STATE_ERROR		0
#define TS_STATE_SCHEDED	1
#define TS_STATE_UNKNOWN	2
// 5-10: unschedulable for different reasons
#define TS_STATE_UNSCHED	5
#define TS_STATE_FAIL_NECESSARY_1	6
#define TS_STATE_INSUFMEM	7
#define TS_STATE_EMPTY	8
#define TS_STATE_UNSCHED_INHERIT	9

#define TS_STATE_LAST_ONE	16

//#define TS_GEN_OFFSET	1

//char action;// 0-done; 1-abort;7-merged
#define TS_ACTION_DONE	0
#define TS_ACTION_ABORT	1
#define TS_ACTION_MERGE	7

// task model: How C changs
#define TASK_MODEL_COLD_WARM	1
#define TASK_MODEL_DISTANCE		2

// job mode
#define	JOB_MODE_AR								0
#define	JOB_MODE_PREEMPTIBLE				1
#define	JOB_MODE_NON_PREEMPTIBLE		2

#define JOB_MODE_ADVANCING_LOOP		1
#define JOB_MODE_ADVANCING_STOP		2


typedef struct _job_t {
	int c;
	int d;
	int p;
	int mode;
//	int pri;
//	int offset;
} job_t, *pjob_t;
// simple task struct, use less memory than class task
typedef struct _task_t {
	char name[8];
	vector<job_t> vec_job;
	int		c;
	int		d;
	int		p; //period
	int		offset;
	int		pri;
} task_t, *ptask_t;

typedef struct _taskset_t {
	char tag[16];
	vector<task_t *> vec;
//	int failedTask;
} taskset_t, *ptaskset_t;

typedef struct _config_info {
	int alg;
	bool merge;
	bool occupyshort;
	bool useoffset;
} config_info, *pconfig_info;

INT64 gcd(INT64 a, INT64 b);
INT64 lcm(INT64 a, INT64 b);

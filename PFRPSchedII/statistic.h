#pragma once

#include "PFRPSchedII.h"
#include "task.h"
#include "taskset.h"

#include <fstream>

#ifdef _THIS_IS_LINUX
#include <sys/time.h>
#include <time.h>
#endif

class statistic
{
private:
	string logfile_0;
	string logfile_1;
	string logfile_2;

	ofstream fout;

	vector<string> logs;

public:

	string tag;
#ifdef _THIS_IS_LINUX
	struct timeval t_start, t_end;
#else
	unsigned long time_s;
#endif
	int num_results[TS_STATE_LAST_ONE];
	int num_tried;
//	int num_sched;
//	int num_unsched;
	int num_lmax_failed;
//	int num_fmem;

	statistic(string ss, int n);
	~statistic(void);

	void start_stat();
	int end_stat(taskset *ts);

	void printLog();
};


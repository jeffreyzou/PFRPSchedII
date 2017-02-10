#include "stdafx.h"
#include "statistic.h"
#include <time.h>
#ifdef _THIS_IS_LINUX
#include <sys/time.h>
#include <string.h>
#else
#include <mmsystem.h>
#pragma comment (lib, "winmm.lib")
#endif

statistic::statistic(string ss, int n)
{
#ifdef _THIS_IS_LINUX
	string	_logfile = "../PFRPSchedII/Results/T"+to_string(n)+"/"+ss;
#else
	string	_logfile = ".\\Results\\T"+to_string(n)+"\\"+ss;
#endif
	tag = "T"+to_string(n)+"_"+ss;

	string columns = "tag, status, run time";
	for(int i=0; i<n; i++)
		columns += ", wcrt["+ to_string(i) + "]";
	for(int i=0; i<n; i++)
		columns += ", wcrtAt["+ to_string(i) + "]";
	for(int i=0; i<n; i++)
		columns += ", average_resp_time["+ to_string(i) + "]";
	for(int i=0; i<n; i++)
		columns += ", total_exec_time["+ to_string(i) + "]";
	columns += ", timesInMode[]";
/*
	logfile_0 = _logfile + "_sch.csv";
	fout.open(logfile_0);
	fout << columns << endl;
	fout.close();

	logfile_1 = _logfile + "_unsch.csv";
	fout.open(logfile_1);
	fout << columns << endl;
	fout.close();
*/
	logfile_2 = _logfile + "_results.csv";
	fout.open(logfile_2);
	fout << columns << endl;
	fout.close();

	memset(num_results, 0, sizeof(num_results));
	num_tried = 0;
//	num_sched = 0;	
//	num_unsched = 0;	
	num_lmax_failed = 0;
//	num_fmem = 0;	
}


statistic::~statistic(void)
{
}

void statistic::start_stat()
{
#ifdef _THIS_IS_LINUX
	gettimeofday(&t_start, NULL);
#else
	time_s = timeGetTime();
#endif	
}

int statistic::end_stat(taskset *ts)
{
#ifdef _THIS_IS_LINUX
	gettimeofday(&t_end, NULL);
	long time_s = ((long)t_start.tv_sec)*1000+(long)t_start.tv_usec/1000;
	long ets = ((long)t_end.tv_sec)*1000+(long)t_end.tv_usec/1000;
#else
	unsigned long ets = timeGetTime();
#endif	
	if(ets < time_s)
		ets += (unsigned long)(-1) - time_s;
	else
		ets -= time_s;

	num_tried++;

	num_results[ts->state]++;
/*
	if(ts->state == TS_STATE_SCHEDED)
		num_sched++;
	else if(ts->state == TS_STATE_INSUFMEM)
		num_fmem++;
	else if(ts->state > TS_STATE_UNSCHED)
	{
		if(ts->lmax_fail_at != -1)
			num_lmax_failed++;
		num_unsched++;
	}
*/

	string line;
	line = ts->tag;
	line += ", " + to_string(ts->state);
	line +=", " + to_string(ets);
	for(int i=0; i<ts->num_task; i++)
		line += ", " + to_string(ts->tq[i]->wcrt);
	for(int i=0; i<ts->num_task; i++)
		line += ", " + to_string(ts->tq[i]->wcrtAt);
	for(int i=0; i<ts->num_task; i++)
	{
		if(ts->tq[i]->job_scheduled)
			line += ", " + to_string(ts->tq[i]->total_resp_time/ts->tq[i]->job_scheduled);
		else
			line += ", -1";
	}
	for(int i=0; i<ts->num_task; i++)
		line += ", " + to_string(ts->tq[i]->total_exec_time);
	INT64 sum;	
	for(int i=0; i<ts->tq[0]->num_job_modes; i++)
	{
		sum = 0;
		for(int j=0; j<ts->num_task; j++)
			sum += ts->tq[j]->timesInMode[i];
		line += ", " +to_string(sum);
	}

	logs.push_back(line);

	return ets;
}

void statistic::printLog()
{
	// to file
	fout.open(logfile_2, fstream::out | std::fstream::app);
	for(vector<string>::iterator it = logs.begin(); it!=logs.end(); it++)
		fout << *it << endl;
	fout.close();
	logs.clear();
}
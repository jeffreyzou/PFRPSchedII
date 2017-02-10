#include "stdafx.h"
#include "task.h"


task::task(void)
{
}

task::~task(void)
{
		if(resp_time) {
			delete resp_time; 
			resp_time=NULL;
		}

		delete []timesInMode;
}

void task:: init()
{
	cur_job_mode = 0;
	c = vec_job[cur_job_mode].c;
	d = vec_job[cur_job_mode].d;
	p = vec_job[cur_job_mode].p;
	num_restarts = 0;
	next_start_tick = offset;
	job_scheduled = 0;
	obsolute_release_time = offset;	
	sched_state = TASK_SCHED_STATE_OK;
	num_job_modes = vec_job.size();
	cur_start_exec_time = 0;

	timesInMode = new INT64[num_job_modes];
	memset(timesInMode, 0, num_job_modes * sizeof(INT64));

	wcrt = 0;
	total_resp_time = 0;
	total_exec_time = 0;
}

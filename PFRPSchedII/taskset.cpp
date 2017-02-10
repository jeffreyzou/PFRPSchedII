#include "stdafx.h"
#include "taskset.h"
#include <time.h>       /* time */

#include "task.h"

#ifdef _THIS_IS_LINUX
// for memset()
#include <string.h>
#endif

#include <algorithm>

extern config_info g_config;
extern vector<taskset_t *> g_vec_tasksets;

extern int g_bcet; // b value
extern int g_system_f; // f value

taskset::taskset(int nt)
{
	num_task = nt;
	LCMs = new INT64[num_task];
	minOffsetsO = new int[num_task];
	minOffsetsZ = new int[num_task];
	memset(minOffsetsZ, 0, num_task*sizeof(int));
	minOffsets = minOffsetsO;
	t_stables = new int[num_task];
	memset(t_stables, 0, num_task*sizeof(int));
	maxOffsets = new int[num_task];
	memset(maxOffsets, 0, num_task*sizeof(int));
	util = 0;
	maxOffset = 0;
	num_nodes_used = new INT64[num_task];
	memset(num_nodes_used, 0, num_task*sizeof(INT64));
	num_nodes_used_after_propagation = new INT64[num_task];
	memset(num_nodes_used_after_propagation, 0, num_task*sizeof(INT64));
	estimateWcrt = new INT64[num_task];
	memset(estimateWcrt, -1, num_task*sizeof(INT64));
	realWcrt = new INT64[num_task];
	memset(realWcrt, -1, num_task*sizeof(INT64));
	wcrt_job = new INT64[num_task];
	memset(wcrt_job, -1, num_task*sizeof(INT64));
	failedTask = "NONE";
	state = TS_STATE_UNSCHED;
	num_ticks = 0;

	jobsInserted = new int*[num_task];
	for(int i=0; i<num_task; i++)
		jobsInserted[i] = new int[num_task];
//	setJobInserted(-1, 0x10);// 0x10 - a big enough number

	task_model = TASK_MODEL_DISTANCE;
//	task_model = TASK_MODEL_COLD_WARM;

	failed_task = -1;
	lmax_fail_at = -1;
	wcrt_failed_job = -1;

	t_stable = 0;
}


taskset::~taskset(void)
{
	delete[] estimateWcrt;
	delete[] realWcrt;
	delete[] wcrt_job;
	delete[] num_nodes_used;
	delete[] num_nodes_used_after_propagation;
	delete[] minOffsetsO;
	delete[] minOffsetsZ;
	delete[] LCMs;

	for(int i=0; i<num_task; i++)
		delete[] jobsInserted[i];
	delete[] jobsInserted;

}


void taskset::reset(bool destroyTq)
{
	memset(num_nodes_used, 0, num_task*sizeof(INT64));
	memset(num_nodes_used_after_propagation, 0, num_task*sizeof(INT64));
	memset(wcrt_job, -1, num_task*sizeof(INT64));
	memset(realWcrt, -1, num_task*sizeof(INT64));
	memset(estimateWcrt, -1, num_task*sizeof(INT64));
	state = TS_STATE_UNSCHED;
	num_ticks = 0;
	failedTask = "NONE";

	failed_task = -1;
	lmax_fail_at = -1;
	wcrt_failed_job = -1;
	
	if(destroyTq)
	{
		for(int i=0; i<(int)tq.size(); i++)
			delete tq[i];
		tq.clear();
	}
	else
	{
//		setJobInserted(-1, tq[0]->num_job_modes);

		for(int i=0; i<(int)tq.size(); i++)
			tq[i]->init();
	}
}

void taskset::gen_vec_c(task_t *tt)
{
	job_t job;
	int f = g_bcet;

	tt->vec_job.clear();
	job.c = (int)ceil(tt->c * (f / 100.0f)); // job->c > 0
//	job.c = max(job.c, 2); // at least 2 for initial_busy?
	job.p = tt->p;
	job.d = tt->d;
	job.mode = JOB_MODE_AR;
	tt->vec_job.push_back(job);
	f += g_system_f;
	f = min(f, 100); // no more than 100%
	while(f < 100)
	{
		job.c = (int)ceil(tt->c * (f / 100.0f)); 
	//	job.c = max(job.c, 2); // at least 2 for initial_busy?
		job.p = tt->p;
		job.d = tt->d;
		job.mode = JOB_MODE_AR;
		tt->vec_job.push_back(job);

		f += g_system_f;
		f = min(f, 100); // no more than 100%
	}
}

int taskset::readFromMem(taskset_t *tset)
{
	vector<task_t *>::iterator it = tset->vec.begin();
	task_t *tt = *it;
	gen_vec_c(tt);
	task *t = new task(tt->name, tt->c, tt->vec_job, JOB_MODE_ADVANCING_STOP, tt->d, tt->p, tt->offset, tt->pri);

	tq.push_back(t);
	int i = 0;
	LCMs[i] =t->p;
	minOffsets[i] = t->offset;
	minOffset = t->offset;
	maxOffset = t->offset;
	maxOffsets[i] = t->offset;
	t_stables[i] = t->offset;
	t_stable = t_stables[i];
	util = t->util;
	it++;
	i++;
	for(; it!= tset->vec.end(); it++, i++)
	{
		tt = *it;
		gen_vec_c(tt);
		t = new task(tt->name, tt->c,tt->vec_job, JOB_MODE_ADVANCING_STOP, tt->d, tt->p, tt->offset,tt->pri);

		LCMs[i] = lcm(LCMs[i-1], t->p);
		minOffsets[i] = min(minOffsets[i-1], t->offset);
		tq.push_back(t);
		// TODO more research needed
		if(t->offset < maxOffsets[i-1])
		{
			maxOffsets[i] = maxOffsets[i-1];
			t_stables[i] = (int)(ceil(maxOffsets[i-1]*1.0/t->p)*t->p+t->offset);
		}
		else
		{
			maxOffsets[i] = t->offset;
			t_stables[i] = t->offset;
		}
		t_stable = max(t_stables[i], t_stable);
		util += t->util;
		maxOffset = max(maxOffset, t->offset);
		minOffset = min(minOffset, t->offset);
	}

//	setJobInserted(-1, tq[0]->num_job_modes);

	return 0;
}

void  taskset::setJobInserted(int task_i, int val)
{
	int k1, k2;
	if(task_i == -1)
	{
		k1 = 0;
		k2 = num_task;
	}
	else
		k1 = k2 = task_i;

	for(int j=k1; j<k2; j++)
		for(int r=0; r<num_task; r++)
			 jobsInserted[j][r] = val;
}


void taskset::printJobInserted(int task_i)
{
	int k1, k2;
	if(task_i == -1)
	{
		k1 = 0;
		k2 = num_task;
	}
	else
		k1 = k2 = task_i;

	for(int j=k1; j<k2; j++)
	{
		for(int r=0; r<num_task; r++)
			cout << jobsInserted[j][r] << ", ";
		cout << endl;
	}
}
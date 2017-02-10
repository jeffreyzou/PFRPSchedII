#pragma once

#include "PFRPSchedII.h"

class tntasksets
{
private:
	int num_job_modes;
	int arr_job_modes[10];
	task_t * parseLine(char *line);
	vector<taskset_t *> *_vec_tasksets;
	int priority_assign_alg;
public:

	string filename;
	tntasksets(void);
	~tntasksets(void);
	
	void reset();
	void priorityAssign(taskset_t * taskset);

	int lmax_failed_task_num;
	int wcrt_failed_task_num;
	int scheded_num;
	int sched_failed_num;
	int mem_failed_num;
	int readTasksetsFromFile(string filepath, vector<taskset_t *>  *vec_tasksets, int sorting, int chunk);
};
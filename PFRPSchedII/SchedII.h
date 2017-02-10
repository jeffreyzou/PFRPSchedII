#pragma once
#include "PFRPSchedII.h"
#include "task.h"
#include "taskset.h"

/*
Task: {<C_0, C_1, C_2, C_3, ..., C_m>, D, T, Pri}

C_0, C_1, ..., C_i, ..., C_m, i is the "distance", either 1) how many jobs of other tasks have been triggered, or 2) how many
tasks that have jobs been triggered, after the last time this job was preempted. The bigger the distance, the longer the C

*/

class SchedII
{
	private:
	taskset *ts;
	config_info *pconfig;

	INT64	wall_clk;
	INT64 Interval_End;

	task *cur_task;
	task *next_task;
	task *idle_task;
//	task *last_excuted_task; // the one other than idle_task

	void init();
	task *findNext();
	task *findNextFP(); // fixed priority
	bool checkDeadlineMiss();
	task *necessaryConditionCheck_1();
	void updateJobMode(task * t, int mode);
	void updateJobInserted();
	int getCurJobMode(task *t);

public:

	SchedII();
	~SchedII(void);

	int sched(taskset *_ts);
};


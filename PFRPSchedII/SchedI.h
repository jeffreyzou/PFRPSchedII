#pragma once

#include "PFRPSchedII.h"
#include "task.h"
#include "taskset.h"

/*
Task: {<C_0, C_1, C_2, C_3, ..., C_m>, D, T, Pri}
cold restart is C_0, i-th resume is C_i, after m resumes, c is C_0 again. for example, cache flushed.
C_i decreases by a number/factor/..., if C_i executed c is less than C_i-C_(i+1)? simplest way is repeat C_i instead of transit to C_(i+1)
*/

class SchedI
{
private:
	taskset *ts;
	config_info *pconfig;

	INT64	wall_clk;
	INT64 Interval_End;

	task *cur_task;
	task *next_task;
	task *idle_task;

	void init();
	task *findNext();
	task *findNextFP(); // fixed priority
	bool checkDeadlineMiss();
	task *necessaryConditionCheck_1();
public:

	SchedI();
	~SchedI(void);

	int sched(taskset *_ts);
};


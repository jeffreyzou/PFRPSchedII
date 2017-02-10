#pragma once

#include "PFRPSchedII.h"
#include "task.h"

class taskset
{
private:

	task_t * parseLine(char *line);

public:
	string tag;
	string failedTask;
	int failed_task;
	INT64 wcrt_failed_job;
	INT64 lmax_fail_at;
	vector<task *> tq;
//	int index_deadline_miss_task_in_tq;
	INT64 *LCMs;
	int *minOffsetsO;
	int *minOffsetsZ;
	int *minOffsets;
	int minOffset;
	int maxOffset;
	int *maxOffsets;
	int *t_stables; // stable sched points
	int t_stable; // max{t_stables}
	INT64 *num_nodes_used;
	INT64 *num_nodes_used_after_propagation;
	INT64 *estimateWcrt;
	INT64 *realWcrt;
	INT64 *wcrt_job;  // the (first) job which has wcrt
	int num_task;
	double util;

	// jobs released between two consective jobs of a task, index is the pri (it is ok for fixed priority sched)
	int **jobsInserted;
	int task_model; // 1 calls schedI; 2 calls schedII

	int state; // end state, 0: scheduled, 1:un_schedulable, 2: insufficient memory, 3: empty ...
	INT64 num_ticks; // how many cpu ticks are used

	taskset(int nt);
	~taskset(void);

	void reset(bool destroyTq);

	void gen_vec_c(task_t *tt);
	int readFromMem(taskset_t *tset);

	void setJobInserted(int task_i, int val);
	void printJobInserted(int task_i);
};


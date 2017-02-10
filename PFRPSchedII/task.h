#pragma once

#include "PFRPSchedII.h"

#define TASK_SCHED_STATE_OK 0
#define TASK_SCHED_STATE_DEADLINE_MISS	1

class task {
public:
	int		c;
	vector<job_t>  vec_job;
	int		d;
	int		p; //period
	int		offsetO;
	int		offset;
	int		pri;
	int		cur_job_mode;
	int		num_restarts; // restart times of a job
	int		num_job_modes;
	int		modeAdvance; // 0: looping, 1: ascending to the last and stop

	INT64	*timesInMode;	 // how many times a mode is used
	INT64	obsolute_release_time;
	INT64	next_start_tick;
	INT64	cur_start_exec_time;

	INT64	job_scheduled;
	std::string	name;
	float	util;

	int sched_state; // 0: ok; 1: deadline miss
	int *resp_time;

	INT64 wcrt;
	INT64 wcrtAt;
	INT64 total_resp_time;
	INT64 total_exec_time;
	
	task();

	task(std::string _name, int _c, vector<job_t> _vec_job, int _modeAdvance, int _d, int _p, int _offset = 0, int _pri = 0) : name(_name), c(_c), vec_job(_vec_job), modeAdvance(_modeAdvance), d(_d), p(_p), offset(_offset), pri(_pri)
	{
		util = (float)((c*1.0)/p);
		offsetO =_offset;
		offsetO =0;
		offset =0;
		resp_time = NULL;

		init();
	};

	~task();

	void init();

//	bool operator<(const task &rhs) const { return p < rhs.p; };

};
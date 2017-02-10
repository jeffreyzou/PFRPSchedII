#include "stdafx.h"
#include "SchedII.h"
#include <fstream>

//#define MY_DBG
#ifdef  MY_DBG
#define MYLOG_OPEN(x)		fout.open(x)
#define MYLOG(x)				fout << (x)
#define MYLOG_CLOSE		fout.close()
#else
#define MYLOG_OPEN(x)	
#define MYLOG(x)
#define MYLOG_CLOSE
#endif 

SchedII::SchedII()
{
	idle_task = new task();
	idle_task->name = "idle";
	idle_task->c = 0xFFFFFFF; 
	idle_task->d = 0xFFFFFFF; 
	idle_task->p = 0xFFFFFFF; 

//	last_excuted_task = NULL;
}


SchedII::~SchedII(void)
{
	delete idle_task;
}

void SchedII::init()
{
	wall_clk = 0;
	cur_task = NULL;
	next_task = NULL;

	idle_task->next_start_tick = wall_clk;
	idle_task->pri = ts->num_task; // the lowest priority
	idle_task->cur_start_exec_time = wall_clk;

	ts->setJobInserted(-1, ts->tq[0]->num_job_modes);
}

int SchedII::sched(taskset *_ts)
{
	ts = _ts;

	init();

	int retval = TS_STATE_SCHEDED;
	INT64 respt;

	task *t;

	string logfile_0 = ".\\Results\\T"+to_string(ts->num_task)+"\\"+ts->tag+"_schedule.csv";
	ofstream fout;
	MYLOG_OPEN(logfile_0);
	string outstring;
	MYLOG("task name, pri, c, p:\n");
	for(vector<task *>::iterator it=ts->tq.begin();it!=ts->tq.end();it++)
	{
		t=(*it);
		outstring = t->name+", "+to_string(t->pri)+", "+to_string(t->c)+", "+to_string(t->p);
		MYLOG(outstring+"\n");
	}
	MYLOG("\nschedule start, end, task, action\n");


	t = necessaryConditionCheck_1();
	if(t)
	{
		retval = TS_STATE_FAIL_NECESSARY_1;
		goto _exit;
	}

	// init
	cur_task =idle_task;
	
	// feasibility interval
	Interval_End = ts->LCMs[ts->num_task-1];

	while(true)
	{
		next_task = findNext();
		if(cur_task == idle_task)
		{
			if(next_task != idle_task)
			{
				// update wall clock
				wall_clk = max(next_task->next_start_tick, wall_clk); 

				// update cur_task -- preempted task, idle_task always preemptible
				cur_task->next_start_tick = wall_clk+1;
			
				// update next_task -- preempting task
				next_task->cur_start_exec_time = wall_clk;

				// set next_task's C
				updateJobMode(next_task, min(getCurJobMode(next_task), next_task->num_job_modes-1));// index of mode starts from 0	
				next_task->timesInMode[next_task->cur_job_mode]++;

				// start the preempting task
				cur_task = next_task;

				// update jobInserted
				updateJobInserted();

				// cur_task execute at lease 1 tick
				wall_clk++;

			}
			else // cur_task==next_task==idle_task, done scheduling
			{
				wall_clk = Interval_End;
				goto _exit;
			}
		}
		else // cur_task != idle_task
		{
			if(next_task == cur_task) // cur_task can finish. next_task==cur_task!=idle_task
			{
				t= cur_task;
				wall_clk = t->cur_start_exec_time + t->c;
				t->total_exec_time += t->c;
				respt = wall_clk - t->obsolute_release_time;
				t->total_resp_time += respt;
				if(t->wcrt <  respt)
				{
					t->wcrt =  respt;
					t->wcrtAt = t->job_scheduled;
					if(t->wcrt > t->vec_job[t->num_job_modes-1].d)
					{
						ts->failed_task = t->pri;
						retval = TS_STATE_UNSCHED;
						goto _exit;
					}
				}
				// update cur_task -- preempted task
				t->job_scheduled++;
				t->obsolute_release_time += t->p;
				t->next_start_tick = t->obsolute_release_time;
				t->num_restarts = 0;
				// "distance" applied, so do not reset cur_job_mode, reset jobInserted instead
				// updateJobMode(t, 0);
				memset(ts->jobsInserted[t->pri], 0, sizeof(int)*ts->num_task);
			
				// update next_task -- preempting task
				idle_task->next_start_tick = wall_clk;
				cur_task = idle_task;

				outstring = to_string(t->cur_start_exec_time)+", "+to_string(wall_clk)+",  "+t->name+", finish!\n";
				MYLOG(outstring);
			}
			else // cur_task is preempted
			{
				wall_clk = max(next_task->next_start_tick, wall_clk);
				cur_task->total_exec_time += (wall_clk - cur_task->cur_start_exec_time);
				outstring = to_string(cur_task->cur_start_exec_time)+", "+to_string(wall_clk)+",  "+cur_task->name+", preempted by, "+next_task->name+"\n";
				MYLOG(outstring);

				// update cur_task -- preempted task
				t = cur_task;
				job_t *jb = &(t->vec_job[t->cur_job_mode]);
				if(jb->mode == JOB_MODE_AR)
				{				
					t->num_restarts++;
					t->next_start_tick = wall_clk + next_task->c;	// not ealier than finish of the preempting job

					//  reset preempted task's jobInserted
					memset(ts->jobsInserted[t->pri], 0, sizeof(int)*ts->num_task);

					// update next_task -- preempting task
					next_task->cur_start_exec_time = wall_clk;

					// set next_task's C
					updateJobMode(next_task, min(getCurJobMode(next_task), next_task->num_job_modes-1));// index of mode starts from 0	
					next_task->timesInMode[next_task->cur_job_mode]++;

					// start the preempting task
					cur_task = next_task;

					// update jobInserted
					updateJobInserted();

					// cur_task execute at lease 1 tick
					wall_clk++;
				}
				else if(jb->mode == JOB_MODE_PREEMPTIBLE)	{; /*TODO*/}
				else if(jb->mode == JOB_MODE_NON_PREEMPTIBLE){;	/*TODO*/}
				else{	; /* nothing to do*/}
			}
		}// cur_task != idle_task

		if(checkDeadlineMiss()) // has deadline miss
		{
			retval = TS_STATE_UNSCHED;
			goto _exit;
		}
	} // while()

_exit:
	if(retval == TS_STATE_SCHEDED)
		MYLOG("\nSUCCESSFUL.\n");
	else if(retval == TS_STATE_FAIL_NECESSARY_1)
	{
		outstring =  "\nFAILED necessary condition 1, "+t->name+", pri="+to_string(t->pri)+".\n";
		MYLOG(outstring);
	}
	else
	{
		t = ts->tq[ts->failed_task];
		outstring = "\nFAILED, "+t->name+", pri="+to_string(t->pri)+", stop at tick="+to_string(wall_clk)+"\n";
		MYLOG(outstring);
	}
	
	MYLOG_CLOSE;
	ts->state = retval;

	return retval;
}

task *SchedII::necessaryConditionCheck_1()
{
	task *t = ts->tq[0];
	int laxity = t->p - t->c;
	for(int i=1; i<ts->num_task; i++)
	{
		t = ts->tq[i];
		if(t->vec_job[0].c > laxity) // the shortest one in TASK_MODEL_COLD_WARM
		{
			ts->failed_task = i;
			return t;
		}
	}
	return NULL;
}

bool SchedII::checkDeadlineMiss()
{
	int i;
	task *t;
	bool retval = false;

	for(i=0; i<ts->num_task; i++)
	{
		t = ts->tq[i];
		if(t->obsolute_release_time+t->p < wall_clk + t->c) // == wall_clk is ok
		{
			t->sched_state = TASK_SCHED_STATE_DEADLINE_MISS;
			ts->failed_task = i;
			//retval =true;
			return true;
		}
	}
	return retval;
}


task* SchedII::findNext()
{
	return findNextFP();
}

// return: cur_task -- cur_task can finish
//				others -- the future next closest higher priority release before finish cur_task, or the highest priority already-released
// if return==cur_task==idle_task, done scheduling
task* SchedII::findNextFP()
{
	if(cur_task->pri == 0) // highest priority
		return cur_task;

	int i;
	task *t, *next;
	INT64 finish_time = cur_task->cur_start_exec_time+cur_task->c;
	finish_time = min(Interval_End, finish_time);
	INT64 next_start=finish_time;

	next = cur_task;
	for(i=0; i<cur_task->pri; i++)
	{
		t = ts->tq[i];
		if(t->next_start_tick <= wall_clk) // highest already-released
			return t;
		if(t->next_start_tick < next_start) // nearest future
		{
			next_start = t->next_start_tick;
			next = t;
		}
	}

	return next;
}

void SchedII::updateJobInserted()
{
//	cout << "before update: \n";
//	ts->printJobInserted(-1);
	for(int i=0; i<ts->num_task; i++)
	{
		if(ts->tq[i] != cur_task)
			ts->jobsInserted[i][cur_task->pri]++;
	}
//	cout << "after update: \n";
//	ts->printJobInserted(-1);
//	cout << endl;
}

int SchedII::getCurJobMode(task *t)
{
	task *ti;
	int count = 0;

	for(int i=0; i<ts->num_task; i++)
	{
		ti = ts->tq[i];
		if(ti != t)
		{
			//count += ts->jobsInserted[ti->pri]; // another alg
			if(ts->jobsInserted[ti->pri] > 0)
				count++;
		}
	}
	return count;
}

void SchedII::updateJobMode(task *t, int mode)
{
	t->cur_job_mode = mode;
	t->c = t->vec_job[mode].c;
	t->d = t->vec_job[mode].d;
	t->p = t->vec_job[mode].p;
}
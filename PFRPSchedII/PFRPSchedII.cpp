// PFRPSchedII.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "PFRPSchedII.h"

#include "statistic.h"
#include "task.h"
#include "taskset.h"
#include "tntasksets.h"

#include "SchedI.h"
#include "SchedII.h"


#ifndef _THIS_IS_LINUX
#include <io.h>

#define ACCESS _access
#define MKDIR(a) _mkdir((a))
#endif

#include <array>
#include <vector>
#include <fstream>

//using namespace std;

config_info g_config;
config_info *g_configs;

string g_srcfolder;
string g_src_file;
string g_resultfolder;
string g_results_file;

int g_chunk;

vector<taskset_t *> *g_vec_tasksets;
#define g_vec_tasksets_sched_size 20
vector<taskset_t *> g_vec_tasksets_sched[g_vec_tasksets_sched_size]; // f_0 is neccessary to f_1, .... it is not true if we don't proceed when C_i^m executed too few

int g_bcet; // b value
int g_b_n; //n-th b
int g_system_f; // f value

statistic *g_stat;

// algorithms
SchedI *g_SchedI;
SchedII *g_SchedII;

void init();
void leave();
void cleanup();

int EUMSchedule(taskset *ts);
void plan_first(int k, int u, int f, int b, int alg);

int callScheduler(taskset *ts, int alg);

#ifdef _THIS_IS_LINUX
int main(int argc, char* argv[])
{
	g_srcfolder	= "../PFRPSchedII/Files";
	g_resultfolder	= "../PFRPSchedII/Results";
#else
int _tmain(int argc, char* argv[])
{
	g_srcfolder	= ".\\Files";
	g_resultfolder	= ".\\Results";

//	system("rmdir .\\Results /s /q");
//	system("xcopy .\\ResultsO .\\Results /e /t /i");

#endif	

	int min_n, max_n, alg;

	if(argc > 1)
	{
		min_n = atoi(argv[1]);
		max_n = atoi(argv[2]);	
		alg = atoi(argv[3]);	
		g_chunk = atoi(argv[4]);	
	}
	else
	{
		min_n = 3;
		max_n = 4;
		alg = 1;	
		g_chunk = 0;
	}

#ifdef _THIS_IS_LINUX
	g_results_file = g_resultfolder + "/results"+"_"+to_string(min_n)+"_"+to_string(max_n)+"_"+to_string(alg)+"_"+to_string(g_chunk)+".csv";
#else
	g_results_file = g_resultfolder + "\\results"+"_"+to_string(min_n)+"_"+to_string(max_n)+"_"+to_string(alg)+"_"+to_string(g_chunk)+".csv";
#endif

	init();

	ofstream fout;
	fout.open(g_results_file);
	fout << "taskset,num_passed,num_failed_necessary" << endl;
	fout.close();

	vector <int> vec_bcet;
	vec_bcet .push_back(20);
	vec_bcet .push_back(50);
	vec_bcet .push_back(80);

	vector <int> vec_f;
	vec_f .push_back(10);
	vec_f .push_back(20);
	vec_f .push_back(30);

	vector <int> vec_u;
	vec_u .push_back(20);
	vec_u .push_back(30);
	vec_u .push_back(40);
	vec_u .push_back(50);
	vec_u .push_back(60);
	vec_u .push_back(70);

	tntasksets* tnts;

	for(vector<int>::iterator itu=vec_u.begin(); itu!=vec_u.end(); itu++)
	{
		for(int k=min_n; k<max_n; k++)
		{
			tnts = new tntasksets(); 
			g_vec_tasksets_sched[0].clear();
			g_vec_tasksets = &g_vec_tasksets_sched[0];
#ifdef _THIS_IS_LINUX
			g_src_file = g_srcfolder+"/T"+to_string(k)+"/"+"_u"+to_string(*itu)+"_"+to_string(g_chunk)+".txt";
#else
			g_src_file = g_srcfolder+"\\T"+to_string(k)+"\\"+"_u"+to_string(*itu)+"_"+to_string(g_chunk)+".txt";
#endif
			tnts->readTasksetsFromFile(g_src_file, g_vec_tasksets, alg, g_chunk);
			if(g_vec_tasksets->size() == 0)
			{
				cout << "file not exists or is empty: " << g_src_file << endl;
				delete tnts;
			}

			for(vector<int>::iterator itf=vec_f.begin(); itf!=vec_f.end(); itf++)
			{
				g_system_f = *itf;
				for(int i=1; i<g_vec_tasksets_sched_size; i++)
					g_vec_tasksets_sched[i].clear();
				g_b_n = 0;
				for(vector<int>::iterator itb=vec_bcet.begin(); itb!=vec_bcet.end(); itb++)
				{
					g_bcet = *itb;
					plan_first(k, *itu, *itf, *itb, alg);
					g_b_n++;
				}
			}

			// AR
			g_system_f = 100;
			g_bcet = 100;
			plan_first(k, *itu, g_system_f, g_bcet, alg);

			delete tnts;
		}
	}

	leave();
	
#ifdef 	_THIS_IS_LINUX
	cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!DONE!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
#else
	cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!DONE!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
//	system("pause");
#endif
	
	return 0;
}

void plan_first(int k, int u, int f, int b, int alg)
{
	ofstream fout;
	string ssreulsts;
	int nt; //=k;
	int sched_num = 0;
	int sched_failed_num = 0;

	g_vec_tasksets = &g_vec_tasksets_sched[g_b_n];
	ssreulsts = "_u"+to_string(u)+"_f"+to_string(f)+"_b"+to_string(b)+"_alg"+to_string(alg)+"_"+to_string(g_chunk);
	if(g_vec_tasksets->empty())
	{
		fout.open(g_results_file, fstream::out | std::fstream::app);
		fout << "T" << k << ssreulsts << "," << 0<< ","  << 0 << endl;
		fout.close();
		return;
	}
	nt = (*g_vec_tasksets->begin())->vec.size();
	g_stat = new statistic(ssreulsts, nt);

	vector<taskset_t *>::iterator itts;
	taskset *ts = new taskset(nt);

	int endstate = TS_STATE_UNSCHED;
	for(itts=g_vec_tasksets->begin(); itts!=g_vec_tasksets->end(); itts++)
	{

		ts->num_task = nt;
		ts->readFromMem(*itts);
		ts->tag = (*itts)->tag;

		cout << "...on: " << (*itts)->tag << " of T" << nt << ssreulsts << endl;

		g_stat->start_stat(); // start clocking
		endstate = callScheduler(ts, alg);
		g_stat->end_stat(ts); // stop clocking

//_schedule_end:

		if(endstate == TS_STATE_SCHEDED)
		{
			g_vec_tasksets_sched[g_b_n+1].push_back(*itts);
			sched_num++;
		}
		else
		{
			if(endstate >= TS_STATE_UNSCHED)
				sched_failed_num++;
			cout <<g_src_file<<" now has "<<g_stat->num_tried-g_stat->num_results[TS_STATE_SCHEDED]<<" of "<<g_stat->num_tried<<" failed tasksets.\n";
		}
		cleanup(); // reset used_list and mmc_nodes
		ts->reset(true); // reset ts statistics such as response_time, etc, // release ts->tq[]

		if(g_stat->num_tried % 100 == 0)
			g_stat->printLog(); // results of T_n tasksets
	}
	g_stat->printLog(); // results of T_n tasksets, the rest
	cout <<g_src_file<<" has "<<g_stat->num_results[TS_STATE_SCHEDED]<<" of "<<g_stat->num_tried<< " schedulable tasksets.\n";

	fout.open(g_results_file, fstream::out | std::fstream::app);
	fout << "T" << k << ssreulsts << "," << g_stat->num_results[TS_STATE_SCHEDED] << ","  << g_stat->num_results[TS_STATE_FAIL_NECESSARY_1]<< endl;
	fout.close();

	delete ts;
	delete g_stat;
}

int callScheduler(taskset *ts, int alg)
{
	int endstate = TS_STATE_UNSCHED;

	switch (alg)
	{
	case 1:
		if(ts->task_model == TASK_MODEL_COLD_WARM)
			endstate = g_SchedI->sched(ts); // RM, EM, UM,... task_model_1
		else if(ts->task_model == TASK_MODEL_DISTANCE)
			endstate = g_SchedII->sched(ts); // RM, EM, UM,... task_model_2
		break;
	case 2: 
		endstate = EUMSchedule(ts); // EUM, task_model_1
		break;
	default:
		break;
	}
	return endstate;
}

void init()
{
	g_SchedI = new SchedI();
	g_SchedII = new SchedII();
}

void leave()
{
	delete g_SchedI;
	delete g_SchedII;
}

void cleanup()
{
}

int EUMReordering(taskset *ts, int i)
{
	int j, k, retval;
//	bool found = false;
	task *ti, *tj, *tk;
	retval = -1;
	ti = ts->tq[i];
	for(j=i-1; j>=0; j--)
	{
		tj = ts->tq[j];
		//if((tj->util < ti->util) || (abs(tj->util-ti->util <0.0000001 ) && tj->c > ti->c)) //RUM
		if((tj->util < ti->util) || (abs(tj->util-ti->util <0.0000001 ) && tj->d > ti->d))
		{
//			found = true;
			retval = j;
			// move
			tk = tj;
			for(k=j; k<i; k++)
			{
				ts->tq[k] = ts->tq[k+1];
				ts->tq[k]->pri = k;
			}
			ts->tq[k] = tk;
			ts->tq[k]->pri = k;
			
			if(j==0)
				break; //return 0;
			i = i-1;
			if(i==0)
				break;//return 0;
			ti = ts->tq[i];
		}
	}
//	if(found == false)
//		return -1;
	return retval;
}
/*
int EUMSchedule(taskset *ts)
{	
	int endstate = TS_STATE_UNKNOWN;
	int i, retval;
	int nt = ts->num_task;
	for(i=1; i<nt; i++)// start at least from t2
	{
		ts->num_task = i+1;
		// schedule ts
		if(ts->task_model == TASK_MODEL_COLD_WARM)
			endstate = g_SchedI->sched(ts);
		else if(ts->task_model == TASK_MODEL_DISTANCE)
			endstate = g_SchedII->sched(ts);
		// if failed: reorder, i=j-1
		if(endstate >= TS_STATE_UNSCHED)
		{
			retval = EUMReordering(ts, i);
			//if(retval == -1)
			if(retval < 0)
				goto _exit;
			i = retval;
			if(i>0)
				i--; // to reverse the i++ at next step
//			ts->reset(false);
		}
		if(ts->num_task < nt)
			ts->reset(false);
	}

_exit:
	ts->state = endstate;
	return endstate;
}
*/
int EUMSchedule(taskset *ts)
{	
	int endstate = TS_STATE_UNKNOWN;
	while(true)
	{
		if(ts->task_model == TASK_MODEL_COLD_WARM)
			endstate = g_SchedI->sched(ts);
		else if(ts->task_model == TASK_MODEL_DISTANCE)
			endstate = g_SchedII->sched(ts);

		// if failed: reorder
		if(endstate >= TS_STATE_UNSCHED)
		{
			if(EUMReordering(ts, ts->failed_task) < 0)//if( == -1)
				break;
			ts->reset(false);
		}
		else // sched
			break;
	}

	ts->state = endstate;
	return endstate;
}


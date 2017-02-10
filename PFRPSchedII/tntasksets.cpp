#include "stdafx.h"
#include "tntasksets.h"

#ifdef _THIS_IS_LINUX
#include <string.h>
#endif
#include <algorithm>

bool sortByP(const task_t* lhs, const task_t* rhs) 
{ 
    return lhs->p < rhs->p;
}
bool sortByC(const task_t* lhs, const task_t* rhs) 
{ 
    return lhs->c < rhs->c;
}
bool sortByV(const task_t* lhs, const task_t* rhs) 
{ 
    return (lhs->d - lhs->c) < (rhs->d - rhs->c);
}
bool sortByU(const task_t* lhs, const task_t* rhs) 
{ 
	return lhs->c/lhs->d < rhs->c/lhs->d;
}

tntasksets::tntasksets(void)
{
	reset();
}


tntasksets::~tntasksets(void)
{
	for(vector<taskset_t *> ::iterator itts = _vec_tasksets->begin(); itts != _vec_tasksets->end(); itts++)
	{
		for(vector<task_t *>::iterator itt = (*itts)->vec.begin(); itt != (*itts)->vec.end(); itt++)
			delete (*itt);
	}
}

void tntasksets::reset()
{
	lmax_failed_task_num = 0;
	wcrt_failed_task_num = 0;
	scheded_num = 0;
	sched_failed_num = 0;
	mem_failed_num = 0;
}

task_t * tntasksets::parseLine(char *line)
{
	if(strlen(line) == 0)
		return NULL;

	char * pch;
	int i;
	char *str[50];
	char *next_token = NULL;
	int num_para;

	num_para = 50;
#ifdef _THIS_IS_LINUX
	pch = strtok_r(line, ",", &next_token);
#else	
	pch = strtok_s(line, ",", &next_token);
#endif
	i = 0;
	while (pch != NULL)
	{
		str[i] = pch;
#ifdef _THIS_IS_LINUX
		pch = strtok_r(NULL, ",", &next_token);
#else	
		pch = strtok_s(NULL, ",", &next_token);
#endif
		i++;
		if(i==num_para)
			break;
	}
	num_para = i;
	//ID,Priority,Offset,Period, Execution Time 1, Execution Time 2, ...
	//T1,3,1,147,58, 55
	task_t *t = new task_t;
	if(!t)	return t;

	if(strlen(str[0]) > sizeof(t->name))
	{
#ifdef _THIS_IS_LINUX
		strncpy(t->name, str[0], 7);
#else	
		strcpy_s(t->name, 7, str[0]);
#endif		
		t->name[8] = '\0';
	}
	else
	{
#ifdef _THIS_IS_LINUX
		strcpy(t->name, str[0]);
#else	
		strcpy_s(t->name, str[0]);
#endif
		t->name[strlen(str[0])]='\0';
	}
	t->p = atoi(str[3]);
	t->d = t->p;
	t->offset = atoi(str[2]);
	t->pri = atoi(str[1]);
	int j;
	for(j=0,i=4; i<num_para; i++,j++)
	{
		job_t jb;
		jb.c = atoi(str[i]);
		jb.d = t->d;
		jb.p = t->p;
		jb.mode = arr_job_modes[j];
		t->vec_job.push_back(jb);
	}
	t->c = t->vec_job[0].c;

	return t;
}

int tntasksets::readTasksetsFromFile(string filepath, vector<taskset_t *> *vec_tasksets, int sorting, int chunk)
{
	filename = filepath;
	_vec_tasksets = vec_tasksets;
	ifstream fin(filename);
	char chline[128];
	if(!fin.good())
	{
		cout << "file not found:" << filename << endl;
		return -1;
	}
	
	int i;
	fin.getline(chline, sizeof(chline));
	// num_task_per_taskset=??
	i = sizeof("num_task_per_taskset=") - 1;
	char *tline = &chline[i];
	int num_task_per_taskset = atoi(tline);
	// num_taskset=??
	fin.getline(chline, sizeof(chline));
	i = sizeof("num_taskset=") - 1;
	tline = &chline[i];
	int num_tasksets = atoi(tline);

	////ID,Priority,Offset,Period, Execution Time
	fin.getline(chline, sizeof(chline));

//	int textfile_linestoskip = g_textfilestart*(num_task_per_taskset+1); // skip the first g_textfilestart tasksets
//	for(i=0; i<textfile_linestoskip; i++)
//		fin.getline(chline, sizeof(chline));

	taskset_t *taskset;
	task_t *t;
	while((num_tasksets>0) && !fin.eof())
	{
		// tag
		taskset = new taskset_t;
		fin.getline(taskset->tag, 16);
		taskset->tag[15] = '\0';
#ifdef _THIS_IS_LINUX
		taskset->tag[strlen(taskset->tag)-1]='\0';
#endif
		i = 0;
		while(i < num_task_per_taskset)
		{
			// task
			fin.getline(chline, 128);
			if(strlen(chline) < 3)
				continue;
			t = parseLine(chline);
			if(!t)
				return 0;
			taskset->vec.push_back(t);
			i++;
		}
		// no task
		if(taskset->vec.size() == 0)
			return 0;

		priority_assign_alg = sorting;
		priorityAssign(taskset);

		_vec_tasksets->push_back(taskset);

		num_tasksets--;
	}

	fin.close();	

	return 1;
}

void tntasksets::priorityAssign(taskset_t *taskset)
{
		if(priority_assign_alg == 0)
			return;

		if(priority_assign_alg == 1)			
			sort(taskset->vec.begin(), taskset->vec.end(), sortByP); //RM
		else if(priority_assign_alg == 2)
			sort(taskset->vec.begin(), taskset->vec.end(), sortByC); //EM
		else if(priority_assign_alg == 3)
			sort(taskset->vec.begin(), taskset->vec.end(), sortByC); //VM

		task_t *t;
		for (int i=0; i<(int)taskset->vec.size(); i++)
		{
			t = taskset->vec.at(i);
			t->pri = i;
		}
}

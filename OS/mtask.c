#include"bootpack.h"

struct TASKCTL *taskctl;
struct TIMER* task_timer;

void task_idle(void)
{
	while(1)
	{
		io_hlt();
	}
}

struct TASK *task_now(void)
{
	/* 現在のレベルのタスク群を取得する */
	struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
	return tl->tasks[tl->now];
}

void task_add(struct TASK *task)
{
	struct TASKLEVEL *tl = &taskctl->level[task->level];
	tl->tasks[tl->running] = task;
	tl->running++;
	task->flags = 2; /* 動作中 */
	return;
}

void task_remove(struct TASK *task)
{
	int i;
	struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
	
	for(i = 0;  i < MAX_TASKLEVELS; i++)
	{
		if(tl->tasks[i] == task)
		{
			break;
		}
	}
	
	tl->running--;
	if(i < tl->now)
	{
		tl->now--;
	}
	if(tl->now >= tl->running)
	{
		tl->now = 0;
	}
	task->flags = 1;
	
	/* ずらす */
	for(; i< tl->running; i++)
	{
		tl->tasks[i] = tl->tasks[i + 1];
	}
	
	return;
}

/* タスクスイッチの時に次のレベルを決定する関数 */
void task_switchsub(void)
{
	int i;
	/* 上位レベルのタスク群を見つける */
	for(i = 0; i < MAX_TASKLEVELS; i++)
	{
		if(taskctl->level[i].running > 0)
		{
			/* 発見 */
			break;
		}
	}
	
	taskctl->now_lv = i;
	taskctl->lv_change = 0;
	return;
}


struct TASK *task_init(struct MEMMAN *memman)
{
	int i;
	struct TASK *task, *idle;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR*)ADR_GDT;
	taskctl = (struct TASKCTL *)memman_alloc_4k(memman, sizeof(struct TASKCTL));
	for(i = 0; i < MAX_TASKS; i++)
	{
		taskctl->tasks0[i].flags = 0;
		taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8;
		set_segmdesc(gdt + TASK_GDT0 +i, 103, (int)&taskctl->tasks0[i].tss, AR_TSS32);
	}
	for(i = 0; i< MAX_TASKLEVELS; i++)
	{
		taskctl->level[i].running = 0;
		taskctl->level[i].now = 0;
	}
	task = task_alloc();
	task->flags = 2; /* 動作中フラグ*/
	task->priority = 2;
	task->level = 0;
	task_add(task);
	task_switchsub();
	load_tr(task->sel);
	task_timer = timer_alloc();
	timer_settime(task_timer, task->priority);
	
	idle = task_alloc();
	idle->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024;
	idle->tss.eip = (int)&task_idle;
	idle->tss.es = 1 * 8;
	idle->tss.cs = 2 * 8;
	idle->tss.ss = 1 * 8;
	idle->tss.ds = 1 * 8;
	idle->tss.fs = 1 * 8;
	idle->tss.gs = 1 * 8;
	task_run(idle, MAX_TASKLEVELS - 1, 1);
	
	return task;
}

struct TASK *task_alloc(void)
{
	int i;
	struct TASK *task;
	for(i = 0; i < MAX_TASKS; i++)
	{
		if(taskctl->tasks0[i].flags == 0)
		{
			task = &taskctl->tasks0[i];
			task->flags = 1;
			task->tss.eflags = 0x00000202;
			task->tss.eax   = 0;
			task->tss.ecx   = 0;
			task->tss.edx   = 0;
			task->tss.ebx   = 0;
			task->tss.ebp   = 0;
			task->tss.esi   = 0;
			task->tss.edi   = 0;
			task->tss.es    = 0;
			task->tss.ds    = 0;
			task->tss.fs    = 0;
			task->tss.gs    = 0;
			task->tss.ldtr  = 0;
			task->tss.iomap = 0x40000000;
			return task;
		}
	}
	
	return 0; /* 全部使用中 */
}

void task_run(struct TASK *task,int level, int priority)
{

	if(level < 0)
	{
		level = task->level;
	}

	if(priority > 0)
	{
		task->priority = priority;
	}
	
	/* 動作中タスクのレベルを変更する */
	if(task->flags == 2 && task->level != level)
	{
		task_remove(task); /* 現在のレベルから外され、動作中タスクから停止中タスクに状態を切り替える*/
	}
	
	/* スリープ中タスクをラウンドロビン対象タスクに入れる */
	if(task->flags != 2)
	{
		task->level = level;
		task_add(task);
	}
	
	taskctl->lv_change = 1; /* 次のタスクスイッチの時にレベルを見直す */
	return;
}

void task_switch(void)
{
	struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
	struct TASK *new_task, *now_task = task_now();
	tl->now++;
	if(tl->now == tl->running)
	{
		tl->now = 0;
	}
	if(taskctl->lv_change != 0)
	{
		task_switchsub();
		tl = &taskctl->level[taskctl->now_lv];
	}
	new_task = task_now();
	timer_settime(task_timer, new_task->priority);
	if(new_task != now_task)
	{
		farjmp(0, new_task->sel);
	}
	
	return;
}

void task_sleep(struct TASK *task)
{
	struct TASK *now_task;
	/* 動作中のタスクの場合 */
	if(task->flags == 2)
	{
		now_task = task_now();
		task_remove(task); /* 動作中タスクから停止中タスクに切り替わる */
		if(task == now_task)
		{
			/* 自分自身のタスクの場合は、次のタスクに切り替える必要がある */
			task_switchsub();
			now_task = task_now(); /* LV切り替え後の現在タスクを取得する */
			farjmp(0, now_task->sel);
		}
		
	}
	return;
}

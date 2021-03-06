#include"bootpack.h"

#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040

#define TIMER_FLAGS_ALLOC 1 /* 確保した状態 */
#define TIMER_FLAGS_USING 2 /* タイマ作動中 */

struct TIMERCTL timerctl;

int timer_cancel(struct TIMER *timer);

void init_pit(void)
{
	struct TIMER *t;
	int i;
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);
	timerctl.count = 0;
	for(i = 0; i < MAX_TIMER; i++)
	{
		timerctl.timers0[i].flags = 0;
	}
	t = timer_alloc();
	t->timeout = 0xffffffff;
	t->flags = TIMER_FLAGS_USING;
	t->next = 0;
	timerctl.t0 = t;
	timerctl.next = 0xffffffff;
	return;
}

void inthandler20(int *esp)
{
	int i;
	struct TIMER *timer;
	char ts = 0;
	
	io_out8(PIC0_OCW2, 0x60);
	timerctl.count++;
	
	if(timerctl.next > timerctl.count)
	{
		return;
	}
	timer = timerctl.t0;
	while(1)
	{
		if(timer->timeout > timerctl.count)
		{
			break;
		}
		/* タイムアウト */
		timer->flags = TIMER_FLAGS_ALLOC;
		if(timer != task_timer)
		{
			fifo32_put(timer->fifo, timer->data);
		}
		else
		{
			ts = 1; /* mt_timerがタイムアウトした */
		}
		timer = timer->next; /* 次のタイマの番地をtimerを代入 */
	}
	timerctl.t0 = timer;
	
	timerctl.next = timerctl.t0->timeout;
	
	if(timerctl.count == 0xffffffff)
	{
		timer_reset();
	}
	
	if(ts != 0)
	{
		task_switch();
	}
	return;
}

void timer_reset(void)
{
	int t0 = timerctl.count;
	int i;
	int eflags;
	eflags = io_load_eflags();
	io_cli();
	timerctl.count -= t0;
	for(i = 0; i < MAX_TIMER; i++)
	{
		if(timerctl.timers0[i].flags == TIMER_FLAGS_USING)
		{
			if(timerctl.timers0[i].timeout != 0xffffffff)
			{
				timerctl.timers0[i].timeout -= t0;
			}
		}
	}
	io_store_eflags(eflags);
}

void timer_settime(struct TIMER *timer, unsigned int timeout)
{
	int e;
	struct TIMER *t, *s;
	timer->timeout = timeout + timerctl.count;
	timer->flags = TIMER_FLAGS_USING;
	e = io_load_eflags();
	io_cli();
	
	t = timerctl.t0;
	if(timer->timeout <= t->timeout)
	{
		timerctl.t0 = timer;
		timer->next = t;
		timerctl.next = timer->timeout;
		io_store_eflags(e);
		return;
	}
	/* どこに入れれば良いかを探す */
	while(1)
	{
		s = t;
		t = t->next;
		if(timer->timeout <= t->timeout)
		{
			s->next = timer;
			timer->next = t;
			io_store_eflags(e);
			return;
		}
	}
	
	return;
}

struct TIMER *timer_alloc(void)
{
	int i;
	for(i = 0; i < MAX_TIMER; i++)
	{
		if(timerctl.timers0[i].flags == 0)
		{
			timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
			timerctl.timers0[i].flags2 = 0;
			return &timerctl.timers0[i];
		}
	}
	
	return 0;
}

void timer_free(struct TIMER *timers0)
{
	timers0->flags  = 0;
	timers0->flags2 = 0;
	return;
}

void timer_init(struct TIMER *timers0, struct FIFO32 *fifo, int data)
{
	timers0->fifo = fifo;
	timers0->data = data;
	return;
}

void timer_cancelall(struct FIFO32 *fifo)
{
	int e, i;
	struct TIMER *t;

	e = io_load_eflags();
	io_cli();
	for(i = 0; i< MAX_TIMER; i++)
	{
		t = &timerctl.timers0[i];
		if(t->flags != 0 && t->flags2 != 0 && t->fifo == fifo)
		{
			timer_cancel(t);
			timer_free(t);
		}
	}
	io_store_eflags(e);
	return;
}

int timer_cancel(struct TIMER *timer)
{
	int e;
	struct TIMER *t;
	
	e = io_load_eflags();
	io_cli(); /* タイマーの割り込みを禁止する */
	if(timer->flags == TIMER_FLAGS_USING)
	{
		/* 先頭の場合 */
		if(timer == timerctl.t0)
		{
			t = timer->next;
			timerctl.t0   = t;
			timerctl.next = t->timeout;
		}
		/* 先頭以外の場合 */
		else
		{
			t = timerctl.t0;
			while(1)
			{
				if(t->next == timer)
				{
					break;
				}
				t = t->next;
			}
			t->next = timer->next; /* timerの直前の次がtimerの次を指すようにする */
		}
		timer->flags = TIMER_FLAGS_ALLOC;
		io_store_eflags(e);
		return 1;
	}
	io_store_eflags(e);
	return 0; /* キャンセル不要 */
}
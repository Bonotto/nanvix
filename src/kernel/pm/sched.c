/*
 * Copyright(C) 2011-2016 Pedro H. Penna   <pedrohenriquepenna@gmail.com>
 *              2015-2016 Davidson Francis <davidsondfgl@hotmail.com>
 *
 * This file is part of Nanvix.
 *
 * Nanvix is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Nanvix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Nanvix. If not, see <http://www.gnu.org/licenses/>.
 */

#include <nanvix/clock.h>
#include <nanvix/const.h>
#include <nanvix/hal.h>
#include <nanvix/pm.h>
#include <signal.h>

/**
 * @brief Schedules a process to execution.
 *
 * @param proc Process to be scheduled.
 */
PUBLIC void sched(struct process *proc)
{
    proc->state = PROC_READY;
    proc->counter = 0;
}

/**
 * @brief Stops the current running process.
 */
PUBLIC void stop(void)
{
    curr_proc->state = PROC_STOPPED;
    sndsig(curr_proc->father, SIGCHLD);
    yield();
}

/**
 * @brief Resumes a process.
 *
 * @param proc Process to be resumed.
 *
 * @note The process must stopped to be resumed.
 */
PUBLIC void resume(struct process *proc)
{
    /* Resume only if process has stopped. */
    if (proc->state == PROC_STOPPED)
        sched(proc);
}

/**
 * @brief Yields the processor.
 */
PUBLIC void yield(void)
{
    struct process *p;    /* Working process.     */
    struct process *next; /* Next process to run. */
    
    /* Re-schedule process for execution. */
    if (curr_proc->state == PROC_RUNNING) {
        
        /**
         * Process used all your quantum, is soon placed
         * a queue with greater quantum.
         */
        if (curr_proc->queue < QUEUE_AMOUNT) {
            curr_proc->queue++;
        }
        
        sched(curr_proc);
        
    } else {
        
        /**
         * Process exited by a system call, then placed
         * a higher priority queue for when he comes
         * back to be serviced soon.
         */
        if (curr_proc->state != PROC_DEAD && curr_proc != IDLE) {
            if (curr_proc->queue > 1) {
                curr_proc->queue--;
            }
        }
    }
    
    /* Remember this process. */
    last_proc = curr_proc;
    
    /* Check alarm. */
    for (p = FIRST_PROC; p <= LAST_PROC; p++)
    {
        /* Skip invalid processes. */
        if (!IS_VALID(p))
            continue;
        
        /* Alarm has expired. */
        if ((p->alarm) && (p->alarm < ticks))
            p->alarm = 0, sndsig(p, SIGALRM);
    }
    
    /* Choose a process to run next. */
    next = IDLE;
    for (p = FIRST_PROC; p <= LAST_PROC; p++)
    {
        
        /* Skip non-ready process. */
        if (p->state != PROC_READY)
            continue;
        
        /**
         * Choose the highest priority queue,
         * if there is more than one process is
         * calculated an "weight" between priority,
         * timeout and nice so chosen with highest weight.
         */
        if (p->queue == next->queue) {
            
            int weight_p = p->priority + p->nice - p->counter;
            int weight_next = next->priority + next->nice - next->counter;
            
            if (weight_p < weight_next) {
                next->counter++;
                next = p;
            } else {
                p->counter++;
            }
            
        } else if (p->queue < next->queue) {
            
            next->counter++;
            next = p;
            
        } else {
            p->counter++;
        }
        
        /**
         * If the process is a long time in a queue,
         * it is placed in the highest priority queue
         * and your counter is reset.
         */
        int aging = ((QUEUE_AMOUNT+1) - p->queue) * AGING_FACTOR;
        
        if (p->counter >= aging && p->queue != 1) {
            p->counter = 0;
            p->queue--;
        }
    }
    
    /* Switch to next process. */
    next->priority = PRIO_USER;
    next->state = PROC_RUNNING;
    next->counter = next->queue * PROC_QUANTUM;
    switch_to(next);
}

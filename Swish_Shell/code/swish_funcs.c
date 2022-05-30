#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "job_list.h"
#include "string_vector.h"
#include "swish_funcs.h"

#define MAX_ARGS 10

int tokenize(char *s, strvec_t *tokens) 
{
    char* word = strtok(s, " ");
    if(word == NULL)
    {
		printf("strtok error in tokenize.");
		return 1;
	}
    while(word != NULL)
    {
       if(strvec_add(tokens, word))
       {
		   printf("strvec_add error in tokenize.");
		   return 1;
	   }
       word = strtok(NULL, " ");
    }
    // Task 0: Tokenize string s
    // Assume each token is separated by a single space (" ")
    // Use the strtok() function to accomplish this
    // Add each token to the 'tokens' parameter (a string vector)
    // Return 0 on success, 1 on error
    return 0;
}

// Task 2: Execute the specified program (token 0) with the
// specified command-line arguments
// THIS FUNCTION SHOULD BE CALLED FROM A CHILD OF THE MAIN SHELL PROCESS
// Hint: Build a string array from the 'tokens' vector and pass this into execvp()
// Another Hint: You have a guarantee of the longest possible needed array, so you
// won't have to use malloc.


// Task 3: Extend this function to perform output redirection before exec()'ing
// Check for '<' (redirect input), '>' (redirect output), '>>' (redirect and append output)
// entries inside of 'tokens' (the strvec_find() function will do this for you)
// Open the necessary file for reading (<), writing (>), or appending (>>)
// Use dup2() to redirect stdin (<), stdout (> or >>)
// DO NOT pass redirection operators and file names to exec()'d program
// E.g., "ls -l > out.txt" should be exec()'d with strings "ls", "-l", NULL

// Task 4: You need to do two items of setup before exec()'ing
// 1. Restore the signal handlers for SIGTTOU and SIGTTIN to their defaults.
// The code in main() within swish.c sets these handlers to the SIG_IGN value.
// Adapt this code to use sigaction() to set the handlers to the SIG_DFL value.


// 2. Change the process group of this process (a child of the main shell).
// Call getpid() to get its process ID then call setpgid() and use this process
// ID as the value for the new process group ID

// Not reachable after a successful exec(), but retain here to keep compiler happy

int run_command(strvec_t *tokens) 
{
    char* argument_list[MAX_ARGS];
    int i = 0;
    int out_redirect = strvec_find(tokens, ">");
    if(out_redirect == 0)
    {
		printf("error with out_redirect\n");
		return 1;
	}
    int append_redirect = strvec_find(tokens, ">>");
    if(append_redirect == 0)
    {
		printf("error with append_redirect\n");
		return 1;
	}
    int in_redirect = strvec_find(tokens, "<");
    if(in_redirect == 0)
    {
		printf("error with append_redirect\n");
		return 1;
	}
    int index = MAX_ARGS;
    if (out_redirect > -1 && out_redirect < index) index = out_redirect;
    if (append_redirect > -1 && append_redirect < index) index = append_redirect;
    if (in_redirect > -1 && in_redirect < index) index = in_redirect;
    for(i= 0; i < index; i++)
    {
        char * t = strvec_get(tokens, i);
        if(t == NULL) 
        {
            break;
        }
        argument_list[i] = t;
    }
    argument_list[i] = NULL;
    // > redirect output
    if(out_redirect > 0)
    {
        char* filename = strvec_get(tokens, out_redirect +1);
        if(filename == NULL)
        {
			printf("filename is NULL\n");
			return 1;
		}
        int f = open(filename, O_WRONLY | O_CREAT | O_TRUNC , S_IWUSR | S_IRUSR);
        if (f == -1) 
        {
            perror("Failed to open input file");
            return 1;
        }
        if(dup2(f, STDOUT_FILENO) == -1)
        {
			perror("dup2 error in run_command first call\n");
		}
        if(close(f) == 1)
		{
			perror("error with close in run_command\n");
		}
    }
    if(append_redirect > 0)
    {
        //redirect to file and append
        char* filename = strvec_get(tokens, append_redirect +1);
        if(filename == NULL)
        {
			printf("filename is NULL\n");
			return 1;
		}
        int f = open(filename, O_WRONLY | O_APPEND | O_CREAT, S_IWUSR | S_IRUSR);
        if (f == -1) 
        {
            perror("Failed to open input file");
            return 1;
        }
        if(dup2(f, STDOUT_FILENO) == -1)
        {
			perror("dup2 error in run_command second call\n");
		}
        if(close(f) == -1)
		{
			perror("error with close in run_command\n");
		}
    }
    // < redirect input
    if( in_redirect > 0)
    {
        char* filename = strvec_get(tokens, in_redirect +1);
        if(filename == NULL)
        {
			printf("filename is NULL\n");
			return 1;
		}
        int f = open(filename, O_RDONLY);
        if (f == -1) 
        {
            perror("Failed to open input file");
            return 1;
        }
       if(dup2(f, STDIN_FILENO) == -1)
       {
		   perror("dup2 error in run_command third call\n");
	   }
	   if(close(f) == -1)
	   {
		   perror("error with close in run_command\n");
	   }
    }
    struct sigaction sac;
    sac.sa_handler = SIG_DFL;
    if (sigfillset(&sac.sa_mask) == -1) {
        perror("sigfillset");
        return 1;
    }
    sac.sa_flags = 0;
    if (sigaction(SIGTTIN, &sac, NULL) == -1 || sigaction(SIGTTOU, &sac, NULL) == -1)
    {
        perror("sigaction");
        return 1;
    }
    pid_t pid = getpid();
    if(setpgid(pid, pid) == -1)
    {
		perror("setpgid error\n");
	}
    if(execvp(argument_list[0], argument_list) == -1)
    {
        perror("exec");
    }
    return 1;
}

//Task 5: Implement the ability to resume stopped jobs in the foreground
// 1. Look up the relevant job information (in a job_t) from the jobs list
//    using the index supplied by the user (in tokens index 1)
//    Feel free to use sscanf() or atoi() to convert this string to an int
// 2. Call tcsetpgrp(STDIN_FILENO, <job_pid>) where job_pid is the job's process ID
// 3. Send the process the SIGCONT signal with the kill() system call
// 4. Use the same waitpid() logic as in main -- dont' forget WUNTRACED
// 5. If the job has terminated (not stopped), remove it from the 'jobs' list
// 6. Call tcsetpgrp(STDIN_FILENO, <shell_pid>). shell_pid is the *current*
//    process's pid, since we call this function from the main shell process

//Task 6: Implement the ability to resume stopped jobs in the background.
// This really just means omitting some of the steps used to resume a job in the foreground:
// 1. DO NOT call tcsetpgrp() to manipulate foreground/background terminal process group
// 2. DO NOT call waitpid() to wait on the job
// 3. Make sure to modify the 'status' field of the relevant job list entry to JOB_BACKGROUND
//    (as it was JOB_STOPPED before this)
    
int resume_job(strvec_t *tokens, job_list_t *jobs, int is_foreground) 
{
	int job_id;
	int status = -1;
	if(sscanf((strvec_get(tokens, 1)), "%d", &job_id) != 1)
	{
		perror("error in sscanf in resume_job\n");
	}
	job_t *job_to_resume = job_list_get(jobs, job_id);
	if(job_to_resume == NULL)
	{
		fprintf(stderr, "Job index out of bounds\n");
		return 1;
	}
	if(is_foreground == 0)//background
	{
		if(kill(job_to_resume->pid, SIGCONT) == -1)
		{
			perror("error with kill() in resume_job\n");
		}
		job_to_resume->status = JOB_BACKGROUND;
		return 0;
	}
	if(tcsetpgrp(STDIN_FILENO, job_to_resume->pid) == -1)
	{
		perror("error with first tcsetpgrp in resume_job\n");
	}
    if(kill(job_to_resume->pid, SIGCONT) == -1)
    {
		perror("error with kill() in resume_job\n");
	}
    if(waitpid(job_to_resume->pid, &status,WUNTRACED) == -1)//parent waits
    {
		printf("waitpid error in resume_job\n");
		return 1;
	}
	if(WIFEXITED(status) || WIFSIGNALED(status))
	{
		if(job_list_remove(jobs, job_id) == 1)
		{
			printf("job_list_remove error in resume_job");
			return 1;
		}
	}
	if(tcsetpgrp(STDIN_FILENO, getpid()) == -1)
	{
		perror("error with second tcsetpgrp in resume_job\n");
	}
    return 0;
}

//check if the job is even a background job.
//Task 6: Wait for a specific job to stop or terminate
// 1. Look up the relevant job information (in a job_t) from the jobs list
//    using the index supplied by the user (in tokens index 1)
// 2. Make sure the job's status is JOB_BACKGROUND (no sense waiting for a stopped job)
// 3. Use waitpid() to wait for the job to terminate, as you have in resume_job() and main().
// 4. If the process terminates (is not stopped by a signal) remove it from the jobs list

int await_background_job(strvec_t *tokens, job_list_t *jobs) 
{
	int jobs_id;
	int status = -1;
	if(sscanf((strvec_get(tokens, 1)), "%d", &jobs_id) != 1)
	{
		perror("error in sscanf in await_background_job\n");
	}
	job_t *job_in_background = job_list_get(jobs, jobs_id);
	if(job_in_background == NULL)
	{
		fprintf(stderr, "Job index out of bounds\n");
	}
	if(job_in_background->status == JOB_STOPPED)
	{
		printf("Job index is for stopped process not background process\n");
		return 1;
	}
	if(waitpid(job_in_background->pid, &status,WUNTRACED) == -1)
	{
		printf("waitpid error in await_background_job\n");
		return 1;
	}
	if(WIFSTOPPED(status))
	{
		job_in_background->status = JOB_STOPPED;
	}
	if(WIFEXITED(status))
	{
		if(job_list_remove(jobs, jobs_id) == 1)
		{
			perror("job_list_remove error in await_background_job\n");
		}
		
	}		
    return 0;
}

//Task 6: Wait for all background jobs to stop or terminate
// 1. Iterate throught the jobs list, ignoring any stopped jobs
// 2. For a background job, call waitpid() with WUNTRACED.
// 3. If the job has stopped (check with WIFSTOPPED), change its
//    status to JOB_STOPPED. If the job has terminated, do nothing until the
//    next step (don't attempt to remove it while iterating through the list).
// 4. Remove all background jobs (which have all just terminated) from jobs list.
//    Use the job_list_remove_by_status() function.


int await_all_background_jobs(job_list_t *jobs) 
{
	int jobs_id = 0;
	int status = -1;
	job_t *job_in_background = job_list_get(jobs, jobs_id);
	//if(job_in_background == NULL)
	//{
		//printf("job_in_background is NULL\n");
		//return 1;
	//}
	if(job_in_background != NULL)
	{
		if(waitpid(job_in_background->pid, &status,WUNTRACED) == -1)
		{
			printf("waitpid error in await_background_job\n");
			return 1;
		}
		if(WIFSTOPPED(status))
		{
			job_in_background->status = JOB_STOPPED;
		}
		job_in_background = job_in_background->next;
	}
	job_list_remove_by_status(jobs, JOB_BACKGROUND);
    return 0;
}

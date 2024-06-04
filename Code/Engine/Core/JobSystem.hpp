#pragma once
#include <deque>
#include <mutex>
#include <vector>
#include <atomic>
#include <thread>

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Job
{
	friend class JobWorkerThread;
public:
	Job(int jobType);
	virtual ~Job() {}
	virtual void Execute() = 0;
	virtual void OnFinished() = 0;

	std::atomic<int> m_jobType = -1;
	int m_executionId = -1;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class JobSystem;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
constexpr int MULTIPURPOSE_THREAD = ~0;
constexpr int DEFAULT_JOB_ID = MULTIPURPOSE_THREAD;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class JobWorkerThread
{
public:
	JobWorkerThread(JobSystem* jobsystem, int workerThreadID);
	~JobWorkerThread();

	void JobWorkerMain();

	std::thread*	  m_thread = nullptr;
	int				  m_threadID;
	std::atomic<bool> m_isQuitting;
	JobSystem*		  m_JobSystem = nullptr;
	int				  m_threadJobType = MULTIPURPOSE_THREAD;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct JobSystemConfig
{
	int	m_numWorkerThreads = 0;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class JobSystem
{
public:
	JobSystem(JobSystemConfig config);
	~JobSystem();
	void Startup();
	void BeginFrame();
	void EndFrame();
	void ShutDown();

	Job* RetrieveJobToExecute(int threadJobType);
	Job* RetrieveCompletedJobs();
	void MoveToCompletedJobs(Job* job);
	void MoveToExecutingJobs(Job* job);
	void RemoveFromExecutingJobs(Job* job);
	void QueueJob(Job* jobToQueue);
	void SetThreadJobType(int threadId, int jobType);
	void ClearQueuedJobs();
	void ClearCompletedJobs();
	bool CheckQueuedJobsOfType(int JobType);
	bool CheckExecutingJobsOfType(int JobType);
	bool CheckCompletedJobsOfType(int JobType);
	int GetJobCountByType(int jobType);
	int GetNumThreads() const { return m_config.m_numWorkerThreads; }
	void WaitUntilCurrentJobsCompletion();
	void WaitUntilQueuedJobsCompletion();
	JobSystemConfig GetConfig();
	int GetNumCompletedJobs();

	JobSystemConfig				  m_config;
	std::vector<JobWorkerThread*> m_workerThreads;
	std::deque<Job*>			  m_jobsQueued;
	std::mutex					  m_jobsQueuedMutex;
	std::deque<Job*>			  m_completedJobs;
	std::mutex					  m_completedJobsMutex;
	std::vector<Job*>			  m_executingJobs;
	std::mutex					  m_executingJobsMutex;
	std::condition_variable		  m_jobAvailableCondition;
	std::atomic<int> m_amountOfExecutingJobs = 0; // Keeps track of current running jobs without having to use mutex + for loop for checking
	std::atomic<int> m_amountOfQueuedJobs = 0; // Keeps track of current running jobs without having to use mutex + for loop for checking
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
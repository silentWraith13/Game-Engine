#include "Engine/Core/JobSystem.hpp"
#include <chrono>

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Job::Job(int jobType)
{
	m_jobType = jobType;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------
JobWorkerThread::JobWorkerThread(JobSystem* jobsystem, int workerThreadID)
	:m_JobSystem(jobsystem), m_threadID(workerThreadID)
{
	m_thread = new std::thread(&JobWorkerThread::JobWorkerMain, this);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
JobWorkerThread::~JobWorkerThread()
{
	m_isQuitting = true;
	m_thread->join();
	delete m_thread;
	m_thread = nullptr;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void JobWorkerThread::JobWorkerMain()
{
	while (!m_isQuitting)
	{
		Job* jobToExecute = m_JobSystem->RetrieveJobToExecute(m_threadJobType);
		if (jobToExecute != nullptr)
		{
			m_JobSystem->MoveToExecutingJobs(jobToExecute);
			jobToExecute->Execute();
			jobToExecute->OnFinished();
			m_JobSystem->RemoveFromExecutingJobs(jobToExecute);
			m_JobSystem->MoveToCompletedJobs(jobToExecute);
		}
		else
		{
			std::this_thread::yield();
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------
JobSystem::JobSystem(JobSystemConfig config)
{
	m_config = config;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
JobSystem::~JobSystem()
{

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void JobSystem::Startup()
{
	m_workerThreads.reserve(m_config.m_numWorkerThreads);
	for (int i = 0; i < m_config.m_numWorkerThreads; ++i)
	{
		JobWorkerThread* newWorkerThread = new JobWorkerThread(this, i);
		m_workerThreads.push_back(newWorkerThread);

		SetThreadJobType(i, newWorkerThread->m_threadJobType);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void JobSystem::ShutDown()
{
	WaitUntilQueuedJobsCompletion();

	for (int i = 0; i < (int)m_workerThreads.size(); i++)
	{
		if (m_workerThreads[i])
		{
			delete m_workerThreads[i];
			m_workerThreads[i] = nullptr;
		}
	}
	m_workerThreads.clear();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Job* JobSystem::RetrieveJobToExecute(int threadJobType)
{
	Job* queuedJob = nullptr;
	m_jobsQueuedMutex.lock(); // lock

	if (!m_jobsQueued.empty()) 
	{
		bool foundJob = false;
		std::deque<Job*>::iterator foundJobIt;
		for (std::deque<Job*>::iterator dequeIt = m_jobsQueued.begin(); dequeIt != m_jobsQueued.end() && !foundJob; dequeIt++)
		{
			if (*dequeIt) 
			{
				Job* job = *dequeIt;
				int jobType = job->m_jobType;
				if ((jobType & threadJobType) != 0)
				{
					queuedJob = job;
					foundJob = true;
					foundJobIt = dequeIt;
				}
			}
		}
		//queuedJob = m_queuedJobs.front();
		if (foundJob) {
			m_jobsQueued.erase(foundJobIt);
		}
	}

	m_jobsQueuedMutex.unlock(); // unlock


	if (queuedJob)
	{
		m_amountOfQueuedJobs--;
		int executionId = -1;

		m_executingJobsMutex.lock(); // lock

		for (executionId = 0; executionId < m_executingJobs.size(); executionId++)
		{
			Job* job = m_executingJobs[executionId];
			if (!job) break;
		}
		if (executionId != -1) 
		{
			if (executionId >= m_executingJobs.size()) 
			{
				m_executingJobs.push_back(queuedJob);
			}
			else 
			{
				m_executingJobs[executionId] = queuedJob;
			}
		}
		else 
		{
			executionId = (int)m_executingJobs.size();
			m_executingJobs.push_back(queuedJob);
		}

		m_executingJobsMutex.unlock(); // unlock

		queuedJob->m_executionId = executionId;
		m_amountOfExecutingJobs++;

	}

	return queuedJob;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Job* JobSystem::RetrieveCompletedJobs()
{
	m_completedJobsMutex.lock();
	Job* completedJob = nullptr;
	if (!m_completedJobs.empty())
	{
		completedJob = m_completedJobs.front();
		m_completedJobs.pop_front();
	}
	m_completedJobsMutex.unlock();
	return completedJob;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void JobSystem::MoveToCompletedJobs(Job* job)
{
	if (job->m_executionId < 0) return;
	m_executingJobsMutex.lock();
	if (job->m_executionId < m_executingJobs.size())
	{
		m_executingJobs[job->m_executionId] = nullptr;
	}
	m_executingJobsMutex.unlock();

	m_completedJobsMutex.lock();
	m_completedJobs.push_back(job);
	m_completedJobsMutex.unlock();
	m_amountOfExecutingJobs--;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void JobSystem::MoveToExecutingJobs(Job* job)
{
	m_executingJobsMutex.lock();
	for (int i = 0; i < (int)m_executingJobs.size(); i++)
	{
		if (m_executingJobs[i] == nullptr)
		{
			m_executingJobs[i] = job;
			m_executingJobsMutex.unlock();
			return;
		}
	}
	m_executingJobs.push_back(job);
	m_executingJobsMutex.unlock();
	return;

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void JobSystem::RemoveFromExecutingJobs(Job* job)
{
	m_executingJobsMutex.lock();
	for (int i = 0; i < (int)m_executingJobs.size(); i++)
	{
		if (m_executingJobs[i] == job)
		{
			m_executingJobs[i] = nullptr;
		}
	}
	m_executingJobsMutex.unlock();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void JobSystem::QueueJob(Job* jobToQueue)
{
	m_jobsQueuedMutex.lock();
	m_jobsQueued.push_back(jobToQueue);
	m_amountOfQueuedJobs++;
	m_jobsQueuedMutex.unlock();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void JobSystem::BeginFrame()
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void JobSystem::EndFrame()
{
	WaitUntilCurrentJobsCompletion();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool JobSystem::CheckQueuedJobsOfType(int JobType)
{
	m_jobsQueuedMutex.lock();
	for (const auto& jobIterator : m_jobsQueued)
	{
		if (jobIterator->m_jobType == JobType)
		{
			m_jobsQueuedMutex.unlock();
			return true;
		}
	}
	m_jobsQueuedMutex.unlock();
	return false;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool JobSystem::CheckExecutingJobsOfType(int JobType)
{
	m_executingJobsMutex.lock();
	for (const auto& jobIterator : m_executingJobs)
	{
		if (jobIterator != nullptr && jobIterator->m_jobType == JobType)
		{
			m_executingJobsMutex.unlock();
			return true;
		}
	}
	m_executingJobsMutex.unlock();
	return false;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool JobSystem::CheckCompletedJobsOfType(int JobType)
{
	m_completedJobsMutex.lock();
	for (const auto& jobIterator : m_completedJobs)
	{
		if (jobIterator->m_jobType == JobType)
		{
			m_completedJobsMutex.unlock();
			return true;
		}
	}
	m_completedJobsMutex.unlock();
	return false;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
int JobSystem::GetJobCountByType(int jobType)
{
	int count = 0;

	m_jobsQueuedMutex.lock();
	for (const auto& job : m_jobsQueued)
	{
		if (job->m_jobType == jobType)
			count++;
	}
	m_jobsQueuedMutex.unlock();

	return count;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
JobSystemConfig JobSystem::GetConfig()
{
	return JobSystemConfig();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
int JobSystem::GetNumCompletedJobs()
{
	m_completedJobsMutex.lock();
	int numCompletedJobs = (int)m_completedJobs.size();
	m_completedJobsMutex.unlock();
	return numCompletedJobs;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void JobSystem::SetThreadJobType(int threadId, int jobType)
{
	if (threadId < 0 || threadId > m_workerThreads.size()) return;
	m_workerThreads[threadId]->m_threadJobType = jobType;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void JobSystem::ClearQueuedJobs()
{
	m_amountOfQueuedJobs = 0;
	m_jobsQueuedMutex.lock();
	m_jobsQueued.clear();
	m_jobsQueuedMutex.unlock();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void JobSystem::ClearCompletedJobs()
{
	m_completedJobsMutex.lock();
	m_completedJobs.clear();
	m_completedJobsMutex.unlock();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void JobSystem::WaitUntilQueuedJobsCompletion()
{
	bool keepRunning = true;
	while (keepRunning) 
	{
		keepRunning = (m_amountOfExecutingJobs != 0) || (m_amountOfQueuedJobs != 0);

		if (keepRunning)
		{
			std::this_thread::yield();
		}
	} 
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void JobSystem::WaitUntilCurrentJobsCompletion()
{
	bool keepRunning = true;
	while (keepRunning)
	{
		keepRunning = (m_amountOfExecutingJobs != 0);
		if (keepRunning)
		{
			std::this_thread::yield();
		}

	} // Do nothing while there are running jobs
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
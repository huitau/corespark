#include "../include/tjcore.h"
using namespace tj::shared;

volatile long Thread::_count = 0;
std::map<int, String> Thread::_names;
CriticalSection Thread::_nameLock;

#ifdef TJ_OS_WIN
	namespace tj {
		namespace shared {
			DWORD WINAPI ThreadProc(LPVOID lpParam) {
				try {
					InterlockedIncrement(&Thread::_count);
					Thread* tr = (Thread*)lpParam;
					if(tr!=0) {
						srand(GetTickCount());
						tr->Run();
					}
				}
				catch(...) {
				}
				
				InterlockedDecrement(&Thread::_count);
				return 0;
			}
		}
	}
#endif

/* Thread */
Thread::Thread() {
	_thread = CreateThread(NULL, 4096, ThreadProc, (LPVOID)this, CREATE_SUSPENDED|STACK_SIZE_PARAM_IS_A_RESERVATION, (LPDWORD)&_id);
	_started = false;
}

Thread::~Thread() {
	CloseHandle(_thread);
}

long Thread::GetThreadCount() {
	return _count;
}

void Thread::SetName(const String& t) {
	ThreadLock lock(&_nameLock);
	_names[_id] = t;
}

int Thread::GetCurrentThreadID() {
	#ifdef _WIN32
		return (unsigned int)(::GetCurrentThreadId());
	#else
		#error Not implemented
	#endif
}

String Thread::GetCurrentThreadName() {
	int tid = GetCurrentThreadID();

	ThreadLock lock(&_nameLock);
	std::map<int, String>::const_iterator it = _names.find(tid);
	if(it!=_names.end()) {
		return it->second;
	}
	return L"";
}

void Thread::SetPriority(Priority p) {
	// Convert priority to Win32 priority code
	int prio = THREAD_PRIORITY_NORMAL;
	switch(p) {
		case PriorityAboveNormal:
		case PriorityHigh:
			prio = THREAD_PRIORITY_ABOVE_NORMAL;
			break;

		case PriorityIdle:
			prio = THREAD_PRIORITY_IDLE;
			break;

		case PriorityBelowNormal:
		case PriorityLow:
			prio = THREAD_PRIORITY_BELOW_NORMAL;
			break;

		case PriorityTimeCritical:
			prio = THREAD_PRIORITY_TIME_CRITICAL;
			break;

		default:
		case PriorityNormal:
			prio = THREAD_PRIORITY_NORMAL;
	}

	SetThreadPriority(_thread, prio);
}

void Thread::Start() {
	DWORD code = -1;
	GetExitCodeThread(_thread, &code);

	if(code==STILL_ACTIVE) {
		// we're running...
		_started = true;
		ResumeThread(_thread);
	}
	else {
		// finished correctly, run again
		CloseHandle(_thread);
		_thread = CreateThread(NULL, 512, ThreadProc, (LPVOID)this, CREATE_SUSPENDED, (LPDWORD)&_id);
		_started = true;
		ResumeThread(_thread);
	}
}

void Thread::WaitForCompletion() {
	if(!_started) {
		// don't wait if the thread was never started
		return;
	}
	
	if(GetCurrentThread()==_thread) {
		return; // Cannot wait on yourself
	}

	WaitForSingleObject(_thread,INFINITE);
}

void Thread::Terminate() {
	TerminateThread(_thread, 0);
}

int Thread::GetID() const {
	return _id;
}

void Thread::Run() {
}

/* Semaphore; Windows implementation */
#ifdef _WIN32
	Semaphore::Semaphore() {
		_sema = CreateSemaphore(NULL, 0, LONG_MAX, 0);
	}

	Semaphore::~Semaphore() {
		CloseHandle(_sema);
	}

	void Semaphore::Release(int n) {
		ReleaseSemaphore(_sema, n, NULL);
	}

	bool Semaphore::Wait() {
		return WaitForSingleObject(_sema, INFINITE) == WAIT_OBJECT_0;
	}
#endif

/* Event; Windows implementation */
#ifdef _WIN32
	Event::Event() {
		_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	Event::~Event() {
		CloseHandle(_event);
	}

	void Event::Signal() {
		SetEvent(_event);
	}

	void Event::Pulse() {
		PulseEvent(_event);
	}

	void Event::Reset() {
		ResetEvent(_event);
	}

	void Event::Wait(int ms) {
		WaitForSingleObject(_event, ms);
	}
#endif

/* ThreadLocal; Windows implementation uses TLS */
#ifdef _WIN32
	ThreadLocal::ThreadLocal() {
		_tls = TlsAlloc();
		if(_tls==TLS_OUT_OF_INDEXES) {
			Throw(L"Cannot create thread-local storage", ExceptionTypeSevere);
		}
	}

	ThreadLocal::~ThreadLocal() {
		TlsFree(_tls);
	}

	int ThreadLocal::GetValue() const {
		return (int)(__int64)TlsGetValue(_tls);
	}

	void ThreadLocal::SetValue(int v) {
		TlsSetValue(_tls, (void*)(__int64)v);
	}
#endif

ThreadLocal::operator int() const {
	return GetValue();
}

void ThreadLocal::operator=(int r) {
	SetValue(r);
}

/** Wait **/
void Wait::For(Thread& t, const Time& out) {
	For(t._thread, out);
}

void Wait::For(Event& e, const Time& out) {
	For(e._event, out);
}

void Wait::For(Semaphore& sm, const Time& out) {
	For(sm._sema, out);
}

Wait::Wait() {
}

Wait::~Wait() {
}

void Wait::Add(Thread& t) {
	_handles.push_back(t._thread);
}

void Wait::Add(Event& evt) {
	_handles.push_back(evt._event);
}

void Wait::Add(PeriodicTimer& h) {
	_handles.push_back(h._timer);
}

void Wait::Add(Semaphore& sm) {
	_handles.push_back(sm._sema);
}

bool Wait::ForAll(const Time& out) {
	unsigned int n = (unsigned int)_handles.size();
	HANDLE* handles = new HANDLE[n];
	std::vector<HANDLE>::iterator it = _handles.begin();
	for(unsigned int a=0;a<n;a++) {
		handles[a] = *it;
		++it;
	}

	bool r = For(handles, n, true, out) >= 0;	
	delete[] handles;
	return r;
}

int Wait::ForAny(const Time& out) {
	unsigned int n = (unsigned int)_handles.size();
	HANDLE* handles = new HANDLE[n];
	std::vector<HANDLE>::iterator it = _handles.begin();
	for(unsigned int a=0;a<n;a++) {
		HANDLE h = *it;
		handles[a] = h;
		++it;
	}

	int r = For(&(handles[0]), n, false, out);
	delete[] handles;
	return r;
}

#ifdef WIN32
	void Wait::For(HANDLE handle, const Time& out) {
		DWORD tms = out.ToInt();
		if(tms<1) {
			tms = INFINITE;
		}
		
		switch(WaitForSingleObject(handle, tms)) {
			case WAIT_FAILED:
				Throw(L"Wait for single thread failed; invalid thread handle?", ExceptionTypeError);
				break;

			case WAIT_ABANDONED:
				Throw(L"Wait for single thread abandoned; this is bad.", ExceptionTypeError);
				break;

			default:
				break;
		}
	}

	int Wait::For(HANDLE* handles, unsigned int n, bool all, const Time& out) {
		DWORD timeOutMs = out.ToInt();
		if(timeOutMs<1) {
			timeOutMs = INFINITE;
		}

		int r = WaitForMultipleObjects(n, handles, all, timeOutMs);
		if(r==WAIT_ABANDONED) {
			Throw(L"Wait was abandoned; probably, a thread or event was destroyed while another thread was waiting on it through this waiting operation", ExceptionTypeError);
		}
		else if(r==WAIT_FAILED) {
			Throw(L"Wait failed; probably, an invalid handle or event was passed to this wait operation", ExceptionTypeError);
		}
		else if(r==WAIT_TIMEOUT) {
			return -1;
		}
		else {
			return r - WAIT_OBJECT_0;
		}
	}
#else
	#error Not implemented
#endif

/** PeriodicTimer **/
PeriodicTimer::PeriodicTimer() {
	#ifdef WIN32
		_timer = CreateWaitableTimer(NULL,FALSE,NULL);
	#else
		#error Not implemented
	#endif
}

PeriodicTimer::~PeriodicTimer() {
	#ifdef WIN32
		CloseHandle(_timer);
	#else
		#error Not implemented
	#endif
}

void PeriodicTimer::Start(const Time& period) {
	LARGE_INTEGER dueTime;
	dueTime.QuadPart = 0;
	SetWaitableTimer(_timer, &dueTime, period.ToInt(), 0, 0, 0);
}

/** CriticalSection **/
CriticalSection::CriticalSection() {
	#ifdef _WIN32
		InitializeCriticalSectionAndSpinCount(&_cs, 1024);
	#endif
}

CriticalSection::~CriticalSection() {
	#ifdef _WIN32
		DeleteCriticalSection(&_cs);
	#endif
}

void CriticalSection::Enter() {
	#ifdef _WIN32
		EnterCriticalSection(&_cs);
	#else
		#error CriticalSection::Enter not implemented on this platform
	#endif
}

void CriticalSection::Leave() {
	#ifdef _WIN32
		LeaveCriticalSection(&_cs);
	#else
		#error CriticalSection::Enter not implemented on this platform
	#endif
}

/** ThreadLock **/
ThreadLock::ThreadLock(CriticalSection *cs): _cs(cs) {
	_cs->Enter();
}

ThreadLock::ThreadLock(strong<Lockable> lockable): _cs(&(lockable->_lock)) {
	_cs->Enter();
}

ThreadLock::ThreadLock(Lockable* lockable): _cs(&(lockable->_lock)) {
	_cs->Enter();
}


ThreadLock::~ThreadLock() {
	_cs->Leave();
}

/** Lockable **/
Lockable::Lockable() {
}

Lockable::~Lockable() {
}
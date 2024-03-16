#pragma once
template<typename T>
concept TimerType = std::convertible_to<T, int> && requires (T t)
{
	{ t() }->std::convertible_to<long>;
	{ t }->std::convertible_to<int>;
};

struct FrameTimer
{
	long operator()()const { return Unsorted::CurrentFrame; }
	operator long() const { return Unsorted::CurrentFrame; }
};

struct SystemTimer
{
	long operator()()const { return timeGetTime() >> 4; }
	operator long() const { return timeGetTime() >> 4; }
};

template<TimerType Clock>
struct TimerStruct
{
	int StartTime;
	Clock CurrentTime;
	int TimeLeft;

	constexpr TimerStruct() :StartTime { -1 }, TimeLeft { 0 } { };
	TimerStruct(noinit_t()){ }
	explicit TimerStruct(int duration) { this->Start(duration); }

	void Start(int duration)
	{
		this->StartTime = this->CurrentTime;
		this->TimeLeft = duration;
	}

	void Stop()
	{
		this->StartTime = -1;
		this->TimeLeft = 0;
	}

	void Pause()
	{
		if (this->IsTicking())
		{
			this->TimeLeft = this->GetTimeLeft();
			this->StartTime = -1;
		}
	}

	void Resume()
	{
		if (!this->IsTicking())
		{
			this->StartTime = this->CurrentTime;
		}
	}

	int GetTimeLeft() const
	{
		if (!this->IsTicking())
		{
			return this->TimeLeft;
		}

		auto passed = this->CurrentTime - this->StartTime;
		auto left = this->TimeLeft - passed;

		return (left <= 0) ? 0 : left;
	}

	// returns whether a ticking timer has finished counting down.
	bool Completed() const
	{
		return this->IsTicking() && !this->HasTimeLeft();
	}

	// Returns whether a delay is active or a timer is still counting down.
	// this is the 'opposite' of Completed() (meaning: incomplete / still busy)
	// and logically the same as !Expired() (meaning: blocked / delay in progress)
	bool InProgress() const
	{
		return this->IsTicking() && this->HasTimeLeft();
	}

	// Returns whether a delay is inactive. same as !InProgress().
	bool Expired() const
	{
		return !this->IsTicking() || !this->HasTimeLeft();
	}

	// Sometimes I want to know if the timer has ever started
	bool HasStarted() const
	{
		return this->IsTicking() || this->HasTimeLeft();
	}

	// Returns whether or not the timer is currently ticking (started but not stopped or paused),
	// regardless of if there is time left or not.
	bool IsTicking() const
	{
		return this->StartTime != -1;
	}

	// Returns whether or not the timer has time left.
	bool HasTimeLeft() const
	{
		return this->GetTimeLeft() > 0;
	}
};

// Timer that counts down from specified value towards zero, counted in frames.
using CDTimerClass = TimerStruct<FrameTimer>;
using SysTimerClass = TimerStruct<SystemTimer>;

static_assert(offsetof(CDTimerClass, TimeLeft) == 0x8);
static_assert(sizeof(SysTimerClass) == 0xC);

// Timer that counts down towards zero at specified rate, counted in frames.
class RateTimer : public CDTimerClass
{
public:
	int Rate { 0 };

	constexpr RateTimer() = default;
	RateTimer(int rate) { RateTimer::Start(rate); }

	void Start(int rate)
	{
		this->Rate = rate;
		this->CDTimerClass::Start(rate);
	}
};

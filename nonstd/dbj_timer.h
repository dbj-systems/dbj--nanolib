#ifndef DBJ_TIMER_INC_
#define DBJ_TIMER_INC_

#include <array>
#include <chrono>
#include <atomic>

namespace dbj::nanolib {

/*
Usage:

stopwatch::precise stopwatch{};

Note: all timings are integer types, no decimals

auto nanos_  = stopwatch.elapsed<unsigned int, std::chrono::nanoseconds>();
auto millis_ = stopwatch.elapsed<unsigned int, std::chrono::milliseconds>();
auto micros_ = stopwatch.elapsed<unsigned int, std::chrono::microseconds>();
auto secos_  = stopwatch.elapsed<unsigned int, std::chrono::seconds>();
*/
// https://wandbox.org/permlink/BHVUDSoZn1Cm8yzo
namespace stopwatch
{
	template <typename CLOCK = std::chrono::high_resolution_clock>
	class engine
	{
		const typename CLOCK::time_point start_point{};

	public:
		engine() : start_point(CLOCK::now())
		{
		}

		template <
			typename REP = typename CLOCK::duration::rep,
			typename UNITS = typename CLOCK::duration>
			REP elapsed() const
		{
			std::atomic_thread_fence(std::memory_order_relaxed);
			auto elapsed_ =
				std::chrono::duration_cast<UNITS>(CLOCK::now() - start_point).count();
			std::atomic_thread_fence(std::memory_order_relaxed);
			return static_cast<REP>(elapsed_);
		}
	};

	using precise = engine<>;
	using system = engine<std::chrono::system_clock>;
	using monotonic = engine<std::chrono::steady_clock>;
} // namespace stopwatch

/*
	times as decimals
*/
struct timer final
{
	using buffer = std::array<char, 24>;
	using CLOCK = typename std::chrono::high_resolution_clock;
	using timepoint = typename CLOCK::time_point;
	const timepoint start_ = CLOCK::now();

	double nano() const
	{
		std::atomic_thread_fence(std::memory_order_relaxed);
		double rez_ = static_cast<double>((CLOCK::now() - start_).count());
		std::atomic_thread_fence(std::memory_order_relaxed);
		return rez_;
	}

	double micro() const { return nano() / 1000.0; }
	double milli() const { return micro() / 1000.0; }
	double seconds() const { return milli() / 1000.0; }
	// double decimal3(double arg) { return (std::round(arg * 1000)) / 1000; }

	enum class kind
	{
		nano,
		micro,
		milli,
		second
	};

	/*
	   timer timer_{} ;

	   auto microsecs_ = as_buffer( timer_ );
	   auto secs_ = as_buffer( timer_, timer::kind::second );
	*/
	friend buffer as_buffer(timer const& timer_, kind which_ = kind::milli)
	{
		buffer retval{ char{0} };
		double arg{};
		char const* unit_{};
		switch (which_)
		{
		case kind::nano:
			arg = timer_.nano();
			unit_ = " nano seconds ";
			break;
		case kind::micro:
			arg = timer_.micro();
			unit_ = " micro seconds ";
			break;
		case kind::milli:
			arg = timer_.milli();
			unit_ = " milli seconds ";
			break;
		default: //seconds
			arg = timer_.seconds();
			unit_ = " seconds ";
		}
		std::snprintf(retval.data(), retval.size(), "%.3f%s", arg, unit_);
		return retval;
	}
}; // timer

} // namespace dbj::nanolib 

#endif // DBJ_TIMER_INC_


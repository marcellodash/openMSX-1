#ifndef THREAD_HH
#define THREAD_HH

namespace openmsx {
namespace Thread {

	// For debugging only
	/** Store ID of the main thread, should be called exactly once from
	  * the main thread.
	  */
	void setMainThread();

	/** Returns true when called from the main thread.
	  */
	bool isMainThread();

} // namespace Thread
} // namespace openmsx

#endif

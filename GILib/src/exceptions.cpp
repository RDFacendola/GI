#include "..\include\exceptions.h"

#ifdef _WIN32

#pragma comment(lib, "StackWalker")

#include <StackWalker.h>	// windows.h will pollute everything with its macros...

#include <sstream>

#undef GetMessage

#endif

using namespace std;
using namespace gi_lib;

namespace{

#ifdef _WIN32

	/// \brief Wrapper around the StackWalker class
	class StackTrace : public StackWalker{

	public:

		/// \brief Default constructor.
		StackTrace() : StackWalker(StackWalkOptions::RetrieveNone) {}

		/// \brief Get the current stack trace.
		/// \return Returns the current stack trace as a string.
		wstring GetStackTrace();

	protected:

		/// \brief Callback used to log each stack entry.
		/// \param eType Type of the entry.
		/// \param entry Details about the entry.
		virtual void OnCallstackEntry(CallstackEntryType eType, CallstackEntry &entry) override;

	private:

		stringstream trace_;	///< \brief Trace dump.

	};

	/////////////////////////// STACK TRACE ////////////////////////

	wstring StackTrace::GetStackTrace(){

		trace_.str("");

		ShowCallstack();

		auto trace = trace_.str();

		return wstring(trace.begin(), trace.end());

	}

	void StackTrace::OnCallstackEntry(CallstackEntryType eType, CallstackEntry &entry){

		StackWalker::OnCallstackEntry(eType, entry);

		if (entry.offset != 0){

			trace_ << entry.moduleName
				   << " - "
				   << entry.undFullName
				   << " ("
				   << std::to_string(entry.lineNumber)
				   << ")"
				   << std::endl;

		}

	}

#endif

}

///////////////////// EXCEPTION //////////////////////

Exception::Exception(const wstring& error, const wstring& location):
error_(error),
location_(location){

#ifdef _WIN32

	StackTrace st;

	stack_trace_ = st.GetStackTrace();

#else

	static_assert(false, "Find the stack trace somehow!");

#endif

}

Exception::Exception(const Exception& other) :
error_(other.error_),
location_(other.location_),
stack_trace_(other.stack_trace_){}

Exception::Exception(Exception&& other):
error_(std::move(other.error_)),
location_(std::move(other.location_)),
stack_trace_(std::move(other.stack_trace_)){}

Exception& Exception::operator=(Exception other){

	other.Swap(*this);

	return *this;

}

const wstring& Exception::GetError() const{

	return error_;
}

const wstring& Exception::GetLocation() const{

	return location_;

}

const wstring& Exception::GetStackTrace() const{

	return stack_trace_;

}

void Exception::Swap(Exception& other){

	std::swap(error_, other.error_);
	std::swap(location_, other.location_);
	std::swap(stack_trace_, other.stack_trace_);

}
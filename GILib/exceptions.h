#pragma once

#include <string>
#include <iomanip>
#include <string>
#include <sstream>

#include "StackWalker.h"

using ::std::wstring;
using ::std::stringstream;

///Used to automatically throw whenever a HRESULT function fails
#define THROW_ON_FAIL(expr) do \
							{ \
								HRESULT hr = expr; \
								if(FAILED(hr)) \
								{ \
									std::wstringstream stream; \
									stream << "FAILED! " << #expr << " (0x" << std::hex << hr << std::dec << ")" << std::endl \
										   << __FILE__ << std::endl \
										   << __FUNCTIONW__ << L" (" << __LINE__ << ")" << std::endl; \
									throw RuntimeException(stream.str()); \
								} \
							}while (0)

///Used to automatically return whenever a HRESULT function fails
#define RETURN_ON_FAIL(expr) do \
							 { \
								HRESULT hr = expr; \
								if (FAILED(hr)){ \
									return hr; \
								} \
							 }while (0)

///Custom stack trace. Output to a string
class StackTrace : public StackWalker{

public:

	StackTrace() : StackWalker(StackWalkOptions::RetrieveNone) {}

	///Return the current stack trace
	wstring GetStackTrace(){

		//Remove the content of the stack trace
		stack_trace.str("");

		ShowCallstack();

		auto trace = stack_trace.str();

		return wstring(trace.begin(), trace.end());

	}

protected:

	virtual void OnCallstackEntry(CallstackEntryType eType, CallstackEntry &entry){

		StackWalker::OnCallstackEntry(eType, entry);

		if (entry.offset == 0){

			return;	//Invalid entry

		}

		stack_trace << entry.moduleName
			<< " - "
			<< entry.undFullName
			<< " ("
			<< std::to_string(entry.lineNumber)
			<< ")"
			<< std::endl;

	}

private:

	stringstream stack_trace;

};

///Runtime exception with stack trace
class RuntimeException{

public:

	/// Creates a new exception
	/** \param error_message The error message associated to the exception
	\param where Where the exception has been thrown from */
	RuntimeException(const wstring & error_message){
		
		StackTrace e;

		stack_trace_ = e.GetStackTrace();
		error_message_ = error_message;

	}

	/// Get the error message
	const wstring & GetErrorMessage() const{

		return error_message_;

	}

	/// Get the stack trace
	const wstring & GetStackTrace() const{

		return stack_trace_;

	}

private:

	wstring error_message_;

	wstring stack_trace_;
	
};


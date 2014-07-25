#pragma once

#include <string>
#include <sstream>

#include "StackWalker.h"

using ::std::wstring;
using ::std::stringstream;

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

	virtual void OnOutput(LPCSTR szText)
	{

		//TODO: Full output somewhere else...

	}

	virtual void OnCallstackEntry(CallstackEntryType eType, CallstackEntry &entry){

		StackWalker::OnCallstackEntry(eType, entry);

		if (entry.offset == 0){

			return;	//Invalid entry

		}

		stack_trace << entry.moduleName
					<< " - "
					<< entry.undFullName 
					<< " (" 
					<< std::to_string( entry.lineNumber ) 
					<< ")" 
					<< std::endl;
		
	}

private:

	stringstream stack_trace;

};
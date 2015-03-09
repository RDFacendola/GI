/// \file exceptions.h
/// \brief Defines classes and macros used for exception handling.
///
/// \author Raffaele D. Facendola

#pragma once

#pragma comment(lib, "StackWalker")

#include <string>
#include <iomanip>
#include <string>
#include <sstream>
#include <map>

#include <StackWalker.h>

#include "macros.h"

using ::std::wstring;
using ::std::stringstream;
using ::std::map;

/// \brief Debug boilerplate used to localize exceptions.
/// The format is "<File>:<Line> (<Function>)"
#define EXCEPTION_LOCATION __FILE__ ":" TO_STRING(__LINE__) " (" __FUNCTION__ ")"

/// \brief Debug boilerplate used to localize exceptions.
/// The format is "<File>:<Line> (<Function>)"
#define EXCEPTION_LOCATION_W CONCATENATE(L, __FILE__) L":" TO_WSTRING(__LINE__) L" (" CONCATENATE(L, __FUNCTION__) L")"

/// \brief Throws an exception.
#define THROW(message) throw Exception(message, EXCEPTION_LOCATION_W)

namespace gi_lib{

	/// \brief Manages the stack trace.
	/// \author Raffaele D. Facendola
	class StackTrace : public StackWalker{

	public:

		/// \brief Default constructor.
		StackTrace() : StackWalker(StackWalkOptions::RetrieveNone) {}

		/// \brief Get the current stack trace.
		/// \return Returns the current stack trace as a string.
		inline wstring GetStackTrace(){

			//Remove the content of the stack trace
			stack_trace.str("");

			ShowCallstack();

			auto trace = stack_trace.str();

			return wstring(trace.begin(), trace.end());

		}

	protected:

		/// \brief Callback used to log each stack entry.
		/// \param eType Type of the entry.
		/// \param entry Details about the entry.
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

	/// \brief Runtime exception
	/// \note A convenient way to throw a runtime exception is the following:
	/// \code {.cpp}
	/// throw RuntimeException("message", {{L"arg0", L"value0"}, {L"arg1", L"value1"}})
	/// \endcode
	/// \author Raffaele D. Facendola
	class Exception{

	public:

		/// \brief Type of the extra arguments.
		typedef map<wstring, wstring> TErrorArgs;

		/// \brief Move ctor.
		/// \param other The r-value reference to the object to move.
		Exception(Exception && other){

			where_ = std::move(other.where_);
			what_ = other.what_;
			stack_trace_ = std::move(other.stack_trace_);
			
		}

		/// \brief Creates a new exception.
		/// \param error_message The error message associated to the exception
		Exception(const wstring & what, const wstring & location){

			StackTrace e;

			where_ = location;
			what_ = what;
			stack_trace_ = e.GetStackTrace();

		}

		/// \brief Unified assigment operator.
		/// \param other The value to assign to this object.
		inline Exception & operator=(Exception other){

			other.Swap(*this);

			return *this;

		}

		/// \brief What happened.
		/// \return Returns a string with the exception cause.
		const wstring & GetWhat() const{

			return what_;

		}

		/// \brief Where the exception was thrown.
		/// \return Returns a string containing the location where the exception was thrown from.
		const wstring & GetWhere() const{

			return where_;

		}

		/// \brief Get the stack trace.
		/// \return Returns the stack trace.
		const wstring & GetStackTrace() const{

			return stack_trace_;

		}

	private:

		/// \brief Swap.
		inline void Swap(Exception & other){

			std::swap(where_, other.where_);
			std::swap(what_, other.what_);
			std::swap(stack_trace_, other.stack_trace_);

		}

		/// \brief Location of the exception.
		wstring where_;

		/// \brief What happened.
		wstring what_;

		wstring stack_trace_;
		
	};

}
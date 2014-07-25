#pragma once

/**
* Unidirectional read-only range
*
* Based on http://www.slideshare.net/rawwell/iteratorsmustgo
*/
template <class T>
class InputRange
{

public:

	/**
	* Is this range empty?
	*
	* \return True if the range is exhausted, false otherwise
	*/
	virtual bool IsEmpty() const = 0;

	/**
	* Remove the next element from the front
	*/
	virtual void PopFront() = 0;

	/**
	* Peek the front element
	*
	* \return The reference of the element in front
	*/
	virtual T & Front() const = 0;

};
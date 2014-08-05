#pragma once

///Two iterators packed together
template <class TValue, class TIterator>
class Range{

public:

	///STL typedef for containers
	typedef TIterator iterator;
	typedef ptrdiff_t difference_type;
	typedef size_t size_type;
	typedef TValue value_type;
	typedef TValue * pointer;
	typedef TValue & reference;

	Range(iterator begin_it, iterator end_it):
		begin_(begin_it),
		end_(end_it){}

	///Begin of the range
	iterator begin(){
		
		return begin_;

	}

	///End of the range
	iterator end() { 
		
		return end_;
	
	}

private:

	iterator begin_;

	iterator end_;

};
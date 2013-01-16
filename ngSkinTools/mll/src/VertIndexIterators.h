#pragma once

template<typename T>
class AbstractVertIndexIterator {
public:
	virtual const bool isDone() const = 0;
	virtual void next() = 0;
	virtual void reset() = 0;
	virtual const T index() const = 0;
	virtual ~AbstractVertIndexIterator(){};
};

template<typename T>
class VertIndexIteratorRandom: public AbstractVertIndexIterator<T> {
private:
	const T * const randomAccess;
	const unsigned int randomCount;
	unsigned int curr;
public:
	VertIndexIteratorRandom(const T *const randomAccess,const unsigned int randomCount):
		randomAccess(randomAccess),
		randomCount(randomCount),
		curr(0)
	{
	}

	virtual ~VertIndexIteratorRandom(){
	}

	const bool isDone() const {
		return curr>=randomCount;
	}
	void next(){
		curr++;
	}
	void reset(){
		curr=0;
	}
	const T index() const {
		return randomAccess[curr];
	}

};

template<typename T>
class VertIndexIteratorRange: public AbstractVertIndexIterator<T>  {
private:
	const T firstVert;
	const T lastVert;

	T curr;
public:
	VertIndexIteratorRange(const T firstVert,const T lastVert):
		firstVert(firstVert),
		lastVert(lastVert),
		curr(firstVert)
	{
	}

	virtual ~VertIndexIteratorRange(){
	}

	const bool isDone() const {
		return curr>lastVert;
	}
	void next(){
		curr++;
	}
	void reset(){
		curr=firstVert;
	}
	const T index() const {
		return curr;
	}

};

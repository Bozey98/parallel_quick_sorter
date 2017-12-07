#include <iostream>
#include <algorithm>
#include <new>
#include <thread>
#include <mutex>
#include <memory>

template <typename T>
class stack
{
public:
	stack();
	~stack();
	stack(stack<T> const&)/*strong*/;
	stack& operator=(stack<T> const&)/*strong*/;
	size_t count()const;
	void push(T const&)/*strong*/;
	auto pop()->std::shared_ptr<T>/*strong*/;
private:
	void swap(stack<T>&);
	mutable std::mutex mutex_;
	T * array_;
	size_t array_size_;
	size_t count_;
};

template <typename T>
stack<T>::stack() : array_{ nullptr }, array_size_{ 0 }, count_{ 0 }{}
template <typename T>
stack<T>::~stack()
{
	delete[] array_;
}
template <typename T>
stack<T>::stack(stack<T> const& other)
{
	std::lock_guard<std::mutex> lock(other.mutex_);
	T new_array = new T[other.array_size_];
	array_size_ = other.array_size_;
	count_ = other.count_;
	array_ = new_array;
	try
	{
		std::copy(other.array_, other.array_ + count_, array_);
	}
	catch (...)
	{
		std::cerr << "ERROR" << std::endl;
		delete[] array_;
	}
}
template <typename T>
stack<T>& stack<T>::operator=(stack<T> const & other)
{
	if (&other != this)
		stack(other).swap(*this);
	return *this;
}
template <typename T>
size_t stack<T>::count()const
{
	std::lock_guard<std::mutex> lock(mutex_);
	return count_;
}
template <typename T>
void stack<T>::push(T const & value)
{
	std::lock_guard<std::mutex> lock(mutex_);
	if (count_ == 0)
	{
		array_size_ = 1;
		array_ = new T[array_size_]();
	}
	else if (array_size_ == count_)
	{
		try
		{
			array_size_ *= 2;
			T * new_array = new T[array_size_]();
			std::copy(array_, array_ + count_, new_array);
			delete[] array_;
			array_ = new_array;
		}
		catch (std::bad_alloc)
		{
			std::cerr << "bad_alloc caught" << std::endl;
		}
	}
	array_[count_] = value;
	count_++;
}
template <typename T>
auto stack<T>::pop() -> std::shared_ptr<T>
{
	std::lock_guard<std::mutex> lock(mutex_);
	if (count_ == 0)
		return nullptr;
	auto top = std::make_shared<T>(array_[count_ - 1]);
	--count_;
	return top;
}
template <typename T>
void stack<T>::swap(stack<T>& other)
{
	std::lock(mutex_, other.mutex_);
	std::swap(array_, other.array_);
	std::swap(array_size_, other.array_size_);
	std::swap(count_, other.count_);
	mutex_.unlock();
	other.mutex_.unlock();
}
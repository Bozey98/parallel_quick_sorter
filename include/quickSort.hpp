#include <future>
#include "lomuto_partition.hpp"
#include "thread_safe_stack.hpp"
#include <thread>


template <typename BidirectionalIterator>
class parallel_quick_sorter_t
{
private:
	struct chunk_to_sort_t
	{
		BidirectionalIterator first;
		BidirectionalIterator last;
		std::promise<void> promise;
		chunk_to_sort_t(BidirectionalIterator left, BidirectionalIterator right) : first{ left }, last{ right }{}
	};
public:
	parallel_quick_sorter_t();
	parallel_quick_sorter_t(parallel_quick_sorter_t const & other) = delete;
	auto operator =(parallel_quick_sorter_t const & other)->parallel_quick_sorter_t & = delete;
	parallel_quick_sorter_t(parallel_quick_sorter_t && other) = delete;
	auto operator =(parallel_quick_sorter_t && other)->parallel_quick_sorter_t & = delete;
	~parallel_quick_sorter_t();

	void do_sort(BidirectionalIterator first, BidirectionalIterator last);
	void sort_thread();
	void try_sort_chunk();
	void sort_chunk(std::shared_ptr<chunk_to_sort_t> chunk);
private:
	std::vector<std::thread> threads_;
	unsigned int const max_threads_count_;
	stack<chunk_to_sort_t> chunks_;
	std::atomic<bool> end_of_data_;
};

template <typename BidirectionalIterator>
parallel_quick_sorter_t<BidirectionalIterator>::parallel_quick_sorter_t() :
	max_threads_count_{ std::thread::hardware_concurrency() - 1 },
	end_of_data_{ false } {}

template <typename BidirectionalIterator>
parallel_quick_sorter_t<BidirectionalIterator>::
~parallel_quick_sorter_t()
{
	end_of_data_ = true;
	for (auto && thread : threads_)
		thread.join();
}

template <typename BidirectionalIterator>
void parallel_quick_sorter_t<BidirectionalIterator>::
do_sort(BidirectionalIterator first, BidirectionalIterator last)
{
	if (first == last) 
		return;

	auto pivot = partition(first, last); 
	auto first_chunk = std::make_shared<chunk_to_sort_t>(chunk_to_sort_t(first, pivot));
	auto first_task = first_chunk->promise.get_future();
	chunks_.push(first_chunk);
	
	if (threads_.size() < max_threads_count_)
        	threads_.push_back(std::thread{ &parallel_quick_sorter_t<BidirectionalIterator>::sort_thread, this });
	
	do_sort(pivot, last);

	while (first_task.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
	        try_sort_chunk();
}

template <typename BidirectionalIterator>
void parallel_quick_sorter_t<BidirectionalIterator>::
try_sort_chunk()
{
	auto chunk = chunks_.pop();
	if (chunk)
		sort_chunk(*chunk);
}

template <typename BidirectionalIterator>
void parallel_quick_sorter_t<BidirectionalIterator>::
sort_thread()
{
	while (!end_of_data_)
	{
		try_sort_chunk;
		std::this_thread::yield();
	}
}

template <typename BidirectionalIterator>
void parallel_quick_sorter_t<BidirectionalIterator>::
sort_chunk(std::shared_ptr<chunk_to_sort_t> chunk)
{
	do_sort(chunk->first, chunk->last);
	chunk.promise.set_value();
}

template <typename BidirectionalIterator>
void parallel_quick_sort(BidirectionalIterator first, BidirectionalIterator last)
{
	if (first == last)
		return;

	parallel_quick_sorter_t<BidirectionalIterator> sorter;

	sorter.do_sort(first, last);
}

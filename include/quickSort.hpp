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
	end_of_data_ = true;// óñòàíàâëèâàåò ôëàã îá îêî÷àíèè äàííûõ
	for (auto && thread : threads_)// îæèäàåò çàâåðøåíèÿ ïîòîêîâ èç ïóëà
		threads_.join();
}

template <typename BidirectionalIterator>
void parallel_quick_sorter_t<BidirectionalIterator>::
do_sort(BidirectionalIterator first, BidirectionalIterator last)
{
	if (first == last) 
		return;

	// ðàçáèâàåò íà ïîðöèþ äàííûõ íà äâå
	auto pivot = partition(first, last); 
	// ïåðâóþ ÷àñòü çàêèäûâàåò â ñòåê çàäà÷
	auto first_chunk = std::make_shared<chunk_to_sort_t>(chunk_to_sort_t(first, pivot));
	auto first_task = first_chunk->promise.get_future();
	chunks_.push(first_chunk);
	// âòîðóþ ÷àñòü ñîðòèðóåò ñàì
	do_sort(pivot, last);

	// â öèêëå ïðîâåðÿåò îòñîðòèðîâàëàñü ëè ïåðâàÿ ÷àñòü
	while (first_task.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
		// åñëè íåò, òî áåðåò ïîðöèþ äàííûõ èç ñòåêà çàäà÷, è ñîðòèðóåò å¸
		sort_thread();
}

template <typename BidirectionalIterator>
void parallel_quick_sorter_t<BidirectionalIterator>::
try_sort_chunk()
{
	// ñîðòèðóåò ïîðöèþ äàííûõ, åñëè îíà åñòü
	auto chunk = chunks_.pop();
	if (chunk)
		sort_chunk(*chunk);
}

template <typename BidirectionalIterator>
void parallel_quick_sorter_t<BidirectionalIterator>::
sort_thread()
{
	while (!end_of_data_)// â öèêëå áåðåò ïîðöèþ äàííûõ èç ñòåêà çàäà÷, è ñîðòèðóåò å¸
	{
		try_sort_chunk;
		std::this_thread::yield();
	}
	// ïðè ýòîì íà êàæäîé èòåðàöèè âîçâðàùàåò óïðàâëåíèå ñèñòåìå
	// ÷òîáû òà ìîãëà äàòü âîçìîæíîñòü ïîðàáîòàòü äðóãèì ïîòîêà
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

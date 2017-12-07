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
	end_of_data_ = true;// устанавливает флаг об окочании данных
	for (auto && thread : threads_)// ожидает завершения потоков из пула
		threads_.join();
}

template <typename BidirectionalIterator>
void parallel_quick_sorter_t<BidirectionalIterator>::
do_sort(BidirectionalIterator first, BidirectionalIterator last)
{
	if (first == last) 
		return;

	// разбивает на порцию данных на две
	auto pivot = partition(first, last); 
	// первую часть закидывает в стек задач
	auto first_chunk = std::make_shared<chunk_to_sort_t>(chunk_to_sort_t(first, pivot));
	auto first_task = first_chunk->promise.get_future();
	chunks_.push(first_chunk);
	// вторую часть сортирует сам
	do_sort(pivot, last);

	// в цикле проверяет отсортировалась ли первая часть
	while (first_task.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
		// если нет, то берет порцию данных из стека задач, и сортирует её
		sort_thread();
}

template <typename BidirectionalIterator>
void parallel_quick_sorter_t<BidirectionalIterator>::
try_sort_chunk()
{
	// сортирует порцию данных, если она есть
	auto chunk = chunks_.pop();
	if (chunk)
		sort_chunk(*chunk);
}

template <typename BidirectionalIterator>
void parallel_quick_sorter_t<BidirectionalIterator>::
sort_thread()
{
	while (!end_of_data_)// в цикле берет порцию данных из стека задач, и сортирует её
	{
		try_sort_chunk;
		std::this_thread::yield();
	}
	// при этом на каждой итерации возвращает управление системе
	// чтобы та могла дать возможность поработать другим потока
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
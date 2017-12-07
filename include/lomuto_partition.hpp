#include <iostream>

template <typename Iterator>
void quickSort(Iterator left, Iterator right)
{
	if (left < right)
	{
		p = partition(left, right);
		quickSort(left, p - 1);
		quickSort(p + 1, right);
	}
}

template <typename Iterator>
void partition(Iterator left, Iterator right)
{
	auto pivot = *right;
	Iterator i = left - 1;
	for (Iterator j = left; j < right - 1; ++j)
	{
		if (*j <= pivot)
		{
			i++;
			std::swap(*i, *j)
		}
	}
	std::swap(*(i + 1), *right);
	return i + 1;
}
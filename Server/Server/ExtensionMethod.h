#pragma once
#include <iterator>
#include <random>
#include <vector>

template <typename T>
void CopySelf(std::vector<T>& v, size_t n)
{
	std::vector<T> tmp;
	for (size_t i = 0; i < n; ++i)
	{
		std::random_device rd;
		std::mt19937 g(rd());
		std::shuffle(v.begin(), v.end(), g);

		std::copy_n(v.begin(), v.size(), std::back_inserter(tmp));
	}
	std::copy(tmp.begin(), tmp.end(), std::back_inserter(v));
}

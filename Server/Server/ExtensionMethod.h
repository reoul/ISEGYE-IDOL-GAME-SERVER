﻿#pragma once
#include <iterator>
#include <vector>

template <typename T>
void CopySelf(std::vector<T>& v, size_t n)
{
	std::vector<T> tmp;
	for (size_t i = 0; i < n; ++i)
	{
		std::copy_n(v.begin(), v.size(), std::back_inserter(tmp));
	}
	std::copy(tmp.begin(), tmp.end(), std::back_inserter(v));
}

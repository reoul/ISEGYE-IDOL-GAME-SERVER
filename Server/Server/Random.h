/**
* Copyright (C) 2015 by Kyung-jin Kim
* e-mail		: devmachine@naver.com
*
*
* Description	: Random Number Generator Class
* Created		: Jun-19,2015
* Last Updated	: Mar-29,2018
* Version		: Random v1.0
*/

/**
* Sample code

************ Generate integer number between -1000 and 1000 ************
Random<int> gen(-1000, 1000);
auto num = gen();

************ Generate floating number between 0.0 and 1.0 ************
Random<double> gen(0.0, 1.0);
auto num = gen();

************ Generate large integer number between 0 and 100000000000 ************
Random<__int64> gen(0, 100000000000);
auto num = gen();

************ Generate integer number by user seed ************
unsigned int seed = 42;
Random<int> gen(-1000, 1000, seed);
auto num = gen();
*/

#pragma once
#include <random>

template <typename Type>
class Random
{
public:
	static_assert(std::is_arithmetic<Type>::value, "Template argument data must be a arithmetic type");

	using IntType = typename std::conditional<(std::is_integral<Type>::value && sizeof(Type) < 2),
		short,
		Type>::type;

	using Distribution = typename std::conditional<std::is_floating_point<Type>::value,
		std::uniform_real_distribution<Type>,
		std::uniform_int_distribution<IntType>>::type;

	using Engine = typename std::conditional<(std::is_integral<Type>::value && sizeof(Type) > 4),
		std::mt19937_64,
		std::mt19937>::type;

	Random(
		Type min = (std::numeric_limits<Type>::min)(), 
		Type max = (std::numeric_limits<Type>::max)(),
		typename Engine::result_type seed = std::random_device()()
		)
		: _engine(seed), _distribution((std::min)(min, max), (std::max)(min, max))
	{
	}

	Type operator()()
	{
		return static_cast<Type>(_distribution(_engine));
	}

private:
	Engine _engine;
	Distribution _distribution;
};
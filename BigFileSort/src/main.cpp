#include <iostream>
#include "ThreadedBigFileSorter.hpp"


int main()
{
	const size_t BUFFER_SIZE = 2000;
	const size_t NUMBERS_QTY = 2000000;

	BigFileSorter* bfm = new BigFileSorter("big_file", BUFFER_SIZE, NUMBERS_QTY);

	time_t time_before_start = time(0);

	bfm->sort();

	std::cout << " --- SORT TIME = " << difftime(time(0), time_before_start) << std::endl;

//	bfm->print();

	delete bfm;

	const size_t NUM_THREADS = 4;

	bfm = new ThreadedBigFileSorter("big_file", BUFFER_SIZE, NUMBERS_QTY, NUM_THREADS);

	time_before_start = time(0);

	bfm->sort();

	std::cout << " --- SORT TIME WITH ADDITIONAL THREADS = " << difftime(time(0), time_before_start) << std::endl;

//	bfm->print();

	delete bfm;

	return 0;
}

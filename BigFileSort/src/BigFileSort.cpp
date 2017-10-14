#include "BigFileManager.hpp"
#include <iostream>


int main()
{
	const size_t NUMBERS_QTY = 200000;
	const size_t MIN_BUFFER_SIZE = 200;

	BigFileManager* bfm = new BigFileManager("big_file", 0);

	for(int k = 1; k <= 5; ++k)
	{
		bfm->generate(NUMBERS_QTY);
//		bfm->print();

		const time_t TIME_BEFORE_START = time(0);

		bfm->sort(MIN_BUFFER_SIZE * k);
//		bfm->print();

		std::cout << " --- SORT TIME = " << difftime(time(0), TIME_BEFORE_START) << std::endl;
	}

	return 0;
}

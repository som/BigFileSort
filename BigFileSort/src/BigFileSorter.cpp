#include "BigFileSorter.hpp"
#include <iostream>
#include <random>
#include <fstream>


BigFileSorter::BigFileSorter(
	const std::string file_name,
	const size_t block_size,
	const size_t numbers_qty
)
	: FILE_NAME(file_name)
	, BLOCK_LENGTH(block_size)
	, BLOCK_LENGTH_HF(BLOCK_LENGTH / 2)
	, BUFFER_SIZE(BLOCK_LENGTH * TYPE_SIZE)
	, BUFFER_SIZE_HF(BUFFER_SIZE / 2)
{
	generate(numbers_qty);

	const uint64_t FILE_SIZE = get_size();

	m_total_blocks_qty = FILE_SIZE / BUFFER_SIZE;

	std::cout << " --- FILE SIZE: " << FILE_SIZE
		<< ", TOTAL BLOCKS: " << m_total_blocks_qty
		<< ", BLOCK LENGTH: " << BLOCK_LENGTH
	<< std::endl;
}


void BigFileSorter::generate(const size_t numbers_qty)
{
	std::cout << " --- Generate file with numbers quantity: " << numbers_qty << std::endl;

	std::ofstream file(FILE_NAME, std::ofstream::binary | std::ofstream::trunc);
	std::mt19937_64 generator(time(0));
	std::uniform_int_distribution<int64_t> uid(
		0, //std::numeric_limits<int64_t>::min(),
		100 //std::numeric_limits<int64_t>::max()
	);
	for(size_t i = 0; i < numbers_qty; ++i)
	{
		int64_t number = uid(generator);
		file.write((char *) &number, TYPE_SIZE);
	}
	file.close();

	std::cout << " --- Finish generate" << std::endl;
}


void BigFileSorter::print() const
{
	std::cout << " --- Print file '" << FILE_NAME << "'" << std::endl;

	std::ifstream file(FILE_NAME, std::ifstream::binary);

	if(!file.is_open())
	{
		std::cout << " --- Can't open file '" << FILE_NAME << "'" << std::endl;
		return;
	}

	uint64_t curr;
	for(int i = 0; !file.eof(); ++i)
	{
		file.seekg(TYPE_SIZE * i);
		file.read((char *) &curr, TYPE_SIZE);
		std::cout << curr << std::endl;
	}
	file.close();

	std::cout << " --- Finish print file" << std::endl;
}


uint64_t BigFileSorter::get_size() const
{
	std::ifstream file(FILE_NAME, std::ifstream::binary);
	if(!file.is_open())
	{
		return -1;
	}

	file.seekg(0, std::ios::end);
	uint64_t file_size = file.tellg();
	file.close();

	return file_size;
}


int BigFileSorter::comparator(const void* p1, const void* p2)
{
	return *(uint64_t *)p1 - *(uint64_t *)p2;
}


void BigFileSorter::process_file (
	std::fstream& file,
	uint64_t* block,
	const uint64_t from,
	const uint64_t to,
	const uint64_t seek_offset,
	const uint64_t process_size,
	const uint64_t rw_block_offset
)
{
	for(uint64_t i = from; i < to; ++i)
	{
		prepare_block(file, block + rw_block_offset, seek_offset + BUFFER_SIZE * i, process_size, [&] {
			qsort(block, BLOCK_LENGTH, TYPE_SIZE, comparator);
		});
	}
}


void BigFileSorter::prepare_block(
	std::fstream& file,
	uint64_t* block,
	const uint64_t seek_offset,
	const uint64_t process_size,
	std::function<void()> callback
)
{
	file.seekg(seek_offset);
	file.read((std::ios::char_type *)block, process_size);
	callback();
	file.seekp(seek_offset);
	file.write((std::ios::char_type *)block, process_size);
};


void BigFileSorter::sort()
{
	std::cout << " --- Start sort file '" << FILE_NAME << "' with buffer size = " << BUFFER_SIZE << std::endl;

	std::fstream file(FILE_NAME, std::ios::in | std::ios::out | std::ios::binary);
	uint64_t* block = new uint64_t[BLOCK_LENGTH];

	process_file(file, block, 0, m_total_blocks_qty, 0, BUFFER_SIZE, 0);

	for(int64_t blocks_qty = m_total_blocks_qty, step = 0; blocks_qty > 0;)
	{
		prepare_block(file, block, BUFFER_SIZE_HF * step, BUFFER_SIZE_HF, [&] {
			process_file(file, block, 1, blocks_qty, BUFFER_SIZE_HF * step, BUFFER_SIZE_HF, BLOCK_LENGTH_HF);
		});
		const uint64_t seek_offset = BUFFER_SIZE_HF * step + BUFFER_SIZE * blocks_qty - BUFFER_SIZE_HF;
		prepare_block(file, block + BLOCK_LENGTH_HF, seek_offset, BUFFER_SIZE_HF, [&] {
			process_file(file, block, 0, blocks_qty - 1, BUFFER_SIZE_HF * (step + 1), BUFFER_SIZE_HF, 0);
		});

		--blocks_qty;
		++step;

		process_file(file, block, 0, blocks_qty, BUFFER_SIZE_HF * step, BUFFER_SIZE, 0);
	}

	file.close();
	delete block;

	std::cout << " --- Finish sorting" << std::endl;
}


BigFileSorter::~BigFileSorter()
{
}


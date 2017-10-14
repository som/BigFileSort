#include "BigFileManager.hpp"
#include <iostream>
#include <random>
#include <fstream>
#include <functional>


BigFileManager::BigFileManager(const std::string file_name, const size_t numbers_qty)
	: FILE_NAME(file_name)
	, m_buffer_size(0)
{
	if(numbers_qty)
	{
		generate(numbers_qty);
	}
}


void BigFileManager::generate(const size_t numbers_qty)
{
	std::ofstream data(FILE_NAME, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
	std::mt19937_64 generator(time(0));
	std::uniform_int_distribution<int64_t> uid(
		std::numeric_limits<int64_t>::min(),
		std::numeric_limits<int64_t>::max()
	);
	std::cout << " --- Generate file with numbers quantity: " << numbers_qty << std::endl;
	for(size_t i = 0; i < numbers_qty; ++i)
	{
		int64_t number = uid(generator);
		data.write((char *) &number, TYPE_SIZE);
	}
	data.close();
	std::cout << " --- Finish generate" << std::endl;
}


void BigFileManager::print() const
{
	std::fstream file(FILE_NAME, std::fstream::in | std::fstream::binary);
	uint64_t curr;
	std::cout << " --- Print file '" << FILE_NAME << "'" << std::endl;
	for(int i = 0; !file.eof(); ++i)
	{
		file.seekg(TYPE_SIZE * i);
		file.read((char *) &curr, TYPE_SIZE);
		std::cout << curr << std::endl;
	}
	std::cout << " --- Finish print file" << std::endl;
	file.close();
}


uint64_t BigFileManager::get_size() const
{
	std::ifstream file(FILE_NAME, std::ifstream::in | std::ifstream::binary);
	if(!file.is_open())
	{
		return -1;
	}

	file.seekg(0, std::ios::end);
	uint64_t file_size = file.tellg();
	file.close();

	return file_size;
}


int BigFileManager::comparator(const void* p1, const void* p2)
{
	return *(uint64_t *)p1 - *(uint64_t *)p2;
}


int BigFileManager::sort(const size_t biffer_size)
{
	m_buffer_size = biffer_size * TYPE_SIZE;

	std::cout << " --- Start sort file '" << FILE_NAME
		<< "' with buffer size = " << m_buffer_size << std::endl;

	const uint64_t FILE_SIZE = get_size();

	const uint64_t BLOCKS_QTY = FILE_SIZE / m_buffer_size;

	const size_t BLOCK_SIZE = m_buffer_size / TYPE_SIZE - (m_buffer_size / TYPE_SIZE) % 2;
	const size_t BLOCK_SIZE_HF = BLOCK_SIZE / 2;

	const size_t BLOCK_BYTE_SIZE = BLOCK_SIZE * TYPE_SIZE;
	const size_t BLOCK_BYTE_SIZE_HF = BLOCK_BYTE_SIZE / 2;

	std::cout << " --- FILE SIZE: " << FILE_SIZE
		<< ", TOTAL BLOCKS: " << BLOCKS_QTY
		<< ", BLOCK SIZE: " << BLOCK_SIZE
	<< std::endl;

	auto process_block = [this, BLOCK_SIZE, BLOCK_BYTE_SIZE] (
		uint64_t* block,
		std::fstream& file,
		const uint64_t from,
		const uint64_t to,
		const uint64_t seek_offset,
		const uint64_t process_size,
		const uint64_t read_write_offset
	) {
		for(uint64_t i = from; i < to; ++i)
		{
			file.seekg(seek_offset + BLOCK_BYTE_SIZE * i);
			file.read((std::ios::char_type *)(block + read_write_offset), process_size);
			qsort(block, BLOCK_SIZE, TYPE_SIZE, comparator);
			file.seekp(seek_offset + BLOCK_BYTE_SIZE * i);
			file.write((std::ios::char_type *)(block + read_write_offset), process_size);
		}
	};

	std::fstream file(FILE_NAME, std::ios::in | std::ios::out | std::ios::binary);
	uint64_t* block = new uint64_t[BLOCK_SIZE];

	process_block(block, file, 0, BLOCKS_QTY, 0, BLOCK_BYTE_SIZE, 0);

	int64_t blocks_qty = BLOCKS_QTY, step = 0;
	while(blocks_qty > 0)
	{
		file.seekg(BLOCK_BYTE_SIZE_HF * step);
		file.read((char *)block, BLOCK_BYTE_SIZE_HF);
		process_block(block, file, 1, blocks_qty, BLOCK_BYTE_SIZE_HF * step, BLOCK_BYTE_SIZE_HF, BLOCK_SIZE_HF);
		file.seekp(BLOCK_BYTE_SIZE_HF * step);
		file.write((char *)block, BLOCK_BYTE_SIZE_HF);

		file.seekg(BLOCK_BYTE_SIZE_HF * step + BLOCK_BYTE_SIZE * blocks_qty - BLOCK_BYTE_SIZE_HF);
		file.read((char *)(block + BLOCK_SIZE_HF), BLOCK_BYTE_SIZE_HF);
		process_block(block, file, 0, blocks_qty - 1, BLOCK_BYTE_SIZE_HF * (step + 1), BLOCK_BYTE_SIZE_HF, 0);
		file.seekp(BLOCK_BYTE_SIZE_HF * step + BLOCK_BYTE_SIZE * blocks_qty - BLOCK_BYTE_SIZE_HF);
		file.write((char *)(block + BLOCK_SIZE_HF), BLOCK_BYTE_SIZE_HF);

		--blocks_qty;
		++step;

		process_block(block, file, 0, blocks_qty, BLOCK_BYTE_SIZE_HF * step, BLOCK_BYTE_SIZE, 0);
	}

	file.close();
	delete block;

	std::cout << " --- Finish sorting" << std::endl;

	return 0;
}


BigFileManager::~BigFileManager() {}


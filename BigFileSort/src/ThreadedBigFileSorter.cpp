#include "ThreadedBigFileSorter.hpp"
#include <cmath>
#include <fstream>


ThreadedBigFileSorter::ThreadedBigFileSorter(
	const std::string file_name,
	const size_t block_size,
	const size_t numbers_qty,
	const size_t num_threads
) : BigFileSorter(file_name, block_size, numbers_qty)
{
	m_queue = new ThreadedQueue(num_threads);
}


const size_t ThreadedBigFileSorter::blocks_per_thread(const size_t total_blocks_qty) const
{
	return floor(total_blocks_qty / m_queue->num_threads());
}


void ThreadedBigFileSorter::prepare_block(
	std::fstream& file,
	uint64_t* block,
	const uint64_t seek_offset,
	const uint64_t process_size,
	std::function<void()> callback
)
{
	{
		std::unique_lock<std::mutex> locker(m_lock_file);
		file.seekg(seek_offset);
		file.read((std::ios::char_type *)block, process_size);
	}
	callback();
	{
		std::unique_lock<std::mutex> locker(m_lock_file);
		file.seekp(seek_offset);
		file.write((std::ios::char_type *)block, process_size);
	}
};


void ThreadedBigFileSorter::process_file (
	std::fstream& file,
	uint64_t* block,
	const uint64_t from,
	const uint64_t to,
	const uint64_t seek_offset,
	const uint64_t process_size,
	const uint64_t rw_block_offset
)
{
	m_queue->reset_done_counter();

	for(uint64_t i = from; i < to; ++i)
	{
		uint64_t curr_seek_offset = seek_offset + BUFFER_SIZE * i;
		m_queue->enqueue([this, &file, block, rw_block_offset, curr_seek_offset, process_size] {
			prepare_block(file, block + rw_block_offset, curr_seek_offset, process_size, [this, block] {
				qsort(block, BLOCK_LENGTH, TYPE_SIZE, comparator);
			});
		});
	}
	m_queue->wait(to - from);
}


ThreadedBigFileSorter::~ThreadedBigFileSorter()
{
	delete m_queue;
}


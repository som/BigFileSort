#ifndef BIGFILEMANAGER_HPP_
#define BIGFILEMANAGER_HPP_
#include <string>


class BigFileManager
{
public:
	/** \brief Конструктор.
	 * \param file_name Имя файла, с которым будем работать.
	 * \param numbers_qty Количество элементов в файле.
	 */
	BigFileManager(const std::string file_name, const size_t numbers_qty = 0);
	virtual ~BigFileManager();

	/** \brief Отсортировать файл.
	 * \param biffer_size Размер буффера данных.
	 */
	int sort(const size_t biffer_size);

	/** \brief Вывести содержимое файла.
	 */
	void print() const;

	/** \brief Сгенерировать или перезаписать файл.
	 * \param numbers_qty Количество элементов в файле.
	 */
	void generate(const size_t numbers_qty);

private:
	const std::string FILE_NAME;
	const size_t TYPE_SIZE = sizeof(int64_t);
	size_t m_buffer_size;

	/** \brief Сравнить 2 элемента.
	 * \details Используется для стандартной функции qsort.
	 * \param el1 1 элемент.
	 * \param el2 2 элемент.
	 */
	static int comparator(const void* el1, const void* el2);

	/** \brief Получить размер файла.
	 */
	uint64_t get_size() const;
};

#endif /* BIGFILEMANAGER_HPP_ */

////////////////////////////////////////////////////////////////////////////////
/// Данная программа генерирует две квадратные матрицы и перемножает их.
/// Все элементы исходных матрицы заполняются значением 1.0 для удобства проверки
/// результата - в результирующей матрице элементы должны быть равны размеру
/// исходных матриц (N).
///
/// Программа принимает на вход один необязательный аргумент:
///     - размер квадратных матриц N (значение по умолчанию = 50)
///
/// Задача перемножения квадратных матриц размерностью N*N имеет сложность O(N^3)
////////////////////////////////////////////////////////////////////////////////
 
#include <cstdio>
#include <chrono>
#include <thread>
#include <vector>
#include <mutex>

using namespace std::chrono;

std::mutex c_mutex;

////////////////////////////////////////////////////////////////////////////////
// Выделение памяти под матрицу размером rows*cols
////////////////////////////////////////////////////////////////////////////////
double** create_matrix(size_t rows, size_t cols)
{
	double **M = new double*[rows];
	for (size_t i = 0; i < rows; ++i)
		M[i] = new double[cols];

	return M;
}

////////////////////////////////////////////////////////////////////////////////
// Освобождение памяти из под матрицы M размером rows*cols
////////////////////////////////////////////////////////////////////////////////
void delete_matrix(double** M, size_t rows, size_t cols)
{
	for (size_t i = 0; i < rows; ++i)
		delete[] M[i];
	delete[] M;
}

////////////////////////////////////////////////////////////////////////////////
// Заполнение матрицы M размером rows*cols значениями
////////////////////////////////////////////////////////////////////////////////
void fill_matrix(double** M, size_t rows, size_t cols, double value)
{
	for (size_t i = 0; i < rows; ++i)
		for (size_t j = 0; j < cols; ++j)
			M[i][j] = value;
}

////////////////////////////////////////////////////////////////////////////////
// Перемножение матриц A и B
////////////////////////////////////////////////////////////////////////////////
void mult_matrix(double** A, double** B, double** C, size_t A_rows, size_t A_cols, size_t B_cols,
	size_t th_idx, int P)
{
	for (size_t i = th_idx; i < A_rows; i += P)
	{
		for (size_t j = 0; j < B_cols; ++j)
		{
			double sum = 0;
			for (size_t k = 0; k < A_cols; ++k)
				sum += A[i][k] * B[k][j];

			std::lock_guard<std::mutex> guard(c_mutex);
			C[i][j] += sum;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Проверка результатов перемножения единичных матриц
////////////////////////////////////////////////////////////////////////////////
void test_result(double** M, size_t rows, size_t cols, double result)
{
	for (size_t i = 0; i < rows; ++i)
		for (size_t j = 0; j < cols; ++j)
			if (M[i][j] != result)
				printf("M[%zu][%zu] = %6.1f\n", i, j, M[i][j]);
}

int main(int argc, char* argv[])
{
	// определение размера решаемой задачи из параметров командной строки
	const size_t N = (argc == 2) ? atoi(argv[1]) : 1000;

	// узнаем число процессоров из переменной окружения
	const char* env_number_of_processors = std::getenv("NUMBER_OF_PROCESSORS");
	const int P = env_number_of_processors == nullptr ? 1 : atoi(env_number_of_processors);

	// выделение памяти под матрицы
	double **A = create_matrix(N, N);
	double **B = create_matrix(N, N);
	double **C = create_matrix(N, N);

	// заполнение матриц
	fill_matrix(A, N, N, 1.0);
	fill_matrix(B, N, N, 1.0);
	fill_matrix(C, N, N, 0.0);

	// время начала рассчетов
	auto start = high_resolution_clock::now();

	// умножение матриц
	std::vector<std::thread> threads;
	for (unsigned long i = 0; i < P; ++i)
		threads.emplace_back(mult_matrix, A, B, C, N, N, N, i, P);

	// ждем завершения
	for (auto& th : threads)
		th.join();

	// время окончания рассчетов
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<milliseconds>(stop - start).count();

	// проверка результатов перемножения
	test_result(C, N, N, static_cast<double>(N));

	// освобожение памяти
	delete_matrix(A, N, N);
	delete_matrix(B, N, N);
	delete_matrix(C, N, N);

	// вывод затраченного времени
	printf("Execution time : %llu ms\n", duration);

	return 0;
}

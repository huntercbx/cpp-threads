////////////////////////////////////////////////////////////////////////////////
// ������ ��������� ������ ������ ������������ ������� ������ � ������
////////////////////////////////////////////////////////////////////////////////
#include <cstdio>
#include <vector>
#include <set>
#include <algorithm>
#include <chrono>
#include <thread>
#include <mutex>

using namespace std::chrono;

////////////////////////////////////////////////////////////////////////////////
// ������� ���������� ����� ��������
////////////////////////////////////////////////////////////////////////////////
const int distance_map[15][15] =
{ { 0, 29, 82, 46, 68, 52, 72, 42, 51, 55, 29, 74, 23, 72, 46},
 {29,  0, 55, 46, 42, 43, 43, 23, 23, 31, 41, 51, 11, 52, 21},
 {82, 55,  0, 68, 46, 55, 23, 43, 41, 29, 79, 21, 64, 31, 51},
 {46, 46, 68,  0, 82, 15, 72, 31, 62, 42, 21, 51, 51, 43, 64},
 {68, 42, 46, 82,  0, 74, 23, 52, 21, 46, 82, 58, 46, 65, 23},
 {52, 43, 55, 15, 74,  0, 61, 23, 55, 31, 33, 37, 51, 29, 59},
 {72, 43, 23, 72, 23, 61,  0, 42, 23, 31, 77, 37, 51, 46, 33},
 {42, 23, 43, 31, 52, 23, 42,  0, 33, 15, 37, 33, 33, 31, 37},
 {51, 23, 41, 62, 21, 55, 23, 33,  0, 29, 62, 46, 29, 51, 11},
 {55, 31, 29, 42, 46, 31, 31, 15, 29,  0, 51, 21, 41, 23, 37},
 {29, 41, 79, 21, 82, 33, 77, 37, 62, 51,  0, 65, 42, 59, 61},
 {74, 51, 21, 51, 58, 37, 37, 33, 46, 21, 65,  0, 61, 11, 55},
 {23, 11, 64, 51, 46, 51, 51, 33, 29, 41, 42, 61,  0, 62, 23},
 {72, 52, 31, 43, 65, 29, 46, 31, 51, 23, 59, 11, 62,  0, 59},
 {46, 21, 51, 64, 23, 59, 33, 37, 11, 37, 61, 55, 23, 59,  0} };


std::mutex best_solution_mutex;
std::vector<size_t> best_solution_found;
int best_estimation = 0;


////////////////////////////////////////////////////////////////////////////////
// ������� ��� ���������� ������ ��������� �������� (������� ��� ��� �����)
// solution - �������
// len      - ���������� ����������� ���������
////////////////////////////////////////////////////////////////////////////////
int estimate(const std::vector<size_t>& solution, size_t len)
{
	if (solution.size() < len)
		return -1;

	int sum = 0;
	for (size_t i = 1; i < len; ++i)
		sum += distance_map[solution[i - 1]][solution[i]];
	if (solution.size() == len)
		sum += distance_map[solution[len - 1]][solution[0]];
	return sum;
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
void walk_route(const std::vector<size_t>& partial_route, size_t len)
{
	int estimation = estimate(partial_route, len);

	// ��������� ������������� ���������
	if (estimation >= best_estimation)
		return;

	// �������� �� ������ �������
	if (partial_route.size() == len)
	{
		// ���� ������� ������ �����, �� ������� ����� ����������� �������
		if (estimation < best_estimation)
		{
			std::lock_guard<std::mutex> guard(best_solution_mutex);
			if (estimation < best_estimation)
			{
				best_estimation = estimation;
				best_solution_found = partial_route;
			}
		}
		return;
	}

	// ������� ��� ������������ ������
	std::set<size_t> unvisited_cities;
	for (size_t i = 0; i < partial_route.size(); ++i)
		unvisited_cities.insert(i);
	for (size_t i = 0; i < len; ++i)
		unvisited_cities.erase(partial_route[i]);

	// ��������� �� ����� ��������� - ������� ������
	std::vector<std::thread> threads;
	for (const auto& city : unvisited_cities)
	{
		auto new_route = partial_route;
		new_route[len] = city;
		if (len <= 3)
			threads.emplace_back(walk_route, new_route, len + 1);
		else
			walk_route(new_route, len + 1);
	}

	// ���� ���������� �������
	for (auto& th : threads)
		th.join();
}

int main(int argc, char* argv[])
{
	// ����������� ������� �������� ������ �� ���������� ��������� ������
	const size_t N = std::min(15, std::max(5, (argc == 2) ? atoi(argv[1]) : 10));

	// ����� ������ ���������
	auto start = high_resolution_clock::now();

	// ���� ��� ������ �������� �������� ������ ���������
	best_solution_found.resize(N);
	for (size_t i = 0; i < N; ++i)
		best_solution_found[i] = i;

	// ������ ������� �������� (������� ���������)
	best_estimation = estimate(best_solution_found, best_solution_found.size());

	std::vector<size_t> route(N);
	route[0] = 0;
	walk_route(route, 1);

	// ����� ��������� ���������
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<milliseconds>(stop - start).count();

	// ����� ����������
	printf("Best solution found (%d) : ", best_estimation);
	for (size_t i = 0; i < best_solution_found.size(); ++i)
		printf(i > 0 ? " -> %zu" : "%zu", best_solution_found[i]);
	printf(" -> %zu\n", best_solution_found[0]);

	// ����� ������������ �������
	printf("Execution time : %llu ms\n", duration);

	return 0;
}

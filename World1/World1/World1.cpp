// Try.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#include <iostream>
#include<atomic>
#include<string>
#include<algorithm>

#define HAVE_STRUCT_TIMESPEC
#include<pthread.h>

#include<vector>


std::atomic<int> Iteration;
std::atomic<int> Stoped;
std::atomic_flag lock_stream = ATOMIC_FLAG_INIT;
pthread_cond_t count_threshold_cv;
pthread_mutex_t count_mutex;

pthread_cond_t stop_cv;
pthread_mutex_t stop_mutex;

struct thread_data
{
	std::atomic<long long> *max_iter;
	std::vector<std::vector<int> > *grid;
	std::vector<std::vector<int> > pred;
	std::vector<int> *cur_left;
	std::vector<int> *cur_right;
	std::vector<int> *left;
	std::vector<int> *right;
	std::vector<int> *prev_cur_left;
	std::vector<int> *prev_cur_right;
	std::vector<int> *prev_left;
	std::vector<int> *prev_right;
	std::vector<std::vector<int> > current;
	std::vector<int> *status;
	pthread_mutex_t mutex;
	pthread_mutex_t left_mutex;
	pthread_mutex_t right_mutex;
	pthread_mutex_t ml_mutex;
	pthread_mutex_t mr_mutex;
	pthread_cond_t cond;
	int start;
	int end;
	int id;
	int num;
	int h;
	int w;
	bool stop;
};

std::vector<std::vector<int>> make_grid(std::vector<std::vector<std::vector<int>>> split) {
	std::vector<std::vector<int>> result;
	int result_w = 0;
	int n = split.size();
	int h = split[0].size();
	result = std::vector<std::vector<int>>(h, std::vector<int>(0));
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < h; ++j) {
			int w = split[i][j].size();
			result_w = 0;
			for (int k = 0; k < w; k++) {
				result[j].emplace_back(split[i][j][k]);
				result_w++;
			}
		}
	}
	return result;
}


int adjacent_to(std::vector<std::vector<int>> &board, int i, int j, int start, int end) {

	int h = board.size();
	int w = board[0].size();
	int k, l, count = 0;
	int sk = (i > 0) ? i - 1 : i;
	int ek = (i + 1 < h) ? i + 1 : i;
	int sl = (j > 0) ? j - 1 : j;
	int el = (j + 1 < w) ? j + 1 : j;

	for (k = sk; k <= ek; k++)
		for (l = sl; l <= el; l++)
			if (board[k][l] == 1) {
				count += board[k][l];
			}
	if (board[i][j] == 1) {
		count -= board[i][j];
	}
	return count;
}

int abs(int a) {
	if (a > 0)
	{
		return a;
	}
	return -a;
}

void * f(void *p) {
	//struct thread_data *data = (thread_data *)p;
	//std::vector<int> &status = *(std::vector<int>*)data->status;
	while (1) {
		struct thread_data *data = (thread_data *)p;
		std::vector<int> &status = *(std::vector<int>*)data->status;
		pthread_mutex_lock(&stop_mutex);
		while (Stoped.load() == 1) {
			data->stop = true;
			std::cout << "Potok number: " << data->id << std::endl;
			pthread_cond_wait(&stop_cv, &stop_mutex);
		}
		pthread_mutex_unlock(&stop_mutex);
		if (Iteration.load() <= status[data->id]) {
			//return 0;
		}
		else
		{

			if (abs(status[data->id] +1 - status[(data->id + 1 + data->num) % data->num]) <= 1 && abs(status[data->id] + 1 - status[(data->id - 1 + data->num) % data->num]) <= 1) {
				pthread_mutex_lock(&data->mutex);
				pthread_mutex_lock(&data->left_mutex);
				pthread_mutex_lock(&data->right_mutex);
				pthread_mutex_lock(&data->ml_mutex);
				pthread_mutex_lock(&data->mr_mutex);
				pthread_mutex_unlock(&data->mutex);
				struct thread_data *data = (thread_data *)p;
				std::vector<std::vector<int>> &grid = *(std::vector<std::vector<int>>*)data->grid;
				std::vector<int> right;
				std::vector<int> left;

				bool type1 = false;
				if (abs(status[data->id] - status[(data->id + 1 + data->num) % data->num]) == 1) {
					type1 = true;
				}
				bool type2 = false;
				if (abs(status[data->id] - status[(data->id - 1 + data->num) % data->num]) == 1) {
					type2 = true;
				}
				if (type1) {
					right = *data->prev_right;
				}
				else {
					right = *data->right;
				}
				if (type2) {
					left = *data->prev_left;
				}
				else {
					left = *data->left;
				}

				std::vector<int> &prev_cur_left = *(std::vector<int>*)data->prev_cur_left;
				std::vector<int> &prev_cur_right = *(std::vector<int>*)data->prev_cur_right;
				std::vector<int> temp1 = *(std::vector<int>*)data->cur_left;
				std::vector<int> temp2 = *(std::vector<int>*)data->cur_right;
				prev_cur_left = temp1;
				prev_cur_right = temp2;


				std::vector<std::vector<int>> copy = grid;
				data->pred = data->current;

				int h = data->current.size();
				int w = data->current[0].size();

				std::vector<std::vector<int>> temp(h, std::vector<int>(w + 2));
				for (int i = 0; i < h; ++i) {
					for (int j = 0; j < w + 2; j++) {
						if (j == 0) {
							temp[i][j] = left[i];
							continue;
						}
						if (j == w + 1) {
							temp[i][j] = right[i];
							continue;
						}
						temp[i][j] = data->current[i][j - 1];
					}
				}

				for (int j = 1; j < w + 1; j++) {
					for (int i = 0; i < h; i++) {
						int a = adjacent_to(temp, i, j, 1, w + 1);
						if (temp[i][j] == 0) {
							if (a == 3) data->current[i][j - 1] = 1;
						}
						else {
							if (a < 2) data->current[i][j - 1] = 0;
							if (a > 3) data->current[i][j - 1] = 0;
						}

					}

				}


				std::vector<int> &cur_left = *(std::vector<int>*)data->cur_left;
				std::vector<int> &cur_right = *(std::vector<int>*)data->cur_right;

				for (int j = 0; j < h; ++j) {
					cur_left[j] = data->current[j][0];
					cur_right[j] = data->current[j][w - 1];
				}

				status[data->id] += 1;
				pthread_mutex_unlock(&data->left_mutex);
				pthread_mutex_unlock(&data->right_mutex);
				pthread_mutex_unlock(&data->ml_mutex);
				pthread_mutex_unlock(&data->mr_mutex);

				//pthread_mutex_unlock(&data->mutex);
			}
		}

	}

	return NULL;
}



// The MAIN function, from here we start the application and run the game loop
int main()
{
	Iteration.store(0);
	Stoped.store(0);
	lock_stream.clear();
	std::cout << "Print number of threads" << std::endl;
	int num_threads;
	std::cin >> num_threads;
	std::cout << "Print size of grid" << std::endl;
	int h, w;
	//int num_threads = 30;
	std::cin >> h >> w;
	std::vector<std::vector<int>> grid(h, std::vector<int>(w));

	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			grid[i][j] = 0;
		}
	}
	grid[3][3] = 1;
	grid[4][3] = 1;
	grid[5][3] = 1;
	grid[5][4] = 1;
	grid[4][5] = 1;



	////


	std::vector<int> starts(num_threads, 0);
	std::vector<int> ends(num_threads, 0);

	int size = w / num_threads;

	for (int i = 0; i < num_threads; ++i) {
		starts[i] = size * i;
		if (i != num_threads - 1)
			ends[i] = starts[i] + size;
		else
			ends[i] = w;
		std::cout << starts[i] << " " << ends[i] << std::endl;
	}

	std::vector<std::vector<std::vector<int>>> split(num_threads);
	for (int i = 0; i < num_threads; ++i) {
		split[i] = std::vector<std::vector<int>>(h, std::vector<int>(ends[i] - starts[i], 0));
	}

	for (int i = 0; i < num_threads; ++i) {
		for (int j = 0; j < h; ++j) {
			for (int k = starts[i]; k < ends[i]; k++) {
				split[i][j][k - starts[i]] = grid[j][k];
			}

		}
	}

	grid = make_grid(split);

	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			std::cout << grid[i][j] << " ";
		}
		std::cout << std::endl;
	}

	///

	int *k = new int[num_threads];
	for (unsigned long long i = 0; i < num_threads; ++i) {
		k[i] = i;
	}
	pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);

	std::vector<int> status(num_threads, 0);
	std::vector<pthread_mutex_t> cond_var_lock(2 * num_threads);
	std::vector<thread_data> td(num_threads);

	std::vector<std::vector<int>> piece(2 * num_threads, std::vector<int>(h, 0));
	std::vector<std::vector<int>> prev_piece(2 * num_threads, std::vector<int>(h, 0));

	for (int i = 0; i < num_threads; ++i) {

		for (int j = 0; j < h; ++j) {
			int k = split[i][0].size() - 1;
			piece[2 * i][j] = split[i][j][0];
			piece[2 * i + 1][j] = split[i][j][k];
		}

	}
	prev_piece = piece;


	for (int i = 0; i < 2 * num_threads; ++i) {
		pthread_mutex_init(&cond_var_lock[i], NULL);
	}

	pthread_mutex_init(&count_mutex, NULL);
	pthread_cond_init(&count_threshold_cv, NULL);

	pthread_mutex_init(&stop_mutex, NULL);
	pthread_cond_init(&stop_cv, NULL);

	std::atomic<long long> max_iteration = -1;
	for (int i = 0; i < num_threads; ++i) {
		td[i].max_iter = &max_iteration;
		td[i].status = &status;
		td[i].h = h;
		td[i].w = w;
		td[i].start = starts[i];
		td[i].end = ends[i];
		td[i].grid = &grid;
		td[i].mutex = count_mutex;
		td[i].ml_mutex = cond_var_lock[2 * i];
		td[i].mr_mutex = cond_var_lock[2 * i + 1];
		td[i].left_mutex = cond_var_lock[(2 * i - 1 + 2 * num_threads) % (2 * num_threads)];
		td[i].right_mutex = cond_var_lock[(2 * i + 2 + 2 * num_threads) % (2 * num_threads)];
		td[i].id = i;
		td[i].num = num_threads;
		td[i].current = split[i];
		td[i].cur_left = &piece[2 * i];
		td[i].cur_right = &piece[2 * i + 1];
		td[i].left = &piece[(2 * i - 1 + 2 * num_threads) % (2 * num_threads)];
		td[i].right = &piece[(2 * i + 2 + 2 * num_threads) % (2 * num_threads)];
		td[i].prev_cur_left = &prev_piece[2 * i];
		td[i].prev_cur_right = &prev_piece[2 * i + 1];
		td[i].prev_left = &prev_piece[(2 * i - 1 + 2 * num_threads) % (2 * num_threads)];
		td[i].prev_right = &prev_piece[(2 * i + 2 + 2 * num_threads) % (2 * num_threads)];
		td[i].stop = false;

	}
	std::string s;
	while (1) {
		std::cin >> s;
		if (s == "start") {
			for (unsigned long long i = 0; i < num_threads; ++i) {
				pthread_create(threads + i, NULL, f, &td[i]);
			}
		}
		if (s == "stop") {
			std::cout << "please, wait" << std::endl;
			Stoped.store(1);
			bool ready = false;
			while (!ready) {
				ready = true;
				for (int i = 0; i < num_threads; ++i) {
					if (td[i].stop == false) {
						ready = false;
					}
				}
			}
			pthread_mutex_lock(&stop_mutex);

			int result = status[0];
			for (int i = 0; i < num_threads; ++i) {
				if (result < status[i]) {
					result = status[i];
				}
			}

			Iteration.store(result);
			Stoped.store(0);
			for (int i = 0; i < num_threads; ++i) {
				td[i].stop = false;
			}

			pthread_cond_broadcast(&stop_cv);
			pthread_mutex_unlock(&stop_mutex);

			bool stady = false;
			while (!stady) {
				stady = true;
				for (int i = 0; i < num_threads; ++i) {
					if (Iteration.load() > status[i]) {
						stady = false;
					}
				}
			}

			for (int i = 0; i < num_threads; ++i) {
				std::cout << status[i] << " ";
			}
			
			std::cout << std::endl;
			std::cout << Iteration.load() << std::endl;
			std::cout << "I am ready" << std::endl;

		}
		if (s == "run") {
			int number;
			std::cin >> number;
			int temper = Iteration.load();
			Iteration.store(number + temper);
			
			std::cout << "I am ready" << std::endl;
			
		}
		if (s == "q") {
			delete k;
			pthread_mutex_destroy(&count_mutex);
			pthread_cond_destroy(&count_threshold_cv);

			return 0;
		}
		if (s == "status") {
			bool ready = true;
			for (int i = 0; i < num_threads; ++i) {
				if (Iteration.load() > status[i]) {
					ready = false;
				}
			}
			if (!ready) {
				std::cout << "Not ready" << std::endl;
			}
			else {
				std::vector<std::vector<std::vector<int>>> result_split(num_threads);
				for (int i = 0; i < num_threads; ++i) {
					result_split[i] = td[i].current;
				}
				grid = make_grid(result_split);
				for (int i = 0; i < h; ++i) {
					for (int j = 0; j < w; ++j) {
						std::cout << grid[i][j] << " ";
					}
					std::cout << std::endl;
				}
				for (int i = 0; i < num_threads; ++i) {
					std::cout << status[i] << " ";
				}
				std::cout << std::endl;
			}

		}
	}

	Iteration.store(21);
	for (unsigned long long i = 0; i < num_threads; ++i) {
		pthread_create(threads + i, NULL, f, &td[i]);
	}

	void *p;
	for (unsigned long long i = 0; i < num_threads; ++i) {
		pthread_join(threads[i], &p);
	}

	std::vector<std::vector<std::vector<int>>> result_split(num_threads);
	for (int i = 0; i < num_threads; ++i) {
		result_split[i] = td[i].current;
	}
	grid = make_grid(result_split);
	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			std::cout << grid[i][j] << " ";
		}
		std::cout << std::endl;
	}


	delete k;
	pthread_mutex_destroy(&count_mutex);
	pthread_cond_destroy(&count_threshold_cv);

	return 0;
}



// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
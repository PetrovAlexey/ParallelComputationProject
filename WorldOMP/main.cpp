#include <iostream>
#include <omp.h>
#include <vector>
#include <string>
int adjacent_to(std::vector<std::vector<int>> &board, int i, int j) {

	int h = board.size();
	int w = board[0].size();
	int k, l, count = 0;
	int sk = (i > 0) ? i - 1 : i;
	int ek = (i + 1 < h) ? i + 1 : i;
	int sl = (j > 0) ? j - 1 : j;
	int el = (j + 1 < w) ? j + 1 : j;

	/*for (k = sk; k <= ek; k++)
		for (l = sl; l <= el; l++)
			if (board[k][l] == 1) {
				count += board[k][l];
			}*/
	if (board[(i + 1 + w) % w][j] == 1) {
		count += board[(i + 1 + w) % w][j];
	}
	if (board[(i + 1 + w) % w][(j + 1 + h) % h] == 1) {
		count += board[(i + 1 + w) % w][(j + 1 + h) % h];
	}
	if (board[(i + 1 + w) % w][(j - 1 + h) % h] == 1) {
		count += board[(i + 1 + w) % w][(j - 1 + h) % h];
	}
	if (board[(i + 0 + w) % w][(j - 1 + h) % h] == 1) {
		count += board[(i + 0 + w) % w][(j - 1 + h) % h];
	}
	if (board[(i + 0 + w) % w][(j + 0 + h) % h] == 1) {
		count += board[(i + 0 + w) % w][(j + 0 + h) % h];
	}
	if (board[(i + 0 + w) % w][(j + 1 + h) % h] == 1) {
		count += board[(i + 0 + w) % w][(j + 1 + h) % h];
	}
	if (board[(i - 1 + w) % w][(j + 0 + h) % h] == 1) {
		count += board[(i - 1 + w) % w][(j + 0 + h) % h];
	}
	if (board[(i - 1 + w) % w][(j + 1 + h) % h] == 1) {
		count += board[(i - 1 + w) % w][(j + 1 + h) % h];
	}
	if (board[(i - 1 + w) % w][(j - 1 + h) % h] == 1) {
		count += board[(i - 1 + w) % w][(j - 1 + h) % h];
	}

	if (board[i][j] == 1) {
		count -= board[i][j];
	}
	return count;
}


int main(int argc, char** argv) {
	omp_set_nested(1);
	std::cout << "Print size of grid" << std::endl;
	int h, w;
	std::cin >> h >> w;
	std::vector<std::vector<int>> grid(h, std::vector<int>(w));
	std::vector<std::vector<int>> prev_grid(h, std::vector<int>(w));
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

	prev_grid = grid;
	////
	std::string s;
	bool quit = false;
	int iter_num=0;
	int cur_num=0;
	#pragma omp parallel num_threads(2)
	{
	#pragma omp single nowait
		{
			while (1) {
				std::cin >> s;
				if (s == "stop") {
					iter_num = cur_num + 1;
				}
				if (s == "run") {
					int number;
					std::cin >> number;
					iter_num += number;
				}
				if (s == "quit") {
					quit = true;
					break;
				}
				if (s == "status") {
					std::cout << iter_num << " " << cur_num << std::endl;
					if (iter_num > cur_num) {
						std::cout << "Not ready" << std::endl;
					}
					else {
						for (int i = 0; i < h; ++i) {
							for (int j = 0; j < w; ++j) {
								std::cout << grid[i][j] << " ";
							}
							std::cout << std::endl;
						}
					}
				}
			} // while
		} // single
		
		int thread_id = omp_get_thread_num();
		//std::cout << thread_id << std::endl;
		while (1) {
			while (iter_num > cur_num) {
				prev_grid = grid;
				#pragma omp parallel shared(prev_grid, grid, h, w, iter_num) num_threads(4)
					{
					#pragma omp for
						for (int j = 0; j < w; j++) {
							//int thread_id = omp_get_thread_num();
							//std::cout << thread_id << std::endl;
							for (int i = 0; i < h; i++) {
								int a = adjacent_to(prev_grid, i, j);
								if (prev_grid[i][j] == 0) {
									if (a == 3) grid[i][j] = 1;
								}
								else {
									if (a < 2) grid[i][j] = 0;
									if (a > 3) grid[i][j] = 0;
								}
							}
						}
					}

					cur_num += 1;
					if (iter_num <= cur_num) {
						break;
					}
					
					///
				}
				if (quit) {
					break;
					break;
				}
			}
	
	}
	return 0;
}

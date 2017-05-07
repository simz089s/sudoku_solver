#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/*
 * Usually compiled using: gcc -o sudoku_solver -std=gnu11 -Ofast sudoku_solver.c -lrt
 * gnu11 and lrt are only needed for time calculations
 */

void solve();
int solve2(int cur_row, int cur_col);
int is_valid(int cur_row, int cur_col, int current);
void find_last_zero();
int is_last_complete(int coord[2]);
void print_sudoku();
void read_sudoku(char* filename);

/* For a 3x3, SIZE=3 and N=9 */
int SIZE;
int N;

/* Sudoku grid converted from text file
 * Must be changed for each size (though a bigger size still works for smaller sizes) */
int Grid[9][9];

/* Last unsolved box coordinates */
int last_zero[2];

/* For backtrack algorithm, when trying each values from 1 up to MID and N down to MID+1 */
int MID;

/* Function that initializes the solving algorithm */
void solve()
{
	find_last_zero(); // See function
	
	/* Find first 0 */
	for (int row = 0; row < N; ++row)
	{
		for (int col = 0; col < N; ++col)
		{
			if ( !Grid[row][col] )
			{
				solve2(row, col);
			}
		}
	}
}

/* Backtrack algorithm for solving */
int solve2(int cur_row, int cur_col)
{
	if ( is_last_complete(last_zero) ) return 1;
	
	// Find next 0 starting from current position
	for (int row = cur_row; row < N; ++row)
	{
		for (int col = cur_col; col < N; ++col)
		{
			if ( !Grid[row][col] )
			{
				for (int val = 1, val2; ; ++val)
				{
					if ( is_valid(row, col, val) )
					{
						Grid[row][col] = val;
						if ( solve2(row, col) ) return 1;
					}
					if ( val == MID ) break;
					val2 = N - val + 1;
					if ( is_valid(row, col, val2) )
					{
						Grid[row][col] = val2;
						if ( solve2(row, col) ) return 1;
					}
				}
				
				Grid[row][col] = 0;
				return 0;
				
			}
		}
		cur_col = 0; // Columns after the starting row and col must begin at 0 every row for this loop
	}
	return 0;
}

/* Does every check (column, row, subsquare) */
int is_valid(int cur_row, int cur_col, int current)
{
	for (int cmp = 0; cmp < N; ++cmp)
	{
		if (cmp != cur_col && current == Grid[cur_row][cmp]) return 0; // Check number not repeated in columns
		if (cmp != cur_row && current == Grid[cmp][cur_col]) return 0; // Check rows (vertical)
	}
	
	// Important to not simplify to preserve integer division
	int start_row = cur_row/SIZE*SIZE;
	int start_col = cur_col/SIZE*SIZE;
	int end_row = SIZE+start_row;
	int end_col = SIZE+start_col;
	// Compare against all subsquare except itself
	for (int row_indexer = start_row; row_indexer < end_row; ++row_indexer)
	{
		for (int col_indexer = start_col; col_indexer < end_col; ++col_indexer)
			if ( col_indexer != cur_col && row_indexer != cur_row && Grid[row_indexer][col_indexer] == current ) return 0;
	}
	
	return 1;
}

/* Knowing solve() always goes left-right and top-down, find most bottomright 0 and just always check that */
void find_last_zero()
{
	/* Solve starts from topleft so starting here from bottomright is fastest */
	for (int row = N-1; row != 0; --row)
	{
		for (int col = N-1; col != 0; --col)
		{
			if ( !Grid[row][col] )
			{
				last_zero[0] = row;
				last_zero[1] = col;
				return;
			}
		}
	}
}

/* Check for completion */
int is_last_complete(int coord[2])
{
	if ( !Grid[coord[0]][coord[1]] ) return 0;
	return 1;
}

/* Made with 3x3 in mind for now */
void print_sudoku()
{
	for( int i = 0; i < N; ++i )
	{
		if ( !(i%SIZE) ) printf("-------------------------------\n");
		for( int j = 0; j < N; ++j )
		{
			if ( !(j%SIZE) ) printf("|");
			printf("%2d ", Grid[i][j]);
		}
		printf("|\n");
	}
	printf("-------------------------------\n");
}

void read_sudoku(char* filename)
{
	FILE* fp = fopen(filename, "rb");
	
	int size;
	while ( fscanf(fp, "%d", &size) < 1 );
	SIZE = size;
	N = size*size;
	int parity = N % 2;
	MID = N/2 + parity;
	
	fseek(fp, 0, SEEK_END);
	long fsize = ftell(fp);
	fseek(fp, 1, SEEK_SET);
	char *sudoku = malloc(fsize + 1);
	fread(sudoku, fsize, 1, fp);
	sudoku[fsize] = 0;
	
	char *p = sudoku;
	while (*p)
	{
		if (*p == 'x') *p = '0';
		if (*p < '0' || *p > '9') *p = ' ';
		++p;
	}
	
	p = sudoku;
	int numbers[N*N];
	int i = 0;
	while (*p)
	{
		if (isdigit(*p))
		{
			int val = strtol(p, &p, 10);
			numbers[i] = val;
			++i;
		}
		else ++p;
	}
	
	for (int row = 0, i = 0; row < N; ++row)
		for (int col = 0; col < N; ++col, ++i)
			Grid[row][col] = numbers[i];
	
	fclose(fp);
	free(sudoku);
}

int main(int argc, char **argv)
{
	char *filename = "extremelyHard3x3.txt\0";
	if (argc > 1) filename = argv[1];
	//~ scanf("%s", filename);
	
	read_sudoku(filename);
	
	print_sudoku();
	
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	solve();
	clock_gettime(CLOCK_MONOTONIC_RAW, &end);
	uint delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
	
	print_sudoku();
	printf("Took about %.5f seconds\n", delta_us/1000000.0);
	
	return 0;
}

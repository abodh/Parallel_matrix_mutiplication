// DOT PRODUCT

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

//change based on the shape of your matrix
// #define ROWS 4096
// #define COLS 4096
#define MINSIZE 128
#define MAXSIZE 4096
#define storage 6

#define index(i,j,cols) i*cols+j

/*
*	Function: initializeMatrix
*
*	Description: Allocates memory for a row x col matrix of floats,
*		initialized to 0.
*
*	Parameters:
*		int rows - number of rows in the matrix or vector
*		int cols - number of columns in the matrix or vector (cols=1)
*
*	Return: (float *) pointing to matrix/vector
*/
float* initializeMatrix(int rows, int cols) {
	//allocate memory for the matrix and set each value to 0
	float* array = (float*)calloc(rows * cols, sizeof(float));
	return array;
}
/*
*	Function: readfile
*
*	Description: Reads in a rows x cols matrix from filename.
*
*	Parameters:
*		char * filename - C-string name of the file to be read
*		int rows - number of rows in the matrix or vector
*		int cols - number of columns in the matrix or vector (cols=1)
*
*	Return: (float *) pointing to matrix/vector
*/
float* readfile(char* filename, int rows, int cols) {

	FILE* mat_file;

	//initialize return
	float* retVal = initializeMatrix(rows, cols);

	//open file, ensure it opened
	mat_file = fopen(filename, "rb");
	if (mat_file == NULL) {
		printf("error: invalid filename \"%s\"\n", filename);
		exit(-1);
	}

	//read data into vector
	fread((void*)retVal, sizeof(float), rows * cols, mat_file);

	//close file
	fclose(mat_file);

	return retVal;
}

int main(int argc, char* argv[])
{
	
	printf("\n DOT PRODUCT \n");
	if (argc < 2) {
		//argc[1] is the filename from commandline
		printf("error: need two arguments for raw matrix filenames\n");
		exit(-1);
	}

	// indices for the loop or the matrices
	int i, j, k, used_matrix_size, a;

	//variables to calculate the computation indices
	double ioStart, ioEnd, execStart, execEnd;

	// storing the values for different sizes of matrices
	int sizeofMAT[storage] = { 0 };
	double ioTime[storage], execTime[storage];
	double MB[storage], FLOP[storage];

	//variables to read the matrices and perform algebra
	float* matA;
	float* matB;
	double c[storage] = { 0 }; // c = A'(1xN) B(Nx1)

	used_matrix_size = MINSIZE; //matrix size currently in use
	a = 0; //storage size index

	while (used_matrix_size <= MAXSIZE)
	{
		//to store the matrix sizes in a matrix to view results in a systematic way
		sizeofMAT[a] = used_matrix_size;

		//flops/s = FLOP/execution time
		//MB/s = MB/ load time
		MB[a] = (8 * used_matrix_size) / pow(10, 6); //2*4*N in bytes where 2 refers to the imported vectors
		FLOP[a] = (2 * used_matrix_size) - 1; //FLOP for dot-product = 2N-1

		ioStart = omp_get_wtime(); //io read time starts
		matA = readfile(argv[1], 1, used_matrix_size);
		matB = readfile(argv[2], used_matrix_size, 1);
		ioEnd = omp_get_wtime(); //io read time ends
		ioTime[a] = ioEnd - ioStart; //total io time to read
		printf("\n total i/o read time of 1 x %d vector and %d x 1 vector = %f sec\n", used_matrix_size, used_matrix_size, ioTime[a]);

		/* From Dr. Hansen's code

			printf("total load time: %f s\n", t0);
			 printf("load speed: %f MB/s\n", MB / t0);
			 printf("element (4,4) = %f\n", matA[index(4, 4, 4096)]);

		*/

		/*
		Previously thought of calculating the transpose of a matrix

		// Finding matA transpose
		for (i = 0; i < used_matrix_size; i++)
		{
			for (j = 0; j < used_matrix_size; j++)
			{
				trans_mat[j][i] = matA[index(i, j, used_matrix_size)];
			}
		}
		*/

		//perform linear algebra here and time how long it takes

		 
		/* checking matrix elements and their specific addresses in the memory

		for (i=0; i<used_matrix_size;i++)
		{	
				printf("\n matA[0][%d] element = %f\n and address = %p\n",i,matA[index(0,i,used_matrix_size)], (void *)&matA[(index(0,i,used_matrix_size))]);
				printf("matB[%d][0] element = %f\n and address = %p\n",i,matB[index(i,0,1)], (void *)&matB[(index(i,0,1))]);
		}
		*/
		
		execStart = omp_get_wtime(); // execution time starts
		for (i = 0; i < used_matrix_size; i++)
		{
			c[a] += matA[index(0, i, used_matrix_size)] * matB[index(i, 0, 1)]; //c for each of the used_matrix_size
		}
		
		execEnd = omp_get_wtime(); // execution time ends
		execTime[a] = execEnd - execStart; // total execution time
		printf("total execution time of dot product of two vectors = %f sec\n", execTime[a]);
		used_matrix_size = used_matrix_size * 2;
		a++;
	}

	
	/*
	printing the scalar product for each of the matrix sizes
	for (i = 0; i < storage; i++)
	{
		printf("\n value of c[%d] = %f \n", i, c[i]);
	}
	*/
	
	for (i = 0; i < storage; i++)
	{
		printf("\n Matrix size: %d \n MBps: %f \n FLOPs = %f \n ", sizeofMAT[i], (MB[i] / ioTime[i]), (FLOP[i] / execTime[i]));
	}
}
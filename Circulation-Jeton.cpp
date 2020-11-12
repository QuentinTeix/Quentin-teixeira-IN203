# include <iostream>
# include <cstdlib>
# include <mpi.h>

int main( int nargs, char* argv[] )
{	
	int myvar = 10;
	MPI_Init( &nargs, &argv );

	MPI_Comm globComm;
	MPI_Comm_dup(MPI_COMM_WORLD, &globComm);

	int nbp;
	MPI_Comm_size(globComm, &nbp);

	int rank;
	MPI_Comm_rank(globComm, &rank);


	std::cout << "Hello World, I'm processus " << rank << " on " << nbp-1 << " processes.\n";

	MPI_Status status;

	int task = rank;
	if (rank>0)
	{
		MPI_Recv(&myvar, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD, &status);
	}
	if (rank!=nbp-1)
	{
		myvar += 1;
		std::cout << "I'm processus " << rank << " on " << nbp-1 << " processes.\n";
		std::cout << "MyVar is : " << myvar << "\n";
		MPI_Send(&myvar, 1, MPI_INT, task+1, 0, MPI_COMM_WORLD);
	}
	else
	{
		myvar += 1;
		std::cout << "I'm processus " << rank << " on " << nbp-1 << " processes.\n";
		std::cout << "MyVar is : " << myvar << "\n";
		MPI_Send(&myvar, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}
	if (rank==0)
	{
		MPI_Recv(&myvar, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD, &status);
		myvar += 1;
		std::cout << "I'm processus " << rank << " on " << nbp-1 << " processes.\n";
		std::cout << "MyVar is : " << myvar << "\n";
	}

	MPI_Finalize();
	return EXIT_SUCCESS;
}

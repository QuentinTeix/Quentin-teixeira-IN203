# include <iostream>
# include <cstdlib>
# include <mpi.h>

int main( int nargs, char* argv[] )
{	
	int myvar;
	MPI_Init( &nargs, &argv );

	MPI_Comm globComm;
	MPI_Comm_dup(MPI_COMM_WORLD, &globComm);

	int nbp;
	MPI_Comm_size(globComm, &nbp);

	int rank;
	MPI_Comm_rank(globComm, &rank);

	std::cout << "Processus " << rank << " awake.\n";

	MPI_Status status;

	int tab[nbp];
	for (int i=0; i<nbp; i++) tab[i]=2*i; //Tableau au choix du programmeur
	
	int task = rank;
	int jeton = tab[rank];
	
	if (rank!=nbp-1 && rank!=0)
	{	
		MPI_Send(&jeton, 1, MPI_INT, task+1, 0, MPI_COMM_WORLD);
		MPI_Recv(&myvar, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD, &status);
		myvar += 1;
		std::cout << "I'm processus " << rank << " on " << nbp-1 << " processes. ";
		std::cout << "I received : " << myvar << "\n";
		
	}
	else if (rank == nbp-1)
	{
		MPI_Send(&jeton, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
		MPI_Recv(&myvar, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD, &status);
		myvar += 1;
		std::cout << "I'm processus " << rank << " on " << nbp-1 << " processes. ";
		std::cout << "I received : " << myvar << "\n";
		
	}
	else if (rank==0)
	{
		MPI_Send(&jeton, 1, MPI_INT, task+1, 0, MPI_COMM_WORLD);
		MPI_Recv(&myvar, 1, MPI_INT, nbp-1, 0, MPI_COMM_WORLD, &status);
		myvar += 1;
		std::cout << "I'm processus " << rank << " on " << nbp-1 << " processes. ";
		std::cout << "I received : " << myvar << "\n";
	}
	/*if (rank>0)
	{
		MPI_Recv(&myvar, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD, &status);
	}*/
	MPI_Finalize();
	return EXIT_SUCCESS;
}

# include <chrono>
# include <random>
# include <iostream>
# include <cstdlib>
# include <mpi.h>
#include <time.h>

time_t timer = time(NULL);

int nb = 10000000;

// Attention , ne marche qu'en C++ 11 ou supérieur :
double approximate_pi( unsigned long nbSamples , int rank)
{
    typedef std::chrono::high_resolution_clock myclock;
    myclock::time_point beginning = myclock::now();
    myclock::duration d = myclock::now() - beginning;
    unsigned seed = d.count();
    std::default_random_engine generator(seed);
    std::uniform_real_distribution <double > distribution( -1.0 ,1.0);
    unsigned long nbDarts = 0;

    // Throw nbSamples darts in the unit square [ -1:1] x [ -1:1]
    for ( unsigned sample = 0; sample < nbSamples; ++sample )
    {
        double x = distribution(generator);
        double y = distribution(generator);

        // Test if the dart is in the unit disk
        int var = 0;
        if ( x*x+y*y<=1 )
        {
            nbDarts ++;
            var = 1;
        }
        MPI_Send(&var, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        //MPI_Isend(&var, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &reqs);
    }

    // Number of nbDarts throwed in the unit disk
    double ratio = double(nbDarts)/double(nbSamples);
    return ratio;
}

int main( int nargs, char* argv[] )
{
	MPI_Init( &nargs, &argv );
	MPI_Comm globComm;
	MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
	int nbp;
	MPI_Comm_size(globComm, &nbp);
	int rank;
	MPI_Comm_rank(globComm, &rank);
    MPI_Status status;
	std::cout << "Processus " << rank << " awake.\n";

    MPI_Request reqs; // required variable for non-blocking calls
    MPI_Status stats[1]; // required variable for Waitall routine

    unsigned long nbEx = (unsigned long)nb/(nbp-1);

    int nbDartTotal = 0;

    if (rank!=0)
    {
        double ratio = approximate_pi(nbEx, rank);
    }
    else
    {
        int var;
        for (int i=0; i<nbEx; i++)
        {
            for (int j=1; j<nbp; j++)
            {
                MPI_Recv(&var, 1, MPI_INT, j, 0, MPI_COMM_WORLD, &status);
                //MPI_Irecv(&var, 1, MPI_INT, j, 0, MPI_COMM_WORLD, &reqs);
                //std::cout << "p" << j << " : " << var << " Received\n";
                nbDartTotal += var;
            }   
        }
        double ratio = double(nbDartTotal)/double(nb);
        double pi = ratio*4;
        std::cout << "Approximation de pi : " << pi << std::endl;
    }
    MPI_Finalize();

    std::cout << "Time : " << time(NULL) - timer << std::endl;
    
	return EXIT_SUCCESS;
}
# include <cassert>
# include <vector>
# include <iostream>
# include <mpi.h>
# include "matvec.cpp"

int main( int nargs, char* argv[] )
{
    MPI_Init( &nargs, &argv );
    MPI_Comm globComm;
    MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
    int nbp;
    MPI_Comm_size(globComm, &nbp);
    int rank;
    MPI_Comm_rank(globComm, &rank);

    const int N = 120;
    Matrix A(N);
    //std::cout  << "A : " << A << std::endl;
    std::vector<double> u( N );
    for ( int i = 0; i < N; ++i ) u[i] = i+1;
    //std::cout << " u : " << u << std::endl;
    //std::vector<double> v = A*u;

    int Nloc = N/nbp;
    std::vector<double> resultat(N);
    for (int i =  rank*Nloc; i<(rank+1)*Nloc; i++)
    {
        std::vector<double> res(N);
        for (int j=0; j<N; j)
            res[j] = u[i] * A(i,j);
        MPI_Reduce (res.data(), resultat.data(), N*Nloc, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );
    }

    std::cout << "A.u = " << resultat << std::endl;
    return EXIT_SUCCESS;
}
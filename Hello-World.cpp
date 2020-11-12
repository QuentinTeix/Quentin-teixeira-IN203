# include <iostream>
# include <string>
# include <cstdlib>
# include <iostream>
# include <mpi.h>
# include <iomanip>
# include <sstream>
# include <fstream>

int main( int nargs, char* argv[] )
{
    MPI_Init(&nargs, &argv);
    int numero_du_processus, nombre_de_processus;

    MPI_Comm_rank(MPI_COMM_WORLD,
                  &numero_du_processus);
    MPI_Comm_size(MPI_COMM_WORLD, 
                  &nombre_de_processus);
    std::cout << "Bonjour, je suis la tâche n° " 
              << numero_du_processus << " sur "
              << nombre_de_processus << " tâches" 
              << std::endl;
 
    MPI_Comm globComm;
    MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
    int nbp;
    MPI_Comm_size(globComm, &nbp);
    int rank;
    MPI_Comm_rank(globComm, &rank);
    std::stringstream fileName;
    fileName << "Output" << std::setfill('0') << std::setw(5) << rank << ".txt";
    std::ofstream output( fileName.str().c_str() );

    output << "Bonjour, je suis la tâche n° " << rank << " sur " << nbp << " tâches." << std::endl;

    output.close();
    MPI_Finalize();
    return EXIT_SUCCESS;
}

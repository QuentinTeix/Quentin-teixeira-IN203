#include <cstdlib>
#include <string>
#include <iostream>
#include <SDL2/SDL.h>        
#include <SDL2/SDL_image.h>
#include <fstream>
#include <ctime>
#include <iomanip>      // std::setw
#include <chrono>
#include <omp.h>

//#include <mpi.h>

#include "parametres.hpp"
#include "galaxie.hpp"
 
int main(int argc, char ** argv)
{
  /* Version mémoire distribuée 
  MPI_Init( &argc, &argv );
  MPI_Comm globComm;
  MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
  int nbp;
  MPI_Comm_size(globComm, &nbp);
  int rank;
  MPI_Comm_rank(globComm, &rank);
  MPI_Status status;
  //MPI_Request reqs[nbp-1];
  

  char commentaire[4096];
  int width, height;
  SDL_Event event;
  SDL_Window   * window;

  parametres param;


  std::ifstream fich("parametre.txt");
  fich >> width;
  fich.getline(commentaire, 4096);
  fich >> height;
  fich.getline(commentaire, 4096);
  fich >> param.apparition_civ;
  fich.getline(commentaire, 4096);
  fich >> param.disparition;
  fich.getline(commentaire, 4096);
  fich >> param.expansion;
  fich.getline(commentaire, 4096);
  fich >> param.inhabitable;
  fich.getline(commentaire, 4096);
  fich.close();

  int deltaT;
  int h = (int)(height/(nbp-1));
  int new_height;
  unsigned long long temps = 0;
  std::chrono::time_point<std::chrono::system_clock> start, end, endbis;

  // RANK 0
  if (rank==0)
  {
    std::cout << "Resume des parametres (proba par pas de temps): " << std::endl;
    std::cout << "\t Chance apparition civilisation techno : " << param.apparition_civ << std::endl;
    std::cout << "\t Chance disparition civilisation techno: " << param.disparition << std::endl;
    std::cout << "\t Chance expansion : " << param.expansion << std::endl;
    std::cout << "\t Chance inhabitable : " << param.inhabitable << std::endl;
    std::cout << "Proba minimale prise en compte : " << 1./RAND_MAX << std::endl;
    std::srand(std::time(nullptr));

    SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);

    window = SDL_CreateWindow("Galaxie", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              width, height, SDL_WINDOW_SHOWN);

    deltaT = (20*52840)/width;
    std::cout << "Pas de temps : " << deltaT << " années" << std::endl;

    std::cout << std::endl;

    // Initialisation de l'affichage
    galaxie g(width, height, param.apparition_civ);
    galaxie_renderer gr(window);
    gr.render(g);
    std::cout << "Affichage initié !\n";

    while (1)
    {
      start = std::chrono::system_clock::now();
      MPI_Request reqs[nbp-1];
      for (int i=1; i<nbp; i++)
      {
        MPI_Irecv(g.data()+(i-1)*h*width, width*h, MPI_BYTE, i, 0, MPI_COMM_WORLD, &reqs[i-1]);
        //std::cout << "En attente des données du processus " << i << std::endl;
      }
      MPI_Waitall(nbp-1, reqs, MPI_STATUSES_IGNORE);
      //std::cout << "Attente des donnnées terminée\n";

      endbis = std::chrono::system_clock::now();
      
      gr.render(g);
      end = std::chrono::system_clock::now();
      std::chrono::duration<double> elaps = end-start;
      std::chrono::duration<double> elapsbis = endbis-start;

      temps += deltaT;
      std::cout << "Temps passe : "
                << std::setw(10) << temps << " années"
                << std::fixed << std::setprecision(3)
                << " | Time for getting data : " << elapsbis.count()*1000 << " ms."
                << " | Time for a loop : " << elaps.count()*1000 << " ms."
                << "\r" << std::flush;
      //_sleep(1000);
      if (SDL_PollEvent(&event) && event.type == SDL_QUIT) {
        std::cout << std::endl << "The end" << std::endl;
        break;
      }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
  }

  //RANK 1
  else if (rank==1)
  {
    //Petite attente pour être sûr que processus 0 soit prêt...
    time_t t = time(0);
    time_t t1 = time(0);
    while (t1-t<1) t1 = time(0);

    //Initialisation
    //std::cout << "Processus 1 ready\n";
    new_height = h+1;
    galaxie g(width, new_height, param.apparition_civ);
    //std::cout << "galaxie g initiée\n";
    galaxie g_next(width, new_height);
    //std::cout << "galaxie g_next initiée\n";
    MPI_Send(g.data()+width*(new_height-2), 2*width, MPI_BYTE, 2, 0, MPI_COMM_WORLD);
    //std::cout << "Données envoyées !\n";

    while (1)
    {
      start = std::chrono::system_clock::now();
      //MAJ
      mise_a_jour(param, width, new_height, g.data(), g_next.data());
      //std::cout << "processus 1 mis à jour !\n";
      g_next.swap(g);

      end = std::chrono::system_clock::now();
      std::chrono::duration<double> elaps = end-start;
      std::cout << "temps de calcul rank 1 = " << elaps.count()*1000 << std::endl;

      // Envoi des données au processus 0
      MPI_Send(g.data(), width*h, MPI_BYTE, 0, 0, MPI_COMM_WORLD);

      //Envoie des données aux autres processus pour mettre à jour les bords
      MPI_Send(g.data()+width*(new_height-2), width, MPI_BYTE, 2, 0, MPI_COMM_WORLD);
      //MPI_Irecv(g.data()+width*(new_height-1), width, MPI_BYTE, 2, 0, MPI_COMM_WORLD, &reqs[rank]);
      MPI_Recv(g.data()+width*(new_height-1), width, MPI_BYTE, 2, 0, MPI_COMM_WORLD, &status);
    }
    
  }

  //RANK 2 -> (nbp-2)
  else if (rank != nbp-1)
  {
    //Petite attente pour être sûr que processus 0 soit prêt...
    time_t t = time(0);
    time_t t1 = time(0);
    while (t1-t<1) t1 = time(0);

    //Initialisation
    new_height = h+2;
    galaxie g(width, new_height, param.apparition_civ);
    galaxie g_next(width, new_height);
    //MPI_Irecv(g.data(), 2*width, MPI_BYTE, rank-1, 0, MPI_COMM_WORLD, &reqs[nbp]);
    MPI_Recv(g.data(), 2*width, MPI_BYTE, rank-1, 0, MPI_COMM_WORLD, &status);
    MPI_Send(g.data()+width*(new_height-2), 2*width, MPI_BYTE, rank+1, 0, MPI_COMM_WORLD);

    while (1)
    {
      start = std::chrono::system_clock::now();
      //MAJ
      mise_a_jour(param, width, new_height, g.data(), g_next.data());
      g_next.swap(g);

      //end = std::chrono::system_clock::now();
      //std::chrono::duration<double> elaps = end-start;
      //std::cout << "temps d'execution rank 2 = " << elaps.count()*1000 << std::endl;

      // Envoi des données au processus 0
      MPI_Send(g.data()+width, width*h, MPI_BYTE, 0, 0, MPI_COMM_WORLD);

      //Envoie des données aux autres processus pour mettre à jour les bords
      //MPI_Irecv(g.data(), width, MPI_BYTE, rank-1, 0, MPI_COMM_WORLD, &reqs[rank]);
      MPI_Recv(g.data(), width, MPI_BYTE, rank-1, 0, MPI_COMM_WORLD, &status);
      MPI_Send(g.data()+width, width, MPI_BYTE, rank-1, 0, MPI_COMM_WORLD);

      MPI_Send(g.data()+width*(new_height-2), width, MPI_BYTE, rank+1, 0, MPI_COMM_WORLD);
      //MPI_Irecv(g.data()+width*(new_height-1), width, MPI_BYTE, rank+1, 0, MPI_COMM_WORLD, &reqs[rank]);
      MPI_Recv(g.data()+width*(new_height-1), width, MPI_BYTE, rank+1, 0, MPI_COMM_WORLD, &status);

      //end = std::chrono::system_clock::now();
      //elaps = end-start;
      //std::cout << "Temps calcul + envoi du processus 2 =" << elaps.count()*1000 << std::endl;
    }
  }

  //RANK nbp-1
  else
  {
    //Petite attente pour être sûr que processus 0 soit prêt...
    time_t t = time(0);
    time_t t1 = time(0);
    while (t1-t<1) t1 = time(0);

    //Initialisation
    new_height = h+1;
    galaxie g(width, new_height, param.apparition_civ);
    galaxie g_next(width, new_height);
    //MPI_Irecv(g.data(), 2*width, MPI_BYTE, nbp-2, 0, MPI_COMM_WORLD, &reqs[nbp]);
    MPI_Recv(g.data(), 2*width, MPI_BYTE, nbp-2, 0, MPI_COMM_WORLD, &status);

    while (1)
    {
      start = std::chrono::system_clock::now();
      //MAJ
      mise_a_jour(param, width, new_height, g.data(), g_next.data());
      g_next.swap(g);

      //end = std::chrono::system_clock::now();
      //std::chrono::duration<double> elaps = end-start;
      //std::cout << "temps d'execution rank 3 = " << elaps.count()*1000 << std::endl;

      // Envoi des données au processus 0
      MPI_Send(g.data()+width, width*h, MPI_BYTE, 0, 0, MPI_COMM_WORLD);

      //Envoie des données aux autres processus pour mettre à jour les bords
      //MPI_Irecv(g.data(), width, MPI_BYTE, nbp-2, 0, MPI_COMM_WORLD, &reqs[rank]);
      MPI_Recv(g.data(), width, MPI_BYTE, nbp-2, 0, MPI_COMM_WORLD, &status);
      MPI_Send(g.data()+width, width, MPI_BYTE, nbp-2, 0, MPI_COMM_WORLD);
    }
  }
  */


////////////////////////
/* Version multiThread
////////////////////////

  char commentaire[4096];
  int width, height;
  SDL_Event event;
  SDL_Window   * window;

  parametres param;


  std::ifstream fich("parametre.txt");
  fich >> width;
  fich.getline(commentaire, 4096);
  fich >> height;
  fich.getline(commentaire, 4096);
  fich >> param.apparition_civ;
  fich.getline(commentaire, 4096);
  fich >> param.disparition;
  fich.getline(commentaire, 4096);
  fich >> param.expansion;
  fich.getline(commentaire, 4096);
  fich >> param.inhabitable;
  fich.getline(commentaire, 4096);
  fich.close();

  std::cout << "Resume des parametres (proba par pas de temps): " << std::endl;
  std::cout << "\t Chance apparition civilisation techno : " << param.apparition_civ << std::endl;
  std::cout << "\t Chance disparition civilisation techno: " << param.disparition << std::endl;
  std::cout << "\t Chance expansion : " << param.expansion << std::endl;
  std::cout << "\t Chance inhabitable : " << param.inhabitable << std::endl;
  std::cout << "Proba minimale prise en compte : " << 1./RAND_MAX << std::endl;
  std::srand(std::time(nullptr));

  SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);

  window = SDL_CreateWindow("Galaxie", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            width, height, SDL_WINDOW_SHOWN);

  galaxie g(width, height, param.apparition_civ);
  galaxie g_next(width, height);
  galaxie_renderer gr(window);

  int deltaT = (20*52840)/width;
  std::cout << "Pas de temps : " << deltaT << " années" << std::endl;

  std::cout << std::endl;

  gr.render(g);
  unsigned long long temps = 0;

  std::chrono::duration<double> elaps1, elaps2, elaps3, elaps;
  std::chrono::time_point<std::chrono::system_clock> start, end1, end2, end;

  int i_loop = 0;

  while (1)
  {
    start = std::chrono::system_clock::now();
    # pragma omp parallel
    {//on utilisera seulement 2 threads
      //# pragma omp master
      //start = std::chrono::system_clock::now();
      # pragma omp sections nowait
      {
        
        # pragma omp section
        {
          mise_a_jour(param, width, height, g.data(), g_next.data());
          end1 = std::chrono::system_clock::now();
        }

        # pragma omp section
        {
          g_next.swap(g);
          gr.render(g);
          end2 = std::chrono::system_clock::now();
        }
      }
    }
    end = std::chrono::system_clock::now();
      
      elaps1 = end1 - start;
      elaps2 = end2 - start;
      elaps3 = end - start;
      elaps += elaps3;

      temps += deltaT;
      std::cout << "Temps passe : "
                << std::setw(10) << temps << " années"
                << std::fixed << std::setprecision(3)
                << "  " << "|  CPU(ms) : calcul " << elaps1.count()*1000
                << "  " << "affichage " << elaps2.count()*1000
                << " | Time for a loop : " << elaps3.count()*1000
                << "\r" << std::flush;
      //_sleep(1000);
      if (SDL_PollEvent(&event) && event.type == SDL_QUIT) {
        std::cout << std::endl << "Average time for a loop = " << elaps.count()*1000/i_loop << std::endl << "The end.\n";
        break;
        }
      i_loop++;
  }

  SDL_DestroyWindow(window);
  SDL_Quit();
  */
  

/////////////////////
/* Version de base */
////////////////////

  char commentaire[4096];
  int width, height;
  SDL_Event event;
  SDL_Window   * window;

  parametres param;


  std::ifstream fich("parametre.txt");
  fich >> width;
  fich.getline(commentaire, 4096);
  fich >> height;
  fich.getline(commentaire, 4096);
  fich >> param.apparition_civ;
  fich.getline(commentaire, 4096);
  fich >> param.disparition;
  fich.getline(commentaire, 4096);
  fich >> param.expansion;
  fich.getline(commentaire, 4096);
  fich >> param.inhabitable;
  fich.getline(commentaire, 4096);
  fich.close();

  std::cout << "Resume des parametres (proba par pas de temps): " << std::endl;
  std::cout << "\t Chance apparition civilisation techno : " << param.apparition_civ << std::endl;
  std::cout << "\t Chance disparition civilisation techno: " << param.disparition << std::endl;
  std::cout << "\t Chance expansion : " << param.expansion << std::endl;
  std::cout << "\t Chance inhabitable : " << param.inhabitable << std::endl;
  std::cout << "Proba minimale prise en compte : " << 1./RAND_MAX << std::endl;
  std::srand(std::time(nullptr));

  SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);

  window = SDL_CreateWindow("Galaxie", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            width, height, SDL_WINDOW_SHOWN);

  galaxie g(width, height, param.apparition_civ);
  galaxie g_next(width, height);
  galaxie_renderer gr(window);

  int deltaT = (20*52840)/width;
  std::cout << "Pas de temps : " << deltaT << " années" << std::endl;

  std::cout << std::endl;

  gr.render(g);
  unsigned long long temps = 0;

  std::chrono::time_point<std::chrono::system_clock> start, end1, end2;
  while (1) {
      start = std::chrono::system_clock::now();
      mise_a_jour(param, width, height, g.data(), g_next.data());
      end1 = std::chrono::system_clock::now();
      g_next.swap(g);
      gr.render(g);
      end2 = std::chrono::system_clock::now();
      
      std::chrono::duration<double> elaps1 = end1 - start;
      std::chrono::duration<double> elaps2 = end2 - end1;
      
      temps += deltaT;
      std::cout << "Temps passe : "
                << std::setw(10) << temps << " années"
                << std::fixed << std::setprecision(3)
                << "  " << "|  CPU(ms) : calcul " << elaps1.count()*1000
                << "  " << "affichage " << elaps2.count()*1000
                << "\r" << std::flush;
      //_sleep(1000);
      if (SDL_PollEvent(&event) && event.type == SDL_QUIT) {
        std::cout << std::endl << "The end" << std::endl;
        break;
      }
  }
  SDL_DestroyWindow(window);
  SDL_Quit();
  

/* Version mémoire distribuée et partagée
  MPI_Init( &argc, &argv );
  MPI_Comm globComm;
  MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
  int nbp;
  MPI_Comm_size(globComm, &nbp);
  int rank;
  MPI_Comm_rank(globComm, &rank);
  MPI_Status status;
  //MPI_Request reqs[nbp-1];
  

  char commentaire[4096];
  int width, height;
  SDL_Event event;
  SDL_Window   * window;

  parametres param;


  std::ifstream fich("parametre.txt");
  fich >> width;
  fich.getline(commentaire, 4096);
  fich >> height;
  fich.getline(commentaire, 4096);
  fich >> param.apparition_civ;
  fich.getline(commentaire, 4096);
  fich >> param.disparition;
  fich.getline(commentaire, 4096);
  fich >> param.expansion;
  fich.getline(commentaire, 4096);
  fich >> param.inhabitable;
  fich.getline(commentaire, 4096);
  fich.close();

  int deltaT;
  int h = (int)(height/(nbp-1));
  int new_height;
  unsigned long long temps = 0;
  std::chrono::time_point<std::chrono::system_clock> start, end, endbis;

  // RANK 0
  if (rank==0)
  {
    std::cout << "Resume des parametres (proba par pas de temps): " << std::endl;
    std::cout << "\t Chance apparition civilisation techno : " << param.apparition_civ << std::endl;
    std::cout << "\t Chance disparition civilisation techno: " << param.disparition << std::endl;
    std::cout << "\t Chance expansion : " << param.expansion << std::endl;
    std::cout << "\t Chance inhabitable : " << param.inhabitable << std::endl;
    std::cout << "Proba minimale prise en compte : " << 1./RAND_MAX << std::endl;
    std::srand(std::time(nullptr));

    SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);

    window = SDL_CreateWindow("Galaxie", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              width, height, SDL_WINDOW_SHOWN);

    deltaT = (20*52840)/width;
    std::cout << "Pas de temps : " << deltaT << " années" << std::endl;

    std::cout << std::endl;

    // Initialisation de l'affichage
    galaxie g(width, height, param.apparition_civ);
    galaxie_renderer gr(window);
    gr.render(g);
    std::cout << "Affichage initié !\n";

    while (1)
    {
      start = std::chrono::system_clock::now();
      MPI_Request reqs[nbp-1];
      for (int i=1; i<nbp; i++)
      {
        MPI_Irecv(g.data()+(i-1)*h*width, width*h, MPI_BYTE, i, 0, MPI_COMM_WORLD, &reqs[i-1]);
        //std::cout << "En attente des données du processus " << i << std::endl;
      }
      MPI_Waitall(nbp-1, reqs, MPI_STATUSES_IGNORE);
      //std::cout << "Attente des donnnées terminée\n";

      endbis = std::chrono::system_clock::now();
      
      gr.render(g);
      end = std::chrono::system_clock::now();
      std::chrono::duration<double> elaps = end-start;
      std::chrono::duration<double> elapsbis = endbis-start;

      temps += deltaT;
      std::cout << "Temps passe : "
                << std::setw(10) << temps << " années"
                << std::fixed << std::setprecision(3)
                << " | Time for getting data : " << elapsbis.count()*1000 << " ms."
                << " | Time for a loop : " << elaps.count()*1000 << " ms."
                << "\r" << std::flush;
      //_sleep(1000);
      if (SDL_PollEvent(&event) && event.type == SDL_QUIT) {
        std::cout << std::endl << "The end" << std::endl;
        break;
      }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
  }

  //RANK 1
  else if (rank==1)
  {
    //Petite attente pour être sûr que processus 0 soit prêt...
    time_t t = time(0);
    time_t t1 = time(0);
    while (t1-t<1) t1 = time(0);

    //Initialisation
    //std::cout << "Processus 1 ready\n";
    new_height = h+1;
    galaxie g(width, new_height, param.apparition_civ);
    //std::cout << "galaxie g initiée\n";
    galaxie g_next(width, new_height);
    //std::cout << "galaxie g_next initiée\n";
    MPI_Send(g.data()+width*(new_height-2), 2*width, MPI_BYTE, 2, 0, MPI_COMM_WORLD);
    //std::cout << "Données envoyées !\n";

    while (1)
    {
      start = std::chrono::system_clock::now();
      //MAJ
      mise_a_jour(param, width, new_height, g.data(), g_next.data());
      //std::cout << "processus 1 mis à jour !\n";
      g_next.swap(g);

      //end = std::chrono::system_clock::now();
      //std::chrono::duration<double> elaps = end-start;
      //std::cout << "temps d'execution rank 1 = " << elaps.count()*1000 << std::endl;

      // Envoi des données au processus 0
      MPI_Send(g.data(), width*h, MPI_BYTE, 0, 0, MPI_COMM_WORLD);

      //Envoie des données aux autres processus pour mettre à jour les bords
      MPI_Send(g.data()+width*(new_height-2), width, MPI_BYTE, 2, 0, MPI_COMM_WORLD);
      //MPI_Irecv(g.data()+width*(new_height-1), width, MPI_BYTE, 2, 0, MPI_COMM_WORLD, &reqs[rank]);
      MPI_Recv(g.data()+width*(new_height-1), width, MPI_BYTE, 2, 0, MPI_COMM_WORLD, &status);
    }
    
  }

  //RANK 2 -> (nbp-2)
  else if (rank != nbp-1)
  {
    //Petite attente pour être sûr que processus 0 soit prêt...
    time_t t = time(0);
    time_t t1 = time(0);
    while (t1-t<1) t1 = time(0);

    //Initialisation
    new_height = h+2;
    galaxie g(width, new_height, param.apparition_civ);
    galaxie g_next(width, new_height);
    //MPI_Irecv(g.data(), 2*width, MPI_BYTE, rank-1, 0, MPI_COMM_WORLD, &reqs[nbp]);
    MPI_Recv(g.data(), 2*width, MPI_BYTE, rank-1, 0, MPI_COMM_WORLD, &status);
    MPI_Send(g.data()+width*(new_height-2), 2*width, MPI_BYTE, rank+1, 0, MPI_COMM_WORLD);

    while (1)
    {
      start = std::chrono::system_clock::now();
      //MAJ
      mise_a_jour(param, width, new_height, g.data(), g_next.data());
      g_next.swap(g);

      //end = std::chrono::system_clock::now();
      //std::chrono::duration<double> elaps = end-start;
      //std::cout << "temps d'execution rank 2 = " << elaps.count()*1000 << std::endl;

      // Envoi des données au processus 0
      MPI_Send(g.data()+width, width*h, MPI_BYTE, 0, 0, MPI_COMM_WORLD);

      //Envoie des données aux autres processus pour mettre à jour les bords
      //MPI_Irecv(g.data(), width, MPI_BYTE, rank-1, 0, MPI_COMM_WORLD, &reqs[rank]);
      MPI_Recv(g.data(), width, MPI_BYTE, rank-1, 0, MPI_COMM_WORLD, &status);
      MPI_Send(g.data()+width, width, MPI_BYTE, rank-1, 0, MPI_COMM_WORLD);

      MPI_Send(g.data()+width*(new_height-2), width, MPI_BYTE, rank+1, 0, MPI_COMM_WORLD);
      //MPI_Irecv(g.data()+width*(new_height-1), width, MPI_BYTE, rank+1, 0, MPI_COMM_WORLD, &reqs[rank]);
      MPI_Recv(g.data()+width*(new_height-1), width, MPI_BYTE, rank+1, 0, MPI_COMM_WORLD, &status);

      //end = std::chrono::system_clock::now();
      //elaps = end-start;
      //std::cout << "Temps calcul + envoi du processus 2 =" << elaps.count()*1000 << std::endl;
    }
  }

  //RANK nbp-1
  else
  {
    //Petite attente pour être sûr que processus 0 soit prêt...
    time_t t = time(0);
    time_t t1 = time(0);
    while (t1-t<1) t1 = time(0);

    //Initialisation
    new_height = h+1;
    galaxie g(width, new_height, param.apparition_civ);
    galaxie g_next(width, new_height);
    //MPI_Irecv(g.data(), 2*width, MPI_BYTE, nbp-2, 0, MPI_COMM_WORLD, &reqs[nbp]);
    MPI_Recv(g.data(), 2*width, MPI_BYTE, nbp-2, 0, MPI_COMM_WORLD, &status);

    while (1)
    {
      start = std::chrono::system_clock::now();
      //MAJ
      mise_a_jour(param, width, new_height, g.data(), g_next.data());
      g_next.swap(g);

      //end = std::chrono::system_clock::now();
      //std::chrono::duration<double> elaps = end-start;
      //std::cout << "temps d'execution rank 3 = " << elaps.count()*1000 << std::endl;

      // Envoi des données au processus 0
      MPI_Send(g.data()+width, width*h, MPI_BYTE, 0, 0, MPI_COMM_WORLD);

      //Envoie des données aux autres processus pour mettre à jour les bords
      //MPI_Irecv(g.data(), width, MPI_BYTE, nbp-2, 0, MPI_COMM_WORLD, &reqs[rank]);
      MPI_Recv(g.data(), width, MPI_BYTE, nbp-2, 0, MPI_COMM_WORLD, &status);
      MPI_Send(g.data()+width, width, MPI_BYTE, nbp-2, 0, MPI_COMM_WORLD);
    }
  }
  */
  
  return EXIT_SUCCESS;
}

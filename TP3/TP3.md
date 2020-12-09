## TP3 de Quentin Teixeira

# Exercice 1
    Avant de faire la parallèlisation, on avait:
        1.88s pour l'assemblage
        0.599s pour produit scalaire
        0s pour la sommation
    
    Après parallèlisation, on obtient toujours
    un temps de sommation quasi nul, et :
    Pour 2 threads: 1.49s assemblage
                    0.30s de produit scalaire
    Pour 3 threads: 1.20s assemblage
                    0.23s de produit scalaire
    Pour 4 threads: 1.16s assemblage
                    0.22s de produit scalaire
    Pour 5 threads: 1.39s assemblage
                    0.19s de produit scalaire
    Pour 6 threads: 1.64s assemblage
                    0.17s de produit scalaire
    Pour 10 threads:1.69s assemblage
                    0.15s de produit scalaire
    
    A partir de 11 thread le temps nécessaire pour effectuer le produit scalaire augmente.
    On voit aussi que le temps nécessaire pour faire l'initialisation des vecteurs
    diminue jusqu'à ce que l'on utilise plus de 4 threads.

    On obtinet une accélération maximale de 0.6/0.15
    soit 4x plus rapide pour 10 threads.
    
    Avec le code dotproduct_thread,
    le temps d'exécution du produit scalire ne
    descend pas en dessus de la seconde,
    donc notre code est le plus rapide.
    Je ne suis pas sûr d'avoir compris comment fonctionne dotproduct_thread, mais je ne vois pas comment améliorer notre code de toute façon.

# Exercice 2

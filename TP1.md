#### Exercice 1 : (Hello World)
    C'est plus clair de mettre la sortie en fichier texte parce que cela apparaît dans l'ordre, chaque processus avec son propre nom de fichier (trié par ordre alphabétique dans les dossiers) alors qu'en sortie du terminal tout sort dans l'ordre d'exécution, soit dans le désordre.

#### Exercice 2 : (Envoie bloquant ou non bloquant)
    L'envoi bloquant est plus sûr car on attend la réponse du buffer de manière automatique, alors qu'il faut le faire manuellement pour l'envoi non bloquant.

#### Exercice 3 : (Circulation de jeton)
    Premier code:
    A l'exécution, les messages sont affichés dans l'ordre des numéros de procéssus car chaque processsus attend que son précédent ait finis sa tâche avant de faire la sienne. En effet chaque processus (sauf le numéro 0) commence par recevoir des données du processus suivant.

    Deuxième code:
    L'ordre d'affichage change à chaque fois que j'exécute le code. L'ordre a l'air plutôt aléatoire. C'est parce que tous les processus peuvent travailler en même temps. En effet, ils commencent tous par géréner un entier et l'envoyer, donc ils peuvent faire ça tous en même temps. Les premiers à afficher un message sont ceux donc le prédécesseur a été le plus rapide à envoyer son entier.

#### Exercice 4 : (calcul de pi par lancer de fléchettes)
    Avec les reception bloquante, comme seule un precessus récupère les donnée, elle sont donc envoyée une par une. Ce n'est pas vraiment efficace puisque chaque processus doit attendre que sa donnée est reçu avant de poursuivre son traitement.
    Pour 10000000 fléchettes, cela prend 5 secondes evirons (time.h n'est pas très précis...)
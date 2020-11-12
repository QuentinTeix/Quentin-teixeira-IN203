Concernant le Hello-World.cpp (Hello World Parallèle) :
C'est plus clair de mettre la sortie en fichier texte
parce que cela apparaît dans l'ordre, chaque processus avec son propre nom de fichier
(trié par ordre alphabétique dans les dossiers) alors qu'en sortie du terminal
tout sort dans l'ordre d'exécution, soit dans le désordre.

Envoie bloquant ou non bloquant :
L'envoi bloquant est plus sûr car on attend la réponse du buffer de manière automatique, alors qu'il faut le faire manuellement pour l'envoi non bloquant

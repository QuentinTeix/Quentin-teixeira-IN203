## TP2 de Teixeira Quentin

# Exercice 1
    Situation sans interblocage:
    Si le processus 0 envoie sa donnée au p2 avant le p1, alors le p2 peut la recevoir et envoie sa donnée au p0 ensuite, et finir le code.
    En fait, il n'y a pas blocage si p0 envoie sa donnée en 1er.

    Situation avec blocage:
    P1 envoie sa donnée avant p0, alors p2 reçoit depuis p1 et envoie sa donnée à p0, qui avait aussi envoyé quelque chose et donc attendait déjà. Et donc p0 et p2 attendent tous les deux la confirmation de reception.

    La probabilité d'un interblocage semble donc être 0.5 : soit c'est p0 soit c'est p1 en 1er.

# Exercice 2
    Selon la loi d'Amdahl pour n>>1, on  S(n>>1) = 1/f.
    Or ici 90% du code est exécuté en parallèle, donc 10% de code n'est pas parallélisable.
    On trouve donc une accélération de S(n>>1) = 10.

    On a supposé n>>1, donc on peut imaginer qu'une vingtaine de noeud devrait faire l'affaire. En reppranant la loi précédente avec n = 500 on tombe à peu près sur le même résulat.

    Avec la loi d'Amdahl, on trouve que f = 1/4 pour S(n) = 4;
    Avec la loi de Gustafson, on a S'(n) = 2(S(n)-s)+s
    On obtient finalement une accélération maximale S'(n) = 7.75

# Exercice 3 : Mandelbrot
    L'idée était de ne créer qu'un seul "pixels" au niveau du pprocessus 0,
    qui envoie le pointeur au autres processus pour que ceux ci puissent travailler directement dessus,
    mais cela n'a pas aboutit pour le moment...

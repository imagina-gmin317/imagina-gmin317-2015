# imagina-gmin317-2015
TP du cours moteur de jeu du master IMAGINA

Ce repository contient l'ensemble des documents de cours et de travaux pratiques.

Tous les étudiants sont invités à créer leur propre version de chaque sous-projet:
- tous les tp sont à rendre une semaine après la séance
- les études documentaires sont à rendre le 5 décembre 2015
- les mini-projets sont à rendre le 10 janvier 2016
- 

La fonction generatePoint() prend en parametre la taille de la grille de point que l'on veut generer. Elle retourne un tableau de flottant comportant 3 case par point (pour les coordoonées x,y,z).

La fonction getSurface() prend en parametre le nombre de point et un tableau de point. Cette fonction retourne un tableau de flottant comportant les points organisé de manière à former des triangles avec la fonction  glDrawArrays().

Pour accéder au point de la huitième ligne et de la troisième colonne dans le tableau, il faut aller à la 8*nombreDeColonne+3 case (et lire les deux prochaines cases pour avoir les coordonnées y et z).

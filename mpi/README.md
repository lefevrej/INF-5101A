# Introduction à MPI

Dans ce dossier vous trouverez le travail réalisé par Valentin FOARE et Josselin LEFÈVRE pour le TP d'introduction à MPI.

  - pi.c : calcul de pi par intégrale
  - data_par.c : distribution des données entre les nœuds et partage des lignes de recouvrement
  - laplace.c : résolution de l'équation de Laplace

### Make

```sh
$ mkdir bin
$ make mon_programme
```
Cela génère un executable /bin/mon_programme.

### Benchmark
Pour faire le Benchmark du programme laplace :
```sh
$ ./benchmark.sh
```
Ce script bash génère un répertoire pour chaque valeur de .1, 1, 10 et 100. Dans chacun de ces répertoires, vous trouverez un fichier par taille de matrice contenant les temps d’exécution pour différents nombre de nœuds.

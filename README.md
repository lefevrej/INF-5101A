# INF-5101A

Vous trouverez dans ce répertoire les travaux pratiques sur le parallélisme.
Nous avons utilisé deux librairies de passage de messages :

- PVM : décomposition de Gauss
- MPI (mpich) : approximation de pi, résolution de l'équation de Laplace

### Utils

Les programmes tels que la décomposition de Gauss et Laplace prennent des fichiers spécifiques en entrée. Ce sont des matrices carrées sous forme de fichiers textes. Pour Laplace, il est intéressant de voir l'effet du programme sur une image. Pour cela, j'ai écrit deux scripts permettant de passer d'une image à un fichier d'entrée et inversement.

Pour utiliser ces scripts Python, vous aurez besoin des librairies suivantes :
- Numpy
- PIL

Utilisation :

```python
#crée une matrice txt de 500 de côté à partir d'image
python img2mat.py image.jpg 500
#crée autant d'image "png" que de fichier mat en entrée
python mat2jpg.py output_image1 output_image2
```

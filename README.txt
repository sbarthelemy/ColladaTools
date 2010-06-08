=============
FiltreCollada
=============

FiltreCollada est une batterie de filtres et d'outils pour le traitement des fichiers Collada


Installation
==============

Pour Ubuntu
----------------

Pré-requis::

    sudo aptitude install cmake
    sudo aptitude install libcollada14dom2.1 \
    libboost-dev libxml2-dev

Construction::

    cd FiltreCollada
    mkdir build
    cd build
    cmake ..
    make

Pour Mac os X
-----------------

ne compile pas à cause de libboost

Contenu
=======

:cmake:
    Module de recherche cmake

:data:
    Exemple de données Collada et HDF5

:hdf5:
    Librairie HDF5

:src:
    Code source des test

:szip:
    Bibliothèque szip


Utilisation
============== 

1. Test de cohérence : coherencytest
2. Filtre dae
3. Transformation d'une trajectoire HDF5 en animation collada

Test de cohérence
-------------------------

Le test de cohérence est appelé grâce à la commande : 

bin/coherencytest *.dae

Il vérifie la bonne construction d'un fichier Collada.

Filtre dae
---------------------

Le filtre dae est appelé avec certaines options qui lui permettent de définir les balises que l'on souhaite supprimer.

	-novisu 	supprime les librairie : library_visual_scenes et toutes les instances de la géometrie qui lui sont liées.
	-noanim		supprime les librairie : library_animations
	-nophys		supprime les librairie : library_physics et toutes les instances de la géometrie qui lui sont liées.
	-noinfo 	supprime le noeud asset
	-o 		spécifie le nom du fichier de sortie 
	
exemple : daeFilter exemple.dae -o exemple-new.dae -nophys -noanim

Transformation d'une trajectoire HDF5 en animation collada
-----------------------------------------------------------

L'executable h5toAnim prend en argument un fichier HDF5 contenant une trajectoire, le chemin du HDF5 vers la trajectoire à l'intérieur du fichier
(/ pour root), ainsi qu'un fichier collada, et intègre l'animation au fichier collada.

exemple : h5toAnim data/exemple.h5 / data/exemple.dae

Le fichier de sortie contenant l'animation sera exemple-new.dae




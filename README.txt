============
ColladaTools
============

ColladaTools est une batterie de filtres et d'outils pour le traitement des fichiers Collada


Installation
==============

Pour Ubuntu
----------------

Pré-requis::

    sudo add-apt-repository ppa:barthelemy/collada
    sudo apt-get update
    sudo aptitude install cmake libcolladadom-dev \
    libboost-dev libboost-system-dev libboost-filesystem-dev


Construction::

    cd ColladaTools
    mkdir build
    cd build
    cmake ..
    make

Pour Mac os X
-------------

Ne compile pas à cause de libboost.

Contenu
=======

:cmake:
    Modules additionnels pour cmake

:data:
    Exemple de données Collada et HDF5

:hdf5:
    Bibliothèque HDF5

:src:
    Code source des outils

:szip:
    Bibliothèque szip


Utilisation
===========

Filtre dae
----------

Le filtre dae est appelé avec certaines options qui lui permettent de définir les balises que l'on souhaite supprimer.

        -novisu         supprime les bibliothèques : library_visual_scenes et toutes les instances de la géometrie qui lui sont liées.
        -noanim         supprime les bibliothèques : library_animations
        -nophys         supprime les bibliothèques : library_physics et toutes les instances de la géometrie qui lui sont liées.
        -noinfo         supprime le nœud asset
        -o              spécifie le nom du fichier de sortie

exemple ::
  daeFilter exemple.dae -o exemple-new.dae -nophys -noanim


Transformation d'une trajectoire HDF5 en animation collada
-----------------------------------------------------------

L'exécutable h5toAnim prend en argument un fichier HDF5 contenant une trajectoire, le chemin du HDF5 vers la trajectoire à l'intérieur du fichier
(/ pour root), ainsi qu'un fichier collada, et intègre l'animation au fichier collada.

exemple ::
  h5toAnim data/exemple.h5 / data/exemple.dae

Le fichier de sortie contenant l'animation sera exemple-new.dae

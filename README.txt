
ColladaTools
============

ColladaTools is a set of tools and filters useful to create and process
collada (.dae) files.

Install
=======

TODO: create & publish binary packages and explain how to use them.


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

L'exécutable h5toAnim prend en argument un fichier HDF5 contenant l'ensemble
des trajectoires correpondant à une animation, ainsi qu'un fichier collada
décrivant une scène, et combine les deux pour créer un nouveau fichier
collada contenant la scène animée.

exemple ::
  h5toAnim data/exemple.h5 / data/exemple.dae

Le fichier de sortie contenant l'animation sera exemple-new.dae


Contenu
=======

:cmake:
    Modules additionnels pour cmake

:data:
    Exemple de données Collada et HDF5

:src:
    Code source des outils


Build from source
=================

Prerequisites
-------------

On Ubuntu lucid lynx ::

    sudo add-apt-repository ppa:barthelemy/collada
    sudo apt-get update
    sudo aptitude install cmake libcolladadom-dev \
    libboost-dev libboost-system-dev libboost-filesystem-dev


On Mac os X, with `macports <http://www.macports.org/>`_::

    sudo port install cmake hdf5-18 collada-dom #FIXME: add boost

FIXME: add Windows details

Build
-----

::

    cd ColladaTools
    mkdir build
    cd build
    cmake ..
    make


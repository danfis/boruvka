FermatCD - Collision Detection 
===============================

Introduction
-------------
**FermatCD** is library for collision detection between arbitrary objects.
FermatCD is being developed by `Daniel Fiser <http://www.danfis.cz>`_
and it is part of the **fermat** library.

FermatCD is fast, flexible and robust and it enables to perform a collision
detection between arbitrary objects composed of several primitive types
such as box, capsule, cylinder, sphere or triangular mesh.

The features of FermatCD are:
    - Collision primitives: sphere, box, capsule, cylinder, plane
    - Its own triangular mesh implementation faster or comparable to `OPCODE <http://www.codercorner.com/Opcode.htm>`_ or `Rapid <http://gamma.cs.unc.edu/OBB/>`_
    - Minkowski Portal Refinement (MPR) algorithm for collision detection between pair of arbitrary convex shapes that is faster than commonly used GJK (Gilbert-Johnson-Keerti) algorithm
    - OBB tree structures are used for each geometry object
    - Contact persistence is available for more stable simulations
    - Sweep And Prune algorithm for broad-phase
    - Parallel collision detection using threads
    - Parallel building of triangular mesh models
    - Easy integration with `ODE <http://www.ode.org>`_


Build and Install
------------------
FermatCD is part of fermat library, so see build and install instructions
of fermat library.


Use FermatCD
-------------
This section contains several short introductionary examples explaining how
to use FermatCD.

All examples are available in *example/* director along with some test data.


Simple Collision Test
~~~~~~~~~~~~~~~~~~~~~~
In this example two geometry objects are created and simple yes/no test
whether the objects collide is performed.

.. literalinclude:: ../examples/cd-simple-collision.c
    :language: c
    :linenos:


Collision Test With Triangular Meshes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This example shows how to create triangular meshes and load geometry
objects from a file.

.. literalinclude:: ../examples/cd-trimesh-collision.c
    :language: c
    :linenos:


Separation Vector
~~~~~~~~~~~~~~~~~~
This example shows how to obtain contact points with separation vectors and
depth of penetration.

.. literalinclude:: ../examples/cd-sep.c
    :language: c
    :linenos:

FermatCD + ODE
~~~~~~~~~~~~~~~
An example how to integrate FermatCD into `ODE <http://www.ode.org>`_ can
be found in *examples/cd-ode.c* file.


FermatCD API
-------------

Parameters
~~~~~~~~~~~
.. include:: fer-cd-params.h.rst

Main Structure
~~~~~~~~~~~~~~~
.. include:: fer-cd-cd.h.rst

Geometry Objects
~~~~~~~~~~~~~~~~~
.. include:: fer-cd-geom.h.rst

Integration with ODE
~~~~~~~~~~~~~~~~~~~~~
.. include:: fer-cd-ode.h.rst


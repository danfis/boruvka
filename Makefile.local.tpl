###
# Boruvka
# --------
# Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
#
#  This file is part of Boruvka.
#
#  Distributed under the OSI-approved BSD License (the "License");
#  see accompanying file BDS-LICENSE for details or see
#  <http://www.opensource.org/licenses/bsd-license.php>.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even the
#  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#  See the License for more information.
##

###
# Template Makefile variable definition file.
#
# Makefile.local file located on a top of the source tree is included as
# first to all Makefiles. This file isn't tracked by git in the repository
# (it is named in .gitignore file) so you can modify it on your local
# machine to have correctly set up a compile chain and you can git
# push/pull at same time without any problems.
#
# If you are not sure how to set up variables type 'make help' and look at
# current settings.
#

# Turn debugging off
# DEBUG = no

# Don't use -pedantic flag in gcc command
# NOPEDANTIC = yes


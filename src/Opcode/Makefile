###
# fermat
# -------
# Copyright (c)2010-2011 Daniel Fiser <danfis@danfis.cz>
#
#  This file is part of fermat.
#
#  Distributed under the OSI-approved BSD License (the "License");
#  see accompanying file BDS-LICENSE for details or see
#  <http://www.opensource.org/licenses/bsd-license.php>.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even the
#  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#  See the License for more information.
##

-include ../../Makefile.include

CXXFLAGS += -I. -Wno-long-long

TARGETS = libopcode.a

OBJS = Ice/IceRandom.o \
	   Ice/IceSegment.o \
	   Ice/IceMatrix3x3.o \
	   Ice/IcePlane.o \
	   Ice/IceUtils.o \
	   Ice/IceRay.o \
	   Ice/IceTriangle.o \
	   Ice/IceHPoint.o \
	   Ice/IcePoint.o \
	   Ice/IceMatrix4x4.o \
	   Ice/IceOBB.o \
	   Ice/IceContainer.o \
	   Ice/IceAABB.o \
	   Ice/IceRevisitedRadix.o \
	   Ice/IceIndexedTriangle.o
OBJS += OPC_BaseModel.o \
	    OPC_RayCollider.o \
		Opcode.o \
		OPC_PlanesCollider.o \
		OPC_Common.o \
		OPC_TreeBuilders.o \
		OPC_SphereCollider.o \
		OPC_Collider.o \
		OPC_Picking.o \
		OPC_OptimizedTree.o \
		OPC_HybridModel.o \
		OPC_AABBTree.o \
		OPC_OBBCollider.o \
		OPC_TreeCollider.o \
		OPC_VolumeCollider.o \
		StdAfx.o \
		OPC_MeshInterface.o \
		OPC_Model.o \
		OPC_AABBCollider.o \
		OPC_LSSCollider.o


#OBJS 		:= $(foreach obj,$(OBJS),.objs/$(obj))

all: $(TARGETS)

libopcode.a: $(OBJS)
	ar cr $@ $(OBJS)
	ranlib $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.objs/%.o: src/%.c fermat/%.h fermat/config.h
	$(CC) $(CFLAGS) -c -o $@ $<
.objs/%.o: src/%.c fermat/config.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS)
	rm -f $(TARGETS)
	rm -f $(BIN_TARGETS)
	
check:
	$(MAKE) -C testsuites check
check-valgrind:
	$(MAKE) -C testsuites check-valgrind

python:
	$(MAKE) -C python

.PHONY: all clean check check-valgrind help

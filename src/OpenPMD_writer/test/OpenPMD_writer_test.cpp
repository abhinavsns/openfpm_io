/*
 * OpenPMD_writer_test.cpp
 *
 *  Created on: May 16, 2021
 *      Author: Pietro Incardona
 */
#include "config.h"
#ifdef HAVE_OPENPMD

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "VCluster/VCluster.hpp"

#include "Grid/grid_dist_id.hpp"
#include "Vector/vector_dist.hpp"
#include "OpenPMD_writer/OpenPMD_writer.hpp"


BOOST_AUTO_TEST_SUITE( openpmd_writer_tests )


BOOST_AUTO_TEST_CASE( openpmd_writer_test_use )
{
	size_t bc[3] = {NON_PERIODIC, NON_PERIODIC, NON_PERIODIC};

	// Domain
	Box<3,double> domain({-0.3,-0.3,-0.3},{1.0,1.0,1.0});

	Vcluster<> & v_cl = create_vcluster();

	// Ghost
	Ghost<3,long int> g(1);

    size_t sz[3] = {20,20,20};

    openfpm::vector<std::string> props({{"scalar"},{"vector"}});

    grid_dist_id<3,double,aggregate<double,double[3]>> grid(sz,domain,g);

    grid.setPropNames(props);

    auto it = grid.getDomainIterator();
    while (it.isNext())
    {
        auto key = it.get();
        auto k = it.getGKey(key);

        grid.template get<0>(key) = k.get(0) + k.get(1) + k.get(2);
        grid.template get<1>(key)[0] = k.get(0)*0.01;
        grid.template get<1>(key)[1] = k.get(1)*0.01;
        grid.template get<1>(key)[2] = k.get(2)*0.01;

        ++it;
    }

    OpenPMD_Writer<decltype(grid),OPENPMD_GRID_DIST> wr;

    wr.write(grid,"test_out.bp");
    wr.write(grid,"test_out.h5");
}

BOOST_AUTO_TEST_CASE( openpmd_writer_part_test_use )
{
	size_t bc[3] = {NON_PERIODIC, NON_PERIODIC, NON_PERIODIC};

	// Domain
	Box<3,double> domain({-0.3,-0.3,-0.3},{1.0,1.0,1.0});

	Vcluster<> & v_cl = create_vcluster();

	// Ghost
	Ghost<3,double> g(0.01);

    size_t sz[3] = {20,20,20};

    openfpm::vector<std::string> props({{"scalar"},{"vector"}});

    vector_dist<3,double,aggregate<double,double[3]>> part(0,domain,bc,g);

    part.setPropNames(props);

    auto it = part.getGridIterator(sz);
    while (it.isNext())
    {
        auto k = it.get();

        part.add();
        part.getLastPos()[0] = k.get(0)*0.01;
        part.getLastPos()[1] = k.get(1)*0.01;
        part.getLastPos()[2] = k.get(2)*0.01;
        part.template getLastProp<0>() = k.get(0) + k.get(1) + k.get(2);
        part.template getLastProp<1>()[0] = k.get(0)*0.01;
        part.template getLastProp<1>()[1] = k.get(1)*0.01;
        part.template getLastProp<1>()[2] = k.get(2)*0.01;

        ++it;
    }

    OpenPMD_Writer<decltype(part),OPENPMD_PART_DIST> wr;

    wr.write(part,"test_out_part.bp");
    wr.write(part,"test_out_part.h5");
}

BOOST_AUTO_TEST_SUITE_END()

#endif


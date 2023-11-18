#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "Grid/grid_dist_id.hpp"
#include "Vector/vector_dist_subset.hpp"
#include "Grid/map_grid.hpp"

#include "catalyst/catalyst_adaptor.hpp"

#include <boost/gil/rgb.hpp>
#include <boost/gil/extension/io/png.hpp>

#include "config.h"
#include <catalyst.hpp>

#if defined(HAVE_CATALYST)

BOOST_AUTO_TEST_SUITE(catalyst_test_suite)

// Test utilities to compare Catalyst-produced image with ground truth.
struct PixelInserter
{
    openfpm::vector<char> &storage;

    PixelInserter(openfpm::vector<char> &s) : storage(s) {}

    void operator()(boost::gil::rgb8_pixel_t p) const
    {
        storage.add(boost::gil::at_c<0>(p));
        storage.add(boost::gil::at_c<1>(p));
        storage.add(boost::gil::at_c<2>(p));
    }
};

void check_png(const std::string &img1_path, const std::string &img2_path)
{
    auto &v_cl = create_vcluster();

    if (v_cl.rank() == 0)
    {
        // Check the file has been generated (we do not check the content)
        boost::gil::rgb8_image_t img1, img2;
        // FIXME ensure both images are accessible
        boost::gil::read_image(img1_path.c_str(), img1, boost::gil::png_tag());
        boost::gil::read_image(img2_path.c_str(), img2, boost::gil::png_tag());

        openfpm::vector<char> storage1, storage2;

        for_each_pixel(const_view(img1), PixelInserter(storage1));
        for_each_pixel(const_view(img2), PixelInserter(storage2));

        BOOST_REQUIRE(storage1.size() != 0);
        BOOST_REQUIRE(storage2.size() != 0);

        BOOST_REQUIRE_EQUAL(storage1.size(), storage2.size());

        size_t diff = 0;
        for (int i = 0; i < storage1.size(); i++)
        {
            diff += abs(storage1.get(i) - storage2.get(i));
        }

        BOOST_REQUIRE(diff < 3000000);
    }
}

BOOST_AUTO_TEST_CASE(catalyst_grid_dist_test)
{
    Vcluster<> &v_cl = create_vcluster();
    size_t bc[3] = {NON_PERIODIC, NON_PERIODIC, NON_PERIODIC};
    Box<3, double> domain({-0.3, -0.3, -0.3}, {1.0, 1.0, 1.0});
    Ghost<3, long int> g(1);
    size_t sz[3] = {20, 20, 20};
    grid_dist_id<3, double, aggregate<double, double[3]>> grid(sz, domain, g);

    openfpm::vector<std::string> prop_names({{"scalar"}, {"vector"}});
    enum
    {
        SCALAR,
        VECTOR
    };
    grid.setPropNames(prop_names);

    auto it = grid.getDomainIterator();
    while (it.isNext())
    {
        auto key = it.get();
        auto k = it.getGKey(key);

        grid.template get<0>(key) = k.get(0) + k.get(1) + k.get(2);
        grid.template get<1>(key)[0] = k.get(0) * 0.01;
        grid.template get<1>(key)[1] = k.get(1) * 0.01;
        grid.template get<1>(key)[2] = k.get(2) * 0.01;

        ++it;
    }

    grid.map();
    grid.ghost_get<SCALAR, VECTOR>();
    // FIXME Cannot open vtk file in ParaView
    // Generate representative dataset to create visualization pipeline in ParaView
    //grid.write("grid_dist_test_data");

    // Visualization with Catalyst Adaptor
    // Images written to ./datasets/ folder
    catalyst_adaptor<decltype(grid), vis_props<SCALAR, VECTOR>, catalyst_adaptor_impl::GRID_DIST> adaptor;
    openfpm::vector<std::string> scripts({{"test_data/catalyst_grid_dist_test_pipeline.py"}});
    adaptor.initialize(scripts);
    adaptor.execute(grid);
    adaptor.finalize();

#ifdef HAVE_PNG
    check_png("catalyst_grid_dist.png", "test_data/catalyst_grid_dist_ground_truth.png");
#endif
}

BOOST_AUTO_TEST_CASE(catalyst_vector_dist_test)
{
    Vcluster<> &v_cl = create_vcluster();
    size_t bc[3] = {NON_PERIODIC, NON_PERIODIC, NON_PERIODIC};
    Box<3, double> domain({-0.3, -0.3, -0.3}, {1.0, 1.0, 1.0});
    Ghost<3, double> g(1);
    size_t sz[3] = {20, 20, 20};
    vector_dist_ws<3, double, aggregate<double, double[3]>> particles(0, domain, bc, g);

    openfpm::vector<std::string> prop_names({{"scalar"}, {"vector"}});
    enum
    {
        SCALAR,
        VECTOR
    };
    particles.setPropNames(prop_names);

    // Assign scalar = x + y + z, vector = {sin(x + y), cos(x + y), 0} properties to the particles
    auto it = particles.getGridIterator(sz);
    while (it.isNext())
    {
        particles.add();
        auto key = it.get();
        double x = key.get(0) * it.getSpacing(0);
        particles.getLastPos()[0] = x;
        double y = key.get(1) * it.getSpacing(1);
        particles.getLastPos()[1] = y;
        double z = key.get(2) * it.getSpacing(2);
        particles.getLastPos()[2] = z;
        particles.getLastProp<0>() = x + y + z;
        particles.getLastProp<1>()[0] = sin(x + y);
        particles.getLastProp<1>()[1] = cos(x + y);
        particles.getLastProp<1>()[2] = 0.0;

        ++it;
    }

    particles.map();
    particles.ghost_get<SCALAR, VECTOR>();
    // Generate representative dataset to create visualization pipeline in ParaView
    //particles.write("vector_dist_test_data");

    // Visualization with Catalyst Adaptor
    // Images written to ./datasets/ folder
    catalyst_adaptor<decltype(particles), vis_props<SCALAR, VECTOR>, catalyst_adaptor_impl::VECTOR_DIST> adaptor;
    openfpm::vector<std::string> scripts({{"test_data/catalyst_vector_dist_test_pipeline.py"}});
    adaptor.initialize(scripts);
    adaptor.execute(particles);
    adaptor.finalize();

#ifdef HAVE_PNG
    check_png("datasets/catalyst_vector_dist.png", "test_data/catalyst_vector_dist_ground_truth.png");
#endif
}
BOOST_AUTO_TEST_SUITE_END()

#endif
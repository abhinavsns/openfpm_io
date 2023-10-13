#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "Grid/grid_dist_id.hpp"

#include "catalyst/insitu_catalyst.hpp"

#include <boost/gil/rgb.hpp>
#include <boost/gil/extension/io/png.hpp>

#if defined(HAVE_CATALYST)

#include "Grid/map_grid.hpp"

BOOST_AUTO_TEST_SUITE( catalyst_test_suite )

/*BOOST_AUTO_TEST_CASE( catalyst_grid_test_use)
{
    size_t sz[3] = {10,10,10};

    openfpm::vector<std::string> props({"scalar"});

    grid_cpu<3,aggregate<double>> grid(sz);
    grid.setMemory();

    auto it = grid.getIterator();
    while (it.isNext())
    {
        auto k = it.get();

        grid.template get<0>(k) = k.get(0) + k.get(1) + k.get(2);

        ++it;
    }

    Box<3,int> dbox({1,1,1},{8,8,8});
    Box<3,double> cellbox({0.0,0.0,0.0},{0.1,0.1,0.1});

    Point<3,double> offset({0.5,0.5,0.5});

    insitu_viscatalyst<decltype(grid),vis_props<0>,insity_catalyst_implementation::GRID> isv;

    isv.initialize("test_data/catalyst_pipeline.py");

    for (int i = 0 ; i < 100 ; i++)
    {
        isv.execute(grid,offset,cellbox,dbox,props);
        sleep(1);
        std::cout << "STEP " << i << std::endl;
    }

    isv.finalize();
}*/

struct PixelInserter
{
        openfpm::vector<char> & storage;

        PixelInserter(openfpm::vector<char> & s) : storage(s) {}

        void operator()(boost::gil::rgb8_pixel_t p) const 
        {
                storage.add(boost::gil::at_c<0>(p));
                storage.add(boost::gil::at_c<1>(p));
                storage.add(boost::gil::at_c<2>(p));
        }
};

void check_png(const std::string & i1, const std::string & i2)
{
    auto & v_cl = create_vcluster();

    if (v_cl.rank() == 0)
    {
        // Check the file has been generated (we do not check the content)
        boost::gil::rgb8_image_t img;
        boost::gil::rgb8_image_t img2;
        boost::gil::read_image(i1.c_str(), img, boost::gil::png_tag());
        boost::gil::read_image(i2.c_str(), img2, boost::gil::png_tag());

        openfpm::vector<char> storage;
        openfpm::vector<char> storage2;

        for_each_pixel(const_view(img), PixelInserter(storage));
        for_each_pixel(const_view(img2), PixelInserter(storage2));

        BOOST_REQUIRE(storage.size() != 0);
        BOOST_REQUIRE(storage2.size() != 0);

        BOOST_REQUIRE_EQUAL(storage.size(),storage2.size());

        size_t diff = 0;
        for (int i = 0 ; i < storage.size() ; i++)
        {
            diff += abs(storage.get(i) - storage2.get(i));
        }

        BOOST_REQUIRE(diff < 3000000);
    }
}

BOOST_AUTO_TEST_CASE( catalyst_dist_test_use)
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

    insitu_viscatalyst<decltype(grid),vis_props<0,1>,insity_catalyst_implementation::GRID_DIST> isv;

    openfpm::vector<std::string> scripts({{"test_data/catalyst_pipeline_surface_scalar.py"},
                                          {"test_data/catalyst_pipeline_volume_scalar.py"},
                                          {"test_data/catalyst_pipeline_arrow.py"},
                                          {"test_data/catalyst_pipeline_isosurface.py"},
                                          {"test_data/catalyst_pipeline_streamline.py"},
                                          {"test_data/catalyst_pipeline_save_file.py"}});

    isv.initialize(scripts);

    isv.execute(grid,19);

#ifdef HAVE_PNG

    check_png("output_arr-19.png","test_data/output_arr-19.png");

#endif

    isv.finalize();
}

BOOST_AUTO_TEST_SUITE_END()


#endif
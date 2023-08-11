/*
 * VTKWriter_unit_tests.hpp
 *
 *  Created on: May 6, 2015
 *      Author: Pietro Incardona
 *
 *      Modified by Abhinav Singh May 17, 2021
 */

#ifndef VTKWRITER_UNIT_TESTS_HPP_
#define VTKWRITER_UNIT_TESTS_HPP_

#include "data_type/aggregate.hpp"
#include <random>
#include "VTKWriter.hpp"
#include "util/SimpleRNG.hpp"

BOOST_AUTO_TEST_SUITE( vtk_writer_test )

/*! \brief Sub-domain vertex graph node
 *
 */
struct vertex
{
	//! The node contain 3 unsigned long integer for communication computation memory and id
	typedef boost::fusion::vector<float,float,float,float,size_t,double,unsigned char,long int> type;

	//! type of the positional field
	typedef float s_type;

	//! Attributes name
	struct attributes
	{
		static const std::string name[];
	};

	//! The data
	type data;

	//! x
	static const unsigned int x = 0;
	//! y
	static const unsigned int y = 1;
	//! z
	static const unsigned int z = 2;
	//! property 1
	static const unsigned int prp1 = 3;
	//! property 2
	static const unsigned int prp2 = 4;
	//! property 3
	static const unsigned int prp3 = 5;
	//! property 4
	static const unsigned int prp4 = 6;
	//! property 5
	static const unsigned int prp5 = 7;

	//! total number of properties boost::fusion::vector
	static const unsigned int max_prop = 8;

	/*!
	 * Default constructor
	 *
	 */
	vertex()
	{

	}

	static inline bool noPointers()
	{
		return true;
	}

	/*! \brief Initialize the VTKVertex
	 *
	 * \param x coordinate x
	 * \param y coordinate y
	 * \param z coordinate z
	 *
	 */
	vertex(float x, float y, float z)
	{
		boost::fusion::at_c<vertex::x>(data) = x;
		boost::fusion::at_c<vertex::y>(data) = y;
		boost::fusion::at_c<vertex::z>(data) = z;
	}
};

const std::string vertex::attributes::name[] = {"x","y","z","prp1","prp2","prp3","prp4","prp5"};

struct vertex2
{
	//! The node contain 3 unsigned long integer for communication computation memory and id
	typedef boost::fusion::vector<float[3],size_t,double> type;

	//! type of the positional field
	typedef float s_type;

	//! Attributes name
	struct attributes
	{
		static const std::string name[];
	};

	//! The data
	type data;

	//! computation property id in boost::fusion::vector
	static const unsigned int x = 0;
	static const unsigned int prp1 = 1;
	static const unsigned int prp2 = 2;

	//! total number of properties boost::fusion::vector
	static const unsigned int max_prop = 3;

	/*!
	 * Default constructor
	 *
	 */
	vertex2()
	{

	}

	static inline bool noPointers()
	{
		return true;
	}

	/*! \brief Initialize the VTKVertex
	 *
	 * \param x coordinate x
	 * \param y coordinate y
	 * \param z coordinate z
	 *
	 */
	vertex2(float x, float y, float z)
	{
		boost::fusion::at_c<vertex::x>(data)[0] = x;
		boost::fusion::at_c<vertex::x>(data)[1] = y;
		boost::fusion::at_c<vertex::x>(data)[2] = z;
	}
};

// use the vertex like the edge
typedef vertex edge;

const std::string vertex2::attributes::name[] = {"x","prp1","prp2"};



BOOST_AUTO_TEST_CASE( vtk_writer_use_graph3D )
{
	Vcluster<> & v_cl = create_vcluster();

	if (v_cl.getProcessUnitID() != 0)
		return;

	// Create some graphs and output them

	// Graph

	Graph_CSR<vertex2,edge> gr;

	// Create a cube graph

	gr.addVertex(vertex2(0.0,0.0,0.0));
	gr.addVertex(vertex2(0.0,0.0,1.0));
	gr.addVertex(vertex2(0.0,1.0,0.0));
	gr.addVertex(vertex2(0.0,1.0,1.0));
	gr.addVertex(vertex2(1.0,0.0,0.0));
	gr.addVertex(vertex2(1.0,0.0,1.0));
	gr.addVertex(vertex2(1.0,1.0,0.0));
	gr.addVertex(vertex2(1.0,1.0,1.0));

	gr.addEdge(0,6);
	gr.addEdge(6,4);
	gr.addEdge(4,0);

	gr.addEdge(0,2);
	gr.addEdge(2,6);
	gr.addEdge(6,0);

	gr.addEdge(0,3);
	gr.addEdge(3,2);
	gr.addEdge(2,0);

	gr.addEdge(0,1);
	gr.addEdge(1,3);
	gr.addEdge(3,0);

	gr.addEdge(2,7);
	gr.addEdge(7,6);
	gr.addEdge(6,2);

	gr.addEdge(2,3);
	gr.addEdge(3,7);
	gr.addEdge(7,2);

	gr.addEdge(4,6);
	gr.addEdge(6,7);
	gr.addEdge(7,4);

	gr.addEdge(4,7);
	gr.addEdge(7,5);
	gr.addEdge(5,4);

	gr.addEdge(0,4);
	gr.addEdge(4,5);
	gr.addEdge(5,0);

	gr.addEdge(0,5);
	gr.addEdge(5,1);
	gr.addEdge(1,0);

	gr.addEdge(1,5);
	gr.addEdge(5,7);
	gr.addEdge(7,1);

	gr.addEdge(1,7);
	gr.addEdge(7,3);
	gr.addEdge(3,1);

	// Write the VTK file

#ifdef OPENFPM_PDATA

	if (v_cl.rank() != 0) {return;}
	std::string c2 = std::string("openfpm_io/test_data/vtk_graph_v2_test.vtk");

#else

	std::string c2 = std::string("test_data/vtk_graph_v2_test.vtk");

#endif

	VTKWriter<Graph_CSR<vertex2,edge>,VTK_GRAPH> vtk(gr);
	vtk.write("vtk_graph_v2.vtk");

	// check that match

	bool test = compare("vtk_graph_v2.vtk",c2);
	BOOST_REQUIRE_EQUAL(true,test);
}

BOOST_AUTO_TEST_CASE( vtk_writer_use_graph3D_edge )
{
	Vcluster<> & v_cl = create_vcluster();

	if (v_cl.getProcessUnitID() != 0)
		return;

	// Create some graphs and output them

	// Graph

	Graph_CSR<vertex2,vertex2> gr;

	// Create a cube graph

	gr.addVertex(vertex2(0.0,0.0,0.0));
	gr.addVertex(vertex2(0.0,0.0,1.0));
	gr.addVertex(vertex2(0.0,1.0,0.0));
	gr.addVertex(vertex2(0.0,1.0,1.0));
	gr.addVertex(vertex2(1.0,0.0,0.0));
	gr.addVertex(vertex2(1.0,0.0,1.0));
	gr.addVertex(vertex2(1.0,1.0,0.0));
	gr.addVertex(vertex2(1.0,1.0,1.0));

	gr.addEdge(0,6,vertex2(0.0,0.0,1.0));
	gr.addEdge(6,4,vertex2(0.0,0.0,1.0));
	gr.addEdge(4,0,vertex2(0.0,0.0,1.0));

	gr.addEdge(0,2,vertex2(0.0,0.0,1.0));
	gr.addEdge(2,6,vertex2(0.0,0.0,1.0));
	gr.addEdge(6,0,vertex2(0.0,0.0,1.0));

	gr.addEdge(0,3,vertex2(0.0,0.0,1.0));
	gr.addEdge(3,2,vertex2(0.0,0.0,1.0));
	gr.addEdge(2,0,vertex2(0.0,0.0,1.0));

	gr.addEdge(0,1,vertex2(0.0,0.0,1.0));
	gr.addEdge(1,3,vertex2(0.0,0.0,1.0));
	gr.addEdge(3,0,vertex2(0.0,0.0,1.0));

	gr.addEdge(2,7,vertex2(0.0,0.0,1.0));
	gr.addEdge(7,6,vertex2(0.0,0.0,1.0));
	gr.addEdge(6,2,vertex2(0.0,0.0,1.0));

	gr.addEdge(2,3,vertex2(0.0,0.0,1.0));
	gr.addEdge(3,7,vertex2(0.0,0.0,1.0));
	gr.addEdge(7,2,vertex2(0.0,0.0,1.0));

	gr.addEdge(4,6,vertex2(0.0,0.0,1.0));
	gr.addEdge(6,7,vertex2(0.0,0.0,1.0));
	gr.addEdge(7,4,vertex2(0.0,0.0,1.0));

	gr.addEdge(4,7,vertex2(0.0,0.0,1.0));
	gr.addEdge(7,5,vertex2(0.0,0.0,1.0));
	gr.addEdge(5,4,vertex2(0.0,0.0,1.0));

	gr.addEdge(0,4,vertex2(0.0,0.0,1.0));
	gr.addEdge(4,5,vertex2(0.0,0.0,1.0));
	gr.addEdge(5,0,vertex2(0.0,0.0,1.0));

	gr.addEdge(0,5,vertex2(0.0,0.0,1.0));
	gr.addEdge(5,1,vertex2(0.0,0.0,1.0));
	gr.addEdge(1,0,vertex2(0.0,0.0,1.0));

	gr.addEdge(1,5,vertex2(0.0,0.0,1.0));
	gr.addEdge(5,7,vertex2(0.0,0.0,1.0));
	gr.addEdge(7,1,vertex2(0.0,0.0,1.0));

	gr.addEdge(1,7,vertex2(0.0,0.0,1.0));
	gr.addEdge(7,3,vertex2(0.0,0.0,1.0));
	gr.addEdge(3,1,vertex2(0.0,0.0,1.0));

	// Write the VTK file

#ifdef OPENFPM_PDATA

	if (v_cl.rank() != 0) {return;}
	std::string c2 = std::string("openfpm_io/test_data/vtk_graph_v4_test.vtk");

#else

	std::string c2 = std::string("test_data/vtk_graph_v4_test.vtk");

#endif

	VTKWriter<Graph_CSR<vertex2,vertex2>,VTK_GRAPH> vtk(gr);
	vtk.write("vtk_graph_v4.vtk");

	// check that match

	bool test = compare("vtk_graph_v4.vtk",c2);
	BOOST_REQUIRE_EQUAL(true,test);
}

struct vertex3
{
	//! The node contain 3 unsigned long integer for communication computation memory and id
	typedef boost::fusion::vector<float[2],size_t,double> type;

	//! type of the positional field
	typedef float s_type;

	//! Attributes name
	struct attributes
	{
		static const std::string name[];
	};

	//! The data
	type data;

	//! computation property id in boost::fusion::vector
	static const unsigned int x = 0;
	static const unsigned int prp1 = 1;
	static const unsigned int prp2 = 2;

	//! total number of properties boost::fusion::vector
	static const unsigned int max_prop = 3;

	/*!
	 * Default constructor
	 *
	 */
	vertex3()
	{

	}

	static inline bool noPointers()
	{
		return true;
	}

	/*! \brief Initialize the VTKVertex
	 *
	 * \param
	 *
	 */
	vertex3(float x, float y)
	{
		boost::fusion::at_c<vertex::x>(data)[0] = x;
		boost::fusion::at_c<vertex::x>(data)[1] = y;
	}
};

// use the vertex like the edge
typedef vertex edge;

const std::string vertex3::attributes::name[] = {"x","prp1","prp2"};

BOOST_AUTO_TEST_CASE( vtk_writer_use_graph2D )
{
	Vcluster<> & v_cl = create_vcluster();

	if (v_cl.getProcessUnitID() != 0)
		return;

	// Create some graphs and output them

	// Graph

	Graph_CSR<vertex3,edge> gr;

	// Create a cube graph

	gr.addVertex(vertex3(0.0,0.0));
	gr.addVertex(vertex3(0.0,1.0));
	gr.addVertex(vertex3(1.0,0.0));
	gr.addVertex(vertex3(1.0,1.0));

	gr.addEdge(0,1);
	gr.addEdge(1,3);
	gr.addEdge(3,2);
	gr.addEdge(2,0);

	// Write the VTK file

#ifdef OPENFPM_PDATA

	if (v_cl.rank() != 0) {return;}
	std::string c2 = std::string("openfpm_io/test_data/vtk_graph_v3_test.vtk");

#else

	std::string c2 = std::string("test_data/vtk_graph_v3_test.vtk");

#endif

	VTKWriter<Graph_CSR<vertex3,edge>,VTK_GRAPH> vtk(gr);
	vtk.write("vtk_graph_v3.vtk");

	// check that match

	bool test = compare("vtk_graph_v3.vtk",c2);
	BOOST_REQUIRE_EQUAL(true,test);
}

BOOST_AUTO_TEST_CASE( vtk_writer_use_graph)
{
	Vcluster<> & v_cl = create_vcluster();

	if (v_cl.getProcessUnitID() != 0)
		return;

	// Create some graphs and output them

	std::cout << "Graph unit test start" << "\n";

	// Graph

	Graph_CSR<vertex,edge> gr;

	// Create a cube graph

	gr.addVertex(vertex(0.0,0.0,0.0));
	gr.addVertex(vertex(0.0,0.0,1.0));
	gr.addVertex(vertex(0.0,1.0,0.0));
	gr.addVertex(vertex(0.0,1.0,1.0));
	gr.addVertex(vertex(1.0,0.0,0.0));
	gr.addVertex(vertex(1.0,0.0,1.0));
	gr.addVertex(vertex(1.0,1.0,0.0));
	gr.addVertex(vertex(1.0,1.0,1.0));

	gr.addEdge(0,6);
	gr.addEdge(6,4);
	gr.addEdge(4,0);

	gr.addEdge(0,2);
	gr.addEdge(2,6);
	gr.addEdge(6,0);

	gr.addEdge(0,3);
	gr.addEdge(3,2);
	gr.addEdge(2,0);

	gr.addEdge(0,1);
	gr.addEdge(1,3);
	gr.addEdge(3,0);

	gr.addEdge(2,7);
	gr.addEdge(7,6);
	gr.addEdge(6,2);

	gr.addEdge(2,3);
	gr.addEdge(3,7);
	gr.addEdge(7,2);

	gr.addEdge(4,6);
	gr.addEdge(6,7);
	gr.addEdge(7,4);

	gr.addEdge(4,7);
	gr.addEdge(7,5);
	gr.addEdge(5,4);

	gr.addEdge(0,4);
	gr.addEdge(4,5);
	gr.addEdge(5,0);

	gr.addEdge(0,5);
	gr.addEdge(5,1);
	gr.addEdge(1,0);

	gr.addEdge(1,5);
	gr.addEdge(5,7);
	gr.addEdge(7,1);

	gr.addEdge(1,7);
	gr.addEdge(7,3);
	gr.addEdge(3,1);

#ifdef OPENFPM_PDATA

	if (v_cl.rank() != 0) {return;}
	std::string c2 = std::string("openfpm_io/test_data/vtk_graph_test.vtk");

#else

	std::string c2 = std::string("test_data/vtk_graph_test.vtk");

#endif

	// Write the VTK file

	VTKWriter<Graph_CSR<vertex,edge>,VTK_GRAPH> vtk(gr);
	vtk.write("vtk_graph.vtk");

	// check that match

	bool test = compare("vtk_graph.vtk",c2);
	BOOST_REQUIRE_EQUAL(true,test);
}

BOOST_AUTO_TEST_CASE( vtk_writer_use_vector_box)
{
	Vcluster<> & v_cl = create_vcluster();

	if (v_cl.getProcessUnitID() != 0)
		return;

#ifdef OPENFPM_PDATA

	if (v_cl.rank() != 0) {return;}
	std::string c2 = std::string("openfpm_io/test_data/vtk_box_test.vtk");
	std::string c3 = std::string("openfpm_io/test_data/vtk_box_3D_test.vtk");
	std::string c4 = std::string("openfpm_io/test_data/vtk_box_3D_2_test.vtk");

#else

	std::string c2 = std::string("test_data/vtk_box_test.vtk");
	std::string c3 = std::string("test_data/vtk_box_3D_test.vtk");
	std::string c4 = std::string("test_data/vtk_box_3D_2_test.vtk");

#endif

	// Create a vector of boxes
	openfpm::vector<Box<2,float>> vb;

	vb.add(Box<2,float>({0.2,0.2},{1.0,0.5}));
	vb.add(Box<2,float>({0.0,0.0},{0.2,0.2}));
	vb.add(Box<2,float>({0.2,0.0},{0.5,0.2}));
	vb.add(Box<2,float>({0.5,0.0},{1.0,0.2}));
	vb.add(Box<2,float>({0.0,0.2},{0.2,0.5}));
	vb.add(Box<2,float>({0.0,0.5},{1.0,1.0}));

	// Create a writer and write
	VTKWriter<openfpm::vector<Box<2,float>>,VECTOR_BOX> vtk_box;
	vtk_box.add(vb);
	vtk_box.write("vtk_box.vtk");

	// Check that match
	bool test = compare("vtk_box.vtk",c2);
	BOOST_REQUIRE_EQUAL(test,true);

	// Create a vector of boxes
	openfpm::vector<Box<3,float>> vb2;

	vb2.add(Box<3,float>({0.2,0.2,0.0},{1.0,0.5,0.5}));
	vb2.add(Box<3,float>({0.0,0.0,0.0},{0.2,0.2,0.5}));
	vb2.add(Box<3,float>({0.2,0.0,0.0},{0.5,0.2,0.5}));
	vb2.add(Box<3,float>({0.5,0.0,0.0},{1.0,0.2,0.5}));
	vb2.add(Box<3,float>({0.0,0.2,0.0},{0.2,0.5,0.5}));
	vb2.add(Box<3,float>({0.0,0.5,0.0},{1.0,1.0,0.5}));

	// Create a writer and write
	VTKWriter<openfpm::vector<Box<3,float>>,VECTOR_BOX> vtk_box2;
	vtk_box2.add(vb2);
	vtk_box2.write("vtk_box_3D.vtk");

	// Check that match
	test = compare("vtk_box_3D.vtk",c3);
	BOOST_REQUIRE_EQUAL(test,true);

	// Create a vector of boxes
	openfpm::vector<Box<3,float>> vb3;
	vb3.add(Box<3,float>({0.2,0.2,0.5},{1.0,0.5,1.0}));
	vb3.add(Box<3,float>({0.0,0.0,0.5},{0.2,0.2,1.0}));
	vb3.add(Box<3,float>({0.2,0.0,0.5},{0.5,0.2,1.0}));
	vb3.add(Box<3,float>({0.5,0.0,0.5},{1.0,0.2,1.0}));
	vb3.add(Box<3,float>({0.0,0.2,0.5},{0.2,0.5,1.0}));
	vb3.add(Box<3,float>({0.0,0.5,0.5},{1.0,1.0,1.0}));

	// Create a writer and write
	VTKWriter<openfpm::vector<Box<3,float>>,VECTOR_BOX> vtk_box3;
	vtk_box3.add(vb2);
	vtk_box3.add(vb3);
	vtk_box3.write("vtk_box_3D_2.vtk");

	// Check that match
	test = compare("vtk_box_3D_2.vtk",c4);
	BOOST_REQUIRE_EQUAL(test,true);
}

/*! \brief fill the grid with some random data
 *
 * \param g Grid to fill
 *
 */
template<typename grid_type> void fill_grid_some_data(grid_type & g)
{
	typedef Point_test<float> p;

	auto it = g.getIterator();

	while (it.isNext())
	{
		g.template get<p::x>(it.get()) = it.get().get(0);
		if (grid_type::dims != 1)
		{g.template get<p::y>(it.get()) = it.get().get(1);}
		else
		{g.template get<p::y>(it.get()) = 0.0;}
		g.template get<p::z>(it.get()) = 0;
		g.template get<p::s>(it.get()) = 1.0;
		g.template get<p::v>(it.get())[0] = g.getGrid().LinId(it.get());
		g.template get<p::v>(it.get())[1] = g.getGrid().LinId(it.get());
		g.template get<p::v>(it.get())[2] = g.getGrid().LinId(it.get());

		g.template get<p::t>(it.get())[0][0] = g.getGrid().LinId(it.get());
		g.template get<p::t>(it.get())[0][1] = g.getGrid().LinId(it.get());
		g.template get<p::t>(it.get())[0][2] = g.getGrid().LinId(it.get());
		g.template get<p::t>(it.get())[1][0] = g.getGrid().LinId(it.get());
		g.template get<p::t>(it.get())[1][1] = g.getGrid().LinId(it.get());
		g.template get<p::t>(it.get())[1][2] = g.getGrid().LinId(it.get());
		g.template get<p::t>(it.get())[2][0] = g.getGrid().LinId(it.get());
		g.template get<p::t>(it.get())[2][1] = g.getGrid().LinId(it.get());
		g.template get<p::t>(it.get())[2][2] = g.getGrid().LinId(it.get());

		++it;
	}
}

/*! \brief fill the grid with some random data
 *
 * \param g Grid to fill
 *
 */
void fill_grid_some_data_prp(grid_cpu<2,Point_test_prp<float>> & g)
{
	typedef Point_test<float> p;

	auto it = g.getIterator();

	while (it.isNext())
	{
		g.template get<p::x>(it.get()) = it.get().get(0);
		g.template get<p::y>(it.get()) = it.get().get(1);
		g.template get<p::z>(it.get()) = 0;
		g.template get<p::s>(it.get()) = 1.0;
		g.template get<p::v>(it.get())[0] = g.getGrid().LinId(it.get());
		g.template get<p::v>(it.get())[1] = g.getGrid().LinId(it.get());
		g.template get<p::v>(it.get())[2] = g.getGrid().LinId(it.get());

		g.template get<p::t>(it.get())[0][0] = g.getGrid().LinId(it.get());
		g.template get<p::t>(it.get())[0][1] = g.getGrid().LinId(it.get());
		g.template get<p::t>(it.get())[0][2] = g.getGrid().LinId(it.get());
		g.template get<p::t>(it.get())[1][0] = g.getGrid().LinId(it.get());
		g.template get<p::t>(it.get())[1][1] = g.getGrid().LinId(it.get());
		g.template get<p::t>(it.get())[1][2] = g.getGrid().LinId(it.get());
		g.template get<p::t>(it.get())[2][0] = g.getGrid().LinId(it.get());
		g.template get<p::t>(it.get())[2][1] = g.getGrid().LinId(it.get());
		g.template get<p::t>(it.get())[2][2] = g.getGrid().LinId(it.get());

		++it;
	}
}

/*! \brief fill the grid with some random data
 *
 *
 */
void fill_grid_some_data_scal(grid_cpu<2,Point_test_scal<float>> & g)
{
	typedef Point_test<float> p;

	auto it = g.getIterator();

	while (it.isNext())
	{
		g.template get<p::x>(it.get()) = it.get().get(0);
		g.template get<p::y>(it.get()) = it.get().get(1);
		g.template get<p::z>(it.get()) = 0;
		g.template get<p::s>(it.get()) = 1.0;

		++it;
	}
}

BOOST_AUTO_TEST_CASE( vtk_writer_use_grids)
{
	Vcluster<> & v_cl = create_vcluster();

	if (v_cl.getProcessUnitID() != 0)
	{return;}

#ifdef OPENFPM_PDATA

	if (v_cl.rank() != 0) {return;}
	std::string c2 = std::string("openfpm_io/test_data/vtk_grids_test_1d.vtk");
	std::string c3 = std::string("openfpm_io/test_data/vtk_grids_test.vtk");
	std::string c4 = std::string("openfpm_io/test_data/vtk_grids_st_test.vtk");
	std::string c5 = std::string("openfpm_io/test_data/vtk_grids_prp_test.vtk");
	std::string c6 = std::string("openfpm_io/test_data/vtk_grids_test.vtk");

#else

	std::string c2 = std::string("test_data/vtk_grids_test_1d.vtk");
	std::string c3 = std::string("test_data/vtk_grids_test.vtk");
	std::string c4 = std::string("test_data/vtk_grids_st_test.vtk");
	std::string c5 = std::string("test_data/vtk_grids_prp_test.vtk");
	std::string c6 = std::string("test_data/vtk_grids_test.vtk");

#endif

	{

		// Create box grids
		Point<1,double> offset1({0.0});
		Point<1,double> spacing1({0.1});
		Box<1,size_t> d1({1},{14});

		// Create box grids
		Point<1,double> offset2({5.0});
		Point<1,float> spacing2({0.2});
		Box<1,size_t> d2({2},{13});

		// Create box grids
		Point<1,double> offset3({0.0});
		Point<1,double> spacing3({0.05});
		Box<1,size_t> d3({3},{11});

		// Create box grids
		Point<1,double> offset4({5.0});
		Point<1,double> spacing4({0.1});
		Box<1,size_t> d4({1},{7});

		size_t sz[] = {16};
		grid_cpu<1,Point_test<double>> g1(sz);
		g1.setMemory();
		fill_grid_some_data(g1);
		grid_cpu<1,Point_test<double>> g2(sz);
		g2.setMemory();
		fill_grid_some_data(g2);
		grid_cpu<1,Point_test<double>> g3(sz);
		g3.setMemory();
		fill_grid_some_data(g3);
		grid_cpu<1,Point_test<double>> g4(sz);
		g4.setMemory();
		fill_grid_some_data(g4);

		g4.template get<Point_test<double>::s>(0) = 1.0/3.0;

		// Create a writer and write
		VTKWriter<boost::mpl::pair<grid_cpu<1,Point_test<double>>,double>,VECTOR_GRIDS> vtk_g;
		vtk_g.add(g1,offset1,spacing1,d1);
		vtk_g.add(g2,offset2,spacing2,d2);
		vtk_g.add(g3,offset3,spacing3,d3);
		vtk_g.add(g4,offset4,spacing4,d4);

		openfpm::vector<std::string> prp_names;
		vtk_g.write("vtk_grids_1d.vtk",prp_names);

	#ifndef SE_CLASS3

		// Check that match
		bool test = compare("vtk_grids_1d.vtk",c2);
		BOOST_REQUIRE_EQUAL(test,true);

	#endif
	}

	{

		// Create box grids
		Point<2,float> offset1({0.0,0.0});
		Point<2,float> spacing1({0.1,0.2});
		Box<2,size_t> d1({1,2},{14,15});

		// Create box grids
		Point<2,float> offset2({5.0,7.0});
		Point<2,float> spacing2({0.2,0.1});
		Box<2,size_t> d2({2,1},{13,15});

		// Create box grids
		Point<2,float> offset3({0.0,7.0});
		Point<2,float> spacing3({0.05,0.07});
		Box<2,size_t> d3({3,2},{11,10});

		// Create box grids
		Point<2,float> offset4({5.0,0.0});
		Point<2,float> spacing4({0.1,0.1});
		Box<2,size_t> d4({1,1},{7,7});

		size_t sz[] = {16,16};
		grid_cpu<2,Point_test<float>> g1(sz);
		g1.setMemory();
		fill_grid_some_data(g1);
		grid_cpu<2,Point_test<float>> g2(sz);
		g2.setMemory();
		fill_grid_some_data(g2);
		grid_cpu<2,Point_test<float>> g3(sz);
		g3.setMemory();
		fill_grid_some_data(g3);
		grid_cpu<2,Point_test<float>> g4(sz);
		g4.setMemory();
		fill_grid_some_data(g4);

		// Create a writer and write
		VTKWriter<boost::mpl::pair<grid_cpu<2,Point_test<float>>,float>,VECTOR_GRIDS> vtk_g;
		vtk_g.add(g1,offset1,spacing1,d1);
		vtk_g.add(g2,offset2,spacing2,d2);
		vtk_g.add(g3,offset3,spacing3,d3);
		vtk_g.add(g4,offset4,spacing4,d4);

		openfpm::vector<std::string> prp_names;
		vtk_g.write("vtk_grids.vtk",prp_names);

	#ifndef SE_CLASS3

		// Check that match
		bool test = compare("vtk_grids.vtk",c3);
		BOOST_REQUIRE_EQUAL(test,true);

	#endif
	}

	{
	// Create box grids
	Point<2,float> offset1({0.0,0.0});
	Point<2,float> spacing1({0.1,0.1});
	Box<2,size_t> d1({1,2},{14,15});

	// Create box grids
	Point<2,float> offset2({0.0,0.0});
	Point<2,float> spacing2({0.1,0.1});
	Box<2,size_t> d2({2,1},{13,15});

	// Create box grids
	Point<2,float> offset3({5.0,5.0});
	Point<2,float> spacing3({0.1,0.1});
	Box<2,size_t> d3({3,2},{11,10});

	// Create box grids
	Point<2,float> offset4({5.0,5.0});
	Point<2,float> spacing4({0.1,0.1});
	Box<2,size_t> d4({1,1},{7,7});

	size_t sz[] = {16,16};
	grid_cpu<2,Point_test<float>> g1(sz);
	g1.setMemory();
	fill_grid_some_data(g1);
	grid_cpu<2,Point_test<float>> g2(sz);
	g2.setMemory();
	fill_grid_some_data(g2);
	grid_cpu<2,Point_test<float>> g3(sz);
	g3.setMemory();
	fill_grid_some_data(g3);
	grid_cpu<2,Point_test<float>> g4(sz);
	g4.setMemory();
	fill_grid_some_data(g4);

	comb<2> cmb;
	cmb.zero();

	comb<2> cmb2;
	cmb2.mone();

	// Create a writer and write
	VTKWriter<boost::mpl::pair<grid_cpu<2,Point_test<float>>,float>,VECTOR_ST_GRIDS> vtk_g;
	vtk_g.add(0,g1,offset1,spacing1,d1,cmb);
	vtk_g.add(0,g2,offset2,spacing2,d2,cmb);
	vtk_g.add(1,g3,offset3,spacing3,d3,cmb);
	vtk_g.add(1,g4,offset4,spacing4,d4,cmb2);

	vtk_g.write("vtk_grids_st.vtk");

	// Check that match
	bool test = compare("vtk_grids_st.vtk",c4);
	BOOST_REQUIRE_EQUAL(test,true);
	}

	{
	// Create box grids
	Point<2,float> offset1({0.0,0.0});
	Point<2,float> spacing1({0.1,0.1});
	Box<2,size_t> d1({1,2},{14,15});

	// Create box grids
	Point<2,float> offset2({0.0,0.0});
	Point<2,float> spacing2({0.1,0.1});
	Box<2,size_t> d2({2,1},{13,15});

	// Create box grids
	Point<2,float> offset3({5.0,5.0});
	Point<2,float> spacing3({0.1,0.1});
	Box<2,size_t> d3({3,2},{11,10});

	// Create box grids
	Point<2,float> offset4({5.0,5.0});
	Point<2,float> spacing4({0.1,0.1});
	Box<2,size_t> d4({1,1},{7,7});

	size_t sz[] = {16,16};
	grid_cpu<2,Point_test_scal<float>> g1(sz);
	g1.setMemory();
	fill_grid_some_data_scal(g1);
	grid_cpu<2,Point_test_scal<float>> g2(sz);
	g2.setMemory();
	fill_grid_some_data_scal(g2);
	grid_cpu<2,Point_test_scal<float>> g3(sz);
	g3.setMemory();
	fill_grid_some_data_scal(g3);
	grid_cpu<2,Point_test_scal<float>> g4(sz);
	g4.setMemory();
	fill_grid_some_data_scal(g4);

	// Create a writer and write
	VTKWriter<boost::mpl::pair<grid_cpu<2,Point_test_scal<float>>,float>,VECTOR_GRIDS> vtk_g;
	vtk_g.add(g1,offset1,spacing1,d1);
	vtk_g.add(g2,offset2,spacing2,d2);
	vtk_g.add(g3,offset3,spacing3,d3);
	vtk_g.add(g4,offset4,spacing4,d4);

	openfpm::vector<std::string> prp_names;
	vtk_g.write("vtk_grids_prp.vtk",prp_names);

	// Check that match
	bool test = compare("vtk_grids_prp.vtk",c5);
	BOOST_REQUIRE_EQUAL(test,true);
	}

	{
	// Create box grids
	Point<2,float> offset1({0.0,0.0});
	Point<2,float> spacing1({0.1,0.2});
	Box<2,size_t> d1({1,2},{14,15});

	// Create box grids
	Point<2,float> offset2({5.0,7.0});
	Point<2,float> spacing2({0.2,0.1});
	Box<2,size_t> d2({2,1},{13,15});

	// Create box grids
	Point<2,float> offset3({0.0,7.0});
	Point<2,float> spacing3({0.05,0.07});
	Box<2,size_t> d3({3,2},{11,10});

	// Create box grids
	Point<2,float> offset4({5.0,0.0});
	Point<2,float> spacing4({0.1,0.1});
	Box<2,size_t> d4({1,1},{7,7});

	size_t sz[] = {16,16};
	grid_cpu<2,aggregate<float,float,float,float,float[3],float[3][3],openfpm::vector<int>> > g1(sz);
	g1.setMemory();
	fill_grid_some_data(g1);
	grid_cpu<2,aggregate<float,float,float,float,float[3],float[3][3],openfpm::vector<int>> > g2(sz);
	g2.setMemory();
	fill_grid_some_data(g2);
	grid_cpu<2,aggregate<float,float,float,float,float[3],float[3][3],openfpm::vector<int>> > g3(sz);
	g3.setMemory();
	fill_grid_some_data(g3);
	grid_cpu<2,aggregate<float,float,float,float,float[3],float[3][3],openfpm::vector<int>> > g4(sz);
	g4.setMemory();
	fill_grid_some_data(g4);

	// Create a writer and write
	VTKWriter<boost::mpl::pair<grid_cpu<2,aggregate<float,float,float,float,float[3],float[3][3],openfpm::vector<int>> >,float>,VECTOR_GRIDS> vtk_g;
	vtk_g.add(g1,offset1,spacing1,d1);
	vtk_g.add(g2,offset2,spacing2,d2);
	vtk_g.add(g3,offset3,spacing3,d3);
	vtk_g.add(g4,offset4,spacing4,d4);

	openfpm::vector<std::string> prp_names;
	vtk_g.write("vtk_grids_unk.vtk",prp_names);

#ifndef SE_CLASS3

	// Check that match
	bool test = compare("vtk_grids_unk.vtk",c6);
	BOOST_REQUIRE_EQUAL(test,true);

#endif

	}

	// Try

	{
		bool ret = is_vtk_writable<Point<3,float>>::value;
		BOOST_REQUIRE_EQUAL(ret,true);
		ret = is_vtk_writable<Point<3,double>>::value;
		BOOST_REQUIRE_EQUAL(ret,true);

		int  dims = vtk_dims<Point<3,float>>::value;
		BOOST_REQUIRE_EQUAL(dims,3);

		dims = vtk_dims<long int>::value;
		BOOST_REQUIRE_EQUAL(dims,1);
	}

}


BOOST_AUTO_TEST_CASE( vtk_writer_use_point_set )
{
	Vcluster<> & v_cl = create_vcluster();

	if (v_cl.getProcessUnitID() != 0)
		return;

#ifdef OPENFPM_PDATA

	if (v_cl.rank() != 0) {return;}
	std::string c2 = std::string("openfpm_io/test_data/vtk_points_test.vtk");
	std::string c3 = std::string("openfpm_io/test_data/vtk_points_pp_test.vtk");
	std::string c4 = std::string("openfpm_io/test_data/vtk_points_pp_header_test.vtk");

#else

	std::string c2 = std::string("test_data/vtk_points_test.vtk");
	std::string c3 = std::string("test_data/vtk_points_pp_test.vtk");
	std::string c4 = std::string("test_data/vtk_points_pp_header_test.vtk");

#endif

	{
	// Create 3 vectors with random particles
	openfpm::vector<Point<3,double>> v1ps;
	openfpm::vector<Point<3,double>> v2ps;
	openfpm::vector<Point<3,double>> v3ps;
	openfpm::vector<aggregate<float,float[3]>> v1pp;
	openfpm::vector<aggregate<float,float[3]>> v2pp;
	openfpm::vector<aggregate<float,float[3]>> v3pp;
	openfpm::vector<aggregate<float,Point<3,float>>> v4pp;

    // set the seed
	// create the random generator engine
	SimpleRNG rng;

	// fill the vector with random data
	v1ps.resize(100);
	v2ps.resize(100);
	v3ps.resize(100);

	v1pp.resize(100);
	v2pp.resize(100);
	v3pp.resize(100);
	v4pp.resize(100);

	for (size_t i = 0 ; i < v1ps.size(); i++)
	{
		v1ps.template get<0>(i)[0] = rng.GetUniform();
		v1ps.template get<0>(i)[1] = rng.GetUniform();
		v1ps.template get<0>(i)[2] = rng.GetUniform();

		v2ps.template get<0>(i)[0] = rng.GetUniform()*0.5;
		v2ps.template get<0>(i)[1] = rng.GetUniform()*0.5;
		v2ps.template get<0>(i)[2] = rng.GetUniform()*0.5;

		v3ps.template get<0>(i)[0] = rng.GetUniform()*0.3;
		v3ps.template get<0>(i)[1] = rng.GetUniform()*0.3;
		v3ps.template get<0>(i)[2] = rng.GetUniform()*0.3;

		v1pp.template get<0>(i) = rng.GetUniform();
		v1pp.template get<1>(i)[0] = rng.GetUniform();
		v1pp.template get<1>(i)[1] = rng.GetUniform();
		v1pp.template get<1>(i)[2] = rng.GetUniform();

		v2pp.template get<0>(i) = rng.GetUniform();
		v2pp.template get<1>(i)[0] = rng.GetUniform();
		v2pp.template get<1>(i)[1] = rng.GetUniform();
		v2pp.template get<1>(i)[2] = rng.GetUniform();

		v3pp.template get<0>(i) = rng.GetUniform();
		v3pp.template get<1>(i)[0] = rng.GetUniform();
		v3pp.template get<1>(i)[1] = rng.GetUniform();
		v3pp.template get<1>(i)[2] = rng.GetUniform();

		v4pp.template get<0>(i) = rng.GetUniform();
		v4pp.template get<1>(i).get(0) = rng.GetUniform();
		v4pp.template get<1>(i).get(1) = rng.GetUniform();
		v4pp.template get<1>(i).get(2) = rng.GetUniform();
	}

	// Create a writer and write
	VTKWriter<boost::mpl::pair<openfpm::vector<Point<3,double>>,openfpm::vector<aggregate<float,float[3]>>>,VECTOR_POINTS> vtk_v;
	vtk_v.add(v1ps,v1pp,75);
	vtk_v.add(v2ps,v2pp,88);
	vtk_v.add(v3ps,v3pp,90);

	openfpm::vector<std::string> prp_names;
	vtk_v.write("vtk_points.vtp",prp_names);
	//auto &v_cl=create_vcluster();
	//size_t n=v_cl.size();
	vtk_v.write_pvtp("vtk_points",prp_names,2);


#ifndef SE_CLASS3

	bool test = true;

	// Check that match
	test = compare("vtk_points.vtp","test_data/vtk_points_test.vtp");
	BOOST_REQUIRE_EQUAL(test,true);

	//It just checks generation of the format and not actual data (File names)
	test = compare("vtk_points.pvtp","test_data/pvtp_points_test.pvtp");
	BOOST_REQUIRE_EQUAL(test,true);

#endif

	// Create a writer and write
	VTKWriter<boost::mpl::pair<openfpm::vector<Point<3,double>>,openfpm::vector<aggregate<float,Point<3,float>>>>,VECTOR_POINTS> vtk_v2;
	vtk_v2.add(v1ps,v4pp,75);

	vtk_v2.write("vtk_points_pp.vtp",prp_names);

#ifndef SE_CLASS3

	// Check that match
	test = compare("vtk_points_pp.vtp","test_data/vtk_points_pp_test.vtp");
	BOOST_REQUIRE_EQUAL(test,true);

#endif

	// Create a writer and write
	VTKWriter<boost::mpl::pair<openfpm::vector<Point<3,double>>,openfpm::vector<aggregate<float,Point<3,float>>>>,VECTOR_POINTS> vtk_v3;
	vtk_v3.add(v1ps,v4pp,75);

	vtk_v3.write("vtk_points_pp_header.vtp",prp_names,"points","time=5.123");

	// We try binary
	vtk_v3.write("vtk_points_pp_header_bin.vtp",prp_names,"points","time=5.123",file_type::BINARY);

#ifndef SE_CLASS3

	// Check that match
	test = compare("vtk_points_pp_header.vtp","test_data/vtk_points_pp_header_test.vtp");
	BOOST_REQUIRE_EQUAL(test,true);

	test = compare("vtk_points_pp_header_bin.vtp","test_data/vtk_points_pp_header_bin_test.vtp");
	BOOST_REQUIRE_EQUAL(test,true);

#endif

	}
}

BOOST_AUTO_TEST_CASE( vtk_writer_use_point_set_properties )
{
	Vcluster<> & v_cl = create_vcluster();

	if (v_cl.getProcessUnitID() != 0)
		return;

#ifdef OPENFPM_PDATA

	if (v_cl.rank() != 0) {return;}
	std::string c2 = std::string("openfpm_io/test_data/vtk_points_with_prp_names_test.vtk");

#else

	std::string c2 = std::string("test_data/vtk_points_with_prp_names_test.vtk");

#endif

	{
	// Create 3 vectors with random particles
	openfpm::vector<Point<3,double>> v1ps;
	openfpm::vector<aggregate<float,float[3]>> v1pp;

    // set the seed
	// create the random generator engine
	SimpleRNG rng;

	// fill the vector with random data
	v1ps.resize(100);
	v1pp.resize(100);

	for (size_t i = 0 ; i < v1ps.size(); i++)
	{
		v1ps.template get<0>(i)[0] = rng.GetUniform();
		v1ps.template get<0>(i)[1] = rng.GetUniform();
		v1ps.template get<0>(i)[2] = rng.GetUniform();


		v1pp.template get<0>(i) = rng.GetUniform();
		v1pp.template get<1>(i)[0] = rng.GetUniform();
		v1pp.template get<1>(i)[1] = rng.GetUniform();
		v1pp.template get<1>(i)[2] = rng.GetUniform();
	}

	openfpm::vector<std::string> prop_names;

	// Create a writer and write adding names to the properties
	VTKWriter<boost::mpl::pair<openfpm::vector<Point<3,double>>,openfpm::vector<aggregate<float,float[3]>>>,VECTOR_POINTS> vtk_v;
	vtk_v.add(v1ps,v1pp,75);
	openfpm::vector<std::string> prp_names({"scalar","vector"});
	vtk_v.write("vtk_points_with_prp_names.vtp",prp_names);

#ifndef SE_CLASS3

	// Check that match
	bool test = compare("vtk_points_with_prp_names.vtp","test_data/vtk_points_with_prp_names_test.vtp");
	BOOST_REQUIRE_EQUAL(test,true);

#endif

	}
}

BOOST_AUTO_TEST_CASE( vtk_writer_use_point_set_check_out_precision )
{
	Vcluster<> & v_cl = create_vcluster();

	if (v_cl.getProcessUnitID() != 0)
		return;

#ifdef OPENFPM_PDATA

	if (v_cl.rank() != 0) {return;}
	std::string c2 = std::string("openfpm_io/test_data/vtk_points_with_prp_names_prec_check_test.vtk");

#else

	std::string c2 = std::string("test_data/vtk_points_with_prp_names_prec_check_test.vtk");

#endif

	{
	// Create 3 vectors with random particles
	openfpm::vector<Point<3,double>> v1ps;
	openfpm::vector<aggregate<float,double[3]>> v1pp;

	// fill the vector with random data
	v1ps.resize(100);
	v1pp.resize(100);

	for (size_t i = 0 ; i < v1ps.size(); i++)
	{
		v1ps.template get<0>(i)[0] = std::numeric_limits<double>::max();
		v1ps.template get<0>(i)[1] = std::numeric_limits<double>::max();
		v1ps.template get<0>(i)[2] = std::numeric_limits<double>::max();


		v1pp.template get<0>(i) = std::numeric_limits<float>::max();
		v1pp.template get<1>(i)[0] = std::numeric_limits<double>::max();
		v1pp.template get<1>(i)[1] = std::numeric_limits<double>::max();
		v1pp.template get<1>(i)[2] = std::numeric_limits<double>::max();
	}

	openfpm::vector<std::string> prop_names;

	// Create a writer and write adding names to the properties
	VTKWriter<boost::mpl::pair<openfpm::vector<Point<3,double>>,openfpm::vector<aggregate<float,double[3]>>>,VECTOR_POINTS> vtk_v;
	vtk_v.add(v1ps,v1pp,75);
	openfpm::vector<std::string> prp_names({"scalar","vector"});
	vtk_v.write("vtk_points_with_prp_names_prec_check.vtp",prp_names);

#ifndef SE_CLASS3

	// Check that match
	bool test = compare("vtk_points_with_prp_names_prec_check.vtp","test_data/vtk_points_with_prp_names_prec_check_test.vtp");
	BOOST_REQUIRE_EQUAL(test,true);

#endif

	}
}

BOOST_AUTO_TEST_CASE( vtk_writer_use_point_set_binary )
{
	Vcluster<> & v_cl = create_vcluster();

	if (v_cl.getProcessUnitID() != 0)
		return;

#ifdef OPENFPM_PDATA

	if (v_cl.rank() != 0) {return;}
	std::string c2 = std::string("openfpm_io/test_data/vtk_points_bin_test.vtk");
	std::string c3 = std::string("openfpm_io/test_data/vtk_points_pp_bin_test.vtk");
	std::string c4 = std::string("openfpm_io/test_data/vtk_points_2d_bin_test.vtk");
	std::string c5 = std::string("openfpm_io/test_data/vtk_points_2d_pp_bin_test.vtk");

#else

	std::string c2 = std::string("test_data/vtk_points_bin_test.vtk");
	std::string c3 = std::string("test_data/vtk_points_pp_bin_test.vtk");
	std::string c4 = std::string("test_data/vtk_points_2d_bin_test.vtk");
	std::string c5 = std::string("test_data/vtk_points_2d_pp_bin_test.vtk");

#endif

	{
		// Create 3 vectors with random particles
		openfpm::vector<Point<3,double>> v1ps;
		openfpm::vector<Point<3,double>> v2ps;
		openfpm::vector<Point<3,double>> v3ps;
		openfpm::vector<aggregate<float,float[3]>> v1pp;
		openfpm::vector<aggregate<float,float[3]>> v2pp;
		openfpm::vector<aggregate<float,float[3]>> v3pp;
		openfpm::vector<aggregate<float,Point<3,float>>> v4pp;

	    // set the seed
		// create the random generator engine
		SimpleRNG rng;

		// fill the vector with random data
		v1ps.resize(100);
		v2ps.resize(100);
		v3ps.resize(100);

		v1pp.resize(100);
		v2pp.resize(100);
		v3pp.resize(100);
		v4pp.resize(100);

		for (size_t i = 0 ; i < v1ps.size(); i++)
		{
			v1ps.template get<0>(i)[0] = rng.GetUniform();
			v1ps.template get<0>(i)[1] = rng.GetUniform();
			v1ps.template get<0>(i)[2] = rng.GetUniform();

			v2ps.template get<0>(i)[0] = rng.GetUniform()*0.5;
			v2ps.template get<0>(i)[1] = rng.GetUniform()*0.5;
			v2ps.template get<0>(i)[2] = rng.GetUniform()*0.5;

			v3ps.template get<0>(i)[0] = rng.GetUniform()*0.3;
			v3ps.template get<0>(i)[1] = rng.GetUniform()*0.3;
			v3ps.template get<0>(i)[2] = rng.GetUniform()*0.3;

			v1pp.template get<0>(i) = rng.GetUniform();
			v1pp.template get<1>(i)[0] = rng.GetUniform();
			v1pp.template get<1>(i)[1] = rng.GetUniform();
			v1pp.template get<1>(i)[2] = rng.GetUniform();

			v2pp.template get<0>(i) = rng.GetUniform();
			v2pp.template get<1>(i)[0] = rng.GetUniform();
			v2pp.template get<1>(i)[1] = rng.GetUniform();
			v2pp.template get<1>(i)[2] = rng.GetUniform();

			v3pp.template get<0>(i) = rng.GetUniform();
			v3pp.template get<1>(i)[0] = rng.GetUniform();
			v3pp.template get<1>(i)[1] = rng.GetUniform();
			v3pp.template get<1>(i)[2] = rng.GetUniform();

			v4pp.template get<0>(i) = rng.GetUniform();
			v4pp.template get<1>(i).get(0) = rng.GetUniform();
			v4pp.template get<1>(i).get(1) = rng.GetUniform();
			v4pp.template get<1>(i).get(2) = rng.GetUniform();
		}

		// Create a writer and write
		VTKWriter<boost::mpl::pair<openfpm::vector<Point<3,double>>,openfpm::vector<aggregate<float,float[3]>>>,VECTOR_POINTS> vtk_v;
		vtk_v.add(v1ps,v1pp,75);
		vtk_v.add(v2ps,v2pp,88);
		vtk_v.add(v3ps,v3pp,90);

		openfpm::vector<std::string> prp_names;
		vtk_v.write("vtk_points_bin.vtp",prp_names,"vtk output","",file_type::BINARY);
		vtk_v.write("vtk_points_bin2.vtp",prp_names,"vtk output","",file_type::BINARY);

#ifndef SE_CLASS3

		bool test = true;

		// Check that match
		test = compare("vtk_points_bin.vtp","test_data/vtk_points_bin_test.vtp");
		BOOST_REQUIRE_EQUAL(test,true);
		test = compare("vtk_points_bin2.vtp","test_data/vtk_points_bin_test.vtp");
		BOOST_REQUIRE_EQUAL(test,true);

#endif

		// Create a writer and write
		VTKWriter<boost::mpl::pair<openfpm::vector<Point<3,double>>,openfpm::vector<aggregate<float,Point<3,float>>>>,VECTOR_POINTS> vtk_v2;
		vtk_v2.add(v1ps,v4pp,75);

		vtk_v2.write("vtk_points_pp_bin.vtp",prp_names,"vtk output","",file_type::BINARY);

#ifndef SE_CLASS3

		// Check that match
		test = compare("vtk_points_pp_bin.vtp","test_data/vtk_points_pp_bin_test.vtp");
		BOOST_REQUIRE_EQUAL(test,true);

#endif

	}


	{
		// Create 3 vectors with random particles
		openfpm::vector<Point<2,double>> v1ps;
		openfpm::vector<Point<2,double>> v2ps;
		openfpm::vector<Point<2,double>> v3ps;
		openfpm::vector<aggregate<float,float[3][3]>> v1pp;
		openfpm::vector<aggregate<float,float[3][3]>> v2pp;
		openfpm::vector<aggregate<float,float[3][3]>> v3pp;
		openfpm::vector<aggregate<float[3],double[2]>> v4pp;

	    // set the seed
		// create the random generator engine
		SimpleRNG rng;

		// fill the vector with random data
		v1ps.resize(100);
		v2ps.resize(100);
		v3ps.resize(100);

		v1pp.resize(100);
		v2pp.resize(100);
		v3pp.resize(100);
		v4pp.resize(100);

		for (size_t i = 0 ; i < v1ps.size(); i++)
		{
			v1ps.template get<0>(i)[0] = rng.GetUniform();
			v1ps.template get<0>(i)[1] = rng.GetUniform();

			v2ps.template get<0>(i)[0] = rng.GetUniform()*0.5;
			v2ps.template get<0>(i)[1] = rng.GetUniform()*0.5;

			v3ps.template get<0>(i)[0] = rng.GetUniform()*0.3;
			v3ps.template get<0>(i)[1] = rng.GetUniform()*0.3;

			v1pp.template get<0>(i) = rng.GetUniform();
			v1pp.template get<1>(i)[0][0] = rng.GetUniform();
			v1pp.template get<1>(i)[0][1] = rng.GetUniform();
			v1pp.template get<1>(i)[0][2] = rng.GetUniform();
			v1pp.template get<1>(i)[1][0] = rng.GetUniform();
			v1pp.template get<1>(i)[1][1] = rng.GetUniform();
			v1pp.template get<1>(i)[1][2] = rng.GetUniform();
			v1pp.template get<1>(i)[2][0] = rng.GetUniform();
			v1pp.template get<1>(i)[2][1] = rng.GetUniform();
			v1pp.template get<1>(i)[2][2] = rng.GetUniform();

			v2pp.template get<0>(i) = rng.GetUniform();
			v2pp.template get<1>(i)[0][0] = rng.GetUniform();
			v2pp.template get<1>(i)[0][1] = rng.GetUniform();
			v2pp.template get<1>(i)[0][2] = rng.GetUniform();
			v2pp.template get<1>(i)[1][0] = rng.GetUniform();
			v2pp.template get<1>(i)[1][1] = rng.GetUniform();
			v2pp.template get<1>(i)[1][2] = rng.GetUniform();
			v2pp.template get<1>(i)[2][0] = rng.GetUniform();
			v2pp.template get<1>(i)[2][1] = rng.GetUniform();
			v2pp.template get<1>(i)[2][2] = rng.GetUniform();

			v3pp.template get<0>(i) = rng.GetUniform();
			v3pp.template get<1>(i)[0][0] = rng.GetUniform();
			v3pp.template get<1>(i)[0][1] = rng.GetUniform();
			v3pp.template get<1>(i)[0][2] = rng.GetUniform();
			v3pp.template get<1>(i)[1][0] = rng.GetUniform();
			v3pp.template get<1>(i)[1][1] = rng.GetUniform();
			v3pp.template get<1>(i)[1][2] = rng.GetUniform();
			v3pp.template get<1>(i)[2][0] = rng.GetUniform();
			v3pp.template get<1>(i)[2][1] = rng.GetUniform();
			v3pp.template get<1>(i)[2][2] = rng.GetUniform();

			v4pp.template get<0>(i)[0] = rng.GetUniform();
			v4pp.template get<0>(i)[1] = rng.GetUniform();
			v4pp.template get<0>(i)[2] = rng.GetUniform();
			v4pp.template get<1>(i)[0] = rng.GetUniform();
			v4pp.template get<1>(i)[1] = rng.GetUniform();
		}

		// Create a writer and write
		VTKWriter<boost::mpl::pair<openfpm::vector<Point<2,double>>,openfpm::vector<aggregate<float,float[3][3]>>>,VECTOR_POINTS> vtk_v;
		vtk_v.add(v1ps,v1pp,75);
		vtk_v.add(v2ps,v2pp,88);
		vtk_v.add(v3ps,v3pp,90);

		openfpm::vector<std::string> stub;

		vtk_v.write("vtk_points_2d_bin.vtp",stub,"vtk output","",file_type::BINARY);

#ifndef SE_CLASS3

		bool test = true;

		// Check that match
		test = compare("vtk_points_2d_bin.vtp","test_data/vtk_points_2d_bin_test.vtp");
		BOOST_REQUIRE_EQUAL(test,true);

#endif

		// Create a writer and write
		VTKWriter<boost::mpl::pair<openfpm::vector<Point<2,double>>,openfpm::vector<aggregate<float[3],double[2]>>>,VECTOR_POINTS> vtk_v2;
		vtk_v2.add(v1ps,v4pp,75);

		vtk_v2.write("vtk_points_2d_pp_bin.vtp",stub,"vtk output","",file_type::BINARY);

#ifndef SE_CLASS3

		// Check that match
		test = compare("vtk_points_2d_pp_bin.vtp","test_data/vtk_points_2d_pp_bin_test.vtp");
		BOOST_REQUIRE_EQUAL(test,true);

#endif
	}
}

BOOST_AUTO_TEST_SUITE_END()

#endif /* VTKWRITER_UNIT_TESTS_HPP_ */

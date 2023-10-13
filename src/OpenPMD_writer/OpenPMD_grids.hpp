/*
 * VTKWriter_grids.hpp
 *
 *  Created on: May 5, 2015
 *      Author: Pietro Incardona
 */

#ifndef OPENPMDWRITER_GRIDS_HPP_
#define OPENPMDWRITER_GRIDS_HPP_

#include <openPMD/openPMD.hpp>
#include "Vector/map_vector.hpp"
#include "OpenPMD_util.hpp"


/*! \brief this class is a functor for "for_each" algorithm
 *
 * This class is a functor for "for_each" algorithm. For each
 * element of the boost::vector the operator() is called.
 * Is mainly used to produce an output for each property
 *
 * \tparam ele_v It is the class ele_v that store the couple vector of position and property
 *
 *
 */
template<typename series_type, typename openpmd_type, typename grid_type, typename St>
struct prop_out_pmd
{
	//! properties names
	const openfpm::vector<std::string> & prop_names;

	openpmd_type & mesh_grid;

	const grid_type & grid;

	series_type & series;

	/*! \brief constructor
	 *
	 * \param v_out string to fill with the vertex properties
	 * \param vv vector we are processing
	 * \param ft ASCII or BINARY format
	 *
	 */
	prop_out_pmd(series_type & series, openpmd_type & mesh_grid, const grid_type & grid, const openfpm::vector<std::string> & prop_names)
	:mesh_grid(mesh_grid),grid(grid),prop_names(prop_names),series(series)
	{};

	/*! \brief It produce an output for each property
	 *
	 * \param t property id
	 *
	 */
    template<typename T>
    void operator()(T& t) const
    {
    	typedef typename boost::mpl::at<typename grid_type::value_type::type,boost::mpl::int_<T::value>>::type ptype;
    	typedef typename std::remove_all_extents<ptype>::type base_ptype;

    	meta_prop_pmd<boost::mpl::int_<T::value> ,grid_type, ptype, is_vtk_writable<base_ptype>::value > m(mesh_grid,grid,prop_names);


    }

    void lastProp()
	{

    }

};

/*!
 *
 * It write a VTK format file in case of grids defined on a space
 *
 * \tparam boost::mpl::pair<G,S>
 *
 * where G is the type of grid S is the type of space, float, double ...
 *
 */
template <typename grid_type>
class OpenPMD_Writer<grid_type,OPENPMD_GRID_DIST>
{

	HeapMemory mem;

public:

	/*!
	 *
	 * VTKWriter constructor
	 *
	 */
	OpenPMD_Writer()
	{}

	/*! \brief It write a VTK file from a graph
	 *
	 * \tparam prp_out which properties to output [default = -1 (all)]
	 *
	 * \param file path where to write
	 * \param name of the graph
	 * \param prop_names properties name (can also be a vector of size 0)
	 * \param ft specify if it is a VTK BINARY or ASCII file [default = ASCII]
	 *
	 * \return true if the function write successfully
	 *
	 */
	bool write(const grid_type & grid, std::string file)
	{
		// open file for writing
		openPMD::Series series = openPMD::Series(
			file,
			openPMD::Access::CREATE,
			MPI_COMM_WORLD
		);

		auto & mesh_grid = series.iterations[0].meshes;

		auto prop_names = grid.getPropNames();

		prop_out_pmd<openPMD::Series,
					 typename std::remove_reference<decltype(mesh_grid)>::type, 
					 grid_type, 
					 typename grid_type::stype> pp(series,mesh_grid,grid,prop_names);

		boost::mpl::for_each_ref<boost::mpl::range_c<int,0,grid_type::value_type::max_prop>>(pp);

		series.flush();

		return true;
	}
};


#endif
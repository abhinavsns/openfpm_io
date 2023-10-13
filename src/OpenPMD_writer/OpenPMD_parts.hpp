/*
 * VTKWriter_grids.hpp
 *
 *  Created on: May 5, 2015
 *      Author: Pietro Incardona
 */

#ifndef OPENPMDWRITER_PARTS_HPP_
#define OPENPMDWRITER_PARTS_HPP_

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
template<typename series_type, typename openpmd_type, typename part_type, typename St>
struct prop_out_ppmd
{
	//! properties names
	const openfpm::vector<std::string> & prop_names;

	openpmd_type & part_pmd;

	const part_type & part;

	series_type & series;

	/*! \brief constructor
	 *
	 * \param v_out string to fill with the vertex properties
	 * \param vv vector we are processing
	 * \param ft ASCII or BINARY format
	 *
	 */
	prop_out_ppmd(series_type & series, openpmd_type & part_pmd, const part_type & part, const openfpm::vector<std::string> & prop_names)
	:part_pmd(part_pmd),part(part),prop_names(prop_names),series(series)
	{};

	/*! \brief It produce an output for each property
	 *
	 * \param t property id
	 *
	 */
    template<typename T>
    void operator()(T& t) const
    {
    	typedef typename boost::mpl::at<typename part_type::value_type::type,boost::mpl::int_<T::value>>::type ptype;
    	typedef typename std::remove_all_extents<ptype>::type base_ptype;

    	meta_prop_ppmd<boost::mpl::int_<T::value> , part_type, ptype, is_vtk_writable<base_ptype>::value > m(part_pmd,part,prop_names);


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
template <typename part_type>
class OpenPMD_Writer<part_type,OPENPMD_PART_DIST>
{

private:

	template<typename part_pmd_type>
	void particlePos(const part_type & part, part_pmd_type & ppmd)
	{
        std::vector<size_t> global_extent({part.size_local()});
        openPMD::Datatype datatype = openPMD::determineDatatype<typename part_type::stype>();
        openPMD::Dataset dataset = openPMD::Dataset(datatype, global_extent);
        auto p = ppmd["position"];
		auto po = ppmd["positionOffset"];

        for (int s = 0 ; s < part_type::dims ; s++)
        {
            std::string comp;

            if (s == 0)
            {comp = "x";}
            else if (s == 1)
            {comp = "y";}
            else if (s == 2)
            {comp = "z";}
            else
            {comp = std::to_string(s);}

            typename part_type::stype * local_data = new typename part_type::stype[part.size_local()];
			typename part_type::stype * local_data_o = new typename part_type::stype[part.size_local()];

		    auto pc = p[comp.c_str()];
			auto pco = po[comp.c_str()];
            pc.resetDataset(dataset);
			pco.resetDataset(dataset);

            // Fill the data
            size_t pi = 0;

            auto it = part.getDomainIterator();

            while (it.isNext())
            {
                auto key = it.get();

                local_data[pi] = part.template getPos(key)[s];
				local_data_o[pi] = 0.0;

                ++pi;
                ++it;
            }

            pc.storeChunk(std::shared_ptr< typename part_type::stype >(local_data),{0}, global_extent);
			pco.storeChunk(std::shared_ptr< typename part_type::stype >(local_data_o),{0}, global_extent);
        }
	}

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
	bool write(const part_type & part, std::string file)
	{
		// open file for writing
		openPMD::Series series = openPMD::Series(
			file,
			openPMD::Access::CREATE,
			MPI_COMM_WORLD
		);

		auto & part_pmd = series.iterations[0].particles["points_ofp"];

		auto prop_names = part.getPropNames();

		particlePos(part,part_pmd);

		prop_out_ppmd<openPMD::Series,
					 typename std::remove_reference<decltype(part_pmd)>::type, 
					 part_type, 
					 typename part_type::stype> pp(series,part_pmd,part,prop_names);

		boost::mpl::for_each_ref<boost::mpl::range_c<int,0,part_type::value_type::max_prop>>(pp);

		series.flush();

		return true;
	}
};


#endif
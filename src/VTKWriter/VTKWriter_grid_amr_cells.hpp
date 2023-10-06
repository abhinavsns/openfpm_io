/*
 * VTKWriter_grids.hpp
 *
 *  Created on: May 5, 2015
 *      Author: Pietro Incardona
 */

#ifndef VTKWRITER_GRID_AMR_HPP_
#define VTKWRITER_GRID_AMR_HPP_

#include <boost/mpl/pair.hpp>
#include "VTKWriter_grids_util.hpp"
#include "is_vtk_writable.hpp"
#include "hash_map/hopscotch_map.h"


/*! \brief this class is a functor for "for_each" algorithm
 *
 * This class is a functor for "for_each" algorithm. For each
 * element of the boost::vector the operator() is called.
 * Is mainly used to produce at output for each property
 *
 * \tparam ele_g element that store the grid and its attributes
 * \param St type of space where the grid live
 *
 */

template<typename AMR_grid_type, typename St>
struct prop_out_ui
{
	//! property output string
	std::string & v_out;

	//! grid that we are processing
	const AMR_grid_type & amr_g;

	//! File type
	file_type ft;

	//! list of names for the properties
	const openfpm::vector<std::string> & prop_names;

	/*! \brief constructor
	 *
	 * \param i patch i
	 * \param v_out string to fill with the vertex properties
	 * \param vg vector of elements to write
	 * \param prop_names properties name
	 * \param ft file type
	 *
	 */
	prop_out_ui(std::string & v_out, const AMR_grid_type & amr_g, const openfpm::vector<std::string> & prop_names ,file_type ft)
	:v_out(v_out),amr_g(amr_g),ft(ft),prop_names(prop_names)
	{};

	/*! It produce an output for each propert
	 *
	 * \param t prop-id
	 *
	 */
    template<typename T>
    void operator()(T& t) const
    {
    	typedef typename boost::mpl::at<typename AMR_grid_type::value_type::type,boost::mpl::int_<T::value>>::type ptype;
    	typedef typename std::remove_all_extents<ptype>::type base_ptype;

    	meta_prop_new_ui<boost::mpl::int_<T::value> ,AMR_grid_type,St, ptype, is_vtk_writable<base_ptype>::value > m(amr_g,v_out,prop_names,ft);
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
template <typename pair>
class VTKWriter<pair,AMR_GRID_UMESH>
{

	/*! \brief Create the VTK point definition
	 *
	 * \param ft file type
	 *
	 * \return the string with the point list
	 *
	 */
    template<typename AMR_grid_type>
	std::string get_point_list(AMR_grid_type & grid, tsl::hopscotch_map<size_t, size_t> & map,size_t & n_points, file_type & opt)
	{
		//! vertex node output string
		std::stringstream v_out;

		v_out<<"      <Points>\n";

        std::stringstream binaryToEncode;
        if (std::is_same<typename pair::second,float>::value == true)
        {
			binaryToEncode << std::setprecision(7);
			if (opt == file_type::ASCII)
			{v_out<<"        <DataArray type=\"Float32\" Name=\"Points\" NumberOfComponents=\"3\" format=\"ascii\">\n";}
			else
			{v_out<<"        <DataArray type=\"Float32\" Name=\"Points\" NumberOfComponents=\"3\" format=\"binary\">\n";}
		}
        else
        {
			binaryToEncode << std::setprecision(16);
			if (opt == file_type::ASCII)
			{v_out<<"        <DataArray type=\"Float64\" Name=\"Points\" NumberOfComponents=\"3\" format=\"ascii\">\n";}
			else
			{v_out<<"        <DataArray type=\"Float64\" Name=\"Points\" NumberOfComponents=\"3\" format=\"binary\">\n";}
		}

		//! For each defined grid
        if (opt == file_type::BINARY)
        {
            size_t tmp=0;
            binaryToEncode.write((const char *)&tmp,sizeof(tmp));
        }

        size_t base = 0;
        size_t cnt = 0;

        int lvl = grid.getNLvl();

        int i = 0;
		for (int i = 0 ; i < lvl ; i++)
		{
			auto it = grid.getDomainIterator(i);

            auto glin_ = grid.getLevel(i).getGridInfo();

            size_t sz[AMR_grid_type::dims];
            for (int j = 0 ; j < AMR_grid_type::dims ; j++)
            {sz[j] = glin_.size(j)+1;}
            grid_sm<AMR_grid_type::dims,typename AMR_grid_type::stype> glin(sz);

			// if there is the next element
			while (it.isNext())
			{
                auto key = it.get();
                auto gkey = it.getGKey(key);
                size_t l = glin.LinId(gkey) + base;

                map[l] = cnt;

				Point<AMR_grid_type::dims,typename AMR_grid_type::stype> p;

				p = grid.getPos(i,key);

				output_point_new<AMR_grid_type::dims,typename AMR_grid_type::stype>(p,binaryToEncode,opt);

				// increment the iterator and counter
                ++cnt;
				++it;
			}

            base += glin.size();
		}

		//! In case of binary we have to add a new line at the end of the list
		if (opt == file_type::BINARY){
		    std::string buffer_out,buffer_bin;
            buffer_bin=binaryToEncode.str();
            *(size_t *)&buffer_bin[0]=buffer_bin.size()-8;
		    buffer_out.resize(buffer_bin.size()/3*4+4);
			unsigned long sz = EncodeToBase64((const unsigned char*)&buffer_bin[0],buffer_bin.size(),(unsigned char*)&buffer_out[0],0);
			buffer_out.resize(sz);
		    v_out << buffer_out<<std::endl;
        }
		else
		{
		    v_out<<binaryToEncode.str();
		}

        n_points = cnt;

        v_out<<"        </DataArray>\n";
        v_out<<"      </Points>\n";
		// return the vertex list
		return v_out.str();
	}

	/*! \brief Create the VTK point definition
	 *
	 * \param ft file type
	 *
	 * \return the string with the point list
	 *
	 */
    template<typename AMR_grid_type>
	std::string get_cell_list(AMR_grid_type & grid, tsl::hopscotch_map<size_t, size_t> & map, size_t & n_cells, file_type & opt)
	{
		//! vertex node output string
		std::stringstream v_out;

		v_out<<"      <Cells>\n";

        std::stringstream binaryToEncode;
        
		binaryToEncode << std::setprecision(7);
		if (opt == file_type::ASCII)
		{v_out<<"        <DataArray type=\"Int32\" Name=\"connectivity\" format=\"ascii\">\n";}
		else
		{v_out<<"        <DataArray type=\"Int32\" Name=\"connectivity\" format=\"binary\">\n";}

		//! For each defined grid
        if (opt == file_type::BINARY)
        {
            size_t tmp=0;
            binaryToEncode.write((const char *)&tmp,sizeof(tmp));
        }

		// cells

		size_t cnt = 0;
        size_t base = 0;
        int lvl = grid.getNLvl();

		for (int i = 0 ; i < lvl ; i++)
		{
			auto it = grid.getDomainIterator(i);

            auto glin_ = grid.getLevel(i).getGridInfo();

            size_t sz[AMR_grid_type::dims];
            for (int i = 0 ; i < AMR_grid_type::dims ; i++)
            {sz[i] = glin_.size(i)+1;}
            grid_sm<AMR_grid_type::dims,typename AMR_grid_type::stype> gsm(sz);

			// if there is the next element
			while (it.isNext())
			{
				auto key = it.get();
                auto gkey = it.getGKey(key);
                size_t l = base+gsm.LinId(gkey);

				if (pair::first::dims == 1)
				{
					if (opt == file_type::ASCII)
					{v_out << map[l] << " " << map[l+1] << std::endl;}
					else
					{
						int tmp;
						tmp = map[l];
						v_out.write((const char *)&tmp,sizeof(int));
						tmp = map[l+1];
						v_out.write((const char *)&tmp,sizeof(int));
					}
					v_out << cnt+gsm.LinId(gkey) << " " << cnt+gsm.LinId(gkey)+1 << std::endl;
				}
				else if (pair::first::dims == 2)
				{
                    if ((map.find(l) != map.end()) && (map.find(l+1) != map.end()) && (map.find(l+gsm.size_s(0)+1) != map.end()) && (map.find(l + gsm.size_s(0)) != map.end()) )
                    {
                        if (opt == file_type::ASCII)
                        {
                            v_out << map[l] << " " << map[l+1] << " " << map[l+gsm.size_s(0)+1] << " " << map[l + gsm.size_s(0)]  << std::endl;
                        }
                        else
                        {
                            int tmp;
                            tmp = map[l];
                            v_out.write((const char *)&tmp,sizeof(int));
                            tmp = map[l+1];
                            v_out.write((const char *)&tmp,sizeof(int));
                            tmp = map[l+gsm.size_s(0)+1];
                            v_out.write((const char *)&tmp,sizeof(int));
                            tmp = map[l + gsm.size_s(0)];
                            v_out.write((const char *)&tmp,sizeof(int));
                        }
                        ++cnt;
                    }
				}
				else
				{
                    // Eliminate
                    if ((map.find(l) != map.end()) && (map.find(l+1) != map.end()) && (map.find(l+gsm.size_s(0)+1) != map.end()) && (map.find(l + gsm.size_s(0)) != map.end()) && (map.find(l+gsm.size_s(1)) != map.end()) && (map.find(l+gsm.size_s(1)+1)  != map.end()) && (map.find(l+gsm.size_s(1)+gsm.size_s(0)+1) != map.end()) && (map.find(l+gsm.size_s(1)+gsm.size_s(0)) != map.end()))
                    {
                        if (opt == file_type::ASCII)
                        {
                            v_out << map[l] << " " << map[l+1] << " " << map[l+gsm.size_s(0)+1] << " " << map[l + gsm.size_s(0)] << " " <<
                                     map[l+gsm.size_s(1)] << " " << map[l+gsm.size_s(1)+1] << " " << map[l+gsm.size_s(1)+gsm.size_s(0)+1] << " " << map[l+gsm.size_s(1)+gsm.size_s(0)] << std::endl;
                        }
                        else
                        {
                            int tmp;
                            tmp = map[l];
                            v_out.write((const char *)&tmp,sizeof(int));
                            tmp = map[l+1];
                            v_out.write((const char *)&tmp,sizeof(int));
                            tmp = map[l+gsm.size_s(0)+1];
                            v_out.write((const char *)&tmp,sizeof(int));
                            tmp = map[l + gsm.size_s(0)];
                            v_out.write((const char *)&tmp,sizeof(int));
                            tmp = map[l+gsm.size_s(1)];
                            v_out.write((const char *)&tmp,sizeof(int));
                            tmp = map[l+gsm.size_s(1)+1];
                            v_out.write((const char *)&tmp,sizeof(int));
                            tmp = map[l+gsm.size_s(1)+gsm.size_s(0)+1];
                            v_out.write((const char *)&tmp,sizeof(int));
                            tmp = map[l+gsm.size_s(1)+gsm.size_s(0)];
                            v_out.write((const char *)&tmp,sizeof(int));
                        }
                        ++cnt;
                    }
				}

				// increment the iterator and counter
				++it;
			}

            base += gsm.size();
		}

		v_out<<"        </DataArray>\n";

		// connectivity

		binaryToEncode << std::setprecision(7);
		if (opt == file_type::ASCII)
		{v_out<<"        <DataArray type=\"Int32\" Name=\"offsets\" format=\"ascii\">\n";}
		else
		{v_out<<"        <DataArray type=\"Int32\" Name=\"offsets\" format=\"binary\">\n";}

		for (int i = 0 ; i < cnt ; i++)
		{
            if (opt == file_type::ASCII)
            {v_out << (i+1)*openfpm::math::pow(2,pair::first::dims) << std::endl;}
            else
            {
                int tmp;
                tmp = i*openfpm::math::pow(2,pair::first::dims);
                v_out.write((const char *)&tmp,sizeof(int));
            }
            // increment the iterator and counter
		}

		v_out<<"        </DataArray>\n";

		// offsets

		if (opt == file_type::ASCII)
		{v_out<<"        <DataArray type=\"Int32\" Name=\"types\" format=\"ascii\">\n";}
		else
		{v_out<<"        <DataArray type=\"Int32\" Name=\"types\" format=\"binary\">\n";}

		for (int i = 0 ; i < cnt ; i++)
		{
            if (opt == file_type::ASCII)
            {
                if (pair::first::dims == 3)
                {v_out << 12 << std::endl;}
                else if (pair::first::dims == 2)
                {v_out << 9 << std::endl;}
                else if (pair::first::dims == 1)
                {v_out << 3 << std::endl;}
            }
            else
            {
                int tmp = 0;
                if (pair::first::dims == 3)
                {tmp = 12;}
                else if (pair::first::dims == 2)
                {tmp = 9;}
                else if (pair::first::dims == 1)
                {tmp = 3;}
                v_out.write((const char *)&tmp,sizeof(int));
            }
		}

		v_out<<"        </DataArray>\n";

		//! In case of binary we have to add a new line at the end of the list
		if (opt == file_type::BINARY)
        {
		    std::string buffer_out,buffer_bin;
            buffer_bin=binaryToEncode.str();
            *(size_t *)&buffer_bin[0]=buffer_bin.size()-8;
		    buffer_out.resize(buffer_bin.size()/3*4+4);
			unsigned long sz = EncodeToBase64((const unsigned char*)&buffer_bin[0],buffer_bin.size(),(unsigned char*)&buffer_out[0],0);
			buffer_out.resize(sz);
		    v_out << buffer_out<<std::endl;
        }
		else
		{
		    v_out<<binaryToEncode.str();
		}
        v_out<<"      </Cells>\n";

        n_cells = cnt;

		// return the vertex list
		return v_out.str();
	}

    public:

	/*!
	 *
	 * VTKWriter constructor
	 *
	 */
	VTKWriter()
	{}


	/*! \brief It write a VTK file from a graph
	 *
	 * \tparam prp_out which properties to output [default = -1 (all)]
	 *
     * \param amr_grid amr grid
	 * \param file path where to write
	 * \param name of the graph
	 * \param prop_names properties name (can also be a vector of size 0)
	 * \param ft specify if it is a VTK BINARY or ASCII file [default = ASCII]
	 *
	 * \return true if the function write successfully
	 *
	 */
	template<int prp = -1, typename AMR_grid_type> bool write(AMR_grid_type & amr_grid,
                                      std::string file,
									  const openfpm::vector<std::string> & prop_names,
									  std::string f_name = "grids",
									  file_type ft = file_type::ASCII)
	{
		// Header for the vtk
		std::string vtk_header;
		// Point list of the VTK
		std::string point_list;
		// Vertex list of the VTK
		std::string vertex_list;
		// Graph header
		std::string vtk_binary_or_ascii;
		// vertex properties header
		std::string point_prop_header;
		// edge properties header
		std::string vertex_prop_header;
		// Data point header
		std::string point_data_header;
		// Data point
		std::string point_data_point;
        std::string point_data_cells;

		// VTK header
		vtk_header = "<VTKFile type=\"UnstructuredGrid\" version=\"1.0\" byte_order=\"LittleEndian\" header_type=\"UInt64\">\n";
        vtk_header +="  <UnstructuredGrid>\n";

        // write the file
        std::ofstream ofs(file);

        ofs << vtk_header;

        tsl::hopscotch_map<size_t, size_t> map;

        size_t n_point = 0;
        size_t n_cells = 0;

		point_data_point.clear();
		point_data_point = get_point_list(amr_grid,map,n_point,ft);

		point_data_cells.clear();
		point_data_cells = get_cell_list(amr_grid,map,n_cells,ft);

        ofs << "      <Piece NumberOfPoints=\"" << std::to_string(n_point) << "\" NumberOfCells=\"" << std::to_string(n_cells) + "\">\n";

		ofs << point_data_point;
		ofs << point_data_cells;

		point_data_point.clear();

		ofs << "        <PointData>\n";

		// For each property in the vertex type produce a point data

		prop_out_ui< AMR_grid_type, typename pair::second > pp(point_data_point, amr_grid, prop_names, ft);

		if (prp == -1)
		{boost::mpl::for_each< boost::mpl::range_c<int,0, pair::first::value_type::max_prop> >(pp);}
		else
		{boost::mpl::for_each< boost::mpl::range_c<int,prp, prp> >(pp);}

		ofs << point_data_point;

		ofs << "    </PointData>\n" <<
				"    <CellData>\n" <<
				"    </CellData>\n" <<
				"  </Piece>\n";

		std::string closingFile="  </UnstructuredGrid>\n</VTKFile>";

		// Check if the file is open
		if (ofs.is_open() == false)
		{std::cerr << "Error cannot create the VTK file: " + file + "\n";}

		ofs << closingFile;

		// Close the file

		ofs.close();

		// Completed succefully
		return true;
	}

};


#endif /* VTKWRITER_GRAPH_HPP_ */
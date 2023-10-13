/*
 * VTKWriter_grids.hpp
 *
 *  Created on: May 5, 2015
 *      Author: Pietro Incardona
 */

#ifndef VTKWRITER_GRIDS_IMAGE_HPP_
#define VTKWRITER_GRIDS_IMAGE_HPP_

#include <boost/mpl/pair.hpp>
#include "VTKWriter_grids_util.hpp"
#include "is_vtk_writable.hpp"

/*! \brief It store one grid
 *
 * \tparam Grid type of grid
 * \tparam St type of space where the grid is defined
 *
 */
template <typename Grid, typename St>
class ele_gi
{
public:

	typedef Grid value_type;

	ele_gi(const Grid & g, 
	       const Point<Grid::dims,St> & offset,
		   const Point<Grid::dims,long int> & offset_i, 
		   const Point<Grid::dims,St> & spacing, 
		   const Box<Grid::dims,long int> & dom)
	:g(g),offset(offset),offset_i(offset_i),spacing(spacing),dom(dom)
	{}

	//! Dataset name
	std::string dataset;
	//! Grid
	const Grid & g;
	//! offset where it start
	Point<Grid::dims,St> offset;
	//! offset where it start
	Point<Grid::dims,long int> offset_i;
	// spacing of the grid
	Point<Grid::dims,St> spacing;
	// Part of the grid that is real domain
	Box<Grid::dims,size_t> dom;
};



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

template<typename ele_g, typename St>
struct prop_out_gi
{
	//! property output string
	std::string & v_out;

	//! grid that we are processing
	const openfpm::vector_std< ele_g > & vg;

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
	prop_out_gi(std::string & v_out, const openfpm::vector_std< ele_g > & vg, const openfpm::vector<std::string> & prop_names ,file_type ft)
	:v_out(v_out),vg(vg),ft(ft),prop_names(prop_names)
	{};

	/*! It produce an output for each propert
	 *
	 * \param t prop-id
	 *
	 */
    template<typename T>
    void operator()(T& t) const
    {
    	typedef typename boost::mpl::at<typename ele_g::value_type::value_type::type,boost::mpl::int_<T::value>>::type ptype;
    	typedef typename std::remove_all_extents<ptype>::type base_ptype;

    	meta_prop_new_i<boost::mpl::int_<T::value> ,ele_g,St, ptype, is_vtk_writable<base_ptype>::value > m(vg,v_out,prop_names,ft);
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
class VTKWriter<pair,VECTOR_GRIDS_UMESH>
{
    //! Global extent of the image
    //Box<pair::first::dims,long int> extent;

    //! Spacing
    //Point<pair::first::dims,typename pair::second> spacing;

	//! Vector of grids

	openfpm::vector< ele_gi<typename pair::first,typename pair::second> > vg;
	/*! \brief Get the total number of points
	 *
	 * \return the total number
	 *
	 */
	size_t get_total()
	{
		size_t tot = 0;

		//! Calculate the full number of vertices
		for (size_t i = 0 ; i < vg.size() ; i++)
		{
			tot += vg.get(i).g.size();
		}
		return tot;
	}


	/*! \brief Get the point data header
	 *
	 * \return a string with the point data header for VTK format
	 *
	 */

	std::string get_point_data_header()
	{
		std::string v_out;

		size_t cnt = 0;
		size_t ccnt = 0;

		for (int i = 0 ; i < vg.size() ; i++)
		{
			//! write the particle position
			auto bx = vg.get(i).dom;
			ccnt += vg.get(i).dom.getVolumeKey();
			bx.enlargeP2(1);
			cnt += bx.getVolumeKey();
		}

		v_out += "      <Piece NumberOfPoints=\"" + std::to_string(cnt) + "\" NumberOfCells=\"" + std::to_string(ccnt) + "\">\n";;

		return v_out;
	}

	/*! \brief Create the VTK point definition
	 *
	 * \param ft file type
	 *
	 * \return the string with the point list
	 *
	 */
	std::string get_point_list(file_type & opt)
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

		for (int i = 0 ; i < vg.size() ; i++)
		{
			//! write the particle position
			auto bx = vg.get(i).dom;
			bx.enlargeP2(1);
			auto it = vg.get(i).g.getIterator(bx.getKP1(),bx.getKP2());

			// if there is the next element
			while (it.isNext())
			{
				Point<pair::first::dims,typename pair::second> p;
				p = it.get().toPoint();

				p = pmul(p,vg.get(i).spacing) + vg.get(i).offset;

				output_point_new<pair::first::dims,typename pair::second>(p,binaryToEncode,opt);

				// increment the iterator and counter
				++it;
			}
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
	std::string get_cell_list(file_type & opt)
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
		for (int i = 0 ; i < vg.size() ; i++)
		{
			//! write the particle position
			auto it = vg.get(i).g.getIterator(vg.get(i).dom.getKP1(),vg.get(i).dom.getKP2());

			size_t sz[pair::first::dims];

			for (int j = 0 ; j < pair::first::dims ; j++)
			{sz[j] = vg.get(i).dom.getKP2().get(j) - vg.get(i).dom.getKP1().get(j) + 2;}

			grid_sm<pair::first::dims,void> gsm(sz);

			// if there is the next element
			while (it.isNext())
			{
				auto key = it.get();
				key -= vg.get(i).dom.getKP1();

				if (pair::first::dims == 1)
				{
					if (opt == file_type::ASCII)
					{v_out << cnt+gsm.LinId(key) << " " << cnt+gsm.LinId(key)+1 << std::endl;}
					else
					{
						int tmp;
						tmp = cnt+gsm.LinId(key);
						v_out.write((const char *)&tmp,sizeof(int));
						tmp = cnt+gsm.LinId(key)+1;
						v_out.write((const char *)&tmp,sizeof(int));
					}
					v_out << cnt+gsm.LinId(key) << " " << cnt+gsm.LinId(key)+1 << std::endl;
				}
				else if (pair::first::dims == 2)
				{
					if (opt == file_type::ASCII)
					{v_out << cnt+gsm.LinId(key) << " " << cnt+gsm.LinId(key)+1 << " " << cnt+gsm.LinId(key)+sz[0] + 1 << " " << cnt+gsm.LinId(key) + sz[0] << std::endl;}
					else
					{
						int tmp;
						tmp = cnt+gsm.LinId(key);
						v_out.write((const char *)&tmp,sizeof(int));
						tmp = cnt+gsm.LinId(key)+1;
						v_out.write((const char *)&tmp,sizeof(int));
						tmp = cnt+gsm.LinId(key) + sz[0] + 1;
						v_out.write((const char *)&tmp,sizeof(int));
						tmp = cnt+gsm.LinId(key) + sz[0];
						v_out.write((const char *)&tmp,sizeof(int));
					}
				}
				else
				{
					if (opt == file_type::ASCII)
					{v_out << cnt+gsm.LinId(key) << " " << cnt+gsm.LinId(key)+1 << " " << cnt+gsm.LinId(key)+gsm.size_s(0)+1 << " " << cnt+gsm.LinId(key) + gsm.size_s(0) << " " <<
				             cnt+gsm.LinId(key)+gsm.size_s(1) << " " << cnt+gsm.LinId(key)+gsm.size_s(1)+1 << " " << cnt+gsm.LinId(key)+gsm.size_s(1)+gsm.size_s(0)+1 << " " << cnt+gsm.LinId(key)+gsm.size_s(1)+gsm.size_s(0) << std::endl;}
					else
					{
						int tmp;
						tmp = cnt+gsm.LinId(key);
						v_out.write((const char *)&tmp,sizeof(int));
						tmp = cnt+gsm.LinId(key)+1;
						v_out.write((const char *)&tmp,sizeof(int));
						tmp = cnt+gsm.LinId(key)+gsm.size_s(0)+1;
						v_out.write((const char *)&tmp,sizeof(int));
						tmp = cnt+gsm.LinId(key) + gsm.size_s(0);
						v_out.write((const char *)&tmp,sizeof(int));
						tmp = cnt+gsm.LinId(key)+gsm.size_s(1);
						v_out.write((const char *)&tmp,sizeof(int));
						tmp = cnt+gsm.LinId(key)+gsm.size_s(1)+1;
						v_out.write((const char *)&tmp,sizeof(int));
						tmp = cnt+gsm.LinId(key)+gsm.size_s(1)+gsm.size_s(0)+1;
						v_out.write((const char *)&tmp,sizeof(int));
						tmp = cnt+gsm.LinId(key)+gsm.size_s(1)+gsm.size_s(0);
						v_out.write((const char *)&tmp,sizeof(int));
					}
				}

				// increment the iterator and counter
				++it;
			}

			auto bx = vg.get(i).dom;
			bx.enlargeP2(1);
			cnt += bx.getVolumeKey();
		}

		v_out<<"        </DataArray>\n";

		// connectivity

		binaryToEncode << std::setprecision(7);
		if (opt == file_type::ASCII)
		{v_out<<"        <DataArray type=\"Int32\" Name=\"offsets\" format=\"ascii\">\n";}
		else
		{v_out<<"        <DataArray type=\"Int32\" Name=\"offsets\" format=\"binary\">\n";}

		cnt = 0;
		for (int i = 0 ; i < vg.size() ; i++)
		{
			//! write the particle position
			auto it = vg.get(i).g.getIterator(vg.get(i).dom.getKP1(),vg.get(i).dom.getKP2());

			// if there is the next element
			while (it.isNext())
			{
				++cnt;

				if (opt == file_type::ASCII)
				{v_out << cnt*openfpm::math::pow(2,pair::first::dims) << std::endl;}
				else
				{
					int tmp;
					tmp = cnt*openfpm::math::pow(2,pair::first::dims);
					v_out.write((const char *)&tmp,sizeof(int));
				}

				// increment the iterator and counter
				++it;
			}
		}

		v_out<<"        </DataArray>\n";

		// offsets

		if (opt == file_type::ASCII)
		{v_out<<"        <DataArray type=\"Int32\" Name=\"types\" format=\"ascii\">\n";}
		else
		{v_out<<"        <DataArray type=\"Int32\" Name=\"types\" format=\"binary\">\n";}

		cnt = 0;
		for (int i = 0 ; i < vg.size() ; i++)
		{
			//! write the particle position
			auto it = vg.get(i).g.getIterator(vg.get(i).dom.getKP1(),vg.get(i).dom.getKP2());

			// if there is the next element
			while (it.isNext())
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

				++cnt;

				// increment the iterator and counter
				++it;
			}
		}

		v_out<<"        </DataArray>\n";

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
        v_out<<"      </Cells>\n";
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

	/*! \brief Add grid dataset
	 *
	 * \param g Grid to add
	 * \param offset grid offset
	 * \param spacing spacing of the grid
	 * \param dom part of the space that is the domain
	 *
	 */
	void add(const typename pair::first & g,
			 const Point<pair::first::dims,typename pair::second> & offset,
			 const Point<pair::first::dims,long int> & offset_i,
			 const Point<pair::first::dims,typename pair::second> & spacing,
			 Box<pair::first::dims,size_t> & dom)
	{
		ele_gi<typename pair::first,typename pair::second> t(g,offset,offset_i,spacing,dom);

		vg.add(t);
	}



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
	template<int prp = -1> bool write(std::string file,
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
		std::string point_data;

		// VTK header
		vtk_header = "<VTKFile type=\"UnstructuredGrid\" version=\"1.0\" byte_order=\"LittleEndian\" header_type=\"UInt64\">\n";
        vtk_header +="  <UnstructuredGrid>\n";

        // write the file
        std::ofstream ofs(file);

        ofs << vtk_header;

		// Get the point data header
		point_data_header = get_point_data_header();

		ofs << point_data_header;

		point_data.clear();
		point_data = get_point_list(ft);

		ofs << point_data;
		point_data.clear();

		point_data = get_cell_list(ft);
		ofs << point_data;
		point_data.clear();

		ofs << "        <PointData>\n";

		// For each property in the vertex type produce a point data

		prop_out_gi< ele_gi<typename pair::first,typename pair::second>, typename pair::second > pp(point_data, vg, prop_names, ft);

		if (prp == -1)
		{boost::mpl::for_each< boost::mpl::range_c<int,0, pair::first::value_type::max_prop> >(pp);}
		else
		{boost::mpl::for_each< boost::mpl::range_c<int,prp, prp> >(pp);}

		ofs << point_data;

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
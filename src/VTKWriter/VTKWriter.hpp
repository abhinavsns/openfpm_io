/*
 * VTKWriter.hpp
 *
 *  Created on: Dec 15, 2014
 *      Author: Pietro Incardona
 */

#ifndef VTKWRITER_HPP_
#define VTKWRITER_HPP_

#include "Graph/map_graph.hpp"
#include <iostream>
#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <fstream>
#include "util/common.hpp"

/*! \brief Get the type Old
 *
 * It convert T to a string identify the corrispondent type in VTK format
 *
 */

template <typename T> std::string getType()
{
    // Create a property string based on the type of the property
    if (std::is_same<T,float>::value)
        return "float";
    else if (std::is_same<T,double>::value)
        return "double";
    else if (std::is_same<T,char>::value)
        return "char";
    else if (std::is_same<T,unsigned char>::value)
        return "unsigned_char";
    else if (std::is_same<T,short>::value)
        return "short";
    else if (std::is_same<T,unsigned short>::value)
        return "unsigned_short";
    else if (std::is_same<T,int>::value)
        return "int";
    else if (std::is_same<T,unsigned int>::value)
        return "unsigned_int";
    else if (std::is_same<T,long int>::value)
        return "int";
    else if (std::is_same<T,unsigned long int>::value )
        return "unsigned_int";
    else if (std::is_same<T,bool>::value )
        return "bit";

    return "";
}



/*! \brief Get the type
 *
 * It convert T to a string identify the corrispondent type in VTK format
 *
 */

template <typename T> std::string getTypeNew()
{
	// Create a property string based on the type of the property
	if (std::is_same<T,float>::value)
		return "Float32";
	else if (std::is_same<T,double>::value)
		return "Float64";
	else if (std::is_same<T,char>::value)
		return "Int8";
	else if (std::is_same<T,unsigned char>::value)
		return "UInt8";
	else if (std::is_same<T,short>::value)
		return "Int16";
	else if (std::is_same<T,unsigned short>::value)
		return "UInt16";
	else if (std::is_same<T,int>::value)
		return "Int32";
	else if (std::is_same<T,unsigned int>::value)
		return "UInt32";
	else if (std::is_same<T,long int>::value)
		return "Int64";
	else if (std::is_same<T,unsigned long int>::value )
		return "UInt64";
	else if (std::is_same<T,bool>::value )
		return "Int8";

	return "";
}

/*! \brief Set a conversion map between A and B
 *
 * Convert A to B
 *
 * \tparam B destination type
 * \tparam A source type
 *
 */

template<typename A>
class convert
{
public:
	template<typename B> static B to(const A & data)
	{
		return static_cast<B>(data);
	}
};

/*! \brief Partial specialization when A is a string
 *
 *
 */

template<>
class convert<std::string>
{
public:
	template<typename B> static B to(const std::string & data)
	{
		return atof(data.c_str());
	}
};

/*! \brief It specify the VTK output file type
 *
 */

enum file_type
{
	BINARY,
	ASCII
};

constexpr int VTK_GRAPH = 1;
constexpr int VECTOR_BOX = 2;
constexpr int VECTOR_GRIDS = 3;
constexpr int VECTOR_ST_GRIDS = 4;
constexpr int DIST_GRAPH = 5;
constexpr int VECTOR_POINTS = 6;
constexpr int VECTOR_GRIDS_UMESH = 7;
constexpr int AMR_GRID_UMESH = 8;
#define VTK_WRITER 0x10000
#define FORMAT_ASCII 0x0
#define FORMAT_BINARY 0x10000000
#define PRINT_GHOST 1

template <typename Object, unsigned int imp>
class VTKWriter
{

};

#include "VTKWriter_graph.hpp"
#include "VTKWriter_vector_box.hpp"
#include "VTKWriter_grids.hpp"
#include "VTKWriter_grids_st.hpp"
#include "VTKWriter_grid_cells.hpp"

// This is only active if MPI compiler work

#ifndef DISABLE_MPI_WRITTERS
#include "VTKWriter_dist_graph.hpp"
#endif

#include "VTKWriter_point_set.hpp"
#include "VTKWriter_grid_amr_cells.hpp"

#endif /* VTKWRITER_HPP_ */

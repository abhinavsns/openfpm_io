//
// Created by jstark on 2019-11-07.
//
/**
 * @file InitGridWithPixel.hpp
 *
 * @brief Header file containing functions for loading pixel values from 2D image or 3D image stack (volume) stored
 * in a binary file onto an openFPM grid.
 *
 * @details Can be run in parallel.
 *
 * @author Justina Stark & Pietro Incardona
 * @date November 2019 - August 2020
 */
#ifndef IMAGE_BASED_RECONSTRUCTION_GETINITIALGRID_HPP
#define IMAGE_BASED_RECONSTRUCTION_GETINITIALGRID_HPP

#include <iostream>
#include <typeinfo>
#include <cmath>
#include <sys/stat.h>

#include "Vector/vector_dist.hpp"
#include "Grid/grid_dist_id.hpp"
#include "data_type/aggregate.hpp"
#include "Decomposition/CartDecomposition.hpp"

#include "level_set/redistancing_Sussman/HelpFunctionsForGrid.hpp"
typedef signed char BYTE;

inline bool exists_test (const std::string& name) {
	struct stat buffer;
	return (stat (name.c_str(), &buffer) == 0);
}



/**@brief Read the number of pixels per dimension from a csv-file in order to create a grid with the same size.
 *
 * @param path_to_file Std::string containing the path to the csv file that holds the image/volume size in
 *                     #pixels per dimension.
 * @return Std::vector containing the count of pixels for each dimension as it was read from the csv file.
 */
std::vector<int> get_size(const std::string & path_to_file)
{
	std::vector<int> stack_dimst_1d;
	//	check if file exists and stream input csv file
	if(!exists_test(path_to_file)){
		std::cout << "------------------------------------------------------------------------" << std::endl;
		std::cout << "Error: file " << path_to_file << " does not exist. Aborting..." << std::endl;
		std::cout << "------------------------------------------------------------------------" << std::endl;
		abort();
	}
	std::ifstream file(path_to_file);


	// get its size
	std::streampos fileSize;
	file.seekg(0, std::ios::end);
	fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	// reserve capacity
	stack_dimst_1d.reserve(fileSize);

	std::string field;
	while ( getline(file, field) )         // 1 field per axis
	{
		std::istringstream iss(field);
		int val;
		iss >> val;
		stack_dimst_1d.push_back(val);       // add the #pixels for current axis to the array
	}

	// reverse order to compensate reverse reading
	std::reverse(std::begin(stack_dimst_1d), std::end(stack_dimst_1d));
	
	return stack_dimst_1d;
}


/**@brief Converts the pixel values stored in a binary file into doubles and loads them onto an OpenFPM grid.
 *
 * @tparam Phi_0 Index of property in which pixel value should be stored.
 * @tparam grid_type Type of OpenFPM grid.
 * @param grid Grid of type grid_type, on which image / image-stack should be loaded.
 * @param file_name Std::string containing the path + name of the binary file that stores the pixel values.
 * @param stack_dims Size_t variable that contains the dimensionality of the image / image stack. Can be given
 *                   manually or loaded from a csv file before (see: #get_size())
 */
template <size_t Phi_0, typename grid_type>
void load_pixel_onto_grid(grid_type & grid, std::string file_name, std::vector <int> & stack_dims)
{
	constexpr size_t x = 0;
	constexpr size_t y = 1;
	constexpr size_t z = 2;
	
	// check if file exists and stream input file
	if(!exists_test(file_name)){
		std::cout << "------------------------------------------------------------------------" << std::endl;
		std::cout << "Error: file " << file_name << " does not exist. Aborting..." << std::endl;
		std::cout << "------------------------------------------------------------------------" << std::endl;
		abort();
	}
	std::ifstream file_stream (file_name, std::ifstream::binary);
	
	auto & v_cl = create_vcluster();
	if (v_cl.rank() == 0)
	{
		for (int d = 0; d < grid_type::dims; d++)
		{
			std::cout << "# grid points in dimension " << d << " = "  << grid.size(d) << std::endl;
		}
	}
	
	// assign pixel values to domain. For each pixel get factor_refinement number of grid points with corresponding value
	auto dom = grid.getDomainIterator();
	std::vector<BYTE> pixel_line; // one x-line of the image stack which will be read
	
	size_t sz_img[grid_type::dims];
	for (int d = 0; d < grid_type::dims; d++)
	{
		sz_img[d] = stack_dims[d];
	}
	grid_sm<grid_type::dims,void> ginfo_image(sz_img); // in order to get the image related key later on
	
	double refinement[grid_type::dims];
	for (int d = 0; d < grid_type::dims; d++)
	{
		refinement[d] = (double) grid.size(d) / (double)stack_dims[d]; // get the factor, by which the grid resolution differs from the image stack resolution
		if (v_cl.rank() == 0) std::cout << "effective refinement in dimension " << d << " = " << refinement[d] << std::endl;
	}
	
	while(dom.isNext())
	{
		auto key = dom.get();
		auto gkey = grid.getGKey(key);
		
		// In case a patch starts within a group of nodes to which same pixel-value should be assigned, get the
		// respective rest-offset
		int rest_offset = (int) (fmod(gkey.get(0), refinement[x])); // get the remainder
		
		
		// get l as the length of one x-line of the original image stack for the specific patch on the processor
		auto & gbox       = grid.getLocalGridsInfo();
		auto & DomBox     = gbox.get(key.getSub()).Dbox;
		size_t patch_size = DomBox.getHigh(0) - DomBox.getLow(0) + 1;
	
		size_t l          = (size_t) ceil( (patch_size + rest_offset) / refinement[x]);
		
		// in case that the grid has a different resolution than the underlying image stack:
		// create a key which is used to get the offset for the file reading
		// the indices in this key are corrected by the refinement factor
		for (int d = 0; d < grid_type::dims; d++)
		{
			gkey.set_d(d, floor(gkey.get(d) / refinement[d]));
		}
		
		// the offset matches the pixel from the image stack to the corresponding current position of the iterator
		// in the grid
		size_t offset = ginfo_image.LinId(gkey);
		file_stream.seekg(offset); // jump to the correct position in the file
		
		// prepare space for the pixel values of one x-line in the original image stack
		pixel_line.resize(l);
		// stream the file and fill pixel values into pixel_line
		file_stream.read((char*) & pixel_line[0], l);
		
		// run over a whole grid-line in x and assign pixel values from pixel_line to grid nodes
		// if the grid is finer in x as the image stack, the same pixel value from pixel_line is
		// assigned refinement[x] times
		for (int k = 0; k < patch_size; ++k)
		{
			auto key = dom.get();
			// get the correct index of the pixel to be read from pixel_line by considering a potential rest-offset,
			// when the patch divides group of nodes that belong to the same pixel
			size_t i = (size_t) floor((k + rest_offset) / refinement[x]);
			grid.template get<Phi_0>(key) = (double) pixel_line[i];
			++dom;
		}
		// now one grid line in x is finished and the iterator dom has advanced accordingly s.t. next loop continues
		// with next line, meaning that y increased by 1 (or z respectivley, when y = ymax + 1)
	}
	
	grid.template ghost_get<Phi_0>();
}



#endif //IMAGE_BASED_RECONSTRUCTION_GETINITIALGRID_HPP

/*
 * OpenPMDWriter.hpp
 *
 *  Created on: Dec 15, 2014
 *      Author: Pietro Incardona
 */

#ifndef OPENPMDWRITER_HPP_
#define OPENPMDWRITER_HPP_


constexpr int OPENPMD_PART_DIST = 2;
constexpr int OPENPMD_GRID_DIST = 3;

constexpr int FORMAT_OPENPMD_HDF5 = 0x1;
constexpr int FORMAT_OPENPMD_ADIOS2 = 0x2;

template <typename Object, unsigned int impl>
class OpenPMD_Writer
{

};

#include "OpenPMD_grids.hpp"
#include "OpenPMD_parts.hpp"

#endif /* VTKWRITER_HPP_ */

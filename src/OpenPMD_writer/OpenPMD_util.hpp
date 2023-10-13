#ifndef OPENPMD_UTIL_HPP_
#define OPENPMD_UTIL_HPP_

//namespace openPMD::auxiliary
//{
//    namespace detail
//    {
//        template< typename T_Value >
//        struct IsContiguousContainer_v< openfpm::vector< T_Value > >
//        {
//            static constexpr bool value = true;
//        };
//    }
//}

template<typename T, typename grid_type, typename mesh_type, typename mesh_component_type>
void set_mesh(grid_type & g, mesh_type & m, mesh_component_type & mc, openPMD::Dataset & dataset)
{
        std::vector<double> goffset;
        std::vector<double> gspacing;
        std::vector<double> gpos;
        std::vector< std::string > lbl;
        
        goffset.resize(grid_type::dims);
        gspacing.resize(grid_type::dims);
        gpos.resize(grid_type::dims);
        for (int i = 0 ; i < grid_type::dims ; i++)
        {
            goffset[i] = g.getDomain().getLow(i);
            gspacing[i] = g.spacing(i);
            gpos[i] = 0;

            std::string comp;

            if (i == 0)
            {comp = "x";}
            else if (i == 1)
            {comp = "y";}
            else if (i == 2)
            {comp = "z";}
            else
            {comp = std::to_string(i);}
            
            lbl.push_back(comp);
        }

        m.setGridGlobalOffset(goffset);
        m.setGridSpacing(gspacing);
        m.setAxisLabels(lbl);
        mc.setPosition(gpos);

        mc.resetDataset(dataset);
}

/*! \brief This class is an helper to create properties output from scalar and compile-time array elements
 *
 * \tparam I It is an boost::mpl::int_ that indicate which property we are writing
 * \tparam ele_g element type that store the grid information
 * \tparam St type of space where the grid live
 * \tparam T the type of the property
 * \tparam is_writable flag that indicate if a property is writable
 *
 */
template<typename I, typename grid_type, typename T, bool is_writable>
struct meta_prop_pmd
{
	/*! \brief Write a vtk compatible type into vtk format
	 *
	 * \param grid OpenPMD node
	 * \param g distributed grid
	 * \param prop_names property names
	 *
	 */
    template<typename openpmd_type>
	inline meta_prop_pmd(openpmd_type & grid, grid_type & g, const openfpm::vector<std::string> & prop_names)
	{
        std::string prop_str;

        if (prop_names.size() < I::value)
        {prop_str += "attr" + std::to_string(I::value);}
        else
        {prop_str = prop_names.get(I::value);}

        openPMD::Extent global_extent;
        global_extent.resize(grid_type::dims);
        for (int i = 0 ; i < grid_type::dims ; i++)
        {global_extent[i] = g.size(i);}

        openPMD::Datatype datatype = openPMD::determineDatatype<T>();
        std::string scalar = openPMD::MeshRecordComponent::SCALAR;
        openPMD::Dataset dataset = openPMD::Dataset(datatype, global_extent);
        openPMD::Mesh m = grid[prop_str.c_str()];
		auto mc = m[scalar.c_str()];

        set_mesh<T>(g,m,mc,dataset);

        // For all patches
        for (int i = 0 ; i < g.getN_loc_grid() ; i++)
        {
            openPMD::Extent chunk_extent;
            openPMD::Offset chunk_offset;

            chunk_extent.resize(grid_type::dims);
            chunk_offset.resize(grid_type::dims);

            auto & gr_i = g.getLocalGridsInfo();

            for (int j = 0 ; j < grid_type::dims; j++)
            {
                chunk_offset[j] = gr_i.get(i).origin.get(j) + gr_i.get(i).Dbox.getLow(j);
                chunk_extent[j] = gr_i.get(i).Dbox.getHigh(j) - gr_i.get(i).Dbox.getLow(j) + 1;
            }

            size_t align = gr_i.get(i).Dbox.getVolumeKey();

            T * local_data = new T[align];

            // Fill the data
            size_t p = 0;

            auto & patch = g.get_loc_grid(i);

            auto it = patch.getIterator(gr_i.get(i).Dbox.getKP1(), gr_i.get(i).Dbox.getKP2());

            while (it.isNext())
            {
                auto key = it.get();

                local_data[p] = patch.template get<I::value>(key);

                ++p;
                ++it;
            }

            mc.storeChunk(std::shared_ptr< T >(local_data), chunk_offset, chunk_extent);
        }
	}
};

/*! \brief This class is an helper to create properties output from scalar and compile-time array elements
 *
 * \tparam I It is an boost::mpl::int_ that indicate which property we are writing
 * \tparam ele_g element type that store the grid information
 * \tparam St type of space where the grid live
 * \tparam T the type of the property
 * \tparam is_writable flag that indicate if a property is writable
 *
 */
template<typename I, typename part_type, typename T, bool is_writable>
struct meta_prop_ppmd
{
	/*! \brief Write a vtk compatible type into vtk format
	 *
	 * \param ppmd OpenPMD node
	 * \param g distributed grid
	 * \param prop_names property names
	 *
	 */
    template<typename openpmd_type>
	inline meta_prop_ppmd(openpmd_type & ppmd, part_type & part, const openfpm::vector<std::string> & prop_names)
	{
        std::string prop_str;

        if (prop_names.size() < I::value)
        {prop_str += "attr" + std::to_string(I::value);}
        else
        {prop_str = prop_names.get(I::value);}

        std::vector<long long unsigned int> global_extent({part.size_local()});
        openPMD::Datatype datatype = openPMD::determineDatatype<T>();
        std::string scalar = openPMD::RecordComponent::SCALAR;
        openPMD::Dataset dataset = openPMD::Dataset(datatype, global_extent);
        auto p = ppmd[prop_str.c_str()];
		auto pc = p[scalar.c_str()];
        pc.resetDataset(dataset);

        T * local_data = new T[part.size_local()];

        // Fill the data
        size_t pi = 0;

        auto it = part.getDomainIterator();

        while (it.isNext())
        {
            auto key = it.get();

            local_data[pi] = part.template getProp<I::value>(key);

            ++pi;
            ++it;
        }

        pc.storeChunk(std::shared_ptr< T >(local_data),{0}, global_extent);
	}
};

//! Partial specialization for N=1 1D-Array
template<typename I, typename grid_type, typename T, size_t N1, bool is_writable>
struct meta_prop_pmd<I, grid_type,T[N1],is_writable>
{
	/*! \brief Write a vtk compatible type into vtk format
	 *
	 * \param vg array of elements to write
	 * \param v_out string containing the string
	 * \param prop_names properties name
	 * \param ft ASCII or BINARY
	 *
	 */
    template<typename openpmd_type>
	inline meta_prop_pmd(openpmd_type & grid, grid_type & g, const openfpm::vector<std::string> & prop_names)
	{
        std::string prop_str;

        if (prop_names.size() < I::value)
        {prop_str += "attr" + std::to_string(I::value);}
        else
        {prop_str = prop_names.get(I::value);}

        std::string scalar = openPMD::MeshRecordComponent::SCALAR;

		openPMD::Datatype datatype = openPMD::determineDatatype<T>();
		openPMD::Extent global_extent;
        global_extent.resize(grid_type::dims);
        for (int i = 0 ; i < grid_type::dims; i++)
        {global_extent[i] = g.size(i);}
		openPMD::Dataset dataset = openPMD::Dataset(datatype, global_extent);

        for (int s = 0 ; s < N1 ; s++)
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

            openPMD::Mesh m = grid[prop_str.c_str()];
            openPMD::MeshRecordComponent mc = m[comp.c_str()];

            set_mesh<T>(g,m,mc,dataset);

            mc.resetDataset(dataset);

            // For all patches
            for (int i = 0 ; i < g.getN_loc_grid() ; i++)
            {
                openPMD::Extent chunk_extent;
                openPMD::Offset chunk_offset;

                chunk_extent.resize(grid_type::dims);
                chunk_offset.resize(grid_type::dims);

                auto & gr_i = g.getLocalGridsInfo();

                for (int j = 0 ; j < grid_type::dims; j++)
                {
                    chunk_offset[j] = gr_i.get(i).origin.get(j) + gr_i.get(i).Dbox.getLow(j);
                    chunk_extent[j] = gr_i.get(i).Dbox.getHigh(j) - gr_i.get(i).Dbox.getLow(j) + 1;
                }

                size_t align = gr_i.get(i).Dbox.getVolumeKey();

                T * local_data = new T[align];

                // Fill the data
                size_t p = 0;

                auto & patch = g.get_loc_grid(i);

                auto it = patch.getIterator(gr_i.get(i).Dbox.getKP1(), gr_i.get(i).Dbox.getKP2());

                while (it.isNext())
                {
                    auto key = it.get();

                    local_data[p] = patch.template get<I::value>(key)[s];

                    ++p;
                    ++it;
                }

                mc.storeChunk(std::shared_ptr< T >(local_data), chunk_offset, chunk_extent);
            }
        }
	}
};


/*! \brief This class is an helper to create properties output from scalar and compile-time array elements
 *
 * \tparam I It is an boost::mpl::int_ that indicate which property we are writing
 * \tparam ele_g element type that store the grid information
 * \tparam St type of space where the grid live
 * \tparam T the type of the property
 * \tparam is_writable flag that indicate if a property is writable
 *
 */
template<typename I, typename part_type, typename T, unsigned int N1, bool is_writable>
struct meta_prop_ppmd<I,part_type,T[N1],is_writable>
{
	/*! \brief Write a vtk compatible type into vtk format
	 *
	 * \param ppmd OpenPMD node
	 * \param g distributed grid
	 * \param prop_names property names
	 *
	 */
    template<typename openpmd_type>
	inline meta_prop_ppmd(openpmd_type & ppmd, part_type & part, const openfpm::vector<std::string> & prop_names)
	{
        std::string prop_str;

        if (prop_names.size() < I::value)
        {prop_str += "attr" + std::to_string(I::value);}
        else
        {prop_str = prop_names.get(I::value);}

        std::vector<long long unsigned int> global_extent({part.size_local()});
        openPMD::Datatype datatype = openPMD::determineDatatype<T>();
        openPMD::Dataset dataset = openPMD::Dataset(datatype, global_extent);
        auto p = ppmd[prop_str.c_str()];

        for (int s = 0 ; s < N1 ; s++)
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

            T * local_data = new T[part.size_local()];

		    auto pc = p[comp.c_str()];
            pc.resetDataset(dataset);

            // Fill the data
            size_t pi = 0;

            auto it = part.getDomainIterator();

            while (it.isNext())
            {
                auto key = it.get();

                local_data[pi] = part.template getProp<I::value>(key)[s];

                ++pi;
                ++it;
            }

            pc.storeChunk(std::shared_ptr< T >(local_data),{0}, global_extent);
        }
	}
};

//! Partial specialization for N=2 2D-Array
template<typename I, typename grid_type ,typename T,size_t N1,size_t N2, bool is_writable>
struct meta_prop_pmd<I, grid_type, T[N1][N2],is_writable>
{

	/*! \brief Write a vtk compatible type into vtk format
	 *
	 * \param vg array of elements to write
	 * \param v_out string containing the string
	 * \param prop_names property names
	 * \param ft ASCII or BINARY
	 *
	 */
	inline meta_prop_pmd(openPMD::Mesh & grid, grid_type & g, const openfpm::vector<std::string> & prop_names)
	{

	}
};


/*! \brief This class is an helper to create properties output from scalar and compile-time array elements
 *
 * \tparam I It is an boost::mpl::int_ that indicate which property we are writing
 * \tparam ele_g element type that store the grid information
 * \tparam St type of space where the grid live
 * \tparam T the type of the property
 * \tparam is_writable flag that indicate if a property is writable
 *
 */
template<typename I, typename part_type, typename T, unsigned int N1, unsigned int N2, bool is_writable>
struct meta_prop_ppmd<I,part_type,T[N1][N2],is_writable>
{
	/*! \brief Write a vtk compatible type into vtk format
	 *
	 * \param ppmd OpenPMD node
	 * \param g distributed grid
	 * \param prop_names property names
	 *
	 */
    template<typename openpmd_type>
	inline meta_prop_ppmd(openpmd_type & ppmd, part_type & part, const openfpm::vector<std::string> & prop_names)
	{
    }
};

#endif

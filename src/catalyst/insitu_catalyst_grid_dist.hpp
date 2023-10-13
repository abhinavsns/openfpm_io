#include "config.h"
#include "insitu_catalyst.hpp"

#ifdef HAVE_CATALYST

#include <catalyst.hpp>
#include <Vector/map_vector.hpp>
#include <boost/mp11.hpp>

/*! \brief this class is a functor for "for_each" algorithm
 *
 */
template<typename grid_type, typename data_p_type, unsigned int ... prp>
struct insitu_prp_out
{
    const openfpm::vector<std::string> & props;

    conduit_cpp::Node & fields;

    data_p_type & data_p;

	typedef typename to_boost_vmpl<prp...>::type vprp;

	__device__ __host__ inline insitu_prp_out(const openfpm::vector<std::string> & props, conduit_cpp::Node & fields, data_p_type & data_p)
	:props(props),fields(fields),data_p(data_p)
	{
	};


	//! It call the copy function for each property
	template<typename T>
	__device__ __host__ inline void operator()(T& t) const
	{
        typedef typename boost::mpl::at<vprp,T>::type prp_val;
        typedef typename boost::mpl::at<typename grid_type::value_type::type,prp_val>::type prp_type;

        if (prp_val::value < props.size())
        {
            // pressure is cell-data.
            fields[props.get(prp_val::value) + "/association"].set("vertex");
            fields[props.get(prp_val::value) + "/topology"].set("mesh");
            fields[props.get(prp_val::value) + "/volume_dependent"].set("false");

            attr_type_set_impl<prp_type>::set(0,data_p.size(),data_p.template get<prp_val::value>(0),props.get(prp_val::value),fields);
        }
        else
        {
            // pressure is cell-data.
            fields["attr" + std::to_string(prp_val::value) + "/association"].set("vertex");
            fields["attr" + std::to_string(prp_val::value) + "/topology"].set("mesh");
            fields["attr" + std::to_string(prp_val::value) + "/volume_dependent"].set("false");

            attr_type_set_impl<prp_type>::set(0,data_p.size(),data_p.template get<prp_val::value>(0),"attr" + std::to_string(prp_val::value),fields);
        }
	}
};

template<unsigned int ... props>
struct vis_props
{};

/*! \brief
 *
 * Implement insitu visualization with catalyst
 * 
 */
template<typename grid_type, unsigned int ... props>
class insitu_viscatalyst<grid_type,vis_props<props ...>,insity_catalyst_implementation::GRID_DIST>
{
    // Sending property object
	typedef object<typename object_creator<typename grid_type::value_type::type, props...>::type> prp_object;

    openfpm::vector<openfpm::vector_soa<prp_object>> data;

public:

    insitu_viscatalyst()
    {}


    void initialize(const std::string & script)
    {
        conduit_cpp::Node node;

        node["catalyst/scripts/script0"].set_string(script);

        catalyst_initialize(conduit_cpp::c_node(&node));
    }

    void initialize(const openfpm::vector<std::string> & script)
    {
        conduit_cpp::Node node;

        for (int i = 0 ; i < script.size() ; i++)
        {
            node["catalyst/scripts/script" + std::to_string(i)].set_string(script.get(i));
        }

        catalyst_initialize(conduit_cpp::c_node(&node));
    }

    void finalize()
    {
        conduit_cpp::Node node;
        catalyst_finalize(conduit_cpp::c_node(&node));
    }

    void execute(grid_type & grid,
            long int cycle = 0, 
            double time = 0)
    {
        // If ghost smaller than one print a warning
        auto ghost = grid.getDecomposition().getGhost();

        for (int i = 0 ; i < grid_type::dims ; i++)
        {
            if (ghost.getHigh(i) < grid.getSpacing()[i])
            {std::cerr << __FILE__ << ":" << __LINE__ << " Warning: ghost must be bigger than one or you will have artifacts on the process boundaries" << std::endl;}
        }

        grid.template ghost_get<props ...>();

        conduit_cpp::Node exec_params;

        // add time/cycle information
        auto state = exec_params["catalyst/state"];
        state["timestep"].set(cycle);
        state["time"].set(time);

        //////////////////////// DO FOR ALL PROPERTIES

        // Add channels.
        // We only have 1 channel here. Let's name it 'grid'.
        auto channel = exec_params["catalyst/channels/grid"];

        ///////////////////////////////////////////////////////////

        // Since this example is using Conduit Mesh Blueprint to define the mesh,
        // we set the channel's type to "mesh".
        channel["type"].set("mesh");

        // now create the mesh.
        auto mesh = channel["data"];

        // start with coordsets (of course, the sequence is not important, just make
        // it easier to think in this order).
        mesh["coordsets/coords/type"].set("uniform");

        data.resize(grid.getN_loc_grid());

        for (int i = 0 ; i < grid.getN_loc_grid() ; i++)
        {
            auto & grid_p = grid.get_loc_grid(i);

            size_t n_pnt = 0;
            auto box = grid.getLocalGridsInfo().get(i).Dbox;
            auto offset = grid.getLocalGridsInfo().get(i).origin;

            for (int i = 0 ; i < grid_type::dims ; i++)
            {
                if (offset.get(i)+box.getHigh(i)+1 != grid.size(i))
                {box.setHigh(i,box.getHigh(i) + 1);}
            }

            auto & grid_loc = grid.get_loc_grid(i);

            auto it = grid_loc.getIterator(box.getKP1(),box.getKP2());

            data.get(i).resize(box.getVolumeKey());

            auto & data_p = data.get(i);

            size_t pnt_c = 0;

            while (it.isNext())
            {
                auto p = it.get();

                object_s_di<decltype(grid_loc.template get_o(p)),decltype(data_p.template get(pnt_c)),OBJ_ENCAP,props...>(grid_loc.template get_o(p),data_p.template get(pnt_c));

                pnt_c++;

                ++it;
            }

            mesh["coordsets/coords/dims/i"].set(box.getHigh(0) - box.getLow(0) + 1);
            mesh["coordsets/coords/dims/j"].set(box.getHigh(1) - box.getLow(1) + 1);
            mesh["coordsets/coords/dims/k"].set(box.getHigh(2) - box.getLow(2) + 1);

            Point<grid_type::dims, typename grid_type::stype> poffset;

            for (int i = 0 ; i < grid_type::dims ; i++)
            {
                poffset.get(i) = (offset.get(i) + box.getLow(i)) * grid.getSpacing()[i];
            }

            mesh["coordsets/coords/origin/x"].set(poffset.get(0));
            mesh["coordsets/coords/origin/y"].set(poffset.get(1));
            mesh["coordsets/coords/origin/z"].set(poffset.get(2));

            mesh["coordsets/coords/spacing/dx"].set(grid.getSpacing()[0]);
            mesh["coordsets/coords/spacing/dy"].set(grid.getSpacing()[1]);
            mesh["coordsets/coords/spacing/dz"].set(grid.getSpacing()[2]);

            // Next, add topology
            mesh["topologies/mesh/type"].set("uniform");
            mesh["topologies/mesh/coordset"].set("coords");

            // Finally, add fields.

            auto fields = mesh["fields"];

            // write the property
            auto & props_n = grid.getPropNames();

            insitu_prp_out<grid_type,openfpm::vector_soa<prp_object>,props ...> ipo(props_n,fields,data_p);

            boost::mpl::for_each_ref< boost::mpl::range_c<int,0,sizeof...(props)> >(ipo);
        }

        //exec_params.print();

        catalyst_execute(conduit_cpp::c_node(&exec_params));
    }

};

#endif
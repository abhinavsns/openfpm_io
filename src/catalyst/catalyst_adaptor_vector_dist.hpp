#include "config.h"
#include "catalyst_adaptor.hpp"

#ifdef HAVE_CATALYST

#include <catalyst.hpp>
#include <Vector/map_vector.hpp>
#include <boost/mp11.hpp>

template <typename vector_type, unsigned int... props>
class catalyst_adaptor<vector_type, vis_props<props...>, catalyst_adaptor_impl::VECTOR_DIST>
{
    // Create typename for (subset of) properties of particle
    typedef object<typename object_creator<typename vector_type::value_type::type, props...>::type> prop_object;
    // SoA storage for (subset of) properties of particles
    openfpm::vector_soa<prop_object> prop_storage;
    // Type of particles coordinates - float/double
    typedef typename vector_type::stype stype;

public:
    catalyst_adaptor()
    {
    }

    // FIXME Extract PARAVIEW_IMPL_DIR at runtime through getenv
    void initialize(const openfpm::vector<std::string> &scripts)
    {
        conduit_cpp::Node node;

        for (int i = 0; i < scripts.size(); i++)
        {
            node["catalyst/scripts/script" + std::to_string(i)].set_string(scripts.get(i));
        }

        // node["catalyst_load/implementation"] = "paraview";
        // node["catalyst_load/search_paths/paraview"] = paraview_impl_dir;

        catalyst_status err = catalyst_initialize(conduit_cpp::c_node(&node));
        if (err != catalyst_status_ok)
        {
            std::cerr << "Failed to initialize Catalyst: " << err << std::endl;
        }
    }

    void finalize()
    {
        conduit_cpp::Node node;
        catalyst_status err = catalyst_finalize(conduit_cpp::c_node(&node));
        if (err != catalyst_status_ok)
        {
            std::cerr << "Failed to finalize Catalyst: " << err << std::endl;
        }
    }

    void execute(vector_type &particles, size_t cycle = 0, double time = 0, const std::string &channel_name = "particles")
    {
        size_t num_particles = particles.size_local();

        // SIMULATION STATE
        conduit_cpp::Node exec_params;
        auto state = exec_params["catalyst/state"];
        state["timestep"].set(cycle);
        state["time"].set(time);

        // MESH
        // FIXME pass channel name as argument
        auto channel = exec_params["catalyst/channels/" + channel_name];
        channel["type"].set_string("mesh");
        auto mesh = channel["data"];
        mesh["coordsets/coords/type"].set_string("explicit");

        // PARTICLES COORDINATES
        // FIXME extract with offset & stride directly from posVector?
        auto pos_vector = particles.getPosVector();
        openfpm::vector<stype> x_coords(num_particles), y_coords(num_particles), z_coords(num_particles);
        for (size_t i = 0; i < num_particles; i++)
        {
            stype *p_coords = pos_vector.template get<0>(i);
            x_coords.get(i) = p_coords[0];
            y_coords.get(i) = p_coords[1];
            z_coords.get(i) = p_coords[2];
        }

        mesh["coordsets/coords/values/x"].set_external((stype *)&x_coords.get(0), x_coords.size(), /*offset=*/0, /*stride=*/sizeof(stype));
        mesh["coordsets/coords/values/y"].set_external((stype *)&y_coords.get(0), y_coords.size(), /*offset=*/0, /*stride=*/sizeof(stype));
        mesh["coordsets/coords/values/z"].set_external((stype *)&z_coords.get(0), z_coords.size(), /*offset=*/0, /*stride=*/sizeof(stype));

        // TOPOLOGY
        mesh["topologies/mesh/type"].set_string("unstructured");
        mesh["topologies/mesh/coordset"].set_string("coords");
        mesh["topologies/mesh/elements/shape"].set_string("point");
        // Connectivity is represented with index array of particles
        std::vector<conduit_int64> connectivity(num_particles);
        std::iota(connectivity.begin(), connectivity.end(), 0);
        mesh["topologies/mesh/elements/connectivity"].set_external(connectivity.data(), connectivity.size());

        // FIELDS
        auto fields = mesh["fields"];
        // Iterate over particles and copy (subset of) their properties into SoA storage
        prop_storage.resize(num_particles);
        auto prop_vector = particles.getPropVector();
        for (size_t i = 0; i < num_particles; i++)
        {
            object_s_di<decltype(prop_vector.template get(i)), decltype(prop_storage.template get(i)), OBJ_ENCAP, props...>(prop_vector.template get(i), prop_storage.template get(i));
        }

        // Iterate over properties and set the respective fields in Conduit node using functor
        auto &prop_names = particles.getPropNames();
        set_prop_val_functor<vector_type, openfpm::vector_soa<prop_object>, props...> prop_setter(prop_names, fields, prop_storage);
        boost::mpl::for_each_ref<boost::mpl::range_c<int, 0, sizeof...(props)>>(prop_setter);

        // CATALYST EXECUTE
        catalyst_status err = catalyst_execute(conduit_cpp::c_node(&exec_params));
        if (err != catalyst_status_ok)
        {
            std::cerr << "Failed to execute Catalyst: " << err << std::endl;
        }
    }
};

#endif
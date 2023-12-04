#include "config.h"
#include "catalyst_adaptor.hpp"

#include <catalyst.hpp>
#include <Vector/map_vector.hpp>
#include <boost/mp11.hpp>

// Catalyst Adaptor for unstructured particles mesh, stored in vector_dist-like data structures.
template <typename vector_type, unsigned int... props>
class catalyst_adaptor<vector_type, vis_props<props...>, catalyst_adaptor_impl::VECTOR_DIST>
{
    // Create typename for (subset of) properties of particle.
    typedef object<typename object_creator<typename vector_type::value_type::type, props...>::type> prop_object;
    // SoA storage for (subset of) properties of particles.
    openfpm::vector_soa<prop_object> prop_storage;
    // Type of particles coordinates (float/double).
    typedef typename vector_type::stype stype;

public:
    catalyst_adaptor()
    {
    }

    void initialize(const openfpm::vector<std::string> &scripts)
    {
        conduit_cpp::Node node;

        for (int i = 0; i < scripts.size(); i++)
        {
            node["catalyst/scripts/script" + std::to_string(i)].set_string(scripts.get(i));
        }

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
        //particles.template map();
        //particles.template ghost_get<props...>();
        size_t num_particles = particles.size_local();

        // Simulation state
        conduit_cpp::Node exec_params;
        auto state = exec_params["catalyst/state"];
        state["timestep"].set(cycle);
        state["time"].set(time);

        // Mesh
        auto channel = exec_params["catalyst/channels/" + channel_name];
        channel["type"].set_string("mesh");
        auto mesh = channel["data"];
        mesh["coordsets/coords/type"].set_string("explicit");

        // Particles coordinates
        auto pos_vector = particles.getPosVector();

        // Coordinates are stored in pos_vector in interleaved fashion:
        // pos_vector = [pos_0.x, pos_0.y, pos_0.z, pos_1.x, pos_1.y, pos_1.z, ..., ].
        // Iterate over pos_vector with stride = 3 * sizeof(stype) and proper offset to extract x/y/z projections.
        if constexpr (vector_type::dims == 3) {
            mesh["coordsets/coords/values/x"].set_external((stype *)&pos_vector.template get<0>(0)[0], num_particles, /*offset=*/0, /*stride=*/3 * sizeof(stype));
            mesh["coordsets/coords/values/y"].set_external((stype *)&pos_vector.template get<0>(0)[0], num_particles, /*offset=*/sizeof(stype), /*stride=*/3 * sizeof(stype));
            mesh["coordsets/coords/values/z"].set_external((stype *)&pos_vector.template get<0>(0)[0], num_particles, /*offset=*/2 * sizeof(stype), /*stride=*/3 * sizeof(stype));
        } else if constexpr (vector_type::dims == 2) {
            mesh["coordsets/coords/values/x"].set_external((stype *)&pos_vector.template get<0>(0)[0], num_particles, /*offset=*/0, /*stride=*/2 * sizeof(stype));
            mesh["coordsets/coords/values/y"].set_external((stype *)&pos_vector.template get<0>(0)[0], num_particles, /*offset=*/sizeof(stype), /*stride=*/2 * sizeof(stype));
        }
        
        // Topology
        mesh["topologies/mesh/type"].set_string("unstructured");
        mesh["topologies/mesh/coordset"].set_string("coords");
        mesh["topologies/mesh/elements/shape"].set_string("point");
        // Connectivity is represented by index array of particles.
        openfpm::vector<conduit_int64> connectivity(num_particles);
        std::iota(connectivity.begin(), connectivity.end(), 0);
        mesh["topologies/mesh/elements/connectivity"].set_external(&connectivity.get(0), connectivity.size());

        // Fields
        auto fields = mesh["fields"];
        // Iterate over particles and copy (subset of) their properties into SoA storage.
        prop_storage.resize(num_particles);
        auto prop_vector = particles.getPropVector();
        for (size_t i = 0; i < num_particles; i++)
        {
            object_s_di<decltype(prop_vector.template get(i)), decltype(prop_storage.template get(i)), OBJ_ENCAP, props...>(prop_vector.template get(i), prop_storage.template get(i));
        }

        // Iterate over properties and pass respective fields values to Conduit node with for-each functor.
        auto &prop_names = particles.getPropNames();
        set_prop_val_functor<vector_type, openfpm::vector_soa<prop_object>, props...> prop_setter(prop_names, fields, prop_storage);
        boost::mpl::for_each_ref<boost::mpl::range_c<int, 0, sizeof...(props)>>(prop_setter);

        // Execute
        catalyst_status err = catalyst_execute(conduit_cpp::c_node(&exec_params));
        if (err != catalyst_status_ok)
        {
            std::cerr << "Failed to execute Catalyst: " << err << std::endl;
        }
    }
};
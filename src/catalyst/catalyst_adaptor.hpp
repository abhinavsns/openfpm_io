#ifndef CATALYST_ADAPTOR
#define CATALYST_ADAPTOR

#include "config.h"

#ifdef HAVE_CATALYST

#include <catalyst.hpp>
#include <Vector/map_vector.hpp>

/* Template for setting properties of particles as fields in Counduit node. */
template <typename T>
struct set_prop_val
{
    static void set(conduit_cpp::Node &fields, const std::string &prop_name, void *prop_storage, size_t num_elements)
    {
        // Skip unsupported properties
    }
};

// Set scalar double property values to Conduit node.
template <>
struct set_prop_val<double>
{
    static void set(conduit_cpp::Node &fields, const std::string &prop_name, double &prop_storage, size_t num_elements)
    {
        fields[prop_name + "/values"].set_external((conduit_float64 *)&prop_storage, num_elements);
    }
};

// Set scalar float property values to Conduit node.
template <>
struct set_prop_val<float>
{
    static void set(conduit_cpp::Node &fields, const std::string &prop_name, double &prop_storage, size_t num_elements)
    {
        fields[prop_name + "/values"].set_external((conduit_float32 *)&prop_storage, num_elements);
    }
};

// Set vec3 double property values to Conduit node.
template <unsigned int N>
struct set_prop_val<double[N]>
{
    template <typename SoA>
    static void set(conduit_cpp::Node &fields, const std::string &prop_name, SoA prop_storage, size_t num_elements)
    {
        if constexpr (N == 3)
        {
            fields[prop_name + "/values/x"].set_external((conduit_float64 *)&prop_storage[0], num_elements, /*offset=*/0, /*stride=*/sizeof(double));
            fields[prop_name + "/values/y"].set_external((conduit_float64 *)&prop_storage[1], num_elements, /*offset=*/0, /*stride=*/sizeof(double));
            fields[prop_name + "/values/z"].set_external((conduit_float64 *)&prop_storage[2], num_elements, /*offset=*/0, /*stride=*/sizeof(double));
        }
    }
};

// Set vec3 float property values to Conduit node.
template <unsigned int N>
struct set_prop_val<float[N]>
{
    template <typename SoA>
    static void set(conduit_cpp::Node &fields, const std::string &prop_name, SoA prop_storage, size_t num_elements)
    {
        if constexpr (N == 3)
        {
            fields[prop_name + "/values/x"].set_external((conduit_float32 *)&prop_storage[0], num_elements, /*offset=*/0, /*stride=*/sizeof(float));
            fields[prop_name + "/values/y"].set_external((conduit_float32 *)&prop_storage[1], num_elements, /*offset=*/0, /*stride=*/sizeof(float));
            fields[prop_name + "/values/z"].set_external((conduit_float32 *)&prop_storage[2], num_elements, /*offset=*/0, /*stride=*/sizeof(float));
        }
    }
};

/*! \brief this class is a functor for "for_each" algorithm
 *
 */
// particles struct - grid_dist / vector_distprp_type
template <typename particles_struct, typename prop_soa, unsigned int... props>
struct set_prop_val_functor
{
    const openfpm::vector<std::string> &prop_names;
    conduit_cpp::Node &fields;
    prop_soa &prop_storage;

    typedef typename to_boost_vmpl<props...>::type vprp;

    __device__ __host__ inline set_prop_val_functor(const openfpm::vector<std::string> &prop_names, conduit_cpp::Node &fields, prop_soa &prop_storage)
        : prop_names(prop_names), fields(fields), prop_storage(prop_storage){};

    // For each property in storage copy it into Conduit node.
    template <typename T>
    __device__ __host__ inline void operator()(T &t) const
    {
        typedef typename boost::mpl::at<vprp, T>::type prp_val;
        typedef typename boost::mpl::at<typename particles_struct::value_type::type, prp_val>::type prp_type;

        auto prop_name = prop_names.get(prp_val::value);

        fields[prop_name + "/association"].set("vertex");
        fields[prop_name + "/topology"].set("mesh");
        fields[prop_name + "/volume_dependent"].set("false");

        set_prop_val<prp_type>::set(fields, prop_name, prop_storage.template get<prp_val::value>(0), prop_storage.size());
    }
};

enum catalyst_adaptor_impl
{
    GRID = 0,
    GRID_DIST = 1,
    VECTOR = 2,
    VECTOR_DIST = 3
};

template <unsigned int... props>
struct vis_props
{
};

/*! \brief
 *
 * Catalyst Adaptor interface for various data storage schemes (structured/unstructured).
 *
 */
template <typename particles_struct, typename vis_props, unsigned int impl>
class catalyst_adaptor
{

public:
    catalyst_adaptor()
    {
    }

    void initialize()
    {
        std::cout << __FILE__ << ":" << __LINE__ << " The implementetion has not been selected or is unknown " << std::endl;
    }

    void finalize()
    {
        std::cout << __FILE__ << ":" << __LINE__ << " The implementetion has not been selected or is unknown " << std::endl;
    }

    void execute(particles_struct &data)
    {
        std::cout << __FILE__ << ":" << __LINE__ << " The implementetion has not been selected or is unknown " << std::endl;
    }
};

#include "catalyst_adaptor_grid_dist.hpp"
#include "catalyst_adaptor_vector_dist.hpp"

#endif

#endif
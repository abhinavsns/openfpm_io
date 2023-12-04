#ifndef CATALYST_ADAPTOR
#define CATALYST_ADAPTOR

#include "config.h"

#include <catalyst.hpp>
#include <Vector/map_vector.hpp>

// Template for setting properties of particles as fields in Counduit node, following Mesh Blueprint.
template <typename... T>
struct set_prop_val
{
    static void set(conduit_cpp::Node &fields, const std::string &prop_name, void *prop_storage_row, size_t num_elements)
    {
        // Skip unsupported properties
    }
};

// Pass scalar double property values to Conduit node.
template <>
struct set_prop_val<double>
{
    /*
    scalar double properties are saved into prop_storage as contiguous double[n] array,
    where n is the number of particles.
    */
    static void set(conduit_cpp::Node &fields, const std::string &prop_name, double &prop_storage_row, size_t num_elements)
    {
        fields[prop_name + "/association"].set("vertex");
        fields[prop_name + "/topology"].set("mesh");
        fields[prop_name + "/volume_dependent"].set("false");
        fields[prop_name + "/values"].set_external((double *)&prop_storage_row, num_elements);
    }
};

// Pass scalar float property values to Conduit node.
template <>
struct set_prop_val<float>
{
    /*
    scalar float properties are saved into prop_storage as contiguous float[n] array,
    where n is the number of particles.
    */
    static void set(conduit_cpp::Node &fields, const std::string &prop_name, float &prop_storage_row, size_t num_elements)
    {
        fields[prop_name + "/association"].set("vertex");
        fields[prop_name + "/topology"].set("mesh");
        fields[prop_name + "/volume_dependent"].set("false");
        fields[prop_name + "/values"].set_external((float *)&prop_storage_row, num_elements);
    }
};

// Pass double[2]/double[3] property values to Conduit node.
template <unsigned int N>
struct set_prop_val<double[N]>
{
    /*
    double[3] properties are saved into prop_storage as multi_array[3].
    It consists of 3 pointers to contiguous double[n] arrays for x/y/z projections:
    prop_storage_row[0] = [val_0.x, val_1.x, ..., val_n.x],
    prop_storage_row[1] = [val_0.y, val_1.y, ..., val_n.y],
    prop_storage_row[2] = [val_0.z, val_1.z, ..., val_n.z],
    where val_i is value of given property for i-th particle.

    Similarly for double[2].
    */
    template <typename multi_array>
    static void set(conduit_cpp::Node &fields, const std::string &prop_name, multi_array prop_storage_row, size_t num_elements)
    {
        fields[prop_name + "/association"].set("vertex");
        fields[prop_name + "/topology"].set("mesh");
        fields[prop_name + "/volume_dependent"].set("false");
        if constexpr (N == 3)
        {
            fields[prop_name + "/values/x"].set_external((double *)&prop_storage_row[0], num_elements, /*offset=*/0, /*stride=*/sizeof(double));
            fields[prop_name + "/values/y"].set_external((double *)&prop_storage_row[1], num_elements, /*offset=*/0, /*stride=*/sizeof(double));
            fields[prop_name + "/values/z"].set_external((double *)&prop_storage_row[2], num_elements, /*offset=*/0, /*stride=*/sizeof(double));
        }
        else if constexpr (N == 2)
        {
            fields[prop_name + "/values/x"].set_external((double *)&prop_storage_row[0], num_elements, /*offset=*/0, /*stride=*/sizeof(double));
            fields[prop_name + "/values/y"].set_external((double *)&prop_storage_row[1], num_elements, /*offset=*/0, /*stride=*/sizeof(double));
        }
    }
};

// Pass float[2]/float[3] property values to Conduit node.
template <unsigned int N>
struct set_prop_val<float[N]>
{
    /*
    float[3] properties are saved into prop_storage as multi_array[3].
    It consists of 3 pointers to contiguous float[n] arrays for x/y/z projections:
    prop_storage_row[0] = [val_0.x, val_1.x, ..., val_n.x],
    prop_storage_row[1] = [val_0.y, val_1.y, ..., val_n.y],
    prop_storage_row[2] = [val_0.z, val_1.z, ..., val_n.z],
    where val_i is value of given property for i-th particle.

    Similarly for float[2].
    */
    template <typename multi_array>
    static void set(conduit_cpp::Node &fields, const std::string &prop_name, multi_array prop_storage_row, size_t num_elements)
    {
        fields[prop_name + "/association"].set("vertex");
        fields[prop_name + "/topology"].set("mesh");
        fields[prop_name + "/volume_dependent"].set("false");
        if constexpr (N == 3)
        {
            fields[prop_name + "/values/x"].set_external((float *)&prop_storage_row[0], num_elements, /*offset=*/0, /*stride=*/sizeof(float));
            fields[prop_name + "/values/y"].set_external((float *)&prop_storage_row[1], num_elements, /*offset=*/0, /*stride=*/sizeof(float));
            fields[prop_name + "/values/z"].set_external((float *)&prop_storage_row[2], num_elements, /*offset=*/0, /*stride=*/sizeof(float));
        }
        else if constexpr (N == 2)
        {
            fields[prop_name + "/values/x"].set_external((float *)&prop_storage_row[0], num_elements, /*offset=*/0, /*stride=*/sizeof(float));
            fields[prop_name + "/values/y"].set_external((float *)&prop_storage_row[1], num_elements, /*offset=*/0, /*stride=*/sizeof(float));
        }
    }
};

// Pass VectorS<2 / 3, double> property values to Conduit node.
template <unsigned int N>
struct set_prop_val<VectorS<N, double>>
{
    /*
    VectorS<3, double> properties are saved into prop_storage as sequence of Point<3, double> objects.
    x/y/z projections are stored in interleaved fashion:
    prop_storage_row = [val_0.x, val_0.y, val_0.z, val_1.x, val_1.y, val_1.z, ..., ]
    where val_i is value of given property for i-th particle.
    Iterate over prop_storage_row with stride = 3 * sizeof(double) and proper offset to extract x/y/z projections.

    Similarly for VectorS<2, double>.
    */
    static void set(conduit_cpp::Node &fields, const std::string &prop_name, VectorS<N, double> &prop_storage_row, size_t num_elements)
    {
        fields[prop_name + "/association"].set("vertex");
        fields[prop_name + "/topology"].set("mesh");
        fields[prop_name + "/volume_dependent"].set("false");
        if constexpr (N == 3)
        {
            fields[prop_name + "/values/x"].set_external((double *)&prop_storage_row[0], num_elements, /*offset=*/0, /*stride=*/3 * sizeof(double));
            fields[prop_name + "/values/y"].set_external((double *)&prop_storage_row[0], num_elements, /*offset=*/sizeof(double), /*stride=*/3 * sizeof(double));
            fields[prop_name + "/values/z"].set_external((double *)&prop_storage_row[0], num_elements, /*offset=*/2 * sizeof(double), /*stride=*/3 * sizeof(double));
        }
        else if constexpr (N == 2)
        {
            fields[prop_name + "/values/x"].set_external((double *)&prop_storage_row[0], num_elements, /*offset=*/0, /*stride=*/2 * sizeof(double));
            fields[prop_name + "/values/y"].set_external((double *)&prop_storage_row[0], num_elements, /*offset=*/sizeof(double), /*stride=*/2 * sizeof(double));
        }
    }
};

// Pass VectorS<2 / 3, float> property values to Conduit node.
template <unsigned int N>
struct set_prop_val<VectorS<N, float>>
{
    /*
    VectorS<3, float> properties are saved into prop_storage as sequence of Point<3, float> objects.
    x/y/z projections are stored in interleaved fashion:
    prop_storage_row = [val_0.x, val_0.y, val_0.z, val_1.x, val_1.y, val_1.z, ..., ]
    where val_i is value of given property for i-th particle.
    Iterate over prop_storage_row with stride = 3 * sizeof(float) and proper offset to extract x/y/z projections.

    Similarly for VectorS<2, float>.
    */
    static void set(conduit_cpp::Node &fields, const std::string &prop_name, VectorS<N, float> &prop_storage_row, size_t num_elements)
    {
        fields[prop_name + "/association"].set("vertex");
        fields[prop_name + "/topology"].set("mesh");
        fields[prop_name + "/volume_dependent"].set("false");
        if constexpr (N == 3)
        {
            fields[prop_name + "/values/x"].set_external((float *)&prop_storage_row[0], num_elements, /*offset=*/0, /*stride=*/3 * sizeof(float));
            fields[prop_name + "/values/y"].set_external((float *)&prop_storage_row[0], num_elements, /*offset=*/sizeof(float), /*stride=*/3 * sizeof(float));
            fields[prop_name + "/values/z"].set_external((float *)&prop_storage_row[0], num_elements, /*offset=*/2 * sizeof(float), /*stride=*/3 * sizeof(float));
        }
        else if constexpr (N == 2)
        {
            fields[prop_name + "/values/x"].set_external((float *)&prop_storage_row[0], num_elements, /*offset=*/0, /*stride=*/2 * sizeof(float));
            fields[prop_name + "/values/y"].set_external((float *)&prop_storage_row[0], num_elements, /*offset=*/sizeof(float), /*stride=*/2 * sizeof(float));
        }
    }
};

// Pass double[N][M] tensor property values to Conduit node, as N*M scalar fields.
template <unsigned int N, unsigned int M>
struct set_prop_val<double[N][M]>
{
    /*
    double[N][M] properties are saved into prop_storage as multi_array[N][M].
    It consists of N*M pointers to contiguous double[n] arrays for respective tensor indices projections:
    prop_storage_row[0][0] = [val_0.0_0, val_1.0_0, ..., val_n.0_0],
    prop_storage_row[0][1] = [val_0.0_1, val_1.0_1, ..., val_n.0_1],
    ...
    prop_storage_row[N-1][M-1] = [val_0.N-1_M-1, val_1.N-1_M-1, ..., val_n.N-1_M-1],
    where val_i is value of given property for i-th particle.

    Tensor is unrolled into N*M scalar properties, named "{tensor}_{i}_{j}".
    */
    template <typename multi_array>
    static void set(conduit_cpp::Node &fields, const std::string &prop_name, multi_array prop_storage_row, size_t num_elements)
    {
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < M; j++)
            {
                std::string prop_name_ij = prop_name + "_" + std::to_string(i) + "_" + std::to_string(j);
                fields[prop_name_ij + "/association"].set("vertex");
                fields[prop_name_ij + "/topology"].set("mesh");
                fields[prop_name_ij + "/volume_dependent"].set("false");
                fields[prop_name_ij + "/values"].set_external((double *)&prop_storage_row[i][j], num_elements, /*offset=*/0, /*stride=*/sizeof(double));
            }
        }
    }
};

// Pass float[N][M] tensor property values to Conduit node, as N*M scalar fields.
template <unsigned int N, unsigned int M>
struct set_prop_val<float[N][M]>
{
    /*
    Memory storage schema:

    float[N][M] properties are saved into prop_storage as multi_array[N][M].
    It consists of N*M pointers to contiguous float[n] arrays for respective tensor indices projections:
    prop_storage_row[0][0] = [val_0.0_0, val_1.0_0, ..., val_n.0_0],
    prop_storage_row[0][1] = [val_0.0_1, val_1.0_1, ..., val_n.0_1],
    ...
    prop_storage_row[N-1][M-1] = [val_0.N-1_M-1, val_1.N-1_M-1, ..., val_n.N-1_M-1],
    where val_i is value of given property for i-th particle.

    Tensor is unrolled into N*M scalar properties, named "{tensor}_{i}_{j}".
    */
    template <typename multi_array>
    static void set(conduit_cpp::Node &fields, const std::string &prop_name, multi_array prop_storage_row, size_t num_elements)
    {
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < M; j++)
            {
                std::string prop_name_ij = prop_name + "_" + std::to_string(i) + "_" + std::to_string(j);
                fields[prop_name_ij + "/association"].set("vertex");
                fields[prop_name_ij + "/topology"].set("mesh");
                fields[prop_name_ij + "/volume_dependent"].set("false");
                fields[prop_name_ij + "/values"].set_external((float *)&prop_storage_row[i][j], num_elements, /*offset=*/0, /*stride=*/sizeof(float));
            }
        }
    }
};

// Pass double[N][M][L] tensor property values to Conduit node, as N*M*L scalar fields.
template <unsigned int N, unsigned int M, unsigned int L>
struct set_prop_val<double[N][M][L]>
{
    /*
    double[N][M][L] properties are saved into prop_storage as multi_array[N][M][L].
    It consists of N*M*L pointers to contiguous double[n] arrays for respective tensor indices projections:
    prop_storage_row[0][0][0] = [val_0.0_0_0, val_1.0_0_0, ..., val_n.0_0_0],
    prop_storage_row[0][0][1] = [val_0.0_0_1, val_1.0_0_1, ..., val_n.0_0_1],
    ...
    prop_storage_row[N-1][M-1][L-1] = [val_0.N-1_M-1_L-1, val_1.N-1_M-1_L-1, ..., val_n.N-1_M-1_L-1],
    where val_i is value of given property for i-th particle.

    Tensor is unrolled into N*M*L scalar properties, named "{tensor}_{i}_{j}_{k}".
    */
    template <typename multi_array>
    static void set(conduit_cpp::Node &fields, const std::string &prop_name, multi_array prop_storage_row, size_t num_elements)
    {
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < M; j++)
            {
                for (int k = 0; k < L; k++)
                {
                    std::string prop_name_ijk = prop_name + "_" + std::to_string(i) + "_" + std::to_string(j) + "_" + std::to_string(k);
                    fields[prop_name_ijk + "/association"].set("vertex");
                    fields[prop_name_ijk + "/topology"].set("mesh");
                    fields[prop_name_ijk + "/volume_dependent"].set("false");
                    fields[prop_name_ijk + "/values"].set_external((double *)&prop_storage_row[i][j][k], num_elements, /*offset=*/0, /*stride=*/sizeof(double));
                }
            }
        }
    }
};

// Pass float[N][M][L] tensor property values to Conduit node, as N*M*L scalar fields.
template <unsigned int N, unsigned int M, unsigned int L>
struct set_prop_val<float[N][M][L]>
{
    /*
    float[N][M][L] properties are saved into prop_storage as multi_array[N][M][L].
    It consists of N*M*L pointers to contiguous float[n] arrays for respective tensor indices projections:
    prop_storage_row[0][0][0] = [val_0.0_0_0, val_1.0_0_0, ..., val_n.0_0_0],
    prop_storage_row[0][0][1] = [val_0.0_0_1, val_1.0_0_1, ..., val_n.0_0_1],
    ...
    prop_storage_row[N-1][M-1][L-1] = [val_0.N-1_M-1_L-1, val_1.N-1_M-1_L-1, ..., val_n.N-1_M-1_L-1],
    where val_i is value of given property for i-th particle.

    Tensor is unrolled into N*M*L scalar properties, named "{tensor}_{i}_{j}_{k}".
    */
    template <typename multi_array>
    static void set(conduit_cpp::Node &fields, const std::string &prop_name, multi_array prop_storage_row, size_t num_elements)
    {
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < M; j++)
            {
                for (int k = 0; k < L; k++)
                {
                    std::string prop_name_ijk = prop_name + "_" + std::to_string(i) + "_" + std::to_string(j) + "_" + std::to_string(k);
                    fields[prop_name_ijk + "/association"].set("vertex");
                    fields[prop_name_ijk + "/topology"].set("mesh");
                    fields[prop_name_ijk + "/volume_dependent"].set("false");
                    fields[prop_name_ijk + "/values"].set_external((float *)&prop_storage_row[i][j][k], num_elements, /*offset=*/0, /*stride=*/sizeof(float));
                }
            }
        }
    }
};

// For-each functor, applied to each property in prop_storage to pass its values array to Conduit node.
template <typename particles_struct, typename prop_soa, unsigned int... props>
struct set_prop_val_functor
{
    const openfpm::vector<std::string> &prop_names;
    conduit_cpp::Node &fields;
    // prop_storage is heterogeneous vector_soa<prop_object>
    // storing properties values of particles in SoA fashion;
    // prop_object is list of chosen properties types.
    prop_soa &prop_storage;

    typedef typename to_boost_vmpl<props...>::type vprp;

    __device__ __host__ inline set_prop_val_functor(const openfpm::vector<std::string> &prop_names, conduit_cpp::Node &fields, prop_soa &prop_storage)
        : prop_names(prop_names), fields(fields), prop_storage(prop_storage){};

    template <typename T>
    __device__ __host__ inline void operator()(T &t) const
    {
        typedef typename boost::mpl::at<vprp, T>::type prp_val;
        typedef typename boost::mpl::at<typename particles_struct::value_type::type, prp_val>::type prp_type;

        std::string prop_name;
        if (prp_val::value < prop_names.size())
        {
            prop_name = prop_names.get(prp_val::value);
        }
        // Unnamed properties
        else
        {
            prop_name = "attr" + std::to_string(prp_val::value);
        }

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
 * Catalyst Adaptor interface. Specializations define how to translate particle mesh into Conduit node format.
 *
 */
template <typename particles_struct, typename vis_props, unsigned int impl>
class catalyst_adaptor
{

public:
    catalyst_adaptor()
    {
    }

    // Initialize Catalyst, pass visualization pipeline scripts from ParaView.
    void initialize()
    {
        std::cout << __FILE__ << ":" << __LINE__ << " The implementetion has not been selected or is unknown " << std::endl;
    }

    // Finalize Catalyst.
    void finalize()
    {
        std::cout << __FILE__ << ":" << __LINE__ << " The implementetion has not been selected or is unknown " << std::endl;
    }

    // Catalyst in-situ visualization, called by either catalyst_execute in the script
    // or triggered by defined trigger, e.g. each (k-th) timestep.
    // Translates simulation data structures into Conduit Mesh Blueprint format.
    void execute(particles_struct &data)
    {
        std::cout << __FILE__ << ":" << __LINE__ << " The implementetion has not been selected or is unknown " << std::endl;
    }
};

#include "catalyst_adaptor_grid_dist.hpp"
#include "catalyst_adaptor_vector_dist.hpp"

#endif